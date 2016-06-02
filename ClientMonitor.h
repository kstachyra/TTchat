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
            //to stwórz
            addChatroom(chatroomId);
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

    void removeClient(FLP_Connection_t * clientId)
    {
        enter();

        //jeśli klient nie istnieje w mapie
        if (clients.find(clientId) == clients.end())
        {
            std::cout <<"\n"<< "klient, którego chcemy usunąć nie istnieje w mapie";
        }
        else
        {
            //zamknij połączenie (FLP_Read i FLP_Write zwrócą błąd, wątki klienta skończą pracę niebawem)
            ////TODO FLP_Close(clientId);

            //TODO joinować czy detachować?
            //clients[clientId]->detachThreads();
            clients[clientId]->joinThreads();

            //usuń klienta z chatroomu
            chatrooms[clients[clientId]->chatroomId]->removeClient(clientId);

            //sprawdzamy, czy nie usunąć chatroomu (czy nie był to ostatni klient tego chatroomu)
            if (chatrooms[clients[clientId]->chatroomId]->isEmpty())
            {
                chatrooms[clients[clientId]->chatroomId]->joinThread();
                removeChatroom(clients[clientId]->chatroomId);
            }

            //zwalniamy pamięć tego klienta
            delete clients[clientId];

            //usuwamy wpis klienta z mapy
            clients.erase(clientId);
        }

        leave();
    }

    uint64_t getChatroomId(FLP_Connection_t *clientId)
    {
        //dostęp do elementów monitora musi być chroniony
        enter();
        uint64_t toReturn = clients[clientId]->chatroomId;
        leave();
        return toReturn;
    }

private:
    void addChatroom(uint64_t chatroomId)
    {
        Chatroom* newChatroom = new Chatroom(chatroomId);
        //wstaw do mapy chatroomów
        chatrooms[chatroomId]=newChatroom;
    }

    void removeChatroom(uint64_t chatroomId)
    {
        //zwalniamy pamięć tego chatroomu
        delete chatrooms[chatroomId];

        //usuwamy wpis chatroomu z mapy
        chatrooms.erase(chatroomId);
    }
};

ClientMonitor clientMonitor;

#endif //TTCHAT_CLIENTMONITOR_H
