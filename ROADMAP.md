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

### Phase 2C — Codebase Modernization (2026-03-11/12)

Made the codebase genuinely maintainable: consistently formatted, logically
organized, properly named, and easy for any developer (or agent) to navigate.

**2C-1. Coding standards:** `clang-format` applied to 82 files. Naming
convention decided: `snake_case` everywhere, `UPPER_CASE` for macros/enums.
18 collision cases resolved via module prefixes.

**2C-2. Source reorganization:** Flat `src/` split into `ide/`, `editor/`,
`compiler/`, `formats/`, `runtime/`, `shared/`, `runner/`. All files renamed
to drop `div` prefix and use descriptive English names. Single-letter runtime
files expanded (i.c → interpreter.c, etc.). 13 dead files deleted.

**2C-3. Monster file splits:** 6 oversized files split into focused modules
(~40K lines reorganized, no behavioral changes):

| Original | Split into |
|----------|-----------|
| `compiler/compiler.c` (11.5K) | `compiler_internal.h` + `compiler.c` + `compiler_parser.c` + `compiler_expression.c` |
| `editor/paint.c` (7.7K) | `paint.c` + `paint_tools.c` + `paint_select.c` + `paint_internal.h` |
| `runtime/debug/debugger.c` (6.7K) | `debugger_internal.h` + `debugger.c` + `debugger_ui.c` + `debugger_inspect.c` + `debugger_code.c` + `debugger_proclist.c` + `debugger_profile.c` |
| `ide/main.c` (5.5K) | `main_internal.h` + `main.c` + `main_desktop.c` + `main_dialogs.c` |
| `editor/code.c` (4.6K) | `editor_internal.h` + `editor.c` + `editor_edit.c` + `editor_render.c` + `editor_file.c` |
| `ide/handler.c` (4.4K) | `handler_internal.h` + `handler.c` + `handler_dialogs.c` + `handler_map.c` + `handler_fonts.c` |

`runtime/functions.c` (6.4K) reviewed and kept as-is — flat list of 150+
independent VM API implementations, already well-sectioned.

**2C-4. Identifier renames:** All identifiers now English snake_case.
~200 PascalCase functions, ~120 Spanish identifiers, struct fields, locals,
and globals renamed across the entire codebase in three sub-phases.

**2C-5. Documentation pass:** All docs (README, ROADMAP, architecture,
compiler pipeline, VM/runtime, SDL3 migration report, glossary) updated to
match the reorganized and renamed codebase.

**Net result of all phases:** codebase reduced from ~130K to ~87K lines. Zero
warnings. All identifiers translated to English snake_case. Source organized
into logical modules with focused files. Architecture documented. Four targets
build clean.

---

## Code Review (2026-03-12)

Five-axis automated audit of the entire `src/` tree covering memory safety,
global state & coupling, magic numbers & limits, error handling, and performance.
Full findings saved in `docs/code-review-2026-03-12.md`. Key takeaways that
shaped the roadmap below:

- **3 real bugs** — division-by-zero in release builds, double-free in BMP
  loader, FPG heap overflow on crafted files
- **File format parsers trust every byte** — no validation of dimensions,
  offsets, or array indices read from files. Biggest safety gap in the codebase.
- **154 globals**, `screen_buffer` written by 4 modules with no ownership, but
  the coupling doesn't block Phase 4 features — fix only what specific features
  require
- **Performance is fine** — O(N^2) process scheduling is irrelevant at typical
  DIV game scale

---

## Phase 3 — Hardening & Bug Fixes

The code review found real bugs and safety gaps that should be fixed before
adding features. Everything here is bounded, testable work — no architecture
astronautics.

### 3A — Real bugs (immediate) ✓

- [x] **Division by zero crashes release builds** — Zero-divisor guards on
  `ldiv`, `lmod`, `ldia`, `lmoa` made unconditional (work in release builds)
- [x] **Double-free in BMP loader** — `CopiaBuffer` freed once with NULL
  assignment, no more multiple-free paths
- [x] **FPG index overflow** — `nIndex >= 1000` bounds check added, stops
  loading when limit reached
- [x] **FPG code field unchecked** — `kkhead.code` validated against `[0, 1000)`
  before use as array index, out-of-range codes skipped

### 3B — File format hardening ✓

- [x] **`validate_image_dimensions()` helper** — Checks positive, <=16384,
  and no 32-bit overflow. Called from all `fmt_is_*` and `fmt_load_*` functions,
  plus FPG reader functions
