/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : WatchDog.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : WatchDog
**     Version   : Bean 02.087, Driver 01.10, CPU db: 2.87.125
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 12/1/2010, 1:27 PM
**     Abstract  :
**         This device "WatchDog" implements a watchdog.
**         When enabled, the watchdog timer counts until it reaches
**         a critical value corresponding to the period specified
**         in 'Properties'. Then, the watchdog causes a CPU reset.
**         Applications may clear the timer before it reaches the critical
**         value. The timer then restarts the watchdog counter.
**         Watchdog is used for increasing the safety of the system
**         (unforeseeable situations can cause system crash or an
**         infinite loop - watchdog can restart the system or handle
**         the situation).
**
**         Note: Watchdog can be enabled or disabled in the initialization
**               code only. If the watchdog is once enabled user have
**               to call Clear method in defined time intervals.
**     Settings  :
**         Watchdog causes             : Reset CPU
**
**         Initial Watchdog state      : Enabled
**
**         Mode of operation           : Normal
**
**
**         High speed mode
**           Watchdog period/frequency
**             microseconds            : 13107
**             milliseconds            : 13
**             Hz                      : 76
**
**         Run register                : SOPT      [$1802]
**         Mode register               : SRS       [$1800]
**         Prescaler register          : SOPT      [$1802]
**     Contents  :
**         Clear - byte WatchDog_Clear(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


/* MODULE WatchDog. */

#include "WatchDog.h"

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
/*
void WatchDog_Clear(void)

**      This method is implemented as macro      **
*/

/* END WatchDog. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
