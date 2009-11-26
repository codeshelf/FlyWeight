/************************************************************************************
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _ROM_SMAC_H
#define _ROM_SMAC_H

#include "../../../PLM/Interface/EmbeddedTypes.h"

typedef struct{
  int32_t calFactor;
  int32_t lnaMinGain;
  int32_t lnaMaxGain;
  int32_t ifaMaxGain;
} LQIParams_t;

/*extern uint32_t* paRFSynVCODivI;
extern uint32_t* paRFSynVCODivF;
extern uint32_t* paRFSynDigTest;
extern uint32_t* paRFSynCoarseTuneTreshold;*/

/*extern uint32_t* paRadioInit;
extern uint32_t* paMacaInit;
extern uint16_t  gRadioInitSize;
extern uint16_t  gMacaInitSize;*/


//void    InitializePhy(void);
//void    PhyPlmeSetCurrentChannelRequest(uint8_t channel);
//void    PhyPlmeSetPanId(uint32_t src);
//void    PhyPlmeSetShortAddress(uint32_t src);
//void    PhyPlmeSetLongAddress(uint8_t address[]);
extern uint8_t PhyPlmeGetLQI();

/**************************************************************************************/
/* This will be part of the LLC                                                       */
/**************************************************************************************/
//uint32_t  InitFromFlash();
//uint32_t  InitFromMemory(uint32_t* pInitArray, uint32_t nLength);

/**************************************************************************************/
/* This will be part of the ROM Utils section                                         */
/**************************************************************************************/
//uint32_t  GetRandomNumber(void);
//uint8_t   GetRandomRange(uint8_t low, uint8_t high);

#endif //_ROM_SMAC_H
