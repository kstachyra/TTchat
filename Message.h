//
// Created by ks on 18.05.16.
//

#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

#include <vector>

class Message
{
public:
    enum messageType {SUBREQ, SUBACK, SUBREF, UNSUB, GETINF, ROOMINF, PULLMSGS, MSG};
    char length;
    std::vector < char > payload;

public:
    Message(messageType type)
    {
        if (type == SUBREQ || type == SUBACK || type == UNSUB)
        {
            length = 4;
        }
    }

    Message(messageType type, char length)
    {

    }

};


#endif //TTCHAT_MESSAGE_H
