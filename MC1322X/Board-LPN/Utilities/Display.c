/************************************************************************************
* This is the source file for Display Driver.
*
*
* (c) Copyright 2008, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#include "../../PLM/Interface/EmbeddedTypes.h"
#include "../../PLM/LibInterface/GPIO_Interface.h"
#include "../../SMAC/Drivers/Interface/Delay.h"
#include "Display.h"
#define gLcdBackLightEnabled_d 1
#if (gLCDSupported_d)



/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/* pin defines */

/* D0  = SPI_SS    (GPIO  4) , PORT LO */
/* D1  = SPI_MISO  (GPIO  5) , PORT LO */
/* D2  = SPI_MOSI  (GPIO  6) , PORT LO */
/* D3  = SPI_SCK   (GPIO  7) , PORT LO */
/* D4  = ADC2      (GPIO  32), PORT HI */
/* D5  = ADC3      (GPIO  33), PORT HI */
/* D6  = ADC4      (GPIO  34), PORT HI */
/* D7  = ADC5      (GPIO  35), PORT HI */
#define LCD_D0  gGpioPin4_c
#define LCD_D1  gGpioPin5_c
#define LCD_D2  gGpioPin6_c
#define LCD_D3  gGpioPin7_c
#define LCD_D4  gGpioPin32_c
#define LCD_D5  gGpioPin33_c
#define LCD_D6  gGpioPin34_c
#define LCD_D7  gGpioPin35_c

#define LCD_DATA_LO_MASK  0x000000F0
#define LCD_DATA_LO_PORT  gGpioPort0_c
#define LCD_DATA_LO_WRITE(x) (x)<<4
#define LCD_DATA_LO_READ(x)  (x)>>4

#define LCD_DATA_HI_MASK  0x0000000F
#define LCD_DATA_HI_PORT  gGpioPort1_c
#define LCD_DATA_HI_WRITE(x) (x)>>4
#define LCD_DATA_HI_READ(x)  (x)<<4

/* A0     = UART2_RTS (GPIO  21)   inverted, 0-data, 1-command */
/* RD     = TMR0      (GPIO  8)    double inverted, active high */
/* WR     = TMR1      (GPIO  9)    double inverted, active high */
/* CS     = UART2_CTS (GPIO  20)   double inverted, active high */
/* BL_PWM = TMR2      (GPIO  10)   1 - light, 0 - dark or PWM variable duty cycle */
/* RST    = RX_ON     (GPIO  45)   active low */
/* RST    = UART2_RX  (GPIO  19)   active low */
#define LCD_A0          gGpioPin21_c
#define LCD_RD          gGpioPin8_c
#define LCD_WR          gGpioPin9_c
#define LCD_CS          gGpioPin20_c
#define LCD_BL_PWM      gGpioPin10_c
#define LCD_RSTN        gGpioPin45_c
#define LCD_RSTN_debug  gGpioPin19_c


/* pin states */
#define LCD_A0_COMMAND  gGpioPinStateHigh_c
#define LCD_A0_DATA     gGpioPinStateLow_c
#define LCD_CS_ACTIVE   gGpioPinStateHigh_c
#define LCD_CS_INACTIVE gGpioPinStateLow_c
#define LCD_WR_ACTIVE   gGpioPinStateHigh_c
#define LCD_WR_INACTIVE gGpioPinStateLow_c
#define LCD_RD_ACTIVE   gGpioPinStateHigh_c
#define LCD_RD_INACTIVE gGpioPinStateLow_c
#define LCD_BL_ON       gGpioPinStateHigh_c
#define LCD_BL_OFF      gGpioPinStateLow_c
#define LCD_RSTN_IDLE   gGpioPinStateHigh_c
#define LCD_RSTN_RESET  gGpioPinStateLow_c

#define ON                      1
#define OFF                     0

#define mFontOffset_c          32

/* number of columns to be cleared in LCD_Task */
#define mMaxDeleteColumns_c    32
#define mMaxLineNumber_c        8
#define mMaxQueueEntries_c     10
#define mEmptyQueue_c          0xFF

/* number of characters to be displayed in LCD_Task */
#define mNumberOfCharToWrite_c  19

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
/* #pragma pack(1) */

typedef struct displayParams_tag{
  uint8_t currentXCoord;
  uint8_t currentYCoord;
  uint8_t currentLine;
  /* Bitmask: 
  *       line i is cleared, bit i from writtenLines is 0
  *       line i is written, bit i from writtenLines is 1 
  */
  uint8_t writtenLines;
  /* Bitmask: 
  *       line i will not be written, bit i from linesToWrite is 0
  *       line i will be written, bit i from linesToWrite is 1 
  */
  uint8_t linesToWrite;  
  uint8_t linesWidthWritten[mMaxLineNumber_c];
  uint8_t currentFontType;
  uint8_t currentStringIndex;
  uint8_t currentString[mMaxLineNumber_c][gMAX_LCD_CHARS_c];
}displayParams_t;

typedef struct lineParams_tag{
  uint8_t xCoord;
  uint8_t yCoord;
  uint8_t page;  
}lineParams_t;

typedef struct pageParams_tag{
  uint8_t xStartCoord;
  uint8_t xEndCoord;
  uint8_t yCoord;
  uint8_t line;
}pageParams_t;

