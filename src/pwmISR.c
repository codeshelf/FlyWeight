/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/


// Every eight pulses load in a new duty cycle.

#include "FreeRTOS.h"
#include "task.h"
#include "radioCommon.h"

void interrupt audioOutISR( void ) {

	if (TPM2C1V < TPM2CNT) {
		TPM2C1V = TPM2CNT + 0x0564;
	}	
}