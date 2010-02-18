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
#include "gwSystemMacros.h"
#include "Leds.h"
#include "spi.h"

#ifdef IS_TOY_NETWORK
#include "deviceQuery.h"
#else
#include "terminalQuery.h"
#endif

RemoteDescStruct gRemoteStateTable[MAX_REMOTES];
NetAddrType gMyAddr = INVALID_REMOTE;
NetworkIDType gMyNetworkID = BROADCAST_NETID;
extern LedFlashRunType gLedFlashSequenceShouldRun;
extern LedFlashSeqCntType gLedFlashSeqCount;
extern LedFlashStruct gLedFlashSeqBuffer[MAX_LED_SEQUENCES];
//extern gwUINT16				gFIFO[8];

// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, NetAddrType inSrcAddr,
        NetAddrType inDestAddr);

// --------------------------------------------------------------------------

gwUINT8 transmitPacket(BufferCntType inTXBufferNum) {
	gwUINT8 result = 0;
	BufferCntType txBufferNum = inTXBufferNum;

	// Transmit the packet.
	result = xQueueGenericSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);

	return result;

}
;

// --------------------------------------------------------------------------

gwUINT8 transmitPacketFromISR(BufferCntType inTXBufferNum) {
	gwUINT8 result = 0;

	// Transmit the packet.
	result = xQueueSendFromISR(gRadioTransmitQueue, &inTXBufferNum, (portTickType) 0);

	return result;

}
;

// --------------------------------------------------------------------------

AckIDType getAckId(BufferCntType inRXBufferNum) {
	// We know we need to ACK if the command ID is not zero.
	return (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_ACKID]);
}
;

// --------------------------------------------------------------------------

ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr) {

	// The command number is in the third half-byte of the packet.
	ECommandGroupIDType result = ((inBufferPtr[CMDPOS_CMDID]) & CMDMASK_CMDID) >> 4;
	return result;
}
;

// --------------------------------------------------------------------------

NetworkIDType getNetworkID(BufferCntType inRXBufferNum) {
	return ((gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_NETID] & PACKETMASK_NETID) >> SHIFTBITS_PKT_NETID);
}

// --------------------------------------------------------------------------

gwBoolean getCommandRequiresACK(BufferCntType inRXBufferNum) {
	return (getAckId != 0);
}

// --------------------------------------------------------------------------

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum) {

	// The command number is in the third half-byte of the packet.
	EndpointNumType result = ((gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ENDPOINT] & CMDMASK_ENDPOINT)
	        >> SHIFTBITS_CMD_ENDPOINT);
	return result;
}
;

// --------------------------------------------------------------------------

NetAddrType getCommandSrcAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	NetAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_ADDR] & CMDMASK_SRC_ADDR) >> 4;
	return result;
}
;

// --------------------------------------------------------------------------

NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	NetAddrType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_ADDR] & CMDMASK_DST_ADDR);
	return result;
}
;

// --------------------------------------------------------------------------

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr) {
	ENetMgmtSubCmdIDType result = (inBufferPtr[CMDPOS_MGMT_SUBCMD]);
	return result;
}
;

// --------------------------------------------------------------------------

ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum) {
	ECmdAssocType result = eCmdAssocInvalid;
	// Make sure the command is actually for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), UNIQUE_ID_BYTES) == 0) {
		result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD]);
	}
	return result;
}
;

// --------------------------------------------------------------------------

EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum) {
	EControlSubCmdIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD]);
	return result;
}
;

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, NetAddrType inSrcAddr,
        NetAddrType inDestAddr) {

	// First clear the packet header.
	memset((void *) gTXRadioBuffer[inTXBufferNum].bufferStorage, 0, CMDPOS_CMDID);

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
	//gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
}
;

// --------------------------------------------------------------------------

void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	int pos;

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocREQ;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_VER] = 0x01;
	// Set the system status register
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT] = GW_GET_SYSTEM_STATUS;

	//	for( pos = 0; pos < 16; pos++ ) {
	//		gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT + 1 + pos] = gFIFO[pos];
	//	}

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_SYSSTAT + pos;
}
;

