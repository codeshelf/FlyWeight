/************************************************************************************
* RF Settings Configuration
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 06/08/2007
* Last Tested:
************************************************************************************/
#ifndef _RF_CONFIG_H_
#define _RF_CONFIG_H_
 



#define gPowerLevel_m30dBm_c   0x00
#define gPowerLevel_m28dBm_c   0x01
#define gPowerLevel_m27dBm_c   0x02
#define gPowerLevel_m26dBm_c   0x03
#define gPowerLevel_m24dBm_c   0x04
#define gPowerLevel_m21dBm_c   0x05
#define gPowerLevel_m19dBm_c   0x06
#define gPowerLevel_m17dBm_c   0x07 
#define gPowerLevel_m16dBm_c   0x08
#define gPowerLevel_m15dBm_c   0x09 
#define gPowerLevel_m11dBm_c   0x0a
#define gPowerLevel_m10dBm_c   0x0b
#define gPowerLevel_m4d5dBm_c  0x0c
#define gPowerLevel_m3dBm_c    0x0d
#define gPowerLevel_m1d5dBm_c  0x0e
#define gPowerLevel_m1dBm_c    0x0f
#define gPowerLevel_1d7dBm_c   0x10
#define gPowerLevel_3dBm_c     0x11
#define gPowerLevel_4d5dBm_c   0x12
 

#define gCcaThreshold_c                    0x39
/* BEGIN Dual RF Settings */
#define gPowerLevelLock_c                FALSE
#define gEnableComplementaryPAOutput_c   FALSE
#define gDualPortRFOperation_c           FALSE
/* END Dual RF Settings */

#define gDefaultPowerLevel_c               gPowerLevel_m1dBm_c

#endif /*_RF_CONFIG_H_*/


