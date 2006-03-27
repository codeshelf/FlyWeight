/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Byte1.H
**     Project   : RTOSDemo
**     Processor : MC9S08GT60CFB
**     Beantype  : ByteIO
**     Version   : Bean 02.053, Driver 03.08, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/7/2006, 6:48 AM
**     Abstract  :
**         This bean "ByteIO" implements an one-byte input/output.
**         It uses one 8-bit port.
**         Note: This bean is set to work in Output direction only.
**         Methods of this bean are mostly implemented as a macros 
**         (if supported by target langauage and compiler).
**     Settings  :
**         Port name                   : PTB
**
**         Initial direction           : Output (direction cannot be changed)
**         Initial output value        : 0 = 000H
**         Initial pull option         : off
**
**         8-bit data register         : PTBD      [0004]
**         8-bit control register      : PTBDD     [0007]
**
**             ----------------------------------------------------
**                   Bit     |   Pin   |   Name
**             ----------------------------------------------------
**                    0      |    22   |   PTB0_AD1P0
**                    1      |    23   |   PTB1_AD1P1
**                    2      |    24   |   PTB2_AD1P2
**                    3      |    25   |   PTB3_AD1P3
**                    4      |    26   |   PTB4_AD1P4
**                    5      |    27   |   PTB5_AD1P5
**                    6      |    28   |   PTB6_AD1P6
**                    7      |    29   |   PTB7_AD1P7
**             ----------------------------------------------------
**     Contents  :
**         PutBit - void Byte1_PutBit(byte Bit,bool Val);
**         NegBit - void Byte1_NegBit(byte Bit);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef Byte1_H_
#define Byte1_H_

/* MODULE Byte1. */

  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Cpu.h"


/*
** ===================================================================
**     Method      :  Byte1_PutBit (bean ByteIO)
**
**     Description :
**         This method writes the new value to the specified bit
**         of the output value.
**     Parameters  :
**         NAME       - DESCRIPTION
**         Bit        - Number of the bit (0 to 7)
**         Val        - New value of the bit (FALSE or TRUE)
**                      FALSE = "0" or "Low", TRUE = "1" or "High"
**     Returns     : Nothing
** ===================================================================
*/
void Byte1_PutBit(byte Bit, bool Val);

/*
** ===================================================================
**     Method      :  Byte1_NegBit (bean ByteIO)
**
**     Description :
**         This method negates (inverts) the specified bit of the
**         output value.
**     Parameters  :
**         NAME       - DESCRIPTION
**         Bit        - Number of the bit to invert (0 to 7)
**     Returns     : Nothing
** ===================================================================
*/
void Byte1_NegBit(byte Bit);



/* END Byte1. */
#endif /* #ifndef __Byte1_H_ */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
