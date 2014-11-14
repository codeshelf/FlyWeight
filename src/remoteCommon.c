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

AckIDType gLastAckId = 0;
UnixTimeType gUnixTime;

extern portTickType gLastPacketReceivedTick;
extern gwBoolean gShouldSleep;
extern NetAddrType gMyAddr;
extern NetworkIDType gMyNetworkID;
extern xQueueHandle gRemoteMgmtQueue;
extern portTickType gLastAssocCheckTickCount;
extern portTickType gLastAssocCheckTickCount;
extern gwUINT8 gAssocCheckCount;

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
	BufferCntType txBufferNum;

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
			ackId = getAckId(gRXRadioBuffer[inRxBufferNum].bufferStorage);
			ackState = eAckStateNotNeeded;
			memset(ackData, 0, ACK_DATA_BYTES);

			if ((ackId > gLastAckId) || (ackId == 1) || (ackId == 0)) {

				if (ackId != 0) {
					gLastAckId = ackId;
				}

				switch (cmdID) {

					case eCommandNetMgmt:
						// We've received a beacon, so reset the watchdog timer.
						GW_WATCHDOG_RESET;
						break;

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
								if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType ) 0)) {
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
								//GW_RESET_MCU();
							} else {
								gUnixTime.byteFields.byte1 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 3];
								gUnixTime.byteFields.byte2 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 2];
								gUnixTime.byteFields.byte3 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 1];
								gUnixTime.byteFields.byte4 = gRXRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME];
							}
							if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType ) 0)) {
								// The management task will handle this packet.
								shouldReleasePacket = FALSE;
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
	//						case eControlSubCmdEndpointAdj:
	//							break;

						case eControlSubCmdScan:
							break;

						case eControlSubCmdMessage:
							ackState = processDisplayMsgSubCommand(inRxBufferNum);
							break;

						case eControlSubCmdLight:
							ackState = processLedSubCommand(inRxBufferNum);
							break;

						case eControlSubCmdSetPosController:
							ackState = processSetPosControllerSubCommand(inRxBufferNum);
							break;

						case eControlSubCmdClearPosController:
							ackState = processClearPosControllerSubCommand(inRxBufferNum);
							break;

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
			}

			// Send an ACK if necessary.
			if ((ackState == eAckStateOk) && (ackId != 0)) {
				txBufferNum = lockTXBuffer();
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
