/*
	FlyWeight
	� Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef RADIOCOMMON_H
#define RADIOCOMMON_H

// Project includes
#include "smacGlue.h"
#include "commandTypes.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"

// --------------------------------------------------------------------------
// Definitions.

// Priorities assigned to demo application tasks.
#define LED_BLINK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define MGMT_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define SERIAL_RECV_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define RADIO_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define KEYBOARD_PRIORITY		( tskIDLE_PRIORITY + 2 )

#define MAX_PACKET_SIZE			123
//#define MAX_PACKET_SIZE			30
#define ACK_DATA_BYTES			8

// The gateway needs fewer buffers.
#ifdef IS_GATEWAY
	#define RX_QUEUE_SIZE			10
	#define TX_QUEUE_SIZE			8
	#define RX_QUEUE_LOW_WATER		2
	#define RX_QUEUE_HIGH_WATER		RX_QUEUE_SIZE - 2
#else
	#ifdef MC1322X
		#define RX_QUEUE_SIZE			10
	#else
		#define RX_QUEUE_SIZE			10
	#endif
	#define RX_QUEUE_LOW_WATER		2
	#define RX_QUEUE_HIGH_WATER		RX_QUEUE_SIZE - 2
//	#define RX_QUEUE_BALANCE		4
	#define TX_QUEUE_SIZE			10
#endif

#define RX_BUFFER_COUNT			RX_QUEUE_SIZE
#define RX_BUFFER_SIZE			MAX_PACKET_SIZE

#define TX_BUFFER_COUNT			TX_QUEUE_SIZE
#define TX_BUFFER_SIZE			MAX_PACKET_SIZE

#define INVALID_RX_BUFFER		99
#define INVALID_TX_BUFFER		99

#define KEYBOARD_QUEUE_SIZE		2

//#define MASTER_TPM2_RATE		0x873
#if defined(XBEE_PINOUT)
	#define SETUP_SPKR_CONTROL		PTCDD |= 0b10000000;
	#define SPKR_AMP_OFF			PTCD  |= 0b10000000;
	#define SPKR_AMP_ON				PTCD  &= 0b01111111;
	#define SETUP_MIC_CONTROL		PTCDD |= 0b10000000;
	#define MIC_AMP_OFF				PTCD  |= 0b10000000;
	#define MIC_AMP_ON				PTCD  &= 0b01111111;
	#define ATD_ON					ATD1C = 0xC4;  ATD1SC = 0xE2;//ATD1SC_CCF = 1; ATD1C_ATDPU = 1;
	#define ATD_OFF					ATD1C_ATDPU = 0;
	#define PWM_LSB_CHANNEL			TPM1C0V
	#define PWM_MSB_CHANNEL			TPM1C1V
	#define TPMSC_PWM				TPM1SC
	#define TPMSC_AUDIO_LOADER		TPM2SC
	#define TPMMOD_AUDIO_LOADER		TPM2MOD
	#define TPMIE_AUDIO_LOADER		TPM2SC_TOIE
	#define TPMOF_AUDIO_LOADER		TPM2SC_TOF
#elif defined(MC1321X)
	#define SETUP_SPKR_CONTROL		PTBDD_PTBDD3 = 1;
	#define SPKR_AMP_OFF			PTBD_PTBD3 = 1; PWM_LSB_CHANNEL = 0x7f;  PWM_MSB_CHANNEL = 0x7f;
	#define SPKR_AMP_ON				PTBD_PTBD3 = 0;
	#define SETUP_MIC_CONTROL		PTBDD_PTBDD7 = 1;
	#define MIC_AMP_OFF				PTBD_PTBD7 = 0;
	#define MIC_AMP_ON				PTBD_PTBD7 = 1;
	#define ATD_ON					ATD1C_ATDPU = 1; //ATD1SC_ATDIE = 1; //ATD1SC_ATDIE = 0;
	#define ATD_OFF					ATD1C_ATDPU = 0; //ATD1SC_ATDIE = 0;
	#define PWM_LSB_CHANNEL			TPM2C3V
	#define PWM_MSB_CHANNEL			TPM2C2V
	#define TPMSC_PWM				TPM2SC
	#define TPMSC_AUDIO_LOADER		TPM1SC
	#define TPMMOD_AUDIO_LOADER		TPM1MOD
	#define TPMIE_AUDIO_LOADER		TPM1SC_TOIE
	#define TPMOF_AUDIO_LOADER		TPM1SC_TOF
#else
	#define SETUP_SPKR_CONTROL
	#define SPKR_AMP_OFF
	#define SPKR_AMP_ON
	#define SETUP_MIC_CONTROL
	#define MIC_AMP_OFF
	#define MIC_AMP_ON
	#define ATD_ON
	#define ATD_OFF
	#define PWM_LSB_CHANNEL			TPM1C2V
	#define TPMSC_PWM				TPM1SC
	#define TPMSC_AUDIO_LOADER		TPM2SC
	#define TPMMOD_AUDIO_LOADER		TPM2MOD
	#define TPMIE_AUDIO_LOADER		TPM2SC_TOIE
	#define TPMOF_AUDIO_LOADER		TPM2SC_TOF
#endif

#define SMAC_TICKS_PER_MS			250

#define MAX_LED_SEQUENCES			8

#ifdef MC1322X
	#define 	kDelayCheckCount	10
#else
	#define		kDelayCheckCount	3
#endif

#define RELEASE_RX_BUFFER(rxBufferNum, ccrHolder)	\
	GW_ENTER_CRITICAL(ccrHolder); \
	if (gRXRadioBuffer[rxBufferNum].bufferStatus != eBufferStateFree) { \
		gRXRadioBuffer[rxBufferNum].bufferStatus = eBufferStateFree; \
	} else if (GW_DEBUG) { \
		debugRefreed(rxBufferNum); \
	} \
	gRXUsedBuffers--; \
	GW_EXIT_CRITICAL(ccrHolder);

#define RELEASE_TX_BUFFER(txBufferNum, ccrHolder)	\
	GW_ENTER_CRITICAL(ccrHolder); \
	if (gTXRadioBuffer[txBufferNum].bufferStatus != eBufferStateFree) { \
		gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree; \
	} else if (GW_DEBUG) { \
		debugRefreed(txBufferNum); \
	} \
	gTXUsedBuffers--; \
	GW_EXIT_CRITICAL(ccrHolder);

#define BACKLIGHT_LOW					"\x7C\x8C"
#define BACKLIGHT_40PERCENT				"\x7C\x8C"
#define BACKLIGHT_50PERCENT				"\x7C\x8E"
#define BACKLIGHT_73PERCENT				"\x7C\x96"
#define BACKLIGHT_100PERCENT			"\x7C\x9D"
#define CURSOR_OFF						"\xFE\x0C"
#define LINE1_FIRST_POS 				"\xFE\x80"
#define LINE2_FIRST_POS 				"\xFE\xC0"
#define LINE3_FIRST_POS 				"\xFE\x94"
#define LINE4_FIRST_POS 				"\xFE\xD4"
#define CLEAR_DISPLAY					"\xFE\x01"

#define DISPLAY_SETUP					BACKLIGHT_73PERCENT CURSOR_OFF

#define DISPLAY_WIDTH					20

#define MAX_DISPLAY_STRING_BYTES		42
#define MAX_SCAN_STRING_BYTES			64

#define RS485_TX_ON						Gpio_SetPinData(gGpioPin21_c, gGpioPinStateHigh_c);
#define RS485_TX_OFF					Gpio_SetPinData(gGpioPin21_c, gGpioPinStateLow_c);

#define POS_CTRL_INIT					0x00
#define POS_CTRL_CLEAR					0x01
#define POS_CTRL_DISPLAY				0x02

#define POS_CTRL_ALL_POSITIONS			0x00

#define TOTAL_ERROR_SETS				80
#define DISTANCE_BETWEEN_ERROR_LEDS		16
	
#define MAX_LED_FLASH_POSITIONS 		TOTAL_ERROR_SETS * 2
#define MAX_LED_SOLID_POSITIONS 		MAX_LED_FLASH_POSITIONS

#define	SCROLL_TIMER					gTmr1_c
#define SCROLL_PRIMARY_SOURCE			gTmrPrimaryClkDiv128_c
#define SCROLL_SECONDARY_SOURCE			gTmrSecondaryCnt0Input_c
#define SCROLL_CLK_RATE					0xffff

// --------------------------------------------------------------------------
// Typedefs

typedef enum {
	eUSBStateStopped,
	eUSBStateStarted
} USBStateType;

typedef gwUINT8				BufferCntType;
typedef gwUINT8				BufferOffsetType;
typedef gwUINT8				BufferStorageType;
typedef BufferStorageType	*BufferStoragePtrType;

typedef enum {
	eBufferStateFree,
	eBufferStateInUse,
//	eBufferStateSoundData
} EBufferStatusType;

typedef struct {
	EBufferStatusType		bufferStatus;
#ifdef MC1322X
	// SMAC 2 has a completely brain-dead way of placing the SMAC header bytes into a message.
	// We deal with that by providing two places where a message can get accessed.
	// For our internal purposes we always use bufferStorage[], but we pass bufferRadioHeader[]
	// to any SMAC routines.
	BufferStorageType		bufferRadioHeader[3];
#endif
	BufferStorageType		bufferStorage[MAX_PACKET_SIZE];
	BufferCntType			bufferSize;
} RadioBufferStruct;

typedef gwUINT8				NetworkIDType;
typedef gwUINT8				AckIDType;
typedef gwUINT8				AckDataType[ACK_DATA_BYTES];
typedef gwUINT8				NetAddrType;
typedef gwUINT8				EndpointNumType;
typedef gwUINT8				KVPNumType;
typedef gwUINT8				RemoteUniqueIDType[UNIQUE_ID_BYTES + 1];
typedef RemoteUniqueIDType	*RemoteUniqueIDPtrType;

typedef struct {
	ERemoteStatusType		remoteState;
	RemoteUniqueIDType		remoteUniqueID;
} RemoteDescStruct;

typedef gwUINT16			SampleRateType;
typedef gwUINT8				SampleSizeType;

typedef union UnixTimeUnionType {
	struct{
		gwUINT8 byte1;
		gwUINT8 byte2;
		gwUINT8 byte3;
		gwUINT8 byte4;
	} byteFields;
	gwTime value;
} UnixTimeType;

#ifdef MC1322X

// From RadioManagement.c
#define MAX_NUM_MSG	4

#define RX_MESSAGE_PENDING(msg) ( \
	   (msg.u8Status.msg_state == MSG_RX_RQST) \
		|| (msg.u8Status.msg_state == MSG_RX_PASSED_TO_DEVICE) \
		|| (msg.u8Status.msg_state == MSG_RX_ACTION_STARTED) \
		|| (msg.u8Status.msg_state == MSG_RX_SYNC_FOUND) \
		/* || (msg.u8Status.msg_state == MSG_RX_RQST_ABORT) */ \
	)

#define TX_MESSAGE_PENDING(msg) ( \
	   (msg.u8Status.msg_state == MSG_TX_RQST) \
		|| (msg.u8Status.msg_state == MSG_TX_PASSED_TO_DEVICE) \
		|| (msg.u8Status.msg_state == MSG_TX_ACTION_STARTED) \
		|| (msg.u8Status.msg_state == MSG_TX_RQST_ABORT) \
	)

typedef struct {
	message_t		msg;
	BufferCntType	bufferNum;
} EMessageHolderType;

#endif

typedef gwUINT32			TimestampType;
typedef gwUINT32			DataSampleType;

// Hoobee types (deprecated)
//typedef gwBoolean			LedFlashRunType;
//typedef gwUINT8				LedFlashSeqCntType;
//typedef gwUINT8				LedValueType;
//typedef gwUINT16			LedFlashTimeType;
//typedef gwUINT8				LedRepeatCntType;

//typedef struct {
//	LedValueType		redValue;
//	LedValueType		greenValue;
//	LedValueType		blueValue;
//	LedFlashTimeType	timeOnMillis;
//	LedFlashTimeType	timeOffMillis;
//	LedRepeatCntType	repeat;
//} LedFlashStruct;

// PFC types. (deprecated)
//typedef struct {
//	ESDCardControlDeviceType	deviceType;
//	union {
//		gwUINT16				delayMillis;
//		struct {
//			gwUINT8 byte1;
//			gwUINT8 byte0;
//		} bytes ;
//	} delay ;
//} SDControlCommandStruct;