typedef enum operation_tag{
  gLCD_WriteOp_c = 0,
  gLCD_ClearDisplayOp_c,
  gLCD_ClearLineOp_c,
  gLCD_MaxOp_c = 0xFF
}operation_t;

typedef struct opLine_tag{
  operation_t opQueue;
  uint8_t lineQueue;  
}opLine_t;

typedef struct queue_tag{
  uint8_t head;
  uint8_t tail;
  opLine_t opLine[mMaxQueueEntries_c];
}queue_t;

/* #pragma pack() */

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

void LCD_SetBacklight(uint8_t brightnessValue);
void LCD_ClearPage(uint8_t startXCoord, uint8_t endXCoord, uint8_t yCoord);
uint8_t LCD_ClearLine(uint8_t line);
static void SendData(uint8_t data);
static void SetHiZ(void);
static void SetOutput(void);
static void InitDisplayWrite(uint8_t xStartCoord, uint8_t yStartCoord);
static uint8_t ReadData(void);
static uint8_t GetData(void);
static uint8_t GetStrlen(const uint8_t *pString);


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static displayParams_t mDisplayParams = {
  /* currentXCoord */
  0x00,
  /* currentYCoord */
  0x00,
  /* currentLine */
  0x00,
  /* writtenLines */
  0xFF,
  /* linesToWrite */
  0x00,
  /* linesWidthWritten */
  0x00,
  /* currentFontType */
  0x00,
  /* currentStringIndex */
  0x00,  
  /* currentString */
  0x00
};

static lineParams_t mLineParams[mMaxLineNumber_c] = {
  {0, 0, 0},
  {0, 8, 1},
  {0, 16, 2},
  {0, 24, 3},
  {0, 32, 4},
  {0, 40, 5},
  {0, 48, 6},
  {0, 56, 7},
};  


/* after initialization, all LCD lines must be erased */
static bool_t mIsLcdInitialized = FALSE;

/* error indicator, used for error reporting */
static volatile lcdError_t mErrorIndicator = gLCD_NoError_c;

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/

const uint8_t gaHexValue[] = "0123456789ABCDEF";

