# DIV Games Studio - Architecture Overview

This document describes the high-level architecture of the DIV Games Studio IDE,
covering the main loop, event handling, window system, menu system, localization,
and file format support.

---

## 1. IDE Main Loop and Event Dispatch

### Entry Point: `main()` in `src/div.c` (line 389)

The `main()` function performs the following startup sequence:

1. **`OSDEP_Init()`** -- Calls `SDL_Init(SDL_INIT_EVERYTHING)` and redirects
   stdout/stderr on Windows.
2. **Clock and flags** -- Sets `system_clock`, `cpu_type`, `return_mode`,
   `exit_requested`, `MustCreate`, etc.
3. **Command-line parsing** -- Supports `INIT` (fresh start), `TEST` (video test),
   and `-c` (command-line compiler mode). In `-c` mode, the compiler runs
   standalone without the GUI and exits.
4. **Working directory** -- Resolves the executable path, changes to the DIV
   runtime directory (where `system/` lives).
5. **File type extensions** -- Populates `tipo[0..16].ext` with file browser
   filter strings for each file type (MAP, PAL, FPG, FNT, PRG, etc.).
6. **`initialize_texts("system/lenguaje.div")`** -- Loads the localization strings.
7. **Joystick + mixer init** -- Opens the first joystick, initializes SDL2_mixer.
8. **`Load_Cfgbin()`** -- Loads `system/setup.bin` configuration file.
9. **`initialization()`** -- Allocates all core buffers (see below).
10. **`init_environment()`** -- Shows copyright dialog, restores desktop session.
11. **`main_loop()`** -- Enters the main event loop.
12. **Shutdown** -- Saves session, calls `finalization()`, `finalize_texts()`,
    returns `return_mode` (0=quit, 1=run program, 3=test failure).

### Initialization: `initialization()` at line 3368

This function allocates all fundamental IDE resources:

- **Video mode detection** via `detect_vesa()` (sets `vga_width`, `vga_height`, `big`)
- **Keyboard** via `kbdInit()`
- **Help system** via `make_helpidx()` + `load_index()`
- **Core buffers**:
  - `undo` -- undo history buffer (`undo_memory` bytes)
  - `screen_buffer` -- virtual screen copy (`vga_width * vga_height` bytes, offset by 6)
  - `dac` / `dac4` -- 768-byte palette buffers
  - `cuad` -- 16384-byte squared-difference table for palette lookups
  - `ghost` -- 65536-byte (256x256) transparency/layering table
  - `toolbar` -- paint toolbar buffer
  - `fill_dac` -- 256-byte fill color flags
  - `error_window` -- error display buffer
  - `mouse_background` -- mouse cursor background save buffer
- **Fonts**: Loads `system/pequeno.fon` or `system/grande.fon` (proportional UI
  font), plus a fixed-width editor font (`SYS06X08.BIN` through `SYS09X16.BIN`)
- **UI graphics**: Loads `GRAF_P.DIV` or `GRAF_G.DIV` (256 indexed icon sprites)
- **Help figures**: Loads `help/help.fig`

### Window array initialization

```c
struct twindow window[max_windows];  // max_windows = 96
#define v window[0]                  // v is always the frontmost window
```

All windows are initialized with `type=0` (empty/inactive).

### Main Loop: `main_loop()` at line 1697

```c
void main_loop(void) {
    div_started = 1;
    do {
        mainloop();
    } while (!exit_requested);
}
```

The actual work is in `mainloop()` (line 857), which runs once per iteration:

1. **`poll_keyboard()`** -- Pumps the SDL event queue, updates `mclock`/`reloj` timestamps.
2. **Window hit-test** -- Scans `window[0..max_windows-1]` to find which window
   the mouse is over (variable `n`). `n=max_windows` means background.
3. **Drag-and-drop** -- Handles dragging sprites/maps between windows and the
   desktop (`dragging` state machine: 0=idle, 1=pending, 2=dragging, 3=done,
   4=dropping, 5=dropped).
4. **Cursor shape** -- Sets `mouse_graf` based on position within the window
   (1=arrow, 2=move, 4=minimize, 5=close, 6=resize, 7=background).
5. **Window focusing** -- If the mouse is on a non-frontmost window, it gets
   moved to `window[0]` (the front). Background windows (`foreground=0`)
   are drawn dimmed.
6. **Click dispatch** -- If the mouse is inside the content area of the front
   window, calls `v.click_handler()` with window-relative coordinates in
   `wmouse_x`/`wmouse_y`.
7. **Title bar interactions** -- Handles window move, minimize, maximize, close.
8. **Keyboard shortcuts** -- F1-F12 hotkeys, Alt+X quit, Ctrl+C, menu shortcuts.
9. **`flush_copy()`** -- Blits the virtual screen (`screen_buffer`) to the display.

