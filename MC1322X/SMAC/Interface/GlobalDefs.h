/************************************************************************************
* This header file defines common data types
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _GLOBAL_DEFS_
#define _GLOBAL_DEFS_

#include "../../PLM/Interface/EmbeddedTypes.h"

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/
typedef void(*callback_t)(void);
typedef void(*vScanCallback_t)(uint8_t);

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

/*
typedef enum{
  false,
  true
} bool_t;


typedef signed char int8_t;        // signed 8 bit integer
typedef unsigned char uint8_t;     // unsigned 8 bit integer
typedef signed short int16_t;      // signed 16 bit integer
typedef unsigned short uint16_t;   // unsigned 16 bit integer
typedef signed long int32_t;       // signed 32 bit integer
typedef unsigned long uint32_t;    // unsigned 32 bit integer

typedef signed short intn16_t;     // signed integer with at least 16 bits (compiler dependent)
typedef unsigned short uintn16_t;  // unsigned integer with at least 16 bits (compiler dependent)
typedef signed char intn8_t;       // signed integer with at least 8 bits (compiler dependent)
typedef unsigned char uintn8_t;    // unsigned integer with at least 8 bits (compiler dependent)
typedef signed long intn32_t;      // signed integer with at least 32 bits (compiler dependent)
typedef unsigned long uintn32_t;   // unsigned integer with at least 32 bits (compiler dependent)


#define FALSE 0
#define TRUE 1

#ifndef NULL
#define NULL (void *)(0)
#endif*/

#define gNull_c (0)

typedef enum FuncReturn_tag
{
  gSuccess_c = 0,
  gFailOutOfRange_c,
  gFailNoResourcesAvailable_c,
  gFailNoValidCondition_c,
  gFailBusy_c,
  gFailCorrupted,
  gFailTest_c,
  gAuthenticationFailed_c,
  gAuthenticationPassed_c,
  gFail_c
} FuncReturn_t;

typedef enum channel_num_tag
{
  gChannel11_c = 0,
  gChannel12_c,
  gChannel13_c,
  gChannel14_c,
  gChannel15_c,
  gChannel16_c,
  gChannel17_c,
  gChannel18_c,
  gChannel19_c,
  gChannel20_c,
  gChannel21_c,
  gChannel22_c,
  gChannel23_c,
  gChannel24_c,
  gChannel25_c,
  gChannel26_c,
  gTotalChannels_c
} channel_num_t;

#define CHANNEL11   gChannel11_c
#define CHANNEL12   gChannel12_c
#define CHANNEL13   gChannel13_c
#define CHANNEL14   gChannel14_c
#define CHANNEL15   gChannel15_c
#define CHANNEL16   gChannel16_c
#define CHANNEL17   gChannel17_c
#define CHANNEL18   gChannel18_c
#define CHANNEL19   gChannel19_c
#define CHANNEL20   gChannel20_c
#define CHANNEL21   gChannel21_c
#define CHANNEL22   gChannel22_c
#define CHANNEL23   gChannel23_c
#define CHANNEL24   gChannel24_c
#define CHANNEL25   gChannel25_c
#define CHANNEL26   gChannel26_c
#define TOTAL_CHANN gTotalChannels_c

#define MAX_CHAN_NUM    (TOTAL_CHANN-1)

#define	MAX_POWER       0x12
#define NOM_POWER       0x0f
#define MIN_POWER	    0x00
#define MAX_POW_TRIM    0x3F

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




/**********************************************************************************/

#endif /* _GLOBAL_DEFS_ */
