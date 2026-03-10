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

## Recommended next session: Sprint G (rename Spanish globals + struct fields)

**All naming sprints complete so far:** Sprint D (single-letter globals), Sprint F (331 function renames).
The next natural step is renaming the 59 Spanish-named global variables and ~30 Spanish
struct field names. These identifiers flow through the entire codebase and are the biggest
remaining readability barrier after functions.

**After Sprint G:** Sprint H (translate Spanish comments), Sprint E (unsafe strings),
Sprint I (file splitting).

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

### Sprint F: Rename Spanish functions to English ✅ DONE (2026-03-10)

Completed: 331 function renames across 4 batches (22 files, touching 39+ files total).
- Batch 1: 5 small files (18 renames)
- Batch 2: 4 medium files (112 renames)
- Batch 3: 3 monster files — div.c, divpaint.c, divc.c (105 renames)
- Batch 4: 10 runtime files (96 renames)

---

### Sprint G: Rename Spanish globals + struct fields

**Goal:** Translate the 59 Spanish-named global variables in `global.h` and ~30 Spanish
struct field names across 8 major structs to English. This is the natural continuation
of Sprint F (functions → globals → fields).

**Part 1 — Global variables** (~59 renames, touches 30-40 files)

These are declared as `GLOBAL_DATA` in `global.h` and referenced throughout the codebase.

| Spanish global | → English | Notes |
|----------------|-----------|-------|
| `vga_an` | `vga_width` | VGA mode width |
| `vga_al` | `vga_height` | VGA mode height |
| `map_an` | `map_width` | current map width |
| `map_al` | `map_height` | current map height |
| `tapiz_an` | `wallpaper_width` | desktop background width |
| `tapiz_al` | `wallpaper_height` | desktop background height |
| `font_an` | `font_width` | font character width |
| `font_al` | `font_height` | font character height |
| `editor_font_an` | `editor_font_width` | editor font char width |
| `editor_font_al` | `editor_font_height` | editor font char height |
| `barra_an` | `toolbar_width` | toolbar width |
| `barra_x` | `toolbar_x` | toolbar x position |
| `barra_y` | `toolbar_y` | toolbar y position |
| `tapiz` | `wallpaper` | desktop background bitmap |
| `mapa_tapiz` | `wallpaper_map` | wallpaper tile map |
| `copia` | `screen_copy` | virtual screen copy |
| `mab` | `selection_mask` | bitmap mask for selection |
| `ghost` | `ghost` | keep — already English |
| `dac` | `palette` | check collision with existing |
| `dac4` | `palette4` | check collision |
| `cuad` | `color_lookup` | squared-difference palette table |
| `reglas[]` | `gradients[]` | color gradient array |
| `regla` | `gradient` | current gradient index |
| `near_regla` | `nearest_gradient` | nearest color lookup |
| `undo` | `undo` | keep — already English |
| `volcado_completo` | `full_redraw` | flag: full screen redraw needed |
| `siguiente_orden` | `next_order` | next window z-order |
| `modo_de_retorno` | `return_mode` | return mode flag |
| `modo_caja` | `mode_rect` | rectangle drawing mode |
| `modo_circulo` | `mode_circle` | circle drawing mode |
| `modo_fill` | `mode_fill` | keep — already English |
| `modo_seleccion` | `mode_selection` | selection mode flag |
| `zoom_background` | `zoom_background` | keep — already English |
| `old_mouse_b` | `prev_mouse_buttons` | previous mouse button state |
| `fondo_raton` | `mouse_background` | mouse background buffer |
| `max_undos` | `max_undos` | keep — already English |
| `undo_memory` | `undo_memory` | keep — already English |
| `tundo` | `undo_table` | undo table struct |
| `iundo` | `undo_index` | undo index |
| `zx`, `zy`, `zan`, `zal` | `zoom_x`, `zoom_y`, `zoom_width`, `zoom_height` | zoom window |

NOTE: Some globals above (ghost, undo, max_undos, etc.) are already English — skip those.
Check for name collisions before each rename (grep the target name first).
The `dac`/`dac4` renames need careful collision analysis — `palette` may already exist.

**Part 2 — Struct field names** (~30 renames across 8 structs, touches 35+ files)

