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
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "commands.h"
#include "remoteMgmtTask.h"

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
extern BufferCntType		gTXCurBufferNum;
extern BufferCntType		gTXUsedBuffers;
gwUINT8			        	gAssocCheckCount = 0;


// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
// We further divide this by two since we average the cur and prev sample to smooth the waveform.
// Now defined in Events.c
//SampleRateType		gMasterSampleRate = 2000 / SAMPLE_SMOOTH_STEPS;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
    gwUINT8		    	ccrHolder;
	BufferCntType		rxBufferNum;
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
//						createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
//						if (transmitPacket(gTXCurBufferNum)){
//						};
//						gAssocCheckCount++;
//						if (gAssocCheckCount > 10) {
//							RESET_MCU;
//						}
//					}
					// Send an AssocCheck every 5 seconds.

				} else {
					// The last read got a packet, so we're active.
					gShouldSleep = FALSE;

					// We just received a valid packet.
					networkID = getNetworkID(rxBufferNum);

					// Only process packets sent to the broadcast network ID and our assigned network ID.
					if ((networkID != BROADCAST_NETID) && (networkID != gMyNetworkID)) {
						RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
						} else {
						cmdID = getCommandID(gRXRadioBuffer[rxBufferNum].bufferStorage);
						cmdDstAddr = getCommandDstAddr(rxBufferNum);

							// Only process commands sent to the broadcast address or our assigned address.
						if ((cmdDstAddr != ADDR_BROADCAST) && (cmdDstAddr != gMyAddr)) {
							RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
						} else {

							switch (cmdID) {

								case eCommandAssoc:
										// This will only return sub-commands if the command GUID matches out GUID
									assocSubCmd = getAssocSubCommand(rxBufferNum);
										if (assocSubCmd == eCmdAssocInvalid) {
											RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
										} else if (assocSubCmd == eCmdAssocRESP) {
											// Reset the clock on the assoc check.
											lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
											// If we're not already running then signal the mgmt task that we just got a command ASSOC resp.
											if (gLocalDeviceState != eLocalStateRun) {
										if (xQueueSend(gRemoteMgmtQueue, &rxBufferNum, (portTickType) 0)) {
										}
											}
									} else if (assocSubCmd == eCmdAssocACK) {
											// Record the time of the last ACK packet we received.
											gLastPacketReceivedTick = xTaskGetTickCount();

											gAssocCheckCount = 0;
										// If the associate state is 1 then we're not associated with this controller anymore.
										// We need to reset the device, so that we can start a whole new session.
											if (1 == gRXRadioBuffer[rxBufferNum].bufferStorage[CMDPOS_ASSOCACK_STATE]) {
											RESET_MCU;
									}
									RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
										}
									break;

								case eCommandInfo:
									// Now that the remote has an assigned address we need to ask it to describe
									// it's capabilities and characteristics.
									processQueryCommand(rxBufferNum, getCommandSrcAddr(rxBufferNum));
									break;

								case eCommandControl:
										// If the packet requires an ACK then send it now.
										ackId = getAckId(rxBufferNum);
										if (ackId != 0) {
											createAckCommand(gTXCurBufferNum, ackId);
											if (transmitPacket(gTXCurBufferNum)){
											}
										}

									// Make sure that there is a valid sub-command in the control command.
									switch (getControlSubCommand(rxBufferNum)) {
										case eControlSubCmdEndpointAdj:
											break;

	#if 0
										case eControlSubCmdMotor:
											processMotorControlSubCommand(rxBufferNum);
											break;
	#endif
											case eControlSubCmdHooBee:
												processHooBeeSubCommand(rxBufferNum);
												break;

										default:
											RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
									}
									break;

								case eCommandAudio:
									// Audio commands are handled by an interrupt routine.
									break;

								default:
									// Bogus command.
									// Immediately free this command buffer since we'll never do anything with it.
									RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
									break;

							}
						}
					}
				}
			}
					// If we're running then send an AssocCheck every 5 seconds.
					if (gLocalDeviceState == eLocalStateRun) {
						if (lastAssocCheckTickCount < xTaskGetTickCount()) {
							lastAssocCheckTickCount = xTaskGetTickCount() + kAssocCheckTickCount;
							createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
							if (transmitPacket(gTXCurBufferNum)){
							}
					gAssocCheckCount++;
					if (gAssocCheckCount > 10) {
					//	RESET_MCU;
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
