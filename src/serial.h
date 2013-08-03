/*
	FlyWeight
	� Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef SERIAL_H
#define SERIAL_H

#include "FreeRTOS.h"
#include "queue.h"
#include "usb.h"
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Defines.

#define GATEWAY_MGMT_QUEUE_SIZE		10

#define END							0300    /* indicates end of frame */
#define ESC							0333    /* indicates byte stuffing */
#define ESC_END						0334    /* ESC ESC_END means END data byte */
#define ESC_ESC						0335    /* ESC ESC_ESC means ESC data byte */

// --------------------------------------------------------------------------
// Functions prototypes.

void sendOneChar(USB_TComData inDataPtr);
void readOneChar(USB_TComData *outDataPtr);
void serialReceiveTask(void *pvParameters);
void serialTransmitFrame(USB_TComData *inDataPtr, gwUINT16 inSize);
BufferCntType serialReceiveFrame(BufferStoragePtrType inFramePtr, BufferCntType inMaxFrameSize);

#endif // SERIAL_H
