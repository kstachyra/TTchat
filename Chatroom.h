#ifndef TTCHAT_CONVERSATION_H
#define TTCHAT_CONVERSATION_H

#include <list>

using namespace std;

class Chatroom
{
public:
    //identyfikator rozmowy
    int id;

private:
    //lista identyfikatorów klientów należących do rozmowy
    list < FLP_Connection_t* > clientList;

    std::queue < Message > chatroomQueue;
    std::mutex chatroomMutex;

    std::thread chatroomThread;
};

#endif //TTCHAT_CONVERSATION_H
