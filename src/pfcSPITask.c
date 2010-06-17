/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "pfcSPITask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "remoteMgmtTask.h"
#include "spi.h"
#include "GPIO_Interface.h"
#include "Leds.h"

// Globals

xTaskHandle gPFCTask = NULL;
xQueueHandle gPFCQueue;
ESDCardState gSDCardState;

extern gwBoolean 	gSDCardBusConnected;
extern gwBoolean 	gSDCardVccConnected;
extern EMessageHolderType	gMsgHolder[MAX_NUM_MSG];
extern gwUINT8				gNextMsgToUse;
extern gwUINT8				gCurMsg;

// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {

	ESDCardResponse result;
	gwUINT8 ccrHolder;
	GpioErr_t error;
	SDControlCommandStruct control;

	if (gPFCQueue) {

		gSDCardState = eSDCardStateReady;

		for (;;) {
			if (xQueueReceive(gPFCQueue, &control, portMAX_DELAY) == pdPASS) {

				if (control.deviceType == eSDCardDeviceType1) {

					// Type1 cards have access to the card insert/uninsert IO pin of the SD card.

					// Power cycle the card so that it can reenter the SDCard mode.
					// (It will be in the SPI mode, and can only recover via power cycle.)
					BUS_SW_OFF;
					VCC_SW_OFF;

					/* Some frames will turn off power to the card after the CARD_UNINSERTED instruction.
					 * As soon as the power goes off, the JFET will "reconnect" card causing the frame
					 * to power the card again.  A power-on reset is the result. */

					GW_ENTER_CRITICAL(error);
					MLMERXDisableRequest(&(gMsgHolder[gCurMsg].msg));

					// Need to start conserving power.
					Led1Off();
					Led2Off();

					CARD_UNINSERTED;

					// Wait long enough for the capacitive charge in the SDCard to dissipate.
					vTaskDelay(control.delay.delayMillis);

					CARD_INSERTED;
					VCC_SW_ON;
					BUS_SW_ON;
					GW_EXIT_CRITICAL(error);

				} else {
					// Type2 cards do not have access to the insert/uninsert IO pin of the SD card slot.

					// Reset the SD Card, and then re-init the card into standby mode.
					enableSPI();
					VCC_SW_OFF;
					DelayMs(50);
					VCC_SW_ON;
					enableSDCardBus();
				}

				if (!disableSPI()) {
					result = eAckStateFailed;
				}
				BUS_SW_ON;

			}
			vTaskDelay(10);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
