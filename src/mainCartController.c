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
#include "string.h"
#include "stdlib.h"

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
// Defines

void setupKbi();
void lcdOff();
void lcdOn();
pfCallback_t preSleep();
void kbiSleepCallback(void);

// --------------------------------------------------------------------------
// Globals

portTickType gLastPacketReceivedTick;

// --------------------------------------------------------------------------

void setupGpio(void) {
	GpioErr_t gpioError;

	// Timer 1
	gpioError = Gpio_SetPinFunction(gGpioPin9_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin9_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin9_c, gGpioPinReadPad_c);

	// Timer 3
	gpioError = Gpio_SetPinFunction(gGpioPin11_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin11_c, gGpioDirOut_c);

	// SSI TX
	gpioError = Gpio_SetPinFunction(gGpioPin0_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin0_c, gGpioDirOut_c);

	// SSI RX
	gpioError = Gpio_SetPinFunction(gGpioPin1_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin1_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin1_c, gGpioPinReadPad_c);

	// SSI FSYNC
	gpioError = Gpio_SetPinFunction(gGpioPin2_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin2_c, gGpioDirOut_c);

	// SSI CLK
	gpioError = Gpio_SetPinFunction(gGpioPin3_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin3_c, gGpioDirOut_c);

	// Micrel power switch to LCD
	gpioError = Gpio_SetPinFunction(gGpioPin13_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin13_c, gGpioDirOut_c);
	gpioError = Gpio_SetPinData(gGpioPin13_c, gGpioPinStateHigh_c);

	// KBI (except KBI4 and KBI7)
	gpioError = Gpio_SetPinDir(gGpioPin22_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin23_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin24_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin25_c, gGpioDirIn_c);
//	gpioError = Gpio_SetPinDir(gGpioPin26_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin27_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin28_c, gGpioDirIn_c);
	gpioError = Gpio_SetPinDir(gGpioPin29_c, gGpioDirIn_c);

	gpioError = Gpio_EnPinPullup(gGpioPin22_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin23_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin24_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin25_c, TRUE);
//	gpioError = Gpio_EnPinPullup(gGpioPin26_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin27_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin28_c, TRUE);
	gpioError = Gpio_EnPinPullup(gGpioPin29_c, TRUE);

	gpioError = Gpio_SelectPinPullup(gGpioPin22_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin23_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin24_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin25_c, gGpioPinPulldown_c);
//	gpioError = Gpio_SelectPinPullup(gGpioPin26_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin27_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin28_c, gGpioPinPulldown_c);
	gpioError = Gpio_SelectPinPullup(gGpioPin29_c, gGpioPinPulldown_c);

	gpioError = Gpio_SetPinReadSource(gGpioPin22_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin23_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin24_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin25_c, gGpioPinReadPad_c);
//	gpioError = Gpio_SetPinReadSource(gGpioPin26_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin27_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin28_c, gGpioPinReadPad_c);
	gpioError = Gpio_SetPinReadSource(gGpioPin29_c, gGpioPinReadPad_c);

	// KBI4 is an output enabled at doze
	gpioError = Gpio_SetPinFunction(gGpioPin26_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin26_c, gGpioDirOut_c);
	gpioError = Gpio_SetPinData(gGpioPin26_c, gGpioPinStateHigh_c);

}

// --------------------------------------------------------------------------

