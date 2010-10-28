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
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "USB.h"
#include "commands.h"

// --------------------------------------------------------------------------
// Global variables.

gwUINT8 			gu8RTxMode;
extern gwUINT8		gCCRHolder;

xTaskHandle			gRadioReceiveTask = NULL;
xTaskHandle			gRadioTransmitTask = NULL;
xTaskHandle			gSerialReceiveTask = NULL;
xTaskHandle			gGatewayManagementTask = NULL;

// The queue used to send data from the radio to the radio receive task.
xQueueHandle		gRadioTransmitQueue = NULL;
xQueueHandle		gRadioReceiveQueue = NULL;

gwTxPacket			gsTxPacket;
gwRxPacket			gRxPacket;

// Radio input buffer
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRXCurBufferNum;
extern BufferCntType		gRXUsedBuffers;

extern RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];

NetAddrType	    	gMainRemote = INVALID_REMOTE;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
    gwUINT8                 ccrHolder;
	BufferCntType			rxBufferNum;
	portTickType			rxQueueTimeout = (portTickType) (200 * portTICK_RATE_MS);
//	ECommandIDType			cmdID;
//	ENetMgmtSubCmdIDType	subCmdID;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {

		for (;;) {

			GW_WATCHDOG_RESET;

			// Don't try to RX if there is no free buffer.
			while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse) {
				vTaskDelay(1);
				GW_WATCHDOG_RESET;
			}

			// Don't try to set up an RX unless we're already done with TX.
			while (gu8RTxMode == TX_MODE) {
				vTaskDelay(1);
				GW_WATCHDOG_RESET;
			}

			//vTaskSuspend(gRadioTransmitTask);

			// Setup for the next RX cycle (if needed since we already might be setup to read).
			if (gu8RTxMode != RX_MODE) {
				gRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
				gRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
				gRxPacket.u8Status = 0;
				MLMERXEnableRequest(&gRxPacket, 0L);
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
				serialTransmitFrame((gwUINT8*) (&gRXRadioBuffer[rxBufferNum].bufferStorage), gRXRadioBuffer[rxBufferNum].bufferSize);
				RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);

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
	gwTxPacket		txPacket;
	BufferCntType	txBufferNum;
	gwUINT8			ccrHolder;

	// Turn the SCi back on by taking RX out of standby.
	GW_USB_INIT;

	for (;;) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, 5) == pdPASS ) {

			vTaskSuspend(gRadioReceiveTask);

			// Disable the RX to prepare for TX.
			MLMERXDisableRequest();

			// Setup for TX.
			gsTxPacket.pu8Data = gTXRadioBuffer[txBufferNum].bufferStorage;
			gsTxPacket.u8DataLength = gTXRadioBuffer[txBufferNum].bufferSize;
			GW_ENTER_CRITICAL(ccrHolder);
			// Keep sending until we succeed.
			while (MCPSDataRequest(&gsTxPacket) != SUCCESS)
				;
			GW_EXIT_CRITICAL(ccrHolder);

			// Set the status of the TX buffer to free.
			RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

			// Prepare to RX responses.
			gRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
			gRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
			gRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gRxPacket, 0L);

			vTaskResume(gRadioReceiveTask);

			// Blink LED1 to let us know we succeeded in transmitting the buffer.
			//if (xQueueSend(gLEDBlinkQueue, &gLED1, (portTickType) 0)) {
			//}

		} else {

		}
	}
}
