/* MODULE Events */


/*Including used modules for compilling procedure*/
#include "Cpu.h"

/*Include shared modules, which are used for whole project*/
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "queue.h"

// Flyweight
#include "gatewayRadioTask.h"
#include "commands.h"
#include "ledBlinkTask.h"
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

interrupt void testadc(void) {
	UINT16 adcval = ATD1R;
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

#ifdef XBEE
	#define PWM_LSB_CHANNEL		TPM1C0V
	#define PWM_MSB_CHANNEL		TPM1C1V
#else
	#define PWM_LSB_CHANNEL		TPM1C2V
#endif
#define		MAX_DRIFT			0x80

bool				gAudioModeRX = TRUE;

//UINT16			gPWMMaxValue = 0xff;
#ifdef XBEE
UINT16				gPWMCenterValue = 0x7fff;
#else
UINT16				gPWMCenterValue = 0x7f;
#endif
BufferOffsetType	gCurPWMOffset = 0;
BufferCntType		gCurPWMRadioBufferNum = 0;

BufferCntType		gTXBuffer = 0;
BufferOffsetType	gTXBufferPos = 0;
bool				gBufferStarted = FALSE;

// The master sound sample rate.  It's the bus clock rate divided by the natural sample rate.
// For example 20Mhz / 10K samples/sec, or 2000.
// We further divide this by two since we average the cur and prev sample to smooth the waveform.
SampleRateType		gMasterSampleRate = 20000000 / 8000;

// The "tuning" time for the master rate to keep the packet flow balanced.
INT16				gMasterSampleRateAdjust = 0;

interrupt void AudioLoader_OnInterrupt(void) {	

	byte	ccrHolder;
	UINT8	sample8b;
#ifdef XBEE
	INT16	sample16b;
	UINT8	lsbSample;
	UINT8	msbSample;
	
#endif
	
	// Figure out if we're in the RX or TX mode for audio.
	// When the user presses the "push-to-talk" button the audio is only going back to the controller.
	// Otherwise the audio is coming from the controller.
	if (!gAudioModeRX) {
#ifdef XBEE
		// TX MODE
		
		// Reset the timer for the next sample. (speed up a little in transmit to make up for broadcast overhead.
		TPM2MOD = gMasterSampleRate - 0x50;

		if (!gBufferStarted) {
			gTXBuffer = gTXCurBufferNum;
			advanceTXBuffer();
			createAudioCommand(gTXBuffer);
			gTXBufferPos = gTXRadioBuffer[gTXBuffer].bufferSize;
			gBufferStarted = TRUE;
		} else {
			if (gTXBufferPos < CMD_MAX_AUDIO_BYTES) {
				sample16b = ATD1R << 5;
				sample8b = linear2ulaw(sample16b);
				gTXRadioBuffer[gTXBuffer].bufferStorage[gTXBufferPos++] = sample8b;
			} else {
				gTXRadioBuffer[gTXBuffer].bufferSize = gTXBufferPos;
				transmitPacketFromISR(gTXBuffer);
				gBufferStarted = FALSE;
			}
		}
#endif
	} else {
		// RX MODE

		// Reset the timer for the next sample.
		TPM2MOD = gMasterSampleRate + gMasterSampleRateAdjust;

		// The buffer for the current command doesn't contain an control/audio command, so advance to the next buffer.
		if (!((getCommandID(gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage) == eCommandAudio) 
			&& (gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStatus == eBufferStateInUse))) {
		
#ifdef XBEE
			//setReg16(PWM_LSB_CHANNEL, gPWMCenterValue);
			//setReg16(PWM_MSB_CHANNEL, gPWMCenterValue);
#else
			//setReg16(PWM_LSB_CHANNEL, gPWMCenterValue);
#endif
			EnterCriticalArg(ccrHolder);
			
				AUDIO_AMP_OFF;
				gCurPWMRadioBufferNum++;
				if (gCurPWMRadioBufferNum >= (RX_BUFFER_COUNT))
					gCurPWMRadioBufferNum = 0;
				
			ExitCriticalArg(ccrHolder);
			
		} else {
		
			AUDIO_AMP_ON;
			sample8b = gRXRadioBuffer[gCurPWMRadioBufferNum].bufferStorage[gCurPWMOffset];
#ifdef XBEE
			// On the XBee module we support 16bit converted uLaw samples.
			// One to 8-bit channel 0, and one to 8-bit channel 1.  
			// The two channels are tied together with different resistor values to give us 16 bit resolution.
			// (By "shifting" the voltage of channel 0 by 256x.)
			sample16b = gPWMCenterValue - ulaw2linear(sample8b);
			msbSample = (sample16b >> 8) & 0xff;
			lsbSample = sample16b & 0xff;
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
				if ((gRXUsedBuffers > RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust > -MAX_DRIFT)) {
					gMasterSampleRateAdjust--;
				} else if ((gRXUsedBuffers < RX_QUEUE_BALANCE) && (gMasterSampleRateAdjust < MAX_DRIFT)) {
					gMasterSampleRateAdjust++;
				}
			}
		}
	}

	// Reset the overflow flag.
	if (TPM2SC_TOF)
		TPM2SC_TOF = 0;
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

/*
** ===================================================================
**     Event       :  USB_OnError (module Events)
**
**     From bean   :  USB [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be
**         read using <GetError> method.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void  USB_OnError(void)
{
  /* Write your code here ... */
}

/* END Events */
