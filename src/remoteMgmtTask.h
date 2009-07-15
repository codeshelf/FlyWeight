/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef REMOTEMGMTTASK_H
#define REMOTEMGMTTASK_H

#include "pub_def.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define GATEWAY_MGMT_QUEUE_SIZE		10

extern xQueueHandle				gRemoteMgmtQueue;
extern RemoteAddrType			gMyAddr;
extern NetworkIDType			gMyNetworkID;

// --------------------------------------------------------------------------
// Local functions prototypes.

void remoteMgmtTask( void *pvParameters );
void sleepThisRemote( UINT8 inSleepMillis );

#endif REMOTEMGMTTASK_H
