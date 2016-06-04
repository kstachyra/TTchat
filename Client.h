#ifndef TTCHAT_CLIENT_H
#define TTCHAT_CLIENT_H
#include <sys/types.h>
#include "flp/flp.h"
#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include "MySemaphore.h"
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

    MySemaphore transmitterEmpty;
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

    Client(FLP_Connection_t * con, uint64_t chatId) : transmitterEmpty(0)
    {
        this->id = con;
        chatroomId = chatId;

        //początkowe wartości mutexów empty na zablokowane

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
    void addToTransmitter(SLPPacket msg)
    {
        //weź dostęp do kolejki
    	std::cout<<"przed transmiteter mutex" << "\n";
        transmitterMutex.lock();
    	std::cout<<"po transmiteter mutex" << "\n";

        //wrzuć wiadomość do kolejki

        //jeśli była pusta to unlock empty
        if (transmitterQueue.empty()) transmitterEmpty.notify();

        transmitterQueue.push(msg);
        //oddaj dostęp do kolejki
        transmitterMutex.unlock();

    	std::cout<<"koniec addtotransmitter" << "\n";

    }

    /*przypisuje wszystkie wiadomości z receiverQueue do wskazanej wskaźnikiem tempQueue*/
    void getFromReceiver(std::queue < SLPPacket >* tempQueue)
    {
        //weź dostęp do kolejki
        receiverMutex.lock();

        //skopiuj oczekujące wiadomości do tymczasowej kolejki
        while (!receiverQueue.empty())
        {
        	std::cout<<"RECEIVER QUEUE nie jest empty" << "\n";
            //włóż do tymczasowej pierwzy element oryginalnej
            tempQueue->push(receiverQueue.front());
            //usuń z oryginalnej
        	std::cout<< "RECEIVER QUEUE PRZED  POPOWANIEM MA: " << receiverQueue.size() <<"\n";

            receiverQueue.pop();
        	std::cout<< "RECEIVER QUEUE PO  POPOWANIEM MA: " << receiverQueue.size() <<"\n";

        } //dopóki coś jest w oryginalnej

        //zwolnij dostęp do oryginalnej kolejki
        receiverMutex.unlock();
    }

    void close()
    {
        FLP_Close(id);
        toClose = 1;
        transmitterEmpty.notify();
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
        transmitterEmpty.wait();
        //weź dostęp do kolejki
        transmitterMutex.lock();

        //skopiuj oczekujące wiadomości do tymczasowej kolejki
        std::queue < SLPPacket > tempQueue;
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
            //przypisz pierwszą z kolejki
            SLPPacket msg = tempQueue.front();
            //usuń ją
            tempQueue.pop();

            std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<"\n";
                        std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<"\n";
                        std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<"\n";

            msg.toDataBuffer(&data, &length);

            std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<"\n";
            std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<"\n";
            std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<"\n";

            isRunning = FLP_Write(id, data, length);

        }

        if (toClose) break;
    }

    sleep(5);
    std::cout<< "transmitterThreadFunc: wątek transmitter KOŃCZY PRACĘ dla klienta " << id <<"\n";
}

void Client::receiverThreadFunc()
{
    size_t  length;
    uint8_t * data;
    bool isRunning;
    SLPPacket msg;

    while (1)
    {
        //przeczytaj wiadomość i zapisz ją do msg
        isRunning = FLP_Read(id, &data, &length);
    	std::cout<< "FLP READ zwraca: " << isRunning <<"\n";


        if (isRunning) //jeśli odczytana poprawnie
        {
            //twórz obiekt wiadomości z bufora danych
            msg = SLPPacket(data, length);

            //zwolnij pamieć
            free (data);

            //weź dostęp do kolejki
            receiverMutex.lock();
            //wrzuć wiadomość do kolejki
        	sleep(3);
            receiverQueue.push(msg);

            //oddaj dostęp do kolejki
            receiverMutex.unlock();
        }
        else break; //isRunning == 0
    }


    sleep(5);
    std::cout<< "receiverThreadFunc: wątek receiver KOŃCZY PRACĘ dla klienta " << id <<"\n";
}

#endif //TTCHAT_CLIENT_H
