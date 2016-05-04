#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
  int sock, csock;
  struct sockaddr_in sin;
  char *host = "127.0.0.1";
  unsigned short port = 1234;

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

  cout << "Listening..." << endl;
  if (listen(sock, SOMAXCONN) != 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  if ((csock = accept(sock, NULL, NULL)) == -1) {
    perror("accept");
    exit(EXIT_FAILURE);
  }
  cout << "Connection accepted." << endl;

  string MSG = "";

  while(MSG != "$")
  {
	cout << "Type message (type $ to disconnect): ";
	getline(cin, MSG);
	int amt = send(csock, MSG.c_str(), MSG.size(), 0);
  	printf("%d bytes sent!\n", amt);
  }
  close(csock);

  close(sock);
  return EXIT_SUCCESS;
}
