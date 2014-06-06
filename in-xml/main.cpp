#include<windows.h>
#include<cstdio>
#include<string>
#include "tinyxml2.h"
#include "../plugin.h"
#include "../playlist.h"
#include "../strings.hpp"
using namespace std;
using namespace tinyxml2;

#define EXPORT __declspec(dllexport)

#define TYPE_XSPF 1
#define TYPE_ASX 2
#define TYPE_WPL 3

struct PFile {
XMLDocument doc;
XMLElement* cur;
string filename;
int type, mode;
};

template <typename T> static XMLElement* addSimple (XMLElement* parent, const char* elname, T val) ;

static int xmlGuessType (XMLDocument& doc, XMLElement*& el, int& type) {
if ( (el=doc.FirstChildElement("playlist")) && (el=el->FirstChildElement("trackList")) && (el=el->FirstChildElement("track")) ) type = TYPE_XSPF;
else if ((el=doc.FirstChildElement("asx")) && (el=el->FirstChildElement("entry")) ) type = TYPE_ASX;
else if ((el=doc.FirstChildElement("smil")) && (el=el->FirstChildElement("body")) && (el=el->FirstChildElement("seq")) && (el=el->FirstChildElement("media")) ) type = TYPE_WPL;
return type;
}

static void xspfStartWrite (XMLDocument& doc, XMLElement*& e) {
doc.InsertEndChild(doc.NewDeclaration());
e = doc.InsertEndChild(doc.NewElement("playlist"));
e->SetAttribute("version", 1);
e->SetAttribute("xmlns", "http://xspf.org/ns/0/");
e = e->InsertEndChild(doc.NewElement("trackList"));
}

static void asxStartWrite (XMLDocument& doc, XMLElement*& e) {
e = doc.InsertEndChild(doc.NewElement("asx"));
e->SetAttribute("version", "1.0");
}

static void wplStartWrite (XMLDocument& doc, XMLElement*& e) {
doc.InsertEndChild(doc.NewDeclaration("<?wpl version=\"1.0\"?>"));
XMLElement* root = doc.InsertEndChild(doc.NewElement("smil"));
e = root->InsertEndChild(doc.NewElement("head"));
e = e->InsertEndChild(doc.NewElement("meta"));
e->SetAttribute("name", "generator");
e->SetAttribute("content", "6player 3.1 http://quentinc.net/");
e = root->InsertEndChild(doc.NewElement("body"));
e = e->InsertEndChild(doc.NewElement("seq"));
}

static void* xmlRealOpen (const char* fn, int mode, int type) {
PFile* p = new PFile();
p->filename = fn;
p->mode = mode;
p->type = type;
p->cur = NULL;
if (mode == PF_OPEN_READ) {
if (p->doc.LoadFile(fn) || !xmlGuessType(p->doc, p->cur, p->type) ) {
delete p;
return NULL;
}}
else if (mode == PF_OPEN_WRITE) {
switch(type) {
case TYPE_XSPF : xspfStartWrite(p->doc, p->cur); break;
case TYPE_ASX: asxStartWrite(p->doc, p->cur); break;
case TYPE_WPL: wplStartWrite(p->doc, p->cur); break;
default: delete p; return NULL; break;
}}
return p;
}

template<int TYPE> static void* xmlOpen (const void* unused, const void* fn, int mode, int flags) {
if (mode==PF_OPEN_READ) return xmlRealOpen(fn, PF_OPEN_READ, 0);
else if (mode == PF_OPEN_WRITE) return xmlRealOpen(fn, PF_OPEN_WRITE, TYPE);
else return NULL;
}

static void xmlClose (PFile* p) {
if (!p) return;
if (p->mode == PF_OPEN_WRITE) p->doc.SaveFile(p->filename.c_str());
delete p;
}

static inline int parseInt (const char* str, int def) {
if (!str) return def;
int i;
if (sscanf(str, "%d", &i)) return i;
return def;
}

static inline int parseDouble (const char* str, double def) {
if (!str) return def;
double i;
if (sscanf(str, "%.14g", &i)) return i;
return def;
}

static inline bool parseBool (const char* str, bool def) {
if (!str) return def;
if (!stricmp(str, "true") || !stricmp(str, "on") || !strcmp(str, "1")) return true;
else if (!stricmp(str, "false") || !stricmp(str, "off") || !strcmp(str, "0")) return false;
else return def;
}

static bool xspfRead (XMLDocument& doc, XMLElement*& cur, playlistitem& it) {
if (!cur) return false;
XMLElement* e = NULL;
if (e = cur->FirstChildElement("location")) it.file = e->GetText();
if (e=cur->FirstChildElement("title")) it.title = e->GetText();
if (e = cur->FirstChildElement("artist")) it.artist = e->GetText();
if (e = cur->FirstChildElement("album")) it.album = e->GetText();
if (e=cur->FirstChildElement("duration")) it.length = parseInt(e->GetText(), -1000) /1000.0;
cur = cur->NextSiblingElement("track");
return !it.file.empty();
}

