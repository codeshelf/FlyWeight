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
#include "spi.h"

// Globals

xTaskHandle gPFCTask = NULL;
xQueueHandle gPFCQueue;
ESDCardState gSDCardState;
gwUINT8 gReadBlock[SD_BLOCK_SIZE];

extern portTickType xTickCount;

// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {

	if (gPFCQueue) {

		gpioInit();
		setupSPI();
		gSDCardState = eSDCardStateReady;

		// Set the SPI speed to 3MHz.
		spiErr_t error;
		spiConfig_t spiConfig;
		error = SPI_GetConfig(&spiConfig);
		spiConfig.Setup.Bits.ClockFreq = ConfigClockFreqDiv4;
		error = SPI_SetConfig(&spiConfig);

//		readBlock(0, gReadBlock);
//		readBlock(20, gReadBlock);
//		readBlock(21, gReadBlock);
//		readBlock(22, gReadBlock);

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

	// Now we're in regular SPI mode, so restore the SS function.
	spiConfig.Setup.Bits.SsSetup = ConfigSsSetupMasterAutoL;
	error = SPI_SetConfig(&spiConfig);

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

void spiInterrupt(void) {
	spiStatus_t status;

	status = SPI_GetStatus();
}
