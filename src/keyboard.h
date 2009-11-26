/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef MC1321X
	#include "gwTypes.h"
	#include "pub_def.h"
#else
#endif

// --------------------------------------------------------------------------
// Defines
#define NO_BUTTON			0
#define PTT_BUTTON			1
#define UP_BUTTON			2
#define DOWN_BUTTON			3
#define AUX_BUTTON			4

/*
	 * The keyboard is a matrix:
	 * XBEE:
	 * -----------------
	 * row1 = PTB0
	 * row2 = PTB1
	 * col1 = PTA5/KBI1P5
	 * col2 = PTA6/KBI1P6
	 *
	 * MC1321X:
	 * -----------------
	 * row1 = PTB0
	 * row2 = PTB1
	 * col1 = PTA4/KBI1P4
	 * col2 = PTA5/KBI1P5
	 *
	 * The keyboard module does nothing until the user presses a key.  That causes one of the PTB outputs
	 * to drive a KBI pin that causes an interrupt.  In that interrupt routine we make the PTB pins
	 * output and the PTA pins input.  Then we iterate through the output pins looking for an input pin
	 * that has the asserted value.  Knowing the row (output) and col (input) tells us the key that
	 * the user pressed.
	 *
	 * In our application we want to know when the user let go of the key, so we run the process again
	 * except that the KBI interrupts are looking for falling edges or low values.
	 */

	#define KB_SETUP_ROW0		PTBDD_PTBDD0
	#define KB_SETUP_ROW1		PTBDD_PTBDD1
	#define KB_ROW0				PTBD_PTBD0
	#define KB_ROW1				PTBD_PTBD1
#if defined(XBEE_PINOUT)
    #define KB_SETUP_COL0       PTADD_PTADD5
    #define KB_SETUP_COL1       PTADD_PTADD6
    #define KB_PEBITS           0b01100000
	#define KB_BIT_COL_OFFSET	5
#elif defined(MC1321X)
    #define KB_SETUP_COL0       PTADD_PTADD4
    #define KB_SETUP_COL1       PTADD_PTADD5
    #define KB_PEBITS           0b00110000
	#define KB_BIT_COL_OFFSET	4
#endif

// --------------------------------------------------------------------------
// Local functions prototypes.
void KBISetup(void);
void restartKeyboardISR(void);
gwBoolean buttonStillPressed(void);

#endif // KEYBOARD_H
