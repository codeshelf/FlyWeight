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
#include "gwSystemMacros.h"

#ifdef IS_PFC
#include "Leds.h"
#include "spi.h"
#endif

#include "deviceQuery.h"
#include "UartLowLevel.h"

RemoteDescStruct gRemoteStateTable[MAX_REMOTES];
NetAddrType gMyAddr = INVALID_REMOTE;
NetworkIDType gMyNetworkID = BROADCAST_NET_NUM;

//extern LedFlashRunType gLedFlashSequenceShouldRun;
//extern LedFlashSeqCntType gLedFlashSeqCount;
//extern LedFlashStruct gLedFlashSeqBuffer[MAX_LED_SEQUENCES];
//gwBoolean gSDCardBusConnected = FALSE;
//gwBoolean gSDCardVccConnected = FALSE;
//gwBoolean gSDCardInSPIMode = FALSE;
//gwUINT32 gCurSDCardAddr = 0;
//gwBoolean gCurSDCardAddrComitted = FALSE;
//gwUINT8 gCurSDCardUpdateResultBitField = 0;
//gwUINT8 gCurSDCardBlock[512];

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
// --------------------------------------------------------------------------

gwUINT8 transmitPacketFromISR(BufferCntType inTXBufferNum) {
	gwUINT8 result = 0;

	// Transmit the packet.
	result = xQueueSendFromISR(gRadioTransmitQueue, &inTXBufferNum, (portTickType) 0);

	return result;

}
// --------------------------------------------------------------------------

AckIDType getAckId(BufferCntType inRXBufferNum) {
	// We know we need to ACK if the command ID is not zero.
	return (gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_ACK_ID]);
}
// --------------------------------------------------------------------------

ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr) {

	// The command number is in the third half-byte of the packet.
	ECommandGroupIDType result = ((inBufferPtr[CMDPOS_CMD_ID]) & CMDMASK_CMDID) >> 4;
	return result;
}
// --------------------------------------------------------------------------

NetworkIDType getNetworkID(BufferCntType inRXBufferNum) {
	return ((gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_NET_NUM] & PACKETMASK_NET_NUM) >> SHIFTBITS_PKT_NET_NUM);
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

// --------------------------------------------------------------------------

NetAddrType getCommandSrcAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	NetAddrType result = gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_SRC_ADDR];
	return result;
}

// --------------------------------------------------------------------------

NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	NetAddrType result = gRXRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_DST_ADDR];
	return result;
}

// --------------------------------------------------------------------------

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr) {
	ENetMgmtSubCmdIDType result = (inBufferPtr[CMDPOS_NETM_SUBCMD]);
	return result;
}

// --------------------------------------------------------------------------

ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum) {
	ECmdAssocType result = eCmdAssocInvalid;
	// Make sure the command is actually for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), UNIQUE_ID_BYTES) == 0) {
		result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD]);
	}
	return result;
}

// --------------------------------------------------------------------------

EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum) {
	EControlSubCmdIDType result = (gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD]);
	return result;
}

// --------------------------------------------------------------------------

void writeAsPString(BufferStoragePtrType inDestPtr, const BufferStoragePtrType inStringPtr, size_t inStringLen) {
	inDestPtr[0] = (gwUINT8) inStringLen;
	memcpy(inDestPtr + 1, inStringPtr, (gwUINT8) inStringLen);
}

// --------------------------------------------------------------------------

gwUINT8 readAsPString(BufferStoragePtrType inDestStringPtr, const BufferStoragePtrType inSrcPtr) {
	gwUINT8 stringLen = (gwUINT8) inSrcPtr[0];
	memcpy(inDestStringPtr, inSrcPtr + 1, (gwUINT8) stringLen);
	inDestStringPtr[stringLen] = (gwUINT8) NULL;
	return stringLen;
}

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, NetAddrType inSrcAddr,
		NetAddrType inDestAddr) {

	// First clear the packet header.
	memset((void * ) gTXRadioBuffer[inTXBufferNum].bufferStorage, 0, CMDPOS_CMD_ID);

	// Write the packet header.
	// (See the comments at the top of commandTypes.h.)
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (inNetworkID << SHIFTBITS_PKT_NET_NUM);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = inSrcAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_DST_ADDR] = inDestAddr;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMD_ID] = (inCmdID << SHIFTBITS_CMD_ID);

	gTXRadioBuffer[inTXBufferNum].bufferSize += 4;
}

// --------------------------------------------------------------------------

