/*
	FlyWeight
	Copyright 2005, 2006, 2007 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$	
*/

#ifndef _FLASH_H_
#define _FLASH_H_

#include "pub_def.h" 

void InitFlash(void);
void WriteFlashByte(UINT8 inByte, UINT8* inAddressPtr);
void FlashProg1(UINT8 inByte, UINT8* inAddressPtr);

#endif // _FLASH_H_