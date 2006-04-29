/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved
   
   $Id$
   $Name$	
*/

#include "radioCommon.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// --------------------------------------------------------------------------

bool transmitCommand(CommandPtrType inCommandP) {
	return FALSE;
};

// --------------------------------------------------------------------------

CommandPtrType receiveCommand() {
	return NULL;
};