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

## Recommended next session: Sprint F (continue) + Sprint E (parallel track)

**Sprint D** is done. **Sprint F** (Spanish→English function names) is in progress:
- Batch 1 done: divclock.c, divbrush.c, diveffec.c, divbasic.c, divcolor.c (18 renames)
- Batch 2 done: divbrow.c, divfpg.c, divedit.c, divhandl.c (112 renames across 20 files)
- Next: the monsters — div.c, divpaint.c, divc.c, then runtime files
- Sprint E (unsafe strings) can slot in anywhere as a parallel track.

---

## Sprint menu — pick one or more

### Sprint A: Fix remaining 626 LOW warnings ✅ DONE (2026-03-10)

Completed: 626→0 warnings across 35 files. All cosmetic (braces, parens, casts).
No behavioral changes. Build is fully warning-clean.

---

### Sprint B: Resolve all OJO markers ✅ DONE (2026-03-10)

Completed: 47 OJO markers resolved across the entire codebase (0 remaining).
6 fixed with code (lenguaje.div check, malloc guards, partial file cleanup),
~22 converted to English WARNING/NOTE, ~12 to TODO, ~7 removed (obsolete).

---

### Sprint C: English comments on top 22 functions ✅ DONE (2026-03-10)

Completed: 22 key functions documented with English comment blocks.
Corrections: `get_token()`→`lexico()`, `make_ghost()`→`crear_ghost()`,
`editar()`→`editor()`, `menu_click()`→`menu_principal2()` (in divhandl.c).
`insertar_nombre()` doesn't exist standalone — logic inline in `lexico()`.

---

### Sprint D: Rename the `r,g,b,c,d,a` generic globals ✅ DONE (2026-03-10)

Completed: All 7 single-letter globals removed from global.h (r,g,b,c,d,a + FILE *f).
- `FILE *f`: 8 functions needed local declarations added (all others already had them)
- `int a`: 13 functions (2 in div.c/divbasic.c, 11 in divpaint.c)
- `int d`: 1 function (mab_aclarar in divpaint.c)
- `int c`: 5 functions across 3 files
- `int r,g,b`: all usages confined to divpalet.c — converted to file-static variables
  (palette functions communicate r,g,b as implicit params between callees)
- 11 files changed, zero warnings, all 4 targets build clean.

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

**Batch 1 — small files** ✅ (done 2026-03-10):
1. `divclock.c` — muestra_reloj → show_clock
2. `divbrush.c` — 5 M3D_* functions renamed
3. `divbasic.c` — 6 functions (rectangulo, salvaguarda, fondo_edicion, etc.)
4. `diveffec.c` — 3 functions (crear_puntos, avanzar_puntos, pintar_explosion)
5. `divcolor.c` — 3 functions (clexico, col_analiza_*)

**Batch 2 — medium files** ✅ (done 2026-03-10):
6. `divbrow.c` — 14 functions (crear_thumbs, muestra_thumb, pinta_listboxbr, etc.)
7. `divfpg.c` — 17 functions (nuevo_fichero, abrir_fichero, cierra_fpg, FPG_crear_*, etc.)
8. `divedit.c` — 53 functions (programa*, f_marcar/desmarcar/cortar/pegar, buscar_texto,
   sustituir_texto, guardar_prg, abrir_programa, lista_procesos, etc.)
9. `divhandl.c` — 28 functions (crear_menu, pinta_menu, actualiza_menu, determina_*,
   nuevo_mapa, crear_listbox, analizar_input, etc.)
   Total: 112 renames across 20 files, zero warnings, all 4 targets build clean.

**Next — the monsters** (these need structural understanding):
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
