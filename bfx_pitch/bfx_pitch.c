#include<windows.h>
#include <stdio.h>
#include <string.h>
#include "../bass.h"
#include "../bass_fx.h"
#include "../bass-addon.h"
#undef bassfunc
const BASS_FUNCTIONS *bassfunc = NULL;


#define CUST_FX_TYPE_ID 0x18000

typedef struct {
HSTREAM pitch, push;
DWORD flags;
} HCustFX;

void WINAPI CustFXFree (HCustFX* hc) {
BASS_StreamFree(hc->pitch);
BASS_StreamFree(hc->push);
free(hc);
}

BOOL WINAPI CustFXSetParam (HCustFX* hc, const float* val) {
return BASS_ChannelSetAttribute(hc->pitch, BASS_ATTRIB_TEMPO_PITCH, *val);
}

BOOL WINAPI CustFXGetParam (HCustFX* hc, float* val) {
return BASS_ChannelGetAttribute(hc->pitch, BASS_ATTRIB_TEMPO_PITCH, val);
}

BOOL WINAPI CustFXReset (HCustFX* hc) {
return TRUE;
}


const static ADDON_FUNCTIONS_FX CustFXFuncs = {
CustFXFree,
CustFXSetParam,
CustFXGetParam,
CustFXReset
};

void CALLBACK CustFXDSPProc (HFX hfx, DWORD chan, void* buf, DWORD len, HCustFX* hc) {
BASS_StreamPutData(hc->push, buf, len);
BASS_ChannelGetData(hc->pitch, buf, len);
}

HFX CALLBACK SetCustFX (DWORD chan, DWORD type, int priority) {
if (type!=CUST_FX_TYPE_ID) return NULL;
BASS_CHANNELINFO info;
BASS_ChannelGetInfo(chan, &info);
HCustFX* hc = malloc(sizeof(HCustFX));
if (!hc) return NULL;
hc->flags = info.flags&( BASS_SAMPLE_FLOAT | BASS_SAMPLE_8BITS );
hc->push = BASS_StreamCreate(info.freq, info.chans, hc->flags | BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
hc->pitch = BASS_FX_TempoCreate(hc->push, BASS_STREAM_DECODE);
HFX hfx = bassfunc->SetFX(chan, CustFXDSPProc, hc, priority, &CustFXFuncs);
if (hfx&&hc->pitch&&hc->push) return hfx;
if (hfx) BASS_ChannelRemoveFX(chan, hfx);
if (hc->pitch) BASS_StreamFree(hc->pitch);
if (hc->push) BASS_StreamFree(hc->push);
free(hc);
return NULL;
}

static BASS_PLUGININFO plugininfo = {0x02040000, 0, NULL };

// Cheat: bass-addon.h bassfunc doesn't work anymore with GCC 4.7
BOOL WINAPI GetBassFunc2 (void) {
HINSTANCE bass = GetModuleHandleA("bass.dll");
bassfunc = GetProcAddress(bass, "_");
return !!bassfunc;
}

const void* WINAPI __declspec(dllexport) BASSplugin(DWORD face) {
switch (face) {
		case BASSPLUGIN_INFO:
			return (void*)&plugininfo;
//		case BASSPLUGIN_CREATE:
//			return (void*)StreamCreateProc;
//		case BASSPLUGIN_CREATEURL:
//			return (void*)StreamCreateURLProc;
	}
	return NULL;
}

BOOL WINAPI __declspec(dllexport) DllMain(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE)hDLL);
			if (HIWORD(BASS_GetVersion())!=BASSVERSION || !GetBassFunc2()) {
				MessageBoxA(0,"Incorrect BASS.DLL version (" BASSVERSIONTEXT " is required)", "BASS", MB_ICONERROR | MB_OK);
				return FALSE;
			}
else {
bassfunc->RegisterPlugin(SetCustFX, PLUGIN_FX_ADD);
}
			break;
}
	return TRUE;
}
