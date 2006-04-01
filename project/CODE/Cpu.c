/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Cpu.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFD
**     Beantype  : MC9S08GT60_48
**     Version   : Bean 01.085, Driver 01.21, CPU db: 2.87.074
**     Datasheet : MC9S08GB60/D Rev. 2.3 12/2004
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/31/2006, 5:18 PM
**     Abstract  :
**         This bean "MC9S08GT60_48" contains initialization of the
**         CPU and provides basic methods and events for CPU core
**         settings.
**     Settings  :
**
**     Contents  :
**         EnableInt  - void Cpu_EnableInt(void);
**         DisableInt - void Cpu_DisableInt(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE Cpu. */

#pragma MESSAGE DISABLE C4002 /* WARNING C4002: Result not used is ignored */

#include "SWI.h"
#include "MC13191IRQ.h"
#include "TickTimer.h"
#include "SW2.h"
#include "SW3.h"
#include "SW4.h"
#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "LED4.h"
#include "UART.h"
#include "USB.h"
#include "MC13191.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PE_Timer.h"
#include "Events.h"
#include "Cpu.h"

/* Global variables */
volatile byte CCR_reg;                 /* Current CCR register */

/*Definition of global shadow variables*/
byte Shadow_PTD;

/*
** ===================================================================
**     Method      :  Cpu_OnClockMonitorInt (bean MC9S08GT60_48)
**
**     Description :
**         This event is envoked if OnClockMonitor flag is changed.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
ISR(Cpu_OnClockMonitorInt)
{
  asm("LDA _ICGS1");
  ICGS1_ICGIF = 1;                     /* Clear ICG interrupt flag */
}


/*
** ===================================================================
**     Method      :  Cpu_Interrupt (bean MC9S08GT60_48)
**
**     Description :
**         The method services the interrupt of the selected peripheral(s)
**         and eventually invokes the beans event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
ISR(Cpu_Interrupt)
{
}
/*
** ===================================================================
**     Method      :  Cpu_DisableInt (bean MC9S08GT60_48)
**
**     Description :
**         Disables maskable interrupts
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/*
void Cpu_DisableInt(void)

**      This method is implemented as macro in the header module. **
*/

/*
** ===================================================================
**     Method      :  Cpu_EnableInt (bean MC9S08GT60_48)
**
**     Description :
**         Enables maskable interrupts
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/*
void Cpu_EnableInt(void)

**      This method is implemented as macro in the header module. **
*/

/*
** ===================================================================
**     Method      :  _EntryPoint (bean MC9S08GT60_48)
**
**     Description :
**         Initializes the whole system like timing and so on. At the end 
**         of this function, the C startup is invoked to initialize stack,
**         memory areas and so on.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
extern void _Startup(void);            /* Forward declaration of external startup function declared in file Start12.c */

#pragma NO_FRAME
#pragma NO_EXIT
void _EntryPoint(void)
{
  /* ### MC9S08GT60_48 "Cpu" init code ... */
  /*  PE initialization code after reset */
  /*  System clock initialization */
  /* SOPT: COPE=0,COPT=1,STOPE=0,??=1,??=0,??=0,BKGDPE=1,??=1 */
  setReg8(SOPT, 0x53);                  
  /* SPMSC1: LVDF=0,LVDACK=0,LVDIE=0,LVDRE=1,LVDSE=1,LVDE=1,??=0,??=0 */
  setReg8(SPMSC1, 0x1C);                
  /* SPMSC2: LVWF=0,LVWACK=0,LVDV=0,LVWV=0,PPDF=0,PPDACK=0,PDC=0,PPDC=0 */
  setReg8(SPMSC2, 0x00);                
  /* ICGC1: ??=0,RANGE=1,REFS=0,CLKS1=0,CLKS0=1,OSCSTEN=1,??=0,??=0 */
  setReg8(ICGC1, 0x4C);                 
  /* ICGC2: LOLRE=0,MFD2=1,MFD1=1,MFD0=1,LOCRE=0,RFD2=0,RFD1=0,RFD0=0 */
  setReg8(ICGC2, 0x70);                 
  ICGTRM = *(unsigned char*)0xFFBE;    /* Initialize ICGTRM register from a non volatile memory */
  while(!ICGS1_LOCK) {                 /* Wait */
  }
  /* Common initialization of the write once registers */

  /*** End of PE initialization code after reset ***/

  __asm   jmp _Startup ;               /* Jump to C startup code */

}

