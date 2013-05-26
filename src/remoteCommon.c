/*
   FlyWeight
   � Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved

   $Id$
   $Name$
*/

#include "remoteCommon.h"
#include "remoteRadioTask.h"
#include "commands.h"
#include "string.h"

// --------------------------------------------------------------------------
// Global variables.

extern portTickType 			gLastPacketReceivedTick;
UnixTimeType 					gUnixTime;

extern gwBoolean 				gShouldSleep;
extern NetAddrType				gMyAddr;
extern NetworkIDType			gMyNetworkID;
extern xQueueHandle				gRemoteMgmtQueue;
extern portTickType 			gLastAssocCheckTickCount;
extern portTickType 			gLastAssocCheckTickCount;
extern gwUINT8 					gAssocCheckCount;


// --------------------------------------------------------------------------

void processRxPacket(BufferCntType inRxBufferNum) {

	ECommandGroupIDType cmdID;
	NetAddrType cmdDstAddr;
	NetworkIDType networkID;
	ECmdAssocType assocSubCmd;
	AckDataType ackData;
	AckIDType ackId;
	gwUINT8 ccrHolder;
	EControlCmdAckStateType ackState;
	gwBoolean shouldReleasePacket;

	shouldReleasePacket = TRUE;

	// The last read got a packet, so we're active.
	gShouldSleep = FALSE;

	// We just received a valid packet.
	networkID = getNetworkID(inRxBufferNum);

	// Only process packets sent to the broadcast network ID and our assigned network ID.
	if ((networkID != BROADCAST_NET_NUM) && (networkID != gMyNetworkID)) {
		// Do nothing
	} else {

		cmdID = getCommandID(gRXRadioBuffer[inRxBufferNum].bufferStorage);
		cmdDstAddr = getCommandDstAddr(inRxBufferNum);

		// Only process commands sent to the broadcast address or our assigned address.
		if ((cmdDstAddr != ADDR_BROADCAST) && (cmdDstAddr != gMyAddr)) {
			// Do nothing.
		} else {

			// Prepare to handle packet ACK.
			ackId = getAckId(inRxBufferNum);
			ackState = eAckStateNotNeeded;
			memset(ackData, 0, ACK_DATA_BYTES);

			switch (cmdID) {

				case eCommandAssoc:
					// This will only return sub-commands if the command GUID matches out GUID
					assocSubCmd = getAssocSubCommand(inRxBufferNum);
					if (assocSubCmd == eCmdAssocInvalid) {
						// Do nothing.
					} else if (assocSubCmd == eCmdAssocRESP) {
						// Reset the clock on the assoc check.
						gLastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
						// If we're not already running then signal the mgmt task that we just got a command ASSOC resp.
						if (gLocalDeviceState != eLocalStateRun) {
							if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType) 0)) {
								// The management task will handle this packet.
								shouldReleasePacket = FALSE;
							}
						}
					} else if (assocSubCmd == eCmdAssocACK) {
						// Record the time of the last ACK packet we received.
						gLastPacketReceivedTick = xTaskGetTickCount();

						gAssocCheckCount = 0;
						// If the associate state is 1 then we're not associated with this controller anymore.
						// We need to reset the device, so that we can start a whole new session.
						if (1 == gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_STATE]) {
							GW_RESET_MCU();
						} else {
							gUnixTime.byteFields.byte1 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 3];
							gUnixTime.byteFields.byte2 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 2];
							gUnixTime.byteFields.byte3 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 1];
							gUnixTime.byteFields.byte4 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME];
						}
					}
					break;

				case eCommandInfo:
					// Now that the remote has an assigned address we need to ask it to describe
					// it's capabilities and characteristics.
					processQueryCommand(inRxBufferNum, getCommandSrcAddr(inRxBufferNum));
					break;

				case eCommandControl:

					// Make sure that there is a valid sub-command in the control command.
					switch (getControlSubCommand(inRxBufferNum)) {
						case eControlSubCmdEndpointAdj:
							break;

#ifdef IS_CODESHELF
						case eControlSubCmdScan:
							break;

						case eControlSubCmdMessage:
							break;

						case eControlSubCmdLight:
							ackState = processLedSubCommand(inRxBufferNum, ackId, ackData);
							break;

#endif

#ifdef MOTOR_CONTROLLER
							case eControlSubCmdMotor:
							processMotorControlSubCommand(inRxBufferNum);
							break;
#endif

#ifdef IS_HOOBEE
							case eControlSubCmdHooBee:
							ackState = processHooBeeSubCommand(inRxBufferNum);
							break;
#endif

#ifdef IS_WALKIETALKIE
							case eCommandAudio:
							// Audio commands are handled by an interrupt routine.
							break;
#endif

#ifdef IS_PFC
						case eControlSubCmdSDCardUpdate:
							// By processing the SDCard updates in the critical region,
							// it prevents the gateway from sending another update until this
							// one completes, because we wont send an ACK until it completes.
							GW_ENTER_CRITICAL(ccrHolder);
							ackState = processSDCardUpdateSubCommand(inRxBufferNum);
							GW_EXIT_CRITICAL(ccrHolder);
							break;

						case eControlSubCmdSDCardUpdateCommit:
							// By processing the SDCard updates in the critical region,
							// it prevents the gateway from sending another update until this
							// one completes, because we wont send an ACK until it completes.
							GW_ENTER_CRITICAL(ccrHolder);
							ackState = processSDCardUpdateCommitSubCommand(inRxBufferNum, ackData);
							GW_EXIT_CRITICAL(ccrHolder);
							break;

						case eControlSubCmdSDCardModeControl:
							// By processing the SDCard mode control in the critical region,
							// it prevents the gateway from sending another update until this
							// one completes, because we wont send an ACK until it completes.
							GW_ENTER_CRITICAL(ccrHolder);
							ackState = processSDCardModeSubCommand(inRxBufferNum, ackId, ackData);
							GW_EXIT_CRITICAL(ccrHolder);
							break;

						case eControlSubCmdSDCardBlockCheck:
							// By processing the SDCard checks in the critical region,
							// it prevents the gateway from sending another update until this
							// one completes, because we wont send an ACK until it completes.
							GW_ENTER_CRITICAL(ccrHolder);
							ackState = processSDCardBlockCheckSubCommand(inRxBufferNum);
							GW_EXIT_CRITICAL(ccrHolder);
							break;
#endif
						default:
							// Bogus command.
							// Immediately free this command buffer since we'll never do anything with it.
							//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
							break;
					}
					break;

				default:
					break;
			}

			// Send an ACK if necessary.
			if ((ackState == eAckStateOk) && (ackId != 0)) {
				BufferCntType txBufferNum = lockTXBuffer();
				createAckPacket(txBufferNum, ackId, ackData);
				if (transmitPacket(txBufferNum)) {
				}
			}

		}
	}

	// If we need to release the packet then do it.
	if (shouldReleasePacket) {
		RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
	}
}
