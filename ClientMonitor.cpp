#include "ClientMonitor.h"

ClientMonitor::ClientMonitor(void)
{

}

void ClientMonitor::addClient(FLP_Connection_t * clientId, uint64_t chatroomId)
{
	enter();

	//tworz nowy obiekt klienta
	Client* newClient = new Client(clientId, chatroomId);
	//wstaw do mapy klientów
	clients[clientId] = newClient;

	//jeśli nie ma takiego chatroomu
	if(chatrooms.find(chatroomId) == chatrooms.end())
	{
		std::cout<<"ClientMonitor.addClient: nie ma takiego chatroomu" << "\n";
		//to stwórz
		addChatroom(chatroomId);
		//dodajdo klienta do chatroomu
		chatrooms[chatroomId]->addClient(clientId);
		//i uruchom wątek chatroomu
		chatrooms[chatroomId]->runThread();
	}
	else //a jak jest
	{
		std::cout<<"ClientMonitor.addClient: jest taki chatroom" << "\n";
		//to tylko dodaj klienta doń
		chatrooms[chatroomId]->addClient(clientId);
	}

	//uuchom wątki klienta
	newClient->runThreads();

	leave();
}

void ClientMonitor::removeClient(FLP_Connection_t * clientId, bool noMutex)
{
	std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa \n";

	enter();
	std::cout<<"BBBBBBBBBBBBBBBBBBBBBBBBBBBBB \n";

	//poinformuj klienta, że go zamykamy
	clients[clientId]->close();

	//jeśli klient nie istnieje w mapie
	if (clients.find(clientId) == clients.end())
	{
		std::cout << "ClientMonitor.removeClient: klient, którego chcemy usunąć nie istnieje w mapie" <<"\n";
	}
	else
	{
		std::cout<<"CCCCCCCCCCCCCCCCCCCCCCCCCCCC \n";

		clients[clientId]->joinThreads();

		std::cout<<"DDDDDDDDDDDDDDDDDDDDDD \n";


		//usuń klienta z chatroomu
		if (noMutex) chatrooms[clients[clientId]->chatroomId]->forceRemoveClient(clientId);
		else chatrooms[clients[clientId]->chatroomId]->removeClient(clientId);

		std::cout<<"EEEEEEEEEEEEEEEEEEEEEE \n";


		//sprawdzamy, czy nie usunąć chatroomu (czy nie był to ostatni klient tego chatroomu)
		/*if (chatrooms[clients[clientId]->chatroomId]->isEmpty())
		{
			std::cout << "removeClient: BYŁ TO OSTATNI KLIENT, USUWAM CHATROOM " << clients[clientId]->chatroomId <<"\n";

			//skoro to był ostatni klient w chatroomie, to wątek chatroomu niebawem skończy pracę
			//TODO joinować czy detachować?

			chatrooms[clients[clientId]->chatroomId]->joinThread();

			removeChatroom(clients[clientId]->chatroomId);
			//TODO usuwać chatroomy a może i nie usuwać chatroomów
		}*/

		std::cout<<"FFFFFFFFFFFFFFFFFFFFFFFFFFFFF \n";


		std::cout<<"ClientMonitor.RemoveClient: usuwam DELETEDELETE clienta " << clientId <<"\n";
		//zwalniamy pamięć tego klienta
		//delete clients[clientId];

		sleep(2);
		std::cout<<"GGGGGGGGGGGGGGGGGGGGGGGGGGGGG \n";
		//usuwamy wpis klienta z mapy
		//clients.erase(clientId);

		std::cout<<"HHHHHHHHHHHHHHHHHHHHHHHHHHHHH \n";
		sleep(2);
		std::cout<<"IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII \n";

	}

	leave();
}

void ClientMonitor::changeChatroomId(FLP_Connection_t *clientId, uint64_t newChatroomId)
{
	enter();
	//usuń ze starego chatroomu
	chatrooms[clients[clientId]->chatroomId]->removeClient(clientId);
	//zmień w kliencie informacje o chatroomie
	clients[clientId]->chatroomId = newChatroomId;

	//jeśli nie ma takiego chatroomu
	if(chatrooms.find(newChatroomId) == chatrooms.end())
	{
		//to stwórz
		addChatroom(newChatroomId);
		//dodajdo klienta do chatroomu
		chatrooms[newChatroomId]->addClient(clientId);
		//i uruchom wątek chatroomu
		chatrooms[newChatroomId]->runThread();
	}
	else //a jak jest
	{
		//to tylko dodaj klienta doń
		chatrooms[newChatroomId]->addClient(clientId);
	}

	leave();
}

void ClientMonitor::addChatroom(uint64_t chatroomId)
{
	Chatroom* newChatroom = new Chatroom(chatroomId);
	//wstaw do mapy chatroomów
	chatrooms[chatroomId]=newChatroom;
}

void ClientMonitor::addToTransmitter(FLP_Connection_t* c, SLPPacket ans)
{
	enter();
	//sprawdza czy klient jeszcze istnieje

	//jeśli nie istnieje
	if (clients.find(c)==clients.end())
	{
		std::cout<<"ClientMonitor.addToTransmitter: klient nie istnieje, ignoruję dodawanie wiadomości" << "\n";
	}
	//a jak istnieje
	else
	{
		clients[c]->addToTransmitter(ans);
	}

	leave();
}

void ClientMonitor::getFromReceiver(FLP_Connection_t* c, std::queue < SLPPacket >* tempQueue)
{
	enter();
	//sprawdza czy klient jeszcze istnieje

	//jeśli nie istnieje
	if (clients.find(c)==clients.end())
	{
		std::cout<<"ClientMonitor.getFromReceiver: klient nie istnieje, ignoruję pobieranie wiadomości" << "\n";
	}
	//a jak istnieje
	else
	{
		clients[c]->getFromReceiver(tempQueue);
	}

	leave();
}

bool ClientMonitor::isClientActive(FLP_Connection_t* c)
{
	enter();
	//jeśli nie jest aktywny
	if (clients[c]->toClose)
	{
		leave();
		return false;
	}

	leave();
	return true;
}

void ClientMonitor::removeChatroom(uint64_t chatroomId)
{
    std::cout << "removeChatroom: usuwam chatroom DELETEDELETE " << chatrooms[chatroomId]->id <<"\n";
    //zwalniamy pamięć tego chatroomu
    delete chatrooms[chatroomId];

    //usuwamy wpis chatroomu z mapy
    chatrooms.erase(chatroomId);
}


