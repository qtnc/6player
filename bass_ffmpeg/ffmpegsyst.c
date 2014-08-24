#include<windows.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#define EXPORT __declspec(dllexport)

typedef enum { false, true } bool;

typedef struct {
AVFormatContext* fmtctx;
AVCodecContext* cctx ;
AVCodec* codec;
AVPacket packet;
AVFrame frame;
CRITICAL_SECTION* cs;
char* buffer;
int bufferCapacity, bufferLength, bufferPos, packetPos, index;
bool seekRequest;
long long curPos, seekPos, length;
void *customHandle, *customBuffer;
int custom;
void(*close)(void*);
} FFMPEG;

void EXPORT ffmpegInit (void) {
av_register_all();
avformat_network_init();
}

FFMPEG* ffmpegOpenImpl (const char* file, AVFormatContext* fmtctx) {
AVCodecContext* cctx = NULL;
AVCodec* codec = NULL;
int index = -1;
printf("Open IMpl\r\n");
if (avformat_open_input(&fmtctx, file, NULL, NULL)<0) return NULL;
printf("After open input\r\n");
if (avformat_find_stream_info(fmtctx, NULL)<0) return NULL;
printf("After find streams\r\n");
printf("%d streams\r\n", fmtctx->nb_streams);
for (int i=0; i<fmtctx->nb_streams; i++) {
if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && index<0) {
index = i; 
//break;
}
else if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE) {
printf("Subtitles found!\r\n");
}}
if (index<0 || index>=fmtctx->nb_streams) return NULL; // No audio
cctx = fmtctx->streams[index]->codec;
printf("Before find decoder\r\n");
codec = avcodec_find_decoder(cctx->codec_id);
printf("After find decoder\r\n");
if (!codec || avcodec_open2(cctx, codec, NULL)) return NULL; // unsupported codec
printf("After open codec\r\n");
FFMPEG* ff = malloc(sizeof(FFMPEG));
ff->fmtctx = fmtctx;
ff->cctx = cctx;
ff->codec = codec;
ff->index = index;
ff->packetPos = -1;
ff->bufferCapacity = 0;
ff->bufferPos = -1;
ff->bufferLength = 0;
ff->buffer = NULL;
ff->seekRequest = false;
ff->curPos = 0;
ff->length = -1;
ff->customBuffer = NULL;
ff->customHandle = NULL;
ff->custom = 0;
ff->close = NULL;
ff->cs = malloc(sizeof(CRITICAL_SECTION));
InitializeCriticalSection(ff->cs);
return ff;
}

FFMPEG* ffmpegOpen (const char* file) {
return ffmpegOpenImpl(file, NULL);
}

FFMPEG* EXPORT ffmpegOpenEx (void* handle, int seekable, int bufsize,  int(*read)(void*,void*,int),  long long(*seek)(void*,long long,int),  void(*close)(void*)  ) {
void* buf = av_malloc(bufsize);
if (!buf) return NULL;
AVFormatContext* fmtctx = avformat_alloc_context();
if (!fmtctx) return NULL;
AVIOContext* io = avio_alloc_context(buf, bufsize, 0, handle, read, NULL, seek);
if (!io) return NULL;
io->seekable = seekable;
fmtctx->iformat = NULL;
fmtctx->oformat = NULL;
fmtctx->priv_data=NULL;
fmtctx->pb = io;
fmtctx->ctx_flags = 0;
fmtctx->flags = AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_DISCARD_CORRUPT;
fmtctx->debug = 0;
strcpy(fmtctx->filename, "Any dummy filename");
printf("probsize = %d\r\n", fmtctx->probesize);
FFMPEG* ff = ffmpegOpenImpl(NULL, fmtctx);
if (ff) {
ff->close = close;
ff->customHandle = handle;
ff->customBuffer = buf;
ff->custom = 1;
}
return ff;
}

void EXPORT ffmpegClose (FFMPEG* ff) {
if (!ff) return;
EnterCriticalSection(ff->cs);
if (ff->cctx) avcodec_close(ff->cctx);
ff->cctx=NULL;
if (ff->fmtctx&&ff->custom) avformat_close_input(&(ff->fmtctx));
ff->fmtctx=NULL;
if (ff->buffer) free(ff->buffer);
ff->buffer = NULL;
if (ff->customBuffer) av_free(ff->customBuffer);
ff->customBuffer = NULL;
if (ff->customHandle && ff->close) ff->close(ff->customHandle);
ff->customHandle = NULL;
LeaveCriticalSection(ff->cs);
DeleteCriticalSection(ff->cs);
free(ff->cs);
free(ff);
}

