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

// Radio input buffer
extern RadioBufferStruct	gRadioBuffer[ASYNC_BUFFER_COUNT];
extern BufferCntType		gCurRadioBufferNum;

//static int				gPulseNum = 0;
static BufferOffsetType	gCurPWMOffset = 2;

/* The queue used to send data from the radio to the radio receive task. */
extern xQueueHandle xRadioTransmitQueue;

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
**     Event       :  USB_OnRxChar (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called after a correct character is
**         received. 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this event is disabled. Only OnFullRxBuf method can be
**         used in DMA mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#ifdef __USB

extern USBStateType gUSBState;
extern UINT8 gLED1;
extern UINT8 gLED2;
extern UINT8 gLED3;
extern UINT8 gLED4;
extern xQueueHandle xLEDBlinkQueue;

BufferOffsetType	gRcvPos = 0;
int maxChars;

void  USB_OnRxChar(void)
{
//	static tTxPacket		gsTxPacket;
//	static BufferCntType	bufferNum;

	byte err;
	
//	DisableInterrupts;
	
	maxChars = 0;
	
	while (USB_GetCharsInRxBuf()) {
		
		// Don't process more than five at a time.
		if (maxChars++ > 5)
			break;
		
		// If the next buffer is not ready then we need to pause receiving.
		if (gRadioBuffer[gCurRadioBufferNum].bufferStatus == eBufferStateFull) {
		
			// Blink LED1 to let us know we succeeded in transmitting the buffer.
			if (uxQueueMessagesWaiting(xLEDBlinkQueue) < LED_BLINK_QUEUE_SIZE) {
				if (xQueueSendFromISR(xLEDBlinkQueue, &gLED2, pdFALSE)) {
				
				}
			}
			
			// The buffer isn't ready, so let's leave and wait for the buffers to clear
			break;
			
		} else {

			// Copy the contents of the buffer.  (Bummer! Should just return the pointer to the buffer.)
			err = USB_RecvChar((byte*) &gRadioBuffer[gCurRadioBufferNum].bufferStorage[gRcvPos]);
	//		USB_SendChar(gRadioBuffer[gCurRadioBufferNum].bufferStorage[gRcvPos]);
			
			if (err == ERR_OK) {
				gRcvPos++;
				if (gRcvPos > ASYNC_BUFFER_SIZE - 1) {
					gRcvPos = 0;
					gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateFull;
					
					//gsTxPacket.pu8Data = gRadioBuffer[gCurRadioBufferNum].bufferStorage;
					//gsTxPacket.u8DataLength = ASYNC_BUFFER_SIZE;
					//MCPSDataRequest(&gsTxPacket);

					// Send the buffer pointer to the transmit task's queue.
					if (uxQueueMessagesWaiting(xRadioTransmitQueue) < RADIO_QUEUE_SIZE) {
						if (xQueueSendFromISR(xRadioTransmitQueue, &gCurRadioBufferNum, pdFALSE)) {
						}
					}
					
					// Setup for the next transmit cycle.
					if (gCurRadioBufferNum == (ASYNC_BUFFER_COUNT - 1))
						gCurRadioBufferNum = 0;
					else
						gCurRadioBufferNum++;
				}
			}
		}
	}
	
//	EnableInterrupts;
}
#endif

/*
** ===================================================================
**     Event       :  AudioOut_OnEnd (module Events)
**
**     From bean   :  AudioOut [PWM]
**     Description :
**         This event is called when the specified number of cycles
**         has been generated. (Only when the bean is enabled -
**         Enable and the events are enabled - EnableEvent).
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#ifdef __AudioOut

#define PWM_MULTIPLIER	8

static byte				gPWMDutyCycle;
static BufferCntType	gCurPWMRadioBufferNum;

void AudioOut_OnEnd(void)
{
	//DisableInterrupts

	// It's OK if the variable overflows - we just want to get every 8th pulse.
//	if (gPulseNum++ % PWM_MULTIPLIER == 0) {
		// Load in the next value from the correct PWM buffer.
		gPWMDutyCycle = gRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
		// "Shift up" from 2's complement to positive decimal values.
		gPWMDutyCycle += 0x80;
		//AudioOut_Disable();
		AudioOut_SetRatio8(gPWMDutyCycle);
		//AudioOut_Enable();
		
		// Increment the buffer pointers.
		gCurPWMOffset += 1;
		if (gCurPWMOffset > ASYNC_BUFFER_SIZE) {
			gCurPWMOffset = 2;
			
			if (gCurPWMRadioBufferNum >= ASYNC_BUFFER_COUNT - 1)
				gCurPWMRadioBufferNum = 0;
			else
				gCurPWMRadioBufferNum++;
		}
//	}

	//EnableInterrupts
}
#endif

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

static UINT8			gPWMDutyCycle;
static BufferCntType	gCurPWMRadioBufferNum = 0;

void AudioLoader_OnInterrupt(void)
{
	//DisableInterrupts

	// It's OK if the variable overflows - we just want to get every 8th pulse.
	if (gRadioBuffer[gCurPWMRadioBufferNum].bufferStatus != eBufferStateFull) {
	
		TPM1C0V = 0;
		
	} else {
		
		// Load in the next value from the correct PWM buffer.
		gPWMDutyCycle = gRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
		// "Shift up" from 2's complement to positive decimal values.
		//gPWMDutyCycle += 0x80;
		//AudioOut_Disable();
		//AudioOut_SetRatio8(gPWMDutyCycle);
		TPM1C0V = gPWMDutyCycle;
		//USB_SendChar(gPWMDutyCycle);
		//AudioOut_Enable();
		
		// Increment the buffer pointers.
		gCurPWMOffset++;
		if (gCurPWMOffset >= ASYNC_BUFFER_SIZE - 2) {
			
			gCurPWMOffset = 2;
			gRadioBuffer[gCurPWMRadioBufferNum].bufferStatus = eBufferStateEmpty;
			
			if (gCurPWMRadioBufferNum >= ASYNC_BUFFER_COUNT - 1)
				gCurPWMRadioBufferNum = 0;
			else
				gCurPWMRadioBufferNum++;
		}
	}
	
	//EnableInterrupts
}
#endif
/*
** ===================================================================
**     Event       :  USB_OnTxChar (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  USB_OnTxChar(void)
{
  /* Write your code here ... */
}

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
void  USB_OnFullRxBuf(void)
{
  /* Write your code here ... */
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

/*
** ===================================================================
**     Event       :  USB_OnFreeTxBuf (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called after the last character in output
**         buffer is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  USB_OnFreeTxBuf(void)
{
  /* Write your code here ... */
}

/* END Events */
