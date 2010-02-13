/************************************************************************************
* Security Management implementation
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 09/05/2007
* Last Tested: 09/05/2007
************************************************************************************/

#include "../Interface/SecurityMngmnt.h"
#include "../../PLM/Interface/EmbeddedTypes.h"
#include "../Drivers/Interface/ghdr/asm.h"


#if SMAC_FEATURE_SECURITY ==  TRUE

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define enable_asm_mod()   (asm_control1 |= BIT0)
#define enable_test()      (asm_control1 |= BIT26)
#define disable_test()     (asm_control1 &= ~BIT26)
#define start_test()       (asm_control0 |= BIT24)
#define test_pass          (asm_status & BIT25) 
#define enable_ctr()       (asm_control1 |= BIT25)
#define disable_ctr()      (asm_control1 &= ~BIT25)
#define ctr_bit            BIT25 
#define enable_cbc()       (asm_control1 |= BIT24)
#define disable_cbc()      (asm_control1 &= ~BIT24)
#define cbc_bit            BIT24
#define set_normal_mode()  (asm_control1 |= BIT1)
#define cipher_done        (asm_status & BIT24) 
#define cipher_start()     (asm_control0 |= BIT24) 
#define cipher_disable()   (asm_control0 &= ~BIT24) 
#define load_mac()         (asm_control0 |=  BIT26)
#define disable_mac()      (asm_control0 &=  ~BIT26)
#define MaxCipherCycles    (0x03)
#define MaxSelfTestCycles  (0x0CE4)
#define MaxNumBlocks       (0x10)
#define BlockSize          (0x04)
#define test_fail          (0x00)
#define test_success       (0x01)
//#define mNumBlocks_Msg     (u8BufferLength/BlockSize)
//#define mResidue_Msg       (u8BufferLength%BlockSize) 
#define MACSize            (0x04)
#define gEnable_c          (0x01)
#define gDisable_c         (0x00)


 #define MAX_BYTES_FOR_CIPHER_PKT (112)
 #define BYTES_PER_WORD (4)
 #define CIPHER_BUFF_SIZE (MAX_BYTES_FOR_CIPHER_PKT/BYTES_PER_WORD)
 uint32_t ciphering_buff[CIPHER_BUFF_SIZE];
 typedef union bytes_2_word_tag{
   uint32_t u32word;
   struct{
    uint8_t Byte0;
    uint8_t Byte1;
    uint8_t Byte2;
    uint8_t Byte3;
   } Bytes;
 }bytes_2_word_tag;
//#endif


/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
static void u8Buff_2_u32Buff(uint8_t* , uint32_t*, uint8_t);
static void u32Buff_2_u8Buff(uint8_t* , uint32_t*, uint8_t);

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
static uint8_t mCipherTestResult;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* CipherEngineInit
*
* This function allows to carry out a self test to verify the operation of the 
* encryption engine.
*
************************************************************************************/

FuncReturn_t CipherEngineInit(void){
  uint16_t unblock=0;

  enable_asm_mod();
  enable_test();
  start_test();
  while(!(test_pass) && (MaxSelfTestCycles > unblock))
  {
    unblock ++;
  }
  if(!(test_pass)){
    mCipherTestResult = test_fail;
    return gFailTest_c;
  }
  else{
    mCipherTestResult = test_success;
    disable_test();
  }
  return gSuccess_c;
}



