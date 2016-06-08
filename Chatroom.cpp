#include "Chatroom.h"
#include <unistd.h>
#include "global.h"
#include "Model/Model.h"
#include <string>

#include <chrono>
#include <time.h>

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
	waitingListLock.lock();
	waitingList.push(c);
	waitingListLock.unlock();
}

void Chatroom::removeClient(FLP_Connection_t* c)
{
	clientListLock.lock();
	clientList.remove(c);
	clientListLock.unlock();
}

/*
 * usuwa klienta z listy chatroomu NIE BLOKUJĄC listy, do użytku tylko gdy lista jest zablokowana
 */
void Chatroom::forceRemoveClient(FLP_Connection_t* c)
{
	std::cout << "chatroomThreadFunc: clientList size:" << clientList.size()  << endl;
	std::cout << "Chatroom::forceRemoveClient: Removing " << c << endl;
	clientList.remove(c);
	std::cout << "chatroomThreadFunc: clientList size:" << clientList.size()  << endl;
}

bool Chatroom::isEmpty()
{
	bool toReturn = false;
	clientListLock.lock();
	if (clientList.empty()) toReturn = true;
	clientListLock.unlock();
	//żeby uniknąć nie odblokowania mutexa
	return toReturn;
}

void Chatroom::chatroomThreadFunc()
{
    std::cout << "chatroomThreadFunc: uruchomiono wątek chatroomu " << id <<"\n";

    //tu przechowujemy informacje o tym, czy ostatnio lista byla pusta
    bool toStop=0;
    bool clientRemoved = false;

    while(1)
    {
    	clientRemoved = false;

    	 if(clientRemoved) std::cout << "chatroomThreadFunc: Next round..." << endl;

    	clientListLock.lock();
        //std::cout<< "chatroomThreadFunc: chatroom " << id << " ma w swojej kolejce wiadomosci " << chatroomQueue.size() <<"\n";

        if(clientRemoved) {
        	std::cout << "chatroomThreadFunc: clientList size:" << clientList.size()  << endl;
        	if(clientList.begin() == clientList.end()) {
        		std::cout << "chatroomThreadFunc: clientList.begin() == clientList.end()" << endl;
        	} else {
        		std::cout << "chatroomThreadFunc: clientList.begin() != clientList.end()" << endl;
        	}
        }

        //dla każdego klienta w rozmowie
        for (auto it = clientList.begin(); it != clientList.end(); ++it)
        {
        	if(clientRemoved) std::cout << "chatroomThreadFunc: Checking next client (after removing another)."  << endl;

        	//sprawdź czy klient jest aktywny
        	if (!clientMonitor.isClientActive(*it))
        	{
            	std::cout<< "chatroomThreadFunc: Znalazłem nieaktywnego klienta, usuwam go." <<"\n";
        		//jeśli nie, to usuń (będąc wewnątrz listy Chatroomu (parametr true)
            	FLP_Connection_t *toRemove = *it;
            	it++;
           		clientMonitor.removeClient(toRemove, true); //TODO jeśli nie będzie aktywnego oczekiwania, to może się cos zjebać, toClose powinno wymusić sprawdzenie przez chatroom aktywności klienta (jakiś nowy mutex? :/)
           		std::cout << "chatroomThreadFunc: Client removed." << endl;
           		clientRemoved = true;
        	}
        	else
        	{
        		//pobierz jego wiadomości
        		if(clientRemoved) std::cout << "chatroomThreadFunc: Client is active."  << endl;
				std::queue < SLPPacket > tempQueue;
				//std::cout << "chatroomThreadFunc: chatroom " << id << " pobiera wiadomości z receiverQueue dla klienta " << (*it)->id <<"\n";

				if(clientRemoved) std::cout << "chatroomThreadFunc: Reading from receiver queue..."  << endl;
				clientMonitor.getFromReceiver((*it), &tempQueue);
				//clientMonitor.clients[(*it)]->getFromReceiver(&tempQueue);

				//TODO dać empty na receiver queue i wywalić wtedy sleepa
				//ale nie może się wątek chatroomu zablokować na jednym tylko z klientów - dać nowy wątek dla każdego klienta dla chatroomu?
				//dać jakieś sprawdzanie wspólne wszystkich semaforów
				//po sprawdzeniu wszystkich klientów chatroom może się zawiesić na swoim semaforze, a każdy z klientów może go obudzić -> NAJLEPSZY POMYSŁ CHYBA
				//sleep(1);

				//dla wszystkich nowopobranych wiadomości
				if(clientRemoved) std::cout << "chatroomThreadFunc: Pushing messages to chatroom queue..."  << endl;
				while (!tempQueue.empty())
				{
					//włóż je do kolejki chatroomu z informacją, od któ©ego klienta jest to wiadomość
					chatroomQueue.push( std::make_pair(tempQueue.front(), (*it)));
					//usuń z tymczasowej
					tempQueue.pop();
				}
        	}
        }

        if(clientRemoved) std::cout << "chatroomThreadFunc: All clients checked." << endl;

        // Add waiting clients
        waitingListLock.lock();
        for(unsigned i=0; i<waitingList.size(); i++) {
        	clientList.push_back(waitingList.front());
        	waitingList.pop();
        }
        waitingListLock.unlock();

        //odblokuj listę, żeby w trakcie manageQueueMassages był do niej dostęp na dodawanie i odejmowanie klientów
        clientListLock.unlock();


        if(clientRemoved) std::cout << "chatroomThreadFunc: Calling manageQueueMessages..." << endl;
        manageQueueMessages();


        //zapisujemy tu informacje o ostatnim stanie pustosci listy
        clientListLock.lock();
        if(clientRemoved) std::cout << "chatroomThreadFunc: Checking if client list is empty..." << endl;
        if (clientList.empty()) toStop = 1;
        //std::cout<< "chatroomThreadFunc: watek czatroomu " << id << " ma " << clientList.size() << " klientow" <<"\n";
        clientListLock.unlock();
        //żeby uniknąć nie odblokowania mutexa

        //TODO co jeśli tutaj dodamy klienta?

        //jeśli lista klientów była pusta, to kończymy pracę wątku chatroomu
        //if (toStop) break; //TODO OGARNAĆ KONIEC CHATROOMU
        if(clientRemoved) std::cout << "chatroomThreadFunc: Done." << endl;
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
        std::cout<< "manageQueueMessages: dla klienta: "<< c <<"\n";

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

	std::cout<<"SUBREWManage: Changing chatroomId" << endl;

	clientMonitor.changeChatroomId(c, newChatroomId);

	std::cout<<"SUBREWManage: Sending SUBACK." << endl;
	SLPPacket ans = SLPPacket(SLPPacket::SUBACK);
	ans.setChatroomId(newChatroomId);

	//albo sUBREF i usuń klienta

	clientMonitor.addToTransmitter(c, ans);
	//TODO nie kopiować ans tylko przekazać adresy
}

void Chatroom::Chatroom::UNSUBManage(SLPPacket* msg, FLP_Connection_t* c)
{
	//clientMonitor.removeClient(c, false);
}

/*
 * wysyła ChatroomInfo do klienta
 */
void Chatroom::GETINFManage(SLPPacket* msg, FLP_Connection_t* c)
{
	uint32_t chatroomId = msg->getChatroomId();

	SLPPacket ans = SLPPacket(SLPPacket::ROOMINF);


	bool gogel;
	model.doesChatRoomExist(chatroomId, &gogel);
	if (!gogel) //czatroom nie istnieje
	{
		//twórz chatroom w bazie danych
		model.newChatRoom(chatroomId);
	}

	uint32_t lastId, numMsgs;

	model.getNumOfMessages(chatroomId, &numMsgs);
	model.getLastMessageId(chatroomId, &lastId);

	ans.setChatroomId(chatroomId);
	ans.setLastMessageID(lastId);
	ans.setNumberOFMessages(numMsgs);

	clientMonitor.addToTransmitter(c, ans);
	//clientMonitor.clients[c]->addToTransmitter(ans);
}

/*
 * wysyła pożądane wiadomości do klienta
 */
void Chatroom::PULLMSGSManage(SLPPacket* pck, FLP_Connection_t* c)
{
	uint64_t chatroomId = pck->getChatroomId();
	uint32_t first = pck->getFirstMessageID();
	uint32_t last = pck->getLastMessageID();

	SLPPacket ans = SLPPacket(SLPPacket::MSGSER, 80);
	for (uint32_t i = last; i>=first; --i)
	{
		Message msg;
		model.getMessage(chatroomId, i, &msg);

		ans.setChatroomId(chatroomId);
		ans.setMessageID(i);
		ans.setTime(msg.timestamp);
		ans.setNick(msg.nick);
		ans.setMessageLength(msg.payloadLength);

		ans.setMessage(msg.payload, msg.payloadLength);

		std::cout<<"PULLMSGSManage: wysyłam" <<"\n";
		ans.print();

		clientMonitor.addToTransmitter(c, ans);
		//clientMonitor.clients[c]->addToTransmitter(ans);
	}
}

/*
 * odbiera wiadomość, wysyła ją do wszystkich w chatroomie i zapsuje do bazy
 */
void Chatroom::MSGCLIManage(SLPPacket* pck, FLP_Connection_t* c)
{
	SLPPacket ans = SLPPacket(SLPPacket::MSGSER, pck->getMessageLength());

	uint64_t chatroomId = pck->getChatroomId();
	uint64_t payloadLength = pck->getMessageLength();

	//ustalam id dla tej wiadomości
	uint32_t nextId;
	model.getNextMessageId(chatroomId, &nextId);

	uint32_t systemTime;
	systemTime = time(NULL);

	ans.setChatroomId(pck->getChatroomId());
	ans.setMessageID(nextId);
	ans.setTime(systemTime);
	ans.setNick(pck->getNick());
	ans.setMessageLength(pck->getMessageLength());
	ans.setMessage(pck->getMessage());
	
	Message msg;
	msg.id = nextId;
	msg.timestamp = systemTime;

	pck->getNick(msg.nick);

	uint8_t msgBuf[payloadLength]; //DAĆ DEFINA
	pck->getMessage(msgBuf, payloadLength);
	msg.payload = msgBuf;

	msg.payload = msgBuf;

	msg.payloadLength = ans.getMessageLength();

	if (model.newMessage(chatroomId, msg)) //dodaję wiadomość do bazy
	{
		for (auto it = clientList.begin(); it!= clientList.end(); ++it)
		{
			std::cout<<"MSGCLIManage: wysyłam" <<"\n";
			ans.print();
			clientMonitor.addToTransmitter((*it), ans);
			//clientMonitor.clients[(*it)]->addToTransmitter(ans);
		}
	}
}