### Dialog Loop: `dialog_loop()` at line 1731

Modal dialogs use a secondary event loop (`show_dialog()` calls
`dialog_loop()` repeatedly until `end_dialog` or `salir_del_dialogo`). This
loop is simpler: it only handles the single dialog window, its title bar, and
the close button. The dialog is always `window[0]`.

### Event Flow: SDL2 to IDE

```
SDL_PollEvent()
    |
    v
read_mouse2() [src/divmouse.c:376]
    |-- SDL_TEXTINPUT     --> ascii
    |-- SDL_MOUSEWHEEL    --> m_b bits 2,3
    |-- SDL_MOUSEMOTION   --> m_x, m_y
    |-- SDL_MOUSEBUTTONDOWN/UP --> m_b bits 0,1
    |-- SDL_KEYDOWN       --> scan_code, kbdFLAGS[], ascii (for BS/Enter/Tab)
    |-- SDL_KEYUP         --> kbdFLAGS[] cleared
    |-- SDL_WINDOWEVENT   --> resize, focus, enter/leave, close
    |-- SDL_QUIT          --> exit_requested = 1
    v
read_mouse() [src/divmouse.c:48]
    |-- Scales m_x/m_y to virtual resolution
    |-- Handles keyboard-as-mouse (OPQA keys in paint mode)
    |-- Handles spacebar-as-click (magic value 0x8001)
    |-- Clamps to vga_width/vga_height bounds
    |-- Updates coord_x/coord_y (map coordinates under cursor)
    v
poll_keyboard() [src/divkeybo.c:127]
    |-- Updates mclock and reloj from SDL_GetTicks()
    |-- (The actual event pumping happens via read_mouse2)
```

The `kbdFLAGS[128]` array maps DOS-style scan codes to key-down state. SDL
keysyms are translated to DOS scan codes via the `OSDEP_key[2048]` lookup table
(initialized in `OSDEP_keyInit()`).

### Shutdown: `finalization()` at line 3682

Frees all buffers allocated in `initialization()`, restores video mode via
`rvmode()`, calls `end_lexcolor()` and `kbdReset()`.

---

## 2. OSDEP Abstraction Layer

Defined in `src/shared/osdep/osd_sdl2.c` and `osd_sdl2.h`.

### Key Types

```c
typedef SDL_Surface  OSDEP_Surface;
typedef SDL_Color    OSDEP_Color;
typedef SDL_Joystick OSDEP_Joystick;
```

### Key Functions

| Function | What it does |
|---|---|
| `OSDEP_Init()` | `SDL_Init(SDL_INIT_EVERYTHING)` |
| `OSDEP_Quit()` | `SDL_Quit()` |
| `OSDEP_SetVideoMode(w,h,bpp,fs)` | Creates window+renderer (first call) or resizes window. Creates 8-bit and 32-bit surfaces + streaming texture. |
| `OSDEP_Flip(s)` | Blits 8-bit surface -> 32-bit surface -> texture -> renderer -> present. This is the core rendering path. |
| `OSDEP_SetPalette(surface,colors,first,n)` | Sets the 8-bit surface's palette via `SDL_SetPaletteColors()` |
| `OSDEP_WarpMouse(x,y)` | `SDL_WarpMouseInWindow()` |
| `OSDEP_SetWindowSize(w,h)` | `SDL_SetWindowSize()` + updates vwidth/vheight |
| `OSDEP_GetTicks()` | `SDL_GetTicks()` |

### 8-bit to 32-bit Rendering Pipeline (IDE)

```
screen_buffer (byte* vga_width*vga_height)  -- the virtual framebuffer (8-bit paletted)
    |
    v
blit_screen(screen_buffer)       -- copies screen_buffer to OSDEP_buffer8->pixels
    |
    v
OSDEP_Flip()
    |-- SDL_BlitSurface(buffer8 -> buffer32)   -- palette conversion
    |-- SDL_UpdateTexture(texture, buffer32)
    |-- SDL_RenderCopy(renderer, texture)
    |-- SDL_RenderPresent(renderer)
```

---

## 3. Window System

### Window Structure: `struct twindow` in `src/global.h` (line 676)

Each window has:
- `type` -- Window type: 0=empty, 1=dialog, 2=menu, 3=palette, 4=timer,
  100=map editor, 101=FPG editor, 102=program editor, etc.
- `foreground` -- 0=background, 1=foreground, 2=minimized
- `paint_handler` -- Function called to repaint the window content
- `click_handler` -- Function called when the user interacts with the window
- `close_handler` -- Function called when the window is closed
- `x, y, an, al` -- Position and dimensions
- `ptr` -- Byte buffer for the window's rendered content
- `mapa` -- Pointer to a `struct tmapa` (for map editors)
- `prg` -- Pointer to a `struct tprg` (for program editors)
- `item[max_items]` -- Up to 24 UI items (buttons, text fields, checkboxes)
- `order` -- Z-order number
- `redraw` -- Flag indicating the window needs redrawing

