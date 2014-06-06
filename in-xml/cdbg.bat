@echo off
for %%i in (*.c) do gcc -c -g -std=gnu99 %%i -w -o obj\%%~ni.o
for %%i in (*.cpp) do g++ -c -g -fpermissive %%i -w -o obj\%%~ni.o
g++ -w -g obj\*.o -shared -o ..\in-xml.dll -Wl,--add-stdcall-alias -L.. -lbass
