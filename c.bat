@echo off
for %%i in (*.cpp) do g++ -g -c %%i -w -fpermissive -std=gnu++0x -o obj\%%~ni.o -DFRENCH
windres res.rc -o obj\rc.o -DFRENCH
g++ -g -w -O3 -Os obj\*.o -o test.exe -L. -lbass -lbass_fx -lbassmidi -lbassenc -lbassmix -lbass_vst -lbass_plus -ltags -lws2_32 -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lluajit -mthreads

