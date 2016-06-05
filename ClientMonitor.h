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
        Client* newClient = new Client(clientId, chatroomId);
        clients[clientId] = newClient;

        //jeśli nie ma takiego chatroomu
        if(chatrooms.find(chatroomId) == chatrooms.end())
        {
            //to stwórz
            addChatroom(chatroomId);
            //dodajdo klienta do chatroomu
            chatrooms[chatroomId]->addClient(newClient);
            //i uruchom wątek chatroomu
            chatrooms[chatroomId]->runThread();
        }
        else //a jak jest
        {
            //to tylko dodaj klienta doń
            chatrooms[chatroomId]->addClient(newClient);
        }

        //uuchom wątki klienta
        newClient->runThreads();

        leave();
    }

    void removeClient(FLP_Connection_t * clientId)
    {
        enter();
        //poinformuj klienta, że go zamykamy
        clients[clientId]->close();
        //jeśli klient nie istnieje w mapie
        if (clients.find(clientId) == clients.end())
        {
            std::cout << "ClientMonitor.removeClient: klient, którego chcemy usunąć nie istnieje w mapie" <<"\n";
        }
        else
        {
            //TODO joinować czy detachować? (wątki z powodu FLP_Close skończą pracę tak szybko jak to możliwe)
            //clients[clientId]->detachThreads();
            clients[clientId]->joinThreads();

            //usuń klienta z chatroomu
            chatrooms[clients[clientId]->chatroomId]->removeClient(clients[clientId]);

            //sprawdzamy, czy nie usunąć chatroomu (czy nie był to ostatni klient tego chatroomu)
            if (chatrooms[clients[clientId]->chatroomId]->isEmpty())
            {
                std::cout << "removeClient: BYŁ TO OSTATNI KLIENT, USUWAM CHATROOM " << clients[clientId]->chatroomId <<"\n";

                //skoro to był ostatni klient w chatroomie, to wątek chatroomu niebawem skończy pracę
                //TODO joinować czy detachować?
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

    /*uint64_t getChatroomId(FLP_Connection_t *clientId)
    {
        //dostęp do elementów monitora musi być chroniony
        enter();
        uint64_t toReturn = clients[clientId]->chatroomId;
        leave();
        // musimy odblokować dostęp do monitora
        return toReturn;
    }*/

    void changeChatroomId(FLP_Connection_t *clientId, uint64_t newChatroomId)
    {
        enter();
        //usuń ze starego chatroomu
        chatrooms[clients[clientId]->chatroomId]->removeClient(clients[clientId]);
        //zmień w kliencie informacje o chatroomie
        clients[clientId]->chatroomId = newChatroomId;

        //jeśli nie ma takiego chatroomu
        if(chatrooms.find(newChatroomId) == chatrooms.end())
        {
            //to stwórz
            addChatroom(newChatroomId);
            //dodajdo klienta do chatroomu
            chatrooms[newChatroomId]->addClient(clients[clientId]);
            //i uruchom wątek chatroomu
            chatrooms[newChatroomId]->runThread();
        }
        else //a jak jest
        {
            //to tylko dodaj klienta doń
            chatrooms[newChatroomId]->addClient(clients[clientId]);
        }

        leave();
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
        std::cout << "removeChatroom: usuwam chatroom " << chatrooms[chatroomId]->id <<"\n";
        //zwalniamy pamięć tego chatroomu
        delete chatrooms[chatroomId];

        //usuwamy wpis chatroomu z mapy
        chatrooms.erase(chatroomId);
    }
};

ClientMonitor clientMonitor;

#endif //TTCHAT_CLIENTMONITOR_H
