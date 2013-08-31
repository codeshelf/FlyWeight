/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef GATEWAYMGMTTASK_H
#define GATEWAYMGMTTASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "UART.h"
#include "radioCommon.h"

extern xQueueHandle	gGatewayMgmtQueue;

// --------------------------------------------------------------------------
// Functions prototypes.

void gatewayMgmtTask(void *pvParameters);

#endif //GATEWAYMGMTTASK_H
