/*
FlyWeight
� Copyright 2005, 2006 Jeffrey B. Williams
All rights reserved

$Id$
$Name$	
*/

#ifndef TOYQUERY_H
#define TOYQUERY_H

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

#define RPOS_RESPONSEKIND					0
#define RPOS_QUERYID						1
#define RPOS_RESPONSE_SIZE					9
#define RPOS_RESPONSE						10

#define QUERY_ACTOR_DESCRIPTOR				1
#define QUERY_ACTOR_KVP_DESCRIPTOR			2
#define QUERY_ENDPOINT_DESCRIPTOR			3
#define QUERY_ENDPOINT_KVP_DESCRIPTOR		4

#define RESPONSE_ACTOR_DESCRIPTOR			1
#define RESPONSE_ACTOR_KVP_DESCRIPTOR		2
#define RESPONSE_ENDPOINT_DESCRIPTOR		3
#define RESPONSE_ENDPOINT_KVP_DESCRIPTOR	4

// --------------------------------------------------------------------------
// Typedefs.
typedef byte QueryIDType[QUERYID_SIZE];

// --------------------------------------------------------------------------
// Function prototypes.
void processQuery(BufferStoragePtrType inQueryPtr, RemoteAddrType inSrcAddr);
void processQueryActorDescriptor(BufferStoragePtrType inResponseBuffer, BufferCntType inResponseBufferSize, RemoteAddrType inSrcAddr);

// --------------------------------------------------------------------------
// Globals.

extern unsigned char ResponseStr[MAX_RESPONSE_SIZE];

#endif TOYQUERY_H
