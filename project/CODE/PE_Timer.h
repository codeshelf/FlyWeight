/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : PE_Timer.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : PE_Timer
**     Version   : Driver 01.02
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/3/2006, 1:40 AM
**     Abstract  :
**         This module "PE_Timer" implements internal methods and definitions
**         used by beans working with timers.
**     Settings  :
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
#ifndef __PE_Timer
#define __PE_Timer
/*Include shared modules, which are used for whole project*/
#include "PE_types.h"
#include "PE_const.h"

/* MODULE PE_Timer. */

bool PE_Timer_LngHi1(dword Low, dword High, word *Out);
/*
** ===================================================================
**     Method      :  PE_Timer_LngHi1 (bean PE_Timer)
**
**     Description :
**         The method transfers 64 bit result to 16 bit ratio value and 
**         returns overflow flag.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/


#endif /* END PE_Timer. */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
