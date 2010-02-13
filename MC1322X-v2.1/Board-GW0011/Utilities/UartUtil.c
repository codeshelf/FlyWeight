/************************************************************************************
* Includes the UART Utils.
*
* (c) Copyright 2008, Freescale Semiconductor, Inc. All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected:
* Last Tested:
*
************************************************************************************/
#include "../../PLM/Interface/EmbeddedTypes.h"
#include "../../PLM/LibInterface/Interrupt.h"
#include "../../SMAC/Interface/GlobalDefs.h"
#include "UartUtil.h"

/******************************************************************************/
/******************************************************************************/

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************/
static uint8_t HexToAscii(uint8_t u8Hex);

void UartEventRead1(UartReadCallbackArgs_t* args);
void UartEventWrite2(UartWriteCallbackArgs_t* args);
void UartEventRead2(UartReadCallbackArgs_t* args);
void UartEventWrite1(UartWriteCallbackArgs_t* args);
void GpioUart1Init(void);
void GpioUart2Init(void);

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
 
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

UartReadStatus_t gu8SCIStatus = gUartReadStatusComplete_c;
volatile uint8_t gu8SCIDataFlag = FALSE;
uint16_t gu16SCINumOfBytes = 0;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************  
* Uart_Init
*
* Initializate the Uart module.
*************************************************************************************/
void Uart_Init(uint8_t *mUARTRxBuffer) 
{
  UartConfig_t pConfig;
  UartCallbackFunctions_t pCallback;
  
#if gUart_PortDefault_d == UART_1

  //initialize GPIOs for UART1 and UART2 
  GpioUart1Init();  

  //configure the uart parameters 
  pConfig.UartParity = gUartParityNone_c;
  pConfig.UartStopBits = gUartStopBits1_c;
  pConfig.UartBaudrate = gUartDefaultBaud_c;
  pConfig.UartFlowControlEnabled = gUart1_EnableHWFlowControl_d;
  pConfig.UartRTSActiveHigh = FALSE;

  
  //mount the interrupts corresponding to UART driver
  IntAssignHandler(gUart1Int_c, (IntHandlerFunc_t)UartIsr1);
  ITC_SetPriority(gUart1Int_c, gItcNormalPriority_c);
  //enable the interrupts corresponding to UART driver
  ITC_EnableInterrupt(gUart1Int_c);

  //initialize the uart
  UartOpen(UART_1,gPlatformClock_c);  
  UartSetConfig(UART_1, &pConfig);  

#if gUart1_EnableHWFlowControl_d == TRUE
  UartSetCTSThreshold(UART_1, gUart_RxFlowControlSkew_d);
#endif

  //configure the Uart Rx and Tx Threshold
  UartSetTransmitterThreshold(UART_1, 5);
  UartSetReceiverThreshold(UART_1, 5);

  //set pCallback functions
  pCallback.pfUartWriteCallback = UartEventWrite1;
  pCallback.pfUartReadCallback = UartEventRead1;
  UartSetCallbackFunctions(UART_1, &pCallback);
#endif

#if gUart_PortDefault_d == UART_2

  //initialize GPIOs for UART1 and UART2
  GpioUart2Init();  

  //configure the uart parameters
  pConfig.UartParity = gUartParityNone_c;
  pConfig.UartStopBits = gUartStopBits1_c;
  pConfig.UartBaudrate = gUartDefaultBaud_c;
  pConfig.UartFlowControlEnabled = gUart2_EnableHWFlowControl_d;

        //mount the interrupts corresponding to UART driver
  IntAssignHandler(gUart2Int_c, (IntHandlerFunc_t)UartIsr2);
  ITC_SetPriority(gUart2Int_c, gItcNormalPriority_c);
  //enable the interrupts corresponding to UART driver
  ITC_EnableInterrupt(gUart2Int_c);
  
  //initialize the uart
  UartOpen(UART_2,gPlatformClock_c);
  UartSetConfig(UART_2,&pConfig);

#if gUart2_EnableHWFlowControl_d == TRUE
  UartSetCTSThreshold(UART_2, gUart_RxFlowControlSkew_d);
#endif

  //configure the Uart Rx and Tx Threshold
  UartSetTransmitterThreshold(UART_2,5);
  UartSetReceiverThreshold(UART_2,5);

  //set pCallback functions
  pCallback.pfUartWriteCallback = UartEventWrite2;
  pCallback.pfUartReadCallback = UartEventRead2;
  UartSetCallbackFunctions(UART_2,&pCallback);

#endif /* gUart2_Enabled_d */
  // global enable interrupts in AITC driver
  IntEnableIRQ();
  
  UartReadData(gUart_PortDefault_d,mUARTRxBuffer,sizeof(mUARTRxBuffer),TRUE);

}                                       

