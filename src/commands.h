/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved
 
 $Id$
 $Name$	
 */

#ifndef COMMANDS_H
#define COMMANDS_H

// Project includes
#include "PE_Types.h"
#include "pub_def.h"
#include "commandTypes.h"
#include "radioCommon.h"
#include "serial.h"

// --------------------------------------------------------------------------
// Definitions.

#define DEVICE_CONTROLLER		0
#define DEVICE_GATEWAY			1
#define DEVICE_REMOTE			2

#define DEVICE_VER_BYTES		1

#define PACKET_HEADER_BYTES		1
#define PACKET_VERSION			0x00
#define BROADCAST_NETID			0b1111

#define INVALID_CHANNEL			-1
#define AUTOMATIC_CHANNEL		127

#define MOTOR1_ENDPOINT		2
#define MOTOR2_ENDPOINT		3

// --------------------------------------------------------------------------
// Typedefs.

typedef enum {
	eMotorCommandInvalid = -1,
	eMotorCommandFreewheel = 0,
	eMotorCommandFwd = 1,
	eMotorCommandBwd = 2,
	eMotorCommandBrake = 3
} EMotorCommandType;

typedef enum {
	eHooBeeBehaviorInvalid = -1,
	eHooBeeBehaviorLedFlash = 1,
	eHooBeeBehaviorSolenoidPush = 2,
	eHooBeeBehaviorSolenoidPull = 3
} EHooBeeBehaviorType;

typedef UINT8 NetworkIDType;
typedef UINT8 ChannelNumberType;

// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to copy/move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */

UINT8 transmitPacket(BufferCntType inTXBufferNum);
UINT8 transmitPacketFromISR(BufferCntType inTXBufferNum);

bool getAckRequired(BufferStoragePtrType inBufferPtr);
ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr);
NetworkIDType getNetworkID(BufferCntType inRXBufferNum);
AckIDType getAckId(BufferCntType inRXBufferNum);

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr);
ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum);
EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum);

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum);
NetAddrType getCommandSrcAddr(BufferCntType inRXBufferNum);
NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum);

UINT8 getLEDVaue(UINT8 inLEDNum, BufferCntType inRXBufferNum);

void createNetCheckRespInboundCommand(BufferCntType inRXBufferNum);
void createAckCommand(BufferCntType inTXBufferNum, AckIDType inAckId);
void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createAssocCheckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createButtonControlCommand(BufferCntType inTXBufferNum, UINT8 inButtonNumber, UINT8 inFunctionType);
void createQueryCommand(BufferCntType inTXBufferNum, NetAddrType inRemoteAddr);
void createAudioCommand(BufferCntType inTXBufferNum);
void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, NetAddrType inRemoteAddr);
#ifdef IS_GATEWAY
void createOutboundNetSetup(void);
#endif

void processNetCheckInboundCommand(BufferCntType inRXBufferNum);
#ifdef IS_GATEWAY
void processNetCheckOutboundCommand(BufferCntType inTXBufferNum);
void processNetIntfTestCommand(BufferCntType inTXBufferNum);
#endif
void processNetSetupCommand(BufferCntType inRXBufferNum);
void processAssocRespCommand(BufferCntType inRXBufferNum);
void processQueryCommand(BufferCntType inRXBufferNum, NetAddrType inRemoteAddr);
void processResponseCommand(BufferCntType inRXBufferNum, NetAddrType inRemoteAddr);

void processMotorControlSubCommand(BufferCntType inRXBufferNum);
void processHooBeeSubCommand(BufferCntType inRXBufferNum);

void createDataSampleCommand(BufferCntType inTXBufferNum, EndpointNumType inEndpoint);
void addDataSampleToCommand(BufferCntType inTXBufferNum, TimestampType inTimestamp, DataSampleType inDataSample, char inUnitsByte);

// --------------------------------------------------------------------------
// Globals.

extern ControllerStateType gControllerState;
extern ELocalStatusType gLocalDeviceState;

#endif COMMANDS_H