const font_t aLCDFont[gLCDNumFonts_c] = 
{
  /* character font with fixed width */ 
  {gLCDNormalIndex,  gLCDNormalFont,  1},
  /* character font with variable width */ 
  {gLCDVerdanaIndex, gLCDVerdanaFont, 2}   
};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/******************************************************************************
* This function initializes the display
*
* Interface assumptions:
*
*
* Return value:
* None
*
******************************************************************************/
void LCD_Init(void)
{

    Gpio_SetPinFunction(LCD_WR, gGpioNormalMode_c);
    Gpio_SetPinFunction(LCD_CS, gGpioNormalMode_c);
    Gpio_SetPinFunction(LCD_RD, gGpioNormalMode_c);
    Gpio_SetPinFunction(LCD_A0, gGpioNormalMode_c);
    Gpio_SetPinFunction(LCD_RSTN, gGpioNormalMode_c);
    Gpio_SetPinFunction(LCD_RSTN_debug, gGpioNormalMode_c);

    Gpio_SetPinData(LCD_WR, LCD_WR_INACTIVE);
    Gpio_SetPinData(LCD_CS, LCD_CS_INACTIVE);
    Gpio_SetPinData(LCD_RD, LCD_RD_INACTIVE);
    Gpio_SetPinData(LCD_A0, LCD_A0_COMMAND);
    Gpio_SetPinData(LCD_RSTN, LCD_RSTN_IDLE);
    Gpio_SetPinData(LCD_RSTN_debug, LCD_RSTN_IDLE);

    /* define the LCD pins as output pins */
    (void)Gpio_SetPinReadSource(LCD_CS,     gGpioPinReadReg_c);
    (void)Gpio_SetPinReadSource(LCD_A0,     gGpioPinReadReg_c);
    (void)Gpio_SetPinReadSource(LCD_RD,     gGpioPinReadReg_c);
    (void)Gpio_SetPinReadSource(LCD_WR,     gGpioPinReadReg_c);
    (void)Gpio_SetPinReadSource(LCD_BL_PWM, gGpioPinReadReg_c);
    (void)Gpio_SetPinReadSource(LCD_RSTN,   gGpioPinReadReg_c);
    (void)Gpio_SetPinReadSource(LCD_RSTN_debug,   gGpioPinReadReg_c);

    (void)Gpio_SetPinDir(LCD_CS,     gGpioDirOut_c);
    (void)Gpio_SetPinDir(LCD_A0,     gGpioDirOut_c);
    (void)Gpio_SetPinDir(LCD_RD,     gGpioDirOut_c);
    (void)Gpio_SetPinDir(LCD_WR,     gGpioDirOut_c);
    (void)Gpio_SetPinDir(LCD_BL_PWM, gGpioDirOut_c);
    (void)Gpio_SetPinDir(LCD_RSTN,   gGpioDirOut_c);
    (void)Gpio_SetPinDir(LCD_RSTN_debug,   gGpioDirOut_c);

    /* data pins */
#if gLCDGpioPinMode_c
    (void)Gpio_SetPinFunction(LCD_D0,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D1,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D2,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D3,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D4,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D5,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D6,      gGpioNormalMode_c);
    (void)Gpio_SetPinFunction(LCD_D7,      gGpioNormalMode_c);

    /* define data pins as inputs */
    (void)Gpio_EnPinPullup(LCD_D0,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D1,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D2,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D3,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D4,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D5,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D6,      TRUE);
    (void)Gpio_EnPinPullup(LCD_D7,      TRUE);

    (void)Gpio_SelectPinPullup(LCD_D0,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D1,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D2,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D3,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D4,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D5,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D6,      gGpioPinPullup_c);
    (void)Gpio_SelectPinPullup(LCD_D7,      gGpioPinPullup_c);

    (void)Gpio_EnPinPuKeeper(LCD_D0,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D1,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D2,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D3,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D4,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D5,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D6,      TRUE);
    (void)Gpio_EnPinPuKeeper(LCD_D7,      TRUE);

    (void)Gpio_SetPinReadSource(LCD_D0,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D1,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D2,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D3,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D4,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D5,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D6,      gGpioPinReadPad_c);
    (void)Gpio_SetPinReadSource(LCD_D7,      gGpioPinReadPad_c);

    (void)Gpio_SetPinDir(LCD_D0,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D1,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D2,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D3,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D4,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D5,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D6,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D7,      gGpioDirIn_c);
#else /* gLCDGpioPinMode_c */
    Gpio_SetPortFunction(LCD_DATA_LO_PORT, gGpioNormalMode_c, LCD_DATA_LO_MASK);
    Gpio_SetPortFunction(LCD_DATA_HI_PORT, gGpioNormalMode_c, LCD_DATA_HI_MASK);

    Gpio_WrPortSetting(LCD_DATA_LO_PORT, gGpioPullUpEnAttr_c, LCD_DATA_LO_MASK, LCD_DATA_LO_MASK);
    Gpio_WrPortSetting(LCD_DATA_HI_PORT, gGpioPullUpEnAttr_c, LCD_DATA_HI_MASK, LCD_DATA_HI_MASK);

    Gpio_WrPortSetting(LCD_DATA_LO_PORT, gGpioPullUpSelAttr_c, LCD_DATA_LO_MASK, LCD_DATA_LO_MASK);
    Gpio_WrPortSetting(LCD_DATA_HI_PORT, gGpioPullUpSelAttr_c, LCD_DATA_HI_MASK, LCD_DATA_HI_MASK);

    Gpio_WrPortSetting(LCD_DATA_LO_PORT, gGpioPullUpKeepAttr_c, LCD_DATA_LO_MASK, LCD_DATA_LO_MASK);
    Gpio_WrPortSetting(LCD_DATA_HI_PORT, gGpioPullUpKeepAttr_c, LCD_DATA_HI_MASK, LCD_DATA_HI_MASK);
    
    Gpio_WrPortSetting(LCD_DATA_LO_PORT, gGpioInputDataSelAttr_c, ~LCD_DATA_LO_MASK, LCD_DATA_LO_MASK);
    Gpio_WrPortSetting(LCD_DATA_HI_PORT, gGpioInputDataSelAttr_c, ~LCD_DATA_HI_MASK, LCD_DATA_HI_MASK);
    
    Gpio_SetPortDir(LCD_DATA_LO_PORT, ~LCD_DATA_LO_MASK, LCD_DATA_LO_MASK);
    Gpio_SetPortDir(LCD_DATA_HI_PORT, ~LCD_DATA_HI_MASK, LCD_DATA_HI_MASK);
#endif /* gLCDGpioPinMode_c */


  Gpio_SetPinData(LCD_RSTN, LCD_RSTN_RESET);
  Gpio_SetPinData(LCD_RSTN_debug, LCD_RSTN_RESET);
  DelayUs(10);
  Gpio_SetPinData(LCD_RSTN, LCD_RSTN_IDLE);
  Gpio_SetPinData(LCD_RSTN_debug, LCD_RSTN_IDLE);
  DelayMs(1);

    /* Turn ON the VDD-VSS power keeping the RES pin = LOW for 0.5us */
    /* When power is stabilized, relese the RES pin (RES pin = HIGH) */
    
    /* Default initialized state: 
     - Display off
	   - Normal display (COM0 --> COM63)
	   - ADC select: normal (RAM column address n <-> SEGn)
	   - Power control register: Booster circuit OFF, Voltage regulator circuit OFF, Voltage follower circuit OFF
	   - LCD power supply bias rate = 1/9
	   - All-indicator lamps-on OFF
	   - Power saving clear
	   - V5 voltage regulator internal resistors Ra and Rb separation
	   - Output conditions of SEG and COM terminals SEG:V2/V3; COM: V1/V4
	   - Read modify write OFF
	   - Static indicator OFF
	   - Display start line set to first line
	   - Column address set to Address 0
	   - Page address set to Page 0
	   - Common output status normal
	   - V5 voltage regulator internal resistor ratio set mode clear
	   - Electronic volume register set mode clear
	   - Test mode clear 
    */

    /* reset display */
    LCD_WriteCommand(gLCDResetCommand_c);
    DelayUs(10);

    /* set LCD bias to 1/9 */
    LCD_WriteCommand(gLCD19BiasCommand_c);

    /* set ADC selection to normal: RAM column address n <-> SEGn */
    /* LCD_WriteCommand(gLCDADCNormalCommand_c); */
    LCD_WriteCommand(gLCDADCReverseCommand_c);

    /* set scan direction of the COM to normal: COM0 --> COM63 */
    LCD_WriteCommand(gLCDComNormalCommand_c);

    /* set the Rb/Ra ratio for V5 to medium (D2D1D0 = 0b100) */
    LCD_WriteCommand( gLCDDefaultRatioCommand_c | (gLCDRatioValue_c & 0x07) );

    /* enable the electronic volume register set command */
    /* this command must always be followed by an electronic volume register set command */
    LCD_WriteCommand(gLCDElectronicVolumeModeSetCommand_c);
    
    LCD_WriteCommand(gLCDElectronicVolumeInitCommand_c);


    /* disable booster circuit, disable voltage regulator circuit, disable voltage follower circuit */
    LCD_WriteCommand(0x2F);
    DelayUs(10);
    /** turn ON */
    LCD_WriteCommand(gLCDDisplayOnCommand_c);
    DelayUs(10);
    
    /* turn on the backlight */
    LCD_SetBacklight(gLCDDefaultBrightnessValue_c);
    
    /* set default font */
    (void)LCD_SetFont(gLCD_NormalFont_c);

    mIsLcdInitialized = FALSE;
}

