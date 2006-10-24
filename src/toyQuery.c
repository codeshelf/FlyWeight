/*
  FlyWeight
  © Copyright 2005, 2006 Jeffrey B. Williams
  All rights reserved
  
  $Id$
  $Name$	
*/

#include "toyQuery.h"

// --------------------------------------------------------------------------
// Local function prototypes


// --------------------------------------------------------------------------

void processQuery(BufferStoragePtrType inQueryPtr, BufferOffsetType inQueryBytes) {

	int queryKind = inQueryPtr[QPOS_QUERYID];
	
	// First figure out which type of query it is.
	switch (queryKind) {
		
		case QUERY_ACTOR_DESCRIPTOR:
			processQueryActorDescriptor(inQueryPtr + QUERY_HDR_SIZE, inQueryBytes - QUERY_HDR_SIZE);
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

void processQueryActorDescriptor(BufferStoragePtrType inQueryPtr, BufferOffsetType inQueryBytes) {
	
}