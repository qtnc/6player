@echo off
for %%i in (*.c) do gcc -c -s -O3 -std=gnu99 %%i -w -o obj\%%~ni.o
g++ -w -s -O3 obj\*.o -shared -o ..\bass_gme.dll -Wl,--add-stdcall-alias -L.. -lbass -L. -lgme