static bool asxRead (XMLDocument& doc, XMLElement*& cur, playlistitem& it) {
if (!cur) return false;
XMLElement* e = NULL;
if ((e = cur->FirstChildElement("ref")) && e->Attribute("href")) it.file = e->Attribute("href");
if (e=cur->FirstChildElement("title")) it.title = e->GetText();
if ((e=cur->FirstChildElement("duration")) || (e=cur->FirstChildElement("DURATION")) ) {
int h=0, m=0, s=0, r = sscanf(e->GetText(), "%02d:%02d:%02d", &h, &m, &s);
if (r==1) { s=h; h=m=0; }
else if (r==2) { s=m; m=h; h=0; }
it.length = h*3600 +m*60 +s;
}
cur = cur->NextSiblingElement("entry");
return !it.file.empty();
}

static bool wplRead (XMLDocument& doc, XMLElement*& cur, playlistitem& it) {
if (!cur) return false;
if (cur->Attribute("src")) it.file = cur->Attribute("src");
cur = cur->NextSiblingElement("media");
return !it.file.empty();
}

static int xmlRead (PFile* p, playlistitem& it) {
if (!p) return false;
switch(p->type) {
case TYPE_XSPF : return xspfRead(p->doc, p->cur, it);
case TYPE_ASX : return asxRead(p->doc, p->cur, it);
case TYPE_WPL : return wplRead(p->doc, p->cur, it);
default: return false;
}}

static inline XMLText* createTextNode (XMLDocument& doc, const char* val) { return doc.NewText(val); }
static inline XMLText* createTextNode (XMLDocument& doc, bool val) { return doc.NewText(val?"true":"false"); }
static inline XMLText* createTextNode (XMLDocument& doc, int i) { 
char val[50]={0};
snprintf(val, 49, "%d", i);
return doc.NewText(val); 
}
static inline XMLText* createTextNode (XMLDocument& doc, double i) { 
char val[50]={0};
snprintf(val, 49, "%.14g", i);
return doc.NewText(val); 
}

template <typename T> static XMLElement* addSimple (XMLElement* parent, const char* elname, T val) {
XMLDocument& doc = const_cast<XMLDocument&>(*parent->GetDocument());
XMLElement* e = doc.NewElement(elname);
e->InsertEndChild(createTextNode(doc, val));
parent->InsertEndChild(e);
return e;
}

static bool xspfWrite (PFile* p, const playlistitem& it) {
XMLElement *e = p->doc.NewElement("track");
addSimple(e, "location", it.file.c_str());
if (!it.title.empty()) addSimple(e, "title", it.title.c_str());
if (!it.artist.empty()) addSimple(e, "artist", it.artist.c_str());
if (!it.album.empty()) addSimple(e, "album", it.album.c_str());
if (it.length>0) addSimple(e, "duration", (int)(1000 * it.length));
p->cur->InsertEndChild(e);
return true;
}

static bool asxWrite (PFile* p, const playlistitem& it) {
XMLElement *e = p->doc.NewElement("entry");
((XMLElement*)(e->InsertEndChild(p->doc.NewElement("ref")))) ->SetAttribute("href", it.file.c_str());
if (!it.title.empty()) addSimple(e, "title", it.title.c_str());
if (it.length>0) {
char buf[15];
snprintf(buf, 14, "%02d:%02d:%02d.00", (int)(it.length/3600), (int)(it.length/60)%60, (int)(it.length)%60);
addSimple(e, "duration", buf);
}
p->cur->InsertEndChild(e);
return true;
}

static bool wplWrite (PFile* p, const playlistitem& it) {
XMLElement *e = p->doc.NewElement("media");
e->SetAttribute("src", it.file.c_str());
p->cur->InsertEndChild(e);
return true;
}

static int xmlWrite (PFile* p, const playlistitem& it) {
if (!p) return false;
switch(p->type){
case TYPE_XSPF : return xspfWrite(p, it);
case TYPE_ASX : return asxWrite(p, it);
case TYPE_WPL : return wplWrite(p, it);
default : return false;
}}


static int asxGet (PFile* fp, int what, void* buf, int len) {
static const char* defex = "asx";
switch(what) {
case PP_DEFEX: return defex;
default: return false;
}}

static int wplGet (PFile* fp, int what, void* buf, int len) {
static const char* defex = "wpl";
switch(what) {
case PP_DEFEX: return defex;
default: return false;
}}

static int xspfGet (PFile* fp, int what, void* buf, int len) {
static const char* defex = "xspf";
switch(what) {
case PP_DEFEX: return defex;
default: return false;
}}

static const int nMax = 3;
static const QCPLUGIN data[] = {
{ PT_PLAYLIST, PF_CAN_READ | PF_CAN_WRITE, "XML Shareable playlist format", "*.xspf", xmlOpen<TYPE_XSPF>, xmlRead, xmlWrite, xmlClose, xspfGet, NULL },
{ PT_PLAYLIST, PF_CAN_READ | PF_CAN_WRITE, "Advanced stream redirector", "*.asx", xmlOpen<TYPE_ASX>, xmlRead, xmlWrite, xmlClose, asxGet, NULL },
{ PT_PLAYLIST, PF_CAN_READ | PF_CAN_WRITE, "Windows media player playlist", "*.wpl", xmlOpen<TYPE_WPL>, xmlRead, xmlWrite, xmlClose, wplGet, NULL },
};

extern "C" LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}
