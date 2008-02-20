/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : KBI_MC1321X.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_KBI
**     Version   : Bean 01.009, Driver 01.01, CPU db: 2.87.087
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 2/19/2008, 5:00 PM
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
**              Interrupt                                  : Vkeyboard
**              Keyboard request interrupt                 : Disabled
**              ISR name                                   : 
**          Initialization 
**            Call Init method                             : yes
**     Contents  :
**         Init - void KBI_MC1321X_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE KBI_MC1321X. */

#include "KBI_MC1321X.h"

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
void KBI_MC1321X_Init(void)
{
  /* KBISC: KBIE=0 */
  clrReg8Bits(KBISC, 0x02);             
  /* KBIPE: KBIPE7=0,KBIPE6=0,KBIPE5=1,KBIPE4=1,KBIPE3=0,KBIPE2=0,KBIPE1=0,KBIPE0=0 */
  setReg8(KBIPE, 0x30);                 
  /* KBISC: KBEDG5=1,KBEDG4=1 */
  setReg8Bits(KBISC, 0x30);             

  /* KBISC: KBACK=1 */
  setReg8Bits(KBISC, 0x04);             
  /* KBISC: KBIE=0 */
  clrReg8Bits(KBISC, 0x02);             
}

/* END KBI_MC1321X. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
