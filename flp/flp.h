#ifndef FLP_H_
#define FLP_H_

#include <cstdint>
#include <iostream>
#include <zconf.h>

typedef struct
{
	int fake_variable;
	bool stop;
} FLP_Connection_t;

bool FLP_Listen(FLP_Connection_t** connection, int socket);

bool FLP_Write(FLP_Connection_t *connection, uint8_t *data, size_t length);

bool FLP_Read(FLP_Connection_t *connection, uint8_t **data, size_t *length);

bool FLP_Close(FLP_Connection_t *connection);

#endif /* FLP_H_ */
