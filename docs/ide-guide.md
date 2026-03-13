# DIV IDE Developer Guide

A comprehensive reference for understanding, modifying, and extending the DIV Games
Studio IDE. Read this before working on menus, dialogs, windows, or any IDE shell
functionality.

**Prerequisites:** Read `docs/architecture-overview.md` first for the rendering
pipeline, OSDEP layer, and file format details. This guide focuses on the IDE's
interactive infrastructure — the parts you need to understand to add or change
UI features.

---

## Source file map

| File | Role |
|------|------|
| `ide/main.c` | Entry point, `main()`, `mainloop()`, `initialization()`, `finalization()` |
| `ide/main_internal.h` | Shared declarations across `main_*.c` modules |
| `ide/main_desktop.c` | Window management, rendering, placement, animations |
| `ide/main_dialogs.c` | Modal loop, `show_dialog()`, UI item system (buttons/text/checkboxes) |
| `ide/handler.c` | Menu infrastructure (`create_menu`, `paint_menu`, `update_menu`), all top-level menus, window finder functions |
| `ide/handler_internal.h` | Shared declarations across `handler_*.c` modules |
| `ide/handler_dialogs.c` | Standard dialogs (error, confirm, file browser, about), listbox widget, progress bar |
| `ide/handler_map.c` | Map viewer window, open/save/new/resize map operations |
| `ide/handler_fonts.c` | Font map generation |
| `ide/window.c` | Low-level drawing primitives (`wbox`, `wwrite`, `wput`, `wrectangle`, etc.) |
| `ide/desktop.c` | Session persistence — save/restore open windows to `system/session.dtf` |
| `ide/mouse.c` | SDL2 event pump, mouse state aggregation, keyboard polling |
| `editor/editor.c` | Code editor entry point and main loop |
| `editor/paint.c` | Paint editor entry point and tool dispatch |

---

## The texts[] string system

All user-visible strings come from `texts[]`, a global array of `byte*` pointers
loaded at startup from `system/lenguaje.div`. This is DIV's localization system.

**Loading:** `initialize_texts()` in `main()` reads the file and populates the array.
`finalize_texts()` frees it on shutdown.

**Usage:** Reference strings by index, never by literal. For example:
```c
v.title = texts[41];          // "Error"
v_text = (char *)texts[43];   // "File not found"
_button(100, x, y, align);    // texts[100] = "OK"
_button(101, x, y, align);    // texts[101] = "Cancel"
```

**Key index ranges:**

| Range | Content |
|-------|---------|
| 34–40 | Window title, copyright, exit |
| 41–49 | Error messages (error, read error, file not found, out of memory, etc.) |
| 100–125 | Button labels (OK, Cancel, Help, Yes, No, etc.) |
| 186–250 | File operation strings (new program, save, open, etc.) |
| 334–349 | Close/save confirmation prompts |
| 392–399 | Demo/beta version strings |
| 460–486 | User info, credits, about dialog |
| 548–577 | Sound system messages, palette action labels |
| 750–760 | Main menu bar items (Programs, Palettes, Maps, Graphics, Fonts, Sounds, System, Help) |
| 775–795 | Palettes menu items |
| 800–820 | Maps menu items |
| 825–845 | Graphics (FPG) menu items |
| 850–865 | Fonts menu items |
| 875–895 | System menu items |
| 900–920 | Programs menu items |
| 925–945 | Sounds menu items |
| 950–965 | Edit menu items |

To add a new menu item or dialog label, add an entry to `system/lenguaje.div` and
reference it by its index. To find what string an index maps to, search the file
or use the IDE itself.

---

## The window system

### The window array and the `v` macro

The IDE manages all windows through a fixed-size array:

```c
struct twindow window[MAX_WINDOWS];
#define v window[0]     // convenience macro for the active window
```

`window[0]` is always the **active** (topmost, interacting) window. The `v` macro
lets handler code refer to it concisely: `v.title`, `v.w`, `v.ptr`, etc.

When a different window needs attention, `move(0, n)` swaps it into position 0.
For temporary access without disturbing the array, `wup(n)` saves the current
`window[0]` and loads window `n`, and `wdown(n)` restores it.