void setupRS485() {
	GpioErr_t gpioError;

	// RS485 Driver controller signal.
	gpioError = Gpio_SetPinFunction(gGpioPin21_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinDir(gGpioPin21_c, gGpioDirOut_c);
	gpioError = Gpio_SetPinData(gGpioPin21_c, gGpioPinStateLow_c);

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

void kbiSleepCallback(void) {
}

// --------------------------------------------------------------------------

void lcdOff() {
	// Turn off the LCD;
	GpioErr_t gpioError;
	gpioError = Gpio_SetPinData(gGpioPin13_c, gGpioPinStateLow_c);

	// Pull the UART1 TX pin low.
	gpioError = Gpio_SetPinFunction(gGpioPin14_c, gGpioNormalMode_c);
	gpioError = Gpio_SetPinData(gGpioPin21_c, gGpioPinStateLow_c);
}

// --------------------------------------------------------------------------

void lcdOn() {
	// Turn off the LCD;
	GpioErr_t gpioError;
	gpioError = Gpio_SetPinData(gGpioPin13_c, gGpioPinStateHigh_c);

	// Restore the UART1 TX pin.
	gpioError = Gpio_SetPinFunction(gGpioPin14_c, gGpioAlternate1Mode_c);
}

// --------------------------------------------------------------------------

pfCallback_t preSleep() {
	setupKbi();

	sendDisplayMessage(CLEAR_DISPLAY, strlen(CLEAR_DISPLAY));
	sendDisplayMessage("SLEEP", 6);
	DelayMs(500);
	lcdOff();
}

// --------------------------------------------------------------------------

void kbiWakeInterruptCallback(void) {
	// Reset the device.
	lcdOn();
	DelayMs(750);
	sendDisplayMessage("WAKE", 4);
	GW_RESET_MCU()
	;
}

// --------------------------------------------------------------------------

void setupKbi(void) {
	crmErr_t crmError;
	ItcErr_t itcError;

	// Setup the KBI handler.
	crmError = CRM_RegisterISR(gCrmKB7WuEvent_c, kbiWakeInterruptCallback);

	crmWuCtrl_t wakeUpCtrl;
	wakeUpCtrl.wuSource = gExtWu_c;
	wakeUpCtrl.ctrl.ext.word = 0;
	wakeUpCtrl.ctrl.ext.bit.kbi7IntEn = TRUE;
	wakeUpCtrl.ctrl.ext.bit.kbi7WuEn = TRUE;
	wakeUpCtrl.ctrl.ext.bit.kbi7WuEvent = 1; // wake on edge
	wakeUpCtrl.ctrl.ext.bit.kbi7WuPol = 1; // wake on positive edge
	wakeUpCtrl.ctrl.timer.timeOut = 0;
	wakeUpCtrl.ctrl.rtc.timeOut = 0;

	crmError = CRM_WuCntl(&wakeUpCtrl);

	itcError = ITC_SetPriority(gCrmInt_c, gItcNormalPriority_c);
	itcError = ITC_EnableInterrupt(gCrmInt_c);

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

	// We don't call this, because we don't want to mess with DPF settings at restart.
	//PlatformPortInit();
	// Setup the radio GPIOs. (Don't know why SMAC doesn't do this.)
	GPIO_REGS_P ->FuncSel2 = gFuncSel2Value_c;

	ITC_Init();
	IntAssignHandler(gMacaInt_c, (IntHandlerFunc_t) MACA_Interrupt);
	ITC_SetPriority(gMacaInt_c, gItcFastPriority_c); // gItcNormalPriority_c
	ITC_EnableInterrupt(gMacaInt_c);
	IntDisableAll();
	MLMERadioInit();
	ResetMaca();

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
#endif

	setupGpio();
	UART_Init(UART_1, 9600, FALSE);
	UART_Init(UART_2, 19200, FALSE);
	setupRS485();
	setupDisplayScroller();

	lcdOff();
	DelayMs(50);
	lcdOn();
//	// Reset to 9600B
//	sendDisplayMessage("\x7c\x12", 2);
//	lcdOff();
//	DelayMs(100);
//	lcdOn();
//	// Set the backlight to 25%
//	sendDisplayMessage("\x7c\x88", 2);
//	lcdOff();
//	DelayMs(100);
//	lcdOn();
//	// Set to 20 chars.
//	sendDisplayMessage("\x7c\x03", 2);
//	lcdOff();
//	DelayMs(100);
//	lcdOn();
//	// Set to 4 lines.
//	sendDisplayMessage("\x7c\x05", 2);
//	lcdOff();
//	DelayMs(100);
//	lcdOn();

	DelayMs(750);

	sendDisplayMessage(CLEAR_DISPLAY, strlen(CLEAR_DISPLAY));
	sendDisplayMessage("DISCONNECTED", 12);

	gLocalDeviceState = eLocalStateStarted;
	GW_RADIO_GAIN_ADJUST(15);
	if (GW_SET_RADIO_CHANNEL(0) == GW_SMAC_SUCCESS) {
	}

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (signed portCHAR *) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask);
	xTaskCreate(radioReceiveTask, (signed portCHAR *) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask);
	//xTaskCreate(keyboardTask, (const signed portCHAR * const) "Keyboard", configMINIMAL_STACK_SIZE, NULL, KEYBOARD_PRIORITY, &gKeyboardTask );
	xTaskCreate(remoteMgmtTask, (signed portCHAR *) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask);
	xTaskCreate(cartControllerTask, (signed portCHAR *) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gCartControllerTask);
	xTaskCreate(scannerReadTask, (signed portCHAR *) "Scan", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gScannerReadTask);

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

//	GW_WATCHDOG_RESET;
//
//	// If we haven't received a packet in by timeout seconds then reset.
//	portTickType ticks = xTaskGetTickCount();
//	if (ticks > (gLastPacketReceivedTick + kNetCheckTickCount)) {
//		GW_RESET_MCU()
//		;
//	}
}
