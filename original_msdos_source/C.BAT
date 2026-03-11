copy 386.mak makefile /y
copy judas.386 judas.lib /y

wmake /a

wstrip d.exe
rem copy d.exe+2acabada.hpp d.exe /b

copy d.exe system\d.386 /y

rem copy 586.mak makefile /y
rem copy judas.586 judas.lib /y

rem wmake /a

rem wstrip d.exe
rem copy d.exe+2acabada.hpp d.exe /b
