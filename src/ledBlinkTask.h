/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef LEDBLINKTASK_H
#define LEDBLINKTASK_H

#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "LED4.h"

// --------------------------------------------------------------------------
// Local functions prototypes.

void vLEDBlinkTask( void *pvParameters );
void LEDOn(int inLED);
void LEDOff(int inLED);

#endif LEDBLINKTASK_H
