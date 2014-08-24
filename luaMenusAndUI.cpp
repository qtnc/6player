#include "consts.h"
#include "luaBase.h"
#include<string>
#include "strings.hpp"
using namespace std;

#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)
#define streq(a,b) (0==strcmp(a,b))

extern HWND win;
extern HACCEL haccel;

void action (int);

typedef struct {
BOOL sub;
HMENU parent, origin;
int position;
union { int command; HMENU menu; };
} menu;

/*const struct keymapping {
const char* name;
short vk;
} KEYNAMES[] = {
#define K(n) {"F" #n, VK_F##n}
K(1), K(2), K(3), K(4), K(5), K(6), K(7), K(8), K(9), K(10), K(11), K(12), K(13), K(14), K(15), K(16), K(17), K(18), K(19), K(20), K(21), K(22), K(23), K(24),
#undef K
{"Insert", VK_INSERT},
{"Delete", VK_DELETE},
{"Home", VK_HOME},
{"End", VK_END},
{"PageUp", VK_PRIOR},
{"PageDown", VK_NEXT},
{"Pause", VK_PAUSE},
{"Left", VK_LEFT},
{"Right", VK_RIGHT},
{"Down", VK_DOWN},
{"Up", VK_UP},
{"Tab", VK_TAB},
{"Enter", VK_RETURN},
{"Space", VK_SPACE},
{"Backspace", VK_BACK},
{"Escape", VK_ESCAPE},
#define K(n) {"Numpad" #n, VK_NUMPAD##n}
K(0), K(1), K(2), K(3), K(4), K(5), K(6), K(7), K(8), K(9),
#undef K
{"Numpad*", VK_MULTIPLY},
{"Numpad+", VK_ADD},
{"Numpad-", VK_SUBTRACT},
{"Numpad/", VK_DIVIDE},
{"Numpad.", VK_DECIMAL},
{"NumpadEnter", VK_SEPARATOR},
{"Numlock", VK_NUMLOCK},
{"ScrollLock", VK_SCROLL},
{",", 188}, {";", 188},
{".", 190}, {":", 190},
{"-", 189}, {"_", 189},
{"{", 220}, {"}", 223}, {"$", 222},
{"[", 186}, {"]", 192}, {"!", 192},
{"^", 221}, {"'", 219}, {"<", 226}, {">", 226}, {"§", 191}, {"°", 191},
{"BrowserBack", 0xA6}, {"BrowserForward", 0xA7}, 
{NULL,0}
};


const wchar_t* getKeyName (int flags, int vk, BOOL i18n) {
wchar_t* wstr = malloc(sizeof(wchar_t*) * 256);
wstr[0]=0;
int pos = 0;
if (flags&FCONTROL) pos += wsprintf(wstr+pos, L"%ls+", i18n?MSG_CTRL:L"Ctrl");
if (flags&FALT) pos += wsprintf(wstr+pos, L"%ls+", i18n?MSG_ALT:L"Alt");
if (flags&FSHIFT) pos += wsprintf(wstr+pos, L"%ls+", i18n?MSG_SHIFT:L"Shift");
if (flags&FVIRTKEY) {
UINT scan = MapVirtualKey(vk, 0);
pos += GetKeyNameText(scan<<16, wstr+pos, 255-pos);
wstr[pos]=0;
}
else wsprintf(wstr+pos, "%c", toupper(vk));
return wstr;
}

BOOL parseKeyName (const char* kn, int* flags, int* key) {
if (!kn) return FALSE;
char* c = kn, *ckn=0, cckn[64]={0};
int f = FVIRTKEY, k = 0;
while (c=strchr(kn,'+')) {
strcpy(cckn,kn);
cckn[c-kn] = 0;
ckn = trim(cckn);
if (strnicmp(ckn, "Ctrl", c-kn)==0) f |= FCONTROL;
else if (strnicmp(ckn, "Shift", c-kn)==0) f |= FSHIFT;
else if (strnicmp(ckn, "Alt", c-kn)==0) f |= FALT;
kn=c+1;
}
strcpy(cckn,kn);
ckn = trim(cckn);
int knl = strlen(ckn);
if (knl==1) {
char zz = tolower(*ckn);
const wchar_t* wkn = strncvt(&zz, 1, CP_ACP, CP_UTF16, NULL);
short x = VkKeyScan(*wkn);
if ((x&0x7F00)!=0x7F00) {
k = x&0xFF;
if (x&0x100) f |= FSHIFT;
if (x&0x200) f |= FCONTROL;
if (x&0x400) f |= FALT;
}
free(wkn);
}
else {
const struct keymapping* km = KEYNAMES;
for (; km->vk && km->name; km++) {
if (stricmp(km->name, ckn)==0) {
k = km->vk;
break;
}}}
if (k!=0) {
*key = k;
*flags = f;
return TRUE;
}
return FALSE;
}*/

