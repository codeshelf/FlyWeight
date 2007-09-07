/*
  FlyWeight
  � Copyright 2005, 2006 Jeffrey B. Williams
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
NetworkIDType		gMyNetworkID = 0x00;

// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ECommandIDType inCmdID, NetworkIDType inNetworkID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

// --------------------------------------------------------------------------

UINT8 transmitPacket(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;

	// Transmit the packet.
	if (xQueueSend(gRadioTransmitQueue, &inTXBufferNum, pdFALSE)) {}

	return result;

};

// --------------------------------------------------------------------------

ECommandIDType getCommand(BufferCntType inRXBufferNum) {

	// The command number is in the third half-byte of the packet.
	ECommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CMDID] & CMDMASK_CMDID) >> 4;
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

ENetMgmtSubCommandIDType getNetMgmtSubCommand(BufferCntType inTXBufferNum) {
	// These come in on the TC buffer from the controller, but they don't get transmitted to the air.
	ENetMgmtSubCommandIDType result = (gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD]);
	return result;
};

// --------------------------------------------------------------------------

EAssocSubCommandIDType getAssocSubCommand(BufferCntType inRXBufferNum) {
	EAssocSubCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD]);
	return result;
};

// --------------------------------------------------------------------------

EControlSubCommandIDType getControlSubCommand(BufferCntType inRXBufferNum) {
	EControlSubCommandIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD]);
	return result;
};

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, ECommandIDType inCmdID, NetworkIDType inNetworkID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr) {

	// The first byte of the packet is the header.
	// The next byte of the packet is the packet length.
	// The next half byte of the packet is the src address
	// The next half byte of the packet is the dst address
	// <--- Now the command starts --->
	// The first half byte of the command is the command ID.
	// The next half byte of the command is reserved.
	// The remaining bytes of the command (and packet) is the command data.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_NETID] |= (inNetworkID << SHIFTBITS_PKT_NETID);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ADDR] = (inSrcAddr << SHIFTBITS_PKT_SRCADDR) | inDestAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMDID] = (inCmdID << SHIFTBITS_CMDID);

	gTXRadioBuffer[inTXBufferNum].bufferSize += 4;
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
};

// --------------------------------------------------------------------------

void createNetSetupCommand(BufferCntType inTXBufferNum, NetworkIDType inNetworkID, ChannelNumberType inChannelNumber) {

	// This command gets setup in the TX buffers, because it only gets sent back to the controller via
	// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
	// directly.
	
	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_NETID] |= (BROADCAST_NETID << SHIFTBITS_PKT_NETID);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ADDR] = (ADDR_CONTROLLER << SHIFTBITS_PKT_SRCADDR) | ADDR_CONTROLLER;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMDID] = (eCommandNetMgmt << SHIFTBITS_CMDID);
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
	
	// Set the sub-command.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCommandSetup;

	// Set the network ID.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SETUP_NETID] = inNetworkID;

	// Set the channel number.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SETUP_CHANNEL] = inChannelNumber;
	
	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_SETUP_CHANNEL + 1;
};

// --------------------------------------------------------------------------

void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	
	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eAssocSubCommandReq;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_VER] = 0x01;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_VER + 1;
};

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandQuery, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + 1;
};

// --------------------------------------------------------------------------

void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, RemoteAddrType inRemoteAddr) {

	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createPacket(inTXBufferNum, eCommandResponse, gMyNetworkID, gMyAddr, inRemoteAddr);

	//memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_RESPONSE]), inResponseBuffer, inResponseSize);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_RESPONSE + inResponseSize + 1;
};

// --------------------------------------------------------------------------

void createControlCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + 1;
};

// --------------------------------------------------------------------------

void processNetSetupCommand(BufferCntType inTXBufferNum) {

	NetworkIDType				networkID;
	ChannelNumberType			channel;
	ChannelNumberType			selectedChannel;
//	UINT8						energyLevel;
//	UINT8						minEnergyLevel;
	UINT8						buffer[16];
	BufferCntType				txBufferNum;

	// Network Setup ALWAYS comes in via the serial interface to the gateway (dongle)
	// This means we process it FROM the TX buffers and SEND from the TX buffers.
	// These commands NEVER go onto the air.
	
	// Get the requested networkID
	networkID = (gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SETUP_NETID] & CMDMASK_NETID) >> SHIFTBITS_CMDNETID;
	
	// Get the requested channel number.
	channel = gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SETUP_CHANNEL];
	
	RELEASE_TX_BUFFER(inTXBufferNum);
	
	if (channel == AUTOMATIC_CHANNEL) {
		// If the channel number is automatic then perform a search of the channels.
		selectedChannel = MLMEScanRequest(SCAN_MODE_CCA, buffer);
	} else {
		// If the channel number is specified then just use that channel.
		selectedChannel = channel;
	}
	MLMESetChannelRequest(selectedChannel);	
	gMyNetworkID = networkID;
	
	// Now send back a network setup response.
	// Wait until we can get an TX buffer
	while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1);
	}
	EnterCritical();
		txBufferNum = gTXCurBufferNum;
		advanceTXBuffer();
	ExitCritical();
	createNetSetupCommand(txBufferNum, networkID, selectedChannel);
	
	// Now send the command to the queue that sends packets to the controller.
	if (xQueueSend(gGatewayMgmtQueue, &txBufferNum, pdFALSE)) {
	}
	
	gLocalDeviceState = eLocalStateRun;
};

// --------------------------------------------------------------------------

void processAssocRespCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType result = INVALID_REMOTE;

	// Let's first make sure that this assign command is for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), UNIQUE_ID_BYTES) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR] & CMDMASK_ASSIGNID) >> SHIFTBITS_CMDID;	
		gLocalDeviceState = eLocalStateAssociated;
	}

	RELEASE_RX_BUFFER(inRXBufferNum);
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

//	if (xQueueSend(gGatewayMgmtQueue, &inRXBufferNum, pdFALSE)) {
//	}

}

// --------------------------------------------------------------------------

EMotorCommandType getMotorCommand(BufferCntType inRXBufferNum) {

	EMotorCommandType result = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1];
	
	return result;
}

// --------------------------------------------------------------------------

void processMotorControlSubCommand(BufferCntType inRXBufferNum) {

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