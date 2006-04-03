/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : AudioLoader.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : TimerInt
**     Version   : Bean 02.110, Driver 01.08, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/3/2006, 1:40 AM
**     Abstract  :
**         This bean "TimerInt" implements a periodic interrupt.
**         When the bean and its events are enabled, the "OnInterrupt"
**         event is called periodically with the period that you specify.
**         TimerInt supports also changing the period in runtime.
**         The source of periodic interrupt can be timer compare or reload
**         register or timer-overflow interrupt (of free running counter).
**     Settings  :
**         Timer name                  : TPM2 (16-bit)
**         Compare name                : TPM21
**         Counter shared              : Yes
**
**         High speed mode
**             Prescaler               : divide-by-4
**             Clock                   : 4998857 Hz
**           Initial period/frequency
**             Xtal ticks              : 24
**             microseconds            : 100
**             seconds (real)          : 0.0001000
**             Hz                      : 9998
**             kHz                     : 10
**
**         Runtime setting             : none
**
**         Initialization:
**              Timer                  : Enabled
**              Events                 : Enabled
**
**         Timer registers
**              Counter                : TPM2CNT   [0061]
**              Mode                   : TPM2SC    [0060]
**              Run                    : TPM2SC    [0060]
**              Prescaler              : TPM2SC    [0060]
**
**         Compare registers
**              Compare                : TPM2C1V   [0069]
**
**         Flip-flop registers
**              Mode                   : TPM2C1SC  [0068]
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

#ifndef __AudioLoader
#define __AudioLoader

/* MODULE AudioLoader. */

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PE_Timer.h"
#include "Cpu.h"

__interrupt void AudioLoader_Interrupt(void);
/*
** ===================================================================
**     Method      :  AudioLoader_Interrupt (bean TimerInt)
**
**     Description :
**         The method services the interrupt of the selected peripheral(s)
**         and eventually invokes the beans event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void AudioLoader_Init(void);
/*
** ===================================================================
**     Method      :  AudioLoader_Init (bean TimerInt)
**
**     Description :
**         Initializes the associated peripheral(s) and the beans 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

#define AudioLoader_SetCV(_Val) { \
  TPM2C1V = (word)(TPM2CNT + _Val);  \
  CmpVal = (word)_Val; \
}
/* END AudioLoader. */

#endif /* ifndef __AudioLoader */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
