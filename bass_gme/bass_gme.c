#include<windows.h>
#include <stdio.h>
#include <string.h>
#include "gme.h"
#include "../bass.h"
#include "../bass-addon.h"
#ifndef bassfunc
const BASS_FUNCTIONS *bassfunc=NULL;
#endif

#define EXPORT __declspec(dllexport)

typedef struct {
Music_Emu* gme;
BOOL useFloat;
int curTrack;
} GME;

extern const ADDON_FUNCTIONS funcs;

static int readFully (BASSFILE file, void* buffer, int length) {
int pos=0, read=0;
while(pos<length && (read=bassfunc->file.Read(file, buffer+pos, length-pos))>0) pos+=read;
return pos;
}

static void WINAPI GMEFree (GME *gme) {
if (!gme) return;
if (gme->gme) gme_delete(gme->gme);
free(gme);
}

static DWORD CALLBACK StreamProc(HSTREAM handle, BYTE *buffer, DWORD length, GME *gme) {
printf("play called with buffer of length %d\r\n", length);
if (gme_track_ended(gme->gme)) {
printf("GME track ended: %d\r\n", gme->curTrack);
gme->curTrack++;
if (gme->curTrack>=gme_track_count(gme->gme)) gme->curTrack=0;
gme_start_track(gme->gme,gme->curTrack);
}
if (gme->useFloat) {
const char* err = NULL;
if (err = gme_play(gme->gme, length/sizeof(float), buffer)) {
printf("GME Playing error: %s\r\n", err);
return BASS_STREAMPROC_END;
}
bassfunc->data.Int2Float(buffer, buffer, length/2, 2);
return length;
} else {
const char* err = NULL;
if ((err=gme_play(gme->gme, length/sizeof(short), buffer))) {
printf("GME playing error: %s\r\n", err);
return BASS_STREAMPROC_END;
}
return length;
} 
printf("Problem playing\r\n");
return -1;
}

static HSTREAM WINAPI StreamCreateProc(BASSFILE file, DWORD flags) {
if (flags&BASS_SAMPLE_8BITS) error(BASS_ERROR_ILLPARAM); // Not supported
char* buffer = malloc(1024), *id=NULL;
readFully(file, buffer, 1024);
id = gme_identify_header(buffer);
if (!id||strlen(id)<=0) {
free(buffer);
error(BASS_ERROR_FILEFORM);
}
long length = bassfunc->file.GetPos(file, BASS_FILEPOS_END);
printf("GME ready: length=%d\r\n", length);
buffer = realloc(buffer, length);
if (!buffer) error(BASS_ERROR_FILEFORM);
readFully(file, buffer+1024, length -1024);
Music_Emu* mu = NULL;
const char* err=NULL;
if ((err=gme_open_data(buffer, length, &mu, 44100)) || !mu || (err=gme_start_track(mu,0))) {
printf("Error loading GME: %s\r\n", err);
free(buffer);
error(BASS_ERROR_FILEFORM);
}
printf("Number of tracks: %d\r\n", gme_track_count(mu));
free(buffer);
bassfunc->file.Close(file);
GME* gme = malloc(sizeof(GME));
gme->gme = mu;
gme->curTrack = 0;
gme->useFloat = flags&BASS_SAMPLE_FLOAT;
	flags&=BASS_SAMPLE_FLOAT|BASS_SAMPLE_8BITS|BASS_SAMPLE_SOFTWARE|BASS_SAMPLE_LOOP|BASS_SAMPLE_3D|BASS_SAMPLE_FX 		|BASS_STREAM_DECODE|BASS_STREAM_AUTOFREE|0x3f000000; // 0x3f000000 = all speaker flags
HSTREAM handle=bassfunc->CreateStream(44100, 2, 0, &StreamProc, gme, &funcs);
	if (!handle) { 
printf("Error creating GME handle\r\n");
GMEFree(gme);
		return 0;
	}
printf("GME Success ! %d, %p, %p, %p\r\n", BASS_ErrorGetCode(), handle, gme, gme->gme);
noerrorn(handle);
}

