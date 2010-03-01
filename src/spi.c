/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "spi.h"
#include "GPIO_Interface.h"

crcType gCRC16;

// --------------------------------------------------------------------------
/*
 *
 */

gwBoolean enableSPI() {

	gwBoolean result = TRUE;

	gwUINT8 attempts = 0;
	spiErr_t spiErr;
	GpioErr_t gpioErr;
	spiConfig_t spiConfig;
	gwUINT32 rcvByte = 0;
	SDArgumentType cmdArg;
	ESDCardResponse spiResult;

	// Enable the DAT0 pullup.
	gpioErr = Gpio_EnPinPullup(SD_DAT0_PULLUP, TRUE);
	gpioErr = Gpio_SelectPinPullup(SD_DAT0_PULLUP, gGpioPinPullup_c);

	// Setup the function enable pins for SPI.
	gpioErr = Gpio_SetPinFunction(SPI_MOSI, gGpioAlternate1Mode_c);
	gpioErr = Gpio_SetPinFunction(SPI_MISO, gGpioAlternate1Mode_c);
	gpioErr = Gpio_SetPinFunction(SPI_CLK, gGpioAlternate1Mode_c);

	// Setup the SPI CS.  (We need to control that manually - auto doesn't work.)
	gpioErr = Gpio_SetPinFunction(SPI_CS, gGpioNormalMode_c);
	gpioErr = Gpio_SetPinDir(SPI_CS, gGpioDirOut_c);
	SPI_CS_OFF;

	//	IntAssignHandler(gSpiInt_c, (IntHandlerFunc_t) spiInterrupt);
	//	ITC_SetPriority(gSpiInt_c, gItcNormalPriority_c);
	//	ITC_EnableInterrupt(gSpiInt_c);

	spiErr = SPI_Open();

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
	spiErr = SPI_SetConfig(&spiConfig);

	// Clock out at least 74 clocks (80 in this case.)
	SPI_CS_ON;
	clockDelay(10);
	SPI_CS_OFF;

	clockDelay(8);

	// Send CMD0 to send the card into SPI mode.
	// (CS will be low due to SPI_SS control from the MCU, so the SDCard will go into SPI mode.)
	cmdArg.word = 0;
	spiResult = sendCommandWithArg(eSDCardCmd0, cmdArg, eResponseIdle, TRUE);
	if (spiResult != eResponseOK) {
		// Sometimes the card doesn't seem to be in the idle state after CMD0 - instead it's ready.
		// Then sometimes is takes a while to get an idle response.
		// This is a way to deal with both cases.
		gwBoolean isStarted = FALSE;
		attempts = 0;
		do {
			if (sendCommandWithArg(eSDCardCmd0, cmdArg, eResponseIdle, TRUE) == eResponseOK) {
				isStarted = TRUE;
//			} else if (sendCommandWithArg(eSDCardCmd0, cmdArg, eResponseOK, TRUE) == eResponseOK) {
//				isStarted = TRUE;
			}
			// If we can't get it going in 25 tries, it's never going to happen.
//			if (attempts++ == 25) {
//				result = disableSPI();
//				return FALSE;
//			}
		} while (!isStarted);
	}

	attempts = 0;
	cmdArg.word = 0;
	do {
		spiResult = sendCommand(eSDCardCmd55, eResponseIdle, TRUE);
		spiResult = sendCommandWithArg(eSDCardCmd41, cmdArg, eResponseOK, TRUE);
		// If we can't get it going in 50 tries, it's never going to happen.
//		if (attempts++ == 250) {
//			result = disableSPI();
//			return FALSE;
//		}
	} while (spiResult != eResponseOK);

	cmdArg.word = SD_BLOCK_SIZE;
	spiResult = sendCommandWithArg(eSDCardCmd16, cmdArg, eResponseOK, TRUE);

	// Set the SPI speed to 3MHz.
	spiErr = SPI_GetConfig(&spiConfig);
	spiConfig.Setup.Bits.ClockFreq = ConfigClockFreqDiv8;
	spiErr = SPI_SetConfig(&spiConfig);

	return result;
}

// --------------------------------------------------------------------------
/*
 *
 */

