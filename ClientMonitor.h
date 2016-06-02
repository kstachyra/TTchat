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
    unordered_map < uint64_t, Chatroom* > chatrooms;

public:
    void addClient(FLP_Connection_t * clientId, uint64_t chatroomId)
    {
        enter();

        //tworz nowy obiekt klienta
        Client* newClient = new Client(clientId);
        clients[clientId] = newClient;

        //jeśli nie ma takiego chatroomu
        if(chatrooms.find(chatroomId) == chatrooms.end())
        {
            //to twórz
            Chatroom* newChatroom = new Chatroom(chatroomId);
            //wstaw do mapy chatroomów
            chatrooms[chatroomId]=newChatroom;
            //dodajdo klienta do chatroomu
            chatrooms[chatroomId]->addClient(clientId);
            //i uruchom wątek chatroomu
            chatrooms[chatroomId]->runThread();
        }
        else //a jak jest
        {
            //to tylko dodaj klienta doń
            chatrooms[chatroomId]->addClient(clientId);
        }

        //uuchom wątki klienta
        newClient->runThreads();

        leave();
    }

    int removeClient(FLP_Connection_t * clientId)
    {
        enter();

        //jeśli klient nie istnieje w mapie
        if (clients.find(clientId) == clients.end())
        {
            std::cout <<"\n"<< "klient, którego chcemy usunąć nie istnieje w mapie";
        }
        else
        {
            //zamknij połączenie (FLP_Read i FLP_Write zwrócą błąd, wątki klienta skończą pracę)
            ////TODO FLP_Close(clientId);

            //TODO joinować czy detachować?
            clients[clientId]->detachThreads();
            //clients[clientId]->joinThreads();

            //usuń klienta z chatroomu
            chatrooms[clients[clientId]->chatroomId]->removeClient(clientId);

            //zwalniamy pamięć tego klienta
            delete clients[clientId];

            //usuwamy wpis klienta z listy
            clients.erase(clientId);
        }

        leave();
    }

    uint64_t getChatroomId(FLP_Connection_t *clientId)
    {
        return clients[clientId]->chatroomId;
    }
};

#endif //TTCHAT_CLIENTMONITOR_H
