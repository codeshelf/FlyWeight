/************************************************************************************
* Board Parameters Configuration
*
* (c) Copyright 2008, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 
* Last Tested:
************************************************************************************/
 
 #ifndef __BOARD_CONFIG_H__
 #define __BOARD_CONFIG_H__

#include "../Interface/FreqDepSettings.h"

#define MC1322XSRB    0
#define MC1322XNCB    1
#define MC1322XUSB    2
#define MC1322XLPB     3
#define MC1322XUSERDEF 4
#define GW0011			5

#define TARGET_BOARD  GW0011


#if TARGET_BOARD == MC1322XSRB
  #define DEFAULT_COARSE_TRIM   0x08 
  #define DEFAULT_FINE_TRIM     0x0F
#endif

#if TARGET_BOARD == MC1322XNCB
  #define DEFAULT_COARSE_TRIM   0x08 
  #define DEFAULT_FINE_TRIM     0x0F
#endif

#if TARGET_BOARD == MC1322XUSB
  #define DEFAULT_COARSE_TRIM   0x0A 
  #define DEFAULT_FINE_TRIM     0x0E
#endif

#if TARGET_BOARD == MC1322XLPB
  #define DEFAULT_COARSE_TRIM   0x0B 
  #define DEFAULT_FINE_TRIM     0x12
#endif

#if TARGET_BOARD == MC1322XUSERDEF 
  #define DEFAULT_COARSE_TRIM_USER_DEF   0x18
  #define DEFAULT_FINE_TRIM_USER_DEF     0x0F     
  #define DEFAULT_COARSE_TRIM DEFAULT_COARSE_TRIM_USER_DEF
  #define DEFAULT_FINE_TRIM DEFAULT_FINE_TRIM_USER_DEF
#endif

#if TARGET_BOARD == GW0011
  #define DEFAULT_COARSE_TRIM   0x14
  #define DEFAULT_FINE_TRIM     0x0f
#endif


/*Clock definitions*/
#define PLATFORM_CLOCK (gCrystalFrequency_c)



#endif /* __BOARD_CONFIG_H__ */
