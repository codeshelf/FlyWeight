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

#define MOTOR1_ENDPOINT		1
#define MOTOR2_ENDPOINT		2

// --------------------------------------------------------------------------
// Typedefs.

typedef enum {
	eMotorCommandInvalid = -1,
	eMotorCommandFreewheel = 0,
	eMotorCommandFwd = 1,
	eMotorCommandBwd = 2,
	eMotorCommandBrake = 3
} EMotorCommandType;


// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to copy/move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */
 
UINT8 transmitPacket(BufferCntType inTXBufferNum);

ERadioCommandIDType getCommandNumber(BufferCntType inRXBufferNum);
ERadioControlCommandIDType getControlCommandNumber(BufferCntType inRXBufferNum);
EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum);
RemoteAddrType getCommandSrcAddr(BufferCntType inRXBufferNum);
RemoteAddrType getCommandDstAddr(BufferCntType inRXBufferNum);

void createWakeCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createAddrAssignAckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr);
void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, RemoteAddrType inRemoteAddr);

void processAssignCommand(BufferCntType inRXBufferNum);
void processQueryCommand(BufferCntType inRXBufferNum,  RemoteAddrType inRemoteAddr);
void processResponseCommand(BufferCntType inRXBufferNum, RemoteAddrType inRemoteAddr);

void processMotorControlCommand(BufferCntType inRXBufferNum);

// --------------------------------------------------------------------------
// Globals.

extern ControllerStateType	gControllerState;
extern ELocalStatusType		gLocalDeviceState;

#endif COMMANDS_H