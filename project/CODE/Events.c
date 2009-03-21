/* MODULE Events */


/*Including used modules for compilling procedure*/
#include "Cpu.h"
#include "Events.h"

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "queue.h"

// Flyweight
#include "remoteRadioTask.h"
#include "commands.h"
#include "ulaw.h"

// SMAC
#include "simple_mac.h"

/*
** ===================================================================
**     Event       :  TimerInt (module Events)
**
**     From bean   :  TickTimer [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the bean is enabled - "Enable" and the events are
**         enabled - "EnableEvent").
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
extern void TimerInt(void);
/*
** ===================================================================
**     Event       :  SW1Int_OnInterrupt (module Events)
**
**     From bean   :  SW1Int [ExtInt]
**     Description :
**         This event is called when the active signal edge/level
**         occurs.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#ifdef __SWI
extern void vButtonPush(void);
#pragma TRAP_PROC SAVE_NO_REGS
void SW1Int_OnInterrupt(void)
{
  /* place your SW1Int interrupt procedure body here */
  vButtonPush();
}
#endif


/*
** ===================================================================
**     Event       :  USB_OnFullRxBuf (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called when the input buffer is full.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

#ifdef __USB
#endif

/*
** ===================================================================
**     Event       :  TimerInt (module Events)
**
**     From bean   :  TickTimer [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the bean is enabled - "Enable" and the events are
**         enabled - "EnableEvent").
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#pragma NO_ENTRY
#pragma NO_EXIT
#pragma NO_FRAME
#pragma NO_RETURN
extern void vPortTickInterrupt(void);
void TimerInt(void)
{
	vPortTickInterrupt();
}

/*
** ===================================================================
**     Event       :  AudioLoader_OnInterrupt (module Events)
**
**     From bean   :  AudioLoader [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the bean is enabled - "Enable" and the events are
**         enabled - "EnableEvent").
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
//#ifdef __AudioLoader

#include "task.h"

#define		MAX_DRIFT			500

bool				gAudioModeRX = TRUE;

//UINT16			gPWMMaxValue = 0xff;
#if defined(XBEE) || defined(MC1321X)
//UINT16				gPWMCenterValue = 0x7fff;
UINT16				gPWMCenterValue = 0x7f;
#else
UINT16				gPWMCenterValue = 0x7f;
#endif
BufferOffsetType	gCurPWMOffset = 0;
BufferCntType		gCurPWMRadioBufferNum = 0;

BufferCntType		gCurAudioTXBuffer = 0;
BufferOffsetType	gCurAudioTXBufferPos = 0;
bool				gCurAudioTXBufferStarted = FALSE;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
SampleRateType		gMasterSampleRate = 2500;//1810; // 20,000,000 Hz / 11,050 samples/sec;

// The "tuning" time for the master rate to keep the packet flow balanced.
INT16				gMasterSampleRateAdjust = 0;

interrupt void AudioLoader_OnInterrupt(void) {

	byte	ccrHolder;
	UINT8	sample8b;
#if defined(XBEE) || defined(MC1321X)
	INT16	sample16b;
	UINT8	lsbSample;
	UINT8	msbSample;

#endif

	// Figure out if we're in the RX or TX mode for audio.
	// When the user presses the "push-to-talk" button the audio is only going back to the controller.
	// Otherwise the audio is coming from the controller.
	if (gAudioModeRX)
	{
		// --- RX MODE ---------------------------------------------

		// Reset the timer for the next sample.
		TPMMOD_AUDIO_LOADER = gMasterSampleRate + gMasterSampleRateAdjust;

		// The buffer for the current command doesn't contain an control/audio command, so advance to the next buffer.
		if (!((getCommandID(gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage) == eCommandAudio)
			&& (gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus == eBufferStateInUse))) {

			SPKR_AMP_OFF;

			EnterCriticalArg(ccrHolder);
				gCurPWMRadioBufferNum++;
				if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
					gCurPWMRadioBufferNum = 0;
			ExitCriticalArg(ccrHolder);

		} else {

			SPKR_AMP_ON;
			sample8b = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
#if defined(XBEE) || defined(MC1321X)
			// On the MC1321X or XBee module we support 16bit converted uLaw samples.
			// One to 8-bit channel 0, and one to 8-bit channel 1.
			// The two channels are tied together with different resistor values to give us 16 bit resolution.
			// (By "shifting" the voltage of channel 0 by 256x.)
			sample16b = ulaw2linear(sample8b);
			//sample16b = gPWMCenterValue - sample16b;
			msbSample = (sample16b >> 8) & 0xff;
			lsbSample = sample16b & 0xff;
			msbSample = gPWMCenterValue - msbSample;
			lsbSample = gPWMCenterValue - lsbSample;
			
			// Only the lower 8 bits of each channel are in use.
			PWM_LSB_CHANNEL = lsbSample;
			PWM_MSB_CHANNEL = msbSample;
#else
			setReg16(PWM_LSB_CHANNEL, gPWMCenterValue - sample8b);
#endif
				// Increment the buffer pointers.
			gCurPWMOffset++;
			if (gCurPWMOffset >= gRXRadioBuffer[gCurPWMRadioBufferNum].bufferSize) {

				gCurPWMOffset = CMDPOS_CONTROL_DATA;

				// The buffers are a shared, critical resource, so we have to protect them before we update.
				EnterCriticalArg(ccrHolder);

					// Indicate that the buffer is clear.
					gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus = eBufferStateFree;

					// Advance to the next buffer.
					gCurPWMRadioBufferNum++;
					if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
						gCurPWMRadioBufferNum = 0;

					// Account for the number of used buffers.
					if (gRXUsedBuffers > 0)
						gRXUsedBuffers--;

				ExitCriticalArg(ccrHolder);

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
	
#if defined(XBEE) || defined(MC1321X)

		// Reset the timer for the next sample. (speed up a little in transmit to make up for broadcast overhead.
		TPMMOD_AUDIO_LOADER = gMasterSampleRate;// - 0x40;

		// Take an audio sample first.
		// Start and wait for a ATD conversion.
		ATD1SC_ATDCO = 0;
		while (!ATD1SC_CCF) {
		}
#if defined(XBEE)
		sample16b = ATD1R << 4;
#else
		sample16b = ATD1R;
#endif
		sample8b = linear2ulaw(sample16b);

		// If we haven't started a buffer yet then start one.
		if (!gCurAudioTXBufferStarted) {
			// Don't try to start a new buffer if none are available.
			if (gTXRadioBuffer[gTXCurBufferNum].bufferStatus == eBufferStateInUse)
				return;
			gCurAudioTXBuffer = gTXCurBufferNum;
			advanceTXBuffer();
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


#endif
	}

	// Reset the overflow flag.
	if (TPMOF_AUDIO_LOADER)
		TPMOF_AUDIO_LOADER = 0;

}
//#endif
/*
** ===================================================================
**     Event       :  PWM1_OnEnd (module Events)
**
**     From bean   :  PWM1 [PWM]
**     Description :
**         This event is called when the specified number of cycles
**         has been generated. (Only when the bean is enabled -
**         Enable and the events are enabled - EnableEvent).
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

extern bool gIsSleeping;
UINT8 inRTI = 0;
#pragma NO_ENTRY
#pragma NO_EXIT
#pragma NO_FRAME
#pragma NO_RETURN
void handleRTI();
void handleRTI() {
		// Ack the RTI
		SRTISC_RTIACK = 1;
		vPortTickInterrupt();  // <------ We never return from this call into the OS task dispatch.
}

ISR(dispatchRTI)
{
	if (!gIsSleeping) {
		handleRTI();
		SRTISC_RTIACK = 1;
	} else {
		// We get here if we got an RTI while in STOP (sleep) mode.
		SRTISC_RTIACK = 1;
	}
}

ISR(TestADC) {
	UINT16 adcval = ATD1R;
}

ISR(LowVoltageDetect) {
	
}

/*
** ===================================================================
**     Event       :  USB_OnError (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be
**         read using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  USB_OnError(void)
{
  /* Write your code here ... */
}

/* END Events */
