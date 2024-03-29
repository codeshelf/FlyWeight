/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "serial.h"
#include "task.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"

//ELocalStatusType		gLocalDeviceState;

// --------------------------------------------------------------------------

void sendOneChar(gwUINT8 portNum, UART_TComData inDataPtr) {

	// Send a character.
	// (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	while (UART_SendChar(portNum, inDataPtr) != GW_USB_OK) {
		// Consider a timeout where we just reset the MCU.
	};

}

// --------------------------------------------------------------------------

void readOneChar(gwUINT8 portNum, UART_TComData *outDataPtr) {
	UART_ReadOneChar(portNum, outDataPtr);
}

// --------------------------------------------------------------------------

void serialTransmitFrame(gwUINT8 portNum, UART_TComData *inDataPtr, gwUINT16 inSize) {

//	UINT16	bytesSent;
	gwUINT16 totalBytesSent;
//	gwUINT8	status;
	gwUINT16 charsSent;

	// Send the frame contents to the controller via the serial port.
	// First send the framing character.
#pragma MESSAGE DISABLE C2706 /* WARNING C2706: Octal # */
	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(portNum, END);

	totalBytesSent = 0;

//	while (totalBytesSent < inSize) {
//		status = USB_SendBlock(inDataPtr + totalBytesSent, inSize - totalBytesSent, &bytesSent);
//		totalBytesSent += bytesSent;
//	}

	for (charsSent = 0; charsSent < inSize; charsSent++) {

		switch (*inDataPtr) {
			/* if it's the same code as an END character, we send a
			 * special two character code so as not to make the
			 * receiver think we sent an END
			 */
			case END:
				sendOneChar(portNum, ESC);
				sendOneChar(portNum, ESC_END);
				break;

				/* if it's the same code as an ESC character,
				 * we send a special two character code so as not
				 * to make the receiver think we sent an ESC
				 */
			case ESC:
				sendOneChar(portNum, ESC);
				sendOneChar(portNum, ESC_ESC);
				break;

				/* otherwise, we just send the character
				 */
			default:
				sendOneChar(portNum, *inDataPtr);
		}
		inDataPtr++;
	}

	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(portNum, END);
	sendOneChar(portNum, END);

	GW_WATCHDOG_RESET;
}

// --------------------------------------------------------------------------

BufferCntType serialReceiveFrame(gwUINT8 portNum, BufferStoragePtrType inFramePtr, BufferCntType inMaxFrameSize) {
	BufferStorageType nextByte;
	BufferCntType bytesReceived = 0;

	// Loop reading bytes until we put together a whole frame.
	// Make sure not to copy them into the frame if we run out of room.

#pragma MESSAGE DISABLE C4000 /* WARNING C4000: condition always true. */
	while (bytesReceived < inMaxFrameSize) {

		readOneChar(portNum, &nextByte);

		switch (nextByte) {

			case END:
				if (bytesReceived) {
					GW_WATCHDOG_RESET;
					return bytesReceived;
				} else {
					break;
				}

			case ESC:
				readOneChar(portNum, &nextByte);

				switch (nextByte) {
					case ESC_END:
						nextByte = END;
						break;
					case ESC_ESC:
						nextByte = ESC;
						break;
					}
				break;

			default:
				break;
		}

		if (bytesReceived < inMaxFrameSize)
			inFramePtr[bytesReceived++] = nextByte;
	}
}

