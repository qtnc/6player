#undef UNICODE
#include<stdio.h>
#include<windows.h>
#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

#define EXPORT __declspec(dllexport)

#define FORMAT_FLAGS BASS_ENCODE_AUTOFREE | 4;
#define FORMAT_NAME "MPEG1 Layer 3 (MP3)"
#define FORMAT_EXT "*.mp3"
#define FORMAT_MIME BASS_ENCODE_TYPE_MP3

static int getCommandLine (char*, int) ;
static int getCommandLine2 (char*, int) ;
static void openOptionsDialog (HWND) ;

static HINSTANCE hdll = NULL;
static const int bitrates[] = { 24, 32, 48, 64, 80, 96, 112, 128, 144, 160, 192, 224, 240, 256, 320 };
static const int nBitrates = 15;
static int quality = 7;

BOOL EXPORT WINAPI DllMain (HINSTANCE h, DWORD r1, LPVOID r2) {
hdll = h;
return 1;
}

static int getCommandLine (char* buf, int bufmax) {
return snprintf(buf, bufmax, "lame.exe -r -h -b %d --quiet --tt \"%%t\" --ta \"%%a\" --tl \"%%l\" - \"%%f\"", bitrates[quality]);
}

static int getCommandLine2 (char* buf, int bufmax) {
return snprintf(buf, bufmax, "lame.exe -r -s 44100 -b %d --quiet -", bitrates[quality]);
}

BOOL CALLBACK dlgproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
if (msg==WM_INITDIALOG) {
HWND hcb = GetDlgItem(hwnd,1001);
for (int i=0; i<nBitrates; i++) {
char ch[20]={0};
snprintf(ch, 19, "%d kbps", bitrates[i]);
SendMessage(hcb, CB_ADDSTRING, 0, ch);
}
SendMessage(hcb, CB_SETCURSEL, quality, 0);
SetFocus(hcb);
}
else if (msg==WM_COMMAND) {
switch(LOWORD(wp)) {
case IDOK : 
quality = SendDlgItemMessage(hwnd,1001, CB_GETCURSEL, 0, 0);
case IDCANCEL: EndDialog(hwnd,1);
}}
return FALSE;
}

void openOptionsDialog (HWND hwnd) {
DialogBoxParam(hdll, "dlg1", hwnd, dlgproc, NULL);
}

static int encfunc (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX: returnstatic(FORMAT_EXT+2);
case PP_ENC_QUALITY : return bitrates[quality];
case PP_MIMETYPE : returnstatic(FORMAT_MIME);
case PP_ENC_FLAGS : return *(DWORD*)ptr |= FORMAT_FLAGS; 
case PP_ENC_COMMANDLINE: return getCommandLine(ptr, size);
case PP_ENC_CAST_COMMANDLINE : return getCommandLine2(ptr, size);
case PP_ENC_OPTIONS_DIALOG : openOptionsDialog(ptr); return 1;
default: return 0;
}}

static const int nMax = 1;
static const QCPLUGIN data[] = {
{ PT_ENCODER, PF_CAN_CAST | PF_HAS_OPTIONS_DIALOG, FORMAT_NAME, FORMAT_EXT, NULL, NULL, NULL, NULL, encfunc, NULL }
};

LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}


