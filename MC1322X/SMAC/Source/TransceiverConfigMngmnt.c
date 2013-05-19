/************************************************************************************
* Transceiver Configuration Management implementation
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected:
* Last Tested:
************************************************************************************/

#include "../../PLM/LibInterface/Platform.h"
#include "../../PLM/LibInterface/CRM_Regs.h"
#include "../../PLM/LibInterface/GPIO_Interface.h"
#include "RF_Config.h"
#include "options_config.h"
#include "board_config.h"
#include "../Drivers/Interface/Delay.h"
#include "../Drivers/Interface/ghdr/maca.h"
#include "../Drivers/LibInterface/rif_inc.h"
#include "../Interface/TransceiverConfigMngmnt.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define MACA_WRITE(reg, src) (reg = src)

#define _802_15_4_PREAMBLE    0x00000000 

#define MACA_CONTINUOUS_TX    0x00000223
#define MACA_CONTINUOUS_RX    0x00000224

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

#define mMaca_freq_init_c       (SMAC_CHANN_11)
#define mMaca_tmren_init_c      (maca_start_clk | maca_cpl_clk | maca_soft_clk)
#define mMaca_clkoffset_init_c  (0x00000000)

#define mMaca_maskirq_init_c ( maca_irq_strt   | \
                               maca_irq_sync   | \
                               maca_irq_cm     | \
                               maca_irq_sftclk | \
                               maca_irq_lvl    | \
                               maca_irq_di     | \
                               maca_irq_acpl )

#define mMaca_clrirq_init_c   (0x0000FE1F)  /* Clear all interrupt sources at init*/
#define mMaca_fltrej_init_c   (0x00000000)


#define mMaca_preamble_init_c 


#define mSMAC_Header    (uint16_t)(0xFF7E)
#define mCodeBytesSize  (sizeof(mSMAC_Header))
#define mReset_c        (0x87651234)
#define gMaxTrimVal_c   (0x0f)
#define gMaxCoarseTrim_c (0x1f)
#define gBulkCapMask_c   (0x10)
#define gMaxFineTrim_c   (0x1f)
#define gXtalTrimMask_c (0xfff0ffff)
#define ZIGBEE_MODE     (0)
#define ISM_MODE        (1)

typedef struct version_and_size_tag {
  uint32_t * version_add;
  uint8_t word_size;
}version_and_size_tag;


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
/*
   8 bits - Mayor
   8 bits - Minor
   7 bits - Year from 1900
   4 bits - Month (valid 1-12)
   5 bits - Day of the month (valid 1-31)
*/
const uint32_t smac_version = 0x0500D6C7;

static bool_t mIsPromiscousMode = FALSE;
static const version_and_size_tag version_registers[MAX_VERSIONED_ENTITY] = {
  {(uint32_t *)(&maca_version), 1},     /* MACA VERSION */
  {(uint32_t *)(&smac_version), 1},     /* SMAC VERSION */
  {(uint32_t *)(0x00000020),    1},     /* BOOTSTRAP VERSION */
};

static void InitializePhy(void);



/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* MLMESoftReset
*
* This function is called to perform a complete reset of the system.
*
************************************************************************************/
void MLMEPHYSoftReset(void)
{
  MLMEPHYResetIndication();
  CRM_REGS_P->SwRst = mReset_c;
}

/************************************************************************************
* MLMEPHYResetIndication
*
* Callback called when a SoftReset has occurred.
*
************************************************************************************/
void MLMEPHYResetIndication(void)
{
  /* Place your application´s code here */
}

/************************************************************************************
* MLMEPHYXtalAdjust
*
* Adjust the transceiver reference clock by a trim value.
*
************************************************************************************/
FuncReturn_t MLMEPHYXtalAdjust(uint8_t u8CoarseTrim, uint8_t u8FineTrim)
{
  if( (gMaxCoarseTrim_c < u8CoarseTrim) || 
      (gMaxFineTrim_c < u8FineTrim)){
    return gFailOutOfRange_c;
  }
  else {
    /*Do nothing */
  }

  if(gBulkCapMask_c & u8CoarseTrim)
  {
    enable_bulk_cap();
  }
  else
  {
    disable_bulk_cap();
  }

  set_xtal_coarse_tune((~gBulkCapMask_c) & u8CoarseTrim);
  set_xtal_fine_tune(u8FineTrim);

  return gSuccess_c;
  
}


