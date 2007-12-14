/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : PWM_XBee.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : Init_TPM
**     Version   : Bean 01.035, Driver 01.07, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 12/13/2007, 3:20 PM
**     Abstract  :
**          This file implements the TPM (TPM1) module initialization
**          according to the Peripheral Initialization Bean settings,
**          and defines interrupt service routines prototypes.
**     Settings  :
**          Bean name                                      : PWM_XBee
**          Device                                         : TPM1
**          Settings
**            Clock settings
**              Clock Source Select                        : Bus rate clock
**              Prescaler                                  : 1
**              Modulo Counter                             : 255
**              Period                                     : 12.8 us
**            Aligned                                      : Left
**          Channels                                       : 2
**            Channel0                                     : 1
**              Capture/compare device                     : TPM10
**              Settings
**                Mode                                     : PWM
**                  PWM Output Action                      : Clear output on compare
**                  ChannelValue                           : 0
**                  DutyCycle                              : 0 us
**              Pin                                        : Used
**                Channel Pin                              : PTD0_TPM1CH0
**                Channel Pin Signal                       : 
**                Pull Resistor                            : autoselected pull
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm1ch0
**                  Channel Interrupt                      : Disabled
**                  ISR Name                               : 
**            Channel1                                     : 2
**              Capture/compare device                     : TPM11
**              Settings
**                Mode                                     : PWM
**                  PWM Output Action                      : Clear output on compare
**                  ChannelValue                           : 0
**                  DutyCycle                              : 0 us
**              Pin                                        : Used
**                Channel Pin                              : PTD1_TPM1CH1
**                Channel Pin Signal                       : 
**                Pull Resistor                            : autoselected pull
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm1ch1
**                  Channel Interrupt                      : Disabled
**                  ISR Name                               : 
**          Pins
**            External Clock Source                        : Disabled
**          Interrupts
**            Overflow Interrupt
**              Interrupt                                  : Vtpm1ovf
**              Overflow Interrupt                         : Disabled
**              ISR Name                                   :
**          Initialization
**          Call Init Method                               : no
**     Contents  :
**         Init - void PWM_XBee_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE PWM_XBee. */

#include "PWM_XBee.h"

/*
** ===================================================================
**     Method      :  PWM_XBee_Init (bean Init_TPM)
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
void PWM_XBee_Init(void)
{

  setReg16(TPM1MOD, 0xFF);              
  setReg16(TPM1C0V, 0x00);              
  /* TPM1C0SC: CH0F=0,CH0IE=0,MS0B=1,MS0A=0,ELS0B=1,ELS0A=0,??=0,??=0 */
  setReg8(TPM1C0SC, 0x28);              

  setReg16(TPM1C1V, 0x00);              
  /* TPM1C1SC: CH1F=0,CH1IE=0,MS1B=1,MS1A=0,ELS1B=1,ELS1A=0,??=0,??=0 */
  setReg8(TPM1C1SC, 0x28);              

  /* TPM1SC: TOF=0,TOIE=0,CPWMS=0,CLKSB=0,CLKSA=1,PS2=0,PS1=0,PS0=0 */
  setReg8(TPM1SC, 0x08);                

}

/* END PWM_XBee. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
