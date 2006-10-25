/*
  FlyWeight
  © Copyright 2005, 2006 Jeffrey B. Williams
  All rights reserved
  
  $Id$
  $Name$	
*/

#include "toyQuery.h"
#include "commands.h"
#include "string.h"

unsigned char ResponseBuffer[MAX_RESPONSE_SIZE];

// --------------------------------------------------------------------------
// Local function prototypes


// --------------------------------------------------------------------------

#define RESPONSE	"HELLO WORLD!"

void processQuery(BufferStoragePtrType inQueryPtr, RemoteAddrType inSrcAddr) {

	int queryKind = inQueryPtr[QPOS_QUERYKIND];
	
	// First figure out which type of query it is.
	switch (queryKind) {
		
		case QUERY_ACTOR_DESCRIPTOR:
			ResponseBuffer[RPOS_RESPONSEKIND] = RESPONSE_ACTOR_DESCRIPTOR;
			memcpy(&ResponseBuffer[RPOS_QUERYID], &inQueryPtr[QPOS_QUERYID], QUERYID_SIZE);
			ResponseBuffer[RPOS_RESPONSE_SIZE] = (byte) strlen(RESPONSE);
			strcat(&ResponseBuffer[RPOS_RESPONSE], RESPONSE);
			processQueryActorDescriptor(ResponseBuffer, RPOS_RESPONSE + strlen(RESPONSE), inSrcAddr);
			break;
			
		case QUERY_ACTOR_KVP_DESCRIPTOR:
			break;
			
		case QUERY_ENDPOINT_DESCRIPTOR:
			break;
			
		case QUERY_ENDPOINT_KVP_DESCRIPTOR:
			break;
												
	};
};

// --------------------------------------------------------------------------

void processQueryActorDescriptor(BufferStoragePtrType inResponseBuffer, BufferCntType inResponseBufferSize, RemoteAddrType inSrcAddr) {	
	createResponseCommand(gTXCurBufferNum, inSrcAddr, inResponseBuffer, inResponseBufferSize);
	if (transmitPacket(gTXCurBufferNum)){
	};	
	gLocalDeviceState = eLocalStateRespSent;	
}