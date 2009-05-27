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
#include "pub_def.h"
#include "smac_config.h"
#include "mcu_hw_config.h"
#include "MC13192_hw_config.h"
#include "simple_mac.h"
#include "xbeeinit.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "hooBeeTask.h"
#include "commands.h"
#include "CPU.h"
#include "keyboardTask.h"
#include "keyboard.h"

// --------------------------------------------------------------------------
// Globals

// --------------------------------------------------------------------------

void vMain( void ) {
	// These got moved into PE pre-init, so that the RTI clock can use the EXTAL.
	// The EXTAL has to be setup and reserved BEFORE initialization of the ICG in PE init.
	//MCUInit();
	//MC13192Init();
	//MLMESetMC13192ClockRate(0);
#ifdef XBEE
	xbeeInit();
#endif
	gLocalDeviceState = eLocalStateStarted;
	MLMEMC13192PAOutputAdjust(15);
	if (MLMESetChannelRequest(0) == SUCCESS) 
		{}

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	xTaskCreate(keyboardTask, (const signed portCHAR * const) "Keyboard", configMINIMAL_STACK_SIZE, NULL, KEYBOARD_PRIORITY, &gKeyboardTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask );
	//xTaskCreate(hooBeeTask, (const signed portCHAR * const) "HooBee", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gHooBeeTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gKeyboardQueue = xQueueCreate(KEYBOARD_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(UINT8));
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(UINT8));
	//gHooBeeQueue = xQueueCreate(HOOBEE_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(UINT8));

	// Setup the SMAC glue.
	initSMACRadioQueueGlue(gRadioReceiveQueue);
	
	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;
	
	/* All the tasks have been created - start the scheduler. */
	vTaskStartScheduler();
	
	/* Should not reach here! */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vApplicationIdleHook( void ) {
	WATCHDOG_RESET;
}