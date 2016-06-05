#include <iostream>
#include <thread>

#include "ClientMonitor.h"
#include "Model/Model.h"
#include "flp/flp.h"

using namespace std;

int main(int argc,  char* argv[])
{
	Model model;
	Message message;
	uint32_t chatRoomId, messageId, numOfMessages;

	if(!model.connect("192.168.43.181", "krystian", "tajne")) {
		printf("Model::connect failed.\n");
	}

	chatRoomId = 6969;

	printf("Creating new chat room.\n");
	if(!model.newChatRoom(chatRoomId)) {
		printf("Model::newChatRoom failed.\n");
	}
	printf("Chat room created successfully.\n");

	if(!model.getLastMessageId(chatRoomId, &messageId)) {
		printf("Model::getLastMessageId failed.\n");
	}
	printf("Last message ID: %u\n", messageId);

	if(!model.getNextMessageId(chatRoomId, &messageId)) {
		printf("Model::getNextMessageId failed.\n");
	}
	printf("Next message ID: %u\n", messageId);

	if(!model.getNumOfMessages(chatRoomId, &numOfMessages)) {
		printf("Model::getNumOfMessages failed.\n");
	}
	printf("Number of messages: %u\n", numOfMessages);

	message.id = messageId;
	message.timestamp = 1236;
	message.payload = (uint8_t*)"Wink wink ;";
	message.payloadLength = strlen((char*)message.payload);
	memset(message.nick, 0, sizeof(message.nick));
	memcpy(message.nick, (const char*)"Jaszczur1337", 12);

	if(!model.newMessage(chatRoomId, message)) {
		printf("Model::getNumOfMessages failed.\n");
	}
	printf("New message successfully created.\n");

	if(!model.getMessage(chatRoomId, messageId, &message)) {
		printf("Model::getMessage failed.\n");
	}
	printf("Message read successfully.\n");

	return 0;
}
