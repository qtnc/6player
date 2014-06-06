#ifndef _QCSTDSTR_H
#define _QCSTDSTR_H
#define DLL

DLL char chrtolower (register char c) {
if (c>='A' && c<='Z') return c +32;
if (c>='À' && c<='Þ') return c+32;
return c;
}
DLL char chrtoupper (register char c) {
if (c>='a' && c<='z') return c-32;
if (c>='à' && c<='þ') return c-32;
return c;
}
DLL int _strlen (register const char *str) {
register int c = 0;
while (*str) {
    str++; c++;
}
return c;
}
DLL int strcmp (const char* s1, const char* s2) {
register int l1 = strlen(s1), l2 = strlen(s2), i;
for (i=0; i<l1&&i<l2; i++) {
if (s1[i]==s2[i]) continue;
return s1[i]-s2[i];
}
return l1-l2;
}
DLL char* trimrn (char* str) {
int l = strlen(str);
if (l>=1 && str[l -1]=='\n') str[l -1]=0;
if (l>=2 && str[l -2]=='\r') str[l -2]=0;
return str;
}
DLL int strpos (register const char *str, register const char *needle, register int i) {
register int j=0;
while (str[i]) {
        if (str[i]==needle[j]) {
if (needle[++j]==0) return i-j+1;
        }
        else j=0;
        i++;
        }
return -1;
}
DLL int strcont (register const char *str, register const char *needle, register int i) {
register int j=0;
while (str[i]) {
        if (str[i]==needle[j]) {
if (needle[++j]==0) return i-j+1;
        }
        else return -1;
        i++;
        }
return -1;
}
DLL char* substr (register const char *str, register int index, register int length) {
register char *buf = (char*)malloc(length +1);
register int i=0;
for (i=0; i < length && str[i]!=0; i++) {
    buf[i] = str[index +i];
}
buf[i]=0;
return buf;
}
DLL char* substr_replace (const char *str, const char *repl, int index, int length) {
    int repln = _strlen(repl), strn = _strlen(str);
    char *buf = (char*)malloc(strn + repln - length +1);
    register int i=0, j=0;
    for (i=0; i < index; i++) buf[j++]=str[i];
    for (i=0; i < repln; i++) buf[j++]=repl[i];
    for (i=index+length; i < strn; i++) buf[j++]=str[i];
    buf[j]=0;
    return buf;
}
DLL char* _strcpy (const char *str) {
register     int n = _strlen(str);
    char *buf = (char*)malloc(n+1);
    register int i=0;
    for (i=0; i<n; i++) buf[i]=str[i];
    buf[n]=0;
    return buf;
}
DLL char* _strcat (register const char *s1, register const char *s2) {
register int n1 = _strlen(s1), n2 = _strlen(s2), i=0;
register char *buf = (char*)malloc(n1 + n2 +1);
for (i=0; i < n1; i++) buf[i]=s1[i];
for (i=0; i < n2; i++) buf[n1+i]=s2[i];
buf[n1+n2]=0;
return buf;
}
DLL int strcat2 (char* c1, int l1, const char* c2) {
int l2 = strlen(c2);
register int i = 0;
for (i=0; i < l2; i++) c1[l1+i] = c2[i];
c1[l1+l2]=0;
return l1+l2;
}
DLL char* str_replace (const char *needle, const char *repl, const char *str) {
    int needlen = _strlen(needle), repln = _strlen(repl);
    int index = -1;
char *buf = _strcpy(str);

while ((index = strpos(buf, needle, index +1)) != -1) {
 char *nbuf = substr_replace(buf, repl, index, needlen);
free(buf);
buf = nbuf;
            index += repln - needlen;
            }
    return buf;
}
DLL char* strtolower (const char *str) {
char *buf = _strcpy(str);
register int i=0;
while (buf[i]) buf[i]=chrtolower(buf[i++]);
return buf;
}
DLL char* strtoupper (const char *str) {
char *buf = _strcpy(str);
register int i=0;
while (buf[i]) buf[i]=chrtoupper(buf[i++]);
return buf;
}

#ifdef _PCRE_H
#define PREG_RESULT_LENGTH 60

