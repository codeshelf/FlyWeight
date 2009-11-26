/************************************************************************************
* Maca Interrupt implementation
*
* (c) Copyright 2008, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected:
* Last Tested:
************************************************************************************/
#include "../../../PLM/Interface/EmbeddedTypes.h"
#include "../Interface/ghdr/maca.h"
#include "../../../PLM/LibInterface/Platform.h"
#include "../../Interface/RadioManagement.h"
#include "../../Interface/TransceiverConfigMngmnt.h"
#include "../Interface/MacaInterrupt.h"



/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define ZERO (0)

#define _is_action_started_interrupt(x)      (ZERO != (maca_irq_strt & x))
#define _is_sync_detected_interrupt(x)       (ZERO != (maca_irq_sync & x))
#define _is_complete_clock_interrupt(x)      (ZERO != (maca_irq_cm & x))
#define _is_checksum_failed_interrupt(x)     (ZERO != (maca_irq_crc & x))
#define _is_filter_failed_interrupt(x)       (ZERO != (maca_irq_flt & x))
#define _is_soft_complete_clock_interrupt(x) (ZERO != (maca_irq_sftclk & x))
#define _is_fifo_level_interrupt(x)          (ZERO != (maca_irq_lvl & x))
#define _is_reset_interrupt(x)               (ZERO != (maca_irq_rst & x))
#define _is_wake_up_interrupt(x)             (ZERO != (maca_irq_wu & x))
#define _is_data_indication_interrupt(x)     (ZERO != (maca_irq_di & x))
#define _is_poll_indication_interrupt(x)     (ZERO != (maca_irq_poll & x))
#define _is_action_complete_interrupt(x)     (ZERO != (maca_irq_acpl & x))


#define _is_an_unused_interrupt(x)  \
              ( _is_checksum_failed_interrupt(x) || \
                _is_filter_failed_interrupt(x)   || \
                _is_poll_indication_interrupt(x))

#define _is_a_timeout_interrupt(x) \
              ( _is_soft_complete_clock_interrupt(x) || \
                _is_complete_clock_interrupt(x) )

#define maca_status_control_code  ((maca_status) & maca_status_cc_mask)


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


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static uint8_t mMacaIsrEntryCounter = 0;


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
bool_t flag;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* MACA_Interrupt
*
* This function is the MACA Interrupt Service Routine
*
************************************************************************************/
void MACA_Interrupt(void)
{
  uint32_t irqSrc;
  irqSrc = maca_irq;
  maca_clrirq = irqSrc;
  mMacaIsrEntryCounter++;

  if( _is_action_started_interrupt(irqSrc) )
  {
    gRadioEvntFlags.Bits.action_started_flag = TRUE;
  }
  if(_is_data_indication_interrupt(irqSrc)){
    maca_control = (control_seq_abort | control_asap);
    if(FALSE == MLMEGetPromiscuousMode()){
#ifndef __ISM_MODE__
      if( ((0x7E) == *((uint8_t *)(maca_dmarx+1))) && 
          ((0xFF) == *((uint8_t *)(maca_dmarx+2)))){
#else
		if( ((0x7E) == *((uint8_t *)(maca_dmarx+2))) && 
			((0xFF) == *((uint8_t *)(maca_dmarx+3)))){
#endif
        gRadioEvntFlags.Bits.data_indication_flag = TRUE;
        gRadioEvntFlags.Bits.action_complete_flag = TRUE;
        gRadioEvntFlags.Bits.ac_success_flag = TRUE;
        execute_current_message_callback();
      }
      else{
        gRadioEvntFlags.Bits.ac_not_completed_flag = TRUE;
      }
    }
    else{
      gRadioEvntFlags.Bits.data_indication_flag = TRUE;
      gRadioEvntFlags.Bits.action_complete_flag = TRUE;
      gRadioEvntFlags.Bits.ac_success_flag = TRUE;
      execute_current_message_callback();
    }
  }
  if(_is_action_complete_interrupt(irqSrc)){
    gRadioEvntFlags.Bits.action_complete_flag = TRUE;
    execute_current_message_callback();    
    switch(maca_status_control_code){
      case(cc_success):
        {
          gRadioEvntFlags.Bits.ac_success_flag = TRUE;
          // notify main task that a new data request can
          // be sent to MACA;
          flag = TRUE;
        }
        break;
      case(cc_timeout):
        {
          gRadioEvntFlags.Bits.ac_timeout_flag = TRUE;
        }
        break;
      case(cc_channel_busy):
        {
          gRadioEvntFlags.Bits.ac_channel_busy_flag = TRUE;
        }
        break;
      case(cc_crc_fail):
        {
          gRadioEvntFlags.Bits.ac_crc_failed_flag = TRUE;
        }
        break;
      case(cc_aborted):
        {
          gRadioEvntFlags.Bits.ac_aborted_flag = TRUE;
          maca_control = 0x00;
        }
        break;
      case(cc_no_ack):
        {
          /* Not used in SMAC */
        }
        break;
      case(cc_no_data):
        {
          /* Not used in SMAC */
        }
        break;
      case(cc_late_start):
        {
          /* Not used in SMAC */
        }
        break;
      case(cc_ext_timeout):
        {
          gRadioEvntFlags.Bits.ac_ext_timeout_flag = TRUE;
        }
        break;
      case(cc_ext_pnd_timeout):
        {
          gRadioEvntFlags.Bits.ac_ext_pnd_timeout_flag = TRUE;
          /* Not used in SMAC */
        }
        break;
      case(cc_nc1):
      case(cc_nc2):
        {
          /* Not used */
        }
        break;
      case(cc_nc3):
        {
          /* Not used in SMAC */
        }
        break;
      case(cc_cc_external_abort):
        {
          gRadioEvntFlags.Bits.ac_external_abort_flag = TRUE;
          /* Not used in SMAC */
        }
        break;
      case(cc_not_completed):
        {
          gRadioEvntFlags.Bits.ac_not_completed_flag = TRUE;
        }
        break;
      case(cc_bus_error):
        {
          gRadioEvntFlags.Bits.ac_dma_bus_error_flag = TRUE;
        }
        break;
      default:
        {
        }
        break;
    }
  }
  if(_is_sync_detected_interrupt(irqSrc)){
    gRadioEvntFlags.Bits.sync_detected_flag = TRUE;
  }
  if(_is_a_timeout_interrupt(irqSrc)){
    gRadioEvntFlags.Bits.timeout_flag = TRUE;
  }
  if(_is_fifo_level_interrupt(irqSrc)){
  }
  if(_is_reset_interrupt(irqSrc)){
  }
  if(_is_wake_up_interrupt(irqSrc)){
  }
  if(_is_an_unused_interrupt(irqSrc)){
  }
  else {
  }
  mMacaIsrEntryCounter--;
}



/************************************************************************************
*************************************************************************************
* Private Debug stuff
*************************************************************************************
************************************************************************************/