### Window types

The `v.type` field determines what kind of window this is:

| Type | Meaning |
|------|---------|
| 0 | Empty slot |
| 1 | Dialog (modal, created by `show_dialog()`) |
| 2 | Dropdown menu |
| 3 | Palette display |
| 4 | Clock |
| 5 | Trash/recycle bin |
| 7 | Progress bar |
| 8 | Mixer |
| 100 | Map editor |
| 101 | FPG graphics editor |
| 102 | Code editor (PRG) |
| 104 | Font editor |
| 105 | PCM/sound editor |
| 107 | Music module player |

Types >= 100 are "major editors" — they participate in activation/deactivation
logic (only one can be fully active at a time) and support minimize/maximize.

### Window foreground state

`v.foreground` controls how a window is drawn and whether it receives input:

| Value | Meaning |
|-------|---------|
| 0 | Background (dimmed, drawn with `blit_region_dark()`) |
| 1 | Foreground (active, fully rendered, receives input) |
| 2 | Minimized (collapsed to icon on desktop) |

### The twindow struct — key fields

```c
struct twindow {
  int type;                          // Window type (see table above)
  int order;                         // Z-order (higher = more recently created)
  int foreground;                    // 0=dimmed, 1=active, 2=minimized
  byte *name;                        // Icon label text
  byte *title;                       // Title bar text
  void_return_type_t paint_handler;  // Called to render window content
  void_return_type_t click_handler;  // Called on mouse interaction
  void_return_type_t close_handler;  // Called on close (cleanup)
  int x, y, w, h;                   // Screen position and size (pixels)
  byte *ptr;                         // Off-screen pixel buffer
  struct tmapa *mapa;                // Map data (for map/paint windows)
  struct tprg *prg;                  // Program data (for code editor windows)
  int redraw;                        // 1=partial repaint, 2=full repaint
  int state;                         // Window-specific state
  byte *aux;                         // Auxiliary data pointer (e.g. FPG*, pcminfo*)
  struct t_item item[MAX_ITEMS];     // Dialog items (buttons, text fields, checkboxes)
  int items;                         // Number of items
  int active_item;                   // Last activated item index
  int selected_item;                 // Currently keyboard-selected item
};
```

### The handler pattern

Every window is defined by three function pointers, conventionally named with
suffixes `0` (init), `1` (paint), and `2` (click), with an optional `3` (close):

| Handler | When called | What it does |
|---------|-------------|--------------|
| `init_handler` (the `0` function) | Once, during `show_dialog()` or `new_window()` | Sets `v.type`, `v.w`, `v.h`, `v.title`, assigns paint/click/close handlers, adds items |
| `paint_handler` (the `1` function) | After init, and whenever `v.redraw` is set | Draws content into `v.ptr` using drawing primitives |
| `click_handler` (the `2` function) | Every frame while the window is active | Processes mouse/keyboard input, updates state |
| `close_handler` (the `3` function) | When the window is closed | Frees resources, saves results |

Example — a simple dialog:
```c
void my_dialog0(void) {           // Init handler
  v.type = 1;                     // Dialog
  v.w = 160; v.h = 80;
  v.title = texts[MY_TITLE_IDX];
  v.paint_handler = my_dialog1;
  v.click_handler = my_dialog2;
  _button(100, v.w - 8, v.h - 14, 2);  // OK button, right-aligned
}

void my_dialog1(void) {           // Paint handler
  int w = v.w / big2, h = v.h / big2;
  _show_items();                   // Draw all registered items
  wwrite(v.ptr, w, h, 4, 12, 0, texts[MY_MSG], c3);
}

void my_dialog2(void) {           // Click handler
  _process_items();                // Update item states from input
  if (v.active_item == 0)          // OK button was activated
    end_dialog = 1;                // Close the dialog
}
```

### Creating windows

There are two ways to create a window:

