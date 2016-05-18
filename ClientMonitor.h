/*
 * monitor do zarządzania klientami, tworzenia, usuwania i przypisywania ich do rozmów
 * dziedziczy po klasie Monitor
 * zapewnia bezpieczny dostęp do współdzielonych zasobów
 */

#ifndef TTCHAT_CLIENTMONITOR_H
#define TTCHAT_CLIENTMONITOR_H

#include <unordered_map>
#include <list>

#include "Monitor.h"
#include "Client.h"
#include "Conversation.h"

using namespace std;


class ClientMonitor : private Monitor
{
public:
    /*int clientCount;
    int conversationCount;*/

    //mapa identyfikator klienta -> klient
    unordered_map < int, Client > clients;

    //mapa aktywnych rozmów id rozmowy -> rozmowa
    unordered_map < int, Conversation > conversations;

public:
    //ClientMonitor() : clientCount(0), conversationCount(0) {}

    int addClient()
    {
        enter();
        Client newClient;
        //potrzebne parametry klienta
        //stworzyć konkstruktor dla klienta
        clients[newClient.id] = newClient;
        leave();

        return 0;
    }
};


#endif //TTCHAT_CLIENTMONITOR_H
