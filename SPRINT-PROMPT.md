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

## Recommended next session: Sprint B + C (parallel)

**Sprint B** (OJO markers) and **Sprint C** (English comments) can run together:
- C is comments-only (zero risk), B is minimal fixes — no file conflicts
- Both build context that makes Sprint D (globals) and F (renames) safer later
- After B+C, do Sprint D (globals) then F (renames, starting small)
- Sprint E (unsafe strings) can slot in anywhere as a parallel track

---

## Sprint menu — pick one or more

### Sprint A: Fix remaining 626 LOW warnings ✅ DONE (2026-03-10)

Completed: 626→0 warnings across 35 files. All cosmetic (braces, parens, casts).
No behavioral changes. Build is fully warning-clean.

---

### Sprint B: Resolve all 27 OJO markers

**Goal:** Address every "OJO" (watch-out) comment left by the original developer.

**OJO locations** (grep for `OJO` in src/):
- `div.c:492` — missing error if `lenguaje.div` not found → add file-exists check
- `div.c:3395`, `divmouse.c:549` — malloc edge case for `barra` buffer
- `divbasic.c:188,727` — background repaint + generic flag
- `divbrow.c:191,1689` — performance tradeoff + incomplete CTRL/SHIFT
- `divc.c` (10 OJOs) — compiler design questions, bounds checks, optimization hints
- `divcolor.c:130` — EOL/EOF handling
- `divedit.c:1022,2608` — error messages, file cleanup
- `divforma.c:1119` — palette save/restore contract
- `divhandl.c:2715,2794,2864` — decompression errors, file cleanup, magic values
- `divinsta.c:864` — unimplemented protected install mode
- `divpaint.c:1793` — undo+backspace refresh bug

**Method:** For each OJO:
1. Read the surrounding code (±50 lines) to understand context
2. Decide: fix it, document it, or remove the OJO if it's no longer relevant
3. If fixing, keep the change minimal — no refactoring
4. If it's a design question with no clear answer, convert to a `// TODO:` with English explanation

---

### Sprint C: English comments on top 20 functions

**Goal:** Add a 2-4 line English comment block above each function explaining what it
does, its key parameters, and any non-obvious behavior.

**Priority functions** (most impactful for future contributors):

IDE core:
- `div.c` → `entorno()` (main IDE loop), `inicializacion()`, `finalizacion()`
- `div.c` → `menu_click()`, `crear_menu()`, `actualiza_menu()`
- `divmouse.c` → `read_mouse2()` (event pump + mouse state)
- `divedit.c` → `editar()` (code editor main loop)

Compiler:
- `divc.c` → `compilar()` (top-level compile entry), `compilar2()`, `expresion()`
- `divc.c` → `get_token()` (lexer), `insertar_nombre()` (symbol table)

Runtime:
- `runtime/i.c` → `interprete()` (VM main loop), `frame_start()`, `carga_pila()`
- `runtime/s.c` → `pinta_sprite()` (sprite blitter), `make_ghost()` (transparency)
- `runtime/f.c` → top-level built-in function dispatcher

Rendering:
- `shared/run/v.c` → `volcadosdl()` (blit 8-bit surface to SDL texture)
- `shared/osdep/osd_sdl2.c` → `OSDEP_Flip()`, `OSDEP_SetVideoMode()`, `OSDEP_SetPalette()`

**Rules:**
- Use the glossary and architecture docs to understand what each function does
- Comment style: `/* brief description \n * detail \n */` above the function
- Don't add comments to functions that are already self-explanatory
- Don't modify any code — comments only

---

### Sprint D: Rename the `r,g,b,c,d,a` generic globals

**Goal:** Eliminate the worst globals that even the original code flags as "OJO! Quitarlos"
(watch out — remove these).

**Target:** `global.h` line ~519: `GLOBAL_DATA int r,g,b,c,d,a;` and `GLOBAL_DATA FILE *f;`

**Method:**
1. For each global (`r`, `g`, `b`, `c`, `d`, `a`, `f`), grep every usage across the codebase
2. In each function that uses them, replace with a properly-named local variable
3. Once all usages are replaced, remove the global declaration from `global.h`

**Difficulty:** Medium-High. These are used as throwaway counters/temps in dozens of
functions. The names `r`, `g`, `b` are particularly tricky because they're also used
meaningfully for red/green/blue in palette code — you need to distinguish those from
the generic counter usage.

