/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "pfcTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "remoteMgmtTask.h"
#include "GPIO_Interface.h"

#define DATA0_INPUT			GPIO.DirResetLo		= 0x040000; GPIO.PuEnLo	= 0x040000; GPIO.PuSelLo = 0x040000;
#define DATA0_OUTPUT		GPIO.DirSetLo		= 0x040000
#define DATA0_ASSERT		GPIO.DataSetLo 		= 0x040000
#define DATA0_DEASSERT		GPIO.DataResetLo 	= 0x040000

#define DATA1_INPUT			GPIO.DirResetLo		= 0x080000; GPIO.PuEnLo	= 0x080000; GPIO.PuSelLo = 0x080000;
#define DATA1_OUTPUT		GPIO.DirSetLo		= 0x080000
#define DATA1_ASSERT		GPIO.DataSetLo 		= 0x080000
#define DATA1_DEASSERT		GPIO.DataResetLo 	= 0x080000

#define DATA2_INPUT			GPIO.DirResetLo		= 0x200000; GPIO.PuEnLo	= 0x200000; GPIO.PuSelLo = 0x200000;
#define DATA2_OUTPUT		GPIO.DirSetLo		= 0x200000
#define DATA2_ASSERT		GPIO.DataSetLo 		= 0x200000
#define DATA2_DEASSERT		GPIO.DataResetLo 	= 0x200000

#define DATA3_INPUT			GPIO.DirResetLo		= 0x100000; GPIO.PuEnLo	= 0x100000; GPIO.PuSelLo = 0x100000;
#define DATA3_OUTPUT		GPIO.DirSetLo		= 0x100000
#define DATA3_ASSERT		GPIO.DataSetLo 		= 0x100000
#define DATA3_DEASSERT		GPIO.DataResetLo 	= 0x100000

#define CARD_BUSY_ON		DATA0_OUTPUT; DATA0_DEASSERT; DATA3_OUTPUT; DATA3_DEASSERT
#define CARD_BUSY_OFF		DATA0_INPUT; DATA3_INPUT

#define WAIT_FRAME_ON		GPIO.DataSetLo 		= 0x000010
#define WAIT_FRAME_OFF		GPIO.DataResetLo 	= 0x000010
#define WRITE_FRAME_ON		GPIO.DataSetLo 		= 0x000020
#define WRITE_FRAME_OFF		GPIO.DataResetLo 	= 0x000020
#define START_FRAME_ON		GPIO.DataSetLo 		= 0x000040
#define START_FRAME_OFF		GPIO.DataResetLo 	= 0x000040
#define RESYNC_FRAME_ON		GPIO.DataSetLo 		= 0x000080
#define RESYNC_FRAME_OFF	GPIO.DataResetLo 	= 0x000080

#define SYNC_ERROR_ON		GPIO.DataSetLo 		= 0x001000
#define SYNC_ERROR_OFF		GPIO.DataResetLo 	= 0x001000
#define TX_FRAME_ON			GPIO.DataSetLo 		= 0x001000
#define TX_FRAME_OFF		GPIO.DataResetLo 	= 0x001000

// Globals

xTaskHandle 		gPFCTask = NULL;
xQueueHandle 		gPFCQueue;
ESDCardState 		gSDCardState;
ESDCardCmdState 	gSDCardCmdState;
ESDCardDataMode		gSDCardDataMode = eSDCardDataModeNarrow;

//USsiSampleType 	gSamples[110];
gwBoolean 			gIsTransmitting = FALSE;
gwUINT32			gBlockLength = 512;
gwUINT16			gRCA = 0x0000;
gwBoolean			gReadyForData = TRUE;

gwBoolean 			gSend13 = FALSE;

extern portTickType xTickCount;

// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {
	gwUINT8 samples;
	TmrErr_t error;

	if (gPFCQueue) {

		gpioInit();
		setupSSI();
		setupTimers();
		gSDCardState = eSDCardStateIdle;
		gSDCardCmdState = eSDCardCmdStateStd;

		// Enable Rx, and disable Tx.
		SSI_SCR_BIT.TE = FALSE;
		SSI_SCR_BIT.RE = TRUE;

		// Start the FSYNC simulation timer.
		//error = TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrEdgSecSrcTriggerPriCntTillComp_c);
		resync(FSYNC_TIMER);

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

	register uint32_t tmpReg;
	GpioErr_t error;

	// Pull-up select: UP type
	//GPIO.PuSelLo |= (GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);
	// Pull-up enable
	//GPIO.PuEnLo  |= (GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);
	// Data select sets these ports to read from pads.
	GPIO.InputDataSelLo &= ~(GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);
	// inputs
	GPIO.DirResetLo = (GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit);
	// outputs
	GPIO.DirSetLo = (GPIO_TIMER3_INOUT_bit | GPIO_SSI_TX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);

	// Setup the function enable pins.
	tmpReg = GPIO.FuncSel0 & ~((FN_MASK << GPIO_TIMER1_INOUT_fnpos) | (FN_MASK << GPIO_TIMER3_INOUT_fnpos) | (FN_MASK
	        << GPIO_SSI_TX_fnpos) | (FN_MASK << GPIO_SSI_RX_fnpos) | (FN_MASK << GPIO_SSI_FSYNC_fnpos) | (FN_MASK
	        << GPIO_SSI_CLK_fnpos));
	GPIO.FuncSel0 = tmpReg | ((FN_ALT << GPIO_TIMER1_INOUT_fnpos) | (FN_ALT << GPIO_TIMER3_INOUT_fnpos) | (FN_ALT
	        << GPIO_SSI_TX_fnpos) | (FN_ALT << GPIO_SSI_RX_fnpos) | (FN_ALT << GPIO_SSI_FSYNC_fnpos) | (FN_ALT
	        << GPIO_SSI_CLK_fnpos));

	error = Gpio_SetPinDir(gGpioPin4_c, gGpioDirOut_c);
	error = Gpio_SetPinDir(gGpioPin5_c, gGpioDirOut_c);
	error = Gpio_SetPinDir(gGpioPin6_c, gGpioDirOut_c);
	error = Gpio_SetPinDir(gGpioPin7_c, gGpioDirOut_c);
	error = Gpio_SetPinDir(gGpioPin12_c, gGpioDirOut_c);

	DATA0_INPUT;
	DATA1_INPUT;
	DATA2_INPUT;
	DATA3_INPUT;
}

// --------------------------------------------------------------------------
/*
 * This routine sets up timer 3 to generate a frame sync (FS) signal for an commands coming from the
 * SD card host controller on the CMD line.  When we see a negative edge on the CMD line (secondary count source)
 * it asserts timer 3 output (the FS pin) and counts the right number of primary source (BusClock /128) clocks
 * until the right time to turn off the timer 3 output (FS pin).
 *
 * Primary counter clock is the bus clock divided by 128.
 * Secondary counter source is the SD card CMD line.
 */
void setupTimers() {

	TmrErr_t error;
	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	// Enable the timers.
	error = TmrEnable(FSYNC_TIMER);
	error = TmrEnable(RXTX_TIMER);

	// Don't run it yet.
	error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
	error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);

	// Register the callback executed when a timer interrupt occurs.
