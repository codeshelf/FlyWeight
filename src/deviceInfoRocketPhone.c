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

#include "gwTypes.h"
#include "radioCommon.h"

const char DEVICE_DESC[] = "RocketPhone";

#define KVP_ENTRIES			2
#define ENDPOINT_ENTRIES	1
#define ENDPOINT_KVPS		1

const gwUINT8 kActorKVPs = KVP_ENTRIES;
const BufferStoragePtrType kActorKVPTable[KVP_ENTRIES][2] = {
	// Key, value
	{ "hw_version", "gw0005.r2" },
	{ "sw_version", "1.0" }
};

const gwUINT8 kActorEndpoints = ENDPOINT_ENTRIES;
const BufferStoragePtrType kActorEndpointTable[ENDPOINT_ENTRIES][3] = {
	// Codec type, description, endpoint number
	{ "ulaw", "ULaw Audio", "\1" }
};

const gwUINT8 kEndpointKVPs = ENDPOINT_KVPS;
const BufferStoragePtrType kEndpointKVPTable[ENDPOINT_KVPS][3] = {
	// Endpoint number, key, value
	{ "\1", "voice", "*" }
};