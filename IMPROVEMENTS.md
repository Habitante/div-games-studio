# DIV Games Studio — Improvement Options

A palette of architectural improvements to keep the codebase moving forward.
Not a roadmap — these are options to evaluate, prioritize, and tackle
incrementally. Some may not be worth the effort; others will unlock years of
future work.

Written 2026-03-14 after a full read of the codebase (~87K lines),
architecture docs, code review, and discussion with Daniel Navarro.

---

## 1. File Format Safety & Modern Format Support

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

## 2. IDE Graphics and Fonts — Self-Hosted Resources

**Problem:** All IDE visual resources live in raw binary formats that can't
be edited with DIV's own tools:

- **Icons/graphics** (`GRAF_P.DIV` / `GRAF_G.DIV`): originally created in
  D3, exported to a raw binary format. Cannot be edited with DIV's paint
  or FPG editors. Adding or modifying icons requires hex-editing.
- **UI font** (`pequeno.fon` / `grande.fon`): proportional font in a
  custom binary format — 1-byte header (height), 256 character descriptors
  (width + glyph offset, 4 bytes each), then 1-byte-per-pixel glyph data
  starting at offset 1025. Two sizes (small/big mode). Used by all menus,
  dialogs, buttons, and window text (`wwrite()` in `window.c`).
- **Editor fonts** (`SYS06X08.BIN`, `SYS08X08.BIN`, `SYS08X11.BIN`,
  `SYS09X16.BIN`): raw 1-byte-per-pixel bitmaps, 256 glyphs each, no
  header at all — just `width * height * 256` bytes of pixel data.
  Indexed as `font[char_code * char_size]`. Used by the code editor
  (`put_char()` in `editor_render.c`) and the paint editor's text tool.

None of these can be viewed or modified using DIV itself. They're opaque
binary blobs from the D3/DOS era.

**Key enabler:** DIV already has built-in tools for round-tripping between
its archive formats and editable MAP atlases:

- **FPG ↔ MAP:** The Graphics menu has "Export map" (FPG → single MAP
  atlas image) and "Import map" (MAP atlas → FPG). This means you can
  export all icons to one flat image, edit it in the paint editor, and
  import it back.
- **FNT ↔ MAP:** The Font menu has the same "Export map" / "Import map"
  workflow. Export a font to a MAP atlas, tweak glyphs visually, import
  back to FNT with metrics preserved.

So the workflow for editing IDE resources would be: open the FPG/FNT in
DIV → export to MAP → edit in paint editor → import back. No external
tools needed.

**What to do:**

### Icons → FPG

- **Convert the icon sets to FPG format** — DIV's own sprite archive
  format, editable with DIV's own FPG and paint editors.
- **Modify `initialization()` in `main.c`** to load icons from FPG instead
  of the raw binary format. The `graf[]` array that stores icon data would
  be populated from FPG graphic entries instead of raw offsets.
- **Ship two FPGs**: `graf_small.fpg` (for normal resolution) and
  `graf_large.fpg` (for big mode), replacing `GRAF_P.DIV`/`GRAF_G.DIV`.
- **Document the icon catalog** — which graphic code maps to which UI
  element — so future contributors can find and edit icons easily.
- **Editing workflow:** open `graf_small.fpg` in DIV → Export map → edit
  the atlas in the paint editor → Import map → save FPG. Done.

### Editor fonts → FNT

- **Convert each SYS*.BIN to FNT format.** Each is a fixed-width font
  (256 glyphs, 1 byte per pixel, no header). The glyph data maps
  directly to FNT character entries with uniform width/height.
- **Modify the font loader in `initialization()`** to load from FNT
  instead of raw binary. The `font` pointer and `char_size` calculation
  stay the same — just the I/O changes.
- **Editing workflow:** open the FNT in DIV's font editor → Export map
  → edit the glyph atlas in the paint editor → Import map → save FNT.

### UI font → FNT

- **Convert `pequeno.fon` / `grande.fon` to DIV's own FNT format** —
  which already supports proportional fonts with per-character metrics
  (variable width, individual glyph bitmaps). The custom binary format
  (1-byte height header + 256 sscar descriptors + glyph data) maps
  naturally to FNT's per-character structure.
- **Replace the raw `fread()` + offset logic in `window.c`** with a
  call to the existing FNT loader. The `sscar` struct and `text_font`
  pointer would be populated from the FNT data instead of raw offsets.
- **Editing workflow:** same as editor fonts — open in font editor,
  export to MAP atlas, edit, import back.

