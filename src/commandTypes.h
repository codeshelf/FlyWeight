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

#define UNIQUE_ID_BYTES			8
#define PRIVATE_GUID			"00000000"

/*
 * The format of a packet on the network is as follows:
 * 
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
#define PCKPOS_VERSION			0
#define PCKPOS_NETID			0
#define PCKPOS_ADDR				1
#define CMDPOS_CMDID			2
#define CMDPOS_ENDPOINT			2
#define CMDPOS_STARTOFCMD		3

// Network Mgmt
#define CMDPOS_MGMT_SUBCMD		3
#define CMDPOS_SETUP_CHANNEL	4
#define CMDPOS_CHECK_TYPE		4
#define CMDPOS_INTF_TEST_NUM	4
#define CMDPOS_CHECK_NETID		5
#define CMDPOS_CHECK_UID		6
#define CMDPOS_CHECK_CHANNEL	14
#define CMDPOS_CHECK_ENERGY		15
#define CMDPOS_CHECK_LINKQ		16

// Assoc Command
#define CMDPOS_ASSOC_SUBCMD		3
#define CMDPOS_ASSOC_UID		4
#define CMDPOS_ASSOCREQ_VER		12
#define CMDPOS_ASSOCRESP_ADDR	12
#define CMDPOS_ASSOCASCK_STATE	12

// Info Command
#define CMDPOS_INFO_SUBCMD		3
#define CMDPOS_INFO_QUERY		4
#define CMDPOS_INFO_RESPONSE	4

// Control Command
#define CMDPOS_CONTROL			3
#define CMDPOS_CONTROL_SUBCMD	3
#define CMDPOS_CONTROL_DATA		4

// Audio Command
#define CMDPOS_AUDIO			3
#define CMD_MAX_AUDIO_BYTES		120

// Command masks
#define PACKETMASK_VERSION		0b11000000
#define PACKETMASK_NETID		0b00111000
#define CMDMASK_SRC_ADDR		0b11110000
#define CMDMASK_DST_ADDR		0b00001111
#define CMDMASK_CMDID			0b11110000
#define CMDMASK_ENDPOINT		0b00001111
#define CMDMASK_ASSIGNID		0b11110000
#define CMDMASK_NETID			0b11100000

#define SHIFTBITS_PKT_VER		6
#define SHIFTBITS_PKT_NETID		3
#define SHIFTBITS_PKT_SRCADDR	4
#define SHIFTBITS_CMDID			4
#define SHIFTBITS_CMDNETID		5
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
	eRemoteStateAssocReqRcvd,
	eRemoteStateQuerySent,
	eRemoteStateRespRcvd,
	eRemoteStateRun
} ERemoteStatusType;

typedef enum {
	eLocalStateUnknown,
	eLocalStateStarted,
	eLocalStateAssocReqSent,
	eLocalStateAssocRespRcvd,
	eLocalStateAssociated,
	eLocalStateQueryRcvd,
	eLocalStateRespSent,
	eLocalStateRun
} ELocalStatusType;

/*
 * Network  commands
 * 
 * CommandNetSetup
 * 
 * This command is sent to the gateway (dongle) by the controller and never gets transmitted to the
 * radio network.  It is used to negotiate the creation of a new network on behalf of the controller
 * when it restarts.
 * 
 * CommandAssocReq
 * 
 * When a remote first starts it broadcasts an associate command which contains a unique ID for the remote.
 * It does this on every channel until a controller responds with an associate response.
 * 
 * CommandAssocResp
 * 
 * The controller responds to the associate request command by assigning a local destination address for the remote.
 * The remote should then act on all messages sent to that address or the broadcast address.
 * 
 * CommandNetCheck
 * 
 * This command is used *after* the remote is associated with a remote.
 * The remote should send the net check request command from time-to-time.  The controller will respond.
 * If the contoller doesn't respond then the remote should assume the controller has quit or reset.
 *
 * QueryCommand
 * 
 * The controller sends a query to the remote asking for details of one or more facilities.
 * 
 * ResponseCommand
 * 
 * The remote responds to the query command with the requested information about the facility.
 * 
 * ControlAudio
 * 
 * The controller or the remote can send a control audio command that contains audio to be played on
 * an endpoint.
 * 
 * ControlMotor
 * 
 * The controller or the remote can send a control motor command that contains instructions to run a
 * motor at the specified enfpoint
 */
typedef enum {
	eCommandInvalid = -1,
	eCommandNetMgmt = 0,
	eCommandAssoc = 1,
	eCommandInfo = 2,
	eCommandControl = 3,
	eCommandAudio = 4
} ECommandGroupIDType;

typedef enum {
	eNetMgmtSubCmdInvalid = -1,
	eNetMgmtSubCmdNetSetup = 1,
	eNetMgmtSubCmdNetCheck = 2,
	eNetMgmtSubCmdNetIntfTest = 3
} ENetMgmtSubCmdIDType;

typedef enum {
	eCmdAssocInvalid = -1,
	eCmdAssocREQ = 1,
	eCmdAssocRESP = 2,
	eCmdAssocCHECK = 3,
	eCmdAssocACK = 4
} ECmdAssocType;

typedef enum {
	eCmdInfoInvalid = -1,
	eCmdInfoQuery = 1,
	eCmdInfoResponse = 2
} EInfoSubCmdIDType;

typedef enum {
	eControlSubCmdInvalid = -1,
	eControlSubCmdEndpointAdj = 1,
	eControlSubCmdMotor = 2,
	eControlSubCmdButton = 3
} EControlSubCmdIDType;

// --------------------------------------------------------------------------
// Function prototypes.


// --------------------------------------------------------------------------
// Globals.


#endif COMMANDSTYPES_H