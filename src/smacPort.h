/*
 * smacPort.h
 *
 * This was made necessary because the f-ups at Freescale totally broke the API when they
 * ported from MC1321x to MC1322x.  If you can believe it, they did it on purpose.
 *
 *  Created on: Mar 11, 2009
 *      Author: jeffw
 */

#ifndef SMACPORT_H_
#define SMACPORT_H_

#include "gwTypes.h"

FuncReturn_t portMLMERXEnableRequest(message_t *msg);
FuncReturn_t portMLMERXDisableRequest(void);
FuncReturn_t portMCPSDataRequest (message_t *msg);
FuncReturn_t portMLMEEnergyDetect(channel_num_t inChannelNum);

#endif /* SMACPORT_H_ */
