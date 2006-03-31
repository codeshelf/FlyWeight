/** ###################################################################
**     Filename  : Events.H
**     Project   : RTOSDemo
**     Processor : MC9S12C32CFU
**     Beantype  : Events
**     Version   : Driver 01.01
**     Compiler  : Metrowerks HC12 C Compiler
**     Date/Time : 17/05/2005, 08:36
**     Abstract  :
**         This is user's event module.
**         Put your event handler code here.
**     Settings  :
**     Contents  :
**         TimerInt - void TimerInt(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2002
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __Events_H
#define __Events_H
/* MODULE Events */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PE_Timer.h"
#include "SWI.h"
#include "MC13191IRQ.h"
#include "TickTimer.h"
#include "SW2.h"
#include "SW3.h"
#include "SW4.h"
#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "LED4.h"
#include "UART.h"
#include "USB.h"

void SW1Int_OnInterrupt(void);
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
void USB_OnFullRxBuf(void);
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

void TimerInt(void);
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

void USB_OnRxChar(void);
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

void USB_OnTxChar(void);
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

void UART_OnError(void);
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

void UART_OnRxChar(void);
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

void UART_OnTxChar(void);
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

void UART_OnFullRxBuf(void);
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

void UART_OnFreeTxBuf(void);
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

/* END Events */
#endif /* __Events_H*/

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 03.33 for 
**     the Motorola HCS12 series of microcontrollers.
**
** ###################################################################
*/
