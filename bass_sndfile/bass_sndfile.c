#include <stdio.h>
#include <string.h>
#include "sndfile.h"
#include "../bass.h"
#include "../bass-addon.h"
#ifndef bassfunc
const BASS_FUNCTIONS *bassfunc=NULL;
#endif

#define EXPORT __declspec(dllexport)

typedef struct {
HSTREAM handle;
BASSFILE file;
SNDFILE* sndf;
SF_INFO info;
BOOL useFloat;
} SNDFStream;

extern const ADDON_FUNCTIONS funcs;
extern const SF_VIRTUAL_IO sfvio;

static sf_count_t BFLength (BASSFILE file) {
return bassfunc->file.GetPos(file, BASS_FILEPOS_END);
}

static sf_count_t BFTell (BASSFILE file) {
return bassfunc->file.GetPos(file, BASS_FILEPOS_CURRENT);
}

static sf_count_t BFSeek (sf_count_t pos, int ance, BASSFILE file) {
switch(ance){
case SEEK_END : pos += BFLength(file); goto _2;
case SEEK_CUR : pos += BFTell(file);
case SEEK_SET: _2: return bassfunc->file.Seek(file, pos)? pos : -1;
default: return -1;
}}

static sf_count_t BFRead (void* buf, sf_count_t len, BASSFILE file) {
return bassfunc->file.Read(file, buf, len);
}


static void WINAPI SNDF_Free (SNDFStream  *stream) {
if (!stream) return;
if (stream->sndf) sf_close(stream->sndf);
free(stream);
}

static DWORD CALLBACK StreamProc(HSTREAM handle, BYTE *buffer, DWORD length, SNDFStream *stream) {
int read;
if (stream->useFloat) {
read = sf_read_float(stream->sndf, buffer, length/sizeof(float));
return read<=0? BASS_STREAMPROC_END : read*sizeof(float);
} else {
read = sf_read_short(stream->sndf, buffer, length/sizeof(short));
return read<=0? BASS_STREAMPROC_END : read*sizeof(short);
} 
return -1;
}

static HSTREAM WINAPI StreamCreateProc(BASSFILE file, DWORD flags) {
if (flags&BASS_SAMPLE_8BITS) error(BASS_ERROR_ILLPARAM); // Not supported
SF_INFO info;
SNDFILE* sndf = sf_open_virtual(&sfvio, SFM_READ, &info, file);
if (!sndf) error(BASS_ERROR_FILEFORM);
SNDFStream* stream = malloc(sizeof(SNDFStream));
memset(stream, 0, sizeof(SNDFStream));
	stream->file=file;
stream->sndf = sndf;
stream->info = info;
stream->useFloat = flags&BASS_SAMPLE_FLOAT;
	flags&=BASS_SAMPLE_FLOAT|BASS_SAMPLE_8BITS|BASS_SAMPLE_SOFTWARE|BASS_SAMPLE_LOOP|BASS_SAMPLE_3D|BASS_SAMPLE_FX 		|BASS_STREAM_DECODE|BASS_STREAM_AUTOFREE|0x3f000000; // 0x3f000000 = all speaker flags
	stream->handle=bassfunc->CreateStream(info.samplerate,info.channels,flags,&StreamProc,stream,&funcs);
	if (!stream->handle) { 
SNDF_Free(stream);
		return 0;
	}
	DWORD fileflags=bassfunc->file.GetFlags(file);
	bassfunc->file.SetStream(file,stream->handle); 
	if (fileflags&BASSFILE_BUFFERED) {
DWORD rate= info.samplerate *  info.channels *(flags&BASS_SAMPLE_FLOAT?4:(flags&BASS_SAMPLE_8BITS?1:2)); 
		bassfunc->file.StartThread(file,rate,0); 
	}
	noerrorn(stream->handle);
}

