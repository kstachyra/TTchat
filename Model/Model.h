/*
 * Model.h
 *
 *  Created on: 4 cze 2016
 *      Author: krystian
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "Model/Message.h"

using namespace std;

class Model {
private:
	MYSQL connection;

public:
	Model(void);
	bool connect(string host, string user, string password, string db);
	bool getLastMessageId(uint32_t* messageID);
	bool getNumOfMessages(uint32_t* numOfMessages);
	bool addMessage(Message message);
	bool getMessage(Message *message);
};

#endif /* MODEL_H_ */
