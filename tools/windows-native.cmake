# Native Windows build using MSYS2 mingw64 + SDL2
# Usage: cmake -DTARGETOS=WINDOWS-NATIVE ..

set(CMAKE_SYSTEM_NAME Windows)

SET(PLATFORM "WINDOWS")

# SDL2 build
SET(HAS_SDL 2)
SET(HAS_SDLTTF 0)
SET(HAS_SDLIMAGE 0)
SET(HAS_ZLIB 1)
SET(HAS_SDLMIXER 1)
SET(HAS_NET 1)
SET(HAS_DLL 0)
SET(HAS_JPEG 0)
SET(HAS_FLI 1)
SET(HAS_MODE8 1)
SET(HAS_GIT 0)

SET(OS_DEFINITIONS " -mwindows ")
SET(OS_LIBS mingw32 gdi32 user32 winmm imm32 ole32 oleaut32 shell32 version uuid)

SET(EXT ".exe")

SET(OSDEP "src/win/osdepwin.c")
