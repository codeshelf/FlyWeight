/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "remoteRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "simple_mac.h"
#include "ledBlinkTask.h"
#include "USB.h"
#include "commands.h"

// SMAC includes
#include "pub_def.h"

// --------------------------------------------------------------------------
// Local variables.

// The queue used to send data from the radio to the radio receive task.
xQueueHandle		gRadioTransmitQueue = NULL;
xQueueHandle		gRadioReceiveQueue = NULL;

tTxPacket			gsTxPacket;
tRxPacket			gsRxPacket;

// Radio input buffer
// There's a 2-byte ID on the front of every packet.
RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gRXCurBufferNum = 0;
BufferCntType		gRXUsedBuffers = 0;

RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
BufferCntType		gTXCurBufferNum = 0;
BufferCntType		gTXUsedBuffers = 0;

portTickType		gBufferTimeMS = (float) (1.0 / (7420.0 / RX_BUFFER_SIZE)) * 1000;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType		rxBufferNum;
	RadioCommandIDType	cmdID;
	RemoteAddrType		cmdSrcAddr;
	
	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {
	
		for (;;) {

			//WatchDog_Clear();
			
			// Don't try to RX if there is no free buffer.
			while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse)
				vTaskDelay(1);

			// Don't try to set up an RX unless we're already done with TX.
			while (gu8RTxMode == TX_MODE)
				vTaskDelay(1);
			
			// Setup for the next RX cycle.
			if (gu8RTxMode != RX_MODE) {
				gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
				gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
				gsRxPacket.u8Status = 0;
				MLMERXEnableRequest(&gsRxPacket, 0L);
			}
		
			// Packets received by the SMAC get put onto the receive queue, and we process them here.
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
			
				cmdID = getCommandNumber(rxBufferNum);
				cmdSrcAddr = getCommandSrcAddr(rxBufferNum);
				
				switch (cmdID) {
				
					case eCommandWake:
						processWakeCommand(rxBufferNum);
						break;
						
					case eCommandResponse:
						processResponseCommand(rxBufferNum, cmdSrcAddr);
						break;
						
					default:
						break;
					
				}
			}
			
			// Blink LED2 to let us know we succeeded in receiving a packet buffer.
			if (xQueueSend(gLEDBlinkQueue, &gLED2, pdFALSE)) {
			
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	tTxPacket		gsTxPacket;
	BufferCntType	txBufferNum;

	// Turn the SCi back on by taking RX out of standby.
	RTS_PORTENABLE;
	RTS_PORTDIRECTION;
	RTS_ON;
		
	for (;;) {
	
		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

			// Disable the RX to prepare for TX.
			MLMERXDisableRequest();

			// Setup for TX.
			gsTxPacket.pu8Data = gTXRadioBuffer[txBufferNum].bufferStorage;
			gsTxPacket.u8DataLength = gTXRadioBuffer[txBufferNum].bufferSize;
			MCPSDataRequest(&gsTxPacket);
			
			// Set the status of the TX buffer to free.
			gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree;
			gTXRadioBuffer[txBufferNum].bufferSize = 0;	
			
			// Prepare to RX responses.
			gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
			gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
			gsRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gsRxPacket, 0L);
						
		} else {
			
		}
		
		// Blink LED1 to let us know we succeeded in transmitting the buffer.
		if (xQueueSend(gLEDBlinkQueue, &gLED1, pdFALSE)) {
		
		}
	}
}

// --------------------------------------------------------------------------

void serialReceiveTask( void *pvParameters ) {

	UINT16			bytesReceived;
	portTickType	lastTick;	
	
	for ( ;; ) {

		// Check if there is enough data in the serial buffer to fill the next *empty* transmit queue.
		if ((USB_GetCharsInRxBuf() >= TX_BUFFER_SIZE) && (gTXRadioBuffer[gTXCurBufferNum].bufferStatus != eBufferStateInUse)) {
			
			USB_RecvBlock((USB_TComData *) &gTXRadioBuffer[gTXCurBufferNum].bufferStorage, TX_BUFFER_SIZE, &bytesReceived);
			
			// Mark the transmit buffer full.
			gTXRadioBuffer[gTXCurBufferNum].bufferStatus = eBufferStateInUse;
			
			advanceTXBuffer();
			
			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &gTXCurBufferNum, pdFALSE)) {
			}
			
			// Wait until the we've sent the right number of packets per second.
			vTaskDelayUntil(&lastTick, gBufferTimeMS);

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