// Aisle Controller types.
typedef enum {
	eLedCycleOff,
	eLedCycleOn
} LedCycleType;

typedef LedCycleType 		LedCycle;
typedef gwUINT8 			LedChannelType;
typedef gwUINT16 			LedPositionType;
typedef gwUINT8 			LedData;

typedef struct {
		LedChannelType		channel;
		LedPositionType 	position;
		LedData				red;
		LedData				green;
		LedData				blue;
} LedDataStruct;

typedef gwUINT8				DisplayCharType;
typedef DisplayCharType		DisplayStringType[MAX_DISPLAY_STRING_BYTES];
typedef DisplayStringType*	DisplayStringPtrType;
typedef gwUINT8				DisplayStringLenType;

typedef gwUINT8				ScanCharType;
typedef ScanCharType		ScanStringType[MAX_SCAN_STRING_BYTES];
typedef ScanStringType*		ScanStringPtrType;
typedef gwUINT8				ScanStringLenType;

// --------------------------------------------------------------------------
// Externs

extern xTaskHandle			gRadioReceiveTask;
extern xTaskHandle			gRadioTransmitTask;
extern xTaskHandle			gSerialReceiveTask;
extern xTaskHandle			gGatewayManagementTask;
extern xTaskHandle			gRemoteManagementTask;
//extern xTaskHandle			gKeyboardTask;
//extern xTaskHandle			gStrainGageTask;
//extern xTaskHandle			gHooBeeTask;
//extern xTaskHandle			gPFCTask;
extern xTaskHandle			gAisleControllerTask;
extern xTaskHandle			gCartControllerTask;
extern xTaskHandle			gScannerReadTask;

/* The queue used to send data from the radio to the radio receive task. */
extern xQueueHandle 		gRadioTransmitQueue;
extern xQueueHandle 		gRadioReceiveQueue;

// Radio RX buffer
extern RadioBufferStruct	gRXRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRXCurBufferNum;
extern BufferCntType		gRXUsedBuffers;

// Radio TX buffer
extern RadioBufferStruct	gTXRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType		gTXCurBufferNum;
extern BufferCntType		gTXUsedBuffers;

extern gwTxPacket			gTxPacket;
extern gwRxPacket			gRxPacket;

extern SampleRateType		gMasterSampleRate;

extern RemoteDescStruct		gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Function prototypes

void advanceRXBuffer(void);
BufferCntType lockRXBuffer(void);
BufferCntType lockTXBuffer(void);
void setupWatchdog(void);
void debugReset();
void debugRefreed(BufferCntType inBufferNum);
void debugCrmCallback(void);

#endif /* RADIOCOMMON_H */
