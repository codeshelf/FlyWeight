/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : USB.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : AsynchroSerial
**     Version   : Bean 02.333, Driver 01.12, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/27/2006, 4:38 PM
**     Abstract  :
**         This bean "AsynchroSerial" implements an asynchronous serial
**         communication. The bean supports different settings of
**         parity, word width, stop-bit and communication speed,
**         user can select interrupt or polling handler.
**         Communication speed can be changed also in runtime.
**         The bean requires one on-chip asynchronous serial channel.
**     Settings  :
**         Serial channel              : SCI2
**
**         Protocol
**             Init baud rate          : 57600baud
**             Width                   : 8 bits
**             Stop bits               : 1
**             Parity                  : none
**             Breaks                  : Disabled
**
**         Registers
**             Input buffer            : SCI2D     [0027]
**             Output buffer           : SCI2D     [0027]
**             Control register        : SCI2C1    [0022]
**             Mode register           : SCI2C2    [0023]
**             Baud setting reg.       : SCI2BD    [0020]
**             Special register        : SCI2S1    [0024]
**
**         Input interrupt
**             Vector name             : Vsci2rx
**
**         Output interrupt
**             Vector name             : Vsci2tx
**
**         Used pins:
**         ----------------------------------------------------------
**           Function | On package           |    Name
**         ----------------------------------------------------------
**            Input   |     3                |  PTC1_RxD2
**            Output  |     2                |  PTC0_TxD2
**             RTS    |     42               |  PTA7_KBI1P7
**         ----------------------------------------------------------
**
**             Note: RTS pin is NOT supported by hardware.
**                   It is handled by software.
**
**
**     Contents  :
**         RecvChar        - byte USB_RecvChar(USB_TComData *Chr);
**         SendChar        - byte USB_SendChar(USB_TComData Chr);
**         GetCharsInRxBuf - word USB_GetCharsInRxBuf(void);
**         GetCharsInTxBuf - word USB_GetCharsInTxBuf(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __USB
#define __USB

/* MODULE USB. */

#include "Cpu.h"



#ifndef __BWUserType_tItem
#define __BWUserType_tItem
  typedef struct {                     /* Item of the index table for possible baudrates */
    word div;                          /* divisor */
    byte val;                          /* values of the prescalers */
  } tItem;
#endif
#ifndef __BWUserType_USB_TError
#define __BWUserType_USB_TError
  typedef union {
    byte err;
    struct {
      bool OverRun  : 1;               /* Overrun error flag */
      bool Framing  : 1;               /* Framing error flag */
      bool Parity   : 1;               /* Parity error flag */
      bool RxBufOvf : 1;               /* Rx buffer full error flag */
      bool Noise    : 1;               /* Noise error flag */
      bool Break    : 1;               /* Break detect */
      bool LINSync  : 1;               /* LIN synchronization error */
    } errName;
  } USB_TError;                        /* Error flags. For languages which don't support bit access is byte access only to error flags possible. */
#endif

#ifndef __BWUserType_USB_TComData
#define __BWUserType_USB_TComData
  typedef byte USB_TComData ;          /* User type for communication. Size of this type depends on the communication data witdh. */
#endif

#define USB_RTS_BUF_SIZE 1             /* Number of characters in rcv. buffer when RTS signal gets activated */


byte USB_RecvChar(USB_TComData *Chr);
/*
** ===================================================================
**     Method      :  USB_RecvChar (bean AsynchroSerial)
**
**     Description :
**         If any data is received, this method returns one
**         character, otherwise it returns an error code (it does
**         not wait for data). This method is enabled only if the
**         receiver property is enabled. 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method only sets the selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInRxBuf method. See an example of a typical usage
**         for details about the communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Chr             - Pointer to a received character
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_RXEMPTY - No data in receiver
**                           ERR_BREAK - Break character is detected(only when <Interrupt service> property
**                           is disabled)
**                           ERR_COMMON - common error occurred (the
**                           GetError method can be used for error
**                           specification)
**                           DMA mode:
**                           If DMA controller is available on the
**                           selected CPU and the receiver is
**                           configured to use DMA controller then
**                           only ERR_OK, ERR_RXEMPTY, and ERR_SPEED
**                           error code can be returned from this
**                           method.
** ===================================================================
*/

byte USB_SendChar(USB_TComData Chr);
/*
** ===================================================================
**     Method      :  USB_SendChar (bean AsynchroSerial)
**
**     Description :
**         Sends one character to the channel. If the bean is
**         temporarily disabled (Disable method) SendChar method
**         only stores data into an output buffer. In case of a zero
**         output buffer size, only one character can be stored.
**         Enabling the bean (Enable method) starts the transmission
**         of the stored data. This method is available only if the
**         transmitter property is enabled.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method only sets selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInTxBuf method. See an example of a typical usage
**         for details about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Chr             - Character to send
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_TXFULL - Transmitter is full
** ===================================================================
*/

word USB_GetCharsInRxBuf(void);
/*
** ===================================================================
**     Method      :  USB_GetCharsInRxBuf (bean AsynchroSerial)
**
**     Description :
**         Returns the number of characters in the input buffer.
**         This method is available only if the receiver property is
**         enabled.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method returns the number of characters in the
**         receive buffer.
**     Parameters  : None
**     Returns     :
**         ---             - The number of characters in the input
**                           buffer.
** ===================================================================
*/

word USB_GetCharsInTxBuf(void);
/*
** ===================================================================
**     Method      :  USB_GetCharsInTxBuf (bean AsynchroSerial)
**
**     Description :
**         Returns the number of characters in the output buffer.
**         This method is available only if the transmitter property
**         is enabled.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method returns the number of characters in the
**         transmit buffer.
**     Parameters  : None
**     Returns     :
**         ---             - The number of characters in the output
**                           buffer.
** ===================================================================
*/

__interrupt void USB_InterruptRx(void);
/*
** ===================================================================
**     Method      :  USB_InterruptRx (bean AsynchroSerial)
**
**     Description :
**         The method services the receive interrupt of the selected 
**         peripheral(s) and eventually invokes the bean's event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

__interrupt void USB_InterruptTx(void);
/*
** ===================================================================
**     Method      :  USB_InterruptTx (bean AsynchroSerial)
**
**     Description :
**         The method services the transmit interrupt of the selected 
**         peripheral(s) and eventually invokes the bean's event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

__interrupt void USB_InterruptError(void);
/*
** ===================================================================
**     Method      :  USB_InterruptError (bean AsynchroSerial)
**
**     Description :
**         The method services the error interrupt of the selected 
**         peripheral(s) and eventually invokes the bean's event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/


void USB_Init(void);
/*
** ===================================================================
**     Method      :  USB_Init (bean AsynchroSerial)
**
**     Description :
**         Initializes the associated peripheral(s) and the bean's 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void USB_SetHigh(void);
/*
** ===================================================================
**     Method      :  USB_SetHigh (bean AsynchroSerial)
**
**     Description :
**         The method reconfigures the bean and its selected peripheral(s)
**         when the CPU is switched to the High speed mode. The method is 
**         called automatically as s part of the CPU SetHighSpeed method.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void USB_SetLow(void);
/*
** ===================================================================
**     Method      :  USB_SetLow (bean AsynchroSerial)
**
**     Description :
**         The method reconfigures the bean and its selected peripheral(s)
**         when the CPU is switched to the Low speed mode. The method is 
**         called automatically as a part of the CPU SetLowSpeed method.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

/* END USB. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/

#endif /* ifndef __USB */