HMENU GetParentMenu (HMENU needle, HMENU origin, int* pos) {
if (!origin) origin = GetMenu(win);
if (origin==needle) return NULL;
for (int i=0, n=GetMenuItemCount(origin); i<n; i++) {
HMENU parent = GetSubMenu(origin, i);
if (parent==needle) {
if (pos) *pos = i;
return origin;
}
else if (parent) {
parent = GetParentMenu(needle, parent, pos);
if (parent) return parent;
}}
return NULL;
}

const char* GetMenuName (HMENU hm, int n) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(hm, n, TRUE, &mii)) return 0;
return mii.dwItemData;
}

BOOL SetMenuName (HMENU hm, int n, const char* name) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_DATA;
if (!GetMenuItemInfo(hm, n, TRUE, &mii)) return FALSE;
if (mii.dwItemData) free(mii.dwItemData);
mii.dwItemData = strdup(name);
return !!SetMenuItemInfo(hm, n, TRUE, &mii);
}

/*static int mh_commandclosure (lua_State* l) {
int cmd = lua_tointeger(l, lua_upvalueindex(1));
action(cmd);
return 0;
}*/

/*static const wchar_t* mh_buildLabel (const wchar_t* base, int flags, int key) {
if (flags<=0 && key<=0) return base;
int wstrl = wcslen(base);
const wchar_t* zzz = malloc(sizeof(wchar_t)* (wstrl+72));
const wchar_t* keyname = getKeyName(flags, key,1);
wsprintf(zzz, L"%ls%lc%ls", base, 8, keyname);
free(keyname);
return zzz;
}*/

static const TCHAR* mh_getLabel (menu* m) {
int ll = GetMenuString(m->parent, m->command, NULL, 0, MF_BYCOMMAND);
TCHAR* wc = malloc(sizeof(TCHAR) * (ll+1));
GetMenuString(m->parent, m->command, wc, ll+1, MF_BYCOMMAND);
wc[ll]=0;
TCHAR* z = wcschr(wc, 8);
if (z)  *z=0;
return wc;
}

static void mh_updateLabel (menu* m, TCHAR* label, int flags, int key) {
/*BOOL alloced = FALSE;
if (!label) {
alloced=TRUE;
label = mh_getLabel(m);
}
if (flags==-1 && key==-1) findAccelerator(m->command, &flags, &key) ;
if (flags!=0 && key!=0) {
const wchar_t* z = mh_buildLabel(label, flags, key);
if (alloced) free(label);
label = z;
}
int flg2 = MF_BYCOMMAND | (m->sub? MF_POPUP : MF_STRING);
ModifyMenu(m->parent, m->command, flg2, m->command, label);
if (alloced || (flags!=0 && key!=0))  free(label);
*/}

