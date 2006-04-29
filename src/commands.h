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
//#include "radioCommon.h"

// --------------------------------------------------------------------------
// Definitions.

// --------------------------------------------------------------------------
// Typedefs

/*
 * The controller state machine;
 * 
 * ControllerStateInit
 * 
 * This is the state of the controller when the controller first starts.
 * 
 * 		-> ControllerStateRun - After initialization.
 */
typedef enum {
	eControllerStateInit,
	eControllerStateRun
} ControllerStateType;

/*
 * The remote state machine;
 * 
 * RemoteStateInit
 * 
 * This is the state of the remote when the remote first starts.
 * 
 * 		-> RemoteStateWakeSent - After initialization the remote broadcasts the wake command.
 * 
 * RemoteStateWakeSent
 * 
 * Yhis is the state after the remote broadcasts the wake command
 * 
 * 		-> RemoteStateRun - After receiving an assign command.
 */
typedef enum {
	eRemoteStateInit,
	eRemoteStateWakeSent,
	eRemoteStateRun
} RemoteStateType;

/*
 * Commands
 * 
 * CommandWake
 * 
 * When a remote first starts it broadcasts a wake command which contains a unique ID for the remote.  This
 * informs the controller that there is a new device that wants to join the network.  The command includes
 * sub-commands that describe the facilities in the remote.
 * 
 * CommandAssign
 * 
 * The controller responds to the wake command by assigning a local destination address for the remote.
 * The remote should then act to all messages sent to that address or the broadcast address.
 * 
 * CommandChannelDesc
 * 
 * The remote contains one of more facilities.  The controller assigns one channel for each facility
 * in use.  (The controller decides what facilities in what remotes to use to drive the concerted action.)
 * The command contains sub-commands that indicate the attributes of the channel.
 * 
 * DataCommand
 * 
 * The controller sends data to the remote on a channel using the data command.
 * 
 * QueryCommand
 * 
 * The controller sends a query to the remote asking for details of one or more facilities.
 * 
 * ResponseCommand
 * 
 * The remote responds to the query command with the requested information about the facility.
 * 
 * 
 */
typedef enum {
	eCommandWake = 0,
	eCommandAssign = 1,
	eCommandChannelDesc = 2,
	eCommandData = 3,
	eCommandResponseReq = 4,
	eCommandResponse = 5
} FlyWeightCommandType;

typedef struct {
	UINT8	cmdID;
	UINT8	dest;
} CommandHeaderType;

typedef byte* CommandPtrType;

// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */
 
CommandPtrType createWakeCommand(void);

// --------------------------------------------------------------------------
// Globals.


#endif COMMANDS_H