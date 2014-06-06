#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../strings.hpp"
#include "../plugin.h"
#include "../playlist.h"
#include<string>
#include<fstream>
using namespace std;

#define EXPORT __declspec(dllexport)

struct PLS {
string line;
union { ios* ios; ofstream* out; ifstream* in; };
union { bool reserve; int count; };
PLS(ofstream* o): out(o), count(0)  {}
PLS(ifstream* i): in(i), reserve(false)  {}
~PLS () { delete ios; }
};

static inline bool checkExtension (string str, const string ext) {
toLowerCase(str);
bool re = str.substr(str.length() -ext.size()) == ext;
return re;
}

static void* m3uOpen (const void* unused, const char* fn, int mode, int flags) {
if (mode==PF_OPEN_READ) {
if (!checkExtension(fn, ".m3u")) return 0;
ifstream* fp = new ifstream(fn);
if (fp&&*fp) return fp;
else delete fp;
}
else if (mode==PF_OPEN_WRITE) {
ofstream* fp = new ofstream(fn);
if (fp&&*fp) return fp;
else delete fp;
}
return 0;
}

static void m3uClose (ios* fp) {
delete fp;
}

static int m3uRead (ifstream& in, playlistitem* it, int unused) {
string str;
if (!getline(in,str)) return false;
else if (str.size()<2 || str[0]==';' || str[0]=='#') return m3uRead(in, it, unused);
trim(str);
it->file = str;
return true;
}

static int m3uWrite (ofstream& out, const playlistitem* it, int unused) {
out << it->file << endl;
}

static PLS* plsWriteHeader (PLS* pls, int nItems) {
if (!pls) return 0;
*pls->out << "[Playlist]" << endl;
return pls;
}

static void* plsOpen (const void* unused, const char* fn, int mode, int nItems) {
if (mode==PF_OPEN_READ) {
if (!checkExtension(fn, ".pls")) return 0;
ifstream* fp = new ifstream(fn);
if (fp&&*fp) return new PLS(fp);
else delete fp;
}
else if (mode==PF_OPEN_WRITE) {
ofstream* fp = new ofstream(fn);
if (fp&&*fp) return plsWriteHeader(new PLS(fp), nItems);
else delete fp;
}
return 0;
}

static bool plsWriteFooter (PLS* pls) {
if (!pls) return false;
*pls->out << "NumberOfEntries=" << pls->count << endl;
*pls->out << "Version=2" << endl;
return true;
}

static void plsClose (PLS* pls) {
plsWriteFooter(pls);
delete pls;
}

static bool plsRealRead (PLS* pls, string& str) {
if (!pls) return false;
if (!getline(*pls->in, str)) return false;
trim(str);
if (str.size()<2 || str[0]==';' || str[0]=='#') return plsRealRead(pls, str);
return true;
}

static int plsRead (PLS* pls, playlistitem* it) {
if (!pls) return false;
#define C(n) it->n.clear();
C(file) C(title) C(artist) C(album) C(genre)
C(tracknum) C(disc) C(copyright) C(comment)
#undef C
bool fileFound = false;
string name, value;
string& line = pls->line;
while (pls->reserve || plsRealRead(pls, line)) { 
pls->reserve = false;
if (!split(line, '=', name, value)) continue;
trim(name); trim(value);
if (!strnicmp(name.c_str(), "file", 4)) {
if (fileFound) {
pls->reserve=TRUE; 
return true;
}
else {
it->file = value;
fileFound=TRUE;
}}
else if (!strnicmp(name.c_str(), "title", 5)) it->title = value;
else if (!strnicmp(name.c_str(), "artist", 6)) it->artist = value;
else if (!strnicmp(name.c_str(), "album", 5)) it->album = value;
else if (!strnicmp(name.c_str(), "genre", 5)) it->genre = value;
else if (!strnicmp(name.c_str(), "subtitle", 8)) it->subtitle = value;
else if (!strnicmp(name.c_str(), "length", 6)) it->length = toDouble(value);
else if (!strnicmp(name.c_str(), "composer", 8)) it->composer = value;
else if (!strnicmp(name.c_str(), "year", 4)) it->year = value;
else if (!strnicmp(name.c_str(), "disc", 4)) it->disc = value;
else if (!strnicmp(name.c_str(), "tracknum", 8)) it->tracknum = value;
else if (!strnicmp(name.c_str(), "copyright", 9)) it->copyright = value;
else if (!strnicmp(name.c_str(), "comment", 7)) it->comment = value;
else if (strnicmp(name.c_str(), "metadataSet", 11)) it->metadataSet = toBool(value);
}
return fileFound;
}

static int plsWrite (PLS* pls, const playlistitem* it) {
if (!pls || it->file.empty()) return false;
pls->count++;
#define P(n) if (!it->n.empty()) *pls->out << #n << pls->count << '=' << it->n << endl;
P(file) P(title)
if (it->length>0) *pls->out << "length" << pls->count << '=' << it->length << endl;
P(artist) P(album) P(genre)
P(subtitle) P(composer) P(year)
P(disc) P(tracknum)
P(copyright) P(comment)
if (it->metadataSet) *pls->out << "metadataSet" << pls->count << '=' << boolalpha << it->metadataSet << endl;
#undef P
return true;
}

static int m3uGet (void* fp, int what, void* buf, int len) {
static const char* m3uDefex = "m3u";
switch(what) {
case PP_DEFEX: return m3uDefex;
default: return false;
}}

static int plsGet (void* fp, int what, void* buf, int len) {
static const char* plsDefex = "pls";
switch(what) {
case PP_DEFEX: return plsDefex;
default: return false;
}}

static const int nMax = 2;
static const QCPLUGIN data[] = {
{ PT_PLAYLIST, PF_CAN_READ | PF_CAN_WRITE, "PLS Playlist", "*.pls", plsOpen, plsRead, plsWrite, plsClose, plsGet, NULL },
{ PT_PLAYLIST, PF_CAN_READ | PF_CAN_WRITE, "M3U Playlist", "*.m3u", m3uOpen, m3uRead, m3uWrite, m3uClose, m3uGet, NULL },
};

extern "C" LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}