/******************************************************************************
* This function clears the display. In case an error occured during add to queue
* operation, the clear display operation will not execute.
*
* Interface assumptions:
*
*
* Return value:
* None
*
******************************************************************************/
void LCD_ClearDisplay(void)
{

  static uint8_t linesToClear = 0x00;
  uint8_t line, deleteWriteLines = 0xFF;
  
  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;
  
    
 /* determine the lines that were written and need to be cleared */   
  linesToClear = mDisplayParams.writtenLines;  
  if(mIsLcdInitialized == TRUE)
  {
 /* determine if there are lines that need to be erased and after need to be written */
    deleteWriteLines = linesToClear & mDisplayParams.linesToWrite;
    if(deleteWriteLines)
    { /* if there are */
     /* mark that lines as deleted and jump over them */
      linesToClear &= ~(deleteWriteLines);  
    }
  }
   line = 0;
   /* find the first written line */
   
  do
  {
    while((!((linesToClear>>line)&0x01))&&(line<mMaxLineNumber_c))
    {
      line++;
    }
   
    if(line!= mMaxLineNumber_c)
    {
     /* in case at least a line is written */
      if(TRUE == LCD_ClearLine(line))
      {
       /* mark line as cleared */
        mDisplayParams.writtenLines &= ~(1 << line);
        linesToClear &= ~(1 << line);
       /* check if there are other lines written */
      }
    }
  }while(line!= mMaxLineNumber_c);
}

/******************************************************************************
* This function writes a string to the display. In case an error occured 
* during add to queue operation, the write string operation will not execute.
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/

void LCD_WriteString_NormalFont(uint8_t line, uint8_t *pstr){
 
 uint8_t len; 
 mDisplayParams.currentXCoord = mLineParams[line].xCoord;;  
 mDisplayParams.currentYCoord = mLineParams[line].yCoord;   
 len = GetStrlen(pstr);
 
 for(uint8_t i=0; ((i< len) && (i < gMAX_LCD_CHARS_c)); i++)
 {
   LCD_WriteCharacter(*pstr,mDisplayParams.currentXCoord, mDisplayParams.currentYCoord);   
   mDisplayParams.currentXCoord += 6;
  pstr++;
 }
}

/******************************************************************************
* This function write a string and a value in decimal or hexdecimal
* to the display
*
* Interface assumptions:
* The pstr must be zero-terminated.
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteStringValue
  (
  uint8_t *pstr,     /* IN: Pointer to text string */
  uint16_t value,    /* IN: Value */
  uint8_t line, 	   /* IN: Line in display */
  LCD_t numberFormat /* IN: Value to show in HEX or DEC */
  )
{

  int16_t divDec=10000, divHex=16;
  uint8_t loop=5, i=0,counter=0, aH[6], aHex[6];
  uint8_t aDec[6], aString[17];
  
  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;

 if(numberFormat == gLCD_DecFormat_c) {
    if(value < 100) {
      loop = 2;
      divDec = 10;
    }
    else if(value >= 100 && value <1000) {
      loop = 3;
      divDec = 100;
    }
    else if(value >= 1000 && value <9999) {
      loop = 4;
      divDec = 1000;
    }

    for(i=0; i<loop; i++) {
      if((value/divDec)!= 0) {
        aDec[counter++] = (value/divDec) + 0x30;
        value = value % divDec;
      }
      else {
        aDec[counter++] = 0x30;
      }
      divDec = divDec/10;
    }
    aDec[counter]='\0';
    counter=0;
    while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=*pstr;
      pstr++;
    }
    i=0;
    while (aDec[i] != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=aDec[i++];
    }
    aString[counter]='\0';
    LCD_WriteString_NormalFont( line, aString );
  }
  else if(numberFormat == gLCD_HexFormat_c) {
    do{
      aH[i]=gaHexValue[value % divHex];
      value=value / divHex;
      i++;
    }
    while(value > 15);
    aH[i]=gaHexValue[value];
    counter=0;
    while(i > 0){
      aHex[counter++]=aH[i--];
    }

    aHex[counter++]=aH[0];
    aHex[counter]='\0';

    counter=0;
    while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=*pstr;
      pstr++;
    }
    i=0;
    while (aHex[i] != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=aHex[i++];
    }
    aString[counter]='\0';
    LCD_WriteString_NormalFont( line, aString );
  }
  else {
    LCD_WriteString_NormalFont( line, "Format unknow" );
  }
}

