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

// SMAC includes
#include "pub_def.h"
//#include "PWM1.h"
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
//extern xQueueHandle xLEDBlinkQueue;


// Radio input buffer
// There's a 2-byte ID on the front of every packet.
RadioBufferStruct	gRadioBuffer[ASYNC_BUFFER_COUNT];
BufferCntType		gCurRadioBufferNum = 0;
BufferCntType		gUsedBuffers;

// --------------------------------------------------------------------------

void vRadioReceiveTask(void *pvParameters) {
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
			;//vTaskDelay(2);
		
		// Setup for the next receive cycle.
		lastTick = xTaskGetTickCount();
		while (gRadioBuffer[gCurRadioBufferNum].bufferStatus == eBufferStateFull)
			;//vTaskDelay(2);
		//USB_SendChar(xTaskGetTickCount() - lastTick);
		
		//USB_SendChar(gCurRadioBufferNum);
		gsRxPacket.pu8Data = (UINT8 *) &(gRadioBuffer[gCurRadioBufferNum].bufferStorage);
		gsRxPacket.u8MaxDataLength = ASYNC_BUFFER_SIZE;
		gsRxPacket.u8Status = 0;
		MLMERXEnableRequest(&gsRxPacket, 0L);
		
		//USB_SendChar(xTaskGetTickCount() - lastTick);
		//lastTick = xTaskGetTickCount();
		
		// Wait until we receive a queue message from the radio receive ISR.
		if (xQueueReceive(gRadioReceiveQueue, &radioState, portTICK_RATE_MS * 500) == pdPASS) {
			
			//USB_SendChar(gCurRadioBufferNum);
			if (radioState == eRadioReset) {
				// We just received a reset from the radio.
				
			} else if (radioState == eRadioReceive) {
				
				// We just received a valid packet.
				// We don't really do anything here since 
				// the PWM audio processor is working at interrupt
				// to get bytes out of the buffer.
								
				// The buffers are a shared, critical resource, so we have to protect them before we update.
				EnterCritical();
				
					gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateFull;
					
					// Advance to the next buffer.
					if (gCurRadioBufferNum >= (ASYNC_BUFFER_COUNT - 1))
						gCurRadioBufferNum = 0;
					else
						gCurRadioBufferNum++;
					
					// Account for the number of used buffers.
					if (gUsedBuffers < ASYNC_BUFFER_COUNT)
						gUsedBuffers++;
					
				ExitCritical();
				
			}
		}
		
		// Blink LED2 to let us know we succeeded in receiving a packet buffer.
		if (xQueueSend(xLEDBlinkQueue, &gLED2, pdFALSE)) {
		
		}	
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vRadioTransmitTask(void *pvParameters) {
	byte		msgP = 0;
	UINT8		gau8TxDataBuffer[16];
	
	gsTxPacket.u8DataLength = 0;
	gsTxPacket.pu8Data = &gau8TxDataBuffer[0]; /* Set the pointer to point to the tx_buffer */

	for ( ;; ) {

		//WatchDog_Clear();

		gsTxPacket.u8DataLength = 4;    /* Initialize the gsTxPacket global */
		MCPSDataRequest(&gsTxPacket);

		MLMERXEnableRequest(&gsRxPacket, 0L);

		if (xQueueReceive(gRadioTransmitQueue, &msgP, portTICK_RATE_MS * 1000) == pdPASS) {
			vTaskDelay(portTICK_RATE_MS * 100);
		}

		MLMERXDisableRequest();
	}

}