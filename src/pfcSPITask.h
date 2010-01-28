/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef PFCSPI_TASK_H
#define PFCSPI_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define PFC_QUEUE_SIZE			2
#define SD_BLOCK_SIZE			512
//#define SD_BLOCK_SIZE			(0x00000200)
#define SD_BLOCK_SHIFT			(9)
#define SD_WAIT_CYCLES 10

typedef enum {
	eResponseOK = 0,
	eResponseIdle = 1
} ER1Response;

typedef enum {
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
	eSDCardCmd17 = 17,
	eSDCardCmd41 = 41,
	eSDCardCmd55 = 55,
	eSDCardCmdInvalid = 255
} ESDCardCommand;

typedef enum {
	eSDCardRespTypeInvalid = 0,
	eSDCardRespTypeNone,
	eSDCardRespType1,
	eSDCardRespType1b,
	eSDCardRespType2,
	eSDCardRespType3,
	eSDCardRespType6
} ESDCardResponseType;

typedef enum {
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
	eSDCardCmdStateStd = 0, eSDCardCmdStateApp
} ESDCardCmdState;

typedef enum {
	eSDCardDataModeNarrow = 0, eSDCardDataModeWide
} ESDCardDataMode;

typedef union {
	gwUINT32 word;
	gwUINT8 bytes[4];
} SDArgumentType;

extern xQueueHandle gPFCQueue;

// --------------------------------------------------------------------------
// Local functions prototypes.

void pfcTask(void *pvParameters);
void gpioInit(void);
void setupSPI(void);

void spiInterrupt(void);
void clockDelay(gwUINT8 inFrames);
gwUINT8 sendCommand(gwUINT8 inSDCommand, gwUINT8 inExpectedResponse);
gwUINT8 sendCommandWithArg(gwUINT8 inSDCommand, SDArgumentType inArgument, gwUINT8 inExpectedResponse);
gwUINT8 readBlock(gwUINT32 inSDBlockAddr, gwUINT8 *inDestPtr);

#endif //PFCSPI_TASK_H
