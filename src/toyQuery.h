/*
FlyWeight
© Copyright 2005, 2006 Jeffrey B. Williams
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

#define QUERY_HDR_SIZE					1

#define QPOS_QUERYID					0

#define QUERY_ACTOR_DESCRIPTOR			1
#define QUERY_ACTOR_KVP_DESCRIPTOR		2
#define QUERY_ENDPOINT_DESCRIPTOR		3
#define QUERY_ENDPOINT_KVP_DESCRIPTOR	4

// --------------------------------------------------------------------------
// Typedefs.


// --------------------------------------------------------------------------
// Function prototypes.
void processQuery(BufferStoragePtrType inQueryPtr, BufferOffsetType inQueryBytes);
void processQueryActorDescriptor(BufferStoragePtrType inQueryPtr, BufferOffsetType inQueryBytes);

// --------------------------------------------------------------------------
// Globals.


#endif TOYQUERY_H
