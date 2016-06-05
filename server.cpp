#include <iostream>
#include <thread>

#include "ClientMonitor.h"
#include "flp/flp.h"
#include "global.h"

using namespace std;

void serverListenThread(unsigned short port);
void serverServiceThread();

int main(int argc,char* argv[])
{
    //uruchomienie wątku nasłuchującego na nowe połączenia
    thread listenThread(serverListenThread, 1234);
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
			//jeśli podany klucz newConnection nie istnieje w mapie
			if (clientMonitor.clients.find(newConnection) == clientMonitor.clients.end())
			{
				//dodaj go do chatroomu dla nowych klientow
				clientMonitor.addClient(newConnection, 0xFFFFFFFFFFFFFFFF);
				std::cout<< "serverListenThread: dodano klienta " << newConnection << " do monitora klientów" <<"\n";
			}
			//jeśli istnieje już
			else
			{
				std::cout<< "serverListenThread: klient próbujący nawiązać połączenie na port nasłuchujący już istnieje w mapie" <<"\n";
			}
        }
    }

    /*sleep(4);
    auto temp = clientMonitor.clients.begin();
    std::cout <<"zmieniam CHATROOM ID dla pierwszego klienta " << (*temp).first <<"\n";
    clientMonitor.changeChatroomId((*temp).first, 0xFABFABFA);*/

    /*//tymczasowe usuwanie kientów
    for (auto it = clientMonitor.clients.begin(); it != clientMonitor.clients.end();)
    {
        auto temp = it;
        temp++;
        sleep(5);
        std::cout<< "serverListenThread: Usuwam klienta " << (*it).first <<"\n";
        clientMonitor.removeClient(it->first);
        std::cout<< "serverListenThread: usunięto klienta " << (*it).first <<"\n";

        it = temp;
    }*/

    FLP_ListenerDeinit(&listener);
}

void serverServiceThread()
{
    std::cout<< "serverServiceThread: uruchomiono wątek serwisowy" <<"\n";
    //TODO spr czy zmiana portu, spr które przestarzałe chatroomy usunąć z bazy
}
