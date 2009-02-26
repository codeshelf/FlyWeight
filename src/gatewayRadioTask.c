/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "gatewayRadioTask.h"
#include "gatewayMgmtTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "simple_mac.h"
#include "ledBlinkTask.h"
#include "USB.h"
#include "commands.h"
#include "Watchdog.h"

// SMAC includes
#include "pub_def.h"

// --------------------------------------------------------------------------
// Global variables.

UINT8 				gu8RTxMode;
extern byte			gCCRHolder;

xTaskHandle			gRadioReceiveTask = NULL;
xTaskHandle			gRadioTransmitTask = NULL;
xTaskHandle			gSerialReceiveTask = NULL;
xTaskHandle			gGatewayManagementTask = NULL;

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

RemoteAddrType		gMainRemote = INVALID_REMOTE;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType			rxBufferNum;
	portTickType			rxQueueTimeout = (portTickType) (200 * portTICK_RATE_MS);
//	ECommandIDType			cmdID;
//	ENetMgmtSubCmdIDType	subCmdID;
	
	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
	
		for (;;) {

			#ifdef __WatchDog
			WatchDog_Clear();
			#endif
			
			// Don't try to RX if there is no free buffer.
			while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse) {
				vTaskDelay(1);
				WatchDog_Clear();
			}

			// Don't try to set up an RX unless we're already done with TX.
			while (gu8RTxMode == TX_MODE) {
				vTaskDelay(1);
				WatchDog_Clear();
			}
			
			//vTaskSuspend(gRadioTransmitTask);
			
			// Setup for the next RX cycle (if needed since we already might be setup to read).
			if (gu8RTxMode != RX_MODE) {
				gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
				gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
				gsRxPacket.u8Status = 0;
				MLMERXEnableRequest(&gsRxPacket, 0L);
			}
		
			//vTaskResume(gRadioTransmitTask);
			
			// Packets received by the SMAC get put onto the receive queue, and we process them here.
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, rxQueueTimeout) == pdPASS) {
			
//				cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
//				if (cmdID == eCommandNetMgmt) {
//					subCmdID = getNetMgmtSubCommand(gRXRadioBuffer[rxBufferNum].bufferStorage);
//					switch (subCmdID) {
//						case eNetMgmtSubCmdNetCheck:
//							processNetCheckInboundCommand(rxBufferNum);
//							break;
//					}
//				}

				// Send the packet to the controller.
				serialTransmitFrame((byte*) (&gRXRadioBuffer[rxBufferNum].bufferStorage), gRXRadioBuffer[rxBufferNum].bufferSize);
				RELEASE_RX_BUFFER(rxBufferNum);
				
				// Blink LED2 to let us know we succeeded in receiving a packet buffer.
				//if (xQueueSend(gLEDBlinkQueue, &gLED2, (portTickType) 0)) {
				//}
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
	byte			ccrHolder;

	// Turn the SCi back on by taking RX out of standby.
	CTS_SETUP;
	RTS_SETUP;
	CTS_ON;
		
	for (;;) {
	
		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, 5) == pdPASS ) {

			vTaskSuspend(gRadioReceiveTask);
			
			// Disable the RX to prepare for TX.
			MLMERXDisableRequest();

			// Setup for TX.
			gsTxPacket.pu8Data = gTXRadioBuffer[txBufferNum].bufferStorage;
			gsTxPacket.u8DataLength = gTXRadioBuffer[txBufferNum].bufferSize;
			EnterCriticalArg(ccrHolder);
			// Keep sending until we succeed.
			while (MCPSDataRequest(&gsTxPacket) != SUCCESS)
				;
			ExitCriticalArg(ccrHolder);
			
			// Set the status of the TX buffer to free.
			RELEASE_TX_BUFFER(txBufferNum);	
			
			// Prepare to RX responses.
			gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
			gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
			gsRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gsRxPacket, 0L);
						
			vTaskResume(gRadioReceiveTask);
			
			// Blink LED1 to let us know we succeeded in transmitting the buffer.
			//if (xQueueSend(gLEDBlinkQueue, &gLED1, (portTickType) 0)) {
			//}
			
		} else {
			
		}
	}
}
