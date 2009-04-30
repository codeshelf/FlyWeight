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
//#include "gatewayRadioTask.h"
//#include "gatewayMgmtTask.h"
//#include "remoteRadioTask.h"
//#include "remoteMgmtTask.h"
#include "NV_Data.h"
#include "flash.h"

#ifdef _TOY_NETWORK_
	#include "deviceQuery.h"
#else
	#include "terminalQuery.h"
#endif

RemoteDescStruct	gRemoteStateTable[MAX_REMOTES];
RemoteAddrType		gMyAddr = INVALID_REMOTE;
NetworkIDType		gMyNetworkID = BROADCAST_NETID;
extern byte			gCCRHolder;


// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, RemoteAddrType inSrcAddr, RemoteAddrType inDestAddr);

// --------------------------------------------------------------------------

UINT8 transmitPacket(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;
	BufferCntType txBufferNum = inTXBufferNum;

	// Transmit the packet.
	if (xQueueGenericSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK)) {}

	return result;

};

// --------------------------------------------------------------------------

UINT8 transmitPacketFromISR(BufferCntType inTXBufferNum) {

	UINT8 result = INITIAL_VALUE;

	// Transmit the packet.
	if (xQueueSendFromISR(gRadioTransmitQueue, &inTXBufferNum, (portTickType) 0)) {}

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
	gTXRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateInUse;
};


// --------------------------------------------------------------------------

void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocREQ;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_VER] = 0x01;
	// Set the system status register
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT] = SRS;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_SYSSTAT + 1;
};

// --------------------------------------------------------------------------

void createAssocCheckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	UINT8 saveATD1C;
	UINT8 saveATD1SC;
	INT8 batteryLevel = 0;

	// Create the command for checking if we're associated
	createPacket(inTXBufferNum, eCommandAssoc, gMyNetworkID, gMyAddr, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocCHECK;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_UID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_VER] = 0x01;
	
#if !defined(XBEE)
	// Save the ATD state and prepare to take a battery measurement.
	saveATD1C = ATD1C;
	saveATD1SC = ATD1SC;
	ATD1C_DJM = 1;
	ATD1C_RES8 = 1;
	ATD1SC_ATDCH = 0x04;
	ATD_ON;
	
	// Take an audio sample first.
	// Start and wait for a ATD conversion.
	ATD1SC_ATDCO = 0;
	while (!ATD1SC_CCF) {
	}

	// Get the measurement.
	// Battery values will range from 1.6V to 3.1V, but there is a voltage divide
	// in the circuit, so ATD values will range from 64-128.  The device starts failing
	// around 70, and if we set 78 as the floor then 2x gives us a nice 100 scale.
	// (Since 128 - 78 - 50)
	batteryLevel = ATD1RH - 78;
	
	// Restore state.
	ATD1SC = saveATD1SC;
	ATD1C = saveATD1C;

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

#ifdef IS_GATEWAY
void createOutboundNetSetup() {
	BufferCntType txBufferNum;

	while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1 * portTICK_RATE_MS);
	}

	vTaskSuspend(gRadioReceiveTask);

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
#endif

// --------------------------------------------------------------------------

void createButtonControlCommand(BufferCntType inTXBufferNum, UINT8 inButtonNumber, UINT8 inFunctionType) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdButton;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA] = inButtonNumber;
	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_DATA + 1] = inFunctionType;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_CONTROL_DATA + 2;
};

// --------------------------------------------------------------------------

void createAudioCommand(BufferCntType inTXBufferNum) {

	createPacket(inTXBufferNum, eCommandAudio, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + 1;
};

// --------------------------------------------------------------------------

void processNetSetupCommand(BufferCntType inTXBufferNum) {

	ChannelNumberType	channel;

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

#ifdef IS_GATEWAY
void processNetIntfTestCommand(BufferCntType inTXBufferNum) {

	BufferCntType txBufferNum;

	/*
	 * At this point we transmit one inbound interface test back over the serial link from the gateway (dongle) itself.
	 *
	 * The only rational way to do this is to use a transmit buffer.  The reason is that the radio may
	 * already be waiting to fill an inbound packet that we already sent to the MAC.  There is no
	 * way to let the MAC know that we're about to switch the current RX buffer.  For this reason
	 * the only safe buffer available to us comes from the TX buffer.
	 */

	// Wait until we can get an TX buffer
	while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1 * portTICK_RATE_MS);
	}

	vTaskSuspend(gRadioReceiveTask);

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
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_MGMT_SUBCMD] = eNetMgmtSubCmdNetIntfTest;
	gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_INTF_TEST_NUM] = 	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_INTF_TEST_NUM];

	gTXRadioBuffer[txBufferNum].bufferSize = CMDPOS_INTF_TEST_NUM + 1;

	serialTransmitFrame((byte*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum);

	vTaskResume(gRadioReceiveTask);

};
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
		gTXRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CHECK_TYPE] = eCmdAssocRESP;

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
#endif

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

// --------------------------------------------------------------------------

void createDataSampleCommand(BufferCntType inTXBufferNum, EndpointNumType inEndpoint) {

	createPacket(inTXBufferNum, eCommandDataSample, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ENDPOINT] |= (CMDMASK_ENDPOINT & inEndpoint);

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_DATA_SUBCMD] = eRemoteDataSubCmdSample;

	gTXRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_SAMPLE_CNT] = 0;

	gTXRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_SAMPLE_CNT + 1;
};

// --------------------------------------------------------------------------

void addDataSampleToCommand(BufferCntType inTXBufferNum, TimestampType inTimestamp, DataSampleType inDataSample, char inUnitsByte) {
	
	UINT8 pos = gTXRadioBuffer[inTXBufferNum].bufferSize;
	
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
	
};