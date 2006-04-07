/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef LEDBLINKTASK_H
#define LEDBLINKTASK_H

#include "pub_def.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "LED4.h"

#define LED_BLINK_QUEUE_SIZE		10

extern UINT8		gLED1;
extern UINT8		gLED2;
extern UINT8		gLED3;
extern UINT8		gLED4;
extern xQueueHandle	xLEDBlinkQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void vLEDBlinkTask( void *pvParameters );
void LEDOn(int inLED);
void LEDOff(int inLED);

#endif LEDBLINKTASK_H
