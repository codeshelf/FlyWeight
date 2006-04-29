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
RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gRXCurBufferNum = 0;
portTickType		gBufferTimeMS = (float) (1.0 / (7420.0 / RX_BUFFER_SIZE)) * 1000;

// --------------------------------------------------------------------------

void radioTransmitTask( void *pvParameters ) {
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
		if ( xQueueReceive( gRadioTransmitQueue, &bufferNum, portTICK_RATE_MS * 500 ) == pdPASS ) {

			// Transmit the buffer.
			gsTxPacket.pu8Data = gRXBuffer[bufferNum].bufferStorage;
			gsTxPacket.u8DataLength = RX_BUFFER_SIZE;
			MCPSDataRequest(&gsTxPacket);
						
			// Wait until the we've sent the right number of packets per second.
			vTaskDelayUntil(&lastTickCount, gBufferTimeMS);
			
			//
			gRXBuffer[bufferNum].bufferStatus = eBufferStateFree;
			
			// At least one buffer got freed, so if we were pausing on serial IO this will resume.
			//USB_START;

		} else {
			
			// Check to see if the buffer is free, so that we can get data again.
			//if (gRXBuffer[gRXCurBufferNum].bufferStatus != eBufferStateFull)
			//	USB_START;
		}
		
		// Turn the SCi back on by clearing the RX buffer.
		//USB_ClearRxBuf();
		
		// Blink LED1 to let us know we succeeded in transmitting the buffer.
		if (xQueueSend(gLEDBlinkQueue, &gLED1, pdFALSE)) {
		
		}
	}
}

// --------------------------------------------------------------------------

void radioReceiveTask( void *pvParameters ) {
	byte	*packetDataP;

	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {
	
		/* Now the queue is created it is safe to enable the radio receive interrupt. */
		for ( ;; ) {
			/* Simply wait for data to arrive from the button push interrupt. */

			if (xQueueReceive(gRadioReceiveQueue, &packetDataP, portMAX_DELAY) == pdPASS) {
				if (xQueueSend(gLEDBlinkQueue, &gLED2, pdFALSE)) {}

				if (xQueueSend(gRadioTransmitQueue, NULL, pdFALSE)) {}

			}

		}

	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void serialReceiveTask( void *pvParameters ) {

	UINT16	bytesReceived;
	
	for ( ;; ) {

		// Check if there is enough data in the serial buffer to fill the next *empty* transmit queue.
		if ((USB_GetCharsInRxBuf() >= RX_BUFFER_SIZE) && (gRXBuffer[gRXCurBufferNum].bufferStatus != eBufferStateInUse)) {
			
			USB_RecvBlock((USB_TComData *) &gRXBuffer[gRXCurBufferNum].bufferStorage, RX_BUFFER_SIZE, &bytesReceived);
			
			// Mark the transmit buffer full.
			gRXBuffer[gRXCurBufferNum].bufferStatus = eBufferStateInUse;
			
			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &gRXCurBufferNum, pdFALSE)) {
			}

			// Now advance the transmit buffer.
			if (gRXCurBufferNum == (RX_BUFFER_COUNT - 1))
				gRXCurBufferNum = 0;
			else
				gRXCurBufferNum++;
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

