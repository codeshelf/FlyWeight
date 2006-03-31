/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef GATEWAYRADIOTASK_H
#define GATEWAYRADIOTASK_H

// Project includes
#include "PE_Types.h"
#include "smacGlue.h"

// --------------------------------------------------------------------------
// Definitions.

// Priorities assigned to demo application tasks.
#define RADIO_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define RADIO_QUEUE_SIZE		4

#define BUFFER_COUNT			RADIO_QUEUE_SIZE
#define BUFFER_SIZE				122

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
	BufferStorageType		bufferStorage[BUFFER_SIZE];
} RadioBufferStruct;
//typedef RadioBufferStruct *RadioBufferPtrType;

void vRadioTransmitTask( void *pvParameters );
void vRadioReceiveTask( void *pvParameters );

#endif GATEWAYRADIOTASK_H

