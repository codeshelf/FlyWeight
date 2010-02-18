/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "pfcSPITask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "remoteMgmtTask.h"
#include "spi.h"

// Globals

xTaskHandle gPFCTask = NULL;
xQueueHandle gPFCQueue;
ESDCardState gSDCardState;
gwUINT8 gReadBlock[SD_BLOCK_SIZE];

extern portTickType xTickCount;

// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {

	ESDCardResponse result;

	if (gPFCQueue) {

//		enableSPI();
		gSDCardState = eSDCardStateReady;

//		result = readBlock(0, gReadBlock);
//		result = readBlock(1, gReadBlock);
//		result = readBlock(2, gReadBlock);
//		result = readBlock(20, gReadBlock);
//		result = readBlock(21, gReadBlock);
//		result = readBlock(22, gReadBlock);

		for (;;) {
			vTaskDelay(10);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
