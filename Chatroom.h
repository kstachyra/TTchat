#ifndef TTCHAT_CONVERSATION_H
#define TTCHAT_CONVERSATION_H

#include "SLPPacket.h"
#include "Client.h"

#include <mutex>
#include <list>
#include <thread>
#include <queue>


using namespace std;

class Chatroom
{
public:
    //identyfikator rozmowy
    uint64_t id;

private:
    //lista klientów należących do rozmowy wraz z mutexem
    list < Client* > clientList;
    std::mutex listMutex;

    //kolejka wiadomości do obsłużenia przez wątek chatroomu
    //std::queue < std::pair < SLPPacket, Client* > > chatroomQueue;
    std::queue <  std::pair < SLPPacket, Client* > > chatroomQueue;

    //wątek chatroomu
    std::thread chatroomThread;

public:
    Chatroom(uint64_t id);

    void runThread();

    void joinThread();

    void addClient(Client* c);

    void removeClient(Client* c);

    bool isEmpty();

private:
    void chatroomThreadFunc();
    void manageQueueMessages();

    /*
     * funkcje do zarządzania wiadomościami
     */
    void SUBREQManage(SLPPacket* msg, Client* c);
    void UNSUBManage(SLPPacket* msg, Client* c);
    void GETINFManage(SLPPacket* msg, Client* c);
    void PULLMSGSManage(SLPPacket* msg, Client* c);
    void MSGCLIManage(SLPPacket* msg, Client* c);
};

#endif //TTCHAT_CONVERSATION_H
