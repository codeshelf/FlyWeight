/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "smacGlue.h"

xQueueHandle	gRadioReceiveQueue = NULL;
ERadioState		gRadioState = eRadioReceive;

void initSMACRadioQueueGlue(xQueueHandle inRadioReceiveQueue) {
	gRadioReceiveQueue = inRadioReceiveQueue;
};

// --------------------------------------------------------------------------
// The SMAC calls MCPSDataIndication() during an ISR when it receives data from the radio.
// We intercept that here, and patch the message over to the radio task's queue.
// In FreeRTOS we can't RTI or task switch during and ISR, so we need to keep this
// short and sweet.  (And let a swapped-in task deal with this during a context switch.)

#pragma CODE_SEG __NEAR_SEG NON_BANKED 
void MCPSDataIndication(tRxPacket *gsRxPacket) {

	// If we haven't initialized the radio receive queue then cause a debug trap.
	if (gRadioReceiveQueue == NULL)
		__asm ("BGND");
	
	// Set the state of the radio at this time.  (The value gets copied into the msg.)
	gRadioState = eRadioReceive;
	
	// Send the message to the radio task's queue.
	if ( xQueueSendFromISR(gRadioReceiveQueue, &gRadioState, pdFALSE) ) {
	}
};

#pragma CODE_SEG DEFAULT

// --------------------------------------------------------------------------

// The SMAC calls MLMEMC13192ResetIndication() during an ISR when it can't receive data from the radio.
// We intercept that here, and patch the message over to the radio task's queue.
// In FreeRTOS we can't RTI or task switch during and ISR, so we need to keep this
// short and sweet.  (And let a swapped-in task deal with this during a context switch.)

void MLMEMC13192ResetIndication() {
	// If we haven't initialized the radio receive queue then cause a debug trap.
	if (gRadioReceiveQueue == NULL)
		__asm ("BGND");
	
	// Set the state of the radio at this time.  (The value gets copied into the msg.)
	gRadioState = eRadioReset;

	// Send the message to the radio task's queue.
	if ( xQueueSendFromISR(gRadioReceiveQueue, &gRadioState, pdFALSE) ) {
	}
};
