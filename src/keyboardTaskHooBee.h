/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef KEYBOARD_TASK_H
#define KEYBOARD_TASK_H

#include "pub_def.h"
#include "FreeRTOS.h"
#include "queue.h"

#define KEBOARD_QUEUE_SIZE		2

#define BUTTON_PRESSED			1
#define BUTTON_RELEASED			2

#define NO_BUTTON				0
#define HOOBEE_ACK_BUTTON		1

extern xQueueHandle	gKeyboardQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void keyboardTask( void *pvParameters );

#endif //KEYBOARD_TASK_H
