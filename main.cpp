#include "consts.h"
#include "strings.hpp"
#include "IniFile.hpp"
#include <windows.h>
#include<prsht.h>
#include<winsock2.h>
#include<commctrl.h>
#include <shlobj.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<map>
#include<vector>
#include<algorithm>
#include<set>
#include<string>
#include<cwchar>
#include<exception>
#include<typeinfo>
#include<cstring>
#include<cstdio>
#include<cstdlib>
#include<ctime>
#include <cmath>
#include "alphanum.hpp"
#include "bass.h"
#include "basswinamp.h"
#include "bass_vst.h"
#include "bassmidi.h"
#include "bass_fx.h"
#include "bassenc.h"
#include "bassmix.h"
#include "bass_plus/bass_plus.h"
#include "tags.h"
#include "plugin.h"
#include "playlist.h"
#include "Socket.hpp"
using namespace std;

// definitions
#define BASS_INITFLAGS 0
#define BASS_STREAMFLAGS BASS_STREAM_PRESCAN | BASS_STREAM_DECODE | BASS_MIDI_DECAYEND  | BASS_MIDI_DECAYSEEK | BASS_MIDI_SINCINTER | BASS_SAMPLE_FLOAT | USE_UNICODE
#define BASS_FXFLAGS BASS_SAMPLE_FLOAT  


typedef struct {
HWND hdlg;
bool cancelled;
TCHAR* title;
} progressinfo;

typedef struct {
int outDevice, inDevice, jingleDevice, feedbackDevice, previewDevice;
bool useJingles, castNoFeedback, castAutoTitle;
} optionsinfo;

typedef struct {
DWORD type;
DWORD curTarget;
HFX hfx;
char data[116];
} effectinfo;

typedef struct {
int type, port, format, bitrate;
tstring host, password, mimetype, name, url, genre, desc;
} castinfo;

typedef BOOL(*__stdcall SAYSTRINGFUNC)(LPCTSTR,BOOL);

// WinAPI specific definitions
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK labelSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR reserved1, DWORD_PTR reserved2);
DWORD HTTPServerTProc (LPVOID lp);

const TCHAR szClassName[ ] = TEXT("QCPLAYER2");
HINSTANCE hinst;
HWND win=NULL, lbl=NULL, playlistdlg=NULL, mididlg=NULL, textdlg=NULL, levelsdlg=NULL, radiodlg = NULL, btnPlay=NULL, btnPrev=NULL, btnNext=NULL;
HMENU lect, exportmenu, effectsMenu;
HACCEL haccel;
tstring filter;
int filterIndex = 0;

// Functions
void executeCustomCommand (int nCmd);
void TransferStdinToSocket (int port) ;
DWORD parseArgs (const TCHAR** argv, int argc, bool startup) ;
void fillOutDevices (unordered_map<int,string>* i1, unordered_map<string,int>* i2) ;
void fillInDevices (unordered_map<int,string>* i1, unordered_map<string,int>* i2);
int findDevice (unordered_map<string,int>& m, string name, int def) ;
string findDevice (unordered_map<int,string>& m, int n, string def) ;
bool setOutDevice (int index) ;
void freeOutDevice (int index) ;
void CALLBACK introModeTimeout (HSYNC handle, DWORD chan, DWORD param, LPVOID udata) ;
char* strtok2 (char*, char) ;
tstring formatTime (int seconds) ;
tstring formatSize (long long size);
long long getFileSize (tstring file) ;
void randinit (void) ;
int randint (int max) ;
void keyDown (int);
void keyUp (int) ;
void action (int);
int parseExternalData (COPYDATASTRUCT*);
void updateWindowTitle () ;
void addFileType (const string&, const string&, int);
void addPlaylistWriteFileType (const string&, const string&);
void addEncodeFileType(const string&, const string&);
TCHAR* inputDialog (const TCHAR* title, const TCHAR* prompt, const TCHAR* text) ;
void gotoTimeDialog () ;
void aboutDialog () ;
void optionsDialog (int=0) ;
void openDialog (BOOL);
void openUrlDialog (BOOL) ;
void openDirDialog (BOOL) ;
void savePlaylistDialog () ;
void saveEncodeDialog () ;
void fileInformationDialog (int) ;
void showPlaylist () ;
int appendToPlaylist (const TCHAR* fn) ;
template<class T> int appendToPlaylist (const basic_string<T>& fn) ;
int appendFileToPlaylist (const string& file) ;
void appendDirectoryToPlaylist (const tstring& url) ;
bool savePlaylist (LPQCPLUGIN, const tstring&) ;
int loadPlaylist (LPQCPLUGIN, const string&) ;
void clearPlaylist (int mode=0, string filter="");
void sortPlaylist (int mode) ;
void doIndexAllPlaylist () ;
DWORD CALLBACK indexAllPlaylist (LPVOID) ;
BOOL isdir (const tstring& fn) ;
void startSongPreview (string fn) ;
bool stopSongPreview () ;
void startJingle (string fn) ;
BOOL loadSong (const string&) ;
void playSong (BOOL) ;
void stopSong (bool fade = true) ;
void pauseSong () ;
void nextSong () ;
void prevSong () ;
void playSongNum (int) ;
void delSongNum (int) ;
void playlistSwap(int,int, int, int);
void setPreviewSongVol (float) ;
void setPitch (DWORD, float) ;
float getPitch (DWORD) ;
float setVol (DWORD, float) ;
float getVol (DWORD) ;
void setSongVol (float) ;
void setRecVol (float) ;
void setCastVol (float) ;
void setRecCastVol (float) ;
void setSongSpeed (float ) ;
void setSongPitch (float) ;
void setSongRate (float) ;
void seekSong (double pos, BOOL rel=TRUE);
void seekPreviewSong (double pos, BOOL rel=TRUE);
void setEqualizerValue (DWORD, DWORD,float );
void setEqualizerValueDelta (DWORD, DWORD,float );
bool startEffect (effectinfo& e) ;
void stopEffect (effectinfo& e) ;
void startRecord () ;
void stopRecord () ;
void startEncode (void);
void startMix (void) ;
void stopMix (void) ;
void updateInfo () ;
DWORD WINAPI encodeautoproc (LPVOID);
tstring getPlaylistItemText (int index) ;
bool acceptPlaylistItem (const playlistitem& it, const string& flt) ;
playlistitem& fillMetadata (playlistitem&, DWORD) ;
DWORD MIDIGetCurrentInstrument (DWORD handle, int channel);
const char* MIDIGetCurrentInstrumentName (DWORD handle, int channel);
void MIDIFillInstrumentsList(map<DWORD,string>&);
DWORD getStream (string, DWORD);
bool acceptRadio (const radio& r, const string& flt) ;
void luaScriptingInit (void) ;

// Global variables
SAYSTRINGFUNC sayString = NULL;
bool shiftDown = false, ctrlDown = false;
DWORD hotkeyKey=0, hotkeyCount=0, hotkeyAction=0, hotkeySub=0;
tstring appdir, curdir;
vector<HPLUGIN> bassPlugins;
vector<LPQCPLUGIN> plugins, encoders, exporters, taggers, playlistWriters;
vector< pair<string, string>  > formats;
vector< pair<string, string> > playlistWriteFormats;
vector< pair<string, string> > encodeFormats;
unordered_map<int,string> typeMap;
vector<effectinfo> effects;
vector<playlistitem> playlist;
map<DWORD,string> midiInstruments;
IniFile ini;
DWORD curHandle = 0, curMixHandle=0, curRecord = 0, curFeedback=0, curFeedback2=0, curEncode=0, curCopyHandle=0, curPreviewHandle=0;
int curDecode = -1, curCasting=-1;
double curStreamLen = 0, curLoopStart=0;
string curDecodeFn, curDecodeDir;
BOOL ignoreUpdate=FALSE, keepDecode = FALSE, useJingles=false, castNoFeedback=true, castAutoTitle = TRUE;
bool playlistRevDir = FALSE, initialized = FALSE;
HFX eqHfx[5]={0,0,0,0,0};
DWORD bInitedDevices=0;
int inDevice= -1, outDevice=1, previewDevice=1, feedbackDevice=1, jingleDevice=1;
int deviceSwitchCount=0;
int crossFade = -6000, fxTarget=1;
bool curLoop = FALSE, curRandom = false, curReverse = FALSE, curIntroMode = FALSE;
double curVol = 0.35f, curRecVol=1.0f, curCastVol=1.0f, curRecCastVol=1.0f;
double curSpeed = 1.0f;
double curTransposition = 0;
double curRate = 1.0f;
int curSong = -1;
float eqGains[5]={0,0,0,0,0};
float eqFreqs[5] = { 300, 700, 1500, 3200, 7000 };
float eqBandwidths[5] = { 18, 18, 18, 18, 18 };

static const BASS_PLUGINFORM baseFormats[] = {
{ BASS_CTYPE_STREAM_WAV_PCM, "Wave", "*.wav" }, 
{ BASS_CTYPE_STREAM_MP1, "MPEG1 Layer 1 (MP1)", "*.mp1" },
{ BASS_CTYPE_STREAM_MP2, "MPEG1 Layer 2 (MP2)", "*.mp2" },
{ BASS_CTYPE_STREAM_MP3, "MPEG1 Layer 3 (MP3)", "*.mp3" },
{ BASS_CTYPE_STREAM_OGG, "OGG Vorbis", "*.ogg" },
{ BASS_CTYPE_STREAM_AIFF, "Audio interchange file format", "*.aif;*.aiff" },
{ BASS_CTYPE_MUSIC_MOD, "Pro Tracker Module", "*.mod" },
{ 0, "ScreamTracker2 Module", "*.stm" },
{ BASS_CTYPE_MUSIC_S3M, "ScreamTracker3 Module", "*.s3m" },
{ BASS_CTYPE_MUSIC_XM, "FastTracker II Module", "*.xm" },
{ BASS_CTYPE_MUSIC_MTM, "MultiTracker Module", "*.mtm;*.nst" },
{ BASS_CTYPE_MUSIC_IT, "ImpulseTracker Module", "*.it" },
{ 0, "Unreal extended Module", "*.umx" },
{ BASS_CTYPE_MUSIC_MO3 | BASS_CTYPE_MUSIC_MOD, "Mo3 Module", "*.mo3" },
{ 0, NULL, NULL }
};

template<typename T> inline T minmax (T a, T b, T c) {
if (b<a) return a;
else if (b>c) return c;
else return b;
}

inline bool IsQCSpecialMessage (const MSG& msg) {
if (msg.message!=WM_KEYDOWN && msg.message!=WM_KEYUP && msg.message!=WM_CHAR) return true;
return (msg.wParam==VK_TAB || msg.wParam==VK_SPACE || msg.wParam==VK_RETURN) ;
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nFunsterStil)
{
try {
hinst = hThisInstance;

const wchar_t* lpszArgumentW = GetCommandLineW();
HWND hw = FindWindow(szClassName, NULL);
if (hw!=NULL) {
COPYDATASTRUCT cp;
cp.dwData = 33;
cp.cbData = sizeof(TCHAR)*(wcslen(lpszArgumentW) +1);
cp.lpData = lpszArgumentW;
int result = SendMessage(hw, WM_COPYDATA, NULL, (LPARAM)(&cp));
if (result==2) SetForegroundWindow(hw);
else if (result==0xFFFE) TransferStdinToSocket(123);
if (result) return 0;
}

randinit();
appdir.clear(); appdir.reserve(400);
curdir.clear(); curdir.reserve(400);
stringSize(appdir) = GetModuleFileName(NULL,  appdir.data(), appdir.capacity() -1);
stringSize(curdir) = GetCurrentDirectory(curdir.capacity() -1, curdir.data());
appdir = appdir.substr(0, appdir.rfind('\\'));

   WNDCLASSEX wincl;
   wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    if (!RegisterClassEx (&wincl)) return 1;
INITCOMMONCONTROLSEX ccex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES |  ICC_HOTKEY_CLASS | ICC_PROGRESS_CLASS | ICC_UPDOWN_CLASS | ICC_TAB_CLASSES  };
if (!InitCommonControlsEx(&ccex)) return 1;

HMENU hmb = LoadMenu(hinst, TEXT("menu"));
lect = GetSubMenu(hmb,1);
effectsMenu = GetSubMenu(hmb,3);
exportmenu = GetSubMenu(GetSubMenu(hmb,2),3);

win = CreateWindowEx (
           0,                               szClassName,
           TEXT("6player"),
           WS_OVERLAPPEDWINDOW,
           CW_USEDEFAULT,                  CW_USEDEFAULT, 544, 370,
           HWND_DESKTOP, hmb,
           hThisInstance,                   NULL                             );

lbl = CreateWindowEx(0, TEXT("STATIC"), TEXT("Loading..."),
WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX | WS_TABSTOP,
5, 295, 539, 40,
win, (HMENU)IDL_INFO,
hinst, NULL);
SetWindowSubclass(lbl, (SUBCLASSPROC)labelSubclassProc, 0, 0);

btnPlay = CreateWindowEx(0, TEXT("BUTTON"), TEXT(">|"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
5, 5, 60, 40,
win, (HMENU)IDM_PAUSE, hinst, NULL);
btnPrev = CreateWindowEx(0, TEXT("BUTTON"), TEXT("<<"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
75, 5, 60, 40,
win, (HMENU)IDM_PREV, hinst, NULL);
btnNext = CreateWindowEx(0, TEXT("BUTTON"), TEXT(">>"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
140, 5, 60, 40,
win, (HMENU)IDM_NEXT, hinst, NULL);

RegisterHotKey(win, IDM_PAUSE, 0, 0xA9);
RegisterHotKey(win, IDM_PAUSE, 0, 0xB3);
RegisterHotKey(win, IDM_PAUSE, 0, 0xB2);
RegisterHotKey(win, IDM_NEXT, 0, 0xA7);
RegisterHotKey(win, IDM_NEXT, 0, 0xB0);
RegisterHotKey(win, IDM_PREV, 0, 0xA6);
RegisterHotKey(win, IDM_PREV, 0, 0xB1);
    ShowWindow (win, SW_SHOW); //nFu	nsterStil);
SetFocus(lbl);
SetTimer(win, 1, 200, NULL);
haccel = LoadAccelerators(hThisInstance, TEXT("accel"));

SetCurrentDirectory(appdir.c_str());
{ 
HINSTANCE hdll = LoadLibrary(TEXT("UniversalSpeech.dll"));
if (hdll) {
sayString = (SAYSTRINGFUNC)GetProcAddress(hdll, "speechSay");
}}

int savedSongPos = -2; long long savedSongNum = -2; string savedSongFile = "";
{
unordered_map<string,int> inDevices, outDevices;
BASS_SetConfig(BASS_CONFIG_UNICODE, TRUE);
BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT,TRUE);
fillOutDevices(NULL, &outDevices);
fillInDevices(NULL, &inDevices);
ini.load("player.conf");
bool crossFadeOn = ini.get("play.crossfade", false);
crossFade = (crossFadeOn? 1 : -1) * ini.get("play.crossfade.time", 6000);
fxTarget = ini.get("fx.target", 1);
curLoop = ini.get("play.loop", true);
curRandom = ini.get("play.random", false);
curReverse = ini.get("play.reverse", false);
curIntroMode = ini.get("play.introMode", false);
curVol = ini.get("play.volume", curVol);
curRecVol = ini.get("play.recVolume", curRecVol);
curCastVol = ini.get("casting.outVolume", curCastVol);
curRecCastVol = ini.get("casting.recVolume", curRecCastVol);
curSpeed = ini.get("play.speed", 1.0);
curTransposition = ini.get("play.pitch", 0.0);
curRate = ini.get("play.rate", 1.0);
savedSongNum = ini.get("cur.num", -1);
savedSongPos = ini.get("cur.pos", -1);
savedSongFile = ini.get<string>("cur.file", "");
outDevice = findDevice(outDevices, ini.get<string>("devices.output", ""), 1);
feedbackDevice = findDevice(outDevices, ini.get<string>("devices.micfeedback", ""), 1);
previewDevice = findDevice(outDevices, ini.get<string>("devices.preview", ""), 1);
jingleDevice = findDevice(outDevices, ini.get<string>("devices.jingleOutput", ""), 1);
inDevice = findDevice(inDevices, ini.get<string>("devices.input", ""), -1);
useJingles = ini.get("casting.jingles", false);
castNoFeedback = ini.get("casting.noFeedback",true);
castAutoTitle = ini.get("casting.autoUpdateTitle", true);
}

if ( !setOutDevice(outDevice) && !setOutDevice(-1) ) {
MessageBox(win, TEXT("Couldn't initialize bass library"), TEXT("Fatal error"), MB_OK | MB_ICONERROR);
PostQuitMessage(1);
exit(1);
}
if (outDevice<=0) outDevice = BASS_GetDevice();

BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);
BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
BASS_SetConfig(BASS_CONFIG_MUSIC_VIRTUAL, 256);
BASS_FX_GetVersion();

{
WIN32_FIND_DATA fd;
memset(&fd, 0, sizeof(fd));
HANDLE hfd = FindFirstFile(TEXT("*.dll"), &fd);
if (hfd) do {
tstring str = fd.cFileName;
if (str==TEXT("bass.dll") || str==TEXT("bass_fx.dll") || str==TEXT("bassmix.dll")) continue;
if (startsWith(str, TEXT("bass"))) {
HPLUGIN plug = BASS_PluginLoad((const char*)(str.c_str()), USE_UNICODE);
if (plug) bassPlugins.push_back(plug);
}
else if (startsWith(str, TEXT("in-")) || startsWith(str, TEXT("enc-")) || startsWith(str, TEXT("exp-")) || startsWith(str, TEXT("tag-"))) {
HINSTANCE dll = LoadLibrary(str.c_str());
if (!dll) continue;
QCPLUGINPROC getData = (QCPLUGINPROC)GetProcAddress(dll, QCPLUGINPROCNAMESTR);
if (!getData) { FreeLibrary(dll); continue; }
LPQCPLUGIN p = NULL;
for (int i=0; (p=getData(i)); i++) {
switch(p->type) {
case PT_INPUT :
plugins.push_back(p);
break;
case PT_PLAYLIST :
if ((p->flags&PF_CAN_READ)) plugins.push_back(p);
if ((p->flags&PF_CAN_WRITE) && p->get) playlistWriters.push_back(p);
break;
case PT_ENCODER :
encoders.push_back(p);
break;
case PT_EXPORTER :
exporters.push_back(p);
break;
case PT_TAGGER :
taggers.push_back(p);
break;
default: break;
}}}
// other types of plugins
} while (FindNextFile(hfd, &fd));
if (hfd) FindClose(hfd);
}

filter.clear();
for (const BASS_PLUGINFORM* frm = baseFormats; frm->name; frm++) addFileType(frm->name, frm->exts, frm->ctype);
typeMap[0x50002] = typeMap[BASS_CTYPE_STREAM_WAV_FLOAT] = typeMap[BASS_CTYPE_STREAM_WAV_PCM];
typeMap[BASS_CTYPE_MUSIC_MO3 | BASS_CTYPE_MUSIC_XM] = typeMap[BASS_CTYPE_MUSIC_MO3 | BASS_CTYPE_MUSIC_IT] = typeMap[BASS_CTYPE_MUSIC_MO3 | BASS_CTYPE_MUSIC_S3M] = typeMap[BASS_CTYPE_MUSIC_MO3 | BASS_CTYPE_MUSIC_MTM] = typeMap[BASS_CTYPE_MUSIC_MO3] = typeMap[BASS_CTYPE_MUSIC_MO3 | BASS_CTYPE_MUSIC_MOD];
typeMap[BASS_CTYPE_STREAM_MF] = "Media foundation codec";
for (int i=0; i<bassPlugins.size(); i++) {
const BASS_PLUGININFO* info = BASS_PluginGetInfo(bassPlugins[i]);
if (info==NULL) continue;
for (int j=0; j < info->formatc; j++) {
const BASS_PLUGINFORM frm = info->formats[j];
addFileType(frm.name, frm.exts, frm.ctype);
}}
for (LPQCPLUGIN p: plugins) {
if (p->desc && p->exts) addFileType(p->desc, p->exts, 0);
}
for (LPQCPLUGIN p: playlistWriters) {
if (p->desc && p->exts) addPlaylistWriteFileType(p->desc, p->exts);
}
for (int i=0, n=encoders.size(); i<n; i++) {
LPQCPLUGIN p = encoders[i];
const char *desc = NULL, *ext = NULL;
if (p->desc && p->exts) {
addEncodeFileType(p->desc, p->exts);
tstring desc2; desc2.clear(); desc2 += toTString(p->desc); desc2 += TEXT("...");
AppendMenu(exportmenu, MF_STRING, IDM_ENCODEALL +i, desc2.c_str());
}}

{ 
ifstream fp("effects.conf");
if (fp) {
effectinfo e;
memset(&e, 0, sizeof(e));
e.hfx=0;
int count = -1;
string line;
while(getline(fp,line)) {
trim(line);
if (line.size()<2 || !line[0] || line[0]=='#' || line[0]==';') continue;
if (line[0]=='[') {
int pos = line.find(']');
if (pos<0 || pos>=line.size()) continue;
if (count++>=0) effects.push_back(e);
memset(&e, 0, sizeof(e));
e.hfx=0;
line = line.substr(1, pos -1);
trim(line);
AppendMenu(effectsMenu, MF_STRING, IDM_EFFECT+count, toTString(line).c_str());
}
else if (count>=0) {
string name, value;
if (!split(line, '=', name, value)) continue;
if (name=="type") e.type = toUnsignedInt(value, 0);
else if (name=="vst") {
e.type = 0x8000000;
memset(e.data+84, 0, 32);
strncpy(e.data+84, value.c_str(), 31);
*(int*)(e.data+80) = -1;
for (int i=0; i<20; i++) *(float*)(e.data +4*i) = -1;
}
else if (name=="preset") *(int*)(e.data +80) = toInt(value);
else if (name[0]=='f') {
int offset = (toUnsignedInt(name.substr(1), 0) -1)*4;
float val = toFloat(value);
*(float*)(&(e.data[offset])) = val;
}
else if (name[0]=='i') {
int offset = (toUnsignedInt(name.substr(1), 0) -1)*4;
int val = toInt(value, 0);
*(int*)(&(e.data[offset])) = val;
}
}}
if (count>=0) effects.push_back(e);
}}

if (playlistWriters.size()>0) loadPlaylist(playlistWriters[0], string("player.") + playlistWriters[0]->getString(&(playlistWriters[0]), PP_DEFEX, 0, 0));

if (ini.get("g.scripting",true)) luaScriptingInit();

int argc = 0;
wchar_t** argv = CommandLineToArgvW(lpszArgumentW,&argc);
parseArgs(argv, argc, true);

if (ini.get("g.reloadLast",true) && savedSongNum>=0 && savedSongNum<playlist.size() && playlist[savedSongNum].file==savedSongFile) {
playSongNum(savedSongNum);
BASS_ChannelSetPosition(curHandle, savedSongPos, BASS_POS_BYTE);
}
else if (playlist.size()>0) nextSong();

RemoveMenu(exportmenu, 0, MF_BYPOSITION);
CheckMenuItem(lect, IDM_LOOP, MF_BYCOMMAND | (curLoop? MF_CHECKED : MF_UNCHECKED));
CheckMenuItem(lect, IDM_RANDOM, MF_BYCOMMAND | (curRandom? MF_CHECKED : MF_UNCHECKED));
CheckMenuItem(lect, IDM_REVERSE, MF_BYCOMMAND | (curReverse? MF_CHECKED : MF_UNCHECKED));
CheckMenuItem(lect, IDM_INTRO_MODE, MF_BYCOMMAND | (curIntroMode? MF_CHECKED : MF_UNCHECKED));
CheckMenuItem(lect, IDM_CROSSFADE, MF_BYCOMMAND | (crossFade>0? MF_CHECKED : MF_UNCHECKED));
CheckMenuRadioItem(GetSubMenu(effectsMenu,0), 0, 3, fxTarget -1, MF_BYPOSITION);

SetCurrentDirectory(curdir.c_str());
if (ini.get("webserver.on",true)) CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HTTPServerTProc, (LPVOID)(0), 0, NULL);
initialized = TRUE;

MSG msg;
    while (GetMessage (&msg, NULL, 0, 0))
    {
if (TranslateAccelerator(win, haccel, &msg)) continue;
if (playlistdlg && IsDialogMessage(playlistdlg, &msg)) continue;
if (radiodlg && IsDialogMessage(radiodlg, &msg)) continue;
if (mididlg && IsDialogMessage(mididlg, &msg)) continue;
if (textdlg && IsDialogMessage(textdlg, &msg)) continue;
if (levelsdlg && IsDialogMessage(levelsdlg, &msg)) continue;
if (IsQCSpecialMessage(msg) && IsDialogMessage(win,&msg)) continue;
TranslateMessage(&msg);
DispatchMessage(&msg);
    }
KillTimer(win, ID_TIMER);
SetCurrentDirectory(appdir.c_str());

savedSongNum = curSong;
if (playlist.size()>0) savedSongFile = playlist[curSong].file;
if (curHandle && BASS_ChannelIsActive(curHandle)==BASS_ACTIVE_PLAYING) savedSongPos = BASS_ChannelGetPosition(curHandle, BASS_POS_BYTE);
else savedSongPos = -1;

for (int i=0; i<bassPlugins.size(); i++) BASS_PluginFree(bassPlugins[i]);
for (int i=0; i<30; i++) freeOutDevice(i);

{
unordered_map<int,string> inDevices, outDevices;
fillInDevices(&inDevices, NULL);
fillOutDevices(&outDevices, NULL);
ini.put("play.crossfade", crossFade>0);
ini.put("play.crossfade.time", abs(crossFade));
ini.put("fx.target", fxTarget);
ini.put("play.loop", curLoop);
ini.put("play.random", curRandom);
ini.put("play.reverse", curReverse);
ini.put("play.introMode", curIntroMode);
ini.put("play.volume", curVol);
ini.put("play.recVolume", curRecVol);
ini.put("casting.outVolume", curCastVol);
ini.put("casting.recVolume", curRecCastVol);
ini.put("play.speed", curSpeed);
ini.put("play.pitch", curTransposition);
ini.put("play.rate", curRate);
ini.put("cur.num", savedSongNum);
ini.put("cur.pos", savedSongPos);
ini.put("cur.file", savedSongFile);
ini.put("devices.input", findDevice(inDevices, inDevice, "Default"));
ini.put("devices.output", findDevice(outDevices, outDevice, "Default"));
ini.put("devices.micfeedback", findDevice(outDevices, feedbackDevice, "Default"));
ini.put("devices.preview", findDevice(outDevices, previewDevice, "Default"));
ini.put("devices.jingleOutput", findDevice(outDevices, jingleDevice, "Default"));
ini.put("casting.jingles", useJingles);
ini.put("casting.noFeedback", castNoFeedback);
ini.put("casting.autoUpdateTitle", castAutoTitle);
ini.save("player.conf");
}

if (playlistWriters.size()>0) savePlaylist(playlistWriters[0], toTString(string("player.") + playlistWriters[0]->getString(&(playlistWriters[0]), PP_DEFEX, 0, 0)));

UnregisterHotKey(win, IDM_PAUSE);
UnregisterHotKey(win, IDM_NEXT);
UnregisterHotKey(win, IDM_PREV);

SetCurrentDirectory(curdir.c_str());
    return msg.wParam;
} 
catch (const exception& e) {
printf("Exception: %s: %s\r\n", typeid(e).name(), e.what());
}
catch (...) {
printf("Exception: unspecified type\r\n");
}
return 1;
}

