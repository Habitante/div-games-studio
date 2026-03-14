// ---------------------------------------------------------------------------
// DIV Games Studio — Keyboard scan code constants
//
// Values match the OSDEP_key mapping (osd_sdl2.c) and the DIV user manual
// predefined constants (Appendix C.6).
//
// Use with the key() macro:  if (key(_ESC)) ...
// Or compare with scan_code: if (scan_code == _F1) ...
// ---------------------------------------------------------------------------

#ifndef DIV_KEYBOARD_H
#define DIV_KEYBOARD_H

// --- System keys ---
#define _ESC       1
#define _BACKSPACE 14
#define _TAB       15
#define _ENTER     28
#define _SPC       57
#define _CAPS_LOCK 58
#define _NUM_LOCK  69
#define _PRNT_SCRN 55
#define _SCROLL_LOCK 70

// Modifier keys (for key() checks; use shift_status & MOD_* for state)
#define _L_CTRL    29
#define _R_CTRL    90   // DOS synthetic; SDL2 maps both Ctrl keys to 29
#define _L_SHIFT   42
#define _R_SHIFT   54
#define _L_ALT     56
#define _R_ALT     92   // DOS synthetic; SDL2 maps both Alt keys to 56

// --- Function keys ---
#define _F1  59
#define _F2  60
#define _F3  61
#define _F4  62
#define _F5  63
#define _F6  64
#define _F7  65
#define _F8  66
#define _F9  67
#define _F10 68
#define _F11 87
#define _F12 88

// --- Navigation keys ---
#define _HOME   71
#define _UP     72
#define _PGUP   73
#define _LEFT   75
#define _CENTER 76   // Numpad 5
#define _RIGHT  77
#define _END    79
#define _DOWN   80
#define _PGDN   81
#define _INS    82
#define _DEL    83

// --- Numpad operator keys ---
#define _C_BACKSLASH 53  // Numpad /
#define _C_ASTERISK  55  // Numpad * (same scan code as Print Screen)
#define _C_MINUS     74  // Numpad -
#define _C_PLUS      78  // Numpad +

// --- Ctrl+navigation extended scan codes (DOS-era) ---
// SDL2 event handlers synthesize these from base key + MOD_CTRL via
// ctrl_nav_code() so that key(_C_UP) etc. work as in the original DOS port.
#define _C_ENTER 89
#define _C_HOME  93
#define _C_UP    94
#define _C_PGUP  95
#define _C_LEFT  96
#define _C_RIGHT 97
#define _C_END   98
#define _C_DOWN  99
#define _C_PGDN  100
#define _C_INS   101
#define _C_DEL   102

// Map a base navigation scan code to its Ctrl+nav extended code, or 0.
// Used by SDL2 event handlers to synthesize DOS-era extended scan codes.
static inline int ctrl_nav_code(int sc) {
  switch (sc) {
  case _ENTER: return _C_ENTER;
  case _HOME:  return _C_HOME;
  case _UP:    return _C_UP;
  case _PGUP:  return _C_PGUP;
  case _LEFT:  return _C_LEFT;
  case _RIGHT: return _C_RIGHT;
  case _END:   return _C_END;
  case _DOWN:  return _C_DOWN;
  case _PGDN:  return _C_PGDN;
  case _INS:   return _C_INS;
  case _DEL:   return _C_DEL;
  default:     return 0;
  }
}

// DOS BIOS extended Ctrl+key scan codes (INT 16h AH values).
// Dead in SDL2 — the base key fires with MOD_CTRL set instead.
// Named here so editor.c switch cases are readable.
#define _CTRL_LEFT  115
#define _CTRL_RIGHT 116
#define _CTRL_PGDN  118
#define _CTRL_PGUP  132

// --- Number row ---
#define _1 2
#define _2 3
#define _3 4
#define _4 5
#define _5 6
#define _6 7
#define _7 8
#define _8 9
#define _9 10
#define _0 11

// --- Letter keys ---
#define _Q 16
#define _W 17
#define _E 18
#define _R 19
#define _T 20
#define _Y 21
#define _U 22
#define _I 23
#define _O 24
#define _P 25
#define _A 30
#define _S 31
#define _D 32
#define _F 33
#define _G 34
#define _H 35
#define _J 36
#define _K 37
#define _L 38
#define _Z 44
#define _X 45
#define _C 46
#define _V 47
#define _B 48
#define _N 49
#define _M 50

// --- Punctuation ---
#define _WAVE       41  // ` ~ (backquote/tilde)
#define _MINUS      12  // - _ key
#define _PLUS       13  // = + key
#define _L_BRACKET  26  // [ { key
#define _R_BRACKET  27  // ] } key
#define _SEMICOLON  39  // ; : key
#define _APOSTROPHE 40  // ' " key
#define _BACKSLASH  43  // \ | key
#define _COMMA      51  // , < key
#define _PERIOD     51  // . > key (same scan code on ES keyboard)
#define _SLASH      51  // / ? key (same scan code on ES keyboard)

#endif // DIV_KEYBOARD_H
