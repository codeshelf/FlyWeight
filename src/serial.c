/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#include "serial.h"
#include "task.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"

ELocalStatusType		gLocalDeviceState;

// --------------------------------------------------------------------------

void sendOneChar(USB_TComData inDataPtr) {

	// Send a character.
	// (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	while (USB_SendChar(inDataPtr) != GW_USB_OK) {
		// Consider a timeout where we just reset the MCU.
	};

}

// --------------------------------------------------------------------------

void readOneChar(USB_TComData *outDataPtr) {
	USB_ReadOneChar(outDataPtr);
}

// --------------------------------------------------------------------------

void serialTransmitFrame(USB_TComData *inDataPtr, gwUINT16 inSize) {

//	UINT16	bytesSent;
	gwUINT16	totalBytesSent;
//	gwUINT8	status;
 	gwUINT16 	charsSent;

	// Send the packet contents to the controller via the serial port.
	// First send the framing character.
#pragma MESSAGE DISABLE C2706 /* WARNING C2706: Octal # */
	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
 	sendOneChar(END);

	totalBytesSent = 0;

//	while (totalBytesSent < inSize) {
//		status = USB_SendBlock(inDataPtr + totalBytesSent, inSize - totalBytesSent, &bytesSent);
//		totalBytesSent += bytesSent;
//	}

	for (charsSent = 0; charsSent < inSize; charsSent++) {

        switch(*inDataPtr) {
			/* if it's the same code as an END character, we send a
			 * special two character code so as not to make the
			 * receiver think we sent an END
			 */
			case END:
				sendOneChar(ESC);
				sendOneChar(ESC_END);
				break;

			/* if it's the same code as an ESC character,
			 * we send a special two character code so as not
			 * to make the receiver think we sent an ESC
			 */
			case ESC:
				sendOneChar(ESC);
				sendOneChar(ESC_ESC);
				break;

			/* otherwise, we just send the character
			 */
			default:
				sendOneChar(*inDataPtr);
		}

		inDataPtr++;
	}

	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(END);
	sendOneChar(END);
}

// --------------------------------------------------------------------------

BufferCntType serialReceiveFrame(BufferStoragePtrType inFramePtr, BufferCntType inMaxPacketSize) {
	BufferStorageType nextByte;
	BufferCntType bytesReceived = 0;
//	gwUINT8 result;
//	USB_TError usbError;

	// Loop reading bytes until we put together a whole packet.
	// Make sure not to copy them into the packet if we run out of room.

	// If there's no character waiting then delay until we get one.
//	if (USB_GetCharsInRxBuf() == 0)
//		vTaskDelay(1);

#pragma MESSAGE DISABLE C4000 /* WARNING C4000: condition always true. */
	while (bytesReceived < inMaxPacketSize) {

//		result = USB_RecvChar(&nextByte);
//		if (result == ERR_RXEMPTY) {
//			//vTaskDelay(1);
//		} else if (result != ERR_OK) {
//			USB_GetError(&usbError);
//		} else {
		readOneChar(&nextByte);

		{
			switch (nextByte) {

				// If it's an END character then we're done with the packet.
				case END:
					if (bytesReceived)
						return bytesReceived;
					else
						break;

				/* If it's the same code as an ESC character, wait and get another character and then figure out
				 * what to store in the packet based on that.
				 */
				case ESC:
					readOneChar(&nextByte);

					/* If "c" is not one of these two, then we have a protocol violation.  The best bet
					 * seems to be to leave the byte alone and just stuff it into the packet
					 */
					switch (nextByte) {
						case ESC_END:
							nextByte = END;
							break;
						case ESC_ESC:
							nextByte = ESC;
							break;
					}

				// Here we fall into the default handler and let it store the character for us.
				default:
					if (bytesReceived < inMaxPacketSize)
						inFramePtr[bytesReceived++] = nextByte;
			}
		}
	}
}

