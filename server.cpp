#include <iostream>
#include <thread>

#include "ClientMonitor.h"
#include "flp/flp.h"
#include "global.h"
#include "Model/Model.h"

using namespace std;

void serverListenThread(uint32_t port);
void serverServiceThread();

char *serverAddress;
uint32_t port;

int main(int argc,char* argv[])
{
	char *databaseAddress;

	if(argc != 3) {
		cout << "Usage: " << argv[0] << " <serverAddress> <databaseAddress>\n";
		return 0;
	}

	databaseAddress = argv[2];
	serverAddress = argv[1];

	//połączenie z baz
	std::cout << "main: Creating model..." << endl;
	if(!model.connect(databaseAddress, "krystian", "tajne"))
	{
			std::cout<<"Model::connect failed.\n";
			exit(0);
	}

	//pobierz aktualny port
	std::cout << "main: Reading listening port from model..." << endl;
	model.getPort(&port);

    //uruchomienie wątku nasłuchującego na nowe połączenia
    thread listenThread(serverListenThread, port);
    //uruchomienie wątku serwisowego
    thread serviceThread(serverServiceThread);






    //poczekaj na wątki
    if (listenThread.joinable()) listenThread.join();
    if (serviceThread.joinable()) serviceThread.join();

	return 0;
}

/*
 * wątki serwera
 */

void serverListenThread(uint32_t port)
{
    std::cout<< "serverListenThread: wątek nasłuchujący na połączenia uruchomiony" <<"\n";

    bool isRunning = true;
    FLP_Connection_t *newConnection;

    FLP_Listener_t listener;
    FLP_ListenerInit(&listener, port, serverAddress);

    while(isRunning)
    {
        isRunning = FLP_Listen(&listener, &newConnection, 5000);

        if (newConnection == NULL)
        {
        	std::cout<< "serverListenThread: timeout, FLP_Listen zwraca true" <<"\n";
        	//aktualizuj port nasłuchujący
        	model.getPort(&port);
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
//    	std::cout<<"\n\n";
//    	std::cout<<"_____STATUS MONITORA_____\n";
//    	std::cout<<"klientów: " << clientMonitor.clients.size() <<"\n";
//    	std::cout<<"chatroomów: " << clientMonitor.chatrooms.size() <<"\n";
//    	std::cout<<"\n\n";
    	sleep(5);
    }
}