void LCD_WriteStringDecValue
  (
  uint8_t *pstr,     /* IN: Pointer to text string */
  uint16_t value,    /* IN: Value */
  uint8_t line, 	   /* IN: Line in display */
  LCD_t numberFormat /* IN: Value to show in HEX or DEC */
  )
{

  int16_t divDec=10000, divHex=16;
  uint8_t loop=5, i=0,counter=0, aH[6], aHex[6];
  uint8_t aDec[6], aString[17] = {0};
  uint8_t u8TempVal;
  uint8_t u8Count;
 
  
  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;

 if(numberFormat == gLCD_DecFormat_c) {
    u8TempVal = (value >> 8);
    
    for( u8Count= 0;  u8Count < 2;  u8Count++)
    {
      if(u8TempVal < 100) 
      {
        loop = 2;
        divDec = 10;
      }
      else if(u8TempVal >= 100 && u8TempVal < 255) {
        loop = 3;
        divDec = 100;
      }
      for(i=0; i<loop; i++) {
        if((u8TempVal/divDec)!= 0) 
        {
          aDec[counter++] = (u8TempVal/divDec) + 0x30;
          u8TempVal = u8TempVal % divDec;
        }
        else {
          aDec[counter++] = 0x30;
        }
        divDec = divDec/10;
      }
      if(u8Count < 1)
      {  
        aDec[counter++] = '.';
      }
      u8TempVal = value;
    } 
      
    aDec[counter]='\0';
    counter=0;
    
    while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c ) 
    {
      aString[counter++]=*pstr;
      pstr++;
    }
    i=0;

    while (aDec[i] != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=aDec[i++];
    }
    LCD_WriteString_NormalFont( line, aString );
  }
 
  else if(numberFormat == gLCD_HexFormat_c) {
    do{
      aH[i]=gaHexValue[value % divHex];
      value=value / divHex;
      i++;
    }
    while(value > 15);
    aH[i]=gaHexValue[value];
    counter=0;
    while(i > 0){
      aHex[counter++]=aH[i--];
    }

    aHex[counter++]=aH[0];
    aHex[counter]='\0';

    counter=0;
    while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=*pstr;
      pstr++;
    }
    i=0;
    while (aHex[i] != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=aHex[i++];
    }
    aString[counter]='\0';
    LCD_WriteString_NormalFont( line, aString );
  }
  else {
    LCD_WriteString_NormalFont( line,"Format unknow"); 
  }
}



/******************************************************************************
* This functions allows to write raw bytes to the LCD, the maximum number of bytes
*	capable per line is 8, this functions transfors every hex simbol in a byte to a char.
*
* Interface assumptions:
*	IN: The pointer to the label to print with the bytes.
*	IN: The bytes to print.
*	IN: The line in the LCD where the bytes with the label.
*	IN: The number of bytes to print in the LCD.
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteBytes
(
	uint8_t   *pstr,   /* IN: The pointer to the label to print with the bytes. */
	uint8_t   *value,  /* IN: The bytes to print in hex. */
	uint8_t   line,    /* IN: The line in the LCD where the bytes with the label. */
	uint8_t   length   /* IN: The number of bytes to print in the LCD. */
)
{

	uint8_t i=0,counter=0, cIndex,auxIndex;
	uint8_t aString[17];

	uint8_t  hexIndex;
	uint8_t aHex[gMAX_LCD_CHARS_c]={'S','i','z','e',' ','N','o','t',' ','V','a','l','i','d','*','*'};

  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;
  
	counter=0;
	while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c )
	{
		aString[counter++]=*pstr;
		pstr++;
	}
	if ((((length*2)+counter) <= gMAX_LCD_CHARS_c) && ((length*2) > 0))
	{
		for (cIndex =0,auxIndex = 0; cIndex < length; cIndex++,auxIndex+=2)
		{
			hexIndex = value[cIndex]&0xf0;
			hexIndex = hexIndex>>4;
			aHex[auxIndex] = gaHexValue[hexIndex];
			hexIndex = value[cIndex] & 0x0f;
			aHex[auxIndex + 1] = gaHexValue[hexIndex];
		}
		aHex[(length * 2)]='\0';
	}
	else
		counter = 0;

	i=0;
	while (aHex[i] != '\0' && counter <gMAX_LCD_CHARS_c )
		aString[counter++]=aHex[i++];

	aString[counter]='\0';
	LCD_WriteString( line, aString );
}

