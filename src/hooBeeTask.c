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

LedFlashSeqCntType	gLedFlashSeqCount;
LedFlashStruct		gLedFlashSeqBuffer[MAX_LED_SEQUENCES];
// --------------------------------------------------------------------------

void hooBeeTask(void *pvParameters) {

	UINT8 i;
	UINT8 j;

//	if (gHooBeeQueue) {
		for (;;) {
			for (j = 0; j < gLedFlashSeqCount; j++) {
				
				for (i = 0; i < gLedFlashSeqBuffer[j].repeat; i++) {

					TPM1C2V = gLedFlashSeqBuffer[j].redValue;
					TPM1C1V = gLedFlashSeqBuffer[j].greenValue;
					TPM1C0V = gLedFlashSeqBuffer[j].blueValue;

					vTaskDelay(gLedFlashSeqBuffer[j].timeOnMillis);

					TPM1C2V = 0;
					TPM1C1V = 0;
					TPM1C0V = 0;

					vTaskDelay(gLedFlashSeqBuffer[j].timeOffMillis);
				}
			}
			
			// Delay for the whole cycle.
			vTaskDelay(5000 * portTICK_RATE_MS);
		}
//	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
