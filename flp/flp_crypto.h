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

/* Exported definitions and macros ----------------------------------------- */
#define FLP_AES_BLOCK_SIZE					((size_t)16)
#define FLP_SESSION_KEY_LENGTH				((size_t)32)
#define FLP_PUBLIC_KEY_LENGTH				((size_t)128)

/* Exported functions ------------------------------------------------------ */
#if defined (__cplusplus)
extern "C" {
#endif

// AES, CTR mode, 256 bit key, 128 bit block
bool FLP_AES_Init(void);
bool FLP_AES_Encrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer);
bool FLP_AES_Decrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer);
bool FLP_AES_GenerateSessionKey(uint8_t *sessionKey);

#if defined (__cplusplus)
}
#endif

#endif /* FLP_CRYPTO_H_ */