The front window is always `window[0]`, accessed via the macro `v`.
When a window gains focus, it is moved to index 0 in the array via `move(0, n)`.

### Window Rendering Primitives: `src/divwindo.c`

All drawing operates on a window's `ptr` buffer (an 8-bit pixel array):

- `wbox()` -- Filled rectangle
- `wrectangle()` -- Rectangle outline
- `wwrite()` -- Text rendering using the proportional UI font
- `wput()` -- Icon/graphic rendering from the `graf[]` array
- `wvolcado()` -- Blit a sub-image into the window
- `wline()` -- Line drawing
- `boton()` -- Draw a text button
- `ratonboton()` -- Hit-test a text button

### Dialog System

Dialogs are created by:
1. Calling `show_dialog(handler0)` where `handler0` sets up the window
2. The handler sets `v.type=1`, `v.title`, `v.an`, `v.al`
3. The handler assigns `v.paint_handler` and `v.click_handler`
4. The handler calls `_button()`, `_get()`, `_flag()` to add UI items
5. `show_dialog()` runs `dialog_loop()` until `end_dialog=1`

### UI Items: `struct t_item` in `src/global.h` (line 653)

Three types:
- **Button** (`type=1`): Text label with position and centering mode.
  `_button(texts_index, x, y, center)`.
- **Text field / Get** (`type=2`): Text input with buffer.
  `_get(texts, x, y, width, buffer, max_len, r0, r1)`.
- **Checkbox / Flag** (`type=3`): Boolean toggle bound to an int variable.
  `_flag(texts, x, y, &variable)`.

Items are processed via `_show_items()` (rendering) and `_process_items()`
(input handling). When an item is activated, `v.active_item` is set to its
index.

---

## 4. Menu System

### Menu Creation: `create_menu()` in `src/divhandl.c`

Menus are built from consecutive entries in the `texts[]` array:

```c
void create_menu(int menu) {
    v.type = 2;                    // Window type = menu
    v.name = texts[menu++];        // Icon text
    v.title = texts[menu];         // Title bar text
    v.an = text_len(texts[menu++]) + 23;
    v.al = 11;
    while ((char *)texts[menu]) {  // Each non-NULL entry = menu item
        v.al += 9;                 // 9 pixels per item
        an = text_len(texts[menu++]) + 7;
        if (v.an < an) v.an = an;
    }
}
```

Menu items use a prefix convention:
- `-` prefix: draws a separator line above the item
- `[shortcut]` suffix: displays the keyboard shortcut right-aligned

### Menu Rendering: `paint_menu()`

Draws each item as text on the window buffer, with separators and shortcuts.

### Menu Interaction: `update_menu()`

Highlights the item under the mouse cursor and executes actions on click.

### Main Menu Bar

The main menu is created by `menu_principal0()`. Each top-level menu
(Programs, Maps, Graphics, Fonts, Sounds, System, etc.) has its own trio of
handler functions: `menu_X0()` (create), `menu_X1()` (paint), `menu_X2()`
(click). These are all in `src/divhandl.c`.

---

## 5. Localization: `texts[]` and `lenguaje.div`

### File Format: `system/lenguaje.div`

The localization file is a simple text format parsed by `src/divlengu.c`:

```
34 "DIV Games Studio"
35 "DIV Games Studio 2"
100 "Accept"
101 "Cancel"
...
```

Rules:
- Lines starting with a number set the current text index
- Quoted strings are the text values
- `\n` produces CR+LF, `\#` produces CR only
- `\\` produces a literal backslash
- `#` starts a comment (until end of line)
- Text indices auto-increment (consecutive quoted strings get sequential numbers)
- The file may be encrypted (starts with `"Zk!"` magic, XOR cipher with key `"lave"`)

### Loading: `initialize_texts()` in `src/divlengu.c` (line 29)

1. Reads the entire file into memory
2. Calls `analyze_texts()` which walks the buffer character by character
3. Numbers update `numero` (the current index)
4. Quoted strings are decoded in-place, null-terminated, and stored as
   `texts[numero]` pointers

### Usage

All UI text references go through `texts[N]` where N is the index from
`lenguaje.div`. This allows the entire IDE to be translated by editing a single
file. The English translation was done by Juan Tamargo.

Examples:
- `texts[34]` = Window title ("DIV Games Studio")
- `texts[100]` through `texts[199]` = Button labels
- `texts[200]` through `texts[299]` = Compiler messages
- `texts[500]+` = Error messages

---

## 6. File Formats

### MAP Format (DIV's native bitmap image)

Defined in `src/divforma.c`, starting at line 100.

