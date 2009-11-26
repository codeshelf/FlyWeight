/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : LED_XBee.H
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_TPM
**     Version   : Bean 01.070, Driver 01.14, CPU db: 2.87.125
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 11/25/2009, 3:49 PM
**     Abstract  :
**          This file implements the TPM (TPM1) module initialization
**          according to the Peripheral Initialization Bean settings,
**          and defines interrupt service routines prototypes.
**     Settings  :
**          Bean name                                      : LED_XBee
**          Device                                         : TPM1
**          Settings
**            Clock settings
**              Clock Source Select                        : Fixed system clock
**              Prescaler                                  : 128
**              Modulo Counter                             : 255
**              Period                                     : 16.384 ms
**            Aligned                                      : Left
**          Channels                                       : 3
**            Channel0                                     : 1
**              Capture/compare device                     : TPM10
**              Settings
**                Mode                                     : Output compare
**                  Output Action                          : Software compare only
**                  ChannelValue                           : 0
**              Pin                                        : Not Used
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm1ch0
**                  Channel Interrupt                      : Enabled
**                  ISR Name                               : LEDRedOff
**            Channel1                                     : 2
**              Capture/compare device                     : TPM11
**              Settings
**                Mode                                     : Output compare
**                  Output Action                          : Software compare only
**                  ChannelValue                           : 0
**              Pin                                        : Not Used
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm1ch1
**                  Channel Interrupt                      : Enabled
**                  ISR Name                               : LEDGreenOff
**            Channel2                                     : 3
**              Capture/compare device                     : TPM12
**              Settings
**                Mode                                     : Output compare
**                  Output Action                          : Software compare only
**                  ChannelValue                           : 0
**              Pin                                        : Not Used
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm1ch2
**                  Channel Interrupt                      : Enabled
**                  ISR Name                               : LEDBlueOff
**          Pins
**            External Clock Source                        : Disabled
**          Interrupts
**            Overflow Interrupt
**              Interrupt                                  : Vtpm1ovf
**              Overflow Interrupt                         : Enabled
**              ISR Name                                   :LEDCheck
**          Initialization
**          Call Init Method                               : yes
**     Contents  :
**         Init - void LED_XBee_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __LED_XBee
#define __LED_XBee

/* MODULE LED_XBee. */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/*
** ===================================================================
**     Method      :  LED_XBee_Init (bean Init_TPM)
**
**     Description :
**         This method initializes registers of the TPM module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in the user code to initialize the
**         module. By default, the method is called by PE
**         automatically; see "Call Init method" property of the
**         bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void LED_XBee_Init(void);

/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see LED_XBee.c file for more information).
** ===================================================================
*/
__interrupt void LEDCheck(void);

__interrupt void LEDRedOff(void);

__interrupt void LEDGreenOff(void);

__interrupt void LEDBlueOff(void);

/* END LED_XBee. */

#endif /* ifndef __LED_XBee */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
