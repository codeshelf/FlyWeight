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

#ifdef _TOY_NETWORK_
	#include "toyQuery.h"
#else
	#include "terminalQuery.h"
#endif

RemoteDescStruct	gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ERadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

// --------------------------------------------------------------------------

UINT8 transmitPacket(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;

	// Setup the packet for transmission.
	//gsTxPacket.u8DataLength = gTXRadioBuffer[inTXBufferNum].bufferSize;
	//gsTxPacket.pu8Data = gTXRadioBuffer[inTXBufferNum].bufferStorage;
	
	// Make sure that the packet size gets set in the first byte of the packet.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SIZE] = gTXRadioBuffer[inTXBufferNum].bufferSize - PACKET_HEADER_BYTES;

	//advanceTXBuffer();

	// Transmit the packet.
	if (xQueueSend(gRadioTransmitQueue, &inTXBufferNum, pdFALSE)) {}

	return result;

};

// --------------------------------------------------------------------------

ERadioCommandIDType getCommandNumber(BufferCntType inRXBufferNum) {

	// The command number is in the third half-byte of the packet.
	ERadioCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CMDID] & CMDMASK_CMDID) >> 4;
	return result;
};

// --------------------------------------------------------------------------

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum) {

	// The command number is in the third half-byte of the packet.
	EndpointNumType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ENDPOINT] & CMDMASK_ENDPOINT);
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

ERadioControlCommandIDType getControlCommandNumber(BufferCntType inRXBufferNum) {

	// The control command number is in the fourth half-byte of the packet.
	ERadioControlCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CTRLID]);
	return result;
};

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, ERadioCommandIDType inCmdID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr) {

	gTXRadioBuffer[inTXBufferNum].bufferSize = 0;
	
	// The first byte of the packet is the header.
	// The next byte of the packet is the packet length.
	// The next half byte of the packet is the src address
	// The next half byte of the packet is the dst address
	// <--- Now the command starts --->
	// The first half byte of the command is the command ID.
	// The next half byte of the command is reserved.
	// The remaining bytes of the command (and packet) is the command data.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_VERSION] = (0x01 << 6) | (0x01 << 3);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SIZE] = 3;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ADDR] = (inSrcAddr << 4) | inDestAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMDID] = (inCmdID << 4);

	gTXRadioBuffer[inTXBufferNum].bufferSize += 4;
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
};

// --------------------------------------------------------------------------

void createWakeCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandWake, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Tell the controller what protocol we know/use.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_DEVICE_TYPE] = DEVICE_REMOTE;
	
	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_WAKE_UID]), inUniqueID, UNIQUE_ID_BYTES);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + DEVICE_TYPE_BYTES + UNIQUE_ID_BYTES;
};

// --------------------------------------------------------------------------

void createAddrAssignAckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAddrAssignAck, gMyAddr, ADDR_CONTROLLER);

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSIGNACK_UID]), inUniqueID, UNIQUE_ID_BYTES);
	
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSIGNACK_ADDR] = (gMyAddr << 4);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + UNIQUE_ID_BYTES + 1;
};

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandQuery, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD;
};

// --------------------------------------------------------------------------

void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, RemoteAddrType inRemoteAddr) {

	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createPacket(inTXBufferNum, eCommandResponse, gMyAddr, inRemoteAddr);

	//memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_RESPONSE]), inResponseBuffer, inResponseSize);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_RESPONSE + inResponseSize;
};

// --------------------------------------------------------------------------

void createControlCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandControl, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD;
};

// --------------------------------------------------------------------------

void processAssignCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType result = INVALID_REMOTE;

	// Let's first make sure that this assign command is for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSIGN_UID]), UNIQUE_ID_BYTES) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSIGN_ADDR] & CMDMASK_ASSIGNID) >> 4;	
	}

	RELEASE_RX_BUFFER(inRXBufferNum);
	
	createAddrAssignAckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
	if (transmitPacket(gTXCurBufferNum)){
	};	
	gLocalDeviceState = eLocalStateAddrAssignAckSent;

};

// --------------------------------------------------------------------------

void processQueryCommand(BufferCntType inRXBufferNum, RemoteAddrType inSrcAddr) {

	processQuery(inRXBufferNum, CMDPOS_QUERY, inSrcAddr);
	RELEASE_RX_BUFFER(inRXBufferNum);
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

EMotorCommandType getMotorCommand(BufferCntType inRXBufferNum) {

	EMotorCommandType result = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1];
	
	return result;
}

// --------------------------------------------------------------------------

void processMotorControlCommand(BufferCntType inRXBufferNum) {

#define MOTOR1_FREE		0b11111100
#define MOTOR1_FWD		0b00000010
#define MOTOR1_BWD		0b00000001
#define MOTOR1_BRAKE	0b00000011

#define MOTOR2_FREE		0b11110011
#define MOTOR2_FWD		0b00001000
#define MOTOR2_BWD		0b00000100
#define MOTOR2_BRAKE	0b00001100

	// Map the endpoint to the motor.
	EndpointNumType endpoint = getEndpointNumber(inRXBufferNum);
	EMotorCommandType motorCommand = getMotorCommand(inRXBufferNum);
	
	PTBDD = 0b11111111;
	switch (endpoint) {
		case MOTOR1_ENDPOINT:
			PTBD &= MOTOR1_FREE;
			
			switch (motorCommand) {
				case eMotorCommandFwd:
					PTBD |= MOTOR1_FWD;
					break;
			
				case eMotorCommandBwd:
					PTBD |= MOTOR1_BWD;
					break;
			
				case eMotorCommandBrake:
					PTBD |= MOTOR1_BRAKE;
					break;
			}
			break;
			
		case MOTOR2_ENDPOINT:
			PTBD &= MOTOR2_FREE;
			
			switch (motorCommand) {
				case eMotorCommandFwd:
					PTBD |= MOTOR2_FWD;
					break;
			
				case eMotorCommandBwd:
					PTBD |= MOTOR2_BWD;
					break;
			
				case eMotorCommandBrake:
					PTBD |= MOTOR2_BRAKE;
					break;
			}
			break;
			
		default:
			break;	
	}
	
	RELEASE_RX_BUFFER(inRXBufferNum);
}