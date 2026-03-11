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

**Completed 2026-03-06/07.** ~41,000 lines removed across 118 files.

Removed: CD-ROM/CDDA, MODE8/VPE 3D engine (38 files), Visor sprite generator (21 files),
DLL plugin system, div1run, network code, dead platform branches (GCW/PSP/Pandora/GP2X),
3D Map Editor, Sprite Generator. Fixed SDL2 port bugs (Shift+letter collision, mouse wheel
runaway, editor .PRG colorizer). Cleaned bundled sdlgfx. Normalized line endings. Added
build.sh wrapper and DLL auto-copy post-build step.

---

## Phase 0.5 — Fix File Encoding (Latin-1 → UTF-8) ✓

**Completed 2026-03-06.** 53 files converted from Latin-1 to UTF-8. All high-byte
string/char literals replaced with `\xNN` hex escapes. Character lookup tables in
divc.c preserved via hex escapes.

---

## Phase 1 — Stabilization & Warnings

**Completed 2026-03-09.** Replaced `-w` with `-Wall -Wextra`, fixed all 342 warnings
(CRITICAL through LOW). Zero warnings since Sprint A (2026-03-10).

Key fixes: operator precedence bug in #include path (divc.c), use-after-free in BMP palette
loader (divforma.c), 6 missing return statements, destructive OSDEP_IsFullScreen() replaced
with proper SDL2 query, fullscreen toggle, video mode dialog, streaming texture, integer
scaling, safe string helpers (div_string.h — 38 high-risk sites + Sprint E's ~1,030 calls).
Removed SDL1/Emscripten preprocessor branches. Normalized path separators.

### Remaining Phase 1 items
- [ ] Audit the `PrintEvent` pattern for similar `#ifdef`-body bugs
- [ ] Fix or remove the commented-out `free()` in runtime stack management (i.c:778)
- [ ] `test_video` startup dialog — could be re-enabled
- [ ] DPI-aware rendering (SDL_WINDOW_ALLOW_HIGHDPI)
- [ ] Unify byte types: pick `uint8_t` everywhere

---

## Phase 2 — Code Cleanup & Readability

One file at a time, make the code understandable to someone who isn't Daniel.
No behavioral changes — pure cleanup.

### Completed cleanup work
- Architecture docs: 7 reports in docs/ (architecture-overview, compiler-pipeline, vm-and-runtime, glossary, video-system-audit, unsafe-string-audit, sdl3-migration-report)
- OJO markers: 47 to 0 resolved (Sprint B)
- English comments: 22 key functions documented (Sprint C), ~3,170 Spanish comments translated (Sprint H)
- Single-letter globals: r,g,b,c,d,a + FILE *f removed (Sprint D)
- Function renames: 331 Spanish to English across 39+ files (Sprint F)
- Global/struct renames: ~131 identifiers across 44 files (Sprint G)
- High-frequency globals: copia to screen_buffer, ventana to window, texto to texts across 39 files (Sprint J)
- Unsafe strings: ~1,030 calls converted to safe helpers across 31 files (Sprint E)
- [ ] Review architecture docs for accuracy (Daniel is the authority on compiler/VM internals)
- [ ] Rename cryptic locals in the hottest paths (divc.c, divedit.c, runtime/i.c)

### Structural improvements
- [ ] Split `divc.c` (7,823 lines) into lexer/parser/codegen modules — Sprint I
- [ ] Split `div.c` (4,940 lines) into desktop/dialogs modules — Sprint I
- [ ] Split `divpaint.c` (4,969 lines) into tools/selection modules — Sprint I
- [ ] Group related globals into context structs where it simplifies things

### Systematic modernization (agent-assisted sprints)

The long-term vision for Phase 2 goes far beyond renaming a few globals. The goal is
to make this codebase genuinely maintainable — by anyone, not just Daniel. This means:

1. **Full English translation** — every Spanish identifier, comment, and variable name
   gets an English equivalent. Largely complete (Sprints D/F/G/H/J — see completed work above).

2. **Meaningful names everywhere** — not just replacing `tapiz` with `wallpaper`, but
   turning `a`, `b`, `c`, `n`, `nn`, `nnn` into names that reveal intent. Single-letter
   globals done; cryptic locals in hottest paths (divc.c, divedit.c, runtime/i.c) remain.

3. **English comments on every non-obvious function** — 22 key functions documented
   (Sprint C), ~3,170 comment lines translated (Sprint H). Ongoing for new/changed code.

4. **Structural decomposition** — the monster files (divc.c, div.c, divpaint.c) should
   be split along natural boundaries that the architecture docs have already identified.
   Next: Sprint I (file splitting).

**Methodology:** This work is ideal for AI agent teams working in focused sprints —
one file or subsystem at a time, using the architecture docs and glossary as context.
Each sprint produces a self-contained PR that can be reviewed and merged independently.
The docs/ directory exists precisely to give agents (and future contributors) the
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

See [`docs/sdl3-migration-report.md`](docs/sdl3-migration-report.md) for the
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
- [ ] **English UI by default** (lenguaje.div translation exists — verify completeness and correctness)
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
