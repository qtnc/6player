@echo off
for %%i in (*.c) do gcc -g -c -std=gnu99 %%i -w -o obj\%%~ni.o
g++ -w -g obj\*.o -shared -o ..\bass_gme.dll -Wl,--add-stdcall-alias -L.. -lbass -L. -lgme
