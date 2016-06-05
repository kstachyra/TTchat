#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

#include <vector>
#include <string>

/*
 * długości pakietów protokołu SLP
 */
#define SUBREQ_LENGTH 6
#define SUBACK_LENGTH 6
#define SUBREF_LENGTH 10
#define UNSUB_LENGTH 6
#define GETINF_LENGTH 6
#define ROOMINF_LENGTH 14
#define PULLMSGS_LENGTH 14

/*
 * bazowe długości zmiennej długości pakietów protokołu SLP (bez payloadlength)
 */
#define MSGSER_LENGTH 48
#define MSGCLI_LENGTH 40


#define ERR_LENGTH 2

using namespace std;

class SLPPacket
{
public:
    //typ ERR oznacza stworzony obiekt Message bez innego określonego typu
    enum messageType {SUBREQ, SUBACK, SUBREF, UNSUB, GETINF, ROOMINF, PULLMSGS, MSGSER, MSGCLI, ERR};

private:
    messageType type;

    int vecLength;
    std::vector < uint8_t > vec;

public:
    /*
     * konstruktor nieznanej wiadomości
     */
    SLPPacket()
    {
        this->type = ERR;
        resizeVec();
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
        setTypeFromVec();
    }

    /*
     * settery i gettery
     */
    int getType()
    {
    	return this->type;
    }

    /*
     * poniższe gettery napisane na sztywno dla długości pakietów, ulegną zmianie przy zmianie koncepcji SLP
     */
    int getRoomID()
    {
        if (type == ERR)
        {
            std::cout<< "SLPPacket: nie można odczytać RoomID dla Message typu ERR" <<"\n";
            return -1;
        }
        return toInt(2, 5);
    }

    void setRoomID(int id)
    {
        if (type == ERR)
        {
            std::cout<< "SLPPacket: SLPPacket: nie można ustawić RoomID dla Message typu ERR" <<"\n";
        }
        else
        {
            intToVec(id, 2, 5);
        }
    }

