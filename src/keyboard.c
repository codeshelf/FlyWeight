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
#include "Cpu.h"
#include "PE_types.h"
#include "pub_def.h"

// --------------------------------------------------------------------------
// Definitions.
#define	KEYBOARD_ROWS		2
#define KEYBOARD_COLS		2
#define SAMPLES				10

portTickType				gLastPressSeenTick;

// --------------------------------------------------------------------------
// Functions.

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


void KBISetup() {

	// KBI1SC: KBIE=0
	KBI1SC_KBIE = 0;
	           
	// Set the appropriate pin enable bits.
	setReg8(KBI1PE, KB_PEBITS);                
	setReg8(KBI1SC, KB_PEBITS);                
	
	KBI1SC_KBACK = 1;            
	KBI1SC_KBIE = 1;          
	
	// Setup the rows as outputs and assert them. 
	KB_SETUP_ROW0 = 1;
	KB_SETUP_ROW1 = 1;
	KB_SETUP_COL0 = 0;
	KB_SETUP_COL1 = 0;
	KB_ROW0 = 1;
	KB_ROW1 = 1;
}

	

ISR(keyboardISR) {
	
	byte ccrHolder;
	UINT8 row;
	UINT8 col;
	UINT8 buttonNum = 0;
	UINT8 sample;
	bool shouldRestartISR = FALSE;
	UINT8 i;
	UINT8 maxPresses;
	UINT8 buttonArray[KEYBOARD_ROWS * KEYBOARD_COLS + 1];
	
	EnterCriticalArg(ccrHolder);
	
	// Disable the KBI interrupt
	KBI1SC_KBIE = 0;
	
	gLastPressSeenTick = 0;
	
	// Debounce delay
	Cpu_Delay100US(2);
	
	for (i = 0; i <= 4; i++) {
		buttonArray[i] = 0;	
	}
	maxPresses = 0;	

	//GET_BUTTON_PRESSED(buttonNum);
	for (sample = 0; sample < SAMPLES; sample++) {							
																			
		buttonNum = 0;									
		for (row = 0; row <= (KEYBOARD_ROWS - 1); ++row) {					
			/* Turn off the row outputs	*/									
			KB_ROW0 = 0;													
			KB_ROW1 = 0;													
																			
			/* Set the current row to high. */								
			PTBD |= 1 << (row);												
																			
			/* Now check the columns. */
			for (col = 0; col <= (KEYBOARD_COLS - 1); ++col) {				
				/* If the column is high then this is the key pressed. */	
				if (PTAD & (1 << (col + KB_BIT_COL_OFFSET))) {				
					buttonNum = (row * 2)  + col + 1;					
				}															
			}																
		}																	
		buttonArray[buttonNum]++;																													
	}																		
	
	buttonNum = 0;
	for (i = 1; i <= 4; i++) {
		if (buttonArray[i] > maxPresses) {
			maxPresses = buttonArray[i];
			buttonNum = i;
		}
	}	
	
	if (buttonNum > 0) {
		// Now that we know what key we pressed send it to the controller.
		// Send the message to the radio task's queue.
		if (xQueueSendFromISR(gKeyboardQueue, &buttonNum, (portTickType) 0)) {
		}
	} else {
		// If for some reason we don't get a real button press then we need to immediately reenable the KBIE.
		shouldRestartISR = TRUE;
	}
	
	ExitCriticalArg(ccrHolder);
	
	if (shouldRestartISR)
		restartKeyboardISR();
}

/*
 * Restart the keyboard interrupt process.
 */
void restartKeyboardISR(void) {
	
	// Reset the row outputs.
	KB_ROW0 = 1;
	KB_ROW1 = 1;

	// Acknowledge the interrupt and re-enable KBIE, so that we can get another.
	KBI1SC_KBACK = 1;
	KBI1SC_KBIE = 1;	
}

/*
 * Check if the last button pressed by the user is still pressed.
 */

bool buttonStillPressed() {
	UINT8	col;
	UINT8	sample;
	bool	anyButtonPressed;
	
	anyButtonPressed = FALSE;									
	/* Turn on the row outputs	*/									
	KB_ROW0 = 1;													
	KB_ROW1 = 1;													
																	
	for (sample = 0; sample < SAMPLES; sample++) {							
		/* Now check the columns. */									
		for (col = 0; col <= (KEYBOARD_COLS - 1); ++col) {				
			/* If the column is high then this is the key pressed. */	
			if (PTAD & (1 << (col + KB_BIT_COL_OFFSET))) {				
				anyButtonPressed = TRUE;					
			}															
		}
	}
	
	if (anyButtonPressed) {
		gLastPressSeenTick = xTaskGetTickCount();
	} else {
		// We need to wait at least 250ms since we last saw a button press to say if it's really no longer pressed.
		if (xTaskGetTickCount() < gLastPressSeenTick + (100 * portTICK_RATE_MS)) {
			anyButtonPressed = TRUE;
		}
	}
	
	return (anyButtonPressed);
}