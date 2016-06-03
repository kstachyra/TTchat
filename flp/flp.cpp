#include "flp.h"

bool FLP_Listen(FLP_Connection_t** connection, int socket)
{
    *connection = new FLP_Connection_t;
    return 0;
}

bool FLP_Write(FLP_Connection_t *connection, uint8_t *data, size_t length)
{
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

bool FLP_Read(FLP_Connection_t *connection, uint8_t **data, size_t *length)
{
    std::cout << "\n" << "FAKE_FLP: READ zapisuje data -- ";
    if (connection->stop==1)
    {
        std::cout << "\n" << "FAKE_FLP: READ zwraca 1 -- connection " << connection;
        return 1;
    }
    *length = 10;


    //tworzę bufor zadanej długości
    *data = new uint8_t[*length];

    //uzupełniam go po kolei czymś
    for (uint8_t i=0 ; i<*length; ++i)
    {
        *data[i] = i;
    }
    std::cout << "\n" << "FAKE_FLP: READ zapisuje data -- ";
    return 0;
}

bool FLP_Close(FLP_Connection_t *connection)
{
    connection->stop = 1;
}