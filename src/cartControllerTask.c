/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "cartControllerTask.h"
#include "commands.h"
#include "radioCommon.h"
#include "remoteMgmtTask.h"
#include "GPIO_Interface.h"
#include "string.h"
#include "Delay.h"

#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

xTaskHandle gCartControllerTask = NULL;
//xQueueHandle gCartControllerQueue;

extern DisplayStringType gDisplayDataLine[2];
extern DisplayStringLenType gDisplayDataLineLen[2];
extern DisplayStringLenType gDisplayDataLinePos[2];

// Globals because we pass them by reference.
ScanStringType gRs485String;
ScanStringLenType gRs485StringPos;

// --------------------------------------------------------------------------

void UartReadCallback(UartReadCallbackArgs_t* args) {
//	gu8SCIDataFlag = TRUE;
//	gu16SCINumOfBytes = args->UartNumberBytesReceived;
//	gu8SCIStatus = args->UartStatus;
}

// --------------------------------------------------------------------------

void UartWriteCallback(UartWriteCallbackArgs_t* args) {
//	args->UartNumberBytesSent = args->UartNumberBytesSent * 1;
}

// --------------------------------------------------------------------------

void displayScrollCallback(TmrNumber_t tmrNumber) {
	gwUINT8 error;
	gwUINT8 remainingSpace;

	for (int line = 0; line < 2; ++line) {

		if (gDisplayDataLineLen[line] > DISPLAY_WIDTH) {
			if (line == 0) {
				error = sendDisplayMessage(LINE1_FIRST_POS, strlen(LINE1_FIRST_POS));
			} else {
				error = sendDisplayMessage(LINE2_FIRST_POS, strlen(LINE2_FIRST_POS));
			}
			error = sendDisplayMessage(&(gDisplayDataLine[line][gDisplayDataLinePos[line]]),
					getMin(DISPLAY_WIDTH, (gDisplayDataLineLen[line] - gDisplayDataLinePos[line])));

			// If there's space at the end of the display then restart from the beginning of the message.
			if ((gDisplayDataLineLen[line] - gDisplayDataLinePos[line]) < DISPLAY_WIDTH) {
				remainingSpace = DISPLAY_WIDTH - (gDisplayDataLineLen[line] - gDisplayDataLinePos[line]);
				error = sendDisplayMessage("   ", getMin(3, remainingSpace));
				if (remainingSpace > 3) {
					error = sendDisplayMessage(&(gDisplayDataLine[line][0]), remainingSpace - 3);
				}
			}

			gDisplayDataLinePos[line] += 3;
			if (gDisplayDataLinePos[line] >= gDisplayDataLineLen[line]) {
				gDisplayDataLinePos[line] = 0;
			}
		}
	}
}

// --------------------------------------------------------------------------

void cartControllerTask(void *pvParameters) {

	gwUINT8 ccrHolder;

	RS485_TX_ON

	gwUINT8 message[] = { POS_CTRL_CLEAR, POS_CTRL_ALL_POSITIONS };
	serialTransmitFrame(UART_2, message, 2);

	vTaskDelay(25);
	RS485_TX_OFF

	for (;;) {

		// Clear the RS485 string.
		gRs485String[0] = 0;
		gRs485StringPos = 0;

		// Wait until there are characters in the FIFO
		while (UART2_REGS_P ->Urxcon == 0) {
			vTaskDelay(1);
		}

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		while ((UART2_REGS_P ->Urxcon != 0) && (gRs485StringPos < MAX_SCAN_STRING_BYTES)) {
			gRs485String[gRs485StringPos++] = UART2_REGS_P ->Udata;
			gRs485String[gRs485StringPos] = 0;

			// If there's no characters - then wait 2ms to see if more will arrive.
			if ((UART2_REGS_P ->Urxcon == 0)) {
				DelayMs(10);
			}
		}
		GW_EXIT_CRITICAL(ccrHolder);

		// Disable all of the position controllers on the cart.
		// TODO

		// Now send the scan string.
		BufferCntType txBufferNum = lockTXBuffer();
		createButtonCommand(txBufferNum, gRs485String[1], gRs485String[2]);
		transmitPacket(txBufferNum);

	}
}
