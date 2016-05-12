/*
 * monitor do zarządzania klientami, tworzenia, usuwania i przypisywania do rozmów
 * dziedziczy po klasie monitor
 * zapewnia bezpieczny dostęp do współdzielonych zasobów
 */

#include "monitor.h"
#include "Client.h"

#include <unordered_map>
#include <list>

using namespace std;

class ClientMonitor : private Monitor
{
public:
    int clientCount;
    int conversationCount;



    struct conversation
    {
        int id;
        list < int > clientList;
    };

    unordered_map < int, Client > clients;
    list < conversation > conversations;

public:
    ClientMonitor() : clientCount(0), conversationCount(0) {}

    int addClient()
    {
        enter();
        Client newClient;
        clients[newClient.id] = newClient;

        leave();
    }
 };