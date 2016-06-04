/*
 * flp_config.h
 *
 *  Created on: 19 maj 2016
 *      Author: krystian
 */

#ifndef FLP_CONFIG_H_
#define FLP_CONFIG_H_

#include <openssl/rsa.h>

/* Library configuration --------------------------------------------------- */
/* Enable general log */
//#define FLP_ENABLE_LOG								true

/* Enable log in crypto module  */
//#define FLP_CRYPTO_ENABLE_LOG						true

#define FLP_AES_BLOCK_SIZE					((size_t)AES_BLOCK_SIZE)

#define FLP_SESSION_KEY_LENGTH				((size_t)32)

#define FLP_PUBLIC_KEY_LENGTH				((size_t)128)

#define FLP_PUBLIC_KEY_PEM_LENGTH			((size_t)271)

#define FLP_RSA_PADDING						RSA_PKCS1_OAEP_PADDING

#endif /* FLP_CONFIG_H_ */
