@echo off
for %%i in (*.c) do gcc -g -c -std=gnu99 %%i -w -o obj\%%~ni.o
gcc -w -g obj\*.o -shared -o ..\bass_sndfile.dll -Wl,--add-stdcall-alias -L.. -lbass -L. -lsndfile-1
