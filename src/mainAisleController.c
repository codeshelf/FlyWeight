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
#include "aisleControllerTask.h"
#include "scannerReadTask.h"
#include "spi.h"
#include "commands.h"
#include "PortConfig.h"
#include "ghdr/maca.h"
#include "Delay.h"

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

// --------------------------------------------------------------------------
// Globals

portTickType	gLastPacketReceivedTick;

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
	MLMERadioInit();
	ResetMaca();
	
	// Setup the CEL Freestar radio controls for PA and Tx/Rx.
	SetDemulatorMode(NCD);

	// The PA's Vreg needs to be "on" always. (Controlled by GPIO42.)
	SetPowerLevelLockMode(TRUE);
	ConfigureRfCtlSignals(gRfSignalANT1_c, gRfSignalFunctionGPIO_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalANT2_c, gRfSignalFunctionGPIO_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalTXON_c, gRfSignalFunction1_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalRXON_c, gRfSignalFunction1_c, TRUE, TRUE);
	SetComplementaryPAState(TRUE);

	IntEnableAll();
	LED_Init();

	setupWatchdog();

#endif

	gLocalDeviceState = eLocalStateStarted;
	GW_RADIO_POWER_ADJUST(0x0c);
	GW_SET_RADIO_CHANNEL(0);
	// FIne turne radios that need it.
	//set_xtal_fine_tune(0x10);
	maca_random = (gwUINT32) ((GUID[4]<<3) + (GUID[5]<<2) + (GUID[6]<<1) + GUID[7]);
	DelayMs(maca_random & 0xff);

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	//xTaskCreate(keyboardTask, (const signed portCHAR * const) "Keyboard", configMINIMAL_STACK_SIZE, NULL, KEYBOARD_PRIORITY, &gKeyboardTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask );
	xTaskCreate(aisleControllerTask, (const signed portCHAR * const) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gAisleControllerTask );
	//xTaskCreate(scannerReadTask, (const signed portCHAR * const) "Scan", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gScannerReadTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	//gKeyboardQueue = xQueueCreate(KEYBOARD_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	//gAisleControllerQueue = xQueueCreate(AISLE_CONTROLLER_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(SDControlCommandStruct));

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

//	GW_WATCHDOG_RESET;
//
//	// If we haven't received a packet in by timeout seconds then reset.
//	portTickType ticks = xTaskGetTickCount();
//	if (ticks > (gLastPacketReceivedTick + kNetCheckTickCount)) {
//		GW_RESET_MCU();
//	}
//	//vTaskDelay(100);
}
