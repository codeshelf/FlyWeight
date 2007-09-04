/************************************************************************************
* This module contains the interface functions used when updating NVM data.
* 
*
* Author(s):  Michael V. Christensen
*
* (c) Copyright 2004, Freescale Semiconductor, Inc. All rights reserved.
*
* Freescale Confidential Proprietary
* Digianswer Confidential
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 29-03-01
* Last Tested:
*
************************************************************************************/

#ifndef _UPDATE_NVM_H_
#define _UPDATE_NVM_H_

// **************************************************************************
#include "DigiType.h"

// **************************************************************************
// Prototypes
bool_t Update_NV_RAM(uint8_t *NV_RAM_Distination_ptr, uint8_t *Source_ptr, uint16_t Source_Length);

// Must be called at startup to setup flash functionality
void NV_Flash_Setup(void);

// **************************************************************************

#endif _UPDATE_NVM_H_
