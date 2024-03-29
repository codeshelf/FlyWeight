/*
   FlyWeight
   � Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved

   $Id$
   $Name$
*/

// --------------------------------------------------------------------------
// Kernel includes

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "pfcSPITask.h"
#include "spi.h"
#include "commands.h"

#ifdef MC1322X
    #include "MacaInterrupt.h"
    #include "TransceiverConfigMngmnt.h"
	#include "Leds.h"
	#include "Delay.h"
	#include "PortConfig.h"
	#include "GPIO_Interface.h"
#elif
	#include "CPU.h"
	#include "keyboardTask.h"
	#include "keyboard.h"
#endif

#define PA_VREG_CONTROL			gGpioPin42_c


// --------------------------------------------------------------------------
// Globals

portTickType		gLastPacketReceivedTick;
extern gwBoolean 	gSDCardBusConnected;
extern gwBoolean 	gSDCardVccConnected;

// --------------------------------------------------------------------------

void vMain( void ) {

#if defined(MC1321X) || defined(MC13192EVB)
	// These got moved into PE pre-init, so that the RTI clock can use the EXTAL.
	// The EXTAL has to be setup and reserved BEFORE initialization of the ICG in PE init.
	//MCUInit();
	//MC13192Init();
	//MLMESetMC13192ClockRate(0);

	// Setup the SMAC glue.
	initSMACRadioQueueGlue(gRadioReceiveQueue);

	#ifdef XBEE
		xbeeInit();
	#endif
#else

	GpioErr_t error;

	// We don't call this, because we don't want to mess with DPF settings at restart.
	//PlatformPortInit();
	// Setup the radio GPIOs. (Don't know why SMAC doesn't do this.)
	GPIO_REGS_P->FuncSel2 = gFuncSel2Value_c;

	ITC_Init();
	IntAssignHandler(gMacaInt_c, (IntHandlerFunc_t) MACA_Interrupt);
	ITC_SetPriority(gMacaInt_c, gItcFastPriority_c); // gItcNormalPriority_c
	ITC_EnableInterrupt(gMacaInt_c);
	IntDisableAll();
	ResetMaca();
	MLMERadioInit();
	SetDemulatorMode(NCD);

	// The PA's Vreg needs to be "on" always. (Controlled by GPIO42.)
	ConfigureRfCtlSignals(gRfSignalANT1_c, gRfSignalFunctionGPIO_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalANT2_c, gRfSignalFunctionGPIO_c, TRUE, TRUE);

	IntEnableAll();
	TmrInit();

	LED_Init();

	// Setup our LEDs.
	error = Gpio_SetPinFunction(gGpioPin22_c, gGpioNormalMode_c);
	error = Gpio_SetPinFunction(gGpioPin23_c, gGpioNormalMode_c);

	// Disable the SPI mode for the pins that the SDCard bus will use.
	disableSPI();

	// Setup the DAT0 pull-up for SDCard mode.
	error = Gpio_SetPinFunction(SD_DAT0_PULLUP, gGpioNormalMode_c);
	error = Gpio_SetPinDir(SD_DAT0_PULLUP, gGpioDirIn_c);
	error = Gpio_EnPinPullup(SD_DAT0_PULLUP, FALSE);

	// Setup the Bus switch.
	error = Gpio_SetPinFunction(BUS_SW_GPIO, gGpioNormalMode_c);
	error = Gpio_SetPinFunction(BUS_CARD_DETECT_GPIO, gGpioNormalMode_c);
	error = Gpio_SetPinDir(BUS_SW_GPIO, gGpioDirOut_c);

	// Setup the Vcc switch.
	error = Gpio_SetPinFunction(VCC_SW_GPIO, gGpioNormalMode_c);
	error = Gpio_SetPinDir(VCC_SW_GPIO, gGpioDirOut_c);
	error = Gpio_SetPinFunction(VCC_HELPER_GPIO, gGpioNormalMode_c);\

	// Turn the bus and the Vcc off.
	BUS_SW_OFF;
	VCC_SW_OFF;

	// Turn the bus and the Vcc on.
//	CARD_UNINSERTED;
	BUS_SW_ON;
	VCC_SW_ON;
	DelayMs(20);
	CARD_INSERTED;

	// Special SD-to-SPI-to-SD test.
//	BUS_SW_OFF;
//	VCC_SW_ON;
//	enableSPI();
//	VCC_SW_OFF;
//	DelayMs(50);
//	VCC_SW_ON;
//	enableSDCardBus();
//	disableSPI();
//	BUS_SW_ON;
	// End test

	setupWatchdog();
#endif

	gLocalDeviceState = eLocalStateStarted;
	GW_RADIO_GAIN_ADJUST(15);
	if (GW_SET_RADIO_CHANNEL(0) == GW_SMAC_SUCCESS)
		{}

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	//xTaskCreate(keyboardTask, (const signed portCHAR * const) "Keyboard", configMINIMAL_STACK_SIZE, NULL, KEYBOARD_PRIORITY, &gKeyboardTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask );
	xTaskCreate(pfcTask, (const signed portCHAR * const) "PFC", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gPFCTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	//gKeyboardQueue = xQueueCreate(KEYBOARD_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	gPFCQueue = xQueueCreate(PFC_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(SDControlCommandStruct));

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();

	/* All the tasks have been created - start the scheduler. */
	vTaskStartScheduler();

	/* Should not reach here! */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vApplicationIdleHook( void ) {

	GW_WATCHDOG_RESET;

	// If we haven't received a packet in by timeout seconds then reset.
	portTickType ticks = xTaskGetTickCount();
	if (ticks > (gLastPacketReceivedTick + kNetCheckTickCount)) {
		GW_RESET_MCU();
	}
}
