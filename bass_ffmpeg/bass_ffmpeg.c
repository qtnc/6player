#include <stdio.h>
#include <string.h>
#include<windows.h>
#include "../bass.h"
#include "../bass-addon.h"
#ifndef bassfunc
const BASS_FUNCTIONS *bassfunc=NULL;
#endif

#define EXPORT __declspec(dllexport)

typedef struct {
HSTREAM handle;
void* ff;
BOOL useFloat;
} FFStream;

extern const ADDON_FUNCTIONS funcs;

void* ffmpegOpen (const char*) ;
void* ffmpegOpenEx (void* handle, int seekable, int bufsize,  int(*read)(void*,void*,int),  long long(*seek)(void*,long long,int),  void(*close)(void*) );
void ffmpegClose (void*) ;
int ffmpegRead (void*, void*, int) ;
int ffmpegGetSampleRate (void*) ;
int ffmpegGetChannels (void*) ;
long long ffmpegGetDuration (void*) ;
long long ffmpegGetPos (void*) ;
void ffmpegSetPos (void*, long long) ;

static BOOL initialized = FALSE;

static void WINAPI FF_Free (FFStream  *stream) {
if (!stream) return;
if (stream->ff) ffmpegClose(stream->ff);
stream->ff = NULL;
free(stream);
}

static int bassfileRead (BASSFILE file, void* buffer, int size) { return bassfunc->file.Read(file, buffer, size); }
static void bassfileClose (BASSFILE file) { bassfunc->file.Close(file); }
static long long bassfileSeek (BASSFILE file, long long pos, int ance) {
switch(ance) {
case 0x10000: return bassfunc->file.GetPos(file, BASS_FILEPOS_END);
case SEEK_END: pos += bassfunc->file.GetPos(file, BASS_FILEPOS_END); goto _2;
case SEEK_CUR: pos += bassfunc->file.GetPos(file, BASS_FILEPOS_CURRENT);
case SEEK_SET : _2: if (bassfunc->file.Seek(file, pos)) return bassfunc->file.GetPos(file, BASS_FILEPOS_CURRENT);
default: return -1;
}}

static DWORD CALLBACK StreamProc(HSTREAM handle, BYTE *buffer, DWORD length, FFStream *stream) {
if (!stream||!stream->ff) return -1;
int read;
if (stream->useFloat) {
read = ffmpegRead(stream->ff, buffer, length/2);
if (read<=0) return BASS_STREAMPROC_END;
bassfunc->data.Int2Float(buffer, buffer, read/2, 2);
return read*2;
} else {
read = ffmpegRead(stream->ff, buffer, length);
return (read<=0? BASS_STREAMPROC_END : read);
} 
return -1;
}

static HSTREAM StreamCreateProc2 (const void* file, int iscustom, int flags) {
if (!initialized) {
ffmpegInit();
initialized=TRUE;
}
void* ff = NULL;
if (iscustom) {
BOOL seekable = !(bassfunc->file.GetFlags(file) & (BASSFILE_BUFFERED | BASSFILE_BLOCK | BASSFILE_RESTRATE));
ff = ffmpegOpenEx(file, seekable, 65536, bassfileRead, bassfileSeek, bassfileClose);
}
else ff = ffmpegOpen(file);
if (!ff) error(BASS_ERROR_FILEFORM);
FFStream* stream = malloc(sizeof(FFStream));
memset(stream, 0, sizeof(FFStream));
stream->ff = ff;
stream->useFloat = flags&BASS_SAMPLE_FLOAT;
	flags&=BASS_SAMPLE_FLOAT|BASS_SAMPLE_8BITS|BASS_SAMPLE_SOFTWARE|BASS_SAMPLE_LOOP|BASS_SAMPLE_3D|BASS_SAMPLE_FX 		|BASS_STREAM_DECODE|BASS_STREAM_AUTOFREE|0x3f000000; // 0x3f000000 = all speaker flags
	stream->handle=bassfunc->CreateStream(ffmpegGetSampleRate(stream->ff), ffmpegGetChannels(stream->ff), flags,&StreamProc,stream,&funcs);
	if (!stream->handle) { 
FF_Free(stream);
		return 0;
	}
//if (flags&BASS_STREAM_RESTRATE) {
//DWORD rate= info.samplerate *  info.channels *(flags&BASS_SAMPLE_FLOAT?4:(flags&BASS_SAMPLE_8BITS?1:2)); 
//		bassfunc->file.StartThread(file,rate,0); 
//	}
	noerrorn(stream->handle);
}

static DWORD CALLBACK StreamCreateURLProc (const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user) {
if (proc) fprintf(stderr, "Warning: downloadproc unsupported for %s\r\n", url);
return StreamCreateProc2(url, 0, flags | BASS_STREAM_RESTRATE);
}

static HSTREAM WINAPI StreamCreateProc(BASSFILE file, DWORD flags) {
if (flags&BASS_SAMPLE_8BITS) error(BASS_ERROR_ILLPARAM); // Not supported
HSTREAM hs = StreamCreateProc2(file, 1, flags);
return hs;
}

