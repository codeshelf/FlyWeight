/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : USB.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : AsynchroSerial
**     Version   : Bean 02.333, Driver 01.12, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/24/2007, 12:46 PM
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
**             Init baud rate          : 250000baud
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
**             RTS    |     38               |  PTA6_KBI1P6
**         ----------------------------------------------------------
**
**             Note: RTS pin is NOT supported by hardware.
**                   It is handled by software.
**
**
**     Contents  :
**         SendChar        - byte USB_SendChar(USB_TComData Chr);
**         RecvBlock       - byte USB_RecvBlock(USB_TComData *Ptr,word Size,word *Rcv);
**         SendBlock       - byte USB_SendBlock(USB_TComData *Ptr,word Size,word *Snd);
**         GetCharsInRxBuf - word USB_GetCharsInRxBuf(void);
**         GetError        - byte USB_GetError(USB_TError *Err);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


/* MODULE USB. */

#pragma MESSAGE DISABLE C4002 /* WARNING C4002: Result not used is ignored */

#include "USB.h"
#include "Events.h"



#define OVERRUN_ERR      0x01          /* Overrun error flag bit   */
#define FRAMING_ERR      0x02          /* Framing error flag bit   */
#define PARITY_ERR       0x04          /* Parity error flag bit    */
#define NOISE_ERR        0x08          /* Noise error flag bit     */
#define CHAR_IN_RX       0x10          /* Char is in RX buffer     */
#define RUNINT_FROM_TX   0x20          /* Interrupt is in progress */
#define FULL_RX          0x40          /* Full receive buffer      */
#define FULL_TX          0x80          /* Full transmit buffer     */

static byte SerFlag;                   /* Flags for serial communication */
                                       /* Bit 0 - Overrun error */
                                       /* Bit 1 - Framing error */
                                       /* Bit 2 - Parity error */
                                       /* Bit 3 - Noise error */
                                       /* Bit 4 - Char in RX buffer */
                                       /* Bit 5 - Interrupt is in progress */
                                       /* Bit 6 - Full RX buffer */
                                       /* Bit 7 - Full TX buffer */
