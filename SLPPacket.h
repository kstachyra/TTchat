#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

#include <vector>
#include <string>

/* długość nicku */
#define NICK_LENGTH 32

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

/*
 * długość wektora dla niezdefiniowanego typu pakietu
 */
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
    SLPPacket();

    /*
     * konstruktor dla typów o stałej długości
     */
    SLPPacket(messageType type);

    /*
     * konstruktor dla typów o zmiennej długości treści wiadomości (podawana długość bajtowa treści wiadomości)
     */
    SLPPacket(messageType type, int length);

    /*
     * konstruktor dla FLP_Read
     */
    SLPPacket(uint8_t *data, size_t length);

    /*
     * konstruktor MSGCLI -> MSGSER
     */
    //SLPPacket(const SLPPacket pck);

    /*
     * settery i gettery
     */
    int getType();

    /*
     * poniższe settey i gettery napisane na sztywno dla długości pakietów, muszą ulec zmianie przy zmianie SLP
     */
    int getChatroomId();

    void setChatroomId(uint64_t id);

    int getReason();

    void setReason(int reason);

    int getLastMessageID();

    void setLastMessageID(int id);

    int getNumberOFMessages();

    void setNumberOFMessages(int num);

    int getFirstMessageID();

    void setFirstMessageID(int id);

    int getMessageID();

    void setMessageID(int id);

    int getTime();

    void setTime(uint32_t timestamp);

    string getNick();

    void getNick(uint8_t* nick);

    void setNick(string nick);

    void setNick(uint8_t* nick);

    int getMessageLength();

    void setMessageLength(int length);

    string getMessage();

    void getMessage(uint8_t* msg, size_t length);

    void setMessage(string msg);

    void setMessage(uint8_t* payload, size_t length);

    void toDataBuffer(uint8_t** data, size_t* length);

    void print();

private:
    /*
     * konwertuje zakres bajtów z vectora to inta i go zwraca
     * 
     */
    int toInt(int first, int last);

    /*
     * konwertuje inta na bajty i zapisuje je w podany zakres
     * 
     */
    void intToVec(uint64_t value, int first, int last);

    /*
     * konwertuje zakres bajtów z vectora do stringa i go zwraca
     */
    string toString(int first, int last);

    /*
     * konwertuje stringa na bajty i zapisuje je w podany zakres
     */
    void stringToVec(string str, int first, int last);

    /*
     * ustawia type na podstawie zawartości vectora
     */
    void setTypeFromVec();

    /*
     * ustawia VECTOR NA PODSTAWIE TYPU
     */
    void setTypeToVec();

    /*
     * dopasowuje dlugosc wektora dla stalej lub zmiennej dlugosci
     */
    void resizeVec();

    void resizeVec(int length);
};


#endif //TTCHAT_MESSAGE_H
