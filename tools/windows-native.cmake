# Native Windows build using MSYS2 mingw32 + SDL2
# Usage: cmake -DTARGETOS=WINDOWS-NATIVE ..

set(CMAKE_SYSTEM_NAME Windows)

SET(PLATFORM "WINDOWS")

SET(HAS_JPEG 1)

SET(OS_DEFINITIONS " -mwindows ")
SET(OS_LIBS mingw32 gdi32 user32 winmm imm32 ole32 oleaut32 shell32 version uuid)

SET(EXT ".exe")

SET(OSDEP "src/win/osdepwin.c")
