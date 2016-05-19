#include <tomcrypt.h>
#include "flp/flp.h"
#include "flp/flp_crypto.h"

int errno;

int main(void)
{
	uint8_t sessionKey[FLP_SESSION_KEY_LENGTH];
	uint8_t initVector[FLP_AES_BLOCK_SIZE];

	unsigned char inBuffer[1024], outBuffer[1024];
	char *infile = "./in.txt", *encfile = "./enc.txt", *decfile = "./dec.txt";
	FILE *fdin, *fdenc, *fddec;
	size_t bytesRead;

	// Initialize FLP_AES module
	if(!FLP_AES_Init()) {
		printf("FLP_AES_Init failed.\n");
		exit(-1);
	}

	// Generate session key
	if(!FLP_AES_GenerateSessionKey(sessionKey)) {
		printf("FLP_AES_GenerateSessionKey failed.\n");
		exit(-1);
	}

	// Encrypt ----------------------------------------------------------------

	// Open files
	fdin = fopen(infile,"rb");
	if (fdin == NULL) {
		perror("Can't open infile.");
		exit(-1);
	}
	fdenc = fopen(encfile,"wb");
	if (fdenc == NULL) {
		perror("Can't open encfile.");
		exit(-1);
	}

	// Read plain text
	bytesRead = fread(inBuffer, 1, sizeof(inBuffer), fdin);

	// Actually encrypt
	if(!FLP_AES_Encrypt(inBuffer, bytesRead, sessionKey, initVector, outBuffer)) {
		printf("FLP_AES_Encrypt failed.\n");
		exit(-1);
	}

	// Save result to file
	if (fwrite(outBuffer, 1, bytesRead, fdenc) != bytesRead) {
		printf("Error writing to output.\n");
		exit(-1);
	}

	// Close files
	fclose(fdin);
	fclose(fdenc);

	// Decrypt ----------------------------------------------------------------

	// Open files
	fdenc = fopen(encfile,"rb");
	if (fdenc == NULL) {
		perror("Can't open encfile.");
		exit(-1);
	}
	fddec = fopen(decfile,"wb");
	if (fddec == NULL) {
		perror("Can't open decfile.");
		exit(-1);
	}

	// Read cryptogram
	bytesRead = fread(inBuffer, 1, sizeof(inBuffer), fdenc);

	// Actually decrypt
	if(!FLP_AES_Decrypt(inBuffer, bytesRead, sessionKey, initVector, outBuffer)) {
		printf("FLP_AES_Decrypt failed.\n");
		exit(-1);
	}

	// Save result to file
	if (fwrite(outBuffer, 1, bytesRead, fddec) != bytesRead) {
		printf("Error writing to file.\n");
		exit(-1);
	}

	// Close files
	fclose(fdenc);
	fclose(fddec);

}