HSTREAM WINAPI EXPORT BASS_FFMPEG_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.Open(mem,file,offset,length,flags,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_FFMPEG_StreamCreateURL(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenURL(url,offset,flags,proc,user,TRUE);
if (bfile) {
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}
if (flags&BASS_UNICODE) return 0; // Unicode not supported for custom URLs
return StreamCreateURLProc(url, offset, flags, proc, user); 
}

HSTREAM WINAPI EXPORT BASS_FFMPEG_StreamCreateFileUser(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenUser(system,flags,procs,user,TRUE);
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

static QWORD WINAPI FF_GetLength(FFStream* stream, DWORD mode) {
if (mode!=BASS_POS_BYTE) errorn(BASS_ERROR_NOTAVAIL); // only support byte positioning
long long len = ffmpegGetDuration(stream->ff);
if (len<0) return -1;
return (stream->useFloat? len*2 : len);
}

static void WINAPI FF_GetInfo(FFStream* stream, BASS_CHANNELINFO *info) {
info->freq = ffmpegGetSampleRate(stream->ff);
info->chans = ffmpegGetChannels(stream->ff);
	info->ctype=0x10000;
info->origres=0;
}

static BOOL WINAPI FF_CanSetPosition(FFStream *stream, QWORD pos, DWORD mode) {
	if ((BYTE)mode!=BASS_POS_BYTE) error(BASS_ERROR_NOTAVAIL); // only support byte positioning (BYTE = ignore flags)
if (stream->useFloat) pos/=2;
	if (pos>=ffmpegGetDuration(stream->ff)) error(BASS_ERROR_POSITION);
	return TRUE;
}

static QWORD WINAPI FF_SetPosition(FFStream* stream, QWORD pos, DWORD mode) {
ffmpegSetPos(stream->ff, pos);
return pos;
}

static const char* WINAPI FF_GetTags (FFStream* stream, DWORD type) {
if (!stream || !stream->ff || (type!=BASS_TAG_OGG && type!=BASS_TAG_MP4)) return NULL;
return ffmpegGetTags(stream->ff);
}

const ADDON_FUNCTIONS funcs={
	0, // flags
FF_Free,
FF_GetLength,
FF_GetTags,
	NULL, // let BASS handle file position
FF_GetInfo,
FF_CanSetPosition,
FF_SetPosition,
	NULL, // let BASS handle the position/looping/syncing (POS/END)
NULL, //	RAW_SetSync,
NULL, //	RAW_RemoveSync,
	NULL, // let BASS decide when to resume a stalled stream
	NULL, // no custom flags
	NULL // no attributes
};

static const BASS_PLUGINFORM frm[] = { 
{ 0, "Audio/video interleaved", "*.avi" },
{ 0, "Flash video", "*.flv;*.swf" },
{ 0, "Apple HTTP Live Streaming", "*.hls" },
{ 0, "MPEG4 video", "*.m4v" },
{ 0, "WebM", "*.webm" },
{ 0, "Theora", "*.ogg;*.ogv" },
{ 0, "VP3, VP5, VP6, Vp8, VPX video", "*.vp3;*.vp5;*.vp6;*.vp6a;*.vp8;*.vpx" },
{ 0, "Quick time movie", "*.mov;*.3gp;*.3g2" },
{ 0, "Nullsoft Streaming Video", "*.nsv" },
{ 0, "ACT voice format", "*.act" },
{ 0, "3GPP AMR", "*.amr;*.amv" },
{ 0, "Windows media video", "*.wmv" },
{ 0, "Advanced streaming format", "*.asf" },
{ 0, "Matroska video", "*.mkv" },
{ 0, "Discworld II BMV", "*.bmv" },
{ 0, "Interplay C93", "*.c93;*.mve" },
{ 0, "Digital video", "*.dv" },
{ 0, "Real media 1.0/14.4, 2.0/28.8", "*.rm;*.ra" },
{ 0, "GSM audio", "*.gsm" },
};
static BASS_PLUGININFO plugininfo = {0x02040000, 18, frm };

const void* WINAPI EXPORT BASSplugin(DWORD face) {
switch (face) {
		case BASSPLUGIN_INFO:
			return (void*)&plugininfo;
		case BASSPLUGIN_CREATE:
			return (void*)StreamCreateProc;
		case BASSPLUGIN_CREATEURL:
			return (void*)StreamCreateURLProc;
	}
	return NULL;
}

BOOL WINAPI EXPORT DllMain(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE)hDLL);
			if (HIWORD(BASS_GetVersion())!=BASSVERSION || !GetBassFunc()) {
				MessageBoxA(0,"Incorrect BASS.DLL version (" BASSVERSIONTEXT " is required)", "BASS", MB_ICONERROR | MB_OK);
				return FALSE;
			}
			break;
}
	return TRUE;
}
