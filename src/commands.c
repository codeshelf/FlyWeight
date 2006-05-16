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

RemoteDescStruct		gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Local function prototypes

void createCommandHeader(BufferCntType inTXBufferNum, RadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

// --------------------------------------------------------------------------

UINT8 transmitCommand(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;
	
	// Setup the packet for transmission.
	//gsTxPacket.u8DataLength = gTXRadioBuffer[inTXBufferNum].bufferSize;
	//gsTxPacket.pu8Data = gTXRadioBuffer[inTXBufferNum].bufferStorage;
		
	advanceTXBuffer();

	// Transmit the packet.
	if (xQueueSend(gRadioTransmitQueue, &inTXBufferNum, pdFALSE)) {
	}
	
	return result;
				
};

// --------------------------------------------------------------------------

RadioCommandIDType getCommandNumber(BufferCntType inRXBufferNum) {
	
	// The command number is in the third half-byte of the message.
	RadioCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CMDID] & CMDMASK_CMDID) >> 4;
	return result;
};

// --------------------------------------------------------------------------

RemoteAddrType getCommandSrcAddr(BufferCntType inRXBufferNum) {
	
	// The source address is in the first half-byte of the message.
	RemoteAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ADDR] & CMDMASK_SRC_ADDR) >> 4;
	return result;
};

// --------------------------------------------------------------------------

RemoteAddrType getCommandDstAddr(BufferCntType inRXBufferNum) {
	
	// The source address is in the first half-byte of the message.
	RemoteAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ADDR] & CMDMASK_DST_ADDR);
	return result;
};

// --------------------------------------------------------------------------

void createCommandHeader(BufferCntType inTXBufferNum, RadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr) {

	// The first half-byte of the command is the source address.
	// The second half-byte of the command is the destination address.
	// The third half-byte of the command is the command number.
	// The fourth half-byte is reserved.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ADDR] = (inSrcAddr << 4) | inDestAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMDID] = (inCmdID << 4);
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = 2;
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
	
};

// --------------------------------------------------------------------------

void createWakeCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {
	
	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createCommandHeader(inTXBufferNum, eCommandWake, ADDR_CONTROLLER, ADDR_BROADCAST);
	
	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_WAKE_UID]), inUniqueID, UNIQUE_ID_LEN);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_WAKE_UID + UNIQUE_ID_LEN;;	
};

// --------------------------------------------------------------------------

void createAssignCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID, RemoteAddrType inRemoteAddr) {
	
	// The destination address is the address assigned to the remote with the matching ID.
	// (We could "broadcast" and have each remote check their assignment, but it seems unnecessary right now.)
	createCommandHeader(inTXBufferNum, eCommandAssign, ADDR_CONTROLLER, ADDR_BROADCAST);
	
	// The fourth half-byte is the assigned address.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSIGN_ADDR] = inRemoteAddr;
	
	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSIGN_UID]), inUniqueID, UNIQUE_ID_LEN);
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSIGN_UID + UNIQUE_ID_LEN;	
};

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {
	
	createCommandHeader(inTXBufferNum, eCommandQuery, ADDR_CONTROLLER, inRemoteAddr);
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = 2;	
};

// --------------------------------------------------------------------------

#define RESP_XML	"+C1:S+"
void createResponseCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {
	
	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createCommandHeader(inTXBufferNum, eCommandResponse, inRemoteAddr, ADDR_CONTROLLER);
	
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_RESPONSE]), RESP_XML, sizeof(RESP_XML));
		
	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_RESPONSE + sizeof(RESP_XML);	
};

// --------------------------------------------------------------------------

#define DESC_XML	"+C1:SS=8:SR=5556+"
void createDescCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {
	
	createCommandHeader(inTXBufferNum, eCommandDesc, ADDR_CONTROLLER, inRemoteAddr);
	
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_RESPONSE]), DESC_XML, sizeof(DESC_XML));
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_RESPONSE + sizeof(DESC_XML);	
};

// --------------------------------------------------------------------------

void createDataCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {
	
	createCommandHeader(inTXBufferNum, eCommandData, ADDR_CONTROLLER, inRemoteAddr);
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = 2;	
};

// --------------------------------------------------------------------------

void processWakeCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType	slot;
	RemoteAddrType	emptySlot = INVALID_REMOTE;
	RemoteAddrType	foundSlot = INVALID_REMOTE;
	
	// When we process the wake command we assign remote to one of the fourteen slots 
	// available to remote device addresses.  (Based on the remote's unique ID.)
	// Address is a 4-bit quantity where 0x00 is the controller and 0x0F is broadcast.
	
	// First search the table for the remote's unique ID, but keep track of the first
	// empty slot we found in case we need to assign the remote to a new slot.
	
	for (slot = 0; slot < MAX_REMOTES; slot ++) {
		// See if the slot is empty or if the unique ID matches.
		if (gRemoteStateTable[slot].remoteState == eRemoteStateUnknown) {
			emptySlot = slot;
		} else if (memcmp(gRemoteStateTable[slot].remoteUniqueID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_WAKE_UID]), UNIQUE_ID_LEN)) {
			foundSlot = slot;
			break;
		}
	}
	
	// Setup the remote's table entry.
	if (foundSlot != INVALID_REMOTE) {
		gRemoteStateTable[foundSlot].remoteState = eRemoteStateWakeRcvd;		
	} else if (emptySlot != INVALID_REMOTE) {
		memcpy(gRemoteStateTable[emptySlot].remoteUniqueID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_WAKE_UID]), UNIQUE_ID_LEN);
		gRemoteStateTable[emptySlot].remoteState = eRemoteStateWakeRcvd;
		foundSlot = emptySlot;
	};
		
	RELEASE_RX_BUFFER(inRXBufferNum);

	// If it's a valid slot then send a message to the gateway management task to handle the state change.
	if (foundSlot != INVALID_REMOTE) {
		if (xQueueSend(gGatewayMgmtQueue, &foundSlot, pdFALSE)) {
		}
	}	
	
};

// --------------------------------------------------------------------------

RemoteAddrType processAssignCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType result = INVALID_REMOTE;
	
	// Let's first make sure that this assign command is for us.
	memcmp(&kUniqueID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSIGN_UID]), UNIQUE_ID_LEN);
	
	// The destination address is the third half-byte of the command.
	result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSIGN_ADDR] & CMDMASK_ASSIGNID);
	
	RELEASE_RX_BUFFER(inRXBufferNum);
	
	return result;
	
};

// --------------------------------------------------------------------------

void processQueryCommand(BufferCntType inRXBufferNum, RemoteAddrType inSrcAddr) {

	createResponseCommand(gTXCurBufferNum, inSrcAddr);
	if (transmitCommand(gTXCurBufferNum)) {
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
	
// --------------------------------------------------------------------------

void processDescCommand(BufferCntType inRXBufferNum, RemoteAddrType inRemoteAddr) {

	gLocalDeviceState = eLocalStateDescRcvd;

	// This command contains the descriptor of the channel data that is to follow.
	
	// Hard coded to assume a sample size of 8 and a sample rate of 5556kSS.
	gMasterSampleRate = 16000000 / 5556;
	TPM2MOD = gMasterSampleRate;
	
	RELEASE_RX_BUFFER(inRXBufferNum);

	if (xQueueSend(gRemoteMgmtQueue, &inRemoteAddr, pdFALSE)) {
	}
	
};