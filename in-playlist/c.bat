@echo off
for %%i in (*.cpp) do g++ -c -s -O3 -Os -fpermissive -std=gnu++0x %%i -w -o obj\%%~ni.o
g++ -w -s -O3 -Os obj\*.o -shared -o ..\in-playlist.dll -Wl,--add-stdcall-alias -L.. -lbass
