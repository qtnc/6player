@echo off
for %%i in (*.cpp) do g++ -c -fpermissive -s -O3 -std=gnu++0x %%i -w -o obj\%%~ni.o
g++ -w -s -O3 obj\*.o ..\obj\Socket.o -shared -o ..\bass_plus.dll -Wl,--add-stdcall-alias -Wl,--out-implib,../libbass_plus.a -L.. -lbass -lws2_32
