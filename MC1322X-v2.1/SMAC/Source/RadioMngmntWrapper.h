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
#ifndef _RADIO_MNGMT_WRAPPER_
#define _RADIO_MNGMT_WRAPPER_

#ifdef MC1322X_GCC
#pragma interface "RadioMngmntWrapper.h"
#endif

#include "../Interface/GlobalDefs.h"
#include "../Drivers/Interface/ghdr/maca.h"
#include "../Drivers/LibInterface/Synthesizer_inc.h"
#include "../Interface/TransceiverConfigMngmnt.h"
#include "options_config.h"
#include "board_config.h"

extern void DelayMs(uint32_t);
extern void DelayUs(uint32_t);
extern const uint8_t gaRFSynVCODivI_c[16];
extern const uint32_t gaRFSynVCODivF_c[16];

extern uint8_t SetChannel(uint8_t channel,  uint8_t RFSynVCODivI, uint32_t RFSynVCODivF);

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/


#define reg_RxAgcCcaEdOut    (*((volatile uint32_t *)(0x80009488)))    
//#define _get_ed_value()       (maca_edvalue)
#define _get_ed_value()       (reg_RxAgcCcaEdOut)


#define _set_channel(x)  SetChannel(x,gaRFSynVCODivI_c[x],gaRFSynVCODivF_c[x])


#define _get_repeater_timer()  (crm_rtc_count)
#define _set_startclk(x)      (maca_startclk = maca_clk + x)
#define _set_cplclk(x)        (maca_cplclk = maca_clk + x)
#define _set_sftclk(x)        (maca_sftclk = maca_clk + x)

/* Radio commands section */
#define command_xcvr_abort() \
  do { \
        maca_tmren = (0); \
        maca_control = (control_asap | control_seq_abort); \
  } while (FALSE)

#define hw_rst_xcvr() (MLMERadioInit())


#ifdef MC1322X_GCC
static __inline uint32_t command_xcvr_ed (uint8_t chann)
#else
static inline uint32_t command_xcvr_ed (uint8_t chann)
#endif
  { 
          uint32_t tmpChannel;
          tmpChannel = GetCurrentChannel(gaRFSynVCODivI_c, gaRFSynVCODivF_c);
          _set_channel(chann);
          maca_control = (control_prm | control_asap | control_seq_ed );
          return tmpChannel;
  }


#define command_xcvr_rx() \
  do { \
        maca_txlen = (uint32_t)((maMessage_Queu[current_msg]->u8BufSize) << 16); \
        maca_dmatx = (uint32_t)&ackBox; \
        maca_dmarx = (uint32_t)(&(maMessage_Queu[current_msg]->pu8Buffer->reserved[0])); \
        if(0 != mCplclk_offset){ \
          _set_cplclk(mCplclk_offset); \
          _set_sftclk(mSftclk_offset); \
          maca_tmren = (maca_cpl_clk | maca_soft_clk); \
        } \
        else{ \
          maca_tmren = (0); \
        } \
        maca_control = (control_prm | control_asap | control_seq_rx); \
  }while(FALSE)


#define command_xcvr_to() \
  do { \
        _set_cplclk((uint32_t)(*(maMessage_Queu[current_msg]->pu8Buffer->u8Data))); \
        _set_sftclk((uint32_t)(*(maMessage_Queu[current_msg]->pu8Buffer->u8Data))); \
        maca_control = (control_prm | control_asap | control_seq_wait); \
  }while(FALSE)


#define command_xcvr_tx() \
  do { \
        maca_txlen = (uint32_t)((maMessage_Queu[current_msg]->u8BufSize)+4); \
        maca_dmatx = (uint32_t)(maMessage_Queu[current_msg]->pu8Buffer->reserved); \
        maca_dmarx = (uint32_t)&ackBox; \
        maca_control = (control_prm | control_mode_no_cca | \
                        control_asap | control_seq_tx); \
  }while(FALSE)

#define get_rx_packet_sz() (((maMessage_Queu[current_msg])->pu8Buffer->reserved[0])-4)

/* Radio events section */
#define clear_all_radio_events() \
  do{ \
    gRadioEvntFlags.Reg = (0x00000000); \
  }while(FALSE)


#define is_action_completed_evnt() (TRUE == gRadioEvntFlags.Bits.action_complete_flag)
#define clear_action_completed_evnt() \
  do { \
    gRadioEvntFlags.Bits.action_complete_flag = FALSE; \
  }while(FALSE)

#define is_action_started_evnt() \
                ( (TRUE == gRadioEvntFlags.Bits.action_started_flag) || \
                  is_action_completed_evnt())

#define clear_action_started_evnt() \
  do { \
    gRadioEvntFlags.Bits.action_started_flag = FALSE; \
  }while(FALSE)



#define is_tx_action_completed_evnt()    is_action_completed_evnt()
#define clear_tx_action_completed_evnt() clear_action_completed_evnt()
#define is_rx_action_completed_evnt()    is_action_completed_evnt()
#define clear_rx_action_completed_evnt() clear_action_completed_evnt()



#define is_ac_success_evnt() (TRUE == gRadioEvntFlags.Bits.ac_success_flag)
#define clear_ac_success_evnt() \
  do { \
    gRadioEvntFlags.Bits.ac_success_flag = FALSE; \
  }while(FALSE)

#define is_tx_ac_success_evnt()    is_ac_success_evnt()
#define clear_tx_ac_success_evnt() clear_ac_success_evnt()
#define is_rx_ac_success_evnt()    is_ac_success_evnt()
#define clear_rx_ac_success_evnt() clear_ac_success_evnt()

#define is_ac_timeout_evnt() (TRUE == gRadioEvntFlags.Bits.ac_timeout_flag)
#define clear_ac_timeout_evnt() \
  do { \
    gRadioEvntFlags.Bits.ac_timeout_flag = FALSE; \
  }while(FALSE)
#define is_timeout_evnt() ((TRUE == gRadioEvntFlags.Bits.timeout_flag) | \
                           (TRUE == gRadioEvntFlags.Bits.ac_ext_timeout_flag) | \
                           (TRUE == gRadioEvntFlags.Bits.ac_ext_pnd_timeout_flag))
#define clear_timeout_evnt() \
  do { \
    gRadioEvntFlags.Bits.timeout_flag = FALSE; \
    gRadioEvntFlags.Bits.ac_ext_timeout_flag = FALSE; \
    gRadioEvntFlags.Bits.ac_ext_pnd_timeout_flag = FALSE; \
  }while(FALSE)

#define is_action_completed_fail() ((TRUE == gRadioEvntFlags.Bits.ac_crc_failed_flag) || \
                                    (TRUE == gRadioEvntFlags.Bits.ac_not_completed_flag) || \
                                    (TRUE == gRadioEvntFlags.Bits.checksum_failed_flag) || \
                                    (TRUE == gRadioEvntFlags.Bits.filter_failed_flag)) 

#define clear_complete_fail_evnt() \
  do { \
    gRadioEvntFlags.Bits.ac_crc_failed_flag = FALSE; \
    gRadioEvntFlags.Bits.ac_not_completed_flag = FALSE; \
    gRadioEvntFlags.Bits.checksum_failed_flag = FALSE; \
    gRadioEvntFlags.Bits.filter_failed_flag = FALSE; \
  }while(FALSE)

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/


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
* This function ...
*
* Interface assumptions: 
*
* Routine limitations. 
*
* Return value: void.
*
* Effects on global data:
*
* Source of algorithm used:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/


/**********************************************************************************/

#endif /* _RADIO_MNGMT_WRAPPER_ */



