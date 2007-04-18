/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : LED1.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : BitIO
**     Version   : Bean 02.066, Driver 03.08, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/17/2007, 6:38 PM
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
**                       20            |  PTD0_TPM1CH0
**             ----------------------------------------------------
**
**         Port name                   : PTD
**
**         Bit number (in port)        : 0
**         Bit mask of the port        : 0001
**
**         Initial direction           : Output (direction cannot be changed)
**         Initial output value        : 1
**         Initial pull option         : off
**
**         Port data register          : PTDD      [000C]
**         Port control register       : PTDDD     [000F]
**
**         Optimization for            : speed
**     Contents  :
**         ClrVal - void LED1_ClrVal(void);
**         SetVal - void LED1_SetVal(void);
**         NegVal - void LED1_NegVal(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef LED1_H_
#define LED1_H_

/* MODULE LED1. */

  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Cpu.h"


/*
** ===================================================================
**     Method      :  LED1_ClrVal (bean BitIO)
**
**     Description :
**         This method clears (sets to zero) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED1_ClrVal() ( \
    (void)clrReg8Bits(PTDD, 0x01)      /* PTDD0=0x00 */ \
  )

/*
** ===================================================================
**     Method      :  LED1_SetVal (bean BitIO)
**
**     Description :
**         This method sets (sets to one) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED1_SetVal() ( \
    (void)setReg8Bits(PTDD, 0x01)      /* PTDD0=0x01 */ \
  )

/*
** ===================================================================
**     Method      :  LED1_NegVal (bean BitIO)
**
**     Description :
**         This method negates (inverts) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED1_NegVal() ( \
    (void)invertReg8Bits(PTDD, 0x01)   /* PTDD0=invert */ \
  )



/* END LED1. */
#endif /* #ifndef __LED1_H_ */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
