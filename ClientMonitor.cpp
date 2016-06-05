#include "ClientMonitor.h"

ClientMonitor::ClientMonitor(void)
{

}

/*static ClientMonitor::ClientMonitor* getInstance(void)
{
		if(!instance) instance = new ClientMonitor;

		return instance;
}*/

void ClientMonitor::addClient(FLP_Connection_t * clientId, uint64_t chatroomId)
{
	enter();

	//tworz nowy obiekt klienta
	Client* newClient = new Client(clientId, chatroomId);
	clients[clientId] = newClient;

	//jeśli nie ma takiego chatroomu
	if(chatrooms.find(chatroomId) == chatrooms.end())
	{
		//to stwórz
		addChatroom(chatroomId);
		//dodajdo klienta do chatroomu
		chatrooms[chatroomId]->addClient(newClient);
		//i uruchom wątek chatroomu
		chatrooms[chatroomId]->runThread();
	}
	else //a jak jest
	{
		//to tylko dodaj klienta doń
		chatrooms[chatroomId]->addClient(newClient);
	}

	//uuchom wątki klienta
	newClient->runThreads();

	leave();
}

void ClientMonitor::removeClient(FLP_Connection_t * clientId)
{
	enter();
	//poinformuj klienta, że go zamykamy
	clients[clientId]->close();
	//jeśli klient nie istnieje w mapie
	if (clients.find(clientId) == clients.end())
	{
		std::cout << "ClientMonitor.removeClient: klient, którego chcemy usunąć nie istnieje w mapie" <<"\n";
	}
	else
	{
		//TODO joinować czy detachować? (wątki z powodu FLP_Close skończą pracę tak szybko jak to możliwe)
		//clients[clientId]->detachThreads();
		clients[clientId]->joinThreads();

		//usuń klienta z chatroomu
		chatrooms[clients[clientId]->chatroomId]->removeClient(clients[clientId]);

		//sprawdzamy, czy nie usunąć chatroomu (czy nie był to ostatni klient tego chatroomu)
		if (chatrooms[clients[clientId]->chatroomId]->isEmpty())
		{
			std::cout << "removeClient: BYŁ TO OSTATNI KLIENT, USUWAM CHATROOM " << clients[clientId]->chatroomId <<"\n";

			//skoro to był ostatni klient w chatroomie, to wątek chatroomu niebawem skończy pracę
			//TODO joinować czy detachować?
			//chatrooms[clients[clientId]->chatroomId]->joinThread();
			//removeChatroom(clients[clientId]->chatroomId);
			//TODO ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŹŹŹŹŹŹŹŹŹŹŹŹŹŹŁŁŁŁŁŁŁŁŁŁŁŁŁŁŁŁŁŁŁĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘĘ
		}

		//zwalniamy pamięć tego klienta
		delete clients[clientId];

		//usuwamy wpis klienta z mapy
		clients.erase(clientId);
	}

	leave();
}

void ClientMonitor::changeChatroomId(FLP_Connection_t *clientId, uint64_t newChatroomId)
{
	enter();
	//usuń ze starego chatroomu
	chatrooms[clients[clientId]->chatroomId]->removeClient(clients[clientId]);
	//zmień w kliencie informacje o chatroomie
	clients[clientId]->chatroomId = newChatroomId;

	//jeśli nie ma takiego chatroomu
	if(chatrooms.find(newChatroomId) == chatrooms.end())
	{
		//to stwórz
		addChatroom(newChatroomId);
		//dodajdo klienta do chatroomu
		chatrooms[newChatroomId]->addClient(clients[clientId]);
		//i uruchom wątek chatroomu
		chatrooms[newChatroomId]->runThread();
	}
	else //a jak jest
	{
		//to tylko dodaj klienta doń
		chatrooms[newChatroomId]->addClient(clients[clientId]);
	}

	leave();
}

void ClientMonitor::addChatroom(uint64_t chatroomId)
{
	Chatroom* newChatroom = new Chatroom(chatroomId);
	//wstaw do mapy chatroomów
	chatrooms[chatroomId]=newChatroom;
}

void ClientMonitor::removeChatroom(uint64_t chatroomId)
{
    std::cout << "removeChatroom: usuwam chatroom " << chatrooms[chatroomId]->id <<"\n";
    //zwalniamy pamięć tego chatroomu
    delete chatrooms[chatroomId];

    //usuwamy wpis chatroomu z mapy
    chatrooms.erase(chatroomId);
}


