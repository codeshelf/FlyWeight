/************************************************************************************
* Led.h
*
* This header file is for LED Driver Interface.
*
* Author(s):  
*
* Copyright (c) 2008, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
Revision history:
   Date                     Author                        Comments
   ----------               ----------------------      -------
************************************************************************************/
#ifndef _LED_H_
#define _LED_H_

#include "EmbeddedTypes.h"
#include "GPIO_Interface.h"
#include "board_config.h"

/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/


/* are LEDs supported */
#define gLEDSupported_d TRUE

/* BEGIN Define LED pin mapping */
/* for the future  use of next platforms: we can use  LED1_PIN_TARGET or LED1_PIN as defines */
#define LED1_PIN_TARGET     gGpioPin22_c
#define LED2_PIN_TARGET     gGpioPin23_c
#define LED3_PIN_TARGET     gGpioPin22_c
#define LED4_PIN_TARGET     gGpioPin23_c

/* Pin levels for LED ON and OFF states */
#define LED_SET     gGpioPinStateHigh_c
#define LED_RESET     gGpioPinStateLow_c
/*END Define LED pin mapping */


/* possible LED values for LED_SetLed() */

#define LED1          0x01  /* LED1 is defined as bit nr 1 */
#define LED2          0x02  /* LED2 is defined as bit nr 2 */
#define LED3          0x04  /* LED3 is defined as bit nr 3 */
#define LED4          0x08  /* LED4 is defined as bit nr 4 */


#if (gLEDSupported_d)

typedef uint8_t LED_t;

#if TARGET_BOARD == MC1322XUSB
  #define LED_ALL       0x01  /* turn on/off all LEDs */
#elif TARGET_BOARD == MC1322XLPB
  #define LED_ALL       0x03  /* turn on/off all LEDs */
#elif TARGET_BOARD == GW0011
  #define LED_ALL       0x03  /* turn on/off all LEDs */
#else 
  #define LED_ALL       0x0F  /* turn on/off all LEDs */
#endif

/* note: all LEDs can operate independantly */
typedef uint8_t LedState_t; /* possible LED int states for LEDs */

/* Define LED pin mapping for driver*/
#define LED1_PIN LED1_PIN_TARGET        /* LED1_PIN is defined as LED1 for the actual target that is in use */
#define LED2_PIN LED2_PIN_TARGET        /* LED2_PIN is defined as LED2 for the actual target that is in use */
#define LED3_PIN LED3_PIN_TARGET        /* LED3_PIN is defined as LED3 for the actual target that is in use */
#define LED4_PIN LED4_PIN_TARGET        /* LED4_PIN is defined as LED4 for the actual target that is in use */

/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/

/*   Macros with functionality to LED   */
#define Led1On()            (void)Gpio_SetPinData(LED1_PIN, LED_SET)    /* Set      LED1 to ON  state */
#define Led1Off()           (void)Gpio_SetPinData(LED1_PIN, LED_RESET)   /* Set      LED1 to OFF state */
#define Led1Toggle()        (void)Gpio_TogglePin (LED1_PIN)                        /* Toggle   LED1 state */
#define GetLed1()           LED_GetLed( LED1 )                                    /* Get      LED1 state */

#define Led2On()            (void)Gpio_SetPinData(LED2_PIN, LED_SET)    /* Set      LED2 to ON  state */
#define Led2Off()           (void)Gpio_SetPinData(LED2_PIN, LED_RESET)   /* Set      LED2 to OFF state */
#define Led2Toggle()        (void)Gpio_TogglePin (LED2_PIN)                        /* Toggle   LED2 state */
#define GetLed2()           LED_GetLed( LED2 )                                    /* Get      LED2 state */

#define Led3On()            (void)Gpio_SetPinData(LED3_PIN, LED_SET)    /* Set      LED3 to ON  state */
#define Led3Off()           (void)Gpio_SetPinData(LED3_PIN, LED_RESET)   /* Set      LED3 to OFF state */
#define Led3Toggle()        (void)Gpio_TogglePin (LED3_PIN)                        /* Toggle   LED3 state */
#define GetLed3()           LED_GetLed( LED3 )                                    /* Get      LED3 state */

#define Led4On()            (void)Gpio_SetPinData(LED4_PIN, LED_SET)    /* Set      LED4 to ON  state */
#define Led4Off()           (void)Gpio_SetPinData(LED4_PIN, LED_RESET)   /* Set      LED4 to OFF state */
#define Led4Toggle()        (void)Gpio_TogglePin (LED4_PIN)                        /* Toggle   LED4 state */
#define GetLed4()           LED_GetLed( LED4 )                                    /* Get      LED4 state */

