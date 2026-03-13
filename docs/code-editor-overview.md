# Code Editor Architecture

The code editor is DIV's built-in program editor for `.PRG` source files.
Each open file gets its own resizable, movable window inside the IDE desktop.
The editor supports syntax coloring, block selection (character and line
modes), clipboard, find/replace, mouse interaction, and scrollbars.

## Source files

| File | Role |
|------|------|
| `editor/editor_internal.h` | Shared header: constants, externs, forward declarations for all editor modules |
| `editor/editor.c` | Core: window handlers (`program0`..`program3`), main `editor()` loop, buffer management |
| `editor/editor_edit.c` | All editing operations: cursor movement, block selection, cut/copy/paste, line I/O |
| `editor/editor_render.c` | Rendering: syntax coloring, full/partial repaints, scrollbars, info bar, cursor, character drawing |
| `editor/editor_file.c` | File I/O and dialogs: open, new, save, find/replace, process list, goto-error, print |

## Data model

### The `tprg` struct (in `global.h`)

Each open source file is represented by a `struct tprg`:

| Field | Purpose |
|-------|---------|
| `buffer` | Heap-allocated byte buffer holding the entire file contents |
| `buffer_len` | Total allocated size of `buffer` |
| `file_len` | Actual content length within `buffer` |
| `num_lines` | Number of lines (CR/LF delimited) |
| `lptr` | Pointer into `buffer` at the start of the current line |
| `vptr` | Pointer into `buffer` at the start of the first visible line |
| `line` | Current cursor line (1-based) |
| `column` | Current cursor column (1-based) |
| `first_line` | First visible line number (for vertical scroll) |
| `first_column` | First visible column (for horizontal scroll) |
| `l[LONG_LINE+4]` | Working copy of the line being edited (max 1024 chars) |
| `line_size` | Original byte length of the current line in `buffer` |
| `w`, `h` | Window dimensions in characters |
| `old_x/y/w/h` | Saved position/size for maximize/restore |
| `path`, `filename` | File location on disk |
| `prev_line` | Previous line number (for partial redraw optimization) |

### Line storage format

Lines in the buffer are delimited by CR+LF (bytes 13, 10). There is no
terminating null per line -- the end of content is tracked by `file_len`.
Tabs are expanded to 4 spaces on load. Lines longer than 1023 characters
are forcibly split on load.

### The edit-line cache (`l[]`)

The editor only edits one line at a time. The current line is copied from
`buffer` into `tprg.l[]` by `read_line()`, edited in-place, then written
back to `buffer` by `write_line()`. This avoids constant memmove operations
while the user types. When the cursor moves to a different line, the
sequence is always: `write_line()` -> move `lptr` -> `read_line()`.

### Buffer growth

`check_memory()` ensures there is always at least `buffer_min` (2048)
bytes of headroom in the buffer. When space runs low, `realloc()` grows
the buffer by `BUFFER_INCREASE` (16384) bytes and adjusts all pointers
(`buffer`, `lptr`, `vptr`, `block_start`, `block_end`).

## Window system integration

Editor windows use the IDE's `twindow` system with `v.type = 102`. Each
window has four handler functions registered in `program0()`:

| Handler | Function | Role |
|---------|----------|------|
| `program0` | Init | Sets dimensions, initializes `tprg` pointers, registers handlers |
| `program1` | Paint | Renders window chrome (maximize, scroll arrows, resize grip) via `repaint_full()` |
| `program2` | Click | Mouse interaction: scrollbars, resize, maximize, mouse block selection, then calls `editor()` |
| `program3` | Close | Frees `buffer` and `tprg` struct |

The global `v_prg` points to the `tprg` being initialized. Once the window
is created, `v.prg` holds the pointer for the frontmost window.

## The `editor()` main loop

`editor()` in `editor.c` is called once per frame from `program2()` (the
click handler). It processes keyboard input and dispatches to the
appropriate editing function. The flow is:

1. **Error display** -- if a compiler error is active, show the error
   cursor and wait for a keypress to dismiss.

2. **Window switch detection** -- if the user switched to a different
   editor window since last frame, flush the previous window's edit line.

3. **Delete previous cursor** -- `delete_text_cursor()` erases the
   blinking cursor from the previous frame.

4. **Block mode dispatch** -- three large switch blocks handle keyboard
   input depending on the current `edit_block_mode`:
   - **Mode 0** (no active block): basic cursor movement, shift+arrows
     to start selections, alt+A/U for mark/unmark, ctrl+X/C/V clipboard.
   - **Mode 1** (single-line block): extend selection left/right with
     shift+arrows, cut/copy/paste/delete the selection.
   - **Mode 2** (multi-line block): extend selection up/down with
     shift+arrows/PgUp/PgDn, line-level cut/copy/paste.

