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
#include "deviceInfo.h"
#include "PE_Types.h"
#include "pub_def.h"

// --------------------------------------------------------------------------
// Local function prototypes
void writeAsPString(BufferStoragePtrType inDestPtr, const BufferStoragePtrType inStringPtr, BufferOffsetType inStringSize);

void writeAsPString(BufferStoragePtrType inDestPtr, const BufferStoragePtrType inStringPtr, BufferOffsetType inStringSize) {
	inDestPtr[0] = inStringSize;
	memcpy(inDestPtr + 1, inStringPtr, inStringSize);
}

// --------------------------------------------------------------------------

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
			
		case QUERY_ACTOR_KVP:
			responseSize = processQueryActorKVP(queryPtr, responsePtr);
			break;
			
		case QUERY_ENDPOINT_DESCRIPTOR:
			responseSize = processQueryEndpointDescriptor(queryPtr, responsePtr);
			break;
			
		case QUERY_ENDPOINT_KVP:
			responseSize = processQueryEndpointKVP(queryPtr, responsePtr);
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

BufferOffsetType processQueryActorDescriptor(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr) {	

	BufferOffsetType curPos = 0;
	
	// Write the response ID.
	inResponsePtr[curPos] = RESPONSE_ACTOR_DESCRIPTOR;
	curPos += 1;
	
	// Copy the query ID into the response.
	memcpy(inResponsePtr + curPos, inQueryPtr + QPOS_QUERYID, QUERYID_SIZE);
	curPos += QUERYID_SIZE;
	
	// Write the GUID into the response.
	memcpy(inResponsePtr + curPos, GUID, UNIQUE_ID_LEN);
	curPos += UNIQUE_ID_LEN;

	// Write the description into the response.
	writeAsPString(inResponsePtr + curPos, DEVICE_DESC, (BufferOffsetType) strlen(DEVICE_DESC));
	curPos += strlen(DEVICE_DESC) + 1;
	
	// Write the KVP count for the remote actor KVPs into the response.
	inResponsePtr[curPos] = ACTOR_KVPS;
	curPos += sizeof(ACTOR_KVPS);
	
	// Write the KVP count for the remote endpoints into the response.
	inResponsePtr[curPos] = ACTOR_ENDPOINTS;
	curPos += sizeof(ACTOR_ENDPOINTS);
	
	// Return the size of the response.
	return curPos;		
}

// --------------------------------------------------------------------------

BufferOffsetType processQueryActorKVP(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr) {	

	BufferOffsetType curPos = 0;
	UINT8 kvpNum;
	
	// Write the response ID.
	inResponsePtr[curPos] = RESPONSE_ACTOR_KVP;
	curPos += 1;
	
	// Copy the query ID into the response.
	memcpy(inResponsePtr + curPos, inQueryPtr + QPOS_QUERYID, QUERYID_SIZE);
	curPos += QUERYID_SIZE;
	
	// Write the GUID into the response.
	memcpy(inResponsePtr + curPos, GUID, UNIQUE_ID_LEN);
	curPos += UNIQUE_ID_LEN;

	// Find the KVP that corresponds to the requested KVP.
	kvpNum = inQueryPtr[QPOS_KVPNUM];
	
	// Write the KVP number into the response.
	inResponsePtr[curPos] = kvpNum;
	curPos += sizeof(kvpNum);
	
	// Write the key.
	writeAsPString(inResponsePtr + curPos, kActorKVPs[kvpNum][KEY_INDEX], strlen(kActorKVPs[kvpNum][KEY_INDEX]));
	curPos += strlen(kActorKVPs[kvpNum][KEY_INDEX]) + 1;
	
	// Write the value.
	writeAsPString(inResponsePtr + curPos, kActorKVPs[kvpNum][VALUE_INDEX], strlen(kActorKVPs[kvpNum][VALUE_INDEX]));
	curPos += strlen(kActorKVPs[kvpNum][VALUE_INDEX]) + 1;
	
	// Return the size of the response.
	return curPos;		
}


