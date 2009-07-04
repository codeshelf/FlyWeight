/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "hooBeeTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"

xTaskHandle			gHooBeeTask = NULL;
xQueueHandle 		gHooBeeQueue;

UINT8	gRedValue;
UINT8	gGreenValue;
UINT8	gBlueValue;
UINT16	gTimeOnMillis;
UINT16	gTimeOffMillis;
UINT8	gRepeat;

// --------------------------------------------------------------------------

void hooBeeTask(void *pvParameters) {

	UINT8 i;

//	if (gHooBeeQueue) {
		for (;;) {
			for (i = 0; i < gRepeat; i++) {

				TPM1C2V = gRedValue;
				TPM1C1V = gGreenValue;
				TPM1C0V = gBlueValue;

				vTaskDelay(gTimeOnMillis);

				TPM1C2V = 0;
				TPM1C1V = 0;
				TPM1C0V = 0;

				vTaskDelay(gTimeOffMillis);
			}

			WATCHDOG_RESET;

			// Delay for the whole cycle.
			vTaskDelay(5000 * portTICK_RATE_MS);
		}
//	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