int EXPORT ffmpegGetSampleRate (FFMPEG* ff) {
if (!ff || !ff->cctx) return -1;
return ff->cctx->sample_rate;
}

int EXPORT ffmpegGetChannels (FFMPEG* ff) {
if (!ff || !ff->cctx) return -1;
return ff->cctx->channels;
}

long long EXPORT ffmpegGetDuration (FFMPEG* ff) {
//fprintf(stderr, "GetDuration\r\n");
if (!ff || !ff->fmtctx) return -1;
if (ff->length>0) return ff->length;
return 2LL
* ff->fmtctx->streams[ff->index]->duration
* ff->fmtctx->streams[ff->index]->time_base.num
* ff->cctx->sample_rate
* ff->cctx->channels
/ ff->fmtctx->streams[ff->index]->time_base.den;
}

static bool ffmpegReadPacket (FFMPEG* ff) {
//fprintf(stderr, "ReadPacket\r\n");
ff->packetPos=0;
while (av_read_frame(ff->fmtctx, &(ff->packet))>=0) {
if (ff->packet.stream_index==ff->index) return true;
av_free_packet(&(ff->packet));
}
return false;
}

static bool ffmpegDecodeAudio (FFMPEG* ff) {
//fprintf(stderr, "DecodeAudio\r\n");
bool gotFrame = false;
while (!gotFrame) {
if (ff->packetPos<0 || ff->packetPos>=ff->packet.size) {
if (ff->packetPos>0) av_free_packet(&(ff->packet));
if (!ffmpegReadPacket(ff)) return false;
}
int decoded = avcodec_decode_audio4(ff->cctx, &(ff->frame), &gotFrame, &(ff->packet));
if (decoded<0) {
av_free_packet(&(ff->packet));
ff->packetPos = -1;
return false;
}
ff->packetPos += decoded;
}
ff->bufferPos = 0;
ff->bufferLength = ff->cctx->channels * ff->frame.nb_samples * sizeof(short); //av_samples_get_buffer_size(NULL, ff->cctx->channels, ff->frame.nb_samples, ff->cctx->sample_fmt, 1);
if (!ff->buffer || ff->bufferCapacity<ff->bufferLength) {
if (ff->buffer) free(ff->buffer);
ff->buffer = malloc(ff->bufferLength);
ff->bufferCapacity = ff->bufferLength;
}
memcpy(ff->buffer, ff->frame.data[0], ff->bufferLength);
return true;
}

int EXPORT ffmpegRead (FFMPEG* ff, short* buf, int buflen) {
//fprintf(stderr, "Read\r\n");
if (!ff || !ff->cctx || !ff->fmtctx) return -1;
EnterCriticalSection(ff->cs);
if (ff->seekRequest) {
int flags = (ff->seekPos<ff->curPos? AVSEEK_FLAG_BACKWARD : 0);
av_seek_frame(ff->fmtctx, -1, ff->seekPos * 1000000LL / (ff->cctx->sample_rate * ff->cctx->channels * sizeof(short)), flags);
avcodec_flush_buffers(ff->codec);
ff->seekRequest = false;
ff->curPos = ff->seekPos*2;
ff->bufferPos = -1;
}
if (ff->bufferPos<0 || ff->bufferPos>=ff->bufferLength) {
if (!ffmpegDecodeAudio(ff)) {
ff->length = ff->curPos;
LeaveCriticalSection(ff->cs);
return 0;
}}
int avail = ff->bufferLength - ff->bufferPos;
if (buflen<avail) avail=buflen;
memcpy(buf, ff->buffer+ff->bufferPos, avail);
ff->bufferPos+=avail;
ff->curPos += avail;
LeaveCriticalSection(ff->cs);
return avail;
}

void EXPORT ffmpegSetPos (FFMPEG* ff, long long pos) {
if (!ff) return;
EnterCriticalSection(ff->cs);
ff->seekPos = pos/2;
ff->seekRequest = true;
LeaveCriticalSection(ff->cs);
}

long long EXPORT ffmpegGetPos (FFMPEG* ff) {
if (!ff) return -1;
return ff->curPos;
}


char* ffmpegGetTags (FFMPEG* ff) {
static char buf[32768];
memset(buf, 0, 32768);
int pos=0;
AVDictionaryEntry *tag = NULL;
while ((tag = av_dict_get(ff->fmtctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
//printf("%s=%s\r\n", tag->key, tag->value);
pos += 1 + snprintf(buf+pos, 32766-pos, "%s=%s", tag->key, tag->value);
}
memset(buf+pos, 0, 32768-pos);
return pos>0? buf : NULL;
}
