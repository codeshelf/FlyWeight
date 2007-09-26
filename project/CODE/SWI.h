/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : SWI.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : InterruptVector
**     Version   : Bean 02.022, Driver 01.06, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 9/25/2007, 11:50 AM
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
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __SWI
#define __SWI

/* MODULE SWI. */

#include "PE_types.h"

/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see SWI.c file for more information).
** ===================================================================
*/
__interrupt void vPortYield(void);

/* END SWI. */

#endif /* ifndef __SWI */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
