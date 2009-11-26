#include "Otap.h"
#include "UartUtil.h"

#if OTAP_ENABLED == TRUE
otap_request_t gbotap_request;
otap_header_t gbotap_header;
otap_startoft_frame_t gbotap_sof;
otap_error_frame_t gbotap_error;

volatile uint32_t gbDataCounter;
volatile uint32_t index;

volatile uint16_t gbmessages_receiving;
volatile uint16_t last_message_number;

volatile uint8_t OTAP_STATE;

bool_t gbOtapExecute;
volatile bool_t ack_received;
volatile bool_t OtapRequest;
volatile bool_t broadcast;
volatile bool_t erase_flash;



crmVRegCntl_t OtapVRegCntl;
nvmType_t OtapNvmType;

message_t OTAP_TX_msg;
message_t *App_RX_msg;
uint8_t TxOtapData[OTAP_TX_SIZE];

void OTAP_Init(message_t * pRxMsg)
{
  gbDataCounter = 0;
  OTAP_STATE = INIT;
  ack_received = FALSE;

  index = 0x00000004;
  last_message_number=0;
  OtapRequest = FALSE;
  broadcast = FALSE;
  erase_flash = TRUE;

  gbotap_request.orc[0] = (uint8_t)(OTAP_REQUEST_CODE&0xFF);
  gbotap_request.orc[1] = (uint8_t)(OTAP_REQUEST_CODE>>8&0xFF);
  gbotap_request.orc[2] = (uint8_t)(OTAP_REQUEST_CODE>>16&0xFF);
  gbotap_request.orc[3] = (uint8_t)(OTAP_REQUEST_CODE>>24&0xFF);
  App_RX_msg = pRxMsg;
  MSG_INIT(OTAP_TX_msg, &TxOtapData, NULL);
  OTAP_TX_msg.u8Status.msg_state = MSG_TX_ACTION_COMPLETE_SUCCESS;
  App_RX_msg->u8Status.msg_state = MSG_RX_ACTION_COMPLETE_SUCCESS;
}

void OTAP_execute(void)
{
  switch(OTAP_STATE)
  {
    case INIT:
//      OTAP_NVMSetup();
      OTAP_STATE = RUN_NORMAL;
      gbOtapExecute = FALSE;
    break;

    case RUN_NORMAL:
      gbOtapExecute = FALSE;
    break;

    case WAIT_4_CONFIRM:
    break;

    case SEND_DESCRIPTOR:
      send_descriptor();
    break;

    case WAIT_4_SOT:
    break;

    case WAIT_4_DATA:
    break;

    case SEND_ACK:
      send_ack();
    break;
/*
    case SEND_ERROR:
      send_error();
    break;
*/
    case FLASHING:
      flash_new_app();
    break;
  }
  if(OTAP_RECEIVE_END() && (TRUE == gbOtapExecute))
  {
    App_RX_msg->u8BufSize = OTAP_RX_SIZE;
    MLMERXEnableRequest(App_RX_msg, 0x000F0000);
  }
}

void send_descriptor(void)
{
  static uint8_t num_ret = 20;
  if(num_ret && !ack_received)
  {
    if(OTAP_TRANSMIT_END())
    {
      OTAP_TX_msg.pu8Buffer->u8Data[0] = DESCRIPTOR_FRAME;
      OTAP_TX_msg.pu8Buffer->u8Data[1] = gbotap_request.coord_addr[0];
      OTAP_TX_msg.pu8Buffer->u8Data[2] = gbotap_request.coord_addr[1];
      OTAP_TX_msg.pu8Buffer->u8Data[3] = DEVICE_ADDRESS0;
      OTAP_TX_msg.pu8Buffer->u8Data[4] = DEVICE_ADDRESS1;
      OTAP_TX_msg.pu8Buffer->u8Data[5] = gbotap_header.sequence_number++;
      OTAP_TX_msg.u8BufSize = 6;
      (void)MCPSDataRequest(&OTAP_TX_msg);
      num_ret--;
    }
  }
  else
  {
    num_ret = 20;
    ack_received = FALSE;
    OTAP_STATE = WAIT_4_SOT;
    gbOtapExecute = FALSE;
  }
}