void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	int pos;

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NET_NUM, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocREQ;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_DEV_VER] = 0x01;
	// Set the system status register
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT] = GW_GET_SYSTEM_STATUS;

	//	for( pos = 0; pos < 16; pos++ ) {
	//		gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT + 1 + pos] = gFIFO[pos];
	//	}

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_SYSSTAT + pos;
}

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
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_DEV_VER] = 0x01;

#if !defined(XBEE_PINOUT)
	// Save the ATD state and prepare to take a battery measurement.
	GW_PREP_ATD(saveATD1C, saveATD1SC);
	GW_MEASURE_BATTERY(batteryLevel);GW_RESTORE_ATD(saveATD1C, saveATD1SC);

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
// --------------------------------------------------------------------------

void createAckPacket(BufferCntType inTXBufferNum, AckIDType inAckId, AckDataType inAckData) {

	createPacket(inTXBufferNum, eCommandNetMgmt, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_PCK_TYPE_BIT] |= 1 << SHIFTBITS_PCK_TYPE;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ACK_ID] = inAckId;
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ACK_DATA]), inAckData, ACK_DATA_BYTES);
	gTXRadioBuffer[inTXBufferNum].bufferSize = PCKPOS_ACK_DATA + ACK_DATA_BYTES;
}
// --------------------------------------------------------------------------

void createQueryCommand(BufferCntType inTXBufferNum, NetAddrType inRemoteAddr) {

	createPacket(inTXBufferNum, eCommandInfo, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INFO_SUBCMD] = eCmdInfoQuery;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_INFO_SUBCMD + 1;
}
// --------------------------------------------------------------------------

void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, NetAddrType inRemoteAddr) {

	// Describe the capabilities and channels of the device.
	// This is free-format command that uses XML for content.
	// Keep in mind that you can't send more than 125 bytes!

	createPacket(inTXBufferNum, eCommandInfo, gMyNetworkID, gMyAddr, inRemoteAddr);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INFO_SUBCMD] = eCmdInfoResponse;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_INFO_SUBCMD + inResponseSize + 1;
}
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
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (BROADCAST_NET_NUM << SHIFTBITS_PKT_NET_NUM);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_DST_ADDR] = ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMD_ID] = (eCommandNetMgmt << SHIFTBITS_CMD_ID);

	// Set the sub-command.
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SUBCMD] = eNetMgmtSubCmdNetSetup;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SETCMD_CHANNEL] = 0;

	gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_NETM_SETCMD_CHANNEL + 1;

	serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}
#endif

// --------------------------------------------------------------------------

#ifdef IS_HOOBEE
void createButtonControlCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNumber, gwUINT8 inFunctionType) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdButton;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA] = inButtonNumber;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1] = inFunctionType;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_CONTROL_DATA + 2;
}

// --------------------------------------------------------------------------

void createAudioCommand(BufferCntType inTXBufferNum) {

	createPacket(inTXBufferNum, eCommandAudio, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + 1;
}
#endif

// --------------------------------------------------------------------------

void processNetSetupCommand(BufferCntType inTXBufferNum) {

	ChannelNumberType channel;
	gwUINT8 ccrHolder;

	// Network Setup ALWAYS comes in via the serial interface to the gateway (dongle)
	// This means we process it FROM the TX buffers and SEND from the TX buffers.
	// These commands NEVER go onto the air.

	// Get the requested channel number.
	channel = gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_SETCMD_CHANNEL];

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

	gLocalDeviceState = eLocalStateRun;
}
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
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (BROADCAST_NET_NUM << SHIFTBITS_PKT_NET_NUM);
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_DST_ADDR] = ADDR_CONTROLLER;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMD_ID] = (eCommandNetMgmt << SHIFTBITS_CMD_ID);

	// Set the sub-command.
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SUBCMD] = eNetMgmtSubCmdNetIntfTest;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_TSTCMD_NUM] =
			gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_TSTCMD_NUM];

	gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_NETM_TSTCMD_NUM + 1;

	serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}
#endif

// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
void processNetCheckOutboundCommand(BufferCntType inTXBufferNum) {
	BufferCntType txBufferNum;
	ChannelNumberType channel;

	vTaskSuspend(gRadioReceiveTask);

	// Switch to the channel requested in the outbound net-check.
	channel = gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_CHANNEL];
	//	MLMESetChannelRequest(channel);

	// We need to put the gateway (dongle) GUID into the outbound packet before it gets transmitted.
	memcpy(&(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_GUID]), GUID, UNIQUE_ID_BYTES);

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

	if (gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_TYPE] == eCmdAssocREQ) {
		gwUINT8 ccrHolder;

		txBufferNum = lockTXBuffer();

		// This command gets setup in the TX buffers, because it only gets sent back to the controller via
		// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
		// directly.

		// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
		//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (BROADCAST_NET_NUM << SHIFTBITS_PKT_NET_NUM);
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = ADDR_CONTROLLER;
		gTXRadioBuffer[txBufferNum].bufferStorage[PCKPOS_DST_ADDR] = ADDR_CONTROLLER;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMD_ID] = (eCommandNetMgmt << SHIFTBITS_CMD_ID);

		// Set the sub-command.
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SUBCMD] = eNetMgmtSubCmdNetCheck;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_TYPE] = eCmdAssocRESP;

		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_NET_NUM] = BROADCAST_NET_NUM;
		memcpy((void *) &(gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_GUID]), PRIVATE_GUID, UNIQUE_ID_BYTES);
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_CHANNEL] = channel;
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_ENERGY] = GW_ENERGY_DETECT(channel);
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_LINKQ] = 0;

		gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_NETM_CHKCMD_LINKQ + 1;

		serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
		RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

		vTaskResume(gRadioReceiveTask);

	}
}
#endif

// --------------------------------------------------------------------------

void processNetCheckInboundCommand(BufferCntType inRXBufferNum) {
	// The gateway (dongle) needs to add the link quality to the channel energy field.
	// This way the gateway can assess channel energy.
}
// --------------------------------------------------------------------------

void processAssocRespCommand(BufferCntType inRXBufferNum) {

	NetAddrType result = INVALID_REMOTE;
	gwUINT8 ccrHolder;

	// Let's first make sure that this assign command is for us.
	if (memcmp(GUID, &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), UNIQUE_ID_BYTES) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR];
		gMyNetworkID = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_NET];
		gLocalDeviceState = eLocalStateAssociated;
	}
}
// --------------------------------------------------------------------------

void processAssocAckCommand(BufferCntType inRXBufferNum) {
	// No can do in FreeRTOS :-(
	//xTaskSetTickCount(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME)]);
}
// --------------------------------------------------------------------------

void processQueryCommand(BufferCntType inRXBufferNum, NetAddrType inSrcAddr) {
	gwUINT8 ccrHolder;

	processQuery(inRXBufferNum, CMDPOS_INFO_QUERY, inSrcAddr);
}
// --------------------------------------------------------------------------

void processResponseCommand(BufferCntType inRXBufferNum, NetAddrType inRemoteAddr) {
	gwUINT8 ccrHolder;

	// Indicate that we received a response for the remote.
	gRemoteStateTable[inRemoteAddr].remoteState = eRemoteStateRespRcvd;
}

// --------------------------------------------------------------------------

EMotorCommandType getMotorCommand(BufferCntType inRXBufferNum) {

	EMotorCommandType result = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1];

	return result;
}

// --------------------------------------------------------------------------

#ifdef IS_MOTORCONTROLLER
EControlCmdAckStateType processMotorControlSubCommand(BufferCntType inRXBufferNum) {

#define MOTOR1_FREE		0xfc    /* 0b11111100 */
#define MOTOR1_FWD		0x02    /* 0b00000010 */
#define MOTOR1_BWD		0x01    /* 0b00000001 */
#define MOTOR1_BRAKE	0x03    /* 0b00000011 */

#define MOTOR2_FREE		0xf3    /* 0b11110011 */
#define MOTOR2_FWD		0x08    /* 0b00001000 */
#define MOTOR2_BWD		0x04    /* 0b00000100 */
#define MOTOR2_BRAKE	0x0c    /* 0b00001100 */

	// Map the endpoint to the motor.
	gwUINT8 ccrHolder;
	EndpointNumType endpoint = getEndpointNumber(inRXBufferNum);
	EMotorCommandType motorCommand = getMotorCommand(inRXBufferNum);

	PTBDD = 0xff;//0b11111111;
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
}
#endif

// --------------------------------------------------------------------------

#ifdef IS_HOOBEE
EControlCmdAckStateType processHooBeeSubCommand(BufferCntType inRXBufferNum) {
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
}
#endif

// --------------------------------------------------------------------------

#ifdef IS_PFC
extern xQueueHandle gPFCQueue;

EControlCmdAckStateType processSDCardModeSubCommand(BufferCntType inRXBufferNum, AckIDType inAckId, AckDataType inOutAckData) {

	EControlCmdAckStateType result = eAckStateOk;
	inOutAckData[0] = SD_MODE_OK;

	SDControlCommandStruct control;
	ESDCardControlActionType action;

	action = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_MODE_ACTION];
	control.deviceType = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_MODE_DEVTYPE];
	control.delay.bytes.byte0 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_MODE_DELAY];
	control.delay.bytes.byte1 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_MODE_DELAY + 1];

	switch (action) {
		case eSDCardActionSdProtocol:
		if (gSDCardInSPIMode) {
			gSDCardInSPIMode = FALSE;
			// Switching back to SD card protocol mode is done in a task since it might take a long time.
			xQueueGenericSend(gPFCQueue, &control, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);
		}
		break;

		case eSDCardActionSpiProtocol:
		if (!gSDCardInSPIMode) {
			if (enableSPI()) {
				gSDCardInSPIMode = TRUE;
			} else {
				inOutAckData[0] = SD_MODE_FAILED;
			}
		}
		break;
	}

	return result;
}

