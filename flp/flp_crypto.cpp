#include "flp_crypto.h"
#include "flp_config.h"
#include <tomcrypt.h>

// TODO: Thread safe version

/* Private definitions and macros ------------------------------------------ */
#if FLP_CRYPTO_ENABLE_LOG
#define FLP_CRYPTO_LOG(format, ...)				printf(format, ##__VA_ARGS__)
#else
#define FLP_CRYPTO_LOG(format, ...)
#endif

/* Private types ----------------------------------------------------------- */
/* Private variables ------------------------------------------------------- */
static int errno;

// CTR module
static symmetric_CTR CTR;

// AES cipher
static bool AESInitialized = false;
static int AESCipherIdx, AESKeySize;
static unsigned long AESBlockSize;

// PRNG (Pseudo Random Number Generator)
static bool PRNGInitialized = false;
static prng_state PRNGState;

/* Private functions' prototypes ------------------------------------------- */
static bool FLP_PRNGInit(void);

/* Exported functions ------------------------------------------------------ */
bool FLP_AES_Init(void)
{
	// Register AES cipher
	if(register_cipher(&aes_desc) < 0) {
		FLP_CRYPTO_LOG("FLP_AES_Init: Could not register AES cipher.\n");
		return false;
	}

	// Find AES cipher index
	AESCipherIdx = find_cipher("aes");
	if(AESCipherIdx == -1) {
		FLP_CRYPTO_LOG("FLP_AES_Init: Could not find AES cipher.\n");
		return false;
	}

	// Get AES block size
	AESBlockSize = cipher_descriptor[AESCipherIdx].block_length;
	if(AESBlockSize != FLP_AES_BLOCK_SIZE) {
		FLP_CRYPTO_LOG("FLP_AES_Init: Selected block size is not supported.\n");
		return false;
	}

	// Check selected session key size
	AESKeySize = FLP_SESSION_KEY_LENGTH;
	if(cipher_descriptor[AESCipherIdx].keysize(&AESKeySize) != CRYPT_OK) {
		FLP_CRYPTO_LOG("FLP_AES_Init: Invalid session key size.\n");
		return false;
	}

	AESInitialized = true;

	return true;
}

bool FLP_AES_Encrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer)
{
	unsigned long bytesGenerated;

	if(!AESInitialized)  {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Initialize AES first.\n");
		return false;
	}

	// Initialize PRNG (Pseudo Random Number Generator)
	if(!FLP_PRNGInit()) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Could not initialize PRNG.\n");
		return false;
	}

	// Generate initialization vector using PRNG
	bytesGenerated = yarrow_read(initVector, AESBlockSize, &PRNGState);
	if(bytesGenerated != AESBlockSize) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Error occurred while generating initialization vector.\n");
		return false;
	}

	// Initialize CTR
	if((errno = ctr_start(AESCipherIdx, initVector, sessionKey, AESKeySize, 0, CTR_COUNTER_LITTLE_ENDIAN, &CTR)) != CRYPT_OK) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Could not start CTR module.\n");
		return false;
	}

	// Encrypt
	if((errno = ctr_encrypt(inBuffer, outBuffer, length, &CTR)) != CRYPT_OK) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Error occurred while encrypting.\n");
		return false;
	}

	return true;
}

bool FLP_AES_Decrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer)
{
	if(!AESInitialized)  {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Initialize AES first.\n");
		return false;
	}

	// Initialize CTR
	if((errno = ctr_start(AESCipherIdx, initVector, sessionKey, AESKeySize, 0, CTR_COUNTER_LITTLE_ENDIAN, &CTR)) != CRYPT_OK) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Could not start CTR module.\n");
		return false;
	}

	// Decrypt
	if((errno = ctr_decrypt(inBuffer, outBuffer, length, &CTR)) != CRYPT_OK) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Error occurred while decrypting.\n");
		return false;
	}

	return true;
}

bool FLP_AES_GenerateSessionKey(uint8_t *sessionKey)
{
	unsigned long bytesGenerated;

	if(!AESInitialized)  {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Initialize AES first.\n");
		return false;
	}

	// Initialize PRNG (Pseudo Random Number Generator)
	if(!FLP_PRNGInit()) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Could not initialize PRNG.\n");
		return false;
	}

	// Generate initialization vector using PRNG
	bytesGenerated = yarrow_read(sessionKey, AESKeySize, &PRNGState);
	if(bytesGenerated != AESKeySize) {
		FLP_CRYPTO_LOG("FLP_AES_Encrypt: Error occurred while generating session key.\n");
		return false;
	}

	return true;
}

/* Private functions ------------------------------------------------------- */
static bool FLP_PRNGInit(void)
{
	if(!PRNGInitialized) {

		// Register PRNG
		if (register_prng(&yarrow_desc) == -1) {
			FLP_CRYPTO_LOG("FLP_PRNGInit: Could not register PRNG.\n");
			return false;
		}

		// Setup yarrow
		if ((errno = rng_make_prng(128, find_prng("yarrow"), &PRNGState, NULL)) != CRYPT_OK) {
			FLP_CRYPTO_LOG("FLP_PRNGInit: Could not setup Yarrow PRNG. %s\n", error_to_string(errno));
			return false;
		}

		PRNGInitialized = true;
	}

	return true;
}
