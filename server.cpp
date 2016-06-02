#include <iostream>
#include <thread>

#include "ClientMonitor.h"
#include "flp/flp.h"

using namespace std;

void serverListenThread(int port);
void serverServiceThread();

int main(int argc,  char* argv[])
{
    //uruchomienie wątku nasłuchującego na nowe połączenia
    thread listenThread(serverListenThread, 1234);
    //uruchomienie wątku serwisowego
    thread serviceThread(serverServiceThread);






    //poczekaj na wątki
    if (listenThread.joinable()) listenThread.join();
    if (serviceThread.joinable()) serviceThread.join();

    for (auto it = clientMonitor.chatrooms.begin(); it != clientMonitor.chatrooms.end(); it++)
    {
        std::cout <<"\n"<< "czekam na watek czatroomu";
        it->second->joinThread();
    }

	return 0;
}

/*
 * wątki serwera
 */

void serverListenThread(int port)
{
    std::cout <<"\n"<< "wątek nasłuchujący na połączenia uruchomiony";

    bool isRunning = 1;
    FLP_Connection_t *newConnection;

    //while(isRunning)
    for (int i=0; i<10; ++i)
    {
        ////TODO isRunning = FLP_Listen(&newConnection, port);
        newConnection++;

        //jeśli podany klucz newConnection nie istnieje w mapie
        if (clientMonitor.clients.find(newConnection) == clientMonitor.clients.end())
        {
            clientMonitor.addClient(newConnection, 0xFFFFFFFF);
            std::cout <<"\n"<< "dodano klienta " << newConnection << " do monitora klientów";
        }
        //jeśli istnieje już
        else
        {
            std::cout <<"\n"<< "klient próbujący nawiązać połączenie na port nasłuchujący już istnieje w mapie";
        }
    }


    for (auto it = clientMonitor.clients.begin(); it != clientMonitor.clients.end();)
    {
        auto temp = it;
        temp++;
        sleep(5);
        //std::cout <<"\n"<< "Usuwam klienta";
        clientMonitor.removeClient(it->first);
        it = temp;
    }
}

void serverServiceThread()
{
    std::cout <<"\n"<< "uruchomiono wątek serwisowy";
}