**Payoff:** DIV can edit its own UI resources — icons, editor fonts, and
UI fonts — using its own tools and the existing FPG/FNT ↔ MAP atlas
round-trip workflow. Adding new toolbar icons, tweaking glyph shapes, or
adjusting font metrics becomes visual work instead of hex editing.
Eating our own dog food.

**Effort:** 2-3 days total (icons: 1 day, editor fonts: 0.5 day, UI
font: 1-1.5 days).
**Risk:** Low — the target formats (FPG, FNT) and the atlas round-trip
tools are well-understood and already working in the codebase.

---

## 3. Replace Magic Numbers with Enums and Named Constants

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

**What to do:**

- **Define enums** for window types, draw modes, cursor IDs, file type
  indices, menu base indices, item types, and process status codes.
- **Define named constants** for palette size (`PALETTE_BYTES = 768`),
  FPG limits (`MAX_FPG_GRAPHICS = 1000`), UI metrics, and any other
  value that appears more than twice.
- **Do this incrementally** — one category at a time, with project-wide
  search-and-replace. Each category is a self-contained commit.
- **Don't over-abstract** — some magic numbers are genuinely one-off
  pixel tweaks. Focus on values that appear in multiple files or that
  would confuse a newcomer.

**Effort:** 1-2 days total, spread across small commits.
**Risk:** Very low — purely mechanical, no behavior change. Compiler
catches any mistakes.

---

## 4. Thicken the OSDEP Abstraction Layer

**Problem:** `OSDEP_*` in `osd_sdl2.c` is meant to abstract all platform
specifics, but SDL2 calls are scattered across 10+ files (mouse.c,
keyboard.c, video.c, pcm.c, sound.c, browser.c, mixer.c, etc.). The
abstraction exists in name but not in practice.

**What to do:**

- **Audio first** (messiest): pull all `Mix_*` calls from pcm.c, sound.c,
  browser.c, mixer.c behind OSDEP audio functions. The IDE and runtime
  have different needs (preview vs game playback) but both should go
  through the same layer.
- **Input second**: centralize `SDL_PollEvent` into OSDEP. Right now both
  `ide/mouse.c` and `shared/run/keyboard.c` have their own event loops
  with subtly different behavior.
- **Rendering third**: pull the 8-bit→32-bit→texture pipeline fully into
  OSDEP. Currently `blit_screen()`, `OSDEP_Flip()`, and surface management
  are split across osd_sdl2.c, shared/run/video.c, and ide/video.c.

**Payoff:** SDL3 migration becomes a single-file change. Future platform
ports (Emscripten/web, mobile) become plausible. Clear boundary between
"what the IDE does" and "how the platform delivers it."

**Effort:** 3-5 days total across three subsystems.
**Risk:** Medium — touching input and rendering paths requires careful
testing. Do one subsystem at a time.

---

## 5. Make Implicit State Machines Explicit

**Problem:** The IDE relies on implicit state conventions that are
documented as "gotchas" rather than enforced by code:

- `v` (= `window[0]`) silently changes identity during `show_dialog()`
- `draw_mode` overloaded: <100 = active tool, >=100 = tool transition
- `mouse_b = 0x8001` = spacebar pretending to be left-click
- `end_dialog` as a bare global any code can set
- `foreground` values 0/1/2 meaning background/active/minimized
- `block_state` 0/1/2 for selection state machine

**What to do:**

- **Wrap state checks in intent-revealing helpers**: `is_tool_active()`,
  `is_tool_transitioning()`, `is_keyboard_click()`, `is_window_minimized()`
  instead of checking magic thresholds in 6 places each.
- **Document state diagrams** for the three main state machines: window
  focus/activation, paint tool dispatch, and editor block selection.
- **Consider making `draw_mode` a proper enum** with named states rather
  than relying on the <100/>=100 convention.

**Effort:** 1-2 days.
**Risk:** Very low — helper functions are purely additive, don't change
control flow.

---

## 6. Testing Infrastructure

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
frame loop and then fall off the `END` to terminate cleanly. Interactive
help examples (which `LOOP` forever waiting for input) need their loops
replaced with bounded iterations.

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

Complement the runtime tests with compilation-only checks:

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

## 7. Code Editor Architecture Modernization

**Problem:** The editor works but is basic by modern standards. Before
bolting on improvements (undo, line numbers, better keybindings), the
architecture needs attention:

- The edit-line cache (`l[]` with `read_line()`/`write_line()`) means only
  one line exists in editable form at a time — every operation that touches
  multiple lines requires careful flush/reload sequences.
