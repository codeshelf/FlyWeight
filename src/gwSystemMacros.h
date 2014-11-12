/*
 * gwSystemMacros.h
 *
 *  Created on: Mar 10, 2009
 *      Author: jeffw
 */

#ifndef GWSYSTEMMACROS_H_
#define GWSYSTEMMACROS_H_

#if defined(MC1321X) || defined(MC13192EVB)
	#include "simple_mac.h"
	#include "Watchdog.h"

	#if (GW_DEBUG)
		#define GW_RESET_MCU()					debugReset();
	#else
		#define GW_RESET_MCU()					__asm ("BGND")
		//#define	GW_RESET_MCU				__asm DCB 0x8D
	#endif

	#define GW_ENTER_CRITICAL(saveState)		EnterCriticalArg(saveState)
	#define GW_EXIT_CRITICAL(restoreState)		ExitCriticalArg(restoreState)
    #define GW_GET_SYSTEM_STATUS                SRS

	#define	GW_WATCHDOG_RESET					WatchDog_Clear()

	#define GW_USB_INIT							CTS_SETUP; RTS_SETUP; CTS_ON;
	#define GW_USB_OK							ERR_OK
	#define GW_CTS_ON							CTS_ON
	#define GW_CTS_OFF							CTS_OFF
	#define GW_USB_BYTE_READY					SCIS1_RDRF
	#define GW_GET_USB_BYTE						SCID

	#define	GW_SMAC_SUCCESS						SUCCESS
	#define	GW_ENERGY_DETECT(channel)			MLMEEnergyDetect()
	#define GW_RADIO_GAIN_ADJUST(val)			MLMEMC13192PAOutputAdjust(val)
	#define GW_SET_RADIO_CHANNEL(channel)		MLMESetChannelRequest(channel)

    #define GW_PREP_ATD(saveATD1C, saveATD1SC) \
                                        \
        /* Save the ATD state and prepare to take a battery measurement. */ \
	    saveATD1C = ATD1C;      \
	    saveATD1SC = ATD1SC;    \
	    ATD1C_DJM = 1;          \
	    ATD1C_RES8 = 1;         \
	    ATD1SC_ATDCH = 0x04;    \
	    ATD_ON;


    #define GW_MEASURE_BATTERY(outBatteryLevel) \
        /* Start and wait for a ATD conversion. */ \
	    ATD1SC_ATDCO = 0;           \
	    while (!ATD1SC_CCF) {       \
	    }                           \
                                    \
	    /* Get the measurement.     \
	     * Battery values will range from 1.6V to 3.1V, but there is a voltage divide           \
	     * in the circuit, so ATD values will range from 64-128.  The device starts failing     \
	     * around 70, and if we set 78 as the floor then 2x gives us a nice 100 scale.          \
	     * (Since 128 - 78 - 50) */ \
	    outBatteryLevel = ATD1RH - 78;

    #define GW_RESTORE_ATD(restoreATD1C, restoreATD1SC)	\
  	    /* Restore state. */    \
	    ATD1SC = restoreATD1SC;    \
	    ATD1C = restoreATD1C;
#else
	#include "Crm.h"
	#include "board_config.h"
	#include "WirelessLinkMngmt.h"
	#include "RadioManagement.h"
    #include "ITC_Interface.h"
	#include "ADC_Interface.h"
	#include "UART_Interface.h"
	#include "smacPort.h"
	#include "portmacro.h"

	// Define this to debug RX/X packet handing, COP/Watchdog and MCU_RESET
	#define GW_DEBUG							TRUE

//	#if TARGET_BOARD == GW0011
		#if (GW_DEBUG)
			#define GW_RESET_MCU()				debugReset();//CRM_SoftReset()
		#else
			#define GW_RESET_MCU()				CRM_SoftReset()
		#endif
//		#define GW_RESET_MCU()					void (*reset)()=(void (*))0; reset();
//	#else
//		#define GW_RESET_MCU()					CRM_SoftReset()
//	#endif

	#define GW_ENTER_CRITICAL(saveState)		saveState = IntDisableIRQ()
	#define GW_EXIT_CRITICAL(restoreState)		IntRestoreIRQ(restoreState)
    #define GW_GET_SYSTEM_STATUS                0

	#define	GW_WATCHDOG_RESET					CRM_CopReset()

	#define GW_USB_INIT
	#define GW_USB_OK							0
	#define GW_CTS_ON
	#define GW_CTS_OFF
	#define GW_USB_BYTE_READY					TRUE
	#define GW_GET_USB_BYTE						UartReadData()

	#define	GW_SMAC_SUCCESS						gSuccess_c
	#define	GW_ENERGY_DETECT(channel)			portMLMEEnergyDetect(channel)
//	#define GW_RADIO_GAIN_ADJUST(val)			MLMEFEGainAdjust(val)
	#define GW_RADIO_POWER_ADJUST(val)			MLMEPAOutputAdjust(val)
	#define GW_SET_RADIO_CHANNEL(channel)		MLMESetChannelRequest(channel)

	#define GW_PREP_ATD(saveATD1, saveATD1SC)	Adc_StartManualConv(gAdcPrimary_c, gAdcBatt_c)
	#define GW_MEASURE_BATTERY(batteryLevel)	Adc_ManualRead(gAdcPrimary_c, &batteryLevel)
	#define GW_RESTORE_ATD(restoreATD1, restoreATD1SC)

#endif

#endif /* GWSYSTEMMACROS_H_ */
