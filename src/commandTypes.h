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
#include "gwTypes.h"

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
#define PCKPOS_HEADER_BITS		0
#define PCKPOS_NETID			0
#define PCKPOS_PACKET_NUM		1
#define PCKPOS_ADDR				2
#define CMDPOS_CMDID			3
#define CMDPOS_ENDPOINT			3
#define CMDPOS_STARTOFCMD		4

// Network Mgmt
#define CMDPOS_MGMT_SUBCMD		CMDPOS_STARTOFCMD
#define CMDPOS_SETUP_CHANNEL	CMDPOS_MGMT_SUBCMD + 1
#define CMDPOS_CHECK_TYPE		CMDPOS_SETUP_CHANNEL + 0
#define CMDPOS_INTF_TEST_NUM	CMDPOS_CHECK_TYPE + 0
#define CMDPOS_CHECK_NETID		CMDPOS_INTF_TEST_NUM + 1
#define CMDPOS_CHECK_UID		CMDPOS_CHECK_NETID + 1
#define CMDPOS_CHECK_CHANNEL	CMDPOS_CHECK_UID + 8
#define CMDPOS_CHECK_ENERGY		CMDPOS_CHECK_CHANNEL + 1
#define CMDPOS_CHECK_LINKQ		CMDPOS_CHECK_ENERGY + 1
#define CMDPOS_ACK_ID			CMDPOS_MGMT_SUBCMD + 1

// Assoc Command
#define CMDPOS_ASSOC_SUBCMD		CMDPOS_STARTOFCMD
#define CMDPOS_ASSOC_UID		CMDPOS_ASSOC_SUBCMD + 1
#define CMDPOS_ASSOCREQ_VER		CMDPOS_ASSOC_UID + 8
#define CMDPOS_ASSOCRESP_ADDR	CMDPOS_ASSOCREQ_VER + 0
#define CMDPOS_ASSOCRESP_NET	CMDPOS_ASSOCRESP_ADDR + 0
#define CMDPOS_ASSOCACK_STATE	CMDPOS_ASSOCRESP_ADDR + 0
#define CMDPOS_ASSOCACK_TIME	CMDPOS_ASSOCACK_STATE + 1
#define CMDPOS_ASSOCREQ_SYSSTAT CMDPOS_ASSOCACK_STATE + 1
#define CMDPOS_ASSOCCHK_BATT	CMDPOS_ASSOCREQ_SYSSTAT + 0

// Info Command
#define CMDPOS_INFO_SUBCMD		CMDPOS_STARTOFCMD
#define CMDPOS_INFO_QUERY		CMDPOS_INFO_SUBCMD + 1
#define CMDPOS_INFO_RESPONSE	CMDPOS_INFO_QUERY + 0

// Control Command
#define CMDPOS_CONTROL			CMDPOS_STARTOFCMD
#define CMDPOS_CONTROL_SUBCMD	CMDPOS_CONTROL + 0
#define CMDPOS_CONTROL_ACKID	CMDPOS_CONTROL_SUBCMD + 1
#define CMDPOS_CONTROL_DATA		CMDPOS_CONTROL_ACKID + 1

// Audio Command
#define CMDPOS_AUDIO			CMDPOS_STARTOFCMD
#define CMD_MAX_AUDIO_BYTES		120

// Data Sample Command
#define CMDPOS_DATA_SUBCMD		CMDPOS_STARTOFCMD
#define CMDPOS_SAMPLE_CNT		CMDPOS_DATA_SUBCMD + 1
#define CMDPOS_SAMPLE_FIRST		CMDPOS_SAMPLE_CNT + 1

// HooBee Command
#define CMDPOS_BEHAVIOR			CMDPOS_CONTROL_DATA
#define CMDPOS_BEHAVIOR_CNT		CMDPOS_BEHAVIOR + 0
#define CMDPOS_BEHAVIOR_SUBCMD	CMDPOS_BEHAVIOR_CNT + 1

// SDCard Control Command
#define CMDPOS_SDCARD_ACTION	CMDPOS_CONTROL_DATA

// SDCard Update Command
#define CMDPOS_SDCARD_ADDR		CMDPOS_CONTROL_DATA
#define CMDPOS_SDCARD_PART		CMDPOS_SDCARD_ADDR + 4
#define CMDPOS_SDCARD_PARTS		CMDPOS_SDCARD_PART + 1
#define CMDPOS_SDCARD_LEN		CMDPOS_SDCARD_PARTS + 1
#define CMDPOS_SDCARD_DATA		CMDPOS_SDCARD_LEN + 1

// Command masks
#define PACKETMASK_VERSION		0xc0    /* 0b11000000 */
#define PACKETMASK_RSV_HDR		0x30	/* 0b00110000 */
#define PACKETMASK_NETID		0x0f    /* 0b00001111 */
#define CMDMASK_SRC_ADDR		0xf0    /* 0b11110000 */
#define CMDMASK_DST_ADDR		0x0f    /* 0b00001111 */
#define CMDMASK_CMDID			0xf0    /* 0b11110000 */
#define CMDMASK_ENDPOINT		0x0f    /* 0b00001111 */
#define CMDMASK_ASSIGN_ADDR		0xf0    /* 0b11110000 */
#define CMDMASK_ASSIGN_NETID	0x0f    /* 0b00001111 */

#define SHIFTBITS_PKT_VER		6
#define SHIFTBITS_RSV_HDR		4
#define SHIFTBITS_PKT_NETID		0
#define SHIFTBITS_PKT_SRCADDR	4
#define SHIFTBITS_CMDID			4
#define SHIFTBITS_CMD_ASGNADDR	4
#define SHIFTBITS_CMD_ASGNNETID	0
#define SHIFTBITS_CMD_ENDPOINT	0
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
 * ControlAudioCommand
 *
 * The controller or the remote can send a control audio command that contains audio to be played on
 * an endpoint.
 *
 * ControlMotorCommand
 *
 * The controller or the remote can send a control motor command that contains instructions to run a
 * motor at the specified endpoint
 *
 * DataSampleCommand
 *
 * The remote collects and sends a command that contains one or more data samples.
 *
 */
typedef enum {
	eCommandInvalid = -1,
	eCommandNetMgmt = 0,
	eCommandAssoc = 1,
	eCommandInfo = 2,
	eCommandControl = 3,
	eCommandAudio = 4,
	eCommandDataSample = 5
} ECommandGroupIDType;

typedef enum {
	eNetMgmtSubCmdInvalid = -1,
	eNetMgmtSubCmdNetSetup = 1,
	eNetMgmtSubCmdNetCheck = 2,
	eNetMgmtSubCmdNetIntfTest = 3,
	eNetMgmtSubCmdAck = 4
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
	eControlSubCmdButton = 3,
	eControlSubCmdHooBee = 4,
	eControlSubCmdSDCardUpdate = 5,
	eControlSubCmdSDCardControl = 6
} EControlSubCmdIDType;

typedef enum {
	eRemoteDataSubCmdInvalid = -1,
	eRemoteDataSubCmdSample = 1,
	eRemoteDataSubCmdRateCtrl = 2,
	eRemoteDataSubCmdCalibrate = 3
} ERemoteDataSubCmdIDType;

typedef enum {
	eSDCardActionInvalid = -1,
	eSDCardActionBusConnect = 1,
	eSDCardActionBusDisconnect = 2,
	eSDCardActionVccConnect = 3,
	eSDCardActionVccDisconnect = 4
} ESDCardControlActionType;

// --------------------------------------------------------------------------
// Function prototypes.


// --------------------------------------------------------------------------
// Globals.


#endif /* COMMANDSTYPES_H */