- Keybindings are hardcoded scan codes in nested switch statements — no
  binding table, no way to remap, and the current bindings don't match
  modern conventions (Alt+A/U/D/C/M are Borland-era shortcuts nobody
  remembers).
- The three block modes (none/character/line) triple the keyboard handling
  code because each mode has its own switch block.
- No undo/redo. The `undo` buffer exists in `global.h` (allocated in
  `initialization()`) but is used only by the paint editor, not the code
  editor.
- Syntax coloring is tightly coupled to rendering (`color_lex()` runs
  during paint), making it hard to add features that need token awareness
  (autocomplete, matching brackets, etc.).

**Phase 1 — Architecture prep (before adding features):**

- **Extract a keybinding table**: scan code + modifiers → action enum.
  Replace the nested switch blocks with a table lookup. This makes
  remapping possible and cuts ~200 lines of redundant switch cases.
- **Unify block mode handling**: the three switch blocks in `editor()`
  share 70% of their cases. Factor out common actions, dispatch by mode
  only where behavior actually differs.
- **Add an undo stack for the code editor**: the paint editor's undo
  buffer pattern could be adapted (snapshot-based), or a simpler
  operation-log approach. This is the #1 user-facing quality-of-life
  improvement.

**Phase 2 — Modern keybindings:**

- Default to standard Ctrl+Z/Y (undo/redo), Ctrl+D (duplicate line),
  Ctrl+/ (toggle comment), Ctrl+G (goto line), Ctrl+Shift+K (delete
  line), etc.
- Keep legacy Borland bindings available as an option.

**Effort:** Phase 1: 3-5 days. Phase 2: 1-2 days.
**Risk:** Medium — the editor's event loop is intricate. Keybinding table
extraction is safe; undo requires careful buffer management.

---

## 8. Compiler Architecture Modernization

**Problem:** The compiler works but is hard to extend. Adding a new
language feature (a new statement type, a new built-in, a new data type)
requires coordinated edits across multiple files with no automated
consistency checks:

- `ltobj.def` defines function codes and struct layouts
- `inter.h` has hand-calculated `end_struct` and `#define` offsets
- `functions.c` has a switch with `case NNN:` matching `ltobj.def` codes
- `compiler_internal.h` has opcode enums that must match `kernel.inc`
- The single-pass design means forward references require workarounds
  and the parser interleaves declaration processing with code generation

**Phase 1 — Single source of truth:**

- **Write a code generator** (Python or even a DIV program) that reads
  `ltobj.def` and emits:
  - `inter.h` offset defines (`_X`, `_Y`, `_Graph`, etc.)
  - `end_struct` calculated value
  - `fname[]` debug table for `functions.c`
  - Optionally: a validation header that both compiler and runtime
    include, so a mismatch fails at compile time instead of at runtime
- This eliminates the "edit three files and pray" pattern. The contract
  becomes a build artifact, not a manual invariant.

**Phase 2 — Data-driven built-in functions:**

- Define built-in function signatures (parameter count, types, return
  type) in a table rather than having the compiler hardcode them from
  `ltobj.def` parsing and the runtime hardcode them in a giant switch.
- A dispatch table (`function_table[]`) indexed by function code, with
  function pointer + arity + name. The `lfun` opcode handler becomes a
  table lookup instead of a 100+ case switch.
- Makes adding new built-in functions a one-line table entry instead of
  a multi-file edit.

**Phase 3 — Two-pass compilation (speculative, evaluate carefully):**

- A first pass that collects all declarations (processes, functions,
  globals, constants) would eliminate forward-reference hacks and
  simplify the parser.
- The existing `psintactico()` pre-parse is already a partial first pass
  (it scans for string literal lengths). Extending it to collect
  declarations is natural.
- **Caution:** this is a significant change to a working compiler. Only
  worth doing if language extensions are planned that genuinely need it.

**Effort:** Phase 1: 1-2 days. Phase 2: 2-3 days. Phase 3: 5+ days.
**Risk:** Phase 1 is low (additive tooling). Phase 2 is medium (changes
runtime dispatch). Phase 3 is high (fundamental compiler change).

---

## 9. Data-Driven IDE Configuration

**Problem:** UI layout, menu structure, toolbar contents, and editor
behavior are all hardcoded in C. Adding a menu item requires editing C
source, recompiling, and redeploying. This made sense in 1997 when the
IDE *was* the product and no one else would modify it, but today it
limits both development velocity and modding potential.

