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
INT16				gMasterSampleRateAdjust = 0;
bool				gOddSample = 0;
UINT8				gCurSample = 0;
UINT8				gPrevSample = 0;

interrupt void AudioLoader_OnInterrupt(void)
{
	
	// This is not a sound buffer, so advance to the next buffer.
	if (getCommandNumber(gCurPWMRadioBufferNum) != eCommandDatagram) {
	
		TPM1C2V = 0x80;
		EnterCritical();
		
			gCurPWMRadioBufferNum++;
			if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
				gCurPWMRadioBufferNum = 0;
			
		ExitCritical();
		
	} else {
		
		if (gOddSample) {
		
			gOddSample = 0;
			TPM1C2VL = gCurSample;
			TPM1C2VH = 0;
		
		} else {
		
			gOddSample = 1;
			
			// The data is in 2's compliment, so switch it back to positive integer range.
			gPrevSample = gCurSample;
			gCurSample = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset] + 0x80;
			TPM1C2VL = ((UINT16) (gPrevSample + gCurSample)) / 2;
			TPM1C2VH = 0;

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
				if ((gRXUsedBuffers > RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust > -0x300)) {
					gMasterSampleRateAdjust--;
				} else if ((gRXUsedBuffers < RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust < 0x300)) {
					gMasterSampleRateAdjust++;
				};
			}
		}
		
		// We can't go too low, or we'll end up missing the next interrupt and making the sample take longer.
		TPM2MOD = gMasterSampleRate + gMasterSampleRateAdjust;
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