// --------------------------------------------------------------------------

void createAssocCheckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {
#if !defined(XBEE_PINOUT)
	gwUINT8 saveATD1C;
	gwUINT8 saveATD1SC;
#endif
	gwATD batteryLevel;

	// Create the command for checking if we're associated
	createPacket(inTXBufferNum, eCommandAssoc, gMyNetworkID, gMyAddr, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocCHECK;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_VER] = 0x01;

#if !defined(XBEE_PINOUT)
	// Save the ATD state and prepare to take a battery measurement.
	GW_PREP_ATD(saveATD1C, saveATD1SC);
	GW_MEASURE_BATTERY(batteryLevel);
	GW_RESTORE_ATD(saveATD1C, saveATD1SC);

	// Nominalize to a 0-100 scale.
	if (batteryLevel < 0) {
		batteryLevel = 0;
	} else {
		// Double to get to a 100 scale.
		batteryLevel = batteryLevel << 1;
	}
#endif
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_BATT] = batteryLevel;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCCHK_BATT + 1;
}
;

// --------------------------------------------------------------------------

void createAckCommand(BufferCntType inTXBufferNum, AckIDType inAckId) {

	createPacket(inTXBufferNum, eCommandNetMgmt, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdAck;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ACK_ID] = inAckId;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ACK_ID + 1;
}
;

// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, NetAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandInfo, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INFO_SUBCMD] = eCmdInfoQuery;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_INFO_SUBCMD + 1;
}
;

// --------------------------------------------------------------------------

void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, NetAddrType inRemoteAddr) {

	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createPacket(inTXBufferNum, eCommandInfo, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INFO_SUBCMD] = eCmdInfoResponse;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_INFO_SUBCMD + inResponseSize + 1;
}
;

// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
void createOutboundNetSetup() {
	BufferCntType txBufferNum;
	gwUINT8 ccrHolder;

	txBufferNum = lockTXBuffer();

	vTaskSuspend(gRadioReceiveTask);

	// This command gets setup in the TX buffers, because it only gets sent back to the controller via
	// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
	// directly.

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NETID] |= (BROADCAST_NETID << SHIFTBITS_PKT_NETID);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_ADDR] = (ADDR_CONTROLLER << SHIFTBITS_PKT_SRCADDR) | ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMDID] = (eCommandNetMgmt << SHIFTBITS_CMDID);
	//gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;

	// Set the sub-command.
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdNetSetup;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_SETUP_CHANNEL] = 0;

	gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_SETUP_CHANNEL + 1;

	serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}
#endif

// --------------------------------------------------------------------------

void createButtonControlCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNumber, gwUINT8 inFunctionType) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdButton;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA] = inButtonNumber;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1] = inFunctionType;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_CONTROL_DATA + 2;
}
;

// --------------------------------------------------------------------------

void createAudioCommand(BufferCntType inTXBufferNum) {

	createPacket(inTXBufferNum, eCommandAudio, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + 1;
}
;

// --------------------------------------------------------------------------

void processNetSetupCommand(BufferCntType inTXBufferNum) {

	ChannelNumberType channel;
	gwUINT8 ccrHolder;

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
	RELEASE_TX_BUFFER(inTXBufferNum, ccrHolder);

	gLocalDeviceState = eLocalStateRun;
}
;

// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
void processNetIntfTestCommand(BufferCntType inTXBufferNum) {

	BufferCntType txBufferNum;
	gwUINT8 ccrHolder;

	/*
	 * At this point we transmit one inbound interface test back over the serial link from the gateway (dongle) itself.
	 *
	 * The only rational way to do this is to use a transmit buffer.  The reason is that the radio may
	 * already be waiting to fill an inbound packet that we already sent to the MAC.  There is no
	 * way to let the MAC know that we're about to switch the current RX buffer.  For this reason
	 * the only safe buffer available to us comes from the TX buffer.
	 */

	txBufferNum = lockTXBuffer();

	vTaskSuspend(gRadioReceiveTask);

	// This command gets setup in the TX buffers, because it only gets sent back to the controller via
	// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
	// directly.

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NETID] |= (BROADCAST_NETID << SHIFTBITS_PKT_NETID);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_ADDR] = (ADDR_CONTROLLER << SHIFTBITS_PKT_SRCADDR) | ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMDID] = (eCommandNetMgmt << SHIFTBITS_CMDID);
	//gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;

	// Set the sub-command.
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdNetIntfTest;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_INTF_TEST_NUM]
	        = gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INTF_TEST_NUM];

	gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_INTF_TEST_NUM + 1;

	serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}
