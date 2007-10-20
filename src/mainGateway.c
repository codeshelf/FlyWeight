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
#include "gatewayRadioTask.h"
#include "gatewayMgmtTask.h"
#include "ledBlinkTask.h"
#include "commands.h"
#include "CPU.h"
#include "WatchDog.h"
#include "NV_Data.h"

// --------------------------------------------------------------------------
// Globals

extern volatile const Init_NV_RAM_Struct_t NV_RAM0;
extern volatile const NV_RAM_Struct_t NV_RAM1;
extern volatile NV_RAM_Struct_t *NV_RAM_ptr;
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
	gControllerState = eControllerStateInit;
	MLMEMC13192PAOutputAdjust(15);
	
	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	xTaskCreate(serialReceiveTask, (const signed portCHAR * const) "SerialRecv", configMINIMAL_STACK_SIZE, NULL, SERIAL_RECV_PRIORITY, &gSerialReceiveTask );
#if  defined(MC13192EVB) || defined (MC13192SARD)
	//xTaskCreate(LEDBlinkTask, (const signed portCHAR * const) "LED Blink", configMINIMAL_STACK_SIZE, NULL, LED_BLINK_PRIORITY, NULL );
#endif
//	xTaskCreate(gatewayMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gGatewayManagementTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
//	gLEDBlinkQueue = xQueueCreate(LED_BLINK_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(UINT8));
//	gGatewayMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(RemoteAddrType));

	// Setup the SMAC glue.
	initSMACRadioQueueGlue(gRadioReceiveQueue);
	
	// Set the state to running
	gControllerState = eControllerStateRun;

	/* All the tasks have been created - start the scheduler. */
	vTaskStartScheduler();

	/* Should not reach here! */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vApplicationIdleHook( void ) {
	// Clear the watchdog timer.
	WatchDog_Clear();
}