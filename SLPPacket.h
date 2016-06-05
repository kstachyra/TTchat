#ifndef TTCHAT_MESSAGE_H
#define TTCHAT_MESSAGE_H

//TODO W SLPPacket dac Model/Message

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

    void setTime(int timestamp);

    string getNick();

    void setNick(string nick);

    int getMessageLength();

    void setMessageLength(int length);

    string getMessage();

    void setMessage(string msg);

    void toDataBuffer(uint8_t** data, size_t* length);

    void print();

private:
    /*
     * konwertuje zakres bajtów z vectora to inta i go zwraca
     * (WAŻNE - int to 4 bajty, brak kontroli tego)
     */
    int toInt(int first, int last);

    /*
     * konwertuje inta na bajty i zapisuje je w podany zakres
     * (WAŻNE - int to 4 bajty, brak kontroli tego)
     */
    void intToVec(int value, int first, int last);

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
    void SetTypeToVec();

    /*
     * dopasowuje dlugosc wektora dla stalej lub zmiennej dlugosci
     */
    void resizeVec();

    void resizeVec(int length);
};


#endif //TTCHAT_MESSAGE_H
