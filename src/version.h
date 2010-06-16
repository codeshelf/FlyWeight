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

#ifndef VERSION_H
#define VERSION_H

#define FIRMWARE_REV			"1.3.0"

#if defined(GW0009R1)
	#define BOARD_REV				"GW0009.v1.r01"
#elif defined(GW0009R2)
	#define BOARD_REV				"GW0009.v1.r02"
#elif defined(GW0010R1)
	#define BOARD_REV				"GW0010.v1.r01"
#elif defined(GW0011)
	#define BOARD_REV				"GW0011.v1.r01"
#elif defined(GW0015)
	#define BOARD_REV				"GW0015.v1.r01"
#else
	#define BOARD_REV				"undefined"
#endif

#endif /* VERSION_H */
