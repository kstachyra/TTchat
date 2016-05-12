#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

#include "protocol.h"

static volatile int exit_request = 0;

static void signal_handler(int sig)
{
    exit_request = 1;
}

int listenSocketInit(unsigned short port)
{
    //inicjalizacja socketa nasłuchującego na nowe połączenia na danym porcie
    int sock, csock;
    struct sockaddr_in sin;
    char *host = "127.0.0.1";

    sigset_t mask;
    sigset_t orig_mask;
    struct sigaction act;

    // Set up signal handler
    memset (&act, 0, sizeof(act));
    act.sa_handler = signal_handler;
    if(sigaction(SIGINT, &act, 0)) {
        perror ("sigaction");
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    // Mask SIGINT
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
        perror ("sigprocmask");
        return 1;
    }

    cout << "Creating socket..." << endl;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &sin.sin_addr) != 1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    cout << "Binding..." << endl;
    if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

int listen()
{
    //nasłuchuje i zwraca co usłyszało
    return 0;
}