/************************************************************************************  
* GpioUart1Init
*
* This function initializate the gpio´s for the Uart1 module
*************************************************************************************/
void GpioUart1Init(void)
{
  register uint32_t tmpReg;

  GPIO.PuSelLo |= (GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit);  // Pull-up select: UP type
  GPIO.PuEnLo  |= (GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit);  // Pull-up enable
  GPIO.InputDataSelLo &= ~(GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit); // read from pads
  GPIO.DirResetLo = (GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit); // inputs
  GPIO.DirSetLo = (GPIO_UART1_CTS_bit | GPIO_UART1_TX_bit);  // outputs
        
  tmpReg = GPIO.FuncSel0 & ~((FN_MASK << GPIO_UART1_RX_fnpos) | (FN_MASK << GPIO_UART1_TX_fnpos));
  GPIO.FuncSel0 = tmpReg | ((FN_ALT << GPIO_UART1_RX_fnpos) | (FN_ALT << GPIO_UART1_TX_fnpos));
  tmpReg = GPIO.FuncSel1 & ~((FN_MASK << GPIO_UART1_CTS_fnpos) | (FN_MASK << GPIO_UART1_RTS_fnpos));
  GPIO.FuncSel1 = tmpReg | ((FN_ALT << GPIO_UART1_CTS_fnpos) | (FN_ALT << GPIO_UART1_RTS_fnpos));
}

/************************************************************************************  
* GpioUart2Init
*
* This function initializate the gpio´s for the Uart2 module
*************************************************************************************/
void GpioUart2Init(void)
{
  register uint32_t tmpReg;

  GPIO.PuSelLo |= (GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit);  // Pull-up select: UP type
  GPIO.PuEnLo  |= (GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit);  // Pull-up enable
  GPIO.InputDataSelLo &= ~(GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit); // read from pads
  GPIO.DirResetLo = (GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit); // inputs
  GPIO.DirSetLo = (GPIO_UART2_CTS_bit | GPIO_UART2_TX_bit);  // outputs

  tmpReg = GPIO.FuncSel1 & ~((FN_MASK << GPIO_UART2_CTS_fnpos) | (FN_MASK << GPIO_UART2_RTS_fnpos)\
  | (FN_MASK << GPIO_UART2_RX_fnpos) | (FN_MASK << GPIO_UART2_TX_fnpos));
  GPIO.FuncSel1 = tmpReg | ((FN_ALT << GPIO_UART2_CTS_fnpos) | (FN_ALT << GPIO_UART2_RTS_fnpos)\
  | (FN_ALT << GPIO_UART2_RX_fnpos) | (FN_ALT << GPIO_UART2_TX_fnpos));
}

/*******************************************************************************  
* UartEventRead1 
*
* Callabck function for the reception process in the Uart1
*******************************************************************************/
void UartEventRead1(UartReadCallbackArgs_t* args)
{
  gu8SCIDataFlag = TRUE;
  gu16SCINumOfBytes = args->UartNumberBytesReceived;
  gu8SCIStatus = args->UartStatus;
}

/*******************************************************************************  
* UartEventWrite1
*
* Callback function for the transmission interruption in the Uart1.
*******************************************************************************/
void UartEventWrite1(UartWriteCallbackArgs_t* args)
{
}

/*******************************************************************************  
* UartEventRead12
*
* Callabck function for the reception process in the Uart2
*******************************************************************************/
void UartEventRead2(UartReadCallbackArgs_t* args)
{
  gu8SCIDataFlag = TRUE; 
  gu16SCINumOfBytes = args->UartNumberBytesReceived;
  gu8SCIStatus = args->UartStatus;
}

/*******************************************************************************  
* UartEventWrite2
*
* Callback function for the transmission interruption in the Uart1.
*******************************************************************************/
void UartEventWrite2(UartWriteCallbackArgs_t* args)
{
}

/*******************************************************************************  
* HexToAscii
*
* This function converts an hexadecimal value in ascii code.
*******************************************************************************/
static uint8_t HexToAscii(uint8_t u8Hex)
{
  u8Hex &= 0x0F;
  return u8Hex + ((u8Hex <= 9) ? '0' : ('A'-10));
}

/*******************************************************************************  
* Uart_putchar
*
* This function print a byte in the serial interface
*******************************************************************************/
void Uart_putchar(uint8_t u8Char)
{
  UartWriteData(gUart_PortDefault_d,&u8Char,1);
}


/*******************************************************************************  
* Uart_Print
*
* This function transmit a string to the initializated Uart. 
*******************************************************************************/
void Uart_Print(uint8_t* pString)
{
  uint8_t WriteBuffer[1];
  uint8_t length = 0;
  uint16_t u8Timeout;
  while(pString[length] != '\0'){
    WriteBuffer[0] = pString[length];
    length++;
    u8Timeout = 0;
    while((UartWriteData(gUart_PortDefault_d,WriteBuffer,1) != gUartErrNoError_c) && (u8Timeout < gUartTimeout_d))
    {
      u8Timeout ++;
    }
  }
}

/*******************************************************************************  
* Uart_getchar
*
* Waits until a character is received in the Uart. 
*******************************************************************************/
char Uart_getchar(uint8_t *u8Char)
{
  while(TRUE != gu8SCIDataFlag){}
  gu8SCIDataFlag = FALSE;
  UartGetByteFromRxBuffer(gUart_PortDefault_d, u8Char);
  
  return *u8Char;
}

