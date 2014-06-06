#include<cstdio>
#include<windows.h>
#include<string>
using namespace std;

bool download (const string& url, const string& file) {
typedef int(*WINAPI func)(void*, const char*, const char*, int, void*) ;
static func f = NULL;
if (!f) {
HINSTANCE h = LoadLibrary("urlmon.dll");
f = (func)GetProcAddress(h, "URLDownloadToFileA");
}
printf("Downloading of <%s> into <%s> ", url.c_str(), file.c_str());
if (!f) return false;
int re = f(NULL, url.c_str(), file.c_str(), 0, NULL);
int le = GetLastError();
printf("\r\nReturn = %d (%#X), error = %d (%#X)", re, re, le, le);
return !re;
}
