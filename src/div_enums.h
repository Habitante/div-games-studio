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

#endif // DIV_ENUMS_H
