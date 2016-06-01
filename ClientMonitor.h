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
#include "Chatroom.h"

using namespace std;


class ClientMonitor : private Monitor
{
public:
    //mapa identyfikator klienta -> klient
    unordered_map < FLP_Connection_t*, Client* > clients;

    //mapa aktywnych rozmów id rozmowy -> rozmowa
    unordered_map < int, Chatroom > chatrooms;

public:

    int addClient(FLP_Connection_t * connection)
    {
        enter();

        Client* newClient = new Client(connection);
        clients[connection] = newClient;
        newClient->runThreads();

        leave();

        return 0;
    }

    int removeClient(FLP_Connection_t * connection)
    {
        enter();

        //jeśli klient nie istnieje w mapie
        if (clients.find(connection) == clients.end())
        {
            LOG(INFO) << "klient, którego chcemy usunąć nie istnieje w mapie";
        }
        else
        {
            //W MONITORZE NIE JOINOWAĆ !!!
            clients[connection]->joinThreads();
            LOG (INFO) <<"zjoinowano w monitorze";

            //zwalniamy pamięć tego klienta
            delete clients[connection];

            //usuwamy wpis klienta z listy
            clients.erase(connection);
        }

        leave();
    }

    void setChatroomId(FLP_Connection_t *connection)
    {
        cout<<clients[connection]->chatroomId;
    }

};

#endif //TTCHAT_CLIENTMONITOR_H
