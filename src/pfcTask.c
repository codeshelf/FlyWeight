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

// Globals

xTaskHandle 		gPFCTask = NULL;
xQueueHandle 		gPFCQueue;
ESDCardState 		gSDCardState;
ESDCardCmdState 	gSDCardCmdState;

USsiSampleType 		gSamples[110];
gwUINT8 			gSampleCnt = 0;
gwBoolean 			gSyncLost = FALSE;
gwBoolean 			gIsTransmitting = FALSE;
portTickType		gTimerValStart;
portTickType		gTimerValStop;

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

	gTimerValStart = GetCntrVal(gTmr0_c);//xTickCount;vTaskGetTickCount();

	error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
	error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);

	error = TmrSetCallbackFunction(FSYNC_TIMER, gTmrComp1Event_c, NULL);
	error = TmrSetCallbackFunction(RXTX_TIMER, gTmrComp1Event_c, resync);

	TMR3_SCTRL_BIT.VAL = 1;
	TMR3_SCTRL_BIT.FORCE = 1;

	TMR1_CSCTRL_BIT.TCF1 = FALSE;
	TMR1_CSCTRL_BIT.TCF1EN = TRUE;

	SetComp1Val(RXTX_TIMER, RXTX_TIMEOUT);
	SetCompLoad1Val(RXTX_TIMER, RXTX_TIMEOUT);

	error = TmrSetMode(RXTX_TIMER, gTmrCntRiseEdgPriSrc_c);

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
	ssiTxRxConfig.bit.ssiFSL = 1; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0; // Data clocked: 0 = on rising edge, 1 = falling edge.
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

	// We only use two time slots in each frame.
	SSI_STMSK = 0x00;
	SSI_SRMSK = 0x00;

	SSI_SFCSR_BIT.RFWM0 = 2;
	SSI_SFCSR_BIT.TFWM0 = 0;

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
}
// --------------------------------------------------------------------------
gwBoolean lastCmd2 = FALSE;
gwBoolean lastCmd3 = FALSE;
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
	gwUINT8 ccr;
	gwBoolean longFrame = FALSE;

	GW_ENTER_CRITICAL(ccr);
	intStatusesP = ((SsiISReg_t*) &SSI_REGS_P->SISR);

	// Deal with the end of the Rx cycle.
	if (SSI_SISR_BIT.RFF /*|| (SSI_SISR_BIT.RDR)*/) {

		error = TmrSetMode(RXTX_TIMER, gTmrNoOperation_c);
		error = TmrSetMode(FSYNC_TIMER, gTmrNoOperation_c);
		SSI_SCR_BIT.RE = FALSE;

		while (SSI_SFCSR_BIT.RFCNT0 > 1) {

			// If we previously lost sync, then the first sample is held over, and we just need to read the second.
//			if (gSyncLost) {
//				cmdSample[1].word = SSI_SRX;
//				gSamples[gSampleCnt++].word = cmdSample[1].word;
//				gSyncLost = FALSE;
//			} else {
				cmdSample[0].word = SSI_SRX;
				cmdSample[1].word = SSI_SRX;
				gSamples[gSampleCnt++].word = cmdSample[0].word;
				gSamples[gSampleCnt++].word = cmdSample[1].word;
				if (gSampleCnt > 100) {
					gSampleCnt = 0;
				}
//			}
			// Check that the next two samples have a valid CRC.
			if (cmdSample[1].bytes.byte3 != crc7(cmdSample, 2)) {
				// We have an invalid CRC.  Give up on cmdSample[0], and go back to get some more data.
//				cmdSample[0].word = cmdSample[1].word;
//				gSyncLost = TRUE;
				resync(FSYNC_TIMER);
			} else {

				// Make sure it's a valid host command by verifying that command bits S=0 and H=1.
				if ((cmdSample[0].bytes.byte1 & 0xC0) == 0x40) {
					ESDCardCommand cmdNum = cmdSample[0].bytes.byte1 & 0x3F;
					ESDCardCommand responseCmd = cmdNum;
					ESDCardResponseType responseType;

					// Generate the response command.
					switch (cmdNum) {
						case eSDCardCmd0:
							responseType = eSDCardRespTypeNone;
							break;

						case eSDCardCmd1:
							//responseType = eSDCardRespType3;
							responseType = eSDCardRespTypeNone;
							break;

						case eSDCardCmd2:
							responseType = eSDCardRespType3;
							lastCmd2 = TRUE;
							break;

						case eSDCardCmd3:
							responseType = eSDCardRespType6;
							break;

						case eSDCardCmd41:
							responseType = eSDCardRespType3;
							//responseType = eSDCardRespTypeNone;
							break;

						case eSDCardCmd55:
							// Indicate that we're in the Application command state.
							gSDCardCmdState = eSDCardCmdStateApp;
							responseType = eSDCardRespType1;
							//responseType = eSDCardRespTypeNone;
							break;

						default:
							// Invalid command.
							responseType = eSDCardRespTypeInvalid;
							responseCmd = eSDCardCmdInvalid;

							//						if (gSampleCnt < 50) {
							//							gSamples[gSampleCnt++].word = cmdSample[0].word;
							//							gSamples[gSampleCnt++].word = cmdSample[1].word;
							//						} else {
							//							gSampleCnt = 0;
							//						}

							// We need to resynchronize the SD card bitstream.
							break;
					}

					// Create the response command.
					if (responseType != eSDCardRespTypeNone) {
						if (responseType == eSDCardRespType1) {
							cmdSample[0].word = 0x00000000;
							cmdSample[0].bytes.byte1 = responseCmd;

							cmdSample[1].word = 0x00000000;
							cmdSample[1].bytes.byte1 = (gSDCardState << 1);
							cmdSample[1].bytes.byte2 = gSDCardCmdState << 5;
							cmdSample[1].bytes.byte3 = crc7(cmdSample, 2);

							// Put the response into the SSI Tx FIFO.
							SSI_STX = cmdSample[0].word;
							SSI_STX = cmdSample[1].word;
						} else if (responseType == eSDCardRespType2) {
							// The CID response (R2) is longer than normal: 136 bits.
							// Expand the SSI frame to 6 words (of 24 bits each).
							// This is 8 bits more than we need, but it simplifies dealing with the FIFO.
							longFrame = TRUE;
							SSI_SCR_BIT.SSIEN = FALSE;
							SSI_STCCR_BIT.WL = SSI_20BIT_WORD;
							SSI_STCCR_BIT.DC = SSI_FRAME_LEN7;
							SSI_SCR_BIT.SSIEN = TRUE;

							/* cmdSample[0].word */SSI_STX = 0x0003f0a4;
							/* cmdSample[1].word */SSI_STX = 0x00075750;
							/* cmdSample[2].word */SSI_STX = 0x00046432;
							/* cmdSample[3].word */SSI_STX = 0x00002001;
							/* cmdSample[4].word */SSI_STX = 0x00000000;
							/* cmdSample[5].word */SSI_STX = 0x00000100;
							/* cmdSample[6].word */SSI_STX = 0x0009c89f;

						} else if (responseType == eSDCardRespType3) {
							// Initial value: start bit = 0, host bit = 0, cmd = 100101, busy bit = 0, 1/2 of OCR (at all voltages);
							cmdSample[0].word = 0x003f80ff;
							cmdSample[1].word = 0x008000ff;
							// Put the response into the SSI Tx FIFO.
							SSI_STX = cmdSample[0].word;
							SSI_STX = cmdSample[1].word;
						}

						// If the response we're just about to send is not the APP_COMMAND response
						// then return to "standard" command mode.
						if (cmdNum != eSDCardCmd55) {
							gSDCardCmdState = eSDCardCmdStateStd;
						}

						gIsTransmitting = TRUE;

//						TMR3_CTRL_BIT.tmrOutputMode = gTmrToggleOFUsingAlternateReg_c;
////						TMR3_CTRL_BIT.tmrCntOnce = FALSE;
////						TMR3_SCTRL_BIT.TCFIE = FALSE;
////						TMR3_SCTRL_BIT.VAL = 1;
////						TMR3_SCTRL_BIT.FORCE = 1;
//
//						SetComp1Val(FCS_TIMER, FSYNC_SUSTAIN_HIGH);
//						SetCompLoad1Val(FCS_TIMER, FSYNC_SUSTAIN_HIGH);
//						if (longFrame) {
//							SetComp2Val(FCS_TIMER, FSYNCR2_SUSTAIN_LOW);
//							SetCompLoad2Val(FCS_TIMER, FSYNCR2_SUSTAIN_LOW);
//						} else {
//							SetComp2Val(FCS_TIMER, FSYNC_SUSTAIN_LOW);
//							SetCompLoad2Val(FCS_TIMER, FSYNC_SUSTAIN_LOW);
//						}

//						gwUINT8 items = SSI_SFCSR_BIT.TFCNT0;
//						gwUINT8 items2;

						gTimerValStop = GetCntrVal(gTmr0_c);//xTickCount;vTaskGetTickCount();
						SSI_SCR_BIT.TE = TRUE;
//						error = TmrSetMode(FCS_TIMER, gTmrCntRiseEdgPriSrc_c);

//						TMR3_SCTRL_BIT.VAL = 0;
//						TMR3_SCTRL_BIT.FORCE = 0;
						TMR3_SCTRL_BIT.OPS = 0;

						// Set up SSI for Rx.
						gwUINT32 maxLoops;
						while ((maxLoops < 5000000) && (!SSI_SISR_BIT.TFS) /* (SSI_SFCSR_BIT.TFCNT0 == items) */) {
							// Wait until a Tx word goes out, or we timeout.
							maxLoops++;
//							items2 = SSI_SFCSR_BIT.TFCNT0;
						}

						// Tx is complete.
//						SSI_SCR_BIT.TE = FALSE;
//						TMR3_CTRL_BIT.tmrCntMode = gTmrNoOperation_c;
//						TMR3_SCTRL_BIT.VAL = 1;
//						TMR3_SCTRL_BIT.FORCE = 1;
						TMR3_SCTRL_BIT.OPS = 1;

						while ((maxLoops < 5000000) && /* (SSI_SFCSR_BIT.TFCNT0 > 0) */(!SSI_SISR_BIT.TFRC) ) {
							// Wait until a Tx word goes out, or we timeout.
							if (SSI_SISR_BIT.TDE) {
								maxLoops++;
							} else {
								maxLoops++;
							}
//							items2 = SSI_SFCSR_BIT.TFCNT0;
						}
//						items2 = SSI_SFCSR_BIT.TFCNT0;

						if (longFrame) {
							// Clear out the garbage samples read by the SSI during Tx (even tho' it shouldn't).
							//						for (gwUINT8 i = 0; i < 5; i++) {
							//							if (SSI_SISR_BIT.RDR) {
							//								cmdSample[9].word = SSI_SRX;
							////								gSamples[gSampleCnt++].word = SSI_SRX;
							////								if (gSampleCnt > 100) {
							////									gSampleCnt = 0;
							////								}
							//							}
							//						}

							SSI_SCR_BIT.SSIEN = FALSE;
							SSI_STCCR_BIT.WL = SSI_24BIT_WORD;
							SSI_STCCR_BIT.DC = SSI_FRAME_LEN2;
							SSI_SCR_BIT.SSIEN = TRUE;
						} else {
							// Clear out the garbage samples read by the SSI during Tx (even tho' it shouldn't).
							for (gwUINT8 i = 0; i < 2; i++) {
								if (SSI_SISR_BIT.RDR) {
									cmdSample[9].word = SSI_SRX;
								}
							}
						}
					}
				}
			}
		}
		waitForNextFrame(FSYNC_TIMER);
	}
	GW_EXIT_CRITICAL(ccr);
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
