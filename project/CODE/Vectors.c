/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Vectors.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Version   : Bean 01.085, Driver 01.21, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 5/11/2006, 9:38 PM
**     Abstract  :
**         This bean "MC9S08GT60_48" contains initialization of the
**         CPU and provides basic methods and events for CPU core
**         settings.
**     Settings  :
**
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


#include "Cpu.h"
#include "SW2.h"
#include "SW3.h"
#include "SW4.h"
#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "RTI1.h"
#include "SWI.h"
#include "MC13191IRQ.h"
#include "PWM.h"
#include "AudioOut.h"
#include "USB.h"
extern void _EntryPoint(void);

void (* const _vect[])() @0xFFCC = {   /* Interrupt vector table */
         testRTI,                      /* Int.no.  0 Vrti (at FFCC)                  Used */
         Cpu_Interrupt,                /* Int.no.  1 Viic1 (at FFCE)                 Unassigned */
         Cpu_Interrupt,                /* Int.no.  2 Vatd1 (at FFD0)                 Unassigned */
         Cpu_Interrupt,                /* Int.no.  3 Vkeyboard1 (at FFD2)            Unassigned */
         USB_InterruptTx,              /* Int.no.  4 Vsci2tx (at FFD4)               Used */
         USB_InterruptRx,              /* Int.no.  5 Vsci2rx (at FFD6)               Used */
         USB_InterruptError,           /* Int.no.  6 Vsci2err (at FFD8)              Used */
         Cpu_Interrupt,                /* Int.no.  7 Vsci1tx (at FFDA)               Unassigned */
         Cpu_Interrupt,                /* Int.no.  8 Vsci1rx (at FFDC)               Unassigned */
         Cpu_Interrupt,                /* Int.no.  9 Vsci1err (at FFDE)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 10 Vspi1 (at FFE0)                 Unassigned */
         AudioLoader_OnInterrupt,      /* Int.no. 11 Vtpm2ovf (at FFE2)              Used */
         Cpu_Interrupt,                /* Int.no. 12 Reserved12 (at FFE4)            Unassigned */
         Cpu_Interrupt,                /* Int.no. 13 Reserved13 (at FFE6)            Unassigned */
         Cpu_Interrupt,                /* Int.no. 14 Reserved14 (at FFE8)            Unassigned */
         Cpu_Interrupt,                /* Int.no. 15 Vtpm2ch1 (at FFEA)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 16 Vtpm2ch0 (at FFEC)              Unassigned */
         audioOutISR,                  /* Int.no. 17 Vtpm1ovf (at FFEE)              Used */
         Cpu_Interrupt,                /* Int.no. 18 Vtpm1ch2 (at FFF0)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 19 Vtpm1ch1 (at FFF2)              Unassigned */
         Cpu_Interrupt,                /* Int.no. 20 Vtpm1ch0 (at FFF4)              Unassigned */
         Cpu_OnClockMonitorInt,        /* Int.no. 21 Vicg (at FFF6)                  Used */
         Cpu_Interrupt,                /* Int.no. 22 Vlvd (at FFF8)                  Unassigned */
         IRQIsr,                       /* Int.no. 23 Virq (at FFFA)                  Used */
         vPortYield,                   /* Int.no. 24 Vswi (at FFFC)                  Used */
         _EntryPoint                   /* Int.no. 25 Vreset (at FFFE)                Reset vector */
 };
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
