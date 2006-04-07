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
#include "mcu_hw_config.h"
#include "MC13192_hw_config.h"
#include "simple_mac.h"
#include "gatewayRadioTask.h"
#include "ledBlinkTask.h"
#include "CPU.h"
//#include "WatchDog.h"

xQueueHandle xRadioTransmitQueue;
xQueueHandle xRadioReceiveQueue;
xQueueHandle xLEDBlinkQueue;

// --------------------------------------------------------------------------

void vMain( void ) {
	MCUInit();
	MC13192Init();
	MLMESetMC13192ClockRate(0);
#ifdef XBEE
	xbeeInit();
#endif
	MLMEMC13192PAOutputAdjust(MAX_POWER);    //Set MAX power setting
	if (MLMESetChannelRequest(15) == SUCCESS) {}

	/* Start the task that will handle the radio */
	xTaskCreate( vRadioTransmitTask, (const signed portCHAR * const) "RadioTransmit", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, NULL );
	//xTaskCreate( vRadioReceiveTask, (const signed portCHAR * const) "RadioReceive", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, NULL );
	xTaskCreate( vSerialReceiveTask, (const signed portCHAR * const) "Serial Recv", configMINIMAL_STACK_SIZE, NULL, SERIAL_RECV_PRIORITY, NULL );
	xTaskCreate( vLEDBlinkTask, (const signed portCHAR * const) "LED Blink", configMINIMAL_STACK_SIZE, NULL, LED_BLINK_PRIORITY, NULL );

	xRadioReceiveQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));
	xRadioTransmitQueue = xQueueCreate(RADIO_QUEUE_SIZE, sizeof(BufferCntType));
	xLEDBlinkQueue = xQueueCreate(LED_BLINK_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(UINT8));

	// Setup the SMAC glue.
	initSMACRadioQueueGlue(xRadioReceiveQueue);

	/* All the tasks have been created - start the scheduler. */
	vTaskStartScheduler();

	/* Should not reach here! */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vApplicationIdleHook( void ) {
	// Clear the watchdog timer.
	//WatchDog_Clear();
}