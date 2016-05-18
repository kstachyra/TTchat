#include <iostream>
#include <thread>

#include "easylogging++.h"
#include "ClientMonitor.h"
#include "Connection.h"
#include "Message.h"

#define ELPP_THREAD_SAFE 1
INITIALIZE_EASYLOGGINGPP

using namespace std;

void easyLoggingInit(string const& confPath);

void serverListenThread(int port);

//monitor klientów
ClientMonitor clientMonitor;

int main(int argc,  char* argv[])
{
    //inicjalizacja loggera z podanym plikiem konfiguracyjnym
	easyLoggingInit("/home/ks/ClionProjects/TTchat/easyLogging.conf");

    //uruchomienie wątku nasłuchującego na nowe połączenia
	thread listenThread(serverListenThread, 1234);

    //poczekaj na wątki
    if (listenThread.joinable()) listenThread.join();


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

    Connection con(port);
    LOG(INFO) << "utworzono połączenie dla wątku nasłuchującego";

    Message x;
    x = con.getMessage();




}

void conversationThread()
{

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