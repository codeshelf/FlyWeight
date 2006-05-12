/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved
   
   $Id$
   $Name$	
*/

#include "radioCommon.h"
#include "cpu.h"

// --------------------------------------------------------------------------

void advanceRXBuffer() {

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	EnterCritical();
	
		gRXRadioBuffer[gRXCurBufferNum].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gRXCurBufferNum++;
		if (gRXCurBufferNum >= (RX_BUFFER_COUNT))
			gRXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		if (gRXUsedBuffers < RX_BUFFER_COUNT)
			gRXUsedBuffers++;
		
	ExitCritical();
}

// --------------------------------------------------------------------------

void advanceTXBuffer() {

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	EnterCritical();
	
		gTXRadioBuffer[gTXCurBufferNum].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gTXCurBufferNum++;
		if (gTXCurBufferNum >= (TX_BUFFER_COUNT))
			gTXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		if (gTXUsedBuffers < TX_BUFFER_COUNT)
			gTXUsedBuffers++;
		
	ExitCritical();
}

