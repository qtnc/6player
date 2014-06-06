#define UNICODE
#include<windows.h>
#include<stdlib.h>
#include<stdio.h>
#include "Socket.hpp"

#define BUFSIZE 4096

void TransferStdinToSocket (int port) {
printf("Opening server on port %d\r\n", port);
Socket::initialize();
Socket server, client;
server.bind(NULL,port);
printf("Waiting for client\r\n");
server.accept(client);
printf("Client accepted\r\n");

char buf[BUFSIZE];
HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
if (!hStdin) return;

int read=0, written=0;
while(
ReadFile(hStdin, buf, BUFSIZE, &read, NULL)
&& read>0
&& (written=client.send(buf,read))
);


printf("Closing sockets\r\n");
client.close();
server.close();
CloseHandle(hStdin);
}
