/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved

   $Id$
   $Name$
 */


#include "remoteRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "remoteMgmtTask.h"
#include "remoteCommon.h"

#if defined(XBEE_PINOUT)
#include "LED_XBee.h"
#elif defined(MC1321X)
#include "PWM_MC1321X.h"
#elif defined(MC13192EVB)
#include "PWM_EVB.h"
#endif

// --------------------------------------------------------------------------
// Global variables.

gwUINT8 			gu8RTxMode;
extern gwBoolean	gIsSleeping;
extern gwBoolean	gShouldSleep;
extern gwUINT8		gSleepCount;
extern gwUINT8		gButtonPressed;
extern gwUINT8		gCCRHolder;
extern portTickType	gLastPacketReceivedTick;

xTaskHandle			gRadioReceiveTask = NULL;
xTaskHandle			gRadioTransmitTask = NULL;
xTaskHandle			gRemoteManagementTask = NULL;


// The queue used to send data from the radio to the radio receive task.
xQueueHandle		gRadioTransmitQueue = NULL;
xQueueHandle		gRadioReceiveQueue = NULL;

gwTxPacket			gTxPacket;
gwRxPacket			gRxPacket;

// Radio input buffer
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRXCurBufferNum;
extern BufferCntType		gRXUsedBuffers;

extern RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType		gTXUsedBuffers;
gwUINT8 gu8RTxMode;
extern gwBoolean gIsSleeping;
extern gwBoolean gShouldSleep;
extern gwUINT8 gSleepCount;
extern gwUINT8 gButtonPressed;
extern gwUINT8 gCCRHolder;
gwUINT8			        	gAssocCheckCount = 0;

// Radio buffers
// There's a 2-byte ID on the front of every packet.
extern RadioBufferStruct gRXRadioBuffer[RX_BUFFER_COUNT];
extern RadioBufferStruct gTXRadioBuffer[TX_BUFFER_COUNT];

portTickType gLastAssocCheckTickCount;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
// We further divide this by two since we average the cur and prev sample to smooth the waveform.
// Now defined in Events.c
//SampleRateType		gMasterSampleRate = 2000 / SAMPLE_SMOOTH_STEPS;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	gwUINT8		    	ccrHolder;
	BufferCntType		rxBufferNum;
	BufferCntType 		txBufferNum;
	ECommandGroupIDType	cmdID;
	NetAddrType			cmdDstAddr;
	NetworkIDType		networkID;
	ECmdAssocType		assocSubCmd;
	portTickType		lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
	AckIDType			ackId;
	portTickType		rxDelay;

	// Start the audio processing.
	//AudioLoader_Enable();

	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {

		gSleepCount = 0;

#if !defined(XBEE_PINOUT) && !defined(MC1322X)
		// Setup the TPM2 timer.
		TPMSC_AUDIO_LOADER = 0b01001000;
		// 16MHz bus clock with a 7.4kHz interrupt freq.
		TPMMOD_AUDIO_LOADER = gMasterSampleRate;
#endif

		for (;;) {

			// Wait until we receive a queue message from the radio receive ISR.
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, 50 * portTICK_RATE_MS) == pdPASS) {

				//WATCHDOG_RESET;

				// Don't try to RX if there is no free buffer.
				while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse)
					vTaskDelay(1 * portTICK_RATE_MS);

				// Don't try to set up an RX unless we're already done with TX.
				while (gu8RTxMode == TX_MODE)
					vTaskDelay(1 * portTICK_RATE_MS);

				// Setup for the next RX cycle.
				if (gu8RTxMode != RX_MODE) {
					gRxPacket.pu8Data = (gwUINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
					gRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
					gRxPacket.u8Status = 0;

					/*	if (gShouldSleep) {
					rxDelay = 25 * SMAC_TICKS_PER_MS;
					//MLMERXEnableRequest(&gsRxPacket, (UINT32) 25 * SMAC_TICKS_PER_MS);
				} else {
					rxDelay = 1000 * SMAC_TICKS_PER_MS;
					//MLMERXEnableRequest(&gsRxPacket, (UINT32) 1000 * SMAC_TICKS_PER_MS);
				}
				rxDelay = 50; */
					MLMERXEnableRequest(&gRxPacket, (UINT32) 1000 * SMAC_TICKS_PER_MS);
				}

				if ((rxBufferNum == 255) && (!gButtonPressed)) {

					if (!gShouldSleep) {
						// We're not sleeping for now until we figure out a better way.
						//gShouldSleep = TRUE;
					} else {
						// We didn't get any packets before the RX timeout.  This is probably a quiet period, so pause for a while.
						sleepThisRemote(50);
					}

					// Every 10th time we wake from sleep send an AssocCheckCommand to the controller.
					//					gSleepCount++;
					//					if (gSleepCount >= 9) {
					//						gSleepCount = 0;
					//						lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
					//						BufferCntType txBufferNum = lockTXBuffer();
					//						createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
					//						if (transmitPacket(txBufferNum)){
					//						};
					//						gAssocCheckCount++;
					//						if (gAssocCheckCount > 10) {
					//							RESET_MCU;
					//						}
					//					}
					// Send an AssocCheck every 5 seconds.

				} else {
					processRxPacket(rxBufferNum);
				}
			}
			// If we're running then send an AssocCheck every 5 seconds.
			if (gLocalDeviceState == eLocalStateRun) {
				if (lastAssocCheckTickCount < xTaskGetTickCount()) {
					lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
					txBufferNum = lockTXBuffer();
					createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) GUID);
					if (transmitPacket(txBufferNum)){
					}
					gAssocCheckCount++;
					if (gAssocCheckCount > 10) {
						//	GW_RESET_MCU;
					}
				}
			}
		}

		/* Will only get here if the queue could not be created. */
		for ( ;; );
	}
}

// --------------------------------------------------------------------------

extern gwBoolean			gAudioModeRX;
extern SampleRateType		gMasterSampleRate;

void radioTransmitTask(void *pvParameters) {
	gwUINT8		    	ccrHolder;
	gwTxPacket			gsTxPacket;
	BufferCntType		txBufferNum;
	//portTickType		lastTick;

	for (;;) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

			gShouldSleep = FALSE;

			//WATCHDOG_RESET;

			// Disable the RX to prepare for TX.
			MLMERXDisableRequest();

			vTaskSuspend(gRadioReceiveTask);

			gsTxPacket.pu8Data = gTXRadioBuffer[txBufferNum].bufferStorage;
			gsTxPacket.u8DataLength = gTXRadioBuffer[txBufferNum].bufferSize;

			//if (!gAudioModeRX)
			//	TPMIE_AUDIO_LOADER = 0;

			MCPSDataRequest(&gsTxPacket);

			//if (!gAudioModeRX)
			//	TPMIE_AUDIO_LOADER = 1;

			// Prepare to RX responses.
			// Don't go into RX mode if the last thing we sent was an audio packet.
			if (!(getCommandID(gTXRadioBuffer[txBufferNum].bufferStorage) == eCommandAudio)) {
				gRxPacket.pu8Data = (gwUINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
				gRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
				gRxPacket.u8Status = 0;
				MLMERXEnableRequest(&gRxPacket, (UINT32) 1000 * SMAC_TICKS_PER_MS);
			}

			// Set the status of the TX buffer to free.
			RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

			vTaskResume(gRadioReceiveTask);

		} else {

		}
	}
}
