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
#ifndef _TRANSCEIVER_POWER_MNGMNT_
#define _TRANSCEIVER_POWER_MNGMNT_

#include "GlobalDefs.h"
#include "../../PLM/LibInterface/Crm.h"
#include "PWR_Config.h"


/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

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

#define gXtal32khz_c          (32)
#define gRingOsc2khz_c        (2)
#define gEnable_c             (0x01)
#define gDisable_c            (0x00) 
#define ENTER_SLEEP_MODE      (0x01)



#define ROSC_EN          (0x01)
#define XTAL32_EN        (0x01)
#define SYS_CNTL_XTAL32_EXISTS   (0x20) 


typedef enum clock_config_tag
{
  gTimerWakeup_c = 0,
  gRTCWakeup_c,
  gMaxNumClock_c 
} clock_config_t;

#define MaxNumClock  (gMaxNumClock_c-1)

typedef enum range_mode_tag
{
  gRangeModeLow_c =0,
  gRangeModeHigh_c,
  gRangeModeNormal_c
} range_mode_t;

typedef enum wakeup_source_tag
{
  gNoWu_c         = 0x00, // Defining that the wake up source not found
  gTimerWuEn_c    = 0x01, // Defining the wake up from the sleep timer
  gRTCWuEn_c      = 0x02, // Defining the wake up from the RTC
  gExtWuKBI4En_c  = 0x10, // Defining the wake up from the pad KBI 4
  gExtWuKBI5En_c  = 0x20, // Defining the wake up from the pad KBI 5
  gExtWuKBI6En_c  = 0x40, // Defining the wake up from the pad KBI 6
  gExtWuKBI7En_c  = 0x80 // Defining the wake up from the pad KBI 7
} wakeup_source_t;


typedef void (*pToDoBeforeSleep_t)(void);

typedef struct
{
              uint8_t SleepType:1;// 0 hibernate / 1 doze
              uint8_t RamRet:2;
              uint8_t McuRet:1;
              uint8_t DigPadRet:1;
              pToDoBeforeSleep_t  pToDoBeforeSleep;
}SleepCtrl_t;


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
* This function set the timeout value for a specific clock.
*
*
* Routine limitations: This routine can configure only the clocks used for wake u.
*
* Parameters: clock_config_t u8Clock    - ID for the clock to be adjusted.  
*             uint32_t u32Count         - Timeout value.
*
* Return value: gFailOutOfRange_c shall be sent for an incorrect parameter.
*               gSuccess_c when the configuration is correct.
*
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
FuncReturn_t SetWakeupTimerTimeout(clock_config_t u8Clock, uint32_t u32Count);



/************************************************************************************
* This function set the wake up sources for the device.
*
*
* Interface assumptions: If the MCU_RET is enabled, the interruption of each wake up
*                       source will be executed, otherwise the system will restart.
*
* Parameters: wakeup_source_t u8Mode    -Set of wakeup sources.
*
* Return value: gFailOutOfRange_c shall be sent for an incorrect parameter.
*               gSuccess_c in case of a correct configuration.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMESetWakeupSource(uint8_t u8Mode, uint8_t u8KBIPol, uint8_t u8KBIEdge);

/************************************************************************************
* This primitive is used to get all the options configured as wakeup source.
*
* Return value: u8Mode, this value represents the options enabled as Wakeup sources.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
uint8_t GetConfiguredWakeupSource(void);

#if (gUseLowPowerMode_c)
/************************************************************************************
* This function places the device into Hibernate mode.
*
*
* Interface assumptions: The validation of the pBeforeHib parameter is made by the 
* CRM driver.
*
* Parameters: uint8_t u8HibClock      - Clock for the hibernate process.
*             crmSleepCtrl_t SleepCtl - Structure with the sleep mode settings.        
*
* Return value: gFailOutOfRange_c will be sent for an incorrect parameter.
*               gFailNoValidCondition_c when any wakeup source is not enabled.
*       	gSuccess_c for a successful execution.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMEHibernateRequest(uint8_t u8HibClock, crmSleepCtrl_t SleepCtl);


/************************************************************************************
* This function places the device into Doze mode.
*
*
* Interface assumptions: The validation of the pBeforeDoze parameter is made by the 
* CRM driver.
*
* Parameters: crmSleepCtrl_t SleepCtl - Structure with the sleep mode settings.
*
* Return value: gFailOutOfRange_c will be sent for an incorrect parameter.
*               gFailNoValidCondition_c when any wakeup source is not enabled.
*       	gSuccess_c for a successful execution.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMEDozeRequest(crmSleepCtrl_t SleepCtl);

#endif
/**********************************************************************************/

#endif /* _TRANSCEIVER_POWER_MNGMNT_ */
