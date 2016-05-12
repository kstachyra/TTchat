#ifndef FLP_H
#define FLP_H

#define FLP_SYMMETRICAL_KEY_LENGTH			64
#define FLP_ASYMMETRICAL_KEY_LENGTH			64

#define FLP_TYPE_CLIENT_HELLO				((uint16_t)0x0000)
#define FLP_TYPE_SERVER_HELLO				((uint16_t)0x0001)
#define FLP_TYPE_DATA						((uint16_t)0x0002)

#define FLP_SET_ACK_BIT(type)				(((uint16_t)type) | 0x8000)

typedef struct {
	uint16_t type;
	uint16_t length;
} FLP_Header_t;

typedef struct {
	int socket;
	uint8_t symmetricalKey[FLP_SYMMETRICAL_KEY_LENGTH];
	uint8_t asymmetricalKey[FLP_ASYMMETRICAL_KEY_LENGTH];
} FLP_Connection_t;

bool FLP_Send(uint8_t *data, size_t length);
uint8_t *FLP_Encrypt(uint8_t *data, size_t dataLength, uint8_t key, size_t keyLength);

#endif /* FLP_H */
