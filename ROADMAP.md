# DIV Games Studio — Roadmap

No deadlines, no pressure. Just a living document for when the mood strikes.

The goal: ship DIV on Steam as a polished retro game-creation toolkit. Lean into the
constraints (256 colors, integer math, cooperative processes) as features, not bugs.
Think PICO-8, not Godot. The integrated IDE is the killer differentiator — no fork
ever replicated it.

---

## Phase 0 — Archaeology & Dead Code Removal

Strip ~35,000 lines of dead weight. Make the codebase honest about what it is today.

### Strip dead subsystems
- [ ] Delete CD-ROM/CDDA code: `cdrom.c`, `divcdrom.c`, `runtime/cdrom.c` (~2,200 lines)
- [ ] Delete MODE8/VPE 3D engine: `runtime/vpe/` (18 files, ~6,500 lines) — it was a regret in 1998
- [ ] Delete Visor 3D sprite generator: `visor/` (9 files, ~5,000 lines) — dead ifdef, never active
- [ ] Delete DLL plugin system: grep for `DIVDLL`, remove infrastructure (~1,000 lines)
- [ ] Delete `div1run/unused/` directory (explicitly marked dead)
- [ ] Audit and likely delete `other/` directory (~17,000 lines of vestigial duplicates)
- [ ] Remove dead `#ifdef` branches: GCW, PSP, Pandora, GP2X, Amiga, Atari ST, PS2 (~200 lines)
- [ ] Delete dead cmake toolchain files (keep only: windows-native, linux, osx, pi)
- [ ] Remove `shared/lib/portrend/` (dead 3D code behind commented ifdef)
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

Ship, get feedback, iterate.

- [ ] Steam Workshop for sharing DIV games and assets
- [ ] Interactive tutorial / guided first-game experience
- [ ] Sample games that showcase DIV's strengths
- [ ] Community-driven documentation and examples
- [ ] Evaluate demand for: true color mode, float type, higher resolutions
      (don't build these until real users ask for them)

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

Things we're deliberately not doing:

- **MODE8 3D anything** — it was a regret in 1998, no need to double down. Deleted.
- **Mobile ports** — touch UX for an IDE is painful. Maybe runtime-only someday.
- **Rewrite in another language** — the C codebase IS DIV; we clean it, not replace it.
- **Backwards-incompatible language changes** — old .PRG files should still compile.
- **Competing with Godot/Unity** — we are a constrained retro-creative-tool, not a
  general-purpose engine. The constraints are features.
- **64-bit port** — the codebase assumes `sizeof(int)==sizeof(void*)` in hundreds of
  places. Fixing this is a multi-month project with no user-visible benefit. Ship 32-bit.

---

## Lessons from the forks

Every DIV fork (Fenix, BennuGD, PixTudio, Gemix, Div GO, DIV DX) eventually stagnated.
The patterns:

1. **Dropping the IDE killed the magic.** Fenix/BennuGD became CLI-only. Users missed
   the all-in-one experience. The IDE IS the product.
2. **Single-developer dependency.** Every fork was one person. Ship something small and
   let the community contribute.
3. **Over-modernization.** Forks died chasing true-color and floats. Ship the working
   8-bit/integer version first. Modernize based on demand.
4. **No games shipped.** No fork made it easy to distribute finished games. The export
   story is critical.
5. **Commercial models failed.** Gemix charged money in a free-tools world. Keep the
   tool free/cheap; revenue comes from the Steam ecosystem.

---

## Notes

- The codebase MUST be compiled as 32-bit (`sizeof(int) == sizeof(void*)` everywhere)
- The `texto[]` / `lenguaje.div` system makes localization straightforward
- The OSDEP layer is the right abstraction boundary for all platform work
- The process model is DIV's core innovation — protect it, document it, celebrate it
- When in doubt about why something is the way it is, ask Daniel
