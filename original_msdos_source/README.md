# DIV Games Studio - Original MS-DOS Source: Rosetta Stone

Cross-reference between the original MS-DOS source (this directory) and the
current SDL2 port (`src/`). Use this to find any original code when you need
to understand legacy behavior, rescue stripped features, or read Daniel's
original implementation.

**Compiler:** Watcom C++ 32-bit (DOS/4GW protected mode)
**Build system:** `386.mak` / `586.mak` (CPU-optimized targets)
**Output:** `D.EXE` (IDE, ~1.4MB), `DIV32RUN` (standalone runtime)

For the Spanish-English identifier glossary, see `docs/archive/glossary-spanish-english.md`.

---

## 1. File Mapping: Original DOS -> Current SDL2

### IDE Shell & Main Loop

| Original (root) | Lines | Current (`src/ide/`) | Notes |
|---|---|---|---|
| `DIV.CPP` | 3916 | `main.c` + `main_desktop.c` + `main_dialogs.c` | Main IDE shell, `entorno()` -> `main_loop()` |
| `DIVDSKTP.CPP` | 1047 | `desktop.c` | Desktop window manager |
| `DIVHANDL.CPP` | 3633 | `handler.c` + `handler_dialogs.c` + `handler_map.c` + `handler_fonts.c` | Menu handlers. `menu_principal()` -> `menu_main()` |
| `DIVBROW.CPP` | 2017 | `browser.c` | File/thumbnail browser |
| `DIVWINDO.CPP` | 539 | `window.c` | UI primitives: `boton()` -> `draw_button()` |
| `DIVMOUSE.CPP` | 208 | `mouse.c` | Mouse input handler |
| `DIVKEYBO.CPP` | 196 | `keyboard.c` | Keyboard input (was IRQ 0x09 handler) |
| `DIVHELP.CPP` | 1443 | `help.c` | Hypertext help system |
| `DIVSETUP.CPP` | 1074 | `setup.c` | Configuration dialog |
| `DIVLENGU.CPP` | 141 | `language.c` | Language/text loading (lenguaje.div) |
| `DIVEFFEC.CPP` | 532 | `effects.c` | Visual transitions/fades |
| `DIVGAMA.CPP` | 279 | `gamma.c` | Gamma correction |
| `DIVINSTA.CPP` | 1338 | `installer.c` | Game packager/installer creator |
| `GRABADOR.CPP` | (small) | `recorder.c` | Screen recording/replay |
| `DIVBASIC.CPP` | 1130 | (split across several files) | Drawing primitives: `writetxt`, `box`, `rectangulo` |

### Code Editor

| Original (root) | Lines | Current (`src/editor/`) | Notes |
|---|---|---|---|
| `DIVEDIT.CPP` | 3208 | `editor.c` + `editor_edit.c` + `editor_render.c` + `editor_file.c` | Full code editor. Split in Phase 2C-3 |

### Graphics Editors

| Original (root) | Lines | Current (`src/editor/`) | Notes |
|---|---|---|---|
| `DIVPAINT.CPP` | 5084 | `paint.c` + `paint_tools.c` + `paint_select.c` | Paint editor. Split in Phase 2C-3 |
| `DIVFPG.CPP` | 1942 | `fpg.c` (editor) | FPG sprite library editor |
| `DIVFONT.CPP` | 1871 | `font.c` | Font editor |
| `DIVPALET.CPP` | 1855 | `palette.c` | Palette editor |
| `DIVCOLOR.CPP` | 302 | (merged into palette/global) | Color palette/DAC operations |
| `DIVSPRIT.CPP` | 495 | `src/ide/sprite.c` | Sprite selector (IDE side) |
| `DIVSPR.CPP` | 1183 | `src/ide/graphics.c` | Graphics library browser |

### Sound

| Original (root) | Lines | Current | Notes |
|---|---|---|---|
| `DIVPCM.CPP` | 2012 | `src/editor/pcm.c` | PCM sample editor |
| `DIVSOUND.CPP` | 106 | `src/ide/sound.c` | Sound browser |
| `DIVMIXER.CPP` | 250 | `src/ide/mixer.c` + `mixer_ui.c` | Audio mixer UI |
| `DIVMIX.CPP` | 161 | (merged into mixer) | Mixer helper |

### Formats / File I/O

| Original (root) | Lines | Current (`src/formats/`) | Notes |
|---|---|---|---|
| `DIVFORMA.CPP` | 1516 | `image.c` | BMP/PCX/JPG loaders |
| `FPGFILE.CPP` | (small) | `fpg.c` (formats) | FPG file format read/write |

### Compiler

| Original (root) | Lines | Current (`src/compiler/`) | Notes |
|---|---|---|---|
| `DIVC.CPP` | 7688 | `compiler.c` + `compiler_parser.c` + `compiler_expression.c` | The entire compiler. Split in Phase 2C-3 |
| `DIVCALC.CPP` | (small) | `calc.c` | Calculator / constant evaluator |
| `DIVFRM.CPP` | 6880 | **STRIPPED** | Code auto-formatter / pretty-printer |

### Runtime (in `SRC/SOURCE/`)

| Original (`SRC/SOURCE/`) | Lines | Current (`src/runtime/`) | Notes |
|---|---|---|---|
| `I.CPP` | 1498 | `interpreter.c` | VM interpreter: `interprete()` -> `interpreter()` |
| `F.CPP` | 4424 | `functions.c` | Built-in DIV functions |
| `S.CPP` | 2143 | `render.c` | Sprite/text/scroll rendering |
| `D.CPP` | 4758 | `debug/debugger.c` + `debugger_ui.c` + `debugger_inspect.c` + `debugger_code.c` + `debugger_proclist.c` + `debugger_profile.c` | Debugger. Split into 6 files in Phase 2C-3 |
| `KERNEL.CPP` | 699 | `debug/kernel.inc` | VM core loop (bytecode switch) |
| `C.CPP` | 903 | `src/shared/run/collision.c` | Collision detection |
| `V.CPP` | 842 | `src/shared/run/video.c` | Video mode switching |
| `IA.CPP` | 569 | `src/shared/run/pathfind.c` | Pathfinding ("IA" = AI) |
| `MOUSE.CPP` | 104 | `src/shared/run/mouse.c` | Runtime mouse input |
| `JOY.CPP` | 97 | (merged/removed) | Joystick input |
| `DIVFLI.CPP` | 104 | `src/runtime/fli/flxplay.c` | FLI/FLC animation player |
| `INTER.H` | (header) | `src/runtime/inter.h` | VM globals, process struct, opcodes |
| `DIVKEYBO.CPP` | (runtime copy) | `src/shared/run/keyboard.c` | Runtime keyboard handler |
| `DIVSOUND.CPP` | (runtime copy) | `src/shared/run/sound.c` | Runtime sound playback |
| `DIVMIXER.CPP` | (runtime copy) | `src/runtime/mixer.c` | Runtime audio mixer |
| `DIVLENGU.CPP` | (runtime copy) | `src/runtime/language.c` | Runtime text loading |
| `DIVTIMER.CPP` | (runtime copy) | (replaced by SDL2 timing) | PIT timer interrupt handler |
| `DET_VESA.CPP` | (runtime copy) | `src/runtime/vesa.c` | VESA mode detection |

### Headers

| Original | Current | Notes |
|---|---|---|
| `GLOBAL.H` | `src/global.h` | All globals, types, macros (~154 globals) |
| `DIV.H` | `src/ide.h` | Process struct, header constants |
| `DIV_STUB.H` | `src/div_stub.h` | Build stub declarations |
| `DIVDLL.H` | (in compiler) | DLL export/import structures |
| `DIVSOUND.H` | `src/shared/divsound.h` | Sound structures (pcminfo) |
| `DIVKEYBO.H` | `src/keyboard.h` | Scancode definitions |
| `DIVMIXER.HPP` | `src/mixer.h` | Mixer hardware definitions |
| `DIVMAP3D.HPP` | (stripped) | 3D map structures |
| `FPGFILE.HPP` | `src/fpg.h` | FPG file format structures |
| `IFS.H` | `src/charset.h` | Font import format structures |
| `GRABADOR.H` | `src/recorder.h` | Screen recorder API |
| `SVGA.H` | (stripped) | SVGA hardware register defs |
| `VISOR.HPP` | (stripped) | 3D viewer structures |
| `ZLIB.H` | (system zlib) | Compression library header |
| `INTER.H` | `src/runtime/inter.h` | VM state, process struct, opcodes |
| `SYSDAC.H` | `src/sysdac.h` | Sound DAC/legacy header |
| `TFLIB.H` | `src/runtime/tflib.h` | Sprite transformation data |
| `TFANIMAT.H` | `src/runtime/tfanimat.h` | Animation state |
| `TOPFLC.H` | `src/shared/run/topflc.h` | FLC format definitions |

