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

// --------------------------------------------------------------------------
// Definitions.

// Priorities assigned to demo application tasks.
#define LED_BLINK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define RADIO_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define RADIO_QUEUE_SIZE		4

#define ASYNC_BUFFER_COUNT			RADIO_QUEUE_SIZE
#define ASYNC_BUFFER_SIZE			121

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
	eBufferStateUnknown,
	eBufferStateEmpty,
	eBufferStateFull
} BufferStatusType;

typedef struct {
	BufferStatusType		bufferStatus;
	BufferStorageType		bufferStorage[ASYNC_BUFFER_SIZE+2];
} RadioBufferStruct;
//typedef RadioBufferStruct *RadioBufferPtrType;

#endif RADIOCOMMON_H