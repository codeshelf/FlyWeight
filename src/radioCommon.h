/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef RADIOCOMMON_H
#define RADIOCOMMON_H

// Project includes
#include "PE_Types.h"
#include "smacGlue.h"
#include "commands.h"

// --------------------------------------------------------------------------
// Definitions.

// Priorities assigned to demo application tasks.
#define LED_BLINK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define SERIAL_RECV_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define RADIO_PRIORITY			( tskIDLE_PRIORITY + 2 )

#define MAX_PACKET_SIZE			121

#define RX_QUEUE_SIZE			5
#define RX_QUEUE_BALANCE		3
#define RX_BUFFER_COUNT			RX_QUEUE_SIZE
#define RX_BUFFER_SIZE			MAX_PACKET_SIZE

#define TX_QUEUE_SIZE			1
#define TX_BUFFER_COUNT			TX_QUEUE_SIZE
#define TX_BUFFER_SIZE			MAX_PACKET_SIZE

#define MASTER_TPM2_RATE		0x873

//#define RTS_ON			PTAD |= 0x40
//#define RTS_OFF			PTAD &= ~0x40
typedef enum {
	eUSBStateStopped,
	eUSBStateStarted
} USBStateType;

#define RTS_ON  /*if (gUSBState == eUSBStateStarted)*/ __asm bclr 6,0x00 //PTA6
#define RTS_OFF __asm bset 6,0x00 //PTA6
#define RTS_PORTENABLE      __asm bclr 6,0x01 // PTA6 - PTAPE
#define RTS_PORTDIRECTION   __asm bset 6,0x03 // PTA6 - PTADD

#define USB_STOP	RTS_OFF/*; gUSBState = eUSBStateStopped */
#define USB_START	RTS_ON /*; gUSBState = eUSBStateStarted */

// --------------------------------------------------------------------------
// Typedefs

typedef byte	BufferCntType;
typedef int		BufferOffsetType;
typedef byte	BufferStorageType;
typedef enum {
	eBufferStateFree,
	eBufferStateInUse
} BufferStatusType;

typedef struct {
	BufferStatusType		bufferStatus;
	BufferStorageType		bufferStorage[MAX_PACKET_SIZE + 4];
} RadioBufferStruct;

/* The queue used to send data from the radio to the radio receive task. */
extern xQueueHandle 		gRadioTransmitQueue;
extern xQueueHandle 		gRadioReceiveQueue;

// Radio RX buffer
extern RadioBufferStruct	gRXBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRXCurBufferNum;
extern BufferCntType		gRXUsedBuffers;

// Radio TX buffer
extern RadioBufferStruct	gTXBuffer[TX_BUFFER_COUNT];
extern BufferCntType		gTXCurBufferNum;
extern BufferCntType		gTXUsedBuffers;

// --------------------------------------------------------------------------
// Prototypes

bool transmitCommand(CommandPtrType inCommandP);
CommandPtrType receiveCommand(void );

#endif RADIOCOMMON_H