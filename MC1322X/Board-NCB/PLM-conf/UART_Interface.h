/*****************************************************************************
* UART / SCI / USB / Serial Port driver declarations.
* 
* This driver supports both SCI1 and SCI2 on the ARM7, each of which can be
* enabled or disabled independantly. On the SRB, USB and NCB boards, the USB
* (SCI1) port is used. 
*
* Copyright (c) 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

/* On the receive side, this module keeps a small circular buffer, and calls
 * a callback function whenever a byte is received. The application can
 * retrieve bytes from the buffer at it's convenience, as long as it does so
 * before the driver's buffer fills up.
 *
 * On the transmit side, this module keeps a list of buffers to be
 * transmitted, and calls an application callback function when the entire
 * buffer has been sent. The application is responsible for ensuring that the
 * buffer is available until the callback function is called.
 *
 * If both SCI ports are enabled, each has it's own Rx circular buffer and
 * list of Tx buffers.
 */

#ifndef _Uart_Interface_h_
#define _Uart_Interface_h_

#include "../LibInterface/Platform.h"
#include "EmbeddedTypes.h"

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/


/* The ARM has two SCI/UART hardware interfaces. Define the one(s) in use. */

#define gUart1_Enabled_d    TRUE
#define gUart2_Enabled_d    FALSE


#ifndef gUart_PortDefault_d
  #if (gUart2_Enabled_d == TRUE)
    #define gUart_PortDefault_d     1
  #else
    #define gUart_PortDefault_d     0
  #endif /* gUart2_Enabled_d */
#endif /* gUart_PortDefault_d */

/*****************************************************************************/

/* Use hardware flow control? */

#define gUart1_EnableHWFlowControl_d     FALSE
#define gUart2_EnableHWFlowControl_d     FALSE


/*****************************************************************************/

/* Tuning definitions. */

/* Number of entries in the transmit-buffers-in-waiting list. */

#define gUart_TransmitBuffers_c     3

/* Size of the driver's Rx circular buffer. These buffers are used to */
/* hold received bytes until the application can retrieve them via the */
/* UartX_GetBytesFromRxBuffer() functions, and are not otherwise accessable */
/* from outside the driver. The size does not need to be a power of two. */

#define gUart_ReceiveBufferSize_c   32

/* If flow control is used, there is a delay before telling the far side */
/* to stop and the far side actually stopping. When there are SKEW bytes */
/* remaining in the driver's Rx buffer, tell the far side to stop */
/* transmitting. */

#define gUart_RxFlowControlSkew_d   8

/* Number of bytes left in the Rx buffer when hardware flow control is */
/* deasserted. */

#define gUart_RxFlowControlResume_d 8


/*
#if gUart_RxFlowControlResume_d >= (gUart_ReceiveBufferSize_c - gUart_RxFlowControlSkew_d)
#error Deassert flow control before it is asserted?
#endif
*/

/*****************************************************************************/

typedef uint16_t UartBaudRate_t;

#define gUARTBaudRate1200_c     ((UartBaudRate_t) 1200)
#define gUARTBaudRate2400_c     ((UartBaudRate_t) 2400)
#define gUARTBaudRate4800_c     ((UartBaudRate_t) 4800)
#define gUARTBaudRate9600_c     ((UartBaudRate_t) 9600)
#define gUARTBaudRate19200_c    ((UartBaudRate_t) 19200)
#define gUARTBaudRate38400_c    ((UartBaudRate_t) 38400)
#define gUARTBaudRate57600_c    ((UartBaudRate_t) 57600)
#define gUARTBaudRate115200_c   ((UartBaudRate_t) 115200)   /* Might not work for all clients */

#define Baudrate_1200   gUARTBaudRate1200_c
#define Baudrate_2400   gUARTBaudRate2400_c
#define Baudrate_4800   gUARTBaudRate4800_c
#define Baudrate_9600   gUARTBaudRate9600_c
#define Baudrate_19200  gUARTBaudRate19200_c
#define Baudrate_38400  gUARTBaudRate38400_c
#define Baudrate_57600  gUARTBaudRate57600_c
#define Baudrate_115200 gUARTBaudRate115200_c

/* Default baud rate. */
#define gUartDefaultBaud_c Baudrate_38400


#endif /*_Uart_Interface_h*/
