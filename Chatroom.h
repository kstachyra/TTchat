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
    list < Client* > clientList;
    std::mutex listMutex;

    //kolejka wiadomości do obsłużenia przez wątek chatroomu
    std::queue < SLPPacket > chatroomQueue;

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
    }

    void joinThread()
    {
        if (chatroomThread.joinable()) chatroomThread.join();
        std::cout <<"\n"<< "zjoinowano wątek chatroomu " << id;
    }

    void addClient(Client* c)
    {
        listMutex.lock();
        clientList.push_back(c);
        listMutex.unlock();
    }

    void removeClient(Client* c)
    {
        listMutex.lock();
        clientList.remove(c);
        listMutex.unlock();
    }

    bool isEmpty()
    {
        bool toReturn = 0;
        listMutex.lock();
        if (clientList.empty()) toReturn = 1;
        listMutex.unlock();
        //żeby uniknąć nie odblokowania mutexa
        return toReturn;
    }

private:
    void chatroomThreadFunc();
    void manageQueueMessages();
};

void Chatroom::chatroomThreadFunc()
{
    std::cout << "\n" << "uruchomiono wątek chatroomu " << id;

    //tu przechowujemy informacje o tym, czy ostatnio lista byla pusta
    bool toStop=0;

    while(1)
    {
        listMutex.lock();
        //dla każdego klienta w rozmowie
        for (auto it = clientList.begin(); it != clientList.end(); ++it)
        {
            std::queue < SLPPacket > tempQueue;
            std::cout << "\n" << "ja chatroom " << id << " pobieram wiadomości z receiverQueue dla klienta " << (*it)->id;
            (*it)->getFromReceiver(&tempQueue);

            //dla wszystkich nowopobranych wiadomości
            while (!tempQueue.empty())
            {
                //włóż je do kolejki chatroomu
                chatroomQueue.push(tempQueue.front());
                //usuń z tymczasowej
                tempQueue.pop();
            }
        }
        std::cout << "\n" << "watek czatroomu " << id << " pracuje sobie i ma klientow: " << clientList.size();
        //odblokuj listę, żeby w trakcie manageQueueMassages był do niej dostęp na dodawanie i odejmowanie klientów
        listMutex.unlock();

        manageQueueMessages();

        std::cout<< "\n" << "chatroom " << id << " ma w swojej kolejce wiadomosci " << chatroomQueue.size();

        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        listMutex.lock();
        if (clientList.empty()) toStop = 1;
        listMutex.unlock();
        //żeby uniknąć nie odblokowania mutexa
        if (toStop) break;
    }
    std::cout << "\n" << "wątek chatroomu kończy pracę " << id;
}

void Chatroom::manageQueueMessages()
{
    sleep(1);
    //!!!TODO ogarniaj co trzeba zrobic ze wszystkimi wiaodmosciami aż wszystkie obsłużysz

    SLPPacket msg;
    while (!chatroomQueue.empty())
    {
        msg = chatroomQueue.front();
        chatroomQueue.pop();

        std::cout << "\n" << "w kolejce chatroomu mam wiadomość typu " << msg.getType();

        std::cout << "\n" << "wysyłam wiadomość do transmittera klienta SubAck";

        msg = SLPPacket(SLPPacket::SUBACK);
        clientList.front()->addToTransmitter(msg);
    }
}


#endif //TTCHAT_CONVERSATION_H
