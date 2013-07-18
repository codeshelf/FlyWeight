/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "cartControllerTask.h"
#include "scannerReadTask.h"
#include "spi.h"
#include "commands.h"
#include "PortConfig.h"
#include "UartLowLevel.h"
#include "Ssi_Interface.h"

#ifdef MC1322X
#include "MacaInterrupt.h"
#include "TransceiverConfigMngmnt.h"
#include "Leds.h"
#include "Delay.h"
#include "PortConfig.h"
#include "GPIO_Interface.h"
#elif
#include "CPU.h"
#include "keyboardTask.h"
#include "keyboard.h"
#endif

// --------------------------------------------------------------------------
// Globals

portTickType gLastPacketReceivedTick;

// --------------------------------------------------------------------------

void setutpGpio(void) {

	register uint32_t tmpReg;
	GpioErr_t gpioError;

	// SSI
	// Pull-up select: UP type
	//GPIO.PuSelLo |= (GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);
	// Pull-up enable
	//GPIO.PuEnLo  |= (GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);
	// Data select sets these ports to read from pads.
	GPIO .InputDataSelLo &= ~(GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);
	// inputs
	GPIO .DirResetLo = (GPIO_TIMER1_INOUT_bit | GPIO_SSI_RX_bit);
	// outputs
	GPIO .DirSetLo = (GPIO_TIMER3_INOUT_bit | GPIO_SSI_TX_bit | GPIO_SSI_FSYNC_bit | GPIO_SSI_CLK_bit);

	// Setup the function enable pins.
	tmpReg = GPIO .FuncSel0 & ~((FN_MASK << GPIO_TIMER1_INOUT_fnpos)| (FN_MASK << GPIO_TIMER3_INOUT_fnpos) | (FN_MASK
			<< GPIO_SSI_TX_fnpos) | (FN_MASK << GPIO_SSI_RX_fnpos) | (FN_MASK << GPIO_SSI_FSYNC_fnpos) | (FN_MASK
			<< GPIO_SSI_CLK_fnpos));

	GPIO .FuncSel0 = tmpReg | ((FN_ALT << GPIO_TIMER1_INOUT_fnpos)| (FN_ALT << GPIO_TIMER3_INOUT_fnpos) | (FN_ALT
			<< GPIO_SSI_TX_fnpos) | (FN_ALT << GPIO_SSI_RX_fnpos) | (FN_ALT << GPIO_SSI_FSYNC_fnpos) | (FN_ALT
			<< GPIO_SSI_CLK_fnpos));

	// SPI bus GPIOs
//	error = Gpio_SetPinDir(gGpioPin4_c, gGpioDirOut_c);
//	error = Gpio_SetPinDir(gGpioPin5_c, gGpioDirOut_c);
//	error = Gpio_SetPinDir(gGpioPin6_c, gGpioDirOut_c);
//	error = Gpio_SetPinDir(gGpioPin7_c, gGpioDirOut_c);

	// I2C
//	error = Gpio_SetPinDir(gGpioPin12_c, gGpioDirOut_c);

	// UART2
	GPIO .PuSelLo |= (GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit);  // Pull-up select: UP type
	GPIO .PuEnLo |= (GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit);  // Pull-up enable
	GPIO .InputDataSelLo &= ~(GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit); // read from pads
	GPIO .DirResetLo = (GPIO_UART2_RTS_bit | GPIO_UART2_RX_bit); // inputs
	GPIO .DirSetLo = (GPIO_UART2_CTS_bit | GPIO_UART2_TX_bit);  // outputs

	tmpReg = GPIO .FuncSel1 & ~((FN_MASK << GPIO_UART2_CTS_fnpos)| (FN_MASK << GPIO_UART2_RTS_fnpos)
	| (FN_MASK << GPIO_UART2_RX_fnpos) | (FN_MASK << GPIO_UART2_TX_fnpos));
	GPIO .FuncSel1 = tmpReg | ((FN_ALT << GPIO_UART2_CTS_fnpos)| (FN_ALT << GPIO_UART2_RTS_fnpos)
	| (FN_ALT << GPIO_UART2_RX_fnpos) | (FN_ALT << GPIO_UART2_TX_fnpos));

	// KBI
	gpioError = Gpio_SetPinDir(gGpioPin22_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin23_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin24_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin25_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin26_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin27_c, gGpioDirIn_c);

	gpioError = Gpio_EnPinPullup(gGpioPin22_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin23_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin24_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin25_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin26_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin27_c, TRUE);

	gpioError = Gpio_SelectPinPullup(gGpioPin22_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin23_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin24_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin25_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin26_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin27_c, gGpioPinPulldown_c);

	gpioError = Gpio_SetPinReadSource(gGpioPin22_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin23_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin24_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin25_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin26_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin27_c, gGpioPinReadPad_c);
}

