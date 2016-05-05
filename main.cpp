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

#include <iostream>

using namespace std;

static volatile int exit_request = 0;

static void signal_handler(int sig)
{
	exit_request = 1;
}

typedef struct client {
	int socket;
	pthread_t receiverThd;
	pthread_t transmitterThd;
	pthread_attr_t receiverThreadAttr;
	pthread_attr_t transmitterThreadAttr;
	struct client *next;
} client_t;

client_t *client_list_head = NULL;

void add_client(client_t *new_client)
{
	client_t *current;

	if(client_list_head == NULL) {
		client_list_head = new_client;
	} else {
		current = client_list_head;
		while(current->next != NULL) current = current->next;
		current->next = new_client;
	}
}

void *receiverThread(void *args)
{
	char msg[1000];
	size_t msg_len;
	client_t *client = (client_t*) args;

	while(true) {
		msg_len = read(client->socket, msg, 1000);
		if(msg_len < 1000) {
			msg[msg_len] = 0;
		} else {
			msg[999] = 0;
		}
		printf("%s", msg);
	}

	return NULL;
}

void *transmitterThread(void *args)
{
	size_t buf_size = 1000;
	char *buf;
	client_t *client = (client_t*) args;

	buf = (char*)malloc(sizeof(char)*buf_size);

	while(true) {
		getline(&buf, &buf_size, stdin);
		write(client->socket, buf, strlen(buf));
	}

	return NULL;
}

/* -------------------------------------------------------------------------
 * Main thread
 * ------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
	int sock, csock;
	struct sockaddr_in sin;
	char *host = "127.0.0.1";
	unsigned short port = 1234;

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

	// Listen for new connections
	while(true) {

		cout << "Listening..." << endl;
		if (listen(sock, SOMAXCONN) != 0) {
			perror("listen");
			exit(EXIT_FAILURE);
		}

		int result;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		result = pselect(sock+1, &rfds, &rfds, (fd_set *) 0, NULL, &orig_mask);
		if (result < 0 && errno != EINTR) {
			perror ("select");
			return 1;
		} else if (exit_request) {

			// Terminate all client threads
			// ...

			break;
		} else {
			if ((csock = accept(sock, NULL, NULL)) == -1) {
				perror("accept");
				exit(EXIT_FAILURE);
			} else {
				cout << "New connection accepted. Creating client thread..." << endl;

				// Allocate memory for client_t structure
				client_t *new_client = (client_t*)malloc(sizeof(client_t));

				new_client->socket = csock;
				new_client->next = NULL;

				// Create new client thread passing clientSocket as a parameter
				pthread_attr_init(&new_client->receiverThreadAttr);
				pthread_attr_init(&new_client->transmitterThreadAttr);

				pthread_create(&new_client->receiverThd, &new_client->receiverThreadAttr, receiverThread, (void *)new_client);
				pthread_create(&new_client->transmitterThd, &new_client->transmitterThreadAttr, transmitterThread, (void *)new_client);

				add_client(new_client);
			}
		}
	}

	cout << "Closing socket used for listening for new clients..." << endl;
	close(sock);

	return EXIT_SUCCESS;
}
