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
    //lista klientów należących do rozmowy wraz z mutexem
    list < Client* > clientList;
    std::mutex listMutex;

    //kolejka wiadomości do obsłużenia przez wątek chatroomu
    std::queue < SLPPacket > chatroomQueue;

    //wątek chatroomu
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
        bool toReturn = false;
        listMutex.lock();
        if (clientList.empty()) toReturn = true;
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
            std::cout << "chatroomThreadFunc: chatroom " << id << " pobiera wiadomości z receiverQueue dla klienta " << (*it)->id <<"\n";
            (*it)->getFromReceiver(&tempQueue);
            //TODO dać empty na receiver queue i wywalić wtedy sleepa
            //ale nie może się wątek chatroomu zablokować na jednym tylko z klientów - dać nowy wątek dla każdego klienta dla chatroomu?
            //dać jakieś sprawdzanie wspólne wszystkich semaforów
            //po sprawdzeniu wszystkich klientów chatroom może się zawiesić na swoim semaforze, a każdy z klientów może go obudzić -> NAJLEPSZY POMYSŁ CHYBA
            sleep(5);

            //dla wszystkich nowopobranych wiadomości
            while (!tempQueue.empty())
            {
                //włóż je do kolejki chatroomu
                chatroomQueue.push(tempQueue.front());
                //usuń z tymczasowej
                tempQueue.pop();
            }
        }
        std::cout<< "chatroomThreadFunc: watek czatroomu " << id << " ppobrał wiadomości dla " << clientList.size() << " klientow" <<"\n";
        //odblokuj listę, żeby w trakcie manageQueueMassages był do niej dostęp na dodawanie i odejmowanie klientów
        listMutex.unlock();

        manageQueueMessages();

        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        listMutex.lock();
        if (clientList.empty()) toStop = 1;
        listMutex.unlock();
        //żeby uniknąć nie odblokowania mutexa
        //TODO co jeśli w tej chwili dodamy klienta do chatroomu?
        //jeśli lista klientów była pusta, to kończymy pracę wątku chatroomu
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
        msg = chatroomQueue.front();
        chatroomQueue.pop();

        std::cout<< "manageQueueMessages: obsługuję wiadomość typu enum " << msg.getType() <<"\n";
        msg.print();

        switch(msg.getType())
        {
        case SLPPacket::SUBREQ:
        {
        	//client->change chatroom id
        	//send SUBACK
        	//albo sUBREF i usuń klienta

        	//clientList.front()->addToTransmitter(msg);
        	break;
        }
        case SLPPacket::UNSUB:
        case SLPPacket::GETINF:
        case SLPPacket::PULLMSGS: //pobieranie z bazy danych i wysyłanie wszystkich do klienta
        case SLPPacket::MSGCLI: //dodawanie do bazy danych
        default: std::cout<<"manageQueueMessages: nie wiem co zrobić z tym typem wiadomości" <<"\n";
        }
    }
}


#endif //TTCHAT_CONVERSATION_H
