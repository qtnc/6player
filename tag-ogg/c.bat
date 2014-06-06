@echo off
for %%i in (*.cpp) do g++ -c -std=gnu++0x %%i -w -fpermissive -o obj\%%~ni.o
g++ -w -s -O3 -Os obj\*.o -shared -o ..\tag-ogg.dll -Wl,--add-stdcall-alias -Wl,--export-all-symbols -L.. -lbass
