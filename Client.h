#ifndef TTCHAT_CLIENT_H
#define TTCHAT_CLIENT_H
#include <sys/types.h>

class Client
{
public:
    int id;

    int socket;
    int terminateEventFD;

    int conversationId;

    //poniżej tymczasowe pola, na potrzeby protokołu
    pthread_t receiverThd;
    pthread_t transmitterThd;
    pthread_attr_t receiverThreadAttr;
    pthread_attr_t transmitterThreadAttr;
};


#endif //TTCHAT_CLIENT_H