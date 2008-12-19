/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Vectors.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Version   : Bean 01.065, Driver 01.31, CPU db: 2.87.125
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 12/19/2008, 9:52 AM
**     Abstract  :
**         This bean "MC13214" contains initialization of the
**         CPU and provides basic methods and events for CPU core
**         settings.
**     Settings  :
**
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


#include "Cpu.h"
#include "RTI1.h"
#include "SWI.h"
#include "MC13191IRQ.h"
#include "USB.h"
#include "LowVoltage.h"
extern near void _EntryPoint(void);

void (* near const _vect[])() @0xFFCC = { /* Interrupt vector table */
         dispatchRTI,                  /* Int.no. 25 Vrti (at FFCC)                  Used */
         Cpu_Interrupt,                /* Int.no. 24 Viic1 (at FFCE)                 Unassigned */
         Cpu_Interrupt,                /* Int.no. 23 Vatd1 (at FFD0)                 Unassigned */
         Cpu_Interrupt,                /* Int.no. 22 Vkeyboard1 (at FFD2)            Unassigned */
         Cpu_Interrupt,                /* Int.no. 21 Vsci2tx (at FFD4)               Unassigned */
         Cpu_Interrupt,                /* Int.no. 20 Vsci2rx (at FFD6)               Unassigned */
         Cpu_Interrupt,                /* Int.no. 19 Vsci2err (at FFD8)              Unassigned */
         USB_InterruptTx,              /* Int.no. 18 Vsci1tx (at FFDA)               Used */
         USB_InterruptRx,              /* Int.no. 17 Vsci1rx (at FFDC)               Used */
         USB_InterruptError,           /* Int.no. 16 Vsci1err (at FFDE)              Used */
         Cpu_Interrupt,                /* Int.no. 15 Vspi1 (at FFE0)                 Unassigned */
         Cpu_Interrupt,                /* Int.no. 14 Vtpm2ovf (at FFE2)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 13 Vtpm2ch4 (at FFE4)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 12 Vtpm2ch3 (at FFE6)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 11 Vtpm2ch2 (at FFE8)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 10 Vtpm2ch1 (at FFEA)              Unassigned */
         Cpu_Interrupt,                /* Int.no.  9 Vtpm2ch0 (at FFEC)              Unassigned */
         Cpu_Interrupt,                /* Int.no.  8 Vtpm1ovf (at FFEE)              Unassigned */
         Cpu_Interrupt,                /* Int.no.  7 Vtpm1ch2 (at FFF0)              Unassigned */
         Cpu_Interrupt,                /* Int.no.  6 Vtpm1ch1 (at FFF2)              Unassigned */
         Cpu_Interrupt,                /* Int.no.  5 Vtpm1ch0 (at FFF4)              Unassigned */
         Cpu_VicgInterrupt,            /* Int.no.  4 Vicg (at FFF6)                  Used */
         LowVoltageDetect,             /* Int.no.  3 Vlvd (at FFF8)                  Used */
         IRQIsr,                       /* Int.no.  2 Virq (at FFFA)                  Used */
         vPortYield,                   /* Int.no.  1 Vswi (at FFFC)                  Used */
         _EntryPoint                   /* Int.no.  0 Vreset (at FFFE)                Reset vector */
};

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
