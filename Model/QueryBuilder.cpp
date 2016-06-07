/*
 * QueryBuilder.cpp
 *
 *  Created on: 5 cze 2016
 *      Author: krystian
 */

#include "QueryBuilder.h"
#include <sstream>

QueryBuilder::QueryBuilder(MYSQL *connection)
{
	this->length = 0;
	this->buffer = (uint8_t*)malloc(sizeof(uint8_t));
	this->buffer[0] = 0;
	this->connection = connection;
}

void QueryBuilder::put(string s, bool escape)
{
	put((uint8_t*)s.c_str(), s.length(), escape);
}

void QueryBuilder::put(const char *s, bool escape)
{
	put((uint8_t*)s, strlen(s), escape);
}

void QueryBuilder::put(uint8_t *data, size_t length, bool escape)
{
	uint8_t escapedData[length*2+1];
	uint8_t *newBuffer;
	size_t newLength;
	ssize_t escapedDataLength;

	// Create legal SQL string
	if(escape) {
		escapedDataLength = mysql_real_escape_string(this->connection, (char*)escapedData, (const char*)data, (unsigned long)length);
		if(escapedDataLength < 0) {
			printf("QueryBuilder::put: mysql_real_escape_string failed.\n");
		}
	} else {
		escapedDataLength = length;
		memcpy(escapedData, data, length);
	}

	// Allocate memory for new buffer.
	// Actual size of the buffer is increased by one, so that we can put terminating character at the end of it.
	newLength = this->length + escapedDataLength;
	newBuffer = (uint8_t*)malloc(newLength + 1);

	// Copy old data and free memory if needed
	if(this->buffer) {
		memcpy(newBuffer, this->buffer, this->length);
		free(this->buffer);
	}

	// Copy new data and place terminating character at the end of it
	memcpy(newBuffer + this->length, escapedData, escapedDataLength);
	newBuffer[newLength] = 0;

	this->buffer = newBuffer;
	this->length = newLength;
}

void QueryBuilder::put(uint32_t num)
{
	std::stringstream s;

	s << num;

	put(s.str(), false);
}

const uint8_t* QueryBuilder::getQuery(void)
{
	return this->buffer;
}

size_t QueryBuilder::getLength(void)
{
	return this->length;
}

size_t QueryBuilder::getRealLength(void)
{
	return this->length + 1;
}

void QueryBuilder::printInfo(void)
{
	unsigned i;

	printf("Length of the contents of the QueryBuilder (not including terminating character): %u\n", (unsigned)this->length);
	printf("Content of the QueryBuilder (hex, not including terminating character):\n");

	for(i=0; i<this->length; i++) {
		printf("%02x", this->buffer[i]);
		if((i+1)%16==0) printf("\n");
	}
	printf("\n");
	printf("Content of the QueryBuilder (ASCII, not including terminating character):\n");

	for(i=0; i<this->length; i++) {
		printf("%c", this->buffer[i]);
		if((i+1)%16==0) printf("\n");
	}
	printf("\n");
}
