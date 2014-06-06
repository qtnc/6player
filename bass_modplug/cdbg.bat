@echo off
for %%i in (*.c) do gcc -g -c -std=gnu99 %%i -w -o obj\%%~ni.o
g++ -w -g obj\*.o -shared -o ..\bassmodplug.dll -Wl,--add-stdcall-alias -L.. -lbass -L. -lmodplug
