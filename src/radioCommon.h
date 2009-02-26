/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved
	
	$Id$
	$Name$	
*/

#ifndef RADIOCOMMON_H
#define RADIOCOMMON_H

// Project includes
#include "PE_Types.h"
#include "cpu.h"
#include "smacGlue.h"
#include "commandTypes.h"

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

// The gateway needs fewer buffers.
#ifdef IS_GATEWAY
	#define RX_QUEUE_SIZE			5
	#define RX_QUEUE_LOW_WATER		2
	#define RX_QUEUE_HIGH_WATER		RX_QUEUE_SIZE - 2
//	#define RX_QUEUE_BALANCE		1
	#define TX_QUEUE_SIZE			5
#else
	#define RX_QUEUE_SIZE			15
	#define RX_QUEUE_LOW_WATER		2
	#define RX_QUEUE_HIGH_WATER		RX_QUEUE_SIZE - 2
//	#define RX_QUEUE_BALANCE		4
	#define TX_QUEUE_SIZE			3
#endif

#define RX_BUFFER_COUNT			RX_QUEUE_SIZE
#define RX_BUFFER_SIZE			MAX_PACKET_SIZE

#define TX_BUFFER_COUNT			TX_QUEUE_SIZE
#define TX_BUFFER_SIZE			MAX_PACKET_SIZE

#define KEYBOARD_QUEUE_SIZE		2

//#define MASTER_TPM2_RATE		0x873
#if defined(XBEE)
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

#define CTS_ON  				PTAD_PTAD6 = 0;
#define CTS_OFF  				PTAD_PTAD6 = 1;
#define CTS_SETUP      			PTAPE_PTAPE6 = 0; PTADD_PTADD6 = 1;

#define USB_STOP				CTS_OFF/*; gUSBState = eUSBStateStopped */
#define USB_START				CTS_ON /*; gUSBState = eUSBStateStarted */

#define MAX_REMOTES				14
#define INVALID_REMOTE			MAX_REMOTES + 1

#define ADDR_CONTROLLER			0x00
#define ADDR_BROADCAST			0x0F

#define SMAC_TICKS_PER_MS		250

#define RELEASE_RX_BUFFER(rxBufferNum)		EnterCriticalArg(gCCRHolder); \
											gRXRadioBuffer[rxBufferNum].bufferStatus = eBufferStateFree; \
											gRXUsedBuffers--; \
											ExitCriticalArg(gCCRHolder);

#define RELEASE_TX_BUFFER(txBufferNum)		EnterCriticalArg(gCCRHolder); \
											gTXRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree; \
											gTXUsedBuffers--; \
											ExitCriticalArg(gCCRHolder);

// --------------------------------------------------------------------------
// Typedefs

typedef enum {
	eUSBStateStopped,
	eUSBStateStarted
} USBStateType;

typedef UINT8				BufferCntType;
typedef UINT8				BufferOffsetType;
typedef UINT8				BufferStorageType;
typedef BufferStorageType	*BufferStoragePtrType;

typedef enum {
	eBufferStateFree,
	eBufferStateInUse,
//	eBufferStateSoundData
} EBufferStatusType;

typedef struct {
	EBufferStatusType		bufferStatus;
	BufferStorageType		bufferStorage[MAX_PACKET_SIZE];
	BufferCntType			bufferSize;
} RadioBufferStruct;

typedef UINT8				NetworkIDType;
typedef UINT8				RemoteAddrType;
typedef UINT8				EndpointNumType;
typedef UINT8				KVPNumType;
typedef byte				RemoteUniqueIDType[UNIQUE_ID_BYTES + 1];
typedef RemoteUniqueIDType	*RemoteUniqueIDPtrType;
typedef struct {
	ERemoteStatusType		remoteState;
	RemoteUniqueIDType		remoteUniqueID;
} RemoteDescStruct;

typedef UINT16				SampleRateType;
typedef UINT8				SampleSizeType;

// --------------------------------------------------------------------------
// Externs

extern xTaskHandle			gRadioReceiveTask;
extern xTaskHandle			gRadioTransmitTask;
extern xTaskHandle			gSerialReceiveTask;
extern xTaskHandle			gGatewayManagementTask;
extern xTaskHandle			gRemoteManagementTask;
extern xTaskHandle			gKeyboardTask;

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

extern tTxPacket			gsTxPacket;
extern tRxPacket			gsRxPacket;

extern SampleRateType		gMasterSampleRate;

extern RemoteDescStruct		gRemoteStateTable[MAX_REMOTES];

// --------------------------------------------------------------------------
// Function prototypes

void advanceRXBuffer(void);
void advanceTXBuffer(void);

#endif RADIOCOMMON_H