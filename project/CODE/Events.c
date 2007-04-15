/* MODULE Events */


/*Including used modules for compilling procedure*/
#include "Cpu.h"
#include "Events.h"

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "queue.h"

// Flyweight
#include "gatewayRadioTask.h"
#include "commands.h"
#include "ledBlinkTask.h"

// SMAC
#include "simple_mac.h"

/*
** ===================================================================
**     Event       :  TimerInt (module Events)
**
**     From bean   :  TickTimer [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the bean is enabled - "Enable" and the events are
**         enabled - "EnableEvent").
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
extern void TimerInt(void);
/*
** ===================================================================
**     Event       :  SW1Int_OnInterrupt (module Events)
**
**     From bean   :  SW1Int [ExtInt]
**     Description :
**         This event is called when the active signal edge/level
**         occurs.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#ifdef __SWI
extern void vButtonPush(void);
#pragma TRAP_PROC SAVE_NO_REGS 
void SW1Int_OnInterrupt(void)
{
  /* place your SW1Int interrupt procedure body here */
  vButtonPush();
}
#endif


/*
** ===================================================================
**     Event       :  USB_OnFullRxBuf (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called when the input buffer is full.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

#ifdef __USB
#endif

/*
** ===================================================================
**     Event       :  TimerInt (module Events)
**
**     From bean   :  TickTimer [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the bean is enabled - "Enable" and the events are
**         enabled - "EnableEvent").
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#pragma NO_ENTRY 
#pragma NO_EXIT 
#pragma NO_FRAME 
#pragma NO_RETURN
extern void vPortTickInterrupt(void);
void TimerInt(void)
{
	vPortTickInterrupt();
}

/*
** ===================================================================
**     Event       :  AudioLoader_OnInterrupt (module Events)
**
**     From bean   :  AudioLoader [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the bean is enabled - "Enable" and the events are
**         enabled - "EnableEvent").
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
//#ifdef __AudioLoader

#include "task.h"

BufferOffsetType	gCurPWMOffset = 0;
BufferCntType		gCurPWMRadioBufferNum = 0;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
// We further divide this by two since we average the cur and prev sample to smooth the waveform.
SampleRateType		gMasterSampleRate = 2000 / SAMPLE_SMOOTH_STEPS;
// The "tuning" time for the master rate to keep the packet flow balanced.
INT16				gMasterSampleRateAdjust = 0;
// Sample processing details.
UINT8				gStepNum = 0;
INT8				gStepSize = 0;
UINT16				gCurModulo = 0;
UINT16				gPrevModulo = 0;
INT16				gPWMMaxValue = 0x110;
// 1/2 PWM max value - 1 (to convert from 2's complement).
INT16				gPWMCenterValue = 0x90;

interrupt void AudioLoader_OnInterrupt(void)
{	
	INT8 sample;
	
	// Reset the timer for the next sample.
	TPM2MOD = gMasterSampleRate + gMasterSampleRateAdjust;
	TPM1MOD = gPWMMaxValue;

	// This is not a sound buffer, so advance to the next buffer.
	if ((getCommandNumber(gCurPWMRadioBufferNum) != eCommandDatagram)
		|| (gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus != eBufferStateInUse)) {
	
		setReg16(TPM1C2V, gPWMMaxValue);
		EnterCritical();
		
			gCurPWMRadioBufferNum++;
			if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
				gCurPWMRadioBufferNum = 0;
			
		ExitCritical();
		
	} else {
	
		if (gStepNum == 0) {
			// The data is in 2's compliment, so switch it back to positive integer range.
			gPrevModulo = gCurModulo;
			// Use a signed int to make the 2's compliment math simpler.
			sample = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
			gCurModulo = gPWMCenterValue - sample;
			gStepSize = (gCurModulo - gPrevModulo) / SAMPLE_SMOOTH_STEPS;
		}
		
		setReg16(TPM1C2V, gPrevModulo + (gStepSize * gStepNum));
		
		gStepNum++;
		if (gStepNum >= SAMPLE_SMOOTH_STEPS) {
			gStepNum = 0;

//		gCurSample = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
//		setReg16(TPM1C2V, gPWMCenterValue + gCurSample);

		// Increment the buffer pointers.
		gCurPWMOffset++;
		if (gCurPWMOffset > RX_BUFFER_SIZE - 1) {
			
			gCurPWMOffset = 4;
			
			// The buffers are a shared, critical resource, so we have to protect them before we update.
			EnterCritical();
			
				// Indicate that the buffer is clear.
				gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus = eBufferStateFree;
				
				// Advance to the next buffer.
				gCurPWMRadioBufferNum++;
				if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
					gCurPWMRadioBufferNum = 0;
				
				// Account for the number of used buffers.
				if (gRXUsedBuffers > 0)
					gRXUsedBuffers--;
				
			ExitCritical();
				
			// Adjust the sampling rate to account for mismatches in the OTA rate.				
			// We can't go too low or high, or we'll end up missing 
			// the next interrupt and making the sample run "long".
			if ((gRXUsedBuffers > RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust > -0x600)) {
				gMasterSampleRateAdjust--;
			} else if ((gRXUsedBuffers < RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust < 0x600)) {
				gMasterSampleRateAdjust++;
			};
		}
		}
	
	}

	// Reset the overflow flag.
	if (TPM2SC_TOF)
		TPM2SC_TOF = 0;
}
//#endif
/*
** ===================================================================
**     Event       :  PWM1_OnEnd (module Events)
**
**     From bean   :  PWM1 [PWM]
**     Description :
**         This event is called when the specified number of cycles
**         has been generated. (Only when the bean is enabled -
**         Enable and the events are enabled - EnableEvent).
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

#pragma NO_ENTRY 
#pragma NO_EXIT 
#pragma NO_FRAME 
#pragma NO_RETURN
void dispatchRTI();
void dispatchRTI() {
	vPortTickInterrupt();
	
	// Ack the RTI
	IRQSC_IRQACK = 1;
}

ISR(testRTI)
{
	dispatchRTI();
}

/*
** ===================================================================
**     Event       :  USB_OnError (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be
**         read using <GetError> method.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  USB_OnError(void)
{
  /* Write your code here ... */
}

/* END Events */
