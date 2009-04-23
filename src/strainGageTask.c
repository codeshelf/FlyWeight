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

#define getOneByteOfSample(tempVar) \
\
		tempVar = 0; \
		for (bitNum = 0; bitNum < 8; bitNum++) {\
			/* Trigger the DOUT pin. */ \
			SCLK = 1;\
			__asm("NOP");\
			SCLK = 0;\
			/* Cpu_Delay100US(1); */ \
			/* Read the DOUT pin */ \
			tempVar = (tempVar << 1) + DOUT;\
		}


xTaskHandle			gStrainGageTask = NULL;
xQueueHandle 		gStrainGageQueue;
UINT8				gSampleCount;

// --------------------------------------------------------------------------

DataSampleType collectSample() {
	DataSampleType	result;
	UINT8			temp1;
	UINT8			temp2;
	UINT8			temp3;
	UINT8			bitNum;
	
	// Take a measurement.
	while (DOUT != 0) {
		WatchDog_Clear();
		vTaskDelay(1);
	}
	
	// 24 bits read, MSB-first.
	getOneByteOfSample(temp1);
	getOneByteOfSample(temp2);
	getOneByteOfSample(temp3);
	
	// Force #DRDY/DOUT high
	SCLK = 1;
	__asm("NOP");
	//Cpu_Delay100US(1);
	SCLK = 0;
	
	result = temp1;
	result = (result << 8) + temp2;
	result = (result << 8) + temp3;

	gSampleCount++;

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
	INPSEL = 0;
	SPEED = 0;
	PWRDOWN = 0;
	Cpu_Delay100US(1);
	PWRDOWN = 1;
	SCLK = 0;
	
	// Set for temp
	GAIN1 = 0;
	TEMP = 1;

	if (gStrainGageQueue) {
		for (;;) {

			// It takes at least 4 samples to settle the inputs after switching mode.
			/*sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();*/
			sample = collectSample();			
			
			// Transmit the measurement.
			createDataSampleCommand(gTXCurBufferNum, scaleEndpoint);
			addDataSampleToCommand(gTXCurBufferNum, xTaskGetTickCount(), sample);
			if (transmitPacket(gTXCurBufferNum)){
			};
			
			if (gSampleCount >= 10) {
				// Capture temp: TEMP = on, Gain = 2x.
				GAIN1 = 1;
				TEMP = 0;
				
				// It takes at least 4 samples to settle the inputs after switching mode.
				/*sample = collectSample();
				sample = collectSample();
				sample = collectSample();
				sample = collectSample();*/
				vTaskDelay(500);
				sample = collectSample();

				// Transmit the measurement.
				createDataSampleCommand(gTXCurBufferNum, tempEndpoint);
				addDataSampleToCommand(gTXCurBufferNum, xTaskGetTickCount(), sample);
				if (transmitPacket(gTXCurBufferNum)){
				};

				// Capture strain: TEMP = off, Gain = 128x.
				GAIN1 = 0;
				TEMP = 1;
			
				gSampleCount = 0;	
			}
			
			// Delay until the next measurement.
			vTaskDelay(1000);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}