    int getReason()
    {
        if (type == SUBREF)
        {
            return toInt(6, 9);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getReason" <<"\n";
        return -1;
    }

    void setReason(int reason)
    {
        if (type == SUBREF)
        {
            intToVec(reason, 6, 9);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setReason" <<"\n";
    }

    int getLastMessageID()
    {
        if (type == ROOMINF)
        {
            return toInt(6, 9);
        }
        else if (type == PULLMSGS)
        {
            return toInt(10, 13);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getLastMessageID" <<"\n";
        return -1;
    }

    void setLastMessageID(int id)
    {
        if (type == ROOMINF)
        {
            intToVec(id, 6, 9);
        }
        else if (type == PULLMSGS)
        {
            intToVec(id, 10, 3);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setLastMessageID" <<"\n";
    }

    int getNumberOFMessages()
    {
        if (type == ROOMINF)
        {
            return toInt(10, 13);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getLastMessageID" <<"\n";
        return -1;
    }

    void setNumberOFMessages(int num)
    {
        if (type == ROOMINF)
        {
            intToVec(num, 10, 13);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setLastMessageID" <<"\n";
    }

    int getFirstMessageID()
    {
        if (type == PULLMSGS)
        {
            return toInt(6, 9);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getFirstMessageID" <<"\n";
        return -1;
    }

    void setFirstMessageID(int id)
    {
        if (type == PULLMSGS)
        {
            intToVec(id, 6, 9);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setFirstMessageID" <<"\n";
    }

    int getMessageID()
    {
        if (type == MSGSER)
        {
            return toInt(10, 13);
        }
        std::cout<< "SLPPacket: błędny tym Message dla getMessageID" <<"\n";
        return -1;
    }

    void setMessageID(int id)
    {
        if (type == MSGSER)
        {
            intToVec(id, 10, 13);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setMessageID" <<"\n";
    }

    int getTime()
    {
        if (type == MSGSER)
        {
            return toInt(14, 17);
        }
        std::cout<< "SLPPacket: błędny tym Message dla getTime" <<"\n";
        return -1;
    }

    void setTime(int timestamp)
    {
        if (type == MSGSER)
        {
            intToVec(timestamp, 14, 17);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setTime" <<"\n";
    }

    string getNick()
    {
        if (type == MSGSER)
        {
            return toString(14, 45);
        }
        else if (type == MSGCLI)
        {
            return toString(6, 37);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getNick" <<"\n";
        return NULL;
    }

    void setNick(string nick)
    {
        if (type == MSGSER)
        {
            stringToVec(nick, 14, 45);
        }
        else if (type == MSGCLI)
        {
            stringToVec(nick, 6, 37);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setNick" <<"\n";
    }

    int getMessageLength()
    {
        if (type == MSGSER)
        {
            return toInt(46, 47);
        }
        else if (type == MSGCLI)
        {
            return toInt(38, 39);
        }
        std::cout<< "SLPPacket: błędny typ Message dla getMessageLength" <<"\n";
        return -1;
    }

    void setMessageLength(int length)
    {
        if (type == MSGSER)
        {
            intToVec(length, 46, 47);
        }
        else if (type == MSGCLI)
        {
            intToVec(length, 38, 39);
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setMessageLength" <<"\n";
    }

    string getMessage()
    {
        if (type == MSGSER)
        {
            return toString(48, 48+getMessageLength());
        }
        else if (type == MSGCLI)
        {
            return toString(40, 40+getMessageLength());
        }
        std::cout<< "SLPPacket: błędny typ Message dla getMessage" <<"\n";
        return NULL;
    }

    void setMessage(string msg)
    {
        if (type == MSGSER)
        {
            stringToVec(msg, 48, 48+getMessageLength());
        }
        else if (type == MSGCLI)
        {
            stringToVec(msg, 40, 40+getMessageLength());
        }
        else std::cout<< "SLPPacket: błędny typ Message dla setMessage" <<"\n";
    }

    void toDataBuffer(uint8_t** data, size_t* length)
    {
    	*length = vec.size();

        *data = new uint8_t[*length];

        for (int i=0; i<*length; ++i)
        {
            (*data)[i] = vec[i];
        }
        setTypeFromVec();
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
     * ustawia type na podstawie zawartości vectora
     */
    void setTypeFromVec()
    {
    	if (vec[0] == 0x00 && vec[1] == 0x10) type = SUBREQ;
    	else if (vec[0] == 0x00 && vec[1] == 0x11) type = SUBACK;
    	else if (vec[0] == 0x00 && vec[1] == 0x12) type = SUBREF;
    	else if (vec[0] == 0x00 && vec[1] == 0x13) type = UNSUB;
    	else if (vec[0] == 0x00 && vec[1] == 0x20) type = GETINF;
    	else if (vec[0] == 0x00 && vec[1] == 0x21) type = ROOMINF;
    	else if (vec[0] == 0x00 && vec[1] == 0x30) type = PULLMSGS;
    	else if (vec[0] == 0x00 && vec[1] == 0x31) type = MSGSER;
    	else if (vec[0] == 0x00 && vec[1] == 0x32) type = MSGCLI;
    	else if (vec[0] == 0x00 && vec[1] == 0x00) type = ERR;
    }

    /*
     * dopasowuje dlugosc wektora dla stalej lub zmiennej dlugosci
     */
    void resizeVec()
    {
    	switch(type)
    	{
    	case SUBREQ: vecLength = SUBREQ_LENGTH; break;
    	case SUBACK: vecLength = SUBACK_LENGTH; break;
    	case SUBREF: vecLength = SUBREF_LENGTH; break;
    	case UNSUB: vecLength = UNSUB_LENGTH; break;
    	case GETINF: vecLength = GETINF_LENGTH; break;
    	case ROOMINF: vecLength = ROOMINF_LENGTH; break;
    	case PULLMSGS: vecLength = PULLMSGS_LENGTH; break;
    	case ERR: vecLength = ERR_LENGTH; break;

    	default: std::cout<< "SLPPacket.resizeVec: błędny typ pakietu (brak długości)" <<"\n";
    	}
    	vec.resize(vecLength);
    }

    void resizeVec(int length)
    {
    	switch(type)
    	{
    	case MSGSER: vecLength = MSGSER_LENGTH + length; break;
    	case MSGCLI: vecLength = MSGCLI_LENGTH + length; break;

    	default:
    	{
    		std::cout<< "SLPPacket: błędny typ pakietu (podana długość dla pakietu o stałej długości)" <<"\n";
    		this->type = ERR;
    	}
    	}
        vec.resize(vecLength);
    }
};


#endif //TTCHAT_MESSAGE_H
