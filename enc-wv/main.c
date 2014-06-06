#undef UNICODE
#include<stdio.h>
#include<windows.h>
#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define EXPORT __declspec(dllexport)

#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

static int getCommandLine (char*, int, int) ;
static void openOptionsDialog (HWND) ;

static HINSTANCE hdll = NULL;
static const int presets[] = { 24, 32, 40, 48, 56, 64, 72, 80, 96, 112, 128, 144, 160, 176, 192, 224, 240, 256, 320, 360, 384, 400, 440, 480, 512, 640, 720, 800, 960, 1024, 1280 };
static const int nPresets = 30;
static int quality = 144;

#define FORMAT_FLAGS BASS_ENCODE_AUTOFREE
#define FORMAT_NAME "Wavpack lossy"
#define FORMAT_NAME2 "Wavpack lossless"
#define FORMAT_EXT "*.wv"

BOOL EXPORT WINAPI DllMain (HINSTANCE h, DWORD r1, LPVOID r2) {
hdll = h;
return 1;
}

static int getCommandLine (char* buf, int bufmax, int n) {
char* base = (n==1?
"wavpack -y - \"%f\""
:"wavpack -y -h -b%d - \"%%f\"");
return snprintf(buf, bufmax, base, quality);
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
if (!sscanf(ch, "%d", &quality) || quality<24 || quality>9600) {
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

static int encfunc (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX : returnstatic(FORMAT_EXT+2);
case PP_ENC_QUALITY : return quality;
case PP_ENC_FLAGS : return *(DWORD*)ptr = FORMAT_FLAGS;
case PP_ENC_COMMANDLINE: return getCommandLine(ptr, size, 0);
case PP_ENC_OPTIONS_DIALOG : openOptionsDialog(ptr); return 1;
default: return 0;
}}

static int encfunc2 (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX: returnstatic(FORMAT_EXT+2);
case PP_ENC_QUALITY: return quality;
case PP_ENC_FLAGS : return *(DWORD*)ptr = FORMAT_FLAGS;
case PP_ENC_COMMANDLINE: return getCommandLine(ptr, size, 1);
default: return 0;
}}

static const int nMax = 2;
static const QCPLUGIN data[] = {
{ PT_ENCODER, PF_HAS_OPTIONS_DIALOG, FORMAT_NAME, FORMAT_EXT, NULL, NULL, NULL, NULL, encfunc, NULL },
{ PT_ENCODER, 0, FORMAT_NAME2, FORMAT_EXT, NULL, NULL, NULL, NULL, encfunc2, NULL },
};

LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}


