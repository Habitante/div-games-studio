# DIV Games Studio

**DIV Games Studio** was a complete game development environment for MS-DOS, created
in the mid-1990s by Daniel Navarro Medrano at Hammer Technologies in Spain. It shipped
as a self-contained IDE with a built-in code editor, compiler, sprite/map/font editors,
a sound manager, and an 8-bit software-rendered runtime — all written in C and x86
assembly targeting real-mode DOS. It was Daniel's first major C project, coming from a
background in Z80 and 8086 assembly programming — which explains both the ambition of
the architecture and the, shall we say, creative coding conventions throughout.

This repository is the open-source continuation of that codebase, ported from its
original DJGPP/DOS toolchain to modern platforms using SDL. The port and ongoing
maintenance were led by Mike Green (MikeDX / DX Games). Daniel (the User here, on 
this machine) remains involved and is the best (possibly only) person who can explain 
why things are the way they are (if memory serves ... its been a while).

> 100% compatible with existing DIV1 and DIV2 code including plugin DLLs — even MODE8 3D.

## What is DIV?

DIV is both a **programming language** and an **IDE**. Programs are written in a
Pascal/C-like language with built-in support for processes (lightweight concurrent
actors), sprite rendering, collision detection, scroll/mode7 backgrounds, sound, and
more. The language compiles to bytecode that runs on its own virtual machine.

The IDE provides a full desktop environment (yes, inside the program) with:

- Multi-file code editor with syntax highlighting
- FPG sprite archive manager
- MAP/sprite editor with drawing tools
- Font editor and generator
- Palette editor
- Sound/PCM player
- One-click compile and run
- Integrated debugger

## Codebase overview

~150,000 lines of C across 136 source files and 81 headers. The code dates back to
the mid-90s and carries all the hallmarks of that era: single-letter variable names,
Spanish comments and identifiers, 8-bit palette rendering, `int`-sized pointer
arithmetic, and lots of global state. It was originally built for 16/32-bit DOS with
DJGPP and has been progressively adapted for modern platforms through an OS-dependency
abstraction layer.

```
src/
  div.c              Main IDE entry point, desktop environment, menus
  divc.c             DIV language compiler (lexer + parser + codegen)
  divbasic.c         Expression evaluator / calculator
  divedit.c          Code editor
  divfont.c          Font editor
  divfpg.c           FPG sprite archive editor
  divpaint.c         Sprite/MAP drawing tools
  divcolor.c         Palette editor
  divmouse.c         Mouse input + SDL event loop (IDE)
  divkeybo.c         Keyboard input + SDL event loop (IDE)
  divmixer.c         Sound playback
  divvideo.c         Video mode setup
  diveffec.c         Screen transition effects
  divhandl.c         Window/dialog handler
  global.h           Master header — types, externs, the lot

  runtime/
    i.c              DIV2 bytecode interpreter (the VM)
    s.c              Sprite rendering engine (8-bit blitter)
    f.c              Runtime built-in functions
    divmixer.c       Runtime sound
    divlengu.c       Runtime language/text strings
    vpe/             MODE8 3D engine (raycaster / voxel)

  div1run/           DIV1-compatible runtime (older bytecode format)

  runner/
    r.c              Launcher — starts the IDE, chains to debugger

  shared/
    osdep/
      osd_sdl2.c/.h  SDL2 OS-dependency layer (display, input, audio)
      osd_sdl12.c/.h SDL 1.2 OS-dependency layer (legacy)
    run/              Shared runtime support (keyboard, mouse, sound, FLI)
    lib/sdlgfx/       Bundled SDL_gfx (framerate limiter)
    lib/zip/           Zip archive support

  win/
    osdepwin.c       Windows-specific helpers (fmemopen shim, etc.)

  visor/             "Visor" — the low-level rendering / UI composition layer

tools/               CMake toolchain files per platform
div/                 Runtime directory (executables, DLLs, data files)
build/               CMake build output
```

### Key architectural notes

- **8-bit paletted rendering**: Everything is drawn to an 8-bit surface, then
  blitted to a 32-bit SDL2 texture for display. The palette is the classic VGA
  256-color model.
- **Process model**: DIV programs use cooperative multitasking. Each `PROCESS` is
  an actor with its own state, rendered as a sprite. The interpreter round-robins
  through active processes each frame.