HSTREAM WINAPI EXPORT BASS_SNDFILE_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.Open(mem,file,offset,length,flags,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_SNDFILE_StreamCreateURL(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenURL(url,offset,flags,proc,user,TRUE);
	if (!bfile) return 0; 
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

HSTREAM WINAPI EXPORT BASS_SNDFILE_StreamCreateFileUser(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user) {
	HSTREAM s;
	BASSFILE bfile =bassfunc->file.OpenUser(system,flags,procs,user,TRUE);
	s=StreamCreateProc(bfile,flags);
	if (!s) bassfunc->file.Close(bfile);
	return s;
}

static QWORD WINAPI SNDF_GetLength(SNDFStream* stream, DWORD mode) {
if (mode!=BASS_POS_BYTE) errorn(BASS_ERROR_NOTAVAIL); // only support byte positioning
	noerrorn(stream->info.frames * stream->info.channels * (stream->useFloat?4:2) );
}

static void WINAPI SNDF_GetInfo(SNDFStream* stream, BASS_CHANNELINFO *info) {
info->freq = stream->info.samplerate;
info->chans = stream->info.channels;
	info->ctype = 0x1E000 | ((stream->info.format>>16)&0xFFF);
switch(stream->info.format&0xFF) {
case 1: case 5 : case 0x10 : case 0x11 : case 0x50: info->origres = 8; break;
case 2 : case 0x41 : case 0x51: info->origres = 16; break;
case 3: case 0x42: info->origres = 24; break;
case 4 : case 6 : info->origres = 32; break;
case 7 : info->origres = 64; break;
case 0x12 : case 0x13 : info->origres = 4; break;
case 0x40 : info->origres = 12; break;
}}

static BOOL WINAPI SNDF_CanSetPosition(SNDFStream *stream, QWORD pos, DWORD mode) {
	if ((BYTE)mode!=BASS_POS_BYTE) error(BASS_ERROR_NOTAVAIL); // only support byte positioning (BYTE = ignore flags)
pos /= (stream->info.channels * (stream->useFloat? sizeof(float) : sizeof(short)));
	if (pos>=stream->info.frames) error(BASS_ERROR_POSITION);
	return TRUE;
}

static QWORD WINAPI SNDF_SetPosition(SNDFStream* stream, QWORD pos, DWORD mode) {
sf_count_t newPos = sf_seek(stream->sndf, pos, SEEK_SET);
if (newPos<0) errorn(BASS_ERROR_POSITION);
return newPos;
}

static const char* WINAPI SNDF_Tags (SNDFStream* sndf, DWORD type) {
if (!sndf || !sndf->sndf || type!=BASS_TAG_OGG) return NULL;
static char buf[32768];
memset(buf, 0, 32768);
int pos=0;
const char *title = sf_get_string(sndf->sndf, SF_STR_TITLE);
const char *artist = sf_get_string(sndf->sndf, SF_STR_ARTIST);
const char *copyright  = sf_get_string(sndf->sndf, SF_STR_COPYRIGHT);
if (title) pos += 1 + snprintf(buf+pos, 32766-pos, "TITLE=%s", title);
if (artist) pos += 1 + snprintf(buf+pos, 32766-pos, "ARTIST=%s", artist);
if (copyright) pos += 1 + snprintf(buf+pos, 32766-pos, "COPYRIGHT=%s", copyright);
memset(buf+pos, 0, 32768-pos);
return pos>0? buf : NULL;
}

const ADDON_FUNCTIONS funcs={
	0, // flags
	SNDF_Free,
	SNDF_GetLength,
SNDF_Tags, // have no tags
	NULL, // let BASS handle file position
	SNDF_GetInfo,
	SNDF_CanSetPosition,
	SNDF_SetPosition,
	NULL, // let BASS handle the position/looping/syncing (POS/END)
NULL, //	RAW_SetSync,
NULL, //	RAW_RemoveSync,
	NULL, // let BASS decide when to resume a stalled stream
	NULL, // no custom flags
	NULL // no attributes
};

const SF_VIRTUAL_IO sfvio = { BFLength, BFSeek, BFRead, NULL, BFTell };

static BASS_PLUGININFO plugininfo = {0x02040000, 0, NULL };

static void SNDF_LoadFormats (void) {
int index=-1, count = 0;
SF_FORMAT_INFO info;
sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof(count));
BASS_PLUGINFORM *form, *forms = malloc((count+1) * sizeof(BASS_PLUGINFORM));
memset(forms, 0, (count+1)*sizeof(BASS_PLUGINFORM));
for (int i=0; i<count; i++) {
info.format = i;
sf_command(NULL, SFC_GET_FORMAT_MAJOR, &info, sizeof(info));
switch(info.format){
case SF_FORMAT_OGG :
case SF_FORMAT_WAV :
case SF_FORMAT_AIFF :
case SF_FORMAT_FLAC :
continue; // Already supported by another BASS plugin
default : {
char tmp[10]={0};
snprintf(tmp, 9, "*.%s", info.extension);
form = forms+(++index);
form->ctype = 0x1E000 | ((info.format>>16)&0xFFF);
form->name = info.name;
form->exts = strdup(tmp);
//printf("%08X, %s, %s\r\n", info.format, info.name, info.extension);
break;
}}}
plugininfo.formats = forms;
plugininfo.formatc = ++index;
//printf("Total %d formats\r\n", plugininfo.formatc);
}

const void* WINAPI EXPORT BASSplugin(DWORD face) {
switch (face) {
		case BASSPLUGIN_INFO:
if (plugininfo.formatc<=0) SNDF_LoadFormats();
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
