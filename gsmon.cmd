@echo off
rem how to use the printer device monitor with GS
setlocal
set gs_lib=c:/ghost/lib;c:/psfonts
set gs_load=10
printmon lpt2 "gs -sDEVICE=ljet3 -sPAPERSIZE=a4 -sOUTPUTFILE=lpt1 -q -"
endlocal
