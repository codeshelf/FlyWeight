/************************************************************************************
* This header file is the interface for the Radio Management implementation
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _RADIO_MNGMT_
#define _RADIO_MNGMT_

#include "GlobalDefs.h"


/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/
#define initial_state_c (0)

#define MAX_TX_RETRIES 0
#define MAX_RX_RETRIES 0
#define MAX_ED_RETRIES 0
#define MAX_TO_RETRIES 0

#define _t_dec(x) (x = ((0 == x)? 0: (x-1)))

#define MSG_INIT(msg, buff, cb) \
  do { \
    msg.pu8Buffer = (smac_pdu_t *)(buff); \
    msg.cbDataIndication = cb; \
  } while(0)

#define set_msg_as_reception(x)              (x.u8Status.msg_type = RX)
#define set_pmsg_as_reception(x)             (x->u8Status.msg_type = RX)
#define set_rx_payload_buffer(x,y)           (x.pu8Buffer = (smac_pdu_t*)(&y))
#define rx_msg_datasize(x)                   (x.u8BufSize)
#define set_rx_callback_function(x,y)        (x.cbDataIndication = y)

#define set_msg_as_transmission(x)           (x.u8Status.msg_type = TX)
#define set_pmsg_as_transmission(x)          (x->u8Status.msg_type = TX)
#define set_tx_frame_buffer(x,y)             (x.pu8Buffer= (smac_pdu_t*)(&y))

#define set_pmsg_state(m,s)                  (m->u8Status.msg_state = s)
#define get_pmsg_type(m)                     (m->u8Status.msg_type)
#define set_pmsg_ed_chann(m,c)               (m->u8BufSize = c)

#define set_pmsg_as_enrgy_detect(x)          (x->u8Status.msg_type = ED)


#define insert_codebytes_to_msg(x) \
do{ \
     x.pu8Buffer->reserved[0]=0x7E; \
     x.pu8Buffer->reserved[1]=0xFF; \
}while(FALSE)

#define insert_codebytes_to_pmsg(x) \
do{ \
     x->pu8Buffer->reserved[0]=0x7E; \
     x->pu8Buffer->reserved[1]=0xFF; \
}while(FALSE)

#define tx_msg_payload_buffer(x,y)           (x.pu8Buffer->u8Data[y])
#define set_tx_datasize(x,y)                 (x.u8BufSize = y)
#define set_tx_callback_function(x,y)        (x.cbDataIndication = y)


#define tx_msg_action_complete(x)            (x.u8Status.msg_state == MSG_TX_ACTION_COMPLETE_SUCCESS \
                                             && x.u8Status.msg_type == TX)

#define rx_msg_action_complete(x)            (x.u8Status.msg_state == MSG_RX_ACTION_COMPLETE_SUCCESS \
                                             && x.u8Status.msg_type == RX)

#define rx_msg_timeout_fail(x)               (x.u8Status.msg_state == MSG_RX_TIMEOUT_FAIL \
                                             && x.u8Status.msg_type == RX)

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
/*
#define action_started_mask     (1<<19)
#define sync_detected_mask      (1<<18)
#define timeout_mask            (1<<17)
#define checksum_failed_mask    (1<<16)
#define data_indication_mask    (1<<15)
#define action_complete_mask    (1<<14)
#define ac_success_mask         (1<<13)
#define ac_timeout_mask         (1<<12)
#define ac_channel_busy_mask    (1<<11)
#define ac_crc_failed_mask      (1<<10)
#define ac_aborted_mask         (1<<9)
#define ac_no_ack_mask          (1<<8)
#define ac_no_data_mask         (1<<7)
#define ac_late_start_mask      (1<<6)
#define ac_ext_timeout_mask     (1<<5)
#define ac_ext_pnd_imeout_mask  (1<<4)
#define ac_pll_unlock_mask      (1<<3)
#define ac_external_abort_mask  (1<<2)
#define ac_not_completed_mask   (1<<1)
#define ac_dma_bus_error_mask   (1<<0)
*/


