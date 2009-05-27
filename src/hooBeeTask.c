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

#define	TEMP			PTBD_PTBD0
#define	TEMP_DIR		PTBDD_PTBDD0
#define	INPSEL			PTBD_PTBD1
#define	INPSEL_DIR		PTBDD_PTBDD1
#define	SPEED			PTBD_PTBD2
#define	SPEED_DIR		PTBDD_PTBDD2
#define	PWRDOWN			PTBD_PTBD3
#define	PWRDOWN_DIR		PTBDD_PTBDD3
#define	DOUT			PTBD_PTBD4
#define	DOUT_DIR		PTBDD_PTBDD4
#define	SCLK			PTBD_PTBD5
#define	SCLK_DIR		PTBDD_PTBDD5
#define	GAIN1			PTBD_PTBD6
#define	GAIN1_DIR		PTBDD_PTBDD6

#define SETUP_STRAIN	GAIN1 = 1; TEMP = 0; vTaskDelay(600);
#define SETUP_TEMP		GAIN1 = 0; TEMP = 1; vTaskDelay(600);

xTaskHandle			gHooBeeTask = NULL;
xQueueHandle 		gHooBeeQueue;

// --------------------------------------------------------------------------

void hooBeeTask(void *pvParameters) {

	if (gHooBeeQueue) {
		for (;;) {

		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}