/******************************************************************************
* This function sends a command to the display controller
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteCommand(uint8_t command)
{
  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;
  
  Gpio_SetPinData(LCD_A0, LCD_A0_COMMAND);
  SetOutput();
  Gpio_SetPinData(LCD_CS, LCD_CS_ACTIVE);
  Gpio_SetPinData(LCD_WR, LCD_WR_ACTIVE);
  SendData(command);
  DelayUs(1);
  Gpio_SetPinData(LCD_WR, LCD_WR_INACTIVE);
  Gpio_SetPinData(LCD_CS, LCD_CS_INACTIVE);
  SetHiZ();
  DelayUs(1);
}

/******************************************************************************
* This function sends a data to the display controller
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteData(uint8_t data)
{
  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;

  Gpio_SetPinData(LCD_A0, LCD_A0_DATA);
  Gpio_SetPinData(LCD_CS, LCD_CS_ACTIVE);
  SetOutput();
  SendData(data);
  Gpio_SetPinData(LCD_WR, LCD_WR_ACTIVE);
  DelayUs(1);
  Gpio_SetPinData(LCD_WR, LCD_WR_INACTIVE);
  Gpio_SetPinData(LCD_CS, LCD_CS_INACTIVE);
  SetHiZ();
  DelayUs(1);  
 
}

/******************************************************************************
* This function sets the font received as parameter as default font
*
* Interface assumptions:
*
*
* Return value:
*      TRUE:  when the font is correctly set
*      FALSE: when an unsupported font is received as parameter
* char
*
*
******************************************************************************/
bool_t LCD_SetFont(lcdFontType_t font)
{
  /* clear the error indicator */
  mErrorIndicator = gLCD_NoError_c;

  if(gLCDNumFonts_c > font)
  {
    mDisplayParams.currentFontType = font;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/******************************************************************************
* This function checks if an error occured during an interface function call.
* User must always call this function to verify if an interface function executed 
* successufully.
*
* Interface assumptions:
*
*
* Return value:  lcdError_t
*      gLCD_NoError_c:   no error
*      gLCD_QueueFull_c: queue is full
*
*
******************************************************************************/
lcdError_t LCD_CheckError(void)
{
  return mErrorIndicator;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/


/******************************************************************************
* This function enables/disables the backlight
*
* Interface assumptions:
*   0 - disable backlight
*   >0 - enables backlight
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_SetBacklight(uint8_t brightnessValue)
{
    GpioPinState_t backLightState;
    backLightState = (0 != brightnessValue) ? gGpioPinStateHigh_c : gGpioPinStateLow_c;
    Gpio_SetPinData(LCD_BL_PWM,  backLightState);
}

/******************************************************************************
* This function sets/ clears the pixel from (XStartCoord, YStartCoord)
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WritePixel(uint8_t xStartCoord, uint8_t yStartCoord, bool_t mode)
{
  uint8_t page;
  uint8_t read;

  if (xStartCoord>=gLCDMaxColumns_c || yStartCoord>=gLCDMaxLines_c)
  {
    return; /* if pixel out of screen exit */
  }

  page = yStartCoord >> 3;
  yStartCoord %= 8;

  LCD_WriteCommand(gLCDDisplayStartLineDefaultCommand_c);
  LCD_WriteCommand(gLCDPageAddressDefaultCommand_c + page);
  LCD_WriteCommand(gLCDNibble0ColumnAddressDefaultCommand_c | ((xStartCoord & 0xF0)>>4));
  LCD_WriteCommand(gLCDNibble1ColumnAddressDefaultCommand_c | (xStartCoord & 0x0F));       

  read = ReadData(); // Dummy read
  read = ReadData();

  LCD_WriteCommand(gLCDDisplayStartLineDefaultCommand_c);
  LCD_WriteCommand(gLCDPageAddressDefaultCommand_c + page);
  LCD_WriteCommand(gLCDNibble0ColumnAddressDefaultCommand_c | ((xStartCoord & 0xF0)>>4));
  LCD_WriteCommand(gLCDNibble1ColumnAddressDefaultCommand_c | (xStartCoord & 0x0F));

  if( TRUE == mode )
  {
    LCD_WriteData((read | (1 << yStartCoord)));
  }
  else
  {  
    LCD_WriteData((read & ~(1 << yStartCoord)));  
  }
}

/******************************************************************************
* This function displays a character starting with xCoord and yCoord
*
* Interface assumptions:
*  yCoord is a multiple of 8
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteCharacter(uint8_t symbol, uint8_t xCoord, uint8_t yCoord)
{
  uint16_t arrayLocation=0; 
  uint8_t i, maxLength;
 
  /* if pixel out of screen, exit */
  if((xCoord >= gLCDMaxColumns_c) && (yCoord >= gLCDMaxLines_c))
  {
    return;
  }
    
  arrayLocation = symbol - mFontOffset_c;
  maxLength = aLCDFont[mDisplayParams.currentFontType].fontHeight;
  /* each character has: 
  *  (aLCDFont[mDisplayParams.currentFontType].pFontIndex[arrayLocation+1] - 
  *   aLCDFont[mDisplayParams.currentFontType].pFontIndex[arrayLocation]) pixels as width 
  *  (aLCDFont[mDisplayParams.currentFontType].fontHeight * 8) pixels as height 
  */
  while(maxLength > 0)
  {
    maxLength--;      
    InitDisplayWrite(xCoord,yCoord);        
    /* mark line as written */
    mDisplayParams.writtenLines |= (1<<(yCoord>>3));   
    for(i = 0;
        i < ((aLCDFont[mDisplayParams.currentFontType].pFontIndexTable[arrayLocation+1] - 
              aLCDFont[mDisplayParams.currentFontType].pFontIndexTable[arrayLocation]) *
              aLCDFont[mDisplayParams.currentFontType].fontHeight);
        i += aLCDFont[mDisplayParams.currentFontType].fontHeight)        
    {
      /* in case the character is representd on multiple lines,
       * (aLCDFont[mDisplayParams.currentFontType].fontHeight > 1),
       * display first the upper part of the character and after the lower part 
       */
      LCD_WriteData(aLCDFont[mDisplayParams.currentFontType].pFontCharTable[aLCDFont[mDisplayParams.currentFontType].pFontIndexTable[arrayLocation]*aLCDFont[mDisplayParams.currentFontType].fontHeight + i + maxLength]);
    }
    yCoord += 8;
  }    
}

