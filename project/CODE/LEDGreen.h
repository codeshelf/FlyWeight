/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : LEDGreen.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : BitIO
**     Version   : Bean 02.071, Driver 03.16, CPU db: 2.87.109
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 8/19/2009, 2:21 PM
**     Abstract  :
**         This bean "BitIO" implements an one-bit input/output.
**         It uses one bit/pin of a port.
**         Note: This bean is set to work in Output direction only.
**         Methods of this bean are mostly implemented as a macros
**         (if supported by target language and compiler).
**     Settings  :
**         Used pin                    :
**             ----------------------------------------------------
**                Number (on package)  |    Name
**             ----------------------------------------------------
**                       28            |  PTB3_AD1P3
**             ----------------------------------------------------
**
**         Port name                   : PTB
**
**         Bit number (in port)        : 3
**         Bit mask of the port        : $0008
**
**         Initial direction           : Output (direction cannot be changed)
**         Initial output value        : 0
**         Initial pull option         : off
**
**         Port data register          : PTBD      [$0004]
**         Port control register       : PTBDD     [$0007]
**
**         Optimization for            : speed
**     Contents  :
**         ClrVal - void LEDGreen_ClrVal(void);
**         SetVal - void LEDGreen_SetVal(void);
**         NegVal - void LEDGreen_NegVal(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef LEDGreen_H_
#define LEDGreen_H_

/* MODULE LEDGreen. */

  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Cpu.h"


/*
** ===================================================================
**     Method      :  LEDGreen_ClrVal (bean BitIO)
**
**     Description :
**         This method clears (sets to zero) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LEDGreen_ClrVal() ( \
    (void)clrReg8Bits(PTBD, 0x08)      /* PTBD3=0x00 */ \
  )

/*
** ===================================================================
**     Method      :  LEDGreen_SetVal (bean BitIO)
**
**     Description :
**         This method sets (sets to one) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LEDGreen_SetVal() ( \
    (void)setReg8Bits(PTBD, 0x08)      /* PTBD3=0x01 */ \
  )

/*
** ===================================================================
**     Method      :  LEDGreen_NegVal (bean BitIO)
**
**     Description :
**         This method negates (inverts) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LEDGreen_NegVal() ( \
    (void)invertReg8Bits(PTBD, 0x08)   /* PTBD3=invert */ \
  )



/* END LEDGreen. */
#endif /* #ifndef __LEDGreen_H_ */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
