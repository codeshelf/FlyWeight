/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : PE_Const.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : PE_Const
**     Version   : Driver 01.00
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/5/2006, 2:52 PM
**     Abstract  :
**         This bean "PE_Const" contains internal definitions
**         of the constants.
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

#ifndef __PE_Const_H
#define __PE_Const_H

/* Constants for detecting running mode */
#define HIGH_SPEED        0x00         /* High speed */
#define LOW_SPEED         0x01         /* Low speed */
#define SLOW_SPEED        0x02         /* Slow speed */

/* Reset cause constants */
#define RSTSRC_POR        0x80         /* Power-on reset        */
#define RSTSRC_PIN        0x40         /* External reset bit    */
#define RSTSRC_COP        0x20         /* COP reset             */
#define RSTSRC_ILOP       0x10         /* Illegal opcode reset  */
#define RSTSRC_ILAD       0x08         /* Illegal address reset */
#define RSTSRC_MODRST     0x04         /* Normal Monitor Mode Entry Reset*/
#define RSTSRC_MENRST     0x04         /* Forced Monitor Mode Entry Reset*/
#define RSTSRC_LVI        0x02         /* Low voltage inhibit reset */

#endif /* _PE_Const_H */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
