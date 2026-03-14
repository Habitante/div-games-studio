# DIV Games Studio — Improvement Plan

Architectural improvements ordered by priority. Front-loads low-risk,
high-payoff work; defers riskier changes until the safety net (tests,
enums, docs) is in place.

Written 2026-03-14 after a full read of the codebase (~87K lines),
architecture docs, code review, and discussion with Daniel Navarro.

---

## Progress

| # | Improvement | Status | Risk |
|---|-------------|--------|------|
| 1 | [Enums & constants](#1-enums--constants) | Sprints 1-2 DONE, sprints 3-6 open | Very low |
| 2 | [Testing infrastructure](#2-testing-infrastructure) | — | None |
| 3 | [File format hardening + modern imports](#3-file-format-hardening--modern-imports) | — | Low |
| 4 | [Help translation quality pass](#4-help-translation-quality-pass) | — | Low |
| 5 | [IDE graphics & fonts to FPG/FNT](#5-ide-graphics--fonts--self-hosted-resources) | — | Low |
| 6 | [Compiler single source of truth](#6-compiler-single-source-of-truth) | — | Low |
| 7 | [State machine cleanup](#7-make-implicit-state-machines-explicit) | — | Very low |
| 8 | [Editor modernization](#8-code-editor-modernization) | — | Medium |
| 9 | [OSDEP thickening](#9-thicken-the-osdep-abstraction-layer) | — | Medium |
| 10 | [Data-driven config](#10-data-driven-ide-configuration) | — | Low |
| 11 | [User manual modernization](#11-user-manual-modernization) | — | Low |
| 12 | [Compiler modernization](#12-compiler-architecture-modernization) | — | Medium-High |
| 13 | [Window/handler streamlining](#13-streamline-the-windowhandler-architecture) | — | Medium |

---

## 1. Enums & constants

**Problem:** The codebase is full of hardcoded values that make the code
fragile and hard to read:

- Window types: `v.type = 102` (what's 102? a code editor)
- Menu text indices: `create_menu(800)` (what's 800? the Maps menu)
- Mouse cursor IDs: `mouse_graf >= 10` (magic threshold for "on canvas")
- `draw_mode < 100` vs `>= 100` (tool transition threshold)
- Palette size `768` appears 50+ times with no named constant
- UI pixel offsets (+7, +23, x9) scattered through menu/window code
- Process field offsets (`_X=26`, `_Y=27`) as raw numbers in `inter.h`
- File type indices (2=MAP, 3=PAL, 4=FPG...) used as bare integers

### Complete inventory

Below is every category of magic number identified from the architecture
docs, the IDE guide, and the editor overviews. Each category lists the
values, their meaning, which files use them, and the enum or constant
name. Categories are grouped by sprint.

#### A. Window types (`v.type`) — DONE

Defined in `div_enums.h`, included by `global.h` and `debugger_internal.h`.

| Value | Meaning | Name |
|-------|---------|------|
| 0 | Empty/inactive slot | `WIN_EMPTY` |
| 1 | Modal dialog | `WIN_DIALOG` |
| 2 | Dropdown menu | `WIN_MENU` |
| 3 | Palette display | `WIN_PALETTE` |
| 4 | Clock | `WIN_CLOCK` |
| 5 | Trash/recycle bin | `WIN_TRASH` |
| 7 | Progress bar | `WIN_PROGRESS` |
| 8 | Mixer | `WIN_MIXER` |
| 100 | Map/paint editor | `WIN_MAP` |
| 101 | FPG graphics editor | `WIN_FPG` |
| 102 | Code editor (PRG) | `WIN_CODE` |
| 104 | Font editor | `WIN_FONT` |
| 105 | PCM/sound editor | `WIN_SOUND` |
| 107 | Music module player | `WIN_MUSIC` |
| 100 | Editor threshold | `WIN_EDITOR_MIN` |

#### B. Window foreground state (`v.foreground`) — DONE

| Value | Meaning | Name |
|-------|---------|------|
| 0 | Background (dimmed) | `WF_BACKGROUND` |
| 1 | Foreground (active) | `WF_FOREGROUND` |
| 2 | Minimized (icon) | `WF_MINIMIZED` |

#### C. File type indices (`v_type`, `file_types[]`) — DONE

| Index | File type | Name |
|-------|-----------|------|
| 2 | MAP (images) | `FT_MAP` |
| 3 | PAL (palettes) | `FT_PAL` |
| 4 | FPG (graphic collections) | `FT_FPG` |
| 5 | FNT (fonts) | `FT_FNT` |
| 6 | IFS (font templates) | `FT_IFS` |
| 7 | Audio (WAV, PCM, OGG...) | `FT_AUDIO` |
| 8 | PRG (programs) | `FT_PRG` |
| 9 | Wallpaper images | `FT_WALLPAPER` |
| 10 | PAL save (restricted exts) | `FT_PAL_SAVE` |
| 11 | Audio save (restricted exts) | `FT_AUDIO_SAVE` |
| 14 | MAP save (restricted exts) | `FT_MAP_SAVE` |
| 16 | Tracker modules (MOD, S3M...) | `FT_MODULE` |

The `_SAVE` variants are the same file type but with a narrower
extension filter for save dialogs (e.g. MAP open accepts JPG/PNG/BMP/...,
MAP save only offers MAP/PCX/BMP). `browser.c` auto-remaps to the save
variant when `v_mode > 0`.

#### D. Dialog item types (`t_item.type`) — DONE

| Value | Widget | Name |
|-------|--------|------|
| 1 | Button | `ITEM_BUTTON` |
| 2 | Text input field | `ITEM_TEXT` |
| 3 | Checkbox/flag | `ITEM_CHECKBOX` |

#### E. Menu text base indices — DONE

Each menu's text block starts at a fixed index in the `texts[]` array.
These appear as arguments to `create_menu()`, `paint_menu()`, and
`update_menu()`.

| Base | Menu | Name |
|------|------|---------------|
| 750 | Main menu bar | `MENU_MAIN` |
| 775 | Palettes | `MENU_PALETTES` |
| 800 | Maps | `MENU_MAPS` |
| 825 | Graphics (FPG) | `MENU_GRAPHICS` |
| 850 | Fonts | `MENU_FONTS` |
| 875 | System | `MENU_SYSTEM` |
| 900 | Programs | `MENU_PROGRAMS` |
| 925 | Sounds | `MENU_SOUNDS` |
| 950 | Edit | `MENU_EDIT` |

Defined in `div_enums.h` as `enum menu_base`.

**Files:** `handler.c`.

#### F. Mouse cursor IDs (`mouse_graf`)

| Value | Meaning | Proposed name |
|-------|---------|---------------|
| 1 | Normal arrow | `CURSOR_ARROW` |
| 2 | Move/drag (title bar) | `CURSOR_MOVE` |
| 4 | Minimize button hover | `CURSOR_MINIMIZE` |
| 5 | Close button hover | `CURSOR_CLOSE` |
| 6 | Resize grip | `CURSOR_RESIZE` |
| 7 | Background window (click to activate) | `CURSOR_ACTIVATE` |
| >= 10 | Paint cursor on canvas | `CURSOR_ON_CANVAS` (threshold) |

**Files:** `main.c`, `mouse.c`, `paint.c`, `paint_tools.c`,
`paint_select.c`, `handler_map.c`.

#### G. Paint editor draw modes (`draw_mode`)

The `draw_mode` variable serves double duty: values 0-13 identify the
active tool, and values >= 100 signal tool transitions (the outer loop
subtracts 100 and dispatches).

| Value | Tool | Proposed name |
|-------|------|---------------|
| 0 | Pixels (point-and-click) | `TOOL_PIXELS` |
| 1 | Pencil/freehand strokes | `TOOL_PENCIL` |
| 2 | Lines | `TOOL_LINES` |
| 3 | Continuous lines | `TOOL_POLYLINE` |
| 4 | Bezier curves | `TOOL_BEZIER` |
| 5 | Continuous bezier | `TOOL_POLYBEZIER` |
| 6 | Rectangles | `TOOL_RECT` |
| 7 | Circles/ellipses | `TOOL_CIRCLE` |
| 8 | Spray | `TOOL_SPRAY` |
| 9 | Fill/flood | `TOOL_FILL` |
| 10 | Selection/cut | `TOOL_SELECT` |
| 11 | Undo history | `TOOL_UNDO` |
| 12 | Control points | `TOOL_CTRLPOINTS` |
| 13 | Text | `TOOL_TEXT` |
| 100 | Tool transition offset | `TOOL_TRANSITION` |

The `draw_mode < 100` / `>= 100` checks become `draw_mode < TOOL_TRANSITION`.

**Files:** `paint.c`, `paint_tools.c`, `paint_select.c`, `mouse.c`,
`handler_map.c`.

#### H. Block selection state (code editor)

| Variable | Value | Meaning | Proposed name |
|----------|-------|---------|---------------|
| `block_state` | 0 | No block | `BLOCK_NONE` |
| `block_state` | 1 | Pivot set | `BLOCK_PIVOT` |
| `block_state` | 2 | Complete | `BLOCK_COMPLETE` |
| `edit_block_mode` | 0 | No selection | `BMODE_NONE` |
| `edit_block_mode` | 1 | Character block | `BMODE_CHAR` |
| `edit_block_mode` | 2 | Line block | `BMODE_LINE` |
| `clipboard_type` | 0 | Character data | `CLIP_CHAR` |
| `clipboard_type` | 1 | Line data | `CLIP_LINE` |

**Files:** `editor.c`, `editor_edit.c`, `editor_render.c`.

#### I. Drag-and-drop state (`dragging`)

| Value | Meaning | Proposed name |
|-------|---------|---------------|
| 0 | Idle | `DRAG_IDLE` |
| 1 | Pending (mouse down) | `DRAG_PENDING` |
| 2 | Dragging (in flight) | `DRAG_ACTIVE` |
| 3 | Done (released) | `DRAG_DONE` |
| 4 | Dropping (over target) | `DRAG_DROPPING` |
| 5 | Dropped (completed) | `DRAG_DROPPED` |

**Files:** `main.c`, `handler_map.c`.

#### J. Mouse button bits (`mouse_b`)

| Bit/Value | Meaning | Proposed name |
|-----------|---------|---------------|
| bit 0 (0x01) | Left button | `MB_LEFT` |
| bit 1 (0x02) | Right button | `MB_RIGHT` |
| bit 2 (0x04) | Scroll wheel down | `MB_SCROLL_DOWN` |
| bit 3 (0x08) | Scroll wheel up | `MB_SCROLL_UP` |
| 0x8001 | Spacebar pretending to be left-click | `MB_KEYBOARD_CLICK` |

**Files:** `mouse.c`, `main.c`, `paint.c`, `paint_tools.c`,
`paint_select.c`, `editor.c`, `handler_map.c`.

#### K. Shift/modifier status bits (`shift_status`)

| Bit | Meaning | Proposed name |
|-----|---------|---------------|
| 0-1 | Shift key | `MOD_SHIFT` (mask 0x03) |
| 2 | Ctrl key | `MOD_CTRL` (0x04) |
| 3 | Alt key | `MOD_ALT` (0x08) |

**Files:** `editor.c`, `editor_edit.c`, `mouse.c`, `main.c`,
`paint_tools.c`.

#### L. Process status codes (`_Status`, runtime)

| Value | Meaning | Proposed name |
|-------|---------|---------------|
| 0 | Dead (free slot) | `PROC_DEAD` |
| 1 | Killed (pending removal) | `PROC_KILLED` |
| 2 | Alive (running) | `PROC_ALIVE` |
| 3 | Sleeping (waiting for function return) | `PROC_SLEEPING` |
| 4 | Frozen (paused, still painted) | `PROC_FROZEN` |

**Files:** `interpreter.c`, `functions.c`, `render.c`, `kernel.inc`.

#### M. Coordinate types (`_Ctype`, runtime)

| Value | Meaning | Proposed name |
|-------|---------|---------------|
| 0 | Screen coordinates | `CTYPE_SCREEN` |
| 1 | Scroll-relative | `CTYPE_SCROLL` |
| 2 | Mode 7-relative | `CTYPE_MODE7` |

**Files:** `interpreter.c`, `render.c`, `functions.c`.

#### N. Sprite flags (`_Flags`, runtime)

| Bit | Meaning | Proposed name |
|-----|---------|---------------|
| 0 | Horizontal flip | `SPRITE_HFLIP` |
| 1 | Vertical flip | `SPRITE_VFLIP` |
| 2 | Ghost (transparency blend) | `SPRITE_GHOST` |

**Files:** `render.c`.

#### O. Repeated numeric constants

| Value | Meaning | Name | Status |
|-------|---------|------|--------|
| 768 | Palette data size (256 * 3 bytes) | `PALETTE_SIZE` | — |
| 256 | Palette entry count | `PALETTE_ENTRIES` | — |
| 576 | Color rules/gradients block size | `GRADIENTS_SIZE` | — |
| 999 / 1000 | Max graphics per FPG | `MAX_FPG_GRAPHICS` | — |
| 1024 | Max line length in editor | `LONG_LINE` (already exists) | — |
| 65536 | Ghost table size (256*256) | `GHOST_TABLE_SIZE` | — |
| 16384 | Squared-difference table size | `CUAD_TABLE_SIZE` | — |
| 9 | Pixels per menu item | `MENU_ITEM_HEIGHT` | DONE |
| 11 | Menu header height | `MENU_HEADER_HEIGHT` | DONE |
| 7 / 23 | Menu item text margin / width padding | `MENU_TEXT_MARGIN` / `MENU_WIDTH_PAD` | DONE |

#### P. Button alignment modes (`_button` center parameter) — DONE

| Value | Position | Name |
|-------|----------|---------------|
| 0 | Top-left | `ALIGN_TL` |
| 1 | Top-center | `ALIGN_TC` |
| 2 | Top-right | `ALIGN_TR` |
| 3 | Middle-left | `ALIGN_ML` |
| 4 | Middle-center | `ALIGN_MC` |
| 5 | Middle-right | `ALIGN_MR` |
| 6 | Bottom-left | `ALIGN_BL` |
| 7 | Bottom-center | `ALIGN_BC` |
| 8 | Bottom-right | `ALIGN_BR` |

Defined in `div_enums.h` as `enum button_align`. Used by `_button()`,
`draw_button()`, `select_button()`, `button_status()`, `mouse_button_hit()`,
and `wwrite()` across IDE, editor, compiler, and debugger modules.

**Files:** `main_dialogs.c`, `handler_dialogs.c`, `handler_map.c`,
`handler_fonts.c`, `window.c`, `debugger_ui.c`, and every dialog init function.

#### Q. File format magic offsets

| Value | Context | Proposed name |
|-------|---------|---------------|
| 8 | Start of palette in MAP/FPG/FNT/PAL | `FMT_PALETTE_OFFSET` |
| 48 | Start of palette in MAP (after header fields) | `MAP_PALETTE_OFFSET` |
| 1352 | Start of gencode in FNT (8 + 768 + 576) | `FNT_GENCODE_OFFSET` |
| 1356 | Start of char table in FNT (1352 + 4) | `FNT_TABLE_OFFSET` |
| 1392 | Start of control points in MAP | `MAP_CTRLPTS_OFFSET` |

**Files:** `image.c`, `fpg.c`, `font.c`, `charset.c`, `browser.c`,
`handler_fonts.c`, `desktop.c`.

### Sprint plan

Each sprint is one self-contained commit.

| Sprint | Categories | Status |
|--------|-----------|--------|
| 1 — Core IDE enums | A. Window types, B. Foreground state, C. File types, D. Item types | DONE |
| 2 — Menu and UI constants | E. Menu bases, P. Alignment modes, O. Menu metrics | DONE |
| 3 — Input state enums | F. Cursor IDs, J. Mouse buttons, K. Modifiers | — |
| 4 — Editor state enums | G. Draw modes, H. Block state, I. Drag-and-drop | — |
| 5 — Palette and format constants | O. Palette/ghost/cuad sizes, `MAX_FPG_GRAPHICS`, Q. Format offsets | — |
| 6 — Runtime enums | L. Process status, M. Coordinate types, N. Sprite flags | — |

### What NOT to enumerate

- **`texts[]` string indices beyond the menu bases** — there are
  hundreds of these, they're inherently tied to `lenguaje.div` line
  numbers, and naming every error message would create more churn than
  clarity. The menu bases are worth naming because they're structural;
  individual string references within dialogs are fine as-is.
- **One-off pixel tweaks** — a `+3` padding in a single dialog paint
  function isn't worth a constant. Only extract values that appear in
  3+ places or that would confuse a newcomer.
- **Process field offsets** — `_X`, `_Y`, `_Graph`, etc. are already
  `#define` constants in `inter.h`. They're used correctly throughout.
  No work needed.

**Effort:** 1-2 days total, spread across 6 small sprints.
**Risk:** Very low — purely mechanical, no behavior change. Compiler
catches any mistakes. Each sprint is independently committable.

---

## 2. Testing infrastructure

**Problem:** Zero tests for 87K lines of C. Every change is a leap of
faith. The code is battle-tested but has no regression safety net.

**Key insight:** DIV can test itself. The inline help (`help.div`)
already contains hundreds of working example programs — one for nearly
every built-in function — and the resources they need (`help.fpg`,
`help.fnt`, `help.map`, `help.pcm`, etc.) already ship in `div/help/`.
DIV programs can write results to files via `fopen`/`fwrite`, so a test
program can run assertions and dump pass/fail counts without human
interaction.

**What to do, in order of ROI:**

### Self-hosted API test suite (highest ROI)

Write DIV programs that systematically test the runtime API — inspired
by (and harvested from) the help.div example programs, but restructured
as non-interactive tests that run, assert, and exit:

- **`test_math.prg`** — `rand()`, `abs()`, `sqrt()`, `pow()`,
  `get_distx()`/`get_disty()`, `get_angle()`, `get_dist()`, etc.
  Pure functions with known inputs/outputs, easy to validate.
- **`test_strings.prg`** — string operations, `itoa()`, `atoi()`,
  `strlen()`, `strcpy()`, `strcat()`, etc.
- **`test_processes.prg`** — process creation, `signal()`, `get_id()`,
  `collision()`, father/son/sibling relationships, priority scheduling.
  Spawn processes, run a few frames, verify state.
- **`test_files.prg`** — `fopen()`/`fread()`/`fwrite()`/`fclose()`,
  `save()`/`load()`, `encode()`. Write data, read it back, compare.
- **`test_graphics.prg`** — `load_fpg()`, `load_map()`, `load_pal()`,
  `map_info()`, `graphic_info()`, using the help resources that already
  ship with DIV.
- **`test_scroll.prg`** — `start_scroll()`, `move_scroll()`, Mode 7.
  Run a few frames, verify no crash.

Each test program follows the same pattern:

```
PROGRAM test_math;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
BEGIN
    // --- rand() ---
    FROM x = 0 TO 100;
        result = rand(1, 10);
        IF (result < 1 OR result > 10)
            failed++;
        ELSE
            passed++;
        END
    END

    // --- get_distx() at 0 degrees = full distance ---
    IF (get_distx(0, 100) == 100) passed++; ELSE failed++; END

    // ... more tests ...

    // Dump results to file for the test runner
    handle = fopen("test_math.result", "w");
    fwrite(offset passed, 1, handle);
    fwrite(offset failed, 1, handle);
    fclose(handle);

    // Exit after a few frames
    FROM x = 0 TO 3; FRAME; END
END
```

The key challenge is that most DIV programs don't have an explicit exit
condition (users Alt+X out of them). Test programs must use a counted
frame loop and then fall off the `END` to terminate cleanly.

### Test runner script

A shell script that orchestrates the test suite:

```bash
cd div
for prg in tests/test_*.prg; do
    system/div-WINDOWS.exe -c "$prg"        # compile
    system/divrun-WINDOWS.exe               # run (self-terminating)
    # parse .result file for failures
done
```

### Compiler golden tests

- A set of `.PRG` files that compile to known-good bytecode.
- A script compiles each and diffs the output `EXEC.EXE` against golden
  files. Catches parser/codegen regressions instantly.
- 10-15 programs covering: basic expressions, control flow, processes,
  structs, arrays, strings, edge cases.
- Also include programs that should *fail* to compile — verifying that
  the compiler correctly rejects invalid syntax and produces the right
  error messages.

### File format round-trip tests

- Create MAP/FPG/FNT in memory, write to disk, read back, compare.
- Catches parser bugs and format hardening regressions.
- Could be DIV programs (using `save()`/`load()`) or C-level tests.

### Build CI

- Run all of the above on every push via GitHub Actions.
- Even just "does it compile with zero warnings on all 4 targets" as a
  baseline would catch regressions.

**Payoff:** DIV tests itself using its own language, its own compiler,
its own runtime, and its own help resources. The documentation becomes
the source material for the test suite. Any regression in the compiler
or runtime that would break a user's program gets caught automatically.

**Effort:** 2-3 days for the self-hosted test suite, 1 day for compiler
golden tests, half a day for CI setup.
**Risk:** None — purely additive. Test programs live in a `tests/`
directory and don't affect the build or runtime.

---

## 3. File format hardening + modern imports

**Problem:** Every file parser trusts input blindly — no validation of
dimensions, offsets, or array indices from file headers. Crafted files can
cause heap overflows, double-frees, and crashes. This blocks any public
release (Steam or otherwise).

Additionally, the import formats are ancient: PCX, BMP (partial), raw PCM.
Users today work with PNG, TIFF, GIF, WebP, OGG, MP3 — not PCX.

**What to do:**

- **Harden existing parsers** (image.c, fpg.c): validate `width*height`
  before malloc, check `fread()` returns, bounds-check array indices from
  file headers, cap RLE decompression output. Surgical fixes, no
  architecture changes needed.
- **Add modern format libraries**: integrate stb_image (PNG, GIF, PSD, TGA,
  HDR — single-header, public domain, trivial to add) and/or libpng/giflib
  for import. For audio: SDL2_mixer already handles OGG/MP3/FLAC — just
  wire up the import paths in the PCM editor and runtime.
- **Keep proprietary formats as-is**: MAP, FPG, FNT, PAL are simple,
  well-documented (see user-manual.md appendix E), and serve the
  project well. No reason to replace them — just harden the readers.
- **Consider adding PNG/BMP export** alongside the existing MAP export,
  so users can easily share their work outside DIV.

**Effort:** 2-3 days for hardening, 1-2 days per new format library.
**Risk:** Low — purely additive, doesn't touch working code paths.

---

## 4. Help translation quality pass

**Context:** DIV shipped with an inline help system (`div/HELP/help.div`,
~22,700 lines / 856KB) — a hypertext reference accessible via F1 that
covers **every** built-in function, keyword, operator, predefined
variable, and compiler error — each with a full description and a
**working example program** the user can click and run directly from
the help page.

Originally written in Spanish by Daniel Navarro, translated to English
by Juan Tamargo. The translation is functional but rough — literal
word-by-word rendering that obscures Daniel's clear pedagogical voice.

Common translation artifacts: "archive" for "file", "clue" for "key",
"terminus" for "term", "module" for "remainder", passive constructions,
occasional untranslated Spanish fragments.

**What to do:**

- **Fix English quality page by page** — rewrite awkward translations
  into natural, clear English while preserving technical accuracy and
  the markup syntax (`{.N,title}`, `{#N,text}`, `{+N,C}`, `{-}`,
  `{bold}`, `{/}`, `{br}`).
- **Standardize terminology**: archivo->file, clave->key, modulo->remainder,
  etc. Apply consistently across all ~22,700 lines.
- **Verify example programs still compile and run** — the code examples
  are the crown jewel of this help system.
- **Fix any broken cross-references** — check that all `{#N,...}` links
  point to existing `{.N,...}` page definitions.
- Agent teams could parallelize this effectively: split the file by
  page ranges, process each range independently, merge results.

The original Spanish user manual exists as `docs/LIBRO.DOC` (Word 97,
~840KB). If the Spanish `help.div` can be located, having both languages
would enable higher-quality retranslation.

**Effort:** 3-5 days with agent parallelization.
**Risk:** Low — text-only changes, no code impact.

---

## 5. IDE graphics & fonts — self-hosted resources

**Problem:** All IDE visual resources live in raw binary formats that can't
be edited with DIV's own tools:

- **Icons/graphics** (`GRAF_P.DIV` / `GRAF_G.DIV`): raw binary from the D3
  era. Cannot be edited with DIV's paint or FPG editors.
- **UI font** (`pequeno.fon` / `grande.fon`): custom binary format.
- **Editor fonts** (`SYS06X08.BIN`, etc.): raw 1-byte-per-pixel bitmaps.

**Key enabler:** DIV already has FPG<->MAP and FNT<->MAP round-trip
workflows (Export map / Import map in the Graphics and Font menus).

**What to do:**

- **Icons -> FPG:** Convert icon sets to FPG, modify `initialization()`
  to load from FPG. Ship `graf_small.fpg` / `graf_large.fpg`.
- **Editor fonts -> FNT:** Convert SYS*.BIN to FNT, modify the font
  loader in `initialization()`.
- **UI font -> FNT:** Convert `pequeno.fon` / `grande.fon` to FNT,
  replace raw `fread()` + offset logic in `window.c`.

**Payoff:** DIV can edit its own UI resources using its own tools.
Eating our own dog food.

**Effort:** 2-3 days total.
**Risk:** Low — target formats and round-trip tools already work.

---

## 6. Compiler single source of truth

**Problem:** Adding a new built-in function or struct field requires
coordinated edits across `ltobj.def`, `inter.h`, and `functions.c`
with no automated consistency checks.

**What to do:**

- **Write a code generator** (Python or even a DIV program) that reads
  `ltobj.def` and emits `inter.h` offset defines, `end_struct`, and
  `fname[]` debug table. Optionally a validation header that both
  compiler and runtime include, so a mismatch fails at compile time.

**Effort:** 1-2 days.
**Risk:** Low — additive tooling.

---

## 7. Make implicit state machines explicit

**Problem:** The IDE relies on implicit state conventions documented as
"gotchas" rather than enforced by code: `draw_mode` overloaded,
`mouse_b = 0x8001` for spacebar-as-click, `foreground` values 0/1/2,
`block_state` 0/1/2, etc.

**What to do:**

- **Wrap state checks in intent-revealing helpers**: `is_tool_active()`,
  `is_tool_transitioning()`, `is_keyboard_click()`,
  `is_window_minimized()`.
- **Document state diagrams** for window focus/activation, paint tool
  dispatch, and editor block selection.
- **Consider making `draw_mode` a proper enum** with named states
  (partially addressed by enums sprint 4).

**Effort:** 1-2 days.
**Risk:** Very low — helper functions are purely additive.

---

## 8. Code editor modernization

**Problem:** The editor works but is basic by modern standards.

- The edit-line cache (`l[]` with `read_line()`/`write_line()`) limits
  multi-line operations.
- Keybindings are hardcoded scan codes in nested switch statements.
- Three block modes triple the keyboard handling code.
- No undo/redo.
- Syntax coloring is tightly coupled to rendering.

**Phase 1 — Architecture prep:**

- Extract a keybinding table (scan code + modifiers -> action enum).
- Unify block mode handling.
- Add an undo stack for the code editor.

**Phase 2 — Modern keybindings:**

- Default to Ctrl+Z/Y, Ctrl+D, Ctrl+/, Ctrl+G, etc.
- Keep legacy Borland bindings available.

**Effort:** Phase 1: 3-5 days. Phase 2: 1-2 days.
**Risk:** Medium — the editor's event loop is intricate.

---

## 9. Thicken the OSDEP abstraction layer

**Problem:** `OSDEP_*` in `osd_sdl2.c` is meant to abstract all platform
specifics, but SDL2 calls are scattered across 10+ files.

**What to do:**

- **Audio first**: pull all `Mix_*` calls behind OSDEP audio functions.
- **Input second**: centralize `SDL_PollEvent` into OSDEP.
- **Rendering third**: pull the 8-bit->32-bit->texture pipeline fully
  into OSDEP.

**Payoff:** SDL3 migration becomes a single-file change. Future platform
ports (Emscripten/web, mobile) become plausible.

**Effort:** 3-5 days total across three subsystems.
**Risk:** Medium — touching input and rendering paths requires careful
testing. Do one subsystem at a time.

---

## 10. Data-driven IDE configuration

**Problem:** UI layout, menu structure, toolbar contents, and editor
behavior are all hardcoded in C.

**What to do (incrementally):**

- **Menu definitions from data** (extending `lenguaje.div`).
- **Toolbar layouts from data** (paint editor's `bar[]` array).
- **Color themes from data** (the 12 UI color values in `SetupFile`).
- **Keybindings from data** (once the keybinding table from section 8
  exists).

**Effort:** 1-2 days per subsystem.
**Risk:** Low if done incrementally.

---

## 11. User manual modernization

The user manual (`docs/user-manual.md`, ~8,700 lines) is the printed
book that shipped with DIV.

**What to do:**

- Strip publisher layout notes (`NOTA PARA MAQUETACION`).
- Fix translation quality (same fixes as help.div).
- Update for SDL2 port where IDE behavior changed.
- Convert to proper markdown.

**Effort:** 2-3 days.
**Risk:** Low — text-only changes.

---

## 12. Compiler architecture modernization

**Phase 2 — Data-driven built-in functions:**

- Define built-in function signatures in a table. A dispatch table
  (`function_table[]`) indexed by function code replaces the 100+ case
  switch in `functions.c`.

**Phase 3 — Two-pass compilation (speculative):**

- A first pass collecting all declarations would eliminate forward-
  reference hacks. Only worth doing if language extensions are planned.

**Effort:** Phase 2: 2-3 days. Phase 3: 5+ days.
**Risk:** Phase 2 medium. Phase 3 high.

---

## 13. Streamline the window/handler architecture

**Problem:** The handler pattern works well but has accumulated quirks:
the `v` macro silently changing during `show_dialog()`, `v_*` parameter
globals as argument passing, etc.

**What to do:**

- **Window type enum** — done (section 1).
- **Consider passing a context struct to `show_dialog()`** instead of
  setting globals.
- **Document the `v`/`wup`/`wdown` pattern** as a first-class concept.

**Effort:** 1-3 days.
**Risk:** Medium for dialog params change, low for everything else.

---

## What NOT to do

These are tempting but historically fatal for DIV forks:

- **Don't rewrite in C++, Rust, or anything else.** The C codebase is
  the product. Every fork that rewrote died.
- **Don't try to eliminate all globals.** 154 globals for an IDE + compiler
  + VM + 6 editors is reasonable.
- **Don't build a "proper" widget toolkit.** The item system is crude but
  works. The IDE's charm is its pixel-perfect 1990s aesthetic.
- **Don't make it 64-bit.** The audience is retro gamedevs. The effort to
  audit every `(int)pointer` cast is enormous and buys nothing.
- **Don't split into libraries or microservices.** The codebase's strength
  is that it's one unified thing.
- **Don't over-modernize.** Every fork that tried to "componentize" or
  "properly architect" DIV died from single-developer burnout combined
  with scope explosion. Improve incrementally, ship often, listen to
  users.
