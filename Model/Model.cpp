/*
 * Model.c
 *
 *  Created on: 4 cze 2016
 *      Author: krystian
 */
#include "Model.h"

void finishWithError(MYSQL *connection) {
	printf("Model::connect: Error: %s\n", mysql_error(connection));
	mysql_close(connection);
}

Model::Model(void)
{
	mysql_init(&this->connection);
}

bool Model::connect(string host, string user, string password, string db)
{
	if(!mysql_real_connect(&this->connection, host.c_str(), user.c_str(), password.c_str(), db.c_str(), 0, NULL, 0))
	{
	    printf("Model::connect: Error: %s\n", mysql_error(&this->connection));
	    return false;
	}

	return true;
}
bool Model::getLastMessageId(uint32_t chatRoomId, uint32_t* messageID)
{
	std::stringstream queryBuilder;
	string query;

	queryBuilder << "SELECT MAX(message_id) FROM messages WHERE chatroom_id = ";
	queryBuilder << chatRoomId;

	query = queryBuilder.str();

	if(mysql_real_query(&this->connection, query.c_str(), strlen(query.c_str()))) {
		finishWithError(&this->connection);
		return false;
	}

	MYSQL_RES *result;

	if((result = mysql_store_result(&this->connection)) == NULL) {
		finishWithError(&this->connection);
		return false;
	}

	int num_fields = mysql_num_fields(result);
	if(num_fields != 1) {
		printf("WTF!?\n");
		return false;
	}

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)))
	  {
	      for(int i = 0; i < num_fields; i++)
	      {
	          printf("%s ", row[i] ? row[i] : "NULL");
	      }
	          printf("\n");
	  }
}

bool Model::getNextMessageId(uint32_t chatRoomId, uint32_t* messageID)
{
	bool result;

	this->getLastMessageId(chatRoomId, messageID);
	if(result) *messageID += 1;

	return result;
}

bool Model::getNumOfMessages(uint32_t chatRoomId, uint32_t* numOfMessages)
{
 /* SELECT COUNT(message_id) FROM messages WHERE chatroom_id = 5422 */
}

bool Model::newMessage(uint32_t chatRoomId, Message message)
{
 /* INSERT INTO `ttchat`.`messages` (`message_id`, `chatroom_id`, `message`, `user_ name`, `created_date`) VALUES ('123', '6969', 'Kacper to też pedal', 'Pawel', '2016-06-22 13:58:48') */
}

bool Model::getMessage(uint32_t chatRoomId, uint32_t messageId, Message *message)
{
 /* SELECT * FROM messages WHERE chatroom_id = 5422 AND message_id = 521 */
}
bool newChatRoom(uint32_t chatRoomId)
{
 /* INSERT INTO `ttchat`.`chatrooms` (`chatroom_id`, `last_message`, `created_date`) VALUES ('0', '0', '2016-06-08 00:00:00') */
}
