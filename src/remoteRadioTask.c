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
#include "simple_mac.h"
#include "commands.h"
#include "remoteMgmtTask.h"
#include "CPU.h"
#include "WatchDog.h"

// SMAC includes
#include "pub_def.h"
#if defined(XBEE)
	#include "PWM_XBee.h"
#elif defined(MC1321X)
	#include "PWM_MC1321X.h"
#elif defined(MC13192EVB)
	#include "PWM_EVB.h"
#endif

// --------------------------------------------------------------------------
// Global variables.

UINT8 				gu8RTxMode;
extern bool			gIsSleeping;
extern bool			gShouldSleep;
extern UINT8		gSleepCount;
extern UINT8		gButtonPressed;
extern byte			gCCRHolder;

xTaskHandle			gRadioReceiveTask = NULL;
xTaskHandle			gRadioTransmitTask = NULL;
xTaskHandle			gRemoteManagementTask = NULL;


// The queue used to send data from the radio to the radio receive task.
xQueueHandle		gRadioTransmitQueue = NULL;
xQueueHandle		gRadioReceiveQueue = NULL;

tTxPacket			gsTxPacket;
tRxPacket			gsRxPacket;

// Radio input buffer
// There's a 2-byte ID on the front of every packet.
RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gRXCurBufferNum = 0;
BufferCntType		gRXUsedBuffers = 0;

RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
BufferCntType		gTXCurBufferNum = 0;
BufferCntType		gTXUsedBuffers = 0;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
// We further divide this by two since we average the cur and prev sample to smooth the waveform.
// Now defined in Events.c
//SampleRateType		gMasterSampleRate = 2000 / SAMPLE_SMOOTH_STEPS;

