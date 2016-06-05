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
private:
	static ClientMonitor* instance;

public:
    //mapa identyfikator klienta -> klient
    unordered_map < FLP_Connection_t*, Client* > clients;

    //mapa aktywnych rozmów id rozmowy -> rozmowa
    unordered_map < uint64_t, Chatroom* > chatrooms;

public:
	ClientMonitor(void);

public:
	static ClientMonitor* getInstance(void);

    void addClient(FLP_Connection_t * clientId, uint64_t chatroomId);

    void removeClient(FLP_Connection_t * clientId);

    /*uint64_t getChatroomId(FLP_Connection_t *clientId)
    {
        //dostęp do elementów monitora musi być chroniony
        enter();
        uint64_t toReturn = clients[clientId]->chatroomId;
        leave();
        // musimy odblokować dostęp do monitora
        return toReturn;
    }*/

    void changeChatroomId(FLP_Connection_t *clientId, uint64_t newChatroomId);

private:
    void addChatroom(uint64_t chatroomId);

    void removeChatroom(uint64_t chatroomId);
};

#endif //TTCHAT_CLIENTMONITOR_H
