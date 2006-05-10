/*
	FlyWeight
	� Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef RADIOCOMMON_H
#define RADIOCOMMON_H

// Project includes
#include "PE_Types.h"
#include "smacGlue.h"
#include "commandTypes.h"

// --------------------------------------------------------------------------
// Definitions.

// Priorities assigned to demo application tasks.
#define LED_BLINK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define MGMT_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define SERIAL_RECV_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define RADIO_PRIORITY			( tskIDLE_PRIORITY + 2 )

#define MAX_PACKET_SIZE			121

#define RX_QUEUE_SIZE			5
#define RX_QUEUE_BALANCE		3
#define RX_BUFFER_COUNT			RX_QUEUE_SIZE
#define RX_BUFFER_SIZE			MAX_PACKET_SIZE

#define TX_QUEUE_SIZE			2
#define TX_BUFFER_COUNT			TX_QUEUE_SIZE
#define TX_BUFFER_SIZE			MAX_PACKET_SIZE

#define MASTER_TPM2_RATE		0x873

//#define RTS_ON				PTAD |= 0x40
//#define RTS_OFF				PTAD &= ~0x40
#define RTS_ON  				/*if (gUSBState == eUSBStateStarted)*/ __asm bclr 6,0x00 //PTA6
#define RTS_OFF 				__asm bset 6,0x00 //PTA6
#define RTS_PORTENABLE      	__asm bclr 6,0x01 // PTA6 - PTAPE
#define RTS_PORTDIRECTION   	__asm bset 6,0x03 // PTA6 - PTADD

#define USB_STOP				RTS_OFF/*; gUSBState = eUSBStateStopped */
#define USB_START				RTS_ON /*; gUSBState = eUSBStateStarted */

#define MAX_REMOTES				14
#define INVALID_REMOTE			MAX_REMOTES + 1
#define UNIQUE_ID_LEN			8

#define ADDR_CONTROLLER			0x00
#define ADDR_BROADCAST			0x0F

// --------------------------------------------------------------------------
// Typedefs

typedef enum {
	eUSBStateStopped,
	eUSBStateStarted
} USBStateType;

typedef byte	BufferCntType;
typedef int		BufferOffsetType;
typedef byte	BufferStorageType;
typedef enum {
	eBufferStateFree,
	eBufferStateInUse,
} EBufferStatusType;

typedef struct {
	EBufferStatusType		bufferStatus;
	BufferStorageType		bufferStorage[MAX_PACKET_SIZE + 4];
	BufferCntType			bufferSize;
} RadioBufferStruct;

typedef UINT8				RemoteAddrType;
typedef byte				RemoteUniqueIDType[UNIQUE_ID_LEN];
typedef RemoteUniqueIDType	*RemoteUniqueIDPtrType;
typedef struct {
	ERemoteStatusType		remoteState;
	RemoteUniqueIDType		remoteUniqueID;
} RemoteDescStruct;

// --------------------------------------------------------------------------
// Externs

/* The queue used to send data from the radio to the radio receive task. */
extern xQueueHandle 		gRadioTransmitQueue;
extern xQueueHandle 		gRadioReceiveQueue;

// Radio RX buffer
extern RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRXCurBufferNum;
extern BufferCntType		gRXUsedBuffers;

// Radio TX buffer
extern RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType		gTXCurBufferNum;
extern BufferCntType		gTXUsedBuffers;

extern tTxPacket			gsTxPacket;
extern tRxPacket			gsRxPacket;

extern UINT8 gu8RTxMode;

extern RemoteDescStruct		gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Prototypes

#endif RADIOCOMMON_H