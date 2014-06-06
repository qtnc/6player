@echo off
for %%i in (*.cpp) do g++ -s -O3 -Os -c %%i -w -fpermissive -std=gnu++0x -o obj\%%~ni.o
windres res.rc -o obj\rc.o
g++ -s -w -O3 -Os obj\*.o -o 6player.exe -L. -lbass -lbass_fx -lbassmidi -lbassenc -lbassmix -lbass_plus -ltags -luser32 -lkernel32 -lws2_32 -lcomdlg32 -lcomctl32 -lluajit -mthreads -mwindows
ren test.exe test.ex2
ren midi.conf midi-old.conf
ren midi-release.conf midi.conf
upx *.dll
upx *.exe
zip -9 -u -q -r 6player.zip *.exe *.dll chorium-compressed.sf2 midi.conf effects.conf readme.txt changelog.txt radios.conf httpdocs\*
ren midi.conf midi-release.conf
ren midi-old.conf midi.conf
ren test.ex2 test.exe
