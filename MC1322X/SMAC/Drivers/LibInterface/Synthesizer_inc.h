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
#ifndef SYNTH_H
#define SYNTH_H

#include "../Interface/FreqDepSettings.h"
/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
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

/************************************************************************************
* This function ...
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern void RadioInit(uint32_t u32ClockFreq, uint8_t u8PN, uint32_t u32LoopDiv);


/************************************************************************************
* This function ...
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern void SetChannel(uint8_t channel,  uint8_t RFSynVCODivI, uint32_t RFSynVCODivF);

/************************************************************************************
* This function ...
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern uint8_t GetCurrentChannel(const uint8_t * , const uint32_t *);


/************************************************************************************
* This function ...
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern void RFSynthDisable(void);

/************************************************************************************
* This function ...
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern void RFSynthEnable(void);

/************************************************************************************
* This function ...
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern void InitIdleToRun (void);


#endif // SYNTH_H

