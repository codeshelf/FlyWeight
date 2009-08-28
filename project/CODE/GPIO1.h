/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : GPIO1.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : Init_GPIO
**     Version   : Bean 01.025, Driver 01.12, CPU db: 2.87.109
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 8/28/2009, 4:26 PM
**     Abstract  :
**          This file implements the General Purpose Input Output (PTA)
**          module initialization according to the Peripheral Initialization
**          Bean settings, and defines interrupt service routines prototypes.
**
**     Settings  :
**          Bean name                                      : GPIO1
**          Device                                         : PTA
**          Port control                                   : Individual pins
**
**          Used pins                                      :
**             ----------------------------------------------------
**                Number (on package)  |    Name
**             ----------------------------------------------------
**                       0             |  PTA0_KBI1P0
**                       1             |  PTA1_KBI1P1
**                       2             |  PTA2_KBI1P2
**                       3             |  PTA3_KBI1P3
**                       4             |  PTA4_KBI1P4
**             ----------------------------------------------------
**
**          Pin0                                           : PTA0_KBI1P0
**            Direction                                    : Output
**            Output value                                 : no initialization
**            Pull resistor                                : no initialization
**            Open drain                                   : push-pull
**
**          Pin1                                           : PTA1_KBI1P1
**            Direction                                    : Input
**            Output value                                 : no initialization
**            Pull resistor                                : no initialization
**            Open drain                                   : push-pull
**
**          Pin2                                           : PTA2_KBI1P2
**            Direction                                    : Input
**            Output value                                 : no initialization
**            Pull resistor                                : no initialization
**            Open drain                                   : push-pull
**
**          Pin3                                           : PTA3_KBI1P3
**            Direction                                    : Input
**            Output value                                 : no initialization
**            Pull resistor                                : no initialization
**            Open drain                                   : push-pull
**
**          Pin4                                           : PTA4_KBI1P4
**            Direction                                    : Input
**            Output value                                 : no initialization
**            Pull resistor                                : no initialization
**            Open drain                                   : push-pull
**
**          Call Init method                               : yes
**     Contents  :
**         Init - void GPIO1_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __GPIO1
#define __GPIO1

/* MODULE GPIO1. */

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */

#include "Cpu.h"


void GPIO1_Init(void);
/*
** ===================================================================
**     Method      :  GPIO1_Init (bean Init_GPIO)
**
**     Description :
**         This method initializes registers of the GPIO module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in the user code to initialize the
**         module. By default, the method is called by PE
**         automatically; see "Call Init method" property of the
**         bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/* END GPIO1. */
#endif /* ifndef __GPIO1 */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
