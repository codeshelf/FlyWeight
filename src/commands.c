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
#include "NV_Data.h"
#include "flash.h"

#ifdef _TOY_NETWORK_
	#include "toyQuery.h"
#else
	#include "terminalQuery.h"
#endif

RemoteDescStruct	gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

// --------------------------------------------------------------------------

UINT8 transmitPacket(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;

	// Transmit the packet.
	if (xQueueSend(gRadioTransmitQueue, &inTXBufferNum, pdFALSE)) {}

	return result;

};

// --------------------------------------------------------------------------

ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr) {

	// The command number is in the third half-byte of the packet.
	ECommandGroupIDType result = ((inBufferPtr[CMDPOS_CMDID]) & CMDMASK_CMDID) >> 4;
	return result;
};

NetworkIDType getNetworkID(BufferCntType inRXBufferNum) {
	return (gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_NETID] & SHIFTBITS_PKT_NETID >> SHIFTBITS_PKT_NETID);
}

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

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr) {
	ENetMgmtSubCmdIDType result = (inBufferPtr[CMDPOS_MGMT_SUBCMD]);
	return result;
};

// --------------------------------------------------------------------------

ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum) {
	ECmdAssocType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD]);
	return result;
};

// --------------------------------------------------------------------------

EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum) {
	EControlSubCmdIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD]);
	return result;
};

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr) {

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

void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	
	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdReqRespREQ;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_VER] = 0x01;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_VER + 1;
};

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandInfo, gMyNetworkID, gMyAddr, inRemoteAddr);
	
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INFO_SUBCMD] = eCmdInfoQuery;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_INFO_SUBCMD + 1;
};

// --------------------------------------------------------------------------

void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, RemoteAddrType inRemoteAddr) {

	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createPacket(inTXBufferNum, eCommandInfo, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INFO_SUBCMD] = eCmdInfoResponse;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_INFO_SUBCMD + inResponseSize + 1;
};

// --------------------------------------------------------------------------

void createOutboundNetsetup() {
	BufferCntType txBufferNum;
	
	vTaskSuspend(gRadioReceiveTask);
	
	while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1);
	}
	
	txBufferNum = gTXCurBufferNum;
	advanceTXBuffer();

	// This command gets setup in the TX buffers, because it only gets sent back to the controller via
	// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
	// directly.
	
	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NETID] |= (BROADCAST_NETID << SHIFTBITS_PKT_NETID);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_ADDR] = (ADDR_CONTROLLER << SHIFTBITS_PKT_SRCADDR) | ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMDID] = (eCommandNetMgmt << SHIFTBITS_CMDID);
	gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;
	
	// Set the sub-command.
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdNetSetup;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_SETUP_CHANNEL] = 0;
		
	gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_SETUP_CHANNEL + 1;

	serialTransmitFrame((byte*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum);
	
	vTaskResume(gRadioReceiveTask);

}

// --------------------------------------------------------------------------

void createControlCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + 1;
};

// --------------------------------------------------------------------------

void processNetSetupCommand(BufferCntType inTXBufferNum) {

	ChannelNumberType			channel;

	// Network Setup ALWAYS comes in via the serial interface to the gateway (dongle)
	// This means we process it FROM the TX buffers and SEND from the TX buffers.
	// These commands NEVER go onto the air.
	
	// Get the requested channel number.
	channel = gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SETUP_CHANNEL];
	
	// Write this value to MV_RAM if it's different than what we already have.
	// NB: Writing to flash causes it to wear out over time.  FSL says 10K write cycles, but there
	// are other issues that may reduce this number.  So only write if we have to.
//	if (NV_RAM_ptr->ChannelSelect != channel) {
//		// (cast away the "const" of the NVRAM channel number.)
//		EnterCritical();
//		Update_NV_RAM(&(NV_RAM_ptr->ChannelSelect), &channel, 1);
//		//WriteFlashByte(channel, &(NV_RAM_ptr->ChannelSelect));
//		ExitCritical();
//	}
	
	MLMESetChannelRequest(channel);
	RELEASE_TX_BUFFER(inTXBufferNum);
	
	gLocalDeviceState = eLocalStateRun;
};

// --------------------------------------------------------------------------

