#ifndef __FILE_HPP__
#define __FILE_HPP__
#include "strings.hpp"
#include<string>

class File {
private :
const void* handle;
File (const File&) = delete;
File& operator= (const File&) = delete;
public:
inline File () : handle(0) {}
inline File (const void* name, int mode = READ) : handle(0) { open(name,mode); }
inline ~File () { close(); }
bool open (const void* path, int flags = READ) ;
int write (const void* buffer, int length) ;
int putc (int) ;
int printf (int max, const void* fmt, ...) ;
int writeVLQ (int n) ;
int read (void* buffer, int length) ;
int readLine (void* buffer, int maxlength, char sep='\n', char ign='\r') ;
int readVLQ (void) ;
bool flush (void) ;
long long seek (long long pos = 0, int mode = CUR);
long long size (void) ;
int close (int returnValue = -1) ;

inline bool write (const char* ch) { int l=strlen(ch); return write(ch,l)==l; }
template<class T> inline bool read (T* x) { return read(x, sizeof(T))==sizeof(T); }
template<class T> inline bool write (const T* x) { return write(x, sizeof(T))==sizeof(T); }

inline operator bool (void) { return handle&&handle!=INVALID_HANDLE_VALUE; }
inline bool operator== (const File& f) { return f.handle==handle; }
inline bool operator!= (const File& f) { return !(f==*this); }
inline File& operator() (long long pos, int mode = SET) { seek(pos,mode); return *this; }
inline File& operator[] (long long pos) { return (*this)(pos); }

static const int
WRITE=0, READ=1, APPEND=2,
DELETE_ON_CLOSE=4,
SET=0, CUR=1, END=2;
};

template <class T> inline File& operator<< (File& f, const T& x) {
f.write(x);
return f;
}

template<class T> inline File& operator>> (File& f, T& x) {
f.read(&x);
return f;
}

template<> inline File& operator<< (File& f, const std::string& s) {
f.write(s.c_str(), s.size());
return f;
}

template<> inline File& operator<< (File& f, const std::wstring& s) {
std::string s2 = toString(s);
f.write(s2.c_str(), s2.size());
return f;
}

template<> inline File& operator>> (File& f,  std::string& s) {
s.clear();
s.reserve(1024);
int re = f.readLine(s.data(), s.capacity());
if (re>=0) stringSize(s) = re;
return f;
}

#endif
