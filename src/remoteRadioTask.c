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
//#include "AudioOut.h"

// SMAC includes
#include "pub_def.h"
#include "PWM.h"

// --------------------------------------------------------------------------
// Local variables.

// The queue used to send data from the radio to the radio receive task.
static xQueueHandle gRadioTransmitQueue;
static xQueueHandle gRadioReceiveQueue;

tTxPacket	gsTxPacket;
tRxPacket	gsRxPacket;

extern UINT8 gLED1;
extern UINT8 gLED2;
extern UINT8 gLED3;
extern UINT8 gLED4;
//extern xQueueHandle xLEDBlinkQueue;


// Radio input buffer
// There's a 2-byte ID on the front of every packet.
RadioBufferStruct	gRadioBuffer[ASYNC_BUFFER_COUNT];
BufferCntType		gCurRadioBufferNum = 0;

// --------------------------------------------------------------------------

void vRadioReceiveTask(void *pvParameters) {
	ERadioState radioState;

	// Create the radio queue that will handle incoming radio packets.
	gRadioReceiveQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(ERadioState));
	initSMACRadioQueueGlue(gRadioReceiveQueue);

	if (gRadioReceiveQueue) {
	
		// Start the audio processing.
		//AudioOut_Enable();
		PWM_Init();
	
		for (;;) {

			// Setup for the next receive cycle.
			while (gRadioBuffer[gCurRadioBufferNum].bufferStatus == eBufferStateFull)
				;
			
			gsRxPacket.pu8Data = (UINT8 *) &(gRadioBuffer[gCurRadioBufferNum].bufferStorage);
			gsRxPacket.u8MaxDataLength = ASYNC_BUFFER_SIZE;
			gsRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gsRxPacket, 0L);
			
			// Wait until we receive a queue message from the radio receive ISR.
			if (xQueueReceive(gRadioReceiveQueue, &radioState, portTICK_RATE_MS * 500) == pdPASS) {
				
				if (radioState == eRadioReset) {
					// We just received a reset from the radio.
					
				} else if (radioState == eRadioReceive) {
					// We just received a valid packet.
					gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateFull;
					
					//if (xQueueSend( gRadioTransmitQueue, NULL, pdFALSE )) {}
					
					// We don't really do anything here since 
					// the PWM audio processor is working at interrupt
					// to get bytes out of the buffer.
					
					// Setup for the next receive cycle.
					if (gCurRadioBufferNum >= (ASYNC_BUFFER_COUNT - 1))
						gCurRadioBufferNum = 0;
					else
						gCurRadioBufferNum++;
						
				}
			}
			
/*			gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateEmpty;
			gsRxPacket.pu8Data = (UINT8 *) &(gRadioBuffer[gCurRadioBufferNum].bufferStorage);
			gsRxPacket.u8MaxDataLength = ASYNC_BUFFER_SIZE;
			gsRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gsRxPacket, 0L);
*/					
			// Blink LED2 to let us know we succeeded in transmitting the buffer.
//			if (xQueueSend(xLEDBlinkQueue, &gLED2, pdFALSE)) {
//			
//			}	
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

	gRadioTransmitQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));

	for ( ;; ) {
		gsTxPacket.u8DataLength = 4;    /* Initialize the gsTxPacket global */
		MCPSDataRequest(&gsTxPacket);

		MLMERXEnableRequest(&gsRxPacket, 0L);

		if (xQueueReceive(gRadioTransmitQueue, &msgP, portTICK_RATE_MS * 1000) == pdPASS) {
			vTaskDelay(portTICK_RATE_MS * 100);
		}

		MLMERXDisableRequest();
	}

}