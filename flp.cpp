#include "flp.h"
#include "stdbool.h"
#include "string.h"

bool FLP_Transmitt(FLP_Connection_t *connection, uint8_t *data, size_t length)
{
	ssize_t bytesSent = 0;

	while(bytesSent != length) {
		bytesSent = write(connection->socket, &data[bytesSent], length - bytesSent);
		if(bytesSent < 0) return false;
	}

	return true;
}

bool FLP_Receive()
{
	FLP_Header_t *header;

	// Read header
	// ...

	if(header->length > 0) {

		// Read payload
		// ...
	}
}

bool FLP_IsDataPacket(FLP_Header_t *header)
{
	if(header->type == FLP_TYPE_DATA) {
		return true;
	} else {
		return false;
	}
}

bool FLP_SendAck(FLP_Connection_t *connection, uint16_t type)
{
	uint8_t buffer[sizeof(FLP_Header_t)];
	FLP_Header_t *header = buffer;

	header->type = FLP_SET_ACK_BIT(type);
	header->length = 0;

	return FLP_Transmitt(connection, buffer, sizeof(FLP_Header_t));
}

bool FLP_SendData(FLP_Connection_t *connection, uint8_t *data, size_t length)
{
	uint8_t buffer[length + sizeof(FLP_Header_t)];
	FLP_Header_t *header = buffer;

	header->type = FLP_TYPE_DATA;
	header->length = length;

	memcpy(buffer + sizeof(FLP_Header_t), data, length);


}
