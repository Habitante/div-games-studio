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

## Recommended next session: Sprint H (translate Spanish comments to English)

**All naming sprints complete so far:** Sprint D (single-letter globals), Sprint F (331
function renames), Sprint G (~131 global + struct field renames). The identifiers are now
largely English. The biggest remaining readability barrier is the ~3,663 lines of Spanish
comments across 47 files.

**After Sprint H:** Sprint E (unsafe strings), Sprint I (file splitting), then the
deferred high-occurrence renames (copia, ventana, texto).

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

---

### Sprint D: Rename the `r,g,b,c,d,a` generic globals ✅ DONE (2026-03-10)

Completed: All 7 single-letter globals removed from global.h (r,g,b,c,d,a + FILE *f).
11 files changed, zero warnings, all 4 targets build clean.

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

### Sprint G: Rename Spanish globals + struct fields ✅ DONE (2026-03-10)

Completed: ~131 identifier renames across 8 batches, 44 files, ~6,900 line changes.

**Part 1 — Global variables (~98 renames):**
- Width/height: `vga_an`→`vga_width`, `map_an`→`map_width`, `font_an`→`font_width`,
  `tapiz_an`→`wallpaper_width`, `barra_an`→`toolbar_width`, etc.
- Wallpaper: `tapiz`→`wallpaper`, `mapa_tapiz`→`wallpaper_map`
- Modes: `modo_caja`→`mode_rect`, `modo`→`draw_mode`, `volcado_completo`→`full_redraw`
- Control: `salir_del_entorno`→`exit_requested`, `fin_dialogo`→`end_dialog`
- Palette: `reglas`→`gradients`, `regla`→`gradient`, `cuad`→`color_lookup`
- Zoom: `zx`→`zoom_win_x`, `hacer_zoom`→`need_zoom`
- Selection: `mab`→`selection_mask` + all compounds
- Dialog: all `v_titulo`→`v_title`, `v_texto`→`v_text`, etc.
- Misc: `ejecutar_programa`→`run_mode`, `barra`→`toolbar`, `mascara`→`file_mask`, etc.

**Part 2 — Struct fields (~33 renames):**
- `tprg`: `linea`→`line`, `columna`→`column`, `num_lineas`→`num_lines`, etc.
- `tmapa`: `codigo`→`code`, `Codigo`→`fpg_code`, `descripcion`→`description`
- `tventana`: `tipo`→`type`, `volcar`→`redraw`, `titulo`→`title`, etc.
- `t_listbox`: `lista`→`list`, `maximo`→`total_items`, `zona`→`zone`, etc.
- `tipo_regla`: `numcol`→`num_colors`, `col`→`colors`
- `tipo_undo`: `modo`→`mode`; `ttipo`: `defecto`→`default_choice`

**Collisions handled:** `gradient` vs `dither_pattern`, `total_items` (not `max_items`),
`fpg_code` (not `code`). **Kept as-is:** `dac`/`dac4` (VGA DAC term, `palette` collision).

**Deferred (high-occurrence, need special handling):**
- `copia` (375 occurrences — also used as function parameter name in divwindo.c)
- `ventana` (619 occurrences across 26 files — the window array)
- `texto` (905 occurrences across 37 files — UI text array)

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

### Sprint J: Rename deferred high-occurrence globals

**Goal:** Rename the 3 Spanish globals deferred from Sprint G due to their high
occurrence counts and entanglement with function parameters.

| Global | Occurrences | Challenge |
|--------|-------------|-----------|
| `copia` → `screen_copy` | 375 across 22 files | Also used as parameter name in divwindo.c functions (`byte *copia`) — need to rename params too, plus `an_copia`/`al_copia` |
| `ventana` → `windows` | 619 across 26 files | The window array. `#define v ventana[0]` must update too. Pervasive. |
| `texto` → `texts` | 905 across 37 files | UI string array. Also used as struct field in `t_item.button.texto` etc. |

**Rules:**
- For `copia`: also rename `copia_surface`→`screen_copy_surface`, and the function
  parameter `byte *copia` in divwindo.c to `byte *dest` (it's a destination buffer)
- For `ventana`: update `#define v ventana[0]` → `#define v windows[0]`
- For `texto`: rename struct fields `t_item.button.texto` etc. too
- Each rename is one batch — build after each

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