static int mh_index (lua_State* l) {
menu* m = lua_touserdata(l,1);
if (lua_isnumber(l,2)) {
if (!m->sub) return 0;
int n = lua_tointeger(l,2);
if (n==0) n=-1;
if (n<0) n += GetMenuItemCount(m->menu);
else n--;
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU;
if (!GetMenuItemInfo(m->menu, n, TRUE, &mii)) return 0;
menu x;
x.sub = mii.hSubMenu!=0;
x.origin = m->origin;
x.parent = m->menu;
x.position = n;
if (mii.hSubMenu!=0) x.menu = mii.hSubMenu;
else x.command = mii.wID;
lua_settop(l,0);
lua_pushudata(l, &x, sizeof(x), "menuhandle");
return 1;
}
string nm = luaL_checkstring(l,2);
if (nm=="checked") {
if (m->sub) luaL_error(l, "unsupported operation");
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_STATE | MIIM_FTYPE;
if (!GetMenuItemInfo(m->parent, m->command, FALSE, &mii)) return 0;
lua_pushboolean(l, 0!=(mii.fState&MFS_CHECKED));
return 1;
}
else if (nm=="radio") {
if (m->sub) luaL_error(l, "unsupported operation");
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
if (!GetMenuItemInfo(m->parent, m->command, FALSE, &mii)) return 0;
lua_pushboolean(l, 0!=(mii.fType&MFT_RADIOCHECK));
return 1;
}
else if (nm=="enabled") {
if (m->sub) luaL_error(l, "unsupported operation");
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_STATE;
if (!GetMenuItemInfo(m->parent, m->command, FALSE, &mii)) return 0;
lua_settop(l,0);
lua_pushboolean(l, 0==(mii.fState&MFS_DISABLED));
return 1;
}
else if (nm=="text") {
string mbc = toString( mh_getLabel(m) );
lua_pushstring(l, mbc);
return 1;
}
else if (nm=="name") {
const char* menuName = GetMenuName(m->parent, m->position);
lua_settop(l,0);
lua_pushstring(l,menuName);
return 1;
}
else if (nm=="accelerator") {
/*int f, k;
if (!(m->sub) && findAccelerator(m->command, &f, &k)) {
const wchar_t* wkn = getKeyName(f,k,0);
const char* kn = strcvt(wkn, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,kn);
free(kn);
free(wkn);
return 1;
}*/
return 0;
}
else if (nm=="onAction") {
if (m->sub) return 0;
if (m->command>=IDM_CUSTOMCOMMAND) {
lua_pushlightuserdata(l, m->command);
lua_gettable(l, LUA_REGISTRYINDEX);
} else {
/*lua_pushinteger(l, m->command);
lua_pushcclosure(l, mh_commandclosure, 1);*/
}
return 1;
}
else if (nm=="builtIn") {
if (m->sub) return 0;
lua_pushboolean(l, m->command<IDM_CUSTOMCOMMAND);
return 1;
}
else if (nm=="parent") {
menu x;
x.sub = TRUE;
x.parent = GetParentMenu(m->parent, m->origin, &(x.position));
x.menu = m->parent;
x.origin = m->origin;
if (!x.parent) return 0;
lua_settop(l,0);
lua_pushudata(l, &x, sizeof(x), "menuhandle");
return 1;
}
else if (nm=="parentHandle") {
lua_pushlightuserdata(l, m->parent);
return 1;
}
else if (nm=="command" || nm=="handle") {
if (m->sub) lua_pushlightuserdata(l, m->command);
else lua_pushinteger(l, m->command);
return 1;
}
//SUITE
else if (luaL_getmetafield(l, 1, nm.c_str() )) return 1;
else if (m->sub) {
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
for (int i=0, n=GetMenuItemCount(m->menu); i<n; i++) {
if (!GetMenuItemInfo(m->menu, i, TRUE, &mii)) continue;
if (mii.dwItemData && streq(nm.c_str(), mii.dwItemData)) {
menu x;
x.sub = mii.hSubMenu!=0;
x.parent = m->menu;
x.origin = m->origin;
x.position = i;
if (mii.hSubMenu!=0) x.menu = mii.hSubMenu;
else x.command = mii.wID;
lua_settop(l,0);
lua_pushudata(l, &x, sizeof(x), "menuhandle");
return 1;
}}}
return 0;
}

