#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

#include <vector>
#include <string>

using namespace std;

class SLPPacket
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
    SLPPacket()
    {
        this->type = ERR;
    }

    /*
     * konstruktor dla typów o stałej długości
     */
    SLPPacket(messageType type)
    {
        this->type = type;
        resizeVec();
    }

    /*
     * konstruktor dla typów o zmiennej długości treści wiadomości (podawana długość bajtowa treści wiadomości)
     */
    SLPPacket(messageType type, int length)
    {
        this->type = type;
        resizeVec(length);
    }

    /*
     * konstruktor dla FLP_Read
     */
    SLPPacket(uint8_t *data, size_t length)
    {
    	vecLength = length;
        vec.assign (data,data+length);
    }


    /*
     * settery i gettery
     */
    int getType()
    {
    	return this->type;
    }

    /*int getRoomID()
    {
        if (type == ERR)
        {
            std::cout<< "SLPPacket: nie można odczytać RoomID dla Message typu ERR" <<"\n";
            return -1;
        }
        return toInt(0, 3);
    }

    void setRoomID(int id)
    {
        if (type == ERR)
        {
            std::cout<< "SLPPacket: SLPPacket: nie można ustawić RoomID dla Message typu ERR" <<"\n";
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
        std::cout<< "SLPPacket: błędny typ Message dla getReason" <<"\n";
        return -1;
    }

    void setReason(int reason)
    {
        if (type == SUBREF)
        {
            intToVec(reason, 4, 7);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setReason" <<"\n";
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
        std::cout<< "SLPPacket: błędny typ Message dla getLastMessageID" <<"\n";
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
        else std::cout<< "SLPPacket: błędny typ Message dla setLastMessageID" <<"\n";
    }

    int getNumberOFMessages()
    {
        if (type == ROOMINF)
        {
            return toInt(8, 11);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getLastMessageID" <<"\n";
        return -1;
    }

    void setNumberOFMessages(int num)
    {
        if (type == ROOMINF)
        {
            intToVec(num, 8, 11);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setLastMessageID" <<"\n";
    }

    int getFirstMessageID()
    {
        if (type == PULLMSGS)
        {
            return toInt(4, 7);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getFirstMessageID" <<"\n";
        return -1;
    }

    void setFirstMessageID(int id)
    {
        if (type == PULLMSGS)
        {
            intToVec(id, 4, 7);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setFirstMessageID" <<"\n";
    }

    int getMessageID()
    {
        if (type == MSGSER)
        {
            return toInt(8, 11);
        }
        std::cout<< "SLPPacket: błędny tym Message dla getMessageID" <<"\n";
        return -1;
    }

    void setMessageID(int id)
    {
        if (type == MSGSER)
        {
            intToVec(id, 8, 11);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setMessageID" <<"\n";
    }

    int getTime()
    {
        if (type == MSGSER)
        {
            return toInt(12, 15);
        }
        std::cout<< "SLPPacket: błędny tym Message dla getTime" <<"\n";
        return -1;
    }

    void setTime(int timestamp)
    {
        if (type == MSGSER)
        {
            intToVec(timestamp, 12, 15);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setTime" <<"\n";
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
        std::cout<< "SLPPacket: błędny typ Message dla getNick" <<"\n";
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
        else std::cout<< "SLPPacket: błędny typ Message dla setNick" <<"\n";
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
        std::cout<< "SLPPacket: błędny typ Message dla getMessageLength" <<"\n";
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
        else std::cout<< "SLPPacket: błędny typ Message dla setMessageLength" <<"\n";
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
        std::cout<< "SLPPacket: błędny typ Message dla getMessage" <<"\n";
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
        else std::cout<< "SLPPacket: błędny typ Message dla setMessage" <<"\n";
    }*/

    void toDataBuffer(uint8_t** data, size_t* length)
    {
    	*length = vec.size();

        *data = new uint8_t[*length];

        for (int i=0; i<*length; ++i)
        {
            (*data)[i] = vec[i];
        }
        switch(this->type)
        {
        case SUBACK:
        	(*data)[0]=0x00;
        	(*data)[1]=0x11;
        	break;
        }
    }

    void print()
    {
    	std::cout<<"SLPPacket.print: ";
    	for (int i=0; i<vecLength; ++i)
    	{
    		printf("%02x", vec[i]);
    	}
    	std::cout<<"\n";
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
            std::cout<< "SLPPacket: nie podano długości dla Message o zmiennej długości!" <<"\n";
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
        vec.resize(vecLength+2);
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
            std::cout<< "SLPPacket: błędny typ pakietu (podana długość dla pakietu o stałej długości)" <<"\n";
            this->type = ERR;
        }
        vec.resize(vecLength+2);
    }
};


#endif //TTCHAT_MESSAGE_H
