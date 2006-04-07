/* MODULE Events */


/*Including used modules for compilling procedure*/
#include "Cpu.h"
#include "Events.h"

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "gatewayRadioTask.h"
#include "ledBlinkTask.h"
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
#ifdef __AudioLoader

#include "task.h"

BufferOffsetType	gCurPWMOffset = 0;
//UINT8				gPWMDutyCycle;
BufferCntType		gCurPWMRadioBufferNum = 0;
INT16				gMasterSampleRateAdjust = 0;
//portTickType		lastTickCount;

void AudioLoader_OnInterrupt(void)
{
//	if (xTaskGetTickCount() - lastTickCount > 5) {
//		USB_SendChar('P');
//		USB_SendChar(gCurPWMOffset);
//		USB_SendChar(gCurPWMRadioBufferNum);
//		USB_SendChar(xTaskGetTickCount() - lastTickCount);
//	}
//	lastTickCount = xTaskGetTickCount();
	
	// It's OK if the variable overflows - we just want to get every 8th pulse.
	if (gRadioBuffer[gCurPWMRadioBufferNum].bufferStatus != eBufferStateFull) {
	
		TPM1C2V = 0x80;
		
	} else {
		
		// Load in the next value from the correct PWM buffer.
		//gPWMDutyCycle = gRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
		
		//AudioLoader_Disable();
		//TPM2C1SC_CH1IE = 0;

		// The data is in 2's complement, so switch it back to positive integer range.
		TPM1C2VL = gRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset] + 0x80;
		TPM1C2VH = 0;

//		USB_SendChar(gRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset]);
		//AudioLoader_Enable();
		//TPM2C1SC_CH1F = 0;
		//TPM2C1SC_CH1IE = 1;
		
		// Increment the buffer pointers.
		gCurPWMOffset++;
		if (gCurPWMOffset > ASYNC_BUFFER_SIZE - 1) {
			
			gCurPWMOffset = 0;
			
			// The buffers are a shared, critical resource, so we have to protect them before we update.
			EnterCritical();
			
				// Indicate that the buffer is clear.
				gRadioBuffer[gCurPWMRadioBufferNum].bufferStatus = eBufferStateEmpty;

				// Advance to the next buffer.
				if (gCurPWMRadioBufferNum >= ASYNC_BUFFER_COUNT - 1)
					gCurPWMRadioBufferNum = 0;
				else
					gCurPWMRadioBufferNum++;
				
				// Account for the number of used buffers.
				if (gUsedBuffers > 0)
					gUsedBuffers--;
				else
					gUsedBuffers = 0;
				
			ExitCritical();
				
			// Adjust the sampling rate to account for mismatches in the OTA rate.				
			if ((gUsedBuffers > RADIO_QUEUE_BALANCE) && (gMasterSampleRateAdjust > -500)) {
				gMasterSampleRateAdjust--;
			} else if ((gUsedBuffers < RADIO_QUEUE_BALANCE) && (gMasterSampleRateAdjust < 500)) {
				gMasterSampleRateAdjust++;
			};
		}
		
		// We can't go too low, or we'll end up missing the next interrupt and making the sample take longer.
		TPM2C1V+=gMasterSampleRateAdjust;
		//if (TPM2C1V < TPM2CNT) {
		//	TPM2C1V = TPM2CNT + 17;
		//}

	}
}
#endif
/* END Events */
