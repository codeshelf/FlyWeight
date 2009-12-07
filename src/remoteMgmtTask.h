/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef REMOTEMGMTTASK_H
#define REMOTEMGMTTASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

extern xQueueHandle				gRemoteMgmtQueue;
extern NetAddrType				gMyAddr;
extern NetworkIDType			gMyNetworkID;

// --------------------------------------------------------------------------
// Local functions prototypes.

void remoteMgmtTask( void *pvParameters );
void sleepThisRemote( gwUINT8 inSleepMillis );

#endif /* REMOTEMGMTTASK_H */