//	error = TmrSetCallbackFunction(FCS_TIMER, gTmrEdgeEvent_c, resync);
//	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrOverEvent_c, startFrame);
//	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrCompEvent_c, startFrame);
//	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrComp1Event_c, startFrame);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.TCFIE = FALSE; // Timer compare IE.
	tmrStatusCtrl.bitFields.TOFIE = FALSE; // Timer overflow IE.
	tmrStatusCtrl.bitFields.IEFIE = FALSE; // Input edge flag IE.
	tmrStatusCtrl.bitFields.IPS = 1; // Input polarity select: 0 = normal, 1 = inverted.
	tmrStatusCtrl.bitFields.CAPMODE = 0; // Capture mode.
	tmrStatusCtrl.bitFields.MSTR = FALSE; // Master mode enable.
	tmrStatusCtrl.bitFields.EEOF = FALSE; // External enable OFLAG.
	tmrStatusCtrl.bitFields.OPS = 1; // Output polarity: 0 = normal, 1 = inverted.
	tmrStatusCtrl.bitFields.OEN = TRUE; // Output enable.
	error = TmrSetStatusControl(FSYNC_TIMER, &tmrStatusCtrl);

	tmrStatusCtrl.bitFields.IEFIE = TRUE; // Input edge flag IE.
	tmrStatusCtrl.bitFields.OEN = FALSE; // Output enable.
	error = TmrSetStatusControl(RXTX_TIMER, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;	// Debug enable.
	tmrComparatorStatusCtrl.bitFields.TCF1EN = TRUE; // Timer compare1 IE.
	tmrComparatorStatusCtrl.bitFields.TCF2EN = FALSE; // Timer compare2 IE.
	tmrComparatorStatusCtrl.bitFields.CL1 = 0x01; // Compare load control 1.
	tmrComparatorStatusCtrl.bitFields.FILT_EN = FALSE; // Filter enable.
	error = TmrSetCompStatusControl(FSYNC_TIMER, &tmrComparatorStatusCtrl);

	tmrComparatorStatusCtrl.bitFields.TCF1EN = FALSE; // Timer compare1 IE.
	error = TmrSetCompStatusControl(RXTX_TIMER, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrSetOF_c;//gTmrSetOnCompClearOnSecInputEdg_c;
	tmrConfig.tmrCoInit = 0; // Co-init: 0 = another channel cannot init this timer.
	tmrConfig.tmrCntDir = 0; // Count dir: 0 = up, 1 = down.
	tmrConfig.tmrCntLen = 1; // Count length: 0 = roll over, 1 = until compare then reinit
	tmrConfig.tmrCntOnce = TRUE; // Count once: 0 = repeatedly, 1 = once only.
	tmrConfig.tmrSecondaryCntSrc = FSYNC_SECONDARY_SOURCE;
	tmrConfig.tmrPrimaryCntSrc = FSYNC_PRIMARY_SOURCE;
	error = TmrSetConfig(FSYNC_TIMER, &tmrConfig);

	tmrConfig.tmrSecondaryCntSrc = FSYNC_SECONDARY_SOURCE;
	tmrConfig.tmrPrimaryCntSrc = FSYNC_PRIMARY_SOURCE;
	error = TmrSetConfig(RXTX_TIMER, &tmrConfig);

	// Config timer to start from 0 after compare event.
	SetLoadVal(FSYNC_TIMER, 0);
	SetLoadVal(RXTX_TIMER, 0);

	// Start the counter at 0.
	SetCntrVal(FSYNC_TIMER, 0);
	SetCntrVal(RXTX_TIMER, 0);
}

// --------------------------------------------------------------------------
/*
 * We are waiting until the Rx/Tx signal is held high at least 48 clock cycles.
 * The only time we would see Rx/Tx high for that long is between commands.
 */

static void resync(TmrNumber_t tmrNumber) {

	gwUINT8 ccr;
	TmrErr_t error;
	TmrConfigReg_t *configP;
	TmrStatusCtrl_t *statusP;
	TmrComparatorStatusCtrl_t *csstatusP;

	GW_ENTER_CRITICAL(ccr);
	RESYNC_FRAME_ON;

	configP = (TmrConfigReg_t*)&TMR3_REGS_P->Ctrl;
	statusP = (TmrStatusCtrl_t*)&TMR3_REGS_P->StatCtrl;
	csstatusP = (TmrComparatorStatusCtrl_t*)&TMR3_REGS_P->CompStatCtrl;

	// Disable/reset the SSI port.
	SSI_Enable(FALSE);
	SSI_SCR_BIT.RE = FALSE;
	SSI_SCR_BIT.TE = FALSE;
	SSI_SIER_BIT.RIE = TRUE;
	SSI_Enable(TRUE);

	error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
	error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);

	error = TmrSetCallbackFunction(FSYNC_TIMER, gTmrEdgeEvent_c, resync);
	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrComp1Event_c, waitForNextFrame);

	SetComp1Val(FSYNC_TIMER, RXTX_TRIGGER_RESYNC);
	SetCompLoad1Val(FSYNC_TIMER, RXTX_TRIGGER_RESYNC);

	SetComp1Val(RXTX_TIMER, RXTX_TRIGGER_RESYNC);
	SetCompLoad1Val(RXTX_TIMER, RXTX_TRIGGER_RESYNC);

	TMR3_CTRL_BIT.tmrOutputMode = gTmrSetOF_c;
	TMR3_SCTRL_BIT.VAL = 1;
	TMR3_SCTRL_BIT.FORCE = 1;
	TMR3_SCTRL_BIT.IEFIE = TRUE;
	TMR3_SCTRL_BIT.IEF = FALSE;

	TMR1_CSCTRL_BIT.TCF1EN = TRUE;
	TMR1_CSCTRL_BIT.TCF1 = FALSE;

	error = TmrSetMode(RXTX_TIMER, gTmrCntRiseEdgPriSrc_c);
	error = TmrSetMode(FSYNC_TIMER, gTmrCntRiseEdgPriSrc_c);

	RESYNC_FRAME_OFF;
	GW_EXIT_CRITICAL(ccr);

}

