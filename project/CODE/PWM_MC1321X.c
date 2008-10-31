/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : PWM_MC1321X.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_TPM
**     Version   : Bean 01.070, Driver 01.14, CPU db: 2.87.125
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 10/31/2008, 9:24 AM
**     Abstract  :
**          This file implements the TPM (TPM2) module initialization
**          according to the Peripheral Initialization Bean settings,
**          and defines interrupt service routines prototypes.
**     Settings  :
**          Bean name                                      : PWM_MC1321X
**          Device                                         : TPM2
**          Settings
**            Clock settings
**              Clock Source Select                        : Bus rate clock
**              Prescaler                                  : 1
**              Modulo Counter                             : 255
**              Period                                     : 12.8 us
**            Aligned                                      : Left
**          Channels                                       : 2
**            Channel0                                     : 1
**              Capture/compare device                     : TPM22
**              Settings
**                Mode                                     : PWM
**                  PWM Output Action                      : Clear output on compare
**                  ChannelValue                           : 0
**                  DutyCycle                              : 0 us
**              Pin                                        : Used
**                Channel Pin                              : PTD5_TPM2CH2
**                Channel Pin Signal                       : 
**                Pull Resistor                            : autoselected pull
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm2ch2
**                  Channel Interrupt                      : Disabled
**                  ISR Name                               : 
**            Channel1                                     : 2
**              Capture/compare device                     : TPM23
**              Settings
**                Mode                                     : PWM
**                  PWM Output Action                      : Clear output on compare
**                  ChannelValue                           : 0
**                  DutyCycle                              : 0 us
**              Pin                                        : Used
**                Channel Pin                              : PTD6_TPM2CH3
**                Channel Pin Signal                       : 
**                Pull Resistor                            : autoselected pull
**              Interrupt
**                Channel Interrupt
**                  Interrupt                              : Vtpm2ch3
**                  Channel Interrupt                      : Disabled
**                  ISR Name                               : 
**          Pins
**            External Clock Source                        : Disabled
**          Interrupts
**            Overflow Interrupt
**              Interrupt                                  : Vtpm2ovf
**              Overflow Interrupt                         : Disabled
**              ISR Name                                   :
**          Initialization
**          Call Init Method                               : yes
**     Contents  :
**         Init - void PWM_MC1321X_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE PWM_MC1321X. */

#include "PWM_MC1321X.h"

/*
** ===================================================================
**     Method      :  PWM_MC1321X_Init (bean Init_TPM)
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
void PWM_MC1321X_Init(void)
{

  /* TPM2SC: TOF=0,TOIE=0,CPWMS=0,CLKSB=0,CLKSA=0,PS2=0,PS1=0,PS0=0 */
  setReg8(TPM2SC, 0x00);               /* Stop and reset counter */ 
  setReg16(TPM2MOD, 0xFF);             /* Period value setting */ 
  (void)getReg8(TPM2C2SC);             /* Channel 0 int. flag clearing (first part) */
  /* TPM2C2SC: CH2F=0,CH2IE=0,MS2B=1,MS2A=0,ELS2B=1,ELS2A=0,??=0,??=0 */
  setReg8(TPM2C2SC, 0x28);             /* Int. flag clearing (2nd part) and channel 0 contr. register setting */ 
  setReg16(TPM2C2V, 0x00);             /* Compare 0 value setting */ 
  (void)getReg8(TPM2C3SC);             /* Channel 1 int. flag clearing (first part) */
  /* TPM2C3SC: CH3F=0,CH3IE=0,MS3B=1,MS3A=0,ELS3B=1,ELS3A=0,??=0,??=0 */
  setReg8(TPM2C3SC, 0x28);             /* Int. flag clearing (2nd part) and channel 1 contr. register setting */ 
  setReg16(TPM2C3V, 0x00);             /* Compare 1 value setting */ 
  (void)getReg8(TPM2SC);               /* Overflow int. flag clearing (first part) */
  /* TPM2SC: TOF=0,TOIE=0,CPWMS=0,CLKSB=0,CLKSA=1,PS2=0,PS1=0,PS0=0 */
  setReg8(TPM2SC, 0x08);               /* Int. flag clearing (2nd part) and timer control register setting */ 

}

/* END PWM_MC1321X. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
