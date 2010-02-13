/************************************************************************************
* This header file is the interface for the Security Management implementation
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/
#ifndef _SECURITY_MNGMNT_
#define _SECURITY_MNGMNT_

#include "GlobalDefs.h"
#include "options_config.h"


#if SMAC_FEATURE_SECURITY == TRUE

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

typedef enum cipher_mode_tag{
  gCTRMode_c = 0,
  gCBCMode_c ,
  gCCMMode_c,
  gMaxCipherMode
} cipher_mode_t;

typedef struct cipher_key_tag{
  uint32_t key0;
  uint32_t key1;
  uint32_t key2;
  uint32_t key3;
} cipher_key_t;
	
typedef struct ctr_value_tag{
  uint32_t ctr0;
  uint32_t ctr1;
  uint32_t ctr2;
  uint32_t ctr3;
}ctr_value_t;


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* CipherEngineInit function performs a test to verify the operation of the 
* encryption engine.
*
*
* Return value: gFailTest_c shall be sent if the Encryption Test fail.
*               gSuccess_c when Encryption test pass.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
extern FuncReturn_t CipherEngineInit(void);

/************************************************************************************
* CipherConfigure function configures all parameter for the cipher process, the
* cipher mode, the key and the ctr value when the CTR mode is selected.
*
* Parameters:  cipher_mode_t u8CipherMode: Cipher mode supported by the ASM module.
*                                          gCTRMode_c for CTR mode
*                                          gCBCMode_c for CBC mode
*                                          gCCMMode_c for CCM mode
*              cipher_key_t key: a pointer to the key which will be used by the 
                                encryption engine.
*              ctr_value ctr: a pointer to the counter used in CTR and CCB modes.
*
* Return value: gSuccess_c if the configuration has been successful.
*               gFailTest_c when the Encryption Engine Test failed or has not been 
*               executed.
*               gFailOutOfRange_c for an incorrect parameter.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*
************************************************************************************/
extern FuncReturn_t CipherConfigure(cipher_mode_t u8CipherMode, cipher_key_t * pCipherKey, ctr_value_t * pCtrValue);


/************************************************************************************
* * CipherMSG function is called to cipher a message in block of 16 bytes, each block 
* is ciphered in 66 cycles, the device with a clock of 24 MHz executes this process in 
* 5.5 microseconds.
*
* Interface assumptions: The Buffer lenght to be decipher must be a multiple of 16 bytes,
*                       also minor to 112 bytes because the payload of a SMAC package is
*                       123 bytes then 112 is the maximun number multiple of 16 bytes. 
*                       For the CCM and CBC modes, the buffer lenght must be 
*                       16 bytes longer that the data size to store the MAC value. 
*
* Parameters: uint32_t * pu8CipherBuffer, buffer to be ciphered.
*             uint8_t u8BufferLength, Buffer Size.
*             
*
* Return value: gFailTest_c when the Encryption Engine Test failed or has not been 
*               executed.
*               gFailOutOfRange_c for an incorrect parameter.
*               gSuccess_c if the cipher process have been successful.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern FuncReturn_t CipherMsg (uint32_t * pu8CipherBuffer, uint8_t u8BufferLength);

/************************************************************************************
* * DecipherMSG function is called to descipher a message.
*
* Interface assumptions: The Buffer lenght to be decipher must be a multiple of 16 bytes,
*                       also minor to 112 bytes because the payload of a SMAC package is
*                       123 bytes then 112 is the maximun number multiple of 16 bytes. 
*                       For the CCM and CBC cipher modes the buffer lenght must be 
*                       16 bytes longer that the data size to store the MAC value. 
*
* Parameters: uint32_t * pu8CipherBuffer, buffer to be ciphered.
*             uint8_t u8BufferLength, Buffer Size.
*             
*
* Return value: gFailTest_c when the Encryption Engine Test failed or has not been 
*               executed.
*               gFailOutOfRange_c for an incorrect parameter.
*               gSuccess_c if the cipher process have been successful.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern FuncReturn_t DecipherMsg (uint32_t *pu32DecipherBuffer, uint8_t u8BufferLength);


/************************************************************************************
* * CipherMsgU8 function is called to cipher a message in block of 16 bytes, each block 
* is ciphered in 66 cycles, the device with a clock of 24 MHz executes this process in 
* 5.5 microseconds.
*
* Interface assumptions: The Buffer lenght to be decipher must be a multiple of 16 bytes,
*                       also minor to 112 bytes because the payload of a SMAC package is
*                       123 bytes then 112 is the maximun number multiple of 16 bytes. 
*                       For the CCM and CBC modes, the buffer lenght must be 
*                       16 bytes longer that the data size to store the MAC value. 
*
* Parameters: uint32_t * pu8CipherBuffer, buffer to be ciphered.
*             uint8_t u8BufferLength, Buffer Size.
*             
*
* Return value: gFailTest_c when the Encryption Engine Test failed or has not been 
*               executed.
*               gFailOutOfRange_c for an incorrect parameter.
*               gSuccess_c if the cipher process have been successful.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern FuncReturn_t CipherMsgU8(uint8_t *pu8CipherBuffer, uint8_t u8BufferLength);


/************************************************************************************
* * DecipherMsgU8 function is called to descipher a message.
*
* Interface assumptions: The Buffer lenght to be decipher must be a multiple of 16 bytes,
*                       also minor to 112 bytes because the payload of a SMAC package is
*                       123 bytes then 112 is the maximun number multiple of 16 bytes. 
*                       For the CCM and CBC cipher modes the buffer lenght must be 
*                       16 bytes longer that the data size to store the MAC value. 
*
* Parameters: uint32_t * pu8CipherBuffer, buffer to be ciphered.
*             uint8_t u8BufferLength, Buffer Size.
*             
*
* Return value: gFailTest_c when the Encryption Engine Test failed or has not been 
*               executed.
*               gFailOutOfRange_c for an incorrect parameter.
*               gSuccess_c if the cipher process have been successful.
*
* Revison history:
*   date      Author    Comments
*   ------    ------    --------
*   
************************************************************************************/
extern FuncReturn_t DecipherMsgU8(uint8_t *pu8CipherBuffer, uint8_t u8BufferLength);

/**********************************************************************************/

#endif //SMAC_FEATURE_SECURITY == TRUE

#endif /* _SECURITY_MNGMNT_ */
