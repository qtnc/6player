#include<stdio.h>
#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

#define EXPORT __declspec(dllexport)

#define FORMAT_FLAGS BASS_ENCODE_AUTOFREE;
#define FORMAT_NAME "Free lossless audio codec (flac)"
#define FORMAT_EXT "*.flac"
#define CMDLINE "flac.exe -s -8 -o \"%f\" -"

static int putstr (const char* src, void* dst, int num, int copy) {
if (!src) return 0;
if (num>0) strncpy(dst, src, num);
else if (!copy) *(char**)dst = src;
else *(char**)dst = strdup(src);
return 1;
}

static int encfunc (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX: returnstatic(FORMAT_EXT+2);
case PP_ENC_FLAGS : return *(DWORD*)ptr = FORMAT_FLAGS;
case PP_ENC_COMMANDLINE: return strncpy(ptr, CMDLINE, size);
default: return 0;
}}

static const int nMax = 1;
static const QCPLUGIN data[] = {
{ PT_ENCODER, 0, FORMAT_NAME, FORMAT_EXT, NULL, NULL, NULL, NULL, encfunc, NULL }
};

LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}


