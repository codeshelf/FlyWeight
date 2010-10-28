/*
  FlyWeight
  © Copyright 2005, 2006 Jeffrey B. Williams
  All rights reserved

  $Id$
  $Name$
*/

#include "audioARM.h"
#include "radioCommon.h"
#include "commands.h"
#include "ulaw.h"
#include "FreeRTOS.h"
#include "I2C_Interface.h"
#include "GPIO_Interface.h"

/*
 * The MC1322X audio uses two timers:
 * 		timer 2: for periodically loading a new PWM sample
 * 		timer 3: the actual PWM signal itself.
 */

#define	MAX_DRIFT			500
#define	SAMPLE_TIMER		gTmr2_c
#define	PWM_TIMER			gTmr3_c
#define  gGpioBeepTimer_c	((GpioPin_t)((uint8_t)gGpioPin8_c + (uint8_t)PWM_TIMER))

gwBoolean				gAudioModeRX = TRUE;

gwUINT16				gPWMCenterValue = 0x7f;
BufferOffsetType		gCurPWMOffset = 0;
BufferCntType			gCurPWMRadioBufferNum = 0;

BufferCntType			gCurAudioTXBuffer = 0;
BufferOffsetType		gCurAudioTXBufferPos = 0;
gwBoolean				gCurAudioTXBufferStarted = FALSE;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
SampleRateType			gMasterSampleRate = 2500;//1810; // 20,000,000 Hz / 11,050 samples/sec;

// The "tuning" time for the master rate to keep the packet flow balanced.
gwSINT16				gMasterSampleRateAdjust = 0;

// --------------------------------------------------------------------------

void audioInit() {
	AdcConfig_t adcConfig;

	Adc_Init();
	Adc_DefaultConfig(adcConfig, configCPU_CLOCK_HZ);
	Adc_SetConfig(&adcConfig);

	I2c_Init();
	Gpio_SetPinFunction(gGpioBeepTimer_c, gGpioAlternate1Mode_c);
	Gpio_SetPinFunction(gGpioPin12_c, gGpioAlternate1Mode_c);
	Gpio_SetPinFunction(gGpioPin13_c, gGpioAlternate1Mode_c);

	Llc_VolumeInit();
	Llc_VolumeSet(80);
}

// --------------------------------------------------------------------------

static void audioLoaderCallback(TmrNumber_t tmrNumber) {
	gwUINT8		ccrHolder;
	gwUINT8		unlockCcrHolder;
	gwUINT8		sample8b;
	gwUINT16	sample16b;
	gwUINT16	sampleAlt16b;

	// Figure out if we're in the RX or TX mode for audio.
	// When the user presses the "push-to-talk" button the audio is only going back to the controller.
	// Otherwise the audio is coming from the controller.
	if (gAudioModeRX)
	{
		// --- RX MODE ---------------------------------------------

		// The buffer for the current command doesn't contain an control/audio command, so advance to the next buffer.
		if (!((getCommandID(gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage) == eCommandAudio)
			&& (gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus == eBufferStateInUse))) {

			TmrSetMode(PWM_TIMER, gTmrNoOperation_c);

			GW_ENTER_CRITICAL(ccrHolder);
			gCurPWMRadioBufferNum++;
			if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
				gCurPWMRadioBufferNum = 0;
			GW_EXIT_CRITICAL(ccrHolder);

		} else {

			TmrSetMode(PWM_TIMER, gTmrCntRiseEdgPriSrc_c);

			sample8b = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];

			sample16b = ulaw2linear(sample8b);
			sample16b = 0x7fff - sample16b;
			sample16b >>= 6;
			sampleAlt16b = 1024 - sample16b;

			// Only the lower 8 bits of each channel are in use.
			SetCompLoad1Val(PWM_TIMER, sample16b);
			SetCompLoad2Val(PWM_TIMER, sampleAlt16b);

			// Increment the buffer pointers.
			gCurPWMOffset++;
			if (gCurPWMOffset >= gRXRadioBuffer[gCurPWMRadioBufferNum].bufferSize) {

				gCurPWMOffset = CMDPOS_CONTROL_DATA;

				// The buffers are a shared, critical resource, so we have to protect them before we update.
				GW_ENTER_CRITICAL(ccrHolder);

				// Indicate that the buffer is clear.
				RELEASE_RX_BUFFER(gCurPWMRadioBufferNum, unlockCcrHolder);

				// Advance to the next buffer.
				gCurPWMRadioBufferNum++;
				if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
					gCurPWMRadioBufferNum = 0;

				// Account for the number of used buffers.
				if (gRXUsedBuffers > 0)
					gRXUsedBuffers--;

				GW_EXIT_CRITICAL(ccrHolder);

				// Adjust the sampling rate to account for mismatches in the OTA rate.
				// We can't go too low or high, or we'll end up missing
				// the next interrupt and making the sample run "long".
				if ((gRXUsedBuffers > RX_QUEUE_HIGH_WATER) && (gMasterSampleRateAdjust > -MAX_DRIFT)) {
					gMasterSampleRateAdjust--;
				} else if ((gRXUsedBuffers > RX_QUEUE_LOW_WATER)) {
					gMasterSampleRateAdjust = 0;
				} else if ((gRXUsedBuffers < RX_QUEUE_LOW_WATER) && (gMasterSampleRateAdjust < MAX_DRIFT)) {
					gMasterSampleRateAdjust++;
				}
			}
		}
	} else if (!gAudioModeRX) {

		// --- TX MODE ---------------------------------------------

		// Take an audio sample first.
		// Start and wait for a ATD conversion.
//		ATD1SC_ATDCO = 0;
//		while (!ATD1SC_CCF) {
//		}
//		sample16b = ATD1R;
		sample8b = linear2ulaw(sample16b);

		// If we haven't started a buffer yet then start one.
		if (!gCurAudioTXBufferStarted) {
			// Don't try to start a new buffer if none are available.
			if (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse)
				return;
			gCurAudioTXBuffer = lockTXBuffer();
			createAudioCommand(gCurAudioTXBuffer);
			gCurAudioTXBufferPos = CMDPOS_AUDIO;
			gCurAudioTXBufferStarted = TRUE;
		}

		// Put the sample into the buffer.
		if (gCurAudioTXBufferStarted) {
			gTXRadioBuffer[gCurAudioTXBuffer].bufferStorage[gCurAudioTXBufferPos++] = sample8b;
		}

		// If the buffer is full then schedule it for send.
		if (gCurAudioTXBufferPos >= (CMD_MAX_AUDIO_BYTES + CMDPOS_AUDIO)) {
			gTXRadioBuffer[gCurAudioTXBuffer].bufferSize = gCurAudioTXBufferPos;
			transmitPacketFromISR(gCurAudioTXBuffer);
			gCurAudioTXBufferStarted = FALSE;
		}
	}
}

