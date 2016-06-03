#include "flp.h"

FLP_Connection_t *FLP_Listen(int socket) {
    sleep(5);
    return new FLP_Connection_t;
}

bool FLP_Write(FLP_Connection_t *connection, uint8_t *data, size_t length) {
    if (connection->stop==1)
    {
        std::cout << "\n" << "FAKE_FLP: WRITE zwraca 1 -- connection " << connection;
        return 1;
    }
    std::cout << "\n" << "FAKE_FLP: WRITE dostał data -- ";
    for (uint8_t i=0 ; i<length; ++i)
    {
        std::cout << data+i;
    }
}

bool FLP_Read(FLP_Connection_t *connection, uint8_t **data, size_t *length) {
    if (connection->stop==1)
    {
        std::cout << "\n" << "FAKE_FLP: READ zwraca 1 -- connection " << connection;
        return 1;
    }

    //tworzę bufor zadanej długości
    uint8_t* newData = new uint8_t[*length];

    //uzupełniam go po kolei czymś
    for (uint8_t i=0 ; i<*length; ++i)
    {
        newData[i] = i;
    }
}

bool FLP_Close(FLP_Connection_t *connection) {
    connection->stop = 1;
}