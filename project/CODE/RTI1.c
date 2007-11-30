/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : RTI1.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : Init_RTI
**     Version   : Bean 01.030, Driver 01.06, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 11/29/2007, 4:26 PM
**     Abstract  :
**          This file implements the RTI (RTI) module initialization
**          according to the Peripheral Initialization Bean settings,
**          and defines interrupt service routines prototypes.
**     Settings  :
**          Bean name                                      : RTI1
**          Timer                                          : RTI
**          Settings                                       : 
**          Clock settings                                 : 
**          Clock Select                                   : External clock
**          Prescaler                                      : 4096
**          Period                                         : 1.024 ms
**          Interrupts                                     : 
**          Interrupt                                      : Vrti
**          Real-Time Interrupt                            : Enabled
**          ISR Name                                       : dispatchRTI
**          Initialization                                 : 
**          Call Init Method                               : no
**     Contents  :
**         Init - void RTI1_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE RTI1. */

#include "RTI1.h"
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

      ISR(dispatchRTI)
      {
      // NOTE: The routine should include the following actions to obtain
      //       correct functionality of the hardware.
      //
      //       You should write 1 to RTIACK to clear RTIF. Writing 0 has no meaning or effect.
      //       Example:   SRTISC_RTIACK = 1;
      }
*/

/*
** ===================================================================
**     Method      :  RTI1_Init (bean Init_RTI)
**
**     Description :
**         This method initializes registers of the RTI module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in the user code to initialize the
**         module. By default, the method is called by PE
**         automatically; see "Call Init method" property of the
**         bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void RTI1_Init(void)
{
  /* SRTISC: RTIF=0,RTIACK=0,RTICLKS=1,RTIE=1,??=0,RTIS2=1,RTIS1=0,RTIS0=0 */
  setReg8(SRTISC, 0x34);                
}

/* END RTI1. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
