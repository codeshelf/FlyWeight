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
#include "UART.h"
#include "serial.h"
#include "string.h"
#include "UartLowLevel.h"
//#include "Delay.h"
//#include "Leds.h"

xQueueHandle			gGatewayMgmtQueue;
ControllerStateType		gControllerState;

// --------------------------------------------------------------------------

void serialReceiveTask( void *pvParameters ) {

	ECommandGroupIDType			cmdID;
	ENetMgmtSubCmdIDType		subCmdID;
	BufferCntType				txBufferNum = 0;
	gwUINT8					ccrHolder;
	
	// Setup the USB interface.
	//DelayMs(100);
	UART_Init(UART_1, 1250000, TRUE);

	// Send a net-setup command to the controller.
	// It will respond with the channel that we should be using.
	createOutboundNetSetup();
	
	for ( ;; ) {
	
		GW_WATCHDOG_RESET;

		// Acquire and lock a TX buffer.
		txBufferNum = lockTXBuffer();

		gTXRadioBuffer[txBufferNum].bufferSize = serialReceiveFrame(gTXRadioBuffer[txBufferNum].bufferStorage, TX_BUFFER_SIZE);

		if (gTXRadioBuffer[txBufferNum].bufferSize > 0) {

		  	cmdID = getCommandID(gTXRadioBuffer[txBufferNum].bufferStorage);
			if (cmdID == eCommandNetMgmt) {
				subCmdID = getNetMgmtSubCommand(gTXRadioBuffer[txBufferNum].bufferStorage);
				switch (subCmdID) {
					case eNetMgmtSubCmdNetCheck:
						processNetCheckOutboundCommand(txBufferNum);
						break;
						
					case eNetMgmtSubCmdNetSetup:
						processNetSetupCommand(txBufferNum);
						// We don't ever want to broadcast net-setup, so continue.
						RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
						continue;
						break;
						
					case eNetMgmtSubCmdNetIntfTest:
						processNetIntfTestCommand(txBufferNum);
						// We don't ever want to broadcast intf-test, so continue.
						RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
						continue;
						break;
				}
			}

			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0) != pdTRUE) {
				// We couldn't queue the buffer, so release it.
				RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
			}

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