BOOL SetClipboardText (tstring str) {
int len = str.size();
HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(1+len));
if (!hMem) return FALSE;
wchar_t* mem = GlobalLock(hMem);
memcpy(mem, str.c_str(), sizeof(TCHAR) * len);
mem[len]=0;
GlobalUnlock(hMem);
if (!OpenClipboard(win)) return FALSE;
EmptyClipboard();
SetClipboardData(CF_UNICODETEXT, hMem);
CloseClipboard();
}

tstring GetClipboardText (void) {
tstring str;
if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(win)) return str;
HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
const wchar_t* hMemData = GlobalLock(hMem);
int len = wcslen(hMemData);
str.clear();
str.reserve(len+1);
TCHAR* buf = str.data();
memcpy(buf, hMemData, sizeof(TCHAR) * len);
buf[len]=0;
stringSize(str) = len;
GlobalUnlock(hMem);
CloseClipboard();
return str;
}

LRESULT CALLBACK labelSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR reserved1, DWORD_PTR reserved2) {
switch(msg){
case WM_KEYDOWN: keyDown(wp); break;
case WM_KEYUP: keyUp(wp); break;
case WM_SETFOCUS: ctrlDown=shiftDown=FALSE; break;
}
return DefSubclassProc(hwnd,msg,wp,lp);
}

LRESULT CALLBACK playlistListSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR reserved1, DWORD_PTR reserved2) {
static tstring input;
static DWORD time = 0;
if (msg==WM_KEYDOWN) switch(wp){
case VK_SHIFT: shiftDown=TRUE; break;
case VK_CONTROL: ctrlDown=TRUE; break;
case VK_DELETE: SendMessage(GetParent(hwnd), WM_COMMAND, 1004, 0);
case VK_UP: if (ctrlDown) { SendMessage(GetParent(hwnd), WM_COMMAND, 1006, 0); return TRUE; } break;
case VK_DOWN: if (ctrlDown) { SendMessage(GetParent(hwnd), WM_COMMAND, 1007, 0); return TRUE; } break;
case VK_SPACE: { 
if (input.size()<=0 || GetTickCount()-time>500)
SendMessage(GetParent(hwnd), WM_COMMAND, 1015, useJingles&&!shiftDown&&!ctrlDown);
} break;
}
else if (msg==WM_KEYUP && wp==VK_SHIFT) shiftDown=FALSE;
else if (msg==WM_KEYUP && wp==VK_CONTROL) ctrlDown=FALSE;
else if (msg==WM_KEYUP) return TRUE;
else if (msg==WM_CHAR && !ctrlDown) {
DWORD k = LOWORD(wp);
if (k>=32) {
DWORD curTime = GetTickCount();
if (curTime-time>500) input.clear();
if (k==32 && input.size()<=0) return TRUE;
time = curTime;
input.push_back(k);
string sInput = toString(input);
int sel = SendMessage(hwnd, LB_GETCURSEL, 0, 0), count = SendMessage(hwnd, LB_GETCOUNT, 0, 0), newsel = -1, cvt=0;
if (input.length()==1) sel++;
for (int i=0; i<count; i++) {
int index = (i+sel)%count;
int pos = SendMessage(hwnd, LB_GETITEMDATA, index, 0);
const playlistitem& it = playlist[pos];
if (startsWith(it.title,sInput) || startsWith(it.artist,sInput) || startsWith(it.file,sInput)) { newsel=index; break; }
else {
int k1 = it.file.find("\\"+sInput);
if (k1>=0 && k1<it.file.size()) { newsel=index; break; }
k1 = it.file.find("/"+sInput);
if (k1>=0 && k1<it.file.size()) { newsel=index; break; }
}}
if (newsel<0 && (cvt=toUnsignedInt(input))>0 && cvt<=count) newsel = cvt -1;
if (newsel>=0) {
SendMessage(hwnd, LB_SETCURSEL, newsel, 0);
SendMessage(GetParent(hwnd), WM_COMMAND, (LBN_SELCHANGE<<16) | 1001, hwnd);
}
else MessageBeep(0);
return TRUE;
}}
else if (msg==WM_KILLFOCUS) stopSongPreview();
return DefSubclassProc(hwnd, msg, wp, lp);
}

BOOL playlistDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static tstring quickFilter;
switch(msg) {
case WM_INITDIALOG: {
HWND h = GetDlgItem(hwnd,1001);
int tabs[] = { 0, 60, 300 };
SetDlgItemText(hwnd, 1002, quickFilter.c_str());
SendMessage(h, LB_SETTABSTOPS, 3, (LPARAM)tabs);
SetWindowSubclass(h, (SUBCLASSPROC)playlistListSubclassProc, 0, 0);
SendMessage(hwnd, WM_USER, 15, 0);
SetFocus(h);
}break;
case WM_ACTIVATE : {
if (wp>0) {
shiftDown=ctrlDown=FALSE;
HWND hList = GetDlgItem(hwnd,1001);
SetFocus(hList);
SendMessage(hList, LB_SETCURSEL, curSong, 0);
}}break;
case WM_COMMAND:
switch(LOWORD(wp)) {
case 1001: 
if (HIWORD(wp)==LBN_SELCHANGE && stopSongPreview()) SendMessage(hwnd, WM_COMMAND, 1015, 0);
else stopSongPreview();
if (HIWORD(wp)!=LBN_DBLCLK) break;
case 1003: {
stopSongPreview();
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETCURSEL, 0, 0);
if (n<0) break;
n = SendMessage(h, LB_GETITEMDATA, n, 0);
playSongNum(n); 
SetFocus(h);
}break;
case 1002: if (HIWORD(wp)==EN_CHANGE) {
SetTimer(hwnd, 1022, 800, NULL);
}break;
case 1004 : {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETCURSEL, 0, 0);
if (n<0) break;
n = SendMessage(h, LB_GETITEMDATA, n, 0);
delSongNum(n); 
SetFocus(h);
}break;
case 1005: clearPlaylist(1, toString(quickFilter)); SendMessage(hwnd,WM_USER,15,0); break;
case 1014: clearPlaylist(2, toString(quickFilter)); SendMessage(hwnd,WM_USER,15,0); break;
case 1016: case 1017: case 1018: case 1019: sortPlaylist(LOWORD(wp) -1016); SendMessage(hwnd,WM_USER,15,0); break;
case 1006: {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETCURSEL, 0, 0);
if (n<1) break;
int n1 = SendMessage(h, LB_GETITEMDATA, n -1, 0);
int n2 = SendMessage(h, LB_GETITEMDATA, n , 0);
playlistSwap(n1, n2, n -1, n);
SetFocus(h);
}break;
case 1007 : {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETCURSEL, 0, 0) ;
int len = SendMessage(h, LB_GETCOUNT, 0, 0);
if (n>=len -1) break;
int n1 = SendMessage(h, LB_GETITEMDATA, n+1, 0);
int n2 = SendMessage(h, LB_GETITEMDATA, n , 0);
playlistSwap(n1, n2, n +1, n);
SetFocus(h);
}break;
case 1008: openDialog(TRUE); SetFocus(GetDlgItem(hwnd,1001)); break;
case 1009: openDirDialog(TRUE); SetFocus(GetDlgItem(hwnd,1001)); break;
case 1010: savePlaylistDialog(); SetFocus(GetDlgItem(hwnd,1001)); break;
case 1011: doIndexAllPlaylist(); SetFocus(GetDlgItem(hwnd,1011)); break;
case 1012 : {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETCURSEL, 0, 0);
if (n<0) break;
n = SendMessage(h, LB_GETITEMDATA, n, 0);
fileInformationDialog(n);
SetFocus(h);
}break;
case 1015 : {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETCURSEL, 0, 0);
if (n<0) break;
n = SendMessage(h, LB_GETITEMDATA, n, 0);
if (n<0 || n>=playlist.size()) break;
if (curPreviewHandle && BASS_ChannelIsActive(curPreviewHandle)) stopSongPreview();
else if (!lp) startSongPreview(playlist[n].file);
else if (lp) startJingle(playlist[n].file);
}break;
case IDCANCEL: playlistdlg=NULL; DestroyWindow(hwnd); break;
}break;
case WM_CONTEXTMENU :
POINT p;
GetCursorPos(&p);
if ((HWND)wp==GetDlgItem(hwnd,1001)) {
static HMENU playlistCtxMenu = NULL;
if (!playlistCtxMenu) playlistCtxMenu = LoadMenu(hinst, TEXT("plctx"));
TrackPopupMenu(playlistCtxMenu, 0, LOWORD(lp), HIWORD(lp), 0, hwnd, NULL);
}
break;
case WM_TIMER : {
KillTimer(hwnd, 1022);
quickFilter.clear();
quickFilter.reserve(512);
stringSize(quickFilter) = GetDlgItemText(hwnd, 1002, quickFilter.data(), quickFilter.capacity() -1);
SendMessage(hwnd, WM_USER, 15, 0);
}break;
case WM_USER: {
HWND hList = GetDlgItem(hwnd,1001);
switch(wp) {
case 15: {
int j=0, sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
TCHAR ch[512]={0};
SendMessage(hList, LB_GETTEXT, sel, (LPARAM)ch);
string sFilter = toString(quickFilter);
bool useFilter = quickFilter.size()>0;
SendMessage(hList, WM_SETREDRAW, FALSE, 0);
SendMessage(hList, LB_RESETCONTENT, 0, 0);
SendMessage(hList, LB_INITSTORAGE, playlist.size(), 128);
for (int i=0; i<playlist.size(); i++) {
if (useFilter && !acceptPlaylistItem(playlist[i], sFilter)) continue;
SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)getPlaylistItemText(i).c_str());
SendMessage(hList, LB_SETITEMDATA, j++, i);
}
SendMessage(hList, WM_SETREDRAW, TRUE, 0);
if (lp==0 && SendMessage(hList, LB_SELECTSTRING, -1, (LPARAM)ch)==LB_ERR) SendMessage(hList, LB_SETCURSEL, sel, 0);
RedrawWindow(hList, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}break;
case 16:
if (GetFocus()==hList) break;
case 17:
SendMessage(hList, LB_SETCURSEL, lp, 0);
break;
//suite
}}break;
case WM_USER+10: {
int pidx = wp, lidx = lp;
HWND hList = GetDlgItem(hwnd,1001);
if (lidx<0) {
tstring ch;
tsnprintf(ch, 19, TEXT("%d."), pidx+1);
lidx = SendMessage(hList, LB_FINDSTRING, -1, (LPARAM)(ch.c_str()));
if (lidx<0 || lidx==LB_ERR) break;
}
int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
SendMessage(hList, WM_SETREDRAW, FALSE, 0);
SendMessage(hList, LB_DELETESTRING, lidx, 0);
SendMessage(hList, LB_INSERTSTRING, lidx, (LPARAM)getPlaylistItemText(pidx).c_str());
SendMessage(hList, LB_SETITEMDATA, lidx, pidx);
SendMessage(hList, WM_SETREDRAW, TRUE, 0);
if (sel!=LB_ERR && sel>=0 && sel<playlist.size()) SendMessage(hList, LB_SETCURSEL, sel, 0);
RedrawWindow(hList, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}break;
}
return FALSE;
}

void showPlaylist () {
if (!playlistdlg) {
playlistdlg = CreateDialogParam(hinst, TEXT(IDD_PLAYLIST), win, (DLGPROC)playlistDlgProc, NULL);
ShowWindow(playlistdlg, SW_SHOW);
}
SetForegroundWindow(playlistdlg);
SendMessage(playlistdlg, WM_USER, 17, curSong);
}

LRESULT CALLBACK radioListSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR reserved1, DWORD_PTR reserved2) {
if (msg==WM_KEYDOWN) switch(wp){
case VK_SHIFT: shiftDown=TRUE; break;
case VK_CONTROL: ctrlDown=TRUE; break;
case VK_DELETE: SendMessage(GetParent(hwnd), WM_COMMAND, 1004, 0);
}
else if (msg==WM_KEYUP && wp==VK_SHIFT) shiftDown=FALSE;
else if (msg==WM_KEYUP && wp==VK_CONTROL) ctrlDown=FALSE;
return DefSubclassProc(hwnd, msg, wp, lp);
}

BOOL radioPropDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static radio* r;
switch(msg) {
case WM_INITDIALOG : {
r = (radio*)(lp);
SetDlgItemText(hwnd, 1001, toTString(r->name).c_str());
SetDlgItemText(hwnd, 1002, toTString(r->description).c_str());
SetDlgItemText(hwnd, 1003, toTString(r->genre).c_str());
SetDlgItemText(hwnd, 1004, toTString(r->country).c_str());
SetDlgItemText(hwnd, 1005, toTString(r->lang).c_str());
tstring s;
for (const string& url: r->urls) s+= toTString(url) + TEXT("\r\n");
SetDlgItemText(hwnd, 1006, s.c_str());
}break;
case WM_COMMAND: switch(LOWORD(wp)) {
case IDOK  : {
TCHAR buf[1024];
GetDlgItemText(hwnd, 1001, buf, 1023);
r->name = buf;
GetDlgItemText(hwnd, 1002, buf, 1023);
r->description = buf;
GetDlgItemText(hwnd, 1003, buf, 1023);
r->genre = buf;
GetDlgItemText(hwnd, 1004, buf, 1023);
r->country = buf;
GetDlgItemText(hwnd, 1005, buf, 1023);
r->lang = buf;
r->urls.clear();
HWND hUrls = GetDlgItem(hwnd, 1006);
for (int i=0, n=SendMessage(hUrls, EM_GETLINECOUNT, 0, 0); i<n; i++) {
SendMessage(hUrls, EM_GETLINE, i, (LPARAM)buf);
if (!*buf || *buf=='#' || *buf==';' || wcslen(buf)<6) continue;
string url = toString(buf);
if (url.find(TEXT("http://"))!=0  && url.find(TEXT("https://"))!=0) continue;
r->urls.push_back(url);
}
EndDialog(hwnd,2);
}break;
case IDCANCEL : EndDialog(hwnd,1); break;
}break;
// other
}
return FALSE;
}

