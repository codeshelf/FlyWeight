/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "remoteMgmtTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwSystemMacros.h"
#include "gwTypes.h"
#include "commands.h"
#include "TransceiverPowerMngmnt.h"

xQueueHandle gRemoteMgmtQueue;
portTickType gLastUserAction;
ELocalStatusType gLocalDeviceState;
extern gwUINT8 gAssocCheckCount;
extern gwUINT8 gCCRHolder;
extern portTickType gSleepWaitMillis;
void preSleep();

// --------------------------------------------------------------------------

void remoteMgmtTask(void *pvParameters) {
	gwUINT8 ccrHolder;
	BufferCntType rxBufferNum = 0;
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	gwBoolean associated;
	gwBoolean checked;
	gwUINT8 trim = 128;
	gwUINT8 assocAttempts = 0;
	ECommandGroupIDType cmdID;
	ECmdAssocType assocSubCmd;
	portTickType ticksSinceLastUserEvent;

	if (gRemoteMgmtQueue) {

		/*
		 * Attempt to associate with our controller.
		 * 1. Send an associate request.
		 * 2. Wait up to 10ms for a response.
		 * 3. If we get a response then start the main proccessing
		 * 4. If no response then change channels and start at step 1.
		 */
		channel = 0;
		associated = FALSE;
		while (!associated) {

			GW_WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			MLMESetChannelRequest(channel);

			// Send an associate request on the current channel.
			txBufferNum = lockTXBuffer();
			createAssocReqCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
			if (transmitPacket(txBufferNum)) {
			};

			// Wait up to 1000ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 1000 * portTICK_RATE_MS) == pdPASS) {
				if (rxBufferNum != 255) {
					// Check to see what kind of command we just got.
					cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
					if (cmdID == eCommandAssoc) {
						assocSubCmd = getAssocSubCommand(rxBufferNum);
						if (assocSubCmd == eCmdAssocRESP) {
							gLocalDeviceState = eLocalStateAssocRespRcvd;
							processAssocRespCommand(rxBufferNum);
							if (gLocalDeviceState == eLocalStateAssociated) {
								associated = TRUE;
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			}

			// If we're still not associated then change channels.
			if (!associated) {
				//	MLMEMC13192XtalAdjust(trim--);
				channel++;
				if (channel > 16) {
					channel = 0;
					assocAttempts++;
					if (assocAttempts % 4) {
						sleep();
					}
				}
			}
		}
		gLocalDeviceState = eLocalStateRun;

		// Check for sleep time.
		gLastUserAction = xTaskGetTickCount();

		checked = FALSE;
		for (;;) {
			if (!checked) {
				BufferCntType txBufferNum = lockTXBuffer();
				createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
				if (transmitPacket(txBufferNum)) {
				}

				// Wait up to 1000ms for a response.
				if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 1000 * portTICK_RATE_MS) == pdPASS) {
					if (rxBufferNum != 255) {
						// Check to see what kind of command we just got.
						cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
						if (cmdID == eCommandAssoc) {
							assocSubCmd = getAssocSubCommand(rxBufferNum);
							if (assocSubCmd == eCmdAssocACK) {
								checked = TRUE;
							}
						}
						RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
					}
				}
				vTaskDelay(50);
			}

			vTaskDelay(100);

			ticksSinceLastUserEvent = xTaskGetTickCount() - gLastUserAction;
			if ((gSleepWaitMillis != 0) && (ticksSinceLastUserEvent > gSleepWaitMillis)) {
				sleep();
			}
		}
	}

	//vTaskSuspend(gRemoteManagementTask);

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