// --------------------------------------------------------------------------
/*
 * Now we know we are between commands, so prepare for the next command frame to arrive.
 */

static void waitForNextFrame(TmrNumber_t tmrNumber) {

	gwUINT8 ccr;
	TmrErr_t error;

	GW_ENTER_CRITICAL(ccr);
	WAIT_FRAME_ON;

	error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
	error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);

	if (gIsTransmitting) {
		gIsTransmitting = FALSE;
	}
	SSI_SCR_BIT.RE = TRUE;

	error = TmrSetCallbackFunction(FSYNC_TIMER, gTmrComp1Event_c, startFrame);
	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrComp1Event_c, NULL);

	// Reestablish the edge trigger timer for the next command.
	TMR3_CTRL_BIT.tmrOutputMode = gTmrSetOnCompClearOnSecInputEdg_c;
	TMR3_SCTRL_BIT.VAL = 1;
	TMR3_SCTRL_BIT.FORCE = 1;
	TMR3_SCTRL_BIT.IEFIE = FALSE;
	TMR3_SCTRL_BIT.IEF = FALSE;
	TMR3_SCTRL_BIT.IEFIE = FALSE;
	TMR3_CSCTRL_BIT.TCF1 = FALSE;
	TMR3_CSCTRL_BIT.TCF1EN = TRUE;

	SetComp1Val(FSYNC_TIMER, FSYNC_TRIGGER_HIGH);
	SetCompLoad1Val(FSYNC_TIMER, FSYNC_TRIGGER_HIGH);

	error = TmrSetMode(FSYNC_TIMER, gTmrEdgSecSrcTriggerPriCntTillComp_c);
	//error = TmrSetMode(RXTX_TIMER, gTmrCntRiseEdgPriSrc_c);

	WAIT_FRAME_OFF;
	GW_EXIT_CRITICAL(ccr);
}


// --------------------------------------------------------------------------
/*
 * We've received a command frame, but if we don't process a "read" in that time
 * then the trigger will fire, causing us to attempt to resync with the command stream.
 */
static void startFrame(TmrNumber_t tmrNumber) {

	gwUINT8 ccr;
	TmrErr_t error;

	GW_ENTER_CRITICAL(ccr);
	START_FRAME_ON;

	error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
	error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);

	error = TmrSetCallbackFunction(FSYNC_TIMER, gTmrComp1Event_c, NULL);
	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrComp1Event_c, resync);

	TMR3_SCTRL_BIT.VAL = 1;
	TMR3_SCTRL_BIT.FORCE = 1;

	TMR1_CSCTRL_BIT.TCF1 = FALSE;
	TMR1_CSCTRL_BIT.TCF1EN = TRUE;

	SetCntrVal(RXTX_TIMER, 0);
	SetComp1Val(RXTX_TIMER, RXTX_TIMEOUT);
	SetCompLoad1Val(RXTX_TIMER, RXTX_TIMEOUT);

	error = TmrSetMode(RXTX_TIMER, gTmrCntRiseEdgPriSrc_c);

	START_FRAME_OFF;
	GW_EXIT_CRITICAL(ccr);
}

// --------------------------------------------------------------------------
/*
 * We receive SD Card commands using the SSI network mode, because it allows us to specify a frame
 * that has two time slots of 24-bit words, we can read an entire SD command of 48 bits in one frame.
 *
 * We generate a frame signal using an internal timer. That timer gets triggered on the
 * start bit of the CMD line taking the line from high (Z pulled high by a pullup) to low.
 * That edge starts a frame.  We then read the next two time slots of 24-bit words (each) for a total of 48 bits.
 */

