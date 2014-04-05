/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "aisleControllerTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"
#include "remoteMgmtTask.h"
#include "Ssi_Interface.h"
#include "GPIO_Interface.h"
#include "UartLowLevel.h"
#include "string.h"

#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

xTaskHandle gAisleControllerTask = NULL;
//xQueueHandle gAisleControllerQueue;

LedCycle gLedCycle = eLedCycleOff;

extern LedPositionType gTotalLedPositions;

extern LedDataStruct gLedFlashData[];
extern LedPositionType gCurLedFlashDataElement;
extern LedPositionType gTotalLedFlashDataElements;
extern LedPositionType gNextFlashLedPosition;

extern LedDataStruct gLedSolidData[];
extern LedPositionType gCurLedSolidDataElement;
extern LedPositionType gTotalLedSolidDataElements;
extern LedPositionType gNextSolidLedPosition;

// --------------------------------------------------------------------------
void gpioInit(void) {

	register uint32_t tmpReg;
	GpioErr_t gpioError;

	// Timer 1
	gpioError = Gpio_SetPinFunction(gGpioPin9_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin9_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin9_c, gGpioPinReadPad_c);

	// Timer 3
	gpioError = Gpio_SetPinFunction(gGpioPin11_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin11_c, gGpioDirOut_c);

	// SSI TX
	gpioError = Gpio_SetPinFunction(gGpioPin0_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin0_c, gGpioDirOut_c);

	// SSI RX
	gpioError = Gpio_SetPinFunction(gGpioPin1_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin1_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin1_c, gGpioPinReadPad_c);

	// SSI FSYNC
	gpioError = Gpio_SetPinFunction(gGpioPin2_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin2_c, gGpioDirOut_c);

	// SSI CLK
	gpioError = Gpio_SetPinFunction(gGpioPin3_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin3_c, gGpioDirOut_c);
}

// --------------------------------------------------------------------------

static void setupSsi() {

	SsiErr_t error;
	SsiConfig_t ssiConfig;
	SsiClockConfig_t ssiClockConfig;
	SsiTxRxConfig_t ssiTxRxConfig;

	SSI_Init();

	IntAssignHandler(gSsiInt_c, (IntHandlerFunc_t) ssiInterrupt);
	ITC_SetPriority(gSsiInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gSsiInt_c);

	SSI_Enable(FALSE);

	// Setup the SSI mode.
	ssiConfig.ssiGatedRxClockMode = FALSE;
	ssiConfig.ssiGatedTxClockMode = TRUE;
	ssiConfig.ssiMode = gSsiNormalMode_c; // Normal mode
	ssiConfig.ssiNetworkMode = FALSE; // Network mode
	ssiConfig.ssiInterruptEn = FALSE; // Interrupts enabled
	error = SSI_SetConfig(&ssiConfig);

	// Setup the SSI clock.
//	ssiClockConfig.ssiClockConfigWord = SSI_SET_BIT_CLOCK_FREQ(24000000, 500000);
	// Clock rate is clk = sys-f / (div + 1) * ((7 * PSR) + 1) * ((PM + 1) * 2)
	ssiClockConfig.bit.ssiDIV2 = 0x00;
	ssiClockConfig.bit.ssiPSR = 0x01; 
	ssiClockConfig.bit.ssiPM = 0x06;
	ssiClockConfig.bit.ssiDC = SSI_FRAME_LEN2; // Two words in each frame.  (Frame divide control.)
	ssiClockConfig.bit.ssiWL = SSI_24BIT_WORD; // 3 - 8 bits, 7 = 16 bits, 9 = 20 bits, b = 24 bits
	error = SSI_SetClockConfig(&ssiClockConfig);

	// Setup Tx.
	ssiTxRxConfig.ssiTxRxConfigWord = 0;
	ssiTxRxConfig.bit.ssiEFS = 0; // Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 0; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0; // Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0; // Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 1; // CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 1; // Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1; // Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1; // Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	ssiTxRxConfig.bit.ssiRxEXT = 0; // Receive sign extension: 0 = off, 1 = on.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeTx_c);

	// Setup Rx.
	ssiTxRxConfig.ssiTxRxConfigWord = 0;
	ssiTxRxConfig.bit.ssiEFS = 0; // Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 0; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0; // Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0; // Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 1; // CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 1; // Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1; // Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1; // Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	ssiTxRxConfig.bit.ssiRxEXT = 0; // Receive sign extension: 0 = off, 1 = on.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeRx_c);

	// Disable the last word of the frame.
	// This is because of some stupid error that doesn't allow us to reliably send out the last
	// word of the frame without causing an overrun.
	//SSI_STMSK = 0x00;
	//SSI_SRMSK = 0x00;

	SSI_SFCSR_BIT .RFWM0 = 4;
	SSI_SFCSR_BIT .TFWM0 = 4;

	SSI_SCR_BIT .TFR_CLK_DIS = 0;
	SSI_STCCR_BIT .DC = 0;

	// Setup the SSI interrupts.
	SSI_SIER_WORD = 0;
	SSI_SIER_BIT .RIE = FALSE;
	SSI_SIER_BIT .ROE_EN = FALSE;
	SSI_SIER_BIT .RFRC_EN = FALSE;
	SSI_SIER_BIT .RDR_EN = FALSE;
	SSI_SIER_BIT .RLS_EN = FALSE;
	SSI_SIER_BIT .RFS_EN = FALSE;
	SSI_SIER_BIT .RFF_EN = FALSE;

	SSI_SIER_BIT .TIE = TRUE;
	SSI_SIER_BIT .TDE_EN = FALSE;
	SSI_SIER_BIT .TFE_EN = TRUE;

	//SSI_STCR_BIT.TSCKP = 1;

	SSI_Enable(TRUE);
}

