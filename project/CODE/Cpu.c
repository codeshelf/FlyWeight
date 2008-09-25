/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : Cpu.C
**     Project   : FlyWeight
**     Processor : MC13213R2
**     Beantype  : MC13214
**     Version   : Bean 01.065, Driver 01.31, CPU db: 2.87.125
**     Datasheet : MC1321xRM Rev. 1.1 10/2006
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 9/24/2008, 8:15 PM
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
**     (c) Copyright UNIS, spol. s r.o. 1997-2008
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE Cpu. */

#pragma MESSAGE DISABLE C4002 /* WARNING C4002: Result not used is ignored */

#include "RTI1.h"
#include "SWI.h"
#include "MC13191IRQ.h"
#include "PWM_MC1321X.h"
#include "AudioLoader_MC1321X.h"
#include "MIC_MC1321X.h"
#include "KBI_MC1321X.h"
#include "LowVoltage.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Events.h"
#include "Cpu.h"


/* Global variables */
volatile byte CCR_reg;                 /* Current CCR register */
byte CpuMode = HIGH_SPEED;             /* Current speed mode */


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
ISR(Cpu_VicgInterrupt)
{
  asm("LDA _ICGS1");
  ICGS1_ICGIF = 1;                     /* Clear ICG interrupt flag */
}

/*
** ===================================================================
**     Method      :  Cpu_Interrupt (bean MC13214)
**
**     Description :
**         The method services unhandled interrupt vectors.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
ISR(Cpu_Interrupt)
{
}



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
/*
void Cpu_DisableInt(void)

**      This method is implemented as macro in the header module. **
*/

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
/*
void Cpu_EnableInt(void)

**      This method is implemented as macro in the header module. **
*/

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
#pragma NO_ENTRY
#pragma NO_EXIT
#pragma MESSAGE DISABLE C5703
void Cpu_Delay100US(word us100)
{
  /* Total irremovable overhead: 16 cycles */
  /* ldhx: 5 cycles overhead (load parameter into register) */
  /* jsr:  5 cycles overhead (jump to subroutine) */
  /* rts:  6 cycles overhead (return from subroutine) */

  /* aproximate irremovable overhead for each 100us cycle (counted) : 8 cycles */
  /* aix:  2 cycles overhead  */
  /* cphx: 3 cycles overhead  */
  /* bne:  3 cycles overhead  */
  asm {
loop:
    /* 100 us delay block begin */
    psha                               /* (2 c) backup A */
    lda CpuMode                        /* (4 c) get CpuMode */
    cmp #HIGH_SPEED                    /* (2 c) compare it to HIGH_SPEED */
    bne label0                         /* (3 c) not equal? goto next section */
    /*
     * Delay
     *   - requested                  : 100 us @ 20MHz,
     *   - possible                   : 2000 c, 100000 ns
     *   - without removable overhead : 1976 c, 98800 ns
     */
    pshh                               /* (2 c: 100 ns) backup H */
    pshx                               /* (2 c: 100 ns) backup X */
    ldhx #$00F5                        /* (3 c: 150 ns) number of iterations */
label1:
    aix #-1                            /* (2 c: 100 ns) decrement H:X */
    cphx #0                            /* (3 c: 150 ns) compare it to zero */
    bne label1                         /* (3 c: 150 ns) repeat 245x */
    pulx                               /* (3 c: 150 ns) restore X */
    pulh                               /* (3 c: 150 ns) restore H */
    nop                                /* (1 c: 50 ns) wait for 1 c */
    nop                                /* (1 c: 50 ns) wait for 1 c */
    nop                                /* (1 c: 50 ns) wait for 1 c */
    bra label2                         /* (3 c) finishing delay, goto end */
label0:
    /*
     * Delay
     *   - requested                  : 100 us @ 15.552MHz,
     *   - possible                   : 1555 c, 99987.14 ns, delta -12.86 ns
     *   - without removable overhead : 1534 c, 98636.83 ns
     */
    pshh                               /* (2 c: 128.6 ns) backup H */
    pshx                               /* (2 c: 128.6 ns) backup X */
    ldhx #$00BE                        /* (3 c: 192.9 ns) number of iterations */
label3:
    aix #-1                            /* (2 c: 128.6 ns) decrement H:X */
    cphx #0                            /* (3 c: 192.9 ns) compare it to zero */
    bne label3                         /* (3 c: 192.9 ns) repeat 190x */
    pulx                               /* (3 c: 192.9 ns) restore X */
    pulh                               /* (3 c: 192.9 ns) restore H */
    nop                                /* (1 c: 64.3 ns) wait for 1 c */
label2:                                /* End of delays */
    pula                               /* (2 c) restore A */
    /* 100 us delay block end */
    aix #-1                            /* us100 parameter is passed via H:X registers */
    cphx #0
    bne loop                           /* next loop */
    rts                                /* return from subroutine */
  }
}

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
void Cpu_SetHighSpeed(void)
{
  if (CpuMode != HIGH_SPEED) {         /* Is an actual cpu mode other than high speed mode? */
    EnterCritical();                   /* If yes then save the PS register */
    /* ICGC1: HGO=0,RANGE=1,REFS=0,CLKS1=1,CLKS0=1,OSCSTEN=1,LOCD=0,??=0 */
    ICGC1 = 0x5C;                      /* Initialization of the ICG control register 1 */
    while(ICGS1_CLKST != 0x03) {}      /* Wait until clock domain is switched */
    /* ICGC2: LOLRE=0,MFD2=0,MFD1=1,MFD0=1,LOCRE=0,RFD2=0,RFD1=0,RFD0=0 */
    ICGC2 = 0x30;                      /* Initialization of the ICG control register 2 */
    ICGTRM = *(byte*)0xFFBE;           /* Initialize ICGTRM register from a non volatile memory */
    while(!ICGS1_LOCK);                /* Wait */
    ExitCritical();                    /* Restore the PS register */
    CpuMode = HIGH_SPEED;              /* Set actual cpu mode to high speed */
  }
}
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
void Cpu_SetSlowSpeed(void)
{
  if (CpuMode != SLOW_SPEED) {         /* Is an actual cpu mode other than slow speed mode? */
    EnterCritical();                   /* If yes then save the PS register */
    /* ICGC1: HGO=0,RANGE=0,REFS=0,CLKS1=0,CLKS0=1,OSCSTEN=1,LOCD=0,??=0 */
    ICGC1 = 0x0C;                      /* Initialization of the ICG control register 1 */
    while(ICGS1_CLKST != 0x01) {}      /* Wait until clock domain is switched */
    /* ICGC2: LOLRE=0,MFD2=1,MFD1=0,MFD0=1,LOCRE=0,RFD2=0,RFD1=0,RFD0=0 */
    ICGC2 = 0x50;                      /* Initialization of the ICG control register 2 */
    ICGTRM = *(byte*)0xFFBE;           /* Initialize ICGTRM register from a non volatile memory */
    while(!ICGS1_LOCK);                /* Wait */
    ExitCritical();                    /* Restore the PS register */
    CpuMode = SLOW_SPEED;              /* Set actual cpu mode to slow speed */
  }
}

