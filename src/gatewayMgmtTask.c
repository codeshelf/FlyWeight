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

xQueueHandle		gGatewayMgmtQueue;
ControllerStateType	gControllerState;

// Figure out how much time to delay for flow-control.
//portTickType		gBufferTimeMS = (float) (1.0 / (10000.0 / RX_BUFFER_SIZE)) * 1000;
// We now do flow-control at the controller on the PC.
portTickType		gBufferTimeMS = 2;

// --------------------------------------------------------------------------

void gatewayMgmtTask(void *pvParameters) {
	RemoteAddrType	slotNum;
//	UINT16 bytesSent;

	if ( gGatewayMgmtQueue ) {
		for ( ;; ) {

			// Whenever we need to handle a state change for a  device, we handle it in this management task.

			if (xQueueReceive(gGatewayMgmtQueue, &slotNum, portMAX_DELAY) == pdPASS) {

				// Just send it over the serial link to the controller.
				serialTransmitFrame((byte*) (&gRXRadioBuffer[gRXCurBufferNum].bufferStorage), gRXRadioBuffer[gRXCurBufferNum].bufferSize);

				//				// Get the state of the remote at the named slot.
				//				switch (gRemoteStateTable[slotNum].remoteState) {
				//
				//					case eRemoteStateWakeRcvd:
				//						// Respond to the remote's WAKE command with an ASSIGN command letting it know which destination slot it has.
				//						createAssignCommand(gTXCurBufferNum, &(gRemoteStateTable[slotNum].remoteUniqueID), slotNum);
				//						if (transmitPacket(gTXCurBufferNum)) {
				//						};
				//						gRemoteStateTable[slotNum].remoteState = eRemoteStateAddrAssignSent;
				//						gMainRemote = slotNum;
				//
				//						// Resignal the manager about the new state.
				//						if (xQueueSend(gGatewayMgmtQueue, &slotNum, pdFALSE)) {
				//						}
				//						break;
				//
				//					case eRemoteStateAddrAssignSent:
				//						// Now that the remote has an assigned address we need to ask it to describe
				//						// it's capabilities and characteristics.
				//						createQueryCommand(gTXCurBufferNum, slotNum);
				//						if (transmitPacket(gTXCurBufferNum)) {
				//						};
				//						gRemoteStateTable[slotNum].remoteState = eRemoteStateQuerySent;
				//
				//						// Resignal the manager about the new state.
				//						if (xQueueSend(gGatewayMgmtQueue, &slotNum, pdFALSE)) {
				//						}
				//						break;
				//
				//					case eRemoteStateRespRcvd:
				//						// Now that the remote has an assigned address we need to ask it to describe
				//						// it's capabilities and characteristics.
				//						createDescCommand(gTXCurBufferNum, slotNum);
				//						if (transmitPacket(gTXCurBufferNum)) {
				//						};
				//						gRemoteStateTable[slotNum].remoteState = eRemoteStateDescSent;
				//
				//						// Resignal the manager about the new state.
				//						if (xQueueSend(gGatewayMgmtQueue, &slotNum, pdFALSE)) {
				//						}
				//						break;
				//
				//					case eRemoteStateDescSent:
				//						gRemoteStateTable[slotNum].remoteState = eRemoteStateRun;
				//						break;
				//
				//					default:
				//						;
				//
				//				}
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void serialReceiveTask( void *pvParameters ) {

	portTickType	lastTick;
	BufferCntType	txBufferNum;

	for ( ;; ) {

		// Check if there is enough data in the serial buffer to fill the next *empty* transmit queue.

//		if ((USB_GetCharsInRxBuf() >= TX_BUFFER_SIZE)
//				&& (gTXRadioBuffer[gTXCurBufferNum].bufferStatus != eBufferStateInUse)
//				&& (gMainRemote != INVALID_REMOTE)) {
//
//			createDataCommand(gTXCurBufferNum, gMainRemote);
//			USB_RecvBlock((USB_TComData *) &gTXRadioBuffer[gTXCurBufferNum].bufferStorage[2], TX_BUFFER_SIZE - 2, &bytesReceived);
			gTXRadioBuffer[gTXCurBufferNum].bufferSize = serialReceiveFrame(gTXRadioBuffer[gTXCurBufferNum].bufferStorage, TX_BUFFER_SIZE);

			if (gTXRadioBuffer[gTXCurBufferNum].bufferSize > 0) {
				// Mark the transmit buffer full.
				gTXRadioBuffer[gTXCurBufferNum].bufferStatus = eBufferStateInUse;

				// Remember the buffer we just filled and then advance the buffer system.
				txBufferNum = gTXCurBufferNum;
				advanceTXBuffer();

				// Now send the buffer to the transmit queue.
				if (xQueueSend(gRadioTransmitQueue, &txBufferNum, pdFALSE)) {
				}

				// Wait until the we've sent the right number of packets per second.
				//vTaskDelayUntil(&lastTick, gBufferTimeMS);
			}
//		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void serialTransmitFrame(USB_TComData *inDataPtr, word inSize) {

	UINT16	bytesSent;
	UINT16	totalBytesSent;
	byte	status;

	// Send the packet contents to the controller via the serial port.
	// First send the framing character.
#pragma MESSAGE DISABLE C2706 /* WARNING C2706: Octal # */
	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	while (USB_SendChar(END) != ERR_OK) {
	};

	totalBytesSent = 0;

	while (totalBytesSent < inSize) {
		status = USB_SendBlock(inDataPtr + totalBytesSent, inSize - totalBytesSent, &bytesSent);
		totalBytesSent += bytesSent;
	}

	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	while (USB_SendChar(END) != ERR_OK) {
	};
}

// --------------------------------------------------------------------------

BufferCntType serialReceiveFrame(BufferStoragePtrType inFramePtr, BufferCntType inMaxPacketSize) {
	BufferStorageType nextByte;
	BufferCntType bytesReceived = 0;
	byte result;
	USB_TError usbError;

	// Loop reading bytes until we put together a whole packet.
	// Make sure not to copy them into the packet if we run out of room.

#pragma MESSAGE DISABLE C4000 /* WARNING C4000: condition always true. */
	while (TRUE) {

//		if (USB_RecvChar(&nextByte) != ERR_OK)
//			return bytesReceived;
//		else {
		result = USB_RecvChar(&nextByte);
		if (result == ERR_RXEMPTY) {
		} else if (result != ERR_OK) {
			USB_GetError(&usbError);
		} else {
		
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
					USB_RecvChar(&nextByte);
	
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
