//
// Created by ks on 18.05.16.
//

#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

#include <vector>
#include <string>

using namespace std;

class Message
{
public:
    //typ ERR oznacza stworzony obiekt Message bez innego określonego typu
    enum messageType {SUBREQ, SUBACK, SUBREF, UNSUB, GETINF, ROOMINF, PULLMSGS, MSGSER, MSGCLI, ERR};

private:
    messageType type;

    /* długość wiadomości w bajtach (nie liczac typu)*/
    int vecLength;
    std::vector < uint8_t > vec;

public:
    /*
     * konstruktor nieznanej wiadomości
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
        resizeVec();
    }

    /*
     * konstruktor dla typów o zmiennej długości treści wiadomości (podawana długość bajtowa treści wiadomości)
     */
    Message(messageType type, int length)
    {
        this->type = type;
        resizeVec(length);
    }

    /*
     * konstruktor dla FLP_Read
     */
    Message(uint8_t *data, size_t length)
    {
        vec.assign (data,data+length);
    }


    /*
     * settery i gettery
     */
    int getRoomID()
    {
        if (type == ERR)
        {
            std::cout <<"\n"<< "nie można odczytać RoomID dla Message typu ERR";
            return -1;
        }
        return toInt(0, 3);
    }

    void setRoomID(int id)
    {
        if (type == ERR)
        {
            std::cout <<"\n"<< "nie można ustawić RoomID dla Message typu ERR";
        }
        else
        {
            intToVec(id, 0, 3);
        }
    }

    int getReason()
    {
        if (type == SUBREF)
        {
            return toInt(4, 7);
        }
        std::cout <<"\n"<< "błędny typ Message dla getReason";
        return -1;
    }

    void setReason(int reason)
    {
        if (type == SUBREF)
        {
            intToVec(reason, 4, 7);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setReason";
    }

    int getLastMessageID()
    {
        if (type == ROOMINF)
        {
            return toInt(4, 7);
        }
        else if (type == PULLMSGS)
        {
            return toInt(8, 11);
        }
        std::cout <<"\n"<< "błędny typ Message dla getLastMessageID";
        return -1;
    }

    void setLastMessageID(int id)
    {
        if (type == ROOMINF)
        {
            intToVec(id, 4, 7);
        }
        else if (type == PULLMSGS)
        {
            intToVec(id, 8, 11);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setLastMessageID";
    }

    int getNumberOFMessages()
    {
        if (type == ROOMINF)
        {
            return toInt(8, 11);
        }
        std::cout <<"\n"<< "błędny typ Message dla getLastMessageID";
        return -1;
    }

    void setNumberOFMessages(int num)
    {
        if (type == ROOMINF)
        {
            intToVec(num, 8, 11);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setLastMessageID";
    }

    int getFirstMessageID()
    {
        if (type == PULLMSGS)
        {
            return toInt(4, 7);
        }
        std::cout <<"\n"<< "błędny typ Message dla getFirstMessageID";
        return -1;
    }

    void setFirstMessageID(int id)
    {
        if (type == PULLMSGS)
        {
            intToVec(id, 4, 7);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setFirstMessageID";
    }

    int getMessageID()
    {
        if (type == MSGSER)
        {
            return toInt(8, 11);
        }
        std::cout <<"\n"<< "błędny tym Message dla getMessageID";
        return -1;
    }

    void setMessageID(int id)
    {
        if (type == MSGSER)
        {
            intToVec(id, 8, 11);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setMessageID";
    }

    int getTime()
    {
        if (type == MSGSER)
        {
            return toInt(12, 15);
        }
        std::cout <<"\n"<< "błędny tym Message dla getTime";
        return -1;
    }

    void setTime(int timestamp)
    {
        if (type == MSGSER)
        {
            intToVec(timestamp, 12, 15);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setTime";
    }

    string getNick()
    {
        if (type == MSGSER)
        {
            return toString(12, 43);
        }
        else if (type == MSGCLI)
        {
            return toString(4, 35);
        }
        std::cout <<"\n"<< "błędny typ Message dla getNick";
        return NULL;
    }

    void setNick(string nick)
    {
        if (type == MSGSER)
        {
            stringToVec(nick, 12, 43);
        }
        else if (type == MSGCLI)
        {
            stringToVec(nick, 4, 35);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setNick";
    }

    int getMessageLength()
    {
        if (type == MSGSER)
        {
            return toInt(44, 45);
        }
        else if (type == MSGCLI)
        {
            return toInt(36, 37);
        }
        std::cout <<"\n"<< "błędny typ Message dla getMessageLength";
        return -1;
    }

    void setMessageLength(int length)
    {
        if (type == MSGSER)
        {
            intToVec(length, 44, 45);
        }
        else if (type == MSGCLI)
        {
            intToVec(length, 36, 37);
        }
        else std::cout <<"\n"<< "błędny typ Message dla setMessageLength";
    }

    string getMessage()
    {
        if (type == MSGSER)
        {
            return toString(46, 46+getMessageLength());
        }
        else if (type == MSGCLI)
        {
            return toString(38, 38+getMessageLength());
        }
        std::cout <<"\n"<< "błędny typ Message dla getMessage";
        return NULL;
    }

    void setMessage(string msg)
    {
        if (type == MSGSER)
        {
            stringToVec(msg, 46, 46+getMessageLength());
        }
        else if (type == MSGCLI)
        {
            stringToVec(msg, 38, 38+getMessageLength());
        }
        else std::cout <<"\n"<< "błędny typ Message dla setMessage";
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
    void intToVec(int value, int first, int last)
    {
        for (int i=first; i<=last; ++i)
        {
            vec[i] = (uint8_t) ((value >> 8*(i-first)) & 0xFF);
        }
    }

    /*
     * konwertuje zakres bajtów z vectora do stringa i go zwraca
     */
    string toString(int first, int last)
    {
        string str;
        for (int i=first; i<=last; ++i)
        {
            str.push_back(vec[i]);
        }
        return str;
    }

    /*
     * konwertuje stringa na bajty i zapisuje je w podany zakres
     */
    void stringToVec(string str, int first, int last)
    {
        int k = 0;
        for (std::string::iterator it=str.begin(); it!=str.end(); ++it)
        {
            vec[first+k] = *it;
            ++k;
            if (k>last) break;
        }
    }

    /*
     * dopasowuje dlugosc wektora dla stalej lub zmiennej dlugosci
     */
    void resizeVec()
    {
        if (type == MSGSER || type ==MSGCLI)
        {
            std::cout <<"\n"<< "nie podano długości dla Message o zmiennej długości!";
            this->type = ERR;
            vecLength = 0;
        }
        else if (type == SUBREQ || type == SUBACK || type == UNSUB || type == GETINF)
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
    void resizeVec(int length)
    {
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
            std::cout <<"\n"<< "błędny typ pakietu (podana długość dla pakietu o stałej długości)";
            this->type = ERR;
        }
        vec.resize(vecLength);
    }
};


#endif //TTCHAT_MESSAGE_H
