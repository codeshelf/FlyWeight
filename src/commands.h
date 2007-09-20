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

// --------------------------------------------------------------------------
// Definitions.

#define DEVICE_CONTROLLER		0
#define DEVICE_GATEWAY			1
#define DEVICE_REMOTE			2

#define DEVICE_VER_BYTES		1

#define PACKET_HEADER_BYTES		1
#define PACKET_VERSION			0x01
#define BROADCAST_NETID			0b111

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

typedef UINT8	NetworkIDType;
typedef UINT8	ChannelNumberType;

// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to copy/move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */
 
UINT8 transmitPacket(BufferCntType inTXBufferNum);

ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr);

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr);
ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum);
EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum);

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum);
RemoteAddrType getCommandSrcAddr(BufferCntType inRXBufferNum);
RemoteAddrType getCommandDstAddr(BufferCntType inRXBufferNum);

void createNetCheckRespInboundCommand(BufferCntType inRXBufferNum);
void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr);
void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, RemoteAddrType inRemoteAddr);
void createOutboundNetsetup(void);

void processNetCheckInboundCommand(BufferCntType inRXBufferNum);
void processNetCheckOutboundCommand(BufferCntType inTXBufferNum);
void processNetSetupCommand(BufferCntType inRXBufferNum);
void processAssocRespCommand(BufferCntType inRXBufferNum);
void processQueryCommand(BufferCntType inRXBufferNum,  RemoteAddrType inRemoteAddr);
void processResponseCommand(BufferCntType inRXBufferNum, RemoteAddrType inRemoteAddr);

void processMotorControlSubCommand(BufferCntType inRXBufferNum);

// --------------------------------------------------------------------------
// Globals.

extern ControllerStateType	gControllerState;
extern ELocalStatusType		gLocalDeviceState;

#endif COMMANDS_H