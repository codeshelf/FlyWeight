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
#include "remoteMgmtTask.h"

xTaskHandle			gHooBeeTask = NULL;
xQueueHandle 		gHooBeeQueue;

LedFlashRunType		gLedFlashSequenceShouldRun;
LedFlashSeqCntType	gLedFlashSeqCount;
LedFlashStruct		gLedFlashSeqBuffer[MAX_LED_SEQUENCES];
// --------------------------------------------------------------------------

void hooBeeTask(void *pvParameters) {

	UINT8 i;
	UINT8 j;
	portTickType	baseTime;

	gLedFlashSequenceShouldRun = FALSE;

#if defined(HW_GW0009R1)
		PTBDD_PTBDD1 = 1;
		PTDDD_PTDDD1 = 1;
		PTDDD_PTDDD0 = 1;
#elif defined(HW_GW0009R2)
		PTBDD_PTBDD2 = 1;
		PTBDD_PTBDD3 = 1;
		PTBDD_PTBDD4 = 1;
#endif

	for (;;) {
		if (gLedFlashSequenceShouldRun) {
			// Make the sequence start on an even 5 seconds.
			baseTime = (xTaskGetTickCount() % 5000);
			vTaskDelayUntil(&baseTime, 5000);

			for (j = 0; j < gLedFlashSeqCount; j++) {
				for (i = 0; i < gLedFlashSeqBuffer[j].repeat; i++) {

						TPM1C0V = gLedFlashSeqBuffer[j].redValue;
						TPM1C1V = gLedFlashSeqBuffer[j].greenValue;
						TPM1C2V = gLedFlashSeqBuffer[j].blueValue;

						vTaskDelay(gLedFlashSeqBuffer[j].timeOnMillis);

						TPM1C0V = 0;
						TPM1C1V = 0;
						TPM1C2V = 0;

						vTaskDelay(gLedFlashSeqBuffer[j].timeOffMillis);
				}
			}
		} else {
			vTaskDelay(50);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
