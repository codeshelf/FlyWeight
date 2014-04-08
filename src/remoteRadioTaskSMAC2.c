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
#include "commands.h"
#include "remoteCommon.h"

// --------------------------------------------------------------------------
// Global variables.

xTaskHandle gRadioReceiveTask = NULL;
xTaskHandle gRadioTransmitTask = NULL;
xTaskHandle gRemoteManagementTask = NULL;

// The queue used to send data from the radio to the radio receive task.
xQueueHandle gRadioTransmitQueue = NULL;
xQueueHandle gRadioReceiveQueue = NULL;
gwTxPacket gTxPacket;
gwRxPacket gRxPacket;

extern BufferCntType gRXCurBufferNum;
extern BufferCntType gRXUsedBuffers;

extern BufferCntType gTXUsedBuffers;
gwUINT8 gu8RTxMode;
extern gwBoolean gIsSleeping;
extern gwBoolean gShouldSleep;
extern gwUINT8 gSleepCount;
extern gwUINT8 gButtonPressed;
extern gwUINT8 gCCRHolder;
gwUINT8 gAssocCheckCount = 1;

// Radio buffers
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct gRXRadioBuffer[RX_BUFFER_COUNT];
extern RadioBufferStruct gTXRadioBuffer[TX_BUFFER_COUNT];

EMessageHolderType gMsgHolder[MAX_NUM_MSG];
gwUINT8 gNextMsgToUse = 0;
gwUINT8 gCurMsg = 0;
gwUINT8 gTotalPendingMsgs = 0;

portTickType gLastAssocCheckTickCount;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum;
	gwUINT8 ccrHolder;
	FuncReturn_t funcErr;
	portTickType lastAssocCheck;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {

		gSleepCount = 0;
		gLastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;

		for (;;) {

			GW_WATCHDOG_RESET;

			GW_ENTER_CRITICAL(ccrHolder);
			if (gCurMsg == gNextMsgToUse) {

				// Setup for the next RX cycle.
				rxBufferNum = lockRXBuffer();

				gMsgHolder[gNextMsgToUse].msg.pu8Buffer = (smac_pdu_t*) (gRXRadioBuffer[rxBufferNum].bufferRadioHeader);
				gMsgHolder[gNextMsgToUse].msg.u8BufSize = RX_BUFFER_SIZE;
				gMsgHolder[gNextMsgToUse].msg.u8Status.msg_type = RX;
				gMsgHolder[gNextMsgToUse].msg.u8Status.msg_state = MSG_RX_RQST;
				gMsgHolder[gNextMsgToUse].msg.cbDataIndication = NULL;
				gMsgHolder[gNextMsgToUse].bufferNum = rxBufferNum;

				funcErr = MLMERXEnableRequest(&(gMsgHolder[gNextMsgToUse].msg), 0);

				gTotalPendingMsgs++;
				gNextMsgToUse++;
				if (gNextMsgToUse >= MAX_NUM_MSG)
					gNextMsgToUse = 0;
			}
			GW_EXIT_CRITICAL(ccrHolder);

			// Keep looping until we've processed all of the messages that we've entered into the queue.
			while (gCurMsg != gNextMsgToUse) {
				if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == RX) {
					int delayCheck = 0;
					do {
						funcErr = process_radio_msg();

						if ((gAssocCheckCount > 0) && (xTaskGetTickCount() - lastAssocCheck > 200)) {
							BufferCntType txBufferNum = lockTXBuffer();
							createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
							if (transmitPacket(txBufferNum)) {
							}
							lastAssocCheck = xTaskGetTickCount();
						}

					} while ((funcErr != gSuccess_c) || (RX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg)));

					if (gMsgHolder[gCurMsg].msg.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS) {
						// Process the packet we just received.
						gRXRadioBuffer[gMsgHolder[gCurMsg].bufferNum].bufferSize = gMsgHolder[gCurMsg].msg.u8BufSize;
						processRxPacket(gMsgHolder[gCurMsg].bufferNum);
						// processRXPacket releases the RX buffer if necessary.
					} else {
						// Probably failed or aborted, release it.
						RELEASE_RX_BUFFER(gMsgHolder[gCurMsg].bufferNum, ccrHolder);
					}

				} else if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == TX) {
					do {
						funcErr = process_radio_msg();
					} while ((funcErr != gSuccess_c) || (TX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg)));
					RELEASE_TX_BUFFER(gMsgHolder[gCurMsg].bufferNum, ccrHolder);
				}

				GW_ENTER_CRITICAL(ccrHolder);
				gTotalPendingMsgs--;
				gCurMsg++;
				if (gCurMsg >= MAX_NUM_MSG)
					gCurMsg = 0;
				GW_EXIT_CRITICAL(ccrHolder);
			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	BufferCntType txBufferNum;
	FuncReturn_t funcErr;
	gwUINT8 txMsgNum;
	gwUINT8 ccrHolder;

	if (gRadioTransmitQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {

				gShouldSleep = FALSE;

				while (gTotalPendingMsgs >= MAX_NUM_MSG) {
					// There's no message space in the queue, so wait.
					vTaskDelay(1);
				}

				GW_ENTER_CRITICAL(ccrHolder);
				// Disable a pending RX to prepare for TX.
				if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == RX) {
					MLMERXDisableRequest(&(gMsgHolder[gCurMsg].msg));
					do {
						funcErr = process_radio_msg();
					} while ((funcErr != gSuccess_c) || (RX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg)));
				}

				// Setup for TX.
				gMsgHolder[gNextMsgToUse].msg.pu8Buffer = (smac_pdu_t *) ((gTXRadioBuffer[txBufferNum].bufferRadioHeader) + 1);
				gMsgHolder[gNextMsgToUse].msg.u8BufSize = gTXRadioBuffer[txBufferNum].bufferSize;
				gMsgHolder[gNextMsgToUse].msg.u8Status.msg_type = TX;
				gMsgHolder[gNextMsgToUse].msg.u8Status.msg_state = MSG_TX_RQST;
				gMsgHolder[gNextMsgToUse].msg.cbDataIndication = NULL;
				gMsgHolder[gNextMsgToUse].bufferNum = txBufferNum;
				txMsgNum = gNextMsgToUse;

				funcErr = MCPSDataRequest(&(gMsgHolder[gNextMsgToUse].msg));

				// If the radio can't TX then we're in big trouble.  Just reset.
				if (funcErr != gSuccess_c) {
					GW_RESET_MCU()
					;
				}

				gTotalPendingMsgs++;
				gNextMsgToUse++;
				if (gNextMsgToUse >= MAX_NUM_MSG)
					gNextMsgToUse = 0;
				GW_EXIT_CRITICAL(ccrHolder);

				while (TX_MESSAGE_PENDING(gMsgHolder[txMsgNum].msg)) {
					// Wait until this TX message is done, before we start another.
					vTaskDelay(1);
				}
//				GW_ENTER_CRITICAL(ccrHolder);
//				funcErr = MLMERXEnableRequest(&(gMsgHolder[gNextMsgToUse].msg), 0);
//				GW_EXIT_CRITICAL(ccrHolder);
			} else {

			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