// --------------------------------------------------------------------------

BufferOffsetType processQueryEndpointDescriptor(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr) {	

	BufferOffsetType curPos = 0;
	UINT8 endpointNum;
	
	// Write the response ID.
	inResponsePtr[curPos] = RESPONSE_ENDPOINT_DESCRIPTOR;
	curPos += 1;
	
	// Copy the query ID into the response.
	memcpy(inResponsePtr + curPos, inQueryPtr + QPOS_QUERYID, QUERYID_SIZE);
	curPos += QUERYID_SIZE;
	
	// Write the GUID into the response.
	memcpy(inResponsePtr + curPos, GUID, UNIQUE_ID_LEN);
	curPos += UNIQUE_ID_LEN;

	// Find the endpoint that corresponds to the requested endpoint.
	endpointNum = (inQueryPtr[QPOS_ENDPOINT_NUM]) >> 4;
	
	// Write the endpoint number into the response.
	inResponsePtr[curPos] = (endpointNum << 4);
	curPos += sizeof(endpointNum);
	
	// Write the endpoint type into the response.
	memcpy(inResponsePtr + curPos, kActorEndpoints[endpointNum][EP_DESC_INDEX], ENDPOINT_TYPE_LEN);
	curPos += ENDPOINT_TYPE_LEN;
	
	// Write the endpoint desc into the response.
	writeAsPString(inResponsePtr + curPos, kActorEndpoints[endpointNum][EP_DESC_INDEX], strlen(kActorEndpoints[endpointNum][EP_DESC_INDEX]));
	curPos += strlen(kActorEndpoints[endpointNum][EP_DESC_INDEX]) + 1;
	
	// Write the KVP count for the remote into the response.
	inResponsePtr[curPos] = (kActorEndpoints[endpointNum][EP_COUNT_INDEX])[0];
	curPos += sizeof(ACTOR_KVPS);
	
	return curPos;
	
}


// --------------------------------------------------------------------------

BufferOffsetType processQueryEndpointKVP(BufferStoragePtrType inQueryPtr, BufferStoragePtrType inResponsePtr) {	

	BufferOffsetType curPos = 0;
	UINT8 endpointNum;
	UINT8 kvpNum;
	
	// Write the response ID.
	inResponsePtr[curPos] = RESPONSE_ENDPOINT_KVP;
	curPos += 1;
	
	// Copy the query ID into the response.
	memcpy(inResponsePtr + curPos, inQueryPtr + QPOS_QUERYID, QUERYID_SIZE);
	curPos += QUERYID_SIZE;
	
	// Write the GUID into the response.
	memcpy(inResponsePtr + curPos, GUID, UNIQUE_ID_LEN);
	curPos += UNIQUE_ID_LEN;

	// Find the endpoint that corresponds to the requested endpoint.
	endpointNum = (inQueryPtr[QPOS_ENDPOINT_NUM]) >> 4;
	
	// Write the endpoint number into the response.
	inResponsePtr[curPos] = (endpointNum << 4);
	curPos += sizeof(endpointNum);
	
	// Find the KVP that corresponds to the requested KVP.
	kvpNum = inQueryPtr[QPOS_KVPNUM];
	
	// Write the KVP number into the response.
	inResponsePtr[curPos] = kvpNum;
	curPos += sizeof(kvpNum);
	
	// Write the key.
	writeAsPString(inResponsePtr + curPos, kEndpointKVPs[kvpNum][KEY_INDEX], strlen(kEndpointKVPs[kvpNum][KEY_INDEX]));
	curPos += strlen(kEndpointKVPs[kvpNum][KEY_INDEX]) + 1;
	
	// Write the value.
	writeAsPString(inResponsePtr + curPos, kEndpointKVPs[kvpNum][VALUE_INDEX], strlen(kEndpointKVPs[kvpNum][VALUE_INDEX]));
	curPos += strlen(kEndpointKVPs[kvpNum][VALUE_INDEX]) + 1;
	
	return curPos;
	
}