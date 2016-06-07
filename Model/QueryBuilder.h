/*
 * QueryBuilder.h
 *
 *  Created on: 5 cze 2016
 *      Author: krystian
 */

#ifndef MODEL_QUERYBUILDER_H_
#define MODEL_QUERYBUILDER_H_

#include <stdlib.h>
#include <string>
#include <cstring>
#include <mysql/mysql.h>

using namespace std;

class QueryBuilder {
private:
	uint8_t *buffer;
	size_t length;
	MYSQL *connection;

public:
	QueryBuilder(MYSQL *connection);
	void put(string s, bool escape);
	void put(const char *s, bool escape);
	void put(uint8_t *data, size_t length, bool escape);
	void put(uint32_t num);

	const uint8_t* getQuery(void);

	/* Size does not include terminating character which is placed at the end of the buffer. */
	size_t getLength(void);
	size_t getRealLength(void);

	void printInfo(void);
};

#endif /* MODEL_QUERYBUILDER_H_ */
