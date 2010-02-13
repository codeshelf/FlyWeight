/************************************************************************************
* This header file is the interface for the Wireless Link Management implementation
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _WIRELESS_LINK_MNGMT_
#define _WIRELESS_LINK_MNGMT_

#include "GlobalDefs.h"
#include "RadioManagement.h"
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

#define tx_pa_amp_ctrl    (*((volatile uint32_t *)(0x80004000))) 
#define pa_target_pwr     (*((volatile uint32_t *)(0x80004000))) 

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
* This data primitive is used to send a SMAC packet.
*
* Interface assumptions: 
*
* Routine limitations. 
*
* Return value:
* gSuccess_c - If the message could be allocated in the queue.
* gFail_c - If the is no memory available to handle the message.                  
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

extern FuncReturn_t MCPSDataRequest (message_t *msg);

/************************************************************************************
* This primitive is used to set the frequency where the MC1322X will operate. 
* Valid parameters are between Channel 11(2405MHz) and Channel 26 (2480MHz), any 
* different value will return an error.
*
* Interface assumptions: 
*
* Routine limitations.
* This function does not check if the radio is busy or a message is being processed
*
* Return value:
* gSuccess_c - If the channel number was inside the range value.
* gFailOutOfRange_c - If the is no memory available to handle the message.                  
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

extern FuncReturn_t MLMESetChannelRequest (channel_num_t u8channel);

/************************************************************************************
* This primitive returns the channel number where the MC1322X is operating.
*
* Interface assumptions: 
*
* Routine limitations.
*
*
* Return value:
*
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

extern uint8_t MLMEGetChannelRequest (void);

/************************************************************************************
* This function enables the MC1322X for a packet reception with or without a 
* timeout using the 250 KHz clock.
*
* Interface assumptions: 
* The timer selection could be configured as a 'Hard Timeout' (receive sequence 
* is terminated immediately) and/or a 'Soft Timeout' (receive sequence is terminated 
* until the end of the current reception) will be defined at Radio Initialization. 
* The user must define the timeout value as a pointer to pu8Buffer. 

* Routine limitations.
* 
* Return value:
* gSuccess_c - If the message could be allocated in the queue.
* gFail_c - If the is no memory available to handle the message.                  
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

extern FuncReturn_t MLMERXEnableRequest (message_t *msg, uint32_t timeout);


/************************************************************************************
* This function disables and releases a specific message for a packet reception 
* from the message queue. The message will be disabled the next time it has the 
* scheduler time.
*
* Interface assumptions: 
*
*
* Routine limitations.
* If the message was successfully aborted the status will be changed.
*
* Return value:
* gSuccess_c - If the message was sucessfully aborted.
* gFail_c - If the message could not been aborted.                  
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

extern FuncReturn_t MLMERXDisableRequest (message_t *msg);

/************************************************************************************
* This function is used to detect the energy measured in dBm of a specific channel.
*
* Interface assumptions: 
* Channel number must be saved in the message pu8BufSize and result will be 
* pointed by pu8Buffer.
*
* Routine limitations.
* 
*
* Return value:
* gSuccess_c - If the message could be allocated in the queue.
* gFail_c - If the is no memory available to handle the message.
*           If the channel number is outside the range.
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

extern FuncReturn_t MLMEEnergyDetect  (message_t *msg, channel_num_t u8channel);


/************************************************************************************
* This function is used to calculate the LQI of the last received message.
*
* Interface assumptions: 
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
extern FuncReturn_t MLMELinkQuality (uint8_t * u8ReturnValue);

/************************************************************************************
* This function adjusts the output power of the transmitter using the Power Amplifier 
* included in the MC1322X. 
*
* Interface assumptions: 
*
*
* Routine limitations.
* 
*
* Return value:
*
*
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

extern FuncReturn_t MLMEPAOutputAdjust (uint8_t u8Power);

/************************************************************************************
* This function allows the user to scan the channels pointed by pu8Buffer and 
* obtain the energy detection of each one and store the results in a table pointed 
* by pu8Buffer. 
*
* Interface assumptions: 
*
*
* Routine limitations.
* 
*
* Return value:
*
*
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

extern FuncReturn_t MLMEScanRequest (uint16_t u16Channels, vScanCallback_t cbFn);
/**********************************************************************************/

#endif /* _WIRELESS_LINK_MNGMT_ */
