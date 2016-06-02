#ifndef TTCHAT_CONVERSATION_H
#define TTCHAT_CONVERSATION_H

#include <list>
#include "ClientMonitor.h"

using namespace std;

class Chatroom
{
public:
    //identyfikator rozmowy
    uint64_t id;

private:
    //lista identyfikatorów klientów należących do rozmowy
    list < FLP_Connection_t* > clientList;
    std::mutex listMutex;

    //kolejka wiadomości do obsłużenia przez wątek chatroomu
    std::queue < Message > chatroomQueue;

    std::thread chatroomThread;

public:
    Chatroom()
    {}

    Chatroom(uint64_t id)
    {
        this->id = id;
    }

    void runThread()
    {
        chatroomThread = std::thread(&Chatroom::chatroomThreadFunc, this);
        std::cout <<"\n"<< "uruchomiono wątek chatroomu wątek chatroomu " << id;
    }

    void joinThread()
    {
        if (chatroomThread.joinable()) chatroomThread.join();
        std::cout <<"\n"<< "zjoinowano wątek chatroomu " << id;
    }

    void addClient(FLP_Connection_t* con)
    {
        listMutex.lock();
        clientList.push_back(con);
        listMutex.unlock();
    }

    void removeClient(FLP_Connection_t* con)
    {
        listMutex.lock();
        clientList.remove(con);
        listMutex.unlock();
    }

private:
    void chatroomThreadFunc();
    void manageQueueMassages();
};

void Chatroom::chatroomThreadFunc()
{
    std::cout << "\n" << "uruchomiono wątek chatroomu " << id;

    //tu przechowujemy informacje o tym, czy ostatnio lista byla pusta
    bool listEmpty;

    while(1)
    {
        listMutex.lock();
        //dla każdego klienta w rozmowie
        for (auto it = clientList.begin(); it != clientList.end(); ++it)
        {
            std::queue < Message > tempQueue;
            //!!!TODO AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA clieMonitor zrobić dostęp :S
            std::cout << "\n" << "pobieram wiadomości z receiverQueue dla klienta " << *it;
            /*clientMonitor.clients[(*it)]->getFromReceiver(&tempQueue);*/

            //dla wszystkich nowopobranych wiadomości
            while (!tempQueue.empty())
            {
                //włóż je do kolejki chatroomu
                chatroomQueue.push(tempQueue.front());
                //usuń z tymczasowej
                tempQueue.pop();
            }
        }
        std::cout << "\n" << "watek czatroomu" << id << " pracuje sobie i ma klientow: " << clientList.size();
        //odblokuj listę, żeby w trakcie manageQueueMassages był do niej dostęp na dodawanie i odejmowanie klientów
        listMutex.unlock();

        manageQueueMassages();

        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        listMutex.lock();
        if (clientList.empty()) break;
        listMutex.unlock();
    }
    std::cout << "\n" << "wątek chatroomu kończy pracę " << id;
}

void Chatroom::manageQueueMassages()
{
    sleep(1);
    //!!!TODO ogarniaj co trzeba zrobic ze wszystkimi wiaodmosciami aż wszystkie obsłużysz
}


#endif //TTCHAT_CONVERSATION_H