### Assembly

| Original | Current | Notes |
|---|---|---|
| `DIVASM.ASM` | (removed) | x86 helpers: `memcpyb`, `call_`, `comp_`/`comp_exit_` |
| `TIMER.ASM` | (replaced by SDL2) | PIT timer interrupt vector |
| `VESA.ASM` | (replaced by SDL2) | VESA video mode setting |

### DOS-specific support

| Original | Current | Notes |
|---|---|---|
| `DIVVIDEO.CPP` | (replaced by `shared/osdep/osd_sdl2.c`) | Direct VGA/VESA register manipulation |
| `DIVTIMER.CPP` | (replaced by SDL2 timing) | PIT clock / interrupt setup |
| `MEM.CPP` | (removed) | DOS DPMI memory allocation |
| `DIVASM.CPP` | (removed) | Inline assembly wrapper |
| `DIVBIN.CPP` | (removed) | Binary file type sniffer |
| `CDROM.CPP` | (removed) | CD-ROM audio playback |
| `DIVCDROM.CPP` | (removed) | CD-ROM support wrapper |
| `DIVSB.CPP` | (removed) | SoundBlaster direct hardware |

---

## 2. Stripped Code (not in current `src/`)

These features existed in the original DOS source but are **not present** in
the current SDL2 port. They are preserved here for potential future rescue.

### Major stripped features

| Original file(s) | Lines | What it was | Why stripped | Rescue notes |
|---|---|---|---|---|
| `DIVMAP3D.CPP` + `DIVMAP3D.HPP` | 4164 | **3D map/level editor** — sector/wall/region editor with texture application, lighting, grid-based 3D scene construction | Not ported to SDL2 | Core 3D editor logic is platform-independent; could be rescued |
| `DIVFRM.CPP` | 6880 | **Code auto-formatter** — pretty-printer for DIV source code | Not ported | Purely text-processing; easy to rescue |
| `VISOR.HPP` + `visor/` tree | large | **3D sprite viewer** (SPRITE3D) — standalone 3D model viewer/renderer | Separate tool, never integrated into SDL2 port | Has its own duplicate copy of runtime source |
| `SRC/VPE/` (38 files) | large | **VPE: 3D rendering engine** — software rasterizer with fixed-point math, texture mapping, BSP/zone rendering, GFX pipeline | Core 3D engine for Mode 8 | Complex but self-contained; key files: `VPE.CPP`, `SCAN.CPP`, `VIEW.CPP`, `GFX.CPP`, `ZONE.CPP` |
| `SRC/SOURCE/JOY.CPP` | 97 | **Joystick input** | DOS-specific (direct port I/O) | Trivial; SDL2 joystick API is better |

### DOS hardware code (replaced by SDL2 OSDEP layer)

| Original file(s) | What it was | SDL2 replacement |
|---|---|---|
| `DIVVIDEO.CPP` (451 lines) | VGA/VESA register manipulation, Mode X setup | `shared/osdep/osd_sdl2.c` |
| `DIVTIMER.CPP` (79 lines) | PIT 8253 timer interrupt handler | SDL2 timing (`SDL_GetTicks`) |
| `DIVASM.ASM` + `DIVASM.CPP` | x86 asm: `memcpyb`, interpreter hooks | Standard C / compiler intrinsics |
| `TIMER.ASM` + `VESA.ASM` | Hardware interrupt vectors, VESA BIOS calls | SDL2 |
| `MEM.CPP` | DPMI real-mode memory allocation | Standard `malloc()` |
| `DIVBIN.CPP` | DOS file type detection | Not needed |
| `CDROM.CPP` + `CDROM.H` + `DIVCDROM.CPP` + `DIVCDROM.H` | CD-ROM audio via DOS interrupts | Could use SDL2_mixer for CD audio |
| `DIVSB.CPP` + `DIVSB.H` | SoundBlaster direct hardware access | SDL2_mixer |
| `DET_VESA.CPP` (IDE copy) | VESA mode enumeration | SDL2 display modes |

