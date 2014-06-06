@echo off
for %%i in (*.c) do gcc -c -s -O3 -Os %%i -w -o obj\%%~ni.o
for %%i in (*.cpp) do g++ -c -s -O3 -Os -fpermissive %%i -w -o obj\%%~ni.o
g++ -w -s -O3 -Os obj\*.o -shared -o ..\in-archive.dll -Wl,--add-stdcall-alias -L.. -lbass -lzlib -L. -larchive3
