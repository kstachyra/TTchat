#include <iostream>
#include <thread>

#include "ClientMonitor.h"
#include "Model/Model.h"
#include "flp/flp.h"
#include "global.h"

using namespace std;

void serverListenThread(unsigned short port);
void serverServiceThread();

int main(int argc,char* argv[])
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

/*
 * wątki serwera
 */

void serverListenThread(unsigned short port)
{
    std::cout<< "serverListenThread: wątek nasłuchujący na połączenia uruchomiony" <<"\n";
    port = 1234;

    bool isRunning = true;
    FLP_Connection_t *newConnection;

    FLP_Listener_t listener;
    FLP_ListenerInit(&listener, port, "192.168.43.65");

    while(isRunning)
    {
        isRunning = FLP_Listen(&listener, &newConnection, 60000);

        if (newConnection == NULL)
        {
        	std::cout<< "serverListenThread: timeout, FLP_Listen zwraca true" <<"\n";
        	//TODO sprawdzić czy port jest aktualny czy nie, jeśli nie to zakończyć wątek (nowy powinien już być urucomiony
        }
        else
        {
        	std::cout<< "serverListenThread: nowe połączenie" <<"\n";
			//jeśli istnieje już
        	if (clientMonitor.clients.find(newConnection) != clientMonitor.clients.end())
			{
				std::cout<< "serverListenThread: klient próbujący nawiązać połączenie na port nasłuchujący już istnieje w mapie" <<"\n";
				//clientMonitor.removeClient(newConnection);
			}

			//jeśli podany klucz newConnection nie istnieje w mapie
			if (clientMonitor.clients.find(newConnection) == clientMonitor.clients.end())
			{
				//dodaj go do chatroomu dla nowych klientow
				clientMonitor.addClient(newConnection, 0xFFFFFFFFFFFFFFFF);
				std::cout<< "serverListenThread: dodano klienta " << newConnection << " do monitora klientów" <<"\n";
			}
        }
    }

    FLP_ListenerDeinit(&listener);
}

void serverServiceThread()
{
    std::cout<< "serverServiceThread: uruchomiono wątek serwisowy" <<"\n";
    //TODO spr czy zmiana portu, spr które przestarzałe chatroomy usunąć z bazy


    while(1)
    {
    	std::cout<<"\n\n";
    	std::cout<<"_____STATUS MONITORA_____\n";
    	std::cout<<"klientów: " << clientMonitor.clients.size() <<"\n";
    	std::cout<<"chatroomów: " << clientMonitor.chatrooms.size() <<"\n";
    	std::cout<<"\n\n";
    	sleep(5);
    }
}