5. **Mouse wheel** -- scrolls the viewport.

6. **Character input** -- if a printable character or Enter was pressed
   (without ctrl/alt), insert or overwrite it.

7. **Post-edit cleanup** -- `remove_spaces()` trims trailing spaces from
   the current line.

8. **Buffer check** -- `check_memory(0)` ensures buffer headroom.

9. **Redraw** -- either `repaint_full()` (full window repaint) or `repaint_partial()`
   (current line only), then `text_cursor()` draws the
   blinking cursor.

## Keyboard bindings

The editor reads raw scan codes via `scan_code` and modifier state via
`shift_status` (bit 0-1 = shift, bit 2 = ctrl, bit 3 = alt).

### Basic navigation (no modifiers)

| Key | Function | Description |
|-----|----------|-------------|
| Arrows | `f_right/left/down/up` | Move cursor one position |
| Home/End | `f_home/f_end` | Start/end of line |
| PgUp/PgDn | `f_page_up/f_page_down` | Scroll one page |
| Insert | `f_insert` | Toggle insert/overwrite mode |
| Delete | `f_delete_char` | Delete character or join lines |
| Backspace | `f_backspace` | Delete character before cursor |
| Tab | `f_tab` | Insert spaces to next tab stop |
| Enter | `f_enter` | Split line or insert new line |

### Ctrl combinations

| Key | Function | Description |
|-----|----------|-------------|
| Ctrl+Right/Left | `f_word_right/f_word_left` | Word-level movement |
| Ctrl+Home/End | `f_bop/f_eop` | Top/bottom of visible page |
| Ctrl+PgUp/PgDn | `f_bof/f_eof` | Beginning/end of file |
| Ctrl+Backspace, Ctrl+Y | `f_delete` | Delete entire line |
| Ctrl+X | `f_cut_block(1)` | Cut block to clipboard |
| Ctrl+C | `f_cut_block(0)` | Copy block to clipboard |
| Ctrl+V | `f_paste_block` | Paste from clipboard |

### Shift combinations (block selection)

| Key | Action |
|-----|--------|
| Shift+Right/Left | Start/extend character-level selection |
| Shift+Up/Down | Start/extend line-level selection |
| Shift+PgUp/PgDn | Extend selection by a page |
| Shift+Home/End | Select to start/end of line |
| Shift+Insert | Paste |
| Shift+Delete | Cut |
| Shift+Tab | Untab (reverse tab) |

### Alt combinations (legacy Borland-style)

| Key | Function | Description |
|-----|----------|-------------|
| Alt+A | `f_mark` | Set/extend block mark |
| Alt+U | `f_unmark` | Remove block selection |
| Alt+D | `f_cut_block(1)` | Cut block |
| Alt+C | Copy + paste block (duplicate at cursor) |
| Alt+M | Cut + paste block (move to cursor) |

## Block selection model

Block selection uses a three-state machine tracked by `block_state`:

- **0** -- No block active
- **1** -- Pivot set (one corner marked, waiting for second)
- **2** -- Complete block (both corners set, ready for operations)

Additional state:

| Variable | Purpose |
|----------|---------|
| `block_start`, `block_end` | Pointers to start/end line positions in buffer |
| `block_col1`, `block_col2` | Column bounds within those lines |
| `kprg` | Which `tprg` owns the block (blocks can span across windows) |
| `edit_block_mode` | 0=none, 1=character block, 2=line block |
| `clipboard` | Heap-allocated clipboard data |
| `clipboard_type` | 0=character block, 1=line block |

`f_cut()` implements the core cut/copy/delete. Mode 0 copies to clipboard,
mode 1 copies then deletes from buffer, mode 2 deletes without updating
clipboard.

## Rendering pipeline

### Full repaint (`repaint_full()`)

Called when `v.redraw` is set (scroll, block change, window resize, etc.):

1. **Syntax color cache** -- walks `color_lex()` from the buffer start
   (or cached position) up to `vptr` to establish the comment state.
2. **Info bar** -- draws line:column indicator at the top.
3. **Scrollbars** -- vertical and horizontal slider positions.
4. **Line-by-line rendering** -- for each visible line:
   - Fill background (normal color, block highlight, or mixed for
     partial-line selections).
   - Run `fill_color_line()` to compute per-character syntax colors
     into `colin[]`.
   - Render characters with `put_char3()`.

### Partial repaint (`repaint_partial()`)

Optimized path when only the current line changed (normal typing):

1. Recalculates syntax colors for just the current line.
2. If the comment state changed (e.g., `/*` was typed), falls back to
   `repaint_full()` for a full repaint.
3. Otherwise redraws only the current line's background and characters.

### Syntax coloring