/* Turn off LEDs */
#define TurnOffLeds()       LED_TurnOffAllLeds()                    /* Turn OFF All LEDs */
/* Turn on LEDs */
#define TurnOnLeds()        LED_TurnOnAllLeds()                     /* Turn ON  All LEDs */
/* Init all leds*/
#define InitLed()            Set_Pins(LED_ALL);                    /*Set settings and state for all LEDs */

/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/

/* LEDs not supported */
#else

#define Led1On()                /* Set      LED1 to ON  state */
#define Led1Off()               /* Set      LED1 to OFF state */
#define Led1Toggle()            /* Toggle   LED1 state */
#define GetLed1()  0            /* Get      LED1 state */

#define Led2On()                /* Set      LED2 to ON  state */
#define Led2Off()               /* Set      LED2 to OFF state */
#define Led2Toggle()            /* Toggle   LED2 state */
#define GetLed2()  0            /* Get      LED2 state */

#define Led3On()                /* Set      LED3 to ON  state */
#define Led3Off()               /* Set      LED3 to OFF state */
#define Led3Toggle()            /* Toggle   LED3 state */
#define GetLed3()  0            /* Get      LED3 state */

#define Led4On()                /* Set      LED4 to ON  state */
#define Led4Off()               /* Set      LED4 to OFF state */
#define Led4Toggle()            /* Toggle   LED4 state */
#define GetLed4()  0            /* Get      LED4 state */

#define TurnOffLeds()           /* Turn OFF All LEDs */
#define TurnOnLeds()            /* Turn ON  All LEDs */

#define InitLed()               /*Set settings and state for all LEDs */


/* define empty prototypes */
/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/

#define LED_Init()                    /* Initializing LEDs state */

#define LED_TurnOffLed(LEDNr)         /* Turn OFF LED state with number LEDNr */
#define LED_TurnOnLed(LEDNr)          /* Turn ON  LED state with number LEDNr */
#define LED_ToggleLed(LEDNr)          /* Toggle   LED state with number LEDNr */

#define LED_StartFlash(LEDNr)         /* Start Flashing LED with number LEDNr */
#define LED_StopFlash(LEDNr)          /* Stop  Flashing LED with number LEDNr */

#define LED_StartSerialFlash()        /* Start Serial Flashing for All LEDs */
#define LED_StopFlashingAllLeds()     /* Stop  Serial Flashing for All LEDs */
#define LED_ExitSerialFlash()         /* Stop  Serial Flashing for LEDs  and closes the timers*/

#define LED_TurnOffAllLeds()          /* Turn OFF state for All LEDs */
#define LED_TurnOnAllLeds()           /* Turn ON  state for All LEDs */

#define LED_GetLed( LEDNr)        0   /* Get State of LEDNr*/                        
#define LED_SetLed(LEDNr, state)      /* Set LEDNr to "state" state */
#define LED_SetHex(hexValue)          /* Set LEDs state to show a binary Value */
#define Set_Pins( LEDNr)              /* Set initial state of LED */

#endif /* gLEDSupported_d */

/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Public Memory Declarations
*******************************************************************************
******************************************************************************/
/* None */


#if (gLEDSupported_d)

/************************************************************************************
* This function initializate the LEDs of the system.
*
* Parameters: None.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void LED_Init(void);                                      /* Initializing LEDs state */


/************************************************************************************
* This function turns off all leds.
*
* Parameters: None.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void LED_TurnOffAllLeds(void);        

/************************************************************************************
* This function turns on all leds.
*
* Parameters: None.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void LED_TurnOnAllLeds(void);                            

/************************************************************************************
* This function turns off the led that receives as parameter.
*
* Parameters: None.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void LED_TurnOffLed(LED_t LEDNr);                         

/************************************************************************************
* This function turns on the led received as parameter.
*
* Parameters: None.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void LED_TurnOnLed(LED_t LEDNr);                          

/************************************************************************************
* Function that toggles LED state with number LEDNr.
*
* Parameters: LED_T LEDNr - Contains the led number to toggle.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void LED_ToggleLed(LED_t LEDNr);                          

/************************************************************************************
* Function that gets the state of the LED.
*
* Parameters: LED_T LEDNr - Contains the led number that wants to know its state.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
LedState_t  LED_GetLed( LED_t LEDNr);                     /* Get State of LEDNr*/

/************************************************************************************
* Function that shows the binary value received as parameter through four LEDs, 
* then the maximum possible u8leds_value is the 0x0F.
*
* Parameters: uint8_t u8HexValue - Hexadecimal value to toggle.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void    LED_SetHex( uint8_t u8HexValue);                    /* Set LEDs state to show a binary Value */

/************************************************************************************
* Function that initializate the GPIOs for the LEDs
*
* Parameters: LED_t LED_t - The number of the number to initializate.
*
* Return value: None.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
void    Set_Pins  ( LED_t LEDNr);                         /* Set initial state of LED */

#endif /*gLEDSupported_d*/
#endif /* _LED_H_ */
