/************************************************************************************
* OTAP Header File
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 05/12/2008
* Last Tested:
************************************************************************************/

#ifndef __OTAP_H__
#define __OTAP_H__

#include "../../PLM/LibInterface/Crm.h"
#include "../../PLM/LibInterface/Timer.h"
#include "../../PLM/LibInterface/Interrupt.h"
#include "../../SMAC/Interface/GlobalDefs.h"
#include "../../SMAC/Interface/TransceiverConfigMngmnt.h"
#include "../../SMAC/Interface/TransceiverPowerMngmnt.h"
#include "../../SMAC/Interface/WirelessLinkMngmt.h"
#include "../../SMAC/Interface/SecurityMngmnt.h"
#include "../../SMAC/Interface/TransceiverPowerMngmnt.h"
#include "../../SMAC/Configure/options_config.h"
#include "../../SMAC/Configure/security_config.h"
#include "../../SMAC/Drivers/Interface/Delay.h"
#include "../../SMAC/Drivers/Interface/MacaInterrupt.h"
#include "../../SMAC/Drivers/Interface/ghdr/maca.h"
#include "../../SMAC/Drivers/Configure/board_config.h"
#include "Leds.h"
#include "Keyboard.h"
#include "../../PLM/LibInterface/NVM.h"
#include "UartUtil.h"
#include "app_config.h"

  #if OTAP_ENABLED == TRUE
  typedef enum {
  INIT,
  RUN_NORMAL,
  WAIT_4_CONFIRM,
  SEND_DESCRIPTOR,
  WAIT_4_SOT,
  WAIT_4_DATA,
  FLASHING,
  SEND_ACK,
  SEND_ERROR,
  MAX_OTAP_FRAME_SIZE = 115,
  } OTAP_State_t;

  typedef struct otap_request_tag {
    uint8_t   orc[4];
    uint8_t   coord_addr[2];
    uint8_t   device_addr[2];
    uint8_t   current_device_addr[MAX_OTAP_FRAME_SIZE];
  } otap_request_t;

  typedef struct otap_header_tag {
    uint8_t   frame_type;
    uint8_t   coord_addr[2];
    uint8_t   device_addr[2];
    uint8_t   sequence_number;
  } otap_header_t;

  typedef struct otap_descriptor_frame_tag {
    otap_header_t  descriptor_header;
    uint8_t   smac_version[4];
    uint8_t   otap_version[4];
    uint8_t   device_type;
    uint8_t   description[16];
  } otap_descriptor_frame_t;

  typedef struct otap_startoft_frame_tag {
    otap_header_t  startoft_header;
    uint8_t   transmission_type;
    uint8_t   messages_to_send[2];
  } otap_startoft_frame_t;

  typedef struct otap_data_frame_tag {
    otap_header_t  data_header;
    uint8_t   message_number[2];
    uint8_t   data[MAX_OTAP_FRAME_SIZE];
  } otap_data_frame_t;

  typedef struct otap_error_frame_tag {
    otap_header_t  error_header;
    uint8_t   error_type;
  } otap_error_frame_t;

  typedef struct otap_wait_frame_tag {
    otap_header_t  wait_header;
    uint8_t   time_in_ms;
  } otap_wait_frame_t;

  typedef enum{
    OTAP_CONFIRM           =   0x10,
    DESCRIPTOR_FRAME       =   0x20,
    START_OF_TRANSMISSION  =   0x30,
    ODATA                  =   0x32,
    END_OF_TRANSMISSION    =   0x34,
    ERROR                  =   0x40,
    OACK                   =   0x50,
    NACK                   =   0x51,
    WAIT                   =   0x52,
  }Otap_Frame_Type_t;

  typedef enum{
    HC08      =   0x10,
    HCS08     =   0x11,
    HCS12     =   0x20,
    COLDFIRE  =   0x30,
    MC1322X   =   0x40,
    IMX       =   0x50,
  }Otap_Device_Type_t;

  typedef enum{
    FLASH_ROUTINES  =   0x10,
    FIRMWARE        =   0x20,
    CAL_VALUES      =   0x30,
    DEVICE_ADDRESS  =   0x40,
  }Otap_Transmission_Type_t;

  typedef enum{
    USER_CANCEL          =   0x10,
    ACK_ERROR            =   0x20,
    COMMUNICATION_ERROR  =   0x30,
    TRANSFER_ERROR       =   0x40,
  }Otap_Error_Type_t;

  #define DEVICE_ADDRESS0 (uint8_t)(OTAP_DEVICE_ADDRESS&0x00FF)
  #define DEVICE_ADDRESS1 (uint8_t)((OTAP_DEVICE_ADDRESS>>8)&0x00FF)

  #define OTAP_RX_BUFFER_SIZE (124)
  #define OTAP_RX_SIZE (smac_pdu_size(OTAP_RX_BUFFER_SIZE))

  #define OTAP_TX_BUFFER_SIZE (124)
  #define OTAP_TX_SIZE (smac_pdu_size(OTAP_TX_BUFFER_SIZE))

  #define NVM_StartNvmRegulator() CRM_VREG_CNTL.vReg1P8VEn = 1

  extern void OTAP_Init(message_t *);
  extern void OTAP_execute(void);
  extern void send_ack(void);
  extern void send_error(void);
  extern void flash_new_app(void);
  extern void send_descriptor(void);
  extern void OTAP_NVMSetup(void);
  extern void OTAP_data_indication_execute(void);

  #define OTAP_TRANSMIT_END() ((MSG_TX_ACTION_COMPLETE_SUCCESS == OTAP_TX_msg.u8Status.msg_state) || \
  (MSG_TX_ACTION_COMPLETE_FAIL == OTAP_TX_msg.u8Status.msg_state) || \
  (MSG_TX_ACTION_COMPLETE_CHANN_BUSY == OTAP_TX_msg.u8Status.msg_state) || \
  (MSG_TX_ABORTED == OTAP_TX_msg.u8Status.msg_state))

   #define OTAP_RECEIVE_END() ((MSG_RX_ACTION_COMPLETE_SUCCESS == App_RX_msg->u8Status.msg_state) || \
  (MSG_RX_ACTION_COMPLETE_FAIL == App_RX_msg->u8Status.msg_state) || \
  (MSG_RX_TIMEOUT_FAIL == App_RX_msg->u8Status.msg_state)) && \
  (FALSE == gbDataIndicationFlag)

   extern uint8_t gbDataIndicationFlag;

   extern bool_t gbOtapExecute;

  #endif

#endif
