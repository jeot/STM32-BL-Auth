/**
  ******************************************************************************
  * @file    fwauth.c
  * @brief   Example of fw signature check.
  *          This file provides set of firmware functions to manage Com
  *          functionalities.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cmox_crypto.h"
#include "fwauth.h"
#include "crypto.h"
#include "ecc_pub_key.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Private function prototypes -----------------------------------------------*/
int32_t SignatureVerify(const uint8_t *pubkey, size_t pubkey_size, const uint8_t *digest, size_t digest_size, const uint8_t *signature, size_t signature_size);

int32_t STM32_SHA256_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength, uint8_t *MessageDigest, int32_t* MessageDigestLength);

void Fatal_Error_Handler(void)
{
  printf("\r\nFatal error! Enter endless loop!\r\n");
  while(1) {};
}


/**
  * @brief  SHA256 HASH digest compute example.
  * @param  InputMessage: pointer to input message to be hashed.
  * @param  InputMessageLength: input data message length in byte.
  * @param  MessageDigest: pointer to output parameter that will handle message digest
  * @param  MessageDigestLength: pointer to output digest length.
  * @retval error status: can be HASH_SUCCESS if success or one of
  *         HASH_ERR_BAD_PARAMETER, HASH_ERR_BAD_CONTEXT,
  *         HASH_ERR_BAD_OPERATION if error occured.
  */
int32_t STM32_SHA256_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength,
                                        uint8_t *MessageDigest, int32_t* MessageDigestLength)
{
  SHA256ctx_stt P_pSHA256ctx;
  uint32_t error_status = HASH_SUCCESS;

  /* Set the size of the desired hash digest */
  P_pSHA256ctx.mTagSize = CRL_SHA256_SIZE;

  /* Set flag field to default value */
  P_pSHA256ctx.mFlags = E_HASH_DEFAULT;

  error_status = SHA256_Init(&P_pSHA256ctx);

  /* check for initialization errors */
  if (error_status == HASH_SUCCESS)
  {
    /* Add data to be hashed */
    error_status = SHA256_Append(&P_pSHA256ctx,
                                 InputMessage,
                                 InputMessageLength);

    if (error_status == HASH_SUCCESS)
    {
      /* retrieve */
      error_status = SHA256_Finish(&P_pSHA256ctx, MessageDigest, MessageDigestLength);
    }
  }

  return error_status;
}


#define MAX_MEMBUF_SIZE 2048

int32_t SignatureVerify(const uint8_t *pubkey, size_t pubkey_size, const uint8_t *digest, size_t digest_size, const uint8_t *signature, size_t signature_size)
{
  cmox_ecc_retval_t rv;
  cmox_ecc_handle_t Ecc_Ctx;
  uint8_t membuf[MAX_MEMBUF_SIZE];
  uint32_t fault_check = CMOX_ECC_AUTH_FAIL;

  /* Initialize cryptographic library */
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    return -1;
  }

  /* Construct a ECC context */
  cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, membuf, sizeof(membuf));
  //cmox_ecc_construct(&Ecc_Ctx, CMOX_MATH_FUNCS_SMALL, membuf, sizeof(membuf));

  /* Verify directly the signature passing
   all the needed parameters */
  rv = cmox_ecdsa_verify(&Ecc_Ctx,
                         CMOX_ECC_CURVE_SECP256R1,
                         //CMOX_ECC_SECP256R1_LOWMEM,
                         pubkey, pubkey_size,
                         digest, digest_size,
                         signature, signature_size,
                         &fault_check);

  /* Cleanup context */
  cmox_ecc_cleanup(&Ecc_Ctx);

  /* Verify API returned value */
  if (rv != CMOX_ECC_AUTH_SUCCESS)
  {
    printf("SignatureVerify Failed. Err: %u", rv);
    return -1;
  }
  /* Verify Fault check variable value */
  if (fault_check != CMOX_ECC_AUTH_SUCCESS)
  {
    printf("SignatureVerify Failed. Err: %u", rv);
    return -2;
  }
  return 0;
}

static void print_buffer(char *name, uint8_t * buf, uint32_t size)
{
  int i;
  if (name != NULL )
  {
    printf("\r\n*** %s *** \r\n", name);
  }
  else
  {
    printf("\r\n");
  }
  for ( i = 0; i < size; i++ )
  {
    printf("%02x",buf[i]);
    if ((i+1)%8 == 0) printf("  ");
    if ((i+1)%16 == 0) printf("\r\n");
  }
  printf("\r\n");
}