// --------------------------------------------------------------------------

EControlCmdAckStateType processSDCardBlockCheckSubCommand(BufferCntType inRXBufferNum) {

	EControlCmdAckStateType result = eAckStateOk;

	GpioErr_t error;
	gwUINT8 totalBlocks;
	gwUINT8 blockNum;
	gwUINT8 offset;
	gwUINT16 blockAddr;
	crc16Type crc;

	totalBlocks = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_BLKCHKCNT];

	for (blockNum = 0; blockNum < totalBlocks; ++blockNum) {
		offset = blockNum * (sizeof(blockAddr) + sizeof(crc));

		// Read the next block number.
		blockAddr = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_BLKCHKDAT + offset];

		// Read the crc for the block.
		crc.value = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_BLKCHKDAT + offset + sizeof(blockAddr)];

		// If the block's CRC doesn't match the expected value then send a failure response.
		//		if (crc.value != crcBlock(blockAddr).value) {
		//			result = eAckStateFailed;
		//		}
	}

	return result;
}

// --------------------------------------------------------------------------

EControlCmdAckStateType processSDCardUpdateSubCommand(BufferCntType inRXBufferNum) {

	EControlCmdAckStateType result = eAckStateNotNeeded;
	gwUINT8 ccrHolder;
	AddressType address;
	gwUINT8 partBitFieldBit;
	gwUINT16 offset;
	gwUINT8 bytes;
	GpioErr_t error;
	ESDCardResponse cardResponse;

	PACKET_LED_ON;

	address.bytes.byte0 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_ADDR];
	address.bytes.byte1 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_ADDR + 1];
	address.bytes.byte2 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_ADDR + 2];
	address.bytes.byte3 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_ADDR + 3];
	partBitFieldBit = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_PARTBIT];
	// Offset is a two-byte value that arrives MSB-first.
	offset = ((gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_OFFSET]) << 8)
	+ gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_OFFSET + 1];
	bytes = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_LEN];

	// If the latest update is for a different address block then reset for processing in the new block.
	if (address.word != gCurSDCardAddr) {
		gCurSDCardAddr = address.word;
		gCurSDCardAddrComitted = FALSE;
		gCurSDCardUpdateResultBitField = 0;
	}

	// Copy the contents of the sub-block into the offset position in the buffer block.
	memcpy(&(gCurSDCardBlock[offset]), &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_UPDATE_DATA]), bytes);
	// Set the bitfield result.
	gCurSDCardUpdateResultBitField |= partBitFieldBit;

	PACKET_LED_OFF;

	return result;
}

// --------------------------------------------------------------------------

EControlCmdAckStateType processSDCardUpdateCommitSubCommand(BufferCntType inRXBufferNum, AckDataType inOutAckData) {
	EControlCmdAckStateType result = eAckStateOk;

	AddressType address;
	gwUINT8 updateBitField;
	BufferCntType txBufferNum;

	address.bytes.byte0 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_COMMIT_ADDR];
	address.bytes.byte1 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_COMMIT_ADDR + 1];
	address.bytes.byte2 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_COMMIT_ADDR + 2];
	address.bytes.byte3 = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_COMMIT_ADDR + 3];
	updateBitField = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_SDCARD_COMMIT_BIFIELD];

	inOutAckData[0] = SD_UPDATE_OK;
	inOutAckData[1] = gCurSDCardUpdateResultBitField;
	if (gCurSDCardAddr != address.word) {
		inOutAckData[0] = SD_UPDATE_BAD_ADDR;
	} else if (gCurSDCardUpdateResultBitField != updateBitField) {
		// Now check to see if we received all of these parts.
		// If not, do nothing.
	} else if (!gCurSDCardAddrComitted) {
		// If we received all of the parts then try commit/write them to the SD card.

		// If we're not in SPI mode then attemp to go into SPI mode.
		if (!gSDCardInSPIMode) {
			if (enableSPI()) {
				gSDCardInSPIMode = TRUE;
			} else {
				inOutAckData[0] = SD_UPDATE_BAD_SPIMODE;
				// Reset the card.
				VCC_SW_OFF;
				vTaskDelay(20);
				VCC_SW_ON;
			}
		}

		// If we're in SPI mode then attempt to update the SD card.
		if (gSDCardInSPIMode) {
			CARD_LED_ON;
			ESDCardResponse writeResult = writeBlock(address.word, (gwUINT8*) &gCurSDCardBlock);
			CARD_LED_OFF;
			if (writeResult == eResponseOK) {
				gCurSDCardAddrComitted = TRUE;
			} else {
				inOutAckData[0] = SD_UPDATE_BAD_WRITE;
			}
		}
	} else {
		// Do nothing, we've already written this addr block to the card.
		// Probably our ACK packet didn't make it back, so we'll send another.
	}

	return result;
}
#endif

