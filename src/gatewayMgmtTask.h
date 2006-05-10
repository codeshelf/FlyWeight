/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef GATEWAYMGMTTASK_H
#define GATEWAYMGMTTASK_H

#include "pub_def.h"
#include "FreeRTOS.h"
#include "queue.h"

#define GATEWAY_MGMT_QUEUE_SIZE		10

extern xQueueHandle	gGatewayMgmtQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void gatewayMgmtTask( void *pvParameters );

#endif GATEWAYMGMTTASK_H
