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
// Functions prototypes.

void vRadioTransmitTask( void *pvParameters );
void vRadioReceiveTask( void *pvParameters );

#endif REMOTERADIOTASK_H