#define				kAssocCheckTickCount	5000 //5 * portTICK_RATE_MS * 1000;
#define				RESET_MCU				__asm DCB 0x8D

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType		rxBufferNum;
	ECommandGroupIDType	cmdID;
	RemoteAddrType		cmdDstAddr;
	ECmdAssocType		assocSubCmd;
	portTickType		lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;

	// Start the audio processing.
	//AudioLoader_Enable();
	
	// The radio receive task will return a pointer to a radio data packet.
	if ( gRadioReceiveQueue ) {
	
		gSleepCount = 0;
	
		// Setup the TPM2 timer.
		TPMSC_AUDIO_LOADER = 0b01001000;
		// 16MHz bus clock with a 7.4kHz interrupt freq.
		TPMMOD_AUDIO_LOADER = gMasterSampleRate;	
		
		//PWM1_Enable();
#if defined(XBEE)
		PWM_XBee_Init();
#elif defined(MC1321X)
		PWM_MC1321X_Init();
#else
		PWM_EVB_Init();
#endif

		for (;;) {

#ifdef __WatchDog
			WatchDog_Clear();
#endif
			
			// Don't try to RX if there is no free buffer.
			while (gRXRadioBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse)
				vTaskDelay(1 * portTICK_RATE_MS);

			// Don't try to set up an RX unless we're already done with TX.
			while (gu8RTxMode == TX_MODE)
				vTaskDelay(1 * portTICK_RATE_MS);
			
			// Setup for the next RX cycle.
			if (gu8RTxMode != RX_MODE) {
				gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
				gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
				gsRxPacket.u8Status = 0;

				if (gShouldSleep) {
					MLMERXEnableRequest(&gsRxPacket, (UINT32) 25 * SMAC_TICKS_PER_MS);
				} else {
					MLMERXEnableRequest(&gsRxPacket, (UINT32) 1000 * SMAC_TICKS_PER_MS);
				}
			}

			// Wait until we receive a queue message from the radio receive ISR.
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
				
				if ((rxBufferNum == 255) && (!gButtonPressed)) {
				
					if (!gShouldSleep) {
						gShouldSleep = TRUE;
					} else {
						// We didn't get any packets before the RX timeout.  This is probably a quiet period, so pause for a while.
						sleepThisRemote(1);
					}
					
					// Every 10th time we wake from sleep send an AssocCheckCommand to the controller.
					gSleepCount++;
					if (gSleepCount >= 9) {
						gSleepCount = 0;
						lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
						createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
						if (transmitPacket(gTXCurBufferNum)){
						};
					}
					// Send an AssocCheck every 5 seconds.
								
				} else {
					// The last read got a packet, so we're active.
					gShouldSleep = FALSE;
					
					// We just received a valid packet.
					// We don't really do anything here since 
					// the PWM audio processor is working at interrupt
					// to get bytes out of the buffer.
					
					cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
					cmdDstAddr = getCommandDstAddr(rxBufferNum);
					
					// Only process broadcast commands or commands addressed to us.
					if ((cmdDstAddr != ADDR_BROADCAST) && (cmdDstAddr != gMyAddr)) {
						RELEASE_RX_BUFFER(rxBufferNum);
					} else {
					
						switch (cmdID) {
						
							case eCommandAssoc:
								assocSubCmd = getAssocSubCommand(rxBufferNum);
								if (assocSubCmd == eCmdAssocRESP) {
									
									gLocalDeviceState = eLocalStateAssocRespRcvd;
									// Signal the manager about the new state.
									if (xQueueSend(gRemoteMgmtQueue, &rxBufferNum, (portTickType) 0)) {
									}
								} else if (assocSubCmd == eCmdAssocACK) {
									// If the associate state is 1 then we're not associated with this controller anymore.
									// We need to reset the device, so that we can start a whole new session.
									if (1 == gRXRadioBuffer[rxBufferNum].bufferStorage[CMDPOS_ASSOCACK_STATE])
										RESET_MCU;
								}
								RELEASE_RX_BUFFER(rxBufferNum);
								break;
								
							case eCommandInfo:
								// Now that the remote has an assigned address we need to ask it to describe
								// it's capabilities and characteristics.
								processQueryCommand(rxBufferNum, getCommandSrcAddr(rxBufferNum));
								break;
								
							case eCommandControl:
								// Make sure that there is a valid sub-command in the control command.
								switch (getControlSubCommand(rxBufferNum)) {
									case eControlSubCmdEndpointAdj:
										break;
										
									case eControlSubCmdMotor:
										processMotorControlSubCommand(rxBufferNum);
										break;
										
									default:
										RELEASE_RX_BUFFER(rxBufferNum);
								}
								break;
								
							case eCommandAudio:
								// Audio commands are handled by an interrupt routine.
								break;
														
							default:
								// Bogus command.
								// Immediately free this command buffer since we'll never do anything with it.
								RELEASE_RX_BUFFER(rxBufferNum);
								break;
							
						}
					}
					// If we're running then send an AssocCheck every 5 seconds.
					if (gLocalDeviceState == eLocalStateRun) {
						if (lastAssocCheckTickCount < xTaskGetTickCount()) {
							lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
							createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
							if (transmitPacket(gTXCurBufferNum)){
							}
						}
					}
				}
			}
		}

		/* Will only get here if the queue could not be created. */
		for ( ;; );
	}
}

// --------------------------------------------------------------------------

extern bool					gAudioModeRX;
extern SampleRateType		gMasterSampleRate;

void radioTransmitTask(void *pvParameters) {
	tTxPacket			gsTxPacket;
	BufferCntType		txBufferNum;
	//portTickType		lastTick;

	for (;;) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

			gShouldSleep = FALSE;
			
#ifdef __WatchDog
			WatchDog_Clear();
#endif

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
				gsRxPacket.pu8Data = (UINT8 *) &(gRXRadioBuffer[gRXCurBufferNum].bufferStorage);
				gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
				gsRxPacket.u8Status = 0;
				MLMERXEnableRequest(&gsRxPacket, (UINT32) 1000 * SMAC_TICKS_PER_MS);
			}
						
			// Set the status of the TX buffer to free.
			RELEASE_TX_BUFFER(txBufferNum);	
			
			vTaskResume(gRadioReceiveTask);	
			
		} else {
			
		}
	}
}