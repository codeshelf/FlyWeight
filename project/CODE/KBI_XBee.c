/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : KBI_XBee.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : Init_KBI
**     Version   : Bean 01.022, Driver 01.09, CPU db: 2.87.125
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 12/1/2010, 1:45 PM
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
**          Bean name                                      : KBI_XBee
**          Device                                         : KBI1
**          Settings 
**            Triggering sensitivity                       : edge
**          Pins 
**            Pin0                                         : Disabled
**            Pin1                                         : Disabled
**            Pin2                                         : Disabled
**            Pin3                                         : Disabled
**            Pin4                                         : Disabled
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
**         Init - void KBI_XBee_Init(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE KBI_XBee. */

#include "KBI_XBee.h"

/*
** ###################################################################
**
**  The interrupt service routine(s) must be implemented
**  by user in one of the following user modules.
**
**  If the "Generate ISR" option is enabled, Processor Expert generates
**  ISR templates in the CPU event module.
**
**  User modules:
**      FlyWeight.c
**      Events.c
**
** ###################################################################
ISR(keyboardISR)
{
  // NOTE: The routine should include the following actions to obtain
  //       correct functionality of the hardware.
  //
  //  Keyboard Interrupt Flag is cleared by writing a 1 to the
  //  KBACK control bit.
  // Example:   KBI1SC_KBACK = 0x01
}
*/

/*
** ===================================================================
**     Method      :  KBI_XBee_Init (bean Init_KBI)
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
void KBI_XBee_Init(void)
{
  /* KBI1SC: KBIE=0 */
  clrReg8Bits(KBI1SC, 0x02);            
  /* KBI1SC: KBEDG7=0,KBEDG6=0,KBEDG5=1,KBEDG4=0,KBIMOD=0 */
  clrSetReg8Bits(KBI1SC, 0xD1, 0x20);   
  /* KBI1PE: KBIPE7=0,KBIPE6=0,KBIPE5=1,KBIPE4=0,KBIPE3=0,KBIPE2=0,KBIPE1=0,KBIPE0=0 */
  setReg8(KBI1PE, 0x20);                
  /* KBI1SC: KBACK=1 */
  setReg8Bits(KBI1SC, 0x04);            
  /* KBI1SC: KBIE=1 */
  setReg8Bits(KBI1SC, 0x02);            
}

/* END KBI_XBee. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
