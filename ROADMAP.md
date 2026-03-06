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

### Still pending from Phase 0
- [ ] Clean up bundled sdlgfx — only `SDL_framerate.c` is actually used
- [ ] Remove hundreds of commented-out code blocks throughout

### Decision: Drop DIV1 runtime?
- [ ] Decide whether to keep `div1run/` (~3,000 lines, separate interpreter)
- Recommendation: drop it. Nobody is running DIV1 bytecode in 2026, and maintaining
  two parallel interpreters doubles the cost of every VM bugfix.
- If kept: clearly mark as legacy, don't ship on Steam.

### Hide deprecated IDE features
- [ ] Remove or hide from menus: CD Player, Clock, Trash icon, DOS Shell, Calculator
- [ ] Remove or hide: 3D Map Editor (MODE8-dependent, useless without VPE)
- [ ] Keep but flag: Sprite Generator (visor-based, evaluate if anyone uses it)

### Build system cleanup
- [ ] Remove dead cmake options and platform branches
- [ ] Add a DLL auto-copy step so first-time build "just works"
- [ ] Add a one-command build script wrapper
- [ ] Add proper `.gitignore` for build/ and div/ runtime artifacts

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
  - 39 Spanish error message strings in `div1run/inter.h`
  - 127 font bitmap data lines in `SDL_gfxPrimitives_font.h`
  - Display strings in `divfont.c`, `divhandl.c`, `divhelp.c`, `divpaint.c`, `runtime/debug/d.c`
- [x] Converted all 53 Latin-1 source files to UTF-8
- [x] Verified: build succeeds, F11 compile works, runtime works, debug works
- 7 "binary" headers remain (font/animation embedded data) — not editable text

---

## Phase 1 — Stabilization & Warnings

Make the compiler tell us what's actually broken. Fix the scariest stuff.

### Enable warnings
- [ ] Replace `-w` (suppress all warnings) with `-Wall -Wextra` in CMakeLists.txt
- [ ] Fix all implicit function declarations
- [ ] Fix pointer/int cast warnings (the 32-bit-assumption cases)
- [ ] Fix unused variable warnings
- [ ] Remove `-fpermissive` once code is clean enough

### Fix known landmines
- [ ] `divkeybo.c:32-33`: DOS BIOS addresses hardcoded (`0x41a`, `0x41c`) — will crash
- [ ] `div.c:2949`: hardcoded `/home/mike/div2015/system/red_panel.png` dev path
- [ ] Audit all 1,493 `sprintf`/`strcpy` calls — migrate critical paths to safe variants
- [ ] Audit the `PrintEvent` pattern for similar `#ifdef`-body bugs (divmouse.c:506 was one)
- [ ] Fix window close button (currently logged but ignored — no `salir_del_entorno`)
- [ ] Fix focus loss handling (no pause on minimize/alt-tab)
- [ ] Fix or remove the commented-out `free()` in runtime stack management (`i.c:778`)

### Normalize basics
- [ ] Unify byte types: pick `uint8_t` everywhere, stop mixing `byte`/`char`/`uchar`
- [ ] Normalize path separators (some code only checks `/`, not `\`)

---

## Phase 2 — Code Cleanup & Readability

One file at a time, make the code understandable to someone who isn't Daniel.
No behavioral changes — pure cleanup.

### Naming
- [ ] Rename the worst single-letter globals (the `r,g,b,c,d,a` "generic counters"
      in global.h:519 that even the original code says "OJO! Quitarlos")
- [ ] Rename cryptic locals in the hottest paths (divc.c, divedit.c, runtime/i.c)
- [ ] Add English comments to the top 20 most cryptic functions
- [ ] Document the meaning of `v.an`/`v.al` (width/height), `tapiz` (wallpaper),
      `papelera` (clipboard), etc. in a glossary or inline

### Architecture
- [ ] Document the call graph from IDE startup → main loop → event processing
- [ ] Document the compiler pipeline: lexer → parser → codegen → bytecode format
- [ ] Document the VM interpreter loop and process scheduling algorithm
- [ ] Document the rendering pipeline: 8-bit surface → palette blit → SDL2 texture
- [ ] Document the FPG/MAP/FNT/PAL binary file formats
- [ ] Document the OSDEP abstraction layer contract
- [ ] Catalogue all 100+ globals in global.h with their roles

### Structural improvements
- [ ] Consider splitting `divc.c` (9,346 lines) into lexer/parser/codegen modules
- [ ] Consider splitting `div.c` (4,678 lines) into menu handling vs desktop management
- [ ] Group related globals into context structs where it simplifies things
- [ ] Normalize string handling patterns across the codebase

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
- **Rewrite in another language** — the C codebase IS DIV; we clean it, not replace it.
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
