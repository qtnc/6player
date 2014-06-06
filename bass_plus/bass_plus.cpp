#include<windows.h>
#include <stdio.h>
#include <string.h>
#include "../bass.h"
#include "../bass-addon.h"
#include "bass_plus.h"
#include "../Socket.hpp"
#include<io.h>
#include<fcntl.h>
#ifndef bassfunc
const BASS_FUNCTIONS *bassfunc=NULL;
#endif
#include<string>
#include "../strings.hpp"
using namespace std;

#define streq(a,b) (!strcmp(a,b))
#define strneq(a,b,n) (!strncmp(a,b,n))

static void CALLBACK DSPStreamCopy (HDSP dsp, DWORD input, void* buffer, DWORD length, DWORD output) {
BASS_StreamPutData(output, buffer, length);
}

static void CALLBACK SyncFreeCopy (HSYNC sync, DWORD source, DWORD unused, DWORD copy) {
BASS_StreamPutData(copy, &unused, BASS_STREAMPROC_END);
}

DWORD EXPORT BASS_StreamCreateCopy (DWORD source, DWORD flags, DWORD dspPriority) {
BASS_CHANNELINFO info;
if (!BASS_ChannelGetInfo(source, &info)) return NULL;
DWORD copy = BASS_StreamCreate(info.freq, info.chans, flags, STREAMPROC_PUSH, source);
if (!copy) return NULL;
BASS_ChannelSetDSP(source, DSPStreamCopy, copy, dspPriority);
BASS_ChannelSetSync(source, BASS_SYNC_FREE | BASS_SYNC_ONETIME | BASS_SYNC_MIXTIME, 0, SyncFreeCopy, copy);
return copy;
}

DWORD EXPORT BASS_StreamGetBitrate (DWORD h) {
QWORD len = BASS_StreamGetFilePosition(h, BASS_FILEPOS_END);
if (len<0) return 0;
double dur = BASS_ChannelBytes2Seconds(h, BASS_ChannelGetLength(h, BASS_POS_BYTE));
if (dur>0) return 0.5 + (len / (125*dur));
DWORD bufMs = BASS_GetConfig(BASS_CONFIG_NET_BUFFER);
return 8 * len / bufMs;
}

static BOOL stdinInUse = FALSE;
static void CALLBACK StdinNoClose (void* unused) {  stdinInUse=FALSE; }
static QWORD CALLBACK StdinLength (void* unused) { return 0; }
static BOOL CALLBACK StdinSeek (QWORD offset, void* unused) { return FALSE;  }
static DWORD CALLBACK StdinRead (void* buf, DWORD len, HANDLE h) { 
DWORD read = -1;
DWORD result = ReadFile(h, buf, len, &read, NULL);
if (!result) printf("Read failed! %d\r\n", GetLastError());
return result? read : -1;
}

DWORD EXPORT BASS_StreamCreateStdin (DWORD flags) {
if (stdinInUse) return NULL;
stdinInUse=TRUE;
HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
if (!h) return NULL;
BASS_FILEPROCS procs = { StdinNoClose, StdinLength, StdinRead, StdinSeek };
return BASS_StreamCreateFileUser( STREAMFILE_BUFFER, flags | BASS_STREAM_RESTRATE | BASS_STREAM_BLOCK, &procs, h);
}

static void CALLBACK RawSockClose (Socket* sock) { sock->close(); delete sock; }
static DWORD CALLBACK RawSockRead (void* buf, DWORD len, Socket* sock) { 
return sock->recv(buf,len);
}

DWORD EXPORT BASS_StreamCreateRawSocket (const char* addr, DWORD flags) {
string str = addr;
int pos = str.find(':');
if (pos<0 || pos>=str.size()) return NULL;
string host = str.substr(0,pos);
int port = toInt(str.substr(pos+1));
printf("Opening stream on host %s, port %d\r\n", host.c_str(), port);
Socket* sock = NULL;
for (int i=0; i<=10; i++) {
sock = new Socket();
if (!sock->open(host.c_str(), port)) break;
printf("Trial %d: Connection failed\r\n", i);
delete sock;
sock = NULL;
Sleep(1000);
}
if (!sock) return NULL;
printf("Connection success\r\n");
BASS_FILEPROCS procs = { RawSockClose, StdinLength, RawSockRead, StdinSeek };
return BASS_StreamCreateFileUser( STREAMFILE_BUFFER, flags | BASS_STREAM_RESTRATE | BASS_STREAM_BLOCK, &procs, sock);
}

static BASS_PLUGININFO plugininfo = {0x02040000, 0, NULL };

const void* WINAPI EXPORT BASSplugin(DWORD face) {
switch (face) {
		case BASSPLUGIN_INFO:
			return (void*)&plugininfo;
//		case BASSPLUGIN_CREATE:
//			return (void*)StreamCreateProc;
//		case BASSPLUGIN_CREATEURL:
//			return (void*)StreamCreateURLProc;
	}
	return NULL;
}

BOOL WINAPI EXPORT DllMain(HANDLE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE)hDLL);
			if (HIWORD(BASS_GetVersion())!=BASSVERSION || !GetBassFunc()) {
				MessageBoxA(0,"Incorrect BASS.DLL version (" BASSVERSIONTEXT " is required)", "BASS", MB_ICONERROR | MB_OK);
				return FALSE;
			}
			break;
}
	return TRUE;
}