### Build/toolchain (not applicable to SDL2)

| Directory/file | What it was |
|---|---|
| `DIV_STUB/` | DOS extender stub (PMODE/W loader) — `CSTRT086.ASM`, `DIV_STUB.CPP` |
| `WSTUB/` | Alternative DOS stub: `WSTUB.C`, `cpuid.asm` |
| `PMODE/` | PMODE/W DOS extender distribution |
| `Watcom/` | Watcom C++ linker configs |
| `INC/` | Shared includes: `PMODE.H`, `SVGA.H`, `VESA.H`, `DEBUG.H`, `ZTIMER.H` |
| `*.mak`, `*.lnk`, `C.BAT`, `ASM.BAT` | Watcom build scripts |
| `2ACABADA.HPP` | Build marker file ("Finished") |

### Satellite tools and data

| Directory | What it was |
|---|---|
| `convert/` | Format converters: `pcx2map.c`, `pcm2wav.c`, `fix.c` |
| `dc/` | `DC.CPP` — unknown tool (possibly data compiler) |
| `sprite3d/` | Standalone 3D sprite animation tool (ANIMATED, COMPLEX, HLRENDER, etc.) |
| `DLL/SOURCE/` | DLL SDK examples: `AGUA.CPP` (water effect), `HBOY.CPP`, `SS1.CPP` + demo programs |
| `ins_div/` | DIV installer creator: `I.CPP`, `DIVLENGU.CPP`, `DIVKEYBO.CPP` |
| `ins_jue/` | Game installer ("juego" = game) |
| `ins_win/` | Windows installer variant |
| `install/` | Installation scripts |
| `del_div/` | DIV uninstaller |
| `mpatch/` | Patch tool |
| `setup/` | Setup configuration |

### Third-party libraries (originals)

| Directory | What it was | SDL2 replacement |
|---|---|---|
| `SRC/JUDAS/` (31 files) | JUDAS sound library (SB, GUS, mixing) | SDL2_mixer |
| `SRC/NETLIB/` (19 files) | IPX/serial network library: `COMM.C`, `NET.C`, `IPXLIB.C`, `RED.CPP` | SDL2_net |
| `SRC/VPE/` (38 files) | VPE 3D engine (see above) | Partially in runtime Mode 7/8 |
| `JPEGLIB/` | libjpeg headers | System libjpeg-turbo |

---

## 3. Key Function Name Changes

The original used Spanish names and PascalCase. All were renamed in Phase 2C.
For the full glossary, see `docs/archive/glossary-spanish-english.md`.

### Most important mappings

| Original (DOS) | Current (SDL2) | What it does |
|---|---|---|
| `entorno()` | `main_loop()` | IDE main loop |
| `compilar()` | `compile()` | Compiler entry point |
| `lexico()` | `lexer()` | Tokenizer |
| `sintactico()` | `parser()` | Parser |
| `sentencia()` | `statement()` | Statement parser |
| `expresion()` | `expression()` | Expression parser |
| `interprete()` | `interpreter()` | VM interpreter |
| `nucleo_exec()` | `core_exec()` | VM core execution loop |
| `volcado()` | `blit_screen()` | Blit framebuffer to display |
| `boton()` | `draw_button()` | Draw UI button |
| `crear_menu()` | `create_menu()` | Create menu |
| `pinta_sprite()` | `paint_sprite()` | Render a process sprite |
| `pinta_textos()` | `paint_texts()` | Render text objects |
| `elimina_proceso()` | `kill_process()` | Kill/remove a process |
| `comprobar_colisiones()` | `check_collisions()` | Collision detection |
| `restaura()` | `restore()` | Restore background |
| `menu_principal()` | `menu_main()` | Main menu bar |
| `find_color()` | `find_color()` | Find nearest palette color |
| `calcula_regla()` | (unchanged logic) | Calculate color gradient |
| `guardar_prg()` | `save_program()` | Save source file |
| `abrir_programa()` | `open_program()` | Open source file |

### Key variable renames

