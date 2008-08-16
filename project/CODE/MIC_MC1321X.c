/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : MIC_MC1321X.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_ADC
**     Version   : Bean 01.115, Driver 01.12, CPU db: 2.87.123
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 8/16/2008, 12:14 PM
**     Abstract  :
**          This file implements the ADC (ATD) module initialization 
**          according to the Peripheral Initialization Bean settings, and defines
**          interrupt service routines prototypes. 

**     Settings  :
**          Bean name                                      : MIC_MC1321X
**          Device                                         : ATD1
**          Prescaler                                      : 4
**          Sample time                                    : 14.6 us
**          Conversion mode                                : Continuous conversion
**          Result data formats                            : 10-bit/left justified/signed
**          ADC Channels                                   : 1
**          Pins                                           : PTB2_AD2
**          Interrupt                                      : Vatd1
**          Conversion complete interrupt                  : Disabled
**          ISR name                                       : testadc
**          Initial channel select                         : Channel 2
**          Call Init method                               : yes
**          Enable module                                  : Enabled

**     Contents  :
**         Init - void MIC_MC1321X_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2006
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE MIC_MC1321X. */

#include "MIC_MC1321X.h"

/*
** ###################################################################
**
**  The interrupt service routine(s) must be implemented
**  by user in one of the following user modules.
**
**  If the "Generate ISR" option is enabled, Processor Expert generates
**  ISR templates in the CPU event module.
**
**  User modules:
**      FlyWeight.c
**
** ###################################################################
ISR( testadc)
{
  // NOTE: The routine should include the following actions to obtain
  //       correct functionality of the hardware.
  //
  // The interrupt will remain pending as long as the CCF flag is set.
  // The CCF bit is cleared whenever the ATD status and control (ATD1SC) register is written.
  // The CCF bit is also cleared whenever the ATD result registers (ATD1RH or ATD1RL) are read.
  // Example: ATD1SC_ATDIE = 0;


}
*/

/*
** ===================================================================
**     Method      :  MIC_MC1321X_Init (bean Init_ADC)
**
**     Description :
**         This method initializes registers of the ADC module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in user code to initialize the module.
**         By default, the method is called by PE automatically; see
**         "Call Init method" property of the bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void MIC_MC1321X_Init(void)
{
  /* ATD1PE: ATDPE7=0,ATDPE6=0,ATDPE5=0,ATDPE4=0,ATDPE3=0,ATDPE2=1,ATDPE1=0,ATDPE0=0 */
  setReg8(ATD1PE, 0x04);               /* Write stops the conversion */ 
  /* ATD1C: ATDPU=1,DJM=0,RES8=0,SGN=1,PRS3=0,PRS2=1,PRS1=0,PRS0=0 */
  setReg8(ATD1C, 0x94);                /* Write stops the conversion */ 
  /* ATD1SC: CCF=0,ATDIE=0,ATDCO=1,ATDCH4=0,ATDCH3=0,ATDCH2=0,ATDCH1=1,ATDCH0=0 */
  setReg8(ATD1SC, 0x22);               /* Write starts a new conversion */ 
}
/* END MIC_MC1321X. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.01 [03.92]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
