#ifndef TTCHAT_CLIENT_H
#define TTCHAT_CLIENT_H
#include <sys/types.h>
#include "flp/flp.h"
#include <thread>
#include <queue>
#include <mutex>
#include <iostream>

#include "Semaphore.h"
#include "SLPPacket.h"


class Client
{
public:
    FLP_Connection_t *id;
    uint64_t chatroomId;

private:
    std::queue < SLPPacket > transmitterQueue;
    std::queue < SLPPacket > receiverQueue;

    std::mutex transmitterMutex;
    std::mutex receiverMutex;

    Semaphore transmitterEmpty;
    Semaphore receiverEmpty;

    std::thread transmitterThread;
    std::thread receiverThread;

    //czy wątki klienta do zamknięcia
    bool toClose = 0;
public:
    Client(FLP_Connection_t* con, uint64_t chatId);

    void setChatroomId(uint64_t chatId);

    void runThreads();

    void joinThreads();

    void detachThreads();

    /*wpisuje do kolejki transmittera daną (jedną) wiadomość*/
    void addToTransmitter(SLPPacket msg);

    /*przypisuje wszystkie wiadomości z receiverQueue do wskazanej wskaźnikiem tempQueue*/
    void getFromReceiver(std::queue < SLPPacket >* tempQueue);

    void close();
private:
    void receiverThreadFunc();
    void transmitterThreadFunc();
};

#endif //TTCHAT_CLIENT_H
