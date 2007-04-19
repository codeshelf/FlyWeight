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
#include "ulaw.h"

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

#ifdef XBEE
	#define PWM_MSB_CHANNEL		TPM1C0V
	#define PWM_LSB_CHANNEL		TPM1C1V
#else
	#define PWM_LSB_CHANNEL		TPM1C2V
#endif

BufferOffsetType	gCurPWMOffset = 0;
BufferCntType		gCurPWMRadioBufferNum = 0;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
// We further divide this by two since we average the cur and prev sample to smooth the waveform.
SampleRateType		gMasterSampleRate = 2000 / SAMPLE_SMOOTH_STEPS;
#ifdef STEPPING_SUPPORT
// Sample processing details.
UINT8				gStepNum = 0;
INT8				gStepSize = 0;
UINT16				gCurModulo = 0;
UINT16				gPrevModulo = 0;
#endif

// The "tuning" time for the master rate to keep the packet flow balanced.
INT16				gMasterSampleRateAdjust = 0;
UINT16				gPWMMaxValue = 0x100;
// 1/2 PWM max value - 1 (to convert from 2's complement).
UINT16				gPWMCenterValue = 0x80;

interrupt void AudioLoader_OnInterrupt(void)
{	
	INT8	sample;
	UINT16	uLawSample;
	
	// Reset the timer for the next sample.
	TPM2MOD = gMasterSampleRate + gMasterSampleRateAdjust;
	TPM1MOD = gPWMMaxValue;

	// This is not a sound buffer, so advance to the next buffer.
	if ((getCommandNumber(gCurPWMRadioBufferNum) != eCommandDatagram)
		|| (gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus != eBufferStateInUse)) {
	
		//setReg16(PWM_LOW_CHANNEL, gPWMMaxValue);
		EnterCritical();
		
			gCurPWMRadioBufferNum++;
			if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
				gCurPWMRadioBufferNum = 0;
			
		ExitCritical();
		
	} else {
	
#ifdef STEPPING_SUPPORT
		if (gStepNum == 0) {
			// The data is in 2's compliment, so switch it back to positive integer range.
			gPrevModulo = gCurModulo;
			// Use a signed int to make the 2's compliment math simpler.
			sample = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
			gCurModulo = gPWMCenterValue - sample;
			gStepSize = (gCurModulo - gPrevModulo) / SAMPLE_SMOOTH_STEPS;
		}
		
		setReg16(PWM_LOW_CHANNEL, gPrevModulo + (gStepSize * gStepNum));
		
		gStepNum++;
		if (gStepNum >= SAMPLE_SMOOTH_STEPS) {
			gStepNum = 0;
#else
		sample = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
#ifdef XBEE
		// On the XBee module we support 16bit converted uLaw samples.
		// One to 8-bit channel 0, and one to 8-bit channel 1.  
		// The two channels are tied together with different resistor values to give us 16 bit resolution.
		// (By "shifting" the voltage of channel 0 by 256x.)
		uLawSample = ulaw2linear(sample);
		// Only the lower 8 bits of each channel are in use.
		setReg16(PWM_MSB_CHANNEL, gPWMCenterValue - (uLawSample >> 8));
		setReg16(PWM_LSB_CHANNEL, gPWMCenterValue - (uLawSample && 0x00ff));
#else
		setReg16(PWM_LSB_CHANNEL, gPWMCenterValue - sample);
#endif
		{
#endif
			// Increment the buffer pointers.
			gCurPWMOffset++;
			if (gCurPWMOffset >= gRXRadioBuffer[gCurPWMRadioBufferNum].bufferSize) {
				
				gCurPWMOffset = CMDPOS_DATAGRAM;
				
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
				if ((gRXUsedBuffers > RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust > -0x80)) {
					gMasterSampleRateAdjust--;
				} else if ((gRXUsedBuffers < RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust < 0x80)) {
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
