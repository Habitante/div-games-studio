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

## Completed work

Everything below is done and committed. Kept here as reference so future sessions
know the history without re-discovering it.

### Phase 0 — Archaeology & Dead Code Removal (2026-03-06/07)
~41,000 lines removed across 118 files. Removed MODE8/VPE 3D engine, Visor sprite
generator, CD-ROM/CDDA, DLL plugin, div1run, network code, dead platform branches,
3D Map Editor. Fixed several SDL2 port bugs.

### Phase 0.5 — Encoding Fix (2026-03-06)
53 files converted Latin-1 → UTF-8. Hex escapes for character lookup tables.

### Phase 1 — Stabilization & Warnings (2026-03-09)
Replaced `-w` with `-Wall -Wextra`, fixed all 342 warnings. Key fixes: operator
precedence bug in divc.c, use-after-free in divforma.c, 6 missing return statements,
safe string helpers (div_string.h). Streaming texture, integer scaling, fullscreen
toggle, video mode dialog. Zero warnings since 2026-03-10.

### Phase 2A — Translation & Safety Sprints (2026-03-10/11)
Nine sprints (A through J) making the codebase readable:

| Sprint | What | Scale |
|--------|------|-------|
| A | 626 LOW warnings → 0 | 35 files |
| B | 47 OJO markers → 0 | code fixes + todo annotations |
| C | 22 key functions documented in English | comment blocks |
| D | 7 single-letter globals removed (r,g,b,c,d,a,f) | 11 files |
| E | ~1,030 unsafe string calls → safe helpers | 31 files |
| F | 331 Spanish function names → English | 39+ files |
| G | ~131 globals + struct field renames | 44 files |
| H | ~3,170 Spanish comment lines → English | 34 files |
| J | 3 high-frequency globals renamed (copia/ventana/texto) | 39 files |

### Phase 2B — Tooling, Docs & Dead Code (2026-03-11)
- Cleanup: 15 root files deleted, reports archived to `docs/archive/`
- Tooling: `.clang-format`, `.editorconfig`, `.clang-tidy` added
- Documentation: compiler pipeline, VM/runtime architecture docs written
- Original MSDOS source backup added for cross-reference
- **Phase 2B-3 audit:** 141 DOS/NOTYET blocks audited across 29 files.
  111 blocks + entire `judas/` directory removed (~15,700 lines).
  7 bugs found and fixed, 8 lost features restored (joystick, FLI palette,
  SetSongPos, browser cleanup, file dialog, keyboard mouse, numpad 5).
  All `__EMSCRIPTEN__` dead code removed from div.c.

**Net result of all phases:** codebase reduced from ~130K to ~87K lines. Zero
warnings. All identifiers in public/global scope translated to English. Architecture
documented. Four targets build clean.

---

## Current Phase — Codebase Modernization

The codebase compiles and works, but it's still far from "nice code." The goal of
this phase is to make it genuinely maintainable — well-organized, consistently
formatted, properly named, and easy for any developer (or agent) to navigate.

We are NOT in a rush. Each step should be done carefully, verified, and documented
before moving on. Every sprint must update project docs (ROADMAP.md, MEMORY.md,
README.md) as part of its deliverables.

### 2C-1. Apply coding standards ✓

- [x] Applied `clang-format` (v20.1.8) to 82 project source files. Third-party
      and data headers excluded (listed in `.clang-format-ignore`).
- [x] `.git-blame-ignore-revs` created so `git blame` skips the formatting commit.
- [x] Snake_case collision analysis completed — 18 collisions found, all in the
      runtime-wrapper vs. implementation pattern. Full report in
      [`docs/snake-case-collision-report.md`](docs/snake-case-collision-report.md).
- [x] **Naming convention decided:** `snake_case` for functions/variables/structs/
      typedefs. `UPPER_CASE` for macros/enum constants. `OSDEP_*`/`SDL_*`/`Mix_*`
      preserved as-is.

### 2C-2. Source reorganization

Current layout is mostly flat (`src/` with 30+ files). Reorganize into logical
modules without going overboard (no single-file folders).

Proposed structure (to be discussed and refined):

