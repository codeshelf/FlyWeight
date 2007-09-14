			INCLUDE	"9S08GB60.inc"

			XDEF	FlashProg1
FLASH:		SECTION

;*********************************************************************
;* FlashProg1 - programs one byte of FLASH
;* This routine waits for the command to complete before returning.
;* assumes location was blank. This routine can be run from FLASH
;*
;* On entry... H:X - points at the FLASH byte to be programmed
;* A holds the data for the location to be programmed
;*
;* Calling convention:
;* jsr FlashProg1
;*
;* Uses: DoOnStack which uses SpSub
;* Returns: H:X unchanged and A = FSTAT shifted left by 2 bits
;* Z=1 if OK, Z=0 if protect violation or access error
;* uses 32 bytes of stack space + 2 bytes for BSR/JSR used to call it
;*********************************************************************
FlashProg1: psha 					;temporarily save entry data
			lda 	#(mFPVIOL+mFACCERR) ;mask
			sta 	FSTAT 			;abort any command and clear errors
			lda 	#mByteProg 		;mask pattern for byte prog command
			bsr 	DoOnStack 		;execute prog code from stack RAM
			ais 	#1 				;deallocate data location from stack
			rts 					;Z = 0 means there was an error

;*********************************************************************
;* DoOnStack - copy SpSub onto stack and call it (see also SpSub)
;* Deallocates the stack space used by SpSub after returning from it.
;* Allows flash prog/erase command to execute out of RAM (on stack)
;* while flash is out of the memory map.
;* This routine can be used for flash byte-program or erase commands
;*
;* Calling Convention:
;* 			psha 					;save data to program (or dummy
;* 									; data for an erase command)
;* 			lda		#(mFPVIOL+mFACCERR) ;mask
;* 			sta 	FSTAT 			;abort any command and clear errors
;* 			lda 	#mByteProg 		;mask pattern for byte prog command
;* 			jsr 	DoOnStack 		;execute prog code from stack RAM
;* 			ais 	#1 				;deallocate data location from stack
;* 									; without disturbing A or CCR
;*
;* or substitute #mPageErase for page erase
;*
;* Uses 29 bytes on stack + 2 bytes for BSR/JSR used to call it
;* returns H:X unchanged and A=0 and Z=1 if no flash errors
;********************************************************************
DoOnStack:	pshx
			pshh					;save pointer to flash
			psha					;save command on stack
			ldhx	#SpSubEnd 		;point at last byte to move to stack
SpMoveLoop:	lda ,x 					;read from flash
			psha 					;move onto stack
			aix 	#-1 			;next byte to move
			cphx 	#SpSub-1 		;past end?
			bne 	SpMoveLoop 		;loop till whole sub on stack
			tsx 					;point to sub on stack
			tpa 					;move CCR to A for testing
			and 	#$08 			;check the I mask
			bne 	I_set 			;skip if I already set
			sei 					;block interrupts while FLASH busy
			lda 	SpSubSize+6,sp 	;preload data for command
			jsr ,x 					;execute the sub on the stack
			cli 					;ok to clear I mask now
			bra 	I_cont 			;continue to stack de-allocation
I_set: 		lda 	SpSubSize+6,sp 	;preload data for command
			jsr ,x 					;execute the sub on the stack
I_cont: 	ais 	#SpSubSize+3 	;deallocate sub body + H:X + command
									;H:X flash pointer OK from SpSub
			lsla 					;A=00 & Z=1 unless PVIOL or ACCERR
			rts 					;to flash where DoOnStack was called

;*********************************************************************
;* SpSub - This variation of SpSub performs all of the steps for
;* programming or erasing flash from RAM. SpSub is copied onto the
;* stack, SP is copied to H:X, and then the copy of SpSub in RAM is
;* called using a JSR 0,X instruction.
;*
;* At the time SpSub is called, the data to be programmed (dummy data
;* for an erase command), is in A and the flash address is on the
;* stack above SpSub. After return, PVIOL and ACCERR flags are in bits
;* 6 and 5 of A. If A is shifted left by one bit after return, it
;* should be zero unless there was a flash error.
;*
;* Uses 24 bytes on stack + 2 bytes if a BSR/JSR calls it
;*********************************************************************
SpSub: 		ldhx 	SpSubSize+4,sp 	;get flash address from stack
			sta 	0,x 			;write to flash; latch addr and data
			lda 	SpSubSize+3,sp 	;get flash command
			sta 	FCMD 			;write the flash command
			lda 	#mFCBEF 		;mask to initiate command
			sta 	FSTAT 			;[pwpp] register command
			nop 					;[p] want min 4~ from w cycle to r
ChkDone: 	lda 	FSTAT 			;[prpp] so FCCF is valid
			lsla 					;FCCF now in MSB
			bpl 	ChkDone 		;loop if FCCF = 0
SpSubEnd: 	rts 					;back into DoOnStack in flash
SpSubSize: 	equ (*-SpSub)
End: