/*
	FlyWeight
	Copyright 2005, 2006, 2007 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$	
*/

#include "flash.h"
#include "IO_Map.h"
#include "Cpu.h"

void InitFlash() {
	FCDIV = (byte) 0b01001100;
//	asm {
//		initFCDIV: equ   %01001100 ;FLASH clock divider
//						; ||||||||
//						; |||||||+-DIV0 \
//						; ||||||+--DIV1 |
//						; |||||+---DIV2 >-- divide by (12+1)
//						; ||||+----DIV3 | BUSCLK/(8*13)~=192,300 Hz
//						; |||+-----DIV4 |
//						; ||+------DIV5 /
//						; |+-------PRDIV8 -- divide (prescale) by 8
//						; +--------DIVLD --- read-only status
//		lda initFCDIV
//		sta FCDIV ;set fFCLK = about 200kHz
//	}
};

void WriteFlashByte(UINT8 inByte, UINT8* inAddressPtr) {
	EnterCritical();
	FlashProg1(inByte, inAddressPtr);
	ExitCritical();
}