| Original | Current | Meaning |
|---|---|---|
| `paleta` | `palette` | 768-byte palette buffer |
| `pila[]` | `stack[]` | VM execution stack |
| `pieza` | `current_token` | Current lexer token |
| `bloque_actual` | `current_scope` | Scope being compiled |
| `reloj` | `frame_clock` | Frame timing clock |
| `ventana[]` | `window[]` | Window array |
| `texto[]` | `texts[]` | Localized UI strings |
| `volcado_completo` | `full_redraw` | Full redraw flag |
| `vga_an` / `vga_al` | `vga_width` / `vga_height` | Screen dimensions |
| `tapiz` | `wallpaper` | Desktop background |
| `tipo[]` | `file_types[]` | File type definitions |
| `mascara[]` | `file_filter[]` | File filter mask |
| `arrastrar` | (drag state) | Window dragging state |

---

## 4. Directory Layout: Original DOS

```
original_msdos_source/
|
|-- *.CPP, *.H            IDE source (65+ files, compiled into D.EXE)
|-- SRC/
|   |-- SOURCE/            Runtime source (compiled into DIV32RUN)
|   |   |-- I.CPP          Interpreter (-> runtime/interpreter.c)
|   |   |-- F.CPP          Built-in functions (-> runtime/functions.c)
|   |   |-- S.CPP          Sprite/scroll render (-> runtime/render.c)
|   |   |-- D.CPP          Debugger (-> runtime/debug/debugger*.c)
|   |   |-- C.CPP          Collision (-> shared/run/collision.c)
|   |   |-- V.CPP          Video (-> shared/run/video.c)
|   |   |-- IA.CPP         Pathfinding (-> shared/run/pathfind.c)
|   |   |-- KERNEL.CPP     VM bytecode switch (-> runtime/debug/kernel.inc)
|   |   |-- MOUSE.CPP      Runtime mouse (-> shared/run/mouse.c)
|   |   |-- JOY.CPP        Joystick (stripped)
|   |   |-- DIVFLI.CPP     FLI player (-> runtime/fli/flxplay.c)
|   |   |-- INTER.H        VM header (-> runtime/inter.h)
|   |   +-- (+ runtime copies of DIVKEYBO, DIVSOUND, DIVMIXER, DIVLENGU)
|   |
|   |-- VPE/               3D rendering engine (38 files, stripped)
|   |-- JUDAS/             Sound library (31 files, replaced by SDL2_mixer)
|   |-- NETLIB/            Network library (19 files, replaced by SDL2_net)
|   +-- INC/               Shared platform headers
|
|-- DIV_STUB/              DOS extender stub
|-- WSTUB/                 Alternative stub
|-- PMODE/                 PMODE/W extender
|-- Watcom/                Compiler configs
|-- JPEGLIB/               JPEG library headers
|
|-- convert/               Conversion tools (pcx2map, pcm2wav, fix)
|-- dc/                    Data compiler tool
|-- sprite3d/              3D sprite animation tool
|-- DLL/SOURCE/            DLL SDK + examples (AGUA, HBOY, SS1)
|-- ins_div/               DIV installer source
|-- ins_jue/               Game installer
|-- ins_win/               Windows installer
|-- install/               Installation scripts
|-- del_div/               Uninstaller
|-- mpatch/                Patch tool
|-- setup/                 Setup tool
|-- visor/                 3D viewer (SPRITE3D)
|
|-- DAT/                   Example game data
|-- FPG/, FNT/, PAL/       Graphics/font/palette assets
|-- MAP/, Wld/             Map/3D world data
|-- PCM/, Mod/             Sound/music samples
|-- FLI/                   FLI animations
|-- PRG/                   Example DIV programs
|-- help/                  Help system data
+-- system/                IDE system files
```

---

## 5. How to Find Code

**"I need to understand how X worked originally"**

1. Find the current SDL2 file in `src/` that implements X
2. Look up the mapping in Section 1 above to find the original DOS file
3. Open the original file and search for the function name (use the glossary
   for Spanish-to-English name mapping)

**"I need to rescue a stripped feature"**

1. Check Section 2 for the stripped feature and its original file
2. The original code is self-contained in the listed files
3. Key rescue candidates: `DIVFRM.CPP` (formatter), `DIVMAP3D.CPP` (3D editor),
   `SRC/VPE/` (3D engine)

**"I'm reading git history and see a Spanish name I don't recognize"**

1. Check `docs/archive/glossary-spanish-english.md` for the full translation table
2. Section 3 above covers the most common function/variable renames
