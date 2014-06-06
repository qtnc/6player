#include<winsock2.h>
#include<cstring>
#include "socket.hpp"
#include<cstdio>

struct scope_lock {
CRITICAL_SECTION& cs;
scope_lock (CRITICAL_SECTION& cs1) : cs(cs1) { EnterCriticalSection(&cs); }
~scope_lock () { LeaveCriticalSection(&cs); }
};
#define SCOPE_LOCK(x) scope_lock ___CS_SCOPE_LOCK_##__LINE__##_ (x)


bool Socket::initialize (void) {
        WSADATA WSAData;
return !WSAStartup(MAKEWORD(2,0), &WSAData);
}

void Socket::deinitialize (void) {
WSACleanup();
}

Socket::Socket (void) : sock(0), csAvail(false)  { }

Socket::~Socket () {
close();
if (csAvail) DeleteCriticalSection(&cs);
}

Socket::Socket (const Socket& s): sock(s.sock), cs(s.cs), csAvail(s.csAvail)  {
Socket& sw = const_cast<Socket&>(s);
sw.csAvail=false;
sw.sock=0;
}

Socket& Socket::operator= (const Socket& s) {
close();
csAvail = s.csAvail;
cs = s.cs;
sock = s.sock;
Socket& sw = const_cast<Socket&>(s);
sw.csAvail=false;
sw.sock=0;
return *this;
}

int Socket::open (const char* host, int port) {
SOCKADDR_IN sin;
struct hostent*	 hostinfo = gethostbyname(host);
if (!hostinfo) return WSAGetLastError();
memset(&sin, 0, sizeof(sin));
sin.sin_family		= AF_INET;
sin.sin_port		= htons(port);
sin.sin_addr.s_addr= *((unsigned long *)(&(hostinfo->h_addr_list[0][0])));
sock = socket(AF_INET,SOCK_STREAM,0);
if (!sock) return WSAGetLastError();
//bind(sock, (SOCKADDR *)&sin, sizeof(sin));
if (SOCKET_ERROR==connect(sock, (SOCKADDR *)&sin, sizeof(sin))) return WSAGetLastError();
if (!csAvail) InitializeCriticalSection(&cs);
csAvail=true;
return 0;
}

int Socket::bind (const char* host, int port) {
SOCKADDR_IN sin;
struct hostent*	 hostinfo = gethostbyname(host);
if (!hostinfo) return WSAGetLastError();
memset(&sin, 0, sizeof(sin));
sin.sin_family		= AF_INET;
sin.sin_port		= htons(port);
sin.sin_addr.s_addr= host? *((unsigned long *)(&(hostinfo->h_addr_list[0][0]))) :0;
sock = socket(AF_INET,SOCK_STREAM,0);
if (!sock) return WSAGetLastError();
if (::bind(sock, (SOCKADDR *)&sin, sizeof(sin)) || listen(sock, SOMAXCONN)) return WSAGetLastError();
if (!csAvail) InitializeCriticalSection(&cs);
csAvail=true;
return 0;
}

int Socket::accept (Socket& s1) {
SCOPE_LOCK(cs);
SOCKET s = ::accept(sock, NULL, 0);
if (s==INVALID_SOCKET) return WSAGetLastError();
if (!s1.csAvail) InitializeCriticalSection(&(s1.cs));
s1.sock = s;
s1.csAvail=true;
return 0;
}

void Socket::close (void) {
if (!csAvail) return;
SCOPE_LOCK(cs);
if (sock) closesocket(sock);
sock=0;
}

int Socket::send (const void* s, int len) {
if (!sock) return -1;
if (len<0) len=strlen((const char*)s);
SCOPE_LOCK(cs);
int n=0, m=1;
while (sock && n<len && m>0 && m!=SOCKET_ERROR) {
m = ::send(sock, s+n, len-n, 0);
if (m==SOCKET_ERROR) { close(); return n; }
n+=m;
}
return n;
}

int Socket::recv (void* buf, int len) {
if (!sock || len<=0) return -1;
int n, pos = 0;
while(pos<len) {
n = ::recv(sock, buf+pos, len-pos, 0);
if (n<=0 || n==SOCKET_ERROR) return 0;
pos+=n;
}
return pos;
}


/*#include<iostream>
using namespace std;

void clientWork (Socket* sptr) {
Socket sock = *sptr;
char c, buf[1024];
int n=0;
while (sock.recv(&c,1)==1) {
if (c=='\n' || c=='\r' || n>=1023) {
buf[n]=0;
n=0;
sock.send(buf, strlen(buf));
sock.send("\r\n", 2);
continue;
}
buf[n++] = c;
}
//end
}

int main (int argc, char** argv) {
int n=0;
Socket::initialize();
Socket server, client;
cout << "Binding..." << endl;
if (n=server.bind(NULL, 81)) {
cout << "Error: " << n << endl;
return 0;
}
cout << "Waiting for clients ... " << endl;
 while (!server.accept(client)) {
cout << "Client accepted" << endl;
newThread(clientWork, &client);
}

}*/

