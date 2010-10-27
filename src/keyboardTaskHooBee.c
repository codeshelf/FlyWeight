/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "keyboardTaskHooBee.h"
#include "keyboard.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"

#define			CHK_KEY_DELAY		10

xQueueHandle 				gKeyboardQueue;
UINT8						gButtonPressed;
extern bool 				gAudioModeRX;
extern bool					gShouldSleep;
extern BufferCntType		gCurAudioTXBuffer;
extern BufferOffsetType		gCurAudioTXBufferPos;
extern bool					gCurAudioTXBufferStarted;
extern LedFlashRunType		gLedFlashSequenceShouldRun;

// --------------------------------------------------------------------------

void keyboardTask(void *pvParameters) {
	UINT8 i;
	UINT8 buttonNum;
	BufferCntType txBufferNum;

	if (gKeyboardQueue) {
		for (;;) {
			GW_WATCHDOG_RESET;

			// If the user has already pressed a button then wait until released
			if (gButtonPressed) {
				gShouldSleep = FALSE;
				if (buttonStillPressed()) {
					// The user is still holding the button.
					//WATCHDOG_RESET;
				} else {
					// Wait a few ms for the remote to stop sending audio packets.
					// (But we need a watchdog here, because the background task won't get called.)
					for (i=0; i < 3; i++) {	
						vTaskDelay(10 * portTICK_RATE_MS);
						GW_WATCHDOG_RESET;
					}

					//  Send a button up message.
					txBufferNum = lockTXBuffer();
					createButtonControlCommand(txBufferNum, gButtonPressed, BUTTON_RELEASED);
					if (transmitPacket(txBufferNum)) {
					};
					gButtonPressed = NO_BUTTON;

					// Indicate button released.
					LEDRedOFF;
					TPM1C0SC_CH0IE = 1;
					TPM1C1SC_CH1IE = 1;
					TPM1C2SC_CH2IE = 1;
					
					// Wait 200ms before restarting to read another button.
					for (i=0; i < 20; i++) {	
						vTaskDelay(10 * portTICK_RATE_MS);
						GW_WATCHDOG_RESET;
					}

					// Start looking for another keypress.
					restartKeyboardISR();
				}
			} else {

				// The keyboard ISR will send us a message when it detects a keypress.
				if (xQueueReceive(gKeyboardQueue, &buttonNum, CHK_KEY_DELAY * portTICK_RATE_MS) == pdPASS) {

					// The user just pressed a button.
					gButtonPressed = buttonNum;
					
					// This will stop the current flashing sequence.
					if (gButtonPressed == HOOBEE_ACK_BUTTON) {
						gLedFlashSequenceShouldRun = FALSE;
						TPM1C0SC_CH0IE = 0;
						TPM1C1SC_CH1IE = 0;
						TPM1C2SC_CH2IE = 0;
						
						// Indicate button released.
						LEDRedON;
					}
					
					// Send an associate request on the current channel.
					txBufferNum = lockTXBuffer();
					createButtonControlCommand(txBufferNum, buttonNum, BUTTON_PRESSED);
					if (transmitPacket(txBufferNum)) {
					};
				}
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
