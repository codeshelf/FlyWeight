/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved
   
   $Id$
   $Name$	
*/

#include "commands.h"
#include "radioCommon.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"

// --------------------------------------------------------------------------
// Local function prototypes

void createCommandHeader(void);

// --------------------------------------------------------------------------

void createCommandHeader() {

};

// --------------------------------------------------------------------------

CommandPtrType createWakeCommand() {
	strcpy(gTXBuffer[gTXCurBufferNum].bufferStorage, "xxx");
};