# DIV Games Studio — Roadmap

No deadlines, no pressure. Just a living document for when the mood strikes.

The goal: ship DIV on Steam as a polished retro game-creation toolkit. Start by
leaning into the classic constraints (256 colors, integer math, cooperative processes)
as features, not bugs — think PICO-8, not Godot. The integrated IDE is the killer
differentiator no fork ever replicated.

But keep the door open. True color rendering, float types, and language modernization
are real aspirations — they just come after shipping something people can use today.
Clean up first, ship second, then modernize based on what real users actually want.

---

## Phase 0 — Archaeology & Dead Code Removal ✓

**Completed 2026-03-06.** 118 files changed, 145 insertions, ~41,000 deletions.

### What was removed
- [x] CD-ROM/CDDA code: `cdrom.c`, `divcdrom.c`, `runtime/cdrom.c` (clock widget preserved → `divclock.c`)
- [x] MODE8/VPE 3D engine: `runtime/vpe/` (38 files deleted)
- [x] Visor 3D sprite generator: `visor/` (21 files deleted, `divspr.c` → stub)
- [x] DLL plugin system: all `DIVDLL` infrastructure removed from divc.c, runtime, div1run
- [x] `div1run/unused/` directory (3 files)
- [x] `other/` directory (7 files)
- [x] Dead `#ifdef` branches: GCW, PSP, Pandora, GP2X, Amiga, Atari ST, PS2
- [x] Dead cmake toolchain files (7 deleted, kept: windows-native, linux, osx, defaults)
- [x] `shared/lib/portrend/` (3 files), `shared/mikedll.c`, `shared/divdll.h`
- [x] Network code: NETLIB/NETPLAY blocks, `net.c`, `net.h`, `netlib.h`
- [x] Fixed CMakeLists.txt `exec_program("git")` → `execute_process()` with fallback

### Additional cleanup (done 2026-03-07)
- [x] Cleaned up bundled sdlgfx — removed unused files, kept only `SDL_framerate.c/.h`
- [x] Dropped DIV1 runtime (`div1run/`, ~3,000 lines) — orphaned, never called from IDE or launcher
- [x] `.gitignore` already in place since initial commit
- [x] Hidden from menus: CD Player (CDDA code removed), DOS Shell (no DOS), disabled Alt+S
- Clock, Trash icon, Calculator: keep — still functional
- [x] Removed 3D Map Editor: `divmap3d.c` (4,198 lines), `divmap3d.hpp`, `div/WLD/` data,
  all menu entries, help docs, help.idx entries, and cross-references
- [x] Removed MODE8 language remnants: 14 function stubs + struct _m8 + M8_* locals from runtime
- [x] Removed CD music language remnants: play_cd/stop_cd/is_playing_cd stubs from runtime
- [x] Extracted brush/texture browser from divmap3d.c → new `divbrush.c` (paint editor dependency)
- [x] Removed Sprite Generator: `divspr.c` stub, menu entry, `div/GENSPR/` data directory, `generador_sprites()` declaration

### Line ending normalization (done 2026-03-07)
- [x] Fixed `divlengu.c` (IDE + runtime) to accept both CRLF and LF-only files
- [x] Fixed `divhelp.c` `tabula_help()`/`tabula_help2()` for CRLF and LF-only
- [x] Replaced broken `ú`/char-250 forced line break in help with `{br}` tag
- [x] Added `.gitattributes` with `* text=auto eol=lf`
- [x] Converted `lenguaje.div`, `lenguaje.int`, `help.div`, `help.idx` to LF
- [x] Converted `lenguaje.int` from Latin-1 to UTF-8

### SDL2 port bug fixes (done 2026-03-07)
- [x] Fixed Shift+letter in editor: `SDL_TEXTINPUT` was setting `scan_code` to ASCII
  value, colliding with DOS scan codes ('M'=77=Right, 'P'=80=Down, 'Q'=81=PgDn)
- [x] Fixed mouse wheel runaway scroll: wheel bits persisted across frames when no
  SDL events were queued, causing continuous scrolling until another event arrived
- [x] Restored `.PRG` extension check for syntax colorizer (was commented out)
- [x] Fixed `f_cortar_bloque(memptrsize)` back to `f_cortar_bloque(int)`
- [x] Removed 3 dead `#ifdef TTF` blocks from `divedit.c`

