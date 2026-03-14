//-----------------------------------------------------------------------------
//      DIV Games Studio — Shared enum definitions
//
//      Included by both global.h (IDE/compiler) and
//      debugger_internal.h (debugger). Keep this header
//      free of dependencies beyond standard C.
//-----------------------------------------------------------------------------

#ifndef DIV_ENUMS_H
#define DIV_ENUMS_H

// Window types (twindow.type)
enum window_type {
  WIN_EMPTY    = 0,
  WIN_DIALOG   = 1,
  WIN_MENU     = 2,
  WIN_PALETTE  = 3,
  WIN_CLOCK    = 4,
  WIN_TRASH    = 5,
  WIN_PROGRESS = 7,
  WIN_MIXER    = 8,
  WIN_MAP      = 100,
  WIN_FPG      = 101,
  WIN_CODE     = 102,
  WIN_FONT     = 104,
  WIN_SOUND    = 105,
  WIN_MUSIC    = 107,
  WIN_EDITOR_MIN = 100,
};

// Window foreground state (twindow.foreground)
enum window_foreground {
  WF_BACKGROUND = 0,
  WF_FOREGROUND = 1,
  WF_MINIMIZED  = 2,
};

// File type indices for file_types[] and v_type
enum file_type_index {
  FT_MAP       = 2,
  FT_PAL       = 3,
  FT_FPG       = 4,
  FT_FNT       = 5,
  FT_IFS       = 6,
  FT_AUDIO     = 7,
  FT_PRG       = 8,
  FT_WALLPAPER = 9,
  FT_PAL_SAVE  = 10,
  FT_AUDIO_SAVE = 11,
  FT_MAP_SAVE  = 14,
  FT_MODULE    = 16,
};

// Dialog item types (t_item.type)
enum item_type {
  ITEM_BUTTON   = 1,
  ITEM_TEXT     = 2,
  ITEM_CHECKBOX = 3,
};

// Menu text base indices (texts[] offsets from lenguaje.div)
enum menu_base {
  MENU_MAIN     = 750,
  MENU_PALETTES = 775,
  MENU_MAPS     = 800,
  MENU_GRAPHICS = 825,
  MENU_FONTS    = 850,
  MENU_SYSTEM   = 875,
  MENU_PROGRAMS = 900,
  MENU_SOUNDS   = 925,
  MENU_EDIT     = 950,
};

// Button/text alignment modes (_button center parameter)
enum button_align {
  ALIGN_TL = 0, // Top-left
  ALIGN_TC = 1, // Top-center
  ALIGN_TR = 2, // Top-right
  ALIGN_ML = 3, // Middle-left
  ALIGN_MC = 4, // Middle-center
  ALIGN_MR = 5, // Middle-right
  ALIGN_BL = 6, // Bottom-left
  ALIGN_BC = 7, // Bottom-center
  ALIGN_BR = 8, // Bottom-right
};

// Menu layout metrics
#define MENU_ITEM_HEIGHT   9
#define MENU_HEADER_HEIGHT 11
#define MENU_TEXT_MARGIN   7
#define MENU_WIDTH_PAD     23

// Mouse button bits (mouse_b)
// Undefine Windows SDK clashes first
#ifdef MB_RIGHT
#undef MB_RIGHT
#endif
#define MB_LEFT           0x01
#define MB_RIGHT          0x02
#define MB_SCROLL_DOWN    0x04
#define MB_SCROLL_UP      0x08
#define MB_KEYBOARD_CLICK 0x8001

// Keyboard modifier bits (shift_status)
#ifdef MOD_SHIFT
#undef MOD_SHIFT
#endif
#ifdef MOD_ALT
#undef MOD_ALT
#endif
#define MOD_RSHIFT  0x01
#define MOD_LSHIFT  0x02
#define MOD_SHIFT   0x03  // Mask: either shift key
#define MOD_CTRL    0x04
#define MOD_ALT     0x08
#define MOD_NUM     0x20  // Num Lock
#define MOD_CAPS    0x40  // Caps Lock
#define MOD_SCROLL  0x80  // Scroll Lock (runtime only)

// ASCII character constants (for ascii variable comparisons)
#define ASCII_BACKSPACE 8
#define ASCII_TAB       9
#define ASCII_ENTER     13
#define ASCII_ESC       0x1b

