/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef SPI_H
#define SPI_H

#include "FreeRTOS.h"
#include "queue.h"
#include "usb.h"
#include "radioCommon.h"
#include "Spi_Interface.h"

// --------------------------------------------------------------------------
// Defines.

#define SD_BLOCK_SIZE			512
//#define SD_BLOCK_SIZE			(0x00000200)
#define SD_BLOCK_SHIFT			(9)
#define SD_WAIT_CYCLES 			10

#define CS_INIT					GPIO.DirSetLo		= 0x00000010
#define CS_ON					GPIO.DataResetLo 	= 0x00000010
#define CS_OFF					GPIO.DataSetLo 		= 0x00000010

#define BUS_SW_INIT				GPIO.DirSetHi		= 0x00000010
#define BUS_SW_ON				GPIO.DataResetHi 	= 0x00000010; gSDCardBusConnected = TRUE; Led1Off();
#define BUS_SW_OFF				GPIO.DataSetHi	 	= 0x00000010; gSDCardBusConnected = FALSE; Led1On();

#define VCC_SW_INIT				GPIO.DirSetLo		= 0x20000000
#define VCC_SW_ON				GPIO.DataSetLo 		= 0x20000000; gSDCardPwrConnected = TRUE;
#define VCC_SW_OFF				GPIO.DataResetLo 	= 0x20000000; gSDCardPwrConnected = FALSE;

typedef union {
	gwUINT16 value;
	gwUINT8 bytes[2];
} crcType;

typedef union {
	gwUINT32 word;
	gwUINT8 bytes[4];
} SDArgumentType;

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
	eSDCardCmd24 = 24,
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
	eResponseOK = 0,
	eResponseIdle,
	eResponseEraseReset,
	eResponseIllegalCmd,
	eResponseCRCError,
	eResponseAddrError,
	eResponseParamError,
	eResponseSPIError,
	eResponseInvalidError,
	eResponseWriteBlockError
} ESDCardResponse;

typedef union {
	gwUINT32 word;
	struct {
		gwUINT8 byte3;
		gwUINT8 byte2;
		gwUINT8 byte1;
		gwUINT8 byte0;
	} bytes ;
} AddressType;


// --------------------------------------------------------------------------
// Functions prototypes.

void setupSPI(void);
void spiInterrupt(void);
void clockDelay(gwUINT8 inFrames);

spiErr_t readByte(gwUINT8* inByte);
spiErr_t writeByte(gwUINT8 inByte);

ESDCardResponse sendCommand(gwUINT8 inSDCommand, gwUINT8 inExpectedResponse, gwBoolean inControlCS);
ESDCardResponse sendCommandWithArg(gwUINT8 inSDCommand, SDArgumentType inArgument, gwUINT8 inExpectedResponse, gwBoolean inControlCS);

ESDCardResponse readBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr);
ESDCardResponse writeBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr);

ESDCardResponse writePartialBlockBegin(gwUINT32 inBlockNumber);
ESDCardResponse writePartialBlock(gwUINT8 *inDataPtr, gwUINT8 inBytes);
ESDCardResponse writePartialBlockEnd();

ESDCardResponse checkResponse(gwUINT8 inExpectedResponse, gwUINT8 inCheckCycles);
crcType crc16(crcType inOldCRC, gwUINT8 inByte);

#endif // SPI_H