/************************************************************************************
* CipherConfigure
*
* This function configures all parameter for the cipher process, the cipher mode, the
* key and the ctr value when the CTR mode is selected.
*
************************************************************************************/
FuncReturn_t CipherConfigure(cipher_mode_t CipherMode, cipher_key_t *pCipherKey, ctr_value_t *pCtrValue){
  
  if(test_fail == mCipherTestResult){
    return gFailTest_c;
  }  
  
  else if(gMaxCipherMode <= CipherMode){
    return gFailOutOfRange_c;
  }
  
  set_normal_mode();
  asm_key0 = pCipherKey->key0;
  asm_key1 = pCipherKey->key1;
  asm_key2 = pCipherKey->key2;
  asm_key3 = pCipherKey->key3;
    
  switch(CipherMode){
    case gCTRMode_c:
    { 
      disable_cbc();
      enable_ctr();
      asm_ctr0 = pCtrValue->ctr0;
      asm_ctr1 = pCtrValue->ctr1;
      asm_ctr2 = pCtrValue->ctr2;
      asm_ctr3 = pCtrValue->ctr3;
      break;
    }
    case gCBCMode_c:
    {
      disable_ctr();
      enable_cbc();
      break;
    }
    case gCCMMode_c:
    {
      enable_cbc();
      enable_ctr();
      asm_ctr0 = pCtrValue->ctr0;
      asm_ctr1 = pCtrValue->ctr1;
      asm_ctr2 = pCtrValue->ctr2;
      asm_ctr3 = pCtrValue->ctr3;
      break;
    }
    default:
      break;
  }
  
 return gSuccess_c;
}
/************************************************************************************
* CipheMsg
*
* This function is called to cipher a message of 112 bytes maximun in CTR mode and 
* 96 bytes in CBC and CCM modes, the buffer length must be multiple of 16 bytes.
*
************************************************************************************/
FuncReturn_t CipherMsg(uint32_t *pu32CipherBuffer, uint8_t u8BufferLength)
{
  uint8_t u8Blocks, unblock;
  uint32_t * pu32AuxCipherBuffer;
  uint8_t mResidue_Msg;
  uint8_t mNumBlocks_Msg;
  
  mResidue_Msg = u8BufferLength%BlockSize;
  mNumBlocks_Msg = u8BufferLength/BlockSize;


  if(test_fail == mCipherTestResult){
    return gFailTest_c;
  }  
  else if( (gNull_c < mResidue_Msg)        || 
           (MaxNumBlocks < mNumBlocks_Msg) || 
           (gNull_c == mNumBlocks_Msg)){
      return gFailOutOfRange_c;
  }
  u8Blocks = mNumBlocks_Msg;
  pu32AuxCipherBuffer = pu32CipherBuffer;

  if(asm_control1 & cbc_bit){
    load_mac();
    u8Blocks--;
  }
  else{
    /*Do nothing*/
  }

  while(gNull_c < u8Blocks){
    unblock = 0;
    asm_data0 = *pu32AuxCipherBuffer ++;
    asm_data1 = *pu32AuxCipherBuffer ++;
    asm_data2 = *pu32AuxCipherBuffer ++;
    asm_data3 = *pu32AuxCipherBuffer ++;
   
    cipher_start();
    while(!(cipher_done) && (MaxCipherCycles > unblock))
    {
      unblock++;
    }
    if(asm_control1 & ctr_bit){
      *pu32CipherBuffer ++ = asm_ctr_result0;
      *pu32CipherBuffer ++ = asm_ctr_result1;
      *pu32CipherBuffer ++ = asm_ctr_result2;
      *pu32CipherBuffer ++ = asm_ctr_result3;

    }
    cipher_disable();
    u8Blocks--;
  }

  if(asm_control1 & cbc_bit){
    disable_mac();
    *pu32AuxCipherBuffer ++ = asm_cbc_result0;
    *pu32AuxCipherBuffer ++ = asm_cbc_result1;
    *pu32AuxCipherBuffer ++ = asm_cbc_result2;
    *pu32AuxCipherBuffer ++ = asm_cbc_result3;
  }
  return gSuccess_c;
}



/************************************************************************************
* DecipheMsg
*
* This function is called to cipher a message of 112 bytes maximun in CTR mode and 
* 96 bytes in CBC and CCM modes, the buffer length must be multiple of 16 bytes.
*
************************************************************************************/
FuncReturn_t DecipherMsg (uint32_t *pu32DecipherBuffer, uint8_t u8BufferLength)
{
  uint32_t MACBuffer[MACSize];
  uint32_t *pu32MACBuffer;
  uint8_t i, ccm_mode;
  FuncReturn_t cipher_result;
  
  if(test_fail == mCipherTestResult){
    return gFailTest_c;
  }  
   
  if(asm_control1 & cbc_bit){
    pu32MACBuffer = pu32DecipherBuffer + (u8BufferLength - MACSize);
    for(i = 0; i < MACSize; i++){
      MACBuffer[i] = *pu32MACBuffer++;
    }
    if(asm_control1 & ctr_bit){
      ccm_mode = gEnable_c;
      disable_cbc();
    }
    else{
      ccm_mode = gDisable_c;
      cipher_result = CipherMsg(pu32DecipherBuffer,u8BufferLength);
      if(cipher_result != gSuccess_c){
        return cipher_result;
      }
      pu32MACBuffer -= MACSize;
      for(i = 0; i < MACSize; i++){
        if (MACBuffer[i] != *pu32MACBuffer++){
          return gAuthenticationFailed_c;        
        }
      }
      return gAuthenticationPassed_c;
    }
  }
   
  if(asm_control1 & ctr_bit){
    cipher_result = CipherMsg(pu32DecipherBuffer,u8BufferLength);
    if(cipher_result != gSuccess_c){
      return cipher_result;
    }
  }
  else{
    /*Do nothing*/
  }
  
  if(gEnable_c == ccm_mode){
    disable_ctr();
    enable_cbc();
    cipher_result = CipherMsg(pu32DecipherBuffer,u8BufferLength);
    if(cipher_result != gSuccess_c){
      return cipher_result;
    }

    pu32MACBuffer -= MACSize;
    for(i = 0; i < MACSize; i++){
      if (MACBuffer[i] != *pu32MACBuffer++){
        enable_ctr();
        return gAuthenticationFailed_c;        
      }
    }
    enable_ctr();
    return gAuthenticationPassed_c;
  }
  else{
    /*Do nothing*/
  }
  return gSuccess_c;
}

