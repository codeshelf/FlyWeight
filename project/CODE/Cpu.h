/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Cpu.H
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : MC13214
**     Version   : Bean 01.065, Driver 01.29, CPU db: 2.87.123
**     Datasheet : MC1321xRM Rev. 1.1 10/2006
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 8/8/2008, 6:25 PM
**     Abstract  :
**         This bean "MC13214" contains initialization of the
**         CPU and provides basic methods and events for CPU core
**         settings.
**     Settings  :
**
**     Contents  :
**         SetHighSpeed - void Cpu_SetHighSpeed(void);
**         SetSlowSpeed - void Cpu_SetSlowSpeed(void);
**         EnableInt    - void Cpu_EnableInt(void);
**         DisableInt   - void Cpu_DisableInt(void);
**         Delay100US   - void Cpu_Delay100US(word us100);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2006
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
#define PEcfg_GW0003Gateway 1


/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

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
extern volatile byte CCR_reg;          /* Current CCR register */
extern byte CpuMode;                   /* Current speed mode */


__interrupt void Cpu_VicgInterrupt(void);
/*
** ===================================================================
**     Method      :  Cpu_VicgInterrupt (bean MC13214)
**
**     Description :
**         This ISR services the 'Loss of lock' or the 'Loss of clock' 
**         interrupt.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

__interrupt void Cpu_Interrupt(void);
/*
** ===================================================================
**     Method      :  Cpu_Interrupt (bean MC13214)
**
**     Description :
**         The method services unhandled interrupt vectors.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

#define   Cpu_DisableInt()  __DI()     /* Disable interrupts */
/*
** ===================================================================
**     Method      :  Cpu_DisableInt (bean MC13214)
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
**     Method      :  Cpu_EnableInt (bean MC13214)
**
**     Description :
**         Enables maskable interrupts
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void Cpu_Delay100US(word us100);
/*
** ===================================================================
**     Method      :  Cpu_Delay100US (bean MC13214)
**
**     Description :
**         This method realizes software delay. The length of delay
**         is at least 100 microsecond multiply input parameter
**         [us100]. As the delay implementation is not based on real
**         clock, the delay time may be increased by interrupt
**         service routines processed during the delay. The method
**         is independent on selected speed mode.
**     Parameters  :
**         NAME            - DESCRIPTION
**         us100           - Number of 100 us delay repetitions.
**     Returns     : Nothing
** ===================================================================
*/

void Cpu_SetSlowSpeed(void);
/*
** ===================================================================
**     Method      :  Cpu_SetSlowSpeed (bean MC13214)
**
**     Description :
**         Sets the slow speed mode. The method is enabled only if
**         <Slow speed mode> is enabled in the bean.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void Cpu_SetHighSpeed(void);
/*
** ===================================================================
**     Method      :  Cpu_SetHighSpeed (bean MC13214)
**
**     Description :
**         Sets the high speed mode. The method is enabled only if <Low
**         speed mode> or <Slow speed mode> are enabled in the bean as
**         well.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void PE_low_level_init(void);
/*
** ===================================================================
**     Method      :  PE_low_level_init (bean MC13214)
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
**     This file was created by UNIS Processor Expert 3.01 [03.92]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
