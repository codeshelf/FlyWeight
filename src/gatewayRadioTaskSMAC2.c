/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "gatewayRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "gatewayMgmtTask.h"
#include "UART.h"
#include "UartLowLevel.h"

// --------------------------------------------------------------------------
// Global variables.

xTaskHandle gRadioReceiveTask = NULL;
xTaskHandle gRadioTransmitTask = NULL;
xTaskHandle gSerialReceiveTask = NULL;
xTaskHandle gGatewayManagementTask = NULL;

// The queue used to send data from the radio to the radio receive task.
xQueueHandle gRadioReceiveQueue = NULL;
xQueueHandle gRadioTransmitQueue = NULL;

// Radio buffers
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct gRXRadioBuffer[RX_BUFFER_COUNT];
extern RadioBufferStruct gTXRadioBuffer[TX_BUFFER_COUNT];

EMessageHolderType gRxMsgHolder;
EMessageHolderType gTxMsgHolder;

NetAddrType gMainRemote = INVALID_REMOTE;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum;
	gwUINT8 ccrHolder;
	FuncReturn_t funcErr;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {

			// Setup for the next RX cycle.
			rxBufferNum = lockRXBuffer();

			gRxMsgHolder.msg.pu8Buffer = (smac_pdu_t*) (gRXRadioBuffer[rxBufferNum].bufferRadioHeader);
			gRxMsgHolder.msg.u8BufSize = RX_BUFFER_SIZE;
			gRxMsgHolder.msg.u8Status.msg_type = RX;
			gRxMsgHolder.msg.u8Status.msg_state = MSG_RX_RQST;
			gRxMsgHolder.msg.cbDataIndication = NULL;
			gRxMsgHolder.bufferNum = rxBufferNum;

			funcErr = MLMERXEnableRequest(&(gRxMsgHolder.msg), 0);

			int delayCheck = 0;
			do {
				funcErr = process_radio_msg();

//				// Every kDelayCheckCount checks we should delay one 1ms, so that the OS idle tasks gets called.
//				if (delayCheck++ == kDelayCheckCount) {
//					vTaskDelay(1);
//					delayCheck = 0;
//				}
			} while ((funcErr != gSuccess_c) || (RX_MESSAGE_PENDING(gRxMsgHolder.msg)));

			if (gRxMsgHolder.msg.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS) {
				// Send the packet to the serial link.
				gRXRadioBuffer[gRxMsgHolder.bufferNum].bufferSize = gRxMsgHolder.msg.u8BufSize;
				serialTransmitFrame(UART_1, (gwUINT8*) (&gRXRadioBuffer[gRxMsgHolder.bufferNum].bufferStorage),
						gRxMsgHolder.msg.u8BufSize);
			}
			RELEASE_RX_BUFFER(gRxMsgHolder.bufferNum, ccrHolder);
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
	gwUINT8 ccrHolder;

	if (gRadioTransmitQueue) {

		// Turn the SCi back on by taking RX out of standby.
		GW_USB_INIT;

		for (;;) {

			// Wait until the UART controller signals us that we have a buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {

				// Setup for TX.
				gTxMsgHolder.msg.pu8Buffer = (smac_pdu_t *) ((gTXRadioBuffer[txBufferNum].bufferRadioHeader) + 1);
				gTxMsgHolder.msg.u8BufSize = gTXRadioBuffer[txBufferNum].bufferSize;
				gTxMsgHolder.msg.u8Status.msg_type = TX;
				gTxMsgHolder.msg.u8Status.msg_state = MSG_TX_RQST;
				gTxMsgHolder.msg.cbDataIndication = NULL;
				gTxMsgHolder.bufferNum = txBufferNum;

				GW_ENTER_CRITICAL(ccrHolder);
				funcErr = MLMERXDisableRequest(&(gRxMsgHolder.msg));
				vTaskSuspend(gRadioReceiveTask);
				GW_EXIT_CRITICAL(ccrHolder);

				do {
					funcErr = MCPSDataRequest(&(gTxMsgHolder.msg));

					// If the radio can't TX then we're in big trouble.  Just reset.
					if (funcErr != gSuccess_c) {
						GW_RESET_MCU()
					}

					do {
						// Wait until this TX message is done, before we start another.
						funcErr = process_radio_msg();
					} while (TX_MESSAGE_PENDING(gTxMsgHolder.msg));
				} while (gTxMsgHolder.msg.u8Status.msg_state == MSG_TX_ACTION_COMPLETE_FAIL);

				RELEASE_TX_BUFFER(gTxMsgHolder.bufferNum, ccrHolder);
				gRxMsgHolder.msg.u8Status.msg_state = MSG_RX_RQST;
				funcErr = MLMERXEnableRequest(&(gRxMsgHolder.msg), 0);
				vTaskResume(gRadioReceiveTask);

			} else {

			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
