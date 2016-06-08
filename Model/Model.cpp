/*
 * Model.c
 *
 *  Created on: 4 cze 2016
 *      Author: krystian
 */
#include "Model.h"
#include "QueryBuilder.h"

static void finishWithError(const char *prefix, MYSQL *connection) {
	MODEL_LOG("%s: Error: %s\n", prefix, mysql_error(connection));
	mysql_close(connection);
}

Model::Model(void)
{
	mysql_init(&this->connection);
}

bool Model::connect(string host, string user, string password)
{
	if(!mysql_real_connect(&this->connection, host.c_str(), user.c_str(), password.c_str(), MODEL_DB_NAME, 0, NULL, 0))
	{
		finishWithError("Model::connect", &this->connection);
	    return false;
	}

	return true;
}

bool Model::getPort(uint32_t* port)
{
	QueryBuilder query(&this->connection);

	query.put("SELECT port FROM configuration ORDER BY conf_id DESC LIMIT 1", false);

	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {
		finishWithError("Model::getPort", &this->connection);
		return false;
	}

	MYSQL_RES *result;

	if((result = mysql_store_result(&this->connection)) == NULL) {
		finishWithError("Model::getPort", &this->connection);
		return false;
	}

	int num_fields = mysql_num_fields(result);
	if(num_fields != 1) {
		finishWithError("Model::getPort", &this->connection);
		return false;
	}

	MYSQL_ROW row;

	if(!(row = mysql_fetch_row(result))) {
		finishWithError("Model::getPort", &this->connection);
		return false;
	}

	*port = atoi(row[0]);

	return true;
}

bool Model::getLastMessageId(uint32_t chatRoomId, uint32_t* messageID)
{
	QueryBuilder query(&this->connection);

	query.put("SELECT MAX(message_id) FROM messages WHERE chatroom_id = ", false);
	query.put(chatRoomId);

	MODEL_LOG("Model::getLastMessageId: Sending query to database...\n");
	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {
		finishWithError("Model::getLastMessageId", &this->connection);
		return false;
	}

	MYSQL_RES *result;

	MODEL_LOG("Model::getLastMessageId: Storing results...\n");
	if((result = mysql_store_result(&this->connection)) == NULL) {
		finishWithError("Model::getLastMessageId", &this->connection);
		return false;
	}

	int num_fields = mysql_num_fields(result);
	MODEL_LOG("Model::getLastMessageId: Number of stored fields: %u.\n", num_fields);
	if(num_fields != 1) {
		finishWithError("Model::getLastMessageId", &this->connection);
		return false;
	}

	MYSQL_ROW row;

	MODEL_LOG("Model::getLastMessageId: Fetching row...\n");
	if(!(row = mysql_fetch_row(result))) {
		finishWithError("Model::getLastMessageId", &this->connection);
		return false;
	}

	if(row[0] != NULL) {
		*messageID = atoi(row[0]);
	} else {
		*messageID = 0;
	}

	return true;
}

bool Model::getNextMessageId(uint32_t chatRoomId, uint32_t* messageID)
{
	bool result;

	result = this->getLastMessageId(chatRoomId, messageID);
	if(result) *messageID += 1;

	return result;
}

bool Model::getNumOfMessages(uint32_t chatRoomId, uint32_t* numOfMessages)
{
	QueryBuilder query(&this->connection);

	query.put("SELECT COUNT(message_id) FROM messages WHERE chatroom_id = ", false);
	query.put(chatRoomId);

	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {

		MODEL_LOG("Model::getNumOfMessages: Sending query failed...\n");
		finishWithError("Model::getNumOfMessages", &this->connection);
		return false;
	}

	MYSQL_RES *result;

	if((result = mysql_store_result(&this->connection)) == NULL) {
		MODEL_LOG("Model::getNumOfMessages: Storing resuts failed...\n");
		finishWithError("Model::getNumOfMessages", &this->connection);
		return false;
	}

	int num_fields = mysql_num_fields(result);
	if(num_fields != 1) {
		MODEL_LOG("Model::getNumOfMessages: Reading number of fields failed...\n");
		finishWithError("Model::getNumOfMessages", &this->connection);
		return false;
	}

	MYSQL_ROW row;

	if(!(row = mysql_fetch_row(result))) {
		MODEL_LOG("Model::getNumOfMessages: Fetching row failed...\n");
		finishWithError("Model::getNumOfMessages", &this->connection);
		return false;
	}

	*numOfMessages = atoi(row[0]);

	return true;
}