static void setupSSI() {

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
	ssiConfig.ssiGatedTxClockMode = FALSE;
	ssiConfig.ssiMode = gSsiNormalMode_c; // Normal mode
	ssiConfig.ssiNetworkMode = TRUE; // Network mode
	ssiConfig.ssiInterruptEn = TRUE; // Interrupts enabled
	error = SSI_SetConfig(&ssiConfig);

	// Setup the SSI clock.
	ssiClockConfig.ssiClockConfigWord = SSI_SET_BIT_CLOCK_FREQ(24000000, 400000);
	ssiClockConfig.bit.ssiDC = SSI_FRAME_LEN2; // Two words in each frame.  (Frame divide control.)
	ssiClockConfig.bit.ssiWL = SSI_24BIT_WORD; // 3 - 8 bits, 7 = 16 bits, 9 = 20 bits, b = 24 bits
	error = SSI_SetClockConfig(&ssiClockConfig);

	// Setup Tx.
	ssiTxRxConfig.ssiTxRxConfigWord = 0;
	ssiTxRxConfig.bit.ssiEFS = 0; // Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 0; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 1; // Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0; // Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 0; // CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 0; // Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1; // Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1; // Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeTx_c);

	// Setup Rx.
	ssiTxRxConfig.ssiTxRxConfigWord = 0;
	ssiTxRxConfig.bit.ssiEFS = 0; // Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 1; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0; // Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0; // Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 0; // CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 0; // Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1; // Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1; // Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	ssiTxRxConfig.bit.ssiRxEXT = 0; // Receive sign extension: 0 = off, 1 = on.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeRx_c);

	// Disable the last word of the frame.
	// This is because of some stupid error that doesn't allow us to reliably send out the last
	// word of the frame without causing an overrun.
	SSI_STMSK = 0x00;
	SSI_SRMSK = 0x00;

	SSI_SFCSR_BIT.RFWM0 = 2;
	SSI_SFCSR_BIT.TFWM0 = 8;

	SSI_Enable(TRUE);

	// Setup the SSI interrupts.
	SSI_SIER_WORD = 0;
	SSI_SIER_BIT.RIE = TRUE;
	SSI_SIER_BIT.ROE_EN = FALSE;
	SSI_SIER_BIT.RFRC_EN = FALSE;
	SSI_SIER_BIT.RDR_EN = FALSE;
	SSI_SIER_BIT.RLS_EN = FALSE;
	SSI_SIER_BIT.RFS_EN = FALSE;
	SSI_SIER_BIT.RFF_EN = TRUE;

	SSI_SIER_BIT.TIE = FALSE;
	SSI_SIER_BIT.TDE_EN = FALSE;
	SSI_SIER_BIT.TFE_EN = FALSE;

	// Setup the card as not busy, and not sending data.
	CARD_BUSY_OFF;
//	DATA1_ASSERT;
//	DATA2_ASSERT;
//	DATA3_ASSERT;

}

