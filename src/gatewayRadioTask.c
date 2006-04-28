/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "gatewayRadioTask.h"
#include "ledBlinkTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "simple_mac.h"
#include "USB.h"

// SMAC includes
#include "pub_def.h"


// Local variables.

UINT8 gu8RTxMode;
extern USBStateType gUSBState;

// Radio input buffer
RadioBufferStruct	gRadioBuffer[ASYNC_BUFFER_COUNT];
BufferCntType		gCurRadioBufferNum = 0;
portTickType		bufferTimeMS = (float) (1.0 / (7420.0 / ASYNC_BUFFER_SIZE)) * 1000;

// --------------------------------------------------------------------------

void vRadioTransmitTask( void *pvParameters ) {
	tTxPacket				gsTxPacket;
	BufferCntType			bufferNum;
	portTickType			lastTickCount;
	

	// Turn the SCi back on by taking RX out of standby.
	RTS_PORTENABLE;
	RTS_PORTDIRECTION;
	RTS_ON;
		
	lastTickCount = xTaskGetTickCount();
	for (;;) {

		// Wait until the SCi controller signals us that we have a full buffer to transmit.
		if ( xQueueReceive( xRadioTransmitQueue, &bufferNum, portTICK_RATE_MS * 500 ) == pdPASS ) {

			// Transmit the buffer.
			gsTxPacket.pu8Data = gRadioBuffer[bufferNum].bufferStorage;
			gsTxPacket.u8DataLength = ASYNC_BUFFER_SIZE;
			MCPSDataRequest(&gsTxPacket);
						
			// Wait until the we've sent the right number of packets per second.
			vTaskDelayUntil(&lastTickCount, bufferTimeMS);
			
			//
			gRadioBuffer[bufferNum].bufferStatus = eBufferStateEmpty;
			
			// At least one buffer got freed, so if we were pausing on serial IO this will resume.
			//USB_START;

		} else {
			
			// Check to see if the buffer is free, so that we can get data again.
			//if (gRadioBuffer[gCurRadioBufferNum].bufferStatus != eBufferStateFull)
			//	USB_START;
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

// --------------------------------------------------------------------------

void vSerialReceiveTask( void *pvParameters ) {

	UINT16	bytesReceived;
	
	for ( ;; ) {

		// Check if there is enough data in the serial buffer to fill the next *empty* transmit queue.
		if ((USB_GetCharsInRxBuf() >= ASYNC_BUFFER_SIZE) && (gRadioBuffer[gCurRadioBufferNum].bufferStatus != eBufferStateFull)) {
			
			USB_RecvBlock((USB_TComData *) &gRadioBuffer[gCurRadioBufferNum].bufferStorage, ASYNC_BUFFER_SIZE, &bytesReceived);
			
			// Mark the transmit buffer full.
			gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateFull;
			
			// Now send the buffer to the transmit queue.
			if (xQueueSend(xRadioTransmitQueue, &gCurRadioBufferNum, pdFALSE)) {
			}

			// Now advance the transmit buffer.
			if (gCurRadioBufferNum == (ASYNC_BUFFER_COUNT - 1))
				gCurRadioBufferNum = 0;
			else
				gCurRadioBufferNum++;
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

