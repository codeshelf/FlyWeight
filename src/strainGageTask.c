/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
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
	
	// If the 24th bit is 1 then we need to sign-extend to all 32 bits.
	result = 0;
	if (temp1 & 0b10000000) {
		result = 0xff;	
	}

	result = (result << 8) + temp1;
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
	
	SETUP_STRAIN;

	if (gStrainGageQueue) {
		for (;;) {

			// It takes at least 4 samples to settle the inputs after switching mode.
			/*sample = collectSample();
			sample = collectSample();
			sample = collectSample();
			sample = collectSample();*/
			sample = collectSample();			
			
			// Transmit the measurement.
			BufferCntType txBufferNum = lockTXBuffer();
			createDataSampleCommand(txBufferNum, scaleEndpoint);
			addDataSampleToCommand(txBufferNum, xTaskGetTickCount(), sample, 'R');
			if (transmitPacket(txBufferNum)){
			};
			
			if (/*FALSE*/ gSampleCount >= 10) {
				// Capture temp: TEMP = on, Gain = 2x.
				SETUP_TEMP;
				sample = collectSample();

				// Transmit the measurement.
				BufferCntType txBufferNum = lockTXBuffer();
				createDataSampleCommand(txBufferNum, tempEndpoint);
				addDataSampleToCommand(txBufferNum, xTaskGetTickCount(), sample, 'R');
				if (transmitPacket(txBufferNum)){
				};

				// Capture strain: TEMP = off, Gain = 128x.
				SETUP_STRAIN;
			
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
