#include "Client.h"

Client::Client(FLP_Connection_t* con, uint64_t chatId) : transmitterEmpty(0), receiverEmpty(0)
{
	this->id = con;
	chatroomId = chatId;
}

void Client::setChatroomId(uint64_t chatId)
{
	//używając tego, pamiętać o zmianie klienta w chatroomach!
	chatroomId = chatId;
}

void Client::runThreads()
{
	transmitterThread = std::thread(&Client::transmitterThreadFunc, this);
	receiverThread =  std::thread(&Client::receiverThreadFunc, this);
}

void Client::joinThreads()
{
	if (transmitterThread.joinable()) transmitterThread.join();
	if (receiverThread.joinable()) receiverThread.join();
}

void Client::detachThreads()
{
	transmitterThread.detach();
	receiverThread.detach();
}

/*wpisuje do kolejki transmittera daną (jedną) wiadomość*/
void Client::addToTransmitter(SLPPacket msg)
{
	//weź dostęp do kolejki
	transmitterMutex.lock();

	//jeśli była pusta to unlock empty
	if (transmitterQueue.empty()) transmitterEmpty.notify();

	//wrzuć wiadomość do kolejki
	transmitterQueue.push(msg);
	//oddaj dostęp do kolejki
	transmitterMutex.unlock();
}

/*przypisuje wszystkie wiadomości z receiverQueue do wskazanej wskaźnikiem tempQueue*/
void Client::getFromReceiver(std::queue < SLPPacket >* tempQueue)
{
	//weź dostęp do kolejki
	receiverMutex.lock();

	//skopiuj oczekujące wiadomości do tymczasowej kolejki
	while (!receiverQueue.empty())
	{
		//włóż do tymczasowej pierwzy element oryginalnej
		tempQueue->push(receiverQueue.front());
		//usuń z oryginalnej
		receiverQueue.pop();
	} //dopóki coś jest w oryginalnej

	//zwolnij dostęp do oryginalnej kolejki
	receiverMutex.unlock();
}

void Client::close()
{
	FLP_Close(id);
	toClose = 1;
	transmitterEmpty.notify();
}

void Client::transmitterThreadFunc()
{
    size_t  length;
    uint8_t * data;
    bool isRunning = 1;

    while(isRunning)
    {
        //jeśli kolejka pusta, to zawieś się na semaforze empty
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

            msg.toDataBuffer(&data, &length);

            isRunning = FLP_Write(id, data, length);

        	delete data;

        }
        if (toClose) break;
    }
    std::cout<< "transmitterThreadFunc: wątek transmitter kończy pracę dla klienta " << id <<"\n";
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

        if (isRunning) //jeśli odczytana poprawnie
        {
            //twórz obiekt wiadomości z bufora danych
            msg = SLPPacket(data, length);

            //zwolnij pamieć
            free (data);

            //weź dostęp do kolejki
            receiverMutex.lock();
            //wrzuć wiadomość do kolejki
            receiverQueue.push(msg);
            //oddaj dostęp do kolejki
            receiverMutex.unlock();
        }
        else break; //isRunning == 0
    }
    close();
    std::cout<< "receiverThreadFunc: wątek receiver KOŃCZY PRACĘ dla klienta " << id <<"\n";
}
