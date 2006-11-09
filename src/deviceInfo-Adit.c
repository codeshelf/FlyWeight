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

const char DEVICE_DESC[] = "Adit ChatHed";

const UINT8 kActorKVPs = 3;
const BufferStoragePtrType kActorKVPTable[3][2] = {
	// Key, value
	{ "gender", "male" },
	{ "age", "32" },
	{ "race", "south asian" }
};

const UINT8 kActorEndpoints = 2;
const BufferStoragePtrType kActorEndpointTable[2][3] = {
	// Codec type, description, endpoint number
	{ "aduo", "PCM Audio", "\1" },
	{ "leds", "PWM LEDs", "\2" }
};

const UINT8 kEndpointKVPs = 3;
const BufferStoragePtrType kEndpointKVPTable[3][3] = {
	// Endpoint number, key, value
	{ "\1", "voice", "*" },
	{ "\2", "eyes", "*" },
	{ "\2", "color", "red" }
};