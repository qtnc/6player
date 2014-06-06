@echo off
for %%i in (*.c) do gcc -c -s -O3 -Os -std=gnu99 %%i -w -o obj\%%~ni.o
gcc -w -s -O3 -Os obj\*.o -shared -o ..\bass_ffmpeg.dll -Wl,--add-stdcall-alias -L.. -lbass -L. -lavformat -lavcodec -lavutil
