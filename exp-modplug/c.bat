@echo off
for %%i in (*.c) do gcc -c -std=gnu99 %%i -w -o obj\%%~ni.o
gcc -w -s -O3 -Os obj\*.o -shared -o ..\exp-modplug.dll -Wl,--add-stdcall-alias -Wl,--export-all-symbols -L.. -lbass -L../bass_modplug -lmodexport
