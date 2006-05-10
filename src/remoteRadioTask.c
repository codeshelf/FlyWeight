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
#include "commands.h"
#include "remoteMgmtTask.h"

// SMAC includes
#include "pub_def.h"
#include "PWM.h"

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

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	ERadioState			radioState;
	portTickType		lastTick;
	BufferCntType		rxBufferNum;
	RadioCommandIDType	cmdID;

	// Start the audio processing.
	//AudioLoader_Enable();
	
	// Setup the TPM2 timer.
	TPM2SC = 0b01001000;
	// 16MHz bus clock with a 7.4kHz interrupt freq.
	TPM2MOD = MASTER_TPM2_RATE;	
	
	//PWM1_Enable();
	PWM_Init();

	for (;;) {

		//WatchDog_Clear();
		
		// Wait until the radio is ready.
		while (gu8RTxMode != IDLE_MODE)
			vTaskDelay(2);
		
		// Setup for the next receive cycle.
		lastTick = xTaskGetTickCount();
		while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse)
			vTaskDelay(2);

		// We have to wait here until we're in a mode that requires receiving.
		while ((gLocalDeviceState != eLocalStateWakeSent) && (gLocalDeviceState != eLocalStateRun))
			vTaskDelay(2);
		
		gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
		gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
		gsRxPacket.u8Status = 0;
		MLMERXEnableRequest(&gsRxPacket, 0L);
		
		// Wait until we receive a queue message from the radio receive ISR.
		if (xQueueReceive(gRadioReceiveQueue, &radioState, portTICK_RATE_MS * 500) == pdPASS) {
			
			if (radioState == eRadioReset) {
				// We just received a reset from the radio.
				
			} else if (radioState == eRadioReceive) {
				
				// We just received a valid packet.
				// We don't really do anything here since 
				// the PWM audio processor is working at interrupt
				// to get bytes out of the buffer.
								
				rxBufferNum = gRXCurBufferNum;
				
				advanceRXBuffer();
				
				cmdID = getCommandNumber(rxBufferNum);
				
				switch (cmdID) {
				
					case eCommandAssign:
						gLocalDeviceState = eLocalStateAddrAssignRcvd;
						// Signal the manager about the new state.
						if (xQueueSend(gRemoteMgmtQueue, &rxBufferNum, pdFALSE)) {
						}
						break;
						
					case eCommandQuery:
						gLocalDeviceState = eLocalStateQueryRcvd;
						// Signal the manager about the new state.
						if (xQueueSend(gRemoteMgmtQueue, &rxBufferNum, pdFALSE)) {
						}
						break;
						
					default:
						break;
					
				}
			}
		}
		
		// Blink LED2 to let us know we succeeded in receiving a packet buffer.
		if (xQueueSend(gLEDBlinkQueue, &gLED2, pdFALSE)) {
		
		}	
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	tTxPacket		gsTxPacket;
	BufferCntType	bufferNum;
	
	for ( ;; ) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if ( xQueueReceive( gRadioTransmitQueue, &bufferNum, portTICK_RATE_MS * 500 ) == pdPASS ) {

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