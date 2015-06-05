@echo off
for %%i in (*.c) do gcc -c -g -std=gnu99 %%i -w -o obj\%%~ni.o
gcc -w -g obj\*.o -shared -o ..\bass_fx_pitch.dll -Wl,--add-stdcall-alias -L.. -lbass_fx -lbass
