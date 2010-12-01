/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved
   
   $Id$
   $Name$	
*/

#include "radioCommon.h"

// Radio input buffer
// There's a 2-byte ID on the front of every packet.
RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gRXCurBufferNum = 0;
BufferCntType		gRXUsedBuffers = 0;

RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
BufferCntType		gTXCurBufferNum = 0;
BufferCntType		gTXUsedBuffers = 0;

// --------------------------------------------------------------------------

void advanceRXBuffer() {

	gwUINT8 ccrHolder;
	
	// The buffers are a shared, critical resource, so we have to protect them before we update.
	EnterCriticalArg(ccrHolder);
	
		gRXRadioBuffer[gRXCurBufferNum].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gRXCurBufferNum++;
		if (gRXCurBufferNum >= (RX_BUFFER_COUNT))
			gRXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		if (gRXUsedBuffers < RX_BUFFER_COUNT)
			gRXUsedBuffers++;
		
	ExitCriticalArg(ccrHolder);
}

BufferCntType lockRXBuffer() {

	BufferCntType result;
	gwUINT8 ccrHolder;
	
	// Wait until there is a free buffer.
	while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1);
		GW_WATCHDOG_RESET;
	}

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	GW_ENTER_CRITICAL(ccrHolder);
	
	result = gRXCurBufferNum;

	gRXRadioBuffer[result].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gRXCurBufferNum++;
		if (gRXCurBufferNum >= (RX_BUFFER_COUNT))
			gRXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		gRXUsedBuffers++;
		
	GW_EXIT_CRITICAL(ccrHolder);

	return result;
}

// --------------------------------------------------------------------------

BufferCntType lockTXBuffer() {

	BufferCntType result;
	gwUINT8 ccrHolder;
	
	// Wait until there is a free buffer.
	while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1);
		GW_WATCHDOG_RESET;
	}

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	GW_ENTER_CRITICAL(ccrHolder);
	
		result = gTXCurBufferNum;
		gTXRadioBuffer[result].bufferStatus = eBufferStateInUse;
		
		// Advance to the next buffer.
		gTXCurBufferNum++;
		if (gTXCurBufferNum >= (TX_BUFFER_COUNT))
			gTXCurBufferNum = 0;
		
		// Account for the number of used buffers.
		gTXUsedBuffers++;
		
	GW_EXIT_CRITICAL(ccrHolder);

	return result;
}

// --------------------------------------------------------------------------

void setupWatchdog() {
#if defined(MC1322X)
	// Setup the COP to interrupt (so that we can catch them and figure out where they come  from.)
	crmCopCntl_t copCntl;
	copCntl.bit.copEn = TRUE;
	copCntl.bit.copTimeOut = 20;
	copCntl.bit.copWP = TRUE;
#if (GW_DEBUG)
	copCntl.bit.copOut = 1;		// 1 = CRM interrupt
#else
	copCntl.bit.copOut = 0;		// 0 = MCU reset
#endif
	CRM_CopCntl(copCntl);

#if (GW_DEBUG)
	IntAssignHandler(gCrmInt_c, (IntHandlerFunc_t) CRM_Isr);
	ITC_SetPriority(gCrmInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gCrmInt_c);
	CRM_RegisterISR(gCrmCOPTimeoutEvent_c, debugCrmCallback);
#endif
#endif
}

// --------------------------------------------------------------------------

void debugReset() {
#if defined(MC1322X)
	// Reset the MCU - this is just a place to set a breakpoint to catch it.
	CRM_SoftReset();
#else
	__asm ("BGND")
#endif
}


// --------------------------------------------------------------------------

void debugCrmCallback(void) {
#if defined(MC1322X)
	// Reset the MCU - this is just a place to set a breakpoint to catch it.
	CRM_SoftReset();
	//gTXUsedBuffers = 0;
#endif
}
// --------------------------------------------------------------------------

void debugRefreed(BufferCntType inBufferNum) {
	// Any code, so that we have a place to set a breakpoint.
	gTXRadioBuffer[inBufferNum].bufferStatus = eBufferStateFree;
}
