/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "keyboard.h"
#include "IO_Map.h"

ISR(keyboardISR) {
	KBI1SC_KBACK = 1;
}