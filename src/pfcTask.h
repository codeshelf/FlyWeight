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

#define PFC_QUEUE_SIZE			2

#define	MAX_DRIFT				500

#define	FSYNC_TIMER				gTmr3_c
#define FSYNC_PRIMARY_SOURCE	gTmrPrimaryClkDiv16_c
#define FSYNC_SECONDARY_SOURCE	gTmrSecondaryCnt1Input_c

#define	RXTX_TIMER				gTmr1_c
#define RXTX_PRIMARY_SOURCE		gTmrPrimaryClkDiv16_c
#define RXTX_SECONDARY_SOURCE	gTmrSecondaryCnt1Input_c

#define SSI_24BIT_WORD			0x0b
#define SSI_20BIT_WORD			0x09
#define SSI_FRAME_LEN2			0x01
#define SSI_FRAME_LEN7			0x06

#define SD_CLK_RATE				400000
#define TMR_CLK_RATE			1500000	/* Bus Clk / Prescaler --> 24,000,000 / 16 */
#define FSYNC_TRIGGER_HIGH		3		/* 1 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 1 * 1,500,000 / 400,000 */
//#define FSYNC_TRIGGER_LOW		1000		/* (was 210) 47 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 47 * 1,500,000 / 400,000 */
//#define FSYNC_SUSTAIN_HIGH		8		/* 8 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 1 * 1,500,000 / 400,000 */
//#define FSYNC_SUSTAIN_LOW		175		/* 47 SD card clocks * TMR_CLK_RATE / SD_CLK_RATE --> 47 * 1,500,000 / 400,000 */
//#define FSYNCR2_SUSTAIN_LOW		1000	/* 5X normal sustain low. */
#define RXTX_TRIGGER_RESYNC		96
#define RXTX_TIMEOUT			2000

typedef  enum {
	eSDCardCmd0 = 0,
	eSDCardCmd1 = 1,
	eSDCardCmd2 = 2,
	eSDCardCmd3 = 3,
	eSDCardCmd5 = 5,
	eSDCardCmd6 = 6,
	eSDCardCmd7 = 7,
	eSDCardCmd9 = 9,
	eSDCardCmd13 = 13,
	eSDCardCmd16 = 16,
	eSDCardCmd41 = 41,
	eSDCardCmd55 = 55,
	eSDCardCmdInvalid = 255
} ESDCardCommand;

typedef  enum {
	eSDCardRespTypeInvalid = 0,
	eSDCardRespTypeNone,
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
	eSDCardStateTransfer,
	eSDCardStateSendData,
	eSDCardStateReceiveData,
	eSDCardStateProgram,
	eSDCardStateDB
} ESDCardState;

typedef enum {
	eSDCardCmdStateStd = 0,
	eSDCardCmdStateApp
} ESDCardCmdState;

typedef enum {
	eSDCardDataModeNarrow = 0,
	eSDCardDataModeWide
} ESDCardDataMode;

typedef union {
	gwUINT32 word;
	struct {
		gwUINT8 byte3;
		gwUINT8 byte2;
		gwUINT8 byte1;
		gwUINT8 unused;
	} bytes ;
} USsiSampleType;


extern xQueueHandle	gPFCQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void pfcTask( void *pvParameters );
void gpioInit(void);
void setupSSI(void);
void setupTimers(void);
gwUINT8 crc7(USsiSampleType *inSamplePtr, gwUINT8 inSamplesToCheck);

void resync(TmrNumber_t tmrNumber);
void waitForNextFrame(TmrNumber_t tmrNumber);
void startFrame(TmrNumber_t tmrNumber);

void ssiInterrupt(void);
void sendCmdResponse(ESDCardCommand inCmdNum, ESDCardResponseType inResponseType);

#endif //PFC_TASK_H
