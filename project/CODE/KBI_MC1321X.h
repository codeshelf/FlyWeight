/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : KBI_MC1321X.H
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_KBI
**     Version   : Bean 01.020, Driver 01.07, CPU db: 2.87.123
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 5/26/2008, 11:58 AM
**     Abstract  :
**          This file implements the KBI (KBI) module initialization
**          according to the Peripheral Initialization Bean settings, and defines
**          interrupt service routines prototypes.
**          The KBI module is the serial peripheral interface module,which allows 
**          full-duplex, synchronous, serial communications with peripheral devices.
**          The keyboard interrupt module (KBI) provides a few independently maskable 
**          external interrupts. When a port pin is enabled for keyboard interrupt 
**          function, an internal pullup/pulldown device is also enabled on the pin.

**     Settings  :
**          Bean name                                      : KBI_MC1321X
**          Device                                         : KBI1
**          Settings 
**            Triggering sensitivity                       : edge
**          Pins 
**            Pin0                                         : Disabled
**            Pin1                                         : Disabled
**            Pin2                                         : Disabled
**            Pin3                                         : Disabled
**            Pin4                                         : Enabled
**              Pin                                        : PTA4_KBIP4
**              Pin signal                                 : 
**            Pin5                                         : Enabled
**              Pin                                        : PTA5_KBIP5
**              Pin signal                                 : 
**            Pin6                                         : Disabled
**            Pin7                                         : Disabled
**          Interrupts 
**            Keyboard request 
**              Interrupt                                  : Vkeyboard1
**              Keyboard request interrupt                 : Enabled
**              ISR name                                   : keyboardISR
**          Initialization 
**            Call Init method                             : yes
**     Contents  :
**         Init - void KBI_MC1321X_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2006
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __KBI_MC1321X
#define __KBI_MC1321X

/* MODULE KBI_MC1321X. */

/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */

#include "Cpu.h"


/*
** ===================================================================
** The interrupt service routine must be implemented by user in one
** of the user modules (see KBI_MC1321X.c file for more information).
** ===================================================================
*/
__interrupt void keyboardISR(void);

void KBI_MC1321X_Init(void);
/*
** ===================================================================
**     Method      :  KBI_MC1321X_Init (bean Init_KBI)
**
**     Description :
**         This method initializes registers of the KBI module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in user code to initialize the module.
**         By default, the method is called by PE automatically; see
**         "Call Init method" property of the bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/* END KBI_MC1321X. */

#endif
/* ifndef __KBI_MC1321X */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.01 [03.92]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
