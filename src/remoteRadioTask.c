/*
   FlyWeight
   � Copyright 2005, 2006 Jeffrey B. Williams
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

SampleRateType		gMasterSampleRate = 0;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType		rxBufferNum;
	RadioCommandIDType	cmdID;
	RemoteAddrType		cmdDstAddr;

	// Start the audio processing.
	//AudioLoader_Enable();
	
	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {
	
		// Setup the TPM2 timer.
		TPM2SC = 0b01001000;
		// 16MHz bus clock with a 7.4kHz interrupt freq.
		TPM2MOD = gMasterSampleRate;	
		
		//PWM1_Enable();
		PWM_Init();

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

			// Wait until we receive a queue message from the radio receive ISR.
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
				
				// We just received a valid packet.
				// We don't really do anything here since 
				// the PWM audio processor is working at interrupt
				// to get bytes out of the buffer.
				
				cmdID = getCommandNumber(rxBufferNum);
				cmdDstAddr = getCommandDstAddr(rxBufferNum);
				
				// Only process broadcast commands or commands addressed to us.
				if ((cmdDstAddr == ADDR_BROADCAST) || (cmdDstAddr == gMyAddr)) {
				
					switch (cmdID) {
					
						case eCommandDatagram:
							gRXRadioBuffer[rxBufferNum].bufferStatus = eBufferStateSoundData;
							break;
						
						case eCommandAddrAssign:
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
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	tTxPacket		gsTxPacket;
	BufferCntType	txBufferNum;
	
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