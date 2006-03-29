/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef SMACGLUE_H
#define SMACGLUE_H

//#include <stdlib.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "queue.h"
#include "pub_def.h"

typedef enum { 
	eRadioUnknown, 
	eRadioReceive, 
	eRadioReset
} ERadioState;

typedef ERadioState *ERadioStatePtrType;

void initSMACRadioQueueGlue(xQueueHandle inRadioReceiveQueue);

#endif // SMACGLUE_H
