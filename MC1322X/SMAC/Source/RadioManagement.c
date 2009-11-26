/************************************************************************************
* Radio Management implementation
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

#include "../Interface/RadioManagement.h"
#include "../Drivers/Interface/Delay.h"
#include "RadioMngmntWrapper.h"

#define PROCESS_TO_ENBLE


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mStartclk_init_c    (0x00000080)
#define mCplclk_init_c      (0x00001E00)
#define mSftclk_init_c      (0x000001FE)

#define is_any_action_complete_evnt() \
                      ( is_tx_action_completed_evnt() || \
                        is_rx_action_completed_evnt() || \
                        is_action_completed_evnt() )

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
static FuncReturn_t process_tx_msg (void);
static FuncReturn_t process_rx_msg (void);
static FuncReturn_t process_ed_msg (void);
static FuncReturn_t process_to_msg (void);
static uint8_t calculate_ed_value (uint32_t);


static bool_t is_a_handled_message(message_t * msg);
const uint8_t gaRFSynVCODivI_c[16] = gaRFSynVCODivIVal_c;
const uint32_t gaRFSynVCODivF_c[16] = gaRFSynVCODivFVal_c;

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
#define MAX_NUM_MSG (4)
#define PROCESS_TIMEOUT  (0xffff)

#define _inc_msg(x) (((MAX_NUM_MSG-1) == x)? 0: x+1)

#ifdef PROCESS_TO_ENBLE
  #define _validate_processs_timeout(x) \
     do { \
       if(gNull_c == mProcess_timeout){ \
         (maMessage_Queu[current_msg])->u8Status.msg_state = x; \
         release_current_message(); \
         hw_rst_xcvr(); \
       } \
       else { \
         /* Do nothing */ \
       } \
    } while(FALSE)
#endif

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static uint8_t mRetries_timmer;

#ifdef PROCESS_TO_ENBLE
  static uint16_t mProcess_timeout;
#endif

static uint8_t current_msg;
static uint8_t next_msg_to_store;
static message_t* maMessage_Queu[MAX_NUM_MSG];
static uint32_t maMessageTO[MAX_NUM_MSG];

//static uint32_t mStartclk_offset = mStartclk_init_c;
static uint32_t mCplclk_offset   = mCplclk_init_c;
static uint32_t mSftclk_offset   = mSftclk_init_c;

#define _set_startclk_offset(x) (mStartclk_offset = x)
#define _set_cplclk_offset(x)   (mCplclk_offset = x)
#define _set_sftclk_offset(x)   (mSftclk_offset = x)


/* This constant must keep the same order than the msg_type_tag enumeration */
static const callback_t cbProcessMsg_c[MAX_MSG_TYPE] = {
          (callback_t)(process_tx_msg),
          (callback_t)(process_rx_msg),
          (callback_t)(process_ed_msg),
          (callback_t)(process_to_msg) };

static const uint8_t mAbortStates[MAX_MSG_TYPE] = {
          MSG_TX_RQST_ABORT,
          MSG_RX_RQST_ABORT,
          MSG_ED_RQST_ABORT,
          MSG_TO_RQST_ABORT };


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
evnt_flags_t gRadioEvntFlags;
// when doing Tx MACA requires a valid pointer in maca_dmarx
// in order to handle possible incoming ACK frames;
// this space can also be allocated dinamically;
uint32_t ackBox[10];

message_t Scan_msg;
vScanCallback_t vScanReqCallBack;
uint16_t u16ChannToScan;
uint8_t u8ScanValPerChann[TOTAL_CHANN];
uint8_t dataScan[1];
bool_t bScanReqFlag;


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
void process_scan_req(void);
channel_num_t get_next_chann_to_scan(channel_num_t);
extern FuncReturn_t MLMEEnergyDetect  (message_t *, channel_num_t);


/************************************************************************************
* abort_message
*
* This function aborts a message
*
************************************************************************************/
FuncReturn_t abort_message (message_t * msg){
  if(FALSE == is_a_handled_message(msg)){
    return gFailOutOfRange_c;
  }
  else{
    /* Do Nothing */
  }
  if(maMessage_Queu[current_msg] == msg){
    command_xcvr_abort();
  }
  else{
    /* Do Nothing */
  }
  msg->u8Status.msg_state = (mAbortStates[msg->u8Status.msg_type]);
  return gSuccess_c;
}

