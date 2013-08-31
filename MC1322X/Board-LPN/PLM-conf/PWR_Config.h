/************************************************************************************
* Function to handle/set the different power down states.
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
***********************************************************************************/
#ifndef _PWR_CONFIG_H_
#define _PWR_CONFIG_H_

/************************************************************************************
*************************************************************************************
* Module configuration constants
*************************************************************************************
************************************************************************************/
//-----------------------------------------------------------------------------
// To enable/disable all of the code for low power mode in the TransceiverConfigMngmnt
// files.
// TRUE =  1: Use Low Power functions (Normal)
// FALSE = 0: Don't use Low Power. Will cut variables and code out. But
//     functions still exist. Useful for debugging and test purposes
#define gUseLowPowerMode_c        FALSE

// To configure the MCU retention mode used during sleep
//   0:          No MCU retention
//   1:          MCU retention is On during sleep
#define gMCURetentionMode_c       TRUE

// To configure the MCU Pad retention mode used during sleep (if MCU retention configured)
//   0:          No Pad retention
//   1:          MCU Pad retention on id MCU retention was configured also 
#define gMCUPadRetentionMode_c    TRUE


// To configure the RAM retention mode used during sleep
//   gRamRet8k_c:         First 8k of RAM retained during sleep
//   gRamRet32k_c:        First 32k of RAM retained during sleep
//   gRamRet64k_c:        First 64k of RAM retained during sleep
//   gRamRet96k_c:        First 96k (All) of RAM retained during sleep
#define gRAMRetentionMode_c       gRamRet96k_c  


#endif /* _PWR_CONFIGURATION_H_ */