**`show_dialog(init_handler)`** — Creates a **modal** dialog:
1. Calls `addwindow()` to find a free slot
2. Sets default values in `v`
3. Calls `init_handler` to customize `v`
4. Allocates `v.ptr` buffer (`w * h` bytes)
5. Draws window chrome (border, title bar gradient, close button)
6. Calls `v.paint_handler` to draw content
7. Optionally plays explosion animation
8. Enters `modal_loop()` — blocks until `end_dialog` or `exit_requested`
9. Calls `v.close_handler` for cleanup

**`new_window(init_handler)`** — Creates a **non-modal** window:
- Same setup steps but doesn't enter `modal_loop()`
- Window participates in the main event loop instead
- Used for menus, editor windows, palette display, progress bars

### The big mode factor

DIV supports a "big" mode that doubles all pixel coordinates. Throughout window
code you'll see:
```c
int w = v.w / big2, h = v.h / big2;  // big2 = big ? 2 : 1
```
The `v.w` and `v.h` store the actual pixel size (doubled in big mode), but drawing
primitives work in logical coordinates, so you divide by `big2` before calling them.

### Window rendering

Each window draws into its own off-screen buffer `v.ptr`. The rendering pipeline:

1. `paint_handler` draws into `v.ptr` using primitives
2. `v.redraw = 1` signals that the buffer has changed
3. `flush_window(n)` blits `window[n].ptr` onto `screen_buffer`
4. `flush_copy()` uploads `screen_buffer` to the SDL2 texture and presents it

Drawing primitives all take `(dest, width, height, ...)` parameters where `dest`
is `v.ptr` and width/height are the logical (divided by big2) dimensions:

| Function | Purpose |
|----------|---------|
| `wbox(ptr, w, h, color, x, y, bw, bh)` | Fill a rectangle |
| `wrectangle(ptr, w, h, color, x, y, bw, bh)` | Draw a rectangle outline |
| `wwrite(ptr, w, h, x, y, align, text, color)` | Draw text (align: 0=left, 1=center, 2=right) |
| `wwrite_in_box(ptr, w, clip_w, h, x, y, align, text, color)` | Draw text with horizontal clipping |
| `wput(ptr, w, h, x, y, gfx_id)` | Draw a UI graphic/icon (from `graf[]`) |
| `wgra(ptr, w, h, color, x, y, gw, gh)` | Draw a gradient bar (used for title bars) |
| `wresalta_box(...)` | Toggle-highlight a rectangle |

**Color constants** (`c0` through `c34`, `c_b_low`, `c_r_low`, etc.) are palette
indices found by `find_colors()` at startup. They adapt to whatever palette is loaded.

---

## The menu system

### How menus are defined

Each menu is defined by a block of consecutive `texts[]` entries:

```
texts[N]   = icon label (shown in the menu bar)
texts[N+1] = window title
texts[N+2] = first menu item  (or NULL to end)
texts[N+3] = second menu item (or NULL to end)
...
```

Items starting with `"-"` render a separator line above the text. Items containing
`"[shortcut]"` display the shortcut right-aligned.

### Menu text indices

| Base index | Menu |
|------------|------|
| 750 | Main menu bar (8 items: Programs, Palettes, Maps, Graphics, Fonts, Sounds, System, Help) |
| 775 | Palettes |
| 800 | Maps |
| 825 | Graphics (FPG) |
| 850 | Fonts |
| 875 | System |
| 900 | Programs |
| 925 | Sounds |
| 950 | Edit (code editor context) |

### Menu infrastructure functions

**`create_menu(base_index)`** — Builds a dropdown menu window from consecutive
`texts[]` entries. Sets `v.type = 2`, calculates width from longest item, height
from item count (9 pixels per item + 11 pixel header).

**`paint_menu(base_index)`** — Renders menu items into `v.ptr`. Draws separator
lines for `-` prefixed items, right-aligns `[shortcut]` text.

**`update_menu(base_index, min_disabled, max_disabled)`** — Per-frame interaction
handler. Highlights the item under the cursor, greys out items in the disabled
range `[min..max]`. Sets `v.state` to the highlighted item number (negative while
mouse button is held, positive on hover).

### Menu dispatch pattern

Each menu follows the `0/1/2` handler convention:

```c
void menu_maps0(void) {
  create_menu(800);                    // Build from texts[800..]
  v.paint_handler = menu_maps1;
  v.click_handler = menu_maps2;
}

void menu_maps1(void) {
  paint_menu(800);                     // Render items
}

void menu_maps2(void) {
  if (find_map_window())               // Context-dependent enable/disable
    update_menu(800, 1, 0);            // All items enabled
  else
    update_menu(800, 3, 9);            // Items 3–9 disabled (no map open)

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {  // On button release
    switch (v.state) {
    case 1: /* New map */   show_dialog(new_map0);  break;
    case 2: /* Open map */  show_dialog(browser0);  break;
    case 3: /* Close map */ ...                     break;
    // ...
    }
  }
}
```

### The main menu bar

`menu_main2()` in `handler.c` dispatches clicks on the top-level menu bar:
- Item 1 → `new_window(menu_programs0)`
- Item 2 → `new_window(menu_palettes0)`
- Item 3 → `new_window(menu_maps0)`
- Item 4 → `new_window(menu_graphics0)`
- Item 5 → `new_window(menu_fonts0)`
- Item 6 → `new_window(menu_sounds0)`
- Item 7 → `new_window(menu_system0)`
- Item 8 → `help(3)` (opens help)

### Adding a new menu

1. Add item strings to `system/lenguaje.div` at a free index range
2. Write `menu_foo0/1/2` functions following the pattern above
3. Add the dispatch case to the parent menu's click handler
4. Add any `find_foo_window()` helper if needed for enable/disable logic

---

## The dialog item system

Dialogs use a mini widget toolkit built into each window's `item[]` array.

### Item types

| Type | Widget | Builder | What it does |
|------|--------|---------|--------------|
| 1 | Button | `_button(text_idx, x, y, align)` | Clickable label, triggers `v.active_item` |
| 2 | Text input | `_get(text_idx, x, y, w, buffer, max_len, min_val, max_val)` | Editable text field with optional numeric range validation |
| 3 | Checkbox | `_flag(text_idx, x, y, &int_value)` | Toggles an integer between 0 and 1 |

### Item lifecycle

1. **Init handler** registers items:
   ```c
   _button(100, v.w - 8, v.h - 14, 2);            // OK button
   _button(101, 7, v.h - 14, 0);                   // Cancel button
   _get(460, 90, 22, 120, (byte*)name_buf, 127, 0, 0);  // Text input
   _flag(569, 4, 40, &my_option);                   // Checkbox
   ```
   The first button/get added becomes the initially selected item.

2. **Paint handler** calls `_show_items()` to render all items.

3. **Click handler** calls `_process_items()` every frame. This:
   - Checks each item's status (hover, pressed, keyboard-selected)
   - Updates visual state (highlight, depress)
   - Sets `v.active_item` when an item is activated (button clicked, enter pressed)
   - Handles Tab to cycle between items
   - Handles Esc to revert text input changes

4. **Click handler** reads `v.active_item` to determine what was activated:
   ```c
   _process_items();
   switch (v.active_item) {
   case 0: v_accept = 1; end_dialog = 1; break;  // OK
   case 1: end_dialog = 1; break;                 // Cancel
   }
   ```

### Button alignment modes

The `center` parameter (0–8) controls anchor point positioning using a 3x3 grid:

| | Left (0,3,6) | Center (1,4,7) | Right (2,5,8) |
|--|---|---|---|
| **Top** | 0 | 1 | 2 |
| **Middle** | 3 | 4 | 5 |
| **Bottom** | 6 | 7 | 8 |

Common usage: `0` = left-aligned, `1` = centered, `2` = right-aligned.

### Text input validation

`_get()` supports numeric range validation via `r0` and `r1`:
- If `r0 == r1` (both 0): free text, value copied as-is
- If `r0 != r1`: input is parsed as integer and clamped to `[r0..r1]`
  with an error dialog if out of range

---

## The dialog v_ parameter system

Modal dialogs communicate through a set of global variables prefixed with `v_`:

| Variable | Type | Purpose |
|----------|------|---------|
| `v_title` | `char*` | Dialog title (for confirm/info dialogs) |
| `v_text` | `char*` | Message body text |
| `v_mode` | `int` | File browser mode: 0=open, 1=save-as, 2=save-to-dir |
| `v_type` | `int` | File type filter (2=map, 3=pal, 4=fpg, 5=fnt, 7=pcm, 8=prg) |
| `v_finished` | `int` | File dialog completed (1=yes) |
| `v_exists` | `int` | Selected file exists (1=yes) |
| `v_accept` | `int` | User clicked OK/accept (1=yes) |
| `v_window` | `int` | Target window index |
| `v_help` | `int` | Help was requested |
| `v_map` | `tmapa*` | Map data pointer |
| `v_prg` | `tprg*` | Program data pointer |
| `input[]` | `char[512]` | Filename/text input buffer (set by file browser) |

### Common dialog usage patterns

**Error dialog:**
```c
v_text = (char *)texts[43];    // "File not found"
show_dialog(err0);             // Shows error with OK button
```

**Confirmation dialog:**
```c
v_title = (char *)texts[188];  // "Close program"
v_text = (char *)window[n].title;
show_dialog(accept0);
if (v_accept) {
  // user confirmed
}
```

**File browser:**
```c
v_type = 8;                    // Program files (*.PRG)
v_mode = 0;                    // Open mode
v_text = (char *)texts[346];   // "Open program"
show_dialog(browser0);
if (v_finished) {
  if (v_exists) {
    // input[] contains selected filename
    // file_types[v_type].path contains the directory
  } else {
    v_text = (char *)texts[43];
    show_dialog(err0);         // "File not found"
  }
}
```

---

## Standard dialog catalog

These are the ready-made dialog init functions in `handler_dialogs.c`:

| Function | Purpose | Inputs | Outputs |
|----------|---------|--------|---------|
| `err0` | Error message + OK | `v_text` | — |
| `errhlp0` | Error + OK + Help buttons | `v_text` | `v_accept` (1 if Help clicked) |
| `info0` | Info message + OK | `v_title`, `v_text` | — |
| `accept0` | OK/Cancel confirmation | `v_title`, `v_text` (optional) | `v_accept` |
| `browser0` | File open/save browser | `v_type`, `v_mode`, `v_text` | `v_finished`, `v_exists`, `input[]` |
| `palette_action0` | Palette conflict resolution | `work_palette[]` | `v_accept` (1=keep current, 2=merge, 3=use new) |
| `copyright0` | Startup copyright/splash | — | — |
| `user_info0` | User name/company input | — | `user1[]`, `user2[]` |
| `about0` | About DIV dialog | — | — |
| `test0` | Video mode test (auto-reverts after timeout) | — | — |
| `progress0` | Progress bar (non-modal) | `progress_title`, `progress_total` | — |

### The progress bar

The progress bar is special — it uses `show_progress()` as a
stateful helper rather than being invoked through `show_dialog()`:

```c
show_progress("Loading...", 0, total);     // First call: creates window
for (int i = 0; i < total; i++) {
  // ... do work ...
  show_progress("Loading...", i, total);   // Update progress
}
show_progress("Loading...", total, total);  // Final call: closes window
```

---

## The listbox widget

Listboxes are used by the file browser and other scrollable list dialogs.

### Structure

```c
struct t_listbox {
  int x, y;          // Position within window
  char *list;        // Flat buffer of fixed-width strings
  int item_width;    // Bytes per item (including null)
  int visible_items; // How many rows are visible
  int w, h;          // Pixel dimensions (auto-calculated if 0)
  int first_visible; // Scroll offset (index of first visible item)
  int total_items;   // Total item count
  int s0, s1, slide; // Scrollbar geometry
  int zone;          // Mouse hit zone (0=none, 2=up, 3=down, 4=slider, 10+=item)
};
```

### API

| Function | Purpose |
|----------|---------|
| `create_listbox(l)` | Initialize and draw the listbox (border, scroll buttons) |
| `paint_listbox(l)` | Redraw items and detect mouse zone |
| `paint_slider(l)` | Redraw scrollbar thumb |
| `update_listbox(l)` | Per-frame interaction: scroll, highlight, slider drag |

