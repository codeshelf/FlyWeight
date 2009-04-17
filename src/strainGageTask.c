/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved
 
 $Id$
 $Name$	
 */

#include "strainGageTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"
#include "Watchdog.h"

#define	TEMP		PTBD_PTBD0
#define	TEMP_DIR	PTBDD_PTBDD0
#define	INPSEL		PTBD_PTBD1
#define	INPSEL_DIR	PTBDD_PTBDD1
#define	SPEED		PTBD_PTBD2
#define	SPEED_DIR	PTBDD_PTBDD2
#define	PWRDOWN		PTBD_PTBD3
#define	PWRDOWN_DIR	PTBDD_PTBDD3
#define	SCLK		PTBD_PTBD5
#define	SCLK_DIR	PTBDD_PTBDD5
#define	DOUT		PTBD_PTBD4
#define	DOUT_DIR	PTBDD_PTBDD4

xTaskHandle			gStrainGageTask = NULL;
xQueueHandle 		gStrainGageQueue;

// --------------------------------------------------------------------------

void strainGageTask(void *pvParameters) {
	BufferCntType txBufferNum;
	UINT8 bit;
	UINT32 sample;
	
	// Setup the ADS2032 interface.
	
	// First setup the data direction.
	TEMP_DIR = 1;
	INPSEL_DIR = 1;
	SPEED_DIR = 1;
	PWRDOWN_DIR = 1;
	SCLK_DIR = 1;
	DOUT_DIR = 0;
	
	// Now setup the default interface.
	TEMP = 1;
	INPSEL = 0;
	SPEED = 0;
	PWRDOWN = 0;
	Cpu_Delay100US(1);
	PWRDOWN = 1;
	SCLK = 0;

	if (gStrainGageQueue) {
		for (;;) {

			// Take a measurement.
			while (DOUT != 0) {
#ifdef __WatchDog
				WatchDog_Clear();
#endif
				vTaskDelay(1);
			}
			
			sample = 0;
			for (bit = 0; bit < 24; bit++) {
				// Read the bit from the pin.
				SCLK = 1;
				Cpu_Delay100US(1);
				SCLK = 0;
				sample |= DOUT << bit;
			}
			// Force #DRDY/DOUT high
			SCLK = 1;
			Cpu_Delay100US(1);
			SCLK = 0;
			
			// Transmit the measurement.
			
			
			// Delay until the next measurement.
			vTaskDelay(500);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}