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
      [`docs/archive/snake-case-collision-report.md`](docs/archive/snake-case-collision-report.md).
- [x] **Naming convention decided:** `snake_case` for functions/variables/structs/
      typedefs. `UPPER_CASE` for macros/enum constants. `OSDEP_*`/`SDL_*`/`Mix_*`
      preserved as-is.

### 2C-2. Source reorganization ✓

Flat `src/` directory reorganized into logical modules. All files renamed to
drop the `div` prefix and use descriptive English names. Single-letter runtime
files expanded (i.c → interpreter.c, etc.). `.hpp` files normalized to `.h`.
13 dead files deleted (~370 lines). `kernel.cpp` renamed to `kernel.inc`
(code fragment `#include`'d by interpreter.c, not a standalone compilation unit).

```
src/
  ide/          main.c, handler.c, handler_dialogs.c, handler_map.c,
                handler_fonts.c, desktop.c, window.c, mouse.c, keyboard.c,
                video.c, graphics.c, effects.c, browser.c, help.c, setup.c,
                gamma.c, language.c, installer.c, packer.c, trash.c, clock.c,
                mixer_ui.c, mixer.c, sound.c, vesa.c, sprite.c, recorder.c
  editor/       editor.c, editor_edit.c, editor_render.c, editor_file.c,
                editor_internal.h, paint.c, brush.c, fpg.c, font.c,
                palette.c, pcm.c, colorizer.c, charset.c
  compiler/     compiler.c, calc.c, imgload.c
  formats/      image.c, fpg.c
  runtime/      interpreter.c, functions.c, render.c, language.c, mixer.c,
                vesa.c, debug/debugger.c, debug/decompiler.c, debug/kernel.inc,
                fli/flxplay.c
  shared/       osdep/, run/(collision.c, video.c, pathfind.c, fli.c,
                keyboard.c, sound.c, mouse.c, pcmtowav.c), lib/, unzip.c
  runner/       runner.c
  win/          osdepwin.c
  (root)        global.h, div_string.h, div_stub.h, madewith.h, sysdac.h,
                mixer.h, fpg.h, charset.h, sound.h, keyboard.h, recorder.h,
                imgload.h, ide.h
```

### 2C-3. Split monster files

Large files that are hard to navigate or maintain effectively:

| File | Lines | Split | Status |
|------|-------|-------|--------|
| `compiler/compiler.c` | ~11,500 | `compiler_internal.h` + `compiler.c` + `compiler_parser.c` + `compiler_expression.c` | ✓ |
| `editor/paint.c` | ~7,700 | `paint.c` + `paint_tools.c` + `paint_select.c` | ✓ |
| `ide/main.c` | ~5,500 | `main_internal.h` (110) + `main.c` (2,742) + `main_desktop.c` (1,754) + `main_dialogs.c` (1,005) | ✓ |
| `runtime/debug/debugger.c` | ~6,700 | `debugger_internal.h` (311) + `debugger.c` (1,093) + `debugger_ui.c` (2,167) + `debugger_inspect.c` (1,444) + `debugger_code.c` (881) + `debugger_proclist.c` (296) + `debugger_profile.c` (689) | ✓ |
| `editor/code.c` | ~4,600 | `editor_internal.h` (191) + `editor.c` (1,122) + `editor_edit.c` (916) + `editor_render.c` (1,170) + `editor_file.c` (1,264) | ✓ |
| `ide/handler.c` | ~4,400 | `handler_internal.h` (89) + `handler.c` (1,501) + `handler_dialogs.c` (994) + `handler_map.c` (1,564) + `handler_fonts.c` (272) | ✓ |

`runtime/functions.c` (~6,400 lines) reviewed and kept as-is — flat list of
150+ independent VM API implementations, already well-sectioned with comments.

**compiler.c split notes:** The actual split differed from the original proposal.
The lexer (~500 lines) stayed in compiler.c rather than getting its own file.
`statement()` went into the parser file (it parses control flow). The "codegen"
file became `compiler_expression.c` since this is a single-pass compiler —
expression parsing and code generation are interleaved.

**handler.c split notes:** Split into menu infrastructure, dialogs, map operations,
and font generation. All 9 Spanish menu function names were renamed to English:
`menu_principal` -> `menu_main`, `menu_programas` -> `menu_programs`,
`menu_edicion` -> `menu_edit`, `menu_paletas` -> `menu_palettes`,
`menu_mapas` -> `menu_maps`, `menu_graficos` -> `menu_graphics`,
`menu_fuentes` -> `menu_fonts`, `menu_sonidos` -> `menu_sounds`,
`menu_sistema` -> `menu_system`. Renames propagated to `global.h`, `desktop.c`,
and `main.c`.

Rules: extract functions, add forward declarations, update CMake. No behavioral
changes. Static/file-scope globals may need to become shared or passed as parameters.

### 2C-4. Identifier renames ✓

All identifiers in the codebase are now English snake_case. Completed in
four sub-phases across multiple sessions:

**2C-4a:** 76 PascalCase/Spanish functions → snake_case (25 files).
All 18 collisions resolved via module prefixes (`sound_`, `fli_`, `fpg_`,
`pal_`, `fmt_`).

**2C-4b:** File-local vars, struct fields, params → English (45 files,
~9,160 lines). Compiler symbols, editor state, runtime globals, paint
module, handler module all renamed.

**2C-4c (2026-03-12):** Final cleanup — ~125 PascalCase functions + ~120
Spanish identifiers renamed. 8 parallel agents in 2 waves:
- Wave 1 (HIGH): divpcm functions/structs, divfont/ifs typedefs+fields,
  fpgfile struct fields, global.h/inter.h globals (paleta→palette,
  tipo→file_types, boton→draw_button, interprete→interpreter, etc.)
- Wave 2 (LOW): all remaining locals across divc, divedit, divcolor,
  divbrow, divhelp, ifs, runtime, divpcm, divpalet, divfpg, divfont,
  div, divwindo. Cross-file: `max_archivos`→`MAX_FILES` (8 files).

### 2C-5. Documentation pass ✓

Updated all docs to match the 2C-2 reorganization and 2C-4 identifier renames:

- [x] README.md — source tree updated with new module layout
- [x] `docs/architecture-overview.md` — file paths + identifier names updated
- [x] `docs/compiler-pipeline.md` — file paths + identifier names updated
- [x] `docs/vm-and-runtime.md` — file paths + identifier names updated
- [x] `docs/sdl3-migration-report.md` — file paths + function/struct names updated
- [x] `docs/snake-case-collision-report.md` — all file paths updated
- [x] `docs/archive/glossary-spanish-english.md` — restructured with old→new name mapping
- [x] MEMORY.md — directory layout, function locations, bug references updated

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