BOOL radioDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static tstring quickFilter;
static vector<radio> radios;
static int selL = 0, selG = 0, selC = 0;
switch(msg){
case WM_INITDIALOG : {
if (radios.size()<=0) SendMessage(hwnd, WM_USER+11, 0, 0);
HWND hList = GetDlgItem(hwnd,1001), hLang = GetDlgItem(hwnd,1008), hGenre = GetDlgItem(hwnd,1009), hCoun = GetDlgItem(hwnd, 1010);
SetDlgItemText(hwnd, 1002, quickFilter.c_str());
SendMessage(hwnd, WM_USER+9, 0, 0);
SendMessage(hwnd, WM_USER+10, 0, 0);
SetWindowSubclass(hList, (SUBCLASSPROC)radioListSubclassProc, 0, 0);
SetFocus(hList);
}break;
case WM_COMMAND: switch(LOWORD(wp)) {
case 1002: case 1008 : case 1009 : case 1010 : if (HIWORD(wp)==EN_CHANGE || HIWORD(wp)==CBN_SELCHANGE) SetTimer(hwnd, 1023, 800, NULL); break;
case 1001 :
if (HIWORD(wp)==LBN_SELCHANGE) {
KillTimer(hwnd, 1024);
SetTimer(hwnd, 1024, 500, NULL);
}break;
case 1003 : {
HWND hList = GetDlgItem(hwnd, 1001);
int n = SendMessage(hList, LB_GETITEMDATA, SendMessage(hList, LB_GETCURSEL, 0, 0), 0);
if (n<0 || n>=radios.size()) return FALSE;
clearPlaylist();
for (int i=0, I=radios[n].urls.size(); i<I; i++) appendToPlaylist(radios[n].urls[i]);
SetFocus(hList);
}break;
case 1004 : {
HWND hList = GetDlgItem(hwnd,1001);
int n = SendMessage(hList, LB_GETCURSEL, 0, 0);
if (n<0) return FALSE;
tstring s1, s2;
s1.reserve(512);
int i = SendMessage(hList, LB_GETITEMDATA, n, 0);
stringSize(s1) = SendMessage(hList, LB_GETTEXT, n, (LPARAM)(s1.data()));
wsnprintf(s2, 512, toTString(MSG_CFMDELRADIO).c_str(), s1.c_str());
if (MessageBox(hwnd, s2.c_str(), TEXT(MSG_CONFIRM), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
radios.erase(radios.begin() +i);
SendMessage(hwnd, WM_USER+9, 0, 0);
SendMessage(hwnd, WM_USER+10, 0, 0);
SendMessage(hwnd, WM_USER+19, 0, 0);
}
SetFocus(hList);
}break;
case 1005 : {
radio r;
if (DialogBoxParam(hinst, TEXT(IDD_RADIOPROPDLG), win, (DLGPROC)radioPropDlgProc, (LPARAM)(&r)) >1 && r.name.size()>=3 && r.urls.size()>=1) {
radios.push_back(r);
SendMessage(hwnd, WM_USER+9, 0, 0);
SendMessage(hwnd, WM_USER+10, 0, 0);
SendMessage(hwnd, WM_USER+19, 0, 0);
}
SetFocus(GetDlgItem(hwnd,1001));
}break;
case 1006 : {
HWND hList = GetDlgItem(hwnd,1001);
int n = SendMessage(hList, LB_GETCURSEL, 0, 0);
if (n<0) return FALSE;
int i = SendMessage(hList, LB_GETITEMDATA, n, 0);
if (DialogBoxParam(hinst, TEXT(IDD_RADIOPROPDLG), win, (DLGPROC)radioPropDlgProc, (LPARAM)(&radios[i])) >1) {
SendMessage(hwnd, WM_USER+9, 0, 0);
SendMessage(hwnd, WM_USER+10, 0, 0);
SendMessage(hwnd, WM_USER+19, 0, 0);
}
SetFocus(hList);
}break;
case IDCANCEL: 
selL = SendDlgItemMessage(hwnd, 1008, CB_GETCURSEL, 0, 0);
selG = SendDlgItemMessage(hwnd, 1009, CB_GETCURSEL, 0, 0);
selC = SendDlgItemMessage(hwnd, 1010, CB_GETCURSEL, 0, 0);
radiodlg=NULL; 
DestroyWindow(hwnd); 
break;
}break;
case WM_CONTEXTMENU :
POINT p;
GetCursorPos(&p);
if ((HWND)wp==GetDlgItem(hwnd,1001)) {
static HMENU radioCtxMenu = NULL;
if (!radioCtxMenu) radioCtxMenu = LoadMenu(hinst, TEXT("radioctx"));
TrackPopupMenu(radioCtxMenu, 0, LOWORD(lp), HIWORD(lp), 0, hwnd, NULL);
}
break;
case WM_TIMER : {
KillTimer(hwnd, wp);
if (wp==1024) {
HWND hList = GetDlgItem(hwnd,1001);
int n = SendMessage(hList, LB_GETITEMDATA, SendMessage(hList, LB_GETCURSEL, 0, 0), 0);
if (n<0 || n>=radios.size()) return FALSE;
if (sayString && radios[n].description.size()>3) sayString(toTString(radios[n].description).c_str(), false);
}
else if (wp==1023) {
quickFilter.clear();
quickFilter.reserve(512);
stringSize(quickFilter) = GetDlgItemText(hwnd, 1002, quickFilter.data(), quickFilter.capacity() -1);
SendMessage(hwnd, WM_USER+10, 0, 0);
}}break;
case WM_USER+9 : {
HWND hList = GetDlgItem(hwnd,1001), hLang = GetDlgItem(hwnd,1008), hGenre = GetDlgItem(hwnd,1009), hCoun = GetDlgItem(hwnd, 1010);
selL = SendDlgItemMessage(hwnd, 1008, CB_GETCURSEL, 0, 0);
selG = SendDlgItemMessage(hwnd, 1009, CB_GETCURSEL, 0, 0);
selC = SendDlgItemMessage(hwnd, 1010, CB_GETCURSEL, 0, 0);
if (selC<0) selC=0;
if (selL<0) selL=0;
if (selG<0) selG=0;
SendMessage(hLang, WM_SETREDRAW, FALSE, 0);
SendMessage(hCoun, WM_SETREDRAW, FALSE, 0);
SendMessage(hGenre, WM_SETREDRAW, FALSE, 0);
SendMessage(hLang, CB_RESETCONTENT, 0, 0);
SendMessage(hCoun, CB_RESETCONTENT, 0, 0);
SendMessage(hGenre, CB_RESETCONTENT, 0, 0);
SendMessage(hLang, CB_ADDSTRING, 0, (LPARAM)TEXT(MSG_ALLLANGS));
SendMessage(hCoun, CB_ADDSTRING, 0, (LPARAM)TEXT(MSG_ALLCOUNTRIES));
SendMessage(hGenre, CB_ADDSTRING, 0, (LPARAM)TEXT(MSG_ALLGENRES));
{
set<string> langs, countries, genres;
for (int i=0; i<radios.size(); i++) {
if (radios[i].lang.size()>2) langs.insert(radios[i].lang);
if (radios[i].country.size()>2) countries.insert(radios[i].country);
if (radios[i].genre.size()>2) genres.insert(radios[i].genre);
}
for (auto  it: langs) SendMessage(hLang, CB_ADDSTRING, 0, (LPARAM)(toTString(it).c_str()));
for (auto it: countries) SendMessage(hCoun, CB_ADDSTRING, 0, (LPARAM)(toTString(it).c_str()));
for (auto it: genres) SendMessage(hGenre, CB_ADDSTRING, 0, (LPARAM)(toTString(it).c_str()));
}
SendMessage(hLang, CB_SETCURSEL, selL, 0);
SendMessage(hCoun, CB_SETCURSEL, selC, 0);
SendMessage(hGenre, CB_SETCURSEL, selG, 0);
SendMessage(hGenre, WM_SETREDRAW, TRUE, 0);
SendMessage(hCoun, WM_SETREDRAW, TRUE, 0);
SendMessage(hLang, WM_SETREDRAW, TRUE, 0);
}break;
case WM_USER+10 : {
HWND hList = GetDlgItem(hwnd,1001), hLang = GetDlgItem(hwnd,1008), hGenre = GetDlgItem(hwnd,1009), hCoun = GetDlgItem(hwnd, 1010);
string genre, country, lang, sFilter=toString(quickFilter);
int useFilter = !quickFilter.empty(), useGenre = SendMessage(hGenre, CB_GETCURSEL, 0, 0), useLang = SendMessage(hLang, CB_GETCURSEL, 0, 0), useCountry = SendMessage(hCoun, CB_GETCURSEL, 0, 0);
if (useLang>0) { TCHAR buf[1024]={0}; SendMessage(hLang, CB_GETLBTEXT, useLang, (LPARAM)buf); lang=toString(buf); }
if (useCountry>0) { TCHAR buf[1024]={0}; SendMessage(hCoun, CB_GETLBTEXT, useCountry, (LPARAM)buf); country=toString(buf); }
if (useGenre>0) { TCHAR buf[1024]={0}; SendMessage(hGenre, CB_GETLBTEXT, useGenre, (LPARAM)buf); genre=toString(buf); }
int newsel = -1, cursel = SendMessage(hList, LB_GETCURSEL, 0, 0);
if (cursel>=0) cursel = SendMessage(hList, LB_GETITEMDATA, cursel, 0);
SendMessage(hList, WM_SETREDRAW, FALSE, 0);
SendMessage(hList, LB_RESETCONTENT, 0, 0);
SendMessage(hList, LB_INITSTORAGE, radios.size(), 128);
for (int i=0; i<radios.size(); i++) {
radio& r = radios[i];
if (useLang>0 && !contains(r.lang, lang)) continue;
if (useCountry>0 && !contains(r.country, country)) continue;
if (useGenre>0 && !contains(r.genre, genre)) continue;
if (useFilter && !acceptRadio(r, sFilter)) continue;
string s = r.name +("\t") + r.genre +("\t") + r.lang +("\t") + r.country;
int idx = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(toTString(s).c_str()));
SendMessage(hList, LB_SETITEMDATA, idx, i);
if (cursel>=0 && cursel==i) newsel=idx;
else if (newsel>=0 && idx<=newsel) newsel++;
}
SendMessage(hList, WM_SETREDRAW, TRUE, 0);
SendMessage(hList, LB_SETCURSEL, newsel<0?0:newsel, 0);
}break;
case WM_USER+11 : {
SetCurrentDirectory(appdir.c_str());
string line;
ifstream fp("radios.conf");
if (!fp) { Beep(8000,250); return FALSE; }
while(getline(fp,line)) {
trim(line);
if (!line[0] || line[0]==';' || line[0]=='#') continue;
radio r;
split_iterator in(line, "\t");
in >> r.name  >> r.description  >> r.genre  >> r.country  >> r.lang;
while(in) { r.urls.push_back(*in); ++in; }
if (r.name.size()<3 || r.urls.size()<=0 || r.urls[0].size()<10) continue;
radios.push_back(r);
}}break;
case WM_USER+19 : {
SetCurrentDirectory(appdir.c_str());
ofstream fp("radios.conf");
if (!fp) return FALSE;
for (radio& r: radios) {
string str;
snprintf(str, 1024, "%1s\t%1s\t%1s\t%1s\t%1s", toString(r.name).c_str(), toString(r.description).c_str(), toString(r.genre).c_str(), toString(r.country).c_str(), toString(r.lang).c_str());
fp << str;
for (const string& url: r.urls) {
snprintf(str, 1024, "\t%1s", toString(url).c_str());
fp << str;
}
fp << endl;
}
}break;
//suite
}
return FALSE;
}

void showRadio () {
if (!radiodlg) {
radiodlg = CreateDialogParam(hinst, TEXT(IDD_RADIO), win, (DLGPROC)radioDlgProc, NULL);
ShowWindow(radiodlg, SW_SHOW);
}
SetForegroundWindow(radiodlg);
}

LRESULT CALLBACK cbSelSubclassProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR reserved1, DWORD_PTR cbId) {
static tstring input;
static DWORD time = 0;
if (msg==WM_KEYDOWN) switch(wp){
case VK_SHIFT: shiftDown=TRUE; break;
case VK_CONTROL: ctrlDown=TRUE; break;
}
else if (msg==WM_KEYUP && wp==VK_SHIFT) shiftDown=FALSE;
else if (msg==WM_KEYUP && wp==VK_CONTROL) ctrlDown=FALSE;
else if (msg==WM_CHAR && !ctrlDown) {
DWORD k = LOWORD(wp);
if (k>=32) {
DWORD curTime = GetTickCount();
if (curTime-time>500) input.clear();
time = curTime;
input.push_back(k);
int sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
if (input.length()==1) sel++;
int newsel = SendMessage(hwnd, CB_FINDSTRING, sel, input.c_str());
if (newsel>=0) {
SendMessage(hwnd, CB_SETCURSEL, newsel, 0);
SendMessage(GetParent(hwnd), WM_COMMAND, (CBN_SELCHANGE<<16) | cbId, hwnd);
SetFocus(GetParent(hwnd));
SetFocus(hwnd);
}
else MessageBeep(0);
return FALSE;
}}
return DefSubclassProc(hwnd,msg,wp,lp);
}

BOOL MIDIPanelDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static unordered_map<DWORD,DWORD> patchToCBIndex;
switch(msg){
case WM_INITDIALOG : {
if (midiInstruments.size()<=0) MIDIFillInstrumentsList(midiInstruments);
DWORD hs = BASS_FX_TempoGetSource(curHandle);
for (int i=0; i<16; i++) {
HWND hLbl=GetDlgItem(hwnd, 1100+i), hPatch = GetDlgItem(hwnd, 1200+i);
DWORD index=-1;
DWORD patch = MIDIGetCurrentInstrument(hs, i);
SetWindowText(hLbl, (toTString(MSG_MIDI_CHANNEL) + toTString(i+1)).c_str());
SetWindowSubclass(hPatch, (SUBCLASSPROC)cbSelSubclassProc, 1200+i, 1200+i);
SendMessage(hPatch, CB_RESETCONTENT, 0, 0);
SendMessage(hPatch, WM_SETREDRAW, FALSE, 0);
if (i==0) patchToCBIndex.clear();
for(auto it: midiInstruments) {
string pName = it.second + " (" + toString((it.first>>14)&0xFF) + "," + toString((it.first>>7)&0x7F) + "," + toString(it.first&0x7F) + ")";
SendMessage(hPatch, CB_ADDSTRING, 0, toTString(pName).c_str());
SendMessage(hPatch, CB_SETITEMDATA, ++index, it.first);
if (i==0) patchToCBIndex[it.first] = index;
if (patch==it.first) SendMessage(hPatch, CB_SETCURSEL, index, 0);
}
//Other channel specific items
SendMessage(hPatch, WM_SETREDRAW, TRUE, 0);
}
SetFocus(GetDlgItem(hwnd,1200));
}break;
case WM_COMMAND:
switch(LOWORD(wp)) {
case 1200 ... 1215:
if (HIWORD(wp)==CBN_SELCHANGE) {
DWORD chan = LOWORD(wp) -1200;
DWORD patch = SendDlgItemMessage(hwnd, 1200+chan, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1200+chan, CB_GETCURSEL, 0, 0), 0);
DWORD handle = BASS_FX_TempoGetSource(curHandle);
BOOL drums = patch>=(1<<21);
BASS_MIDI_StreamEvent(handle, chan, MIDI_EVENT_DRUMS, drums);
if (!drums) BASS_MIDI_StreamEvent(handle, chan, MIDI_EVENT_BANK, (patch>>14)&0x7F);
if (!drums) BASS_MIDI_StreamEvent(handle, chan, MIDI_EVENT_BANK_LSB, (patch>>7)&0x7F);
BASS_MIDI_StreamEvent(handle, chan, MIDI_EVENT_PROGRAM, patch&0x7F);
}break;
case IDCANCEL: 
ShowWindow(hwnd, SW_HIDE);
break;
}break;
case WM_USER+105:
switch(wp){
case 0 ... 15: {
auto it = patchToCBIndex.find(lp);
if (it!=patchToCBIndex.end()) {
HWND hcb = GetDlgItem(hwnd,1200+wp);
SendMessage(hcb, CB_SETCURSEL, it->second, 0);
}}break;
//other WM_USER messages
}break;
default: break;
}
return FALSE;
}

BOOL TextPanelDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_COMMAND: switch(LOWORD(wp)){
case IDCANCEL: 
ShowWindow(hwnd, SW_HIDE);
break;
}break;
case WM_USER+105: switch(wp) {
case 100: {
tstring text = toTString((const char*)lp);
HWND h = GetDlgItem(hwnd,1000);
int len = GetWindowTextLength(h);
SendMessage(h, EM_SETSEL, len, len);
SendMessage(h, EM_REPLACESEL, 0, text.c_str());
SendMessage(h, EM_SETSEL, len, len+text.size());
}break;
case 101: case 102:
SetDlgItemText(hwnd, 1000, NULL); 
break;
}break;
default: break;
}
return FALSE;
}

BOOL LevelsPanelDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_INITDIALOG : {
for (int i=1000; i<=1003; i++) {
HWND h = GetDlgItem(hwnd,i);
SendMessage(h, TBM_SETRANGEMIN, FALSE, 0);
SendMessage(h, TBM_SETRANGEMAX, FALSE, 1000);
SendMessage(h, TBM_SETLINESIZE, 0, 1);
SendMessage(h, TBM_SETPAGESIZE, 0, 50);
}
SendDlgItemMessage(hwnd, 1000, TBM_SETPOS, TRUE, 1000 * curVol);
SendDlgItemMessage(hwnd, 1001, TBM_SETPOS, TRUE, 1000 * curRecVol);
SendDlgItemMessage(hwnd, 1002, TBM_SETPOS, TRUE, 1000 * curCastVol);
SendDlgItemMessage(hwnd, 1003, TBM_SETPOS, TRUE, 1000 * curRecCastVol);
SetFocus(GetDlgItem(hwnd,1000));
}break;
case WM_COMMAND: switch(LOWORD(wp)){
case IDCANCEL: 
ShowWindow(hwnd, SW_HIDE);
break;
}break;
case WM_HSCROLL : {
int ival = SendMessage(lp, TBM_GETPOS, 0, 0);
float val = ival/1000.0f;
switch(GetDlgCtrlID(lp)) {
case 1000: setSongVol(val); break;
case 1001: setRecVol(val); break;
case 1002: setCastVol(val); break;
case 1003: setRecCastVol(val); break;
}}break;
break;
default: break;
}
return FALSE;
}

void showLevelsPanel (void) {
if (!levelsdlg) {
levelsdlg = CreateDialogParam(hinst, TEXT(IDD_LEVELSDLG), win, (DLGPROC)LevelsPanelDlgProc, NULL);
}
ShowWindow(levelsdlg, SW_SHOW);
SetForegroundWindow(levelsdlg);
}

void showTextPanel (void) {
if (!textdlg) {
textdlg = CreateDialogParam(hinst, TEXT(IDD_TEXTDLG), win, (DLGPROC)TextPanelDlgProc, NULL);
}
ShowWindow(textdlg, SW_SHOW);
SetForegroundWindow(textdlg);
}

void showMIDIPanel (void) {
if (!mididlg) {
mididlg = CreateDialogParam(hinst, TEXT(IDD_MIDI), win, (DLGPROC)MIDIPanelDlgProc, NULL);
}
ShowWindow(mididlg, SW_SHOW);
SetForegroundWindow(mididlg);
DWORD hs = BASS_FX_TempoGetSource(curHandle);
for (int i=0; i<16; i++) SendMessage(mididlg, WM_USER+105, i, MIDIGetCurrentInstrument(hs,i));
}

BOOL inputDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG : {
const TCHAR** s = (const TCHAR**)lp;
SetWindowText(hwnd, *(s++));
SetDlgItemText(hwnd, 1001, *(s++));
SetDlgItemText(hwnd, 1002, *(s++));
SetFocus(GetDlgItem(hwnd,1002));
}return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
HWND hTxt = GetDlgItem(hwnd, 1002);
int txtlen = GetWindowTextLength(hTxt);
TCHAR* str = (TCHAR*)malloc(sizeof(TCHAR) * (txtlen+1));
GetWindowText(hTxt, str, txtlen+1);
str[txtlen]=0;
	EndDialog(hwnd, (INT_PTR)str);
}return TRUE;
case IDCANCEL : EndDialog(hwnd, 1); return TRUE;
}break;
}
return FALSE;
}

TCHAR* inputDialog (const TCHAR* title, const TCHAR* prompt, const TCHAR* text) {
INT_PTR re = DialogBoxParam(hinst, TEXT(IDD_INPUTDLG), win, (DLGPROC)inputDlgProc, (LPARAM)(&title));
if (re==0 || re==1) return NULL;
else return (TCHAR*)re;
}

