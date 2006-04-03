/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "gatewayRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "simple_mac.h"
#include "USB.h"

// SMAC includes
#include "pub_def.h"


// Local variables.

/* The queue used to send data from the radio to the radio receive task. */
xQueueHandle xRadioTransmitQueue;
xQueueHandle xRadioReceiveQueue;

UINT8 gu8RTxMode;
extern UINT8 gLED1;
extern UINT8 gLED2;
extern UINT8 gLED3;
extern UINT8 gLED4;
extern xQueueHandle xLEDBlinkQueue;

// Radio input buffer
RadioBufferStruct	gRadioBuffer[ASYNC_BUFFER_COUNT];
BufferCntType		gCurRadioBufferNum = 0;

// --------------------------------------------------------------------------

void vRadioTransmitTask( void *pvParameters ) {
	static tTxPacket		gsTxPacket;
	static BufferCntType	bufferNum;
	portTickType			lastTickCount;
	const portTickType		frequency = 10;

	xRadioTransmitQueue = xQueueCreate(RADIO_QUEUE_SIZE, sizeof(gCurRadioBufferNum));

	// Turn the SCi back on by taking RX out of standby.
	RTS_PORTENABLE;
	RTS_PORTDIRECTION;
	RTS_ON;
		
	lastTickCount = xTaskGetTickCount();
	for (;;) {

		// Wait until the SCi controller signals us that we have a full buffer to transmit.
		if ( xQueueReceive( xRadioTransmitQueue, &bufferNum, portTICK_RATE_MS * 500 ) == pdPASS ) {

			// Now we need to wait to make the OTA baud rate 11K. (11KB/S / 121B)ms
			//vTaskDelayUntil(&lastTickCount, portTICK_RATE_MS * 14);
			
			// Transmit the buffer.
			gsTxPacket.pu8Data = gRadioBuffer[bufferNum].bufferStorage;
			gsTxPacket.u8DataLength = ASYNC_BUFFER_SIZE;
			MCPSDataRequest(&gsTxPacket);
			
			// At least one buffer got freed, so if we were pausing on serial IO this will resume.
			RTS_ON;
		} else {
			// Transmit the buffer.
			gsTxPacket.pu8Data = "IDLE";
			gsTxPacket.u8DataLength = 4;
			//MCPSDataRequest(&gsTxPacket);
		}
		
		// Turn the SCi back on by clearing the RX buffer.
		//USB_ClearRxBuf();
		
		// Blink LED1 to let us know we succeeded in transmitting the buffer.
		if (xQueueSend(xLEDBlinkQueue, &gLED1, pdFALSE)) {
		
		}
	}
}

// --------------------------------------------------------------------------

void vRadioReceiveTask( void *pvParameters ) {
	byte	*packetDataP;

	// The radio receive task will return a pointer to a radio data packet.

	/* Create the queue used by the producer and consumer. */
	xRadioReceiveQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));
	initSMACRadioQueueGlue(xRadioReceiveQueue);

	if ( xRadioReceiveQueue ) {
	
		/* Now the queue is created it is safe to enable the radio receive interrupt. */
		for ( ;; ) {
			/* Simply wait for data to arrive from the button push interrupt. */

			if ( xQueueReceive( xRadioReceiveQueue, &packetDataP, portMAX_DELAY ) == pdPASS ) {
				if (xQueueSend( xLEDBlinkQueue, &gLED2, pdFALSE )) {}

				if (xQueueSend( xRadioTransmitQueue, NULL, pdFALSE )) {}

			}

		}

	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

