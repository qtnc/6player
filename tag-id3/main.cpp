#define UNICODE
#include<windows.h>
#include<stdio.h>
#include "../bass.h"
#include "../bass_aac.h"
#include "../playlist.h"
#include "../plugin.h"
#include<string>
#include "../strings.hpp"
using namespace std;

#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

#define EXPORT __declspec(dllexport)

static int execute (const TCHAR* cmdline) {
PROCESS_INFORMATION proc;
STARTUPINFO si;
memset(&si, 0, sizeof(si));
si.cb = sizeof(si);
si.dwFlags = STARTF_FORCEONFEEDBACK;
int result = CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &proc);
if (!result) return -GetLastError();
WaitForSingleObject(proc.hProcess, INFINITE);
GetExitCodeProcess(proc.hProcess, &result);
CloseHandle(proc.hThread);
CloseHandle(proc.hProcess);
return result;
}

static BOOL tagCheck (const char* filename, DWORD handle) {
BASS_CHANNELINFO info;
if (!BASS_ChannelGetInfo(handle,&info)) return FALSE;
return 
info.ctype == BASS_CTYPE_STREAM_MP3
|| info.ctype == BASS_CTYPE_STREAM_AAC
|| info.ctype == BASS_CTYPE_STREAM_MP4;
}

static BOOL tagUpdate (playlistitem& it, DWORD handle) {
string cmd; cmd.clear(); cmd.reserve(1024);
cmd += string("id3.exe -M -d ");
#define T(n,l) if(!it.n.empty()) cmd += string("-" #l " \"") + it.n + string("\" ");
T(title,t) T(artist,a) T(album,l) T(genre,g)
T(year,y) T(comment,c) T(tracknum,n)
#undef T
cmd += string("\"") + it.file + string("\"");
return !execute(toWString(cmd).c_str());
}

static void* tagOpen (const void* unused, const void* data1, DWORD mode, DWORD data2) {
switch(mode){
case PF_TAG_CHECK: return tagCheck(data1, data2);
case PF_OPEN_WRITE: return tagUpdate( *(playlistitem*)data1, data2 );
case PF_OPEN_READ: return FALSE;
default: return FALSE;
}}

static const int nMax = 1;
static const QCPLUGIN data[] = {
{ PT_TAGGER, PF_CAN_WRITE, NULL, NULL, tagOpen, NULL, NULL, NULL, NULL, NULL }
};

extern "C" LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}


