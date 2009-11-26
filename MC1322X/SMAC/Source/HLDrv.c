/************************************************************************************
* High Level Drivers implementation
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 01/25/2007
* Last Tested:
************************************************************************************/

#include "../../PLM/LibInterface/Crm.h"
#include "../Interface/HLDrv.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#define countsWaiting     1000

#define isTimingRefInvalid()        ( gMaxRtcTimingReference_c <= mTimingRef )
#define isClkToConfigInvalid()      ( gMaxClkToConfig_c <= mClkToConfig )
#define isRegisterToConfigInvalid() ( gMaxPowerRegisterToConfig_c <= mRegisterToConfig )

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

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
* DRVConfigureRTC
*
* This primitive lets configure the Real Time Clock (RTC) for call back 
* function after some timeout and the reference clock for the RTC.
*
************************************************************************************/
FuncReturn_t DRVConfigureRTC(crm_rtc_timingReference_t mTimingRef, uint32_t mTimeout, callback_t pfUserCallbackFn){

  if ( isTimingRefInvalid() ) {
    return gFailOutOfRange_c;
  }
  else if ( NULL == pfUserCallbackFn ){
    return gFailOutOfRange_c;
  }

  CRM_RTCSetTimeout(mTimeout);
  
  if ( gTimerRef_2Khz_c == mTimingRef ) {
    CRM_RingOscillatorEnable();
    CRM_Xtal32Disable();
  }
  else if (gTimerRef_32Khz_c == mTimingRef) {
    CRM_2kToXtal32Switch(TRUE, NULL); 
  }
  else {
    /* Do nothing */
  }
  if (CRM_RegisterISR(gCrmRTCWuEvent_c, pfUserCallbackFn) == gCrmErrNoError_c) 
  {
    CRM_RTCInterruptEnable();
    return gSuccess_c;
  }
  else {
     return gFail_c;
  }
}

/************************************************************************************
* DRVSetPowerSorce
*
* This function sets the power power configuration for the MCU. This settings are 
* retainded during sleep modes.
*
************************************************************************************/
FuncReturn_t DRVSetPowerSource(crmPowerSource_t pwSource)
{
  if(gCrmErrNoError_c == CRM_SetPowerSource(pwSource))
  {
    return gSuccess_c;
  }
  else
  {
    return gFail_c;
  }
}

/************************************************************************************
* DRVConfigureVoltageRegTrimm
*
* This function is called to adjust the behavior of the specific device.
*
************************************************************************************/

FuncReturn_t DRVConfigureVolageRegTrimm(crmTrimmedDevice_t trimmedDevice,uint8_t trimmValue)
{
  if(gCrmErrNoError_c == CRM_VRegTrimm(trimmedDevice,trimmValue))
  {
    return gSuccess_c;
  }
  else 
  {
    return gFail_c;
  }
}

/************************************************************************************
* DRVConfigureVoltageControl
*
* This function is called to configure one of the following three voltage regulators:
* Bulk Voltage regulator, 1.5 voltage regulator or 1.8 voltage regulator.
*
************************************************************************************/
FuncReturn_t DRVConfigureVoltageControl(crmVRegCntl_t* pVRegCntl)
{
  if(gCrmErrNoError_c == CRM_VRegCntl(pVRegCntl))
  {
    return gSuccess_c;
  }
  else  
  {
    return gFail_c;
  }
}


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



