#include "../bass.h"
#include "../bassmidi.h"

int main (int argc, char** argv) {
if (!BASS_Init(0, 44100, 0, 0, NULL)) { printf("Couldn't initialize bass\r\n"); return 1; }
HSOUNDFONT sf = BASS_MIDI_FontInit(argv[1],0);
if (!sf) { printf("Couldn't open SF2\r\n"); BASS_Free(); return 1; }
if (!BASS_MIDI_FontPack(sf, argv[2], argv[3], 0)) {
printf("Unable to encode SF2. Error #%d\r\n", BASS_ErrorGetCode());
BASS_Free();
return 1;
}
BASS_Free();
return 0;
}