// --------------------------------------------------------------------------
void ssiInterrupt(void) {

	// The Tx FIFO queue is below the watermark, so provide more output data if we haven't transmitted all the bits yet.

	if (SSI_SISR_BIT .TFE) {
		if (gLedCycle == eLedCycleOff) {
			while ((SSI_SFCSR_BIT .TFCNT0 < 8) && (gNextSolidLedPosition < gTotalLedPositions)) {
				SSI_STX = getNextSolidData();
			}
			if (!(gNextSolidLedPosition < gTotalLedPositions)) {
				SSI_SIER_BIT .TFE_EN = FALSE;
			}
		} else {
			while ((SSI_SFCSR_BIT .TFCNT0 < 8) && (gNextFlashLedPosition < gTotalLedPositions)) {
				SSI_STX = getNextFlashData();
			}
			if (!(gNextFlashLedPosition < gTotalLedPositions)) {
				SSI_SIER_BIT .TFE_EN = FALSE;
			}
		}
	}
}

// --------------------------------------------------------------------------
void aisleControllerTask(void *pvParameters) {

/*
 * This routine lights the LED strips by serial clock driving the strip's WS2801 ICs.
 *
 * Each LED position has a WS2801 that clocks in 8 bits for each color: RGB.  You clock 24 bits onto the serial
 * bus to send the data to the first IC.  As you clock in 24 more bits the first IC's data shifts
 * out and into the next IC down the bus.  You do this until you've clocked in all of the data for all
 * of the LEDs on the bus.  When there is no clock for 500us then ICs all latch up their current values
 * and light the LED with it.  It ends up looking like a giant shift register.
 *
 * There are two user-visible LED cycles: on and off.  This allows for LED flashing: 500ms off, 250ms on.
 *
 * There are some LEDs that don't flash because they are solid.  For instance, work path separators. Solid
 * LEDs are always lit in both the on and off cycles.  If an LED position has both a solid and flashing
 * request then we show the solid value in the off cycle and the flash value in the on cycle.
 *
 * The LED data is 99% 0-bits clocked onto the bus.  It's not practical to hold the thousands of bytes/bits
 * in the MCU's limited memory.  Instead we keep an array of non-zero LEDs in position order.  The host sends
 * us this LED data in bus-position order.  E.g. 3, 4, 5, (blue) 70, 71, 72 (blue).
 *
 * We also may need to flash more than one color at a position.  (We may even need to flash several colors at
 * a position.)  To deal with these multiple flash sequences the host sends the LED data in bus-position order
 * and loop back to an lower position with a new sequence.  For each color we need to flash at the same position
 * there will be a separate sequence.  E.g. 3, 4, 5 (blue) 70, 71, 72 (blue) then 3, 4, 5 (green) 70, 71, 72 (blue).
 * That's two sequences: blue/blue, green/blue.
 *
 * Whenever the host sends us an LED position of -1 it means to wipe out all of the current sequences
 * and rebuild them from the new data that will arrive.
 *
 * To accomplish serial bus Tx in the MC13224v we use the SSI module in gated-clock mode.  We start the Tx process by
 * filling the Tx FIFO buffer with 8 bytes.  When the buffer goes below the low-water mark an interrupt
 * fires, and that ISR fills the Tx FIFO buffer with more data.  We continue to fill that buffer until we've
 * clocked out all of the data for the bus.  If there is no more data to clock out to the LED serial bus then
 * we don't put any more data in the FIFO during the ISR. (We also won't see the interrupt again until
 * we refill the buffer in the next cycle.)
 *
 */

gwUINT8 ccrHolder;

// Create some fake test data.
LedDataStruct ledData;
ledData.channel = 1;
ledData.red = 0x3e;
ledData.green = 0x0;
ledData.blue = 0x0;

int index = 0;
for (int errorSet = 0; errorSet < TOTAL_ERROR_SETS; ++errorSet) {
// Light the first LED in this set.
	ledData.position = errorSet * DISTANCE_BETWEEN_ERROR_LEDS;
	gLedFlashData[index++] = ledData;

// Light the last LED in this group.
	ledData.position = errorSet * DISTANCE_BETWEEN_ERROR_LEDS + (DISTANCE_BETWEEN_ERROR_LEDS - 1);
	gLedFlashData[index++] = ledData;
}

gLedCycle = eLedCycleOff;
gTotalLedPositions = TOTAL_ERROR_SETS * DISTANCE_BETWEEN_ERROR_LEDS;
gTotalLedFlashDataElements = index;
gTotalLedSolidDataElements = 0;

gpioInit();
setupSsi();

// Enable Rx, and disable Tx.
SSI_SCR_BIT .TE = TRUE;
SSI_SCR_BIT .RE = FALSE;

for (;;) {
	if (gLedCycle == eLedCycleOff) {
		// Write 8 words into the FIFO - that will cause the FIFO low watermark ISR to execute.
		gCurLedSolidDataElement = 0;
		gNextSolidLedPosition = 0;
		GW_ENTER_CRITICAL(ccrHolder);
		while ((SSI_SFCSR_BIT .TFCNT0 < 8) && (gNextSolidLedPosition < gTotalLedPositions)) {
			SSI_STX = getNextSolidData();
		}
		GW_EXIT_CRITICAL(ccrHolder);
		SSI_SIER_BIT .TFE_EN = TRUE;

		// A small chunk of the time will be used to service the ISR to complete the data transmission.
		vTaskDelay(LED_OFF_TIME);
		gLedCycle = eLedCycleOn;
	} else {
		// Write 8 words into the FIFO - that will cause the FIFO low watermark ISR to execute.
		gCurLedFlashDataElement = 0;
		gNextFlashLedPosition = 0;
		GW_ENTER_CRITICAL(ccrHolder);
		while ((SSI_SFCSR_BIT .TFCNT0 < 8) && (gNextFlashLedPosition < gTotalLedPositions)) {
			SSI_STX = getNextFlashData();
		}
		GW_EXIT_CRITICAL(ccrHolder);
		SSI_SIER_BIT .TFE_EN = TRUE;

		// A small chunk of the time will be used to service the ISR to complete the data transmission.
		vTaskDelay(LED_ON_TIME);
		gLedCycle = eLedCycleOff;
	}
}
}

