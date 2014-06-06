#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define EXPORT __declspec(dllexport)
#define returnstatic(s) { static typeof(s) staticValue = s; return staticValue; }

#define FORMAT_FLAGS BASS_ENCODE_AUTOFREE | BASS_ENCODE_PCM;
#define FORMAT_NAME "Wave"
#define FORMAT_EXT "*.wav"
#define FORMAT_MIME "audio/wav"
#define FORMAT_CMDLINE "%f"

static int encfunc (void* handle, int what, void* ptr, int size) {
switch(what){
case PP_DEFEX : returnstatic(FORMAT_EXT+2);
case PP_MIMETYPE : returnstatic(FORMAT_MIME);
case PP_ENC_FLAGS : return *(DWORD*)ptr = FORMAT_FLAGS;
case PP_ENC_COMMANDLINE: returnstatic(FORMAT_CMDLINE);
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


