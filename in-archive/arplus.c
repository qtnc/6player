#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "archive.h"
#include "archive_entry.h"

typedef struct {
const char* fn;
struct archive* ar;
struct archive_entry* entry;
const char* path;
int size, pos;
} ARFILE;

void ar_close (ARFILE* f) {
if (!f) return;
if (f->ar) archive_read_finish(f->ar);
if (f->fn) free(f->fn);
ar_reset(f);
}

void ar_reset (ARFILE* f) {
if (!f) return;
f->entry = NULL;
f->path = NULL;
f->size = 0;
f->pos = 0;
f->fn = NULL;
f->ar = NULL;
}

int ar_opendir (ARFILE* f, const char* fn) {
if (!f) return 0;
struct archive* ar = archive_read_new();
if (!ar) return 0;
archive_read_support_compression_all(ar);
archive_read_support_format_all(ar);
if (archive_read_open_filename(ar, fn, 4096)) {
archive_read_finish(ar);
return 0;
}
//fprintf(stderr, "Format: 0x%X (%s)\r\n", archive_format(ar), archive_format_name(ar));
const char* fn2 = strdup(fn);
ar_close(f);
f->ar = ar;
f->fn = fn2;
return 1;
}

int ar_readdir (ARFILE* f) {
if (!f||!f->ar) return 0;
int re = archive_read_next_header(f->ar, &(f->entry));
if (re!=0) {
//fprintf(stderr, "archive_read_next_header returned %d: %s\r\n", re, archive_error_string(f));
return 0;
}
if (f->entry) {
f->path = archive_entry_pathname(f->entry);
f->size = archive_entry_size(f->entry);
//fprintf(stderr, "%s (%d)\r\n", f->path, f->size);
}
return 1;
}

int ar_open (ARFILE* f, const char* fn, const char* path) {
if (!f) return 0;
ar_close(f);
f->pos = 0;
if (!ar_opendir(f, fn)) return 0;
while (ar_readdir(f)) {
if (f->size>0 && (!f->path || 0==stricmp(f->path, path))) return 1;
}
ar_close(f);
return 0;
}

int ar_read (ARFILE* f, void* buf, size_t len) {
if (f->pos + len > f->size) len = f->size -f->pos;
int read =0, pos = 0;
while (pos<len && (read = archive_read_data(f->ar, buf+pos, len-pos))>0) pos+=read;
f->pos += pos;
return pos;
}

size_t ar_ftell (ARFILE* f) {
if (!f||!f->ar) return -1;
return f->pos;
}

int ar_frewind (ARFILE* f) {
if (!f||!f->path||!f->fn) return 0;
char* path2 = strdup(f->path);
char* fn2 = strdup(f->fn);
int re = ar_open(f, fn2, path2);
free(path2);
free(fn2);
return re;
}

int ar_skip (ARFILE* f, int num) {
if (!f||!f->ar) return 0;
if (num<=0) return 1;
char buf[4096];
int r, x = num>4096? 4096 : num;
while(num>0 && (r=ar_read(f, buf, x))>0) {
num -= r;
x = num>4096? 4096 : num;
}
return 1;
}

int ar_fseek (ARFILE* f, int pos, int ance) {
if (!f||!f->ar) return 0;
switch(ance){
case SEEK_END: return ar_fseek(f, f->size +pos, SEEK_SET);
case SEEK_CUR: return ar_fseek(f, f->pos + pos, SEEK_SET);
case SEEK_SET:
if (pos<0 || pos>f->size) return 0;
else if (pos==f->pos) return 1;
else if (pos>f->pos) return ar_skip(f, pos-f->pos);
else if (pos<f->pos) return ar_frewind(f) && ar_fseek(f, pos, SEEK_SET);
default: return 0;
}}

void ar_close_free (ARFILE* f) {
if (!f) return;
ar_close(f);
free(f);
}

ARFILE* ar_new_open (const char* fn, const char* path) {
ARFILE* f = malloc(sizeof(ARFILE));
ar_reset(f);
if (!ar_open(f, fn, path)) {
ar_close_free(f);
f=NULL;
}
return f;
}

ARFILE* ar_new_opendir (const char* fn) {
ARFILE* f = malloc(sizeof(ARFILE));
ar_reset(f);
if (!ar_opendir(f, fn)) {
ar_close_free(f);
f=NULL;
}
return f;
}

const char* ar_entryname (ARFILE* f) { return f->path; }
int ar_entrysize (ARFILE* f) { return f->size; }



/*int main (int argc, char** argv) {
fprintf(stderr, "Reading %s in %s\r\n", argv[2], argv[1]);
ARFILE* ar = NULL;
if (ar = ar_new_open(argv[1], argv[2])) {
char buf[65536];
int num = ar_read(ar, buf, sizeof(buf));
buf[num]=0;
fwrite(buf, 1, num, (FILE*)stdout);
}
else fprintf(stderr, "%s in %s not found\r\n", argv[2], argv[1]);
ar_close_free(ar);
return 0;
}*/

