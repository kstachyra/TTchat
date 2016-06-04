#include "flp_crypto.h"

// TODO: Thread safe version

/* Private definitions and macros ------------------------------------------ */
#if FLP_CRYPTO_ENABLE_LOG
#define FLP_CRYPTO_LOG(format, ...)				printf(format, ##__VA_ARGS__)
#else
#define FLP_CRYPTO_LOG(format, ...)
#endif

/* Private types ----------------------------------------------------------- */
typedef struct {
	uint8_t initVector[FLP_AES_BLOCK_SIZE];
	unsigned int num;
	unsigned char ecount[FLP_AES_BLOCK_SIZE];
} CTR_State_t;

/* Private variables ------------------------------------------------------- */
/* Private functions' prototypes ------------------------------------------- */
void CTR_Init(CTR_State_t *state, uint8_t initVector[FLP_AES_BLOCK_SIZE]);

/* Exported functions ------------------------------------------------------ */
bool FLP_Crypto_AESEncrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer)
{
	AES_KEY key;
	CTR_State_t CTR_State;

	// Generate initialization vector
	FLP_CRYPTO_LOG("FLP_Crypto_AESEncrypt: Generating initialization vector...\n");
	if(!RAND_bytes(initVector, AES_BLOCK_SIZE)) {
		FLP_CRYPTO_LOG("FLP_Crypto_AESEncrypt: Generating initialization vector failed.\n");
		return false;
	}

	// Initialize CTR state
	FLP_CRYPTO_LOG("FLP_Crypto_AESEncrypt: Initializing CTR state.\n");
	CTR_Init(&CTR_State, initVector);

	// Set encryption key
	FLP_CRYPTO_LOG("FLP_Crypto_AESEncrypt: Setting encryption key.\n");
	if(AES_set_encrypt_key(sessionKey, FLP_SESSION_KEY_LENGTH*8, &key) < 0) {
		FLP_CRYPTO_LOG("FLP_Crypto_AESEncrypt: Setting AES encryption key failed.\n");
		return false;
	}

	FLP_CRYPTO_LOG("FLP_Crypto_AESEncrypt: Encrypting data.\n");
	AES_ctr128_encrypt(inBuffer, outBuffer, length, &key, CTR_State.initVector, CTR_State.ecount, &CTR_State.num);

	return true;
}

bool FLP_Crypto_AESDecrypt(uint8_t *inBuffer, size_t length, uint8_t *sessionKey, uint8_t *initVector, uint8_t *outBuffer)
{
	AES_KEY key;
	CTR_State_t CTR_State;

	// Initialize CTR state
	CTR_Init(&CTR_State, initVector);

	// Set encryption key
	if(AES_set_encrypt_key(sessionKey, FLP_SESSION_KEY_LENGTH*8, &key) < 0) {
		FLP_CRYPTO_LOG("FLP_Crypto_AESDecrypt: Setting AES encryption key failed.\n");
		return false;
	}

	AES_ctr128_encrypt(inBuffer, outBuffer, length, &key, CTR_State.initVector, CTR_State.ecount, &CTR_State.num);

	return true;
}

bool FLP_Crypto_AESGenerateSessionKey(uint8_t *sessionKey)
{
	// Generate initialization vector
	if(!RAND_bytes(sessionKey, FLP_SESSION_KEY_LENGTH)) {
		FLP_CRYPTO_LOG("FLP_Crypto_AESGenerateSessionKey: Generating session key failed.\n");
		return false;
	}

	return true;
}

bool FLP_Crypto_RSAEncryptSessionKey(uint8_t *data, ssize_t length, uint8_t *publicKey, uint8_t *encrypted)
{
	RSA *rsa = NULL;
	BIO *keybio;

	// Create buffer for null terminated key
	keybio = BIO_new_mem_buf(publicKey, -1);
	if(keybio == NULL) {
		FLP_CRYPTO_LOG("FLP_Crypto_RSAEncryptSessionKey: Failed to create key BIO.\n");
		return false;
	}

	// Read public key
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	if(rsa == NULL) {
		FLP_CRYPTO_LOG("FLP_Crypto_RSAEncryptSessionKey: Failed to create RSA.\n");
		return false;
	}

	RSA_public_encrypt(length, data, encrypted, rsa, FLP_RSA_PADDING);

	return true;
}

/* Private functions ------------------------------------------------------- */
void CTR_Init(CTR_State_t *state, uint8_t initVector[FLP_AES_BLOCK_SIZE])
{
    state->num = 0;
    memset(state->ecount, 0, FLP_AES_BLOCK_SIZE);
    memcpy(state->initVector, initVector, FLP_AES_BLOCK_SIZE);
}