// --------------------------------------------------------------------------

void setupUart(void) {
	UartConfig_t uartConfig;
	UartCallbackFunctions_t uartCallBack;
	UartErr_t uartErr;

	//Uart_Init();
	// GpioUart2Init();

	uartConfig.UartBaudrate = 9600;
	uartConfig.UartFlowControlEnabled = FALSE;
	uartConfig.UartParity = gUartParityNone_c;
	uartConfig.UartStopBits = gUartStopBits1_c;
	uartConfig.UartRTSActiveHigh = FALSE;

	uartErr = UartOpen(UART_2, 24000);
	if (uartErr == gUartErrNoError_c) {
		uartErr = UartSetConfig(UART_2, &uartConfig);
		if (uartErr == gUartErrNoError_c) {

			// Set the BAUD rate to precisely 1,250,000.
			//uartErr = UartGetConfig(UART_1, &uartConfig);
			//UART2_REGS_P->Ubr = 0xc34fea60;
			//uartErr = UartGetConfig(UART_1, &uartConfig);

			//set pCallback functions
			uartCallBack.pfUartWriteCallback = NULL;			//UartEventWrite1;
			uartCallBack.pfUartReadCallback = NULL;			//UartEventRead1;
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

// --------------------------------------------------------------------------

static void setupSsi() {

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
	ssiConfig.ssiGatedTxClockMode = TRUE;
	ssiConfig.ssiMode = gSsiNormalMode_c; // Normal mode
	ssiConfig.ssiNetworkMode = FALSE; // Network mode
	ssiConfig.ssiInterruptEn = FALSE; // Interrupts enabled
	error = SSI_SetConfig(&ssiConfig);

	// Setup the SSI clock.
	//ssiClockConfig.ssiClockConfigWord = SSI_SET_BIT_CLOCK_FREQ(24000000, 3400000);
	ssiClockConfig.bit.ssiDIV2 = 0x0; // 24M / 1
	ssiClockConfig.bit.ssiPSR = 0x0; // 24M / 1
	ssiClockConfig.bit.ssiPM = 24; // 24M / 24 ( /2) = 500K
	ssiClockConfig.bit.ssiDC = SSI_FRAME_LEN2; // Two words in each frame.  (Frame divide control.)
	ssiClockConfig.bit.ssiWL = SSI_24BIT_WORD; // 3 - 8 bits, 7 = 16 bits, 9 = 20 bits, b = 24 bits
	error = SSI_SetClockConfig(&ssiClockConfig);

	// Setup Tx.
	ssiTxRxConfig.ssiTxRxConfigWord = 0;
	ssiTxRxConfig.bit.ssiEFS = 0; // Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 0; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0; // Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0; // Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 1; // CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 1; // Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1; // Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1; // Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	ssiTxRxConfig.bit.ssiRxEXT = 0; // Receive sign extension: 0 = off, 1 = on.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeTx_c);

	// Setup Rx.
	ssiTxRxConfig.ssiTxRxConfigWord = 0;
	ssiTxRxConfig.bit.ssiEFS = 0; // Early frame sync: 0 = off, 1 = on.
	ssiTxRxConfig.bit.ssiFSL = 0; // Frame sync length: 0 = one word, 1 = one clock.
	ssiTxRxConfig.bit.ssiFSI = 0; // Frame sync invert: 0 = active high, 1 = active low.
	ssiTxRxConfig.bit.ssiSCKP = 0; // Data clocked: 0 = on rising edge, 1 = falling edge.
	ssiTxRxConfig.bit.ssiSHFD = 0; // Data shift direction: 0 = MSB-first, 1 = LSB-first.
	ssiTxRxConfig.bit.ssiCLKDIR = 1; // CLK source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFDIR = 1; // Frame sync source: 0 = external, 1 = internal.
	ssiTxRxConfig.bit.ssiFEN = 1; // Tx/Rx FIFO: 0 = disabled, 1 = enabled.
	ssiTxRxConfig.bit.ssiBIT0 = 1; // Tx/Rx bit0 of TSX/RSX: 0 = bit31, 1 = bit0.
	ssiTxRxConfig.bit.ssiRxEXT = 0; // Receive sign extension: 0 = off, 1 = on.
	error = SSI_SetTxRxConfig(&ssiTxRxConfig, gSsiOpTypeRx_c);

	// Disable the last word of the frame.
	// This is because of some stupid error that doesn't allow us to reliably send out the last
	// word of the frame without causing an overrun.
	//SSI_STMSK = 0x00;
	//SSI_SRMSK = 0x00;

	SSI_SFCSR_BIT .RFWM0 = 4;
	SSI_SFCSR_BIT .TFWM0 = 5;

	SSI_SCR_BIT .TFR_CLK_DIS = 0;
	SSI_STCCR_BIT .DC = 0;

	// Setup the SSI interrupts.
	SSI_SIER_WORD = 0;
	SSI_SIER_BIT .RIE = FALSE;
	SSI_SIER_BIT .ROE_EN = FALSE;
	SSI_SIER_BIT .RFRC_EN = FALSE;
	SSI_SIER_BIT .RDR_EN = FALSE;
	SSI_SIER_BIT .RLS_EN = FALSE;
	SSI_SIER_BIT .RFS_EN = FALSE;
	SSI_SIER_BIT .RFF_EN = FALSE;

	SSI_SIER_BIT .TIE = FALSE;
	SSI_SIER_BIT .TDE_EN = FALSE;
	SSI_SIER_BIT .TFE_EN = FALSE;

	// Enable Rx, and disable Tx.
	SSI_SCR_BIT .TE = TRUE;
	SSI_SCR_BIT .RE = FALSE;

	SSI_Enable(TRUE);
}

// --------------------------------------------------------------------------

void setupDisplayScroller() {

	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	/* Enable hw timer 1 */
	TmrEnable(SCROLL_TIMER);
	/* Don't stat the timer yet */
	TmrSetMode(SCROLL_TIMER, gTmrNoOperation_c);

	/* Register the callback executed when a timer interrupt occur */
	TmrSetCallbackFunction(SCROLL_TIMER, gTmrComp1Event_c, displayScrollCallback);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.OEN = 1;
	TmrSetStatusControl(SCROLL_TIMER, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;
	tmrComparatorStatusCtrl.bitFields.TCF1EN = TRUE;
	//tmrComparatorStatusCtrl.bitFields.CL1 = 0x01;
	TmrSetCompStatusControl(SCROLL_TIMER, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrToggleOF_c;
	tmrConfig.tmrCoInit = FALSE; /*co-channel counter/timers can not force a re-initialization of this counter/timer*/
	tmrConfig.tmrCntDir = FALSE; /*count-up*/
	tmrConfig.tmrCntLen = TRUE; /*count until compare*/
	tmrConfig.tmrCntOnce = FALSE; /*count repeatedly*/
	tmrConfig.tmrSecondaryCntSrc = SCROLL_SECONDARY_SOURCE;
	tmrConfig.tmrPrimaryCntSrc = SCROLL_PRIMARY_SOURCE;
	TmrSetConfig(SCROLL_TIMER, &tmrConfig);

	/* Config timer to raise interrupts at 20Hz */
	SetComp1Val(SCROLL_TIMER, SCROLL_CLK_RATE);
	SetCompLoad1Val(SCROLL_TIMER, SCROLL_CLK_RATE);

	/* Config timer to start from 0 after compare event */
	SetLoadVal(SCROLL_TIMER, 0);

	/* Start the counter at 0. */
	SetCntrVal(SCROLL_TIMER, 0);

	/* Setup the interrupt handling to catch the TMR1 interrupts. */
//	IntAssignHandler(gTmrInt_c, (IntHandlerFunc_t) TmrIsr);
//	ITC_SetPriority(gTmrInt_c, gItcNormalPriority_c);
//	ITC_EnableInterrupt(gTmrInt_c);
//	TmrSetMode(SCROLL_TIMER, gTmrCntRiseEdgPriSrc_c);
}

// --------------------------------------------------------------------------

portTickType gLastButtonPressTick;

void kbiInterruptCallback(void) {
	ScanStringType message;

	CRM_WuTimerInterruptDisable();

	// Clear the KBI int status bits (by writing "1" to them).
	CRM_STATUS .extWuEvt = 0x2;

	// Don't allow any double-presses or switch bouncing for up to 1 sec after we register a good button.
	if (xTaskGetTickCountNoCritical() > gLastButtonPressTick + 1000) {

		gwUINT32 loops = 0;

		gwUINT8 buttonNum = 0;
		while ((buttonNum == 0) && (loops++ < 50000)) {
			if (GPIO .DataLo & BIT26) {
				buttonNum = 1;
			} else if (GPIO .DataLo & BIT25) {
				buttonNum = 2;
			} else if (GPIO .DataLo & BIT24) {
				buttonNum = 3;
			} else if (GPIO .DataLo & BIT23) {
				buttonNum = 4;
			} else if (GPIO .DataLo & BIT22) {
				buttonNum = 5;
			}
		}

		if (buttonNum != 0) {
			// Now send the scan string.
			BufferCntType txBufferNum = lockTXBuffer();
			sprintf(message, "B%%%d", buttonNum);
			createScanCommand(txBufferNum, &message, strlen(message));
			transmitPacketFromISR(txBufferNum);

			gLastButtonPressTick = xTaskGetTickCountNoCritical();
		}
	}
	CRM_WuTimerInterruptEnable();

}

// --------------------------------------------------------------------------

void setupKbi(void) {
	// Setup the KBI handler.
	CRM_RegisterISR(gCrmKB5WuEvent_c, kbiInterruptCallback);

	crmWuCtrl_t wakeUpCtrl;
	wakeUpCtrl.wuSource = gExtWu_c;
	wakeUpCtrl.ctrl.ext.word = 0;
	wakeUpCtrl.ctrl.ext.bit.kbi5IntEn = TRUE;
	wakeUpCtrl.ctrl.ext.bit.kbi5WuEn = TRUE;
	wakeUpCtrl.ctrl.ext.bit.kbi5WuEvent = 1; // wake on edge
	wakeUpCtrl.ctrl.ext.bit.kbi5WuPol = 1; // wake on positive edge

	CRM_WuCntl(&wakeUpCtrl);
}

// --------------------------------------------------------------------------

void vMain(void) {

#if defined(MC1321X) || defined(MC13192EVB)
	// These got moved into PE pre-init, so that the RTI clock can use the EXTAL.
	// The EXTAL has to be setup and reserved BEFORE initialization of the ICG in PE init.
	//MCUInit();
	//MC13192Init();
	//MLMESetMC13192ClockRate(0);

	// Setup the SMAC glue.
	initSMACRadioQueueGlue(gRadioReceiveQueue);

#ifdef XBEE
	xbeeInit();
#endif
#else

	GpioErr_t error;

	// We don't call this, because we don't want to mess with DPF settings at restart.
	//PlatformPortInit();
	// Setup the radio GPIOs. (Don't know why SMAC doesn't do this.)
	GPIO_REGS_P ->FuncSel2 = gFuncSel2Value_c;

	ITC_Init();
	IntAssignHandler(gMacaInt_c, (IntHandlerFunc_t) MACA_Interrupt);
	ITC_SetPriority(gMacaInt_c, gItcFastPriority_c); // gItcNormalPriority_c
	ITC_EnableInterrupt(gMacaInt_c);
	IntDisableAll();
	ResetMaca();
	MLMERadioInit();

	// Setup the CEL Freestar radio controls for PA and Tx/Rx.
	SetDemulatorMode(NCD);

	// The PA's Vreg needs to be "on" always. (Controlled by GPIO42.)
	SetPowerLevelLockMode(TRUE);
	ConfigureRfCtlSignals(gRfSignalANT1_c, gRfSignalFunctionGPIO_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalANT2_c, gRfSignalFunctionGPIO_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalTXON_c, gRfSignalFunction1_c, TRUE, TRUE);
	ConfigureRfCtlSignals(gRfSignalRXON_c, gRfSignalFunction1_c, TRUE, TRUE);
	SetComplementaryPAState(TRUE);

	IntEnableAll();
	LED_Init();

	setupWatchdog();
	setupKbi();

#endif

	setutpGpio();
	setupSsi();
	setupUart();
	setupDisplayScroller();

	// The the display boot complete.
	DelayMs(250);

	// Set the backlight to 40%
	// Setting backlight, or any EEPROM param, causes the display to hang until POR.
	//sendDisplayMessage(DISPLAY_SETUP, strlen(DISPLAY_SETUP));

	sendDisplayMessage(CLEAR_DISPLAY, strlen(CLEAR_DISPLAY));
	sendDisplayMessage(LINE1_POS1, strlen(LINE1_POS1));
	sendDisplayMessage("DISCONNECTED", 12);

	gLocalDeviceState = eLocalStateStarted;
	GW_RADIO_GAIN_ADJUST(15);
	if (GW_SET_RADIO_CHANNEL(0) == GW_SMAC_SUCCESS) {
	}

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const ) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY,
			&gRadioTransmitTask);
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const ) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY,
			&gRadioReceiveTask);
	//xTaskCreate(keyboardTask, (const signed portCHAR * const) "Keyboard", configMINIMAL_STACK_SIZE, NULL, KEYBOARD_PRIORITY, &gKeyboardTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const ) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY,
			&gRemoteManagementTask);
	xTaskCreate(cartControllerTask, (const signed portCHAR * const ) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY,
			&gCartControllerTask);
	xTaskCreate(scannerReadTask, (const signed portCHAR * const ) "Scan", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY,
			&gScannerReadTask);

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	//gKeyboardQueue = xQueueCreate(KEYBOARD_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	//gCartControllerQueue = xQueueCreate(CART_CONTROLLER_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(SDControlCommandStruct));

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();

	/* All the tasks have been created - start the scheduler. */
	vTaskStartScheduler();

	/* Should not reach here! */
	for (;;)
		;
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

	GW_WATCHDOG_RESET;

	// If we haven't received a packet in by timeout seconds then reset.
	portTickType ticks = xTaskGetTickCount();
	if (ticks > (gLastPacketReceivedTick + kNetCheckTickCount)) {
		GW_RESET_MCU()
		;
	}
}
