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
#include "remoteMgmtTask.h"


// SMAC includes
#ifdef MC1321X
	#include "pub_def.h"
	#if defined(XBEE)
		#include "PWM_XBee.h"
	#elif defined(MC13192EVB)
		#include "PWM_EVB.h"
	#else
		#include "PWM_MC1321X.h"
	#endif
#else
#endif

void processRxPacket(BufferCntType inRxBufferNum);
void setupAudioLoader(void);
void setupPWM(void);

// --------------------------------------------------------------------------
// Global variables.

gwUINT8 			gu8RTxMode;
extern gwBoolean	gIsSleeping;
extern gwBoolean	gShouldSleep;
extern gwUINT8		gSleepCount;
extern gwUINT8		gButtonPressed;
extern gwUINT8		gCCRHolder;
extern portTickType	gLastPacketReceivedTick;

xTaskHandle			gRadioReceiveTask = NULL;
xTaskHandle			gRadioTransmitTask = NULL;
xTaskHandle			gRemoteManagementTask = NULL;


// The queue used to send data from the radio to the radio receive task.
xQueueHandle		gRadioTransmitQueue = NULL;
xQueueHandle		gRadioReceiveQueue = NULL;

gwTxPacket			gTxPacket;
gwRxPacket			gRxPacket;

// Radio input buffer
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRXCurBufferNum;
extern BufferCntType		gRXUsedBuffers;

extern RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType		gTXCurBufferNum;
extern BufferCntType		gTXUsedBuffers;
gwUINT8			        	gAssocCheckCount = 0;

EMessageHolderType	gMsgHolder[MAX_NUM_MSG];
gwUINT8				gNextMsgToUse = 0;
gwUINT8				gCurMsg = 0;
gwUINT8				gTotalPendingMsgs = 0;

portTickType		gLastAssocCheckTickCount;
UnixTimeType		gUnixTime;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType		rxBufferNum;
	gwUINT8				ccrHolder;
	FuncReturn_t 		funcErr;
	portTickType		readCheckTicks;

	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {

		gSleepCount = 0;

#ifdef USE_AUDIO
		setupAudioLoader();
		setupPWM();
#endif
		gLastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;

		for (;;) {

			GW_WATCHDOG_RESET;

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
					readCheckTicks = xTaskGetTickCount();
					while (RX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg)) {

						//						if (get_current_message() != gMsgHolder[gCurMsg].msg)
						//							break;

						funcErr = process_radio_msg();

						// If we have not received an RX in 200ms then check for sleep.
						if ((xTaskGetTickCount() - readCheckTicks) > (configTICK_RATE_HZ / 5)) {

							if (!gShouldSleep) {
								// We're not sleeping for now until we figure out a better way.
								//gShouldSleep = TRUE;
							} else {
								// We didn't get any packets before the RX timeout.  This is probably a quiet period, so pause for a while.
								sleepThisRemote(50);
							}

							// Every 10th time we wake from sleep send an AssocCheckCommand to the controller.
							//							if (gLocalDeviceState == eLocalStateRun) {
							//								gSleepCount++;
							//								if ((gSleepCount >= 9) && (gTXRadioBuffer[gTxAssocCheckBufferNum].bufferStatus == eBufferStateFree)) {
							//									gSleepCount = 0;
							//									gLastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
							//									gTxAssocCheckBufferNum = lockTXBuffer();
							//									createAssocCheckCommand(gTxAssocCheckBufferNum, (RemoteUniqueIDPtrType) GUID);
							//									if (transmitPacket(gTxAssocCheckBufferNum)){
							//									};
							//								}
							//							}
							readCheckTicks = xTaskGetTickCount();
						}
					}
					//Led1On();
					if (gMsgHolder[gCurMsg].msg.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS) {
						gRXRadioBuffer[gMsgHolder[gCurMsg].bufferNum].bufferSize = gMsgHolder[gCurMsg].msg.u8BufSize;
						processRxPacket(gMsgHolder[gCurMsg].bufferNum);
					} else {
					}
					//Led1Off();

					RELEASE_RX_BUFFER(gMsgHolder[gCurMsg].bufferNum, ccrHolder);

				} else if (gMsgHolder[gCurMsg].msg.u8Status.msg_type == TX) {
					//Led2On();
					while ((funcErr != gSuccess_c) || (TX_MESSAGE_PENDING(gMsgHolder[gCurMsg].msg))) {
						funcErr = process_radio_msg();
					}
					//Led2Off();
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

		/* Will only get here if the queue could not be created. */
		for ( ;; );
	}
}

// --------------------------------------------------------------------------

