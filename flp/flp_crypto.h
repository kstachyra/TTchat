/*
 * flp_crypto.h
 *
 *  Created on: 12 maj 2016
 *      Author: krystian
 */

#ifndef FLP_CRYPTO_H_
#define FLP_CRYPTO_H_

/* Exported definitions and macros ----------------------------------------- */
#define FLP_SYMMETRICAL_KEY_LENGTH			64
#define FLP_ASYMMETRICAL_KEY_LENGTH			64

/* Exported functions ------------------------------------------------------ */
void FLP_Encrypt(void);
void FLP_Decrypt(void);

#endif /* FLP_CRYPTO_H_ */
