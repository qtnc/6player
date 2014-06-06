#include "consts.h"
extern "C" {
#include<lua/lua.h>
#include<lua/lauxlib.h>
#include<lua/lualib.h>
}
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#include "strings.hpp"
#include "Socket.hpp"
#include<vector>
#include "playlist.h"
using namespace std;

struct playlistitemdata {
playlistitem* it;
int num;
};

extern int curSong;
extern vector<playlistitem> playlist;

lua_State* L = NULL;

static int luaL_fielderror (lua_State* L, const char* classname, const char* fieldname) {
return luaL_error(L, "Unable to access field %s for object of type %s: this fiels may not exist, be read only, or the type of value supplied to write was unexpected.", fieldname, classname);
}

static int lua_getbacktrace (lua_State* l) {
lua_getglobal(l, "debug");
lua_getfield(l, -1, "traceback");
lua_pushvalue(l,1);
lua_pushinteger(l,2);
if (lua_pcall(l, 2, 1,0)) {
printf("Error in error handling !\r\n");
}
return 1;
}

/*const char* callfunc (lua_State* L, void* p, int nArgs, int nRet) {
lua_pushcfunction(L, lua_getbacktrace);
lua_insert(L, -1 -nArgs);
lua_pushluafunction(L,p);
lua_insert(L, -1 -nArgs);
if (lua_pcall(L, nArgs, nRet, -2 -nArgs)) {
return lua_tostring(L,-1);
}
return NULL;
}*/

static inline void lua_pushstring (lua_State* L, const string& str) {
lua_pushlstring(L, str.c_str(), str.size());
}

static inline void lua_regt (lua_State* L, const char* name, int(*func)(lua_State*), int cc=0) {
lua_pushcclosure(L, func, cc);
lua_setfield(L, -2, name);
}

static void* lua_pushudata (lua_State* L, void* udata, size_t size, const char* name) {
void* ptr = lua_newuserdata(L, size);
if (!ptr) return NULL;
luaL_getmetatable(L, name);
lua_setmetatable(L, -2);
memcpy(ptr, udata, size);
return ptr;
}

static void lua_pushplaylistitem (lua_State* L, playlistitem& it, int num = -1) {
playlistitemdata data = { &it, num };
lua_pushudata(L, &data, sizeof(data), "playlistitem");
}

static playlistitem& lua_toplaylistitem (lua_State* L, int index, int* num=0) {
playlistitemdata* data = lua_topointer(L,index);
if (num) *num = data->num;
return *(data->it);
}

static int playlist_len (lua_State* L) {
lua_pushinteger(L, playlist.size());
return 1;
}

static int playlist_index (lua_State* L) {
if (!lua_isnumber(L,2)) {
lua_rawget(L,-2);
return 1;
}
int index = lua_tointeger(L,2);
if (index==0) index = curSong;
else if (index>0) index--;
else index += playlist.size();
if (index<0 || index>=playlist.size()) return 0;
lua_pushplaylistitem(L, playlist[index], index);
return 1;
}

static int playlistitem_index (lua_State* L) {
playlistitem& it = lua_toplaylistitem(L,1);
string name = lua_tostring(L,2);
#define G(n,m,t) if (name==#n) { lua_push##t(L, it.m); return 1; }
#define S(n) G(n,n,string)
S(file) S(title) S(artist) S(album)
S(genre) S(subtitle) S(composer) S(copyright) S(year) 
S(disc) S(tracknum) S(comment)
G(duration,length,number)
G(todate,metadataSet,boolean)
#undef S
#undef G
else return luaL_getmetafield(L, 1, name.c_str());
}

static int playlistitem_newindex (lua_State* L) {
int index = -1;
playlistitem& it = lua_toplaylistitem(L, 1, &index);
string name = lua_tostring(L,2);
#define G(n,m,t) else if (name==#n && lua_is##t(L,3)) it.m = lua_to##t(L,3);
#define S(n) G(n,n,string)
if(false){}
S(file) S(title) S(artist) S(album)
S(genre) S(subtitle) S(composer) S(copyright) S(year) 
S(disc) S(tracknum) S(comment)
G(duration,length,number)
G(todate,metadataSet,boolean)
#undef S
#undef G
else return luaL_fielderror(L, "playlistitem", name.c_str());
return 0;
}

void luaRunWebScript (Socket* sock, string filename, string method, string uri, string query, unordered_map<string,string>& headers, string data) {
string status = "HTTP/1.1 200 OK\r\n\r\nIt works !";
sock->send(status.c_str(), status.size());
}

void luaScriptingInit (void) {
L = lua_open();
luaL_openlibs(L);

lua_settop(L,0);
luaL_newmetatable(L, "playlistitem");
lua_regt(L, "__index", playlistitem_index);
lua_regt(L, "__newindex", playlistitem_newindex);

lua_settop(L,0);
lua_newtable(L);
lua_regt(L, "__len", playlist_len);
lua_regt(L, "__index", playlist_index);
lua_pushvalue(L,-1);
lua_setmetatable(L,-2);
lua_setglobal(L,"playlist");

lua_newtable(L);
lua_setglobal(L, "player");

if (luaL_dofile(L,"autoexec.lua")) printf("ERROR: %s\r\n", lua_tostring(L,-1));
}