/************************************************************************************
* MLMEGetRficVersion
*
* This function shall provide the calling function with version of the IC and the 
* version of the software.
*
************************************************************************************/
FuncReturn_t MLMEGetRficVersion(Versioned_Entity_t Entity, uint32_t *Buffer)
{
  uint8_t i;

  if(MAX_VERSIONED_ENTITY <= Entity){
    return gFailOutOfRange_c;
  }
  else{
    /* Do nothing */
  }

  for(i=0; i<version_registers[Entity].word_size; i++)
  {
    *Buffer++ = *((version_registers[Entity].version_add)+i);
  }

  return gSuccess_c;
}

/************************************************************************************
* MLMETestMode
*
* This SMAC primitive was added to place the device into MACA tests modes.
*
************************************************************************************/
void MLMETestMode (Test_Mode_t u8Mode)
{
  disable_test_mode_continuos_tx_nomod();

  switch(u8Mode){
    case(SMAC_TEST_MODE_IDLE):
      {
        maca_control = SMAC_MACA_CNTL_INIT_STATE;
      }
      break;
          
    case(SMAC_TEST_MODE_CONTINUOUS_TX_NOMOD):
      {
        maca_control = SMAC_MACA_CNTL_INIT_STATE;
        DelayMs(2);
        set_test_mode_continuos_tx_nomod();
      }
      break;
      
    case(SMAC_TEST_MODE_CONTINUOUS_TX_MOD):
      {
        maca_control = SMAC_MACA_CNTL_INIT_STATE;
        DelayMs(2);
        maca_control = MACA_CONTINUOUS_TX;
      }
      break;
      
    case(SMAC_TEST_MODE_CONTINUOUS_RX):
      {
        maca_control = SMAC_MACA_CNTL_INIT_STATE;
        DelayMs(2);
        maca_control = MACA_CONTINUOUS_RX;
      }
      break;    
    
    default:
      maca_control = SMAC_MACA_CNTL_INIT_STATE;
      break;
  }
}


/************************************************************************************
* MLMESetPromiscuousMode
*
* This function sets the promiscous mode to on/off
*
************************************************************************************/
FuncReturn_t MLMESetPromiscuousMode(bool_t isPromiscousMode)
{
  mIsPromiscousMode = isPromiscousMode;
  return gSuccess_c;
}

/************************************************************************************
* MLMEGetPromiscuousMode
*
* This function gets the current promiscous mode to on/off.
*
************************************************************************************/
bool_t MLMEGetPromiscuousMode(void)
{
  return mIsPromiscousMode;
}

