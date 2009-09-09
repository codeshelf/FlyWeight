/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "remoteMgmtTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"
#include "simple_mac.h"

#define				RESET_MCU				__asm DCB 0x8D

xQueueHandle 			gRemoteMgmtQueue;
ELocalStatusType		gLocalDeviceState;
bool					gIsSleeping;
bool					gShouldSleep;
UINT8					gSleepCount;
extern int				gAssocCheckCount;
extern byte				gCCRHolder;

// --------------------------------------------------------------------------

void remoteMgmtTask( void *pvParameters ) {
	BufferCntType		rxBufferNum = 0;
	ChannelNumberType	channel;
	bool				associated;
	UINT8				trim = 128;
	UINT8				assocAttempts = 0;
	ECommandGroupIDType	cmdID;
	ECmdAssocType		assocSubCmd;

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

		//WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			MLMESetChannelRequest(channel);
			
			// Send an associate request on the current channel.
			createAssocReqCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
			if (transmitPacket(gTXCurBufferNum)) {
			};
			
			// Wait up to 200ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 200 * portTICK_RATE_MS) == pdPASS) {
				if (rxBufferNum != 255) {
					// Check to see what kind of command we just got.
					cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
					if (cmdID == eCommandAssoc) {
						assocSubCmd = getAssocSubCommand(rxBufferNum);
						if (assocSubCmd == eCmdAssocRESP) {
							gLocalDeviceState = eLocalStateAssocRespRcvd;
							processAssocRespCommand(rxBufferNum);
							if (gLocalDeviceState == eLocalStateAssociated) {
								associated = TRUE;
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum);
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
//			// Periodically send a NetCheck packet to the controller.
//			vTaskDelay(2500 * portTICK_RATE_MS);
//			if (gLocalDeviceState == eLocalStateRun) {
//				createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
//				if (transmitPacket(gTXCurBufferNum)){
//				};
//				gAssocCheckCount++;
//				if (gAssocCheckCount > 10) {
//					RESET_MCU;
//				}
//			}
//		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// Define this if the MCU should really sleep (hibernate in FSL parlance).
//#define REAL_SLEEP

void sleepThisRemote(UINT8 inSleepMillis) {

#ifdef REAL_SLEEP
	byte ccrHolder;
	int i;
	UINT8 saveLoaderState;
	UINT8 sleepCycles = inSleepMillis >> 6;  // Divide by SRTICS_RTIS value below.	
#endif

#ifndef REAL_SLEEP
		// "Fake sleep" for HooBeez
		vTaskDelay(inSleepMillis * portTICK_RATE_MS);
#else
		// Real sleep for RocketPhones
		EnterCriticalArg(ccrHolder);
		gIsSleeping = TRUE;
		Cpu_SetSlowSpeed();
		MLMEHibernateRequest();
		//TPMIE_PWM = 0;
		saveLoaderState = TPMIE_AUDIO_LOADER;
		TPMIE_AUDIO_LOADER = 0;
		SRTISC_RTICLKS = 0;
		//SRTISC_RTIS = 0;
		// Using the internal osc, "6" on RTIS will cause the MCU to sleep for 1024ms.
		SRTISC_RTIS = 2;// 1 = 8ms, 2 = 32ms, 3 = 64, 4 = 128, 5 = 256, 6 = 1024ms;
		ExitCriticalArg(ccrHolder);
		
		for (i = 0; i < sleepCycles; i++) {
			__asm("STOP");
			
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
		TPMIE_AUDIO_LOADER = saveLoaderState;
		ExitCriticalArg(ccrHolder);
#endif
}
