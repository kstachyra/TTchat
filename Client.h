#ifndef TTCHAT_CLIENT_H
#define TTCHAT_CLIENT_H
#include <sys/types.h>
#include "flp/flp.h"
#include "Message.h"
#include "easylogging++.h"
#include <thread>
#include <queue>
#include <mutex>


class Client
{
public:
    FLP_Connection_t *connection;
    int chatroomId;

private:
    std::queue <Message> transmitterQueue;
    std::queue <Message> receiverQueue;

    std::mutex transmitterMutex;
    std::mutex receiverMutex;

    std::mutex transmitterEmpty;
    std::mutex receiverEmpty;

    std::thread transmitterThread;
    std::thread receiverThread;

public:
    Client()
    {
        connection = NULL;
        chatroomId = 0;

        //początkowe wartości mutexów empty na zablokowane
        transmitterEmpty.lock();
        receiverEmpty.lock();
    }

    Client(FLP_Connection_t * con)
    {
        this->connection = con;

        //początkowe wartości mutexów empty na zablokowane
        transmitterEmpty.lock();
        receiverEmpty.lock();
    }

    void setChatroomId(FLP_Connection_t* con, int id)
    {
        chatroomId = id;
    }

    void runThreads()
    {
        transmitterThread = std::thread(&Client::transmitterThreadFunc, this);
        receiverThread =  std::thread(&Client::receiverThreadFunc, this);


        /*//odłączam wątki
        transmitterThread.detach();
        receiverThread.detach();*/
    }

    void joinThreads()
    {
        if (transmitterThread.joinable()) transmitterThread.join();
        if (receiverThread.joinable()) receiverThread.join();
    }

    void addToTransmitter(Message msg)
    {
        transmitterMutex.lock();
        transmitterQueue.push(msg);
        transmitterMutex.unlock();

    }

    void addToReceiver(Message msg)
    {
        receiverMutex.lock();
        receiverQueue.push(msg);
        receiverMutex.unlock();
    }

private:
    void receiverThreadFunc();
    void transmitterThreadFunc();
};

void Client::transmitterThreadFunc()
{
    size_t  length;
    uint8_t * data;
    bool isRunning = 1;

    while(isRunning)
    {
        //jeśli kolejka pusta, to zawieś się na mutexie empty
        transmitterEmpty.lock();
        //weź dostęp do kolejki
        transmitterMutex.lock();

        //skopiuj oczekujące wiadomości do tymczasowej kolejki
        std::queue < Message > tempQueue;
        while (!transmitterQueue.empty())
        {
            //włóż do tymczasowej pierwzy element oryginalnej
            tempQueue.push(transmitterQueue.front());
            //usuń z oryginalnej
            transmitterQueue.pop();
        } //dopóki coś jest w oryginalnej

        //zwolnij dostęp do oryginalnej kolejki
        transmitterMutex.unlock();

        //wyślij wszystkie wiadomości z tymczasowej kolejki
        while (!tempQueue.empty())
        {
            //weź pierwszą z kolejki
            Message msg = tempQueue.front();
            //usuń ją
            tempQueue.pop();

            //!!!PZETŁUMACZ NA data i length!!!

            ////isRunning = FLP_Write(connection, data, length);
        }
    }
    /*for (int i=0; i<100; ++i)
    {
        LOG(INFO) << "pracuje wątek transmitterThread" << this;
    }*/
}

void Client::receiverThreadFunc()
{
    size_t  length;
    uint8_t * data;
    bool isRunning = 1;

    while(isRunning)
    {
        //isRunning = FLP_Read(connection, &data, &length);
        Message newMessage = Message(data, length);
    }
    /*for (int i=0; i<100; ++i)
    {
        LOG(INFO) << "pracuje wątek reciverThread" << this;
    }*/
}

#endif //TTCHAT_CLIENT_H