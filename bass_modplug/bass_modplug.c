#include <stdio.h>
#include <string.h>
#include "modplug.h"
#include "../bass.h"
#include "../bass-addon.h"
#ifndef bassfunc
const BASS_FUNCTIONS *bassfunc=NULL;
#endif

#define smagic(b,s) (!strncmp((b), (s), sizeof((s)) -1))
#define nmagic(b,n) ((*(unsigned int*)(b))==(n))
#define hmagic(b,n) ((*(unsigned short*)(b))==(n))
#define cmagic1(b,s,c) (smagic(b,s) && *(c)>='0' && *(c)<='9')
#define cmagic2(b,s,c,d) (smagic(b,s) && *(c)>='0' && *(c)<='9' && *(d)>='0' && *(d)<='9')

#define EXPORT __declspec(dllexport)

typedef struct {
HSTREAM handle;
ModPlugFile* mod;
BOOL useFloat;
} MPStream;

extern const ADDON_FUNCTIONS funcs;

static BOOL tryMusicFirst = TRUE;
static DWORD modFlags = BASS_MUSIC_POSRESET | BASS_MUSIC_SURROUND | BASS_MUSIC_SURROUND2 | BASS_MUSIC_PRESCAN | BASS_MUSIC_SINCINTER;
static DWORD modFlagsFwd = BASS_MUSIC_DECODE   | BASS_SAMPLE_FLOAT | BASS_MUSIC_AUTOFREE | BASS_SAMPLE_LOOP | BASS_MUSIC_NOSAMPLE | 0x3f000000;

static void modPlugInit (void) {
//MODPLUG_ENABLE_NOISE_REDUCTION
ModPlug_Settings s = { MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION | MODPLUG_ENABLE_REVERB | MODPLUG_ENABLE_MEGABASS | MODPLUG_ENABLE_SURROUND, 2, 16, 44100, MODPLUG_RESAMPLE_SPLINE, 40, 75, 40, 75, 40, 15, -1 };
ModPlug_SetSettings(&s);
}

static void WINAPI MP_Free (MPStream  *stream) {
if (!stream) return;
if (stream->mod) ModPlug_Unload(stream->mod);
free(stream);
}

static DWORD CALLBACK StreamProc(HSTREAM handle, BYTE *buffer, DWORD length, MPStream *stream) {
int read;
if (stream->useFloat) {
read = ModPlug_Read(stream->mod, buffer, length/2);
if (read<=0) return BASS_STREAMPROC_END;
bassfunc->data.Int2Float(buffer, buffer, read/2, 2);
return read*2;
} else {
read = ModPlug_Read(stream->mod, buffer, length);
return (read<=0? BASS_STREAMPROC_END : read);
} 
return -1;
}

static int checkUMXHeader (const char* buf, int len) {
for (int pos=0x40; pos<0x500; pos++) {
if (smagic(buf+pos, "IMPM") || smagic(buf+pos, "Extended Module") || smagic(buf+pos, "SCRM") || nmagic(buf+pos, 0x2e4b2e4d)) return 1;
}
return 0;
}

static inline int checkModHeaders (const char* buf, int len) {
return
smagic(buf, "IMPM")
|| smagic(buf, "Extended Module")
|| smagic(buf+44, "SCRM")
|| smagic(buf, "MTM")
|| smagic(buf, "MO3")
|| smagic(buf, "AMF") || smagic(buf, "Extreme") || smagic(buf, "DBM0") || smagic(buf, "DDMF") || smagic(buf, "DMDL")
|| smagic(buf, "MMD") || smagic(buf, "MT20") || smagic(buf, "OKTA") || smagic(buf+44, "PTMF")
|| smagic(buf+20, "!SCREAM!")
|| hmagic(buf, 0x6669)  || hmagic(buf, 0x4e4a)
|| nmagic(buf, 0xFE524146) || nmagic(buf, 0x204d5350)
|| checkUMXHeader(buf, 1084)
|| smagic(buf, "MAS_UTrack_V00")
|| smagic(buf+1080, "M.K.")  || smagic(buf+1080, "M!K!") || smagic(buf+1080, "M&K!") || smagic(buf+1080, "N.T.") || smagic(buf+1080, "CD81")  || smagic(buf+1080, "OKTA")
|| cmagic1(buf+1080, "FLT", buf+1083)
|| cmagic1(buf+1081, "CHN", buf+1080)
|| cmagic2(buf+1082, "CH", buf+1080, buf+1081)
|| cmagic2(buf+1082, "CN", buf+1080, buf+1081)  
|| cmagic1(buf+1080, "TDZ", buf+1083)
;//END
}

