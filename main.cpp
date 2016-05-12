#include <iostream>
#include <thread>
#include <unordered_map>

#include "easylogging++.h"
#include "monitor.cpp"
#include "protocol.h"

#define ELPP_THREAD_SAFE
INITIALIZE_EASYLOGGINGPP

using namespace std;

void hello();
//void* ProducentX(void *CZAS);
void easyLoggingInit(string const& confPath);
//int mainamo();

void serverListenThread(int port);

//monitor klientów
ClientMonitor clientMonitor;

int main(int argc,  char* argv[])
{
    //inicjalizacja loggera z podanym plikiem konfiguracyjnym
	easyLoggingInit("/home/ks/ClionProjects/TTchat/easyLogging.conf");

    //uruchomienie wątku nasłuchującego na nowe połączenia
	thread listenThread(serverListenThread, 1234);


    if (listenThread.joinable())
        listenThread.join();


	return 0;
}

void easyLoggingInit(string const& confPath)
{
    el::Configurations conf(confPath);
    el::Loggers::reconfigureAllLoggers(conf);
    LOG(INFO) << "zainicjowano easylogging++";
}


void serverListenThread(int port)
{
    listenSocketInit(port);
    LOG(INFO) << "wątek nasłuchujący na połączenia uruchomiony";
    int x = listen();

    if (x==1)
        clientMonitor.addClient();

}

void clientReciverThread()
{

}

void clientTransmitterThread()
{

}

void serviceThread()
{
    LOG(INFO) << "uruchomiono wątek serwisowy";
}