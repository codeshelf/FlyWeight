#include "UART.h"
#include "board_config.h"
#include "UartLowLevel.h"
#include "UART_Interface.h"
#include "ITC_Interface.h"
#include "Platform.h"
#include "GPIO_Interface.h"

#define gPlatformClock_c  (PLATFORM_CLOCK/1000)
#define gUartTimeout_d    (300)

#define BUFFER_SIZE		32
#define UART_BLOCK_SIZE	32

void UartEventRead1(UartReadCallbackArgs_t* args);
void UartEventWrite1(UartWriteCallbackArgs_t* args);
void GpioUart1Init(void);
void GpioUart2Init(void);

UartReadStatus_t gu8SCIStatus = gUartReadStatusComplete_c;
uint8_t gu8SCIDataFlag = FALSE;
uint16_t gu16SCINumOfBytes = 0;

uint8_t gCircularBuffer[BUFFER_SIZE + 1];
uint8_t gCircularBufferPos;
uint8_t gCircularBufferEnd = BUFFER_SIZE + 1;
gwBoolean gCircularBufferIsEmpty = TRUE;

void GpioUart1Init(void) {
	register uint32_t tmpReg;
	GpioErr_t gpioError;

	// RX
	gpioError = Gpio_SetPinFunction(gGpioPin15_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin15_c, gGpioDirIn_c);
	gpioError = Gpio_EnPinPullup(gGpioPin15_c, TRUE);
	gpioError = Gpio_SelectPinPullup(gGpioPin15_c, gGpioPinPullup_c);

	// TX
	gpioError = Gpio_SetPinFunction(gGpioPin14_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin14_c, gGpioDirOut_c);

	// RTS
	gpioError = Gpio_SetPinFunction(gGpioPin17_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin17_c, gGpioDirIn_c);
	gpioError = Gpio_EnPinPullup(gGpioPin17_c, TRUE);
	gpioError = Gpio_SelectPinPullup(gGpioPin17_c, gGpioPinPullup_c);

	// CTS
	gpioError = Gpio_SetPinFunction(gGpioPin16_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin16_c, gGpioDirOut_c);
}

void GpioUart2Init(void) {
	register uint32_t tmpReg;
	GpioErr_t gpioError;

	// RX
	gpioError = Gpio_SetPinFunction(gGpioPin19_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin19_c, gGpioDirIn_c);
	gpioError = Gpio_EnPinPullup(gGpioPin19_c, TRUE);
	gpioError = Gpio_SelectPinPullup(gGpioPin19_c, gGpioPinPullup_c);

	// TX
	gpioError = Gpio_SetPinFunction(gGpioPin18_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin18_c, gGpioDirOut_c);

	// RTS
	gpioError = Gpio_SetPinFunction(gGpioPin21_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin21_c, gGpioDirIn_c);
	gpioError = Gpio_EnPinPullup(gGpioPin21_c, TRUE);
	gpioError = Gpio_SelectPinPullup(gGpioPin21_c, gGpioPinPullup_c);

	// CTS
	gpioError = Gpio_SetPinFunction(gGpioPin20_c, gGpioAlternate1Mode_c);
	gpioError = Gpio_SetPinDir(gGpioPin20_c, gGpioDirOut_c);
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
 * Init the UART port.
 */

void UART_Init(gwUINT8 uartNum, gwUINT32 baudrate, gwBoolean enableFlowControl) {
	UartConfig_t uartConfig;
	UartCallbackFunctions_t uartCallBack;
	UartErr_t uartErr;

	//Uart_Init();
	if (uartNum == UART_1) {
		GpioUart1Init();
	} else {
		GpioUart2Init();
	}

	uartConfig.UartBaudrate = baudrate;
	uartConfig.UartFlowControlEnabled = enableFlowControl;
	uartConfig.UartParity = gUartParityNone_c;
	uartConfig.UartStopBits = gUartStopBits1_c;
	uartConfig.UartRTSActiveHigh = FALSE;

	uartErr = UartOpen(uartNum, gPlatformClock_c);
	if (uartErr == gUartErrNoError_c) {
		uartErr = UartSetConfig(uartNum, &uartConfig);
		if (uartErr == gUartErrNoError_c) {

			// Set the BAUD rate to precisely 1,250,000.
			//uartErr = UartGetConfig(uartNum, &uartConfig);
			//UART1_REGS_P->Ubr = 0xc34fea60;
			//uartErr = UartGetConfig(uartNum, &uartConfig);

			//set pCallback functions
			uartCallBack.pfUartWriteCallback = UartEventWrite1;
			uartCallBack.pfUartReadCallback = NULL;			//UartEventRead1;
			//UartSetCallbackFunctions(uartNum, &uartCallBack);

			UartSetCTSThreshold(uartNum, 24);
			UartSetTransmitterThreshold(uartNum, 8);
			UartSetReceiverThreshold(uartNum, 24);
		}
	}

	// Setup the interrupts corresponding to UART driver.
//	IntAssignHandler(gUart1Int_c, (IntHandlerFunc_t)UartIsr1);
//	ITC_SetPriority(gUart1Int_c, gItcNormalPriority_c);
	// Enable the interrupts corresponding to UART driver.
//	ITC_EnableInterrupt(gUart1Int_c);
}

/*
 * Close the UART port.
 */
void UART_Close(gwUINT8 uartNum) {
	UartErr_t uartErr;
	uartErr = UartClose(uartNum);
}

/*
 * Read one character from the UART port.
 */
void UART_ReadOneChar(gwUINT8 uartNum, UART_TComData *outChr) {

	gwUINT8 bytesToRead;
	gwUINT8 byteNum;
	UartRegs_t *uartRegPtr;

	if (uartNum == UART_1) {
		uartRegPtr = UART1_REGS_P;
	} else {
		uartRegPtr = UART2_REGS_P;
	}

	// Loop until the buffer has something in it.
	if (gCircularBufferIsEmpty == TRUE) {
		// Wait until there are characters in the FIFO
		while (uartRegPtr ->Urxcon == 0) {
			vTaskDelay(1);
		}
		bytesToRead = uartRegPtr ->Urxcon;
		gCircularBufferIsEmpty = FALSE;
		for (byteNum = 0; byteNum < bytesToRead; ++byteNum) {
			gCircularBufferEnd++;
			if (gCircularBufferEnd > BUFFER_SIZE) {
				gCircularBufferEnd = 0;
			}
			gCircularBuffer[gCircularBufferEnd] = uartRegPtr ->Udata;
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
 * Send one character to the UART port.
 */
gwUINT8 UART_SendChar(gwUINT8 uartNum, UART_TComData inChar) {
	gwUINT8 bytesInTx;
	UartRegs_t *uartRegPtr;

	if (uartNum == UART_1) {
		uartRegPtr = UART1_REGS_P;
	} else {
		uartRegPtr = UART2_REGS_P;
	}

	// Until there is free space in the FIFO don't send a char.
	while (uartRegPtr->Utxcon < 1) {
		vTaskDelay(1);
	}
	uartRegPtr->Udata = inChar;

	return gUartErrNoError_c;
}
