/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef HOOBEE_TASK_H
#define HOOBEE_TASK_H

#include "pub_def.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define HOOBEE_QUEUE_SIZE		2


extern xQueueHandle	gHooBeeQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void hooBeeTask( void *pvParameters );

#endif //HOOBEE_TASK_H
