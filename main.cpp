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

#include <iostream>

using namespace std;

static volatile int exit_request = 0;

static void signal_handler(int sig)
{
	exit_request = 1;
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

				// Create new client thread passing clientSocket as a parameter
				// ...
			}
		}
	}

	cout << "Closing socket used for listening for new clients..." << endl;
	close(sock);

	return EXIT_SUCCESS;
}
