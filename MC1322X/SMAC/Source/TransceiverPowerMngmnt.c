/************************************************************************************
* Transceiver Power Management implementation
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 01/11/2007
* Last Tested:
************************************************************************************/

#include "../Interface/TransceiverPowerMngmnt.h"
#include "../Drivers/LibInterface/rif_inc.h"
#include "../../PLM/LibInterface/Platform.h"
#include "../../PLM/LibInterface/CRM_Regs.h"


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define gExtWuKBI_c           (gExtWuKBI4En_c | gExtWuKBI5En_c | gExtWuKBI6En_c | gExtWuKBI7En_c)
#define crm_sleep_cntl_st     ((crm_sleep_cntl_t *)(&crm_sleep_cntl))
#define _set_ram_ret_size(x)  (crm_sleep_cntl_st->Bits.RAM_RET = x)
#define _set_mcu_ret_state(x) (crm_sleep_cntl_st->Bits.MCU_RET = x)


/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
void CrmGoToSleep(SleepCtrl_t* pSleepCtrl);


/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
#define mMaxKBI_c                (0x0F)
#define mMaxWakeupSource_c       (0xF3)
#define mWuKBIShiftSize_c        (0x04)
#define mNoWakeupSource_c        (0x00)
#define mRam_Ret_Size_c          (0x11)
#define mMCU_Ret_Enable_c        (0x01)
#define mMCU_Ret_Disable_c       (0x00)
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* SetWakeupTimerTimeout
*
* This function is used to set the timeout value for a specific clock.
*
************************************************************************************/

FuncReturn_t SetWakeupTimerTimeout(clock_config_t u8Clock, uint32_t u32TimeOut){

  FuncReturn_t retTmp;  

  if(gTimerWakeup_c==u8Clock)
  {
    CRM_REGS_P->WuTimeout = u32TimeOut;
    retTmp = gSuccess_c;
  }
  else
  {
    if(gRTCWakeup_c!=u8Clock)
    {
      retTmp = gFailOutOfRange_c;
    }
    else
    {
      CRM_RTCSetTimeout(u32TimeOut); 
      retTmp = gSuccess_c;
    }
  }
  return retTmp;
}


/************************************************************************************
* MLMESetWakeupSource
*
* This function configures the device auto wake up capability. Three types of wakeups 
* are possible: 1) An external wakeup signal though 4 pads, 2) An internal time wake up 
* and 3)A Real Time Clock timeout.
*
************************************************************************************/
FuncReturn_t MLMESetWakeupSource(uint8_t u8Mode, uint8_t u8KBIPol,uint8_t u8KBIEdge)
{
  FuncReturn_t tmpRteurn;
  crmWuCtrl_t WuCtrl;
  u8Mode &= mMaxWakeupSource_c;

  tmpRteurn = gFailNoValidCondition_c;

  if(gNull_c != (u8Mode & gTimerWuEn_c))
  { 
       WuCtrl.wuSource = gTimerWu_c;
       WuCtrl.TIMER_WU_EN = 1;
       WuCtrl.TIMER_WU_INT_EN = 1;
       CRM_WuCntl(&WuCtrl);
       tmpRteurn = gSuccess_c;
  }
  else
  {
      //Do nothing
  }

  if(gNull_c != (u8Mode & gRTCWuEn_c))
  { 
     WuCtrl.wuSource = gRtcWu_c;
     WuCtrl.RTC_WU_EN = 1;
     WuCtrl.RTC_WU_INT_EN = 1;
     CRM_WuCntl(&WuCtrl); 
     tmpRteurn = gSuccess_c;
  }
  else{
      //Do nothing
  }

  if(gNull_c != (u8Mode & gExtWuKBI_c))
  {
     u8Mode &= gExtWuKBI_c;
     WuCtrl.wuSource = gExtWu_c;
     WuCtrl.KBI_WU_EN = (u8Mode >> 4); 
     WuCtrl.KBI_INT_EN = (u8Mode >> 4);
     WuCtrl.KBI_WU_POL = (u8KBIPol & mMaxKBI_c);
     WuCtrl.KBI_WU_EVENT =(u8KBIEdge & mMaxKBI_c);
     CRM_WuCntl(&WuCtrl); 
     tmpRteurn = gSuccess_c;
  }
  else{
    //Do nothing/
  }

  return tmpRteurn;
}



/************************************************************************************
* MLMEGetConfiguredWakeupSource
*
* Return the wake up sources that were configured through the MLMESetWakeSource 
* fucntion.
*
************************************************************************************/

uint8_t GetConfiguredWakeupSource(void){
  uint8_t u8Mode = 0;
  u8Mode = CRM_REGS_P->WuCntl;
  u8Mode &= mMaxWakeupSource_c; 
  return u8Mode;
}

#if (gUseLowPowerMode_c)
/************************************************************************************
* MLMEHibernateRequest
*
* This function places the device into Hibernate mode and permit to configure a clock
*for hibernate mode (XTAL32kHz or RINGOSC2kHz)
*
************************************************************************************/
FuncReturn_t MLMEHibernateRequest(uint8_t u8HibClock, crmSleepCtrl_t SleepCtl)
{
  if(mNoWakeupSource_c == GetConfiguredWakeupSource){
      return gFailNoValidCondition_c;
  }
  else{
    // Do nothing 
  }
  if(gXtal32khz_c != u8HibClock) {
    if(gRingOsc2khz_c != u8HibClock){
      return gFailOutOfRange_c;
    }
    else{
      // Do nothing 
    }
  }
  else {
    CRM_2kToXtal32Switch(TRUE, NULL); 
  }
  
  SleepCtl.sleepType = gHibernate_c;
  CRM_GoToSleep(&SleepCtl);
  return gSuccess_c;
}

/************************************************************************************
* MLMEDozeRequest
*
* This function places the device into Doze mode and verify that any Wake up 
*Source had been enabled unless.
*
************************************************************************************/
FuncReturn_t MLMEDozeRequest(crmSleepCtrl_t SleepCtl)
{
  if(mNoWakeupSource_c == GetConfiguredWakeupSource()){
    return gFailNoValidCondition_c;
  }
  else{
    SleepCtl.sleepType = gDoze_c;
 
    CRM_GoToSleep(&SleepCtl);
  }
  return gSuccess_c;
}

#endif

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private Debug stuff
*************************************************************************************
************************************************************************************/

