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
#include "Leds.h"

// --------------------------------------------------------------------------
// Global variables.

xTaskHandle					gRadioReceiveTask = NULL;
xTaskHandle					gRadioTransmitTask = NULL;
xTaskHandle					gSerialReceiveTask = NULL;
xTaskHandle					gGatewayManagementTask = NULL;

// The queue used to send data from the radio to the radio receive task.
xQueueHandle				gRadioReceiveQueue = NULL;
xQueueHandle				gRadioTransmitQueue = NULL;

// Radio buffers
extern RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
extern RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];

EMessageHolderType			gMsgHolder[MAX_NUM_MSG];
gwUINT8						gNextMsgToUse = 0;
gwUINT8						gCurMsg = 0;

NetAddrType					gMainRemote = INVALID_REMOTE;

void sendReadMsg() {

}

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType	rxBufferNum;
	FuncReturn_t 	funcErr;
	gwUINT8			ccrHolder;
	gwUINT8			i;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {
			GW_ENTER_CRITICAL(ccrHolder);
			if (gCurMsg == gNextMsgToUse) {

				// Setup for the next RX cycle.
				rxBufferNum = lockRXBuffer();

				gMsgHolder[gNextMsgToUse].msg.pu8Buffer = (smac_pdu_t* )(gRXRadioBuffer[rxBufferNum].bufferRadioHeader);
				gMsgHolder[gNextMsgToUse].msg.u8BufSize = RX_BUFFER_SIZE;
				gMsgHolder[gNextMsgToUse].msg.u8Status.msg_type = RX;
				gMsgHolder[gNextMsgToUse].msg.u8Status.msg_state = MSG_RX_RQST;
				gMsgHolder[gNextMsgToUse].msg.cbDataIndication = NULL;
				gMsgHolder[gNextMsgToUse].bufferNum = rxBufferNum;

				funcErr = MLMERXEnableRequest(&(gMsgHolder[gNextMsgToUse].msg), 2500);

				gNextMsgToUse++;
				if (gNextMsgToUse >= MAX_NUM_MSG)
					gNextMsgToUse = 0;
			}
			GW_EXIT_CRITICAL(ccrHolder);

			// Keep looping until we've processed all of the messages that we've entered into the queue.
			while (gCurMsg != gNextMsgToUse) {
				if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == RX) {
					while ((funcErr != gSuccess_c) || RX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg)) {
						funcErr = process_radio_msg();
					}
					Led1On();
					if (gMsgHolder[gCurMsg].msg.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS) {
						// Send the packet to the controller.
						gRXRadioBuffer[gMsgHolder[gCurMsg].bufferNum].bufferSize = gMsgHolder[gCurMsg].msg.u8BufSize;
						serialTransmitFrame((gwUINT8*) (&gRXRadioBuffer[gMsgHolder[gCurMsg].bufferNum].bufferStorage), gMsgHolder[gCurMsg].msg.u8BufSize);
					}
					Led1Off();


					RELEASE_RX_BUFFER(gMsgHolder[gCurMsg].bufferNum, ccrHolder);
				} else if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == TX) {
					Led2On();
					while ((funcErr != gSuccess_c) || (TX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg))) {
						funcErr = process_radio_msg();
					}
					Led2Off();
					RELEASE_TX_BUFFER(gMsgHolder[gCurMsg].bufferNum, ccrHolder);
				}

				GW_ENTER_CRITICAL(ccrHolder);
				gCurMsg++;
				if (gCurMsg >= MAX_NUM_MSG)
					gCurMsg = 0;
				GW_EXIT_CRITICAL(ccrHolder);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	BufferCntType	txBufferNum;
	BufferCntType	rxBufferNum;
	FuncReturn_t 	funcErr;
	gwUINT8			txMsgNum;
	gwUINT8			ccrHolder;

	// Turn the SCi back on by taking RX out of standby.
	GW_USB_INIT;

	for (;;) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

			//vTaskSuspend(gRadioReceiveTask);

			GW_ENTER_CRITICAL(ccrHolder);
			// Disable a pending RX to prepare for TX.
			if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == RX) {
				MLMERXDisableRequest(&(gMsgHolder[gCurMsg].msg));
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

			gNextMsgToUse++;
			if (gNextMsgToUse >= MAX_NUM_MSG)
				gNextMsgToUse = 0;
			GW_EXIT_CRITICAL(ccrHolder);

			// Now that a Tx message is in the MACA queue it's OK to turn Rx task back on.
			//vTaskResume(gRadioReceiveTask);

			while ((funcErr != gSuccess_c) || (TX_MESSAGE_PENDING(gMsgHolder[txMsgNum].msg))) {
				// Don't loop around for another Tx message until we finish with this one.
			}
		} else {

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
