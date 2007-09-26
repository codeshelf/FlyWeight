/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "PE_types.h"
#include "pub_def.h"

// --------------------------------------------------------------------------
// Defines
#define NO_BUTTON			0
#define PTT_BUTTON			1
#define UP_BUTTON			2
#define DOWN_BUTTON			3
#define AUX_BUTTON			4

// --------------------------------------------------------------------------
// Local functions prototypes.
void KBISetup(void);
void restartKeyboardISR(void);
bool buttonStillPressed(UINT8 inButtonNum);

#endif // KEYBOARD_H
