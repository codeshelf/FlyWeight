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
#include "simple_mac.h"

xQueueHandle 		gRemoteMgmtQueue;
ELocalStatusType	gLocalDeviceState;
bool				gIsSleeping;
bool				gShouldSleep;
UINT8				gSleepCount;

// --------------------------------------------------------------------------

void remoteMgmtTask( void *pvParameters ) {
	BufferCntType		rxBufferNum = 0;
	ChannelNumberType	channel;
	bool				associated;
	UINT8				trim = 128;
	UINT8				assocAttempts = 0;

	if ( gRemoteMgmtQueue ) {
		
		/*
		 * Attempt to associate with our controller.
		 * 1. Send an associate request.
		 * 2. Wait up to 10ms for a response.
		 * 3. If we get a response then start the main proccessing
		 * 4. If no response then change channels and start at step 1.
		 */
		channel = 0;
		associated = FALSE;
		while (!associated) {
			// Set the channel to the current channel we're testing.
			MLMESetChannelRequest(channel);
			
			// Send an associate request on the current channel.
			createAssocReqCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
			if (transmitPacket(gTXCurBufferNum)) {
			};
			
			// Wait up to 100ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 100 * portTICK_RATE_MS) == pdPASS) {
				if (rxBufferNum != 255) {
					switch (gLocalDeviceState) {
						case eLocalStateAssocRespRcvd:
							processAssocRespCommand(rxBufferNum);
							if (gLocalDeviceState == eLocalStateAssociated) {
								associated = TRUE;
							}
							break;
					}
				}
			}
			
			// If we're still not associated then change channels.
			if (!associated) {
			//	MLMEMC13192XtalAdjust(trim--);
				channel++;
				if (channel > 16) {
					channel = 0;
					assocAttempts++;
					if (assocAttempts % 2) {
						sleepThisRemote(5);
					}
				}
				// Delay 100ms before changing channels.
				//vTaskDelay(250 * portTICK_RATE_MS);
			}
		}
		gLocalDeviceState = eLocalStateRun;
				
		vTaskSuspend(gRemoteManagementTask);
//		for ( ;; ) {
//
//			// Whenever we need to handle a state change for a  device, we handle it in this management task.
//			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
//			
//				// Get the state of the remote at the named slot.
//				switch (gLocalDeviceState) {
//
//					case eCommandAssoc:
//						processAssocRespCommand(rxBufferNum);
//						break;
//					
//					// If we're in the run state, and receive a command then we need to handle that command.
//					case eLocalStateRun:
//						
//						switch (getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage)) {
//						
//							case eCommandInfo:
//								// Now that the remote has an assigned address we need to ask it to describe
//								// it's capabilities and characteristics.
//								processQueryCommand(rxBufferNum, getCommandSrcAddr(rxBufferNum));
//								break;		
//								
//							case eCommandControl:
//								break;
//								
//							default:
//								break;
//						}
//						break;
//						
//					default:
//						RELEASE_RX_BUFFER(rxBufferNum);
//				
//				}
//			}
//		}

	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

void sleepThisRemote(UINT8 inSleepSeconds) {

	byte ccrHolder;
	int i;
	
	if (TRUE) {
		//vTaskDelay(250 * portTICK_RATE_MS);
		EnterCriticalArg(ccrHolder);
		gIsSleeping = TRUE;
		Cpu_SetSlowSpeed();
		MLMEHibernateRequest();
		//TPMIE_PWM = 0;
		TPMIE_AUDIO_LOADER = 0;
		SRTISC_RTICLKS = 0;
		//SRTISC_RTIS = 0;
		// Using the internal osc, "6" on RTIS will cause the MCU to sleep for 1024ms.
		SRTISC_RTIS = 6;
		ExitCriticalArg(ccrHolder);
		
		for (i = 0; i < inSleepSeconds; i++) {
		//	__asm("STOP");
			
			// If a KBI woke us up then don't keep sleeping.
			if (KBI1SC_KBF) {
				break;
			}
		}
		
		EnterCriticalArg(ccrHolder);
		gIsSleeping = FALSE;
		MLMEWakeRequest();
		// Wait for the MC13192's modem ClkO to warm up.
		Cpu_Delay100US(100);
		Cpu_SetHighSpeed();
		SRTISC_RTICLKS = 1;
		SRTISC_RTIS = 4;
		//TPMIE_PWM = 1;
		TPMIE_AUDIO_LOADER = 1;
		ExitCriticalArg(ccrHolder);
	}
}