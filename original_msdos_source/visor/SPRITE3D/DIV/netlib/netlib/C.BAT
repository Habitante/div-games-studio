@echo off
del net.lib
wmake
rem wcl386 -c -5r -mf net comlib ipxlib comm dpmi_net
if not exist *.err goto ok
echo ERRORES!!!!
goto fin
:ok
wlib  net.lib +net +comlib +ipxlib +dpmi_net +comm
copy net.lib ..
:fin
