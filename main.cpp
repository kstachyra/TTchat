/*
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include "flp/flp.h"

#include <iostream>

using namespace std;

static volatile int exit_request = 0;

static void signal_handler(int sig)
{
	exit_request = 1;
}

int main(int argc, char *argv[])
{
	char *host = "192.168.61.40";
	unsigned short port = 1234;
	FLP_Listener_t FLP_Listener;
	FLP_Connection_t *FLP_Connection;
	uint8_t *message = (uint8_t*)"Tajna wiadomosc.\n";
	size_t receivedMessageLength;

	struct sigaction act;

	// Set up signal handler
	memset (&act, 0, sizeof(act));
	act.sa_handler = signal_handler;
	if(sigaction(SIGINT, &act, 0)) {
		perror ("sigaction");
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);

	FLP_ListenerInit(&FLP_Listener, port, host);

	// Listen for new connections
	while(true) {

		printf("Listening for new FLP connection... \n");
		if(FLP_Listen(&FLP_Listener, &FLP_Connection, 500)) {

			// Check if new connection was established or timeout occurred
			if(FLP_Connection) {
				printf("New connection established.\n");

				printf("Sending message: ");
				unsigned int i;
				for(i=0; i<strlen((char*)message); i++) {
					printf("%02x", message[i]);
				}
				printf("\n");

				if(FLP_Write(FLP_Connection, message, strlen((char*)message))) {
					printf("Message sent successfully.\n");
				} else {
					printf("Sending message failed.\n");
				}

				printf("Trying to read message...");
				if(FLP_Read(FLP_Connection, &message, &receivedMessageLength)) {
					printf("Message read successfully.\n");
				} else {
					printf("Reading message failed.\n");
				}

				printf("Received message: ");
				for(i=0; i<receivedMessageLength; i++) {
					printf("%02x", message[i]);
				}
				printf("\n");

				while(!exit_request);

				return EXIT_SUCCESS;

			} else if(exit_request) {
				return EXIT_SUCCESS;
			}

		} else {

			printf("Could not establish new connection.\n");
			return 0;
		}

	}

	return EXIT_SUCCESS;
}
*/