// --------------------------------------------------------------------------
void ssiInterrupt(void) {

	/*
	 *
	 * Extract the command from the SSI Rx FIFO by reading 2 samples out of the FIFO.
	 * Only the LSB 24-bits of each sample will contain valid data extracted from the command line.
	 *
	 * The SD commands map to the two bytes like this:
	 *
	 * sample 1:	XXXXXXXX|SHCCCCCC|AAAAAAAA|AAAAAAAA
	 *				--------|SH CMD  |arg byte|arg byte
	 * sample 2:	XXXXXXXX|AAAAAAAA|AAAAAAAA|CCCCCCCS
	 * 				--------|arg byte|arg byte|CRC    S
	 */
	TmrErr_t error;
	USsiSampleType cmdSample[10];
	SsiISReg_t *intStatusesP;
	SsiFCSReg_t *fcsValuesP;
	gwUINT8 ccr;

	GW_ENTER_CRITICAL(ccr);
	intStatusesP = ((SsiISReg_t*) &SSI_REGS_P->SISR);
	fcsValuesP =  ((SsiFCSReg_t*) &SSI_REGS_P->SFCSR);

	// Deal with the end of the Rx cycle.
	WRITE_FRAME_ON;
	if (SSI_SISR_BIT.RFF) {

		error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);
		error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
		SSI_SCR_BIT.RE = FALSE;

		while (SSI_SFCSR_BIT.RFCNT0 > 1) {

			cmdSample[0].word = SSI_SRX;
			cmdSample[1].word = SSI_SRX;

			// Check that the next two samples have a valid CRC.
			// Make sure it's a valid host command by verifying that command bits S=0 and H=1.
			// There's a weird case where with some cards the clock turns off then on before a CMD0,
			// but it doesn't turn on soon enough to catch a proper CMD0.
			// We see these malformed CMD0s as shifted one bit: 0x20 instead of 0x40.
			if ((cmdSample[1].bytes.byte3 != crc7(cmdSample, 2))
					&& (cmdSample[0].bytes.byte1 != 0x20)
					&& ((cmdSample[0].bytes.byte1 & 0x40) != 0x40)) {
				// We have an invalid CRC.  Give up on cmdSample[0], and go back to get some more data.
				SYNC_ERROR_ON;
				resync(FSYNC_TIMER);
				SYNC_ERROR_OFF;
			} else {
				// See the above test for why we check for 0x20 as a command.
				ESDCardCommand cmdNum = eSDCardCmdInvalid;
				if ((cmdSample[0].bytes.byte1 != 0x20)) {
					cmdNum = cmdSample[0].bytes.byte1 & 0x3F;
				} else {
					cmdNum = eSDCardCmd0;
				}
				ESDCardResponseType responseType;

				// Generate the response command.
				switch (cmdNum) {
					case eSDCardCmd0:
						responseType = eSDCardRespTypeNone;
						gSDCardState = eSDCardStateIdle;
						gSDCardDataMode = eSDCardDataModeNarrow;
						gRCA = 0x0000;
						break;

					case eSDCardCmd1:
						//responseType = eSDCardRespType3;
						responseType = eSDCardRespTypeNone;
						break;

					case eSDCardCmd2:
						responseType = eSDCardRespType2;
						gSDCardState = eSDCardStateIdent;
						break;

					case eSDCardCmd3:
						responseType = eSDCardRespType6;
						gSDCardState = eSDCardStateStandby;
						// Pick a wonky number that's easy to see on the logic analyzer.
						gRCA = 0xfe7f;
						break;

					case eSDCardCmd5:
						responseType = eSDCardRespTypeNone;
						break;

					case eSDCardCmd6:
						responseType = eSDCardRespType1;
						// If bits 0 and 1 of the AC are both 1 then the host is asking for wide mode.
						if ((cmdSample[1].word && 0x000200) == 0x000200) {
							gSDCardDataMode = eSDCardDataModeWide;
						} else {
							gSDCardDataMode = eSDCardDataModeNarrow;
						}
						break;

					case eSDCardCmd7:
						responseType = eSDCardRespType1b;
						if (gSDCardState == eSDCardStateTransfer) {
							gSDCardState = eSDCardStateStandby;
						} else {
							gSDCardState = eSDCardStateTransfer;
						}
						break;

					case eSDCardCmd9:
						responseType = eSDCardRespType2;
						gSDCardState = eSDCardStateStandby;
						break;

					case eSDCardCmd13:
						responseType = eSDCardRespType1;
						//responseType = eSDCardRespTypeNone;
						CARD_BUSY_OFF;
						break;

					case eSDCardCmd16:
						gBlockLength = (cmdSample[0].word << 16) + (cmdSample[1].word >> 8);
						// Weird, this command comes when the clock stops.
						// Don't try to respond - just wait for the next command.
						//responseType = eSDCardRespType1;
						responseType = eSDCardRespTypeNone;
						CARD_BUSY_ON;
						break;

					case eSDCardCmd41:
						responseType = eSDCardRespType3;
						gSDCardState = eSDCardStateReady;
						break;

					case eSDCardCmd55:
						// Indicate that we're in the Application command state.
						gSDCardCmdState = eSDCardCmdStateApp;
						responseType = eSDCardRespType1;
						break;

					default:
						// Invalid command.
						responseType = eSDCardRespTypeInvalid;
				}

				// Create and send the command response.
				if ((responseType == eSDCardRespTypeInvalid) || (responseType == eSDCardRespTypeNone)) {
				} else {
					if (cmdNum == eSDCardCmd13) {
						if (gSend13) {
							sendCmdResponse(eSDCardCmd13, eSDCardRespType1);
							gSend13 = FALSE;
						} else {
							sendCmdResponse(eSDCardCmd5, eSDCardRespType1);
							gSend13 = TRUE;
						}
					} else {
						sendCmdResponse(cmdNum, responseType);
					}
					SSI_SIER_BIT.RIE = TRUE;
				}
			}
		}
		waitForNextFrame(FSYNC_TIMER);
	}
	WRITE_FRAME_OFF;
	GW_EXIT_CRITICAL(ccr);
}

// --------------------------------------------------------------------------

