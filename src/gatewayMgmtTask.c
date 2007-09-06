/*
FlyWeight
 Copyright 2005, 2006 Jeffrey B. Williams
All rights reserved

$Id$
$Name$	
*/

#include "gatewayMgmtTask.h"
#include "gatewayRadioTask.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"
#include "USB.h"
#include "PE_Types.h"
#include "simple_mac.h"
#include "string.h"

xQueueHandle		gGatewayMgmtQueue;
ControllerStateType	gControllerState;

#define	kBufferSize		10
#define	kHighWaterMark  5

static UINT8		gCurrentBufferPos = 0;
static UINT8		gCurrentBufferSize = 0;
static USB_TComData	gSCIBuffer[kBufferSize];

void checkUSBInterface(void);

// --------------------------------------------------------------------------

void gatewayMgmtTask(void *pvParameters) {
	BufferCntType	rxBufferNum;
//	UINT16 bytesSent;

	if ( gGatewayMgmtQueue ) {
		for ( ;; ) {

			// Whenever we need to handle a state change for a  device, we handle it in this management task.

			if (xQueueReceive(gGatewayMgmtQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {

				// Just send it over the serial link to the controller.
				serialTransmitFrame((byte*) (&gRXRadioBuffer[rxBufferNum].bufferStorage), gRXRadioBuffer[rxBufferNum].bufferSize);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void serialReceiveTask( void *pvParameters ) {

	ECommandIDType			cmdID;
	ENetMgmtSubCommandIDType	subCmdID;
	BufferCntType				txBufferNum;

	for ( ;; ) {

		// Don't try to get a frame if there is no free buffer.
		while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
			//if (uxQueueMessagesWaiting(gRadioTransmitQueue) < TX_QUEUE_SIZE) {
			//	if (xQueueSend(gRadioTransmitQueue, &gTXCurBufferNum, pdFALSE)) {
			//	}
			//}
			vTaskDelay(1);
		}

		gTXRadioBuffer[gTXCurBufferNum].bufferSize = serialReceiveFrame(gTXRadioBuffer[gTXCurBufferNum].bufferStorage, TX_BUFFER_SIZE);

		if (gTXRadioBuffer[gTXCurBufferNum].bufferSize > 0) {
			// Mark the transmit buffer full.
			gTXRadioBuffer[gTXCurBufferNum].bufferStatus = eBufferStateInUse;
			
			// Remember the buffer we just filled and then advance the buffer system.
			txBufferNum = gTXCurBufferNum;
			advanceTXBuffer();

			cmdID = getCommand(txBufferNum);
			if (cmdID == eCommandNetMgmt) {
				subCmdID = getNetMgmtSubCommand(txBufferNum);
				if (subCmdID == eNetMgmtSubCommandSetup) {
					processNetSetupCommand(txBufferNum);
					// Continue processing new frames. (Don't transmit this frame.)
					continue;
				}
			}

			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &txBufferNum, pdFALSE)) {
			}

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void sendOneChar(USB_TComData inDataPtr) {

	// Send a character. 
	// (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	while (USB_SendChar(inDataPtr) != ERR_OK) {
		// Consider a timeout where we just reset the MCU.
	};

}

// --------------------------------------------------------------------------

void readOneChar(USB_TComData *outDataPtr) {

	// Read a character. 
	// (For some stupid reason the Freescale USB routine doesn't try very hard, so we have to loop until it succeeds.)
/*	UINT8 error;
	do {
		error = USB_RecvChar(inDataPtr);
		if ((error != ERR_OK) && (error != ERR_RXEMPTY)) {
			error ++;
			error --;
		}
	} while (error == ERR_RXEMPTY);
*/
	// New way of reading.
#pragma MESSAGE DISABLE C4000 /* WARNING C4000: Always true */
	while (TRUE) {
		if (gCurrentBufferPos < gCurrentBufferSize) {
			*outDataPtr = gSCIBuffer[gCurrentBufferPos++];
			return;
		} else {
			checkUSBInterface();
		}
	}
}


// --------------------------------------------------------------------------

void serialTransmitFrame(USB_TComData *inDataPtr, word inSize) {

//	UINT16	bytesSent;
	UINT16	totalBytesSent;
//	byte	status;
 	word 	charsSent;

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
}

// --------------------------------------------------------------------------

BufferCntType serialReceiveFrame(BufferStoragePtrType inFramePtr, BufferCntType inMaxPacketSize) {
	BufferStorageType nextByte;
	BufferCntType bytesReceived = 0;
//	byte result;
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

// --------------------------------------------------------------------------

/*
	We're not able to read the SCI using interrupts.  The problem is that the data rate is very
	high (in order to support multiple audio channels).  The RDRF fills very fast,	and if we 
	(or FreeRTOS or SMAC) have suspended global interrupts then we don't get the Rx interrupt
	for that RDRF flag.  Moreover, when interrupts get turned back on we have no way of knowing
	we just missed the RDRF flag.  The result is that the Rx buffer doesn't get cleared and we end
	up with a serial overrun error.  At 250000 baud we're missing about 2-3% of the characters.
	Due to the low-power nature of the device, we are not really able to handle such a high
	level of data loss.  (There is no retry, ECC, etc.)
	
	So...
	
	What we do instead is we poll the SCI.  We've ramped the baud rate to the maximum (1250000 baud)
	and when we need a character we check to see if a local buffer has any.  If that buffer is empty
	then we disable global interrupts, assert RTS and check for arriving characters.  After we
	get a certain numbers of characters into the local buffer, we unassert RTS and keep reading characters 
	until RDRF settles.  After that we reenable global interrupts and continue processing of the 
	normal OS tasks.  At 1250000 baud it takes about 20-30 uSecs to read 10 or so characters.  
	This is only 1/10th of an OS quantum.  Since the gateway only services the SCI and the radio 
	it is safe to suspend global interrupts for this short time.  (The radio asserts the IRQ pin which
	we detect immediately when global interrupts resume.)
*/

void checkUSBInterface() {

	UINT8			loopCheck;
	USB_TComData	lostChar;
	
	gCurrentBufferPos = 0;
	gCurrentBufferSize = 0;
	
	// Disable interrupts, so that this is all we're doing.
	EnterCritical();
	
	// Turn RTS on.
	RTS_ON;
	
	// Loop until RDRF is on, or until we timeout.
	loopCheck = 0;
	while (!SCI2S1_RDRF) {
		loopCheck++;
		if (loopCheck > 100) {
			RTS_OFF;
			ExitCritical();
			return;
		}
	}
	
	// Read characters until the high water mark, but keep reading characters until RDRF settles.
	loopCheck = 0;
	while (loopCheck < 250) {
	
		if (SCI2S1_RDRF) {
			if (gCurrentBufferSize <= kBufferSize) {
				gSCIBuffer[gCurrentBufferSize++] = SCI2D;
			} else {
				lostChar = SCI2D;
			}
		} else {
			// No RDRF
			loopCheck++;
		}
	
		// If we've read to the high water mark then turn RTS off.
		if (gCurrentBufferSize > kHighWaterMark) {
			RTS_OFF;
		}
	}
	
	// Resume normal OS/interrupt processing.
	ExitCritical();
}
