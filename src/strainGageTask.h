/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef STRAINGAGE_TASK_H
#define STRAINGAGE_TASK_H

#include "pub_def.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define STRAINGAGE_QUEUE_SIZE		2

extern xQueueHandle	gStrainGageQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

DataSampleType collectSample();
void strainGageTask( void *pvParameters );

#endif //STRAINGAGE_TASK_H
