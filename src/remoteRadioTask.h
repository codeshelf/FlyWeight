/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved
   
   $Id$
   $Name$	
*/

#ifndef REMOTERADIOTASK_H
#define REMOTERADIOTASK_H

// Project includes
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Externs

// --------------------------------------------------------------------------
// Defines

#define				kAssocCheckTickCount	3000 //2.5 * portTICK_RATE_MS * 1000;
#define				kNetCheckTickCount		15000  // 2 * kAssocCheckTickCount
#define				RESET_MCU				__asm DCB 0x8D

// --------------------------------------------------------------------------
// Functions prototypes.

void radioReceiveTask( void *pvParameters );
void radioTransmitTask( void *pvParameters );

#endif REMOTERADIOTASK_H

