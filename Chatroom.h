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
    /*Chatroom()
    {}*/

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
        std::cout << "Chatroom.joinThread: zjoinowano wątek chatroomu " << id <<"\n";
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
    std::cout << "chatroomThreadFunc: uruchomiono wątek chatroomu " << id <<"\n";

    //tu przechowujemy informacje o tym, czy ostatnio lista byla pusta
    bool toStop=0;

    while(1)
    {
        listMutex.lock();
        //dla każdego klienta w rozmowie
        std::cout<< "chatroomThreadFunc: chatroom " << id << " ma w swojej kolejce wiadomosci " << chatroomQueue.size() <<"\n";
        for (auto it = clientList.begin(); it != clientList.end(); ++it)
        {
            std::queue < SLPPacket > tempQueue;
            std::cout << "chatroomThreadFunc: ja chatroom " << id << " pobieram wiadomości z receiverQueue dla klienta " << (*it)->id <<"\n";
            (*it)->getFromReceiver(&tempQueue);
            //TODO dać empty na receiver queue
            sleep(3);
            std::cout << "chatroomThreadFunc: wywołano get Receiver po sleep(3)" << "\n";

            //dla wszystkich nowopobranych wiadomości
            while (!tempQueue.empty())
            {
                //włóż je do kolejki chatroomu
                chatroomQueue.push(tempQueue.front());
                //usuń z tymczasowej
                tempQueue.pop();
            }
        }
        std::cout<< "chatroomThreadFunc: watek czatroomu " << id << " pracuje sobie i ma klientow: " << clientList.size() <<"\n";
        //odblokuj listę, żeby w trakcie manageQueueMassages był do niej dostęp na dodawanie i odejmowanie klientów
        listMutex.unlock();

        manageQueueMessages();

        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        listMutex.lock();
        if (clientList.empty()) toStop = 1;
        listMutex.unlock();
        //żeby uniknąć nie odblokowania mutexa
        if (toStop) break;
    }
    std::cout<< "chatroomThreadFunc: wątek chatroomu kończy pracę " << id <<"\n";
}

void Chatroom::manageQueueMessages()
{
    //!!!TODO ogarniaj co trzeba zrobic ze wszystkimi wiaodmosciami aż wszystkie obsłużysz, można dać mapę na funkcję w różnych przypadkach
    SLPPacket msg;
    while (!chatroomQueue.empty())
    {
    	std::cout<< "manageQueueMessages: kolejka ma " << chatroomQueue.size() << "\n";
        msg = chatroomQueue.front();
        chatroomQueue.pop();

        std::cout<< "manageQueueMessages: w kolejce chatroomu mam wiadomość typu " << msg.getType() <<"\n";
        msg.print();
        std::cout<< "manageQueueMessages: wysyłam wiadomość do transmittera klienta SubAck" <<"\n";

        msg = SLPPacket(SLPPacket::SUBACK);
        clientList.front()->addToTransmitter(msg);
    }
}


#endif //TTCHAT_CONVERSATION_H
