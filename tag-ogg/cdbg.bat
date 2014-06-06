@echo off
for %%i in (*.cpp) do g++ -c -g -std=gnu++0x %%i -w -fpermissive -o obj\%%~ni.o
g++ -w -g obj\*.o -shared -o ..\tag-ogg.dll -Wl,--add-stdcall-alias -Wl,--export-all-symbols -L.. -lbass
