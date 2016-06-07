#include "Chatroom.h"
#include <unistd.h>
#include "global.h"

Chatroom::Chatroom(uint64_t id)
{
	this->id = id;
}

void Chatroom::runThread()
{
	chatroomThread = std::thread(&Chatroom::chatroomThreadFunc, this);
}

void Chatroom::joinThread()
{
	if (chatroomThread.joinable()) chatroomThread.join();
	std::cout << "Chatroom.joinThread: zjoinowano wątek chatroomu " << id <<"\n";
}

void Chatroom::detachThread()
{
	if (chatroomThread.joinable()) chatroomThread.detach();
}


void Chatroom::addClient(FLP_Connection_t* c)
{
	listMutex.lock();
	clientList.push_back(c);
	listMutex.unlock();
}

void Chatroom::removeClient(FLP_Connection_t* c)
{
	listMutex.lock();
	clientList.remove(c);
	listMutex.unlock();
}

bool Chatroom::isEmpty()
{
	bool toReturn = false;
	listMutex.lock();
	if (clientList.empty()) toReturn = true;
	listMutex.unlock();
	//żeby uniknąć nie odblokowania mutexa
	return toReturn;
}

void Chatroom::chatroomThreadFunc()
{
    std::cout << "chatroomThreadFunc: uruchomiono wątek chatroomu " << id <<"\n";

    //tu przechowujemy informacje o tym, czy ostatnio lista byla pusta
    bool toStop=0;

    while(1)
    {
        listMutex.lock();
        //dla każdego klienta w rozmowie
        //std::cout<< "chatroomThreadFunc: chatroom " << id << " ma w swojej kolejce wiadomosci " << chatroomQueue.size() <<"\n";
        for (auto it = clientList.begin(); it != clientList.end(); ++it)
        {
            std::queue < SLPPacket > tempQueue;
            //std::cout << "chatroomThreadFunc: chatroom " << id << " pobiera wiadomości z receiverQueue dla klienta " << (*it)->id <<"\n";


            clientMonitor.getFromReceiver((*it), &tempQueue);
            //clientMonitor.clients[(*it)]->getFromReceiver(&tempQueue);


            //TODO dać empty na receiver queue i wywalić wtedy sleepa
            //ale nie może się wątek chatroomu zablokować na jednym tylko z klientów - dać nowy wątek dla każdego klienta dla chatroomu?
            //dać jakieś sprawdzanie wspólne wszystkich semaforów
            //po sprawdzeniu wszystkich klientów chatroom może się zawiesić na swoim semaforze, a każdy z klientów może go obudzić -> NAJLEPSZY POMYSŁ CHYBA
            //sleep(1);

            //dla wszystkich nowopobranych wiadomości
            while (!tempQueue.empty())
            {
                //włóż je do kolejki chatroomu z informacją, od któ©ego klienta jest to wiadomość
                chatroomQueue.push( std::make_pair(tempQueue.front(), (*it)));
                //usuń z tymczasowej
                tempQueue.pop();
            }
        }
        //odblokuj listę, żeby w trakcie manageQueueMassages był do niej dostęp na dodawanie i odejmowanie klientów
        listMutex.unlock();

        manageQueueMessages();

        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        listMutex.lock();
        if (clientList.empty()) toStop = 1;
        //std::cout<< "chatroomThreadFunc: watek czatroomu " << id << " ma " << clientList.size() << " klientow" <<"\n";
        listMutex.unlock();
        //żeby uniknąć nie odblokowania mutexa

        //jeśli lista klientów była pusta, to kończymy pracę wątku chatroomu
        if (toStop) break;
    }
    std::cout<< "chatroomThreadFunc: wątek chatroomu kończy pracę " << id <<"\n";
}

