#ifndef TTCHAT_CLIENT_H
#define TTCHAT_CLIENT_H
#include <sys/types.h>
#include "flp/flp.h"
#include "Message.h"
#include <thread>
#include <queue>
#include <mutex>
#include <iostream>


class Client
{
public:
    FLP_Connection_t *id;
    uint64_t chatroomId;

private:
    std::queue < Message > transmitterQueue;
    std::queue < Message > receiverQueue;

    std::mutex transmitterMutex;
    std::mutex receiverMutex;

    std::mutex transmitterEmpty;
    //std::mutex receiverEmpty; -> odkomentować w konstruktorach w razie potrzeby

    std::thread transmitterThread;
    std::thread receiverThread;

    //czy wątki klienta do zamknięcia
    bool toClose = 0;
public:
    /*Client()
    {
        id = NULL;
        chatroomId = 0xFFFFFFFF;

        //początkowe wartości mutexów empty na zablokowane
        transmitterEmpty.lock();
        //receiverEmpty.lock();
    }

    Client(FLP_Connection_t * con)
    {
        this->id = con;
        chatroomId = 0xFFFFFFFF;

        //początkowe wartości mutexów empty na zablokowane
        transmitterEmpty.lock();
        //receiverEmpty.lock();
    }*/

    Client(FLP_Connection_t * con, uint64_t chatId)
    {
        this->id = con;
        chatroomId = chatId;

        //początkowe wartości mutexów empty na zablokowane
        transmitterEmpty.lock();
        //receiverEmpty.lock();
    }

    void setChatroomId(uint64_t chatId)
    {
        //używając tego, pamiętać o zmianie klienta w chatroomach!
        chatroomId = chatId;
    }

    void runThreads()
    {
        transmitterThread = std::thread(&Client::transmitterThreadFunc, this);
        receiverThread =  std::thread(&Client::receiverThreadFunc, this);
    }

    void joinThreads()
    {
        if (transmitterThread.joinable()) transmitterThread.join();
        if (receiverThread.joinable()) receiverThread.join();
    }

    void detachThreads()
    {
        transmitterThread.detach();
        receiverThread.detach();
    }

    /*wpisuje do kolejki transmittera daną (jedną) wiadomość*/
    void addToTransmitter(Message msg)
    {
        //weź dostęp do kolejki
        transmitterMutex.lock();
        //wrzuć wiadomość do kolejki

        //TODO jeśli była pusta to empty.unlock()

        transmitterQueue.push(msg);
        //oddaj dostęp do kolejki
        transmitterMutex.unlock();
    }

    /*przypisuje wszystkie wiadomości z receiverQueue do wskazanej wskaźnikiem tempQueue*/
    void getFromReceiver(std::queue < Message >* tempQueue)
    {
        //weź dostęp do kolejki
        receiverMutex.lock();

        //skopiuj oczekujące wiadomości do tymczasowej kolejki
        while (!receiverQueue.empty())
        {
            //włóż do tymczasowej pierwzy element oryginalnej
            tempQueue->push(receiverQueue.front());
            //usuń z oryginalnej
            receiverQueue.pop();
        } //dopóki coś jest w oryginalnej

        //zwolnij dostęp do oryginalnej kolejki
        receiverMutex.unlock();
    }

    void close()
    {
        FLP_Close(id);
        toClose = 1;
        //if (!transmitterEmpty.try_lock());
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
        //std::cout<<"DSADSADASDASDASDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaSDAS";
        //TODO ZAWIESZA SIE NA EMPTY! jak odiwiesić gdy chcemy skończyć? :S unlock unlocked?
        //jeśli kolejka pusta, to zawieś się na mutexie empty
        //transmitterEmpty.lock();
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

        //powiedz, że na pewno nic nie będzie w kolejce, jeśli ktoś zdążył coś dodać (trylock) - może zwrócić false i nie zablokować, więc po wejściu do sekcji krytycznej i tak sprawdzamy czy pusta
        transmitterEmpty.try_lock();

        //zwolnij dostęp do oryginalnej kolejki
        transmitterMutex.unlock();

        //wyślij wszystkie wiadomości z tymczasowej kolejki
        while (!tempQueue.empty())
        {
            //przypisz pierwszą z kolejki
            Message msg = tempQueue.front();
            //usuń ją
            tempQueue.pop();

            msg.toDataBuffer(data, length);

            isRunning = FLP_Write(id, data, length);
        }

        if (toClose) break;
    }

    sleep(5);
    std::cout <<"\n"<< "wątek transmitter KOŃCZY PRACĘ dla klienta " << id;
}

void Client::receiverThreadFunc()
{
    size_t  length;
    uint8_t * data;
    bool isRunning = 1;
    Message msg;

    //TODO isRunning FLP zwraca 0 i 1 kiedy finalnie oryginalnie jak ma byc? Isrunning czy isStoped?

    while (1)
    {
        //przeczytaj wiadomość i zapisz ją do msg
        isRunning = FLP_Read(id, &data, &length);
        if (isRunning) //jeśli odczytana poprawnie
        {
            msg = Message(data, length);

            //weź dostęp do kolejki
            receiverMutex.lock();
            //wrzuć wiadomość do kolejki
            receiverQueue.push(msg);
            //oddaj dostęp do kolejki
            receiverMutex.unlock();
        }
        else break; //isRunning == 0
    }


    sleep(5);
    std::cout <<"\n"<< "wątek receiver KOŃCZY PRACĘ dla klienta " << id;
}

#endif //TTCHAT_CLIENT_H