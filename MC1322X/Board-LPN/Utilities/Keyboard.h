/************************************************************************************
* This header file is for Keyboard Driver Interface.
*
* Copyright (c) 2008, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#ifndef _KEYBOARDHAL_H_
#define _KEYBOARDHAL_H_

#include "../../PLM/LibInterface/GPIO_Interface.h"
#include "../../PLM/LibInterface/Crm.h"


/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* BEGIN Define Key pin mapping */
/* Select the port */
#define gKbGpioPort_c           gGpioPort0_c

/* Select the pins for switch 1-4. Possible values: 26, 27, 28, 29  */
#define gKbGpioPinSwitch1_c     gGpioPin26_c
#define gKbGpioPinSwitch2_c     gGpioPin27_c
#define gKbGpioPinSwitch3_c     gGpioPin28_c
#define gKbGpioPinSwitch4_c     gGpioPin29_c

/* Center key pin for joystick */
#define gCenterKey_c     gGpioPin22_c
/* END Define Key pin mapping */




/* Select the port */
#define gKeyBoardSupported_d TRUE
#define gJoystickSupported_d TRUE


#if (gKeyBoardSupported_d || gJoystickSupported_d)

#define gKeyboardModule_d TRUE

#if (TARGET_BOARD == MC1322XSRB || TARGET_BOARD == MC1322XNCB)
#if (gJoystickSupported_d) 
  #define gKbGpioMask_c  ((1 << gKbGpioPinSwitch1_c) | (1 << gKbGpioPinSwitch2_c) | \
                          (1 << gKbGpioPinSwitch3_c) | (1 << gKbGpioPinSwitch4_c) | (1 << gCenterKey_c))
#else
  #define gKbGpioMask_c  ((1 << gKbGpioPinSwitch1_c) | (1 << gKbGpioPinSwitch2_c) | \
                          (1 << gKbGpioPinSwitch3_c) | (1 << gKbGpioPinSwitch4_c))
#endif
#else
#if  TARGET_BOARD == MC1322XLPB
  #define gKbGpioMask_c ((1 << gKbGpioPinSwitch1_c) | 1 << gCenterKey_c))
#endif
#endif 



#define gKbGpioAllZeros_c 0x00000000
#define gKbGpioAllOnes_c  0xFFFFFFFF


/* Enable Wake up for all 4 keys*/
#define gKbiWuEN_c 0x0f
/*Select egde sense for all  keys*/
#define gKbiWuEVENT_c  0x0f
/*Select wake up polarity to be negativ edge*/
#define gKbiWuPOL_c  0x00
/*enable interrupts for all keys */
#define gKbiIntEN_c  0x0f


#define gSWITCH1_MASK_c    0x01 //pin 26
#define gSWITCH2_MASK_c    0x02 //pin 27
#define gSWITCH3_MASK_c    0x04 //pin 28
#define gSWITCH4_MASK_c    0x08 //pin 29
#define gSWITCH_MASK_c     (gSWITCH1_MASK_c | gSWITCH2_MASK_c | gSWITCH3_MASK_c | gSWITCH4_MASK_c)


/* Read the port and shift it for better detection of key pressed*/
#define SwitchReadPins KbReadPins()

#define SwitchGet ((SwitchReadPins & gSWITCH1_MASK_c) ^ gSWITCH1_MASK_c) |\
                  ((SwitchReadPins & gSWITCH2_MASK_c) ^ gSWITCH2_MASK_c) |\
                  ((SwitchReadPins & gSWITCH3_MASK_c) ^ gSWITCH3_MASK_c) |\
                  ((SwitchReadPins & gSWITCH4_MASK_c) ^ gSWITCH4_MASK_c)
/*Enable and disable one/more kye interrupt*/
#define KbEnableIrq(SWITCH_MASK) CRM_WU_CNTL.extWuIEn |= SWITCH_MASK 
#define KbDisableIrq(SWITCH_MASK) CRM_WU_CNTL.extWuIEn  &= ~SWITCH_MASK

/*Enable and Disable all external interrupts*/
#define KbDisableAllIrq() KbDisableIrq(gSWITCH_MASK_c) 
#define KbEnableAllIrq() KbEnableIrq (gSWITCH_MASK_c)

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

/*Init the GPIO as input, normal functin mode, pull up enable and read from pads */
extern void KbGpioInit(void);


#else

  #define KbGpioInit()
  #define gKeyboardModule_d FALSE

#endif /*gKeyBoardSupported_d*/

#endif /*_KEYBOARDHAL_H_*/