### Still pending from Phase 0
- [x] Remove hundreds of commented-out code blocks throughout
  (52 files audited, ~1,500 lines removed, 28 review items resolved)
- [x] Remove remaining `#ifdef TTF` dead code (~25 blocks across div.c, divwindo.c,
  divpalet.c, divsetup.c, divhelp.c, divvideo.c, global.h, osdep.h)

### Build system cleanup ✓
- [x] Remove dead cmake options and platform branches
  (removed SDL1 branch, libgit2, SDL_image, DEMOTARGET, APPLE block, dead JUDAS refs;
   deleted 4 dead toolchains: windows.cmake, linux.cmake, osx.cmake, pi.cmake;
   deleted 6 dead scripts: app.sh, makedll.sh, makedroid.bat, makegcw.bat, makehtml.bat, makepnd.bat;
   CMakeLists.txt reduced from 376 to 161 lines)
- [x] Add a DLL auto-copy step so first-time build "just works"
  (`tools/copy_dlls.sh` — uses objdump to recursively find and copy mingw DLLs; runs as post-build step)
- [x] Add a one-command build script wrapper
  (`build.sh` at project root — sets PATH, runs cmake + make; supports `--clean`)

---

## Phase 0.5 — Fix File Encoding (Latin-1 → UTF-8) ✓

**Completed 2026-03-06.** 53 files converted, all string literals hex-escaped.

The codebase was Latin-1 encoded (from the MS-DOS era). Modern editors and tools
silently corrupted files by converting them to UTF-8, breaking character lookup
tables used by the compiler's lexical analyzer.

### What was done
- [x] Replaced all high bytes in string/char literals with `\xNN` hex escapes
  - 4 character lookup tables in `divc.c` (used by lexer for accented identifiers)
  - 1 corrupted `lower[256]` table in `div.c` (was already broken in git HEAD)
  - 39 Spanish error message strings in `div1run/inter.h` (div1run since removed)
  - 127 font bitmap data lines in `SDL_gfxPrimitives_font.h` (sdlgfx since cleaned up)
  - Display strings in `divfont.c`, `divhandl.c`, `divhelp.c`, `divpaint.c`, `runtime/debug/d.c`
- [x] Converted all 53 Latin-1 source files to UTF-8
- [x] Verified: build succeeds, F11 compile works, runtime works, debug works
- 7 "binary" headers remain (font/animation embedded data) — not editable text

---

## Phase 1 — Stabilization & Warnings

Make the compiler tell us what's actually broken. Fix the scariest stuff.

### Enable warnings ✓ (done 2026-03-09)
- [x] Replace `-w` (suppress all warnings) with `-Wall -Wextra` in CMakeLists.txt
  - Original baseline: 1,745 warnings → after Phase 0 dead code removal: 342
  - Suppressed low-value: `-Wno-shadow -Wno-unused-parameter -Wno-sign-compare
    -Wno-missing-field-initializers -Wno-char-subscripts`
  - Third-party files (zip.c, SDL_framerate.c) suppressed with per-file `-w`
- [x] Fix all CRITICAL/HIGH warnings: use-after-free, -Wrestrict (52 sites),
      missing returns, abs-on-unsigned (14), pointer casts (50+), maybe-uninitialized (128),
      empty-body (19), int-in-bool-context (3)
- [x] Fix pointer/int cast warnings — replaced `(memptrsize)` with `(uintptr_t)`/`(intptr_t)`
- [x] Remove `-fpermissive` (was a no-op in C mode anyway)
- [x] Fix all 626 LOW-severity warnings (Sprint A, 2026-03-10): misleading-indentation (260),
  parentheses (144), dangling-else (80), pointer-sign (51), format strings (28),
  missing-braces (18), implicit-fallthrough (11), unknown-pragmas (7), misc (27).
  Added braces, parentheses, casts, `/* fall through */` comments across 35 files.
  Zero behavioral changes. **Build is now fully warning-clean.**
- Original baseline (2026-03-08): 1,745 warnings with `-Wall -Wextra -Wshadow`; all CRITICAL/HIGH fixed.

### Critical bugs fixed (found via `-Wall -Wextra` audit, 2026-03-08)
- [x] `divc.c:1854`: `!ivnom.b[0]!='.'` operator precedence bug — `#include` path
      "skip dot-files" check was always true. Present since original code.