- [x] **Dangerous `fread()` return checks** — Checked returns on dimension/
  palette reads in `fpg_read_header`, `fpg_read_image_header`, `fpg_open`,
  and `fmt_load_dac_bmp` (fields that control allocations or array indices)
- [x] **PCX RLE bounds checking** — `bytes_per_line` validated against
  `map_width` before RLE decode to prevent buffer overrun
- [x] **FPG palette read validation** — `fpg_open` checks 768-byte palette
  fread return, closes file on truncation
- [x] **MAP/FPG num_points validation** — Clamped to [0, 256] in
  `fmt_load_map`; rejected in `fpg_read_image_header`, `fpg_remap_to_pal`,
  `fpg_delete`, `fpg_delete_many`

### 3C — Runtime & SDL robustness ✓

- [x] **Check SDL init calls** — `SDL_Init()`, `SDL_CreateWindowAndRenderer()`,
  `SDL_CreateRGBSurface()`, `SDL_CreateTexture()` in `osd_sdl2.c` now checked
  for failure with `SDL_Log` error messages; `OSDEP_SetVideoMode` returns NULL
  on any failure
- [x] **Compiler `longjmp` cleanup** — `comp()` now calls `free_resources()`
  on the error path (when `setjmp` returns nonzero), freeing `mem`, `loc`,
  `frm`, `vnom`, and closing open file handles
- [x] **Unchecked `fopen()` in Windows `fmemopen` shim** — `_tempnam()` and
  both `fopen()` calls now checked for NULL; `tmpcount` bounds-checked against
  array size; returns NULL instead of crashing
- [x] **Runtime `exit()` without cleanup** — `_exit_dos()` (functions.c) and
  `exer()` (interpreter.c) now call `closefiles()` to clean up temp files
  before exit. `atexit(OSDEP_Quit)` already handles SDL shutdown

---

## Phase 4 — Minimum Viable Steam Product

The features needed so a modern developer won't rage-quit in the first 10
minutes. Phase 3 hardening ensures the foundation is solid first.

### Critical UX

- [ ] **Undo/redo in code editor** — the single most important missing feature.
  The paint editor has its own undo system (`undo`/`undo_table` globals); the
  code editor needs a separate one (operation log in `tprg`, not shared with
  paint). Not blocked by global state — just needs new code in `editor/`
- [ ] **UI scaling** (2x, 3x, 4x integer scale of internal resolution) — UI
  layout uses hardcoded pixel offsets everywhere (+7, +23, x9 for menus).
  Start with SDL render scaling of the final output (easy), defer internal
  layout scaling (hard, magic numbers everywhere)
- [ ] **English UI by default** (verify `lenguaje.div` translation completeness)
- [ ] **Project templates** ("Hello World", "Platformer", "Shoot'em Up"
  skeletons)
- [ ] **One-click export** (package `divrun` + bytecode + data into
  distributable zip)
- [ ] **Modern gamepad support** (SDL_GameController API, not raw
  SDL_Joystick) — joystick code is cleanly isolated in `osd_sdl2.c`

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

## Phase 5 — SDL2 → SDL3 Migration

Not urgent — SDL2 will be maintained for years. See
[`docs/sdl3-migration-report.md`](docs/sdl3-migration-report.md) for the full
analysis (~450 call sites). Pre-requisite: verify 32-bit SDL3 package
availability.

Biggest win: `MIX_SetTrackFrequencyRatio()` replaces the 60-line hand-written
`freqEffect` sample-rate converter in `shared/run/sound.c` and eliminates the
WAV-header-wrapping hacks in the PCM editor.

---

## Phase 6 — Community & Polish (post-launch)

Ship, get feedback, iterate.

- [ ] Steam Workshop for sharing DIV games and assets
- [ ] Interactive tutorial / guided first-game experience
- [ ] Sample games showcasing DIV's strengths
- [ ] Community-driven documentation and examples

---

## Phase 7 — DIV Reborn (aspirational, driven by demand)

Deeper modernizations that would make DIV genuinely competitive as a creative
tool. Only pursue based on community demand after shipping.

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

### Codebase re-architecture (only if needed for the above)

The code review found 154 globals in `global.h`, `screen_buffer` written by 4
modules with no ownership, and every `*_internal.h` re-including `global.h`
anyway. The 80 .c files are subroutines of one program, not modules. This is
the cost of the 1990s — fix only what specific features require:

- [ ] Renderer abstraction (if true color needs a different pipeline)
- [ ] Per-editor context structs (if multiple undo systems need isolation)
- [ ] Module-local headers (if compile times become a problem)
- [ ] Test harness — compiler, VM, and file format parser tests

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
