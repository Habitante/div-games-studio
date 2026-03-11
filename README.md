# DIV Games Studio

**DIV Games Studio** was a complete game development environment for MS-DOS, created
in the mid-1990s by Daniel Navarro Medrano at Hammer Technologies in Spain. It shipped
as a self-contained IDE with a built-in code editor, compiler, sprite/map/font editors,
a sound manager, and an 8-bit software-rendered runtime — all written in C and x86
assembly targeting real-mode DOS. It was Daniel's first major C project, coming from a
background in Z80 and 8086 assembly programming — which explains both the ambition of
the architecture and the, shall we say, creative coding conventions throughout.

This repository is the open-source continuation of that codebase, ported from its
original DJGPP/DOS toolchain to modern platforms using SDL2. The SDL port was led by
Mike Green (MikeDX / DX Games). Daniel Navarro has resumed active development,
cleaning up the codebase and working toward a Steam release as a retro game-creation
toolkit.

> The original DIV IDE — code editor, sprite tools, compiler, debugger — running
> natively on modern platforms. Think PICO-8 with a full integrated desktop.

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

~109,000 lines of C across 82 source files and 44 headers. The code dates back to
the mid-90s and carries all the hallmarks of that era: single-letter variable names,
Spanish comments and identifiers, 8-bit palette rendering, `int`-sized pointer
arithmetic, and lots of global state. It was originally built for 16/32-bit DOS with
DJGPP and has been progressively adapted for modern platforms through an OS-dependency
abstraction layer. A significant cleanup (Phase 0) removed ~35K lines of dead code
including MODE8/VPE 3D, the DLL plugin system, CD-ROM support, and other legacy
subsystems.

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
  divbrush.c         Brush/texture thumbnail browser (paint editor)
  global.h           Master header — types, externs, the lot

  runtime/
    i.c              DIV2 bytecode interpreter (the VM)
    s.c              Sprite rendering engine (8-bit blitter)
    f.c              Runtime built-in functions
    divmixer.c       Runtime sound
    divlengu.c       Runtime language/text strings

  runner/
    r.c              Launcher — starts the IDE, chains to debugger

  shared/
    osdep/
      osd_sdl2.c/.h  SDL2 OS-dependency layer (display, input, audio)
      osd_sdl12.c/.h SDL 1.2 OS-dependency layer (legacy)
    run/              Shared runtime support (keyboard, mouse, sound, FLI)
    lib/sdlgfx/       Bundled SDL_gfx framerate limiter
    lib/zip/           Zip archive support

  win/
    osdepwin.c       Windows-specific helpers (fmemopen shim, etc.)

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
- **UI strings and menus**: All IDE text comes from `system/lenguaje.div`, loaded
  into the `texts[]` array at startup. Entries are numbered (`391 "Some text"`).
  Menus are defined as consecutive blocks anchored at a base index — e.g. the Maps
  menu starts at `800`, so `texts[800]` is the tab label, `texts[801]` the title,
  and `texts[802..]` are the menu items in order. `create_menu(800)` reads entries
  sequentially until the next block. To add/remove a menu item, edit `lenguaje.div`
  and the corresponding `case` in the menu's click handler.

## Documentation

The `docs/` directory contains detailed analyses of the codebase — essential reading
for anyone trying to understand this 1990s C codebase. Think of them as Rosetta stones.

| Report | What it covers |
|--------|---------------|
| [architecture-overview.md](docs/architecture-overview.md) | IDE startup → main loop → event processing; OSDEP abstraction layer; FPG/MAP/FNT/PAL binary formats |
| [compiler-pipeline.md](docs/compiler-pipeline.md) | DIV language compiler: lexer → parser → codegen; 127-opcode EML instruction set |
| [vm-and-runtime.md](docs/vm-and-runtime.md) | Stack-based VM interpreter; FRAME-based cooperative process scheduling; 8-bit rendering pipeline |
| [sdl3-migration-report.md](docs/sdl3-migration-report.md) | Future SDL2→SDL3 migration plan: ~450 call sites, effort estimates |

Older reports (glossary, video system audit, unsafe string audit) have been moved to
`docs/archive/` — they were useful during early cleanup phases but are now largely
superseded by the renamed codebase.

See [ROADMAP.md](ROADMAP.md) for the development plan and project vision.

## Supported platforms

| Platform | Status |
|----------|--------|
| Windows (32-bit, MSYS2/MinGW) | Working |
| Linux (x86/x64) | Builds, needs testing |
| macOS | Builds, needs testing |
| Raspberry Pi | Builds, needs testing |

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
  mingw-w64-i686-libjpeg-turbo \
  mingw-w64-i686-pkgconf
```

Add `C:\msys64\mingw32\bin` and `C:\msys64\usr\bin` to your **User** PATH.
Put `mingw32\bin` **first** — before any `mingw64` entry:

```powershell
[Environment]::SetEnvironmentVariable('Path',
  'C:\msys64\mingw32\bin;C:\msys64\usr\bin;' +
  [Environment]::GetEnvironmentVariable('Path','User'),
  'User')
```

> **Important — MINGW64 shell PATH injection:** If you use Git Bash, MSYS2's
> MINGW64 shell, or a VS Code terminal that runs under MINGW64, the shell
> automatically prepends `/mingw64/bin` to `$PATH` on startup — regardless of
> your Windows environment settings. This causes the 32-bit compiler (`cc1.exe`)
> to silently crash because it loads 64-bit DLLs from the wrong directory.
>
> **Do not add `C:\msys64\mingw64\bin` to your PATH.** If your shell still shows
> `/mingw64/bin` first (check with `echo $PATH`), prepend the correct path
> before building:
>
> ```bash
> export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"
> ```
>
> Alternatively, build from a regular Command Prompt or the MSYS2 **MinGW32**
> shell (not MinGW64), which prepends the correct 32-bit paths.

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
cp /c/msys64/mingw32/bin/{SDL2.dll,SDL2_mixer.dll,SDL2_net.dll,zlib1.dll,libjpeg-8.dll} div/system/
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
- **2015+** — Cross-platform support added (Linux, macOS, RPi)
- **2026** — Daniel Navarro resumes active development; major codebase cleanup

## License

GNU General Public License. See [LICENCE.txt](LICENCE.txt).
