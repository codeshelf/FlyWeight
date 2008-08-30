/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved
 
 $Id$
 $Name$	
 */

#include "keyboardTask.h"
#include "keyboard.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"
#ifdef __WatchDog
	#include "Watchdog.h"
#endif

#define			CHK_KEY_DELAY		10

xQueueHandle 			gKeyboardQueue;
UINT8					gButtonPressed;
extern bool 			gAudioModeRX;
extern bool				gShouldSleep;
extern BufferCntType	gCurAudioTXBuffer;
extern BufferOffsetType	gCurAudioTXBufferPos;
extern bool				gCurAudioTXBufferStarted;

// --------------------------------------------------------------------------

void keyboardTask(void *pvParameters) {
	UINT8 buttonNum;
	BufferCntType txBufferNum;

	if (gKeyboardQueue) {
		for (;;) {
#ifdef __WatchDog
			WatchDog_Clear();
#endif

			// If the user has already pressed a button then wait until released
			if (gButtonPressed) {
				gShouldSleep = FALSE;
				if (buttonStillPressed()) {
					// The user is still holding the button.
				} else {
					// The user just released the button.
					if (gButtonPressed == PTT_BUTTON) {
					
						// Now put us into RX mode.
						gAudioModeRX = TRUE;
						
						// Turn off the ATD module
						ATD_OFF;
						MIC_AMP_OFF;

						// Wait a few ms for the remote to stop sending audio packets.
						vTaskDelay(20 * portTICK_RATE_MS);

						// Send the last TX packet.
						if (gTXRadioBuffer[gCurAudioTXBuffer].bufferStatus == eBufferStateInUse) {
							gTXRadioBuffer[gCurAudioTXBuffer].bufferSize = gCurAudioTXBufferPos;
							transmitPacket(gCurAudioTXBuffer);
							gCurAudioTXBufferStarted = FALSE;
						}
						
					}

					// Wait  a few ms for the remote to stop sending audio packets.
					vTaskDelay(25 * portTICK_RATE_MS);

					// Wait until a TX packet is free.
					while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
						vTaskDelay(1 * portTICK_RATE_MS);
					}

					//  Send a button up message.
					txBufferNum = gTXCurBufferNum;
					advanceTXBuffer();
					createButtonControlCommand(txBufferNum, gButtonPressed, BUTTON_RELEASED);
					if (transmitPacket(txBufferNum)) {
					};
					gButtonPressed = NO_BUTTON;
					
					// Wait 250ms before restarting to read another button.
					vTaskDelay(250 * portTICK_RATE_MS);
					
					// Start looking for another keypress.
					restartKeyboardISR();
				}
			} else {

				// The keyboard ISR will send us a message when it detects a keypress.
				if (xQueueReceive(gKeyboardQueue, &buttonNum, CHK_KEY_DELAY * portTICK_RATE_MS) == pdPASS) {

					// The user just pressed a button.
					gButtonPressed = buttonNum;
					
					// Send an associate request on the current channel.
					txBufferNum = gTXCurBufferNum;
					advanceTXBuffer();
					createButtonControlCommand(txBufferNum, buttonNum, BUTTON_PRESSED);
					if (transmitPacket(txBufferNum)) {
					};
					
					if (gButtonPressed == PTT_BUTTON) {
						// Wait a few ms for the controller to stop sending audio packets.
						vTaskDelay(5 * portTICK_RATE_MS);
						
						// Now put us into TX mode.
						gAudioModeRX = FALSE;
						
						// Turn on the ATD module
						ATD_ON;
						MIC_AMP_ON;
					}
				}
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}