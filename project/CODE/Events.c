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
#include "simple_mac.h"

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
// Radio input buffer
extern RadioBufferStruct	gRadioBuffer[ASYNC_BUFFER_COUNT];
extern BufferCntType		gCurRadioBufferNum;

#ifdef __USB
void  USB_OnFullRxBuf(void)
{
/*	word	bytesReceived;
	byte	err;
	
	RTS_OFF;
	
	// Copy the contents of the buffer.  (Bummer! Should just return the pointer to the buffer.)
	err = USB_RecvBlock((byte*) &gRadioBuffer[gCurRadioBufferNum].bufferStorage, BUFFER_SIZE, &bytesReceived);
	gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateFull;
	
	// Send the buffer pointer to the transmit task's queue.
	if (xQueueSend(xRadioTransmitQueue, &gCurRadioBufferNum, pdFALSE)) {
	
	}
	
	// Setup for the next transmit cycle.
	gCurRadioBufferNum++;
	if (gCurRadioBufferNum > (BUFFER_COUNT - 1))
		gCurRadioBufferNum = 0;
	gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateEmpty;
*/
}
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
BufferOffsetType	gRcvPos = 0;

void  USB_OnRxChar(void)
{
//	static tTxPacket		gsTxPacket;
//	static BufferCntType	bufferNum;

	byte err;
	
	// Copy the contents of the buffer.  (Bummer! Should just return the pointer to the buffer.)
	err = USB_RecvChar((byte*) &gRadioBuffer[gCurRadioBufferNum].bufferStorage[gRcvPos]);
	
	if (err == ERR_OK) {
		gRcvPos++;
		if (gRcvPos > ASYNC_BUFFER_SIZE - 1) {
			gRcvPos = 0;
			gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateFull;
			
			//gsTxPacket.pu8Data = gRadioBuffer[gCurRadioBufferNum].bufferStorage;
		//	gsTxPacket.u8DataLength = ASYNC_BUFFER_SIZE;
			//MCPSDataRequest(&gsTxPacket);

			// Send the buffer pointer to the transmit task's queue.
			if (xQueueSendFromISR(xRadioTransmitQueue, &gCurRadioBufferNum, pdFALSE)) {
			
			}
			
			// Setup for the next transmit cycle.
			if (gCurRadioBufferNum == (ASYNC_BUFFER_COUNT - 1))
				gCurRadioBufferNum = 0;
			else
				gCurRadioBufferNum++;
				
			gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateEmpty;
		}
	}
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
**     Event       :  UART_OnError (module Events)
**
**     From bean   :  UART [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be
**         read using <GetError> method.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  UART_OnError(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  UART_OnRxChar (module Events)
**
**     From bean   :  UART [AsynchroSerial]
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
void  UART_OnRxChar(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  UART_OnTxChar (module Events)
**
**     From bean   :  UART [AsynchroSerial]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  UART_OnTxChar(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  UART_OnFullRxBuf (module Events)
**
**     From bean   :  UART [AsynchroSerial]
**     Description :
**         This event is called when the input buffer is full.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  UART_OnFullRxBuf(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  UART_OnFreeTxBuf (module Events)
**
**     From bean   :  UART [AsynchroSerial]
**     Description :
**         This event is called after the last character in output
**         buffer is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  UART_OnFreeTxBuf(void)
{
  /* Write your code here ... */
}

/* END Events */