/****************************************************************************************
* This function clears the pixels between startXCoord and endXCoord within a single page
* (page located at yCoord)
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_ClearPage(uint8_t startXCoord, uint8_t endXCoord, uint8_t yCoord)
{
  uint8_t i;
  
  InitDisplayWrite(startXCoord, yCoord);
  for(i = startXCoord; i < endXCoord; i++)
  {
    LCD_WriteData(OFF);    
  }    
}

/****************************************************************************************
* This function clears an entire LCD line
*
* Interface assumptions:
*     char line = 0..7
*
*
* Return value:
*     TRUE if the line was cleared
*     FALSE if the line is not cleared yet
*     0xFF  if the line parameter is not valid
* None
*
*
******************************************************************************/
uint8_t LCD_ClearLine(uint8_t line)
{
  static pageParams_t pageToDelete;
  static bool_t isPageDeleted = TRUE;
  
  if(line < gLCDMaxRamDataPages_c)
  {
    /* save the page coordinates */
    if(isPageDeleted)
    {
      pageToDelete.xStartCoord = mLineParams[line].xCoord;
      pageToDelete.xEndCoord   = gLCDMaxColumns_c;
      pageToDelete.yCoord      = mLineParams[line].yCoord;
      pageToDelete.line        = line; 
    }
    
    /* delete only mMaxDeleteColumns_c columns once because of time constraints */
    if(((pageToDelete.xEndCoord - pageToDelete.xStartCoord) >= mMaxDeleteColumns_c))
    {
      /* the page will be deleted with (gLCDMaxColumns_c/mMaxDeleteColumns_c) consecutive operations */
      isPageDeleted            = FALSE;
      LCD_ClearPage(pageToDelete.xStartCoord, 
                    pageToDelete.xStartCoord + mMaxDeleteColumns_c, 
                    pageToDelete.yCoord); 
      /* compute the new xStartCoord for the current page */
      pageToDelete.xStartCoord = pageToDelete.xStartCoord + mMaxDeleteColumns_c;        
      return FALSE;  
    }
    else
    {    
      LCD_ClearPage(pageToDelete.xStartCoord, 
                    pageToDelete.xEndCoord, 
                    pageToDelete.yCoord);
      isPageDeleted            = TRUE;
      return TRUE;  
    } 
  }
  else
  {
    return 0xFF;
  }
}

/******************************************************************************
* This function process an LCD operation (clear, write string).
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/


/******************************************************************************
* This function sets a certain value to the LCD DATA pins
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
static void SendData(uint8_t data)
{
#if gLCDGpioPinMode_c
    /* bit |7|6|5|4|3|2|1|0| */
    (void)Gpio_SetPinData(LCD_D0, (data & 0x01)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D1, (data & 0x02)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D2, (data & 0x04)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D3, (data & 0x08)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D4, (data & 0x10)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D5, (data & 0x20)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D6, (data & 0x40)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
    (void)Gpio_SetPinData(LCD_D7, (data & 0x80)? gGpioPinStateHigh_c : gGpioPinStateLow_c);
#else /* gLCDGpioPinMode_c */    
    Gpio_SetPortData(LCD_DATA_LO_PORT, LCD_DATA_LO_WRITE(data), LCD_DATA_LO_MASK); 
    Gpio_SetPortData(LCD_DATA_HI_PORT, LCD_DATA_HI_WRITE(data), LCD_DATA_HI_MASK);
    
#endif /* gLCDGpioPinMode_c */     
}

/******************************************************************************
* This function sets the LCD DATA pins as inputs
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
static void SetHiZ(void)
{
    /* change data pins direction back to input */
#if gLCDGpioPinMode_c
    (void)Gpio_SetPinDir(LCD_D0,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D1,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D2,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D3,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D4,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D5,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D6,      gGpioDirIn_c);
    (void)Gpio_SetPinDir(LCD_D7,      gGpioDirIn_c);
#else /* gLCDGpioPinMode_c */ 
    Gpio_SetPortDir(LCD_DATA_LO_PORT, ~LCD_DATA_LO_MASK, LCD_DATA_LO_MASK); 
    Gpio_SetPortDir(LCD_DATA_HI_PORT, ~LCD_DATA_HI_MASK, LCD_DATA_HI_MASK); 
    
#endif /* gLCDGpioPinMode_c */

}

/******************************************************************************
* This function sets the LCD DATA pins as outputs
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
static void SetOutput(void)
{
  /* change data pins direction to output */
#if gLCDGpioPinMode_c
  (void)Gpio_SetPinDir(LCD_D0,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D1,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D2,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D3,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D4,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D5,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D6,      gGpioDirOut_c);
  (void)Gpio_SetPinDir(LCD_D7,      gGpioDirOut_c);
