/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#include "spi.h"

// --------------------------------------------------------------------------

gwUINT8 sendCommandWithArg(gwUINT8 inSDCommand, SDArgumentType inArgument, gwUINT8 inExpectedResponse) {
	gwUINT8 counter;
	gwUINT8 rcvByte = 0;
	spiErr_t error;

	// Send the command byte.
	error = writeByte(inSDCommand | 0x40);

	// Send the arguments.
	for (counter = 4; counter > 0; counter--) {
		error = writeByte(inArgument.bytes[counter - 1]);
	}

	// Send the CRC.
	error = writeByte(0x95);

	// Get the R1 response.
	counter = SD_WAIT_CYCLES;
	do {
		error = readByte(&rcvByte);
		counter--;
	} while ((rcvByte != inExpectedResponse) && counter > 0);

	if (counter)
		return (0);
	else
		return (1);
}

// --------------------------------------------------------------------------

gwUINT8 sendCommand(gwUINT8 inSDCommand, gwUINT8 inExpectedResponse) {
	gwUINT8 counter;
	gwUINT8 rcvByte = 0;
	spiErr_t error;

	// Send the command byte.
	error = writeByte(inSDCommand | 0x40);

	// Send the CRC.
	error = writeByte(0x95);

	// Get the R1 response.
	counter = SD_WAIT_CYCLES;
	do {
		error = readByte(&rcvByte);
		counter--;
	} while ((rcvByte != inExpectedResponse) && counter > 0);

	if (counter)
		return (0);
	else
		return (1);
}

// --------------------------------------------------------------------------

gwUINT8 readBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr) {
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;
	spiErr_t error;
	SDArgumentType cmdArg;

	cmdArg.word = inBlockNumber << SD_BLOCK_SHIFT;
	if (sendCommandWithArg(eSDCardCmd17, cmdArg, eResponseOK)) {
		// Command IDLE fail
		return (4);
	}

	// Wait for a response.
	while ((rcvByte & 0xff) != 0xfe) {
		error = readByte(&rcvByte);
	}

	// Read the bytes from the block.
	for (counter = 0; counter < SD_BLOCK_SIZE; counter++) {
		error = readByte(&rcvByte);
		*inDataPtr++ = (rcvByte & 0xff);
	}

	error = readByte(&rcvByte);
	error = readByte(&rcvByte);
	error = readByte(&rcvByte);

	return (0);
}

// --------------------------------------------------------------------------

gwUINT8 writeBlock(gwUINT32 inBlockNumber, gwUINT8 *inDataPtr) {
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;
	spiErr_t error;
	SDArgumentType cmdArg;

	cmdArg.word = inBlockNumber << SD_BLOCK_SHIFT;
	if (sendCommandWithArg(eSDCardCmd24, cmdArg, eResponseOK)) {
	}

	error = writeByte(0xfe);

	for (counter = 0; counter < SD_BLOCK_SIZE; counter++) {
		error = writeByte(*inDataPtr++);
	}

	error = writeByte(0xff);
	error = writeByte(0xff);

	error = readByte(&rcvByte);
	if ((rcvByte & 0x0F) != 0x05) {
		return (3);
	}

	do {
		error = readByte(&rcvByte);
	} while (rcvByte == 0x00);

	return (0);
}

// --------------------------------------------------------------------------

spiErr_t writePartialBlockBegin(gwUINT32 inBlockNumber) {

	spiErr_t error;
	SDArgumentType cmdArg;

	cmdArg.word = inBlockNumber << SD_BLOCK_SHIFT;
	if (sendCommandWithArg(eSDCardCmd24, cmdArg, eResponseOK)) {
	}
	error = writeByte(0xfe);

	return error;
}

// --------------------------------------------------------------------------

spiErr_t writePartialBlock(gwUINT8 *inDataPtr, gwUINT8 inBytes) {

	spiErr_t error;

	for (gwUINT8 counter = 0; counter < inBytes; counter++) {
		error = writeByte(*inDataPtr++);
	}

	return error;
}

// --------------------------------------------------------------------------

spiErr_t writePartialBlockEnd() {

	spiErr_t error;
	gwUINT8 rcvByte = 0;
	gwUINT16 counter;

	error = writeByte(0xff);
	error = writeByte(0xff);

	// Get the data write response.
	counter = SD_WAIT_CYCLES;
	do {
		error = readByte(&rcvByte);
		counter--;
	} while (((rcvByte & 0x10) == 0x10) && counter > 0);

	// Now we have the result, figure out what it is.
	if ((rcvByte & 0x0F) != 0x05) {
		return (3);
	}

	// Wait until the write operation is complete.
	do {
		error = readByte(&rcvByte);
	} while (rcvByte == 0x00);

	return error;
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
		SPI_WriteSync(0xFF000000);
	}
}

// --------------------------------------------------------------------------

gwUINT16 crc16(gwUINT16 inOldCRC, gwUINT8 inByte) {
	gwUINT16 crc;
	gwUINT16 x;

	x = ((inOldCRC  >> 8) ^ inByte) & 0xff;
	x ^= x >> 4;

	crc = (inOldCRC << 8) ^ (x << 12) ^ ( x << 5) ^ x;

	crc &= 0xffff;

	return crc;
}