BOOL fileInfoDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static int pid = 0;
static LPQCPLUGIN pTagger = NULL;
static playlistitem* itp = NULL;
switch(msg){
case WM_INITDIALOG: {
pid=lp;
if (pid<0 || pid>=playlist.size()) EndDialog(hwnd,1);
int tabs[] = { 10, 100 };
SendDlgItemMessage(hwnd, 1001, LB_SETTABSTOPS, 2, (LPARAM)tabs);
SendMessage(hwnd, WM_USER+2, 0, 0);
SetFocus(GetDlgItem(hwnd,1001));
}break;
case WM_COMMAND: 
switch(LOWORD(wp)) {
case IDOK : {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETITEMDATA, SendMessage(h, LB_GETCURSEL, 0, 0), 0);
if (n>0 && n<=11) {
playlistitem& it = playlist[pid];
string dummy, &s = (
n==1? it.title :
n==2? it.artist :
n==3? it.album :
n==4? it.genre :
n==5? it.subtitle :
n==6? it.composer :
n==7? it.year :
n==8? it.disc :
n==9? it.tracknum :
n==10? it.copyright :
n==11? it.comment :
dummy);
TCHAR* value = inputDialog(TEXT(MSG_FI_MODIFYDLG), TEXT(MSG_FI_MODIFYDLG2), toTString(s).c_str());
if (value) {
s = toString(value);
SendMessage(hwnd, WM_USER +2, 0, 0);
updateWindowTitle();
free(value);
}
SetFocus(h);
}
}break;
case IDCANCEL :
EndDialog(hwnd,1);
break;
case 1002 :
if (itp&&pTagger && pTagger->open(pTagger, itp, PF_OPEN_WRITE, NULL)) MessageBeep(MB_ICONASTERISK);
else MessageBox(hwnd, toTString(MSG_FI_TAGSAVEFAILED).c_str(), TEXT(ERROR), MB_OK | MB_ICONSTOP);
break;
case 1500 : {
HWND h = GetDlgItem(hwnd,1001);
int n = SendMessage(h, LB_GETITEMDATA, SendMessage(h, LB_GETCURSEL, 0, 0), 0);
playlistitem& it = playlist[pid];
string dummy, &s = (
n==1? it.title :
n==2? it.artist :
n==3? it.album :
n==4? it.genre :
n==5? it.subtitle :
n==6? it.composer :
n==7? it.year :
n==8? it.disc :
n==9? it.tracknum :
n==10? it.copyright :
n==11? it.comment :
n==-1? it.file :
dummy);
if (s.size()>0) SetClipboardText(toTString(s));
}break;
}
if (HIWORD(wp)==LBN_DBLCLK) SendMessage(hwnd, WM_COMMAND, IDOK, 0);
break;
case WM_CONTEXTMENU : {
HWND h = GetDlgItem(hwnd,1001);
if (h!=(HWND)wp) return TRUE;
int n = SendMessage(h, LB_GETITEMDATA, SendMessage(h, LB_GETCURSEL, 0, 0), 0);
static HMENU infoCtxMenu1 = NULL;
if (!infoCtxMenu1) infoCtxMenu1 = LoadMenu(hinst, TEXT("i1ctx"));
EnableMenuItem(infoCtxMenu1, IDOK, n>0&&n<20? MF_ENABLED : MF_DISABLED);
EnableMenuItem(infoCtxMenu1, 1500, n!=0? MF_ENABLED : MF_DISABLED);
if (n!=0) TrackPopupMenu(infoCtxMenu1, 0, LOWORD(lp), HIWORD(lp), 0, hwnd, NULL);
}break;
case WM_USER +2: {
#define additem(s,v) SendMessage(h, LB_ADDSTRING, 0, (LPARAM)(s)); SendMessage(h, LB_SETITEMDATA, count++, v); /*printf("Add item: count=%d, value=%d, string=%s, error=%d\r\n", count, v, s, GetLastError())*/
HWND h = GetDlgItem(hwnd,1001);
int count=0, sel = SendMessage(h, LB_GETCURSEL, 0, 0);
SendMessage(h, LB_RESETCONTENT, 0, 0);
playlistitem& it = playlist[pid]; itp = &it;
BOOL iscur = (pid==curSong);
BASS_CHANNELINFO info;
DWORD curHandle2 = NULL;
if (iscur) curHandle2 =  BASS_FX_TempoGetSource(curHandle);
else {
curHandle2 = getStream(it.file, BASS_MUSIC_NOSAMPLE);
fillMetadata(it,curHandle2);
}
pTagger=NULL;
for (LPQCPLUGIN p: taggers) if (p->open(p, it.file.c_str(), PF_TAG_CHECK, curHandle2)) { pTagger=p; break; }
EnableWindow(GetDlgItem(hwnd,1002), !!pTagger);
if (contains(it.file, "://")) {
tstring s = TEXT(MSG_FI_URL) + toTString(it.file);
additem(s.c_str(), -1);
} else {
int idx1 = it.file.rfind('\\'), idx2 = it.file.rfind('/');
if (idx1<0 || idx1>=it.file.size()) idx1=idx2;
tstring s1 = TEXT(MSG_FI_FILENAME) + toTString(it.file.substr(idx1+1));
tstring s2 = TEXT(MSG_FI_FILEPATH) + toTString(it.file.substr(0,idx1));
tstring s3 = TEXT(MSG_FI_FILESIZE) + formatSize(getFileSize(toTString(it.file)));
additem(s1.c_str(), -1);
additem(s2.c_str(), -1);
additem(s3.c_str(), 0);
}
tstring sTitle = TEXT(MSG_FI_TITLE) + (it.title.size()>0? toTString(it.title) : TEXT(MSG_FI_NONE)),
sArtist = TEXT(MSG_FI_ARTIST) + (it.artist.size()>0? toTString(it.artist) : TEXT(MSG_FI_NONE)),
sAlbum = TEXT(MSG_FI_ALBUM) + (it.album.size()>0? toTString(it.album) : TEXT(MSG_FI_NONE)),
sSubtitle = TEXT(MSG_FI_SUBTITLE) + (it.subtitle.size()>0? toTString(it.subtitle) : TEXT(MSG_FI_NONE)),
sGenre = TEXT(MSG_FI_GENRE) + (it.genre.size()>0? toTString(it.genre) : TEXT(MSG_FI_NONE)),
sComposer = TEXT(MSG_FI_COMPOSER) + (it.composer.size()>0? toTString(it.composer) : TEXT(MSG_FI_NONE)),
sYear = toTString(MSG_FI_YEAR) + (it.year.size()>0? toTString(it.year) : TEXT(MSG_FI_NONE)),
sCopyright = TEXT(MSG_FI_COPYRIGHT) + (it.copyright.size()>0? toTString(it.copyright) : TEXT(MSG_FI_NONE)),
sDisc = toTString(MSG_FI_DISC) + (it.disc.size()>0? toTString(it.disc) : TEXT(MSG_FI_NONE)),
sTracknum = toTString(MSG_FI_TRACKNUM) + (it.tracknum.size()>0? toTString(it.tracknum) : TEXT(MSG_FI_NONE)),
sComment = TEXT(MSG_FI_COMMENT) + (it.comment.size()>0? toTString(it.comment) : TEXT(MSG_FI_NONE)),
sDuration = toTString(MSG_FI_DURATION) + formatTime(it.length);
additem(sDuration.c_str(), 0);
additem(TEXT("\xA0"),0);
additem(sTitle.c_str(), 1);
additem(sSubtitle.c_str(), 5);
additem(sArtist.c_str(), 2);
additem(sAlbum.c_str(), 3);
additem(sGenre.c_str(), 4);
additem(sComposer.c_str(), 6);
additem(sYear.c_str(), 7);
additem(sDisc.c_str(), 8);
additem(sTracknum.c_str(), 9);
additem(sCopyright.c_str(), 10);
additem(sComment.c_str(), 11);
additem(TEXT("\xA0"),0);
if (BASS_ChannelGetInfo(curHandle2, &info)) {
DWORD bitrate = BASS_StreamGetBitrate(curHandle2);
tstring sFreq, sChan, sBitres, sBitrate, sType = toTString(typeMap[info.ctype]);
wsnprintf(sFreq, 64, TEXT("%s :\t%d Hz"), toTString(MSG_FI_FREQ).c_str(), info.freq);
wsnprintf(sChan, 64, TEXT("%s :\t%d"), TEXT(MSG_FI_NCHAN), info.chans);
if (bitrate>0 && bitrate<100000) wsnprintf(sBitrate, 128, toTString(MSG_FI_BITRATE).c_str(), bitrate);
if (info.origres>0) wsnprintf(sBitres, 64, toTString(MSG_FI_BITRES).c_str(), info.origres);
if (sType.size()<=0)  wsnprintf(sType, 8, TEXT("%#8x"), info.ctype);
if (sType.size()>0) additem((TEXT(MSG_FI_CTYPE) + sType).c_str(), 0);
if (sBitrate.size()>0 && bitrate>0 && bitrate<100000) additem(sBitrate.c_str(), 0);
additem(sFreq.c_str(), 0);
additem(sChan.c_str(), 0);
if (sBitres.size()>0 && info.origres>0) additem(sBitres.c_str(), 0);
if (info.ctype&BASS_CTYPE_MUSIC_MOD) {
additem(TEXT("\xA0"),0);
additem(TEXT(MSG_FI_MODINFO),0);
int base = BASS_TAG_MUSIC_INST;
if (info.ctype == BASS_CTYPE_MUSIC_MOD || info.ctype == BASS_CTYPE_MUSIC_S3M || info.ctype == (BASS_CTYPE_MUSIC_MOD | BASS_CTYPE_MUSIC_MO3) || info.ctype == (BASS_CTYPE_MUSIC_S3M | BASS_CTYPE_MUSIC_MO3)) base = BASS_TAG_MUSIC_SAMPLE;
int num = 0;
const char* inst = NULL;
while (inst = BASS_ChannelGetTags(curHandle2, base+num)) {
tstring buf;
tsnprintf(buf, 255, TEXT(MSG_FI_MODINST), ++num, inst);
additem(buf.c_str(),0);
}}
else if (info.ctype == BASS_CTYPE_STREAM_MIDI && pid==curSong) {
DWORD chans = 16;
float f0=16;
if (BASS_ChannelGetAttribute(curHandle2, BASS_ATTRIB_MIDI_CHANS, &f0)) chans=f0;
additem(TEXT("\xA0"),0);
additem(TEXT(MSG_FI_MODINFO),0);
for (int i=0; i<chans; i++) {
const char* inst = MIDIGetCurrentInstrumentName(curHandle2, i);
tstring buf;
wsnprintf(buf, 255, TEXT(MSG_FI_MODINST), i+1, inst);
additem(buf.c_str(), 0);
}}
//other specific type of information
}
if (!iscur) { BASS_StreamFree(curHandle2); BASS_MusicFree(curHandle2); }
if (sel>=0) SendMessage(h, LB_SETCURSEL, sel, 0);
#undef additem
}break;
}
return FALSE;
}

void fileInformationDialog (int pid) {
if (pid<0 || pid>=playlist.size()) return;
DialogBoxParam(hinst, TEXT(IDD_INFODLG), win, (DLGPROC)fileInfoDlgProc, pid);
}

BOOL progressDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static progressinfo* p = NULL;
switch(msg){
case WM_INITDIALOG : 
p = (progressinfo*)lp;
p->hdlg = hwnd;
SetWindowText(hwnd, p->title);
break;
case WM_COMMAND :
if (LOWORD(wp)==IDCANCEL) {
p->cancelled = true;
p->hdlg=NULL;
EndDialog(hwnd,1);
}break;
case WM_USER+12 :
SendDlgItemMessage(hwnd, 1002, PBM_SETPOS, wp, 0);
break;
case WM_USER+13 :
SendDlgItemMessage(hwnd, 1002, PBM_SETRANGE32, wp, lp);
break;
case WM_USER+14 :
SetDlgItemText(hwnd, 1001, (const TCHAR*)lp);
break;
}
return FALSE;
}

void changeOutDevices (int out, int preview, int feedback, int jingle) {
if (!setOutDevice(out)) return;
if (curHandle) BASS_ChannelSetDevice(curHandle, out);
if (curHandle) BASS_ChannelSetDevice(BASS_FX_TempoGetSource(curHandle), out);
if (curMixHandle) BASS_ChannelSetDevice(curMixHandle, out);
if (curPreviewHandle) {
setOutDevice(preview);
BASS_ChannelSetDevice(curPreviewHandle, preview);
}
if (curFeedback) {
setOutDevice(feedback);
BASS_ChannelSetDevice(curFeedback, feedback);
}
#define F(o) if(o!=out && o!=jingle && o!=preview && o!=feedback) freeOutDevice(o);
F(outDevice) F(previewDevice) F(jingleDevice) F(feedbackDevice)
#undef F
BASS_SetDevice(out);
outDevice = out;
feedbackDevice = feedback;
previewDevice = preview;
jingleDevice = jingle;
}

void changeInDevice (int in) {
inDevice = in;
if (curRecord) {
stopRecord();
startRecord();
}}

BOOL OptionsDevicesDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static optionsinfo*o = NULL;
switch(msg){
case WM_INITDIALOG : {
o = (optionsinfo*)(((PROPSHEETPAGE*)lp)->lParam);
unordered_map<string,int> inDevices, outDevices;
fillOutDevices(NULL, &outDevices);
fillInDevices(NULL, &inDevices);
outDevices[""] = 1;
inDevices[""] = -1;
HWND hOut = GetDlgItem(hwnd,1001), hPre = GetDlgItem(hwnd,1002), hIn = GetDlgItem(hwnd,1003), hFeed=GetDlgItem(hwnd,1004), hJingle=GetDlgItem(hwnd,1005);
int j=-1;
SendMessage(hOut, CB_RESETCONTENT, 0, 0);
SendMessage(hIn, CB_RESETCONTENT, 0, 0);
SendMessage(hPre, CB_RESETCONTENT, 0, 0);
SendMessage(hFeed, CB_RESETCONTENT, 0, 0);
SendMessage(hJingle, CB_RESETCONTENT, 0, 0);
for (auto it: outDevices) {
printf("%d=%s\r\n", it.second, it.first.c_str());
string sName = it.first;
if (sName=="") sName = MSG_DEFAULTDEVICE;
SendMessage(hOut, CB_ADDSTRING, 0, toTString(sName).c_str());
SendMessage(hPre, CB_ADDSTRING, 0, toTString(sName).c_str());
SendMessage(hFeed, CB_ADDSTRING, 0, toTString(sName).c_str());
SendMessage(hJingle, CB_ADDSTRING, 0, toTString(sName).c_str());
SendMessage(hOut, CB_SETITEMDATA, ++j, it.second);
SendMessage(hPre, CB_SETITEMDATA, j, it.second);
SendMessage(hFeed, CB_SETITEMDATA, j, it.second);
SendMessage(hJingle, CB_SETITEMDATA, j, it.second);
if (o->outDevice==it.second) SendMessage(hOut, CB_SETCURSEL, j, 0);
if (o->previewDevice==it.second) SendMessage(hPre, CB_SETCURSEL, j, 0);
if (o->feedbackDevice==it.second) SendMessage(hFeed, CB_SETCURSEL, j, 0);
if (o->jingleDevice==it.second) SendMessage(hJingle, CB_SETCURSEL, j, 0);
}
j=0;
SendMessage(hIn, CB_RESETCONTENT, 0, 0);
SendMessage(hIn, CB_ADDSTRING, 0, (LPARAM)toTString(MSG_DEFAULTDEVICE).c_str());
SendMessage(hIn, CB_SETITEMDATA, 0, -1);
SendMessage(hIn, CB_SETCURSEL, 0, 0);
for (auto it: inDevices) {
if (it.first.size()<=0) continue;
SendMessage(hIn, CB_ADDSTRING, 0, toTString(it.first).c_str());
SendMessage(hIn, CB_SETITEMDATA, ++j, it.second);
if (o->inDevice==it.second) SendMessage(hIn, CB_SETCURSEL, j, 0);
}
EnableWindow(hJingle, o->useJingles);
SetFocus(hOut);
}break;
case WM_COMMAND :
switch(LOWORD(wp)) {
}break;
case WM_NOTIFY : 
switch (((LPPSHNOTIFY)lp)->hdr.code) {
case PSN_SETACTIVE :
EnableWindow(GetDlgItem(hwnd,1005), o->useJingles);
break;
case PSN_KILLACTIVE :
SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
return TRUE;
case PSN_RESET : break;
case PSN_APPLY :
o->outDevice = SendDlgItemMessage(hwnd, 1001, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1001, CB_GETCURSEL, 0, 0), 0);
o->previewDevice = SendDlgItemMessage(hwnd, 1002, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1002, CB_GETCURSEL, 0, 0), 0);
o->inDevice = SendDlgItemMessage(hwnd, 1003, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1003, CB_GETCURSEL, 0, 0), 0);
o->feedbackDevice = SendDlgItemMessage(hwnd, 1004, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1004, CB_GETCURSEL, 0, 0), 0);
o->jingleDevice = SendDlgItemMessage(hwnd, 1005, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1005, CB_GETCURSEL, 0, 0), 0);
SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
return TRUE;
}break;
}
return FALSE;
}

BOOL OptionsIntegrationDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_INITDIALOG : {
int reuse = ini.get("explorer.reuseInstances",1);
CheckDlgButton(hwnd, 1001, reuse? BST_UNCHECKED : BST_CHECKED);
CheckDlgButton(hwnd, 1005, reuse==2? BST_CHECKED : BST_UNCHECKED);
CheckDlgButton(hwnd, 1006, !ini.get("explorer.openPreview",true)? BST_UNCHECKED : BST_CHECKED);
CheckRadioButton(hwnd, 1002, 1004, 1002+ini.get("explorer.openFlags",1));
BOOL act = IsDlgButtonChecked(hwnd,1001)==BST_UNCHECKED;
EnableWindow(GetDlgItem(hwnd,1005), act);
EnableWindow(GetDlgItem(hwnd,1004), act);
EnableWindow(GetDlgItem(hwnd,1003), act);
EnableWindow(GetDlgItem(hwnd,1002), act);
SetFocus(GetDlgItem(hwnd, 1006));
}break;
case WM_COMMAND :
switch(LOWORD(wp)) {
case 1001: if(HIWORD(wp)==BN_CLICKED){
BOOL act = IsDlgButtonChecked(hwnd,1001)==BST_UNCHECKED;
EnableWindow(GetDlgItem(hwnd,1005), act);
EnableWindow(GetDlgItem(hwnd,1004), act);
EnableWindow(GetDlgItem(hwnd,1003), act);
EnableWindow(GetDlgItem(hwnd,1002), act);
}break;
// other commands
}break;
case WM_NOTIFY : 
switch (((LPPSHNOTIFY)lp)->hdr.code) {
case PSN_KILLACTIVE :
SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
return TRUE;
case PSN_RESET : break;
case PSN_APPLY : {
if (IsDlgButtonChecked(hwnd,1001)==BST_CHECKED) ini.put("explorer.reuseInstances",0);
else ini.put("explorer.reuseInstances", IsDlgButtonChecked(hwnd,1005)==BST_CHECKED? 2:1);
for (int i=0; i<3; i++) if (IsDlgButtonChecked(hwnd,1002+i)==BST_CHECKED) { ini.put("explorer.openFlags",i); break; }
ini.put("explorer.openPreview", IsDlgButtonChecked(hwnd,1006)==BST_CHECKED);
SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
}return TRUE;
}break;
}
return FALSE;
}

BOOL OptionsCastingDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static optionsinfo*o = NULL;
switch(msg){
case WM_INITDIALOG :
o = (optionsinfo*)(((PROPSHEETPAGE*)lp)->lParam);
CheckDlgButton(hwnd, 1001, o->useJingles? BST_CHECKED : BST_UNCHECKED);
CheckDlgButton(hwnd, 1002, o->castNoFeedback? BST_CHECKED : BST_UNCHECKED);
CheckDlgButton(hwnd, 1003, o->castAutoTitle? BST_CHECKED : BST_UNCHECKED);
break;
case WM_COMMAND :
switch(LOWORD(wp)) {
case 1001: o->useJingles = IsDlgButtonChecked(hwnd,1001)==BST_CHECKED; break;
}break;
case WM_NOTIFY : 
switch (((LPPSHNOTIFY)lp)->hdr.code) {
case PSN_KILLACTIVE :
SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
return TRUE;
case PSN_RESET : break;
case PSN_APPLY :
o->castAutoTitle = IsDlgButtonChecked(hwnd, 1003)==BST_CHECKED;
o->castNoFeedback = IsDlgButtonChecked(hwnd, 1002)==BST_CHECKED;
o->useJingles = IsDlgButtonChecked(hwnd, 1001)==BST_CHECKED;
SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
return TRUE;
}break;
}
return FALSE;
}

BOOL OptionsAdvancedDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_INITDIALOG :
CheckDlgButton(hwnd, 1001, ini.get("g.scripting", true)? BST_CHECKED : BST_UNCHECKED);
CheckDlgButton(hwnd, 1002, ini.get("webserver.on", true)? BST_CHECKED : BST_UNCHECKED);
SetDlgItemText(hwnd, 1003, toTString(ini.get("webserver.port", 88)).c_str() );
EnableWindow(GetDlgItem(hwnd,1003), BST_CHECKED==IsDlgButtonChecked(hwnd,1002));
break;
case WM_COMMAND :
switch(LOWORD(wp)) {
case 1002:
EnableWindow(GetDlgItem(hwnd,1003), BST_CHECKED==IsDlgButtonChecked(hwnd,1002));
break;
}break;
case WM_NOTIFY : 
switch (((LPPSHNOTIFY)lp)->hdr.code) {
case PSN_KILLACTIVE :
SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
return TRUE;
case PSN_RESET : break;
case PSN_APPLY :
ini.put("g.scripting", IsDlgButtonChecked(hwnd,1001)==BST_CHECKED);
ini.put("webserver.on", IsDlgButtonChecked(hwnd,1002)==BST_CHECKED);
ini.put("webserver.port", (int)GetDlgItemInt(hwnd, 1003, NULL, FALSE));
SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
return TRUE;
}break;
}
return FALSE;
}

BOOL OptionsGeneralDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_INITDIALOG :
CheckDlgButton(hwnd, 1001, ini.get("g.reloadLast",true)? BST_CHECKED : BST_UNCHECKED);
CheckDlgButton(hwnd, 1002, ini.get("g.checkForUpdates",true)? BST_CHECKED : BST_UNCHECKED);
break;
case WM_COMMAND :
switch(LOWORD(wp)) {
}break;
case WM_NOTIFY : 
switch (((LPPSHNOTIFY)lp)->hdr.code) {
case PSN_KILLACTIVE :
SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
return TRUE;
case PSN_RESET : break;
case PSN_APPLY :
ini.put("g.reloadLast", IsDlgButtonChecked(hwnd,1001)==BST_CHECKED);
ini.put("g.checkForUpdates", IsDlgButtonChecked(hwnd,1002)==BST_CHECKED);
SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
return TRUE;
}break;
}
return FALSE;
}

