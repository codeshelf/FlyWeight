
#include "USB.h"
#include "UartLowLevel.h"
#include "board_config.h"
#include "UART_Interface.h"
#include "ITC_Interface.h"
#include "Platform.h"

#define gPlatformClock_c  (PLATFORM_CLOCK/1000)
#define gUartTimeout_d    (300)

#define BUFFER_SIZE		32
#define UART_BLOCK_SIZE	32

void UartEventRead1(UartReadCallbackArgs_t* args);
void UartEventWrite1(UartWriteCallbackArgs_t* args);
void GpioUart1Init(void);

UartReadStatus_t gu8SCIStatus = gUartReadStatusComplete_c;
uint8_t gu8SCIDataFlag = FALSE;
uint16_t gu16SCINumOfBytes = 0;

uint8_t gCircularBuffer[BUFFER_SIZE];
uint8_t	gCircularBufferPos;
uint8_t gCircularBufferEnd = BUFFER_SIZE + 1;
gwBoolean gCircularBufferIsEmpty = TRUE;

/*
 * GPIO init.
 */
void GpioUart1Init(void) {
	register uint32_t tmpReg;

	GPIO.PuSelLo |= (GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit);  // Pull-up select: UP type
	GPIO.PuEnLo  |= (GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit);  // Pull-up enable
	GPIO.InputDataSelLo &= ~(GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit); // read from pads
	GPIO.DirResetLo = (GPIO_UART1_RTS_bit | GPIO_UART1_RX_bit); // inputs
	GPIO.DirSetLo = (GPIO_UART1_CTS_bit | GPIO_UART1_TX_bit);  // outputs

	tmpReg = GPIO.FuncSel0 & ~((FN_MASK << GPIO_UART1_RX_fnpos) | (FN_MASK << GPIO_UART1_TX_fnpos));
	GPIO.FuncSel0 = tmpReg | ((FN_ALT << GPIO_UART1_RX_fnpos) | (FN_ALT << GPIO_UART1_TX_fnpos));
	tmpReg = GPIO.FuncSel1 & ~((FN_MASK << GPIO_UART1_CTS_fnpos) | (FN_MASK << GPIO_UART1_RTS_fnpos));
	GPIO.FuncSel1 = tmpReg | ((FN_ALT << GPIO_UART1_CTS_fnpos) | (FN_ALT << GPIO_UART1_RTS_fnpos));
}

/*
 *
 */
void UartEventRead1(UartReadCallbackArgs_t* args) {
  gu8SCIDataFlag = TRUE;
  gu16SCINumOfBytes = args->UartNumberBytesReceived;
  gu8SCIStatus = args->UartStatus;
}

/*
 *
 */
void UartEventWrite1(UartWriteCallbackArgs_t* args) {
}

/*
 * Init the USB port.
 */

void USB_Init(void) {
	UartConfig_t uartConfig;
	UartCallbackFunctions_t uartCallBack;
	UartErr_t uartErr;

	//Uart_Init();
	 GpioUart1Init();

	uartConfig.UartBaudrate = 1250000;
	uartConfig.UartFlowControlEnabled = TRUE;
	uartConfig.UartParity = gUartParityNone_c;
	uartConfig.UartStopBits = gUartStopBits1_c;
	uartConfig.UartRTSActiveHigh = FALSE;

	uartErr = UartOpen(UART_1, gPlatformClock_c);
	if (uartErr == gUartErrNoError_c) {
		uartErr = UartSetConfig(UART_1, &uartConfig);
		if (uartErr == gUartErrNoError_c) {

			// Set the BAUD rate to precisely 1,250,000.
			//uartErr = UartGetConfig(UART_1, &uartConfig);
			UART1_REGS_P->Ubr = 0xc34fea60;
			//uartErr = UartGetConfig(UART_1, &uartConfig);

			//set pCallback functions
			uartCallBack.pfUartWriteCallback = UartEventWrite1;
			uartCallBack.pfUartReadCallback = NULL;//UartEventRead1;
			//UartSetCallbackFunctions(UART_1, &uartCallBack);

			UartSetCTSThreshold(UART_1, 24);
			UartSetTransmitterThreshold(UART_1, 8);
			UartSetReceiverThreshold(UART_1, 24);
		}
	}

	// Setup the interrupts corresponding to UART driver.
//	IntAssignHandler(gUart1Int_c, (IntHandlerFunc_t)UartIsr1);
//	ITC_SetPriority(gUart1Int_c, gItcNormalPriority_c);
	// Enable the interrupts corresponding to UART driver.
//	ITC_EnableInterrupt(gUart1Int_c);
}

/*
 * Read one character from the USB port.
 */
void USB_ReadOneChar(USB_TComData *outChr) {

	gwUINT8 bytesToRead;
	gwUINT8 byteNum;

	// Loop until the buffer has something in it.
	if (gCircularBufferIsEmpty == TRUE) {
		// Wait until there are characters in the FIFO
		while (UART1_REGS_P->Urxcon == 0) {
			vTaskDelay(1);
		}
		bytesToRead = UART1_REGS_P->Urxcon;
		gCircularBufferIsEmpty = FALSE;
		for (byteNum = 0; byteNum < bytesToRead; ++byteNum) {
			gCircularBufferEnd++;
			if (gCircularBufferEnd > BUFFER_SIZE) {
				gCircularBufferEnd = 0;
			}
			gCircularBuffer[gCircularBufferEnd] = UART1_REGS_P->Udata;
		}
//		gCircularBufferPos++;
//		if (gCircularBufferPos > BUFFER_SIZE) {
//			gCircularBufferPos = 0;
//		}
	}

	// Get the first character out of the buffer.
	*outChr = gCircularBuffer[gCircularBufferPos];

	if (gCircularBufferPos == gCircularBufferEnd) {
		gCircularBufferIsEmpty = TRUE;
	}
	gCircularBufferPos++;
	if (gCircularBufferPos > BUFFER_SIZE) {
		gCircularBufferPos = 0;
	}
}

/*
 * Send one character to the USB port.
 */
gwUINT8 USB_SendChar(USB_TComData inChar) {
	gwUINT8 bytesInTx;
//	UartErr_t result;
//	result = UartWriteData(UART_1, &Chr, 1);
//	return result;
	// Until there is free space in the FIFO don't send a char.
	while (UART1_REGS_P->Utxcon < 1) {
		vTaskDelay(1);
	}
	UART1_REGS_P->Udata = inChar;
	return gUartErrNoError_c;
}