- [x] `divforma.c:1403`: use-after-free — BMP palette loader read from freed
      `CopiaBuffer`. Moved `free()` to after palette memcpy in each branch.
- [x] Missing return statements: `divc.c` div_open_file_mode(), `div.c` GetHeapFree()/
      GetMemoryFree(), `divpack.c` pack(), `divsound.c` GetSongPos()/GetSongLine(),
      `divkeybo.c` GetIRQVector(). Added appropriate fallback returns.
- [x] `div.c` determina_unidades(): void function had `return -1`/`return 0` — fixed.

### Fix known landmines
- [x] `divkeybo.c`: DOS BIOS pointers (`0x417`, `0x41a`, `0x41c`) guarded with `#ifdef DOS`
- [x] `div.c:2949`: `red_panel.png` already fixed to relative path; `v.c` recording path fixed too
- [x] Audit unsafe `sprintf`/`strcpy` calls — see `reports/unsafe-string-audit.md`.
      Created `src/div_string.h` with safe helpers (div_strcpy, div_strcat, div_snprintf,
      IS_PATH_SEP). Fixed 38 high-risk `strcpy(dest, input)` buffer overflow sites across
      12 files, 2 overlapping sprintf (UB), and 15 path separator sites (only checked `/`).
      ~690 remaining strcpy are low-risk internal string copies.
- [ ] Audit the `PrintEvent` pattern for similar `#ifdef`-body bugs (divmouse.c:506 was one)
- [x] Fix window close button — was already working via `SDL_QUIT` → `salir_del_entorno`.
      Fixed inner loops (dialog, paint color/mask pickers) that blocked exit until dismissed.
