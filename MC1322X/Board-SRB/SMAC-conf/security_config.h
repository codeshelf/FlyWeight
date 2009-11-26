/************************************************************************************
* Security Parameters Configuration
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 06/07/2007
* Last Tested:
************************************************************************************/
 
 
/* This file contain the features to be included within Security Module*/
 
#ifndef __SMAC_SECURITY_CONFIG_H__
#define __SMAC_SECURITY_CONFIG_H__

  #define AES_DEFAULT_MODE AES_CTR_MODE

  #define KEY_0_INIT_VALUE 0x00112233
  #define KEY_1_INIT_VALUE 0x44556677
  #define KEY_2_INIT_VALUE 0x8899AABB
  #define KEY_3_INIT_VALUE 0xCCDDEEFF

  #define CTR_0_INIT_VALUE 0x00112233
  #define CTR_1_INIT_VALUE 0x44556677
  #define CTR_2_INIT_VALUE 0x8899AABB
  #define CTR_3_INIT_VALUE 0xCCDDEEFF

  #define AES_CTR_MODE gCTRMode_c 
  #define AES_CBC_MODE gCBCMode_c
  #define AES_CCM_MODE gCCMMode_c

#endif /* __SMAC_SECURITY_CONFIG_H__ */
