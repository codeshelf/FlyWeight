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

#define UNIQUE_ID_LEN			8

// Command format positioning constants.
// Header
#define CMDPOS_ADDR				0
#define CMDPOS_CMDID			1

// Wake
#define CMDPOS_WAKE_UID			2

// Assign
#define CMDPOS_ASSIGN_ADDR		2
#define CMDPOS_ASSIGN_UID		3

// Query response
#define CMDPOS_RESPONSE			2

#define CMDMASK_SRC_ADDR		0xf0
#define CMDMASK_DST_ADDR		0x0f
#define CMDMASK_CMDID			0xf0
#define CMDMASK_ASSIGNID		0x0f

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
	eRemoteStateDescSent,
	eRemoteStateRun
} ERemoteStatusType;

typedef enum {
	eLocalStateUnknown,
	eLocalStateWakeSent,
	eLocalStateAddrAssignRcvd,
	eLocalStateQueryRcvd,
	eLocalStateRespSent,
	eLocalStateDescRcvd,
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
 * DescCommand
 * 
 * The descriptor command tell the remote about the format of the data destined for a channel.
 * 
 */
typedef enum {
	eCommandWake = 0,
	eCommandAssign = 1,
	eCommandChannelDesc = 2,
	eCommandData = 3,
	eCommandQuery = 4,
	eCommandResponse = 5,
	eCommandDesc = 6
} RadioCommandIDType;

// --------------------------------------------------------------------------
// Function prototypes.


// --------------------------------------------------------------------------
// Globals.


#endif COMMANDSTYPES_H