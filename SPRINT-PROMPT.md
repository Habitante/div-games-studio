# DIV Games Studio — Modernization Sprint

You are working on DIV Games Studio, a 1990s DOS game-creation IDE ported to SDL2.
~85K lines of C, originally written in Spanish by Daniel Navarro at Hammer Technologies.
The codebase compiles and runs on Windows (32-bit MinGW). All 4 targets build clean.

## Before you start

Read these files to understand the codebase (use Agent/Explore for parallel reads):

1. `ROADMAP.md` — project vision, completed phases, what's next
2. `reports/architecture-overview.md` — IDE startup, main loop, OSDEP layer, file formats
3. `reports/compiler-pipeline.md` — DIV compiler: lexer, parser, 127-opcode bytecode
4. `reports/vm-and-runtime.md` — stack VM, process scheduling, rendering pipeline
5. `reports/glossary-spanish-english.md` — ~150 Spanish identifiers translated to English
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

## Completed sprints (for reference)

- **Sprint A** (2026-03-10): 626 LOW warnings → 0 across 35 files
- **Sprint B** (2026-03-10): 47 OJO markers → 0 across entire codebase
- **Sprint C** (2026-03-10): 22 key functions documented with English comments
- **Sprint D** (2026-03-10): 7 single-letter globals removed from global.h
- **Sprint E** (2026-03-11): ~1,030 unsafe string calls → safe helpers across 31 files
  (5 exceptions: 4 in divinsta.c heap ptrs, 1 in 3rd-party zip.c)
- **Sprint F** (2026-03-10): 331 Spanish function names → English across 39+ files
- **Sprint G** (2026-03-10): ~131 globals + struct fields renamed, 44 files, ~6,900 lines
- **Sprint H** (2026-03-10): ~3,170 Spanish comment lines → English across 34 files
- **Sprint J** (2026-03-11): 3 high-frequency globals renamed, 39 files, ~4,034 lines
  (copia→screen_buffer, ventana→window, texto→texts)

---

## Recommended next session: Sprint I (file splitting)

The codebase is now predominantly English, warning-clean, and string-safe.
The main remaining structural issue is the 3 monster files.

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
- Unify byte types: pick `uint8_t` everywhere, stop mixing `byte`/`char`/`uchar`
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
- `reports/glossary-spanish-english.md` (for translations)
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
