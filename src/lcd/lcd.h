/******************** (C) COPYRIGHT 2010 Newhaven Display ********************
* File Name          : lcd.h
* Author             : Curt Lagerstam
* Date First Issued  : 8/10/2009
* Description        : This file contains all the functions prototypes for the
*                      lcd demo software driver.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, NEWHAVEN DISPLAY SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_H
#define __LCD_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* LCD Control pins */
#define RS     GPIO_Pin_4   /* PC.04 */
#define RW     GPIO_Pin_5   /* PC.05 */
#define nWR    GPIO_Pin_5   /* PC.05 */
#define E1     GPIO_Pin_6   /* PC.06 */
#define nRD    GPIO_Pin_6   /* PC.06 */
#define E2     GPIO_Pin_7   /* PC.07 */
#define CS1    GPIO_Pin_8   /* PC.08 */
#define CS2    GPIO_Pin_9   /* PC.09 */
#define DISP   GPIO_Pin_9   /* PC.09 */
#define RES    GPIO_Pin_10  /* PC.10 */
#define DB0    GPIO_Pin_0   /* PB.00 */
#define DB1    GPIO_Pin_1   /* PB.01 */
#define DB2    GPIO_Pin_2   /* PB.02 */
#define DB3    GPIO_Pin_3   /* PB.03 */
#define DB4    GPIO_Pin_4   /* PB.04 */
#define DB5    GPIO_Pin_5   /* PB.05 */
#define DB6    GPIO_Pin_6   /* PB.06 */
#define DB7    GPIO_Pin_7   /* PB.07 */
#define DB8    GPIO_Pin_8   /* PB.08 */
#define DB9    GPIO_Pin_9   /* PB.09 */
#define DB10    GPIO_Pin_10   /* PB.10 */
#define DB11    GPIO_Pin_11   /* PB.11 */
#define DB12    GPIO_Pin_12   /* PB.12 */
#define DB13    GPIO_Pin_13   /* PB.13 */
#define DB14    GPIO_Pin_14   /* PB.14 */
#define DB15    GPIO_Pin_15   /* PB.15 */

#define Line1          0
#define Line2          0xC0
#define Line3          0xC0
#define Line4          0xC0

/*----- 3.5-320240MF-A -----*/
int TFT_35M_demo(void);
/*----- 4.3-480272MF-A -----*/
int TFT_43M_demo(void);
/*----- 5.7-640480WF-C -----*/
int TFT_57F_demo(void);
/*----- Chars ----------*/
int x1x8_demo(void);
int x1x12_demo(void);
int x1x16_demo(void);
int x2x8_demo(void);
int x2x12_demo(void);
int x2x16_demo(void);
int x2x20_demo(void);
int x2x24_demo(void);
int x2x40_demo(void);
int x4x16_demo(void);
int x4x20_demo(void);
int x4x40_demo(void);
/*----- 12864AZ ------------*/
int x12864AZ_demo(void);

#endif /* __LCD_H */

/******************* (C) COPYRIGHT 2010 Newhaven Display *****END OF FILE****/
