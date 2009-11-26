/************************************************************************************
* Init platform
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _PLATFORMINIT_H_
#define _PLATFORMINIT_H_


/*
  Activates the NVM regulator via CRM
*/
void StartNvmReg(void);

/*
  Stops the NVM regulator via CRM
*/
void StopNvmReg(void);


/************************************************************************************
* Perform the complete GPIO port initialization
*
* Interface assumptions:
*
* Return value:
*   NONE
*
* Revision history:
*
*    Date    Author    Comments
*   ------   ------    --------
*   190505   JT        Created
*
************************************************************************************/
void PlatformPortInit(void);


#endif /* _PLATFORMINIT_H_  */ 