/*******************************************************************************  
* Uart_Tx
*
* Transmits a buffer to the UART. 
*******************************************************************************/
void Uart_Tx(const uint8_t * msg, uint8_t length)
{
  uint8_t WriteBuffer[1];
  uint32_t i;
  uint16_t u8Timeout;

  for(i=0;i<length;i++)
  {
    if(*msg == '\n' ){
      WriteBuffer[0] = '\r';
      u8Timeout = 0;
      while((UartWriteData(gUart_PortDefault_d,WriteBuffer,1) != gUartErrNoError_c) && (u8Timeout < gUartTimeout_d))
      {
        u8Timeout ++;
      }
    }
    WriteBuffer[0] = *msg;
    
    u8Timeout = 0;
    while((UartWriteData(gUart_PortDefault_d,WriteBuffer,1) != gUartErrNoError_c) && u8Timeout < gUartTimeout_d )
    {
      u8Timeout ++;
    }
   
    msg++;
  }
}

void Uart_Poll(uint8_t *pRxBuffer)
{
  if (gu8SCIDataFlag) {
    gu8SCIDataFlag = FALSE;
    UartGetByteFromRxBuffer(gUart_PortDefault_d, pRxBuffer);
  } 
}

/*******************************************************************************  
* AsciitoHex
*
* This function converts an ascii code in a hexadecimal value.
*******************************************************************************/
uint8_t AsciitoHex(uint8_t u8Ascii)
{
  if ((u8Ascii > 47) && (u8Ascii <= 57)){
    return u8Ascii - '0';
  }
  else{ 
    if((u8Ascii > 64) && (u8Ascii <= 70))
    { 
      return (u8Ascii - 'A' + 10);
    }
    else if((u8Ascii > 96) && (u8Ascii <= 102))
    {
      return (u8Ascii - 'a' + 10);
    } 
  }
}

/*******************************************************************************  
* Uart_PrintHex
*
* This function transmit a hexadecimal value to the initializated Uart. 
*******************************************************************************/
void Uart_PrintHex(uint8_t *pu8Hex, uint8_t u8len, uint8_t u8flags)
{
  uint8_t hexString[3];
  if(! (u8flags & gPrtHexBigEndian_c))
    pu8Hex = pu8Hex + (u8len-1);
  
  while(u8len)
  {
    hexString[2] = '\0';
    hexString[1] = HexToAscii( *pu8Hex );
    hexString[0] = HexToAscii((*pu8Hex)>>4);
    
    Uart_Print((uint8_t*) hexString);
    
    if(u8flags & gPrtHexCommas_c)
      Uart_Print((uint8_t*)",");
    pu8Hex = pu8Hex + (u8flags & gPrtHexBigEndian_c ? 1 : -1);
    u8len--;
  }
  if(u8flags & gPrtHexNewLine_c)
    Uart_Print((uint8_t*)"\n");
}

/*******************************************************************************  
* Uart_PrintByteDec
*
* This function transmit a decimal value to the initializated Uart
*******************************************************************************/
void Uart_PrintByteDec(uint8_t u8Dec)
{
  uint8_t decString[3];
  uint8_t  tem;
  tem = u8Dec & 0x0F;
  decString[2] = '\0';
  decString[0]= tem/10;
  decString[1] = tem%10;

  tem = ((u8Dec>>4) & 0x0F);
  tem = tem * 16;
  
  if(decString[1] + (tem%10) >= 10){
    decString[0] = ( decString[0] + (tem/10) + 1 + '0');
    decString[1] = ( decString[1] + (tem%10) - 10 +'0');
  }
  else{
    decString[0] = ( decString[0] + (tem/10) + '0');
    decString[1] = ( decString[1] + (tem%10) + '0');
  }
  Uart_Print((uint8_t*) decString);
}

/*******************************************************************************  
* Uart_PrintShortDec
*
* This function transmit a decimal value to the initializated Uart
*******************************************************************************/
void Uart_PrintShortDec(uint16_t u16Dec)
{
  uint8_t decString[6];
  uint16_t  temp;

  decString[5] = '\0';
  temp = u16Dec;

  decString[4] = (temp%10) + '0';
  temp = temp/10;
  decString[3] = (temp%10) + '0';
  temp = temp/10;
  decString[2] = (temp%10) + '0';
  temp = temp/10;
  decString[1] = (temp%10) + '0';
  temp = temp/10;

  decString[0] = temp + '0';

  Uart_Print((uint8_t*) decString);
}

/*******************************************************************************  
* mem_cmp 
*
* This function compare two strings, return FALSE when these are diferents or 
* TRUE when are equals.
*******************************************************************************/
bool_t mem_cmp(uint8_t * str1, uint8_t * str2, uint8_t sz)
{
  uint8_t i;
  for(i=0; i<sz; i++)
  {
    if( (str1[i]) != (str2[i]))
    {
      return FALSE;
    }
  }
  return TRUE;
}

