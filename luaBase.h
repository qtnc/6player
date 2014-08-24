#ifndef _____LUA_BASE_H_____
#define _____LUA_BASE_H_____
#include "consts.h"
#include<string>
extern "C" {
#include<lua/lua.h>
#include<lua/lauxlib.h>
#include<lua/lualib.h>
}

void* lua_pushudata (lua_State* L, void* udata, size_t size, const char* name);
int luaL_checkboolean(lua_State*,int);

inline void lua_newclass (lua_State* L, const char* name) {
luaL_newmetatable(L,name);
lua_pushvalue(L,-1);
}

inline void lua_pushstring (lua_State* L, const std::string& str) {
lua_pushlstring(L, str.c_str(), str.size());
}

#endif