void optionsDialog (int startPage) {
const int nPages = 5;
optionsinfo o;
o.inDevice = inDevice;
o.outDevice = outDevice;
o.jingleDevice = jingleDevice;
o.previewDevice = previewDevice;
o.feedbackDevice = feedbackDevice;
o.useJingles = useJingles;
o.castAutoTitle = castAutoTitle;
o.castNoFeedback = castNoFeedback;
PROPSHEETHEADER psh;
PROPSHEETPAGE pages[nPages];
{
PROPSHEETPAGE& p = pages[0];
p.dwSize = sizeof(PROPSHEETPAGE);
p.dwFlags = PSP_DEFAULT;
p.hInstance = hinst;
p.pszTemplate = TEXT(IDD_OPTIONS_GENERAL);
p.pfnDlgProc = OptionsGeneralDlgProc;
p.lParam = &o;
}
{
PROPSHEETPAGE& p = pages[1];
p.dwSize = sizeof(PROPSHEETPAGE);
p.dwFlags = PSP_DEFAULT;
p.hInstance = hinst;
p.pszTemplate = TEXT(IDD_OPTIONS_DEVICES);
p.pfnDlgProc = OptionsDevicesDlgProc;
p.lParam = &o;
}
{
PROPSHEETPAGE& p = pages[2];
p.dwSize = sizeof(PROPSHEETPAGE);
p.dwFlags = PSP_DEFAULT;
p.hInstance = hinst;
p.pszTemplate = TEXT(IDD_OPTIONS_CASTING);
p.pfnDlgProc = OptionsCastingDlgProc;
p.lParam = &o;
}
{
PROPSHEETPAGE& p = pages[3];
p.dwSize = sizeof(PROPSHEETPAGE);
p.dwFlags = PSP_DEFAULT;
p.hInstance = hinst;
p.pszTemplate = TEXT(IDD_OPTIONS_INTEGRATION);
p.pfnDlgProc = OptionsIntegrationDlgProc;
p.lParam = &o;
}
{
PROPSHEETPAGE& p = pages[4];
p.dwSize = sizeof(PROPSHEETPAGE);
p.dwFlags = PSP_DEFAULT;
p.hInstance = hinst;
p.pszTemplate = TEXT(IDD_OPTIONS_ADVANCED);
p.pfnDlgProc = OptionsAdvancedDlgProc;
p.lParam = &o;
}
psh.dwSize = sizeof(psh);
psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
psh.hwndParent = win;
psh.hInstance = hinst;
psh.pfnCallback = NULL;
psh.pszCaption = TEXT(MSG_OPTIONSDLG);
psh.nPages = nPages;
psh.nStartPage = startPage;
psh.ppsp = pages;
if (PropertySheet(&psh)) {
changeOutDevices(o.outDevice, o.previewDevice, o.feedbackDevice, o.jingleDevice);
changeInDevice(o.inDevice);
useJingles = o.useJingles;
castAutoTitle = o.castAutoTitle;
castNoFeedback = o.castNoFeedback;
}}

BOOL castDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static castinfo* c = NULL;
switch(msg){
case WM_INITDIALOG : {
c = (castinfo*)lp;
SendDlgItemMessage(hwnd, 1001, CB_ADDSTRING, 0, (LPARAM)TEXT(MSG_SHOUTCAST));
SendDlgItemMessage(hwnd, 1001, CB_ADDSTRING, 0, (LPARAM)TEXT(MSG_ICECAST));
SendDlgItemMessage(hwnd, 1001, CB_ADDSTRING, 0, (LPARAM)TEXT(MSG_CASTDIRECT));
SendDlgItemMessage(hwnd, 1001, CB_SETCURSEL, c->type, 0);
SetDlgItemText(hwnd, 1002, c->host.c_str());
SetDlgItemText(hwnd, 1004, c->name.c_str());
SetDlgItemText(hwnd, 1005, c->password.c_str());
SetDlgItemText(hwnd, 1007, c->url.c_str());
SetDlgItemText(hwnd, 1008, c->genre.c_str());
SetDlgItemText(hwnd, 1009, c->desc.c_str());
SetDlgItemInt(hwnd, 1003, c->port, FALSE);
for (int i=0, j=0; i<encoders.size(); i++) {
LPQCPLUGIN p = encoders[i];
if (!(p->flags&PF_CAN_CAST)) continue;
if (c->format<0) c->format=i;
SendDlgItemMessage(hwnd, 1006, CB_ADDSTRING, 0, (LPARAM)(toTString(p->desc).c_str()));
SendDlgItemMessage(hwnd, 1006, CB_SETITEMDATA, j++, i);
if (c->format==i) SendDlgItemMessage(hwnd, 1006, CB_SETCURSEL, j -1, 0);
}
}break;
case WM_COMMAND: switch(wp) {
case 2000 : {
int n = SendDlgItemMessage(hwnd, 1006, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1006, CB_GETCURSEL, 0, 0), 0);
LPQCPLUGIN p = encoders[n];
if(p->flags&PF_HAS_OPTIONS_DIALOG) p->query(NULL, PP_ENC_OPTIONS_DIALOG, hwnd, sizeof(HWND));
}break;
case IDCANCEL: EndDialog(hwnd,1); break;
case IDOK :
#define S(x,n) { int len = GetWindowTextLength(GetDlgItem(hwnd,n)); c->x.clear(); TCHAR buf[n+1]; GetDlgItemText(hwnd, n, buf, len+1); c->x=buf; }
S(host, 1002) S(name, 1004) S(password, 1005)
S(url, 1007) S(genre, 1008) S(desc, 1009)
#undef S
c->port = GetDlgItemInt(hwnd, 1003, NULL, FALSE);
c->type = SendDlgItemMessage(hwnd, 1001, CB_GETCURSEL, 0, 0);
c->format = SendDlgItemMessage(hwnd, 1006, CB_GETITEMDATA, SendDlgItemMessage(hwnd, 1006, CB_GETCURSEL, 0, 0), 0);
EndDialog(hwnd,2);
break;
}break;
}
return FALSE;
}

void stopCasting (void) {
stopMix();
CheckMenuItem(lect, IDM_CAST, MF_BYCOMMAND | MF_UNCHECKED);
if (curFeedback&&castNoFeedback) BASS_ChannelSetAttribute(curFeedback, BASS_ATTRIB_VOL, curRecVol);
}

void castDialog () {
castinfo c;
c.port = 8000;
c.type=0;
c.format = -1;
if (DialogBoxParam(hinst, TEXT(IDD_CASTDLG), win, (DLGPROC)castDlgProc, (LPARAM)&c) >1) {
tstring addr;
switch(c.type){
case 0 : // shoutcast
wsnprintf(addr, 512, TEXT("%s:%d"), c.host.c_str(), c.port);
break;
case 1 : // Icecast
wsnprintf(addr, 512, TEXT("%s:%d/mount"), c.host.c_str(), c.port);
break;
case 2: // direct
wsnprintf(addr, 512, TEXT("0.0.0.0:%d"), c.port);
break;
}
if (curCasting>=0) { stopCasting(); Sleep(1000); }
QCPLUGIN& e = *(encoders[c.type]);
curCasting = c.type;
curDecode = c.format;
const char* mimetype = e.get(NULL, PP_MIMETYPE, 0, 0);
int quality = e.get(NULL, PP_ENC_QUALITY, 0, 0);
startMix();
startEncode();
if (
(c.type!=2 && !BASS_Encode_CastInit(curEncode, toString(addr).c_str(), toString(c.password).c_str(), mimetype, toString(c.name).c_str(), toString(c.url).c_str(), toString(c.genre).c_str(), toString(c.desc).c_str(), NULL, quality, FALSE))
|| (c.type==2 && !BASS_Encode_ServerInit(curEncode, toString(addr).c_str(), 88200, 88200, 0, NULL, NULL))
) {
MessageBox(win, TEXT(ERROR), toTString(MSG_CASTFAIL).c_str(), MB_OK | MB_ICONSTOP);
}
else {
if (curFeedback&&castNoFeedback) BASS_ChannelSetAttribute(curFeedback, BASS_ATTRIB_VOL, 0);
CheckMenuItem(lect, IDM_CAST, MF_BYCOMMAND | MF_CHECKED);
}}}

int CALLBACK BrowseForFolderCallback(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
TCHAR szPath[MAX_PATH+1];
	switch(uMsg) 	{
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED: 
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szPath))  		{
			SendMessage(hwnd, BFFM_SETSTATUSTEXT,0,(LPARAM)szPath);	
		} 		break;
case BFFM_VALIDATEFAILED:
MessageBeep(0);
return 1;
	}
	return 0;
}

BOOL BrowseFolders(HWND hwnd, LPTSTR lpszFolder, LPTSTR lpszTitle) {
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	BOOL bResult = FALSE;
	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_STATUSTEXT | BIF_USENEWUI| BIF_EDITBOX | BIF_VALIDATE | 0x200;
	bi.lpfn = BrowseForFolderCallback;
	bi.lParam = (LPARAM)lpszFolder;
	pidl = SHBrowseForFolder(&bi);
	if (pidl && SHGetPathFromIDList(pidl,lpszFolder))  					return TRUE;
else 			return FALSE;
}

void handleHotkey (HWND hwnd) {
DWORD time = GetTickCount();
BOOL stillDown = GetAsyncKeyState(hotkeyKey)<0, 
sub = time-hotkeySub<=1500;
if (sub) hotkeySub=time; 
if (stillDown) { if (++hotkeyCount>3) {
if (hotkeyAction==IDM_NEXT) keyDown(sub? 'Q' : VK_RIGHT);
else if (hotkeyAction==IDM_PREV) keyDown(sub? 'A' : VK_LEFT);
} return; }
KillTimer(hwnd,2);
if (hotkeyCount>1 && hotkeyAction==IDM_PAUSE) hotkeySub=time;
if (hotkeyCount==1) {
if (sub && hotkeyAction==IDM_NEXT) keyDown('W');
else if (sub && hotkeyAction==IDM_PREV) keyDown('S');
else action(hotkeyAction);
}
else if (hotkeyCount>=7 && hotkeyAction==IDM_PAUSE) keyDown('O');
hotkeyKey = hotkeyCount = hotkeyAction=0;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
case WM_KEYDOWN: keyDown(wParam); break;
case WM_KEYUP: keyDown(wParam); break;
case WM_TIMER : 
switch(wParam){
case 1: updateInfo(); break;
case 2:  handleHotkey(hwnd); break;
}break;
case WM_HOTKEY :
if (GetAsyncKeyState((lParam>>16))<0) {
if (hotkeyCount<=0) {
hotkeyKey = (lParam>>16);
hotkeyAction=LOWORD(wParam);
hotkeyCount=1;
SetTimer(hwnd, 2, 250, NULL);
}
break;
}//no break
case WM_COMMAND : 
action(LOWORD(wParam)); 
break;
case WM_COPYDATA : return parseExternalData((COPYDATASTRUCT*)lParam); 
case WM_ACTIVATE: if(wParam>0) {
ctrlDown = shiftDown = FALSE; 
SetFocus(lbl);
}break;
/*case WM_DEVICECHANGE :
if (wParam==0x07) PostMessage(hwnd, WM_USER+2000, 25, 0);
break;
case WM_USER+2000 :
if (wParam<=0) {
fillDeviceList();
printf("Before = %d, %d\r\n", curInDevice, curOutDevice);
curInDevice = BASS_RecordGetDevice();
curOutDevice = BASS_GetDevice();
printf("After = %d, %d\r\n", curInDevice, curOutDevice);
auto(out2, outDevices.find(curOutDeviceName));
auto(in2, inDevices.find(curOutDeviceName));
if (in2!=inDevices.end()) switchInDevice(in2->second);
if (out2!=outDevices.end()) switchOutDevice(out2->second);
} else {
Sleep(10);
PostMessage(hwnd, WM_USER+2000, wParam -1, 0);
}break;*/
       case WM_DESTROY:             PostQuitMessage (0);       break;
default :             return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}


// Natural code
void keyUp (int k) {
switch (k) {
case VK_CONTROL : ctrlDown = false; break;
case VK_SHIFT : shiftDown = false; break;
}}

void keyDown (int k) {
if (k==VK_CONTROL) ctrlDown=TRUE;
else if (k==VK_SHIFT) shiftDown=TRUE;
else if (!ctrlDown) 
switch (k) {
case VK_UP : case 0xAF : case 0x68 :
if (shiftDown&& ctrlDown) setRecCastVol(curRecCastVol +0.025f);
else if (ctrlDown&&!shiftDown) setCastVol(curCastVol +0.025f);
else if (shiftDown&&!ctrlDown) setRecVol(curRecVol +0.025f);
else setSongVol(curVol + 0.025f); 
break;
case VK_DOWN : case 0xAE : case 0x62 :
if (shiftDown&& ctrlDown) setRecCastVol(curRecCastVol -0.025f);
else if (ctrlDown&&!shiftDown) setCastVol(curCastVol -0.025f);
else if (shiftDown&&!ctrlDown) setRecVol(curRecVol -0.025f);
else setSongVol(curVol -0.025f); 
break;
case VK_RIGHT : case 0x66 : seekSong(shiftDown? 30 : 5); break;
case VK_LEFT : case 0x64 : seekSong(shiftDown? -30 : -5); break;
case VK_NEXT : seekSong((shiftDown? 300 : 30)); break;
case VK_PRIOR : seekSong(-(shiftDown? 300 : 30)); break;
case 'X' : case 0xA8 : case 0x60 : playSong(TRUE); break;
case 'C' : case ' ' : case 0xB3 : case 0xB2 : case 0xA9 : case 0x65 : pauseSong(); break;
case 'V' : stopSong(false); break;
case 'L' :  case VK_END : case 0xB0 : case 0xA7 : case 0x69 : nextSong(); break;
case 'K' : case VK_HOME : case 0xB1 : case 0xA6 : case 0x67 : prevSong(); break;
case 'A' : 
if (shiftDown) setSongRate(curRate -0.025f);
else setSongSpeed(curSpeed -0.025f); 
break;
case 'Q' : 
if (shiftDown) setSongRate(curRate +0.025f);
else setSongSpeed(curSpeed + 0.025f); 
break;
case 'W' : 
if (shiftDown) setSongPitch(curTransposition + 0.1f);
else setSongPitch(curTransposition +1); 
break;
case 'S' : 
if (shiftDown) setSongPitch(curTransposition -0.1f);
else setSongPitch(curTransposition -1);  
break;
case 'I' : case 0x61 :
if (shiftDown) {
action(IDM_INTRO_MODE);
if (sayString) sayString(toTString(curIntroMode? (MSG_INTRO_MODE_ON) : (MSG_INTRO_MODE_OFF)).c_str(), TRUE);
} else { 
action(IDM_LOOP); 
if (sayString) sayString(toTString(curLoop? MSG_LOOP_ON : MSG_LOOP_OFF).c_str(), TRUE);
}
break;
case 'P' : case 0x63 : 
if (shiftDown) {
action(IDM_REVERSE);
if (sayString) sayString(toTString(curReverse? (MSG_REVERSE_ON) : (MSG_REVERSE_OFF)).c_str(), TRUE);
} else {
action(IDM_RANDOM); 
if (sayString) sayString(toTString(curRandom? (MSG_RANDOM_ON) : (MSG_RANDOM_OFF)).c_str(), TRUE);
}
break;
case 'M' :
action(IDM_CROSSFADE);
if (sayString) sayString(toTString(crossFade>0? (MSG_CROSSFADE_ON) : (MSG_CROSSFADE_OFF)).c_str(), TRUE);
break;
case 'O' :  action(IDM_RESET);  break;
case VK_DELETE : delSongNum(curSong); break;
case 'E' :  setEqualizerValueDelta(curHandle, 0, 0.5);  break;
case 'R' : setEqualizerValueDelta(curHandle, 1, 0.5); break;
case 'T' : setEqualizerValueDelta(curHandle, 2, 0.5); break;
case 'Z' : setEqualizerValueDelta(curHandle, 3, 0.5); break;
case 'U' :  setEqualizerValueDelta(curHandle, 4, 0.5);  break;
case 'D' : setEqualizerValueDelta(curHandle, 0, -0.5); break;
case 'F' : setEqualizerValueDelta(curHandle, 1, -0.5); break;
case 'G' : setEqualizerValueDelta(curHandle, 2, -0.5); break;
case 'H' : setEqualizerValueDelta(curHandle, 3, -0.5); break;
case 'J' : setEqualizerValueDelta(curHandle, 4, -0.5); break;
}
else if (ctrlDown) switch(k) {
case VK_UP :
if (!shiftDown && curRecord) setVol(curRecord, getVol(curRecord) +0.025f);
break;
case VK_DOWN :
if (!shiftDown && curRecord) setVol(curRecord, getVol(curRecord) -0.025f);
break;
}}

