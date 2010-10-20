/******************** (C) COPYRIGHT 2009 Newhaven Display ********************
* File Name          : ui_lcd.h
* Author             : Curt Lagerstam
* Date First Issued  : 7/10/2009
* Description        : This file contains all the functions prototypes for the
*                      UI lcd software driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UILCD_H
#define __UILCD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* LCD Control pins */
#define CtrlPin_UICSB    GPIO_Pin_1   /* PC.01 */
#define CtrlPin_UIRS     GPIO_Pin_3   /* PC.03 */
#define CtrlPin_UISCL    GPIO_Pin_2   /* PC.02 */
#define CtrlPin_UISDA    GPIO_Pin_0   /* PC.00 */

#define UILine1          0
#define UILine2          0xC0
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
void delay(int time);
void UILCD_Init(void);
void UILCD_Clear(void);
void UILCD_SetCursor(uint8_t Xpos, uint16_t Ypos);
void UILCD_DisplayChar(char Ascii);
void UILCD_DisplayString(char *ptr);
void UILCD_ScrollText(uint8_t Line, uint8_t *ptr);
void UILCD_SelectDisplay(uint8_t Selection);
/*----- Medium layer function -----*/
void UILCD_WriteCom(uint8_t Command);
void UILCD_WriteData(uint8_t Ascii);
/*----- Low layer function -----*/
void UILCD_CtrlLinesConfig(void);
//void UILCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal);
#endif /* __UILCD_H */

/******************* (C) COPYRIGHT 2009 Newhaven Display *****END OF FILE****/
