/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
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
#include "GPIO_Interface.h"

#ifdef MC1322X
    #include "MacaInterrupt.h"
    #include "TransceiverConfigMngmnt.h"
	#include "Leds.h"
#elif
	#include "CPU.h"
	#include "keyboardTask.h"
	#include "keyboard.h"
#endif

// --------------------------------------------------------------------------
// Globals

portTickType		gLastPacketReceivedTick;
extern gwBoolean 	gSDCardBusConnected;
extern gwBoolean 	gSDCardPwrConnected;

// --------------------------------------------------------------------------

void vMain( void ) {

	GpioErr_t error;

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
	ITC_Init();
	IntAssignHandler(gMacaInt_c, (IntHandlerFunc_t) MACA_Interrupt);
	ITC_SetPriority(gMacaInt_c, gItcFastPriority_c); // gItcNormalPriority_c
	ITC_EnableInterrupt(gMacaInt_c);
	IntDisableAll();
	ResetMaca();
	MLMERadioInit();
	IntEnableAll();
	TmrInit();

	LED_Init();

	// Setup our LED, Bus switch, and VCC switch.
	error = Gpio_SetPinFunction(gGpioPin22_c, gGpioNormalMode_c);
	error = Gpio_SetPinFunction(gGpioPin23_c, gGpioNormalMode_c);
	error = Gpio_SetPinFunction(gGpioPin29_c, gGpioNormalMode_c);
	error = Gpio_SetPinFunction(gGpioPin36_c, gGpioNormalMode_c);
	BUS_SW_INIT;
	BUS_SW_ON;
	VCC_SW_INIT;
	VCC_SW_ON;

	crmCopCntl_t copCntl;
	copCntl.bit.copEn = FALSE;
	copCntl.bit.copTimeOut = 127;
	copCntl.bit.copWP = TRUE;
	copCntl.bit.copOut = 0;
	CRM_CopCntl(copCntl);

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
	gPFCQueue = xQueueCreate(PFC_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));

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
	if (xTaskGetTickCount() > (gLastPacketReceivedTick + kNetCheckTickCount)) {
		GW_RESET_MCU;
	}
}
