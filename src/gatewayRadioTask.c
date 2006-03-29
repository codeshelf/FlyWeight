/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#include "gatewayRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "simple_mac.h"
#include "SW1Int.h"

// SMAC includes
#include "pub_def.h"


// Local variables.

/* The queue used to send data from the radio to the radio receive task. */
static xQueueHandle xRadioTransmitQueue;
static xQueueHandle xRadioReceiveQueue;

UINT8 gu8RTxMode;
extern UINT8 gLED1;
extern UINT8 gLED2;
extern UINT8 gLED3;
extern UINT8 gLED4;
extern xQueueHandle xLEDBlinkQueue;


// --------------------------------------------------------------------------

void vRadioTransmitTask( void *pvParameters ) {
	tTxPacket gsTxPacket;
	tRxPacket gsRxPacket;
	UINT8 gau8TxDataBuffer[16] = APPNAME;
	UINT8 gau8RxDataBuffer[16];

	byte	*msgP;

	gsRxPacket.u8MaxDataLength = 0;
	gsRxPacket.pu8Data = &gau8RxDataBuffer[0];
	gsRxPacket.u8MaxDataLength = 16;    /* Arbitrary, bigger than xXyYzZ format. */
	gsRxPacket.u8Status = 0;                /* initialize status to 0. */

	gsTxPacket.pu8Data = &gau8TxDataBuffer[0]; /* Set the pointer to point to the tx_buffer */

	xRadioTransmitQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));

	for (;;) {
		gsTxPacket.u8DataLength = 4;    /* Initialize the gsTxPacket global */
		MCPSDataRequest(&gsTxPacket);

		if (xQueueSend(xLEDBlinkQueue, &gLED1, pdFALSE)) {}

		MLMERXEnableRequest(&gsRxPacket, 0L);

		if ( xQueueReceive( xRadioTransmitQueue, &msgP, portTICK_RATE_MS * 1000 ) == pdPASS ) {
			vTaskDelay(portTICK_RATE_MS * 100);
		}

		MLMERXDisableRequest();
	}

}

// --------------------------------------------------------------------------

void vRadioReceiveTask( void *pvParameters ) {
	byte	*packetDataP;

	// The radio receive task will return a pointer to a radio data packet.

	/* Create the queue used by the producer and consumer. */
	xRadioReceiveQueue = xQueueCreate(RADIO_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(unsigned portBASE_TYPE));
	initSMACRadioQueueGlue(xRadioReceiveQueue);

	if ( xRadioReceiveQueue ) {
		/* Now the queue is created it is safe to enable the radio receive interrupt. */
		SW1Int_Enable();

		for ( ;; ) {
			/* Simply wait for data to arrive from the button push interrupt. */

			if ( xQueueReceive( xRadioReceiveQueue, &packetDataP, portMAX_DELAY ) == pdPASS ) {
				if (xQueueSend( xLEDBlinkQueue, &gLED2, pdFALSE )) {}

				if (xQueueSend( xRadioTransmitQueue, NULL, pdFALSE )) {}

			}

		}

	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}