HSTREAM WINAPI EXPORT BASS_GME_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.Open(mem,file,offset,length,flags,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_GME_StreamCreateURL(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenURL(url,offset,flags,proc,user,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_GME_StreamCreateFileUser(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenUser(system,flags,procs,user,TRUE);
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

static QWORD WINAPI GMEGetLength(GME* gme, DWORD mode) {
if (mode!=BASS_POS_BYTE) errorn(BASS_ERROR_NOTAVAIL); // only support byte positioning
return 1000000;
track_info_t info;
if (gme_track_info(gme->gme, &info, 0)) return -1;
printf("Duration: %d\r\n", info.length);
return info.length<=0? -1 : info.length * 44100LL * 4LL / 1000LL;
}

static void WINAPI GMEGetInfo(GME* gme, BASS_CHANNELINFO *info) {
info->freq = 44100;
info->chans = 2;
info->origres = 16;
	info->ctype = 0x1EFFF;
}

static BOOL WINAPI GMECanSetPosition(GME *gme, QWORD pos, DWORD mode) {
	if ((BYTE)mode!=BASS_POS_BYTE) error(BASS_ERROR_NOTAVAIL); // only support byte positioning (BYTE = ignore flags)
QWORD length = GMEGetLength(gme, BASS_POS_BYTE);
pos = pos * 1000LL / (44100LL * 2LL * (gme->useFloat? sizeof(float) : sizeof(short)));
//if (pos<0 || pos>=length) error(BASS_ERROR_POSITION);
	return TRUE;
}

static QWORD WINAPI GMESetPosition(GME* gme, QWORD pos, DWORD mode) {
if (mode!=BASS_POS_BYTE) errorn(BASS_ERROR_POSITION);
DWORD newPos = pos * 1000LL / (44100LL * 2LL * (gme->useFloat? sizeof(float) : sizeof(short)));
//if (gme_seek(gme->gme, newPos)) errorn(BASS_ERROR_POSITION);
return pos;
}

static const char* WINAPI GMETags (GME* gme, DWORD type) {
if (!gme || !gme->gme || type!=BASS_TAG_OGG) return NULL;
track_info_t info;
if (gme_track_info(gme->gme, &info, 0)) return NULL;
static char buf[32768];
memset(buf, 0, 32768);
int pos=0;
const char *title = info.song;
const char *artist = info.author;
const char *copyright  = info.copyright;
const char *comment = info.comment;
if (title) pos += 1 + snprintf(buf+pos, 32766-pos, "TITLE=%s", title);
if (artist) pos += 1 + snprintf(buf+pos, 32766-pos, "ARTIST=%s", artist);
if (copyright) pos += 1 + snprintf(buf+pos, 32766-pos, "COPYRIGHT=%s", copyright);
if (comment) pos += 1 + snprintf(buf+pos, 32766-pos, "COMMENT=%s", copyright);
memset(buf+pos, 0, 32768-pos);
return pos>0? buf : NULL;
}

const ADDON_FUNCTIONS funcs={
	0, // flags
GMEFree,
GMEGetLength,
GMETags, // have no tags
	NULL, // let BASS handle file position
GMEGetInfo,
GMECanSetPosition,
GMESetPosition,
	NULL, // let BASS handle the position/looping/syncing (POS/END)
NULL, //	RAW_SetSync,
NULL, //	RAW_RemoveSync,
	NULL, // let BASS decide when to resume a stalled stream
	NULL, // no custom flags
	NULL // no attributes
};

static const BASS_PLUGINFORM frm[] = 
{ 0, "GME", "*.gme" };
static BASS_PLUGININFO plugininfo = {0x02040000, 1, frm  };

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
			break;
}
	return TRUE;
}
