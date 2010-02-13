/************************************************************************************
* Wireless Link Management implementation
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/

#include "../Interface/WirelessLinkMngmt.h"
#include "../Interface/RadioManagement.h"
#include "RadioMngmntWrapper.h"
#include "../Drivers/Interface/ghdr/rcv_spi_regs.h"
#include "../Drivers/LibInterface/rif_inc.h"
#include "../Drivers/LibInterface/Synthesizer_inc.h"
#include "RF_Config.h"


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

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

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* MCPSDataRequest
*
* This data primitive is used to send a SMAC packet.
*
************************************************************************************/
FuncReturn_t MCPSDataRequest (message_t *msg)
{
  set_pmsg_as_transmission(msg);
  set_pmsg_state(msg, MSG_TX_RQST);
  insert_codebytes_to_pmsg(msg);
  return handle_new_message(msg, gNull_c);
}

/************************************************************************************
* MLMESetChannelRequest
*
* This primitive is used to set the frequency where the MC1322X will operate.
*
************************************************************************************/
FuncReturn_t MLMESetChannelRequest (channel_num_t u8channel)
{
  if(MAX_CHAN_NUM < u8channel){
    return gFailOutOfRange_c;   
  }
  else {
    if(gSuccess_c != _set_channel(u8channel))
      return gFailNoValidCondition_c;
  }
  return gSuccess_c;
}

/************************************************************************************
* MLMEGetChannelRequest
*
* This primitive returns the channel number where the MC1322X is operating.
*
************************************************************************************/
uint8_t MLMEGetChannelRequest (void)
{
  return GetCurrentChannel(gaRFSynVCODivI_c, gaRFSynVCODivF_c);
}

/************************************************************************************
* MLMERXEnableRequest
*
* This function enables the MC1322X for a packet reception with or without a 
* timeout using the 250 KHz clock.
*
************************************************************************************/
FuncReturn_t MLMERXEnableRequest (message_t *msg, uint32_t timeout)
{
  set_pmsg_as_reception(msg);
  return handle_new_message(msg, timeout);
}

/************************************************************************************
* MLMERXDisableRequest
*
* This function enables the MC1322X for a packet reception with or without a 
* timeout using the 250 KHz clock.
*
************************************************************************************/
FuncReturn_t MLMERXDisableRequest (message_t *msg)
{
  if(RX != get_pmsg_type(msg)){
    return gFailNoValidCondition_c;
  }
  else{
    return abort_message(msg);
  }
}

/************************************************************************************
* MLMEEnergyDetect 
*
* This function enables the MC1322X for a packet reception with or without a 
* timeout using the 250 KHz clock.
*
************************************************************************************/
FuncReturn_t MLMEEnergyDetect  (message_t *msg, channel_num_t u8channel)
{
  
  if( NULL == msg->pu8Buffer){
    return gFailOutOfRange_c;
  }
  else{
     set_pmsg_as_enrgy_detect(msg);
     set_pmsg_ed_chann(msg, u8channel);
     return handle_new_message(msg,gNull_c);
  }
   
}

/************************************************************************************
* MLMELinkQuality 
*
* This function computes the LQI value of the last received message, according to 
* the following formula:
*
*    DAGC_RSSI =   Cal_factor_dB 
*                - (LNA_min_gain_dB + lna_state*(LNA_max_gain_dB-LNA_min_gain_dB)) 
*                + (IFA_max_gain_dB - 3*ifa_state) 
*                + rx_dagc_gain_state  
*
************************************************************************************/
FuncReturn_t MLMELinkQuality (uint8_t * u8ReturnValue)
{
  *u8ReturnValue = PhyPlmeGetLQI();
  return gSuccess_c;
}



/************************************************************************************
* MLMEPAOutputAdjust    
*
* This function adjusts the output power of the transmitter using the Power 
* Amplifier included in the MC1322X.  
*
************************************************************************************/
FuncReturn_t MLMEPAOutputAdjust (uint8_t u8Power )
{
  
  if (MAX_POWER < u8Power){
    return gFailOutOfRange_c;
  }
  else{
    if(gSuccess_c != SetPower(u8Power))
      return gFailNoValidCondition_c;
  }
  return gSuccess_c;      
  
}

/************************************************************************************
* MLMEScanRequest    
*
* The MLMEScanRequest function configures an energy detect sequence in all channels
* received as parameters.
*
************************************************************************************/
FuncReturn_t MLMEScanRequest (uint16_t u16Channels, vScanCallback_t cbFn)
{
  bScanReqFlag = TRUE;
  u16ChannToScan = u16Channels;
  vScanReqCallBack = cbFn;
  return gSuccess_c;
}
/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private Debug stuff
*************************************************************************************
************************************************************************************/

