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


// --------------------------------------------------------------------------
// Typedefs.


// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to copy/move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */
 
UINT8 transmitPacket(BufferCntType inTXBufferNum);

RadioCommandIDType getCommandNumber(BufferCntType inRXBufferNum);
RemoteAddrType getCommandSrcAddr(BufferCntType inRXBufferNum);
RemoteAddrType getCommandDstAddr(BufferCntType inRXBufferNum);

void createWakeCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createAddrAssignAckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createQueryCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr);
void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, RemoteAddrType inRemoteAddr);
void createDataCommand(BufferCntType inTXBufferNum, RemoteAddrType inRemoteAddr);

void processAssignCommand(BufferCntType inRXBufferNum);
void processQueryCommand(BufferCntType inRXBufferNum,  RemoteAddrType inRemoteAddr);
void processResponseCommand(BufferCntType inRXBufferNum, RemoteAddrType inRemoteAddr);

// --------------------------------------------------------------------------
// Globals.

extern ControllerStateType	gControllerState;
extern ELocalStatusType		gLocalDeviceState;

#endif COMMANDS_H