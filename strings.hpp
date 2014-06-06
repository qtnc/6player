#ifndef _____STRINGS_HPP_8_____
#define _____STRINGS_HPP_8_____
#include<string>
#include<cstring>
#include<cwchar>
#include<algorithm>
#include<cstdlib>
#include<windows.h>

template<class T>
struct basic_split_iterator {
const std::basic_string<T> str;
std::basic_string<T> delims;
int first, last;

basic_split_iterator (const std::basic_string<T>& str, const std::basic_string<T>& delims, int last=0, int first=0) ;
std::basic_string<T> operator* (void) ;
std::basic_string<T> rest (void) ;
void delim (const std::basic_string<T>& delims1) ;
const std::basic_string<T>& delim (void) ;
basic_split_iterator<T>& operator++ (void) ;
bool valid (void) ;
basic_split_iterator<T>& begin (void) ;
basic_split_iterator<T>& end (void) ;
bool operator!= (const basic_split_iterator<T>&) ;
operator bool (void) ;
};
typedef basic_split_iterator<char> split_iterator;
typedef basic_split_iterator<wchar_t> wsplit_iterator;

template <class T> basic_split_iterator<T>::basic_split_iterator (const std::basic_string<T>& str1, const std::basic_string<T>& delims1, int last1, int first1) :
str(str1), delims(delims1), first(first1), last(last1) {
++(*this);
}

template <class T> basic_split_iterator<T>& basic_split_iterator<T>::operator++  (void) {
first = str.find_first_not_of(delims, last);
last = str.find_first_of(delims, first);
return *this;
}

template <class T> std::basic_string<T> basic_split_iterator<T>::operator* (void) {
return std::basic_string<T>( str.begin()+first, last>=0? str.begin()+last : str.end() );
}

template <class T> std::basic_string<T> basic_split_iterator<T>::rest (void) {
return std::basic_string<T>( str.begin()+first, str.end());
}

template <class T> bool basic_split_iterator<T>::valid (void) {
return first>=0;
}

template <class T> const std::basic_string<T>& basic_split_iterator<T>::delim (void) { return delims; }
template <class T> void basic_split_iterator<T>::delim (const std::basic_string<T>& d) { delims=d; }

template <class T> basic_split_iterator<T>& basic_split_iterator<T>::begin () {  return *this;  }
template <class T> basic_split_iterator<T>& basic_split_iterator<T>::end () {  return *this;  }
template <class T> bool basic_split_iterator<T>::operator!= (const basic_split_iterator<T>& unused) {  return valid();  }
template <class T> basic_split_iterator<T>::operator bool  (void) {  return valid();  }

template<class T> basic_split_iterator<T>& operator>> (basic_split_iterator<T>& it, std::basic_string<T>& out) {
out = *it;
++it;
return it;
}

template<class T> bool split (const std::basic_string<T>& src, T sep, std::basic_string<T>& dst1, std::basic_string<T>& dst2) {
int pos = src.find(sep);
if (pos<0 || pos>=src.size()) return false;
dst1 = src.substr(0, pos);
dst2 = src.substr(pos+1);
return true;
}

template<class T> bool split (const std::basic_string<T>& src, T sep, std::basic_string<T>& dst1, std::basic_string<T>& dst2, std::basic_string<T>& dst3) {
std::basic_string<T> tmp;
return split(src, sep, dst1, tmp) && split(tmp, sep, dst2, dst3);
}

template <class T> void replace (std::basic_string<T>& str, const std::basic_string<T>& needle, const std::basic_string<T>& repl) {
for (int pos=0; (pos = str.find(needle))>=0 && pos<str.size(); ) {
str.erase(str.begin()+pos, str.begin()+needle.size()+pos);
str.insert(pos, repl);
pos += repl.size();
}
}

template <class T> void replace (std::basic_string<T>& str, const T* needle, const std::basic_string<T>& repl) { replace(str, std::basic_string<T>(needle), repl); }
template <class T> void replace (std::basic_string<T>& str, const std::basic_string<T>& needle, const T* repl) { replace(str, needle, std::basic_string<T>(repl)); }
template <class T> void replace (std::basic_string<T>& str, const T* needle, const T* repl) { replace(str, std::basic_string<T>(needle), std::basic_string<T>(repl)); }

template <class T> void trim (std::basic_string<T>& s) {
int first=0, last=s.size()  -1;
while (first<s.size() && s[first]<32) first++;
while (last>0 && s[last]<32) last--;
if (first>0) s.erase(s.begin(), s.begin()+first);
if (last<s.size()-1) s.erase(s.begin()+last+1, s.end());
}

template <class T> void toLowerCase (std::basic_string<T>& str) {
transform(str.begin(), str.end(), str.begin(), tolower);
}

template <class T> void toUpperCase (std::basic_string<T>& str) {
transform(str.begin(), str.end(), str.begin(), toupper);
}

// Sprintf++
int wsnprintf (std::wstring& str, int max, const std::wstring& fmt, ...) ;
int snprintf (std::string& str, int max, const std::string& fmt, ...) ;

bool isNumeric (const std::string& str) ;

// Conversion 

inline int toInt (const std::string& str, int base=0) {
return strtol(str.c_str(), 0, base);
}

inline unsigned int toUnsignedInt (const std::string& str, int base=0) {
return strtoul(str.c_str(), 0, base);
}

inline int toInt (const std::wstring& str, int base=0) {
return wcstol(str.c_str(), 0, base);
}