`color_lex()` is a lexer that tokenizes DIV source into categories:
symbols (`p_sym`), reserved words (`p_res`), predefined identifiers
(`p_pre`), string literals (`p_lit`), numbers (`p_num`), comments
(`p_rem`), identifiers (`p_id`), and whitespace (`p_spc`). Each category
maps to a palette color index stored in `colin[]`.

Coloring is only active for `.PRG` files (checked via the window title)
and when `colorizer` is enabled in settings.

The colorizer state is cached at `vptr` position (`_cached_color_source`,
`_cached_in_comment`) so that scrolling forward/backward doesn't require
re-lexing from the start of the file every frame.

### Character rendering

Three functions handle glyph rendering into the 8-bit paletted window
buffer:

| Function | Purpose |
|----------|---------|
| `put_char()` | Basic: renders glyph in block or normal color |
| `put_char2()` | Renders glyph in a specific color (used for error cursor) |
| `put_char3()` | Renders glyph with per-character syntax color |

All three index into the `font` bitmap array using `c * char_size` and
stamp non-zero pixels. The font dimensions are `editor_font_width` x
`editor_font_height` (6x8, 8x12, or 9x16 depending on resolution).

### Text cursor

`text_cursor()` draws a blinking cursor using `system_clock & 4` as the
blink timer. In insert mode, only the bottom half of the character cell
blinks. In overwrite mode, the full cell blinks. The cursor color changes
when inside a block selection.

## File I/O

### Opening files (`open_program()`)

1. `fopen()` the file, `fread()` into a newly allocated buffer.
2. Reject binary files (any null byte = not a text file).
3. Normalize line endings: lone CR or LF becomes CR+LF pair.
4. Expand tabs to 4 spaces.
5. Force-split lines longer than 1023 characters.
6. Count lines, set `num_lines`.
7. Call `new_window(program0)` to create the editor window.

### Saving files (`save_program()`)

1. `write_line()` to flush the edit cache.
2. `fwrite()` the buffer contents.
3. On partial write, delete the partial file and show an error.
4. Update window title bar.
5. Special case: saving `help.div` regenerates the help index.

### New file (`program0_new()`)

Creates a file on disk with a timestamped header comment and a
`PROGRAM name; BEGIN ... END` template, then opens it via `open_program()`.

## Dialogs

### Find (`find_text0` / `find_text`)

Modal dialog with a text input, "whole word" flag, and "case sensitive"
flag. `find_text()` searches forward from the cursor, wrapping the
cursor position if not found.

### Replace (`replace_text0` / `replace_text`)

Modal dialog with search and replacement inputs plus the same flags.
On each match, shows a Yes/No/All/Cancel confirmation dialog. Tracks
replacement count and shows a summary at the end.

### Process list (`process_list0`)

Scans the source buffer for `PROCESS` declarations, builds a scrollable
list dialog. Supports alphabetical sorting. Double-click or Enter jumps
to the selected process. If the cursor is on a process name in the
source, that process is pre-selected.

### Goto error (`goto_error()`)

After a compilation error, navigates the editor to `error_line` /
`error_col` and highlights the error position with a red cursor.

## Key globals (in `global.h`)

| Variable | Purpose |
|----------|---------|
| `v_prg` | Temporary pointer used during window creation |
| `eprg` | Points to `tprg` with the active compiler error |
| `editor_font_width/height` | Character cell dimensions (set at startup) |
| `LONG_LINE` | Maximum line length (1024) |
| `tab_size` | Tab stop interval |
| `colorizer` | Whether syntax coloring is enabled |
| `scan_code` | Current keyboard scan code |
| `ascii` | Current ASCII character |
| `shift_status` | Modifier key bitmask (shift/ctrl/alt) |
| `mouse_b` | Mouse button state |
| `big` / `big2` | Double-size mode flag / multiplier |

## Limitations and improvement opportunities

- **Max line length**: 1024 characters (`LONG_LINE`), hard-coded into
  buffer layout and rendering.
- **No undo**: the editor has no undo/redo mechanism.
- **No line numbers**: the gutter area only shows the cursor position in
  the info bar, not line numbers alongside the text.
- **Linear search**: find/replace searches character-by-character from
  the cursor position forward with no indexing.
- **Single clipboard**: one global clipboard shared across all windows,
  no clipboard history.
- **No auto-indent beyond enter**: `f_enter()` copies leading whitespace
  from the current line, but there is no block indent/outdent.
- **Tab expansion on load**: tabs are converted to spaces on file open
  and never restored on save.
- **Font sizes fixed at startup**: the editor font is chosen during
  `initialization()` based on screen resolution and cannot be changed
  at runtime.
- **Syntax coloring is PRG-only**: the colorizer only activates for
  files with `.PRG` in the window title.
