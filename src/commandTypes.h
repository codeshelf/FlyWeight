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
#define DEVICE_VERSION_NUM		1
#define DEVICE_VERSION_NUM_LEN	1

/*
 * The format of a packet on the network is as follows:
 * 
 * 1B - Packet length
 * 2b - Version
 * 3b - Network number
 * 3b - Reserved
 * 4b - Packet source address
 * 4b - Packet dest address
 * 4b - Command ID
 * 4b - Command endpoint
 * nB - Command bytes
 * 
 * Where B = byte, b = bit
 * 
 */

// Command format positioning constants.
// Packet
#define PCKPOS_SIZE				0
#define PCKPOS_ADDR				2
#define CMDPOS_CMDID			3
#define CMDPOS_STARTOFCMD		4

// Wake Command
#define CMDPOS_PROTOCOL_ID		4
#define CMDPOS_WAKE_UID			5

// Assign Command
#define CMDPOS_ASSIGN_UID		4
#define CMDPOS_ASSIGN_ADDR		12

// Assign Ack Command
#define CMDPOS_ASSIGNACK_UID	4
#define CMDPOS_ASSIGNACK_ADDR	12

// Data Command

// Query Command
#define CMDPOS_QUERY			4

// Response Command
#define CMDPOS_RESPONSE			4

// Command masks
#define PACKETMASK_VERSION		0xc0
#define PACKETMASK_NETWORK_NUM	0x38
#define CMDMASK_SRC_ADDR		0xf0
#define CMDMASK_DST_ADDR		0x0f
#define CMDMASK_CMDID			0xf0
#define CMDMASK_ASSIGNID		0xf0

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
	eRemoteStateQuerySent,
	eRemoteStateRespRcvd,
	eRemoteStateRun
} ERemoteStatusType;

typedef enum {
	eLocalStateUnknown,
	eLocalStateWakeSent,
	eLocalStateAddrAssignRcvd,
	eLocalStateAddrAssignAckSent,
	eLocalStateQueryRcvd,
	eLocalStateRespSent,
	eLocalStateRun
} ELocalStatusType;

/*
 * Network  commands
 *
 * CommandDatagram
 * 
 * These are the command sent between the devices to/from the non-zero endpoints and carry the data to "run".
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
 * CommandAssignAck
 *
 * Let the controller know that we received the address assignment.  (Sent from the assigned address,
 * and includes the unique ID of the device.
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
	eCommandInvalid = 0,
	eCommandDatagram = 1,
	eCommandWake = 2,
	eCommandAddrAssign = 3,
	eCommandAddrAssignAck = 4,
	eCommandQuery = 5,
	eCommandResponse = 6,
} RadioCommandIDType;

// --------------------------------------------------------------------------
// Function prototypes.


// --------------------------------------------------------------------------
// Globals.


#endif COMMANDSTYPES_H