/*
	BASSWV 2.4 C/C++ header file
	Copyright (c) 2007-2012 Un4seen Developments Ltd.

	See the BASSWV.CHM file for more detailed documentation
*/

#ifndef BASSWV_H
#define BASSWV_H

#include "bass.h"

#if BASSVERSION!=0x204
#error conflicting BASS and BASSWV versions
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSWVDEF
#define BASSWVDEF(f) WINAPI f
#endif

// BASS_CHANNELINFO type
#define BASS_CTYPE_STREAM_WV	0x10500

HSTREAM BASSWVDEF(BASS_WV_StreamCreateFile)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags);
HSTREAM BASSWVDEF(BASS_WV_StreamCreateURL)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user);
HSTREAM BASSWVDEF(BASS_WV_StreamCreateFileUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user);

#ifdef __cplusplus
}
#endif

#endif
