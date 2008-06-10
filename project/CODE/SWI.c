/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : SWI.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : InterruptVector
**     Version   : Bean 02.022, Driver 01.07, CPU db: 2.87.123
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 5/26/2008, 11:58 AM
**     Abstract  :
**         This bean "InterruptVector" gives an access to interrupt vector.
**         The purpose of this bean is to allocate the interrupt vector
**         in the vector table. Additionally it can provide settings of
**         the interrupt priority register.
**         The interrupt handling routines must be implemented by the user.
**     Settings  :
**         Interrupt vector            : Vswi
**         Handle name                 : vPortYield
**     Contents  :
**         No public methods
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2006
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


/* MODULE SWI. */

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

ISR(vPortYield)
{
}
*/

/* END SWI. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.01 [03.92]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