static int mh_newindex (lua_State* l) {
menu* m = lua_touserdata(l,1);
string nm = luaL_checkstring(l,2);
if (nm=="checked") {
if (m->sub) luaL_error(l, "unsupported operation");
int flags = MF_BYCOMMAND | (luaL_checkboolean(l,3)? MF_CHECKED : MF_UNCHECKED);
CheckMenuItem(m->parent, m->command, flags);
}
else if (nm=="enabled") {
if (m->sub) luaL_error(l, "unsupported operation");
int flags = MF_BYCOMMAND | (luaL_checkboolean(l,3)? MF_ENABLED : MF_DISABLED);
EnableMenuItem(m->parent, m->command, flags);
}
else if (nm=="name")  SetMenuName(m->parent, m->position, luaL_checkstring(l,3));
else if (nm=="text") {
tstring str = toTString( luaL_checkstring(l,3) );
mh_updateLabel(m, str.c_str(), -1, -1);
}
else if (nm=="onAction") {
if (m->sub || m->command<IDM_CUSTOMCOMMAND) luaL_error(l, "unsupported operation");
if (!lua_isfunction(l,2)) luaL_typerror(l, 2, "function");
lua_pushlightuserdata(l, m->command);
lua_pushvalue(l,3);
lua_rawset(l, LUA_REGISTRYINDEX);
}
else if (nm=="accelerator") {
/*const char* str = NULL;
if (!lua_isnil(l,3)) str = luaL_checkstring(l,3);
int f, k;
if (!(m->sub) && str && parseKeyName(str, &f, &k)) {
BOOL b = removeAccelerator(m->command);
addAccelerator(f, k, m->command);
mh_updateLabel(m, NULL, f, k);
}
else if (!str) {
BOOL b = removeAccelerator(m->command);
mh_updateLabel(m, NULL, 0, 0);
}*/}
else if (nm=="radio") {
BOOL b = luaL_checkboolean(l,3);
MENUITEMINFO mii;
mii.cbSize = sizeof(MENUITEMINFO);
mii.fMask = MIIM_FTYPE;
if (!GetMenuItemInfo(m->parent, m->command, FALSE, &mii)) return 0;
if (b) mii.fType|=MFT_RADIOCHECK;
else mii.fType&=~MFT_RADIOCHECK;
if (!SetMenuItemInfo(m->parent, m->command, FALSE, &mii)) return 0;
return 1;
}
// SUITE
else luaL_argerror(l, 2, "unknown property");
return 0;
}

static int mh_additem (lua_State* l) {
/*menu* m = lua_touserdata(l,1);
if (!m->sub) return 0;
int k = 2, pos = 0;
if (lua_isnumber(l,k)) pos = lua_tointeger(l,k++);
if (pos==0) pos = -1;
else if (pos>0) pos--;
else if (pos<-1) pos += GetMenuItemCount(m->menu);
string str = luaL_checkstring(l,k++);
const char* sShortcut = NULL;
if (lua_isstring(l,k)) sShortcut = lua_tostring(l,k++);
else if (lua_isnoneornil(l,k)) k++;
if (!lua_isfunction(l,k)) luaL_typerror(l, k, "function");
void* p = lua_topointer(l,k++);
if (!p) return 0;
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
int command = addCustomCommand(p) + IDM_CUSTOMCOMMAND;
int kFlags = 0, key=0;
if (parseKeyName(sShortcut, &kFlags, &key)) {
addAccelerator(kFlags, key, command);
const wchar_t* z = mh_buildLabel(wstr, kFlags, key);
if (z!=wstr) {
free(wstr);
wstr = z;
}}
menu it;
it.sub = FALSE;
it.parent = m->menu;
it.origin = m->origin;
it.position = (pos==-1? GetMenuItemCount(m->menu) -1 : pos);
it.command = command;
InsertMenu(m->menu, pos, MF_STRING | MF_BYPOSITION, command, wstr);
DrawMenuBar(win);
free(wstr);
lua_pushlightuserdata(l, command);
lua_pushluafunction(l, p);
lua_rawset(l, LUA_REGISTRYINDEX);
lua_settop(l,0);
lua_pushudata(l, &it, sizeof(it), "menuhandle");
return 1;
*/}