gwBoolean disableSPI() {

	gwBoolean result = FALSE;

	spiErr_t spiErr;
	GpioErr_t gpioErr;

	spiErr = SPI_Close();

	// Set all of the SPI-related pins to normal mode.
	gpioErr = Gpio_SetPinFunction(SPI_MOSI, gGpioNormalMode_c);
	gpioErr = Gpio_SetPinFunction(SPI_MISO, gGpioNormalMode_c);
	gpioErr = Gpio_SetPinFunction(SPI_CLK, gGpioNormalMode_c);
	gpioErr = Gpio_SetPinFunction(SPI_CS, gGpioNormalMode_c);

	// Set the IO dir of the SPI pins to input.
	gpioErr = Gpio_SetPinDir(SPI_MOSI, gGpioDirIn_c);
	gpioErr = Gpio_SetPinDir(SPI_MISO, gGpioDirIn_c);
	gpioErr = Gpio_SetPinDir(SPI_CLK, gGpioDirIn_c);
	gpioErr = Gpio_SetPinDir(SPI_CS, gGpioDirIn_c);

	// Disable pull-ups on the SPI lines.
	gpioErr = Gpio_EnPinPullup(SPI_MOSI, FALSE);
	gpioErr = Gpio_EnPinPullup(SPI_MISO, FALSE);
	gpioErr = Gpio_EnPinPullup(SPI_CLK, FALSE);
	gpioErr = Gpio_EnPinPullup(SPI_CS, FALSE);

//	gpioErr = Gpio_SelectPinPullup(SPI_MOSI, gGpioPinPullup_c);
//	gpioErr = Gpio_SelectPinPullup(SPI_MISO, gGpioPinPullup_c);
//	gpioErr = Gpio_SelectPinPullup(SPI_CLK, gGpioPinPullup_c);
//	gpioErr = Gpio_SelectPinPullup(SPI_CS, gGpioPinPullup_c);

	// Disable the pull-up on the DAT0 line.
	gpioErr = Gpio_EnPinPullup(SD_DAT0_PULLUP, FALSE);

	return result;
}

// --------------------------------------------------------------------------

void spiInterrupt(void) {
	spiStatus_t status;

	status = SPI_GetStatus();
}

// --------------------------------------------------------------------------

ESDCardResponse sendCommandWithArg(gwUINT8 inSDCommand, SDArgumentType inArgument, gwUINT8 inExpectedResponse,
        gwBoolean inControlCS) {
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;
	ESDCardResponse result;

	if (inControlCS) {
		SPI_CS_ON;
	}

	// Send the command byte.
	if (writeByte(inSDCommand | 0x40) != gSpiErrNoError_c) {
		return eResponseSPIError;
	}

	// Send the arguments.
	for (counter = 4; counter > 0; counter--) {
		if (writeByte(inArgument.bytes[counter - 1]) != gSpiErrNoError_c) {
			result = eResponseSPIError;
		}
	}

	// Send the CRC.
	if (writeByte(0x95) != gSpiErrNoError_c) {
		return eResponseSPIError;
	}

	// Check the response.
	if (checkResponse(inExpectedResponse, SD_WAIT_CYCLES) != eResponseOK) {
		SPI_CS_OFF;
		return eResponseInvalidError;
	}

	if (inControlCS) {
		SPI_CS_OFF;
	}

	return eResponseOK;
}

// --------------------------------------------------------------------------

ESDCardResponse sendCommand(gwUINT8 inSDCommand, gwUINT8 inExpectedResponse, gwBoolean inControlCS) {
	gwUINT8 rcvByte = 0;
	ESDCardResponse result;

	if (inControlCS) {
		SPI_CS_ON;
	}

	// Send the command byte.
	if (writeByte(inSDCommand | 0x40) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}

	// Send the CRC.
	if (writeByte(0x95) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}

	// Check the response.
	if (checkResponse(inExpectedResponse, SD_WAIT_CYCLES) != eResponseOK) {
		SPI_CS_OFF;
		return eResponseInvalidError;
	}

	if (inControlCS) {
		SPI_CS_OFF;
	}

	return eResponseOK;
}

// --------------------------------------------------------------------------

ESDCardResponse readBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr) {
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;
	SDArgumentType cmdArg;
	ESDCardResponse result;

	SPI_CS_ON;

	cmdArg.word = inBlockNumber << SD_BLOCK_SHIFT;
	result = sendCommandWithArg(eSDCardCmd17, cmdArg, eResponseOK, FALSE);
	if (result != eResponseOK) {
		SPI_CS_OFF;
		return (result);
	}

	// Check the response.
	if (checkResponse(0xfe, 20) != eResponseOK) {
		SPI_CS_OFF;
		return eResponseInvalidError;
	}

	// Read the bytes from the block.
	for (counter = 0; counter < SD_BLOCK_SIZE; counter++) {
		if (readByte(&rcvByte) != gSpiErrNoError_c) {
			SPI_CS_OFF;
			return eResponseSPIError;
		}
		*inDataPtr++ = (rcvByte & 0xff);
	}

	if (readByte(&rcvByte) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}
	if (readByte(&rcvByte) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}
	if (readByte(&rcvByte) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}

	SPI_CS_OFF;

	return eResponseOK;
}

