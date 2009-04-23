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

#define	TEMP		PTBD_PTBD0
#define	TEMP_DIR	PTBDD_PTBDD0
#define	INPSEL		PTBD_PTBD1
#define	INPSEL_DIR	PTBDD_PTBDD1
#define	SPEED		PTBD_PTBD2
#define	SPEED_DIR	PTBDD_PTBDD2
#define	PWRDOWN		PTBD_PTBD3
#define	PWRDOWN_DIR	PTBDD_PTBDD3
#define	DOUT		PTBD_PTBD4
#define	DOUT_DIR	PTBDD_PTBDD4
#define	SCLK		PTBD_PTBD5
#define	SCLK_DIR	PTBDD_PTBDD5
#define	GAIN1		PTBD_PTBD6
#define	GAIN1_DIR	PTBDD_PTBDD6

xTaskHandle			gStrainGageTask = NULL;
xQueueHandle 		gStrainGageQueue;

// --------------------------------------------------------------------------

DataSampleType collectSample() {
	DataSampleType	result;
	UINT8			bit;
	
	// Take a measurement.
	while (DOUT != 0) {
		WatchDog_Clear();
		vTaskDelay(1);
	}
	
	result = 0;
	// 24 bits read, MSB-first.
	for (bit = 0; bit < 24; bit++) {
		// Read the bit from the pin.
		SCLK = 1;
		Cpu_Delay100US(1);
		SCLK = 0;
		result <<= 1;
		result |= DOUT;
	}
	// Force #DRDY/DOUT high
	SCLK = 1;
	Cpu_Delay100US(1);
	SCLK = 0;
	
	return result;
}

// --------------------------------------------------------------------------

void strainGageTask(void *pvParameters) {
	DataSampleType	sample;
	EndpointNumType scaleEndpoint = 1;
	EndpointNumType tempEndpoint = 2;
	
	// Setup the ADS2032 interface.
	
	// First setup the data direction.
	TEMP_DIR = 1;
	INPSEL_DIR = 1;
	SPEED_DIR = 1;
	PWRDOWN_DIR = 1;
	SCLK_DIR = 1;
	DOUT_DIR = 0;
	GAIN1_DIR = 1;
	
	// Now setup the default interface.
	TEMP = 0;
	INPSEL = 0;
	SPEED = 0;
	PWRDOWN = 0;
	Cpu_Delay100US(1);
	PWRDOWN = 1;
	SCLK = 0;
	GAIN1 = 1;

	if (gStrainGageQueue) {
		for (;;) {

			// Capture strain: TEMP = off, Gain = 128x.
			GAIN1 = 1;
			TEMP = 0;
			
			// It takes at least 4 samples to settle the inputs after switching mode.
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			
			// Transmit the measurement.
			createDataSampleCommand(gTXCurBufferNum, scaleEndpoint);
			addDataSampleToCommand(gTXCurBufferNum, xTaskGetTickCount(), sample);
			if (transmitPacket(gTXCurBufferNum)){
			};
			
			// Capture temp: TEMP = on, Gain = 2x.
			GAIN1 = 0;
			TEMP = 1;
			
			// It takes at least 4 samples to settle the inputs after switching mode.
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();

			// Transmit the measurement.
			createDataSampleCommand(gTXCurBufferNum, tempEndpoint);
			addDataSampleToCommand(gTXCurBufferNum, xTaskGetTickCount(), sample);
			if (transmitPacket(gTXCurBufferNum)){
			};	
			
			// Delay until the next measurement.
			vTaskDelay(2000);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}