/*
** ===================================================================
**     Method      :  _EntryPoint (bean MC13214)
**
**     Description :
**         Initializes the whole system like timing and so on. At the end 
**         of this function, the C startup is invoked to initialize stack,
**         memory areas and so on.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
extern void _Startup(void);            /* Forward declaration of external startup function declared in file Start12.c */

/* User declarations */
#include "pub_def.h"
#include "mcu_hw_config.h"
#include "MC13192_hw_config.h"
#include "simple_mac.h"
#include "NV_Data.h"
//#include "update_nvm.h"
//#include "flash.h"


#pragma NO_FRAME
#pragma NO_EXIT
void _EntryPoint(void)
{

  /*** User code before PE initialization ***/
        PTADD = 0xFF;
        PTAD  = 0x00;
        PTBDD = 0xFF;
        PTBSE = 0x03;
        PTBD  = 0x00;
        PTCDD = 0xFF;
        PTCD  = 0x00;
        PTDDD = 0xFF;
        PTDD  = 0x00;
        PTEDD = 0xFF;
        PTED  = 0x00;
        PTGDD = 0xFF;
        PTGD  = 0x00;
  MCUInit();
  MC13192Init();
  MLMESetMC13192ClockRate(2);
  /*** End of user code before PE initialization ***/

  /* ### MC13214 "Cpu" init code ... */
  /*  PE initialization code after reset */
  /* Common initialization of the write once registers */
  /* SOPT: COPE=0,COPT=1,STOPE=1,??=1,??=0,??=0,BKGDPE=1,??=1 */
  setReg8(SOPT, 0x73);                  
  /* SPMSC1: LVDF=0,LVDACK=0,LVDIE=0,LVDRE=1,LVDSE=1,LVDE=1,??=0,??=0 */
  setReg8(SPMSC1, 0x1C);                
  /* SPMSC2: LVWF=0,LVWACK=0,LVDV=0,LVWV=0,PPDF=0,PPDACK=0,PDC=0,PPDC=0 */
  setReg8(SPMSC2, 0x00);                
  /*  System clock initialization */
  /* ICGC1: HGO=0,RANGE=1,REFS=0,CLKS1=1,CLKS0=1,OSCSTEN=1,LOCD=0,??=0 */
  setReg8(ICGC1, 0x5C);                 
  /* ICGC2: LOLRE=0,MFD2=0,MFD1=1,MFD0=1,LOCRE=0,RFD2=0,RFD1=0,RFD0=0 */
  setReg8(ICGC2, 0x30);                 
  ICGTRM = *(unsigned char*)0xFFBE;    /* Initialize ICGTRM register from a non volatile memory */
  while(!ICGS1_LOCK) {                 /* Wait */
  }
  /*** End of PE initialization code after reset ***/

  __asm   jmp _Startup ;               /* Jump to C startup code */

}

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
void PE_low_level_init(void)
{
  /* Common initialization of the CPU registers */
  /* PTDPE: PTDPE6=0,PTDPE5=0 */
  clrReg8Bits(PTDPE, 0x60);             
  /* PTAPE: PTAPE5=1,PTAPE4=1 */
  setReg8Bits(PTAPE, 0x30);             
  /* PTASE: PTASE7=0,PTASE6=0,PTASE5=0,PTASE4=0,PTASE3=0,PTASE2=0,PTASE1=0,PTASE0=0 */
  setReg8(PTASE, 0x00);                 
  /* PTBSE: PTBSE7=0,PTBSE6=0,PTBSE5=0,PTBSE4=0,PTBSE3=0,PTBSE2=0,PTBSE1=0,PTBSE0=0 */
  setReg8(PTBSE, 0x00);                 
  /* PTCSE: PTCSE7=0,PTCSE6=0,PTCSE5=0,PTCSE4=0,PTCSE3=0,PTCSE2=0,PTCSE1=0,PTCSE0=0 */
  setReg8(PTCSE, 0x00);                 
  /* PTDSE: PTDSE7=0,PTDSE6=0,PTDSE5=0,PTDSE4=0,PTDSE2=0,PTDSE0=0 */
  clrReg8Bits(PTDSE, 0xF5);             
  /* PTESE: PTESE7=0,PTESE6=0,PTESE1=0,PTESE0=0 */
  clrReg8Bits(PTESE, 0xC3);             
  /* PTGSE: PTGSE2=0,PTGSE1=0,PTGSE0=0 */
  clrReg8Bits(PTGSE, 0x07);             
  /* ### Shared modules init code ... */
  /* ### Init_RTI "RTI1" init code ... */
  /* ### Call "RTI1_Init();" init method in a user code, i.e. in the main code */
  /* ### Note:   To enable automatic calling of the "RTI1" init code here must be
                 set the property Call Init method to 'yes'
   */
  /* ### Init_TPM "PWM_MC1321X" init code ... */
  PWM_MC1321X_Init();
  /* ### Init_ADC "MIC_MC1321X" init code ... */
  MIC_MC1321X_Init();
  /* ### Init_KBI "KBI_MC1321X" init code ... */
  KBI_MC1321X_Init();
  __EI();                              /* Enable interrupts */
}

/* Initialization of the CPU registers in FLASH */

/* NVPROT: FPOPEN=1,FPDIS=1,FPS2=1,FPS1=1,FPS0=1,??=1,??=1,??=1 */
const unsigned char NVPROT_INIT @0x0000FFBD = 0xFF;

/* NVOPT: KEYEN=0,FNORED=1,??=1,??=1,??=1,??=1,SEC01=1,SEC00=0 */
const unsigned char NVOPT_INIT @0x0000FFBF = 0x7E;

/* END Cpu. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.03 [04.07]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
