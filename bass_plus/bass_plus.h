#ifndef ___BASS_PLUS_H___
#define ___BASS_PLUS_H___
#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT __declspec(dllexport)

DWORD EXPORT BASS_StreamCreateCopy (DWORD source, DWORD flags, DWORD dspPriority) ;
DWORD EXPORT BASS_StreamGetBitrate (DWORD stream) ;
DWORD EXPORT BASS_StreamCreateStdin (DWORD flags) ;
DWORD EXPORT BASS_StreamCreateRawSocket (const char* host, DWORD flags);

#ifdef __cplusplus
} // extern "C"
#endif
#endif
