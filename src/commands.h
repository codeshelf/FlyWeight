/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved
 
 $Id$
 $Name$	
 */

#ifndef COMMANDS_H
#define COMMANDS_H

// Project includes
#include "gwTypes.h"
#include "commandTypes.h"
#include "radioCommon.h"
#include "serial.h"

// --------------------------------------------------------------------------
// Definitions.

#define DEVICE_CONTROLLER		0
#define DEVICE_GATEWAY			1
#define DEVICE_REMOTE			2

//#define MOTOR1_ENDPOINT		2
//#define MOTOR2_ENDPOINT		3

#define LED_SAMPLE_BYTES		5

#if defined(GW0009R1)
	#define LEDRedON	PTBD_PTBD1 = 1;
	#define LEDGreenON	PTDD_PTDD1 = 1;
	#define LEDBlueON	PTDD_PTDD0 = 1;

	#define LEDRedOFF	PTBD_PTBD1 = 0;
	#define LEDGreenOFF	PTDD_PTDD1 = 0;
	#define LEDBlueOFF	PTDD_PTDD0 = 0;
#elif defined(GW0009R2)
	#define LEDRedON	PTBD_PTBD2 = 1;
	#define LEDGreenON	PTBD_PTBD3 = 1;
	#define LEDBlueON	PTBD_PTBD4 = 1;

	#define LEDRedOFF	PTBD_PTBD2 = 0;
	#define LEDGreenOFF	PTBD_PTBD3 = 0;
	#define LEDBlueOFF	PTBD_PTBD4 = 0;
#endif

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

typedef enum {
	eAckStateOk = 0,
	eAckStateNotNeeded,
	eAckStateFailed
} EControlCmdAckStateType;


typedef gwUINT8 ChannelNumberType;

// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to copy/move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */

gwUINT8 transmitPacket(BufferCntType inTXBufferNum);
gwUINT8 transmitPacketFromISR(BufferCntType inTXBufferNum);

gwBoolean getAckRequired(BufferStoragePtrType inBufferPtr);
ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr);
NetworkIDType getNetworkID(BufferCntType inRXBufferNum);
AckIDType getAckId(BufferCntType inRXBufferNum);

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr);
ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum);
EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum);

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum);
NetAddrType getCommandSrcAddr(BufferCntType inRXBufferNum);
NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum);

gwUINT8 getLEDVaue(gwUINT8 inLEDNum, BufferCntType inRXBufferNum);

void createNetCheckRespInboundCommand(BufferCntType inRXBufferNum);
void createAckPacket(BufferCntType inTXBufferNum, AckIDType inAckId, AckDataType inAckData);
void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createAssocCheckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID);
void createButtonControlCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNumber, gwUINT8 inFunctionType);
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

EControlCmdAckStateType processMotorControlSubCommand(BufferCntType inRXBufferNum);
EControlCmdAckStateType processHooBeeSubCommand(BufferCntType inRXBufferNum);
EControlCmdAckStateType processSDCardModeSubCommand(BufferCntType inRXBufferNum, AckIDType inAckId, AckDataType inOutAckData);
EControlCmdAckStateType processSDCardUpdateSubCommand(BufferCntType inRXBufferNum);
EControlCmdAckStateType processSDCardUpdateCommitSubCommand(BufferCntType inRXBufferNum, AckDataType inOutAckData);
EControlCmdAckStateType processSDCardBlockCheckSubCommand(BufferCntType inRXBufferNum);

void createDataSampleCommand(BufferCntType inTXBufferNum, EndpointNumType inEndpoint);
void addDataSampleToCommand(BufferCntType inTXBufferNum, TimestampType inTimestamp, DataSampleType inDataSample, char inUnitsByte);

// --------------------------------------------------------------------------
// Globals.

extern ControllerStateType gControllerState;
extern ELocalStatusType gLocalDeviceState;

#endif /* COMMANDS_H */
