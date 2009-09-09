/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : RTI1.H
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_RTI
**     Version   : Bean 01.045, Driver 01.14, CPU db: 2.87.125
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 9/8/2009, 10:57 AM
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
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __RTI1
#define __RTI1

/* MODULE RTI1. */

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */

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
void RTI1_Init(void);

/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see RTI1.c file for more information).
** ===================================================================
*/
__interrupt void dispatchRTI(void);

/* END RTI1. */

#endif /* ifndef __RTI1 */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