| Struct | Field | → English | Notes |
|--------|-------|-----------|-------|
| `tprg` | `linea` | `line` | current line |
| `tprg` | `columna` | `column` | current column |
| `tprg` | `num_lineas` | `num_lines` | line count |
| `tprg` | `linea_vieja` | `prev_line` | previous line |
| `tprg` | `primera_linea` | `first_line` | first visible line |
| `tprg` | `primera_columna` | `first_column` | first visible column |
| `tprg` | `buffer_lon` | `buffer_len` | buffer length |
| `tprg` | `file_lon` | `file_len` | file length |
| `tmapa` | `map_an` | `map_width` | map width |
| `tmapa` | `map_al` | `map_height` | map height |
| `tmapa` | `codigo` | `code` | map identifier code |
| `tmapa` | `Codigo` | `fpg_code` | FPG code |
| `tmapa` | `descripcion` | `description` | map description |
| `tmapa` | `grabado` | `saved` | saved-to-disk flag |
| `tventana` | `tipo` | `type` | window type |
| `tventana` | `orden` | `order` | z-order |
| `tventana` | `primer_plano` | `foreground` | foreground flag |
| `tventana` | `nombre` | `name` | icon name |
| `tventana` | `titulo` | `title` | window title |
| `tventana` | `estado` | `state` | button/item state |
| `tventana` | `botones` | `buttons` | pressed buttons |
| `tventana` | `volcar` | `redraw` | needs-redraw flag |
| `t_listbox` | `lista` | `list` | list pointer |
| `t_listbox` | `lista_an` | `item_width` | item width |
| `t_listbox` | `lista_al` | `visible_items` | visible item count |
| `t_listbox` | `inicial` | `first_visible` | first visible index |
| `t_listbox` | `maximo` | `max_items` | total item count |
| `t_listbox` | `zona` | `zone` | selected zone |
| `t_listbox` | `botones` | `buttons` | up/down pressed |
| `t_listbox` | `creada` | `created` | created flag |
| `tipo_regla` | `numcol` | `num_colors` | number of colors |
| `tipo_regla` | `tipo` | `type` | gradient type |
| `tipo_regla` | `fijo` | `fixed` | fixed flag |
| `tipo_regla` | `col[33]` | `colors[33]` | color array |
| `tipo_undo` | `codigo` | `code` | map identifier |
| `tipo_undo` | `modo` | `mode` | entry mode |
| `ttipo` | `defecto` | `default_choice` | default selection |

**Collision warnings:**
- `tprg.linea` → `line` — very common word; grep carefully for false positives
- `tmapa.codigo` / `tipo_undo.codigo` → `code` — same concern
- `tventana.tipo` / `tipo_regla.tipo` → `type` — these are struct fields so scope is limited
- Struct field renames are safer than globals (no global namespace collision possible)

**Rules (same as Sprint F plus):**
- Rename the declaration in the struct/global AND all access sites project-wide
- For struct fields, search `structvar.fieldname` and `ptr->fieldname` patterns
- For globals, search the bare name (they're accessed directly)
- Be very careful with short names like `an`, `al` — they may appear as substrings
  (e.g., `man`, `plan`, `canal`). Use word-boundary-aware search.
- Build after each batch to confirm nothing broke
- Do globals first (Part 1), then struct fields (Part 2)
- Batch by struct or by naming pattern (all `*_an`/`*_al` → `*_width`/`*_height` together)

---

### Sprint H: Translate Spanish comments to English

**Goal:** Translate ~3,663 lines of Spanish comments across 47 files to English.

**Top targets by Spanish comment density:**
| File | Spanish comment lines | % of file |
|------|-----------------------|-----------|
| divc.c | 777 | 9% |
| runtime/f.c | 295 | 6% |
| global.h | 273 | — |
| runtime/debug/d.c | 269 | — |
| divedit.c | 264 | 7% |
| div.c | 252 | 5% |
| divpaint.c | 203 | 4% |
| divhandl.c | 196 | 5% |

**Rules:**
- Translate the meaning, don't transliterate (e.g., "pone el color" → "set the color",
  not "puts the color")
- Keep the same comment style (// or /* */) and position
- Don't add comments where none exist — only translate existing ones
- This sprint is highly parallelizable: each file is independent
- One agent per file, build after each file

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
