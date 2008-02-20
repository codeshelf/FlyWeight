/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : MIC_MC1321X.H
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_ADC
**     Version   : Bean 01.062, Driver 01.06, CPU db: 2.87.087
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 2/19/2008, 5:00 PM
**     Abstract  :
**          This file implements the ADC (ATD) module initialization 
**          according to the Peripheral Initialization Bean settings, and defines
**          interrupt service routines prototypes. 

**     Settings  :
**          Bean name                                      : MIC_MC1321X
**          Device                                         : ATD1
**          Prescaler                                      : 4
**          Sample time                                    : 14.6 us
**          Conversion mode                                : Continuous conversion
**          Result data formats                            : 10-bit/right justified/unsigned
**          ADC Channels                                   : 1
**          Pins                                           : PTB2_AD2
**          Interrupt                                      : Vatd
**          Conversion complete interrupt                  : Enabled
**          ISR name                                       : testadc
**          Initial channel select                         : Channel 2
**          Call Init method                               : yes
**          Enable module                                  : Enabled

**     Contents  :
**         Init - void MIC_MC1321X_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


#ifndef __MIC_MC1321X
#define __MIC_MC1321X

/* MODULE MIC_MC1321X. */

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */

#include "Cpu.h"


/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see MIC_MC1321X.c file for more information).
** ===================================================================
*/
__interrupt void testadc(void);

void MIC_MC1321X_Init(void);
/*
** ===================================================================
**     Method      :  MIC_MC1321X_Init (bean Init_ADC)
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

/* END MIC_MC1321X. */

#endif /* ifndef __MIC_MC1321X */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
