#include<stdio.h>
#include<windows.h>
#include "../bass.h"
#include "../bass_vst.h"

extern "C" int getch ();

int main (int argc, char** argv) {
BASS_Init(-1, 44100, 0, 0, 0);
DWORD stream = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, STREAMPROC_DUMMY, NULL);
if (!stream) { printf("ERROR: BASS_StreamCreate: %d\r\n", BASS_ErrorGetCode()); return 1; }
DWORD vst = BASS_VST_ChannelSetDSP(stream, argv[1], 0, 0);
if (!vst) { printf("ERROR: BASS_VST_ChannelSetDSP: %d\r\n", BASS_ErrorGetCode()); return 1; }
BASS_VST_INFO info;
if (!BASS_VST_GetInfo(vst,&info)) { printf("ERROR: BASS_VST_GetInfo: %d\r\n", BASS_ErrorGetCode()); return 1; }
printf("Plugin name: %s\r\n", info.effectName);
printf("Plugin product: %s\r\n", info.productName);
printf("Plugin vendor: %s %#08x\r\n", info.vendorName, info.vendorVersion);
printf("Plugin ID: %#08x\r\n", info.uniqueID);
printf("Plugin version: %#08x\r\n", info.effectVersion);
printf("Plugin VST version: %#08x\r\n", info.effectVstVersion);
printf("Plugin host version: %#08x\r\n", info.hostVstVersion);
printf("Input channels: %d\r\nOutput channels: %d\r\n", info.chansIn, info.chansOut);
if (info.hasEditor) printf("Has GUI %dx%d\r\n", info.editorWidth, info.editorHeight);
else printf("No GUI\r\n");

int nParams = BASS_VST_GetParamCount(vst);
BASS_VST_PARAM_INFO params[3*nParams];
printf("\r\n%d parameters :\r\n", nParams);
printf("Index. Name, Unit, \tMin, Max, Default\r\n");
for (int i=0; i<nParams; i++) {
BASS_VST_PARAM_INFO &p = params[3*i], &p1 = params[3*i+1], &p2 = params[3*i+2];
BASS_VST_GetParamInfo(vst, i, &p);
BASS_VST_SetParam(vst, i, 0);
BASS_VST_GetParamInfo(vst, i, &p1);
BASS_VST_SetParam(vst, i, 1);
BASS_VST_GetParamInfo(vst, i, &p2);
printf("%d. %s, %s, \t%s, %s, %s\r\n", i, p.name, p.unit, p1.display, p2.display, p.display);
}
int nPresets = BASS_VST_GetProgramCount(vst);
printf("\r\n%d presets : \r\n", nPresets);
printf("Index. Name,\t");
for (int i=0; i<nParams; i++) printf("%s,\t", params[3*i].name);
printf("\r\n");
for (int j=0; j<nPresets; j++) {
const char* name = BASS_VST_GetProgramName(vst, j);
const float* values = BASS_VST_GetProgramParam(vst, j);
printf("%d. %s,\t", j, name);
for (int i=0; i<nParams; i++) {
float min=0, max=0, val=values[i];
sscanf(params[3*i+1].display, "%f", &min);
sscanf(params[3*i+2].display, "%f", &max);
if (min!=max) val = (1-val) * (max-min);
printf("%g,\t", val);
}
printf("\r\n");
}


BASS_StreamFree(stream);
stream = BASS_StreamCreateFile(FALSE, "../test.ogg", 0, 0, BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP);
BASS_ChannelPlay(stream, TRUE);
getch();
BASS_VST_ChannelSetDSP(stream, argv[1], 0, 0);
getch();
return 0;
}