#ifdef IS_DATASAMPLER
// --------------------------------------------------------------------------

void createDataSampleCommand(BufferCntType inTXBufferNum, EndpointNumType inEndpoint) {

	createPacket(inTXBufferNum, eCommandDataSample, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ENDPOINT] |= (CMDMASK_ENDPOINT & inEndpoint);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_DATA_SUBCMD] = eRemoteDataSubCmdSample;

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SAMPLE_CNT] = 0;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_SAMPLE_CNT + 1;
}

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
#endif

// --------------------------------------------------------------------------

#ifdef IS_CODESHELF

EControlCmdAckStateType processMessageSubCommand(BufferCntType inRXBufferNum) {
	EControlCmdAckStateType result = eAckStateOk;
	gwUINT8 stringLen;
	BufferStorageType message1Str[MAX_MESSAGE_STRING_BYTES];
	BufferStorageType message2Str[MAX_MESSAGE_STRING_BYTES];

	BufferStoragePtrType bufferPtr = gRXRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE;
	stringLen = readAsPString(message1Str, bufferPtr);

	bufferPtr = gRXRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE + stringLen + 1;
	stringLen = readAsPString(message2Str, bufferPtr);

	UartWriteData(UART_2, LINE1_POS1, strlen(LINE1_POS1));
	UartWriteData(UART_2, message1Str, strlen(message1Str));

	UartWriteData(UART_2, LINE2_POS1, strlen(LINE2_POS1));
	UartWriteData(UART_2, message2Str, strlen(message2Str));

	return result;
}

// --------------------------------------------------------------------------

extern LedPositionType gTotalLedPositions;

extern LedDataStruct gLedFlashData[];
extern LedPositionType gCurLedFlashDataElement;
extern LedPositionType gTotalLedFlashDataElements;
extern LedPositionType gNextFlashLedPosition;

extern LedDataStruct gLedSolidData[];
extern LedPositionType gCurLedSolidDataElement;
extern LedPositionType gTotalLedSolidDataElements;
extern LedPositionType gNextSolidLedPosition;

EControlCmdAckStateType processLedSubCommand(BufferCntType inRXBufferNum) {

	EControlCmdAckStateType result = eAckStateOk;
	gwUINT8 ccrHolder;
	gwUINT8 effect;
	gwUINT8 sampleCount;
	gwUINT8 sampleNum;
	LedDataStruct ledData;

	effect = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_EFFECT];
	sampleCount = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLE_COUNT];

	for (sampleNum = 0; sampleNum < sampleCount; ++sampleNum) {

		ledData.channel = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_CHANNEL];
		//memcpy(&ledData.position, (void *) &(gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 0)]), sizeof(ledData.position));
		ledData.position = (gwUINT16) gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 0)] << 8;
		ledData.position += (gwUINT16) gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 1)];
		ledData.red = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 2)];
		ledData.green = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 3)];
		ledData.blue = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 4)];

		switch (effect) {
			case eLedEffectSolid:
				if (ledData.position == ((gwUINT16) -1)) {
					gTotalLedSolidDataElements = 0;
					gCurLedSolidDataElement = 0;
					gNextSolidLedPosition = 0;
				} else {
					gLedSolidData[gTotalLedSolidDataElements] = ledData;
					gTotalLedSolidDataElements += 1;
				}
				break;

			case eLedEffectFlash:
				if (ledData.position == ((gwUINT16) -1)) {
					gTotalLedFlashDataElements = 0;
					gCurLedFlashDataElement = 0;
					gNextFlashLedPosition = 0;
				} else {
					gLedFlashData[gTotalLedFlashDataElements] = ledData;
					gTotalLedFlashDataElements += 1;
				}
				break;

			case eLedEffectError:
				break;

			case eLedEffectMotel:
				break;

			default:
				break;
		}
	}
	return result;
}

// --------------------------------------------------------------------------

#endif
