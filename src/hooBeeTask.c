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

#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

xTaskHandle			gHooBeeTask = NULL;
xQueueHandle 		gHooBeeQueue;

LedFlashRunType		gLedFlashSequenceShouldRun;
LedFlashSeqCntType	gLedFlashSeqCount;
LedFlashStruct		gLedFlashSeqBuffer[MAX_LED_SEQUENCES];
// --------------------------------------------------------------------------

void hooBeeTask(void *pvParameters) {

	UINT8 i;
	UINT8 j;
	UINT8 k;
	portTickType	offtime;
	portTickType	ontime;

	gLedFlashSequenceShouldRun = FALSE;

#if defined(GW0009R1)
		PTBDD_PTBDD1 = 1;
		PTDDD_PTDDD1 = 1;
		PTDDD_PTDDD0 = 1;
#elif defined(GW0009R2)
		PTBDD_PTBDD2 = 1;
		PTBDD_PTBDD3 = 1;
		PTBDD_PTBDD4 = 1;
#endif

	for (;;) {
		if (gLedFlashSequenceShouldRun) {
			// Make the sequence start on an even 5 seconds.
			//baseTime = (xTaskGetTickCount() % 5000);
			//vTaskDelayUntil(&baseTime, 5000);

			for (i = 0; i < gLedFlashSeqCount; i++) {
				ontime = gLedFlashSeqBuffer[i].timeOnMillis;
				offtime = gLedFlashSeqBuffer[i].timeOffMillis;
				for (j = 0; j < gLedFlashSeqBuffer[i].repeat; j++) {
					for (k = ontime; k > 0; k-=2) {
						TPM1C0V = getMax(0, gLedFlashSeqBuffer[i].redValue - k);
						TPM1C1V = getMax(0, gLedFlashSeqBuffer[i].greenValue - k);
						TPM1C2V = getMax(0, gLedFlashSeqBuffer[i].blueValue - k);
						vTaskDelay(2);
					}
					
					for (k = 0; k < ontime; k+=2) {
						TPM1C0V = getMax(0, gLedFlashSeqBuffer[i].redValue - k);
						TPM1C1V = getMax(0, gLedFlashSeqBuffer[i].greenValue - k);
						TPM1C2V = getMax(0, gLedFlashSeqBuffer[i].blueValue - k);
						vTaskDelay(2);
					}
					
					TPM1C0V = 0;
					TPM1C1V = 0;
					TPM1C2V = 0;
					vTaskDelay(offtime);
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
