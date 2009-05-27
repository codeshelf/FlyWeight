/*
FlyWeight
Copyright 2005, 2006 Jeffrey B. Williams
All rights reserved

$Id$	
*/

/**
 * This file holds constant information that is identical for each device of this type.
 * Key and index values should  be all lower case.  Only descriptions should contain upper-case.
 */

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "PE_Types.h"
#include "pub_def.h"
#include "radioCommon.h"

#define KEY_INDEX			0
#define VALUE_INDEX			1

#define EP_TYPE_INDEX		0
#define EP_DESC_INDEX		1
#define EP_COUNT_INDEX		2

#define EPKVP_NUM_INDEX		0
#define EPKVP_KEY_INDEX		1
#define EPKVP_VALUE_INDEX	2

#define ENDPOINT_TYPE_LEN	4

#define KVP_ENTRIES			2
#define ENDPOINT_ENTRIES	2
#define ENDPOINT_KVPS		2

extern char DEVICE_DESC[];

extern const UINT8 kActorKVPs;
extern const BufferStoragePtrType kActorKVPTable[KVP_ENTRIES][2];

extern const UINT8 kActorEndpoints;
extern const BufferStoragePtrType kActorEndpointTable[ENDPOINT_ENTRIES][3];

extern const UINT8 kEndpointKVPs;
extern const BufferStoragePtrType kEndpointKVPTable[ENDPOINT_KVPS][3];

#endif DEVICEINFO_H