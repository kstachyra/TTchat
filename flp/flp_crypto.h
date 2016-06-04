/*
 * flp_crypto.h
 *
 *  Created on: 12 maj 2016
 *      Author: krystian
 */

#ifndef FLP_CRYPTO_H_
#define FLP_CRYPTO_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>

#include "flp_config.h"

/* Exported definitions and macros ----------------------------------------- */
/* Exported functions ------------------------------------------------------ */

// AES, CTR mode, 256 bit key, 128 bit block
bool FLP_Crypto_AESEncrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer);
bool FLP_Crypto_AESDecrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer);
bool FLP_Crypto_AESGenerateSessionKey(uint8_t *sessionKey);

// RSA
bool FLP_Crypto_RSAEncryptSessionKey(uint8_t *data, ssize_t length, uint8_t *publicKey, uint8_t *encrypted);

#endif /* FLP_CRYPTO_H_ */
