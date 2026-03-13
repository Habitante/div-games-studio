# Paint Editor Architecture

The paint editor is DIV's bitmap editor, invoked when the user edits a
MAP graphic.  It takes over the full screen, replacing the IDE desktop
with a zoomed view of the bitmap and a floating toolbar.

## Source files

| File | Role |
|------|------|
| `editor/paint.c` | Core: primitives (line, bezier, circle), toolbar rendering, zoom viewport, color ruler, UI helpers |
| `editor/paint_tools.c` | 14 interactive tool modes (`edit_scr`, `edit_mode_0`..`edit_mode_9`, `edit_mode_11`..`edit_mode_13`) |
| `editor/paint_select.c` | Selection/block operations (`edit_mode_10`, cut, paste, mask, effects) |
| `editor/paint_internal.h` | Shared declarations across the three files above |
| `ide/mouse.c` | `read_mouse()` — input dispatch: SDL events, keyboard navigation (OPQA/arrows), coordinate transforms |
| `ide/handler_map.c` | Entry point — opens the editor, runs the tool dispatch loop |

## Entry and tool dispatch

`handler_map.c` contains the outer loop.  It subtracts 100 from
`draw_mode` and dispatches to the matching `edit_mode_N()` function.
Each tool function runs its own inner loop until the user exits (right
click, ESC) or switches mode (F-keys, toolbar click).  Switching sets
`draw_mode` to `N + 100`; the outer loop picks up the new value next
iteration.

```
handler_map.c outer loop:
  do {
    draw_mode -= 100;
    switch (draw_mode) {
      case 0: edit_mode_0(); break;   // Pixels
      case 1: edit_mode_1(); break;   // Pencil/strokes
      case 2: edit_mode_2(); break;   // Lines
      case 3: edit_mode_3(); break;   // Continuous lines
      case 4: edit_mode_4(); break;   // Bezier curves
      case 5: edit_mode_5(); break;   // Continuous bezier
      case 6: edit_mode_6(); break;   // Rectangles
      case 7: edit_mode_7(); break;   // Circles/ellipses
      case 8: edit_mode_8(); break;   // Spray
      case 9: edit_mode_9(); break;   // Fill
      case 10: edit_mode_10(); break; // Selection/cut
      case 11: edit_mode_11(); break; // Undo history
      case 12: edit_mode_12(); break; // Control points
      case 13: edit_mode_13(); break; // Text
      default: edit_scr(); break;     // No tool selected
    }
  } while (draw_mode >= 100);
```

`draw_mode < 100` = inside a tool's inner loop (editing).
`draw_mode >= 100` = tool just changed; outer loop re-dispatches.

## Typical tool inner loop

Every tool follows the same pattern:

```
do {
    draw_help();                    // F1 help toggle
    edit_ruler();                   // calls read_mouse(), select_zoom(), test_mouse()
    select_fx() / select_color();   // toolbar interactions
    ... tool-specific drawing ...   // only when mouse_graf >= 10 (cursor on canvas)
    blit_edit();                    // move_zoom, render zoom, toolbar, cursor, select_mode
} while (!exit && !right_click && !ESC && draw_mode < 100 && !toolbar_close);
```

`edit_ruler()` is the call that drives input each frame — it calls
`read_mouse()` internally.

## Input flow (`read_mouse()` in `ide/mouse.c`)

`read_mouse()` handles both real mouse and keyboard-simulated cursor:

1. `read_mouse2()` — polls SDL events, updates `m_x`/`m_y`/`m_b`
2. **Real mouse branch** — fires when `m_x`/`m_y` differ from last
   known position (`last_set_mx`/`last_set_my`), or button changed.
   Copies SDL state into `mouse_x`/`mouse_y`/`mouse_b`.
3. **Keyboard branch** (else-if, paint mode only) — restores position
   from `mouse_shift_x`/`mouse_shift_y`, applies arrow/OPQA/shift
   offsets, SPACE simulates left-click (`mouse_b = 0x8001`).
4. **Clamp** — `mouse_x`/`mouse_y` clamped to screen bounds.
   `real_mouse_x`/`real_mouse_y` keep pre-clamp values for edge-scroll.
5. **Coordinate transform** — `coord_x`/`coord_y` computed from
   `mouse_shift_x`/`mouse_shift_y` relative to the zoom viewport.

Key subtlety: `test_mouse()`/`select_zoom()` (called between frames)
can modify `mouse_x`/`mouse_y` for big-mode toolbar adjustment.
The "real mouse moved?" comparison uses `last_set_mx`/`last_set_my`
(tracking `m_x`/`m_y` as last programmed by `set_mouse()`) rather
than `mouse_x` to avoid false positives from these external modifications.

## Zoom viewport

The bitmap is displayed through a zoom viewport:

- `zoom` (0..3) — magnification level (1x, 2x, 4x, 8x)
- `zoom_x`, `zoom_y` — top-left bitmap pixel visible in viewport
- `zoom_win_x`, `zoom_win_y` — screen position of viewport
- `zoom_win_width`, `zoom_win_height` — viewport size in screen pixels

When the bitmap is smaller than the screen, the viewport is centered.
When larger, it fills the screen and `move_zoom()` scrolls it when
`real_mouse_x`/`real_mouse_y` go past screen bounds.

Coordinate transform: `coord_x = zoom_x + (mouse_shift_x - zoom_win_x) / (1 << zoom)`

## Toolbar

The toolbar is a floating bar rendered into a separate `toolbar` buffer
and composited onto the screen.  It can be dragged anywhere.

- `toolbar_x`, `toolbar_y` — screen position
- `toolbar_width` — current width (varies by tool's icon count)
- `bar[]` array — icon IDs for the current tool's toolbar
- `draw_bar()` — rebuilds toolbar bitmap
- `select_mode()` / `select_icon()` — handle mode-switch clicks

Left icon area (position 10): mode selector (F2-F12 shortcuts).
Right icon area (position 48+): tool-specific icons.

## Cursor types (`mouse_graf`)

`test_mouse()` / `test_mouse_box2()` set `mouse_graf` each frame:

- `>= 10` (typically `current_mouse - zoom`): paint cursor, on canvas — drawing allowed
- `1`: arrow cursor — over toolbar or outside editing area
- `2`, `5`: window title-bar / close-button cursors

Drawing operations gate on `mouse_graf >= 10`.

## Big mode (`big`, `big2`)

When the screen resolution is low, `big = 1` and `big2 = 2` enable
double-size rendering.  Toolbar dimensions and mouse coordinates are
scaled by `big2`.  The `mouse_shift` flag prevents repeated coordinate
adjustment within a single frame.

## Key globals (in `global.h`)

| Variable | Purpose |
|----------|---------|
| `draw_mode` | Current tool (0-13) or tool+100 during transitions |
| `mouse_x`, `mouse_y` | Current cursor position (screen coords) |
| `mouse_shift_x`, `mouse_shift_y` | Saved cursor position for keyboard nav |
| `real_mouse_x`, `real_mouse_y` | Pre-clamp position (used by `move_zoom()` for scrolling) |
| `coord_x`, `coord_y` | Cursor position in bitmap pixel coords |
| `mouse_graf` | Current cursor graphic (determines if drawing is allowed) |
| `mouse_b` | Button state (bit 0=left, 1=right, 2=wheel-down, 3=wheel-up) |
| `hotkey` | 1=keyboard shortcuts active (0 during text input toolbar) |
| `color` | Current drawing color (palette index 0-255) |
| `map`, `map_width`, `map_height` | Current bitmap data and dimensions |
