/*
 * flp_crypto.h
 *
 *  Created on: 12 maj 2016
 *      Author: krystian
 */

#ifndef FLP_CRYPTO_H_
#define FLP_CRYPTO_H_

#include <stdint.h>

/* Exported definitions and macros ----------------------------------------- */
#define FLP_SESSION_KEY_LENGTH				64
#define FLP_PUBLIC_KEY_LENGTH				64

/* Exported functions ------------------------------------------------------ */
extern void FLP_Encrypt(void);
extern void FLP_Decrypt(void);
extern void FLP_GenerateSessionKey(uint8_t *sessionKey);

#endif /* FLP_CRYPTO_H_ */
