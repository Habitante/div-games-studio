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
