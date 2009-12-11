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
#include "gwTypes.h"
#include "remoteMgmtTask.h"
#include "Ssi_Interface.h"

xTaskHandle 	gPFCTask = NULL;
xQueueHandle 	gPFCQueue;
ESDCardState	gSDCardState;
ESDCardCmdState gSDCardCmdState;


// --------------------------------------------------------------------------

void pfcTask(void *pvParameters) {

	if (gPFCQueue) {

		setupSSI();
		setupCommandIntercept();
		gSDCardState = eSDCardStateIdle;
		gSDCardCmdState = eSDCardCmdStateStd;
		TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrEdgSecSrcTriggerPriCntTillComp_c);

		for (;;) {
			vTaskDelay(10);
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
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

void setupCommandIntercept() {

	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	/* Enable hw timer 1 */
	TmrEnable(SSI_FRAMESYNC_TIMER);
	/* Don't stat the timer yet */
	TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrNoOperation_c);

	/* Register the callback executed when a timer interrupt occur */
	TmrSetCallbackFunction(SSI_FRAMESYNC_TIMER, gTmrComp1Event_c, commandCallback);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.TCFIE = 1;
	tmrStatusCtrl.bitFields.TOFIE = 1;
	tmrStatusCtrl.bitFields.OPS = 1;
	tmrStatusCtrl.bitFields.OEN = 1;
	TmrSetStatusControl(SSI_FRAMESYNC_TIMER, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	//tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;
	//tmrComparatorStatusCtrl.bitFields.TCF1EN = TRUE;
	//tmrComparatorStatusCtrl.bitFields.CL1 = 0x01;
	TmrSetCompStatusControl(SSI_FRAMESYNC_TIMER, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrSetOnCompClearOnSecInputEdg_c;
	tmrConfig.tmrCoInit = FALSE; /*co-chanel counter/timers can not force a re-initialization of this counter/timer*/
	tmrConfig.tmrCntDir = FALSE; /*count-up*/
	tmrConfig.tmrCntLen = TRUE; /*count until compare*/
	tmrConfig.tmrCntOnce = FALSE; /*count repeatedly*/
	tmrConfig.tmrSecondaryCntSrc = SECONDARY_SOURCE;
	tmrConfig.tmrPrimaryCntSrc = PRIMARY_SOURCE;
	TmrSetConfig(SSI_FRAMESYNC_TIMER, &tmrConfig);

	/* 8 clock cycles after the CMD pin registers we can deassert the FCS. */
	SetComp1Val(SSI_FRAMESYNC_TIMER, FS_CLOCK_COUNT);
	SetCompLoad1Val(SSI_FRAMESYNC_TIMER, FS_CLOCK_COUNT);

	/* Config timer to start from 0 after compare event */
	SetLoadVal(SSI_FRAMESYNC_TIMER, 0);

	/* Start the counter at 0. */
	SetCntrVal(SSI_FRAMESYNC_TIMER, 0);

	/* Setup the interrupt handling to catch the TMR0 interrupts. */
	IntAssignHandler(gTmrInt_c, (IntHandlerFunc_t) TmrIsr);
	ITC_SetPriority(gTmrInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gTmrInt_c);
}

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
	SSI_Enable(TRUE);

	ssiConfig.ssiMode = gSsiNormalMode_c;	// Normal mode
	ssiConfig.ssiNetworkMode = TRUE; 		// Network mode
	ssiConfig.ssiInterruptEn = TRUE; 		// Interrupts enabled
	error = SSI_SetConfig(&ssiConfig);
	SSI_SCR_BIT.TE = FALSE; 				// Tx off
	SSI_SCR_BIT.RE = TRUE; 					// Rx on

	ssiClockConfig.bit.ssiPM = 0; 			// There is no internal clocking in this system.
	ssiClockConfig.bit.ssiDC = 2;			// Two words in each frame.  (Frame divide control.)
	ssiClockConfig.bit.ssiWL = 0x0b; 		// 3 - 8 bits, 7 = 16 bits, b = 24 bites
	ssiClockConfig.bit.ssiPSR = 0;			// Prescaler bypassed
	ssiClockConfig.bit.ssiDIV2 = 0;			// Divide clock bypassed
	error = SSI_SetClockConfig(&ssiClockConfig);

	ssiTxRxConfig.bit.ssiBIT0 = 1;			// Tx/Rx w.r.t. bit0 of the TSX/RSX.
	ssiTxRxConfig.bit.ssiCLKDIR = 0;		// External CLK input.
	ssiTxRxConfig.bit.ssiEFS = 0;			// Early frame sync off.
	ssiTxRxConfig.bit.ssiFDIR = 0;			// Frame sync external.
	ssiTxRxConfig.bit.ssiFEN = 1;			// Tx FIFO enabled.
	ssiTxRxConfig.bit.ssiFSI = 0;			// Frame sync not inverted.
	ssiTxRxConfig.bit.ssiFSL = 0;			// Frame sync is word length.
	ssiTxRxConfig.bit.ssiRxEXT = 0;			// Receive sign extension turned off.
	ssiTxRxConfig.bit.ssiSCKP = 1;			// Data clocked on falling CLK edge.
	ssiTxRxConfig.bit.ssiSHFD = 0;			// Data shift direction MSB-first.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeTx_c);
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeRx_c);

	// We only use two time slots in each frame.
	SSI_STMSK = 0x02;
	SSI_SRMSK = 0x02;
}

// --------------------------------------------------------------------------
/*
 * This routine gets called when there is an interrupt from the timer.  We're only interested in
 * interrupts for timer 1 since that means we reached 48 clocks after the frist SD Card CMD edge.
 * The entire CMD should now live inside the SSI Rx FIFO.
 */
static void commandCallback(TmrNumber_t tmrNumber) {

	// Disable the CMS line edge trigger and FS timer.
	TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrNoOperation_c);

	// SSI Rx mode off, Tx mode off
	SSI_SCR_BIT.RE = FALSE;

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
	USsiSampleType cmdSample1;
	USsiSampleType cmdSample2;

	cmdSample1.word = SSI_SRX;
	cmdSample2.word = SSI_SRX;

	// Make sure it's a host command by looking at the H bit in sample #1.
	if (cmdSample1.bytes.byte2 & 0x40) {
		ESDCardCommand cmdNum = cmdSample1.bytes.byte1 & 0x3F;
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
				cmdSample1.word = 0x00000000;
				cmdSample1.bytes.byte1 = responseCmd;

				// Initial value: stop bit = 1;
				cmdSample2.word = 0x00000001;
				cmdSample2.bytes.byte1 = gSDCardState << 1;
				cmdSample2.bytes.byte2 = gSDCardCmdState << 5;
				cmdSample2.bytes.byte3 = (crc7(&cmdSample1.bytes.byte0, &cmdSample2.bytes.byte0) < 1) + 1;
			} else if (responseType == eSDCardRespType3) {
				// Initial value: start bit = 0, host bit = 0, cmd = 111111, 1/2 of OCR (at all voltages);
				cmdSample1.word = 0x003f00ff;

				// Initial value: 1/2 of OCR (at all voltages), crc = 1111111, stop bit = 1;
				cmdSample2.word = 0x00ff00ff;
			}
			// Put the response into the SSI Tx FIFO.
			SSI_STX = cmdSample1.word;
			SSI_STX = cmdSample2.word;

			// Turn on the FCS for 48 clock cycles.

			// If the response we're just about to send is not the APP_COMMAND response
			// then return to "standard" command mode.
			if (cmdNum != eSDCardCmd55) {
				gSDCardCmdState = eSDCardCmdStateStd;
			}
		}
	}

	// SSI Rx mode on, Tx mode off
	SSI_SCR_BIT.TE = FALSE;
	SSI_SCR_BIT.RE = TRUE;

	// Reenable the CMD signal edge trigger and the FS timer for the next command.
	TmrSetMode(SSI_FRAMESYNC_TIMER, gTmrEdgSecSrcTriggerPriCntTillComp_c);
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