static int readFully (BASSFILE file, void* buffer, int length) {
int pos=0, read=0;
while(pos<length && (read=bassfunc->file.Read(file, buffer+pos, length-pos))>0) pos+=read;
return pos;
}

static HSTREAM WINAPI StreamCreateProc(BASSFILE file, DWORD flags) {
char* buffer = malloc(1084);
if (!buffer || readFully(file, buffer, 1084)<4 || !checkModHeaders(buffer, 1084)) error(BASS_ERROR_FILEFORM);
DWORD length = bassfunc->file.GetPos(file, BASS_FILEPOS_END);
buffer = realloc(buffer, length);
if (!buffer) error(BASS_ERROR_FILEFORM);
readFully(file, buffer+1084, length -1084);
if (tryMusicFirst) {
DWORD f = modFlags | (flags & modFlagsFwd);
if (!(f&BASS_SAMPLE_LOOP)) f|=BASS_MUSIC_STOPBACK; 
HMUSIC mod = BASS_MusicLoad(TRUE, buffer, 0, length, f, 0);
if (mod) {
bassfunc->file.Close(file);
free(buffer);
noerrorn(mod);
}}

ModPlugFile* mod = ModPlug_Load(buffer, length);
free(buffer);
if (!mod) error(BASS_ERROR_FILEFORM);
bassfunc->file.Close(file);
ModPlug_SetMasterVolume(mod,512);
MPStream* stream = malloc(sizeof(MPStream));
memset(stream, 0, sizeof(MPStream));
stream->mod = mod;
stream->useFloat = flags&BASS_SAMPLE_FLOAT;
	flags&=BASS_SAMPLE_FLOAT|BASS_SAMPLE_8BITS|BASS_SAMPLE_SOFTWARE|BASS_SAMPLE_LOOP|BASS_SAMPLE_3D|BASS_SAMPLE_FX 		|BASS_STREAM_DECODE|BASS_STREAM_AUTOFREE|0x3f000000; // 0x3f000000 = all speaker flags
	stream->handle=bassfunc->CreateStream(44100, 2, flags,&StreamProc,stream,&funcs);
	if (!stream->handle) { 
free(buffer);
MP_Free(stream);
		return 0;
	}
noerrorn(stream->handle);
}

