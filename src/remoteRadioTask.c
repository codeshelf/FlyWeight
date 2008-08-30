/*
   FlyWeight
   � Copyright 2005, 2006 Jeffrey B. Williams
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
#ifdef __WatchDog
	#include "WatchDog.h"
#endif

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
bool				gIsSleeping;
bool				gShouldSleep;
UINT8				gSleepCount;
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

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	byte				ccrHolder;
	BufferCntType		rxBufferNum;
	ECommandGroupIDType	cmdID;
	RemoteAddrType		cmdDstAddr;
	ECmdAssocType		assocSubCmd;

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
						
						__asm("STOP")
						
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
					
					// Every 10th time we wake from sleep send an AssocCheckCommand to the controller.
					gSleepCount++;
					if (gSleepCount >= 9) {
						gSleepCount = 0;
						createAssocCheckCommand(gTXCurBufferNum, (RemoteUniqueIDPtrType) GUID);
						if (transmitPacket(gTXCurBufferNum)){
						};
					}
								
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
									if (1 == gRXRadioBuffer[rxBufferNum].bufferStorage[CMDPOS_ASSOCASCK_STATE])
										__asm DCB 0x8D;
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
				}
			}
			
			// Blink LED2 to let us know we succeeded in receiving a packet buffer.
			//if (xQueueSend(gLEDBlinkQueue, &gLED2, (portTickType) 0)) {
			//}	
		}

		/* Will only get here if the queue could not be created. */
		for ( ;; );
	}
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	tTxPacket			gsTxPacket;
	BufferCntType		txBufferNum;
	//portTickType		lastTick;

	for (;;) {

		// Wait until the SCI controller signals us that we have a full buffer to transmit.
		if (xQueueReceive( gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

			gShouldSleep = FALSE;
			
			// Disable the RX to prepare for TX.
			MLMERXDisableRequest();
			
			vTaskSuspend(gRadioReceiveTask);
			
			gsTxPacket.pu8Data = gTXRadioBuffer[txBufferNum].bufferStorage;
			gsTxPacket.u8DataLength = gTXRadioBuffer[txBufferNum].bufferSize;
			MCPSDataRequest(&gsTxPacket);
			
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
		
		// Blink LED1 to let us know we succeeded in transmitting the buffer.
		//if (xQueueSend(gLEDBlinkQueue, &gLED1, (portTickType) 0)) {
		//}
	}
}