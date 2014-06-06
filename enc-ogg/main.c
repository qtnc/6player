// NENC OGG
#undef UNICODE
#include<stdio.h>
#include<windows.h>
#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define EXPORT __declspec(dllexport)

#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

static int getCommandLine (char*, int) ;
static int getCommandLine2 (char*, int) ;
static void openOptionsDialog (HWND) ;

static HINSTANCE hdll = NULL;
static int quality = 4;

#define FORMAT_FLAGS BASS_ENCODE_AUTOFREE;
#define FORMAT_NAME "OGG Vorbis"
#define FORMAT_EXT "*.ogg"
#define FORMAT_MIME BASS_ENCODE_TYPE_OGG

BOOL EXPORT WINAPI DllMain (HINSTANCE h, DWORD r1, LPVOID r2) {
hdll = h;
return 1;
}

static int getCommandLine (char* buf, int bufmax) {
return snprintf(buf, bufmax, "oggenc.exe -Q -q %d -o \"%%f\" -t \"%%t\" -a \"%%a\" -l \"%%l\" --ignorelength -", quality);
}

static int getCommandLine2 (char* buf, int bufmax) {
return snprintf(buf, bufmax, "oggenc.exe -r -Q -q %d -", quality);
}

static BOOL CALLBACK dlgproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
if (msg==WM_INITDIALOG) {
HWND hcb = GetDlgItem(hwnd,1001);
for (int i=0; i<=10; i++) {
char ch[10]={0};
snprintf(ch, 9, "%d", i);
SendMessage(hcb, CB_ADDSTRING, 0, ch);
}
SendMessage(hcb, CB_SETCURSEL, quality, 0);
SetFocus(hcb);
}
else if (msg==WM_COMMAND) {
switch(LOWORD(wp)) {
case IDOK : quality = SendDlgItemMessage(hwnd,1001, CB_GETCURSEL, 0, 0);
case IDCANCEL: EndDialog(hwnd,1);
}}
return FALSE;
}

static void openOptionsDialog (HWND hwnd) {
DialogBoxParam(hdll, "dlg1", hwnd, dlgproc, NULL);
}

static int encfunc (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX: returnstatic(FORMAT_EXT+2);
case PP_ENC_QUALITY : return quality;
case PP_MIMETYPE : returnstatic(FORMAT_MIME);
case PP_ENC_FLAGS : return *(DWORD*)ptr = FORMAT_FLAGS; 
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


