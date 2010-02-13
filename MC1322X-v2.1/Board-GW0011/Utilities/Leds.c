/************************************************************************************
* Leds.c
*
* This file contains util funcions for the LEDs management.
*
* Author(s): 
*
* Copyright (c) 2008, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
Revision history:
   Date                     Author                                    Comments
   ----------               ----------------------                    -------
************************************************************************************/
#include "Leds.h"

#if (gLEDSupported_d)

/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/

/*None*/

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
* LED_Init
*
* Initialize the LED system.
*******************************************************************************/
void LED_Init(void)
{
  InitLed();
  TurnOffLeds();
  Led3Off();
}


/******************************************************************************
* Set_Pins
*
* Set a PIN with Direction Out, this PIN can be used as a LED.
*******************************************************************************/
void Set_Pins( LED_t LEDNr )
{
  if (LEDNr & LED1){
    Gpio_SetPinFunction(LED1_PIN, gGpioNormalMode_c);
    Gpio_SetPinReadSource(LED1_PIN, gGpioPinReadReg_c);
    Gpio_SetPinDir(LED1_PIN, gGpioDirOut_c);
  Gpio_SetPinData(LED1_PIN, gGpioPinStateHigh_c);
  }
  if (LEDNr & LED2){
    Gpio_SetPinFunction(LED2_PIN, gGpioNormalMode_c);
    Gpio_SetPinReadSource(LED2_PIN, gGpioPinReadReg_c);
    Gpio_SetPinDir(LED2_PIN, gGpioDirOut_c);
    Gpio_SetPinData(LED2_PIN, gGpioPinStateHigh_c);
  }
  if (LEDNr & LED3){
    Gpio_SetPinFunction(LED3_PIN, gGpioNormalMode_c);
    Gpio_SetPinReadSource(LED3_PIN, gGpioPinReadReg_c);
    Gpio_SetPinDir(LED3_PIN, gGpioDirOut_c);
    Gpio_SetPinData(LED3_PIN, gGpioPinStateHigh_c);
  }
  if (LEDNr & LED4){
    Gpio_SetPinFunction(LED4_PIN, gGpioNormalMode_c);
    Gpio_SetPinReadSource(LED4_PIN, gGpioPinReadReg_c);
    Gpio_SetPinDir(LED4_PIN, gGpioDirOut_c);
    Gpio_SetPinData(LED4_PIN, gGpioPinStateHigh_c);
  }
}

/******************************************************************************
* LED_GetLed
*
* Get LED state.
*******************************************************************************/
LedState_t LED_GetLed( LED_t LEDNr)                      
{
  GpioPinState_t gpioPinState;
  LED_t LEDpinsState = 0;
  int8_t ReturnSingleLED = 0;
  
  if ( (LEDNr == LED1) || (LEDNr == LED2) || (LEDNr == LED3) || (LEDNr == LED4) )
    ReturnSingleLED = 1;
  else
    ReturnSingleLED = 0;
  
  if (LEDNr & LED1)
  {
    Gpio_GetPinData(LED1_PIN, &gpioPinState);
    if (ReturnSingleLED)
      return !gpioPinState;
  }
  LEDpinsState |= gpioPinState << 0;
  
  if (LEDNr & LED2)
  {
    Gpio_GetPinData(LED2_PIN, &gpioPinState);
    if (ReturnSingleLED)
      return !gpioPinState;
  }
  LEDpinsState |= gpioPinState << 1;
  
  if (LEDNr & LED3)
  {
    Gpio_GetPinData(LED3_PIN, &gpioPinState);
    if (ReturnSingleLED)
      return !gpioPinState;
  }
  LEDpinsState |= gpioPinState << 2;
  
  if (LEDNr & LED4)
  {
    Gpio_GetPinData(LED4_PIN, &gpioPinState);
    if (ReturnSingleLED)
      return !gpioPinState;
  }
  LEDpinsState |= gpioPinState << 3;
  
  return ~LEDpinsState;
  
}

/******************************************************************************
* LED_ToggleLed
*
* Toggles on or more LEDs. Doesn't check/affect flashing states.
*******************************************************************************/
void LED_ToggleLed(LED_t LEDNr)
{
  if (LEDNr & LED1){    
    Led1Toggle();
  }
  if (LEDNr & LED2){    
    Led2Toggle();
  }
  if (LEDNr & LED3){    
    Led3Toggle();
  }
  if (LEDNr & LED4){    
    Led4Toggle();
  }
}

/******************************************************************************
* LED_TurnOffLed
*
* Turns off on or more LEDs. Doesn't check/affect flashing states.
*******************************************************************************/
void LED_TurnOffLed(LED_t LEDNr)
{
  if (LEDNr & LED1)
    Led1Off();
  if (LEDNr & LED2)
    Led2Off();
  if (LEDNr & LED3)
    Led3Off();
  if (LEDNr & LED4)
    Led4Off();
}

/******************************************************************************
* LED_TurnOnLed
*
* Turn on one or more LEDs. Doesn't check/affect flashing states.
*******************************************************************************/
void LED_TurnOnLed(LED_t LEDNr)
{
  if (LEDNr & LED1)
    Led1On();
  if (LEDNr & LED2)
    Led2On();
  if (LEDNr & LED3)
    Led3On();
  if (LEDNr & LED4)
    Led4On();
}

/******************************************************************************
* LED_TurnOffAllLeds
*
* Turn off all LEDs. 
*******************************************************************************/
void LED_TurnOffAllLeds(void)
{
  LED_TurnOffLed(LED_ALL);
}

/******************************************************************************
* LED_TurnOnAllLeds
*
* Turn on all LEDs. 
*******************************************************************************/
void LED_TurnOnAllLeds(void)
{
  LED_TurnOnLed(LED_ALL);
}

/************************************************************************************
* ShowBinaryLedValue
*
* The purpose of this function is to show the binary value received as parameter 
* through four LEDs, then the maximum possible u8leds_value is the 0x0F.
*
************************************************************************************/
void LED_SetHex(uint8_t u8HexValue){

  TurnOffLeds(); 

  if(0x01 & u8HexValue)
  {
    Led4On();
  }
  if((0x01<<1) & u8HexValue)
  {
    Led3On();
  }
  if((0x01<<2) & u8HexValue)
  {
    Led2On();
  }
  if((0x01<<3) & u8HexValue)
  {
    Led1On();
  }
  
}

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Unit Testing
*******************************************************************************
******************************************************************************/

/* None */

#endif /* gLEDSupported_d */

