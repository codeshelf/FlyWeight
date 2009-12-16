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

xTaskHandle 	gPFCTask = NULL;
xQueueHandle 	gPFCQueue;
ESDCardState	gSDCardState;
ESDCardCmdState gSDCardCmdState;

// --------------------------------------------------------------------------

gwUINT8 sample[]  = { 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0 };

void pfcTask(void *pvParameters) {

	gwUINT8 samples;

	if (gPFCQueue) {

		gpioInit();
		setupSSI();
		setupTimers();
		gSDCardState = eSDCardStateIdle;
		gSDCardCmdState = eSDCardCmdStateStd;

		// Start the FSYNC simulation timer.
		TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrEdgSecSrcTriggerPriCntTillComp_c);

//		SSI_SCR_BIT.TE = TRUE;

		for (;;) {
//			samples = SSI_SFCSR_BIT.TFCNT0;
////			if (samples == 0) {
//			for (gwUINT8 i; i < 5; i++) {
//				SSI_STX = 0x00aaaaaa;
//				SSI_STX = 0x00aaaaf0;
////				SSI_STX = 0x00aaaaaa;
////				SSI_STX = 0x00aaaaaa;
////				SSI_STX = 0x00aaaaaa;
////				SSI_STX = 0x00aaaaaa;
////				SSI_STX = 0x00aaaaaa;
////				SSI_STX = 0x00aaaaaa;
//				SSI_SCR_BIT.TE = TRUE;
//				//SSI_TxData(&sample, 4, gSsiWordSize32bit_c, 1);
//				samples = SSI_SFCSR_BIT.TFCNT0;
//				while (samples >= 1) {
//					//vTaskDelay(1);
//					samples = SSI_SFCSR_BIT.TFCNT0;
//				}
//			}
//			SSI_SCR_BIT.TE = FALSE;
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
	tmpReg = GPIO.FuncSel0 & ~((FN_MASK << GPIO_TIMER1_INOUT_fnpos) | (FN_MASK << GPIO_TIMER3_INOUT_fnpos) | (FN_MASK << GPIO_SSI_TX_fnpos) | (FN_MASK << GPIO_SSI_RX_fnpos) | (FN_MASK << GPIO_SSI_FSYNC_fnpos) | (FN_MASK << GPIO_SSI_CLK_fnpos));
	GPIO.FuncSel0 = tmpReg | ((FN_ALT << GPIO_TIMER1_INOUT_fnpos) | (FN_ALT << GPIO_TIMER3_INOUT_fnpos) | (FN_ALT << GPIO_SSI_TX_fnpos) | (FN_ALT << GPIO_SSI_RX_fnpos) | (FN_ALT << GPIO_SSI_FSYNC_fnpos) | (FN_ALT << GPIO_SSI_CLK_fnpos));
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

	// Enable FS timer.
	error = TmrEnable(SSI_FRAMESYNC_TIMER);
	// Don't run t yet.
	error = TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrNoOperation_c);

	// Register the callback executed when a timer interrupt occurs.
	error = TmrSetCallbackFunction(SSI_FRAMESYNC_TIMER, gTmrCompEvent_c, timerCallback);
	error = TmrSetCallbackFunction(SSI_FRAMESYNC_TIMER, gTmrComp1Event_c, timerCallback);
	error = TmrSetCallbackFunction(SSI_FRAMESYNC_TIMER, gTmrComp2Event_c, timerCallback);
	error = TmrSetCallbackFunction(SSI_FRAMESYNC_TIMER, gTmrEdgeEvent_c, timerCallback);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.TCFIE = TRUE;				// Timer compare IE.
	tmrStatusCtrl.bitFields.TOFIE = FALSE;				// Timer overflow IE.
	tmrStatusCtrl.bitFields.IEFIE = FALSE;				// Input edge flag IE.
	tmrStatusCtrl.bitFields.IPS = 1;					// Input polarity select: 0 = normal, 1 = inverted.
	tmrStatusCtrl.bitFields.CAPMODE = 0;				// Capture mode.
	tmrStatusCtrl.bitFields.MSTR = FALSE;				// Master mode enable.
	tmrStatusCtrl.bitFields.EEOF = FALSE;				// External enable OFLAG.
	tmrStatusCtrl.bitFields.OPS = 1;					// Output polarity: 0 = normal, 1 = inverted.
	tmrStatusCtrl.bitFields.OEN = TRUE;					// Output enable.
	error = TmrSetStatusControl(SSI_FRAMESYNC_TIMER, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
//	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;	// Debug enable.
	tmrComparatorStatusCtrl.bitFields.TCF1EN = FALSE;	// Timer compare1 IE.
	tmrComparatorStatusCtrl.bitFields.TCF2EN = FALSE;	// Timer compare2 IE.
	tmrComparatorStatusCtrl.bitFields.CL1 = 0x01;		// Compare load control 1.
	error = TmrSetCompStatusControl(SSI_FRAMESYNC_TIMER, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrSetOnCompClearOnSecInputEdg_c;
	tmrConfig.tmrCoInit = 0; 					// Co-init: 0 = another channel cannot init this timer.
	tmrConfig.tmrCntDir = 0;					// Count dir: 0 = up, 1 = down.
	tmrConfig.tmrCntLen = 1;					// Count length: 0 = roll over, 1 = until compare then reinit
	tmrConfig.tmrCntOnce = 0;					// Count once: 0 = repeatedly, 1 = once only.
	tmrConfig.tmrSecondaryCntSrc = SECONDARY_SOURCE;
	tmrConfig.tmrPrimaryCntSrc = PRIMARY_SOURCE;
	error = TmrSetConfig(SSI_FRAMESYNC_TIMER, &tmrConfig);

	// After the CMD pin asserts a falling edge we can assert the FSYNC for HIGH SD Card clock cycles, and deassert for LOW cycles.
	SetComp1Val(SSI_FRAMESYNC_TIMER, FSYNC_CLK_CNT_HIGH);
	SetCompLoad1Val(SSI_FRAMESYNC_TIMER, FSYNC_CLK_CNT_HIGH);

	// Config timer to start from 0 after compare event.
	SetLoadVal(SSI_FRAMESYNC_TIMER, 0);

	// Start the counter at 0.
	SetCntrVal(SSI_FRAMESYNC_TIMER, 0);

//	IntAssignHandler(gTmrInt_c, (IntHandlerFunc_t) TmrIsr);
//	ITC_SetPriority(gTmrInt_c, gItcNormalPriority_c);
//	ITC_EnableInterrupt(gTmrInt_c);
}

// --------------------------------------------------------------------------
/*
 * This routine gets called when there is an interrupt from the timer.  We're only interested in
 * interrupts for timer 1 since that means we reached 48 clocks after the frist SD Card CMD edge.
 * The entire CMD should now live inside the SSI Rx FIFO.
 */
static void timerCallback(TmrNumber_t tmrNumber) {

	TmrConfigReg_t config;
	TmrStatusCtrl_t status;

	config.uintValue = TMR3_CTRL_WORD;
	status.uintValue = TMR3_SCTRL_WORD;

	// Stop the timer, and clear the interrupt flag.
	//TMR3_CTRL_BIT.tmrCntMode = gTmrNoOperation_c;

	if (config.bitFields.tmrCntMode == gTmrEdgSecSrcTriggerPriCntTillComp_c) {
		// We just caught an edge, so switch to PWM mode.
		// In PWM mode we toggle the signal low until the end of the 2nd count.
		// (At which point we will interrupt and come back here to switch into secondary edge mode.)
		TMR3_CTRL_BIT.tmrOutputMode = gTmrSetOF_c; // (NB: OPS is reversed.)
		TMR3_SCTRL_BIT.TCFIE = TRUE;
//		TMR3_CSCTRL_BIT.TCF1EN = TRUE;
//		TMR3_CSCTRL_BIT.TCF2EN = FALSE;

		SetComp1Val(SSI_FRAMESYNC_TIMER, FSYNC_CLK_CNT_LOW);
		SetCompLoad1Val(SSI_FRAMESYNC_TIMER, FSYNC_CLK_CNT_LOW);

		TMR3_CTRL_BIT.tmrCntMode = gTmrCntRiseEdgPriSrc_c;

	} else if (config.bitFields.tmrCntMode == gTmrCntRiseEdgPriSrc_c) {
//		if (isTCF2) {
			// We just completed a PWM, so switch to secondary edge tirgger mode.
			TMR3_CTRL_BIT.tmrOutputMode = gTmrSetOnCompClearOnSecInputEdg_c;
			TMR3_SCTRL_BIT.TCFIE = TRUE;
//			TMR3_CSCTRL_BIT.TCF1EN = TRUE;
//			TMR3_CSCTRL_BIT.TCF2EN = FALSE;

			SetComp1Val(SSI_FRAMESYNC_TIMER, FSYNC_CLK_CNT_HIGH);
			SetCompLoad1Val(SSI_FRAMESYNC_TIMER, FSYNC_CLK_CNT_HIGH);

			TMR3_CTRL_BIT.tmrCntMode = gTmrEdgSecSrcTriggerPriCntTillComp_c;
//		}
	}
	TMR3_SCTRL_BIT.TCF = 0;
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
	SSI_Enable(FALSE);

	// Setup the SSI mode.
	ssiConfig.ssiMode = gSsiNormalMode_c;	// Normal mode
	ssiConfig.ssiNetworkMode = TRUE; 		// Network mode
	ssiConfig.ssiInterruptEn = TRUE; 		// Interrupts enabled
	error = SSI_SetConfig(&ssiConfig);

	// Setup the SSI clock.
	ssiClockConfig.ssiClockConfigWord = SSI_SET_BIT_CLOCK_FREQ(24000000, 100000);
	ssiClockConfig.bit.ssiDC = 1;			// Two words in each frame.  (Frame divide control.)
	ssiClockConfig.bit.ssiWL = 0x0b; 		// 3 - 8 bits, 7 = 16 bits, b = 24 bits
	error = SSI_SetClockConfig(&ssiClockConfig);

	// Setup Tx.
	ssiTxRxConfig.bit.ssiEFS = 0;			// Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 1;			// Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0;			// Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0;			// Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0;			// Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 0;		// CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 0;			// Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1;			// Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1;			// Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeTx_c);

	// Setup Rx.
	ssiTxRxConfig.bit.ssiEFS = 0;			// Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 1;			// Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0;			// Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0;			// Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0;			// Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 0;		// CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 0;			// Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1;			// Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1;			// Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	ssiTxRxConfig.bit.ssiRxEXT = 0;			// Receive sign extension: 0 = off, 1 = on.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeRx_c);

	// We only use two time slots in each frame.
	SSI_STMSK = 0x00;
	SSI_SRMSK = 0x00;

	SSI_SFCSR_BIT.RFWM0 = 2;

	SSI_Enable(TRUE);

	// Setup the SSI interrupts.
	SSI_SIER_BIT.RIE = TRUE;
	SSI_SIER_BIT.RFRC_EN = TRUE;
//	SSI_SIER_BIT.RDR_EN = TRUE;
//	SSI_SIER_BIT.RFS_EN = TRUE;
	SSI_SIER_BIT.RFF_EN = TRUE;

	IntAssignHandler(gSsiInt_c, (IntHandlerFunc_t) ssiInterrupt);
	ITC_SetPriority(gSsiInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gSsiInt_c);

	// Enable Rx, and disable Tx.
	SSI_SCR_BIT.TE = FALSE;
	SSI_SCR_BIT.RE = TRUE;

}

// --------------------------------------------------------------------------

void ssiInterrupt(void) {

	// SSI Rx mode off, Tx mode off
	//SSI_SCR_BIT.RE = FALSE;

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
	USsiSampleType cmdSample[8];
//	USsiSampleType cmdSample[1];
	SsiISReg_t intStatuses;

	intStatuses.word = SSI_SISR_WORD;

	// Make sure it's a host command by looking at the H bit in sample #1.
	if ((SSI_SISR_BIT.RFRC != 0) || (SSI_SISR_BIT.RFF != 0)) {

		gwUINT8 samples = SSI_SFCSR_BIT.RFCNT0;

		//	cmdSample[0].word = SSI_SRX;
		//	cmdSample[1].word = SSI_SRX;
		for (gwUINT8 i = 0; i <= 7; i++) {
			if (SSI_SFCSR_BIT.RFCNT0) {
				cmdSample[i].word = SSI_SRX;
			} else {
				cmdSample[i].word = 0;
			}
		}

		if (cmdSample[0].bytes.byte2 & 0x40) {
			ESDCardCommand cmdNum = cmdSample[0].bytes.byte1 & 0x3F;
			ESDCardCommand responseCmd = cmdNum;
			ESDCardResponseType responseType = eSDCardRespTypeInvalid;

			// Generate the response command.
			switch (cmdNum) {
			case eSDCardCmd0:
				break;

			case eSDCardCmd2:
				break;

			case eSDCardCmd3:
				break;

			case eSDCardCmd41:
				responseType = eSDCardRespType3;
				break;

			case eSDCardCmd55:
				// Indicate that we're in the Application command state.
				gSDCardCmdState = eSDCardCmdStateApp;
				responseType = eSDCardRespType1;
				break;
			}

			// Create the response command.
			if (responseCmd != eSDCardCmdInvalid) {
				if (responseType == eSDCardRespType1) {
					// Initial value: start bit = 0;
					cmdSample[0].word = 0x00000000;
					cmdSample[0].bytes.byte1 = responseCmd;

					// Initial value: stop bit = 1;
					cmdSample[1].word = 0x00000001;
					cmdSample[1].bytes.byte1 = gSDCardState << 1;
					cmdSample[1].bytes.byte2 = gSDCardCmdState << 5;
					cmdSample[1].bytes.byte3 = (crc7(&cmdSample[0].bytes.byte0, &cmdSample[1].bytes.byte0) < 1) + 1;
				} else if (responseType == eSDCardRespType3) {
					// Initial value: start bit = 0, host bit = 0, cmd = 111111, 1/2 of OCR (at all voltages);
					cmdSample[0].word = 0x003f00ff;

					// Initial value: 1/2 of OCR (at all voltages), crc = 1111111, stop bit = 1;
					cmdSample[1].word = 0x00ff00ff;
				}
				// Put the response into the SSI Tx FIFO.
				SSI_STX = cmdSample[0].word;
				SSI_STX = cmdSample[1].word;

				// Turn on the FCS for 48 clock cycles.

				// If the response we're just about to send is not the APP_COMMAND response
				// then return to "standard" command mode.
				if (cmdNum != eSDCardCmd55) {
					gSDCardCmdState = eSDCardCmdStateStd;
				}
			}
		}
	}

//	// SSI Rx mode on, Tx mode off
//	SSI_SCR_BIT.TE = TRUE;
//	while (SSI_SFCSR_BIT.RFCNT0 > 0) {
//		// Wait.
//	}
//	SSI_SCR_BIT.TE = FALSE;

	// Reset the SSI interrupt flags, and timers.
	SSI_SISR_BIT.RFRC = FALSE;

}

// --------------------------------------------------------------------------

gwUINT8 crc7(gwUINT8 *inSample1Ptr, gwUINT8 *inSample2Ptr) {
	gwUINT8 i, a;
	gwUINT8 crc, data;

	crc = 0;
	for (a = 0; a < 5; a++) {
		if (a <= 2) {
			data = inSample1Ptr[a];
		} else {
			data = inSample2Ptr[a-3];
		}
		for (i = 0; i < 8; i++) {
			crc <<= 1;

			if ((data & 0x80) ^ (crc & 0x80))
				crc ^= 0x09;
			data <<= 1;
		}
	}
	crc = (crc << 1) | 1;
	return (crc);
}