### The file browser's listboxes

The file browser (`browser0` and friends) uses four pre-allocated listboxes:

| Listbox | Contents | Buffer | Item width |
|---------|----------|--------|------------|
| `file_list` | Files matching current mask | `files_buf[MAX_FILES * FILE_SIZE]` | `FILE_SIZE` |
| `dir_list` | Subdirectories | `dirs_buf[MAX_FOLDERS * FOLDER_SIZE]` | `FOLDER_SIZE` |
| `drive_list` | Available drives | `drives_buf[26 * 5]` | 5 |
| `ext_list` | File extension filters | `ext[26 * 6]` | 6 |

The `analyze_input()` function handles typing in the filename field: wildcard
expansion, directory navigation, extension appending, and file existence checks.

---

## The main event loop

### Entry point: `mainloop()` in `main.c`

Called repeatedly by `main_loop()` until `exit_requested` is set. Each iteration:

1. **Poll input:** `poll_keyboard()` reads keyboard state
2. **Hit-test windows:** Walk `window[]` array to find which window the mouse is over
3. **Handle drag-and-drop:** Check if a graphic is being dragged between windows
4. **Determine cursor shape:** Set `mouse_graf` based on what the mouse is over:
   - Over nothing → `1` (normal pointer)
   - Over title bar → `2` (move cursor)
   - Over minimize button → `4`
   - Over close button → `5`
   - Over dimmed window → `7` (scroll-up icon, indicating "click to activate")
5. **Activate/focus:** If a non-frontmost window is clicked, `move(0, n)` brings it
   to front and dims any overlapping windows
6. **Dispatch to click handler:** If mouse is over the content area of the active
   window, compute `wmouse_x`/`wmouse_y` (window-relative coords) and call
   `v.click_handler`
7. **Handle chrome:** Title bar click → `move_window()`, minimize click →
   `minimize()`, close click → `close_window()`
8. **Keyboard shortcuts:** Ctrl+N/O/S/Z etc. dispatch to menu actions
9. **Render:** `flush_window(0)` if `v.redraw` is set, then `flush_copy()`

### Modal dialog loop: `dialog_loop()` in `main_dialogs.c`

A simplified version of the main loop used inside `modal_loop()`:

1. Only tests mouse against the active dialog window (position 0)
2. Computes `wmouse_x`/`wmouse_y` and calls `v.click_handler`
3. Handles title bar drag (`move_window()`) and close button
4. ESC closes the dialog (unless a text field is being edited)
5. Clicking outside the dialog closes it (for menus, `draw_mode < 100`)
6. Calls `v.click_handler` at least once per frame even with no mouse input
   (so text cursor blinks, animations play, etc.)
7. `flush_copy()` at end of each iteration

---

## Mouse input

### Key global variables

| Variable | Purpose |
|----------|---------|
| `mouse_x`, `mouse_y` | Screen-space mouse position |
| `mouse_b` | Button bitmask: bit 0=left, bit 1=right, bit 2=scroll-down, bit 3=scroll-up |
| `prev_mouse_buttons` | Previous frame's `mouse_b` (for edge detection) |
| `wmouse_x`, `wmouse_y` | Window-relative coords (set before calling click handler) |
| `mouse_graf` | Current cursor graphic ID |
| `scan_code` | Last keyboard scan code |
| `ascii` | Last ASCII character typed |
| `shift_status` | Modifier key bitmask |

### Edge detection pattern

Button release is detected by comparing current and previous state:
```c
if ((prev_mouse_buttons & 1) && !(mouse_b & 1))  // Left button just released
```

This pattern is used everywhere for menu selection, button activation, etc.

### Hit testing

Two helper macros/functions for point-in-rectangle tests:

| Function | Coordinates | Usage |
|----------|-------------|-------|
| `mouse_in(x1, y1, x2, y2)` | Screen space (absolute corners) | Main loop window detection |
| `wmouse_in(x, y, w, h)` | Window space (relative, x/y/width/height) | Inside click handlers |

---

## Window finder functions

