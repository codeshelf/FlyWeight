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
#include "ghdr/maca.h"

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
extern NetAddrType gMyAddr;
extern NetworkIDType gMyNetworkID;

// Radio buffers
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct gRXRadioBuffer[RX_BUFFER_COUNT];
extern RadioBufferStruct gTXRadioBuffer[TX_BUFFER_COUNT];

EMessageHolderType gRxMsgHolder;
EMessageHolderType gTxMsgHolder;

portTickType gLastAssocCheckTickCount;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum;
	gwUINT8 ccrHolder;
	FuncReturn_t funcErr;
	portTickType lastAssocCheck;
	BufferCntType lockedBufferNum;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {

		gSleepCount = 0;
		gLastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;

		for (;;) {

			// Setup for the next RX cycle.
			lockedBufferNum = lockRXBuffer();

			gRxMsgHolder.msg.pu8Buffer = (smac_pdu_t*) (gRXRadioBuffer[lockedBufferNum].bufferRadioHeader);
			gRxMsgHolder.msg.u8BufSize = RX_BUFFER_SIZE;
			gRxMsgHolder.msg.u8Status.msg_type = RX;
			gRxMsgHolder.msg.u8Status.msg_state = MSG_RX_RQST;
			gRxMsgHolder.msg.cbDataIndication = NULL;
			gRxMsgHolder.bufferNum = lockedBufferNum;

			funcErr = MLMERXEnableRequest(&(gRxMsgHolder.msg), 1000 * 250000);

			do {
				funcErr = process_radio_msg();
			} while ((funcErr != gSuccess_c) || (RX_MESSAGE_PENDING(gRxMsgHolder.msg)));

			if (gRxMsgHolder.msg.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS) {
				// Process the packet we just received.
				gRXRadioBuffer[gRxMsgHolder.bufferNum].bufferSize = gRxMsgHolder.msg.u8BufSize;
				processRxPacket(gRxMsgHolder.bufferNum);
				// processRXPacket releases the RX buffer if necessary.
			} else {
				// Probably failed or aborted, release it.
				RELEASE_RX_BUFFER(gRxMsgHolder.bufferNum, ccrHolder);
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
	ECommandGroupIDType cmdId;
	gwBoolean shouldRetry;
	portTickType retryTickCount;
	NetAddrType cmdDstAddr;
	NetworkIDType networkID;
	gwUINT8 ccrHolder;
	BufferCntType lockedBufferNum;

	if (gRadioTransmitQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {

				gShouldSleep = FALSE;

				// Setup for TX.
				gTxMsgHolder.msg.pu8Buffer = (smac_pdu_t *) ((gTXRadioBuffer[txBufferNum].bufferRadioHeader) + 1);
				gTxMsgHolder.msg.u8BufSize = gTXRadioBuffer[txBufferNum].bufferSize;
				gTxMsgHolder.msg.u8Status.msg_type = TX;
				gTxMsgHolder.msg.u8Status.msg_state = MSG_TX_RQST;
				gTxMsgHolder.msg.cbDataIndication = NULL;
				gTxMsgHolder.bufferNum = txBufferNum;

				GW_ENTER_CRITICAL(ccrHolder);
				vTaskSuspend(gRadioReceiveTask);
				funcErr = MLMERXDisableRequest(&(gRxMsgHolder.msg));
				GW_EXIT_CRITICAL(ccrHolder);

				shouldRetry = FALSE;
				retryTickCount = xTaskGetTickCount();
				do {
					funcErr = MCPSDataRequest(&(gTxMsgHolder.msg));
					do {
						// Wait until this TX message is done, before we start another.
						funcErr = process_radio_msg();
					} while ((funcErr != gSuccess_c) || (TX_MESSAGE_PENDING(gTxMsgHolder.msg)));

					if (!(gTxMsgHolder.msg.u8Status.msg_state == MSG_TX_ACTION_COMPLETE_SUCCESS)) {
						shouldRetry = TRUE;
						RELEASE_TX_BUFFER(gTxMsgHolder.bufferNum, ccrHolder);
						vTaskDelay(maca_random & 0xff);
					} else if ((getAckId(gTXRadioBuffer[txBufferNum].bufferStorage) != 0)
							&& (getCommandID(gTXRadioBuffer[txBufferNum].bufferStorage) != eCommandNetMgmt)) {
						// If the TX packet had an ACK id then retry TX until we get the ACK or reset.

						RELEASE_RX_BUFFER(gRxMsgHolder.bufferNum, ccrHolder);

						// Setup for the next RX cycle.
						lockedBufferNum = lockRXBuffer();

						gRxMsgHolder.msg.pu8Buffer = (smac_pdu_t*) (gRXRadioBuffer[lockedBufferNum].bufferRadioHeader);
						gRxMsgHolder.msg.u8BufSize = RX_BUFFER_SIZE;
						gRxMsgHolder.msg.u8Status.msg_type = RX;
						gRxMsgHolder.msg.u8Status.msg_state = MSG_RX_RQST;
						gRxMsgHolder.msg.cbDataIndication = NULL;
						gRxMsgHolder.bufferNum = lockedBufferNum;

						funcErr = MLMERXEnableRequest(&(gRxMsgHolder.msg), 0);

						do {
							funcErr = process_radio_msg();
						} while ((funcErr != gSuccess_c) || (RX_MESSAGE_PENDING(gRxMsgHolder.msg)));

						if (gRxMsgHolder.msg.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS) {
							networkID = getNetworkID(gRxMsgHolder.bufferNum);
							cmdDstAddr = getCommandDstAddr(gRxMsgHolder.bufferNum);
							if (!((getAckId(gTXRadioBuffer[txBufferNum].bufferStorage) == getAckId(gRXRadioBuffer[gRxMsgHolder.bufferNum].bufferStorage))
									&& (networkID == gMyNetworkID) 
									&& (cmdDstAddr == gMyAddr))) {
								shouldRetry = TRUE;
							}
						} else {
							shouldRetry = TRUE;
							RELEASE_TX_BUFFER(gTxMsgHolder.bufferNum, ccrHolder);
							//vTaskDelay(50);
						}
					}
					
					// If the radio can't TX then we're in big trouble.  Just reset.
					if (((shouldRetry) && ((xTaskGetTickCount() - retryTickCount) > 500)) || (funcErr != gSuccess_c)) {
						GW_RESET_MCU()
					}
				} while (shouldRetry);

				RELEASE_TX_BUFFER(gTxMsgHolder.bufferNum, ccrHolder);
				vTaskResume(gRadioReceiveTask);

			} else {

			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