/************************************************************************************
* is_a_handled_message
*
* This function check if a given message is placed at the queu
*
************************************************************************************/
static bool_t is_a_handled_message(message_t * msg){
  uint8_t i;
  for(i=0; i<MAX_NUM_MSG; i++){
    if(maMessage_Queu[i] == msg){
      return TRUE;
    }
    else{
      /* Do Nothing */
    }
  }
  return FALSE;
}


/************************************************************************************
* change_current_message_state
*
* This function the current message state to the "mssg_state"
*
************************************************************************************/
void change_current_message_state (any_mssg_state_t mssg_state){
  if(NULL != maMessage_Queu[current_msg]){
    (maMessage_Queu[current_msg])->u8Status.msg_state = (mssg_state.any_type);
  }
  else{
    /* Do Nothing */
  }
}


/************************************************************************************
* change_current_message_state
*
* This function the callback assosiated with the current message
*
************************************************************************************/
void execute_current_message_callback (void){
  if(NULL != maMessage_Queu[current_msg]){
    if(NULL != ((maMessage_Queu[current_msg])->cbDataIndication)){
      (maMessage_Queu[current_msg])->cbDataIndication();
    }
    else {
      /* Do Nothing */
    }
  }
  else{
    /* Do Nothing */
  }
}

/************************************************************************************
* get_current_message
*
* This function returns a pointer to the message that is been processed, it will
* return NULL if there is no message.
*
************************************************************************************/
message_t* get_current_message (void){
  return (maMessage_Queu[current_msg]);
}


/************************************************************************************
* handle_new_message
*
* This function link a new msg to the queu.
*
************************************************************************************/
FuncReturn_t handle_new_message (message_t * msg, uint32_t timeout){
  if(NULL != maMessage_Queu[next_msg_to_store]){
    return gFailNoResourcesAvailable_c;
  }
  else{
    msg->u8Status.msg_state = initial_state_c;
    maMessage_Queu[next_msg_to_store] = msg;
    maMessageTO[next_msg_to_store]= timeout;
    next_msg_to_store = _inc_msg(next_msg_to_store);
  }
  return gSuccess_c;
}


/************************************************************************************
* release_current_message
*
* This function releases a message from the queu.
*
************************************************************************************/
FuncReturn_t release_current_message (void){
  if(NULL != maMessage_Queu[current_msg]){
    maMessage_Queu[current_msg] = NULL;
    maMessageTO[current_msg] = 0;
    current_msg = _inc_msg(current_msg);
  }
  else{
    /* Do Nothing */
  }
  return gSuccess_c;
}


/************************************************************************************
* radio_manager_init
*
* This initializes all the variables involved in the radio management.
*
************************************************************************************/
FuncReturn_t radio_manager_init (void){
  uint8_t queu_count;
  for(queu_count=0; queu_count<MAX_NUM_MSG; queu_count++){
    maMessage_Queu[queu_count] = NULL;
    maMessageTO[queu_count] = 0;
  }
  current_msg = 0;
  next_msg_to_store = 0;
  clear_all_radio_events();
  bScanReqFlag = FALSE;
  MSG_INIT(Scan_msg, &dataScan, NULL);
  Scan_msg.u8Status.msg_state = MSG_ED_ACTION_COMPLETE_FAIL;
  Scan_msg.u8Status.msg_type = ED;
  u16ChannToScan = 0xFFFF;
  return gSuccess_c;
}