int32_t FW_Verify(void)
{
  FW_Meta_t *pFWMeta = (FW_Meta_t *)FW_META_DATA_ADD;
  uint32_t hash_status = 0;
  uint32_t sig_status = -1;
  uint8_t MetaDigest[FW_HASH_LEN] = {0};
  int32_t MetaDigestLength = FW_HASH_LEN;

  /* enable CRC to allow cryptolib to work */
  __CRC_CLK_ENABLE();

  printf("\r\nStart APP FW Verification...\r\n");
  printf("FW Meta data saved on flash @0x%08x:\r\n", FW_META_DATA_ADD);
  printf("FW Magic: 0x%08x\r\n", pFWMeta->FWMagic);
  printf("FW Size: 0x%08x\r\n", pFWMeta->FwSize);
  printf("FW Version: 0x%08x\r\n", pFWMeta->FwVersion);
  print_buffer("FW HASH", pFWMeta->FwTag, FW_HASH_LEN);
  print_buffer("META DATA HASH", pFWMeta->MetaTag, FW_HASH_LEN);
  print_buffer("META DATA SIGNATURE", pFWMeta->MetaSig, FW_META_SIG_LEN);

  /* 1. Check the magic number of fw meta data */
  printf("\r\n Check FW Magic\r\n");
  if ( pFWMeta->FWMagic != FW_MAGIC )
  {
    goto ERROR;
  }
  printf(" FW Meta data Magic check OK!\r\n");

  /* 2. Verify fw meta data */
  printf("\r\n Check FW Meta data\r\n");
  /* 2.1 Compute meta data hash */
  printf("\r\n Check FW Meta data hash\r\n");
  hash_status = STM32_SHA256_HASH_DigestCompute(
		  (uint8_t*)pFWMeta, sizeof(FW_Meta_t) - FW_HASH_LEN - FW_META_SIG_LEN,
          &MetaDigest[0], &MetaDigestLength);
  if ((hash_status == HASH_SUCCESS) && (MetaDigestLength == FW_HASH_LEN))
  {
    /* 2.2 Compare meta data hash with the stored hash */
    int i;
    print_buffer("Computed META DATA HASH", MetaDigest, FW_HASH_LEN);

    for (i = 0; i < FW_HASH_LEN; i++)
    {
      if ( pFWMeta->MetaTag[i] != MetaDigest[i] )
      {
        goto ERROR;
      }
    }
    print_buffer("Saved META HASH", pFWMeta->MetaTag, MetaDigestLength);
    printf(" FW Meta data Hash check OK!\r\n");

  }
  else
  {
    goto ERROR;
  }

  /* 2.3 Verify meta data signature*/
  printf("\r\n Check FW Meta data signature\r\n");
  sig_status = SignatureVerify(SIGN_ECC_PUB_KEY, 64, pFWMeta->MetaTag, 32, pFWMeta->MetaSig, 64);

  if (sig_status == 0) {
    printf(" FW Meta data Signature check OK!\r\n");
  } else {
    printf(" FW Meta data Signature check FAIL!\r\n");
    goto ERROR;
  }

  /* 3. Compute fw hash and compare with fw tag in meta data */
  printf("\r\n Check FW Hash\r\n");

  /* 3.1 compute the fw hash */
  hash_status = STM32_SHA256_HASH_DigestCompute(
		  (uint8_t*)FW_ADD, pFWMeta->FwSize,
          &MetaDigest[0], &MetaDigestLength);
  if ((hash_status == HASH_SUCCESS) && (MetaDigestLength == FW_HASH_LEN))
  {
    int i;
    print_buffer("Computed FW HASH", MetaDigest, MetaDigestLength);
    print_buffer("Saved FW HASH", pFWMeta->FwTag, FW_HASH_LEN);
    /* 3.2 Compare fw hash with fw tag */
    for (i = 0; i < FW_HASH_LEN; i++)
    {
      if ( pFWMeta->FwTag[i] != MetaDigest[i] )
      {
        goto ERROR;
      }
    }
    printf(" FW Hash check OK!\r\n");
  }
  else
  {
    goto ERROR;
  }

  printf("\r\n\r\n");
  return 0;

ERROR:
  printf("ERROR\n");
  sig_status = -1;
  Fatal_Error_Handler();

  return sig_status;
}
