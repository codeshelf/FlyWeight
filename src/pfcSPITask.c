/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "pfcSPITask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "remoteMgmtTask.h"
#include "GPIO_Interface.h"
#include "Spi_Interface.h"
#include "crc.h"

// Globals

xTaskHandle gPFCTask = NULL;
xQueueHandle gPFCQueue;
ESDCardState gSDCardState;
ESDCardCmdState gSDCardCmdState;
ESDCardDataMode gSDCardDataMode = eSDCardDataModeNarrow;
gwUINT8 gReadBlock[SD_BLOCK_SIZE];

extern portTickType xTickCount;

// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {
	TmrErr_t error;

	if (gPFCQueue) {

		gpioInit();
		setupSPI();
		gSDCardState = eSDCardStateIdle;
		gSDCardCmdState = eSDCardCmdStateStd;

		readBlock(0, gReadBlock);
		readBlock(1, gReadBlock);
		readBlock(2, gReadBlock);
		readBlock(20, gReadBlock);
		readBlock(21, gReadBlock);
		readBlock(22, gReadBlock);

		for (;;) {
			vTaskDelay(10);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}

// --------------------------------------------------------------------------

void gpioInit(void) {

	GpioErr_t error;

	// Setup the function enable pins for SPI.
	error = Gpio_SetPinFunction(gGpioPin4_c, gGpioAlternate1Mode_c);
	error = Gpio_SetPinFunction(gGpioPin5_c, gGpioAlternate1Mode_c);
	error = Gpio_SetPinFunction(gGpioPin6_c, gGpioAlternate1Mode_c);
	error = Gpio_SetPinFunction(gGpioPin7_c, gGpioAlternate1Mode_c);

}

// --------------------------------------------------------------------------
/*
 *
 */

static void setupSPI() {

	spiErr_t error;
	spiConfig_t spiConfig;
	gwUINT32 rcvByte = 0;
	SDArgumentType cmdArg;

//	IntAssignHandler(gSpiInt_c, (IntHandlerFunc_t) spiInterrupt);
//	ITC_SetPriority(gSpiInt_c, gItcNormalPriority_c);
//	ITC_EnableInterrupt(gSpiInt_c);

	error = SPI_Open();

	// Setup the SPI mode.
	spiConfig.ClkCtrl.Word = 0;
	spiConfig.ClkCtrl.Bits.ClockCount = 8;
	spiConfig.ClkCtrl.Bits.DataCount = 8;
	spiConfig.Setup.Word = 0;
	spiConfig.Setup.Bits.ClockFreq = ConfigClockFreqDiv128;
	spiConfig.Setup.Bits.ClockPhase = ConfigClockPhase2ndEdge;
	spiConfig.Setup.Bits.ClockPol = ConfigClockPolNegative;
	spiConfig.Setup.Bits.MisoPhase = ConfigMisoPhaseSameEdge;
	spiConfig.Setup.Bits.Mode = ConfigModeMaster;
	spiConfig.Setup.Bits.SdoInactive = ConfigSdoInactiveH;
	spiConfig.Setup.Bits.SsDelay = ConfigSsDelay1Clk;
	spiConfig.Setup.Bits.SsSetup = ConfigSsSetupMasterLow;
	spiConfig.Setup.Bits.S3Wire = ConfigS3WireInactive;
	error = SPI_SetConfig(&spiConfig);

	// Clock out at least 74 clocks (80 in this case.)
	clockDelay(10);

	// Send CMD0 to send the card into SPI mode.
	// (CS will be low due to SPI_SS control from the MCU, so the SDCard will go into SPI mode.)
	cmdArg.word = 0;
	if (sendCommandWithArg(eSDCardCmd0, cmdArg, eResponseIdle)) {
	}

	cmdArg.word = 0;
	sendCommand(eSDCardCmd55, eResponseIdle);
	while (sendCommandWithArg(eSDCardCmd41, cmdArg, eResponseOK)) {
		sendCommand(eSDCardCmd55, eResponseIdle);
	}

	cmdArg.word = SD_BLOCK_SIZE;
	if (sendCommandWithArg(eSDCardCmd16, cmdArg, eResponseOK)) {
	}
}

// --------------------------------------------------------------------------

gwUINT8 sendCommandWithArg(gwUINT8 inSDCommand, SDArgumentType inArgument, gwUINT8 inExpectedResponse) {
	gwUINT8 counter;
	gwUINT32 sendSample;
	gwUINT32 rcvSample = 0;
	spiErr_t error;

	// Send the command byte.
	sendSample = (inSDCommand | 0x40) << 24;
	error = SPI_WriteSync(sendSample);

	// Send the arguments.
	for (counter = 4; counter > 0; counter--) {
		sendSample = inArgument.bytes[counter-1] << 24;
		error = SPI_WriteSync(sendSample);
	}

	// Send the CRC.
	error = SPI_WriteSync(0x95000000);

	// Get the R1 response.
	counter = SD_WAIT_CYCLES;
	do {
		SPI_REGS_P->TxData = 0xffffffff;
		error = SPI_ReadSync(&rcvSample);
		counter--;
	} while (((rcvSample & 0x7f) != inExpectedResponse) && counter > 0);

	if (counter)
		return (0);
	else
		return (1);
}

// --------------------------------------------------------------------------

gwUINT8 sendCommand(gwUINT8 inSDCommand, gwUINT8 inExpectedResponse) {
	gwUINT8 counter;
	gwUINT32 sendSample;
	gwUINT32 rcvSample = 0;
	spiErr_t error;

	// Send the command byte.
	sendSample = (inSDCommand | 0x40) << 24;
	error = SPI_WriteSync(sendSample);

	// Send the CRC.
	error = SPI_WriteSync(0x95000000);

	// Get the R1 response.
	counter = SD_WAIT_CYCLES;
	do {
		SPI_REGS_P->TxData = 0xffffffff;
		error = SPI_ReadSync(&rcvSample);
		counter--;
	} while (((rcvSample & 0x7f) != inExpectedResponse) && counter > 0);

	if (counter)
		return (0);
	else
		return (1);
}

// --------------------------------------------------------------------------

gwUINT8 readBlock(gwUINT32 inSDBlockAddr, gwUINT8 *inDestPtr) {
	gwUINT32 rcvSample = 0;
	gwUINT16 counter;
	spiErr_t error;
	SDArgumentType cmdArg;

	cmdArg.word = inSDBlockAddr << SD_BLOCK_SHIFT;
	if (sendCommandWithArg(eSDCardCmd17, cmdArg, eResponseOK)) {
		// Command IDLE fail
		return (4);
	}

	// Wait for a response.
	while ((rcvSample & 0xff) != 0xfe) {
		SPI_REGS_P->TxData = 0xffffffff;
		error = SPI_ReadSync(&rcvSample);
	}

	// Read the bytes from the block.
	for (counter = 0; counter < SD_BLOCK_SIZE; counter++) {
		SPI_REGS_P->TxData = 0xffffffff;
		error = SPI_ReadSync(&rcvSample);
		*inDestPtr++ = (rcvSample & 0xff);
	}

	SPI_REGS_P->TxData = 0xffffffff;
	error = SPI_ReadSync(&rcvSample);
	SPI_REGS_P->TxData = 0xffffffff;
	error = SPI_ReadSync(&rcvSample);
	SPI_REGS_P->TxData = 0xffffffff;
	error = SPI_ReadSync(&rcvSample);

	return (0);
}

// --------------------------------------------------------------------------

void clockDelay(gwUINT8 inFrames) {
	while (inFrames--) {
		SPI_WriteSync(0xFF000000);
	}
}

// --------------------------------------------------------------------------

void spiInterrupt(void) {
	spiStatus_t status;

	status = SPI_GetStatus();
}