;
#endif

// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
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

	if (gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CHECK_TYPE] == eCmdAssocREQ) {
		gwUINT8 ccrHolder;

		txBufferNum = lockTXBuffer();

		// This command gets setup in the TX buffers, because it only gets sent back to the controller via
		// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
		// directly.

		// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
		//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NETID] |= (BROADCAST_NETID << SHIFTBITS_PKT_NETID);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_ADDR] = (ADDR_CONTROLLER << SHIFTBITS_PKT_SRCADDR) | ADDR_CONTROLLER;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMDID] = (eCommandNetMgmt << SHIFTBITS_CMDID);
		//gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;

		// Set the sub-command.
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdNetCheck;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_TYPE] = eCmdAssocRESP;

		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_NETID] = BROADCAST_NETID;
		memcpy((void *) &(gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_UID]), PRIVATE_GUID, UNIQUE_ID_BYTES);
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_CHANNEL] = channel;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_ENERGY] = GW_ENERGY_DETECT(channel);
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_LINKQ] = 0;

		gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_CHECK_LINKQ + 1;

		serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
		RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

		vTaskResume(gRadioReceiveTask);

	}
}
;
#endif

// --------------------------------------------------------------------------

void processNetCheckInboundCommand(BufferCntType inRXBufferNum) {
	// The gateway (dongle) needs to add the link quality to the channel energy field.
	// This way the gateway can assess channel energy.
}
;

// --------------------------------------------------------------------------

void processAssocRespCommand(BufferCntType inRXBufferNum) {

	NetAddrType result = INVALID_REMOTE;
	gwUINT8 ccrHolder;

	// Let's first make sure that this assign command is for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), UNIQUE_ID_BYTES) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR] & CMDMASK_ASSIGN_ADDR)
		        >> SHIFTBITS_CMD_ASGNADDR;
		gMyNetworkID = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_NET] & CMDMASK_ASSIGN_NETID)
		        >> SHIFTBITS_CMD_ASGNNETID;
		gLocalDeviceState = eLocalStateAssociated;
	}

	RELEASE_RX_BUFFER(inRXBufferNum, ccrHolder);
}
;

// --------------------------------------------------------------------------

void processAssocAckCommand(BufferCntType inRXBufferNum) {
	// No can do in FreeRTOS :-(
	//xTaskSetTickCount(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME)]);
}
;

// --------------------------------------------------------------------------

void processQueryCommand(BufferCntType inRXBufferNum, NetAddrType inSrcAddr) {
	gwUINT8 ccrHolder;

	processQuery(inRXBufferNum, CMDPOS_INFO_QUERY, inSrcAddr);
	RELEASE_RX_BUFFER(inRXBufferNum, ccrHolder);
}
;

// --------------------------------------------------------------------------

void processResponseCommand(BufferCntType inRXBufferNum, NetAddrType inRemoteAddr) {
	gwUINT8 ccrHolder;

	// Indicate that we received a response for the remote.
	gRemoteStateTable[inRemoteAddr].remoteState = eRemoteStateRespRcvd;

	RELEASE_RX_BUFFER(inRXBufferNum, ccrHolder);

}

// --------------------------------------------------------------------------

EMotorCommandType getMotorCommand(BufferCntType inRXBufferNum) {

	EMotorCommandType result = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1];

	return result;
}

// --------------------------------------------------------------------------