bool Model::newMessage(uint32_t chatRoomId, Message message)
{
	QueryBuilder query(&this->connection);

	query.put("INSERT INTO `ttchat`.`messages` (`message_id`, `chatroom_id`, `message`, `user_ name`, `creation_date`) VALUES ('", false);
	query.put(message.id);
	query.put("', '", false);
	query.put(chatRoomId);
	query.put("', '", false);
	query.put(message.payload, message.payloadLength, true);
	query.put("', '", false);
	query.put(message.nick, sizeof(message.nick), true);
	query.put("', '", false);
	query.put(message.timestamp);
	query.put("')", false);

	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {
		finishWithError("Model::newMessage", &this->connection);
		return false;
	}

	return true;
}

bool Model::getMessage(uint32_t chatRoomId, uint32_t messageId, Message *message)
{
	QueryBuilder query(&this->connection);

	if(!message) return false;

	query.put("SELECT `message_id`, `message`, `user_ name`, `creation_date` FROM messages WHERE chatroom_id = ", false);
	query.put(chatRoomId);
	query.put(" AND message_id = ", false);
	query.put(messageId);

	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {
		finishWithError("Model::getMessage", &this->connection);
		return false;
	}

	MYSQL_RES *result;

	if((result = mysql_store_result(&this->connection)) == NULL) {
		finishWithError("Model::getMessage", &this->connection);
		return false;
	}

	int num_fields;
	MYSQL_ROW row;
	unsigned long *lengths;

	row = mysql_fetch_row(result);
	if(!row) {
		printf("Model::getMessage: mysql_fetch_row failed.\n");
		finishWithError("Model::getMessage", &this->connection);
		return false;
	}

	num_fields = mysql_num_fields(result);
	if(num_fields != 4) {
		printf("Model::getMessage: mysql_num_fields returned unexpected number of fields (num_fields=%u).\n", num_fields);
		return false;
	}

	lengths = mysql_fetch_lengths(result);

	// Extract id
	if(lengths[0] <= 0) {
		printf("Model::getMessage: length of the ID field should be greater than zero.\n");
		return false;
	}
	message->id = atoi(row[0]);

	// Extract payload
	message->payload = (uint8_t*)malloc(lengths[1]);
	memcpy(message->payload, row[1], lengths[1]);
	message->payloadLength = lengths[1];

	// Extract nick
	if(lengths[2] != MESSAGE_NICK_LENGTH) {
		printf("Model::getMessage: unsupported length of the nick field..\n");
		return false;
	}
	memcpy(message->nick, row[2], MESSAGE_NICK_LENGTH);

	// Extract timestamp
	if(lengths[3] <= 0) {
		printf("Model::getMessage: length of the timestamp field should be greater than zero.\n");
		return false;
	}
	message->timestamp = atoi(row[3]);

	return true;
}

bool Model::newChatRoom(uint32_t chatRoomId)
{
	QueryBuilder query(&this->connection);

	query.put("INSERT INTO `ttchat`.`chatrooms` (`chatroom_id`, `last_message`) VALUES ('", false);
	query.put(chatRoomId);
	query.put("', '0', '0')", false);

	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {
		finishWithError("Model::getNumOfMessages", &this->connection);
		return false;
	}

	return true;
}

bool Model::doesChatRoomExist(uint32_t chatRoomId, bool *gogel)
{
	printf("GOGEL!!!!!!!!!11one\n");

	QueryBuilder query(&this->connection);

	query.put("SELECT COUNT(chatroom_id) FROM chatrooms WHERE chatroom_id = ", false);
	query.put(chatRoomId);

	MODEL_LOG("Model::doesChatRoomExist: Sending query to database...\n");
	if(mysql_real_query(&this->connection, (const char*)query.getQuery(), (unsigned long)query.getLength())) {
		finishWithError("Model::getLastMessageId", &this->connection);
		return false;
	}

	MYSQL_RES *result;

	MODEL_LOG("Model::doesChatRoomExist: Storing results...\n");
	if((result = mysql_store_result(&this->connection)) == NULL) {
		finishWithError("Model::getLastMessageId", &this->connection);
		return false;
	}

	int num_fields = mysql_num_fields(result);
	MODEL_LOG("Model::doesChatRoomExist: Number of stored fields: %u.\n", num_fields);
	if(num_fields != 1) {
		finishWithError("Model::doesChatRoomExist", &this->connection);
		return false;
	}

	MYSQL_ROW row;

	MODEL_LOG("Model::doesChatRoomExist: Fetching row...\n");
	if(!(row = mysql_fetch_row(result))) {
		finishWithError("Model::doesChatRoomExist", &this->connection);
		return false;
	}

	if(row[0] != NULL) {
		if(atoi(row[0]) == 1) {
			*gogel = true;
		} else {
			*gogel = false;
		}
	} else {
		MODEL_LOG("Model::doesChatRoomExist: WTF?!\n");
		return false;
	}

	return true;
}
