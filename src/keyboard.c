/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "keyboard.h"
#include "IO_Map.h"
#include "PE_types.h"
#include "pub_def.h"

	/*
	 * The keyboard is a matrix:
	 * row1 = PTB0
	 * row2 = PTB1
	 * col1 = PTA5/KBI1P5
	 * col2 = PTA6/KBI1P6
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
	 

void KBISetup() {

	// KBI1SC: KBIE=0
	clrReg8Bits(KBI1SC, 0x02);            
	// KBI1PE: KBIPE7=0,KBIPE6=1,KBIPE5=1,KBIPE4=0,KBIPE3=0,KBIPE2=0,KBIPE1=0,KBIPE0=0
	setReg8(KBI1PE, 0x60);                
	
	// KBI1SC: KBACK=1
	setReg8Bits(KBI1SC, 0x04);            
	// KBI1SC: KBIE=1
	setReg8Bits(KBI1SC, 0x02);            
	
	// Setup the rows as outputs and assert them. 
	PTBDD_PTBDD0 = 1;
	PTBDD_PTBDD1 = 1;
	PTBD_PTBD0 = 1;
	PTBD_PTBD1 = 1;
}

ISR(keyboardISR) {
	
	UINT8 row;
	UINT8 col;
	UINT8 keyPress = 0;
	
	for (row = 1; row <= 2; ++row) {
		// Turn off the row outputs
		PTBD &= 0b11111100;
		// Set the current row to high.
		PTBD |= row;
		
		// Now check the columns.
		for (col = 1; col <= 2; ++col) {
			// If the column is high then this is the key pressed.
			if (PTAD && col) {
				keyPress = row + col;
			}
		}
	}
	
	// Acknowledge the interrupt, so that we can get another.
	KBI1SC_KBACK = 1;
}