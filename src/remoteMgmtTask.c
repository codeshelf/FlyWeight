/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "remoteMgmtTask.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"

xQueueHandle 				gRemoteMgmtQueue;
ELocalStatusType			gLocalDeviceState;
RemoteAddrType				gMyAddr = INVALID_REMOTE;
//const RemoteUniqueIDType	kUniqueID = "22345678";

// --------------------------------------------------------------------------

void remoteMgmtTask( void *pvParameters ) {
	BufferCntType	rxBufferNum = 0;

	if ( gRemoteMgmtQueue ) {
	
		// Signal ourselves to start the process.
		if (xQueueSend(gRemoteMgmtQueue, &rxBufferNum, pdFALSE)) {
		}

		for ( ;; ) {

			// Whenever we need to handle a state change for a  device, we handle it in this management task.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
			
				// Get the state of the remote at the named slot.
				switch (gLocalDeviceState) {

					case eLocalStateJustWoke:
						// If we're in the init mode then we need to transmit a wake command.
						createWakeCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
						if (transmitPacket(gTXCurBufferNum)) {
						};
						gLocalDeviceState = eLocalStateWakeSent;
						break;
		
					case eLocalStateAddrAssignRcvd:
						processAssignCommand(rxBufferNum);
						break;
						
					case eLocalStateQueryRcvd:
						// Now that the remote has an assigned address we need to ask it to describe
						// it's capabilities and characteristics.
						processQueryCommand(rxBufferNum, getCommandSrcAddr(rxBufferNum));
						break;
						
					// If we're in the run state, and receive a command then we need to handle that command.
					case eLocalStateRun:
						
						switch (getCommandNumber(rxBufferNum)) {
						
							case eCommandEndpointAdjust:
								break;
								
							case eCommandControl:
								break;
								
							default:
								break;
						}
						break;
						
					default:
						;
				
				}
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}