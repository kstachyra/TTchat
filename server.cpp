#include <iostream>
#include <thread>

#include "ClientMonitor.h"
#include "flp/flp.h"

using namespace std;

void serverListenThread(unsigned short port);
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

	return 0;
}

/*
 * wątki serwera
 */

void serverListenThread(unsigned short port)
{
    std::cout <<"\n"<< "wątek nasłuchujący na połączenia uruchomiony";
    port = 1234;

    bool isRunning = true;
    FLP_Connection_t *newConnection;

    FLP_Listener_t listener;
    FLP_ListenerInit(&listener, port, "192.168.43.65");

    while(isRunning)
    {
        isRunning = FLP_Listen(&listener, &newConnection, port);

        //jeśli podany klucz newConnection nie istnieje w mapie
        if (clientMonitor.clients.find(newConnection) == clientMonitor.clients.end())
        {
            //dodaj go do chatroomu dla nowych klientow
            clientMonitor.addClient(newConnection, 0xFFFFFFFF);
            std::cout <<"\n"<< "dodano klienta " << newConnection << " do monitora klientów";
        }
        //jeśli istnieje już
        else
        {
            std::cout <<"\n"<< "klient próbujący nawiązać połączenie na port nasłuchujący już istnieje w mapie";
        }
    }

    /*sleep(4);
    auto temp = clientMonitor.clients.begin();
    std::cout << "\n" << "zmieniam CHATROOM ID dla pierwszego klienta " << (*temp).first;
    clientMonitor.changeChatroomId((*temp).first, 0xFABFABFA);*/

    /*//tymczasowe usuwanie kientów
    for (auto it = clientMonitor.clients.begin(); it != clientMonitor.clients.end();)
    {
        auto temp = it;
        temp++;
        sleep(5);
        std::cout <<"\n"<< "Usuwam klienta " << (*it).first;
        clientMonitor.removeClient(it->first);
        std::cout <<"\n"<< "usunięto klienta " << (*it).first;

        it = temp;
    }*/

    FLP_ListenerDeinit(&listener);
}

void serverServiceThread()
{
    std::cout <<"\n"<< "uruchomiono wątek serwisowy";
}
