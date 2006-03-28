#include <stdlib.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pub_def.h"

xQueueHandle	gRadioReceiveQueue;

void initMC13191(xQueueHandle inRadioReceiveQueue) {

	gRadioReceiveQueue = inRadioReceiveQueue;

};

/*-----------------------------------------------------------*/

#pragma CODE_SEG __NEAR_SEG NON_BANKED 
//#pragma NO_ENTRY
//#pragma NO_EXIT
//#pragma NO_FRAME
//#pragma NO_ENTRY
//#pragma NO_RETURN
void MCPSDataIndication(tRxPacket *gsRxPacket) {

	/* Send the incremented value down the queue.  The button push task is
	blocked waiting for the data.  As the button push task is high priority
	it will wake and a context switch should be performed before leaving
	the ISR. */

	if ( xQueueSendFromISR( gRadioReceiveQueue, &gsRxPacket, pdFALSE ) ) {
		/* NOTE: This macro can only be used if there are no local
		variables defined.  This function uses a static variable so it's
		use is permitted.  If the variable were not static portYIELD() 
		would have to be used in it's place. */ 
		//portTASK_SWITCH_FROM_ISR();
		//portYIELD();
		//__asm( "rti" );
	}
};

#pragma CODE_SEG DEFAULT

/*-----------------------------------------------------------*/

void MLMEMC13192ResetIndication() {
}

;
