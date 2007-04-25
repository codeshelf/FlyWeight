/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : IO_Map.C
**     Project   : FlyWeight
**     Processor : MC9S08GT60CFB
**     Beantype  : IO_Map
**     Version   : Driver 01.06
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 4/24/2007, 12:46 PM
**     Abstract  :
**         IO_Map.h - implements an IO device's mapping. 
**         This module contains symbol definitions of all peripheral 
**         registers and bits. 
**     Settings  :
**
**     Contents  :
**         No public methods
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/
/* Based on CPU DB MC9S08GT60_44, version 2.87.086 (RegistersPrg V1.118) */
/* DataSheet : MC9S08GB60/D Rev. 2.3 12/2004 */

#include "PE_types.h"
#include "IO_Map.h"

/*lint -save -esym(765, *) */


/* * * * *  8-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile PTADSTR _PTAD;                                    /* Port A Data Register; 0x00000000 */
volatile PTAPESTR _PTAPE;                                  /* Pullup Enable for Port A; 0x00000001 */
volatile PTASESTR _PTASE;                                  /* Slew Rate Control Enable for Port A; 0x00000002 */
volatile PTADDSTR _PTADD;                                  /* Data Direction Register A; 0x00000003 */
volatile PTBDSTR _PTBD;                                    /* Port B Data Register; 0x00000004 */
volatile PTBPESTR _PTBPE;                                  /* Pullup Enable for Port B; 0x00000005 */
volatile PTBSESTR _PTBSE;                                  /* Slew Rate Control Enable for Port B; 0x00000006 */
volatile PTBDDSTR _PTBDD;                                  /* Data Direction Register B; 0x00000007 */
volatile PTCDSTR _PTCD;                                    /* Port C Data Register; 0x00000008 */
volatile PTCPESTR _PTCPE;                                  /* Pullup Enable for Port C; 0x00000009 */
volatile PTCSESTR _PTCSE;                                  /* Slew Rate Control Enable for Port C; 0x0000000A */
volatile PTCDDSTR _PTCDD;                                  /* Data Direction Register C; 0x0000000B */
volatile PTDDSTR _PTDD;                                    /* Port D Data Register; 0x0000000C */
volatile PTDPESTR _PTDPE;                                  /* Pullup Enable for Port D; 0x0000000D */
volatile PTDSESTR _PTDSE;                                  /* Slew Rate Control Enable for Port D; 0x0000000E */
volatile PTDDDSTR _PTDDD;                                  /* Data Direction Register D; 0x0000000F */
volatile PTEDSTR _PTED;                                    /* Port E Data Register; 0x00000010 */
volatile PTEPESTR _PTEPE;                                  /* Pullup Enable for Port E; 0x00000011 */
volatile PTESESTR _PTESE;                                  /* Slew Rate Control Enable for Port E; 0x00000012 */
volatile PTEDDSTR _PTEDD;                                  /* Data Direction Register E; 0x00000013 */
volatile IRQSCSTR _IRQSC;                                  /* Interrupt Request Status and Control Register; 0x00000014 */
volatile KBI1SCSTR _KBI1SC;                                /* KBI1 Status and Control; 0x00000016 */
volatile KBI1PESTR _KBI1PE;                                /* KBI1 Pin Enable Register; 0x00000017 */
volatile SCI1C1STR _SCI1C1;                                /* SCI1 Control Register 1; 0x0000001A */
volatile SCI1C2STR _SCI1C2;                                /* SCI1 Control Register 2; 0x0000001B */
volatile SCI1S1STR _SCI1S1;                                /* SCI1 Status Register 1; 0x0000001C */
volatile SCI1S2STR _SCI1S2;                                /* SCI1 Status Register 2; 0x0000001D */
volatile SCI1C3STR _SCI1C3;                                /* SCI1 Control Register 3; 0x0000001E */
volatile SCI1DSTR _SCI1D;                                  /* SCI1 Data Register; 0x0000001F */
volatile SCI2C1STR _SCI2C1;                                /* SCI1 Control Register 1; 0x00000022 */
volatile SCI2C2STR _SCI2C2;                                /* SCI2 Control Register 2; 0x00000023 */
volatile SCI2S1STR _SCI2S1;                                /* SCI2 Status Register 1; 0x00000024 */
volatile SCI2S2STR _SCI2S2;                                /* SCI2 Status Register 2; 0x00000025 */
volatile SCI2C3STR _SCI2C3;                                /* SCI2 Control Register 3; 0x00000026 */
volatile SCI2DSTR _SCI2D;                                  /* SCI2 Data Register; 0x00000027 */
volatile SPI1C1STR _SPI1C1;                                /* SPI1 Control Register 1; 0x00000028 */
volatile SPI1C2STR _SPI1C2;                                /* SPI1 Control Register 2; 0x00000029 */
volatile SPI1BRSTR _SPI1BR;                                /* SPI1 Baud Rate Register; 0x0000002A */
volatile SPI1SSTR _SPI1S;                                  /* SPI1 Status Register; 0x0000002B */
volatile SPI1DSTR _SPI1D;                                  /* SPI1 Data Register; 0x0000002D */
volatile TPM1SCSTR _TPM1SC;                                /* TPM 1 Status and Control Register; 0x00000030 */
volatile TPM1C0SCSTR _TPM1C0SC;                            /* TPM 1 Timer Channel 0 Status and Control Register; 0x00000035 */
volatile TPM1C1SCSTR _TPM1C1SC;                            /* TPM 1 Timer Channel 1 Status and Control Register; 0x00000038 */
volatile PTGDSTR _PTGD;                                    /* Port G Data Register; 0x00000044 */
volatile PTGPESTR _PTGPE;                                  /* Pullup Enable for Port G; 0x00000045 */
volatile PTGSESTR _PTGSE;                                  /* Slew Rate Control Enable for Port G; 0x00000046 */
volatile PTGDDSTR _PTGDD;                                  /* Data Direction Register G; 0x00000047 */
volatile ICGC1STR _ICGC1;                                  /* ICG Control Register 1; 0x00000048 */
volatile ICGC2STR _ICGC2;                                  /* ICG Control Register 2; 0x00000049 */
volatile ICGS1STR _ICGS1;                                  /* ICG Status Register 1; 0x0000004A */
volatile ICGS2STR _ICGS2;                                  /* ICG Status Register 2; 0x0000004B */
volatile ICGTRMSTR _ICGTRM;                                /* ICG Trim Register; 0x0000004E */
volatile ATD1CSTR _ATD1C;                                  /* ATD1 Control Register; 0x00000050 */
volatile ATD1SCSTR _ATD1SC;                                /* ATD1 Status and Control Register; 0x00000051 */
volatile ATD1PESTR _ATD1PE;                                /* ATD1 Pin Enable; 0x00000054 */
volatile IIC1ASTR _IIC1A;                                  /* IIC1 Address Register; 0x00000058 */
volatile IIC1FSTR _IIC1F;                                  /* IIC1 Frequency Divider Register; 0x00000059 */
volatile IIC1CSTR _IIC1C;                                  /* IIC1 Control Register; 0x0000005A */
volatile IIC1SSTR _IIC1S;                                  /* IIC1 Status Register; 0x0000005B */
volatile IIC1DSTR _IIC1D;                                  /* IIC1 Data I/O Register; 0x0000005C */
volatile TPM2SCSTR _TPM2SC;                                /* TPM 2 Status and Control Register; 0x00000060 */
volatile TPM2C0SCSTR _TPM2C0SC;                            /* TPM 2 Timer Channel 0 Status and Control Register; 0x00000065 */
volatile TPM2C1SCSTR _TPM2C1SC;                            /* TPM 2 Timer Channel 1 Status and Control Register; 0x00000068 */
volatile SRSSTR _SRS;                                      /* System Reset Status; 0x00001800 */
volatile SBDFRSTR _SBDFR;                                  /* System Background Debug Force Reset Register; 0x00001801 */
volatile SOPTSTR _SOPT;                                    /* System Options Register; 0x00001802 */
volatile SRTISCSTR _SRTISC;                                /* System RTI Status and Control Register; 0x00001808 */
volatile SPMSC1STR _SPMSC1;                                /* PM Status and Control 1 Register; 0x00001809 */
volatile SPMSC2STR _SPMSC2;                                /* PM Status and Control 2 Register; 0x0000180A */
volatile DBGCAHSTR _DBGCAH;                                /* Debug Comparator A High Register; 0x00001810 */
volatile DBGCALSTR _DBGCAL;                                /* Debug Comparator A Low Register; 0x00001811 */
volatile DBGCBHSTR _DBGCBH;                                /* Debug Comparator B High Register; 0x00001812 */
volatile DBGCSTR _DBGC;                                    /* Debug Control Register; 0x00001816 */
volatile DBGTSTR _DBGT;                                    /* Debug Trigger Register; 0x00001817 */
volatile DBGSSTR _DBGS;                                    /* Debug Status Register; 0x00001818 */
volatile FCDIVSTR _FCDIV;                                  /* FLASH Clock Divider Register; 0x00001820 */
volatile FOPTSTR _FOPT;                                    /* FLASH Options Register; 0x00001821 */
volatile FCNFGSTR _FCNFG;                                  /* FLASH Configuration Register; 0x00001823 */
volatile FPROTSTR _FPROT;                                  /* FLASH Protection Register; 0x00001824 */
volatile FSTATSTR _FSTAT;                                  /* FLASH Status Register; 0x00001825 */
volatile FCMDSTR _FCMD;                                    /* FLASH Command Register; 0x00001826 */
/* const volatile NVBACKKEY0STR _NVBACKKEY0;               The register can be initialized in the CPU module. Backdoor Comparison Key 0; 0x0000FFB0 */
/* const volatile NVBACKKEY1STR _NVBACKKEY1;               The register can be initialized in the CPU module. Backdoor Comparison Key 1; 0x0000FFB1 */
/* const volatile NVBACKKEY2STR _NVBACKKEY2;               The register can be initialized in the CPU module. Backdoor Comparison Key 2; 0x0000FFB2 */
/* const volatile NVBACKKEY3STR _NVBACKKEY3;               The register can be initialized in the CPU module. Backdoor Comparison Key 3; 0x0000FFB3 */
/* const volatile NVBACKKEY4STR _NVBACKKEY4;               The register can be initialized in the CPU module. Backdoor Comparison Key 4; 0x0000FFB4 */
/* const volatile NVBACKKEY5STR _NVBACKKEY5;               The register can be initialized in the CPU module. Backdoor Comparison Key 5; 0x0000FFB5 */
/* const volatile NVBACKKEY6STR _NVBACKKEY6;               The register can be initialized in the CPU module. Backdoor Comparison Key 6; 0x0000FFB6 */
/* const volatile NVBACKKEY7STR _NVBACKKEY7;               The register can be initialized in the CPU module. Backdoor Comparison Key 7; 0x0000FFB7 */
/* const volatile NVPROTSTR _NVPROT;                       The register can be initialized in the CPU module. Nonvolatile FLASH Protection Register; 0x0000FFBD */
/* const volatile NVOPTSTR _NVOPT;                         The register can be initialized in the CPU module. Nonvolatile FLASH Options Register; 0x0000FFBF */


