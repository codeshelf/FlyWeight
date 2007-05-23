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

#include "PE_Types.h"
#include "pub_def.h"
#include "radioCommon.h"

const char DEVICE_DESC[] = "Adit ChatHead";

#define KVP_ENTRIES			3
#define ENDPOINT_ENTRIES	3
#define ENDPOINT_KVPS		4

const UINT8 kActorKVPs = KVP_ENTRIES;
const BufferStoragePtrType kActorKVPTable[KVP_ENTRIES][2] = {
	// Key, value
	{ "gender", "male" },
	{ "age", "32" },
	{ "race", "south asian" }
};

const UINT8 kActorEndpoints = ENDPOINT_ENTRIES;
const BufferStoragePtrType kActorEndpointTable[ENDPOINT_ENTRIES][3] = {
	// Codec type, description, endpoint number
	{ "ulaw", "ULaw Audio", "\1" },
	{ "motr", "Eyes/Ears Motor", "\2" },
	{ "motr", "Feet/Mouth Motor", "\3" }
};

const UINT8 kEndpointKVPs = ENDPOINT_KVPS;
const BufferStoragePtrType kEndpointKVPTable[ENDPOINT_KVPS][3] = {
	// Endpoint number, key, value
	{ "\1", "voice", "*" },
	{ "\2", "eyes", "*" },
	{ "\2", "color", "brown" },
	{ "\3", "mouth", "*" }
};