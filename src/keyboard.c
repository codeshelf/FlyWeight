/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "keyboard.h"
#include "keyboardTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "IO_Map.h"
#include "PE_types.h"
#include "pub_def.h"

	/*
	 * The keyboard is a matrix:
	 * row1 = PTB4
	 * row2 = PTB5
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
	PTBDD_PTBDD4 = 1;
	PTBDD_PTBDD5 = 1;
	PTBD_PTBD4 = 1;
	PTBD_PTBD5 = 1;
}

ISR(keyboardISR) {
	
	UINT8 	row;
	UINT8 	col;
	UINT8 	buttonNum = 0;
	UINT8 	tickVal;
	
	// Disable the KBI interrupt
	KBI1SC_KBIE = 0;
	
	// Debounce delay for 4ms
	tickVal = xTaskGetTickCount() + (4 * portTICK_RATE_MS);
	while (xTaskGetTickCount() < tickVal) {	
	}
	
	for (row = 0; row <= 1; ++row) {
		// Turn off the row outputs
		PTBD &= 0b11001111;
		// Set the current row to high.
		PTBD |= 1 << (row + 4);
		
		// Now check the columns.
		for (col = 0; col <= 1; ++col) {
			// If the column is high then this is the key pressed.
			if (PTAD & (1 << (col + 5))) {
				buttonNum = (row * 2)  + col + 1;
			}
		}
	}
	
	if (buttonNum > 0) {
		// Now that we know what key we pressed send it to the controller.
		// Send the message to the radio task's queue.
		if (xQueueSendFromISR(gKeyboardQueue, &buttonNum, pdFALSE)) {
		}
	}
	
	// Reset the row outputs.
	PTBD |= 0b00110000;

	// Acknowledge the interrupt and re-enable KBIE, so that we can get another.
	KBI1SC_KBACK = 1;
	KBI1SC_KBIE = 1;
}