typedef union evnt_flags_tag {
  struct {
    uint32_t action_started_flag:1;
    uint32_t sync_detected_flag:1;
    uint32_t timeout_flag:1;
    uint32_t checksum_failed_flag:1;
    uint32_t data_indication_flag:1;
    uint32_t action_complete_flag:1;
    uint32_t ac_success_flag:1;
    uint32_t ac_timeout_flag:1;
    uint32_t ac_channel_busy_flag:1;
    uint32_t ac_crc_failed_flag:1;
    uint32_t ac_aborted_flag:1;
    uint32_t ac_no_ack_flag:1;
    uint32_t ac_no_data_flag:1;
    uint32_t ac_late_start_flag:1;
    uint32_t ac_ext_timeout_flag:1;
    uint32_t ac_ext_pnd_timeout_flag:1;
    uint32_t ac_pll_unlock_flag:1;
    uint32_t ac_external_abort_flag:1;
    uint32_t ac_not_completed_flag:1;
    uint32_t ac_dma_bus_error_flag:1;
    uint32_t not_used_f00:1;
    uint32_t not_used_f01:1;
    uint32_t not_used_f02:1;
    uint32_t not_used_f03:1;
    uint32_t not_used_f04:1;
    uint32_t not_used_f05:1;
    uint32_t not_used_f06:1;
    uint32_t not_used_f07:1;
    uint32_t not_used_f08:1;
    uint32_t not_used_f09:1;
    uint32_t not_used_f10:1;
    uint32_t not_used_f11:1;
  } Bits;
  uint32_t Reg;
}evnt_flags_t;

typedef struct smac_pdu_tag {
  uint8_t  reserved[2];
  uint8_t  u8Data[1];
} smac_pdu_t;

typedef union msg_flags_tag {
  struct{
    uintn8_t reserved:5;
    uintn8_t sw_endian:1;
    uintn8_t aborted:1;
    uintn8_t ciphered:1;
  } Bits;
  uintn8_t Reg;
}msg_flags_t;

typedef enum msg_tx_state_tag{
  MSG_TX_RQST = initial_state_c,
  MSG_TX_PASSED_TO_DEVICE,
  MSG_TX_ACTION_STARTED,
  MSG_TX_ACTION_COMPLETE_CHANN_BUSY,
  MSG_TX_ACTION_COMPLETE_SUCCESS,
  MSG_TX_ACTION_COMPLETE_FAIL,
  MSG_TX_RQST_ABORT,
  MSG_TX_ABORTED,
  MAX_MSG_TX_STATE
}msg_tx_state_t;


typedef enum msg_rx_state_tag{
  MSG_RX_RQST = initial_state_c,
  MSG_RX_PASSED_TO_DEVICE,
  MSG_RX_ACTION_STARTED,
  MSG_RX_SYNC_FOUND,
  MSG_RX_TIMEOUT_FAIL,
  MSG_RX_ACTION_COMPLETE_SUCCESS,
  MSG_RX_ACTION_COMPLETE_FAIL,
  MSG_RX_RQST_ABORT,
  MSG_RX_ABORTED,
  MAX_MSG_RX_STATE
}msg_rx_state_t;


typedef enum msg_ed_state_tag{
  MSG_ED_RQST = initial_state_c,
  MSG_ED_PASSED_TO_DEVICE,
  MSG_ED_ACTION_COMPLETE_SUCCESS,
  MSG_ED_ACTION_COMPLETE_FAIL,
  MSG_ED_RQST_ABORT,
  MSG_ED_ABORTED,
  MAX_MSG_ED_STATE
}msg_ed_state_t;


typedef enum msg_to_state_tag {
  MSG_TO_RQST = initial_state_c,
  MSG_TO_PASSED_TO_DEVICE,
  MSG_TO_ACTION_COMPLETE_SUCCESS,
  MSG_TO_ACTION_COMPLETE_FAIL,
  MSG_TO_RQST_ABORT,
  MSG_TO_ABORTED,
  MAX_MSG_TO_STATE
}msg_to_state_t;


typedef enum msg_type_tag {
  TX = 0,
  RX,
  ED,
  TO,
  MAX_MSG_TYPE
} msg_type_t;


typedef struct msg_status_tag {
  msg_type_t  msg_type:3;
  uint8_t     msg_state:5;
} msg_status_t;


