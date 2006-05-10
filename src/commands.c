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

BufferCntType createCommandHeader(BufferCntType inTXBufferNum, RadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

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
	RadioCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[1] & 0xF0) >> 4;
	return result;
};

// --------------------------------------------------------------------------

RemoteAddrType getCommandSrcAddr(BufferCntType inRXBufferNum) {
	
	// The source address is in the first half-byte of the message.
	RemoteAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[0] & 0xF0) >> 4;
	return result;
};

// --------------------------------------------------------------------------

BufferCntType createCommandHeader(BufferCntType inTXBufferNum, RadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr) {

	// The first half-byte of the command is the source address.
	// The second half-byte of the command is the destination address.
	// The third half-byte of the command is the command number.
	// The fourth half-byte is reserved.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[0] = (inSrcAddr << 4) | inDestAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[1] = (inCmdID << 4);
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = 2;
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
	
	return 2;
};

// --------------------------------------------------------------------------

void createWakeCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {
	
	BufferCntType	pos = 0;

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	pos = createCommandHeader(inTXBufferNum, eCommandWake, ADDR_CONTROLLER, ADDR_BROADCAST);
	
	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[pos]), inUniqueID, UNIQUE_ID_LEN);
	pos += UNIQUE_ID_LEN;	
	gTXRadioBuffer[inTXBufferNum].bufferSize = pos;	
};

// --------------------------------------------------------------------------

void createAssignCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID, RemoteAddrType inRemoteAddr) {
	
	BufferCntType	pos = 0;
	
	// The destination address is the address assigned to the remote with the matching ID.
	// (We could "broadcast" and have each remote check their assignment, but it seems unnecessary right now.)
	pos = createCommandHeader(inTXBufferNum, eCommandAssign, ADDR_CONTROLLER, ADDR_BROADCAST);
	
	// The third half-byte is the assigned address.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[pos] = (inRemoteAddr << 4);
	pos += 1;
	
	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[pos]), inUniqueID, UNIQUE_ID_LEN);
	pos += UNIQUE_ID_LEN;
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = pos;	
};

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {
	
	BufferCntType	pos = 0;

	pos = createCommandHeader(inTXBufferNum, eCommandQuery, ADDR_CONTROLLER, inRemoteAddr);
	
};

// --------------------------------------------------------------------------

#define RESP_XML	"+C1:S+"
void createResponseCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {
	
	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	BufferCntType	pos = 0;

	pos = createCommandHeader(inTXBufferNum, eCommandQuery, inRemoteAddr, ADDR_CONTROLLER);
	
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[pos]), RESP_XML, sizeof(RESP_XML));
	pos += sizeof(RESP_XML);
		
	gTXRadioBuffer[inTXBufferNum].bufferSize = pos;	
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
		} else if (memcmp(gRemoteStateTable[slot].remoteUniqueID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[1]), UNIQUE_ID_LEN)) {
			foundSlot = slot;
			break;
		}
	}
	
	// Setup the remote's table entry.
	if (foundSlot != INVALID_REMOTE) {
		gRemoteStateTable[foundSlot].remoteState = eRemoteStateWakeRcvd;		
	} else if (emptySlot != INVALID_REMOTE) {
		memcpy(gRemoteStateTable[emptySlot].remoteUniqueID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[1]), UNIQUE_ID_LEN);
		gRemoteStateTable[emptySlot].remoteState = eRemoteStateWakeRcvd;
		foundSlot = emptySlot;
	};
	
	// If it's a valid slot then send a message to the gateway management task to handle the state change.
	if (foundSlot != INVALID_REMOTE) {
		if (xQueueSend(gGatewayMgmtQueue, &foundSlot, pdFALSE)) {
		}
	}	
	
};

// --------------------------------------------------------------------------

RemoteAddrType processAssignCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType result = INVALID_REMOTE;
	
	// The destination address is the third half-byte of the command.
	result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[1] & 0xF) >> 4;
	
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

	if (xQueueSend(gGatewayMgmtQueue, &inRemoteAddr, pdFALSE)) {
	}
	
};