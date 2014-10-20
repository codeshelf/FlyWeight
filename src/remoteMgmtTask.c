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
#include "gwSystemMacros.h"
#include "gwTypes.h"
#include "commands.h"
#include "TransceiverPowerMngmnt.h"

xQueueHandle gRemoteMgmtQueue;
ELocalStatusType gLocalDeviceState;
gwBoolean gIsSleeping;
gwBoolean gShouldSleep;
gwUINT8 gSleepCount;
extern gwUINT8 gAssocCheckCount;
extern gwUINT8 gCCRHolder;
void preSleep();

// --------------------------------------------------------------------------

void remoteMgmtTask(void *pvParameters) {
	gwUINT8 ccrHolder;
	BufferCntType rxBufferNum = 0;
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	gwBoolean associated;
	gwBoolean checked;
	gwUINT8 trim = 128;
	gwUINT8 assocAttempts = 0;
	ECommandGroupIDType cmdID;
	ECmdAssocType assocSubCmd;

	if (gRemoteMgmtQueue) {

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

			GW_WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			MLMESetChannelRequest(channel);

			// Send an associate request on the current channel.
			txBufferNum = lockTXBuffer();
			createAssocReqCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
			if (transmitPacket(txBufferNum)) {
			};

			// Wait up to 1000ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 50 * portTICK_RATE_MS) == pdPASS) {
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
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
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
						//sleep(5);
					}
				}
				// Delay 100ms before changing channels.
				//vTaskDelay(250 * portTICK_RATE_MS);
			}
		}
		gLocalDeviceState = eLocalStateRun;

		checked = FALSE;
		for (;;) {

			if (!checked) {
				BufferCntType txBufferNum = lockTXBuffer();
				createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
				if (transmitPacket(txBufferNum)) {
				}
			}

			// Wait up to 1000ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 1000 * portTICK_RATE_MS) == pdPASS) {
				if (rxBufferNum != 255) {
					// Check to see what kind of command we just got.
					cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
					if (cmdID == eCommandAssoc) {
						assocSubCmd = getAssocSubCommand(rxBufferNum);
						if (assocSubCmd == eCmdAssocACK) {
							checked = TRUE;
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			}
			vTaskDelay(50);
		}
		//vTaskSuspend(gRemoteManagementTask);
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}

void sleep() {
	crmSleepCtrl_t sleepCtl;
	gwUINT8 ccrHolder;
	FuncReturn_t funcRet;

	sleepCtl.sleepType = gHibernate_c;
	sleepCtl.mcuRet = gMcuRet_c;
	sleepCtl.ramRet = gRamRet96k_c;
	sleepCtl.digPadRet = 1;
	sleepCtl.pfToDoBeforeSleep = &preSleep;

	GW_ENTER_CRITICAL(ccrHolder);

	TmrSetMode(gTmr0_c, gTmrNoOperation_c);
	ITC_DisableInterrupt(gTmrInt_c);

//	CRM_GoToSleep(&sleepCtl);
	funcRet = MLMEHibernateRequest(gRingOsc2khz_c, sleepCtl);
	GW_RESET_MCU();

	GW_EXIT_CRITICAL(ccrHolder);
}

// Define this if the MCU should really sleep (hibernate in FSL parlance).
//#define REAL_SLEEP

//void sleep(gwUINT8 inSleepMillis) {
//
//#ifdef REAL_SLEEP
//	gwUINT8 ccrHolder;
//	gwUINT8 i;
//	gwUINT8 saveLoaderState;
//	gwUINT8 sleepCycles = inSleepMillis >> 6;  // Divide by SRTICS_RTIS value below.
//#endif
//
//#ifndef REAL_SLEEP
//	// "Fake sleep" for HooBeez
//	vTaskDelay(inSleepMillis * portTICK_RATE_MS);
//#else
//	// Real sleep for RocketPhones
//	EnterCriticalArg(ccrHolder);
//	gIsSleeping = TRUE;
//	Cpu_SetSlowSpeed();
//	MLMEHibernateRequest();
//	//TPMIE_PWM = 0;
//	saveLoaderState = TPMIE_AUDIO_LOADER;
//	TPMIE_AUDIO_LOADER = 0;
//	SRTISC_RTICLKS = 0;
//	//SRTISC_RTIS = 0;
//	// Using the internal osc, "6" on RTIS will cause the MCU to sleep for 1024ms.
//	SRTISC_RTIS = 2;// 1 = 8ms, 2 = 32ms, 3 = 64, 4 = 128, 5 = 256, 6 = 1024ms;
//	ExitCriticalArg(ccrHolder);
//
//	for (i = 0; i < sleepCycles; i++) {
//		__asm("STOP");
//
//		// If a KBI woke us up then don't keep sleeping.
//		if (KBI1SC_KBF) {
//			break;
//		}
//	}
//
//	EnterCriticalArg(ccrHolder);
//	gIsSleeping = FALSE;
//	MLMEWakeRequest();
//	// Wait for the MC13192's modem ClkO to warm up.
//	Cpu_Delay100US(100);
//	Cpu_SetHighSpeed();
//	SRTISC_RTICLKS = 1;
//	SRTISC_RTIS = 4;
//	//TPMIE_PWM = 1;
//	TPMIE_AUDIO_LOADER = saveLoaderState;
//	ExitCriticalArg(ccrHolder);
//#endif
//}
