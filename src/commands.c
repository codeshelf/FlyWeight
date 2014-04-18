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
#include "GPIO_Interface.h"

#include "deviceQuery.h"
#include "UartLowLevel.h"

RemoteDescStruct gRemoteStateTable[MAX_REMOTES];
NetAddrType gMyAddr = INVALID_REMOTE;
NetworkIDType gMyNetworkID = BROADCAST_NET_NUM;

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

AckIDType getAckId(BufferStoragePtrType inBufferPtr) {
	// We know we need to ACK if the command ID is not zero.
	return (inBufferPtr[PCKPOS_ACK_ID]);
}
// --------------------------------------------------------------------------

void setAckId(BufferStoragePtrType inBufferPtr) {
	portTickType ticks = xTaskGetTickCount();
	inBufferPtr[PCKPOS_ACK_ID] = (&ticks)[0];
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
	memset((void *) gTXRadioBuffer[inTXBufferNum].bufferStorage, 0, CMDPOS_CMD_ID);

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

	//vTaskSuspend(gRadioReceiveTask);

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

	serialTransmitFrame(UART_1, (gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}
#endif

// --------------------------------------------------------------------------

void createScanCommand(BufferCntType inTXBufferNum, ScanStringPtrType inScanStringPtr, ScanStringLenType inScanStringLen) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	setAckId(gTXRadioBuffer[inTXBufferNum].bufferStorage);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdScan;

	writeAsPString(gTXRadioBuffer[inTXBufferNum].bufferStorage + CMDPOS_CONTROL_DATA, (BufferStoragePtrType) inScanStringPtr,
			inScanStringLen);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + inScanStringLen + 2;
}

// --------------------------------------------------------------------------

void createButtonCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNum, gwUINT8 inValue) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdButton;

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_BUTTON_NUM] = inButtonNum;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_BUTTON_VAL] = inValue;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_BUTTON_VAL + 1;
}

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

	//vTaskSuspend(gRadioReceiveTask);

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

	serialTransmitFrame(UART_1, (gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}
#endif

// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
void processNetCheckOutboundCommand(BufferCntType inTXBufferNum) {
	BufferCntType txBufferNum;
	ChannelNumberType channel;

	//vTaskSuspend(gRadioReceiveTask);

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

		serialTransmitFrame(UART_1, (gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
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

		BufferCntType txBufferNum = lockTXBuffer();
		createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
		if (transmitPacket(txBufferNum)) {
		}

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

#ifdef IS_CODESHELF

DisplayStringType gDisplayDataLine[2];
DisplayStringLenType gDisplayDataLineLen[2];
DisplayStringLenType gDisplayDataLinePos[2];

EControlCmdAckStateType processMessageSubCommand(BufferCntType inRXBufferNum) {
	EControlCmdAckStateType result = eAckStateOk;

	BufferStoragePtrType bufferPtr = gRXRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE;
	gDisplayDataLineLen[0] = readAsPString(gDisplayDataLine[0], bufferPtr);

	bufferPtr = gRXRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE + gDisplayDataLineLen[0] + 1;
	gDisplayDataLineLen[1] = readAsPString(gDisplayDataLine[1], bufferPtr);

	sendDisplayMessage(LINE1_POS1, strlen(LINE1_POS1));
	sendDisplayMessage(gDisplayDataLine[0], getMin(DISPLAY_WIDTH, strlen(gDisplayDataLine[0])));

	sendDisplayMessage(LINE2_POS1, strlen(LINE2_POS1));
	sendDisplayMessage(gDisplayDataLine[1], getMin(DISPLAY_WIDTH, strlen(gDisplayDataLine[1])));

	if ((gDisplayDataLineLen[1] <= DISPLAY_WIDTH) && (gDisplayDataLineLen[1] <= DISPLAY_WIDTH)) {
		stopScrolling();
	} else {
		gDisplayDataLinePos[0] = 0;
		gDisplayDataLinePos[1] = 0;

		startScrolling();
	}
	return result;
}

// --------------------------------------------------------------------------

void startScrolling() {
	TmrSetMode(SCROLL_TIMER, gTmrCntRiseEdgPriSrc_c);
}

// --------------------------------------------------------------------------

void stopScrolling() {
	TmrSetMode(SCROLL_TIMER, gTmrNoOperation_c);
}

// --------------------------------------------------------------------------

gwUINT8 sendDisplayMessage(char* isDisplayMsgPtr, gwUINT8 inMsgLen) {

	gwUINT16 charsSent;

	for (charsSent = 0; charsSent < inMsgLen; charsSent++) {
		while (UART1_REGS_P ->Utxcon < 1) {
			// Temporarily (while we're calling this from the KBI ISR) we can't use vTaskDelay since it will reset the ISR.
			DelayMs(1);
		}
		UART1_REGS_P ->Udata = *isDisplayMsgPtr;
		isDisplayMsgPtr++;
	}

	return gUartErrNoError_c;
}

// --------------------------------------------------------------------------

gwUINT8 sendRs485Message(char* isMsgPtr, gwUINT8 inMsgLen) {

	gwUINT16 charsSent;

	for (charsSent = 0; charsSent < inMsgLen; charsSent++) {
		while (UART2_REGS_P ->Utxcon < 1) {
			// Temporarily (while we're calling this from the KBI ISR) we can't use vTaskDelay since it will reset the ISR.
			DelayMs(1);
		}
		UART2_REGS_P ->Udata = *isMsgPtr;
		isMsgPtr++;
	}

	return gUartErrNoError_c;
}

// --------------------------------------------------------------------------

LedPositionType gTotalLedPositions;

LedDataStruct gLedFlashData[MAX_LED_FLASH_POSITIONS];
LedPositionType gCurLedFlashDataElement;
LedPositionType gTotalLedFlashDataElements;
LedPositionType gNextFlashLedPosition;

LedDataStruct gLedSolidData[MAX_LED_SOLID_POSITIONS];
LedPositionType gCurLedSolidDataElement;
LedPositionType gTotalLedSolidDataElements;
LedPositionType gNextSolidLedPosition;

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
		ledData.position = (gwUINT16) gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES
				+ (sampleNum * LED_SAMPLE_BYTES + 0)] << 8;
		ledData.position += (gwUINT16) gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES
				+ (sampleNum * LED_SAMPLE_BYTES + 1)];
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

EControlCmdAckStateType processRequestQtySubCommand(BufferCntType inRXBufferNum) {
	EControlCmdAckStateType result = eAckStateOk;

	gwUINT8 pos = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMSPOS_POSITION];
	gwUINT8 reqQty = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_REQ_QTY];
	gwUINT8 minQty = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MIN_QTY];
	gwUINT8 maxQty = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MAX_QTY];
	gwUINT8 freq = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_FREQ];
	gwUINT8 dutyCycle = gRXRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_DUTYCYCLE];

	RS485_TX_ON;
	gwUINT8 message[] = {POS_CTRL_DISPLAY, pos, reqQty, minQty, maxQty, freq, dutyCycle};
	serialTransmitFrame(UART_2, message, 7);

	// Wait until all of the TX bytes have been sent.
	while (UART1_REGS_P->Utxcon < 32) {
		vTaskDelay(1);
	}
	vTaskDelay(25);

	RS485_TX_OFF;

	return result;
}


#endif
