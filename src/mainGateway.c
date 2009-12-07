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
#include "gatewayRadioTask.h"
#include "gatewayMgmtTask.h"
#include "commands.h"
#ifdef MC1322X
    #include "MacaInterrupt.h"
    #include "TransceiverConfigMngmnt.h"
	#include "Leds.h"
#endif

// --------------------------------------------------------------------------
// Globals

extern void ResetMaca(void);

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

#else
	ITC_Init();
	IntAssignHandler(gMacaInt_c, (IntHandlerFunc_t) MACA_Interrupt);
	ITC_SetPriority(gMacaInt_c, gItcFastPriority_c); // gItcNormalPriority_c
	ITC_EnableInterrupt(gMacaInt_c);
	IntDisableAll();
	ResetMaca();
	MLMERadioInit();
	IntEnableAll();

	LED_Init();

	crmCopCntl_t copCntl;
	copCntl.bit.copEn = TRUE;
	copCntl.bit.copTimeOut = 127;
	copCntl.bit.copWP = TRUE;
	copCntl.bit.copOut = 0;
	CRM_CopCntl(copCntl);

#endif

	gControllerState = eControllerStateInit;
	GW_RADIO_GAIN_ADJUST(15);

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	xTaskCreate(serialReceiveTask, (const signed portCHAR * const) "SerialRecv", configMINIMAL_STACK_SIZE, NULL, SERIAL_RECV_PRIORITY, &gSerialReceiveTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));

	// Set the state to running
	gControllerState = eControllerStateRun;

	/* All the tasks have been created - start the scheduler. */
	vTaskStartScheduler();

	/* Should not reach here! */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vApplicationIdleHook( void ) {
	GW_WATCHDOG_RESET;
}
