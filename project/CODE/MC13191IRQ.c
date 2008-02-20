/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : MC13191IRQ.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : InterruptVector
**     Version   : Bean 02.022, Driver 01.06, CPU db: 2.87.087
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 2/19/2008, 5:00 PM
**     Abstract  :
**         This bean "InterruptVector" gives an access to interrupt vector.
**         The purpose of this bean is to allocate the interrupt vector
**         in the vector table. Additionally it can provide settings of
**         the interrupt priority register.
**         The interrupt handling routines must be implemented by the user.
**     Settings  :
**         Interrupt vector            : Virq
**         Handle name                 : IRQIsr
**         Interrupt request reg.      : IRQSC     [0014]
**         Interrupt enable reg.       : IRQSC     [0014]
**     Contents  :
**         No public methods
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


/* MODULE MC13191IRQ. */

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

ISR(IRQIsr)
{
}
*/

/* END MC13191IRQ. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
