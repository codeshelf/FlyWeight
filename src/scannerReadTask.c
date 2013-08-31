/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "scannerReadTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gatewayRadioTask.h"
#include "commands.h"
#include "Delay.h"

xTaskHandle gScannerReadTask = NULL;

ScanStringType gScanString;
ScanStringLenType gScanStringPos;
extern LedPositionType gTotalLedSolidDataElements;
extern LedPositionType gTotalLedFlashDataElements;

// --------------------------------------------------------------------------

void scannerReadTask(void *pvParameters) {

	gwUINT8 ccrHolder;

	for (;;) {

		// Clear the scan string.
		gScanString[0] = NULL;
		gScanStringPos = 0;

		// Wait until there are characters in the FIFO
		while (UART1_REGS_P ->Urxcon == 0) {
			vTaskDelay(1);
		}

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		while ((UART1_REGS_P ->Urxcon != 0) && (gScanStringPos < MAX_SCAN_STRING_BYTES)) {
			gScanString[gScanStringPos++] = UART1_REGS_P ->Udata;
			gScanString[gScanStringPos] = NULL;

			// If there's no characters - then wait 2ms to see if more will arrive.
			if ((UART1_REGS_P ->Urxcon == 0)) {
				DelayMs(4);
			}
		}
		GW_EXIT_CRITICAL(ccrHolder);

		// Disable all of the LEDs on the cart.
		gTotalLedSolidDataElements = 0;
		gTotalLedFlashDataElements = 0;

		// Now send the scan string.
		BufferCntType txBufferNum = lockTXBuffer();
		createScanCommand(txBufferNum, &gScanString, gScanStringPos - 1);
		transmitPacket(txBufferNum);

	}
}
