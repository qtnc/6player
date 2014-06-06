#ifndef _____SOCKET_HPP_8_____
#define _____SOCKET_HPP_8_____
#include<stdexcept>

#define newThread(f,p) CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)((void*)f), (LPVOID)p, 0, NULL)

struct Socket {
SOCKET sock;
CRITICAL_SECTION cs;
bool csAvail;

Socket(void);
Socket (const Socket&) ;
Socket& operator= (const Socket&) ;
~Socket();
int open(const char*, int) ;
int bind (const char*, int) ;
int accept (Socket&) ;
int recv(void*, int) ;
int send (const void*, int) ;
void close (void) ;

static bool initialize (void) ;
static void deinitialize (void) ;
};

struct socket_error: std::runtime_error {
socket_error (const char* what1) : runtime_error(what1) {}
};

#endif