**What to do (incrementally, not all at once):**

- **Menu definitions from data**: menus are already partly data-driven
  (text comes from `lenguaje.div`), but the structure (which items exist,
  their shortcuts, enable/disable logic) is in C. A small menu definition
  format (or extending `lenguaje.div`) could describe menu trees
  declaratively.
- **Toolbar layouts from data**: the paint editor's `bar[]` array that
  defines toolbar icons per tool mode could come from a config file
  instead of being hardcoded in `paint.c`.
- **Color themes from data**: the 12 UI color values in `SetupFile` are
  already configurable via setup.bin. Exposing them as a named theme
  file would let users customize the IDE look.
- **Keybindings from data**: once the keybinding table exists (see
  section 7), loading it from a file is trivial.

**Payoff:** faster iteration on UI changes, user customization, modding
community potential.

**Effort:** varies per subsystem — 1-2 days each.
**Risk:** Low if done incrementally. Don't try to make everything
data-driven at once.

---

## 10. Streamline the Window/Handler Architecture

**Problem:** The handler pattern (init/paint/click/close as 0/1/2/3
functions) is consistent and works well, but has accumulated quirks:

- The `v` macro for `window[0]` means any nested `show_dialog()` call
  silently changes what `v` refers to. The `wup()`/`wdown()` workaround
  is error-prone and non-obvious.
- Window types are bare integers scattered through the code with no
  central registry.
- The `v_` parameter globals (`v_text`, `v_mode`, `v_type`, `v_accept`,
  etc.) are a poor man's argument passing — they work but create hidden
  coupling between caller and dialog.

**What to do:**

- **Window type enum** (see section 3) — immediate clarity.
- **Consider passing a context struct to `show_dialog()`** instead of
  setting globals before the call. A `struct dialog_params` with the
  fields currently spread across `v_*` globals would make the interface
  explicit. The dialog reads params from the struct, writes results back
  to it. Callers own the struct on the stack.
- **Document the `v`/`wup`/`wdown` pattern** as a first-class concept
  rather than a gotcha. Consider whether a `with_window(n, callback)`
  helper could make the save/restore pattern less error-prone.

**Effort:** 1-3 days depending on scope.
**Risk:** Medium for the dialog params change (touches many callers),
low for everything else.

---

## 11. Documentation — User Manual and Inline Help

**Context:** DIV shipped with two comprehensive learning resources, both
written by Daniel Navarro, that were critical to its success as a
self-contained development tool:

- **User's Handbook** (`docs/user-manual.md`, ~8,700 lines / 570KB) — the
  printed book that shipped with DIV. 8 chapters covering installation,
  the IDE menus, the paint editor, game design concepts, programming
  tutorial (variables, control flow, processes, FRAME), advanced topics
  (collision, scroll, Mode 7), plus appendices with keyboard shortcuts,
  error messages, predefined data, file format specs, and CD-ROM contents.
  Still contains the original Spanish layout notes for the publisher
  (`NOTA PARA MAQUETACIÓN`).

- **Inline help** (`div/HELP/help.div`, ~22,700 lines / 856KB) — a
  hypertext reference system that runs inside the IDE itself (F1 key).
  Custom markup format with page definitions (`{.N,title}`), cross-reference
  links (`{#N,text}`), inline images from `help.fpg`, and bold formatting.
  Covers **every** built-in function, keyword, operator, predefined
  variable, and compiler error — each with a full description and a
  **working example program** the user can click and run directly from
  the help page. Also includes FAQ, tutorials, and "what's new" sections.
  The help viewer is implemented in `ide/help.c` with a figure file
  `div/HELP/help.fig` for inline images.

Both resources were originally written in Spanish and translated to
English by Juan Tamargo. The translations are functional but rough —
literal word-by-word rendering that obscures Daniel's clear pedagogical
voice. Common translation artifacts:

- "archive" instead of "file" (Spanish "archivo" = file)
- "clue" instead of "key" (Spanish "clave" = key/password)
- "terminus" instead of "term" (Spanish "término")
- "module" instead of "remainder" (Spanish "módulo" = modulo/remainder)
- "it is necessary to indicate" instead of "you need to specify"
- Passive/impersonal constructions where English prefers direct address
- Occasional untranslated Spanish fragments and layout notes

**What to do:**

### Phase 1 — Translation quality pass on `help.div` (highest priority)

This is what users interact with inside the tool. The markup format is
simple and well-defined, making it safe for automated processing:

