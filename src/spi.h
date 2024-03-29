/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef SPI_H
#define SPI_H

#include "FreeRTOS.h"
#include "queue.h"
#include "UART.h"
#include "radioCommon.h"
#include "Spi_Interface.h"

// --------------------------------------------------------------------------
// Defines.

#define SD_BLOCK_SIZE			512
//#define SD_BLOCK_SIZE			(0x00000200)
#define SD_BLOCK_SHIFT			(9)
#define SPI_WAIT_CYCLES 		10
#define SD_WAIT_CYCLES 			30

#define VCC_SW_GPIO				gGpioPin29_c
#define VCC_HELPER_GPIO			gGpioPin25_c
#define BUS_SW_GPIO				gGpioPin36_c
#define BUS_CARD_DETECT_GPIO	gGpioPin14_c
#define SPI_MOSI				gGpioPin6_c
#define SPI_MISO				gGpioPin5_c
#define SPI_CLK					gGpioPin7_c
#define SPI_CS					gGpioPin4_c
#define SD_DAT0_PULLUP			gGpioPin3_c

#define SPI_CS_ON				Gpio_SetPinData(SPI_CS, gGpioPinStateLow_c);
#define SPI_CS_OFF				Gpio_SetPinData(SPI_CS, gGpioPinStateHigh_c);

#define CARD_LED_ON				Led1On();
#define CARD_LED_OFF			Led1Off();
#define PACKET_LED_ON			Led2On();
#define PACKET_LED_OFF			Led2Off();

#define BUS_SW_ON				Gpio_SetPinData(BUS_SW_GPIO, gGpioPinStateLow_c); \
								gSDCardBusConnected = TRUE; \
								//CARD_LED_OFF;

#define BUS_SW_OFF				Gpio_SetPinData(BUS_SW_GPIO, gGpioPinStateHigh_c); \
								gSDCardBusConnected = FALSE; \
								//CARD_LED_ON;

#define VCC_SW_ON				Gpio_SetPinDir(VCC_HELPER_GPIO, gGpioDirIn_c); \
								Gpio_EnPinPullup(VCC_HELPER_GPIO, TRUE); \
								Gpio_SelectPinPullup(VCC_HELPER_GPIO, gGpioPinPullup_c); \
								Gpio_SetPinData(VCC_SW_GPIO, gGpioPinStateHigh_c); \
								gSDCardVccConnected = TRUE;

#define VCC_SW_OFF				/* Gpio_SetPinDir(VCC_HELPER_GPIO, gGpioDirIn_c); */ \
								/* Gpio_EnPinPullup(VCC_HELPER_GPIO, TRUE); */ \
								/* Gpio_SelectPinPullup(VCC_HELPER_GPIO, gGpioPinPulldown_c); */ \
								Gpio_SetPinDir(VCC_HELPER_GPIO, gGpioDirOut_c); \
								Gpio_SetPinData(VCC_HELPER_GPIO, gGpioPinStateLow_c); \
								Gpio_SetPinData(VCC_SW_GPIO, gGpioPinStateLow_c); \
								gSDCardVccConnected = FALSE;

#define CARD_INSERTED			Gpio_SetPinDir(BUS_CARD_DETECT_GPIO, gGpioDirOut_c); \
								Gpio_SetPinData(BUS_CARD_DETECT_GPIO, gGpioPinStateLow_c);

#define CARD_UNINSERTED			Gpio_SetPinDir(BUS_CARD_DETECT_GPIO, gGpioDirOut_c); \
								Gpio_SetPinData(BUS_CARD_DETECT_GPIO, gGpioPinStateHigh_c); \
								vTaskDelay(500);

typedef union {
	gwUINT16 value;
	gwUINT8 bytes[2];
} crc16Type;

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
	eSDCardCmd8 = 8,
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

typedef gwUINT8 ResponseArrayType;

// --------------------------------------------------------------------------
// Functions prototypes.

gwBoolean enableSPI(void);
gwBoolean disableSPI(void);
void spiInterrupt(void);
void clockDelay(gwUINT8 inFrames);

spiErr_t readByte(gwUINT8* inByte);
spiErr_t writeByte(gwUINT8 inByte);

ESDCardResponse sendCommand(gwUINT8 inSDCommand, gwUINT8 inExpectedResponse, gwBoolean inControlCS);
ESDCardResponse sendCommandWithArg(gwUINT8 inSDCommand, SDArgumentType inArgument, gwUINT8 inExpectedResponse, gwBoolean inControlCS);

ESDCardResponse readBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr);
ESDCardResponse writeBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr);

ESDCardResponse writePartialBlockBegin(gwUINT32 inBlockNumber);
ESDCardResponse writePartialBlock(gwUINT8 *inDataPtr, gwUINT16 inBytes);
ESDCardResponse writePartialBlockEnd();

ESDCardResponse checkResponse(gwUINT8 inExpectedResponse);

crc16Type crc16(crc16Type inOldCRC, gwUINT8 inByte);
ESDCardResponse crcBlock(gwUINT32 inBlockNumber);

ESDCardResponse sendSDCardBusCommand(gwUINT8 inSDCommand, SDArgumentType inArgument);
ESDCardResponse getSDCardBusResponse(ResponseArrayType *outResponse, gwUINT8 inByteCount);
gwBoolean enableSDCardBus(void);

#endif // SPI_H
