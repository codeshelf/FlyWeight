/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : MC13191IRQ.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : InterruptVector
**     Version   : Bean 02.022, Driver 01.06, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/29/2006, 3:51 PM
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

#ifndef __MC13191IRQ
#define __MC13191IRQ

/* MODULE MC13191IRQ. */

#include "PE_types.h"

/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see MC13191IRQ.c file for more information).
** ===================================================================
*/
__interrupt void IRQIsr(void);

/* END MC13191IRQ. */

#endif /* ifndef __MC13191IRQ */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