- **OS abstraction**: The `OSDEP_*` functions in `osd_sdl2.c` abstract all
  platform-specific operations (window creation, input, audio). Porting to a new
  platform means implementing this interface.
- **32-bit code**: The codebase assumes `sizeof(int) == sizeof(void*)` in many
  places. It must be compiled as 32-bit.

## Supported platforms

| Platform | Status |
|----------|--------|
| Windows (32-bit) | Working |
| Linux (x86/x64) | Working |
| macOS | Working |
| Raspberry Pi | Working |
| Android | Working |
| HTML/JS (Emscripten) | Runtime export only |
| PSP, GCW-Zero, Pandora, GP2X | Experimental |

## Building on Windows

### Prerequisites

Install [MSYS2](https://www.msys2.org/), then open an MSYS2 terminal and run:

```bash
pacman -S --needed \
  mingw-w64-i686-gcc \
  mingw-w64-i686-cmake \
  mingw-w64-i686-make \
  mingw-w64-i686-SDL2 \
  mingw-w64-i686-SDL2_mixer \
  mingw-w64-i686-SDL2_net \
  mingw-w64-i686-zlib \
  mingw-w64-i686-pkgconf
```

Add `C:\msys64\mingw32\bin` to your system PATH (use PowerShell, **not** `setx`):

```powershell
[Environment]::SetEnvironmentVariable('Path',
  [Environment]::GetEnvironmentVariable('Path','User') + ';C:\msys64\mingw32\bin',
  'User')
```

### Build

```bash
cd build
cmake -G "MinGW Makefiles" -DTARGETOS=WINDOWS-NATIVE -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev ..
mingw32-make -j%NUMBER_OF_PROCESSORS%
```

This compiles all targets and automatically deploys executables to `div/system/`
and the launcher to `div/`.

### First-time DLL setup

Copy the required DLLs to the runtime directory (only needed once, or when
updating MSYS2):

```bash
# To div/ (for the launcher)
cp /c/msys64/mingw32/bin/{SDL2.dll,libgcc_s_dw2-1.dll,libwinpthread-1.dll} div/

# To div/system/ (for the IDE and runtimes)
cp /c/msys64/mingw32/bin/{SDL2.dll,SDL2_mixer.dll,SDL2_net.dll,zlib1.dll} div/system/
cp /c/msys64/mingw32/bin/{libgcc_s_dw2-1.dll,libwinpthread-1.dll} div/system/
cp /c/msys64/mingw32/bin/{libFLAC.dll,libmpg123-0.dll,libogg-0.dll} div/system/
cp /c/msys64/mingw32/bin/{libopus-0.dll,libopusfile-0.dll,libvorbis-0.dll} div/system/
cp /c/msys64/mingw32/bin/{libvorbisfile-3.dll,libwavpack-1.dll,libxmp.dll} div/system/
```

### Run

```bash
cd div
d-WINDOWS.exe
```

Or run the IDE directly:

```bash
cd div
system\div-WINDOWS.exe INIT
```

### Build outputs

| Binary | Location | Purpose |
|--------|----------|---------|
| `d-WINDOWS.exe` | `div/` | Launcher — starts IDE, chains to debugger |
| `div-WINDOWS.exe` | `div/system/` | The IDE, compiler, and all editors |
| `divrun-WINDOWS.exe` | `div/system/` | DIV2 bytecode runtime |
| `div1run-WINDOWS.exe` | `div/system/` | DIV1 bytecode runtime |
| `divdbg-WINDOWS.exe` | `div/system/` | Runtime with integrated debugger |

## Building on Linux

```bash
mkdir build && cd build
cmake -DTARGETOS=LINUX -DUSE_SDL=2 ..
make -j$(nproc)
```

## History

- **1997** — DIV Games Studio 1 released (Hammer Technologies, Spain)
- **1998** — DIV Games Studio 2 released, with MODE8 3D engine
- **~2014** — Source code released under GNU GPL; Mike Green begins SDL port
- **2015+** — Cross-platform support added (Linux, macOS, Android, RPi, handhelds)
- **2024+** — Ongoing maintenance and modernization

## License

GNU General Public License. See [LICENCE.txt](LICENCE.txt).