void send_ack(void)
{
  static uint8_t num_ret = 20;
  if(gbotap_header.frame_type == ODATA || gbotap_header.frame_type == START_OF_TRANSMISSION
     || gbotap_header.frame_type == END_OF_TRANSMISSION)
  {
    if(OTAP_TRANSMIT_END())
    {
      OTAP_TX_msg.pu8Buffer->u8Data[0] = OACK;
      OTAP_TX_msg.pu8Buffer->u8Data[1] = gbotap_request.coord_addr[0];
      OTAP_TX_msg.pu8Buffer->u8Data[2] = gbotap_request.coord_addr[1];
      OTAP_TX_msg.pu8Buffer->u8Data[3] = DEVICE_ADDRESS0;
      OTAP_TX_msg.pu8Buffer->u8Data[4] = DEVICE_ADDRESS1;
      OTAP_TX_msg.pu8Buffer->u8Data[5] = gbotap_header.sequence_number++;
      OTAP_TX_msg.u8BufSize = 6;
      (void)MCPSDataRequest(&OTAP_TX_msg);
      if(gbotap_header.frame_type == END_OF_TRANSMISSION && num_ret)
      {
        num_ret--;
      }
    }
    if(gbotap_header.frame_type == ODATA || gbotap_header.frame_type == START_OF_TRANSMISSION)
    {
      OTAP_STATE = WAIT_4_DATA;
      gbOtapExecute = TRUE;
    }
    else if(gbotap_header.frame_type == END_OF_TRANSMISSION && num_ret)
    {
      OTAP_STATE = SEND_ACK;
      gbOtapExecute = TRUE;
    }
    else if(gbotap_header.frame_type == END_OF_TRANSMISSION && !num_ret)
    {
      OTAP_STATE = FLASHING;
      gbOtapExecute = TRUE;
      num_ret = 20;
    }
  }
}

void send_error(void)
{
  if(OTAP_TRANSMIT_END())
  {
    OTAP_TX_msg.pu8Buffer->u8Data[0] = ERROR;
    OTAP_TX_msg.pu8Buffer->u8Data[1] = gbotap_request.coord_addr[0];
    OTAP_TX_msg.pu8Buffer->u8Data[2] = gbotap_request.coord_addr[1];
    OTAP_TX_msg.pu8Buffer->u8Data[3] = DEVICE_ADDRESS0;
    OTAP_TX_msg.pu8Buffer->u8Data[4] = DEVICE_ADDRESS1;
    OTAP_TX_msg.pu8Buffer->u8Data[5] = gbotap_header.sequence_number++;
    OTAP_TX_msg.pu8Buffer->u8Data[6] = gbotap_error.error_type;
    OTAP_TX_msg.u8BufSize = 7;
    (void)MCPSDataRequest(&OTAP_TX_msg);
    OTAP_STATE = RUN_NORMAL;
  }
}

void flash_new_app(void)
{
  OTAP_STATE = RUN_NORMAL;
  MLMEPHYSoftReset();
}

void OTAP_NVMSetup(void)
{
  OtapVRegCntl.vReg = g1P8VReg_c;
  OtapVRegCntl.cntl.vReg1P8VEn = TRUE;
  CRM_VRegCntl(&OtapVRegCntl);
  DelayMs(1);
  NVM_Detect(gNvmInternalInterface_c, &OtapNvmType);
}

