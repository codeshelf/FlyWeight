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

#define ENDPOINT_TYPE_LEN	4

extern char DEVICE_DESC[];

extern const UINT8 ACTOR_KVPS;
extern const BufferStoragePtrType kActorKVPs[3][2];

extern const UINT8 ACTOR_ENDPOINTS;
extern const BufferStoragePtrType kActorEndpoints[2][3];

extern const BufferStoragePtrType kEndpointKVPs[3][3];

#endif DEVICEINFO_H