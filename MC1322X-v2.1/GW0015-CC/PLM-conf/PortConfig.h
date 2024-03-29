/************************************************************************************
* This is a template header file.
*
* Author(s): B02753
*
* (c) Copyright 2005, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _PORT_CONFIG_H_
#define _PORT_CONFIG_H_

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

/* BEGIN GPIO Register Values */
  #define gDirLoValue_c               0x03C00000     //MBAR_GPIO + 0x00
  #define gDirHiValue_c               0x00000000     //MBAR_GPIO + 0x04
  #define gDataLoValue_c              0x03C00000     //MBAR_GPIO + 0x08
  #define gDataHiValue_c              0x00000000     //MBAR_GPIO + 0x0C
  #define gPuEnLoValue_c              0xFFFFFFFF     //MBAR_GPIO + 0x10
  #define gPuEnHiValue_c              0xFFFFFFFF     //MBAR_GPIO + 0x14
  #define gFuncSel0Value_c            0x00000000     //MBAR_GPIO + 0x18
  #define gFuncSel1Value_c            0x00000000     //MBAR_GPIO + 0x1C
  #define gFuncSel2Value_c            0x05400000     //MBAR_GPIO + 0x20
  #define gFuncSel3Value_c            0x00000000     //MBAR_GPIO + 0x24
  #define gInputDataSelLoValue_c      0x00000000     //MBAR_GPIO + 0x28
  #define gInputDataSelHiValue_c      0x00000000     //MBAR_GPIO + 0x2C
  #define gPuSelLoValue_c             0x00003000     //MBAR_GPIO + 0x30
  #define gPuSelHiValue_c             0x8001C000     //MBAR_GPIO + 0x34
  #define gHystEnLoValue_c            0x00000000     //MBAR_GPIO + 0x38
  #define gHystEnHiValue_c            0x00000000     //MBAR_GPIO + 0x3C
  #define gPuKeepLoValue_c            0xC0000000     //MBAR_GPIO + 0x40
  #define gPuKeepHiValue_c            0x000000DF     //MBAR_GPIO + 0x44
/* END GPIO Register Values */
  
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
#endif /* _PORT_CONFIG_H_ */

