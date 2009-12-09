/*
	FlyWeight
	� Copyright 2005, 2006 Jeffrey B. Williams
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

#define PFC_QUEUE_SIZE		2

#define	MAX_DRIFT			500
#define	SD_FCS_TIMER		gTmr1_c
#define	SD_CMD_TIMER		gTmr2_c
#define SD_CLK_TIMER		gTm23_c
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
		gwUINT8 byte0;
		gwUINT8 byte1;
		gwUINT8 byte2;
		gwUINT8 byte3;
	} bytes ;
} USsiSampleType;


extern xQueueHandle	gPFCQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void pfcTask( void *pvParameters );
void commandCallback(TmrNumber_t tmrNumber);
void setupSSI();
void setupCommandIntercept();
gwUINT8 crc7(gwUINT8 * inDataPtr, gwUINT8 inBytes);

#endif //PFC_TASK_H