/************************************************************************************
* DecipherMsgU8
*
* This function is called to cipher a message of 112 bytes maximun in CTR mode and 
* 96 bytes in CBC and CCM modes, the buffer length must be multiple of 16 bytes.
*
************************************************************************************/
FuncReturn_t DecipherMsgU8(uint8_t *pu8CipherBuffer, uint8_t u8BufferLength)
{
   uint8_t lenght;
   uint8_t mResidue_Msg;
   uint8_t mNumBlocks_Msg;
   FuncReturn_t fnReturn;
   lenght = u8BufferLength/4;


  mResidue_Msg = u8BufferLength%(BlockSize*4);
  mNumBlocks_Msg = u8BufferLength/(BlockSize*4);


 if( (gNull_c < mResidue_Msg)        || 
     (MaxNumBlocks < mNumBlocks_Msg) || 
     (gNull_c == mNumBlocks_Msg)){
      return gFailOutOfRange_c;
  }

   u8Buff_2_u32Buff(pu8CipherBuffer, &ciphering_buff[0], lenght);
   fnReturn = DecipherMsg(&ciphering_buff[0], lenght);
   u32Buff_2_u8Buff(pu8CipherBuffer, &ciphering_buff[0], lenght);
   return fnReturn;
}


/************************************************************************************
* CipherMsgU8
*
* This function is called to cipher a message of 112 bytes maximun in CTR mode and 
* 96 bytes in CBC and CCM modes, the buffer length must be multiple of 16 bytes.
*
************************************************************************************/
FuncReturn_t CipherMsgU8(uint8_t *pu8CipherBuffer, uint8_t u8BufferLength)
{
   uint8_t lenght;
   uint8_t mResidue_Msg;
   uint8_t mNumBlocks_Msg;
   FuncReturn_t fnReturn;
   lenght = u8BufferLength/4;


  mResidue_Msg = u8BufferLength%(BlockSize*4);
  mNumBlocks_Msg = u8BufferLength/(BlockSize*4);


 if( (gNull_c < mResidue_Msg)        || 
     (MaxNumBlocks < mNumBlocks_Msg) || 
     (gNull_c == mNumBlocks_Msg)){
      return gFailOutOfRange_c;
  }

   u8Buff_2_u32Buff(pu8CipherBuffer, &ciphering_buff[0], lenght);
   fnReturn = CipherMsg(&ciphering_buff[0], lenght);
   u32Buff_2_u8Buff(pu8CipherBuffer, &ciphering_buff[0], lenght);
   return fnReturn;
}



/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
static void u8Buff_2_u32Buff(uint8_t* u8Buff, uint32_t* u32Buff, uint8_t size)
{
  uint8_t i;
  bytes_2_word_tag tmp;
  for(i=0; i<size; i++)
  {
    tmp.Bytes.Byte0 = *u8Buff++;
    tmp.Bytes.Byte1 = *u8Buff++;
    tmp.Bytes.Byte2 = *u8Buff++;
    tmp.Bytes.Byte3 = *u8Buff++;
    *u32Buff = tmp.u32word;
    u32Buff++;
  }
}

static void u32Buff_2_u8Buff(uint8_t* u8Buff, uint32_t* u32Buff, uint8_t size)
{
  uint8_t i;
  bytes_2_word_tag tmp;
  for(i=0; i<size; i++)
  {
    tmp.u32word = *u32Buff;
    *u8Buff++ = tmp.Bytes.Byte0;
    *u8Buff++ = tmp.Bytes.Byte1;
    *u8Buff++ = tmp.Bytes.Byte2;
    *u8Buff++ = tmp.Bytes.Byte3;
    u32Buff++;
  }
}



/************************************************************************************
*************************************************************************************
* Private Debug stuff
*************************************************************************************
************************************************************************************/

#endif

