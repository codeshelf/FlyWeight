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

// SMAC includes
#include "pub_def.h"

// --------------------------------------------------------------------------
// Local variables.

// The queue used to send data from the radio to the radio receive task.
static xQueueHandle gRadioTransmitQueue;
static xQueueHandle gRadioReceiveQueue;

tTxPacket	gsTxPacket;
tRxPacket	gsRxPacket;
UINT8		gau8TxDataBuffer[16] = APPNAME;
UINT8		gau8RxDataBuffer[16];

// Radio input buffer
RadioBufferStruct	gRadioBuffer[BUFFER_COUNT];
BufferCntType		gCurRadioBufferNum = 0;
BufferCntType		gCurPWMRadioBufferNum = 0;
BufferOffsetType	gCurPWMOffset = 0;

// --------------------------------------------------------------------------

void vRadioReceiveTask(void *pvParameters) {
	ERadioState radioState;

	// Create the radio queue that will handle incoming radio packets.
	gRadioReceiveQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));
	initSMACRadioQueueGlue(gRadioReceiveQueue);

	if (gRadioReceiveQueue) {
		for (;;) {

			// Setup for the first receive cycle.
			gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateEmpty;
			gsRxPacket.pu8Data = (UINT8 *) &(gRadioBuffer[gCurRadioBufferNum].bufferStorage);
			gsRxPacket.u8MaxDataLength = BUFFER_SIZE;
			gsRxPacket.u8Status = 0;
			MLMERXEnableRequest(&gsRxPacket, 0L);
			
			// Wait until we receive a queue message from the radio receive ISR.
			if (xQueueReceive(gRadioReceiveQueue, &radioState, portMAX_DELAY) == pdPASS) {
				
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
					gCurRadioBufferNum++;
					if (gCurRadioBufferNum > (BUFFER_COUNT - 1))
						gCurRadioBufferNum = 0;
						
					gRadioBuffer[gCurRadioBufferNum].bufferStatus = eBufferStateEmpty;
					gsRxPacket.pu8Data = (UINT8 *) &(gRadioBuffer[gCurRadioBufferNum].bufferStorage);
					gsRxPacket.u8MaxDataLength = BUFFER_SIZE;
					gsRxPacket.u8Status = 0;
					MLMERXEnableRequest(&gsRxPacket, 0L);
				}
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

// --------------------------------------------------------------------------

void vRadioTransmitTask(void *pvParameters) {
	byte		msgP = 0;

	gsRxPacket.u8MaxDataLength = 0;
	gsRxPacket.pu8Data = &gau8RxDataBuffer[0];
	gsRxPacket.u8MaxDataLength = 16;
	gsRxPacket.u8Status = 0;

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