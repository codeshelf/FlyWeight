/*
FlyWeight
© Copyright 2005, 2006 Jeffrey B. Williams
All rights reserved

$Id$
$Name$	
*/

#ifndef DEVICEQUERY_H
#define DEVICEQUERY_H

// Project includes
#include "PE_Types.h"
#include "pub_def.h"
#include "radioCommon.h"
#include "commandTypes.h"

// --------------------------------------------------------------------------
// Definitions.

#define MAX_RESPONSE_SIZE					80

#define QUERY_HDR_SIZE						1
#define QUERYID_SIZE						8

#define QPOS_QUERYKIND						0
#define QPOS_QUERYID						1
#define QPOS_KVPNUM							9
#define QPOS_KVPENDPOINT_NUM				10

#define QPOS_ENDPOINT_NUM					9

#define RPOS_RESPONSEKIND					0
#define RPOS_QUERYID						1
#define RPOS_RESPONSE						9

#define QUERY_ACTOR_DESCRIPTOR				1
#define QUERY_ACTOR_KVP						2
#define QUERY_ENDPOINT_DESCRIPTOR			3
#define QUERY_ENDPOINT_KVP					4

#define RESPONSE_ACTOR_DESCRIPTOR			1
#define RESPONSE_ACTOR_KVP					2
#define RESPONSE_ENDPOINT_DESCRIPTOR		3
#define RESPONSE_ENDPOINT_KVP				4

#ifdef CHATHEAD_DEVICE
#define REMOTE_DEVICE_TYPE 					1
#else
#define REMOTE_DEVICE_TYPE					2
#endif

// --------------------------------------------------------------------------
// Typedefs.
typedef byte QueryIDType[QUERYID_SIZE];

// --------------------------------------------------------------------------
// Function prototypes.
void processQuery(BufferCntType inRXBufferNum, BufferOffsetType inStartOfQuery, NetAddrType inSrcAddr);
BufferOffsetType processQueryActorDescriptor(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr);
BufferOffsetType processQueryActorKVP(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr);	
BufferOffsetType processQueryEndpointDescriptor(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr);	
BufferOffsetType processQueryEndpointKVP(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr);	
// --------------------------------------------------------------------------
// Globals.

#endif DEVICEQUERY_H
