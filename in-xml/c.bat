@echo off
for %%i in (*.c) do gcc -c -s -O3 -Os -std=gnu99 %%i -w -o obj\%%~ni.o
for %%i in (*.cpp) do g++ -c -s -O3 -Os -fpermissive -std=gnu++0x %%i -w -o obj\%%~ni.o
g++ -w -s -O3 -Os obj\*.o -shared -o ..\in-xml.dll -Wl,--add-stdcall-alias -L.. -lbass
