/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : WatchDog.H
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : WatchDog
**     Version   : Bean 02.060, Driver 01.04, CPU db: 2.87.086
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 10/24/2007, 7:48 PM
**     Abstract  :
**     Settings  :
**         Watchdog causes             : Reset CPU
**
**         Initial Watchdog state      : Enabled
**
**
**
**         High speed mode
**             Prescaler               : divide-by-262144
**           Watchdog period/frequency
**             microseconds            : 13107
**             milliseconds            : 13
**             Hz                      : 76
**
**         Run register                : SOPT      [1802]
**         Mode register               : SRS       [1800]
**         Prescaler register          : SOPT      [1802]
**     Contents  :
**         Clear - byte WatchDog_Clear(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef __WatchDog
#define __WatchDog

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* MODULE WatchDog. */

#include "Cpu.h"


#define WatchDog_Clear() (SRS = 0xFF,ERR_OK)
/*
** ===================================================================
**     Method      :  WatchDog_Clear (bean WatchDog)
**
**     Description :
**         Clears the watchdog timer (it makes the timer restart from
**         zero).
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - The Bean is disabled
** ===================================================================
*/

/* END WatchDog. */

#endif /* ifndef __WatchDog */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
