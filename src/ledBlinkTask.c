/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "ledBlinkTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"
#include "gatewayRadioTask.h"

UINT8 gLED1 = 1;
UINT8 gLED2 = 2;
UINT8 gLED3 = 3;
UINT8 gLED4 = 4;
xQueueHandle xLEDBlinkQueue;

// --------------------------------------------------------------------------

void vLEDBlinkTask( void *pvParameters ) {
	UINT8	ledNum;

	xLEDBlinkQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));

	if ( xLEDBlinkQueue ) {
		for ( ;; ) {
			/* Simply wait for data to arrive from the button push interrupt. */

			if ( xQueueReceive( xLEDBlinkQueue, &ledNum, portMAX_DELAY ) == pdPASS ) {
				LEDOn(ledNum);
				vTaskDelay(portTICK_RATE_MS * 100);
				LEDOff(ledNum);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void LEDOn(int inLEDNum) {
	switch (inLEDNum) {

		case 1:
			LED1_ClrVal();
			break;

		case 2:
			LED2_ClrVal();
			break;

		case 3:
			LED3_ClrVal();
			break;

		case 4:
			LED4_ClrVal();
			break;

	}
}

// --------------------------------------------------------------------------

void LEDOff(int inLEDNum) {
	switch (inLEDNum) {

		case 1:
			LED1_SetVal();
			break;

		case 2:
			LED2_SetVal();
			break;

		case 3:
			LED3_SetVal();
			break;

		case 4:
			LED4_SetVal();
			break;

	}
}