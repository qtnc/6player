@echo off
for %%i in (*.cpp) do g++ -c -g -std=gnu++0x %%i -w -o obj\%%~ni.o
g++ -w -g obj\*.o -shared -o ..\bass_plus.dll -Wl,--add-stdcall-alias -Wl,--out-implib,../libbass_plus.a -L.. -lbass