/* * * * *  16-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile SCI1BDSTR _SCI1BD;                                /* SCI1 Baud Rate Register; 0x00000018 */
volatile SCI2BDSTR _SCI2BD;                                /* SCI2 Baud Rate Register; 0x00000020 */
volatile TPM1CNTSTR _TPM1CNT;                              /* TPM 1 Counter Register; 0x00000031 */
volatile TPM1MODSTR _TPM1MOD;                              /* TPM 1 Timer Counter Modulo Register; 0x00000033 */
volatile TPM1C0VSTR _TPM1C0V;                              /* TPM 1 Timer Channel 0 Value Register; 0x00000036 */
volatile TPM1C1VSTR _TPM1C1V;                              /* TPM 1 Timer Channel 1 Value Register; 0x00000039 */
volatile ICGFLTSTR _ICGFLT;                                /* ICG Filter Register; 0x0000004C */
volatile ATD1RSTR _ATD1R;                                  /* ATD1 Result Data; 0x00000052 */
volatile TPM2CNTSTR _TPM2CNT;                              /* TPM 2 Counter Register; 0x00000061 */
volatile TPM2MODSTR _TPM2MOD;                              /* TPM 2 Timer Counter Modulo Register; 0x00000063 */
volatile TPM2C0VSTR _TPM2C0V;                              /* TPM 2 Timer Channel 0 Value Register; 0x00000066 */
volatile TPM2C1VSTR _TPM2C1V;                              /* TPM 2 Timer Channel 1 Value Register; 0x00000069 */
volatile SDIDSTR _SDID;                                    /* System Integration Module Part ID Register; 0x00001806 */
volatile DBGFSTR _DBGF;                                    /* Debug FIFO Register; 0x00001814 */

/*lint -restore */

/* EOF */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