#if 0
void processMotorControlSubCommand(BufferCntType inRXBufferNum) {

#define MOTOR1_FREE		0xfc    /* 0b11111100 */
#define MOTOR1_FWD		0x02    /* 0b00000010 */
#define MOTOR1_BWD		0x01    /* 0b00000001 */
#define MOTOR1_BRAKE	0x03    /* 0b00000011 */

#define MOTOR2_FREE		0xf3    /* 0b11110011 */
#define MOTOR2_FWD		0x08    /* 0b00001000 */
#define MOTOR2_BWD		0x04    /* 0b00000100 */
#define MOTOR2_BRAKE	0x0c    /* 0b00001100 */

	// Map the endpoint to the motor.
	EndpointNumType endpoint = getEndpointNumber(inRXBufferNum);
	EMotorCommandType motorCommand = getMotorCommand(inRXBufferNum);

	PTBDD = 0xff; //0b11111111;
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
#endif

// --------------------------------------------------------------------------

void processHooBeeSubCommand(BufferCntType inRXBufferNum) {
	gwUINT8 ccrHolder;
	gwUINT8 pos = CMDPOS_BEHAVIOR_CNT;
	gwUINT8 behaviorCnt;
	gwUINT8 behaviorNum;
	gwUINT8 behaviorType;
	gwUINT8 dataByteCnt;

	// Map the endpoint to the LED number.
	EndpointNumType endpoint = getEndpointNumber(inRXBufferNum);

	gLedFlashSeqCount = 0;
	gLedFlashSequenceShouldRun = TRUE;

	behaviorCnt = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];

	for (behaviorNum = 1; behaviorNum <= behaviorCnt; behaviorNum++) {

		// Read the behavior type.
		behaviorType = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];
		dataByteCnt = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];

		switch (behaviorType) {

			case eHooBeeBehaviorLedFlash:
				if (gLedFlashSeqCount < MAX_LED_SEQUENCES) {
					gLedFlashSeqBuffer[gLedFlashSeqCount].redValue = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];
					gLedFlashSeqBuffer[gLedFlashSeqCount].greenValue = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];
					gLedFlashSeqBuffer[gLedFlashSeqCount].blueValue = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];
					memcpy(&gLedFlashSeqBuffer[gLedFlashSeqCount].timeOnMillis,
					        (void *) &(gRXRadioBuffer[inRXBufferNum].bufferStorage[pos]), sizeof(LedFlashTimeType));
					pos += sizeof(LedFlashTimeType);
					memcpy(&gLedFlashSeqBuffer[gLedFlashSeqCount].timeOffMillis,
					        (void *) &(gRXRadioBuffer[inRXBufferNum].bufferStorage[pos]), sizeof(LedFlashTimeType));
					pos += sizeof(LedFlashTimeType);
					gLedFlashSeqBuffer[gLedFlashSeqCount].repeat = gRXRadioBuffer[inRXBufferNum].bufferStorage[pos++];
					gLedFlashSeqCount++;
				}
				break;

			default:
				break;

		}

	}

	RELEASE_RX_BUFFER(inRXBufferNum, ccrHolder);
}

// --------------------------------------------------------------------------

gwBoolean gSDCardBusConnected = FALSE;
gwBoolean gSDCardVccConnected = FALSE;

void processSDCardActionSubCommand(BufferCntType inRXBufferNum) {

	GpioErr_t error;
	ESDCardControlActionType action;

	action = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_ACTION];

	switch (action) {
		case eSDCardActionSdProtocol:
			disableSPI();
			BUS_SW_ON;

			// Power cycle the card so that it can reenter the SDCard mode.
			// (It will be in the SPI mode, and can only recover via power cycle.)
			VCC_SW_OFF;
			vTaskDelay(100);
			VCC_SW_ON;
			break;

		case eSDCardActionSpiProtocol:
			// Disconnect the SDCard from the SDCard bus, set the DAT0 pull-up, and call the SPI init routine.
			BUS_SW_OFF;
			error = Gpio_EnPinPullup(SD_DAT0_PULLUP, TRUE);
			error = Gpio_SelectPinPullup(SD_DAT0_PULLUP, gGpioPinPullup_c);
			enableSPI();
			break;
	}

}

// --------------------------------------------------------------------------

gwUINT32 gCurSDCardAddress = 0;
gwUINT8 gCurSDCardPartNumber = 0;
gwBoolean gIsSDCardUpdating = FALSE;