HSTREAM WINAPI EXPORT BASS_ModPlug_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.Open(mem,file,offset,length,flags,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_ModPlug_StreamCreateURL(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenURL(url,offset,flags,proc,user,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_ModPlug_StreamCreateFileUser(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenUser(system,flags,procs,user,TRUE);
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

static BOOL exportModule (const char* inFile, const char* outFile, char(*func)(ModPlugFile*,const char*) ) {
BASSFILE file = NULL;
char result = 0, *buffer = NULL;
ModPlugFile*  mod = NULL;

printf("Opening %s...\r\n", inFile);
file =bassfunc->file.Open(FALSE, inFile, 0, 0, 0, TRUE);
if (!file) goto error;
DWORD length = bassfunc->file.GetPos(file, BASS_FILEPOS_END);
printf("Length = %d\r\n", length);
buffer = malloc(length);
if (!buffer) goto error;
printf("Reading file...\r\n");
if (readFully(file, buffer, length)<=0) goto error;
mod = ModPlug_Load(buffer, length);
printf("Mod = %p\r\n", mod);
if (!mod) goto error;
result = func(mod, outFile);
printf("Result = %d\r\n", result);
error :
if (mod) ModPlug_Unload(mod);
if (buffer) free(buffer);
if (file) bassfunc->file.Close(file);
return result;
}

#define E(x) BOOL EXPORT BASS_ModPlug_Export##x (const char* inFile, const char* outFile) { return exportModule(inFile, outFile, ModPlug_Export##x); }
E(IT) E(XM) E(S3M) E(MOD)
#undef E

static QWORD WINAPI MP_GetLength(MPStream* stream, DWORD mode) {
if (mode!=BASS_POS_BYTE) errorn(BASS_ERROR_NOTAVAIL); // only support byte positioning
	noerrorn(ModPlug_GetLength(stream->mod) * 441 * 2 * (stream->useFloat?4:2)  /10);
}

static void WINAPI MP_GetInfo(MPStream* stream, BASS_CHANNELINFO *info) {
info->freq = 44100;
info->chans = 2;
	info->ctype = 0x20010;
info->origres = 16; 
for (int n=ModPlug_GetModuleType(stream->mod); n>0; n>>=1) 	info->ctype++;
}

static BOOL WINAPI MP_CanSetPosition(MPStream *stream, QWORD pos, DWORD mode) {
	if ((BYTE)mode!=BASS_POS_BYTE) error(BASS_ERROR_NOTAVAIL); // only support byte positioning (BYTE = ignore flags)
pos /= (441 * 2 * (stream->useFloat? sizeof(float) : sizeof(short)) /10);
	if (pos>=ModPlug_GetLength(stream->mod)) error(BASS_ERROR_POSITION);
	return TRUE;
}

static QWORD WINAPI MP_SetPosition(MPStream* stream, QWORD pos, DWORD mode) {
long newPos = pos / (441 * 2 * (stream->useFloat? sizeof(float) : sizeof(short)) /10);
if (newPos<0 || newPos>=ModPlug_GetLength(stream->mod)) errorn(BASS_ERROR_POSITION);
ModPlug_Seek(stream->mod, newPos);
return pos;
}

static const char* WINAPI MP_Tags (MPStream* stream, DWORD type) {
if (!stream || !stream->mod) return NULL;
switch(type){
case BASS_TAG_MUSIC_NAME: return ModPlug_GetName(stream->mod);
case BASS_TAG_MUSIC_MESSAGE: return ModPlug_GetMessage(stream->mod);
}
if (type>=BASS_TAG_MUSIC_INST && type<BASS_TAG_MUSIC_INST+ModPlug_NumInstruments(stream->mod)) {
static char buffer[256];
ModPlug_InstrumentName(stream->mod, type-BASS_TAG_MUSIC_INST, buffer);
return buffer;
}
if (type>=BASS_TAG_MUSIC_SAMPLE && type<BASS_TAG_MUSIC_SAMPLE+ModPlug_NumSamples(stream->mod)) {
static char buffer[256];
ModPlug_SampleName(stream->mod, type-BASS_TAG_MUSIC_SAMPLE, buffer);
return buffer;
}
return NULL;
}

const ADDON_FUNCTIONS funcs={
	0, // flags
	MP_Free,
	MP_GetLength,
MP_Tags, // have no tags
	NULL, // let BASS handle file position
MP_GetInfo,
MP_CanSetPosition,
MP_SetPosition,
	NULL, // let BASS handle the position/looping/syncing (POS/END)
NULL, //	RAW_SetSync,
NULL, //	RAW_RemoveSync,
	NULL, // let BASS decide when to resume a stalled stream
	NULL, // no custom flags
	NULL // no attributes
};

static const BASS_PLUGINFORM frm[] = { 
{ 0x20017, "669 Composer module", "*.669" },
{ 0x20020, "Oktalyzer module", "*.okt" },
{ 0x20018, "ULT Module", "*.ult" },
{ 0x2001C, "ASYLUM Music Format", "*.amf" },
{ 0x20026, "ASYLUM Music Format v0", "*.amf0" },
{ 0x20024, "DigiBooster Pro Module", "*.dbm" },
{ 0x20022, "Delusion digital music format", "*.dmf" },
{ 0x2001E, "DSIK module", "*.dsm" },
{ 0x2001F, "DigiTracker MDL module", "*.mdl" },
{ 0x2001A, "Farandole module", "*.far" },
{ 0x20025, "MadTracker 2 Module", "*.mt2" },
{ 0x20023, "PolyTracker module", "*.ptm" },
{ 0x20027, "PSM Module", "*.psm" },
{ 0x20014, "MET module", "*.met" },
{ 0x2001D, "AMS Module", "*.ams" },
};
static BASS_PLUGININFO plugininfo = {0x02040000, 13, frm };

const void* WINAPI EXPORT BASSplugin(DWORD face) {
switch (face) {
		case BASSPLUGIN_INFO:
			return (void*)&plugininfo;
		case BASSPLUGIN_CREATE:
			return (void*)StreamCreateProc;
//		case BASSPLUGIN_CREATEURL:
//			return (void*)StreamCreateURLProc;
	}
	return NULL;
}

BOOL WINAPI EXPORT DllMain(HANDLE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE)hDLL);
			if (HIWORD(BASS_GetVersion())!=BASSVERSION || !GetBassFunc()) {
				MessageBoxA(0,"Incorrect BASS.DLL version (" BASSVERSIONTEXT " is required)", "BASS", MB_ICONERROR | MB_OK);
				return FALSE;
			}
modPlugInit();
			break;
}
	return TRUE;
}
