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
    std::mutex chatroomMutex;
    std::mutex chatroomEmpty;

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
};

void Chatroom::chatroomThreadFunc()
{
    std::cout << "\n" << "uruchomiono wątek chatroomu " << id;

    //tu przechowujemy informacje o tym, czy ostatnio lista byla pusta
    bool listEmpty;

    do
    {
        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        listMutex.lock();
        listEmpty = this->clientList.empty();
        listMutex.unlock();

        listMutex.lock();
        //dla każdego klienta w rozmowie
        for (auto it = clientList.begin(); it != clientList.end(); ++it)
        {
            //clientMonitor.clients[(*iterator)];
            //!!!TODO pobieraj przychdzace do swojej kolejki
            std::queue < Message > tempQueue;
            //clientMonitor.clients[(*it)]->getFromReceiver(tempQueue);
        }
        listMutex.unlock();

        //!!!TODO ogarniaj co trzeba zrobic ze wszystkimi wiaodmosciami aż wszystkie obsłużysz

        std::cout << "\n" << "watek czatroomu" << id << " pracuje sobie i ma klientow: " << clientList.size();

    } while(!listEmpty);
    std::cout << "\n" << "wątek chatroomu kończy pracę " << id;
}


#endif //TTCHAT_CONVERSATION_H