void action (int n) {
ctrlDown = false;
shiftDown = false;
switch (n) {
case IDM_PAUSE : pauseSong(); break;
case IDM_NEXT : nextSong(); break;
case IDM_PREV : prevSong(); break;
case IDM_VOLPLUS: setPreviewSongVol(0.05f); break;
case IDM_VOLMINUS: setPreviewSongVol(-0.05f);  break;
case IDM_SEEK_LEFT: seekPreviewSong(-5); break;
case IDM_SEEK_RIGHT: seekPreviewSong(5); break;
case IDM_OPEN : openDialog(FALSE); break;
case IDM_OPENAPPEND : openDialog(TRUE); break;
case IDM_OPENURL : openUrlDialog(FALSE); break;
case IDM_OPENURLAPPEND : openUrlDialog(TRUE); break;
case IDM_OPENDIR: openDirDialog(FALSE); break;
case IDM_OPENDIRAPPEND: openDirDialog(TRUE); break;
case IDM_SAVEPLAYLIST : savePlaylistDialog(); break;
case IDM_ENCODE : saveEncodeDialog(); break;
case IDM_LOOP :
curLoop=!curLoop;
if (curHandle!=NULL) {
BASS_ChannelFlags(curHandle, (curLoop? BASS_SAMPLE_LOOP : 0), BASS_SAMPLE_LOOP);
BASS_ChannelFlags(curHandle, curLoop? BASS_MUSIC_STOPBACK:0, BASS_MUSIC_STOPBACK);
}
CheckMenuItem(lect, IDM_LOOP, MF_BYCOMMAND | (curLoop? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_RANDOM :
curRandom = !curRandom;
CheckMenuItem(lect, IDM_RANDOM, MF_BYCOMMAND | (curRandom? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_REVERSE :
curReverse=!curReverse;
CheckMenuItem(lect, IDM_REVERSE, MF_BYCOMMAND | (curReverse? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_INTRO_MODE:
curIntroMode=!curIntroMode;
CheckMenuItem(lect, IDM_INTRO_MODE, MF_BYCOMMAND | (curIntroMode? MF_CHECKED : MF_UNCHECKED));
if (curIntroMode) {
if (BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetPosition(curHandle, BASS_POS_BYTE)) >=15) { if (curReverse) prevSong(); else nextSong(); }
else BASS_ChannelSetSync(curHandle, BASS_SYNC_POS, BASS_ChannelSeconds2Bytes(curHandle, 15), introModeTimeout, NULL);
}
break;
case IDM_CROSSFADE :
crossFade *= -1;
CheckMenuItem(lect, IDM_CROSSFADE, MF_BYCOMMAND | (crossFade>0? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_EFF_MODE1 :
case IDM_EFF_MODE2 :
case IDM_EFF_MODE3 :
fxTarget = 1+n -IDM_EFF_MODE1;
CheckMenuRadioItem(GetSubMenu(effectsMenu,0), 0, 3, fxTarget -1, MF_BYPOSITION);
break;
case IDM_RESET :
setSongPitch(0);
setSongSpeed(1.0f);
setSongRate(1.0f);
for (int i=0; i < 5; i++) setEqualizerValue(curHandle,i,0);
break;
case IDM_SHOWPLAYLIST: showPlaylist(); break;
case IDM_SHOWMIDI: showMIDIPanel(); break;
case IDM_SHOWTEXTPANEL: showTextPanel(); break;
case IDM_SHOWLEVELS: showLevelsPanel(); break;
case IDM_SHOWINFO: fileInformationDialog(curSong); break;
case IDM_RADIO: showRadio(); break;
case IDM_GOTOTIME: gotoTimeDialog(); break;
case IDM_DELCURSONG :
if (curSong<0 || curSong>=playlist.size()) break;
delSongNum(curSong);
break;
case IDM_RECORD:
if (curRecord) stopRecord();
else startRecord();
CheckMenuItem(lect, IDM_RECORD, MF_BYCOMMAND | (curRecord? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_INDEXPLAYLIST: doIndexAllPlaylist(); break;
case IDM_OPTIONS: optionsDialog(); break;
case IDM_OPTIONS_DEVICES: optionsDialog(1); break;
case IDM_CAST: 
if (curCasting>=0) stopCasting();
else castDialog(); 
break;
case IDM_ABOUT : aboutDialog(); break;
case IDM_QUIT : PostQuitMessage(0); break;
case IDM_DEBUG : PostMessage(win, WM_DEVICECHANGE, 0x07, 0); break;
}
if (n>=IDM_EFFECT && n<=IDM_EFFECT+500) {
effectinfo& e = effects[n - IDM_EFFECT];
if (e.hfx) stopEffect(e);
else if (!startEffect(e)) MessageBeep(MB_OK);
CheckMenuItem(effectsMenu, n, MF_BYCOMMAND | (e.hfx? MF_CHECKED : MF_UNCHECKED));
}
else if (n>=IDM_ENCODEALL && n<=IDM_ENCODEALL+100) {
curDecodeDir.clear();
TCHAR dir[512]={0};
if (!BrowseFolders(win, dir, TEXT(MSG_FILESAVEALL2))) return;
curDecodeDir = toString(dir);
stopSong();
curSong = -1;
keepDecode = TRUE;
n -= IDM_ENCODEALL;
curDecode = n;
LPQCPLUGIN p = encoders[n];
if (p->flags&PF_HAS_OPTIONS_DIALOG) p->query(NULL, PP_ENC_OPTIONS_DIALOG, win, sizeof(HWND));
if (curReverse) prevSong(); else nextSong();
}
else if (n>=IDM_CUSTOMCOMMAND && n<=IDM_CUSTOMCOMMAND+500) {
executeCustomCommand(n);
}
//actions suite
}

void aboutDialog () {
MessageBox(win, toTString(MSG_ABOUTTEXT).c_str(), toTString(MSG_ABOUTTITLE).c_str(), MB_OK|MB_ICONINFORMATION);
}

int parseExternalData (COPYDATASTRUCT* cp) {
int param = ini.get("explorer.reuseInstances",1);
if (!param) return param;
TCHAR* lpszArgumentW = (TCHAR*)(cp->lpData);
int argc = 0;
wchar_t** argv = CommandLineToArgvW(lpszArgumentW,&argc);
DWORD result = parseArgs(argv, argc, false);
LocalFree(argv);
return result? result : param;
}

static DWORD StdinPassing (LPVOID lp) {
clearPlaylist();
appendToPlaylist(TEXT("?tcp?localhost:123"));
return 0;
}

DWORD parseArgs (const TCHAR** argv, int argc, bool startup) {
DWORD result = 0;
int omode = ini.get("explorer.openFlags", 1);
bool clear = startup||(omode&1), listen=(omode&2);
int jumpto = -1;
for (int i=1; i<argc; i++) {
if (argv[i][0]=='/') {
wstring arg = argv[i];
if (arg==L"/a") listen=clear=false;
else if (arg==L"/c") clear=true;
else if (arg==L"/p") { listen=true; clear=false; }
} else {
if (!startup && !wcscmp(argv[i],L"-")) {
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StdinPassing, (LPVOID)(0), 0, NULL);
result = 0xFFFE;
continue;
}
if (clear) clearPlaylist();
appendToPlaylist(argv[i]);
if (listen) jumpto=playlist.size()-1;
clear=false;
listen = false;
}}
if (jumpto>=0 && jumpto<playlist.size()) playSongNum(jumpto);
return result;
}

void CALLBACK resetWindowTitle (HSYNC hs, DWORD ch, DWORD data, LPVOID udata) {
playlist[curSong].metadataSet = false;
updateWindowTitle();
}

void updateWindowTitle () {
const playlistitem& it  = fillMetadata(playlist[curSong], BASS_FX_TempoGetSource(curHandle));
tstring str, tstr = it.artist.size()>0? toTString(it.artist) + TEXT(" - ") + toTString(it.title) : toTString(it.title);
wsnprintf(str, 512, TEXT("%d. %s -- 6player"), curSong+1, tstr.c_str());
SetWindowText(win, str.c_str());
if (playlistdlg) SendMessage(playlistdlg, WM_USER+10, curSong, -1);
//if (curCasting>=0 && castingAutoTitle) BASS_Encode_CastSetTitle(curEncode, tstr.c_str(), NULL);
}

void addFileType (const string& desc, const string& ext, int ctype) {
if (desc.size()>0 && ext.size()>0) {
pair<string,string> p;
p.first = desc;
p.second = ext;
formats.push_back(p);
}
if (ctype>0 && typeMap.find(ctype)==typeMap.end()) typeMap[ctype] = desc;
}

void addPlaylistWriteFileType (const string& desc, const string& ext) {
pair<string,string> p;
p.first = desc;
p.second = ext;
playlistWriteFormats.push_back(p);
}

void addEncodeFileType (const string& desc, const string& ext) {
pair<string,string> p;
p.first = desc;
p.second = ext;
encodeFormats.push_back(p);
}

bool startEffect (effectinfo& e)  {
if (e.hfx) stopEffect(e);
e.curTarget = fxTarget;
DWORD targetHandle = 0;
if (e.curTarget==1 && curHandle) targetHandle = curHandle;
else if (e.curTarget ==2 && curRecord)  targetHandle = curRecord;
else if (e.curTarget==3 && curMixHandle) targetHandle = curMixHandle;
if (e.type==0x8000000) { // VST
e.hfx = BASS_VST_ChannelSetDSP(targetHandle, e.data+84, 0, 1);
if (!e.hfx) e.hfx=-1;
else {
int preset = *(int*)(e.data+80);
if (preset>0 && preset<=BASS_VST_GetProgramCount(e.hfx)) BASS_VST_SetProgram(e.hfx, preset);
for (int i=0, n=BASS_VST_GetParamCount(e.hfx); i<n && i<20; i++) {
float val = *(float*)(e.data +4*i);
if (val>=0 && val<=1) BASS_VST_SetParam(e.hfx, i, val);
}}}
else {
e.hfx = BASS_ChannelSetFX(targetHandle, e.type, 1);
if (e.hfx) BASS_FXSetParameters(e.hfx, e.data);
else e.hfx = -1;
}
return true;
}

void stopEffect (effectinfo& e) {
DWORD targetHandle = 0;
if (e.curTarget==1) targetHandle = curHandle;
else if (e.curTarget==2) targetHandle = curRecord;
else if (e.curTarget==3) targetHandle = curMixHandle;
if (e.type==0x8000000) BASS_VST_ChannelRemoveDSP(targetHandle, e.hfx);
else BASS_ChannelRemoveFX(targetHandle, e.hfx);
e.hfx=NULL;
}

void stopMix () {
BASS_Mixer_ChannelRemove(curCopyHandle);
BASS_ChannelStop(curMixHandle);
BASS_StreamFree(curMixHandle);
BASS_StreamFree(curCopyHandle);
curMixHandle=NULL;
curCopyHandle = NULL;
}

void startMix (void) {
if (curMixHandle) stopMix();
curMixHandle = BASS_Mixer_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE | BASS_MIXER_RESUME);
if (curRecord) BASS_Mixer_StreamAddChannel(curMixHandle, curRecord, BASS_STREAM_AUTOFREE);
if (curHandle) {
curCopyHandle = BASS_StreamCreateCopy(curHandle, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE, 0);
BASS_Mixer_StreamAddChannel(curMixHandle, curCopyHandle, BASS_STREAM_AUTOFREE);
}
for (int i=0, n=effects.size(); i<n; i++) {
effectinfo& e = effects[i];
if (e.hfx&&e.curTarget==3) {
e.hfx = BASS_ChannelSetFX(curMixHandle, e.type, 1);
if (e.hfx) BASS_FXSetParameters(e.hfx, e.data);
}}
}

BOOL CALLBACK recordproc (HRECORD hrec, const void* buf, DWORD len, HSTREAM hs) {
static const float maxdiff = 0.1f;
static float oldLevel = 1;
BASS_StreamPutData(hs, buf, len);
/*DWORD dLevel = BASS_ChannelGetLevel(hrec);
float level = 1 - ((LOWORD(dLevel) + HIWORD(dLevel)) / 65536.0);
level *= level;
float diff = level-oldLevel;
if (diff>maxdiff) diff=maxdiff;
else if (diff<-maxdiff) diff=-maxdiff;
level = oldLevel+diff;
oldLevel=level;
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_VOL, curVol*level);
BASS_ChannelSetAttribute(curCopyHandle, BASS_ATTRIB_VOL, curCastVol*level);*/
return TRUE;
}

void startRecord () {
if (curRecord) stopRecord();
BASS_RecordInit(inDevice);
BASS_RecordSetDevice(inDevice);
inDevice = BASS_RecordGetDevice();
setOutDevice(feedbackDevice);
curFeedback = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, STREAMPROC_PUSH, NULL);
if (curFeedback&&castNoFeedback&&curCasting>=0) BASS_ChannelSetAttribute(curFeedback, BASS_ATTRIB_VOL, 0);
else BASS_ChannelSetAttribute(curFeedback, BASS_ATTRIB_VOL, curRecVol);
setOutDevice(outDevice);
curRecord = BASS_RecordStart(44100, 2, BASS_SAMPLE_FLOAT, recordproc, curFeedback);
if (curMixHandle) {
if (curFeedback2) { BASS_ChannelStop(curFeedback2); BASS_Mixer_ChannelRemove(curFeedback2); BASS_StreamFree(curFeedback2); }
curFeedback2 = BASS_StreamCreateCopy(curRecord, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE, 0);
BASS_ChannelSetAttribute(curFeedback2, BASS_ATTRIB_VOL, curRecCastVol);
BASS_Mixer_ChannelFlags(curCopyHandle, BASS_MIXER_LIMIT, BASS_MIXER_LIMIT);
BASS_Mixer_StreamAddChannel(curMixHandle, curFeedback2, BASS_STREAM_AUTOFREE);
}
if (curFeedback) BASS_ChannelPlay(curFeedback, FALSE);
for (int i=0, n=effects.size(); i<n; i++) {
effectinfo& e = effects[i];
if (e.hfx&&e.curTarget==2) {
e.hfx = BASS_ChannelSetFX(curRecord, e.type, 1);
if (e.hfx) BASS_FXSetParameters(e.hfx, e.data);
}}
}

void stopRecord () {
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_VOL, curVol);
BASS_ChannelSetAttribute(curCopyHandle, BASS_ATTRIB_VOL, curCastVol);
BASS_ChannelStop(curRecord);
if (curMixHandle) BASS_Mixer_ChannelRemove(curRecord);
if (curFeedback) BASS_ChannelStop(curFeedback);
curFeedback = NULL;
curRecord = NULL;
}

void startEncode (void) {
Beep(800,300);
LPQCPLUGIN p = encoders[curDecode];
DWORD flags = 0;
p->get(p, PP_ENC_FLAGS, &flags, sizeof(DWORD));
tstring cmdl;
{
char buf[1024];
p->get(p, curCasting>=0? PP_ENC_CAST_COMMANDLINE : PP_ENC_COMMANDLINE, buf, 1024);
cmdl = toTString(buf);
}
if (curDecodeFn.size()<=0) {
const string& curFile = playlist[curSong].file;
int bks = curFile.rfind('\\'), sls = curFile.rfind('/');
if (bks>=curFile.size()) bks = -1;
if (sls>=curFile.size()) sls = -1;
if (sls>bks) bks=sls;
curDecodeFn = curDecodeDir + "\\" + curFile.substr(bks+1) + "." + toString(p->query(NULL, PP_DEFEX, 0, 0));
}
playlistitem& it = playlist[curSong];
DWORD curHandle2 =  BASS_FX_TempoGetSource(curHandle);
fillMetadata(it, curHandle2);
replace(cmdl, TEXT("%f"), toTString(curDecodeFn));
replace(cmdl, TEXT("%t"), toTString(it.title));
replace(cmdl, TEXT("%a"), toTString(it.artist));
replace(cmdl, TEXT("%l"), toTString(it.album));
printf("Command line = %ls\r\n", cmdl.c_str());
if (curCasting>=0) flags |= BASS_ENCODE_NOHEAD | BASS_ENCODE_FP_16BIT;
p->get(p, PP_ENC_FLAGS, &flags, sizeof(int));
curEncode = BASS_Encode_Start(curMixHandle? curMixHandle : curHandle, (const char*)(cmdl.c_str()), flags | USE_UNICODE, NULL, NULL);
printf("enc=%p, h=%p, mix=%p, copy=%p, encodedOn=%p, err=%d\r\n", curEncode, curHandle, curMixHandle, curCopyHandle, BASS_Encode_GetChannel(curEncode), BASS_ErrorGetCode());
if (curEncode) {
p->get(p, PP_ENC_PROC, (void*)curHandle, curEncode);
DWORD threadId;
CreateThread(NULL, 0, encodeautoproc, (LPVOID)(curMixHandle? curMixHandle:curHandle), 0, &threadId);
}
else MessageBox(win, TEXT(ERROR), toTString(MSG_ENCODEFAIL).c_str(), MB_OK | MB_ICONSTOP);
if (!keepDecode) curDecode = -1;
curDecodeFn.clear();
}

void startJingle (string fn) {
BASS_Mixer_ChannelFlags(curCopyHandle, BASS_MIXER_LIMIT, BASS_MIXER_LIMIT);
setOutDevice(jingleDevice);
DWORD jingle = getStream(fn, BASS_SAMPLE_FX | BASS_STREAM_AUTOFREE);
setOutDevice(outDevice);
DWORD jingleCopy = BASS_StreamCreateCopy(jingle, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE, 0);
BASS_ChannelSetAttribute(jingle, BASS_ATTRIB_VOL, curVol);
BASS_ChannelSetAttribute(jingleCopy, BASS_ATTRIB_VOL, curCastVol);
BASS_Mixer_StreamAddChannel(curMixHandle, jingleCopy, BASS_STREAM_AUTOFREE);
BASS_ChannelPlay(jingle,TRUE);
}

void startSongPreview (string fn) {
if (isdir(toTString(fn))) return;
if (curPreviewHandle) stopSongPreview();
setOutDevice(previewDevice);
curPreviewHandle = getStream(fn, BASS_SAMPLE_LOOP | BASS_SAMPLE_FX);
BASS_ChannelSetAttribute(curPreviewHandle, BASS_ATTRIB_VOL, curVol);
BASS_ChannelPlay(curPreviewHandle, FALSE);
setOutDevice(outDevice);
}

bool stopSongPreview () {
if (!curPreviewHandle) return false;
BASS_ChannelStop(curPreviewHandle);
BASS_StreamFree(curPreviewHandle);
BASS_MusicFree(curPreviewHandle);
curPreviewHandle=NULL;
return true;
}

BOOL loadSong (const string& fn) {
if (curHandle) stopSong();
DWORD flags = (curDecode>=0? BASS_STREAM_DECODE:0) | (curLoop&&(curDecode<0||curCasting>=0)?BASS_SAMPLE_LOOP:0);
curHandle = getStream(fn, flags);
if (curHandle==NULL) return FALSE;
curStreamLen = 0;
playSong(TRUE);
return TRUE;
}

void CALLBACK introModeTimeout (HSYNC handle, DWORD chan, DWORD param, LPVOID udata) {
if (!curIntroMode) return;
stopSong();
if (curReverse) prevSong();
else nextSong();
}

void playSong (BOOL restart) {
if (curHandle==NULL) {
if (curSong>=0 && curSong<playlist.size()) {
BOOL result = loadSong(playlist[curSong].file);
if (!result) {
playlist.erase(playlist.begin() +curSong);
if (playlistRevDir) { if (curReverse) nextSong(); else prevSong(); }
else { curSong--; if (curReverse) prevSong(); else nextSong(); }
}}
return;
}
playlistRevDir = FALSE;
updateWindowTitle();

if (curDecode<0 && crossFade>0 && restart && BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetLength(curHandle, BASS_POS_BYTE))>=30) {
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_VOL, 0);
BASS_ChannelSlideAttribute(curHandle, BASS_ATTRIB_VOL, curVol, crossFade);
if (curCopyHandle) BASS_ChannelSetAttribute(curCopyHandle, BASS_ATTRIB_VOL, 0);
if (curCopyHandle) BASS_ChannelSlideAttribute(curCopyHandle, BASS_ATTRIB_VOL, curCastVol, crossFade);
}
if (curIntroMode) BASS_ChannelSetSync(curHandle, BASS_SYNC_POS, BASS_ChannelSeconds2Bytes(curHandle, 15), introModeTimeout, NULL);
if (curDecode>=0) startEncode();
else BASS_ChannelPlay(curHandle, restart);
if (playlistdlg) SendMessage(playlistdlg, WM_USER, 16, curSong);
}

void pauseSong () {
if (curHandle==NULL) return;
int state = BASS_ChannelIsActive(curHandle);
if (state == BASS_ACTIVE_PLAYING) BASS_ChannelPause(curHandle);
else playSong(FALSE);
}

void CALLBACK freeSong (HSYNC sync, DWORD handle, DWORD data = 0, LPVOID udata = NULL) {
if (curMixHandle) BASS_Mixer_ChannelRemove(handle);
BASS_ChannelStop(handle);
DWORD d1 = BASS_FX_TempoGetSource(handle);
BASS_StreamFree(handle);
BASS_StreamFree(d1);
BASS_MusicFree(d1);
BASS_StreamFree(handle);
if (handle==curHandle) curHandle = NULL;
}

void stopSong (bool fade) {
if (curHandle==NULL) return;
if (!fade || crossFade<=0 || BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetLength(curHandle, BASS_POS_BYTE))<30) freeSong(NULL, curHandle);
else {
BASS_ChannelSetSync(curHandle, BASS_SYNC_MIXTIME | BASS_SYNC_ONETIME | BASS_SYNC_SLIDE, 0, freeSong, NULL);
BASS_ChannelSlideAttribute(curHandle, BASS_ATTRIB_VOL, 0.0f, crossFade);
if (curCopyHandle) BASS_ChannelSlideAttribute(curCopyHandle, BASS_ATTRIB_VOL, 0.0f, crossFade);
if (curMixHandle) BASS_Mixer_ChannelFlags(curCopyHandle, BASS_MIXER_LIMIT, BASS_MIXER_LIMIT);
curHandle = NULL;
}}

void delSongNum (int n) {
if (n<0 || n>=playlist.size()) return;
if (n==curSong) stopSong();
playlist.erase(playlist.begin() +n);
if (curSong==n) playSong(TRUE);
if (playlistdlg) SendMessage(playlistdlg, WM_USER, 15, 0);
}

void playSongNum (int n) {
if (n<0 || n>=playlist.size()) return;
if (curHandle && BASS_ChannelIsActive(curHandle)) stopSong();
curHandle = NULL;
if (curDecode && !keepDecode) curDecode = -1;
curSong=n;
playSong(TRUE);
}

void nextSong () {
playlistRevDir = FALSE;
if (curRandom && !keepDecode) {
curSong = randint(playlist.size());
}
else {
curSong++;
if (curSong>=playlist.size()) {
if (keepDecode) { keepDecode = FALSE; curDecode = -1; }
curSong = 0;
}}
if (curHandle && BASS_ChannelIsActive(curHandle)) stopSong();
curHandle = NULL;
playSong(TRUE);
if (curDecode && !keepDecode) curDecode = -1;
}

void prevSong () {
playlistRevDir = TRUE;
if (curRandom) {
curSong = randint(playlist.size());
}
else {
curSong--;
if (curSong<0) curSong+=playlist.size();
}
if (curHandle && BASS_ChannelIsActive(curHandle)) stopSong();
curHandle = NULL;
playSong(TRUE);
if (curDecode && !keepDecode) curDecode = -1;
}

void playlistSwap (int n1, int n2, int n1p, int n2p) {
if (n1<0 || n2<0 || n1>=playlist.size() || n2>=playlist.size()) return;
playlistitem tmp = playlist[n1];
playlist[n1] = playlist[n2];
playlist[n2] = tmp;
if (curSong==n1) curSong=n2;
else if (curSong==n2) curSong=n1;
updateWindowTitle();
if (playlistdlg) {
SendMessage(playlistdlg, WM_USER+10, n1, n1p);
SendMessage(playlistdlg, WM_USER+10, n2, n2p);
SendMessage(playlistdlg, WM_USER, 17, n1p);
}}

float getVol (DWORD h) {
float f=0;
BASS_ChannelGetAttribute(h, BASS_ATTRIB_VOL, &f);
return f;
}

float setVol (DWORD h, float f) {
f = minmax(0.0f,f,1.0f);
BASS_ChannelSetAttribute(h, BASS_ATTRIB_VOL, f);
return f;
}

float getPitch (DWORD h) {
float f=0;
BASS_ChannelGetAttribute(h, BASS_ATTRIB_TEMPO_PITCH, &f);
return f;
}

void setPitch (DWORD h, float f) {
BASS_ChannelSetAttribute(h, BASS_ATTRIB_TEMPO_PITCH, minmax(-60.0f,f,60.0f));
}

void setPreviewSongVol (float f) {
if (curPreviewHandle) {
float vol = curVol;
BASS_ChannelGetAttribute(curPreviewHandle, BASS_ATTRIB_VOL, &vol);
vol+=f;
if (vol>1) vol=0;
if (vol<0) vol=0;
BASS_ChannelSetAttribute(curPreviewHandle, BASS_ATTRIB_VOL, vol);
}
else setSongVol(curVol +f);
}

void setCastVol (float f) {
curCastVol = minmax(0.0f,f,1.0f);
if (curCopyHandle==NULL) return;
BASS_ChannelSetAttribute(curCopyHandle, BASS_ATTRIB_VOL, curCastVol);
if (levelsdlg) SendDlgItemMessage(levelsdlg, 1002, TBM_SETPOS, TRUE, 1000 * curCastVol);
}

void setRecVol (float f) {
curRecVol = minmax(0.0f,f,1.0f);
if (curFeedback==NULL) return;
BASS_ChannelSetAttribute(curFeedback, BASS_ATTRIB_VOL, curRecVol);
if (levelsdlg) SendDlgItemMessage(levelsdlg, 1001, TBM_SETPOS, TRUE, 1000 * curRecVol);
}

void setRecCastVol (float f) {
curRecCastVol = minmax(0.0f,f,1.0f);
if (curFeedback2==NULL) return;
BASS_ChannelSetAttribute(curFeedback2, BASS_ATTRIB_VOL, curRecCastVol);
if (levelsdlg) SendDlgItemMessage(levelsdlg, 1003, TBM_SETPOS, TRUE, 1000 * curRecCastVol);
}

void setSongVol (float f) {
curVol = minmax(0.0f,f,1.0f);
if (curHandle==NULL) return;
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_VOL, curVol);
if (levelsdlg) SendDlgItemMessage(levelsdlg, 1000, TBM_SETPOS, TRUE, 1000 * curVol);
}


void setSongSpeed (float f) {
if (f>10) f=10;
if (f<0.1f) f = 0.1f;
curSpeed = f;
if (curHandle==NULL) return;
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_TEMPO, 100 * (curSpeed -1));
}

void setSongPitch (float n) {
curTransposition = minmax(-60.0f, n, 60.0f);
if (curHandle==NULL) return;
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_TEMPO_PITCH, curTransposition);
}

void setSongRate (float f) {
if (curHandle==NULL) return;
if (f<0.1f) f = 0.1f;
else if (f>=10.0f) f=10.0f;
curRate = f;
float freq = 44100;
BASS_ChannelGetAttribute(BASS_FX_TempoGetSource(curHandle), BASS_ATTRIB_FREQ, &freq);
freq *= curRate;
BASS_ChannelSetAttribute(curHandle, BASS_ATTRIB_TEMPO_FREQ, freq);
}

void seekPreviewSong (double delta, BOOL rel) {
if (!curPreviewHandle) { seekSong(delta,rel); return; }
if (rel) delta += BASS_ChannelBytes2Seconds(curPreviewHandle, BASS_ChannelGetPosition(curPreviewHandle, BASS_POS_BYTE));
BASS_ChannelSetPosition(curPreviewHandle, BASS_ChannelSeconds2Bytes(curPreviewHandle, delta), BASS_POS_BYTE);
}

