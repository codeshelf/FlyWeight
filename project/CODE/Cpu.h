/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Cpu.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : MC9S08GT60_44
**     Version   : Bean 01.101, Driver 01.21, CPU db: 2.87.086
**     Datasheet : MC9S08GB60/D Rev. 2.3 12/2004
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/28/2006, 5:20 PM
**     Abstract  :
**         This bean "MC9S08GT60_44" contains initialization of the
**         CPU and provides basic methods and events for CPU core
**         settings.
**     Settings  :
**
**     Contents  :
**         SetHighSpeed - void Cpu_SetHighSpeed(void);
**         SetLowSpeed  - void Cpu_SetLowSpeed(void);
**         EnableInt    - void Cpu_EnableInt(void);
**         DisableInt   - void Cpu_DisableInt(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __Cpu
#define __Cpu

/* Active configuration define symbol */
#define PEcfg_XBeeRemote 1


/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PE_Timer.h"

/* MODULE Cpu. */


#ifndef __BWUserType_tPowerDownModes
#define __BWUserType_tPowerDownModes
  typedef enum {                       /*  */
    PowerDown,
    PartialPowerDown,
    StandBy
  } tPowerDownModes;
#endif

/* Global variables */
extern volatile byte CCR_reg;          /* Current CCR reegister */
extern byte CpuMode;                   /* Current speed mode */




#define   Cpu_DisableInt()  __DI()     /* Disable interrupts */
/*
** ===================================================================
**     Method      :  Cpu_DisableInt (bean MC9S08GT60_44)
**
**     Description :
**         Disables maskable interrupts
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

#define   Cpu_EnableInt()  __EI()      /* Enable interrupts */
/*
** ===================================================================
**     Method      :  Cpu_EnableInt (bean MC9S08GT60_44)
**
**     Description :
**         Enables maskable interrupts
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void Cpu_SetLowSpeed(void);
/*
** ===================================================================
**     Method      :  Cpu_SetLowSpeed (bean MC9S08GT60_44)
**
**     Description :
**         Sets the low speed mode. The method is enabled only if
**         low speed mode is enabled in the bean.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void Cpu_SetHighSpeed(void);
/*
** ===================================================================
**     Method      :  Cpu_SetHighSpeed (bean MC9S08GT60_44)
**
**     Description :
**         Sets the high speed mode. The method is enabled only if
**         low or slow speed mode is enabled in the bean as well.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

__interrupt void Cpu_Interrupt(void);
/*
** ===================================================================
**     Method      :  Cpu_Interrupt (bean MC9S08GT60_44)
**
**     Description :
**         The method services the interrupt of the selected peripheral(s)
**         and eventually invokes the beans event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

__interrupt void Cpu_OnClockMonitorInt(void);
/*
** ===================================================================
**     Method      :  Cpu_OnClockMonitorInt (bean MC9S08GT60_44)
**
**     Description :
**         This event is envoked if OnClockMonitor flag is changed.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void PE_low_level_init(void);
/*
** ===================================================================
**     Method      :  PE_low_level_init (bean MC9S08GT60_44)
**
**     Description :
**         Initializes beans and provides common register initialization. 
**         The method is called automatically as a part of the 
**         application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

/* END Cpu. */

#endif /* ifndef __Cpu */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