/*
** ===================================================================
**     Method      :  PE_low_level_init (bean MC9S08GT60_48)
**
**     Description :
**         Initializes beans and provides common register initialization. 
**         The method is called automatically as a part of the 
**         application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void PE_low_level_init(void)
{
  /* Common initialization of the CPU registers */
  /* PTAPE: PTAPE5=1,PTAPE4=1,PTAPE3=1 */
  setReg8Bits(PTAPE, 0x38);             
  /* PTAD: PTAD6=1,PTAD1=1 */
  setReg8Bits(PTAD, 0x42);              
  /* PTADD: PTADD6=1,PTADD5=0,PTADD4=0,PTADD3=0,PTADD1=1 */
  clrSetReg8Bits(PTADD, 0x38, 0x42);    
  /* PTDD: PTDD4=1,PTDD3=1,PTDD1=1,PTDD0=1 */
  setReg8Bits(PTDD, 0x1B);              
  /* PTDPE: PTDPE4=0,PTDPE3=0,PTDPE1=0,PTDPE0=0 */
  clrReg8Bits(PTDPE, 0x1B);             
  /* PTDDD: PTDDD4=1,PTDDD3=1,PTDDD1=1,PTDDD0=1 */
  setReg8Bits(PTDDD, 0x1B);             
  /* PTEDD: PTEDD5=1,PTEDD4=0,PTEDD3=0,PTEDD1=0,PTEDD0=1 */
  clrSetReg8Bits(PTEDD, 0x1A, 0x21);    
  /* PTED: PTED5=0,PTED0=1 */
  clrSetReg8Bits(PTED, 0x20, 0x01);     
  /* PTCDD: PTCDD1=0,PTCDD0=1 */
  clrSetReg8Bits(PTCDD, 0x02, 0x01);    
  /* PTCD: PTCD0=1 */
  setReg8Bits(PTCD, 0x01);              
  /* PTASE: PTASE7=0,PTASE6=0,PTASE5=0,PTASE4=0,PTASE3=0,PTASE2=0,PTASE1=0,PTASE0=0 */
  setReg8(PTASE, 0x00);                 
  /* PTBSE: PTBSE7=0,PTBSE6=0,PTBSE5=0,PTBSE4=0,PTBSE3=0,PTBSE2=0,PTBSE1=0,PTBSE0=0 */
  setReg8(PTBSE, 0x00);                 
  /* PTCSE: PTCSE7=0,PTCSE6=0,PTCSE5=0,PTCSE4=0,PTCSE3=0,PTCSE2=0,PTCSE1=0,PTCSE0=0 */
  setReg8(PTCSE, 0x00);                 
  /* PTDSE: PTDSE4=0,PTDSE3=0,PTDSE2=0,PTDSE1=0,PTDSE0=0 */
  clrReg8Bits(PTDSE, 0x1F);             
  /* PTESE: PTESE5=0,PTESE4=0,PTESE3=0,PTESE2=0,PTESE1=0,PTESE0=0 */
  clrReg8Bits(PTESE, 0x3F);             
  /* PTGSE: PTGSE3=0,PTGSE2=0,PTGSE1=0,PTGSE0=0 */
  clrReg8Bits(PTGSE, 0x0F);             
  /* ### MC9S08GT60_48 "Cpu" init code ... */
  /* ### TimerInt "TickTimer" init code ... */
  TickTimer_Init();
  /* ### BitIO "SW2" init code ... */
  /* ### BitIO "SW3" init code ... */
  /* ### BitIO "SW4" init code ... */
  /* ### BitIO "LED1" init code ... */
  Shadow_PTD |= 0x01;                  /* Initialize pin shadow variable bit */
  /* ### BitIO "LED2" init code ... */
  Shadow_PTD |= 0x02;                  /* Initialize pin shadow variable bit */
  /* ### BitIO "LED3" init code ... */
  Shadow_PTD |= 0x08;                  /* Initialize pin shadow variable bit */
  /* ### BitIO "LED4" init code ... */
  /* ### Asynchro serial "UART" init code ... */
  UART_Init();
  /* ### Asynchro serial "USB" init code ... */
  USB_Init();
  /* ###  Synchro master "MC13191" init code ... */
  MC13191_Init();
  __EI();                              /* Enable interrupts */
}

/* Initialization of the CPU registers in FLASH */

/* NVPROT: FPOPEN=1,FPDIS=1,FPS2=1,FPS1=1,FPS0=1,??=1,??=1,??=1 */
const volatile NVPROTSTR _NVPROT @0x0000FFBD = { 0xFF };

/* NVOPT: KEYEN=0,FNORED=1,??=1,??=1,??=1,??=1,SEC01=1,SEC00=0 */
const volatile NVOPTSTR _NVOPT @0x0000FFBF = { 0x7E };

/* END Cpu. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
