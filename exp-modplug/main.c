#include<stdio.h>
#include "../bass.h"
#include "../bassenc.h"
#include "../plugin.h"

#define EXPORT __declspec(dllexport)

#define E(x) extern BOOL BASS_ModPlug_Export##x (const char* inFile, const char* outFile );
E(IT) E(XM) E(S3M) E(MOD)
#undef E

static int exportQuery (const char* inFile, const char* outFile, DWORD handle,  DWORD flags, BOOL(*func)(const char*, const char*) ) {
if (flags&PF_EXP_CHECK) {
BASS_CHANNELINFO info;
if (!BASS_ChannelGetInfo(handle,&info)) return 0;
return (info.ctype&0x20000) && inFile && !strchr(inFile,'?');
}
return func(inFile, outFile);
}

#define E(x) static int exportQuery##x (const char* inFile, DWORD flags, const char* outFile, DWORD handle) { return exportQuery(inFile, outFile, handle, flags, BASS_ModPlug_Export##x); }
E(IT) E(XM) E(S3M) E(MOD)
#undef E

static const int nMax = 4;
static const QCPLUGIN data[] = {
{ PT_EXPORTER, 0, "ImpulseTracker Module", "*.it", NULL, NULL, NULL, NULL, exportQueryIT, NULL },
{ PT_EXPORTER, 0, "FastTracker II Module", "*.xm", NULL, NULL, NULL, NULL, exportQueryXM, NULL },
{ PT_EXPORTER, 0, "ScreamTracker3 Module", "*.s3m", NULL, NULL, NULL, NULL, exportQueryS3M, NULL },
{ PT_EXPORTER, 0, "ProTracker Module", "*.mod", NULL, NULL, NULL, NULL, exportQueryMOD, NULL },
};

LPQCPLUGIN EXPORT QCPLUGINPROCNAME (int index) {
if (index<0 || index>=nMax) return NULL;
return data+index;
}


