#define UNICODE
#include<string.h>
#include<string>
#include<cstdio>
#include<cstdarg>
#include<cctype>
#include "consts.h"
#include "strings.hpp"
using namespace std;


int snprintf (string& str, int n, const string& fmt, ...) {
str.clear(); str.reserve(n+2);
char* c = const_cast<char*>(str.data());
int* ci = (int*)c;

va_list lst;
va_start(lst,fmt);
ci[-3] = vsnprintf(c, n, fmt.c_str(), lst);
va_end(lst);
}

int wsnprintf (wstring& str, int n, const wstring& fmt, ...) {
str.clear(); str.reserve(n+2);
wchar_t* c = const_cast<wchar_t*>(str.data());
int* ci = (int*)c;

va_list lst;
va_start(lst,fmt);
ci[-3] = vsnwprintf(c, n, fmt.c_str(), lst);
va_end(lst);
}


tstring formatTime (int n) {
int h = n/3600, m=(n/60)%60, s=n%60;
tstring str;
if (h>0) tsnprintf(str, 10, TEXT("%02d:%02d:%02d"), h, m, s);
else tsnprintf(str, 6, TEXT("%02d:%02d"), m, s);
return str;
}

tstring formatSize (long long l) {
tstring s;
if (l<1024) tsnprintf(s, 16, TEXT("%d") TEXT(MSG_SIZE_B), l);
else if (l<1048576LL) tsnprintf(s, 64, TEXT("%.3g") TEXT(MSG_SIZE_KB) TEXT(" (%lld") TEXT(MSG_SIZE_B) TEXT(")"), l/1024.0, l);
else if (l<1073741824LL) tsnprintf(s, 64, TEXT("%.3g") TEXT(MSG_SIZE_MB) TEXT(" (%lld") TEXT(MSG_SIZE_B) TEXT(")"), l/1048576.0, l);
else if (l<1099511627776LL) tsnprintf(s, 64, TEXT("%.3g") TEXT(MSG_SIZE_GB) TEXT(" (%lld") TEXT(MSG_SIZE_B) TEXT(")"), l/1073741824.0, l);
else tsnprintf(s, 64, TEXT("%.3g") TEXT(MSG_SIZE_TB) TEXT(" (%lld") TEXT(MSG_SIZE_B) TEXT(")"), l/1099511627776.0, l);
return s;
}

char* strtok2 (char* str, char delim) {
static char* s = NULL;
if (str) s = str;
if (!s||!*s) return NULL;
char* c = strchr(s, delim), *a = s;
if (!c||!*c) s=NULL;
else {
*c=0;
s = c+1;
}
return a;
}

long long getFileSize (tstring fn) {
HANDLE handle = CreateFile(fn.c_str(), GENERIC_READ, FILE_SHARE_READ,   0, OPEN_EXISTING, 0, 0);
if (!handle) return -1;
long long ll = -1;
GetFileSizeEx(handle, (PLARGE_INTEGER)&ll);
CloseHandle(handle);
return ll;
}

