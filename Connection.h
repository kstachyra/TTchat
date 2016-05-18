//
// Created by ks on 18.05.16.
//

#ifndef TTCHAT_CONNECTION_H
#define TTCHAT_CONNECTION_H


#include "flp/flp.h"
#include "Message.h"

class Connection
{
    FLP_Connection_t id;

    int socket;

    void connect(int i)
    {
        this->socket = socket;
        FLP_Connect(socket);
    }

    void disconnect()
    {

    }


public:

    /*
     * konstruktor nowego połączenia, jako parametr potrzebuje nr portu
     */
    Connection(int socket)
    {
        //connect(socket);                                                             @TODO connect
    }

    /*
     * funkcja zwracająca przychodzącą wiadomość dla tego połączenia
     */
    Message getMessage()
    {
        return Message();
    }


    /*
     * funkcja wysyłająca wiadomość do tego połączenia
     */
    bool sendMessage(Message m)
    {
        return false;
    }

};


#endif //TTCHAT_CONNECTION_H