static byte ErrFlag = 0;               /* Error flags mirror of SerFlag */
byte USB_InpLen;                       /* Length of the input buffer content */
static byte InpIndxR;                  /* Index for reading from input buffer */
static byte InpIndxW;                  /* Index for writing to input buffer */
static USB_TComData InpBuffer[USB_INP_BUF_SIZE]; /* Input buffer for SCI commmunication */
byte USB_OutLen;                       /* Length of the output buffer content */
static byte OutIndxR;                  /* Index for reading from output buffer */
static byte OutIndxW;                  /* Index for writing to output buffer */
static USB_TComData OutBuffer[USB_OUT_BUF_SIZE]; /* Output buffer for SCI commmunication */


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
byte USB_RecvChar(USB_TComData *Chr)
{
  byte Result = ERR_OK;                /* Return error code */

  if(USB_InpLen > 0) {                 /* Is number of received chars greater than 0? */
    EnterCritical();                   /* Save the PS register */
    USB_InpLen--;                      /* Decrease number of received chars */
    *Chr = InpBuffer[InpIndxR];        /* Received char */
    if (++InpIndxR >= USB_INP_BUF_SIZE) /* Is the index out of the buffer? */
      InpIndxR = 0;                    /* Set the index to the start of the buffer */
    if(USB_InpLen <= USB_RTS_BUF_SIZE)
      PTAD &= ~0x40;                   /* Set RTS to the low level */
    Result = (byte)((SerFlag & (OVERRUN_ERR|FRAMING_ERR|PARITY_ERR|NOISE_ERR|FULL_RX))?ERR_COMMON:ERR_OK);
    SerFlag &= ~(OVERRUN_ERR|FRAMING_ERR|PARITY_ERR|NOISE_ERR|FULL_RX|CHAR_IN_RX); /* Clear all errors in the status variable */
    ExitCritical();                    /* Restore the PS register */
  } else {
    return ERR_RXEMPTY;                /* Receiver is empty */
  }
  return Result;                       /* Return error code */
}

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
byte USB_SendChar(USB_TComData Chr)
{
  if(USB_OutLen == USB_OUT_BUF_SIZE)   /* Is number of chars in buffer is the same as a size of the transmit buffer */
    return ERR_TXFULL;                 /* If yes then error */
  EnterCritical();                     /* Save the PS register */
  USB_OutLen++;                        /* Increase number of bytes in the transmit buffer */
  OutBuffer[OutIndxW] = Chr;           /* Store char to buffer */
  if (++OutIndxW >= USB_OUT_BUF_SIZE)  /* Is the index out of the buffer? */
    OutIndxW = 0;                      /* Set the index to the start of the buffer */
  SCI2C2_TIE = 1;                      /* Enable transmit interrupt */
  ExitCritical();                      /* Restore the PS register */
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  USB_RecvBlock (bean AsynchroSerial)
**
**     Description :
**         If any data is received, this method returns the block of
**         the data and its length (and incidental error), otherwise
**         it returns an error code (it does not wait for data).
**         This method is available only if non-zero length of the
**         input buffer is defined and the receiver property is
**         enabled.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method only sets the selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInRxBuf method. See an example of a typical usage
**         for details about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Ptr             - Pointer to the block of received data
**         Size            - Size of the block
**       * Rcv             - Pointer to real number of the received
**                           data
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_RXEMPTY - No data in receiver
**                           ERR_VALUE - Parameter is out of range.
**                           ERR_COMMON - common error occurred (the
**                           GetError method can be used for error
**                           specification)
**                           DMA mode:
**                           If DMA controller is available on the
**                           selected CPU and the receiver is
**                           configured to use DMA controller then
**                           only ERR_OK, ERR_RXEMPTY, and ERR_SPEED
**                           error codes can be returned from this
**                           method.
** ===================================================================
*/
byte USB_RecvBlock(USB_TComData *Ptr, word Size, word *Rcv)
{
  word count;                          /* Number of received chars */
  byte result = ERR_OK;                /* Last error */

  for(count = 0; count < Size; count++) {
    result = USB_RecvChar(Ptr++);
    if(result != ERR_OK) {             /* Receiving given number of chars */
      break;                           /* Break data block receiving */
    }
  }
  *Rcv = count;                        /* Return number of received chars */
  return result;                       /* Return last error code*/
}

/*
** ===================================================================
**     Method      :  USB_SendBlock (bean AsynchroSerial)
**
**     Description :
**         Sends a block of characters to the channel.
**         This method is available only if non-zero length of the
**         output buffer is defined and the transmitter property is
**         enabled.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method only sets the selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInTxBuf method. See typical usage for details
**         about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Ptr             - Pointer to the block of data to send
**         Size            - Size of the block
**       * Snd             - Pointer to number of data that are sent
**                           (moved to buffer)
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_TXFULL - It was not possible to send
**                           requested number of bytes
** ===================================================================
*/
byte USB_SendBlock(USB_TComData * Ptr, word Size, word *Snd)
{
  word count;                          /* Number of sent chars */
  byte result = ERR_OK;                /* Last error */

  for(count = 0; count < Size; count++) {
    result = USB_SendChar(*Ptr++);
    if(result != ERR_OK) {             /* Sending given number of chars */
      break;                           /* Break data block sending */
    }
  }
  *Snd = count;                        /* Return number of sent chars */
  return result;                       /* Return last error code*/
}

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
/*
word USB_GetCharsInRxBuf(void)

**      This method is implemented as a macro. See header module. **
*/

/*
** ===================================================================
**     Method      :  USB_GetError (bean AsynchroSerial)
**
**     Description :
**         Returns a set of errors on the channel (errors that
**         cannot be returned by given methods). The errors
**         accumulate in a set; after calling [GetError] this set is
**         returned and cleared.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Err             - Pointer to the returned set of errors
**     Returns     :
**         ---             - Error code (if GetError did not succeed),
**                           possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
byte USB_GetError(USB_TError *Err)
{
  EnterCritical();                     /* Save the PS register */
  Err->err = 0;
  Err->errName.OverRun = ((ErrFlag & OVERRUN_ERR) != 0); /* Overrun error */
  Err->errName.Framing = ((ErrFlag & FRAMING_ERR ) != 0); /* Framing error */
  Err->errName.Parity = ((ErrFlag & PARITY_ERR) != 0); /* Parity error */
  Err->errName.RxBufOvf = ((ErrFlag & FULL_RX) != 0); /* Buffer overflow */
  Err->errName.Noise = ((ErrFlag & NOISE_ERR) != 0); /* Noise error */
  ErrFlag = 0;                         /* Clear error flags */
  ExitCritical();                      /* Restore the PS register */
  return ERR_OK;                       /* OK */
}

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
#define ON_ERROR      1
#define ON_FULL_RX    2
#define ON_RX_CHAR    4
#define ON_IDLE_CHAR  8
ISR(USB_InterruptRx)
{
  USB_TComData Data;                   /* Temporary variable for data */
  byte StatReg = getReg(SCI2S1);
  byte OnFlags = 0;                    /* Temporary variable for flags */

  PTAD |= 0x40;
  Data = SCI2D;                        /* Read data from the receiver */
  if(USB_InpLen < USB_INP_BUF_SIZE) {  /* Is number of bytes in the receive buffer lower than size of buffer? */
    USB_InpLen++;                      /* Increse number of chars in the receive buffer */
    InpBuffer[InpIndxW] = Data;        /* Save received char to the receive buffer */
    if (++InpIndxW >= USB_INP_BUF_SIZE) /* Is the index out of the buffer? */
      InpIndxW = 0;                    /* Set the index to the start of the buffer */
  } else {
    SerFlag |= FULL_RX;                /* If yes then set flag buffer overflow */
    ErrFlag |= FULL_RX;
    OnFlags |= ON_ERROR;               /* Set flag "OnError" */
  }
  if(OnFlags & ON_ERROR) {             /* Was error flag detect? */
    USB_OnError();                     /* If yes then invoke user event */
  }
  if(USB_InpLen < USB_RTS_BUF_SIZE)    /* Is number of chars in the receive buffer lower than size of the RTS buffer? */
    /* PTAD: PTAD6=0 */
    PTAD &= ~0x40;                     /* Set RTS to low level */
}

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
#define ON_FREE_TX  1
#define ON_TX_CHAR  2
ISR(USB_InterruptTx)
{
  SerFlag &= ~RUNINT_FROM_TX;          /* Reset flag "running int from TX" */
  if(USB_OutLen) {                     /* Is number of bytes in the transmit buffer greater then 0? */
    USB_OutLen--;                      /* Decrease number of chars in the transmit buffer */
    SerFlag |= RUNINT_FROM_TX;         /* Set flag "running int from TX"? */
    SCI2S1;                            /* Reset interrupt request flag */
    SCI2D = OutBuffer[OutIndxR];       /* Store char to transmitter register */
    if (++OutIndxR >= USB_OUT_BUF_SIZE) /* Is the index out of the buffer? */
      OutIndxR = 0;                    /* Set the index to the start of the buffer */
  }
  else {
    SCI2C2_TIE = 0;                    /* Disable transmit interrupt */
  }
}

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
ISR(USB_InterruptError)
{
  byte StatReg = getReg(SCI2S1);
  byte OnFlags = 0;                    /* Temporary variable for flags */

  if(StatReg & SCI2S1_OR_MASK)         /* Is overrun error detected? */
    OnFlags |= OVERRUN_ERR;            /* If yes then set an internal flag */
  if(StatReg & SCI2S1_NF_MASK)         /* Is noise error detected? */
    OnFlags |= NOISE_ERR;              /* If yes then set an internal flag */
  if (StatReg & SCI2S1_FE_MASK) {      /* Is framing error detected? */
    OnFlags |= FRAMING_ERR;            /* If yes then set an internal flag */
    }
  if(StatReg & SCI2S1_PF_MASK)
    OnFlags |= PARITY_ERR;
  SerFlag |= OnFlags;                  /* Copy flags status to SerFlag status variable */
  ErrFlag |= OnFlags;                  /* Copy flags status to ErrFlag status variable */
  SCI2D;                               /* Dummy read of data register - clear error bits */
  if(SerFlag & (OVERRUN_ERR|FRAMING_ERR|PARITY_ERR|NOISE_ERR)) { /* Was any error set? */
    USB_OnError();                     /* If yes then invoke user event */
  }
}

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
void USB_Init(void)
{
  SerFlag = 0;                         /* Reset flags */
  USB_InpLen = 0;                      /* No char in the receive buffer */
  InpIndxR = InpIndxW = 0;             /* Reset indices */
  USB_OutLen = 0;                      /* No char in the transmit buffer */
  OutIndxR = OutIndxW = 0;             /* Reset indices */
  /* SCI2C1: LOOPS=0,SCISWAI=0,RSRC=0,M=0,WAKE=0,ILT=0,PE=0,PT=0 */
  setReg8(SCI2C1, 0x00);               /* Configure the SCI */ 
  /* SCI2C3: R8=0,T8=0,TXDIR=0,??=0,ORIE=0,NEIE=0,FEIE=0,PEIE=0 */
  setReg8(SCI2C3, 0x00);               /* Disable error interrupts */ 
  /* SCI2C2: TIE=0,TCIE=0,RIE=0,ILIE=0,TE=0,RE=0,RWU=0,SBK=0 */
  setReg8(SCI2C2, 0x00);               /* Disable all interrupts */ 
  SCI2BDH = 0x00;                      /* Set high divisor register (enable device) */
  SCI2BDL = 0x05;                      /* Set low divisor register (enable device) */
      /* SCI2C3: ORIE=1,NEIE=1,FEIE=1,PEIE=1 */
  SCI2C3 |= 0x0F;                      /* Enable error interrupts */
  SCI2C2 |= ( SCI2C2_TE_MASK | SCI2C2_RE_MASK | SCI2C2_RIE_MASK); /*  Enable transmitter, Enable receiver, Enable receiver interrupt */
}



/* END USB. */


/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