void Chatroom::manageQueueMessages()
{
    SLPPacket msg;
    while (!chatroomQueue.empty())
    {
        msg = chatroomQueue.front().first;
        FLP_Connection_t* c = chatroomQueue.front().second;
        chatroomQueue.pop();

        std::cout<< "manageQueueMessages: obsługuję wiadomość typu enum " << msg.getType() <<"\n";
        msg.print();

        switch(msg.getType())
        {
        case SLPPacket::SUBREQ: SUBREQManage(&msg, c); break;
        case SLPPacket::UNSUB: UNSUBManage(&msg, c); break;
        case SLPPacket::GETINF: GETINFManage(&msg, c); break;
        case SLPPacket::PULLMSGS: PULLMSGSManage(&msg, c); break; //pobieranie z bazy danych i wysyłanie wszystkich do klienta
        case SLPPacket::MSGCLI: MSGCLIManage(&msg, c); break; //dodawanie do bazy danych
        default: std::cout<<"manageQueueMessages: nie wiem co zrobić z tym typem wiadomości" <<"\n";
        }
    }
}

void Chatroom::SUBREQManage(SLPPacket* msg, FLP_Connection_t* c)
{
	uint64_t newChatroomId = msg->getChatroomId();

	clientMonitor.changeChatroomId(c, newChatroomId);

	SLPPacket ans = SLPPacket(SLPPacket::SUBACK);
	ans.setChatroomId(newChatroomId);


	//albo sUBREF i usuń klienta

	clientMonitor.addToTransmitter(c, ans);
	//TODO nie kopiować ans tylko przekazać adresy

	//clientMonitor.clients[c]->addToTransmitter(ans);
}

void Chatroom::Chatroom::UNSUBManage(SLPPacket* msg, FLP_Connection_t* c)
{
	
}

/*
 * wysyła ChatroomInfo do klienta
 */
void Chatroom::GETINFManage(SLPPacket* msg, FLP_Connection_t* c)
{
	uint64_t newChatroomId = msg->getChatroomId();

	SLPPacket ans = SLPPacket(SLPPacket::ROOMINF);

	//TODO pobrać dane z bazy danych i wstawić
	ans.setChatroomId(newChatroomId);
	ans.setLastMessageID(20);
	ans.setNumberOFMessages(12);
	//

	clientMonitor.addToTransmitter(c, ans);
	//clientMonitor.clients[c]->addToTransmitter(ans);
}

/*
 * wysyła pożądane wiadomości do klienta
 */
void Chatroom::PULLMSGSManage(SLPPacket* msg, FLP_Connection_t* c)
{
	uint64_t chatroomId = msg->getChatroomId();
	int first = msg->getFirstMessageID();
	int last = msg->getLastMessageID();

	SLPPacket ans = SLPPacket(SLPPacket::MSGSER, 80);
	for (int i = last; i>=first; --i)
	{
		//TODO dobierz się z modelu tutaj
		ans.setChatroomId(chatroomId);
		ans.setMessageID(i);
		ans.setTime(555+i);
		ans.setNick("Kacper");
		ans.setMessageLength(80);
		ans.setMessage(std::to_string(i));

		std::cout<<"PULLMSGSManage: wysyłam";
		ans.print();
		std::cout<<"\n";

		clientMonitor.addToTransmitter(c, ans);
		//clientMonitor.clients[c]->addToTransmitter(ans);
	}
}

/*
 * odbiera wiadomość, wysyła ją do wszystkich w chatroomie i zapsuje do bazy
 */
void Chatroom::MSGCLIManage(SLPPacket* msg, FLP_Connection_t* c)
{
	SLPPacket ans = SLPPacket(SLPPacket::MSGSER, msg->getMessageLength());

	ans.setChatroomId(msg->getChatroomId());
	ans.setMessageID(1234);
	ans.setTime(5555);
	ans.setNick(msg->getNick());
	ans.setMessageLength(msg->getMessageLength());
	ans.setMessage(msg->getMessage());

	for (auto it = clientList.begin(); it!= clientList.end(); ++it)
	{
		clientMonitor.addToTransmitter((*it), ans);
		//clientMonitor.clients[(*it)]->addToTransmitter(ans);
	}

	//TODO dodaj do bazy
}
