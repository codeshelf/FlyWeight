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
#include "version.h"

const char DEVICE_DESC[] = "HooBee";

#define KVP_ENTRIES			2
#define ENDPOINT_ENTRIES	2
#define ENDPOINT_KVPS		2

const UINT8 kActorKVPs = KVP_ENTRIES;
const BufferStoragePtrType kActorKVPTable[KVP_ENTRIES][2] = {
	// Key, value
	{ "hw_version", BOARD_REV },
	{ "sw_version", FIRMWARE_REV }
};

const UINT8 kActorEndpoints = ENDPOINT_ENTRIES;
const BufferStoragePtrType kActorEndpointTable[ENDPOINT_ENTRIES][3] = {
	// Codec type, description, endpoint number
	{ "mood", "Mood Light", "\1" },
	{ "poke", "Poke", "\2" }
};

const UINT8 kEndpointKVPs = ENDPOINT_KVPS;
const BufferStoragePtrType kEndpointKVPTable[ENDPOINT_KVPS][3] = {
	// Endpoint number, key, value
	{ "\1", "mood", "*" },
	{ "\2", "poke", "*" }
};