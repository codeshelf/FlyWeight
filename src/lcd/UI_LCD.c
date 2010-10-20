/******************** (C) COPYRIGHT 2009 Newhaven Display ********************
* File Name          : UI_LCD.c
* Author             : Curt Lagerstam
* Date First Issued  : 7/10/2009
* Description        : This file includes the LCD driver for NHD-C0216CZ.
********************************************************************************
* History:
* mm/dd/yyyy
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ui_lcd.h"
//#include "FreeRTOS.h"
//#include "task.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void delay(int time)
{
  int i,j;
  for(i=0;i<(time^1000);i++){
  	for(j=0;j<(time^1000);j++){;}}
}
/*******************************************************************************
* Function Name  : UILCD_Init
* Description    : Initializes LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_Init(void)
{
/* Configure the LCD Control pins --------------------------------------------*/
  delay(200);
  GPIO_WriteBit(GPIOC, CtrlPin_UIRS, Bit_SET);
  GPIO_WriteBit(GPIOC, CtrlPin_UICSB, Bit_SET);
  delay(200);
  GPIO_WriteBit(GPIOC, CtrlPin_UISCL, Bit_SET);
  GPIO_WriteBit(GPIOC, CtrlPin_UISDA, Bit_SET);
  UILCD_WriteCom(0x30);
  delay(200);
  UILCD_WriteCom(0x30);
  delay(200);
  UILCD_WriteCom(0x30);
  UILCD_WriteCom(0x39);
  UILCD_WriteCom(0x14);
  UILCD_WriteCom(0x56);
  UILCD_WriteCom(0x6D);
  UILCD_WriteCom(0x70);
  UILCD_WriteCom(0x0C);
  UILCD_WriteCom(0x06);
  UILCD_WriteCom(0x01);
  delay(5000);
  //set CGRAM
  UILCD_WriteCom(0x38);
  UILCD_WriteCom(0x40);
  UILCD_WriteData(0x00);//address 0 = up/left arrow
  UILCD_WriteData(0x1E);
  UILCD_WriteData(0x18);
  UILCD_WriteData(0x14);
  UILCD_WriteData(0x12);
  UILCD_WriteData(0x01);
  UILCD_WriteData(0x00);
  UILCD_WriteData(0x00);
  UILCD_WriteData(0x00);//address 1 = down/left arrow
  UILCD_WriteData(0x00);
  UILCD_WriteData(0x01);
  UILCD_WriteData(0x12);
  UILCD_WriteData(0x14);
  UILCD_WriteData(0x18);
  UILCD_WriteData(0x1E);
  UILCD_WriteData(0x00);
  UILCD_WriteData(0x00);//address 2 = right arrow
  UILCD_WriteData(0x04);
  UILCD_WriteData(0x06);
  UILCD_WriteData(0x1F);
  UILCD_WriteData(0x06);
  UILCD_WriteData(0x04);
  UILCD_WriteData(0x00);
  UILCD_WriteData(0x00);
  UILCD_WriteCom(0x39);
  delay(500);
}
/*******************************************************************************
* Function Name  : UILCD_Clear
* Description    : Clears the LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_Clear(void)
{
  UILCD_WriteCom(0x01);
}

/*******************************************************************************
* Function Name  : UILCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position.
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_SetCursor(uint8_t Xpos, uint16_t Ypos)
{
  unsigned char address;
  address = (Xpos * 40) + Ypos;
  address = 0x80 + (address & 0x7F);
  UILCD_WriteCom(address);
}
/*******************************************************************************
* Function Name  : UILCD_DisplayChar
* Description    : Displays one character.
* Input          : - Line: the Line where to display the character shape .
*                    This parameter can be one of the following values:
*                       - Linex: where x can be 0..9
*                  - Column: start column address.
*                  - Ascii: character ascii code, must be between 0x20 and 0x7E.
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_DisplayChar(char Ascii)
{
  UILCD_WriteData(Ascii);  
}

/*******************************************************************************
* Function Name  : UILCD_DisplayString
* Description    : Displays a maximum of 32 char on the LCD.
* Input          : - Line: the starting Line where to display the character shape.
*                    This parameter can be one of the following values:
*                       - Linex: where x can be 0..9
*                  - *ptr: pointer to string to display on LCD.
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_DisplayString(char *ptr)
{
  while (*ptr)  {
    UILCD_WriteData(*ptr++);
	}
}

/*******************************************************************************
* Function Name  : UILCD_ScrollText
* Description    :
* Input          :
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_ScrollText(uint8_t Line, uint8_t *ptr)
{
  ;
}

/*******************************************************************************
* Function Name  : UILCD_WriteCom
* Description    : Writes Command to the LCD.
* Input          : - Command
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_WriteCom(uint8_t Command)
{
  int i;
  GPIO_WriteBit(GPIOC, CtrlPin_UICSB, Bit_RESET);
  GPIO_WriteBit(GPIOC, CtrlPin_UIRS, Bit_RESET);
  for(i=0;i<8;i++){
  	if((Command & 0x80)==0){GPIO_WriteBit(GPIOC, CtrlPin_UISDA, Bit_RESET);}
	else {GPIO_WriteBit(GPIOC, CtrlPin_UISDA, Bit_SET);}
	GPIO_WriteBit(GPIOC, CtrlPin_UISCL, Bit_SET);
	GPIO_WriteBit(GPIOC, CtrlPin_UISCL, Bit_RESET);
	Command <<= 1;
	}
  GPIO_WriteBit(GPIOC, CtrlPin_UICSB, Bit_SET);
}

/*******************************************************************************
* Function Name  : UILCD_WriteData
* Description    : Writes data to the LCD.
* Input          : - Ascii
* Output         : None
* Return         : None
*******************************************************************************/
void UILCD_WriteData(uint8_t Ascii)
{
  int i;
  GPIO_WriteBit(GPIOC, CtrlPin_UICSB, Bit_RESET);
  GPIO_WriteBit(GPIOC, CtrlPin_UIRS, Bit_SET);
  for(i=0;i<8;i++){
  	if((Ascii & 0x80)==0){GPIO_WriteBit(GPIOC, CtrlPin_UISDA, Bit_RESET);}
	else {GPIO_WriteBit(GPIOC, CtrlPin_UISDA, Bit_SET);}
	GPIO_WriteBit(GPIOC, CtrlPin_UISCL, Bit_SET);
	GPIO_WriteBit(GPIOC, CtrlPin_UISCL, Bit_RESET);
	Ascii <<= 1;
	}
  GPIO_WriteBit(GPIOC, CtrlPin_UICSB, Bit_SET);
}
/******************* (C) COPYRIGHT 2009 Newhaven Display *****END OF FILE****/
