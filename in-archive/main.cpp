#include <windows.h>
#include <stdio.h>
#include <string.h>
#include<zlib.h>
#include "bzlib.h"
#include "../plugin.h"
#include "../playlist.h"
#include "../bass.h"
#include<string>
using namespace std;

struct ARFILE;

struct ARINFO {
ARFILE* ar;
string fn;
};

extern "C" {
void ar_close_free (ARFILE* f) ;
int ar_readdir (ARFILE* f) ;
int ar_read (ARFILE* f, void* buf, size_t len) ;
int ar_fseek (ARFILE* f, int pos, int ance) ;
ARFILE* ar_new_open (const char* fn, const char* path) ;
ARFILE* ar_new_opendir (const char* fn) ;
const char* ar_entryname (ARFILE* f) ;
int ar_entrysize (ARFILE* f) ;
}

#define EXPORT __declspec(dllexport)

static ARINFO* arDirOpen (const void* unused, const char* fn, int mode, int flags) {
if (mode!=PF_OPEN_READ) return NULL;
ARFILE* dir = ar_new_opendir(fn);
//printf("Trying to open %s using arDirOpen: %s\r\n", fn, dir?"success":"failed");
if (!dir) return NULL;
ARINFO* i = new ARINFO();
i->ar = dir;
i->fn = fn;
return i;
}

static int arDirRead (ARINFO* f, playlistitem& it, int unused) {
if (!f||!f->ar) return false;
if (!ar_readdir(f->ar)) return false;
string path = ar_entryname(f->ar);
it.file = f->fn + string("?") + path;
//printf("Reading entry: %s, %s\r\n", path.c_str(), it.file.c_str());
return true;
}

static void arDirClose (ARINFO* i) {
if (!i) return;
if (i->ar) ar_close_free(i->ar);
delete i;
}

static void CALLBACK arFileClose (ARFILE* f) {
ar_close_free(f);
}

static QWORD CALLBACK arFileLength (ARFILE* f) {
return ar_entrysize(f);
}

static DWORD CALLBACK arFileRead (void* buffer, DWORD length, ARFILE* f) {
return ar_read(f, buffer, length);
}

static BOOL CALLBACK arFileSeek (QWORD pos, ARFILE* f) {
return ar_fseek(f, pos, SEEK_SET);
}

DWORD EXPORT BASS_Archive_StreamCreate (const char* fn, const char* path, DWORD flags) {
static BASS_FILEPROCS procs = { arFileClose, arFileLength, arFileRead, arFileSeek };
ARFILE* f = ar_new_open(fn, path);
//printf("Trying to open %s?%s using arFileOpen: %s\r\n", fn, path, f?"success":"failed");
if (!f) return 0;
DWORD stream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, flags, &procs, f);
if (!stream) {
ar_close_free(f);
return 0;
}
return stream;
}

static DWORD arFileOpen (const void* unused, const char* cfile, int mode, DWORD flags) {
string file = cfile;
int pos = file.find("?");
if (pos<=0 || pos>=file.size()) return 0;
string fn = file.substr(0, pos);
string path = file.substr(pos+1);
//printf("Decomosing path <%s> in <%s>, <%s>\r\n", file.c_str(), fn.c_str(), path.c_str());
return BASS_Archive_StreamCreate(fn.c_str(), path.c_str(), flags);
}

static void CALLBACK gzFileClose (void* fp) {
if (fp) gzclose(fp);
}

static QWORD CALLBACK gzFileLength (void* fp) {
return 0;
}

static DWORD CALLBACK gzFileRead (void* buffer, DWORD length, void* fp) {
return gzread(fp, buffer, length);
}

static BOOL CALLBACK gzFileSeek (QWORD pos, void* fp) {
return gzseek(fp, pos, SEEK_SET) == pos;
}

DWORD EXPORT BASS_GZ_StreamCreate (const char* fn, DWORD flags) {
static BASS_FILEPROCS procs = { gzFileClose, gzFileLength, gzFileRead, gzFileSeek };
void* fp = gzopen(fn, "rb");
//printf("Trying to open %s using GZ: %s\r\n", fn, fp&&gzdirect(fp)?"success":"failed");
if (!fp ) return 0;
if (gzdirect(fp)) { gzclose(fp); return 0; }
DWORD stream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, flags, &procs, fp);
if (!stream) return 0;
return stream;
}

static DWORD gzFileOpen (const void* unused, const char* fn, int mode, DWORD flags) {
return BASS_GZ_StreamCreate(fn, flags);
}

static const int nMax = 3;
static const QCPLUGIN data[] = {
{ PT_INPUT, PF_CAN_READ, "Compressed modules", "*.mdz;*.xmz;*.itz;*.s3z;*.umz;*.stz", arFileOpen, NULL, NULL, NULL, NULL, NULL },
{ PT_INPUT, PF_CAN_READ, "Compressed MIDI", "*.miz;*.rmz;*.midz;*.rmidz", gzFileOpen, NULL, NULL, NULL, NULL, NULL },
{ PT_PLAYLIST, PF_CAN_READ, "Archive files", "*.zip;*.tar;*.gz;*.bz2;*.tgz;*.tbz;*.iso;*.cab;*.7z;*.rar", arDirOpen, arDirRead, NULL, arDirClose, NULL, NULL },
};

extern "C" LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}
