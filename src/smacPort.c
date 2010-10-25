/*
 * smacPort.c
 *
 * This is a compatibility layer that makes SMAC 2 behave like SMAC 1, so that we can support
 * both the MC1321x and the MC1322x in the same codebase.
 *
 *  Created on: Mar 11, 2009
 *      Author: jeffw
 */

#include "smacPort.h"
#include "RadioManagement.h"
#include "WirelessLinkMngmt.h"

/*
 * Make MLMEEnergyDetect backward-compatible with SMAC 1.
 */

message_t gEDMsg;
uint8_t gEDMsgData[4];
FuncReturn_t	portMLMEEnergyDetect(channel_num_t inChannelNum) {

	FuncReturn_t result;

	MSG_INIT(gEDMsg, &gEDMsgData, NULL);
	gEDMsg.u8Status.msg_type = ED;
	result = MLMEEnergyDetect(&gEDMsg, inChannelNum);
	while (gEDMsg.u8Status.msg_state != MSG_ED_ACTION_COMPLETE_SUCCESS) {
		process_radio_msg();
	}

	// The result after we've read it.
	result = gEDMsg.pu8Buffer->u8Data[0];

	return result;
}


