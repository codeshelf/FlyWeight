/** ###################################################################
**     Filename  : FlyWeight.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Version   : Driver 01.09
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/12/2006, 4:32 PM
**     Abstract  :
**         Main module.
**         Here is to be placed user's code.
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
/* MODULE FlyWeight */


/* Including used modules for compiling procedure */
#include "Cpu.h"
#include "Events.h"
#include "RTI1.h"
#include "SWI.h"
#include "MC13191IRQ.h"
#include "SW2.h"
#include "SW3.h"
#include "SW4.h"
#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "USB.h"
#include "PWM.h"
#include "AudioOut.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

extern void vMain( void );

void main(void)
{
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* Just jump to the real main(). */
  __asm
  {
  	 jmp vMain
  }
  

  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END FlyWeight */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