// --------------------------------------------------------------------------

void setupAudioLoader() {

	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	/* Enable hw timer 1 */
	TmrEnable(SAMPLE_TIMER);
	/* Don't stat the timer yet */
	TmrSetMode(SAMPLE_TIMER, gTmrNoOperation_c);

	/* Register the callback executed when a timer interrupt occur */
	TmrSetCallbackFunction(SAMPLE_TIMER, gTmrComp1Event_c, audioLoaderCallback);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.OEN = 1;
	TmrSetStatusControl(SAMPLE_TIMER, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;
	tmrComparatorStatusCtrl.bitFields.TCF1EN = TRUE;
	//tmrComparatorStatusCtrl.bitFields.CL1 = 0x01;
	TmrSetCompStatusControl(SAMPLE_TIMER, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrToggleOF_c;
	tmrConfig.tmrCoInit = FALSE; /*co-chanel counter/timers can not force a re-initialization of this counter/timer*/
	tmrConfig.tmrCntDir = FALSE; /*count-up*/
	tmrConfig.tmrCntLen = TRUE; /*count until compare*/
	tmrConfig.tmrCntOnce = FALSE; /*count repeatedly*/
	tmrConfig.tmrSecondaryCntSrc = gTmrSecondaryCnt0Input_c; /*secondary count source not needed*/
	tmrConfig.tmrPrimaryCntSrc = gTmrPrimaryClkDiv8_c; /*primary count source is IP BUS clock divide by 8 prescaler*/
	TmrSetConfig(SAMPLE_TIMER, &tmrConfig);

	/* Config timer to raise interrupts at 8KHz */
	SetComp1Val(SAMPLE_TIMER, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));
	SetCompLoad1Val(SAMPLE_TIMER, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));

	/* Config timer to start from 0 after compare event */
	SetLoadVal(SAMPLE_TIMER, 0);

	/* Start the counter at 0. */
	SetCntrVal(SAMPLE_TIMER, 0);

	/* Setup the interrupt handling to catch the TMR0 interrupts. */
	IntAssignHandler(gTmrInt_c, (IntHandlerFunc_t) TmrIsr);
	ITC_SetPriority(gTmrInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gTmrInt_c);

	TmrSetMode(SAMPLE_TIMER, gTmrCntRiseEdgPriSrc_c);
}

// --------------------------------------------------------------------------

void setupPWM() {

	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	/* Enable hw timer 1 */
	TmrEnable(PWM_TIMER);
	/* Don't stat the timer yet */
	TmrSetMode(PWM_TIMER, gTmrNoOperation_c);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.FORCE = 1;
	tmrStatusCtrl.bitFields.OEN = 1;
	TmrSetStatusControl(PWM_TIMER, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;
	tmrComparatorStatusCtrl.bitFields.TCF2EN = TRUE;
	tmrComparatorStatusCtrl.bitFields.CL1 = TRUE;
	tmrComparatorStatusCtrl.bitFields.CL2 = TRUE;
	//tmrComparatorStatusCtrl.bitFields.CL1 = 0x01;
	TmrSetCompStatusControl(PWM_TIMER, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrToggleOFUsingAlternateReg_c;
	tmrConfig.tmrCoInit = FALSE; /*co-chanel counter/timers can not force a re-initialization of this counter/timer*/
	tmrConfig.tmrCntDir = FALSE; /*count-up*/
	tmrConfig.tmrCntLen = TRUE; /*count until compare*/
	tmrConfig.tmrCntOnce = FALSE; /*count repeatedly*/
	tmrConfig.tmrSecondaryCntSrc = gTmrSecondaryCnt0Input_c; /*secondary count source not needed*/
	tmrConfig.tmrPrimaryCntSrc = gTmrPrimaryClkDiv1_c; /*primary count source is IP BUS clock divide by 8 prescaler*/
	TmrSetConfig(PWM_TIMER, &tmrConfig);

	/* Config timer to raise interrupts at 120KHz */
	SetComp1Val(PWM_TIMER, (65536 / 2));
	SetComp2Val(PWM_TIMER, (65536 / 2));
	SetCompLoad1Val(PWM_TIMER, (65536 / 2));
	SetCompLoad2Val(PWM_TIMER, (65536 / 2));

	/* Config timer to start from 0 after compare event */
	//SetLoadVal(PWM_TIMER, 0);

	/* Start the counter at 0. */
	SetCntrVal(PWM_TIMER, 0);
}
