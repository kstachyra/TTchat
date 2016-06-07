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
    list < FLP_Connection_t* > clientList;
    std::mutex listMutex;

    //kolejka wiadomości do obsłużenia przez wątek chatroomu z informacją, który klient ją umieścił
    std::queue <  std::pair < SLPPacket, FLP_Connection_t* > > chatroomQueue;

    //wątek chatroomu
    std::thread chatroomThread;

public:
    Chatroom(uint64_t id);

    void runThread();

    void joinThread();

    void detachThread();

    void addClient(FLP_Connection_t* c);

    void removeClient(FLP_Connection_t* c);

    bool isEmpty();

private:
    void chatroomThreadFunc();
    void manageQueueMessages();

    /*
     * funkcje do zarządzania wiadomościami
     */
    void SUBREQManage(SLPPacket* msg, FLP_Connection_t* c);
    void UNSUBManage(SLPPacket* msg, FLP_Connection_t* c);
    void GETINFManage(SLPPacket* msg, FLP_Connection_t* c);
    void PULLMSGSManage(SLPPacket* msg, FLP_Connection_t* c);
    void MSGCLIManage(SLPPacket* msg, FLP_Connection_t* c);
};

#endif //TTCHAT_CONVERSATION_H
