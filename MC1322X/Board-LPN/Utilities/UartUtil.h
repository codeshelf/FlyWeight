/************************************************************************************
* Includes the UART Utils.
*
* (c) Copyright 2008, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected:
* Last Tested:
*
************************************************************************************/
#ifndef _UART_UTIL_H_
#define _UART_UTIL_H_

#include "../../PLM/LibInterface/UartLowLevel.h"
#include "../../SMAC/Drivers/Configure/board_config.h"
#include "../../PLM/Interface/UART_Interface.h"
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************/
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/
#define gPlatformClock_c  (PLATFORM_CLOCK/1000)
#define gUartTimeout_d    (300)
/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/* If you have to print a hex number you can choose between
   BigEndian=1/LittleEndian=0, newline, commas or spaces (between bytes) */
enum {
  gPrtHexBigEndian_c = 1<<0,
  gPrtHexNewLine_c   = 1<<1,
  gPrtHexCommas_c    = 1<<2,
  gPrtHexSpaces_c    = 1<<3
};

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern UartReadStatus_t gu8SCIStatus;
extern uint16_t gu16SCINumOfBytes;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* Function that initializate the Uart module, the module to initializate is the Uart
* defined as default in the board_config.h file. The baud rate and the system clock
* frequency are also defined in the board_config.h.
*
* Interface assumptions:
*   This function initializate the respective SCI interruption.
*
* Return value:
*   None
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
************************************************************************************/
void Uart_Init(void);

/************************************************************************************
* Function that prints out a string on the UART interface. String needs to be
* 0-terminated.
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t * pString - String that will be printed on the serial interface.
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
************************************************************************************/
void Uart_Print(uint8_t* pString);

/************************************************************************************
* Function that prints out one byte on the UART interface as a hexdecimal string.
* E.g. the byte 0x5E is printed as the string "5E". It is up to the user to add
* decorations before or after the string is printed.
* Example:
*   Uart_Print("Number is: 0x");
*   Uart_PrintHex(0x5E,1,0);
*   Uart_Print("\r\n");
*   This will result in "Number is 0x5E" being printed over the UART with line shift.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t * pHex a pointer to the data that will be printed as hexadecimal.
*             uint8_t len the length of the value in bytes.
*             uint8_t uint8_t flags indicator of a specific functionality.
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void Uart_PrintHex(uint8_t *pu8Hex, uint8_t u8len, uint8_t u8flags);

/************************************************************************************
* The Uart_PrintByteDec function prints out one byte on the UART interface as a
* decimal string. E.g. the byte 0x0E is printed as the string "14". It is up to the
* user to add decorations before or after the string is printed.
* Example:
*   Uart_Print("Number is: ");
*   Uart_PrintHex(0x0E);
*   Uart_Print("\r\n");
*   This will result in "Number is 14" being printed over the UART with line shift.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t u8Dec number in hexadecinal to be printed in its corresponding
*                           decimal value.
*
* Parameters: uint8_t u8dec a pointer to the data that will be printed as hexadecimal.
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void Uart_PrintByteDec(uint8_t u8Dec);


/************************************************************************************
* The Uart_PrintShortDec function transmit a decimal value to the Uart.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint16_t u16Dec number in hexadecinal to be printed in its corresponding
*                           decimal value.
*
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void Uart_PrintShortDec(uint16_t u16Dec);


void Uart_Tx(const uint8_t * msg, uint8_t length);
/************************************************************************************
* Function that waits for a character from the Uart interface.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t * pRxBuffer - Pointer to the buffer where the received
*                                   character is stored.
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
************************************************************************************/
uint8_t Uart_Poll(uint8_t *pRxBuffer);


/************************************************************************************
* Function that waits for a character from the Uart interface.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t * pRxBuffer - Pointer to the buffer where the received
*                                   character is stored.
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
************************************************************************************/
char Uart_getchar(uint8_t * u8Char);


/************************************************************************************
* Function that converts a ascii byte in hexadecimal value.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t u8Ascii - Value in ascci
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
************************************************************************************/
uint8_t AsciitoHex(uint8_t ascii);

/************************************************************************************
* Function that converts a hexadecimal byte in ascii code.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
* Parameters: uint8_t u8Hex - Value in ascci
*
* Revision history:
*   date      Author    Comments
*   ------    ------    --------
************************************************************************************/
static uint8_t HexToAscii(uint8_t u8Hex);

bool_t mem_cmp(uint8_t * str1, uint8_t * str2, uint8_t sz);
void Uart_putchar(uint8_t u8Char);
char Uart_getchar1(void);



#endif /* _UART_UTIL_H_ */