void OTAP_data_indication_execute(void)
{
  uint16_t message_number;
  uint8_t success_value[4] = {'O','K','O','K'};
  uint8_t i;

    gbotap_header.frame_type = App_RX_msg->pu8Buffer->u8Data[1];
    gbotap_request.device_addr[0] = App_RX_msg->pu8Buffer->u8Data[4];
    gbotap_request.device_addr[1] = App_RX_msg->pu8Buffer->u8Data[5];

    if(App_RX_msg->pu8Buffer->u8Data[1] == gbotap_request.orc[0] && App_RX_msg->pu8Buffer->u8Data[2] == gbotap_request.orc[1] &&
       App_RX_msg->pu8Buffer->u8Data[3] == gbotap_request.orc[2] && App_RX_msg->pu8Buffer->u8Data[4] == gbotap_request.orc[3] && !OtapRequest)
    {
      gbOtapExecute = TRUE;
      OtapRequest = TRUE;
      gbotap_request.coord_addr[0] = App_RX_msg->pu8Buffer->u8Data[5];
      gbotap_request.coord_addr[1] = App_RX_msg->pu8Buffer->u8Data[6];
      gbotap_request.device_addr[0] = App_RX_msg->pu8Buffer->u8Data[7];
      gbotap_request.device_addr[1] = App_RX_msg->pu8Buffer->u8Data[8];
      if(gbotap_request.device_addr[0] == 0xFF && gbotap_request.device_addr[1] == 0xFF)
      {
        broadcast = TRUE;
        for(i = 0; i < 115; i+=2)
        {
          if(App_RX_msg->pu8Buffer->u8Data[9+i] == DEVICE_ADDRESS0 &&
             App_RX_msg->pu8Buffer->u8Data[10+i] == DEVICE_ADDRESS1)
          {
            break;
          }
          else
          {
            ;
          }
        }
        if(i>114)
        {
          OTAP_STATE = WAIT_4_CONFIRM;
          gbOtapExecute = TRUE;
        }
        else
        {
          OTAP_STATE = RUN_NORMAL;
          OtapRequest = FALSE;
        }
      }
      else if(gbotap_request.device_addr[0] == DEVICE_ADDRESS0 &&
              gbotap_request.device_addr[1] == DEVICE_ADDRESS1)
      {
        OTAP_STATE = WAIT_4_CONFIRM;
        gbOtapExecute = TRUE;
        broadcast = FALSE;
      }
      else
      {
        OTAP_STATE = RUN_NORMAL;
        broadcast = FALSE;
        OtapRequest = FALSE;
      }
    }
    else if(gbotap_header.frame_type == OTAP_CONFIRM && OtapRequest)
    {
      if((gbotap_request.device_addr[0] == DEVICE_ADDRESS0 &&
         gbotap_request.device_addr[1] == DEVICE_ADDRESS1) ||
         (gbotap_request.device_addr[0] == 0xFF && gbotap_request.device_addr[1] == 0xFF))
      {
        OTAP_STATE = SEND_DESCRIPTOR;
        gbOtapExecute = TRUE;
      }
      else
      {
        OTAP_STATE = RUN_NORMAL;
        broadcast = FALSE;
        OtapRequest = FALSE;
      }
    }
    else if(gbotap_header.frame_type == OACK && OtapRequest)
    {
      ack_received = TRUE;
      if((gbotap_request.device_addr[0] == DEVICE_ADDRESS0 &&
         gbotap_request.device_addr[1] == DEVICE_ADDRESS1) ||
         (gbotap_request.device_addr[0] == 0xFF && gbotap_request.device_addr[1] == 0xFF))
      {
        if(broadcast)
        {
          OTAP_STATE = RUN_NORMAL;
          broadcast = FALSE;
          OtapRequest = FALSE;
        }
        else
        {
          OTAP_STATE = WAIT_4_SOT;
          gbOtapExecute = FALSE;
        }
      }
    }
    else if((gbotap_header.frame_type == START_OF_TRANSMISSION) && (OtapRequest))
    {
      gbOtapExecute = TRUE;
      gbotap_sof.messages_to_send[0] = App_RX_msg->pu8Buffer->u8Data[8];
      gbotap_sof.messages_to_send[1] = App_RX_msg->pu8Buffer->u8Data[9];
      gbmessages_receiving = gbotap_sof.messages_to_send[0]<<8;
      gbmessages_receiving |= gbotap_sof.messages_to_send[1];
      if(gbotap_request.device_addr[0] == DEVICE_ADDRESS0 &&
         gbotap_request.device_addr[1] == DEVICE_ADDRESS1)
      {
        OTAP_STATE = SEND_ACK;
        gbOtapExecute = TRUE;
      }
      else
      {
        OTAP_STATE = WAIT_4_DATA;
        gbOtapExecute = TRUE;
      }
    }
    else if(gbotap_header.frame_type == ODATA && OtapRequest)
    {
      if(gbotap_request.device_addr[0] == DEVICE_ADDRESS0 &&
         gbotap_request.device_addr[1] == DEVICE_ADDRESS1)
      {
        message_number = (App_RX_msg->pu8Buffer->u8Data[7]<<8) | App_RX_msg->pu8Buffer->u8Data[8];

        if(erase_flash)
        {
          OTAP_NVMSetup();
          NVM_StartNvmRegulator();
          while(!CRM_VRegIsReady(V_REG_MASK_1P8V));  // Wait for regulator to become active
          NVM_Erase(gNvmInternalInterface_c, OtapNvmType, 0x000000FF);
          NVM_Write(gNvmInternalInterface_c, OtapNvmType, (void *)(&success_value[0]), 0x00000000, 4);
          NVM_Write(gNvmInternalInterface_c, OtapNvmType, (void *)(&App_RX_msg->pu8Buffer->u8Data[9]), index, 115);
          gbDataCounter = App_RX_msg->pu8Buffer->u8Data[9] | App_RX_msg->pu8Buffer->u8Data[10]<<8 | \
                          App_RX_msg->pu8Buffer->u8Data[11]<<16 | App_RX_msg->pu8Buffer->u8Data[12]<<24;
          index += 115;
          last_message_number++;
          erase_flash = FALSE;
        }
        else
        {
          if(last_message_number+1 == message_number)
          {
            NVM_Write(gNvmInternalInterface_c, OtapNvmType, (void *)(&App_RX_msg->pu8Buffer->u8Data[9]), index, 115);
            index += 115;
            last_message_number++;
          }
        }
        OTAP_STATE = SEND_ACK;
        gbOtapExecute = TRUE;
      }
    }
    else if(gbotap_header.frame_type == END_OF_TRANSMISSION && OtapRequest)
    {
      if(gbotap_request.device_addr[0] == DEVICE_ADDRESS0 &&
         gbotap_request.device_addr[1] == DEVICE_ADDRESS1)
      {
        OTAP_STATE = SEND_ACK;
        OtapRequest = FALSE;
        broadcast = FALSE;
      }
    }
}
#endif