- [x] Fix focus loss handling: runtime pauses game + audio on alt-tab/minimize,
      resumes on focus regain. IDE unaffected (editor doesn't need to pause).
- [x] Fix mouse-outside-window: IDE and runtime now report `mouse_x=mouse_y=-1` when
      mouse leaves the window, so UI hover/hit tests fail naturally instead of freezing
      at the last edge position.
- [x] Fix spacebar-as-click in paint editor: magic value `0xfffd` had mouse wheel bits
      set (bits 2-3), causing SDL port's `select_zoom()` to misinterpret spacebar as
      wheel-up → zoom-out. Changed to `0x8001` (bit 0 = left click, bit 15 = spacebar marker).
- [x] Fix `select_zoom()` crash: SDL port added mouse wheel zoom with `(zoom-1)%4` which
      produces -1 when zoom=0 (C modulo preserves sign), then `1 << -1` = undefined behavior.
      Fixed with explicit bounds checking and separated wheel/Z-key/icon into distinct branches.
- [ ] Fix or remove the commented-out `free()` in runtime stack management (`i.c:778`)

### Fix video mode / display system
The entire video mode and fullscreen system is broken in the SDL2 port.
In the original DOS version, everything was fullscreen (no choice), and the
IDE listed available VGA/VESA modes. The SDL2 port made everything windowed
but left the underlying infrastructure broken:

- [x] `OSDEP_IsFullScreen()` — was destroying the window + calling SDL_Quit().
      Replaced with `SDL_GetWindowFlags()` query. Alt+Enter and shutdown no longer crash.
- [x] `OSDEP_ListModes()` / `detectar_vesa()` — fallback had 8 modes but `num_modos`
      stayed 0. Fixed: `num_modos=8`, video mode dialog now shows entries.
- [x] `OSDEP_SetVideoMode()` — now honors the `fs` parameter. Uses
      `SDL_WINDOW_FULLSCREEN_DESKTOP` for fullscreen, `SDL_WINDOW_RESIZABLE` for windowed.
      Added `SDL_RenderSetLogicalSize()` + `SDL_RenderSetIntegerScale()` for proper
      pixel-art scaling with letterboxing at any window size.
- [x] `OSDEP_SetCaption()` — now calls `SDL_SetWindowTitle()` (was storing but never applying).
- [x] `OSDEP_SetPalette()` — now uses actual `firstcolor`/`ncolors` params (was hardcoded 0,256).
- [x] Texture changed from STATIC to STREAMING (`SDL_TEXTUREACCESS_STREAMING`) for
      efficient per-frame updates.
- [x] Removed dead `divWindow`/`divRender`/`divTexture` globals (v.c, divvideo.c).
- [x] Removed dead code in runtime `volcadosdl()` (NULL pointer SDL calls).
- [x] Removed dead Mode X planar copy and manual 32/24/16-bit pixel conversion in
      IDE `volcadosdl()` — surface is always 8-bit, OSDEP_Flip handles conversion.
- [x] Removed dead `nothing()` function from divvideo.c.
- [x] Alt+Enter fullscreen toggle now works in both IDE and runtime.
- See [`reports/video-system-audit.md`](reports/video-system-audit.md) for the full display system analysis.
- [ ] `test_video` startup dialog disabled (item 9) — could be re-enabled now.
- [ ] DPI-aware rendering (`SDL_WINDOW_ALLOW_HIGHDPI`) — future improvement.

### Normalize basics
- [x] Remove SDL1 / Emscripten preprocessor branches (30 `#ifdef SDL2` / `#ifndef SDL2` /
      `#ifdef SDL` / `__EMSCRIPTEN__` blocks removed across 11 files; deleted `osd_sdl12.c`,
      `osd_sdl12.h`, `osd_sdl.h`; merged declarations into `osd_sdl2.h`; removed `-DSDL2=2`
      from CMakeLists.txt)
- [ ] Unify byte types: pick `uint8_t` everywhere, stop mixing `byte`/`char`/`uchar`
- [x] Normalize path separators — 15 sites fixed via `IS_PATH_SEP()` macro (done 2026-03-09)

---

## Phase 2 — Code Cleanup & Readability

One file at a time, make the code understandable to someone who isn't Daniel.
No behavioral changes — pure cleanup.

### Architecture docs ✓ (initial docs done 2026-03-09)
- [x] Document the call graph from IDE startup → main loop → event processing
      → [`reports/architecture-overview.md`](reports/architecture-overview.md)
- [x] Document the compiler pipeline: lexer → parser → codegen → bytecode format
      → [`reports/compiler-pipeline.md`](reports/compiler-pipeline.md) (127-opcode EML instruction set documented)
- [x] Document the VM interpreter loop and process scheduling algorithm
      → [`reports/vm-and-runtime.md`](reports/vm-and-runtime.md) (stack VM, FRAME-based cooperative multitasking)
- [x] Document the rendering pipeline: 8-bit surface → palette blit → SDL2 texture
      → [`reports/vm-and-runtime.md`](reports/vm-and-runtime.md) (rendering section)
- [x] Document the FPG/MAP/FNT/PAL binary file formats
      → [`reports/architecture-overview.md`](reports/architecture-overview.md) (file formats section)
- [x] Document the OSDEP abstraction layer contract
      → [`reports/architecture-overview.md`](reports/architecture-overview.md) (OSDEP section)
- [x] Catalogue all 100+ globals in global.h with their roles
      → [`reports/glossary-spanish-english.md`](reports/glossary-spanish-english.md) (~150 identifiers translated)
- [ ] Review docs for accuracy (Daniel is the authority on compiler/VM internals)

### Naming — initial pass
- [ ] Rename the worst single-letter globals (the `r,g,b,c,d,a` "generic counters"
      in global.h:519 that even the original code says "OJO! Quitarlos")
- [ ] Rename cryptic locals in the hottest paths (divc.c, divedit.c, runtime/i.c)
- [ ] Add English comments to the top 20 most cryptic functions
- [ ] Document the meaning of `v.an`/`v.al` (width/height), `tapiz` (wallpaper),
      `papelera` (clipboard), etc. in a glossary or inline

### Structural improvements
- [ ] Consider splitting `divc.c` (9,346 lines) into lexer/parser/codegen modules
- [ ] Consider splitting `div.c` (4,678 lines) into menu handling vs desktop management
- [ ] Group related globals into context structs where it simplifies things
- [ ] Normalize string handling patterns across the codebase

### Systematic modernization (agent-assisted sprints)

The long-term vision for Phase 2 goes far beyond renaming a few globals. The goal is
to make this codebase genuinely maintainable — by anyone, not just Daniel. This means:

1. **Full English translation** — every Spanish identifier, comment, and variable name
   gets an English equivalent. The glossary (`reports/glossary-spanish-english.md`) is
   the Rosetta stone; the architecture docs provide the structural understanding needed
   to rename safely.

2. **Meaningful names everywhere** — not just replacing `tapiz` with `wallpaper`, but
   turning `a`, `b`, `c`, `n`, `nn`, `nnn` into names that reveal intent. This requires
   understanding what each variable actually does, file by file.

3. **English comments on every non-obvious function** — not boilerplate docstrings, but
   the kind of "here's what this does and why" commentary that lets a new developer
   navigate 100K lines of C without having to reverse-engineer each function.

4. **Structural decomposition** — the monster files (divc.c, div.c, divpaint.c) should
   be split along natural boundaries that the architecture docs have already identified.

**Methodology:** This work is ideal for AI agent teams working in focused sprints —
one file or subsystem at a time, using the architecture docs and glossary as context.
Each sprint produces a self-contained PR that can be reviewed and merged independently.
The reports/ directory exists precisely to give agents (and future contributors) the
context they need to make correct changes without Daniel having to explain everything
from scratch each time.

**Constraints:** Every rename must be project-wide (no half-renamed identifiers). Every
change must compile and pass the same smoke tests. Behavioral changes are out of scope —
if a function is buggy, file a separate issue, don't "fix" it during a rename sprint.

---

## Phase 2.5 — SDL2 → SDL3 Migration

Migrate from SDL2/SDL2_mixer to SDL3/SDL3_mixer. Not urgent — SDL2 will be
maintained for years — but SDL3_mixer in particular offers real improvements
for our audio code (native frequency ratio, raw PCM loading, unified audio type).

**Pre-requisite:** Verify 32-bit package availability across platforms before starting.
Check `mingw-w64-i686-SDL3`, `mingw-w64-i686-SDL3_mixer` in MSYS2, plus Linux/macOS
equivalents if cross-platform is a goal.

See [`reports/sdl3-migration-report.md`](reports/sdl3-migration-report.md) for the
full analysis (~450 call sites, ~5-8 days estimated effort).

### SDL_mixer → SDL3_mixer (largest task, ~3-5 days)
- [ ] Replace channel-based model with explicit MIX_Track pool
- [ ] Delete `freqEffect` callback — replaced by `MIX_SetTrackFrequencyRatio()`
- [ ] Delete WAV-header-wrapping hacks (`DIVMIX_LoadPCM`, `SaveSoundMem`) — replaced by `MIX_LoadRawAudio()`
- [ ] Make `SoundData` the authoritative buffer in PCM editor (MIX_Audio is opaque)
- [ ] Port IDE audio: divpcm.c (~40 sites), divbrow.c, divmixer.c, divsound.c
- [ ] Port runtime audio: shared/run/divsound.c (~35 sites), i.c
- [ ] Update structs: `Mix_Chunk*` / `Mix_Music*` → `MIX_Audio*` in global.h, divsound.h

### Event system (~1-2 days)
- [ ] Flatten window events (SDL_WINDOWEVENT sub-dispatch → top-level event types)
- [ ] Rename all event constants (SDL_KEYDOWN → SDL_EVENT_KEY_DOWN, etc.)
- [ ] Flatten keyboard struct (event.key.keysym.sym → event.key.key)
- [ ] Rebuild OSDEP_key[] table from scratch (SDL3 keycode values differ completely)
- [ ] Add resize re-entrancy guard (SDL_SetWindowSize now fires RESIZED event)

### Surface creation & palette (~0.5-1 day)
- [ ] SDL_CreateRGBSurface → SDL_CreateSurface with pixel format enums
- [ ] Add SDL_CreateSurfacePalette() after 8-bit surface creation (SDL3 doesn't auto-create)
- [ ] Update surface->format access (now an enum, not a struct pointer)
- [ ] SDL_FreeSurface → SDL_DestroySurface (~6 sites)

### Trivial renames & build system (~0.5 day)
- [ ] SDL_RWops → SDL_IOStream, SDL_RWFromMem → SDL_IOFromMem (~12 sites)
- [ ] SDL_RenderCopy → SDL_RenderTexture, SDL_ShowCursor → SDL_HideCursor
- [ ] SDL_INIT_EVERYTHING → explicit subsystem flags
- [ ] Joystick index→ID overhaul (osd_sdl2.c)
- [ ] Update CMakeLists.txt: SDL3 packages, include paths, link targets
- [ ] Delete osd_sdl12.c / osd_sdl12.h (SDL 1.2 dead code)
- [ ] Consider using SDL's Coccinelle patches for automated renames

---

## Phase 3 — Minimum Viable Steam Product

The features needed so a modern developer won't rage-quit in the first 10 minutes.

### Critical UX
- [ ] **Undo/redo in code editor** — the single most important missing feature
- [ ] **UI scaling** (2x, 3x, 4x integer scale of internal resolution)
- [ ] **English UI by default** (lenguaje.div translation exists — verify completeness)
- [ ] **Project templates** ("Hello World", "Platformer", "Shoot'em Up" skeletons)
- [ ] **One-click export** (package divrun + bytecode + data → distributable folder/zip)
- [ ] **Modern gamepad support** (SDL_GameController API, not raw SDL_Joystick)

### Window management
- [ ] Window close button must work (set `salir_del_entorno`)
- [ ] Pause/resume on minimize and focus loss
- [ ] Proper fullscreen toggle (Alt+Enter convention)
- [ ] DPI-aware rendering (SDL_WINDOW_ALLOW_HIGHDPI)

### Steam integration
- [ ] Steamworks SDK: basic integration (launch, overlay compatibility)
- [ ] Achievements (nice-to-have: "Compiled first program", "First 60fps game", etc.)
- [ ] Steam Cloud save for user projects (store in `%APPDATA%\DIV Games Studio\`)
- [ ] Proper file path system (command-line args or env vars for project directory)

### Distribution
- [ ] Automate DLL bundling in build system
- [ ] Code-sign executables (Windows SmartScreen)
- [ ] Produce a clean ~5MB distribution package

---

## Phase 4 — Community & Polish (post-launch)

Ship, get feedback, iterate. Gather real user data on what matters most.

- [ ] Steam Workshop for sharing DIV games and assets
- [ ] Interactive tutorial / guided first-game experience
- [ ] Sample games that showcase DIV's strengths
- [ ] Community-driven documentation and examples
- [ ] Collect feedback on what users actually want next

---

## Phase 5 — DIV Reborn (aspirational, driven by demand)

The door stays open. These are the deeper modernizations that would make DIV
genuinely competitive as a creative tool, not just a nostalgia piece. Pursue them
based on community demand and what feels right.

### Codebase re-architecture
The cleanup work in Phase 2 (English names, comments, structural decomposition) is
the foundation. Once the code is readable and well-documented, deeper re-architecture
becomes tractable:

- [ ] **Module boundaries** — extract clear subsystems with defined interfaces
  (compiler, VM, renderer, editor, asset pipeline) instead of the current "everything
  talks to everything through globals" model
- [ ] **Reduce global state** — move from 100+ globals in `global.h` to context structs
  passed explicitly. This is the single biggest maintainability win.
- [ ] **Clean header hierarchy** — replace the "include global.h everywhere" pattern
  with per-module headers that declare only what they need
- [ ] **Test harness** — unit tests for the compiler, VM, and file format parsers.
  Currently untestable because everything depends on the full IDE being initialized.

The re-architecture is designed to be done incrementally, one module at a time,
using the same agent-assisted sprint methodology as the Phase 2 naming work. The
architecture docs and glossary make each sprint self-contained: an agent (or human
contributor) can pick up a module, understand its boundaries, and refactor it without
needing to understand the entire 100K-line codebase.

### Rendering modernization
- [ ] **True color (32-bit RGBA) rendering mode** — alongside classic 8-bit palette
  - Daniel has wanted this for years; always wished DIV allowed more than 256 colors
  - Approach: add a 32-bit render path in the OSDEP/blitter layer, switchable per project
  - Keep 8-bit mode as default for backward compat and retro charm
  - PNG/modern image format import in editors
  - Alpha blending / per-pixel transparency
- [ ] **Native high-resolution support** — not just integer scaling of 320x200
  - Requires UI layout to be resolution-aware (the biggest piece of work)

### Language & VM evolution
- [ ] **Float/fixed-point type** — the most-requested language feature across all forks
  - Difficulty: 8/10 — touches compiler (divc.c), VM (runtime/i.c), and memory layout
  - Approach options:
    a) Native float type with new opcodes (lfadd, lfsub, etc.) — clean but invasive
    b) Fixed-point via library functions (less invasive, keeps int VM, but awkward syntax)
    c) Implicit fixed-point (integers represent thousandths, compiler handles scaling)
  - Must not break existing programs — new keyword/type, not changed semantics for int
  - The earlier phases (splitting divc.c, documenting the VM) make this tractable
- [ ] **Relaxed syntax** — quality-of-life improvements without breaking compatibility
  - Modern comparison operators alongside classic ones?
  - Optional semicolons or other ergonomic tweaks?
  - Better string handling (first-class strings, not char arrays with special opcodes)?
  - Better error messages from the compiler
- [ ] **Larger address space** — currently limited by int-sized pointers

### Editor improvements
- [ ] Syntax-aware autocomplete (function/variable list from compiler symbol table)
- [ ] Code folding
- [ ] Go-to-definition / go-to-line
- [ ] Brace/bracket matching
- [ ] Dark mode / theme support

### Export & distribution
- [ ] Web export (Emscripten — runtime partially exists already)
- [ ] "Made with DIV" branding for exported games
- [ ] Standalone installer generation for exported games

### Notes on Phase 5
- These are expensive changes. Don't start them until Phases 0-3 are solid.
- The cleanup work (splitting divc.c, documenting the VM, normalizing types) in
  Phases 1-2 is what makes Phase 5 *possible*. Don't skip the foundations.
- BennuGD proved that true color and floats attract users. The trick is shipping
  the working 8-bit version first so you have a community to build for.
- Daniel is the only person who fully understands the compiler and VM internals.
  Phase 2 documentation is critical before attempting Phase 5.

---

## License

### Current state
- Codebase is GNU GPL (applied when open-sourced ~2014, SDL port by Mike Green)
- Daniel Navarro Medrano is the original author
- GPL on the runtime (`divrun`) means games made with DIV must also be GPL — this
  blocks commercial game development

### Proposed: Dual license with runtime exception
- **IDE, compiler, editors**: remain GPL (open source, community contributions welcome)
- **Runtime** (`divrun`): GPL + Runtime Linking Exception
  - "Programs compiled with DIV and linked against the DIV runtime may be distributed
    under any license, including proprietary."
  - Precedent: GCC's libgcc exception, Classpath exception (Java), LGPL (Qt)
- Requires agreement from Mike Green (contributed SDL port under GPL)
- Third-party components to audit:
  - Judas sound library — unknown original license, needs investigation
  - SDL2/SDL2_mixer/SDL2_net — zlib license (permissive, no issue)
  - Bundled sdlgfx — zlib license (no issue)
  - Bundled miniz — MIT/zlib (no issue)

---

## Non-goals

Things we're deliberately not doing (or not doing *yet*):

- **MODE8 3D anything** — it was a regret in 1998, no need to double down. Deleted.
- **Network/multiplayer code** — never really worked properly. Strip it.
- **Mobile ports** — touch UX for an IDE is painful. Maybe runtime-only someday.
- **Rewrite in another language** — the C codebase IS DIV. We modernize it in place:
  rename, decompose, document, and re-architect — but always in C, always incrementally.
- **Breaking old programs** — existing .PRG files should still compile (except MODE8
  and network-specific code, which we're removing). New features are additive.
- **Competing with Godot/Unity on features** — we are a focused creative tool with
  a unique personality. The constraints are features until we choose to relax them.
- **64-bit port** — `sizeof(int)==sizeof(void*)` assumed in hundreds of places.
  Multi-month project with no user-visible benefit. Ship 32-bit.

---

## Lessons from the forks

Every DIV fork (Fenix, BennuGD, PixTudio, Gemix, Div GO, DIV DX) eventually stagnated.
The patterns:

1. **Dropping the IDE killed the magic.** Fenix/BennuGD became CLI-only. Users missed
   the all-in-one experience. The IDE IS the product.
2. **Single-developer dependency.** Every fork was one person. Ship something small and
   let the community contribute.
3. **Over-modernization before shipping.** Forks died chasing true-color and floats
   before they had a working product. True color and floats are good goals — but ship
   the working 8-bit/integer version first, then modernize with a community behind you.
4. **No games shipped.** No fork made it easy to distribute finished games. The export
   story is critical.
5. **Commercial models failed.** Gemix charged money in a free-tools world. Keep the
   tool free/cheap; revenue comes from the Steam ecosystem.

---

## Notes

- The codebase MUST be compiled as 32-bit (`sizeof(int) == sizeof(void*)` everywhere)
- Source files are now UTF-8 (converted from Latin-1 in Phase 0.5)
- The `texto[]` / `lenguaje.div` system makes localization straightforward
- The OSDEP layer is the right abstraction boundary for all platform work
- The process model is DIV's core innovation — protect it, document it, celebrate it
- When in doubt about why something is the way it is, ask Daniel