typedef union any_mssg_state_tag {
  msg_tx_state_t tx;
  msg_rx_state_t rx;
  msg_ed_state_t ed;
  msg_to_state_t to;
  uint8_t        any_type;
}any_mssg_state_t;


#define smac_pdu_size(x) (sizeof(smac_pdu_t)+x-1)

/* ------------------------------------------------------------------------------*/
/*     HOW TO USE message_t STRUCTURE                                            */
/*                  +--------------+--------------+--------------+--------------|*/
/*                  |     TX       |     RX       |      ED      |      TO      |*/
/* |----------------+--------------+--------------+--------------+--------------|*/
/* |    u8Status    |                Msg type and state                         |*/
/* |----------------+--------------+--------------+--------------+--------------|*/
/* |                | Location of  | Start add. of| Addr to store|   Timeout    |*/
/* |   pu8Buffer    | the data to  |the reception | the ED value |    value     |*/
/* |                | be transmited| buffer       |              |              |*/
/* |----------------+--------------+--------------+--------------+--------------|*/
/* |                | Size in bytes|  Number of   |   Channel    |              |*/
/* |   u8BufSize    |   to be      |  received    |  to assess   |      *       |*/
/* |                |  transmited  |   bytes      |     ED       |              |*/
/* |----------------+--------------+--------------+--------------+--------------|*/
/* |cbDataIndication|  Callback to be executed when the requested action finish |*/
/* |----------------+--------------+--------------+--------------+--------------|*/
typedef struct message_tag {
  msg_status_t u8Status;
  smac_pdu_t   *pu8Buffer;
  uint8_t      u8BufSize;
  callback_t   cbDataIndication;
} message_t;


typedef enum repeat_option_tag {
  DO_NOT_REPEAT = 0,
  REPEAT_AFTER_WAIT_PERIOD,
  REPEAT_ASAP
} repeat_option_t;

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
extern evnt_flags_t gRadioEvntFlags;
extern bool_t bScanReqFlag;
extern vScanCallback_t vScanReqCallBack;
extern uint16_t u16ChannToScan;
extern uint8_t u8ScanValPerChann[TOTAL_CHANN];

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* This function aborts a message.
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
extern FuncReturn_t abort_message (message_t * msg);


/************************************************************************************
* This function changes the state of the current message.
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
extern void change_current_message_state (any_mssg_state_t mssg_state);



/************************************************************************************
* This function excutes the current message callback.
*
* Interface assumptions: Application must not call this function
*
* Routine limitations. 
*
* Return value: 
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
extern void execute_current_message_callback (void);


/************************************************************************************
* This function returns a pointer to the message that is been processed, it will 
* return NULL if there is no message.
*
* Interface assumptions: None
*
* Routine limitations. 
*
* Return value: The pointer to the message that is been processed.
*
* Effects on global data: None
*
* Source of algorithm used:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern message_t* get_current_message (void);

/************************************************************************************
* This function handle a new message to the queu.
*
* Interface assumptions: 
*
* Routine limitations. 
*
* Return value: SUCSESS - Released from the queu.
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
extern FuncReturn_t handle_new_message (message_t *, uint32_t);


/************************************************************************************
* This function process any TX/RX pending msg.
*
* Interface assumptions: This function shall be called on periodic basis
*
* Routine limitations. 
*
* Return value:
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
extern FuncReturn_t process_radio_msg (void);

/************************************************************************************
* This function initializes all the module variables involved in the radio management
*
* Interface assumptions: It must be called during the inicialization
*
* Routine limitations.
*
* Return value: SUCSESS - Any case
*
* Effects on global data: None
*
* Source of algorithm used:
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern FuncReturn_t radio_manager_init (void);

/************************************************************************************
* This function releases the current message from the queu.
*
* Interface assumptions: Application will not modify the data passed to this handler
*                        until the message status is updated to RELEASED
*
* Routine limitations. 
*
* Return value: SUCSESS - If the message could be allocated.
*               FAIL_NO_RESOURCES_AVILABLE - If the is no memory available to
*                         handle the message.
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
extern FuncReturn_t release_current_message (void);


/**********************************************************************************/

#endif /* _RADIO_MNGMT_ */