void processRxPacket(BufferCntType inRxBufferNum) {

	ECommandGroupIDType	cmdID;
	NetAddrType			cmdDstAddr;
	NetworkIDType		networkID;
	ECmdAssocType		assocSubCmd;
	portTickType		lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
	AckIDType			ackId;
	gwUINT8				ccrHolder;

	// The last read got a packet, so we're active.
	gShouldSleep = FALSE;

	// We just received a valid packet.
	networkID = getNetworkID(inRxBufferNum);

	// Only process packets sent to the broadcast network ID and our assigned network ID.
	if ((networkID != BROADCAST_NETID) && (networkID != gMyNetworkID)) {
		//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
	} else {

		cmdID = getCommandID(gRXRadioBuffer[inRxBufferNum].bufferStorage);
		cmdDstAddr = getCommandDstAddr(inRxBufferNum);

		// Only process commands sent to the broadcast address or our assigned address.
		if ((cmdDstAddr != ADDR_BROADCAST) && (cmdDstAddr != gMyAddr)) {
			//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
		} else {

			switch (cmdID) {

			case eCommandAssoc:
				// This will only return sub-commands if the command GUID matches out GUID
				assocSubCmd = getAssocSubCommand(inRxBufferNum);
				if (assocSubCmd == eCmdAssocInvalid) {
					//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
				} else if (assocSubCmd == eCmdAssocRESP) {
					// Reset the clock on the assoc check.
					lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
					// If we're not already running then signal the mgmt task that we just got a command ASSOC resp.
					if (gLocalDeviceState != eLocalStateRun) {
						if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType) 0)) {
						}
					}
				} else if (assocSubCmd == eCmdAssocACK) {
					// Record the time of the last ACK packet we received.
					gLastPacketReceivedTick = xTaskGetTickCount();

					gAssocCheckCount = 0;
					// If the associate state is 1 then we're not associated with this controller anymore.
					// We need to reset the device, so that we can start a whole new session.
					if (1 == gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_STATE]) {
						GW_RESET_MCU;
					} else {
						gUnixTime.byteFields.byte1 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 3];
						gUnixTime.byteFields.byte2 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 2];
						gUnixTime.byteFields.byte3 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 1];
						gUnixTime.byteFields.byte4 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME];
					}
				}
				//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
				break;

			case eCommandInfo:
				// Now that the remote has an assigned address we need to ask it to describe
				// it's capabilities and characteristics.
				processQueryCommand(inRxBufferNum, getCommandSrcAddr(inRxBufferNum));
				break;

			case eCommandControl:
				// If the packet requires an ACK then send it now.
				ackId = getAckId(inRxBufferNum);
				if (ackId != 0) {
					createAckCommand(gTXCurBufferNum, ackId);
					if (transmitPacket(gTXCurBufferNum)){
					}
				}

				// Make sure that there is a valid sub-command in the control command.
				switch (getControlSubCommand(inRxBufferNum)) {
				case eControlSubCmdEndpointAdj:
					break;

#if 0
				case eControlSubCmdMotor:
					processMotorControlSubCommand(inRxBufferNum);
					break;

				case eControlSubCmdHooBee:
					processHooBeeSubCommand(inRxBufferNum);
					break;
#endif
				case eCommandAudio:
					// Audio commands are handled by an interrupt routine.
					break;

				default:
					// Bogus command.
					// Immediately free this command buffer since we'll never do anything with it.
					//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
					break;

				}
			}
		}
	}
	// If we're running then send an AssocCheck every 5 seconds.
	if (gLocalDeviceState == eLocalStateRun) {
		if (lastAssocCheckTickCount < xTaskGetTickCount()) {
			lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
			createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
			if (transmitPacket(gTXCurBufferNum)){
			}
			gAssocCheckCount++;
			if (gAssocCheckCount > 10) {
				//	GW_RESET_MCU;
			}
		}
	}
}

// --------------------------------------------------------------------------

extern gwBoolean			gAudioModeRX;
extern SampleRateType		gMasterSampleRate;

void radioTransmitTask(void *pvParameters) {
	BufferCntType	txBufferNum;
	FuncReturn_t 	funcErr;
	gwUINT8			txMsgNum;
	gwUINT8			ccrHolder;

	for (;;) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

			gShouldSleep = FALSE;

			while(gTotalPendingMsgs >= 4) {
				// There's no message space in the queue, so wait.
				vTaskDelay(1);
			}

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

			gTotalPendingMsgs++;
			gNextMsgToUse++;
			if (gNextMsgToUse >= MAX_NUM_MSG)
				gNextMsgToUse = 0;
			GW_EXIT_CRITICAL(ccrHolder);

			while (TX_MESSAGE_PENDING(gMsgHolder[txMsgNum].msg)) {
				// Wait until this TX message is done, before we start another.
				vTaskDelay(1);
			}
		} else {

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
