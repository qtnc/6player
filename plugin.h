#ifndef ___QC_PLUGINS___
#define ___QC_PLUGINS___
#include "bass.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PT_INPUT 1
#define PT_PLAYLIST 2
#define PT_ENCODER 3
#define PT_EXPORTER 4
#define PT_TAGGER 5

#define PF_READ 0
#define PF_WRITE 1
#define PF_OPEN 2
#define PF_OPEN_READ PF_OPEN | PF_READ
#define PF_OPEN_WRITE PF_OPEN | PF_WRITE
#define PF_CLOSE 8
#define PF_EXP_CHECK 16
#define PF_TAG_CHECK 16

#define PF_CAN_READ 1
#define PF_CAN_WRITE 2
#define PF_UNICODE 4
#define PF_CAN_CAST 8
#define PF_HAS_OPTIONS_DIALOG 16

#define PP_ENC_QUALITY 0x300
#define PP_MIMETYPE 0x301
#define PP_ENC_FLAGS 0x302
#define PP_ENC_COMMANDLINE 0x303
#define PP_ENC_CAST_COMMANDLINE 0x304
#define PP_ENC_PROC 0x305
#define PP_DEFEX 0x306
#define PP_ENC_OPTIONS_DIALOG 0x307
#define PP_CAST_HEADERS 0x308

typedef struct {
unsigned int type, flags;
const char *desc, *exts;
void*(*open)(const void* lpPlugin, const void* file, int mode, int flags);
int(*read)(void* handle, void* buffer, int size);
int(*write)(void* handle, const void* buffer, int size);
void(*close)(void* handle);
union {
int(*get)(void* handle, int what, void* buffer, unsigned int size);
int(*query)(void* handle, int what, void* buffer, unsigned int size);
const char*(*getString)(void* handle, int what, void* buffer, unsigned int size);
void*(*getPointer)(void* handle, int what, void* buffer, unsigned int size);
};
int(*set)(void* handle, int what, const void* buffer, unsigned int size);
} QCPLUGIN, *LPQCPLUGIN;

#define QCPLUGINPROCNAME qcPlugin_getInfo40
#define QCPLUGINPROCNAMESTR "qcPlugin_getInfo40"
typedef LPQCPLUGIN(*QCPLUGINPROC)(int);

#ifdef __cplusplus
} // extern "C"
#endif
#endif