- **Fix English quality page by page** — rewrite awkward translations
  into natural, clear English while preserving technical accuracy and
  the markup syntax (`{.N,title}`, `{#N,text}`, `{+N,C}`, `{-}`,
  `{bold}`, `{/}`, `{br}`).
- **Standardize terminology**: archivo→file, clave→key, módulo→remainder,
  etc. Apply consistently across all ~22,700 lines.
- **Verify example programs still compile and run** — the code examples
  are the crown jewel of this help system. Each should be tested against
  the current compiler.
- **Fix any broken cross-references** — check that all `{#N,...}` links
  point to existing `{.N,...}` page definitions.
- Agent teams could parallelize this effectively: split the file by
  page ranges, process each range independently, merge results.

### Phase 2 — User manual modernization

- **Strip publisher layout notes** — remove the `NOTA PARA MAQUETACIÓN`
  blocks and image placement instructions (they reference PCX files from
  the original print layout).
- **Fix translation quality** — same terminology and style fixes as
  help.div, but this is a lower priority since users don't see this
  inside the tool.
- **Update for SDL2 port** — Chapter 1 (installation) and Chapter 2
  (menus) may need updates if any IDE behavior changed since the DOS era.
  The paint editor chapter (3-4) and language chapters (5-8) are largely
  timeless.
- **Convert to proper markdown** — add headings, code blocks, tables
  where appropriate. Currently it's plain text with minimal formatting.

### Phase 3 — Help content expansion

- **Add help pages for any missing functions or features** — if built-in
  functions were added since DIV2, they need help pages with the standard
  format (signature, description, example program, see-also links).
- **Add a "Getting Started" tutorial** — the current help assumes the
  user has the printed manual. A self-contained beginner tutorial
  accessible from the help index would make DIV approachable without
  the book.
- **Consider adding help pages for the IDE itself** — the current help
  focuses on the programming language; the IDE menus and editors are
  only documented in the printed manual.

### The original Spanish text

The original Spanish user manual exists as `docs/LIBRO.DOC` (Word 97,
~840KB). If the Spanish `help.div` can be located (the current file
header says "Castellano" but the content is the English translation),
having both languages would enable higher-quality retranslation —
working from the Spanish original rather than trying to fix broken
English in isolation.

**Payoff:** DIV's self-contained documentation was a killer feature in
1997 — every function documented with a runnable example, accessible
from inside the tool. Fixing the English brings this to the quality
level modern users expect, and makes DIV approachable to the global
audience a Steam release would reach.

**Effort:** Phase 1 (help.div): 3-5 days with agent parallelization.
Phase 2 (manual): 2-3 days. Phase 3 (expansion): ongoing.
**Risk:** Low — text-only changes, no code impact. The main risk is
introducing technical inaccuracies during rewriting, mitigated by
testing the example programs.

---

## What NOT to Do

These are tempting but historically fatal for DIV forks:

- **Don't rewrite in C++, Rust, or anything else.** The C codebase is
  the product. Every fork that rewrote died.
- **Don't try to eliminate all globals.** 154 globals for an IDE + compiler
  + VM + 6 editors is reasonable. The coupling is real but doesn't block
  feature work.
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

---

## Suggested Sequencing

If tackling these in order, a reasonable progression might be:

1. **Enums & constants** (section 3) — quick wins, makes everything after
   it cleaner
2. **Testing infrastructure** (section 6) — safety net before bigger
   changes
3. **File format hardening + modern imports** (section 1) — unblocks
   public release
4. **Help translation quality pass** (section 11 phase 1) — high-impact,
   user-facing, parallelizable with agent teams, no code risk
5. **IDE graphics & fonts to FPG/FNT** (section 2) — quick win, enables
   self-hosted resource editing
6. **Compiler single source of truth** (section 8 phase 1) — eliminates
   the most fragile coupling
7. **State machine cleanup** (section 5) — prep for editor/paint work
8. **Editor modernization** (section 7) — user-facing quality of life
9. **OSDEP thickening** (section 4) — prep for SDL3 and future platforms
10. **Data-driven config** (section 9) — accelerates future iteration
11. **User manual modernization** (section 11 phase 2) — lower priority
    than help.div but important for Steam release
12. **Compiler modernization** (section 8 phases 2-3) — only if language
    evolution is planned
13. **Window/handler streamlining** (section 10) — nice-to-have, lowest
    urgency

This ordering front-loads low-risk, high-payoff work and defers the
riskier architectural changes until the safety net (tests, enums, docs)
is in place.
