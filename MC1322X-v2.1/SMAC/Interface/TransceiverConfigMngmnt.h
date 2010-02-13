/************************************************************************************
* This header file 
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _TRANSCEIVER_CONFIG_MNGMNT_
#define _TRANSCEIVER_CONFIG_MNGMNT_

#include "GlobalDefs.h"
#include "RadioManagement.h"
#include "../Drivers/LibInterface/Synthesizer_inc.h"

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef enum Test_Mode_tag
{
  SMAC_TEST_MODE_IDLE = 0,
  SMAC_TEST_MODE_PULSE_PRBS9_TX_RX,  
  SMAC_TEST_MODE_PER_TX_RX,
  SMAC_TEST_MODE_RANGE_TX_RX,
  SMAC_TEST_MODE_CONTINUOUS_TX_MOD,
  SMAC_TEST_MODE_CONTINUOUS_TX_NOMOD,
  SMAC_TEST_MODE_CONTINUOUS_RX,
  SMAC_MAX_TEST_MODES
} Test_Mode_t;

#define MAX_TEST_MODES  (SMAC_MAX_TEST_MODES - 1)

typedef enum Speed_Mode_tag
{
  SMAC_SPEEDMODE_LOW = 0,
  SMAC_SPEEDMODE_HIGH,
  SMAC_MAX_SPEEDMODE
} Speed_Mode_t;

typedef enum Versioned_Entity_tag
{
//HW_JTAG_ID_REGISTER = 0,
  HW_MACA_MC1322X_ID,
  SW_SMAC_VERSION,
  SW_BOOTSTRAP_VERSION,
  MAX_VERSIONED_ENTITY
}Versioned_Entity_t;



// Valid values for ConfigureRFCtlSignals signalType
typedef enum {
  gRfSignalANT1_c,
  gRfSignalANT2_c,
  gRfSignalTXON_c,
  gRfSignalRXON_c,
  gRfSignalMax_c
}RfSignalType_t;


// Valid values for ConfigureRFCtlSignals signalFunction
typedef enum {
  gRfSignalFunctionGPIO_c,
  gRfSignalFunction1_c,
  gRfSignalFunction2_c,
  gRfSignalFunctionMax_c
}RfSignalFunction_t;


typedef union jtag_id_reg_tag
{
  struct
  {
    uint32_t  RESERVED1:1;
    uint32_t  MANUFACTURER_ID:11;
    uint32_t  PART_NUMBER:16;
    uint32_t  VERSION:4;
  } Bits;
  uint32_t Reg;
} jtag_id_reg_t;


typedef enum BuckTypes_tag
{
  BUCK_DISABLE = 0, 
  BUCK_BYPASS,
  BUCK_ENABLE
}BuckTypes_t;


#define set_xtal_coarse_tune(x)   (CRM_XTAL_CNTL.cTune = x)
#define set_xtal_fine_tune(x)     (CRM_XTAL_CNTL.fTune = x)
#define enable_bulk_cap()         (CRM_XTAL_CNTL.bulktune = 1)
#define disable_bulk_cap()        (CRM_XTAL_CNTL.bulktune = 0)


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* This function is called to perform a complete reset of the system.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
void MLMEPHYSoftReset(void);


/************************************************************************************
* This callback function needs to be filled by the application. This allows the SMAC 
* to call this function when a SoftReset has occurred.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
void MLMEPHYResetIndication(void);



/************************************************************************************
* This function is used to adjust the transceiver reference clock by a trim value.
* The trim is a four bits value as specified at the CRM documentation.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMEPHYXtalAdjust(uint8_t u8CoarseTrim, uint8_t u8FineTrim);



/************************************************************************************
* This function shall provide the calling function with version of the IC and the 
* version of the software.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMEGetRficVersion(Versioned_Entity_t Entity, uint32_t *Buffer);



/************************************************************************************
* This SMAC primitive was added to place the device into MACA tests modes.
*
* Interface assumptions: The use of this primitive could cause to abort any message
*                       in process.
*                        It is recommended use it just for testing propouses and 
*                       to clear the message queu before using this function.
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
void MLMETestMode (Test_Mode_t u8Mode);


/************************************************************************************
* This function configures the different parameters of the MAC accelerator and the 
* radio interface
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMERadioInit(void);

/************************************************************************************
* This function sets the promiscous mode to on/off.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
FuncReturn_t MLMESetPromiscuousMode(bool_t isPromiscousMode);


/************************************************************************************
* This function gets the current promiscous mode to on/off.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
bool_t MLMEGetPromiscuousMode(void);


/************************************************************************************
* This function configures the RF control signals.
*
* Interface assumptions:
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
void ConfigureRfCtlSignals(RfSignalType_t, RfSignalFunction_t, bool_t, bool_t);


/************************************************************************************
* This function set the buck regulator state.
*
* Interface assumptions: 
*
* Return value:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
void ConfigureBuckRegulator(BuckTypes_t BuckRegState);


/**********************************************************************************/

#endif /* _TRANSCEIVER_CONFIG_MNGMNT_ */
