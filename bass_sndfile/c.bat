@echo off
for %%i in (*.c) do gcc -c -s -O3 -std=gnu99 %%i -w -o obj\%%~ni.o
gcc -w -s -O3 obj\*.o -shared -o ..\bass_sndfile.dll -Wl,--add-stdcall-alias -L.. -lbass -L. -lsndfile-1
