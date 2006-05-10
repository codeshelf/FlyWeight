/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "gatewayMgmtTask.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"

xQueueHandle			gGatewayMgmtQueue;
ControllerStateType		gControllerState;

// --------------------------------------------------------------------------

void gatewayMgmtTask( void *pvParameters ) {
	RemoteAddrType	slotNum;

	if ( gGatewayMgmtQueue ) {
		for ( ;; ) {

			// Whenever we need to handle a state change for a  device, we handle it in this management task.
			if (xQueueReceive(gGatewayMgmtQueue, &slotNum, portMAX_DELAY) == pdPASS) {
			
				// Get the state of the remote at the named slot.
				switch (gRemoteStateTable[slotNum].remoteState) {

					case eRemoteStateWakeRcvd:
						// Respond to the remote's WAKE command with an ASSIGN command letting it know which destination slot it has.
						createAssignCommand(gTXCurBufferNum, &(gRemoteStateTable[slotNum].remoteUniqueID), slotNum);
						if (transmitCommand(gTXCurBufferNum)) {
						};
						gRemoteStateTable[slotNum].remoteState = eRemoteStateAddrAssignSent;
						
						// Resignal the manager about the new state.
						if (xQueueSend(gGatewayMgmtQueue, &slotNum, pdFALSE)) {
						}
						break;
						
					case eRemoteStateAddrAssignSent:
						// Now that the remote has an assigned address we need to ask it to describe
						// it's capabilities and characteristics.
						createQueryCommand(gTXCurBufferNum, slotNum);
						if (transmitCommand(gTXCurBufferNum)) {
						};
						gRemoteStateTable[slotNum].remoteState = eRemoteStateQuerySent;

						// Resignal the manager about the new state.
						if (xQueueSend(gGatewayMgmtQueue, &slotNum, pdFALSE)) {
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