char* _regex_errormsg = NULL;
int _regex_errorof = -1;
const int _qcstdstr_cmask = PCRE_ANCHORED | PCRE_AUTO_CALLOUT | PCRE_CASELESS | PCRE_DOLLAR_ENDONLY | PCRE_DOTALL | PCRE_DUPNAMES | PCRE_EXTENDED | PCRE_EXTRA | PCRE_FIRSTLINE | PCRE_MULTILINE |   PCRE_NEWLINE_ANY | PCRE_NEWLINE_CR |   PCRE_NEWLINE_CRLF |PCRE_NEWLINE_LF | PCRE_NO_AUTO_CAPTURE | PCRE_UNGREEDY | PCRE_UTF8 | PCRE_NO_UTF8_CHECK ;
const int _qcstdstr_emask = PCRE_ANCHORED  | PCRE_NEWLINE_ANY | PCRE_NEWLINE_CR | PCRE_NEWLINE_CRLF | PCRE_NEWLINE_LF |  PCRE_NOTBOL   | PCRE_NOTEOL | PCRE_NOTEMPTY | PCRE_NO_UTF8_CHECK ;   

char* preg_error () { return _regex_errormsg; }
int preg_error_offset () { return _regex_errorof; }
int preg_match (const char *regex, const char *str, const int **matches, int options, int offset) {
pcre* reg = pcre_compile(regex, options&_qcstdstr_cmask, &_regex_errormsg, &_regex_errorof, NULL);
if (reg==NULL) return -2;

int *vector = (int*)malloc(sizeof(int) * PREG_RESULT_LENGTH);
int result = pcre_exec(reg, NULL, str, _strlen(str), offset, options&_qcstdstr_emask, vector, PREG_RESULT_LENGTH);
if (matches!=NULL) *matches = vector;
else free(vector);
pcre_free(reg);
return result;
}
int preg_search (const char *regex, const char *str, int options, int offset) {
int *matches = NULL;
int result = preg_match(regex, str, &matches, options, offset);
if (result>=0) return matches[0];
else return result;
}
char* preg_replace_callback (const char *regex, char*(*callback)(const char*, const int*, int, void*), const char *str, int options, void* param) {
pcre* reg = pcre_compile(regex, options&_qcstdstr_cmask, &_regex_errormsg, &_regex_errorof, NULL);
if (reg==NULL) return NULL;

options&=_qcstdstr_emask;
int offset = -1;
char *buf = _strcpy(str);
int *vector = (int*)malloc(sizeof(int) * PREG_RESULT_LENGTH);
int result = 0;

while ((result = pcre_exec(reg, NULL, buf, _strlen(buf), offset +1, options, vector, PREG_RESULT_LENGTH)) >=0) {
char *buf2 = callback(buf, vector, result, param);
char *buf3 = substr_replace(buf, buf2, vector[0], vector[1]-vector[0]);
offset = vector[0] + _strlen(buf2);
free(buf);
free(buf2);
buf = buf3;
}
if (result< -1) return NULL;
free(vector);
pcre_free(reg);
return buf;
}
char* _qcstdstr_preg_replace (const char *str, const int *matches, int nmatches, void* param) {
static char buf4[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buf5[]="$0";

char* buf = _strcpy((char*)param);
int i=0;
for (i=0; i < nmatches; i++) {
buf5[1] = buf4[i];
char *buf2 = substr(str, matches[i*2], matches[1+i*2]-matches[i*2]);
char *buf3 = str_replace(buf5, buf2, buf);
free(buf);
free(buf2);
buf = buf3;
}
return buf;
}
char* preg_replace (const char *regex, const char *repl, const char *str, int options) {
return preg_replace_callback(regex, _qcstdstr_preg_replace, str, options, repl);
}
char* preg_quote (const char *str) {
return preg_replace("[-\\(\\)\\[\\]\\{\\}\\+\\*\\?\\\\\\.\\:\\!]", "\\$0", str, 0);
}
char* trim (const char *str) {
return preg_replace("^\\s*(.*?)\\s*$", "$1", str, PCRE_DOTALL);
}

//char*(*)(const char*, const regmatch_t*, int, void*)
#endif
#endif