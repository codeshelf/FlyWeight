/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "gatewayRadioTask.h"
#include "simple_mac.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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

UINT8 				gu8RTxMode;

// Radio input buffer
RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gRXCurBufferNum = 0;
BufferCntType		gRXUsedBuffers = 0;

RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
BufferCntType		gTXCurBufferNum = 0;
BufferCntType		gTXUsedBuffers = 0;

portTickType		gBufferTimeMS = (float) (1.0 / (7420.0 / RX_BUFFER_SIZE)) * 1000;

tTxPacket			gsTxPacket;
tRxPacket			gsRxPacket;

// --------------------------------------------------------------------------

void radioReceiveTask( void *pvParameters ) {
	ERadioState			radioState;
	BufferCntType		rxBufferNum;
	RadioCommandIDType	cmdID;
	RemoteAddrType		cmdSrcAddr;
	portTickType		lastTick;	
	
	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {
	
		// Now the queue is created it is safe to enable the radio receive interrupt.
		for ( ;; ) {

			//WatchDog_Clear();
			
			// Wait until the radio is ready.
			while (gu8RTxMode != IDLE_MODE)
				vTaskDelay(2);
			
			// Setup for the next receive cycle.
			lastTick = xTaskGetTickCount();
			while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse)
				vTaskDelay(2);

			gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
			gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
			gsRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gsRxPacket, 0L);
		
			// Packets received by the SMAC get put onto the receive queue, and we process them here.
			if (xQueueReceive(gRadioReceiveQueue, &radioState, portMAX_DELAY) == pdPASS) {
				if (xQueueSend(gLEDBlinkQueue, &gLED2, pdFALSE)) {
				}

				if (radioState == eRadioReset) {
					// We just received a reset from the radio.
					
				} else if (radioState == eRadioReceive) {
					
					advanceRXBuffer();
					
					// All packets sent in FlyWeight are commands.
					// Parse the command.
					
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
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void radioTransmitTask( void *pvParameters ) {
	tTxPacket				gsTxPacket;
	BufferCntType			bufferNum;

	// Turn the SCi back on by taking RX out of standby.
	RTS_PORTENABLE;
	RTS_PORTDIRECTION;
	RTS_ON;
		
	for (;;) {
	
		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &bufferNum, portTICK_RATE_MS * 500 ) == pdPASS ) {

			// Disable the RX to prepare for TX.
			MLMERXDisableRequest();

			// Transmit the buffer.
			gsTxPacket.pu8Data = gTXRadioBuffer[bufferNum].bufferStorage;
			gsTxPacket.u8DataLength = gTXRadioBuffer[bufferNum].bufferSize;
			MCPSDataRequest(&gsTxPacket);
			
			// Set the status of the buffer to free.
			gTXRadioBuffer[bufferNum].bufferStatus = eBufferStateFree;
			gTXRadioBuffer[bufferNum].bufferSize = 0;	
			
			// Prepare to receive responses.
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

// --------------------------------------------------------------------------

void advanceRXBuffer() {

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	EnterCritical();
	
		gRXRadioBuffer[gRXCurBufferNum].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gRXCurBufferNum++;
		if (gRXCurBufferNum >= (RX_BUFFER_COUNT))
			gRXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		if (gRXUsedBuffers < RX_BUFFER_COUNT)
			gRXUsedBuffers++;
		
	ExitCritical();
}

// --------------------------------------------------------------------------

void advanceTXBuffer() {

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	EnterCritical();
	
		gTXRadioBuffer[gTXCurBufferNum].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gTXCurBufferNum++;
		if (gTXCurBufferNum >= (TX_BUFFER_COUNT))
			gTXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		if (gTXUsedBuffers < TX_BUFFER_COUNT)
			gTXUsedBuffers++;
		
	ExitCritical();
}