#else /* gLCDGpioPinMode_c */
   
    Gpio_SetPortDir(LCD_DATA_LO_PORT, LCD_DATA_LO_MASK, LCD_DATA_LO_MASK);      
    Gpio_SetPortDir(LCD_DATA_HI_PORT, LCD_DATA_HI_MASK, LCD_DATA_HI_MASK);         
    
#endif /* gLCDGpioPinMode_c */

}

/******************************************************************************
* This function initilizes the display for a data write (line, page, column)
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
static void InitDisplayWrite(uint8_t xStartCoord, uint8_t yStartCoord)
{

  uint8_t page;  
  
  page = yStartCoord >> 3;

  LCD_WriteCommand(gLCDDisplayStartLineDefaultCommand_c);
  LCD_WriteCommand(gLCDPageAddressDefaultCommand_c + page);
  LCD_WriteCommand(gLCDNibble0ColumnAddressDefaultCommand_c | ((xStartCoord & 0xF0)>>4));
  LCD_WriteCommand(gLCDNibble1ColumnAddressDefaultCommand_c | (xStartCoord & 0x0F));         
}

/******************************************************************************
* This function reads the current location from display data RAM
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
static uint8_t ReadData(void)
{
  uint8_t ReturnValue;
  Gpio_SetPinData(LCD_A0, LCD_A0_DATA);
  Gpio_SetPinData(LCD_CS, LCD_CS_ACTIVE);
  DelayUs(1);
  Gpio_SetPinData(LCD_RD, LCD_RD_ACTIVE);
  DelayUs(1);
  ReturnValue = GetData();
  Gpio_SetPinData(LCD_RD, LCD_RD_INACTIVE);
  Gpio_SetPinData(LCD_CS, LCD_CS_INACTIVE);
  DelayUs(1);
  return ReturnValue;
}

/******************************************************************************
* This function sets the data from the current RAM display location
* to LCD DATA pins
*
* Interface assumptions:
*
*
* Return value:
* unsigned 8 bit
*
*
******************************************************************************/
static uint8_t GetData(void)
{
    uint8_t        ReturnValue = 0;
#if gLCDGpioPinMode_c
    GpioPinState_t Bitdata;
    /* bit |7|6|5|4|3|2|1|0| */
    Gpio_GetPinData(LCD_D7, &Bitdata);
    ReturnValue = (Bitdata)? (1<<7) : 0;
    Gpio_GetPinData(LCD_D6, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<6) : 0;
    Gpio_GetPinData(LCD_D5, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<5) : 0;
    Gpio_GetPinData(LCD_D4, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<4) : 0;
    Gpio_GetPinData(LCD_D3, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<3) : 0;
    Gpio_GetPinData(LCD_D2, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<2) : 0;
    Gpio_GetPinData(LCD_D1, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<1) : 0;
    Gpio_GetPinData(LCD_D0, &Bitdata);
    ReturnValue |= (Bitdata)? (1<<0) : 0;
    
#else /* gLCDGpioPinMode_c */
    uint32_t temp;
    Gpio_GetPortData(LCD_DATA_LO_PORT, &temp);
    ReturnValue = LCD_DATA_LO_READ(LCD_DATA_LO_MASK & temp);
    Gpio_GetPortData(LCD_DATA_HI_PORT, &temp);
    ReturnValue |= LCD_DATA_HI_READ(LCD_DATA_HI_MASK & temp);

#endif /* gLCDGpioPinMode_c */
    
    return ReturnValue;
}

/******************************************************************************
* This function gets the length of a string and return the length
*
* Interface assumptions:
*
*
* Return value:
* char
*
*
******************************************************************************/
static uint8_t GetStrlen
  (
  const uint8_t *pString /* IN: Pointer to text string */
  )
{
  int8_t count=0, length=0;

  while (*pString != '\0' && count <gMAX_LCD_CHARS_c ) {
    count++;
    length++;
    pString++;
  }

  /* Check boundries */
  if ( length > gMAX_LCD_CHARS_c ) {
    length = gMAX_LCD_CHARS_c;
  }

  return length;
}

/******************************************************************************
* This function draws a 24x24 pixels Icon
*
* Interface assumptions:
*
*
* Return value:
* char
*
*
******************************************************************************/
void LCD_DrawIcon(uint8_t u8XCoord, uint8_t u8YPage, uint8_t * pu8Icon)
{
  uint8_t u8i;
  uint8_t u8j;
  
  if (u8XCoord >= gLCDMaxColumns_c || u8YPage >= gLCDMaxRamDataPages_c)
  {
    return; /* if pixel out of screen exit */
  }

  for(u8i = 0; u8i < 3; u8i++)
  {
    LCD_WriteCommand(gLCDDisplayStartLineDefaultCommand_c);
    LCD_WriteCommand(gLCDPageAddressDefaultCommand_c + u8YPage++);
    LCD_WriteCommand(gLCDNibble0ColumnAddressDefaultCommand_c | ((u8XCoord & 0xF0)>>4));
    LCD_WriteCommand(gLCDNibble1ColumnAddressDefaultCommand_c | (u8XCoord & 0x0F));
    for(u8j = 0; u8j < 24; u8j++)
    {
      LCD_WriteData(*pu8Icon); 
      pu8Icon++;
    }
  }
  
}

#endif /* (gLCDSupported_d == 1) */
