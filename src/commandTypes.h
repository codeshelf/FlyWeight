/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef COMMANDSTYPES_H
#define COMMANDSTYPES_H

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
	eControllerStateUnknown,
	eControllerStateInit,
	eControllerStateRun
} ControllerStateType;

/*
 * The remote device state machine.  (From the POV of the controller.)
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
	eRemoteStateUnknown,
	eRemoteStateWakeRcvd,
	eRemoteStateAddrAssignSent,
	eRemoteStateQuerySent,
	eRemoteStateRespRcvd,
	eRemoteStateRun
} ERemoteStatusType;

typedef enum {
	eLocalStateUnknown,
	eLocalStateWakeSent,
	eLocalStateAddrAssignRcvd,
	eLocalStateQueryRcvd,
	eLocalStateRespSent,
	eLocalStateRun
} ELocalStatusType;

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
	eCommandQuery = 4,
	eCommandResponse = 5
} RadioCommandIDType;

// --------------------------------------------------------------------------
// Function prototypes.


// --------------------------------------------------------------------------
// Globals.


#endif COMMANDSTYPES_H