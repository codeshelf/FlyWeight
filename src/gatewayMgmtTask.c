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
#include "serial.h"
#include "PE_Types.h"
#include "simple_mac.h"
#include "string.h"
#ifdef __WatchDog
	#include "Watchdog.h"
#endif

xQueueHandle			gGatewayMgmtQueue;
ControllerStateType		gControllerState;
extern byte				gCCRHolder;


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
	BufferCntType				txBufferNum;

	// Send a net-setup command to the controller.
	// It will respond with the channel that we should be using.
	createOutboundNetsetup();
	serialTransmitFrame((byte*) (&gTXRadioBuffer[txBufferNum].bufferStorage), gTXRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum);
	
	
	for ( ;; ) {
	
		#ifdef __WatchDog
		WatchDog_Clear();
		#endif

		// Don't try to get a frame if there is no free buffer.
		while (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse) {
			vTaskDelay(1);
		}

		gTXRadioBuffer[gTXCurBufferNum].bufferSize = serialReceiveFrame(gTXRadioBuffer[gTXCurBufferNum].bufferStorage, TX_BUFFER_SIZE);

		if (gTXRadioBuffer[gTXCurBufferNum].bufferSize > 0) {
			// Mark the transmit buffer full.
			gTXRadioBuffer[gTXCurBufferNum].bufferStatus = eBufferStateInUse;
			
			// Remember the buffer we just filled and then advance the buffer system.
			txBufferNum = gTXCurBufferNum;
			advanceTXBuffer();

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
						continue;
						break;
				}
			}

			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0)) {
			}

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
