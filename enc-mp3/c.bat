@echo off
for %%i in (*.c) do gcc -c -std=gnu99 %%i -w -o obj\%%~ni.o -s -O3 -Os
windres res.rc -o obj\rc.o
gcc -w -s -O3 -Os obj\*.o -shared -o ..\enc-mp3.dll -Wl,--add-stdcall-alias -Wl,--export-all-symbols -L.. -lbass
