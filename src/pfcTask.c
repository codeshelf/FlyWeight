/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "pfcTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "remoteMgmtTask.h"

xTaskHandle			gPFCTask = NULL;
xQueueHandle 		gPFCQueue;

// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {

	if (gPFCQueue) {
		for (;;) {
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
