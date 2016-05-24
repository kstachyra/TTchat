#include "flp/flp_crypto.h"

int main(int argc, char *argv[])
{
	int i;
    uint8_t sessionKey[FLP_SESSION_KEY_LENGTH + 1];
    uint8_t plainText[1024];
    uint8_t encrypted[1024];
    uint8_t decrypted[1024];
    uint8_t initVector[FLP_AES_BLOCK_SIZE];

    printf("Generating session key... ");
    if(FLP_Crypto_AESGenerateSessionKey(sessionKey)) {
    	printf("OK.\n");
    } else {
    	printf("FAILED.\n");
    }

    sessionKey[FLP_SESSION_KEY_LENGTH] = 0;

    printf("Session key: ");
    for(i=0; i<FLP_SESSION_KEY_LENGTH; i++) {
    	printf("%x", sessionKey[i]);
    }
    printf("\n");

    printf("Input plain text:");
    gets((char*)plainText);

    printf("Encrypting plain text... ");
    if(FLP_Crypto_AESEncrypt(plainText, strlen((char *)plainText) + 1, sessionKey, initVector, encrypted)) {
    	printf("OK.\n");
    } else {
    	printf("FAILED.\n");
    }

    printf("Decrypting plain text... ");
    if(FLP_Crypto_AESDecrypt(encrypted, strlen((char *)plainText) + 1, sessionKey, initVector, decrypted)) {
    	printf("OK.\n");
    } else {
    	printf("FAILED.\n");
    }

    printf("Decrypted message: %s\n", decrypted);

	return 0;
}
