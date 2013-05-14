/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef AISLE_CONTROLLER_TASK_H
#define AISLE_CONTROLLER_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define AISLE_CONTROLLER_QUEUE_SIZE		2

#define MAX_LED_FLASH_POSITIONS 		50
#define MAX_LED_SOLID_POSITIONS 		10
#define LED_OFF_TIME					500
#define LED_ON_TIME						250

#define	MAX_DRIFT				500

#define	FSYNC_TIMER				gTmr3_c
#define FSYNC_PRIMARY_SOURCE	gTmrPrimaryClkDiv16_c
#define FSYNC_SECONDARY_SOURCE	gTmrSecondaryCnt1Input_c

#define	RXTX_TIMER				gTmr1_c
#define RXTX_PRIMARY_SOURCE		gTmrPrimaryClkDiv16_c
#define RXTX_SECONDARY_SOURCE	gTmrSecondaryCnt1Input_c

#define SSI_24BIT_WORD			0x0b
#define SSI_20BIT_WORD			0x09
#define SSI_FRAME_LEN2			0x01
#define SSI_FRAME_LEN7			0x06

#define SD_CLK_RATE				400000
#define TMR_CLK_RATE			1500000	/* Bus Clk / Prescaler --> 24,000,000 / 16 */
#define FSYNC_TRIGGER_HIGH		3		/* 1 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 1 * 1,500,000 / 400,000 */
//#define FSYNC_TRIGGER_LOW		1000		/* (was 210) 47 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 47 * 1,500,000 / 400,000 */
//#define FSYNC_SUSTAIN_HIGH		8		/* 8 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 1 * 1,500,000 / 400,000 */
//#define FSYNC_SUSTAIN_LOW		175		/* 47 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 47 * 1,500,000 / 400,000 */
//#define FSYNCR2_SUSTAIN_LOW		1000	/* 5X normal sustain low. */
#define RXTX_TRIGGER_RESYNC		96
#define RXTX_TIMEOUT			2000

extern xQueueHandle gAisleControllerQueue;

typedef union {
	gwUINT32 word;
	struct {
		gwUINT8 byte3;
		gwUINT8 byte2;
		gwUINT8 byte1;
		gwUINT8 unused;
	} bytes;
} USsiSampleType;

// --------------------------------------------------------------------------
// Local functions prototypes.

void aisleControllerTask(void *pvParameters);
void ssiInterrupt(void);
gwUINT32 getNextSolidData(void);
gwUINT32 getNextFlashData(void);
LedDataStruct getNextDataStruct(void);

#endif //AISLECONTROLLER_TASK_H