Handler code needs to find open windows of specific types. These functions
scan `window[]` and return the index of the first matching window (or 0 if none):

| Function | Finds window type |
|----------|-------------------|
| `find_program_window()` | 102 (code editor) — also sets `v_window` |
| `find_map_window()` | 100 (map editor) |
| `find_and_load_map()` | 100 — also loads `v_map`, `map`, `map_width`, `map_height` |
| `find_fpg_window()` | 101 (FPG graphics editor) |
| `find_font_window()` | 104 (font editor) |
| `find_pcm_window()` | 105 (sound editor) |
| `find_mod_window()` | 107 (music module player) |

These are used both for menu enable/disable logic and for finding the target
window when a menu action operates on "the current X."

---

## Window management operations

These functions in `main_desktop.c` handle window lifecycle and layout:

| Function | What it does |
|----------|--------------|
| `addwindow()` | Find free slot in `window[]`, shift array to make room at position 0 |
| `divdelete(n)` | Free window `n` — calls `close_handler`, frees `v.ptr`, clears slot |
| `close_window()` | Close `window[0]` with optional implosion animation |
| `move(a, b)` | Swap window positions (bring `b` to position `a`) |
| `move_window()` | Interactive drag: follows mouse until button release, then `on_window_moved()` |
| `activate()` | "Activate" a type>=100 window (calls its click handler to enter its editor loop) |
| `minimize()` | Collapse window to icon (sets `foreground=2`, saves position) |
| `maximize()` | Restore minimized window |
| `flush_window(n)` | Blit `window[n].ptr` onto `screen_buffer` |
| `flush_copy()` | Upload `screen_buffer` to SDL2 texture, present frame |
| `on_window_moved(x, y, w, h)` | Repaint area vacated by moved window |
| `update_box(x, y, w, h)` | Redraw all windows that overlap a given screen rectangle |
| `explode(x, y, w, h)` | Window-open animation |

### Window placement

New windows are centered on screen by `show_dialog()`:
```c
x = vga_width / 2 - w / 2;
y = vga_height / 2 - h / 2;
```

Non-modal windows created by `new_window()` are placed at the position set by
the init handler or at a default calculated position.

---

## Desktop persistence (session save/restore)

`desktop.c` handles saving and restoring the entire IDE state between sessions.

### Save: `download_desktop()`

Called on clean exit. Writes `system/session.dtf`:
1. Header: `"dtf\x1a\x0d\x0a\x00"`
2. Video mode (width, height, big flag)
3. Window count
4. Palette (768 bytes DAC) + ghost table (64KB)
5. For each open window: type, position, and type-specific data:
   - Map windows: palette, bitmap data, zoom state, control points
   - Code windows: filename, path, source buffer, cursor position
   - FPG windows: full FPG file data
   - Sound windows: PCM data
   - Font windows: font data
   - Calculator: display state

### Restore: `upload_desktop()`

Called at startup if `system/session.dtf` exists. Recreates windows at their
saved positions. If the video resolution changed since the session was saved,
the `vid_mode_changed` flag adjusts window placement.

### Config: `system/setup.bin`

The `SetupFile` struct is saved/loaded by `save_config()` / `load_config()` and
controls IDE preferences: video mode, editor font, auto-save, sound settings,
window animation, etc.

---

## How editors integrate with the IDE shell

Each editor (code, paint, font, sound) is a window with type >= 100 that uses
the standard handler pattern but has its own internal event loop.

### Code editor (type 102)

- Init: `carga_programa0()` creates the window, loads the `.prg` file into
  `v.prg` (a `tprg` struct with source buffer, cursor state, etc.)
- Click handler: `editor()` in `editor/editor.c` — a full editor loop that
  handles keyboard input, text manipulation, syntax coloring, and rendering
- The editor becomes "activated" when clicked (via `activate()`) and runs its
  loop until the user clicks outside the window
- See `docs/code-editor-overview.md` for detailed internals

### Paint editor (type 100)

- A map window becomes a paint editor when double-clicked or when Maps > Edit
  is selected