void sendCmdResponse(ESDCardCommand inCmdNum, ESDCardResponseType inResponseType) {

	USsiSampleType cmdSample[10];
	gwBoolean longFrame;
	gwUINT32 maxLoops;

	longFrame = FALSE;
	switch (inResponseType) {
		case eSDCardRespType1:
		case eSDCardRespType1b:
			cmdSample[0].word = 0x00000000;
			cmdSample[0].bytes.byte1 = inCmdNum;

			// Compute and set the card's statuses for the reply command.
			cmdSample[1].word = 0x00000000;
			cmdSample[1].bytes.byte1 = (gSDCardState << 1) + gReadyForData;
			cmdSample[1].bytes.byte2 = gSDCardCmdState << 5;
			cmdSample[1].bytes.byte3 = crc7(cmdSample, 2);

			if (inCmdNum == eSDCardCmd13) {
				cmdSample[0].bytes.byte3 |= 0x08;
			}

			// Put the response into the SSI Tx FIFO.
			SSI_STX = cmdSample[0].word;
			SSI_STX = cmdSample[1].word;
			break;
		case eSDCardRespType2:
			// The CID response (R2) is longer than normal: 136 bits.
			// Expand the SSI frame to 6 words (of 24 bits each).
			// This is 8 bits more than we need, but it simplifies dealing with the FIFO.
			longFrame = TRUE;
			SSI_Enable(FALSE);
			SSI_STCCR_BIT.WL = SSI_20BIT_WORD;
			SSI_STCCR_BIT.DC = SSI_FRAME_LEN7;
			SSI_Enable(TRUE);

			maxLoops = 0;
			while (maxLoops < 80) {
				maxLoops++;
			}

			if (inCmdNum == eSDCardCmd2) {
				// CID - See separate document.
				SSI_STX = 0x0003f0a4;
				SSI_STX = 0x00075750;
				SSI_STX = 0x00046432;
				SSI_STX = 0x00002001;
				SSI_STX = 0x00000000;
				SSI_STX = 0x00000100;
				SSI_STX = 0x0009c89f;
			} else {
				// CSD - See separate document.
				SSI_STX = 0x0003f003;
				SSI_STX = 0x00060029;
				SSI_STX = 0x00017598;
				SSI_STX = 0x00000676;
				SSI_STX = 0x000da400;
				SSI_STX = 0x00008640;
				SSI_STX = 0x0000025f;
			}
			break;
		case eSDCardRespType3:
			// Initial value: start bit = 0, host bit = 0, cmd = 100101, busy bit = 0, 1/2 of OCR (at all voltages);
			cmdSample[0].word = 0x003f8030;
			cmdSample[1].word = 0x000000ff;
			// Put the response into the SSI Tx FIFO.
			SSI_STX = cmdSample[0].word;
			SSI_STX = cmdSample[1].word;
			break;

		case eSDCardRespType6:
			cmdSample[0].word = 0x00030000;
			cmdSample[1].word = 0x00000000;

			// Add the RCA to the response.
			cmdSample[0].word |= gRCA;
			cmdSample[1].word |= gSDCardState << 17;
			cmdSample[1].word |= 1 << 16;

			cmdSample[1].bytes.byte3 = crc7(cmdSample, 2);

			// Put the response into the SSI Tx FIFO.
			SSI_STX = cmdSample[0].word;
			SSI_STX = cmdSample[1].word;
			break;
	}

	// If the response we're just about to send is not the APP_COMMAND response
	// then return to "standard" command mode.
	if (inCmdNum != eSDCardCmd55) {
		gSDCardCmdState = eSDCardCmdStateStd;
	}

	gIsTransmitting = TRUE;
	// Reset the frame complete flags.
	SSI_SISR_BIT.RFRC = TRUE;
	SSI_SISR_BIT.TFRC = TRUE;
	gwUINT32 items = SSI_SFCSR_BIT.TFCNT0;

	// Start Tx.
	SSI_SIER_BIT.RIE = FALSE;
	SSI_SCR_BIT.TE = TRUE;

	// A slight delay before we assert fysnc.
	// This allows the MCU to catch up after enabling TE.
	maxLoops = 0;
	while (maxLoops < 105) {
		maxLoops++;
	}

	TMR3_SCTRL_BIT.OPS = 0;
	//TX_FRAME_ON;

	// Wait until something goes out.
	maxLoops = 0;
	while ((maxLoops < 5000) && (SSI_SFCSR_BIT.TFCNT0 == items)) {
		// Wait until a Tx word goes out, or we timeout.
		maxLoops++;
	}
	TMR3_SCTRL_BIT.OPS = 1;

	maxLoops = 0;
	while (maxLoops < 100) {
		maxLoops++;
	}

	SSI_SCR_BIT.TE = FALSE;
	GPIO.DataResetLo = 0x1000;

	// Prepare for completion of Tx.
	//TX_FRAME_OFF;
	maxLoops = 0;
	if (longFrame) {
		while ((maxLoops < 500) && (!SSI_SISR_BIT.RFRC)) {
			// Wait until a Tx word goes out, or we timeout.
			maxLoops++;
		}
	} else {
		while ((maxLoops < 500) && (!SSI_SISR_BIT.TFRC)) {
			// Wait until a Tx word goes out, or we timeout.
			maxLoops++;
		}
	}
	GPIO.DataResetLo = 0x1000;

	if (longFrame) {
		SSI_Enable(FALSE);
		SSI_STCCR_BIT.WL = SSI_24BIT_WORD;
		SSI_STCCR_BIT.DC = SSI_FRAME_LEN2;
		SSI_Enable(TRUE);
	} else {
	}
	// Clear out the garbage samples read by the SSI during Tx (even tho' it shouldn't).
	while (SSI_SFCSR_BIT.RFCNT0 > 0) {
		cmdSample[9].word = SSI_SRX;
	}

	//					if (inCmdNum == eSDCardCmd13) {
	//						DATA0_OUTPUT;
	//						DATA1_OUTPUT;
	//						DATA2_OUTPUT;
	//						DATA3_OUTPUT;
	//						for (int i = 0; i < 256; i++) {
	//							if (i & 0x01) {
	//								DATA0_ASSERT;
	//								DATA1_ASSERT;
	//								DATA2_ASSERT;
	//								DATA3_ASSERT;
	//							} else {
	//								DATA0_DEASSERT;
	//								DATA1_DEASSERT;
	//								DATA2_DEASSERT;
	//								DATA3_DEASSERT;
	//							}
	//							maxLoops = 0;
	//							while (maxLoops++ < 5) {
	//
	//							}
	//						}
	//					}

}