**Rules:**
- Every rename must be project-wide — no file can still reference the old global
- Name locals descriptively: `row`, `col`, `idx`, `count`, `red`, `green`, `blue`, etc.
- Build after each global is fully removed to confirm
- `f` (FILE* global) → replace with local `FILE *file` or `FILE *fp` in each function

---

### Sprint E: Convert remaining unsafe strings (by file)

**Goal:** Convert strcpy/sprintf/strcat to safe helpers in the highest-density files.

**Top targets by unsafe call count:**
| File | strcpy | sprintf | strcat | Total |
|------|--------|---------|--------|-------|
| divhandl.c | 108 | 8 | 24 | 140 |
| div.c | 108 | 4 | 20 | 132 |
| runtime/debug/decompiler.c | 78 | 52 | 17 | 147 |
| divinsta.c | 78 | 13 | 37 | 128 |
| runtime/debug/d.c | 57 | 46 | 58 | 161 |
| divfont.c | 33 | 8 | 23 | 64 |

**Safe helpers** (already in `src/div_string.h`):
- `div_strcpy(dst, sizeof(dst), src)` — bounded copy
- `div_strcat(dst, sizeof(dst), src)` — bounded concatenation
- `div_snprintf(dst, sizeof(dst), fmt, ...)` — bounded format

**Rules:**
- Use `sizeof(dst)` when dst is a stack array — this is the common case
- For heap-allocated or pointer-passed buffers, you need to find the allocation size
- Don't change behavior — just add bounds checking
- One file per agent, build after each file

---

### Sprint F: Rename Spanish functions to English (one file at a time)

**Goal:** Translate function names from Spanish to English using the glossary.

**Start with small, self-contained files** (to establish the pattern):
1. `divclock.c` (~100 lines) — clock widget, very isolated
2. `divbrush.c` (~300 lines) — brush browser, few cross-references
3. `divbasic.c` (~800 lines) — calculator/expression evaluator
4. `diveffec.c` (~500 lines) — screen transition effects
5. `divcolor.c` (~600 lines) — palette editor

**Then move to larger files**, one per sprint:
6. `divbrow.c` (2,066 lines) — file browser
7. `divfpg.c` (2,112 lines) — FPG editor
8. `divedit.c` (3,344 lines) — code editor
9. `divhandl.c` (3,515 lines) — window/dialog handler

**Leave the monsters for last** (these need structural understanding):
10. `div.c` (4,900 lines), `divpaint.c` (4,949 lines), `divc.c` (7,795 lines)

**Rules:**
- Use the glossary (`reports/glossary-spanish-english.md`) as the primary reference
- Every rename must be project-wide: rename the function, its declaration, and ALL call sites
- Update any comments that reference the old name
- Build after each rename to confirm nothing broke
- Don't rename functions that are part of the OSDEP interface (those are already English)
- Don't rename struct field names yet (that's a separate, more invasive sprint)

**Common translations:**
| Spanish | English | Notes |
|---------|---------|-------|
| crear_* | create_* | |
| borrar_* | delete_* | |
| buscar_* | find_* / search_* | |
| pintar_* / dibujar_* | draw_* / paint_* | |
| cargar_* | load_* | |
| guardar_* | save_* | |
| cerrar_* / abrir_* | close_* / open_* | |
| leer_* / escribir_* | read_* / write_* | |
| poner_* / quitar_* | set_* / remove_* | |
| mover_* / copiar_* | move_* / copy_* | |
| inicializ* | init_* / initialize_* | |
| finaliz* | finalize_* / cleanup_* | |
| detectar_* | detect_* | |
| mostrar_* / ocultar_* | show_* / hide_* | |
| cambiar_* | change_* / switch_* | |
| seleccion* | select_* | |
| comprob* | check_* / verify_* | |
| descomprim* | decompress_* | |
| actualiz* | update_* / refresh_* | |
| entorno | environment / main_loop | context-dependent |
| volcado | blit / flip | screen dump |
| tecla | key / keypress | |
| raton | mouse | |
| ventana | window | |

---

## Agent team methodology

For any sprint, use parallel agents where possible:

```
Agent 1: File A (read → modify → build-verify)
Agent 2: File B (read → modify → build-verify)
Agent 3: File C (read → modify → build-verify)
```

**Independence rule:** Two agents can work in parallel ONLY if they modify different
files. If a rename touches multiple files (Sprint D, F), it must be done sequentially
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
6. **No new files** unless splitting a large file (Sprint G, future).
7. **Commit after each sprint** with a descriptive message.