inline unsigned int toUnsignedInt (const std::wstring& str, int base=0) {
return wcstoul(str.c_str(), 0, base);
}

inline long long toLongLong (const std::string& str, int base=0) {
return strtoll(str.c_str(), 0, base);
}

inline unsigned long long toUnsignedLongLong (const std::string& str, int base=0) {
return strtoull(str.c_str(), 0, base);
}

inline double toDouble (const std::string& str) {
return strtod(str.c_str(), 0);
}

inline float toFloat (const std::string& str) {
return strtof(str.c_str(), 0);
}

inline bool toBool (const std::string& str) {
return str=="1" || !stricmp(str.c_str(),"true") || !stricmp(str.c_str(),"on");
}

template <class T> inline size_t& stringSize (std::basic_string<T>& s) {
return ((size_t*)(s.data()))[-3];
}

inline const std::string& toString (const std::string& s) { return s; }
inline const std::wstring& toWString (const std::wstring& ws) { return ws; }
inline std::string toString (const char* str) { return std::string(str?str:""); }
inline std::wstring toWString (const wchar_t* ws) { return std::wstring(ws?ws:L""); }
inline std::wstring toWString2 (const wchar_t* ws) { return toWString(ws); }

inline std::string toString (const std::wstring& ws, int cp = CP_UTF8) {
size_t nSize = WideCharToMultiByte(cp, 0, ws.data(), ws.size(), NULL, 0, NULL, NULL);
std::string s(nSize, '\0');
stringSize(s) = WideCharToMultiByte(cp, 0, ws.data(), ws.size(), s.data(), nSize, NULL, NULL);
return s;
}

inline std::wstring toWString (const std::string& s, int cp = CP_UTF8) {
size_t nSize = MultiByteToWideChar(cp, 0, s.data(), s.size(), NULL, 0);
std::wstring ws(nSize, L'\0');
stringSize(ws) = MultiByteToWideChar(cp, 0, s.data(), s.size(), ws.data(), nSize);
return ws;
}

inline std::string toString (const wchar_t* ws) { std::wstring z(ws?ws:L""); return toString(z); }
inline std::wstring toWString (const char* str) { std::string z(str?str:""); return toWString(z); }

template <class T> inline T fromString (const std::string& s) { throw 0; }
template<> inline std::string fromString (const std::string& s) { return s; }
template<> inline std::wstring fromString (const std::string& s) { return toWString(s); }
template<> inline int fromString (const std::string& s) { return toInt(s); }
template<> inline long long fromString (const std::string& s) { return toLongLong(s); }
template<> inline unsigned long long fromString (const std::string& s) { return toUnsignedLongLong(s); }
template<> inline double fromString (const std::string& s) { return toDouble(s); }
template<> inline float fromString (const std::string& s) { return (float)toDouble(s); }
template<> inline bool fromString (const std::string& s) { return toBool(s); }

inline std::string toString (int n, int base = 10) {
char buf[64]={0};
ltoa(n, buf, base);
return buf;
}

inline std::wstring toWString (int n, int base = 10) {
return toWString(toString(n,base));
}

inline std::string toString (long long n, int base = 10) {
char buf[64]={0};
lltoa(n, buf, base);
return buf;
}

inline std::string toString (DWORD n, int base = 10) {
char buf[64]={0};
lltoa(n, buf, base);
return buf;
}

inline std::string toString (unsigned long long n, int base = 10) {
char buf[64]={0};
ulltoa(n, buf, base);
return buf;
}

inline std::string toString (double d) {
char buf[64]={0};
snprintf(buf, 63, "%.14g", d);
return buf;
}

inline std::string toString (bool b) {
return b?"true":"false";
}

template <class T> bool contains (const std::basic_string<T>& s1, const std::basic_string<T>& s2) {
if (s1.size()<=0 || s2.size()<=0) return false;
for (int i=0, j=0, n=s1.size(), m=s2.size(); i<n; i++) {
if (tolower(s1[i])==tolower(s2[j++])) {
if (j>=m) return true;
}
else j=0;
}
return false;
}

template <class T> bool startsWith (const std::basic_string<T>& s1, const std::basic_string<T>& s2) {
if (s1.size()<=0 || s2.size()<=0) return false;
for (int i=0, j=0, n=s1.size(), m=s2.size(); i<n; i++) {
if (tolower(s1[i])==tolower(s2[j++])) {
if (j>=m) return true;
}
else return false;
}
return false;
}

template <class T> bool endsWith (const std::basic_string<T>& s1, const std::basic_string<T>& s2) {
if (s1.size()<=0 || s2.size()<=0) return false;
for (int n=s1.size(), m=s2.size(), i=n-1, j=m-1; i>=0; i--) {
if (tolower(s1[i])==tolower(s2[j--])) {
if (j<0) return true;
}
else return false;
}
return false;
}

template <class T> inline bool contains (const std::basic_string<T>& s1, const T* s2) { return contains(s1, std::basic_string<T>(s2)); }
template <class T> inline bool startsWith (const std::basic_string<T>& s1, const T* s2) { return startsWith(s1, std::basic_string<T>(s2)); }
template <class T> inline bool endsWith (const std::basic_string<T>& s1, const T* s2) { return endsWith(s1, std::basic_string<T>(s2)); }

#endif


/*
#include<iostream>
using namespace std;
int main (int argc, char** argv) {
string str = "bobobababibibubu";
cout << str << endl;
replace(str, "b", "k");
cout << str << endl;
replace(str, "ko", "zi");
cout << str << endl;
}
*/