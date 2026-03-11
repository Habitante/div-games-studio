# DIV Games Studio — Modernization Sprint

You are working on DIV Games Studio, a 1990s DOS game-creation IDE ported to SDL2.
~85K lines of C, originally written in Spanish by Daniel Navarro at Hammer Technologies.
The codebase compiles and runs on Windows (32-bit MinGW). All 4 targets build clean.

## Before you start

Read these files to understand the codebase (use Agent/Explore for parallel reads):

1. `ROADMAP.md` — project vision, completed phases, what's next
2. `docs/architecture-overview.md` — IDE startup, main loop, OSDEP layer, file formats
3. `docs/compiler-pipeline.md` — DIV compiler: lexer, parser, 127-opcode bytecode
4. `docs/vm-and-runtime.md` — stack VM, process scheduling, rendering pipeline
5. `docs/archive/glossary-spanish-english.md` — ~150 Spanish identifiers translated to English
6. `src/global.h` — all 180+ globals, type aliases, macros
7. `src/div_string.h` — safe string helpers already available

## Build & verify

```bash
export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"
cd /c/Src/div/build
cmake -G "MinGW Makefiles" -DTARGETOS=WINDOWS-NATIVE -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev ..
mingw32-make -j$(nproc)
```

All 4 targets must build: `d`, `div`, `divrun`, `divdbg`. After any change, rebuild and
confirm zero warnings. Current baseline: **0 warnings** (Sprint A completed 2026-03-10).

---

## Current state and next priorities

**All translation + safety sprints complete:** Sprint A (warnings), B (OJO markers),
C (function comments), D (single-letter globals), E (unsafe strings — 1,030 calls),
F (331 function renames), G (~131 global/struct renames), H (~3,170 comment translations),
J (copia/ventana/texto renames).

**Next: Phase 2B modernization** — see `plans/cozy-fluttering-sun.md` or discuss with Daniel.
Priorities: dead code removal (135 #ifdef blocks), source reorganization into modules,
file renaming, monster file splitting (divc.c, div.c, divpaint.c).

---

## Completed sprints (reference)

| Sprint | Date | What | Scale |
|--------|------|------|-------|
| A | 2026-03-10 | 626 LOW warnings → 0 | 35 files |
| B | 2026-03-10 | 47 OJO markers → 0 | 6 code fixes, ~22 WARNING/NOTE, ~12 TODO |
| C | 2026-03-10 | 22 key functions documented | English comment blocks |
| D | 2026-03-10 | 7 single-letter globals removed | 11 files |
| E | 2026-03-11 | ~1,030 unsafe string calls → safe helpers | 31 files, 2 overflows found |
| F | 2026-03-10 | 331 Spanish function names → English | 39+ files |
| G | 2026-03-10 | ~131 globals + struct fields → English | 44 files, ~6,900 lines |
| H | 2026-03-10 | ~3,170 Spanish comments → English | 34 files |
| J | 2026-03-11 | 3 high-frequency globals renamed | 39 files, ~4,034 lines |

## Safe string helpers reference (from Sprint E)

**Helpers** (in `src/div_string.h`):
- `div_strcpy(dst, sizeof(dst), src)` — bounded copy
- `div_strcat(dst, sizeof(dst), src)` — bounded concatenation
- `div_snprintf(dst, sizeof(dst), fmt, ...)` — bounded format
- `DIV_STRCPY`/`DIV_STRCAT`/`DIV_SPRINTF` — macro versions using sizeof automatically

**Rules for future string work:**
- Use `sizeof(dst)` when dst is a stack array
- For heap-allocated or pointer-passed buffers, find the allocation size
- For overlapping buffers, use `memmove()` instead
- Don't change behavior — just add bounds checking

---

### Sprint I: Split monster files

**Goal:** Split the 3 largest files along natural boundaries identified in the
architecture docs.

| File | Lines | Proposed split |
|------|-------|----------------|
| `divc.c` | 7,823 | `divc_lexer.c` + `divc_parser.c` + `divc_codegen.c` |
| `divpaint.c` | 4,969 | `divpaint.c` + `divpaint_tools.c` + `divpaint_select.c` |
| `div.c` | 4,940 | `div.c` + `div_desktop.c` + `div_dialogs.c` |

**Rules:**
- Extract functions into new files, add forward declarations in headers
- Update CMakeLists.txt to compile the new files
- No behavioral changes — pure structural refactoring
- Static/file-scope globals may need to become shared or passed as parameters

---

### Future work (lower priority)

- Rename cryptic locals in hottest paths (divc.c, divedit.c, runtime/i.c)
- Unify byte types: pick `uint8_t` everywhere, stop mixing `byte`/`char`/`uchar` (warning, some code might rely on having some signed or unsigned bytes)
- Fix or remove commented-out `free()` in runtime stack management (i.c:778)
- Audit `PrintEvent` pattern for similar `#ifdef`-body bugs
- Re-enable `test_video` startup dialog
- DPI-aware rendering (`SDL_WINDOW_ALLOW_HIGHDPI`)

---

## Agent team methodology

For any sprint, use parallel agents where possible:

```
Agent 1: File A (read → modify → build-verify)
Agent 2: File B (read → modify → build-verify)
Agent 3: File C (read → modify → build-verify)
```

**Independence rule:** Two agents can work in parallel ONLY if they modify different
files. If a rename touches multiple files (Sprint D, F, G), it must be done sequentially
or by a single agent.

**Context loading:** Each agent should read:
- The target file
- `docs/archive/glossary-spanish-english.md` (for translations)
- `src/global.h` (for type/macro context)
- Any header the target file includes

## Hard constraints (never violate these)

1. **Must compile.** All 4 targets. Zero warnings (baseline is 0 as of Sprint A).
2. **No behavioral changes.** If a function is buggy, file it separately — don't fix it
   during a rename/cleanup sprint.
3. **Project-wide consistency.** No half-renamed identifiers. If you rename `crear_menu`
   to `create_menu`, every single reference in the entire codebase must change.
4. **32-bit only.** `sizeof(int) == sizeof(void*)`. Don't introduce 64-bit assumptions.
5. **UTF-8 source files.** All source files are UTF-8. Use `\xNN` for high bytes in
   string/char literals (lookup tables, font data).
6. **No new files** unless splitting a large file (Sprint I).
7. **Commit after each sprint** with a descriptive message.
