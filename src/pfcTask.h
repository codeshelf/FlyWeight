/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef PFC_TASK_H
#define PFC_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define PFC_QUEUE_SIZE		2

extern xQueueHandle	gPFCQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void pfcTask( void *pvParameters );

#endif //PFC_TASK_H
