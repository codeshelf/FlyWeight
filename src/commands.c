/*
  FlyWeight
  © Copyright 2005, 2006 Jeffrey B. Williams
  All rights reserved
  
  $Id$
  $Name$	
*/

#include "commands.h"
#include "radioCommon.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"
#include "simple_mac.h"
#include "gatewayRadioTask.h"
#include "gatewayMgmtTask.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"

RemoteDescStruct	gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, RadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

// --------------------------------------------------------------------------

UINT8 transmitPacket(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;

	// Setup the packet for transmission.
	//gsTxPacket.u8DataLength = gTXRadioBuffer[inTXBufferNum].bufferSize;
	//gsTxPacket.pu8Data = gTXRadioBuffer[inTXBufferNum].bufferStorage;
	
	// Make sure that the packet size gets set in the first byte of the packet.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[0] = gTXRadioBuffer[inTXBufferNum].bufferSize;

	advanceTXBuffer();

	// Transmit the packet.
	if (xQueueSend(gRadioTransmitQueue, &inTXBufferNum, pdFALSE)) {}

	return result;

};

// --------------------------------------------------------------------------

RadioCommandIDType getCommandNumber(BufferCntType inRXBufferNum) {

	// The command number is in the third half-byte of the packet.
	RadioCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CMDID] & CMDMASK_CMDID) >> 4;
	return result;
};

// --------------------------------------------------------------------------

RemoteAddrType getCommandSrcAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	RemoteAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_ADDR] & CMDMASK_SRC_ADDR) >> 4;
	return result;
};

// --------------------------------------------------------------------------

RemoteAddrType getCommandDstAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	RemoteAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_ADDR] & CMDMASK_DST_ADDR);
	return result;
};

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, RadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr) {

	gTXRadioBuffer[inTXBufferNum].bufferSize = 0;
	
	// The first byte of the packet is the packet length.
	// The next half byte of the packet is the src address
	// The next half byte of the packet is the dst address
	// <--- Now the command starts --->
	// The first half byte of the command is the command ID.
	// The next half byte of the command is reserved.
	// The remaining bytes of the command (and packet) is the command data.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SIZE] = 3;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ADDR] = (inSrcAddr << 4) | inDestAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMDID] = (inCmdID << 4);

	gTXRadioBuffer[inTXBufferNum].bufferSize += 3;
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
};

// --------------------------------------------------------------------------

void createWakeCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandWake, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Tell the controller what protocol we know/use.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_PROTOCOL_ID] = CURRENT_PROTOCOL_NUM;
	
	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_WAKE_UID]), inUniqueID, UNIQUE_ID_LEN);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + PROTOCOL_ID_LEN + UNIQUE_ID_LEN;
};

// --------------------------------------------------------------------------

void createAddrAssignAckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAddrAssignAck, gMyAddr, ADDR_CONTROLLER);

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSIGNACK_UID]), inUniqueID, UNIQUE_ID_LEN);
	
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSIGNACK_ADDR] = (gMyAddr << 4);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + UNIQUE_ID_LEN + 1;
};

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandQuery, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD;
};

// --------------------------------------------------------------------------

#define RESP_XML	"+C1:S+"
void createResponseCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createPacket(inTXBufferNum, eCommandResponse, gMyAddr, inRemoteAddr);

	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_RESPONSE]), RESP_XML, sizeof(RESP_XML));

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + sizeof(RESP_XML);
};

// --------------------------------------------------------------------------

void createDataCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandDatagram, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD;
};

// --------------------------------------------------------------------------

void processAssignCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType result = INVALID_REMOTE;

	// Let's first make sure that this assign command is for us.
	if (memcmp(&kUniqueID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSIGN_UID]), UNIQUE_ID_LEN) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSIGN_ADDR] & CMDMASK_ASSIGNID) >> 4;	
	}

	RELEASE_RX_BUFFER(inRXBufferNum);
	
	createAddrAssignAckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) &kUniqueID);
	if (transmitPacket(gTXCurBufferNum)){
	};	
	gLocalDeviceState = eLocalStateAddrAssignAckSent;

};

// --------------------------------------------------------------------------

void processQueryCommand(BufferCntType inRXBufferNum, RemoteAddrType inSrcAddr) {

	createResponseCommand(gTXCurBufferNum, inSrcAddr);
	if (transmitPacket(gTXCurBufferNum)){
	};	
	gLocalDeviceState = eLocalStateRespSent;
};

// --------------------------------------------------------------------------

void processResponseCommand(BufferCntType inRXBufferNum, RemoteAddrType inRemoteAddr) {

	// Indicate that we received a response for the remote.
	gRemoteStateTable[inRemoteAddr].remoteState = eRemoteStateRespRcvd;

	RELEASE_RX_BUFFER(inRXBufferNum);

	// Figure out what channels are available.

	if (xQueueSend(gGatewayMgmtQueue, &inRemoteAddr, pdFALSE)) {
	}

}