/************************************************************************************
* process_radio_msg
*
* This function process any TX/RX/EnergyDetect/Timeout pending msg.
*
************************************************************************************/
FuncReturn_t process_radio_msg (void){

  if(NULL != maMessage_Queu[current_msg]){
    if(MAX_MSG_TYPE > (maMessage_Queu[current_msg])->u8Status.msg_type){
      cbProcessMsg_c[(maMessage_Queu[current_msg])->u8Status.msg_type]();
      DelayUs(9);
    }
    else {
      /* Do Nothing */
    }
  }
  else {
    /* Do Nothing */
  }

  if(TRUE == bScanReqFlag)
  {
    process_scan_req();
  }

  return gSuccess_c;
}


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* process_tx_msg
*
* This function process any TX pending msg.
*
************************************************************************************/
static FuncReturn_t process_tx_msg (void){
  msg_tx_state_t state;
  state = (msg_tx_state_t)((maMessage_Queu[current_msg])->u8Status.msg_state);
  switch(state)
  {
    case MSG_TX_RQST:
      {
        clear_all_radio_events();
        mRetries_timmer = MAX_TX_RETRIES;

#ifdef PROCESS_TO_ENBLE
        mProcess_timeout = PROCESS_TIMEOUT;
#endif
        command_xcvr_tx();
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_TX_PASSED_TO_DEVICE;
      }
      break;
    case MSG_TX_PASSED_TO_DEVICE:
      {
        if(is_action_started_evnt()){
          clear_action_started_evnt();
          (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_TX_ACTION_STARTED;
        }
        else{
#ifdef PROCESS_TO_ENBLE
          _t_dec(mProcess_timeout);
#endif
        }
      }
      break;
    case MSG_TX_ACTION_STARTED:
      {
        if(is_tx_action_completed_evnt()){
          clear_tx_action_completed_evnt();
          if(is_tx_ac_success_evnt()) {
            clear_tx_ac_success_evnt();
            (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                       MSG_TX_ACTION_COMPLETE_SUCCESS;
            release_current_message();
          }
          else {
            if(gNull_c != mRetries_timmer){
              _t_dec(mRetries_timmer);
              clear_all_radio_events();
              command_xcvr_tx();
              (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                              MSG_TX_PASSED_TO_DEVICE;
            }
            else {
              (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                          MSG_TX_ACTION_COMPLETE_FAIL;
              release_current_message();
            }
          }
        }
        else {
#ifdef PROCESS_TO_ENBLE
          _t_dec(mProcess_timeout);
#endif
        }
      }
      break;
    case MSG_TX_RQST_ABORT:
      {
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_TX_ABORTED;
        release_current_message();
      }
      break;
    case MSG_TX_ABORTED:
    case MSG_TX_ACTION_COMPLETE_SUCCESS:
    case MSG_TX_ACTION_COMPLETE_FAIL:
    default:
      {
        release_current_message();
      }
      break;
  }
#ifdef PROCESS_TO_ENBLE

  _validate_processs_timeout(MSG_TX_ACTION_COMPLETE_FAIL);
#endif
  return gSuccess_c;
}

/************************************************************************************
* process_rx_msg
*
* This function process any RX pending msg.
*
************************************************************************************/
static FuncReturn_t process_rx_msg (void){
  static bool_t is_there_any_time_out;
  msg_rx_state_t state;
  state = (msg_rx_state_t)((maMessage_Queu[current_msg])->u8Status.msg_state);
  switch(state)
  {
    case MSG_RX_RQST:
      {
    	  // JBW/GW - 19MAR09: Fix timeout problems
    	  is_there_any_time_out = (maMessageTO[current_msg] != 0);
//        is_there_any_time_out = FALSE;
//        is_there_any_time_out = TRUE;
        clear_all_radio_events();
        mRetries_timmer = MAX_RX_RETRIES;
#ifdef PROCESS_TO_ENBLE
//        mProcess_timeout = PROCESS_TIMEOUT;
        mProcess_timeout = 0x0130;
#endif
        _set_cplclk_offset(maMessageTO[current_msg]);
        command_xcvr_rx();
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_RX_PASSED_TO_DEVICE;
      }
      break;
    case MSG_RX_PASSED_TO_DEVICE:
      {
#ifdef PROCESS_TO_ENBLE
          _t_dec(mProcess_timeout);
#endif
        if(is_action_started_evnt()){
          clear_action_started_evnt();
          (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_RX_ACTION_STARTED;
        }
        else{
          /* Do nothing */
        }
      }
      break;
    case MSG_RX_ACTION_STARTED:
      {
        if(is_rx_action_completed_evnt()){
          clear_rx_action_completed_evnt();
          if(is_rx_ac_success_evnt()) {
            clear_rx_ac_success_evnt();
            (maMessage_Queu[current_msg])->u8BufSize = get_rx_packet_sz();
            (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                       MSG_RX_ACTION_COMPLETE_SUCCESS;
            release_current_message();
          }

          else {
            if(is_timeout_evnt()){
                clear_timeout_evnt();
                (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_RX_TIMEOUT_FAIL;
                release_current_message();
            }
            else {
              if(gNull_c != mRetries_timmer){
                _t_dec(mRetries_timmer);
                clear_all_radio_events();
                (maMessage_Queu[current_msg])->u8BufSize = (0x00);
                command_xcvr_rx();
                (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                              MSG_RX_PASSED_TO_DEVICE;
              }
//              else {
              if(gNull_c == mRetries_timmer){

                (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                          MSG_RX_ACTION_COMPLETE_FAIL;

                release_current_message();

              }
            }
          }

        }
        else {
#ifdef PROCESS_TO_ENBLE
          _t_dec(mProcess_timeout);
#endif
        }
      }
      break;
    case MSG_RX_SYNC_FOUND:
      {
        /* Not used at this time */
      }
      break;
    case MSG_RX_RQST_ABORT:
      {
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_RX_ABORTED;
        release_current_message();
      }
      break;
    case MSG_RX_ABORTED:
    case MSG_RX_TIMEOUT_FAIL:
    case MSG_RX_ACTION_COMPLETE_SUCCESS:
    case MSG_RX_ACTION_COMPLETE_FAIL:
    default:
      {
        release_current_message();
      }
      break;
  }
#ifdef PROCESS_TO_ENBLE
  if(TRUE == is_there_any_time_out){
    _validate_processs_timeout(MSG_RX_ACTION_COMPLETE_FAIL);
  }
#endif
  return gSuccess_c;
}

/************************************************************************************
* process_ed_msg
*
* This function process any Energy Detect pending msg.
*
************************************************************************************/
static FuncReturn_t process_ed_msg (void){
  static uint32_t prev_chann;
  msg_ed_state_t state;
  state = (msg_ed_state_t)((maMessage_Queu[current_msg])->u8Status.msg_state);
  switch(state)
  {
    case MSG_ED_RQST:
      {
        clear_all_radio_events();
        mRetries_timmer = MAX_ED_RETRIES;
#ifdef PROCESS_TO_ENBLE
        mProcess_timeout = PROCESS_TIMEOUT;
#endif
        /* Use u8BufSize to store the desired channel */
        if((uintn8_t)MAX_SMAC_CHANNELS > ((maMessage_Queu[current_msg])->u8BufSize)){
          prev_chann = command_xcvr_ed((maMessage_Queu[current_msg])->u8BufSize);
          (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_ED_PASSED_TO_DEVICE;
        }
        else{
          (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                          MSG_ED_ACTION_COMPLETE_FAIL;
          release_current_message();
        }
      }
      break;
    case MSG_ED_PASSED_TO_DEVICE:
      {
        if(is_action_completed_evnt()){
          clear_action_completed_evnt();
          if(is_ac_success_evnt()) {
            clear_ac_success_evnt();
            (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                       MSG_ED_ACTION_COMPLETE_SUCCESS;
            *((maMessage_Queu[current_msg])->pu8Buffer->u8Data) = calculate_ed_value(_get_ed_value());
            _set_channel(prev_chann);
            release_current_message();
          }
          else {
            if(gNull_c != mRetries_timmer){
              _t_dec(mRetries_timmer);
              clear_all_radio_events();
              (uint32_t)command_xcvr_ed((maMessage_Queu[current_msg])->u8BufSize);
            }
            else {
              (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                          MSG_ED_ACTION_COMPLETE_FAIL;
              _set_channel(prev_chann);
              release_current_message();
            }
          }
        }
        else {
#ifdef PROCESS_TO_ENBLE
          _t_dec(mProcess_timeout);
#endif
        }
      }
      break;
    case MSG_ED_RQST_ABORT:
      {
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_ED_ABORTED;
        release_current_message();
      }
      break;
    case MSG_ED_ABORTED:
    case MSG_ED_ACTION_COMPLETE_SUCCESS:
    case MSG_ED_ACTION_COMPLETE_FAIL:
    default:
      {
        release_current_message();
      }
      break;
  }
#ifdef PROCESS_TO_ENBLE
  _validate_processs_timeout(MSG_ED_ACTION_COMPLETE_FAIL);
#endif
  return gSuccess_c;
}

/************************************************************************************
* process_to_msg
*
* This function process any Timeout pending msg.
*
************************************************************************************/
static FuncReturn_t process_to_msg (void){
  msg_to_state_t state;
  state = (msg_to_state_t)((maMessage_Queu[current_msg])->u8Status.msg_state);
  switch(state)
  {
    case MSG_TO_RQST:
      {
        clear_all_radio_events();
        mRetries_timmer = MAX_TO_RETRIES;
#ifdef PROCESS_TO_ENBLE
        mProcess_timeout = PROCESS_TIMEOUT;
#endif
        command_xcvr_to();
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_TO_PASSED_TO_DEVICE;
      }
      break;
    case MSG_TO_PASSED_TO_DEVICE:
      {
        if(is_action_completed_evnt()){
          clear_action_completed_evnt();
          if(( is_ac_success_evnt()) ||
             ( is_ac_timeout_evnt()) ||
             ( is_timeout_evnt())) {
            clear_ac_success_evnt();
            clear_ac_timeout_evnt();
            clear_timeout_evnt();
            (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                       MSG_TO_ACTION_COMPLETE_SUCCESS;
            release_current_message();
          }
          else {
            if(gNull_c != mRetries_timmer){
              _t_dec(mRetries_timmer);
              clear_all_radio_events();
              command_xcvr_to();
            }
            else {
              (maMessage_Queu[current_msg])->u8Status.msg_state = \
                                                          MSG_TO_ACTION_COMPLETE_FAIL;
              release_current_message();
            }
          }
        }
        else {
#ifdef PROCESS_TO_ENBLE
          _t_dec(mProcess_timeout);
#endif
        }
      }
      break;
    case MSG_TO_RQST_ABORT:
      {
        (maMessage_Queu[current_msg])->u8Status.msg_state = MSG_TO_ABORTED;
        release_current_message();
      }
      break;
    case MSG_TO_ABORTED:
    case MSG_TO_ACTION_COMPLETE_SUCCESS:
    case MSG_TO_ACTION_COMPLETE_FAIL:
    default:
      {
        release_current_message();
      }
      break;
  }
#ifdef PROCESS_TO_ENBLE
  _validate_processs_timeout(MSG_TO_ACTION_COMPLETE_FAIL);
#endif

  return gSuccess_c;
}

/************************************************************************************
* calculate_ed_value function
*
* This function is called to calculate the Energy Dectect value.
*
************************************************************************************/
uint8_t calculate_ed_value(uint32_t x_agc_cca_ed_out)
{
  uint32_t u32EdVal;

  u32EdVal = x_agc_cca_ed_out & 0x000000FF;

  if(u32EdVal<0x0F)
  {
    u32EdVal = 0x00;
  }
  else if(u32EdVal>0x64)
  {
    u32EdVal = 0xFF;
  }
  else
  {
    u32EdVal = ((u32EdVal<<1)+u32EdVal)-0x2D;
  }

  return (uint8_t)(u32EdVal);
}

/************************************************************************************
* process_scan_req function
*
* This function is called to procees Channels Scan Requests.
*
************************************************************************************/
void process_scan_req(void)
{
  channel_num_t prevChann;
  static uint8_t u8BestVal = 0;
  static channel_num_t bestChann = CHANNEL26;
  static channel_num_t EdCh2Evaluate = CHANNEL11;
  static bool_t isSecondPass = FALSE;

  if(MSG_ED_ACTION_COMPLETE_SUCCESS == Scan_msg.u8Status.msg_state)
  {
    u8ScanValPerChann[EdCh2Evaluate] = Scan_msg.pu8Buffer->u8Data[0];
    if(u8ScanValPerChann[EdCh2Evaluate] > u8BestVal)
    {
      u8BestVal = u8ScanValPerChann[EdCh2Evaluate];
      bestChann = EdCh2Evaluate;
    }
    if( (0 != (u8ScanValPerChann[EdCh2Evaluate])) ||
        (TRUE == isSecondPass))
    {
      prevChann = EdCh2Evaluate;
      EdCh2Evaluate = get_next_chann_to_scan(EdCh2Evaluate);
      isSecondPass = FALSE;
      if(prevChann > EdCh2Evaluate)
      {
        bScanReqFlag = FALSE;
        vScanReqCallBack((uint8_t)(bestChann));
        u8BestVal = 0;
        bestChann = CHANNEL26;
      }
    }
    else
    {
      isSecondPass = TRUE;
    }
    if(FALSE == is_a_handled_message(&Scan_msg))
    {
      MLMEEnergyDetect(&Scan_msg, EdCh2Evaluate);
    }
  }
  else if( (MSG_ED_ABORTED == Scan_msg.u8Status.msg_state) ||
           (MSG_ED_ACTION_COMPLETE_FAIL == Scan_msg.u8Status.msg_state) )
  {
    MLMEEnergyDetect(&Scan_msg, EdCh2Evaluate);
  }
}


/************************************************************************************
* get_next_chann_to_scan function
*
* This function answer which is the next m.
*
************************************************************************************/
channel_num_t get_next_chann_to_scan(channel_num_t currentCh)
{
  channel_num_t nextChann;
  uint8_t timeOut;
  nextChann = currentCh;

  timeOut = 0;
  while(TOTAL_CHANN > timeOut)
  {
    if(CHANNEL26 > nextChann)
    {
      nextChann++;
    }
    else
    {
      nextChann = CHANNEL11;
    }
    if(u16ChannToScan & (0x01<<nextChann))
      break;
    timeOut++;
  }
  return nextChann;
}

/************************************************************************************
*************************************************************************************
* Private Debug stuff
*************************************************************************************
************************************************************************************/