**Header** (1394+ bytes):

| Offset | Size | Field |
|--------|------|-------|
| +0 | 8 | Magic: `"map\x1a\x0d\x0a\x00\x00"` |
| +8 | 2 | Width (uint16) |
| +10 | 2 | Height (uint16) |
| +12 | 4 | Code (int32, graphic ID) |
| +16 | 32 | Description (ASCIIZ, 32 bytes) |
| +48 | 768 | Palette (256 RGB triplets, 0-63 range) |
| +816 | 576 | Color rules (16 `tipo_regla` structs) |
| +1392 | 2 | Number of control points (uint16) |
| +1394 | N*4 | Control points (pairs of int16 x,y) |
| +1394+N*4 | W*H | Pixel data (8-bit paletted, row-major) |

Detection: `es_MAP()` checks for `"map\x1a\x0d\x0a"` magic.

### PCX Format

Defined in `src/divforma.c`, starting at line 176.

Standard ZSoft PCX format. DIV supports:
- 8-bit, 1-plane (256-color paletted)
- 1-bit, 4-plane (16-color)
- 8-bit, 3-plane (24-bit RGB, quantized to palette)

Detection: `es_PCX()` checks byte 2 (encoding=1) and validates bits_per_pixel
and color_planes combinations.

### BMP Format

Defined in `src/divforma.c`. Supports standard Windows BMP with:
- 1, 4, 8, and 24 bits per pixel
- BI_RGB (uncompressed) and BI_RLE8 (run-length encoded)

### JPEG Format

Enabled when `HAS_JPEG=1`. Uses libjpeg-turbo. Decompress produces a 24-bit
image which is quantized to the current 256-color palette.

### FPG Format (sprite collection / "Fichero de Paquetes Graficos")

Defined in `src/fpgfile.c` and `src/fpgfile.hpp`.

**File structure:**

| Offset | Size | Field |
|--------|------|-------|
| +0 | 7 | Magic: `"fpg\x1a\x0d\x0a\x00"` |
| +7 | 1 | Version byte (0 = 32-bit) |
| +8 | 768 | Palette (256 RGB triplets) |
| +776 | 576 | Color rules |
| +1352 | ... | Graphic entries (repeated) |

**Per-graphic entry** (`HeadFPG` struct, `src/fpgfile.hpp` line 1):

| Offset | Size | Field |
|--------|------|-------|
| +0 | 4 | COD (int32 graphic code, 1-999) |
| +4 | 4 | LONG (int32 total length) |
| +8 | 32 | Description (ASCIIZ) |
| +40 | 12 | Filename (ASCIIZ) |
| +52 | 4 | Width (int32) |
| +56 | 4 | Height (int32) |
| +60 | 4 | Number of control points (int32) |
| +64 | N*4 | Control points (pairs of int16 x,y) |
| +64+N*4 | W*H | Pixel data (8-bit paletted) |

FPG files can hold up to 999 graphics, indexed by their COD field.

### PAL Format (palette)

Magic: `"pal\x1a\x0d\x0a"`. Contains a 768-byte palette (256 RGB values, 0-63
range) followed by color rules.

### FNT Format (bitmap font)

Managed in `src/divfont.c`. DIV bitmap fonts store each character glyph as a
small indexed bitmap within an FPG-like container. The font file contains a
header followed by per-character metrics and pixel data. Character codes map to
glyph entries.

---

## 7. Key Data Structures

### `struct tmapa` (`src/global.h` line 710)

Represents an open map/image in the editor:
- `map` -- Pixel buffer (byte pointer)
- `map_an`, `map_al` -- Width and height
- `zoom`, `zoom_x/y`, `zoom_cx/cy` -- Viewport state
- `puntos[512]` -- Up to 256 control points (x,y pairs as int16)
- `code` -- Graphic ID (for FPG association)
- `description[32]` -- Description text
- `path`, `filename` -- File on disk

### `struct tprg` (`src/global.h` line 724)

Represents an open program source file in the editor:
- `buffer` -- Source text buffer
- `buffer_lon`, `file_lon` -- Buffer capacity and file size
- `num_lineas` -- Line count
- `linea`, `columna` -- Cursor position
- `lptr` -- Pointer to current line in buffer
- `vptr` -- Pointer to first visible line
- `primera_linea`, `primera_columna` -- Scroll position
- `l[1024+4]` -- Current line being edited
- `font_an`, `font_al` -- Editor font dimensions

### `SetupFile` struct (`src/global.h` line 267)

Saved/loaded from `system/setup.bin`. Contains:
- Video mode settings
- Undo configuration
- Directory paths for each file type
- Desktop wallpaper settings
- Color theme (12 RGB values for UI colors)
- Editor font, paint cursor, window behavior preferences
- Audio volume settings
- Mouse ratio and fullscreen flag