static int mh_addsubitem (lua_State* l) {
menu* m = lua_touserdata(l,1);
if (!m->sub) return 0;
int k = 2, pos = 0;
if (lua_isnumber(l,k)) pos = lua_tointeger(l,k++);
if (pos==0) pos=-1;
else if (pos>0) pos--;
else if (pos<-1) pos += GetMenuItemCount(m->menu);
tstring str = toTString( luaL_checkstring(l,k++) );
HMENU h = CreateMenu();
InsertMenu(m->menu, pos, MF_BYPOSITION | MF_STRING | MF_POPUP, h, str.c_str() );
DrawMenuBar(win);
menu it;
it.sub = TRUE;
it.parent = m->menu;
it.origin = m->origin;
it.position = GetMenuItemCount(m->menu) -1;
it.menu = h;
lua_settop(l,0);
lua_pushudata(l, &it, sizeof(it), "menuhandle");
return 1;
}

static int mh_createpopup (lua_State* l) {
HMENU h = CreatePopupMenu();
menu it;
it.sub = TRUE;
it.parent = NULL;
it.origin = h;
it.position = -2;
it.menu = h;
lua_settop(l,0);
lua_pushudata(l, &it, sizeof(it), "menuhandle");
return 1;
}

static int mh_showpopup (lua_State* l) {
menu* m = lua_touserdata(l,1);
if (!m || !m->menu || !m->sub || m->parent || m->position>=-1) return 0;
POINT p;
GetCursorPos(&p);
TrackPopupMenu(m->menu, 0, p.x, p.y, 0, GetForegroundWindow(), NULL);
return 0;
}

static int mh_deleteitem (lua_State* l) {
menu* m = lua_touserdata(l,1);
if (lua_isnoneornil(l,2)) {
const char* menuName = GetMenuName(m->parent, m->position);
if (menuName) free(menuName);
DeleteMenu(m->parent, m->command, MF_BYCOMMAND);
if (!m->sub) {
//removeCustomCommand(m->command);
//removeAccelerator(m->command);
} 
DrawMenuBar(win);
return 0;
}
else {
if (!m->sub) return 0;
menu* x = lua_touserdata(l,2);
const char* menuName = GetMenuName(m->menu, x->position);
if (menuName) free(menuName);
DeleteMenu(m->menu, x->command, MF_BYCOMMAND);
DrawMenuBar(win);
if (!x->sub) {
//removeCustomCommand(x->command);
//removeAccelerator(x->command);
}
lua_settop(l,1);
return 1;
}}

static int mh_gc (lua_State* l) {
menu* m = lua_touserdata(l,1);
if (m && m->menu && m->sub && !m->parent && m->position<-1) DestroyMenu(m->menu);
m->menu = NULL;
return 0;
}

static int mh_length (lua_State* l) {
menu* m = lua_touserdata(l,1);
if (m->sub) lua_pushinteger(l, GetMenuItemCount(m->menu));
else lua_pushnil(l);
return 1;
}

void luamenuapi_pushmainmenu (lua_State* l) {
HMENU h = GetMenu(win);
if (!h) return 0;
SetMenuName(h, 0, "file");
SetMenuName(h, 1, "player");
SetMenuName(h, 2, "playlist");
SetMenuName(h, 3, "effects");
SetMenuName(h, 4, "help");
menu x;
x.sub = TRUE;
x.origin = h;
x.parent = NULL;
x.position = -1;
x.menu = h;
lua_pushudata(l, &x, sizeof(x), "menuhandle");
}

int luaopen_menuapi (lua_State* l) {
lua_newclass(l, "menuhandle");
lregt(l, "add", mh_additem);
lregt(l, "addSubMenu", mh_addsubitem);
lregt(l, "remove", mh_deleteitem);
lregt(l, "createPopupMenu", mh_createpopup);
lregt(l, "show", mh_showpopup);
lregt(l, "free", mh_gc);
lregt(l, "__len", mh_length);
lregt(l, "__index", mh_index);
lregt(l, "__newindex", mh_newindex);
lregt(l, "__gc", mh_gc);
lua_pop(l,1);
return 0;
}

