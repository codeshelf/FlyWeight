/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : LowVoltage.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : InterruptVector
**     Version   : Bean 02.022, Driver 01.07, CPU db: 2.87.109
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 8/25/2009, 4:25 PM
**     Abstract  :
**         This bean "InterruptVector" gives an access to interrupt vector.
**         The purpose of this bean is to allocate the interrupt vector
**         in the vector table. Additionally it can provide settings of
**         the interrupt priority register.
**         The interrupt handling routines must be implemented by the user.
**     Settings  :
**         Interrupt vector            : Vlvd
**         Handle name                 : LowVoltageDetect
**         Interrupt request reg.      : SPMSC1    [$1809]
**         Interrupt enable reg.       : SPMSC1    [$1809]
**     Contents  :
**         No public methods
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __LowVoltage
#define __LowVoltage

/* MODULE LowVoltage. */

#include "PE_types.h"

/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see LowVoltage.c file for more information).
** ===================================================================
*/
__interrupt void LowVoltageDetect(void);

/* END LowVoltage. */

#endif /* ifndef __LowVoltage */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
