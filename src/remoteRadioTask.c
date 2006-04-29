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
#include "ledBlinkTask.h"
//#include "WatchDog.h"
//#include "AudioLoader.h"
//#include "USB.h"
#include "commands.h"

// SMAC includes
#include "pub_def.h"
#include "PWM.h"

// --------------------------------------------------------------------------
// Local variables.

// The queue used to send data from the radio to the radio receive task.
xQueueHandle gRadioTransmitQueue = NULL;
xQueueHandle gRadioReceiveQueue = NULL;

tTxPacket	gsTxPacket;
tRxPacket	gsRxPacket;

extern UINT8 gLED1;
extern UINT8 gLED2;
extern UINT8 gLED3;
extern UINT8 gLED4;
extern UINT8 gu8RTxMode;

extern xQueueHandle 	gLEDBlinkQueue;
extern RemoteStateType	gRemoteState;


// Radio input buffer
// There's a 2-byte ID on the front of every packet.
RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gRXCurBufferNum = 0;
BufferCntType		gRXUsedBuffers = 0;

RadioBufferStruct	gTXRadioBuffer[RX_BUFFER_COUNT];
BufferCntType		gTXCurRadioBufferNum = 0;
BufferCntType		gTXUsedBuffers = 0;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	ERadioState		radioState;
	portTickType	lastTick;

	// Start the audio processing.
	//AudioLoader_Enable();
	
	// Setup the TPM2 timer.
	TPM2SC = 0b01001000;
	// 16MHz bus clock with a 7.4kHz interrupt freq.
	TPM2MOD = MASTER_TPM2_RATE;	
	
	//PWM1_Enable();
	PWM_Init();

	for (;;) {

		//WatchDog_Clear();
		
		// Wait until the radio is ready.
		while (gu8RTxMode != IDLE_MODE)
			vTaskDelay(2);
		
		// Setup for the next receive cycle.
		lastTick = xTaskGetTickCount();
		while (gRXBuffer[gRXCurBufferNum].bufferStatus == eBufferStateInUse)
			vTaskDelay(2);

		// We have to wait here until we're in a mode that requires receiving.
		while ((gRemoteState != eRemoteStateWakeSent) && (gRemoteState != eRemoteStateRun))
			vTaskDelay(2);
		
		gsRxPacket.pu8Data = (UINT8 *) &(gRXBuffer[gRXCurBufferNum].bufferStorage);
		gsRxPacket.u8MaxDataLength = RX_BUFFER_SIZE;
		gsRxPacket.u8Status = 0;
		MLMERXEnableRequest(&gsRxPacket, 0L);
		
		//USB_SendChar(xTaskGetTickCount() - lastTick);
		//lastTick = xTaskGetTickCount();
		
		// Wait until we receive a queue message from the radio receive ISR.
		if (xQueueReceive(gRadioReceiveQueue, &radioState, portTICK_RATE_MS * 500) == pdPASS) {
			
			if (radioState == eRadioReset) {
				// We just received a reset from the radio.
				
			} else if (radioState == eRadioReceive) {
				
				// We just received a valid packet.
				// We don't really do anything here since 
				// the PWM audio processor is working at interrupt
				// to get bytes out of the buffer.
								
				// The buffers are a shared, critical resource, so we have to protect them before we update.
				EnterCritical();
				
					gRXBuffer[gRXCurBufferNum].bufferStatus = eBufferStateInUse;
					
					// Advance to the next buffer.
					if (gRXCurBufferNum >= (RX_BUFFER_COUNT - 1))
						gRXCurBufferNum = 0;
					else
						gRXCurBufferNum++;
					
					// Account for the number of used buffers.
					if (gRXUsedBuffers < RX_BUFFER_COUNT)
						gRXUsedBuffers++;
					
				ExitCritical();
				
			}
		}
		
		// Blink LED2 to let us know we succeeded in receiving a packet buffer.
		if (xQueueSend(gLEDBlinkQueue, &gLED2, pdFALSE)) {
		
		}	
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	byte			msgP = 0;
	UINT8			gau8TxDataBuffer[16];
	CommandPtrType	wakeCommand;
	
	gsTxPacket.u8DataLength = 0;
	gsTxPacket.pu8Data = &gau8TxDataBuffer[0]; /* Set the pointer to point to the tx_buffer */

	for ( ;; ) {

		//WatchDog_Clear();
		
		// If we're in the init mode then we need to transmit a wake command.
		if (gRemoteState == eRemoteStateInit) {

			wakeCommand = createWakeCommand();
			transmitCommand(wakeCommand);
			
			gsTxPacket.u8DataLength = 4;    /* Initialize the gsTxPacket global */
			MCPSDataRequest(&gsTxPacket);
			
			gRemoteState == eRemoteStateWakeSent;

		}
	}

}