void seekSong (double delta, BOOL rel) {
if (!curHandle) return;
if (curStreamLen<=0) curStreamLen = BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetLength(curHandle, BASS_POS_BYTE));
double dpos = rel? BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetPosition(curHandle, BASS_POS_BYTE)) :0;
dpos += delta / curSpeed;
if (dpos>=curStreamLen) dpos = curStreamLen -0.01;
else if (dpos<0) dpos=0;
BASS_ChannelSetPosition(curHandle, BASS_ChannelSeconds2Bytes(curHandle, dpos), BASS_POS_BYTE);
}

void gotoTimeDialog () {
double dpos = BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetPosition(curHandle, BASS_POS_BYTE));
tstring initialText;
if (dpos<60) tsnprintf(initialText, 19, TEXT("%d.%02d"), (int)dpos, ((int)(dpos*100.0))%100);
else if (dpos<3600) wsnprintf(initialText, 19, TEXT("%d:%02d.%02d"), (int)(dpos/60.0), ((int)dpos)%60, ((int)(dpos*100.0))%100);
else wsnprintf(initialText, 19, TEXT("%d:%02d:%02d.%02d"), (int)(dpos/3600.0), ((int)dpos)%3600, ((int)dpos)%60, ((int)(dpos*100.0))%100);
TCHAR* str = inputDialog(toTString(MSG_GOTOTIME_TITLE).c_str(), toTString(MSG_GOTOTIME_PROMPT).c_str(), initialText.c_str());
if (str) {
TCHAR* c=str;
int mode=0;
if (*c=='+') { mode=1; c++; }
else if (*c=='-') { mode=-1; c++; }
int h=0, m=0, s=0, u=0;
string s0 = toString(c);
if (
(h=m=s=u=0) || sscanf(s0.c_str(), "%u:%u:%u.%u", &h, &m, &s, &u)>=3
|| (h=m=s=u=0) || sscanf(s0.c_str(), "%u:%u.%u", &m, &s, &u)>=2
|| (h=m=s=u=0) || sscanf(s0.c_str(), "%u.%u", &s, &u)>=1
) {
dpos = u/100.0 + s + m*60 + h*3600;
if (mode) dpos *= mode;
seekSong(dpos, mode!=0);
}
free(str);
}}

void openUrlDialog (BOOL append) {
TCHAR * url = inputDialog(toTString(MSG_OPENURL_TITLE).c_str(), toTString(MSG_OPENURL_PROMPT).c_str(), TEXT(""));
if (url) {
if (!append) clearPlaylist();
appendToPlaylist(url);
free(url);
}}

void openDirDialog (BOOL append) {
TCHAR dir[300]={0};
if (BrowseFolders(win, dir, TEXT(MSG_DLGOPENDIR))) {
if (!append) clearPlaylist();
appendDirectoryToPlaylist(dir);
if (playlistdlg) SendMessage(playlistdlg, WM_USER, 15, 0);
}}

LRESULT CALLBACK openDialogHookHook (int code, DWORD wp, int lp) {
if (code<0) return CallNextHookEx(NULL, code, wp, lp);
else if (code==HC_ACTION && lp>=0) {
if (wp==VK_F11 || wp==VK_F12) {
bool sd = GetAsyncKeyState(VK_SHIFT);
shiftDown=sd;
if (shiftDown&&wp==VK_F11) action(IDM_SEEK_LEFT);
else if (shiftDown&&wp==VK_F12) action(IDM_SEEK_RIGHT);
else if (wp==VK_F11) action(IDM_VOLMINUS);
else if (wp==VK_F12) action(IDM_VOLPLUS);
}
}
return CallNextHookEx(NULL, code, wp, lp);
}

INT_PTR CALLBACK openDialogHook (HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
static string curSelection;
static HHOOK hook = NULL;
if (msg==WM_NOTIFY) {
OFNOTIFY* ofno = (OFNOTIFY*)lp;
if (ofno->hdr.code==CDN_SELCHANGE) {
TCHAR fn[512]={0};
DWORD fns = SendMessage(GetParent(hdlg), CDM_GETFILEPATH, 511, fn);
if (fns>0 && ini.get("explorer.openPreview",true)) {
fn[fns]=0;
curSelection = toString(fn);
stopSongPreview();
KillTimer(hdlg, 1063);
SetTimer(hdlg, 1063, 2000, NULL);
}}}
else if (msg==WM_TIMER) {
KillTimer(hdlg,1063);
startSongPreview(curSelection);
}
else if (msg==WM_INITDIALOG) {
hook = SetWindowsHookEx(WH_KEYBOARD, openDialogHookHook, NULL, GetCurrentThreadId());
printf("Hook = %p\r\n", hook);
}
else if (msg==WM_DESTROY) {
if (hook) UnhookWindowsHookEx(hook);
}
return 0;
}

void openDialog (BOOL append) {
OPENFILENAME ofn;
int pathlen = 262144;
TCHAR* path = (TCHAR*)malloc(sizeof(TCHAR) * pathlen);
ZeroMemory(path, pathlen*sizeof(TCHAR));
                ZeroMemory(&ofn, sizeof(OPENFILENAME));

if (filter.size()<=0) {
tstring allexts;
allexts.reserve(1024);
filter.reserve(4096);
BOOL first = TRUE;
for (int i=0; i < formats.size(); i++) {
const string& name = formats[i].first;
const string& exts = formats[i].second;
if (!first) allexts+=TEXT(";");
allexts += toTString(exts);
filter += toTString(name) + TEXT("( ") + toTString(exts) + TEXT(")\1") + toTString(exts) + TEXT("\1");
first = FALSE;
}
filterIndex = formats.size();
filter += toTString(MSG_ALLSUPFILES "\1") + allexts  + toTString("\1" MSG_ALLFILES "\1*.*\1\1\1");
for (int i=0; i<filter.size(); i++) if (filter[i]==1) filter[i]=0;
}

                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = win;
ofn.lpstrFile = path;
ofn.nMaxFile = pathlen;
ofn.lpstrFilter = filter.data();
ofn.nFilterIndex = filterIndex +1;
ofn.lpfnHook = (LPOFNHOOKPROC)openDialogHook;
ofn.Flags =                        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST  | OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_ENABLEHOOK;

BOOL result = GetOpenFileName(&ofn);
stopSongPreview();
if (result) {
if (!append) clearPlaylist();
if (ofn.nFileOffset<=0) appendToPlaylist(ofn.lpstrFile);
else {
ofn.lpstrFile[ofn.nFileOffset -1]=0;
TCHAR* dir = ofn.lpstrFile;
int pos = ofn.nFileOffset, lpos = pos;
TCHAR* fnx = ofn.lpstrFile;
while (true) {
pos++;
if (fnx[pos]==0) {
if (pos-lpos<=0) break;
tstring fn = dir; 
fn += TEXT("\\");
fn += (fnx+lpos);
appendToPlaylist(fn);
lpos = pos+1;
}
} // end while
}
if (playlistdlg) SendMessage(playlistdlg, WM_USER, 15, 0);
}
if (path) free(path);
}

void encodeCopyFile (tstring dstFile) {
if (!CopyFile(toTString(playlist[curSong].file).c_str(), toTString(dstFile).c_str(), false))
MessageBox(win, toTString(MSG_ENCODECOPYORIGFAIL).c_str(), toTString(ERROR).c_str(), MB_OK | MB_ICONSTOP);
}

void startEncode (int type, string file) {
while(ignoreUpdate) Sleep(1);
ignoreUpdate=true;
freeSong(NULL,curHandle);
curDecode = type;
curDecodeFn = file;
playSong(TRUE);
ignoreUpdate=false;
}

void startExport (LPQCPLUGIN p, string inFile, string outFile, DWORD handle) {
BOOL result = p->get(inFile.c_str(), PF_OPEN_WRITE, outFile.c_str(), handle);
printf("RESULT=%d\r\n", result);
}

INT_PTR CALLBACK saveEncodeDialogHook (HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
static OPENFILENAME* ofn = NULL;
if (msg==WM_COMMAND && LOWORD(wp)==4172) {
int n = ofn->nFilterIndex -2;
if (n>=0 && n<encoders.size()) {
LPQCPLUGIN p = encoders[n];
if (p->flags&PF_HAS_OPTIONS_DIALOG) p->query(NULL, PP_ENC_OPTIONS_DIALOG, hdlg, sizeof(HWND));
}}
else if (msg==WM_NOTIFY) {
OFNOTIFY* ofno = (OFNOTIFY*)lp;
if (ofno->hdr.code==CDN_TYPECHANGE) {
ofn = ofno->lpOFN;
int n = ofn->nFilterIndex -2;
EnableWindow(GetDlgItem(hdlg,4172), n>=0&&n<encoders.size()&&encoders[n]->flags&PF_HAS_OPTIONS_DIALOG);
}}
else if (msg==WM_INITDIALOG) {
ofn = (OPENFILENAME*)lp;
int n = ofn->nFilterIndex -2;
EnableWindow(GetDlgItem(hdlg,4172), n>=0&&n<encoders.size()&&encoders[n]->flags&PF_HAS_OPTIONS_DIALOG);
}
return 0;
}

void saveEncodeDialog () {
OPENFILENAME ofn;
vector<LPQCPLUGIN> possibleExporters;
for (auto it: exporters) if (it->get(playlist[curSong].file.c_str(), PF_EXP_CHECK, NULL, BASS_FX_TempoGetSource(curHandle))) possibleExporters.push_back(it);
TCHAR path[512]={0};
tstring flt; flt.clear();
flt += toTString(MSG_ENCODECOPYORIG) + TEXT("\1*.*\1");
for (int i=0; i < encodeFormats.size(); i++) {
const string& name = encodeFormats[i].first;
const string& ext = encodeFormats[i].second;
flt += toTString(name); flt +=  TEXT(" ("); flt +=toTString(ext); flt += TEXT(")\1");
 flt += toTString(ext); flt += TEXT("\1");
}
for (auto it: possibleExporters) {
flt += toTString(it->desc); flt +=  TEXT(" ("); flt +=toTString(it->exts); flt += TEXT(")\1");
 flt += toTString(it->exts); flt += TEXT("\1");
}
flt += TEXT("\1\1\1");
for (int i=0; i<flt.size(); i++) if (flt[i]==1) flt[i]=0;

                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = win;
ofn.lpstrFile = path;
ofn.nMaxFile = 512;
ofn.lpstrFilter = flt.data();
ofn.nFilterIndex = 1;
ofn.hInstance = hinst;
ofn.lpTemplateName = TEXT(IDD_ENCODESAVEDLG);
ofn.lpfnHook = (LPOFNHOOKPROC)saveEncodeDialogHook;
ofn.Flags =                        OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;

if (GetSaveFileName(&ofn)) {
int type = ofn.nFilterIndex;
if (--type==0) encodeCopyFile(ofn.lpstrFile);
else if (--type<encoders.size()) startEncode(type, toString(ofn.lpstrFile));
else if ((type-=encoders.size()) <possibleExporters.size()) startExport(possibleExporters[type], playlist[curSong].file, toString(ofn.lpstrFile), BASS_FX_TempoGetSource(curHandle));
}}

void savePlaylistDialog () {
if (playlistWriteFormats.size()<=0) {
MessageBox(win, toTString(MSG_NOWRITEPLAYLIST).c_str(), TEXT(INFO), MB_OK|MB_ICONINFORMATION);
return;
}

OPENFILENAME ofn;
TCHAR path[512]={0};

tstring flt; flt.clear();
for (int i=0; i < playlistWriteFormats.size(); i++) {
const string& name = playlistWriteFormats[i].first;
const string& ext = playlistWriteFormats[i].second;
flt += toTString(name) + TEXT(" (") + toTString(ext) + TEXT(")\1") + toTString(ext) + TEXT("\1");
}
flt += TEXT("\1\1\1");
for (int i=0; i<flt.size(); i++) if (flt[i]==1) flt[i]=0;

                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = win;
ofn.lpstrFile = path;
ofn.nMaxFile = 512;
ofn.lpstrFilter = flt.data();
ofn.nFilterIndex = 1;
ofn.Flags =                        OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_EXPLORER;

if (GetSaveFileName(&ofn)==TRUE) {
tstring fn = ofn.lpstrFile;
int type = ofn.nFilterIndex -1;
savePlaylist(playlistWriters[type], fn);
}}

bool savePlaylist (LPQCPLUGIN p, const tstring& fn) {
void* fp = 0;
if (USE_UNICODE&&p->flags&PF_UNICODE) fp = p->open(p, toTString(fn).c_str(), PF_OPEN_WRITE | PF_UNICODE, playlist.size());
else fp = p->open(p, toString(fn).c_str(), PF_OPEN_WRITE, playlist.size());
if (!fp) return false;
for(playlistitem& it: playlist) {
int re = p->write(fp, &it, 0);
}
p->close(fp);
return true;
}

int loadPlaylist (LPQCPLUGIN p, const string& fn) {
void* fp = 0;
if (USE_UNICODE&&(p->flags&PF_UNICODE)) fp = (DWORD)(p->open(p, toTString(fn).c_str(), PF_OPEN_READ | PF_UNICODE, 0));
else fp = (DWORD)(p->open(p, toString(fn).c_str(), PF_OPEN_READ, 0));
if (!fp) return false;
bool ok=false;
while(true){
playlistitem it;
if (!p->read(fp, &it, 0)) break;
playlist.push_back(it);
ok=true;
}
p->close(fp);
if (initialized && curSong== -1) { if (curReverse) prevSong(); else nextSong(); }
return ok;
}

void sortPlaylist (int mode) {
string file = curSong>=0&&curSong<playlist.size()? playlist[curSong].file : ("");
sort(playlist.begin(), playlist.end(), [=](const playlistitem& a, const playlistitem& b){
switch(mode){
case 0: return alphanum_comp(a.file, b.file)<0;
case 1: return alphanum_comp(a.title, b.title)<0;
case 2: return alphanum_comp(a.artist, b.artist)<0;
case 3: return alphanum_comp(a.album, b.album)<0;
default: return alphanum_comp(a.file, b.file)<0;
}});
for (int i=0; i<playlist.size(); i++) if (playlist[i].file==file) { curSong=i; break; }
updateWindowTitle();
}

void clearPlaylist (int mode, string filter) {
string file = curSong>=0&&curSong<playlist.size()? playlist[curSong].file : ("");
int newcur = -1;
if (mode==0) playlist.clear();
else if (mode==1) {
for (int i=0, n=playlist.size(); i<n; i++) {
if (acceptPlaylistItem(playlist[i], filter)) {
playlist.erase(playlist.begin() +(i--));
n--;
}}
for (int i=0; i<playlist.size(); i++) if (playlist[i].file==file) { newcur=i; break; }
}
else if (mode==2) {
for (int i=0, n=playlist.size(); i<n; i++) {
if (!acceptPlaylistItem(playlist[i], filter)) {
playlist.erase(playlist.begin() +(i--));
n--;
}}
for (int i=0; i<playlist.size(); i++) if (playlist[i].file==file) { newcur=i; break; }
}
if (newcur<0 || newcur>=playlist.size()) {
if (curHandle!=NULL) stopSong();
curSong = -1;
}
else curSong = newcur;
}

BOOL isdir (const tstring& fn) {
DWORD n = GetFileAttributes(fn.c_str());
return n!=INVALID_FILE_ATTRIBUTES && 0!=(n&FILE_ATTRIBUTE_DIRECTORY);
}

int appendToPlaylist (const TCHAR* fn) { appendToPlaylist(tstring(fn)); }
template<class T> int appendToPlaylist (const basic_string<T>& fn) {
tstring fn2 = toTString(fn);
if (isdir(fn2)) appendDirectoryToPlaylist(fn2);
else appendFileToPlaylist(toString(fn));
}

int appendFileToPlaylist (const string& fn) {
playlistitem o;
o.file = toString(fn);
playlist.push_back(o);
if (initialized && curSong== -1) { if (curReverse) prevSong(); else nextSong(); }
return playlist.size() -1;
}

void appendDirectoryToPlaylist (const tstring& dir1) {
tstring str, dir = dir1;
WIN32_FIND_DATA find;
if (dir[dir.size() -1]!='\\') dir += TEXT("\\");
str = dir + TEXT("*.*");
HANDLE handle = FindFirstFile(str.c_str(), &find);
if (handle!=INVALID_HANDLE_VALUE && handle!=NULL) {
do {
if (find.cFileName[0]!='.') {
str = dir + find.cFileName;
appendToPlaylist(str);
}} while (FindNextFile(handle, &find));
FindClose(handle);
}}

inline void setEqualizerValueDelta (DWORD handle, DWORD idx, float delta) {
setEqualizerValue(handle, idx, eqGains[idx] +delta);
}

void setEqualizerValue (DWORD handle, DWORD idx, float gain) {
if (handle==NULL) return;
if (gain < -15) gain = -15;
if (gain > 15) gain = 15;
eqGains[idx] = gain;
if (gain==0) {
if (eqHfx[idx]!=NULL) {
BASS_ChannelRemoveFX(handle, eqHfx[idx]);
}
eqHfx[idx]=NULL;
}
else {
if (eqHfx[idx]==NULL) eqHfx[idx] = BASS_ChannelSetFX(handle, BASS_FX_DX8_PARAMEQ, 3);
if (eqHfx[idx]!=NULL) {
BASS_DX8_PARAMEQ d;
d.fBandwidth = eqBandwidths[idx];
d.fCenter = eqFreqs[idx];
d.fGain = gain;
BASS_FXSetParameters(eqHfx[idx], &d);
}}}

void updateInfo () {
if (ignoreUpdate) return;
ignoreUpdate=true;
if (curHandle==NULL) {
SetWindowText(lbl, TEXT(MSG_NOPLAYING));
ignoreUpdate=false;
return;
}

int state = BASS_ChannelIsActive(curHandle);
if (state== BASS_ACTIVE_STOPPED) {
if (BASS_ErrorGetCode()==BASS_ERROR_DEVICE) SendMessage(win, WM_USER+2000, 0, 0);
else if (curLoop || crossFade<=0 || curStreamLen<30) { if (curReverse) prevSong(); else nextSong(); }
ignoreUpdate=false;
return;
}
if (curStreamLen<=0) curStreamLen = BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetLength(curHandle, BASS_POS_BYTE));
double dpos = BASS_ChannelBytes2Seconds(curHandle, BASS_ChannelGetPosition(curHandle, BASS_POS_BYTE));

int errorcode = BASS_ErrorGetCode();
if (errorcode) printf("Error=%d\r\n",errorcode);
int pos = (int)(dpos / curSpeed);
int len = (int)(curStreamLen / curSpeed);
int spdprc = (int)round(100 * curSpeed);
int vlprc = (int)round(100 * curVol);
if (!curLoop && ( (crossFade>0 && curStreamLen>=30 && curStreamLen-dpos<=crossFade/1000.0)  ) ) { if (curReverse) prevSong(); else nextSong(); }

tstring str;
wsnprintf(str, 1024, TEXT("%s / %s, spd %d%%, pitch %+.3g, vol %d%%"), formatTime(pos).c_str(), formatTime(len).c_str(), spdprc, curTransposition, vlprc);
SetWindowText(lbl, str.c_str());
ignoreUpdate=false;
}

DWORD WINAPI encodeautoproc (LPVOID lpv) {
DWORD handle = (DWORD)lpv;
printf("encoded handle = %p\r\n", handle);
Beep(1600,300);
DWORD dummylen = 65536;
char dummy[dummylen];
while(BASS_ChannelIsActive(handle) && BASS_Encode_IsActive(curEncode)) if (BASS_ChannelGetData(handle, dummy, dummylen)<dummylen) Sleep(1);
printf("Error=%d\r\n", BASS_ErrorGetCode());
printf("Activity: %d, %d\r\n", BASS_Encode_IsActive(curEncode), BASS_ChannelIsActive(handle));
Beep(3200,300);
BASS_Encode_StopEx(handle,TRUE);
if (curMixHandle) stopMix();
curEncode=0;
curCasting=-1;
return 0;
}

playlistitem& fillMetadata (playlistitem& it, DWORD handle) {
if (it.metadataSet) return it;
it.length = BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetLength(handle, BASS_POS_BYTE));
it.metadataSet = true;

const char* tags = TAGS_Read(handle, "%ICAP(%ITRM(%TITL))\1%ICAP(%ITRM(%ARTI))\2%ICAP(%ITRM(%ALBM))\3%ICAP(%ITRM(%GNRE))\4%ICAP(%ITRM(%SUBT))\5%ICAP(%ITRM(%COMP))\6%ICAP(%ITRM(%COPY))\7%ICAP(%ITRM(%YEAR))\x0E%ICAP(%ITRM(%DISC))\x0F%ICAP(%ITRM(%TRCK))\x10%ICAP(%ITRM(%CMNT))\x11");
if (tags && tags[0]>31) {
const char
*title = strtok2(const_cast<char*>(tags), 1), 
*artist = strtok2(NULL, 2), 
*album = strtok2(NULL, 3), 
*genre  = strtok2(NULL, 4), 
*subtitle = strtok2(NULL, 5),
*composer  = strtok2(NULL, 6), 
*copyright  = strtok2(NULL, 7), 
*year  = strtok2(NULL, 14), 
*disc  = strtok2(NULL, 15), 
*tracknum= strtok2(NULL, 16),
*comment = strtok2(NULL, 17);
#define F(x) if (x && strlen(x)>0) it.x=toUTF8String(x); else it.x.clear();
F(title) F(artist) F(album) F(genre) F(subtitle)
F(year) F(composer) F(disc) F(tracknum) F(copyright) F(comment)
#undef F
return it;
}

tags = BASS_ChannelGetTags(handle, BASS_TAG_META);
if (tags && strlen(tags)>0) {
it.title.clear(); it.album.clear(); it.artist.clear(); it.genre.clear(); it.subtitle.clear();
char* beg = strstr(tags, "StreamTitle='"), *end = strstr(tags, "';");
if (beg&&end) it.title = string(beg+13, end-beg-13);
else if (beg) it.title = toString((beg+13));
return it;
}

