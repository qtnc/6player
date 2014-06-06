// ENC AAC+MP4
#undef UNICODE
#include<stdio.h>
#include<windows.h>
#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define EXPORT __declspec(dllexport)

#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

#define FORMAT_FLAGS BASS_ENCODE_AUTOFREE;
#define MP4_FORMAT_NAME "MPEG4 (MP4)"
#define AAC_FORMAT_NAME "Advanced audio coding (AAC)"
#define MP4_FORMAT_EXT "*.m4a"
#define AAC_FORMAT_EXT "*.aac"
#define AAC_FORMAT_MIME BASS_ENCODE_TYPE_AAC

static int getCommandLine (char*, int, int) ;
static void openOptionsDialog (HWND) ;

static HINSTANCE hdll = NULL;
static int presets[] = { 10, 15, 20, 25, 30, 35, 40, 50, 60, 70, 75, 80, 90, 100, 120, 125, 140, 150, 160, 175, 180, 200, 225, 240, 250, 275, 300, 350, 400, 500 };;
static int nPresets = 30;
static int quality = 100;


BOOL EXPORT WINAPI DllMain (HINSTANCE h, DWORD r1, LPVOID r2) {
hdll = h;
return 1;
}

static int getCommandLine (char* buf, int bufmax, int n) {
char* base = (n==0?
"faac.exe -q %d -o \"%%f\" -"
:"faac.exe -q %d --title \"%%t\" --artist \"%%a\" --album \"%%l\" -o \"%%f\" -");
return snprintf(buf, bufmax, base, quality);
}

static int getCommandLine2 (char* buf, int bufmax) {
return snprintf(buf, bufmax, "faac.exe -q %d -P -", quality);
}

static BOOL CALLBACK dlgproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
if (msg==WM_INITDIALOG) {
HWND hcb = GetDlgItem(hwnd,1001);
char ch[20]={0};
for (int i=0; i<nPresets; i++) {
snprintf(ch, 19, "%d", presets[i]);
SendMessage(hcb, CB_ADDSTRING, 0, ch);
}
snprintf(ch, 20, "%d", quality);
SetWindowText(hcb, ch);
SetFocus(hcb);
}
else if (msg==WM_COMMAND) {
switch(LOWORD(wp)) {
case IDOK : {
char ch[20]={0};
GetDlgItemText(hwnd, 1001, ch, 20);
if (!sscanf(ch, "%d", &quality) || quality<10 || quality>500) {
SetFocus(GetDlgItem(hwnd,1001));
MessageBeep(MB_OK);
break;
}}
case IDCANCEL: EndDialog(hwnd,1);
}}
return FALSE;
}

static void openOptionsDialog (HWND hwnd) {
DialogBoxParam(hdll, "dlg1", hwnd, dlgproc, NULL);
}

static int aacFunc (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX: returnstatic(AAC_FORMAT_EXT+2);
case PP_ENC_QUALITY: return quality;
case PP_MIMETYPE : returnstatic(AAC_FORMAT_MIME);
case PP_ENC_FLAGS : return *(DWORD*)ptr = FORMAT_FLAGS;
case PP_ENC_COMMANDLINE: return getCommandLine(ptr, size, 0);
case PP_ENC_CAST_COMMANDLINE : return getCommandLine2(ptr, size);
case PP_ENC_OPTIONS_DIALOG : openOptionsDialog(ptr); return 1;
default: return 0;
}}

static int mp4Func (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX: returnstatic(MP4_FORMAT_EXT+2);
case PP_ENC_QUALITY : return quality;
case PP_ENC_COMMANDLINE: return getCommandLine(ptr, size, 1);
case PP_ENC_OPTIONS_DIALOG : openOptionsDialog(ptr); return 1;
default: return 0;
}}

static const int nMax = 2;
static const QCPLUGIN data[] = {
{ PT_ENCODER, PF_CAN_CAST | PF_HAS_OPTIONS_DIALOG, AAC_FORMAT_NAME, AAC_FORMAT_EXT, NULL, NULL, NULL, NULL, aacFunc, NULL },
{ PT_ENCODER, PF_HAS_OPTIONS_DIALOG, MP4_FORMAT_NAME, MP4_FORMAT_EXT, NULL, NULL, NULL, NULL, mp4Func, NULL },
};

LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}


