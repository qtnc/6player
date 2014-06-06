#define UNICODE
#define _WIN32_IE 0x0400
#define _WIN32_WINNT 0x0501
#include<windows.h>
#include "File.hpp"
#include<cstdarg>
#include<cstdio>
#include<cstdlib>

bool File::open (const void* path, int flags) {
handle = CreateFile(
path,
flags&READ? GENERIC_READ : GENERIC_WRITE,
flags&READ? FILE_SHARE_READ : (flags&APPEND? FILE_SHARE_WRITE : 0),
0,
flags&READ? OPEN_EXISTING : (flags&APPEND? OPEN_ALWAYS : CREATE_ALWAYS),
flags&DELETE_ON_CLOSE? FILE_FLAG_DELETE_ON_CLOSE : 0,
0);
return handle&&handle!=INVALID_HANDLE_VALUE;
}

int File::write (const void* buf, int len) {
int written=0;
return WriteFile(handle, buf, len, &written, NULL)? written : -1;
}

int File::read (void* buf, int len) {
int read=0;
return ReadFile(handle, buf, len, &read, 0)? (read!=0? read : close(read))  : close();
}

int File::readLine (void* buf, int len, char sep, char ign) {
char* c = buf;
int num=0, re=0;
*c=0;
while (num<len-2 && (re=read(c,1))==1) {
if (*c==ign) *c=0;
else if (*c==sep) { *c=0; break; }
else num++;
c++;
}
if (re<0) return close();
else return num;
}

int File::readVLQ (void) {
unsigned char u = 0;
int val=0, re=0;
bool neg = false;
while((re=read(&u,1))==1) {
val<<=7;
val |= (u&0x7F);
if (u<0x80) break;
else if (u==0x80 && val==0) neg=true;
}
if (neg) val*= -1;
return re>0? val : -1;
}

int File::putc (int n) { return write(&n,1); }

int File::writeVLQ (int x) {
if (x<0) {
if (putc(128)!=1) return -1;
x*= -1;
}
if (x<128) return putc(x);
else if (x<16384) return putc(((x>>7)&0x7F)|0x80)==1 && putc(x&0x7F)==1 ? 2 : -1;
else if (x<2097152) return putc(((x>>14)&0x7F)|0x80)==1 && putc(((x>>7)&0x7F)|0x80)==1 && putc(x&0x7F)==1 ? 3 : -1;
else if (x<268435456) return putc(((x>>21)&0x7F)|0x80)==1 && putc(((x>>14)&0x7F)|0x80)==1 && putc(((x>>7)&0x7F)|0x80)==1 && putc(x&0x7F)==1 ? 4 : -1;
else return putc(((x>>28)&0x7F)|0x80)==1 && putc(((x>>21)&0x7F)|0x80)==1 && putc(((x>>14)&0x7F)|0x80)==1 && putc(((x>>7)&0x7F)|0x80)==1 && putc(x&0x7F)==1 ? 5 : -1;
}

int File::printf (int max, const void* fmt, ...) {
std::string str;
str.clear(); str.reserve(max+2);
char* c = const_cast<char*>(str.data());
int* ci = (int*)c;

va_list lst;
va_start(lst,fmt);
ci[-3] = vsnprintf(c, max, fmt, lst);
va_end(lst);

return write(c, ci[-3]);
}

bool File::flush (void) {
return FlushFileBuffers(handle);
}

long long File::seek (long long pos, int mode) {
long long newpos=0;
return SetFilePointerEx(handle, *(LARGE_INTEGER*)&pos, (LARGE_INTEGER*)&newpos, mode)? newpos: -1;
}

long long File::size (void) {
long long len=0;
return GetFileSizeEx(handle, (LARGE_INTEGER*)&len)? len : -1;
}

int File::close (int re) {
CloseHandle(handle);
handle = 0;
return re;
}