- `activate()` calls the click handler which enters the paint editor loop
- `draw_mode` tracks the current tool (0=paint, 1–8=specific tools, 100+=menu)
- See `docs/paint-editor-overview.md` for detailed internals

### Pattern: wup/wdown for cross-window operations

When a menu action needs to operate on a window that isn't currently active:
```c
if ((n = find_program_window())) {
  wup(n);           // Save window[0], load window[n] into window[0]
  write_line();      // Operate on it
  read_line();
  wdown(n);          // Restore original window[0]
  flush_window(n);   // Redraw it at its original position
}
```

---

## File type system

The `file_types[]` array manages per-type state for the file browser:

```c
struct {
  char path[PATH_MAX+1];  // Current directory for this file type
  char *ext;               // Filter string (e.g. "*.PRG *.*")
  int default_choice;      // Last selected extension filter index
  int first_visible;       // Scroll position in file list
} file_types[24];
```

Key type indices:

| Index | File type |
|-------|-----------|
| 2 | Maps (MAP, PCX, BMP, JPG, PNG, etc.) |
| 3 | Palettes (PAL, FPG, FNT, MAP, etc.) |
| 4 | FPG files |
| 5 | FNT (font) files |
| 7 | Audio files (WAV, PCM, MP3, OGG, etc.) |
| 8 | Program files (PRG) |
| 9 | Wallpaper images |
| 16 | Tracker modules (MOD, S3M, XM, MID) |

Set `v_type` before calling `show_dialog(browser0)` to filter the file list.

---

## Cookbook: common IDE modifications

### Adding a new menu item to an existing menu

1. Add the item text to `system/lenguaje.div` (in the menu's text range, before
   the terminating NULL entry)
2. In the menu's click handler (e.g. `menu_system2`), add a `case` for the new
   item number
3. If the item should be conditionally disabled, adjust the `update_menu()` call's
   min/max range

### Creating a new dialog

1. Write `my_dialog0/1/2` functions following the handler pattern
2. Register items in `my_dialog0` with `_button`, `_get`, `_flag`
3. Call `_show_items()` in `my_dialog1`
4. Call `_process_items()` in `my_dialog2` and check `v.active_item`
5. Set `end_dialog = 1` to close, `v_accept = 1` to signal success
6. Invoke with `show_dialog(my_dialog0)`

### Creating a new tool window (non-modal)

1. Write init/paint/click handler functions
2. Set `v.type` to a unique value (or reuse an existing type if appropriate)
3. The click handler runs every frame — handle mouse via `wmouse_x`/`wmouse_y`
4. Set `v.redraw = 1` when content changes
5. Invoke with `new_window(my_window0)`

### Adding a new editor type

1. Assign a new type number >= 100
2. Write init/paint/click handlers
3. The click handler should implement an internal event loop (like `editor()`)
4. Add a `find_foo_window()` function to `handler.c`
5. Add menu items to open/close/save the new editor type
6. Add save/restore support in `desktop.c` for session persistence

---

## Key gotchas

- **`v` is `window[0]`** — any `show_dialog()` call inside a click handler
  will push the current window aside. After the dialog returns, `v` points to
  a different window. Use `wup(n)`/`wdown(n)` if you need to preserve context.

- **`v.active_item` is transient** — it's set to the index of the just-activated
  item by `_process_items()` and reset to -1 on the next call. Check it
  immediately after `_process_items()`.

- **`end_dialog` vs `close_window()`** — Set `end_dialog = 1` from inside a
  modal dialog's click handler. Call `close_window()` from outside or for
  non-modal windows. Don't mix them up.

- **`prev_mouse_buttons` for edge detection** — Always use the
  `(prev_mouse_buttons & 1) && !(mouse_b & 1)` pattern for button release.
  Testing `mouse_b` alone will fire every frame the button is held.

- **big mode** — Always divide `v.w` and `v.h` by `big2` before passing to
  drawing primitives. Forgetting this doubles all coordinates in big mode.

- **`flush_window()` then `flush_copy()`** — Window buffer changes aren't
  visible until both calls happen. In modal dialogs this happens automatically
  at the end of `dialog_loop()`.
