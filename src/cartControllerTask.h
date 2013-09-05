/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef CART_CONTROLLER_TASK_H
#define CART_CONTROLLER_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"
#include "UartLowLevel.h"

#define CART_CONTROLLER_QUEUE_SIZE		2

#define MAX_LED_FLASH_POSITIONS 		50
#define MAX_LED_SOLID_POSITIONS 		10
#define LED_OFF_TIME					500
#define LED_ON_TIME						250

#define	MAX_DRIFT						500

#define SSI_24BIT_WORD					0x0b
#define SSI_20BIT_WORD					0x09
#define SSI_8BIT_WORD					0x03
#define SSI_FRAME_LEN2					0x01
#define SSI_FRAME_LEN7					0x06

#define SD_CLK_RATE						400000
#define TMR_CLK_RATE					1500000	/* Bus Clk / Prescaler --> 24,000,000 / 16 */

extern xQueueHandle gCartControllerQueue;

typedef union {
	gwUINT32 word;
	struct {
		gwUINT8 byte3;
		gwUINT8 byte2;
		gwUINT8 byte1;
		gwUINT8 byte0;
	} bytes;
} ULedSampleType;

// --------------------------------------------------------------------------
// Local functions prototypes.

void cartControllerTask(void *pvParameters);
void displayScrollCallback(TmrNumber_t tmrNumber);
void UartReadCallback(UartReadCallbackArgs_t* args);
void UartWriteCallback(UartWriteCallbackArgs_t* args);


#endif //CART_CONTROLLER_TASK_H
