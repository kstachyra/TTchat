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
#include <sstream>
#include "Message.h"

#define MODEL_ENABLE_LOG

#define MODEL_DB_NAME					((const char*)"ttchat")

#ifdef MODEL_ENABLE_LOG
	#define MODEL_LOG(format, ...)				printf(format, ##__VA_ARGS__)
#else
	#define MODEL_LOG(format, ...)
#endif

using namespace std;

class Model {
private:
	MYSQL connection;

public:
	Model(void);
	bool connect(string host, string user, string password);
	bool getLastMessageId(uint32_t chatRoomId, uint32_t* messageID);
	bool getNextMessageId(uint32_t chatRoomId, uint32_t* messageID);
	bool getNumOfMessages(uint32_t chatRoomId, uint32_t* numOfMessages);
	bool newMessage(uint32_t chatRoomId, Message message);
	bool getMessage(uint32_t chatRoomId, uint32_t messageId, Message *message);
	bool newChatRoom(uint32_t chatRoomId);
};

#endif /* MODEL_H_ */