// --------------------------------------------------------------------------

ESDCardResponse writeBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr) {

	writePartialBlockStart(inBlockNumber);
	writePartialBlock(inDataPtr, SD_BLOCK_SIZE);
	writePartialBlockEnd();
}

// --------------------------------------------------------------------------

ESDCardResponse writePartialBlockBegin(gwUINT32 inBlockNumber) {

	SDArgumentType cmdArg;
	ESDCardResponse result;

	SPI_CS_ON;

	cmdArg.word = inBlockNumber;
	result = sendCommandWithArg(eSDCardCmd24, cmdArg, eResponseOK, FALSE);
	if (result != eResponseOK) {
		SPI_CS_OFF;
		return result;
	}

	if (writeByte(0xfe) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}

	return eResponseOK;

}

// --------------------------------------------------------------------------

ESDCardResponse writePartialBlock(gwUINT8 *inDataPtr, gwUINT8 inBytes) {
	gwUINT16 counter;

	for (counter = 0; counter < inBytes; counter++) {
		if (writeByte(*inDataPtr++) != gSpiErrNoError_c) {
			SPI_CS_OFF;
			return eResponseSPIError;
		}
	}

	return eResponseOK;
}

// --------------------------------------------------------------------------

ESDCardResponse writePartialBlockEnd() {
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;
	SDArgumentType cmdArg;

	if (writeByte(0xff) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}
	if (writeByte(0xff) != gSpiErrNoError_c) {
		SPI_CS_OFF;
		return eResponseSPIError;
	}

	// Get the data write response.
	counter = SD_WAIT_CYCLES;
	do {
		if (readByte(&rcvByte) != gSpiErrNoError_c) {
			SPI_CS_OFF;
			return eResponseSPIError;
		}
		counter--;
	} while (((rcvByte & 0x10) == 0x10) && counter > 0);

	if ((rcvByte & 0x0f) != 0x05) {
		SPI_CS_OFF;
		if ((rcvByte & 0x0f) == 0x0b) {
			return eResponseCRCError;
		} else {
			return eResponseWriteBlockError;
		}
	}

	do {
		if (readByte(&rcvByte) != gSpiErrNoError_c) {
			SPI_CS_OFF;
			return eResponseSPIError;
		}
	} while (rcvByte == 0x00);

	SPI_CS_OFF;

	return eResponseOK;
}

// --------------------------------------------------------------------------

ESDCardResponse checkResponse(gwUINT8 inExpectedResponse, gwUINT8 inCheckCycles) {
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;

	// Get the R1 response, and make sure it matches what we expected.
	counter = 0;
	do {
		if (readByte(&rcvByte) != gSpiErrNoError_c) {
			SPI_CS_OFF;
			return eResponseSPIError;
		}
	} while ((rcvByte != inExpectedResponse) && (counter++ < inCheckCycles));

	if (counter < inCheckCycles)
		return eResponseOK;
	else
		return eResponseInvalidError;
}

// --------------------------------------------------------------------------

spiErr_t readByte(gwUINT8* inByte) {

	spiErr_t error;
	gwUINT32 rcvSample = 0;

	// SPI on this chip is weird: the Tx/Rx reg is 32 bits wide.
	// On Tx we send the 8 MSB bits, and on Rx we get the 8 LSB bits.

	// For reading the outbound Tx reg should be all 1s to avoid writing out values.
	SPI_REGS_P->TxData = 0xffffffff;
	error = SPI_ReadSync(&rcvSample);
	// Mask off all but the LSB 8 bits.
	*inByte = rcvSample & 0xff;

	return error;

}

// --------------------------------------------------------------------------

spiErr_t writeByte(gwUINT8 inByte) {
	// SPI on this chip is weird: the Tx/Rx reg is 32 bits wide.
	// On Tx we send the 8 MSB bits, and on Rx we get the 8 LSB bits.
	gwUINT32 writeSample = (inByte << 24);
	return SPI_WriteSync(writeSample);
}

// --------------------------------------------------------------------------

void clockDelay(gwUINT8 inFrames) {
	while (inFrames--) {
		writeByte(0xFF);
	}
}

// --------------------------------------------------------------------------

crcType crc16(crcType inOldCRC, gwUINT8 inByte) {
	crcType crc;
	gwUINT16 x;

	x = ((inOldCRC.value >> 8) ^ inByte) & 0xff;
	x ^= x >> 4;

	crc.value = (inOldCRC.value << 8) ^ (x << 12) ^ (x << 5) ^ x;

	crc.value &= 0xffff;

	return crc;
}
