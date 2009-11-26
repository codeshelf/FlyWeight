/*
 * gwTypes.h
 *
 *  Created on: Mar 10, 2009
 *      Author: jeffw
 */

#ifndef GWTYPES_H_
#define GWTYPES_H_

#if defined(MC1321X) || defined(MC13192EVB)
	#include "PE_Types.h"
	#include "pub_def.h"

	#define gwBoolean		bool
	#define gwUINT8			UINT8
	#define gwSINT8			INT8
	#define gwUINT16		UINT16

	#define gwATD			INT8

	#define gwTxPacket		tTxPacket
	#define gwRxPacket		tRxPacket

	#define gwISR			__interrupt void

#else
	#include "EmbeddedTypes.h"
    #include "GlobalDefs.h"
	#include "RadioManagement.h"

	#define gwBoolean		bool_t
	#define gwUINT8			uint8_t
	#define gwSINT8			int8_t
	#define gwUINT16		uint16_t

	#define gwATD			uint16_t

	#define gwTxPacket		message_t
	#define gwRxPacket		message_t

	#define	gwISR			void
#endif

#endif /* GWTYPES_H_ */
