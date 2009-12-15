/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef PFC_TASK_H
#define PFC_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"
#include "Timer.h"
#include "Timer_Regs.h"
#include "Ssi_Interface.h"

#define PFC_QUEUE_SIZE		2

#define	MAX_DRIFT			500
#define	SSI_FRAMESYNC_TIMER	gTmr3_c
#define SSI_FRAMESYNC_EVENT	gTmrComp1Event_c

#define PRIMARY_SOURCE		gTmrPrimaryClkDiv32_c
#define SECONDARY_SOURCE	gTmrSecondaryCnt1Input_c

#define SD_CLK_RATE			400000
#define TMR_CLK_RATE		750000	/* Bus Clk / Prescaler --> 24,000,000 / 32 */
#define FS_CLOCK_COUNT		3		/* 8 clocks * TMR_CLK_RATE / SD SD_CLK_RATE --> 8 * 375,000 / 400,000 */

#define  gGpioBeepTimer_c	((GpioPin_t)((uint8_t)gGpioPin8_c + (uint8_t)PWM_TIMER))

typedef  enum {
	eSDCardCmd0 = 0,
	eSDCardCmd2 = 2,
	eSDCardCmd3 = 3,
	eSDCardCmd41 = 41,
	eSDCardCmd55 = 55,
	eSDCardCmdInvalid = 255
} ESDCardCommand;

typedef  enum {
	eSDCardRespTypeInvalid = 0,
	eSDCardRespType1,
	eSDCardRespType1b,
	eSDCardRespType2,
	eSDCardRespType3,
	eSDCardRespType6
} ESDCardResponseType;

typedef  enum {
	eSDCardStateIdle = 0,
	eSDCardStateReady,
	eSDCardStateIdent,
	eSDCardStateStandby,
	eSDCardStateTx,
	eSDCardStateData,
	eSDCardStateRx,
	eSDCardStateProgram,
	eSDCardStateDB
} ESDCardState;

typedef enum {
	eSDCardCmdStateStd = 0,
	eSDCardCmdStateApp
} ESDCardCmdState;

typedef union {
	gwUINT32 word;
	struct {
		gwUINT8 byte3;
		gwUINT8 byte2;
		gwUINT8 byte1;
		gwUINT8 byte0;
	} bytes ;
} USsiSampleType;


extern xQueueHandle	gPFCQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void pfcTask( void *pvParameters );
void gpioInit(void);
void setupSSI();
void ssiInterrupt(void);
void setupTimers();
void timerCallback(TmrNumber_t tmrNumber);
gwUINT8 crc7(gwUINT8 *inSample1Ptr, gwUINT8 *inSample2Ptr);

#endif //PFC_TASK_H
