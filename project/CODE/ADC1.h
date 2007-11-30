/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : ADC1.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : Init_ADC
**     Version   : Bean 01.062, Driver 01.06, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 11/24/2007, 3:26 PM
**     Abstract  :
**          This file implements the ADC (ATD1) module initialization 
**          according to the Peripheral Initialization Bean settings, and defines
**          interrupt service routines prototypes. 

**     Settings  :
**          Bean name                                      : ADC1
**          Device                                         : ATD1
**          Prescaler                                      : 10
**          Sample time                                    : 32 us
**          Conversion mode                                : Continuous conversion
**          Result data formats                            : 10-bit/left justified/unsigned
**          ADC Channels                                   : 1
**          Pins                                           : PTB2_AD1P2
**          Interrupt                                      : Vatd1
**          Conversion complete interrupt                  : Disabled
**          ISR name                                       : 
**          Initial channel select                         : Channel 2
**          Call Init method                               : yes
**          Enable module                                  : Enabled

**     Contents  :
**         Init - void ADC1_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


#ifndef __ADC1
#define __ADC1

/* MODULE ADC1. */

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */

#include "Cpu.h"



void ADC1_Init(void);
/*
** ===================================================================
**     Method      :  ADC1_Init (bean Init_ADC)
**
**     Description :
**         This method initializes registers of the ADC module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in user code to initialize the module.
**         By default, the method is called by PE automatically; see
**         "Call Init method" property of the bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/* END ADC1. */

#endif /* ifndef __ADC1 */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
