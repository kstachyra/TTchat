//
// Created by ks on 18.05.16.
//

#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

#include <vector>

class Message
{
public:
    enum messageType {SUBREQ, SUBACK, SUBREF, UNSUB, GETINF, ROOMINF, PULLMSGS, MSGSER, MSGCLI, ERR};

private:
    messageType type;

    /* długość wiadomości w bajtach (nie liczac typu)*/
    int vecLength;
    std::vector < uint8_t > vec;

public:
    /*
     * konstruktor nieznanej wiadomości (do odbierania)
     */
    Message()
    {
        this->type = ERR;
    }

    /*
     * konstruktor dla typów o stałej długości
     */
    Message(messageType type)
    {
        this->type = type;
        if (type == MSGSER || type ==MSGCLI)
        {
            LOG(INFO) << "nie podano długości dla Message o zmiennej długości!";
            this->type = ERR;
        }
        if (type == SUBREQ || type == SUBACK || type == UNSUB || type == GETINF)
        {
            vecLength = 4;
        }
        else if (type == SUBREF)
        {
            vecLength = 8;
        }
        else if (type == ROOMINF || type == PULLMSGS)
        {
            vecLength = 12;
        }
        vec.resize(vecLength);
    }

    /*
     * konstruktor dla typów o zmiennej długości
     */
    Message(messageType type, int length)
    {
        this->type = type;
        if (type == MSGSER)
        {
            vecLength = 46 + length;
        }
        else if  (type == MSGCLI)
        {
            vecLength = 38 + length;
        }
        else
        {
            LOG(INFO) << "błędny typ pakietu (podana długość dla pakietu o stałej długości)";
            this->type = ERR;
        }
        vec.resize(vecLength);
    }

    int getRoomID()
    {
        if (type == ERR) return -1;
        return toInt(0, 3);
    }

    void setRoomID(int id)
    {
        if (type == ERR)
        {
            LOG(INFO) << "nie można ustawić RoomID dla Message typu ERR";
        }
        else
        {
            toVec(id, 0, 3);
        }
    }

    int getReason()
    {
        if (type == SUBREF)
        {
            return toInt(4, 7);
        }
        LOG(INFO) << "błędny typ Message dla getReason"
        return -1;
    }

    int setReason(int reason)
    {
        if (type == SUBREF)
        {
            toVec(reason, 4, 7);
        }
        LOG(INFO) << "błędny typ Message dla setReason"
    }

private:
    /*
     * konwertuje zakres bajtów z vectora to inta i go zwraca
     * (WAŻNE - int to 4 bajty, brak kontroli tego)
     */
    int toInt(int first, int last)
    {
        int Int32 = 0;
        for (int i = last; i >= first; --i)
        {
            Int32 = (Int32 << 8) + vec[i];
        }
        return Int32;
    }

    /*
     * konwertuje inta na bajty i zapisuje je w podany zakres
     * (WAŻNE - int to 4 bajty, brak kontroli tego)
     */
    void toVec(int value, int first, int last)
    {
        for (int i=first; i<=last; ++i)
        {
            vec[i] = (uint8_t) ((value >> 8*(i-first)) & 0xFF);
        }
    }
};


#endif //TTCHAT_MESSAGE_H