```
src/
  ide/          IDE core: div.c, divhandl.c, divmouse.c, divkeybo.c, divvideo.c,
                diveffec.c, divbrow.c, divbasic.c, divmixer.c, divsound.c, divtimer.c
  editor/       Editors: divedit.c, divpaint.c, divfpg.c, divfont.c, divcolor.c,
                divbrush.c, divpcm.c, divforma.c
  compiler/     Compiler: divc.c (eventually split), divpnum.c, ifs.c, newfuncs.c
  runtime/      VM + runtime: i.c, f.c, s.c, divlengu.c, divmixer.c
                (already partially organized)
  shared/       Cross-cutting: osdep/, run/, lib/, div_string.h, global.h
  runner/       Launcher: r.c
```

- [ ] Finalize folder structure (discuss with Daniel)
- [ ] Move files, update `#include` paths and CMakeLists.txt
- [ ] Rename files where names are misleading (identify candidates first)
- [ ] Update README.md source tree and docs to reflect new layout

### 2C-3. Split monster files

Three files are too large to navigate or maintain effectively:

| File | Lines | Proposed split |
|------|-------|----------------|
| `divc.c` | 7,824 | `divc_lexer.c` + `divc_parser.c` + `divc_codegen.c` |
| `divpaint.c` | 4,970 | `divpaint.c` + `divpaint_tools.c` + `divpaint_select.c` |
| `div.c` | 4,721 | `div.c` + `div_desktop.c` + `div_dialogs.c` |

Rules: extract functions, add forward declarations, update CMake. No behavioral
changes. Static/file-scope globals may need to become shared or passed as parameters.

### 2C-4. Remaining identifier renames

**2C-4a (done):** 76 PascalCase/Spanish functions renamed to snake_case with
module prefixes across 25 files. All 18 snake_case collisions resolved.
Modules: `sound_` (18), `fli_` (16), `fpg_` (12), `pal_` (10), `fmt_` (20).

**2C-4b (done):** File-local variables, struct fields, and parameters renamed
across 45 files (~9,160 lines). Three batches:

- **Compiler (divc.c + d.c):** 68 rename mappings, ~4,560 replacements.
  `struct objeto`→`object`, 23 union members decoded (`vglo`→`var_global`,
  `pigl`→`ptr_int_global`, `sloc`→`struct_local`, etc.), `pieza`→`current_token`
  (905 hits), `bloque_actual`→`current_scope`, `pila`→`eval_stack`, lex_ele
  fields, error vars, 4 function renames.
- **Editor (divedit.c + divcolor.c):** 27 renames, ~715 replacements.
  `papelera`→`clipboard`, `kbloque`→`block_state`, `modo_cursor`→`cursor_mode`,
  `cpieza`→`color_token`, `volcado_saltado`→`skip_full_blit`.
- **Runtime + cross-file (44 files):** `reloj`→`frame_clock` (11 files),
  `wvolcado`→`blit_region` (10 files), `ancho`→`width`/`alto`→`height` (19 files),
  `an`→`w`/`al`→`h` (targeted regex to preserve English comments),
  `espacio`→`spacing`, `espaciado`→`letter_spacing`, `nomitidos`→`num_skipped`,
  `retra`→`retrace_pending`, `mouse_pintado`→`mouse_drawn`.

Still remaining — mostly deeper locals in large files:

- [x] divc.c — compiler structs, state vars, union members ✓
- [x] divedit.c — clipboard, selection, cursor state ✓
- [x] runtime/i.c — timing, blit, rendering vars ✓
- [x] divpaint.c — paint editor locals + cross-file struct fields ✓
- [x] divhandl.c — dialog/window manager locals + cross-file functions ✓

### 2C-5. Documentation pass

After reorganization, update all docs to match the new reality:

- [ ] README.md — source tree, line counts, build instructions
- [ ] `docs/architecture-overview.md` — file paths, module descriptions
- [ ] `docs/compiler-pipeline.md` — paths if compiler files moved/split
- [ ] `docs/vm-and-runtime.md` — paths if runtime files moved

---

## Future: Testing Framework

**Deferred until codebase is in better shape.** To be discussed in detail when we
get there. Some initial thoughts:

- The main challenge is that everything depends on the full IDE being initialized
  (global state, SDL, loaded files). True unit testing requires decoupling.
- Possible starting points: compiler tests (compile known .PRG files, verify
  bytecode output), file format tests (load/save FPG/MAP/FNT round-trips),
  runtime function tests (call individual f.c functions with known stack state).
- A test harness is a prerequisite for autonomous agent improvement loops.

---

## Phase 2.5 — SDL2 → SDL3 Migration

