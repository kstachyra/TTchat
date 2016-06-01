#include <iostream>
#include <thread>

#include "easylogging++.h"
#include "ClientMonitor.h"
#include "flp/flp.h"

#define ELPP_THREAD_SAFE
INITIALIZE_EASYLOGGINGPP

using namespace std;

void easyLoggingInit(string const& confPath);

void serverListenThread(int port);
void serverServiceThread();

//monitor klientów
ClientMonitor clientMonitor;

int main(int argc,  char* argv[])
{
    //inicjalizacja loggera z podanym plikiem konfiguracyjnym
	easyLoggingInit("/home/ks/ClionProjects/TTchat/easyLogging.conf");

    //uruchomienie wątku nasłuchującego na nowe połączenia
    thread listenThread(serverListenThread, 1234);
    //uruchomienie wątku serwisowego
    thread serviceThread(serverServiceThread);






    //poczekaj na wątki
    if (listenThread.joinable()) listenThread.join();
    if (serviceThread.joinable()) serviceThread.join();


	return 0;
}

void easyLoggingInit(string const& confPath)
{
    el::Configurations conf(confPath);
    el::Loggers::reconfigureAllLoggers(conf);
    LOG(INFO) << "zainicjowano easylogging++";
}


/*
 * wątki serwera
 */

void serverListenThread(int port)
{
    LOG(INFO) << "wątek nasłuchujący na połączenia uruchomiony";

    bool isRunning = 1;
    FLP_Connection_t *newConnection;

    //while(isRunning)
    {
        //isRunning = FLP_Listen(&newConnection, port);

        //jeśli podany klucz newConnection nie istnieje w mapie
        if (clientMonitor.clients.find(newConnection) == clientMonitor.clients.end())
        {
            clientMonitor.addClient(newConnection);
            LOG(INFO) << "dodano klienta do monitora klientów";
        }
        //jeśli istnieje już
        else
        {
            LOG(INFO) << "klient próbujący nawiązać połączenie na port nasłuchujący już istnieje na liście";
        }

        LOG(INFO) << "usuwamy klienta z monitora przy pomocy removeClient";
        clientMonitor.removeClient(newConnection);
        LOG(INFO) << "usunięto klienta";
    }
}

void serverServiceThread()
{
    LOG(INFO) << "uruchomiono wątek serwisowy";
}