void processSDCardUpdateSubCommand(BufferCntType inRXBufferNum) {
	gwUINT8 ccrHolder;
	AddressType address;
	gwUINT8 partNumber;
	gwUINT8 totalParts;
	gwUINT8 bytes;
	ESDCardResponse result;

	address.bytes.byte0 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_ADDR];
	address.bytes.byte1 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_ADDR + 1];
	address.bytes.byte2 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_ADDR + 2];
	address.bytes.byte3 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_ADDR + 3];
	partNumber = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_PART];
	totalParts = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_PARTS];
	bytes = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_LEN];

	Led2Off();
	if ((gSDCardBusConnected) || (!gSDCardVccConnected)) {
		// The SDCard is still connected to the SDCard bus or it has no power, so we can't send any SPI commands to it.
	} else if ((address.word == gCurSDCardAddress) && (partNumber == gCurSDCardPartNumber)) {
		// A resend (because we got the packet, but the gateway didn't get the ACK).
	} else {
		if (partNumber == 1) {
			if (gIsSDCardUpdating) {
				// Error - we received a 1st part, but were already updating.
				GW_RESET_MCU;
			} else {
				// Start updating the block.
				gIsSDCardUpdating = TRUE;
				gCurSDCardPartNumber = partNumber;
				gCurSDCardAddress = address.word;
				result = writePartialBlockBegin(address.word);
				result = writePartialBlock(&(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_DATA]), bytes);
				if (result != eResponseOK) {
					GW_RESET_MCU;
				}
				Led2On();
			}
		} else {
			if (!gIsSDCardUpdating) {
				// Error - we received a part greater than 1, but we were not updating.
				GW_RESET_MCU;
			} else if ((gCurSDCardPartNumber + 1) != partNumber) {
				// Error - we received a part that is not +1 greater than the last.
				GW_RESET_MCU;
			} else {
				gCurSDCardPartNumber = partNumber;
				if (partNumber == totalParts) {
					// Last part - stop the updating.
					gIsSDCardUpdating = FALSE;
					result = writePartialBlock(&(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_DATA]), bytes);
					if (result != eResponseOK) {
						GW_RESET_MCU;
					}
					result = writePartialBlockEnd();
					if (result != eResponseOK) {
						GW_RESET_MCU;
					}
					Led2On();
				} else {
					// Continuation of the updating.
					result = writePartialBlock(&(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_DATA]), bytes);
					if (result != eResponseOK) {
						GW_RESET_MCU;
					}
					Led2On();
				}
			}
		}

	}

	RELEASE_RX_BUFFER(inRXBufferNum, ccrHolder);
}

// --------------------------------------------------------------------------

void createDataSampleCommand(BufferCntType inTXBufferNum, EndpointNumType inEndpoint) {

	createPacket(inTXBufferNum, eCommandDataSample, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ENDPOINT] |= (CMDMASK_ENDPOINT & inEndpoint);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_DATA_SUBCMD] = eRemoteDataSubCmdSample;

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SAMPLE_CNT] = 0;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_SAMPLE_CNT + 1;
}
;

// --------------------------------------------------------------------------

void addDataSampleToCommand(BufferCntType inTXBufferNum, TimestampType inTimestamp, DataSampleType inDataSample, char inUnitsByte) {
	gwUINT8 pos = gTXRadioBuffer[inTXBufferNum].bufferSize;

	// Increase the sample count and adjust the packet length.
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SAMPLE_CNT]++;

	// Add the time stamp.
	memcpy((void *) &gTXRadioBuffer[inTXBufferNum].bufferStorage[pos], &inTimestamp, sizeof(inTimestamp));
	pos += sizeof(inTimestamp);

	// Add the data sample.
	memcpy((void *) &gTXRadioBuffer[inTXBufferNum].bufferStorage[pos], &inDataSample, sizeof(inDataSample));
	pos += sizeof(inDataSample);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[pos] = inUnitsByte;
	pos += sizeof(inUnitsByte);

	gTXRadioBuffer[inTXBufferNum].bufferSize = pos;

}
;
