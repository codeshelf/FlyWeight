/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "keyboardTask.h"
#include "keyboard.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"

xQueueHandle gKeyboardQueue;

// --------------------------------------------------------------------------

void keyboardTask( void *pvParameters ) {
	UINT8			buttonNum;
	BufferCntType	txBufferNum;

	if ( gKeyboardQueue ) {
		for ( ;; ) {
			//WatchDog_Clear();

			// When another task wants to blink it sends us a message with the LED number to blink.
			// Turn that LED on for 100ms and turn it off.
			if ( xQueueReceive( gKeyboardQueue, &buttonNum, portMAX_DELAY ) == pdPASS ) {
				
				txBufferNum = gTXCurBufferNum;
				advanceTXBuffer();
				
				// Send an associate request on the current channel.
				createButtonControlCommand(txBufferNum, buttonNum, BUTTON_PRESSED);
				if (transmitPacket(txBufferNum)) {
				};
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}