void processNetCheckOutboundCommand(BufferCntType inTXBufferNum) {
	BufferCntType txBufferNum;
	ChannelNumberType channel;

	vTaskSuspend(gRadioReceiveTask);
	
	// Switch to the channel requested in the outbound net-check.
	channel = gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CHECK_CHANNEL];
	MLMESetChannelRequest(channel);

	// We need to put the gateway (dongle) GUID into the outbound packet before it gets transmitted.
	memcpy(&(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CHECK_UID]), GUID, UNIQUE_ID_BYTES);

	/*
	 * At this point we transmit one inbound net-check back over the serial link from the gateway (dongle) itself.
	 * The reason is that we need to guarantee that at least one net-check goes back to the controller
	 * for each channel.  If there are no other controllers operating or listening at least the controller
	 * will have a net-check from the dongle itself.  This is necessary, so that the controller can 
	 * assess the energy detect (ED) for each channel.  Even though there may be no other controllers 
	 * on the channel, but there may be other systems using the channel with different protocols or
	 * modulation schemes.
	 * 
	 * The only rational way to do this is to use a transmit buffer.  The reason is that the radio may
	 * already be waiting to fill an inbound packet that we already sent to the MAC.  There is no
	 * way to let the MAC know that we're about to switch the current RX buffer.  For this reason
	 * the only safe buffer available to us comes from the TX buffer.
	 */

	if (gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CHECK_TYPE] == eCmdReqRespREQ) {
		// Wait until we can get an TX buffer
		while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
			vTaskDelay(1);
		}
		//EnterCritical();
			txBufferNum = gTXCurBufferNum;
			advanceTXBuffer();
		//ExitCritical();
	
		// This command gets setup in the TX buffers, because it only gets sent back to the controller via
		// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
		// directly.
		
		// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
		//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NETID] |= (BROADCAST_NETID << SHIFTBITS_PKT_NETID);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_ADDR] = (ADDR_CONTROLLER << SHIFTBITS_PKT_SRCADDR) | ADDR_CONTROLLER;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMDID] = (eCommandNetMgmt << SHIFTBITS_CMDID);
		gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;
		
		// Set the sub-command.
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdNetCheck;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_TYPE] = eCmdReqRespRESP;
		
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_NETID] = BROADCAST_NETID;
		memcpy((void *) &(gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_UID]), PRIVATE_GUID, UNIQUE_ID_BYTES);
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_CHANNEL] = channel;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_ENERGY] = MLMEEnergyDetect();
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_LINKQ] = 0;
		
		gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_CHECK_LINKQ + 1;
	
		serialTransmitFrame((byte*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
		RELEASE_TX_BUFFER(txBufferNum);
		
		vTaskResume(gRadioReceiveTask);

	}
};
	
// --------------------------------------------------------------------------

void processNetCheckInboundCommand(BufferCntType inRXBufferNum) {
	// The gateway (dongle) needs to add the link quality to the channel energy field.
	// This way the gateway can assess channel energy.
};

// --------------------------------------------------------------------------

void processAssocRespCommand(BufferCntType inRXBufferNum) {

	RemoteAddrType result = INVALID_REMOTE;

	// Let's first make sure that this assign command is for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), UNIQUE_ID_BYTES) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR] & CMDMASK_ASSIGNID) >> SHIFTBITS_CMDID;	
		gMyNetworkID = getNetworkID(inRXBufferNum);
		gLocalDeviceState = eLocalStateAssociated;
	}

	RELEASE_RX_BUFFER(inRXBufferNum);
};

// --------------------------------------------------------------------------

void processQueryCommand(BufferCntType inRXBufferNum, RemoteAddrType inSrcAddr) {

	processQuery(inRXBufferNum, CMDPOS_INFO_QUERY, inSrcAddr);
	RELEASE_RX_BUFFER(inRXBufferNum);
};

// --------------------------------------------------------------------------

void processResponseCommand(BufferCntType inRXBufferNum, RemoteAddrType inRemoteAddr) {

	// Indicate that we received a response for the remote.
	gRemoteStateTable[inRemoteAddr].remoteState = eRemoteStateRespRcvd;

	RELEASE_RX_BUFFER(inRXBufferNum);

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