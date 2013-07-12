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

#define		kAssocCheckTickCount	3000 // 3 seconds
#define		kNetCheckTickCount		4 * kAssocCheckTickCount  // four ACK packets missed then reset.

// --------------------------------------------------------------------------
// Functions prototypes.

void radioReceiveTask( void *pvParameters );
void radioTransmitTask( void *pvParameters );

#endif // REMOTERADIOTASK_H

