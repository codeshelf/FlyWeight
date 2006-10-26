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

// --------------------------------------------------------------------------
// Local function prototypes
void writeAsPString(BufferStoragePtrType inDestPtr, BufferStoragePtrType inStringPtr, BufferOffsetType inStringSize);

void writeAsPString(BufferStoragePtrType inDestPtr, BufferStoragePtrType inStringPtr, BufferOffsetType inStringSize) {
	inDestPtr[0] = inStringSize;
	memcpy(inDestPtr + 1, inStringPtr, inStringSize);
}

// --------------------------------------------------------------------------

#define RESPONSE	"HELLO WORLD!"

void processQuery(BufferCntType inRXBufferNum, BufferOffsetType inStartOfQuery, RemoteAddrType inSrcAddr) {

	BufferOffsetType responseSize = 0;
	
	// Read the query directly from the RX buffer at the start of query position.
	BufferStoragePtrType queryPtr = gRXRadioBuffer[inRXBufferNum].bufferStorage + inStartOfQuery;
	
	// Write the response directly into the TX buffer at the response position offset.
	BufferStoragePtrType responsePtr = gTXRadioBuffer[gTXCurBufferNum].bufferStorage + CMDPOS_RESPONSE;
	
	byte queryKind = queryPtr[QPOS_QUERYKIND];

	// First figure out which type of query it is.
	switch (queryKind) {
		
		case QUERY_ACTOR_DESCRIPTOR:
			responseSize = processQueryActorDescriptor(queryPtr, responsePtr);
			break;
			
		case QUERY_ACTOR_KVP_DESCRIPTOR:
			break;
			
		case QUERY_ENDPOINT_DESCRIPTOR:
			break;
			
		case QUERY_ENDPOINT_KVP_DESCRIPTOR:
			break;												
	};
	
	if (responseSize > 0) {
		createResponseCommand(gTXCurBufferNum, responseSize, inSrcAddr);
		if (transmitPacket(gTXCurBufferNum)){
		};	
		gLocalDeviceState = eLocalStateRespSent;
	}	
}

// --------------------------------------------------------------------------

#define DESC		"JEFFREY!"
#define KVP_CNT		"3"

BufferOffsetType processQueryActorDescriptor(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr) {	

	BufferOffsetType curPos = 0;
	
	// Write the response ID.
	inResponsePtr[curPos] = RESPONSE_ACTOR_DESCRIPTOR;
	curPos += 1;
	
	// Copy the query ID into the response.
	memcpy(inResponsePtr + curPos, inQueryPtr + QPOS_QUERYID, QUERYID_SIZE);
	curPos += QUERYID_SIZE;
	
	// Write the GUID into the response.
	writeAsPString(inResponsePtr + curPos, GUID, (BufferOffsetType) strlen(GUID));
	curPos += strlen(GUID) + 1;
	
	// Write the description into the response.
	writeAsPString(inResponsePtr + curPos, DESC, (BufferOffsetType) strlen(DESC));
	curPos += strlen(DESC) + 1;
	
	// Write the KVP count for the remote into the response.
	writeAsPString(inResponsePtr + curPos, KVP_CNT, (BufferOffsetType) strlen(KVP_CNT));
	curPos += strlen(KVP_CNT) + 1;
	
	// Return the size of the response.
	return curPos;		
}