// Mouse cursor IDs (mouse_graf)
// Window-chrome cursors (1-7)
#define CURSOR_ARROW      1
#define CURSOR_MOVE       2
#define CURSOR_BUSY       3
#define CURSOR_MINIMIZE   4
#define CURSOR_CLOSE      5
#define CURSOR_RESIZE     6
#define CURSOR_ACTIVATE   7
// Scrollbar/widget cursors (7-16) — used by code editor, help viewer,
// browser, debugger, and listbox widgets. Some overlap window-chrome
// values (same cursor graphic, different context).
#define CURSOR_SCROLL_UP     7   // Vertical scrollbar up arrow (= CURSOR_ACTIVATE)
#define CURSOR_SCROLL_DOWN   9   // Vertical scrollbar down arrow
#define CURSOR_SCROLL_LEFT  10   // Horizontal scrollbar left arrow
#define CURSOR_SCROLL_RIGHT 11   // Horizontal scrollbar right arrow
#define CURSOR_GRIP         12   // Corner resize grip
#define CURSOR_SCROLL_TRACK 13   // Scrollbar track (drag to position)
#define CURSOR_HSCROLL_TRACK 14  // Horizontal scrollbar track
#define CURSOR_MAXIMIZE     15   // Maximize button
#define CURSOR_MENU_HOVER   16   // Menu item highlight
// Paint canvas threshold
#define CURSOR_ON_CANVAS  10     // Values >= this mean cursor is on canvas

// Paint editor draw modes (draw_mode)
enum draw_mode_id {
  TOOL_PIXELS     = 0,   // Point-and-click
  TOOL_PENCIL     = 1,   // Freehand strokes
  TOOL_LINES      = 2,   // Lines
  TOOL_POLYLINE   = 3,   // Continuous lines
  TOOL_BEZIER     = 4,   // Bezier curves
  TOOL_POLYBEZIER = 5,   // Continuous bezier
  TOOL_RECT       = 6,   // Rectangles
  TOOL_CIRCLE     = 7,   // Circles/ellipses
  TOOL_SPRAY      = 8,   // Spray
  TOOL_FILL       = 9,   // Flood fill
  TOOL_SELECT     = 10,  // Selection/cut
  TOOL_UNDO       = 11,  // Undo history
  TOOL_CTRLPOINTS = 12,  // Control points
  TOOL_TEXT       = 13,  // Text
  TOOL_PASTE      = 90,  // Paste graphic (map editor)
  TOOL_TRANSITION = 100, // Tool transition offset (+100 = in menu overlay)
};

// Code editor block selection state (block_state)
enum block_state_id {
  BLOCK_NONE     = 0, // No block
  BLOCK_PIVOT    = 1, // Pivot set (block_start, block_col1)
  BLOCK_COMPLETE = 2, // Complete (block_start/end, block_col1/2)
};

// Code editor block mode (edit_block_mode)
enum block_mode_id {
  BMODE_NONE = 0, // No selection
  BMODE_CHAR = 1, // Character block
  BMODE_LINE = 2, // Line block
};

// Clipboard content type (clipboard_type)
enum clipboard_type_id {
  CLIP_CHAR = 0, // Character data
  CLIP_LINE = 1, // Line data
};

// Drag-and-drop state (dragging)
enum drag_state {
  DRAG_IDLE     = 0, // Idle
  DRAG_PENDING  = 1, // Pending (mouse down)
  DRAG_ACTIVE   = 2, // Dragging (in flight)
  DRAG_DONE     = 3, // Done (released)
  DRAG_DROPPING = 4, // Dropping (over target)
  DRAG_DROPPED  = 5, // Dropped (completed)
};

// Palette and color table sizes
#define PALETTE_SIZE     768   // 256 colors × 3 bytes (RGB)
#define GRADIENTS_SIZE   576   // 16 gradients × 36 bytes each
#define GHOST_TABLE_SIZE 65536 // 256 × 256 transparency lookup
#define CUAD_TABLE_SIZE  16384 // 128 × 128 squared-difference lookup

// FPG limits
#define MAX_FPG_GRAPHICS 1000 // Maximum graphics per FPG file

// File format header offsets (MAP/FPG/FNT)
#define FNT_GENCODE_OFFSET 1352 // 8 (magic) + 768 (palette) + 576 (gradients)
#define FNT_TABLE_OFFSET   1356 // FNT_GENCODE_OFFSET + 4 (gencode field)
#define MAP_CTRLPTS_OFFSET 1392 // Control points offset in MAP files

// Process status codes (_Status)
enum proc_status {
  PROC_DEAD     = 0, // Free slot
  PROC_KILLED   = 1, // Pending removal
  PROC_ALIVE    = 2, // Running
  PROC_SLEEPING = 3, // Waiting for function return
  PROC_FROZEN   = 4, // Paused (still painted)
};

// Coordinate types (_Ctype)
enum coord_type {
  CTYPE_SCREEN = 0, // Screen coordinates
  CTYPE_SCROLL = 1, // Scroll-relative
  CTYPE_MODE7  = 2, // Mode 7-relative
};

// Sprite flags (_Flags) — bit field
#define SPRITE_HFLIP 1 // Bit 0: horizontal flip
#define SPRITE_VFLIP 2 // Bit 1: vertical flip
#define SPRITE_GHOST 4 // Bit 2: ghost/transparency blend

#endif // DIV_ENUMS_H