tags = BASS_ChannelGetTags(handle, BASS_TAG_ICY);
if (!tags) tags = BASS_ChannelGetTags(handle, BASS_TAG_HTTP);
if (tags && strlen(tags)>0) {
for (; *tags; tags+=strlen(tags)+1) {
if (!strnicmp(tags, "ici-name:", 9)) {
it.title.clear(); it.album.clear(); it.artist.clear(); it.genre.clear(); it.subtitle.clear();
it.title = toUTF8String(tags+9);
return it;
}}}

tags = BASS_ChannelGetTags(handle, BASS_TAG_MIDI_TRACK);
if (tags && strlen(tags)>0) {
it.album.clear(); it.artist.clear(); it.genre.clear(); it.subtitle.clear();
it.title = toUTF8String(tags);
return it;
}

it.title.clear(); it.album.clear(); it.artist.clear(); it.genre.clear(); it.subtitle.clear();
const string& fn = it.file;
int n1 = fn.rfind('/'), n2 =  fn.rfind('\\');
if (n1>=fn.size()&&n2>=fn.size()) it.title = fn;
else if (n1>=fn.size()) it.title = fn.substr(n2+1);
else it.title = fn.substr(n1+1);
return it;
}

tstring getPlaylistItemText (int i) {
string cbuf;
const playlistitem& it = playlist[i];
if (it.title.size()>0 || it.length>0) {
if (it.artist.size()>0) snprintf(cbuf, 511, ("%d.\t%s - %s\t%ls"), i+1, it.artist.c_str(), it.title.c_str(), formatTime(it.length).c_str());
else snprintf(cbuf, 511, ("%d.\t%s\t%ls"), i+1, it.title.c_str(), formatTime(it.length).c_str());
}
else snprintf(cbuf, 511, ("%d.\t%s"), i+1, it.file.c_str());
return toTString(cbuf);
}

bool acceptRadio (const radio& r, const string& flt) {
return contains(r.name, flt) || contains(r.lang, flt) || contains(r.genre, flt) || contains(r.urls[0], flt) || contains(r.description, flt);
}

bool acceptPlaylistItem (const playlistitem& it, const string& flt)  {
return contains(it.file, flt) || contains(it.title, flt) || contains(it.artist, flt) || contains(it.album, flt) || contains(it.genre, flt) || contains(it.subtitle, flt);
}

void doIndexAllPlaylist () {
progressinfo p;
p.hdlg=NULL;
p.cancelled = false;
p.title = TEXT(MSG_INDEXDLG);
bool active = curHandle && BASS_ACTIVE_PLAYING==BASS_ChannelIsActive(curHandle);
if (active)  BASS_ChannelPause(curHandle);
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)indexAllPlaylist, (LPVOID)(&p), 0, NULL);
DialogBoxParam(hinst, TEXT(IDD_PROGRESSDLG), win, (DLGPROC)progressDlgProc, (LPARAM)(&p));
if (active) BASS_ChannelPlay(curHandle, FALSE);
}

DWORD indexAllPlaylist (LPVOID lp) {
int sz = playlist.size();
progressinfo* p = (progressinfo*)lp;
while (!p->hdlg) Sleep(10);
if (p->hdlg) SendMessage(p->hdlg, WM_USER+13, 0, sz);
DWORD last = GetTickCount(), time=0;
for (int i=0; !p->cancelled&&i<playlist.size(); i++) {
playlistitem& it = playlist[i];
if (!it.metadataSet) {
DWORD stream = getStream(it.file, BASS_MUSIC_NOSAMPLE);
if (!stream) {
playlist.erase(playlist.begin()  +i);
if (curSong>=i) curSong--;
i--;
continue;
}
fillMetadata(it, stream);
BASS_StreamFree(stream);
BASS_MusicFree(stream);
}
if (p->hdlg) { // && (last=GetTickCount()) -time >= 50) {
time=last;
tstring ch;
wsnprintf(ch, 300, toTString(MSG_INDEXPROGRESS).c_str(), i, playlist.size());
if (i>=sz) SendMessage(p->hdlg, WM_USER+13, 0, sz=playlist.size());
SendMessage(p->hdlg, WM_USER+12, i, 0);
SendMessage(p->hdlg, WM_USER+14, 0, (LPARAM)(ch.c_str()));
}}
if (playlistdlg) SendMessage(playlistdlg, WM_USER, 15, 0);
if (p->hdlg) SendMessage(p->hdlg, WM_COMMAND, IDCANCEL, 0);
return 0;
}

void CALLBACK UserStreamFree (HSYNC sync, DWORD stream, DWORD data, void* udata) {
if (udata) free(udata);
}

bool setOutDevice (int index) {
return BASS_SetDevice(index) || (BASS_Init( index, 44100, BASS_INITFLAGS, win, NULL) && BASS_SetDevice(index));
}

void freeOutDevice (int index) {
if (BASS_SetDevice(index)) BASS_Free();
}

void fillOutDevices (unordered_map<int,string>* m1, unordered_map<string,int>*m2) {
if (m1) m1->clear();
if (m2) m2->clear();
BASS_DEVICEINFO dev;
for (int i=2; BASS_GetDeviceInfo(i,&dev); i++) {
if (!(dev.flags&BASS_DEVICE_ENABLED)) continue;
string name = dev.name;
if (m2) (*m2)[name] = i;
if (m1) (*m1)[i] = name;
}}

void fillInDevices (unordered_map<int,string>* m1, unordered_map<string,int>*m2) {
if (m1) m1->clear();
if (m2) m2->clear();
BASS_DEVICEINFO dev;
for (int i=0; BASS_RecordGetDeviceInfo(i,&dev); i++) {
if (!(dev.flags&BASS_DEVICE_ENABLED)) continue;
string name = dev.name;
if (m2) (*m2)[name] = i;
if (m1) (*m1)[i] = name;
}}

int findDevice (unordered_map<string,int>& m, string name, int def)  {
auto it = m.find(name);
if (it==m.end()) return def;
return it->second;
}

string findDevice (unordered_map<int,string>& m, int index, string def)  {
auto it = m.find(index);
if (it==m.end()) return def;
return it->second;
}

int SockRecvLine (Socket& sock, string& str) {
char c;
int val = -100;
str.clear();
str.reserve(1024);
while ((val=sock.recv(&c,1))==1) {
if (c=='\n') return 0;
if (c!='\r') str+=c;
}
return val;
}

DWORD HTTPResponseAuto (Socket* sock, const string& statusLine, const string& explanations) {
string status = "HTTP/1.0 " + statusLine + "\r\n\r\n<html><head><title>" + statusLine + "</title></head><body><h1>" + statusLine + "</h1><p>" + explanations + "</p></body></html>";
sock->send(status.c_str(), status.size());
sock->close();
delete sock;
return 0;
}

DWORD HTTPResponseFile (Socket* sock, ifstream& in) {
string status = "HTTP/1.0 200 Ok\r\nCache-Control: no-cache, no-store, must-revalidate\r\n\r\n";
sock->send(status.c_str(), status.size());
char buf[4096];
int n=0;
while(in){
in.read(buf,4096);
sock->send(buf, in.gcount());
n+=in.gcount();
}
sock->close();
delete sock;
printf("Sent %d bytes\r\n", n);
return 0;
}

void luaRunWebScript (Socket*, string, string, string, string, unordered_map<string,string>&, string);
DWORD HTTPResponseScript (Socket* sock, string filename, string method, string uri, string query, unordered_map<string,string>& headers, string data) {
luaRunWebScript (sock, filename, method, uri, query, headers, data);
sock->close();
delete sock;
return 0;
}

DWORD HTTPServerProc (Socket* sock) {
string str="", top="", method="GET", uri="", query="", data="", dummy;
unordered_map<string,string> headers;
int pos=0;
int re = SockRecvLine(*sock, top);
while (!SockRecvLine(*sock, str)) {
if (str.length()==0) break;
string name, value;
if (!split(str, ':', name, value)) continue;
trim(name); trim(value);
toLowerCase(name);
headers[name] = value;
}
str.clear();
if (!split(top, ' ', method, uri, dummy)) return HTTPResponseAuto(sock, "400 Bad request", "The server didn't understand your request. Please conform to HTTP 1.0.");
pos = uri.find("?");
if (pos>0) {
query = uri.substr(pos+1);
uri = uri.substr(0,pos);
}
if (uri.find("/action/")==0) {
query = uri.substr(8);
action(toInt(query));
return HTTPResponseAuto(sock, "206 No contents", "");
} 
string filename = "./httpdocs" +uri;
ifstream in(filename);
if (!in) return HTTPResponseAuto(sock, "404 Not found", "The requested URI <strong>" + uri + "</strong> was not found on this server.");
if (endsWith(filename, ".lua") || endsWith(filename, ".lc")) 
return HTTPResponseAuto(sock, "501 Not implemented", "Lua scripts is not yet implemented in this version of 6player.");
//return HTTPResponseScript(sock, filename, method, uri, query, headers, data);
else if (in) return HTTPResponseFile(sock, in);
return HTTPResponseAuto(sock, "500 Internal server error", "The server encountered an error and is unable to fullfill your request.");
return 0;
}

DWORD HTTPServerTProc (LPVOID lp) {
Socket::initialize();
Socket server;
if (server.bind(NULL, ini.get("webserver.port",88))) return 1;
while(true) {
Socket* client = new Socket();
if (server.accept(*client)) break;
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HTTPServerProc, (LPVOID)(client), 0, NULL);
}
return 0;
}

void MIDIFillInstrumentsList (map<DWORD,string>& map) {
DWORD handle = 0;
DWORD nFonts = BASS_MIDI_StreamGetFonts(handle, (void*)NULL, 0);
BASS_MIDI_FONTEX fonts[nFonts];
BASS_MIDI_StreamGetFonts(handle, fonts, nFonts | BASS_MIDI_FONT_EX);
for (int i=nFonts; i>=0; i--) {
BASS_MIDI_FONTEX& font = fonts[i];
BASS_MIDI_FONTINFO info;
BASS_MIDI_FontGetInfo(font.font,&info);
DWORD* presets = new DWORD[info.presets];
BASS_MIDI_FontGetPresets(font.font, presets);
for (int j=0; j<info.presets; j++) {
DWORD val, x=presets[j], preset =  LOWORD(x), bank = HIWORD(x);
if (font.spreset>=0 && preset!=font.spreset && info.presets>1) continue; 
if (font.sbank>=0 && bank!=font.sbank && info.presets>1) continue;
if (font.spreset==-1 && bank<128) bank+=font.dbank;
else if (font.spreset>=0) bank = font.dbank;
val = (preset&0x7F) | ((font.dbanklsb&0x7F)<<7) | ((bank&0xFF)<<14);
if (map.find(val)!=map.end()) continue;
const char* name = BASS_MIDI_FontGetPreset(fonts[i].font, LOWORD(x), HIWORD(x));
if (name) map[val] = name;
}
delete[] presets;
}
}

DWORD MIDIGetCurrentInstrument (DWORD stream, int channel) {
if (midiInstruments.size()<=0) MIDIFillInstrumentsList(midiInstruments);
DWORD 
bank = BASS_MIDI_StreamGetEvent(stream, channel, MIDI_EVENT_BANK)&0x7F,
banklsb = BASS_MIDI_StreamGetEvent(stream, channel, MIDI_EVENT_BANK_LSB)&0x7F,
preset = BASS_MIDI_StreamGetEvent(stream, channel, MIDI_EVENT_PROGRAM)&0x7F,
drum = BASS_MIDI_StreamGetEvent(stream, channel, MIDI_EVENT_DRUMS);
DWORD val = preset | (banklsb<<7) | (bank<<14) | (drum? 1<<21 : 0);
auto it = midiInstruments.find(val);
if (it!=midiInstruments.end()) return val;
val = preset |  (bank<<14) | (drum? 1<<21 : 0);
it = midiInstruments.find(val);
if (it!=midiInstruments.end()) return val;
val = preset | (banklsb<<7) | (drum? 1<<21 : 0);
it = midiInstruments.find(val);
if (it!=midiInstruments.end()) return val;
val = preset | (drum? 1<<21 : 0);
it = midiInstruments.find(val);
if (it!=midiInstruments.end()) return val;
val = (preset&0x78) | (drum? 1<<21 : 0);
it = midiInstruments.find(val);
if (it!=midiInstruments.end()) return val;
return 0;
}

const char* MIDIGetCurrentInstrumentName (DWORD stream, int channel) {
if (midiInstruments.size()<=0) MIDIFillInstrumentsList(midiInstruments);
DWORD val = MIDIGetCurrentInstrument(stream, channel);
auto it = midiInstruments.find(val);
if (it!=midiInstruments.end()) return it->second.c_str();
else return NULL;
}

void MIDILoadSubsystem (void) {
static bool midiInitialized = false;
if (midiInitialized) return;
ifstream sbf("midi.conf");
if (sbf) {
vector<BASS_MIDI_FONTEX> banks;
unordered_map<string,HSOUNDFONT> fonts;
string line, file, name, value;
while (getline(sbf,line)) {
if (!split(line, '=', name, value)) continue;
trim(name); trim(value);
if (name=="font") {
istringstream in(value);
int sPreset= -1, sBank = -1, dPreset = -1, dBank = 0, dBankLSB = 0;
HSOUNDFONT font = 0;
if (!(in>>file)) continue;
in >> sBank >> sPreset >> dBank >> dPreset >> dBankLSB;
replace(file, "\xA0", " ");
auto it = fonts.find(file);
if (it!=fonts.end()) font = it->second;
if (!font) font = BASS_MIDI_FontInit(toTString(file).c_str(), USE_UNICODE);
if (!font) {
printf("Error loading font %s: %d\r\n", file.c_str(), BASS_ErrorGetCode());
continue;
}
BASS_MIDI_FONTEX sf;
sf.font = font;
sf.spreset = sPreset;
sf.sbank = sBank;
sf.dpreset = dPreset;
sf.dbank = dBank;
sf.dbanklsb = dBankLSB;
banks.push_back(sf);
}
else if (name=="autofont") BASS_SetConfig(BASS_CONFIG_MIDI_AUTOFONT, toInt(value) || toBool(value));
else if (name=="compact") BASS_SetConfig(BASS_CONFIG_MIDI_COMPACT, toBool(value));
else if (name=="maxvoices") BASS_SetConfig(BASS_CONFIG_MIDI_VOICES, toInt(value));
}
if (banks.size()>0) BASS_MIDI_StreamSetFonts(0, &(banks[0]), banks.size() | BASS_MIDI_FONT_EX);
}//end soundbank configuration file parsing
midiInitialized = true;
}

void CALLBACK MIDISyncProgram (HSYNC sync, DWORD stream, DWORD index, void* unused) {
if (!mididlg) return;
DWORD chan = HIWORD(index);
DWORD patch = MIDIGetCurrentInstrument(stream, chan);
SendMessage(mididlg, WM_USER+105, chan, patch);
}

void CALLBACK MIDISyncLyric (HSYNC sync, DWORD stream, DWORD index, void* unused) {
if (!textdlg) return;
BASS_MIDI_MARK mark;
if (BASS_MIDI_StreamGetMark(stream, BASS_MIDI_MARK_LYRIC, index, &mark)) {
BOOL clear = !!(mark.text[0]=='/' || mark.text[0]=='\\');
if (clear) SendMessage(textdlg, WM_USER+105, mark.text[0]=='/'?101:102, 0);
SendMessage(textdlg, WM_USER+105, 100, mark.text+clear);
}}

void CALLBACK MIDILoopStart (HSYNC sync, DWORD ch, DWORD data, DWORD pos) {
BASS_ChannelSetPosition(ch, pos, BASS_POS_BYTE);
}

void MIDILoadSpecific (DWORD stream) {
MIDILoadSubsystem();
int midiMarks = BASS_MIDI_StreamGetMarks(stream, -1, BASS_MIDI_MARK_MARKER, NULL);
if (midiMarks>0) {
BASS_MIDI_MARK marks[midiMarks];
BASS_MIDI_StreamGetMarks(stream, -1, BASS_MIDI_MARK_MARKER, marks);
for (int i=0; i<midiMarks; i++) {
if (marks[i].text && !stricmp(marks[i].text, "loopstart")) {
BASS_ChannelSetSync(stream, BASS_SYNC_MIXTIME | BASS_SYNC_POS, BASS_ChannelGetLength(stream, BASS_POS_BYTE) -0, MIDILoopStart, marks[i].pos);
}}}
BASS_ChannelSetSync(stream, BASS_SYNC_MIDI_MARK, BASS_MIDI_MARK_TEXT, MIDISyncLyric, NULL);
BASS_ChannelSetSync(stream, BASS_SYNC_MIDI_MARK, BASS_MIDI_MARK_LYRIC, MIDISyncLyric, NULL);
BASS_ChannelSetSync(stream, BASS_SYNC_MIDI_EVENT, MIDI_EVENT_PROGRAM, MIDISyncProgram, NULL);
}

DWORD getFirstStreamFromPlaylist (LPQCPLUGIN p, const string& fn, DWORD flags) {
void* fp = 0;
if (USE_UNICODE&&(p->flags&PF_UNICODE)) fp = (DWORD)(p->open(p, toTString(fn).c_str(), PF_OPEN_READ | PF_UNICODE, 0));
else fp = (DWORD)(p->open(p, toString(fn).c_str(), PF_OPEN_READ, 0));
if (!fp) return 0;
DWORD stream=0;
BOOL result = true;
while (result&&!stream) {
playlistitem it;
result = p->read(fp, &it, 0);
if (result) stream = getStream(it.file, flags);
}
p->close(fp);
return stream;
}

DWORD getStream (string fn, DWORD mode) {
printf("Load stream: %s\r\n", fn.c_str());
DWORD stream = NULL;
DWORD streamFlags = BASS_STREAMFLAGS;
if (mode&BASS_SAMPLE_FX) streamFlags&=~BASS_STREAM_DECODE;
if (mode&BASS_STREAM_AUTOFREE) streamFlags|=BASS_STREAM_AUTOFREE;
if (fn.size()>=7 && startsWith(fn, "file://")) fn = (fn.substr(startsWith(fn, "file:///")? 7:6));
if (contains(fn, "://")) {
if (!curDecode) streamFlags |= BASS_STREAM_RESTRATE;
stream = BASS_StreamCreateURL((toString(fn).c_str()), 0, (streamFlags)&~BASS_UNICODE, NULL, NULL);
BASS_ChannelSetSync(stream, BASS_SYNC_MIXTIME | BASS_SYNC_META, 0, resetWindowTitle, NULL);
BASS_ChannelSetSync(stream, BASS_SYNC_MIXTIME | BASS_SYNC_OGG_CHANGE, 0, resetWindowTitle, NULL);
}
else if (fn=="-" || fn=="?stdin") stream = BASS_StreamCreateStdin(streamFlags);
else if (fn.size()>6 && startsWith(fn, "?tcp?")) stream = BASS_StreamCreateRawSocket(fn.c_str() +5, streamFlags&~BASS_UNICODE);
else stream = (DWORD)BASS_StreamCreateFile(false, toTString(fn).c_str(), 0, 0, (streamFlags));
if (!stream) for (int i=0; !stream&&i < plugins.size(); i++) {
LPQCPLUGIN p = plugins[i];
switch(p->type) {
case PT_INPUT : 
if (USE_UNICODE&&(p->flags&PF_UNICODE)) stream = (DWORD)(p->open(p, toTString(fn).c_str(), PF_OPEN_READ | PF_UNICODE, streamFlags));
else stream = (DWORD)(p->open(p, toString(fn).c_str(), PF_OPEN_READ, streamFlags));
break;
case PT_PLAYLIST : {
if (mode&BASS_SAMPLE_FX) return getFirstStreamFromPlaylist(p,fn, mode);
else if (loadPlaylist(p, fn)) return NULL;
}break;
// other types of plugins
default: break; 
}}
// other types of loads ?
if ((mode&BASS_MUSIC_NOSAMPLE) || (mode&BASS_SAMPLE_FX) || !stream) return stream;

BASS_CHANNELINFO info;
BASS_ChannelGetInfo(stream,&info);
if (info.ctype == BASS_CTYPE_STREAM_MIDI) MIDILoadSpecific(stream);

DWORD fxflags = BASS_FXFLAGS;
if (mode&BASS_STREAM_DECODE) fxflags |= BASS_STREAM_DECODE;
if (mode&BASS_SAMPLE_LOOP) fxflags |= BASS_SAMPLE_LOOP;
//if (decode) fxflags &= (~BASS_STREAM_AUTOFREE);

HSTREAM fx = BASS_FX_TempoCreate(stream, fxflags);
BASS_ChannelSetAttribute(fx, BASS_ATTRIB_TEMPO_PITCH, curTransposition);
BASS_ChannelSetAttribute(fx, BASS_ATTRIB_TEMPO, 100 * (curSpeed -1));
BASS_ChannelSetAttribute(fx, BASS_ATTRIB_VOL, curVol);
if (curMixHandle) {
curCopyHandle = BASS_StreamCreateCopy(fx, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE, 0);
BASS_Mixer_StreamAddChannel(curMixHandle, curCopyHandle, BASS_STREAM_AUTOFREE);
}
for (int i=0; i < 5; i++) {
eqHfx[i]=0;
setEqualizerValue(fx, i, eqGains[i]);
}
for (int i=0, n=effects.size(); i<n; i++) {
effectinfo& e = effects[i];
if (e.hfx&&e.curTarget==1) {
e.hfx = BASS_ChannelSetFX(fx, e.type, 1);
if (e.hfx) BASS_FXSetParameters(e.hfx, e.data);
}}
return fx;
}