// --------------------------------------------------------------------------
gwUINT32 getNextSolidData() {
// The default is to return zero for a blank LED.
ULedSampleType result;
result.word = 0x0;

// Check if there are any more solid LED values to display.
if (gCurLedSolidDataElement < gTotalLedSolidDataElements) {
// Check if the current LED data value matches the position and channel we want to light.
	if ((gLedSolidData[gCurLedSolidDataElement].position == gNextSolidLedPosition)
			&& (gLedSolidData[gCurLedSolidDataElement].channel == 1)) {
		result.bytes.byte1 = gLedSolidData[gCurLedSolidDataElement].red;
		result.bytes.byte2 = gLedSolidData[gCurLedSolidDataElement].green;
		result.bytes.byte3 = gLedSolidData[gCurLedSolidDataElement].blue;
		gCurLedSolidDataElement += 1;
	}
}
gNextSolidLedPosition += 1;
return result.word;
}

// --------------------------------------------------------------------------
gwUINT32 getNextFlashData() {
// The default is to return zero for a blank LED.
ULedSampleType result;
result.word = 0x0;

// Check if there are any more flash LED values to display.
if (gCurLedFlashDataElement < gTotalLedFlashDataElements) {

	if (gLedFlashData[gCurLedFlashDataElement].position < gNextFlashLedPosition) {
		// The next flash data element position is lower than the current LED then we've reached the end of a sequence.
		// We return zeros until we start back at the beginning of the LED strip in the next sequence.
	} else if ((gLedFlashData[gCurLedFlashDataElement].position == gNextFlashLedPosition)
			&& (gLedFlashData[gCurLedFlashDataElement].channel == 1)) {
		// The current LED data value matches the position and channel we want to light.
		result.bytes.byte1 = gLedFlashData[gCurLedFlashDataElement].red;
		result.bytes.byte2 = gLedFlashData[gCurLedFlashDataElement].green;
		result.bytes.byte3 = gLedFlashData[gCurLedFlashDataElement].blue;
		gCurLedFlashDataElement += 1;
	}
}
gNextFlashLedPosition += 1;
return result.word;
}
