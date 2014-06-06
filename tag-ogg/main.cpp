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
int result = CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, 0/*CREATE_NO_WINDOW /*| DETACHED_PROCESS*/, NULL, NULL, &si, &proc);
if (!result) return -GetLastError();
WaitForSingleObject(proc.hProcess, 3000);
result = -1;
GetExitCodeProcess(proc.hProcess, &result);
if (result) TerminateProcess(proc.hProcess,&result);
CloseHandle(proc.hThread);
CloseHandle(proc.hProcess);
return result;
}

static BOOL tagCheck (const char* filename, DWORD handle) {
BASS_CHANNELINFO info;
if (!BASS_ChannelGetInfo(handle,&info)) return FALSE;
return 
info.ctype == BASS_CTYPE_STREAM_OGG;
}

static BOOL tagUpdate (playlistitem& it, DWORD handle) {
DeleteFileW(toWString(it.file+".tmp").c_str());
string cmd; cmd.clear(); cmd.reserve(1024);
cmd += string("oggz-comment.exe -d -o \"") + it.file + string(".tmp\" ");
#define T(n,l) if(!it.n.empty()) cmd += string("\"" #l "=") + it.n + string("\" ");
T(title,TITLE) T(artist,ARTIST) T(album,ALBUM) T(genre,GENRE)
T(comment,COMMENT) T(year,DATE) T(composer,COMPOSER) T(subtitle,SUBTITLE)
T(tracknum,TRACKNUMBER) T(disc,DISCNUMBER) T(copyright,COPYRIGHT)
#undef T
cmd += string("\"") + it.file + string("\"");
int result = execute(toWString(cmd).c_str());
if (!result) {
CopyFileW(toWString(it.file+".tmp").c_str(), toWString(it.file).c_str(), FALSE);
DeleteFileW(toWString(it.file+".tmp").c_str());
}
return !result;
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


