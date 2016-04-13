#include <iostream>
#include <unistd.h>
using namespace std;

void print();

int main()
{
    cout << "Hello, World!" << endl;

    for (int i=0; i<3; ++i)
    {
        print();
    }
    return 0;

    sleep(50);
}

void print()
{
    cout<<"HsHsHs";
}