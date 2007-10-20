/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "smacGlue.h"
#include "radioCommon.h"
//#include "USB.h"

extern xQueueHandle	gRadioReceiveQueue;
ERadioState			gRadioState = eRadioReceive;

void initSMACRadioQueueGlue(xQueueHandle inRadioReceiveQueue) {
	gRadioReceiveQueue = inRadioReceiveQueue;
};

// --------------------------------------------------------------------------
// The SMAC calls MCPSDataIndication() during an ISR when it receives data from the radio.
// We intercept that here, and patch the message over to the radio task's queue.
// In FreeRTOS we can't RTI or task switch during and ISR, so we need to keep this
// short and sweet.  (And let a swapped-in task deal with this during a context switch.)

void MCPSDataIndication(tRxPacket *gsRxPacket) {

    if (gsRxPacket->u8Status == SUCCESS) {

		// If we haven't initialized the radio receive queue then cause a debug trap.
		if (gRadioReceiveQueue == NULL)
			__asm ("BGND");
			
		// Set the buffer receive size to the size of the packet received.
		gRXRadioBuffer[gRXCurBufferNum].bufferSize = gsRxPacket->u8DataLength;
	
		// Send the message to the radio task's queue.
		if (xQueueSendFromISR(gRadioReceiveQueue, &gRXCurBufferNum, (portTickType) 0)) {
		}
		
		advanceRXBuffer();
		
	}
};


// --------------------------------------------------------------------------

// The SMAC calls MLMEMC13192ResetIndication() during an ISR when it can't receive data from the radio.
// We intercept that here, and patch the message over to the radio task's queue.
// In FreeRTOS we can't RTI or task switch during and ISR, so we need to keep this
// short and sweet.  (And let a swapped-in task deal with this during a context switch.)

void MLMEMC13192ResetIndication() {
	// If we haven't initialized the radio receive queue then cause a debug trap.
	//if (gRadioReceiveQueue == NULL)
	//	__asm ("BGND");
	
	// Set the state of the radio at this time.  (The value gets copied into the msg.)
	gRadioState = eRadioReset;

	// Send the message to the radio task's queue.
	//if ( xQueueSendFromISR(gRadioReceiveQueue, &gRadioState, (portTickType) 0) ) {
	//}
};
