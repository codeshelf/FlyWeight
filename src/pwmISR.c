/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/


// Every eight pulses load in a new duty cycle.

int gPulseNum = 0;

void interrupt pwmISR( void )
{
	// It's OK if the variable overflows - we just want to get every 8th pulse.
	if (gPulseNum++ % 8 == 0) {
	
	}
}