Not urgent — SDL2 will be maintained for years. See
[`docs/sdl3-migration-report.md`](docs/sdl3-migration-report.md) for the full
analysis (~450 call sites). Pre-requisite: verify 32-bit SDL3 package availability.

---

## Phase 3 — Minimum Viable Steam Product

The features needed so a modern developer won't rage-quit in the first 10 minutes.

### Critical UX
- [ ] **Undo/redo in code editor** — the single most important missing feature
- [ ] **UI scaling** (2x, 3x, 4x integer scale of internal resolution)
- [ ] **English UI by default** (verify lenguaje.div translation completeness)
- [ ] **Project templates** ("Hello World", "Platformer", "Shoot'em Up" skeletons)
- [ ] **One-click export** (package divrun + bytecode + data → distributable zip)
- [ ] **Modern gamepad support** (SDL_GameController API, not raw SDL_Joystick)

### Window management
- [ ] Proper fullscreen toggle (Alt+Enter convention)
- [ ] DPI-aware rendering (SDL_WINDOW_ALLOW_HIGHDPI)

### Steam integration
- [ ] Steamworks SDK: basic integration (launch, overlay)
- [ ] Steam Cloud save for user projects
- [ ] Proper file path system (command-line args or env vars)

### Distribution
- [ ] Automate DLL bundling in build system
- [ ] Code-sign executables (Windows SmartScreen)
- [ ] Clean ~5MB distribution package

---

## Phase 4 — Community & Polish (post-launch)

Ship, get feedback, iterate.

- [ ] Steam Workshop for sharing DIV games and assets
- [ ] Interactive tutorial / guided first-game experience
- [ ] Sample games showcasing DIV's strengths
- [ ] Community-driven documentation and examples

---

## Phase 5 — DIV Reborn (aspirational, driven by demand)

Deeper modernizations that would make DIV genuinely competitive as a creative tool.
Only pursue based on community demand after shipping.

### Codebase re-architecture
- [ ] Module boundaries with defined interfaces (compiler, VM, renderer, editor)
- [ ] Reduce global state — context structs passed explicitly
- [ ] Clean header hierarchy — per-module headers, not "global.h everywhere"
- [ ] Test harness — unit tests for compiler, VM, file format parsers

### Rendering modernization
- [ ] True color (32-bit RGBA) alongside classic 8-bit palette mode
- [ ] PNG/modern image format import
- [ ] Alpha blending / per-pixel transparency
- [ ] Native high-resolution support (resolution-aware UI layout)

### Language & VM evolution
- [ ] Float/fixed-point type (the most-requested feature across all forks)
- [ ] Relaxed syntax / quality-of-life improvements
- [ ] Better string handling, better error messages
- [ ] Larger address space

### Editor improvements
- [ ] Syntax-aware autocomplete
- [ ] Code folding, go-to-definition, brace matching
- [ ] Dark mode / theme support

### Export & distribution
- [ ] Web export (Emscripten)
- [ ] Standalone installer generation for exported games

---

## License

### Current state
GNU GPL (applied when open-sourced ~2014, SDL port by Mike Green).

### Proposed: Dual license with runtime exception
- **IDE, compiler, editors**: GPL (open source, community contributions)
- **Runtime** (`divrun`): GPL + Runtime Linking Exception — games compiled with
  DIV may be distributed under any license, including proprietary
- Precedent: GCC's libgcc exception, Classpath exception
- Requires agreement from Mike Green
- Third-party: SDL2/mixer/net (zlib), sdlgfx (zlib), miniz (MIT) — all permissive

---

## Non-goals

- **MODE8 3D** — deleted, not coming back
- **Network/multiplayer** — never worked properly, stripped
- **Mobile ports** — touch UX for an IDE is painful
- **Rewrite in another language** — modernize in C, incrementally
- **Breaking old programs** — existing .PRG files must still compile
- **Competing with Godot/Unity** — focused creative tool with unique personality
- **64-bit port** — `sizeof(int)==sizeof(void*)` assumed everywhere. Ship 32-bit.

---

## Lessons from the forks

Every DIV fork (Fenix, BennuGD, PixTudio, Gemix, Div GO, DIV DX) stagnated.
Patterns: dropping the IDE killed the magic; single-developer dependency; over-
modernization before shipping; no games shipped; commercial models failed. The IDE
IS the product. Ship the working version first, modernize with a community behind you.