// --------------------------------------------------------------------------

//void restartReadCycle() {
//	TmrErr_t error;
//
//	gIsTransmitting = FALSE;
//	error = TmrSetMode(FCS_TIMER, gTmrNoOperation_c);
//
//	// Reestablish the edge trigger timer for the next command.
//	TMR3_CTRL_BIT.tmrOutputMode = gTmrSetOnCompClearOnSecInputEdg_c;
//
//	SetComp1Val(FCS_TIMER, FSYNC_TRIGGER_HIGH);
//	SetCompLoad1Val(FCS_TIMER, FSYNC_TRIGGER_HIGH);
//
//	TMR3_SCTRL_BIT.VAL = 1;
//	TMR3_SCTRL_BIT.FORCE = 1;
//
//	SSI_SCR_BIT.RE = TRUE;
//	error = TmrSetMode(FCS_TIMER, gTmrEdgSecSrcTriggerPriCntTillComp_c);
//}

// --------------------------------------------------------------------------

gwUINT8 crc7(USsiSampleType *inSamplePtr, gwUINT8 inSamplesToCheck) {
	gwUINT8 i;
	gwUINT8 sampleNum;
	gwUINT8 dataByteNum;
	gwUINT8 crc;
	gwUINT8 data;

	crc = 0;
	// Check each sample.
	for (sampleNum = 0; sampleNum < inSamplesToCheck; sampleNum++) {
		// Check each byte of each sample.
		for (dataByteNum = 0; dataByteNum < 3; dataByteNum++) {
			// The last byte of the last sample is not part of the CRC.
			if ((sampleNum < (inSamplesToCheck - 1)) || (dataByteNum < 2)) {
				data = ((gwUINT8 *) (inSamplePtr + sampleNum))[2 - dataByteNum];

				// Use this next data byte in the CRC.
				for (i = 0; i < 8; i++) {
					crc <<= 1;
					if ((data & 0x80) ^ (crc & 0x80))
						crc ^= 0x09;
					data <<= 1;
				}
			}
		}
	}
	crc = (crc << 1) | 1;
	return (crc);
}
