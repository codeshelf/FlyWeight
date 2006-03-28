/* MODULE Events */


/*Including used modules for compilling procedure*/
#include "Cpu.h"
#include "Events.h"

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PE_Timer.h"

/*
** ===================================================================
**     Event       :  TickTimerInt (module Events)
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
extern void TickTimerInt(void);
#pragma TRAP_PROC SAVE_NO_REGS 
void TickTimerInt(void)
{
  /* Write your code here ... */
  vPortTickInterrupt();
}


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
extern void vButtonPush(void);
#pragma TRAP_PROC SAVE_NO_REGS 
void SW1Int_OnInterrupt(void)
{
  /* place your SW1Int interrupt procedure body here */
  vButtonPush();
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
void  USB_OnRxChar(void)
{
  /* Write your code here ... */
}

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
