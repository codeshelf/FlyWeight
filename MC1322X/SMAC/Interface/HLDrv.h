/************************************************************************************
* This header file 
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _MC1322X_HL_DRV_
#define _MC1322X_HL_DRV_


#include "GlobalDefs.h"
#include "../../PLM/LIBInterface/Crm.h"



/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/
#define CRM_Xtal32Disable()             CRM_XTAL32_CNTL.xtal32En = 0; 


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
/* Enumeration type used in the fucntion DRVConfigureRTC */
typedef enum crm_rtc_timingReference_tag
{
  gTimerRef_2Khz_c =  0, // Defining the 2Khz as timing reference for RTC
  gTimerRef_32Khz_c,     // Defining the 32Khz as timing reference for RTC
  gMaxRtcTimingReference_c
} crm_rtc_timingReference_t;

/* Enumeration type used in the fucntion DRVClockManagement */
typedef enum crm_clkToConfig_tag
{
  gXtal_Clk_c =  0, // Specify the XTAL clock
  gXtal32_Clk_c,    // Specify the 32 Khz XTAL clock
  gRingOsc_Clk_c,   // Specify the Ring Oscilator
  gMaxClkToConfig_c
} crm_clkToConfig_t;

/* Enumeration type used in the fucntion DRVPowerManagement */
typedef enum crm_powerRegisterToConfig_tag
{
  gSYS_CNTL_c =  0, // Specify the system control register
  gVREG_CNTL_c,    // Specify the voltage regulator control register
  gVREG_TRIM_c,   // Specify the voltage trim control register
  gMaxPowerRegisterToConfig_c
} crm_powerRegisterToConfig_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* This primitive lets configure the Real Time Clock (RTC) for call back 
* function after some timeout and the reference clock for the RTC.  
*
* Inputs: fmu
*   mTimingRef: Enumeration type RTC_TimingReference_t
*   cTimeout: Counts that will wait to call the pvUserCallbackFn.
*   pfUserCallbackFn: Function to be called on timeout.
*
* Interface assumptions:
*   None.
*
* Return value:
*   The value returned is one of the enumeration type FuncReturn_t
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t DRVConfigureRTC(crm_rtc_timingReference_t mTimingRef, uint32_t mTimeout, callback_t pfUserCallbackFn);

/************************************************************************************
* This function allows the user to configure the registers related with the 
* power management. System control (SYS_CNTL), voltage regulator control 
* (VREG_CNTL), voltage regulator trim (VREG_TRIM) and the software reset 
* register (SW_RST) in a friendly manner. The configurable characteristics of 
* power are: the voltage regulators (Buck, 1.8V, 1.5V, 0.9V), the output 
* voltage for the I/O pins, the voltage interface for the non-volatile memory 
* and the software reset.
*
* Inputs: 
*   mClkToConfig: Enumeration type RegisterToConfig_t
*   psClkCfg: Pointer to the power control structure register with the data to 
*       configure.
*
* Interface assumptions:
*   None.
*
* Return value:
*   The value returned is one of the enumeration type FuncReturn_t
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
//FuncReturn_t DRVPowerManagement(crm_powerRegisterToConfig_t mRegisterToConfig, uint32_t * pPowerCfg);


FuncReturn_t DRVConfigureVolageRegTrimm(crmTrimmedDevice_t trimmedDevice,uint8_t trimmValue);

/************************************************************************************
* This function allows the user to configure .
*
* Inputs: 
*   mClkToConfig: Enumeration type RegisterToConfig_t
*   psClkCfg: Pointer to the power control structure register with the data to 
*       configure.
*
* Interface assumptions:
*   None.
*
* Return value:
*   The value returned is one of the enumeration type FuncReturn_t
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t DRVSetPowerSource(crmPowerSource_t pwSource);


/**********************************************************************************/

#endif /* _MC1322X_HL_DRV_ */
