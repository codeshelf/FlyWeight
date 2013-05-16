/************************************************************************************
* Application Parameters Configuration 
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
 
 
/* This file contain the features to be included within SMAC platform*/
 
#ifndef __SMAC_APP_CONFIG_H__
#define __SMAC_APP_CONFIG_H__
 
  #define MANUAL              0
  #define SERIAL              1
  #define CHANNEL_NUMBER      CHANNEL11
  #define INTERFACE_TYPE      MANUAL
  #define NUM_TRANSMISSIONS   999
  #define OTAP_ENABLED        FALSE
  #define OTAP_REQUEST_CODE   0x03020100
  #define OTAP_DEVICE_ADDRESS 0x0001

#endif /* __SMAC_APP_CONFIG_H__ */

