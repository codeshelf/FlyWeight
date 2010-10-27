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
#include "gwSystemMacros.h"
#include "gwTypes.h"
#include "commands.h"
#include "USB.h"
#include "serial.h"
#include "string.h"
//#include "Delay.h"
//#include "Leds.h"

xQueueHandle			gGatewayMgmtQueue;
ControllerStateType		gControllerState;


// --------------------------------------------------------------------------

//void gatewayMgmtTask(void *pvParameters) {
//	BufferCntType	rxBufferNum;
//
//	if ( gGatewayMgmtQueue ) {
//		for ( ;; ) {
//
//			// Whenever we need to handle a state change for a  device, we handle it in this management task.
//
//			if (xQueueReceive(gGatewayMgmtQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
//
//				// Just send it over the serial link to the controller.
//				serialTransmitFrame((byte*) (&gRXRadioBuffer[rxBufferNum].bufferStorage), gRXRadioBuffer[rxBufferNum].bufferSize);
//				RELEASE_RX_BUFFER(rxBufferNum);
//			}
//		}
//	}
//
//	/* Will only get here if the queue could not be created. */
//	for ( ;; );
//}

// --------------------------------------------------------------------------

void serialReceiveTask( void *pvParameters ) {

	ECommandGroupIDType			cmdID;
	ENetMgmtSubCmdIDType		subCmdID;
	BufferCntType				txBufferNum = 0;
	gwUINT8					ccrHolder;
	
	// Setup the USB interface.
	//DelayMs(100);
	USB_Init();

	// Send a net-setup command to the controller.
	// It will respond with the channel that we should be using.
	createOutboundNetSetup();
	serialTransmitFrame((gwUINT8*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
	
	
	for ( ;; ) {
	
		GW_WATCHDOG_RESET;

		// Acquire and lock a TX buffer.
		txBufferNum = lockTXBuffer();

		gTXRadioBuffer[txBufferNum].bufferSize = serialReceiveFrame(gTXRadioBuffer[txBufferNum].bufferStorage, TX_BUFFER_SIZE);

		if (gTXRadioBuffer[txBufferNum].bufferSize > 0) {

			gwBoolean sendCmdToRadio = TRUE;
		  	cmdID = getCommandID(gTXRadioBuffer[txBufferNum].bufferStorage);
			if (cmdID == eCommandNetMgmt) {
				subCmdID = getNetMgmtSubCommand(gTXRadioBuffer[txBufferNum].bufferStorage);
				switch (subCmdID) {
					case eNetMgmtSubCmdNetCheck:
						processNetCheckOutboundCommand(txBufferNum);
						sendCmdToRadio = TRUE;
						break;
						
					case eNetMgmtSubCmdNetSetup:
						processNetSetupCommand(txBufferNum);
						// We don't ever want to broadcast net-setup, so continue.
						RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
						sendCmdToRadio = FALSE;
						continue;
						break;
						
					case eNetMgmtSubCmdNetIntfTest:
						processNetIntfTestCommand(txBufferNum);
						// We don't ever want to broadcast intf-test, so continue.
						RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
						sendCmdToRadio = FALSE;
						continue;
						break;
				}
			}

			// Now send the buffer to the transmit queue.
			if (sendCmdToRadio) {
				if (xQueueSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0)) {
				}
			}

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
