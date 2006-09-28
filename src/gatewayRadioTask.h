/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef GATEWAYRADIOTASK_H
#define GATEWAYRADIOTASK_H

// Project includes
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Externs

extern RemoteAddrType	gMainRemote;

// --------------------------------------------------------------------------
// Function prototypes.

void radioReceiveTask(void *pvParameters);
void radioTransmitTask(void *pvParameters);

#endif GATEWAYRADIOTASK_H