/************************************************************************************
* MLMERadioInit
*
* This function configures the different parameters of the MAC accelerator and the 
* radio interface
*
************************************************************************************/
FuncReturn_t MLMERadioInit(void)
{
  static uint8_t i=0;  
  uint32_t u32LoopDiv;  

  u32LoopDiv = ((gDigitalClock_RN_c<<25) + gDigitalClock_RAFC_c);

  if (0==i){
    i = 1;
    RadioInit(PLATFORM_CLOCK, gDigitalClock_PN_c, u32LoopDiv);
    MLMEPHYXtalAdjust(DEFAULT_COARSE_TRIM, DEFAULT_FINE_TRIM);
    MLMESetPromiscuousMode(SMAC_PROMISCUOUS_MODE);
    radio_manager_init();
  }
  
  if(PLATFORM_CLOCK != 24000000)
  {
    InitIdleToRun();
  }
  
  InitializePhy(); // Re-init MACA

  return gSuccess_c;
}


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* InitializePhy
*
* This function configures the different parameters of the MAC accelerator and the 
* radio interface
*
************************************************************************************/
static void InitializePhy(void)
{
  volatile uint8_t cnt;
  
  MACA_WRITE(maca_reset, maca_reset_rst);
 
  for(cnt=0; cnt < 100; cnt++); 

  MACA_WRITE(maca_reset      , maca_reset_cln_on);                      // 0x80004004
  MACA_WRITE(maca_control    , control_seq_nop);                        // 0x8000400c
  MACA_WRITE(maca_tmren      , maca_start_clk | maca_cpl_clk);          // 0x80004040
  MACA_WRITE(maca_divider    , gMACA_Clock_DIV_c);                      // 0x80004114
  MACA_WRITE(maca_warmup     , 0x00180012);                             // 0x80004118
  MACA_WRITE(maca_eofdelay   , 0x00000004);                             // 0x80004148
  MACA_WRITE(maca_ccadelay   , 0x001a0022);                             // 0x8000414c
  MACA_WRITE(maca_txccadelay , 0x00000025);                             // 0x80008154
  MACA_WRITE(maca_framesync  , 0x000000A7);                             // 0x80004124
  MACA_WRITE(maca_clk        , 0x00000008);                             // 0x80004048
  MACA_WRITE(maca_maskirq    , (maca_irq_cm   | maca_irq_acpl | 
                                maca_irq_rst  | maca_irq_di) );
  MACA_WRITE(maca_slotoffset , 0x00350000);                             // 0x80004064
  // MACA

}


/************************************************************************************
* ConfigureRfCtlSignals()
*
* This function ...
*
************************************************************************************/
void ConfigureRfCtlSignals(RfSignalType_t signalType, RfSignalFunction_t function, bool_t gpioOutput, bool_t gpioOutputHigh )
{
 /* Validate the input paramters */
 if( (signalType >= gRfSignalMax_c) || (function >= gRfSignalFunctionMax_c) )
 {
  /* SignalType or function out of range */ 
  return;
 }
 
 if( (signalType < gRfSignalTXON_c) && (function > gRfSignalFunction1_c) )
 {
  /* Function2 requested for ANT_1 or ANT_2 */
  return;
 }
 
 if(function > 0)
 {
  /* Function 1 or 2 was requested for the RF pin. Apply the change at the GPIO function level for the requested signal */
  Gpio_SetPinFunction((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), (GpioFunctionMode_t)function); 
 }
 else
 {
  /* GPIO mode requested for the RF pin. Apply the function, direction and level for the requested signal */
  Gpio_SetPinFunction((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), gGpioNormalMode_c); 
  if(gpioOutput == TRUE)
  {  
   Gpio_SetPinDir((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), gGpioDirOut_c);
   Gpio_SetPinReadSource((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), gGpioPinReadReg_c);
   Gpio_SetPinData((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), (GpioPinState_t)gpioOutputHigh); 
  }
  else
  {
   Gpio_SetPinDir((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), gGpioDirIn_c);
   Gpio_SetPinReadSource((GpioPin_t)((GpioPin_t)gGpioPin42_c+(GpioPin_t)signalType), gGpioPinReadPad_c);
  }  
 }
 
}


/************************************************************************************
* ConfigureBuckRegulator
*
* This function enables or bypass the buck regulator 
*
************************************************************************************/
void ConfigureBuckRegulator(BuckTypes_t BuckRegState)
{
 
  if(BUCK_DISABLE == BuckRegState)
  {
    CRM_REGS_P->SysCntl &= 0xFFFFFFFE;
    CRM_REGS_P->VregCntl = 0x00000F78;
  }
  if(BUCK_ENABLE == BuckRegState)
  {
    CRM_REGS_P->SysCntl |= BIT0;  //Enables buck regutation as power supply
    CRM_REGS_P->VregCntl = 0x00000F7B;
  }
  
  if(BUCK_BYPASS == BuckRegState)
  {
    CRM_REGS_P->SysCntl &= 0xFFFFFFFE;
    CRM_REGS_P->VregCntl = 0x00000F7C;
  }
}


/************************************************************************************
*************************************************************************************
* Private Debug stuff
*************************************************************************************
************************************************************************************/


