/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : LED4.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : BitIO
**     Version   : Bean 02.066, Driver 03.08, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/7/2006, 1:02 AM
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
**                       24            |  PTD4_TPM2CH1
**             ----------------------------------------------------
**
**         Port name                   : PTD
**
**         Bit number (in port)        : 4
**         Bit mask of the port        : 0010
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
**         ClrVal - void LED4_ClrVal(void);
**         SetVal - void LED4_SetVal(void);
**         NegVal - void LED4_NegVal(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef LED4_H_
#define LED4_H_

/* MODULE LED4. */

  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Cpu.h"


/*
** ===================================================================
**     Method      :  LED4_ClrVal (bean BitIO)
**
**     Description :
**         This method clears (sets to zero) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED4_ClrVal() ( \
    (void)clrReg8Bits(PTDD, 0x10)      /* PTDD4=0x00 */ \
  )

/*
** ===================================================================
**     Method      :  LED4_SetVal (bean BitIO)
**
**     Description :
**         This method sets (sets to one) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED4_SetVal() ( \
    (void)setReg8Bits(PTDD, 0x10)      /* PTDD4=0x01 */ \
  )

/*
** ===================================================================
**     Method      :  LED4_NegVal (bean BitIO)
**
**     Description :
**         This method negates (inverts) the output value.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED4_NegVal() ( \
    (void)invertReg8Bits(PTDD, 0x10)   /* PTDD4=invert */ \
  )



/* END LED4. */
#endif /* #ifndef __LED4_H_ */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
