
//-----------------------------------------------------------------------------
//      Module that contains the code editor
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#include <time.h>

void _completo(void);
void error_cursor(void);
void f_unmark(void);
void f_mark(void);
void f_down(void);
void f_up(void);
void f_right(void);
void f_left(void);
void maximize(void);
void resize(void);
int get_slide_x(void);
void editor(void);
void _parcial(void);
void error_cursor2(void);
void delete_text_cursor(void);
void f_cut_block(int mode);
void f_paste_block(void);
int linelen(byte *p);
void f_end(void);
void f_word_right2(void);
void f_word_left(void);
void f_page_down(void);
void f_scroll(int n);
void f_mscroll(int n);
void f_page_up(void);
void f_untab(void);
void f_insert(void);
void f_delete_char(void);
void f_backspace(void);
void f_tab(void);
void f_delete(void);
void f_word_right(void);
void f_bof(void);
void f_eof(void);
void f_eop(void);
void f_enter(void);
void f_overwrite(void);
void f_insert_char(void);
void remove_spaces(void);
void check_memory(int block_len);
void text_cursor(void);
int in_block(void);
void f_cut(int mode);
void test_cursor2(void);
void delete_line(void);
void info_bar(void);
void scrollbars(void);
void put_char3(byte *ptr, int w, byte c, int block, byte color);
void test_cursor(void);
void put_char2(byte *ptr, int w, byte c, byte color);
void paint_process_segment(void);


//-----------------------------------------------------------------------------
//      Constants
//-----------------------------------------------------------------------------

#define buffer_grow 16384 // Edit buffer growth increment
#define buffer_min  2048  // Minimum margin space

#define cr  13 // Carriage return
#define lf  10 // newline / linefeed
#define tab 9  // tabulation

extern int ibuf; // A pointer to the queue buffer
extern int fbuf; // Pointer to queue buffer end

//-----------------------------------------------------------------------------

extern int error_number; // Error number
extern int error_line;   // Line error (note, can be num_lines + 1)
extern int error_col;    // Error column num

//-----------------------------------------------------------------------------
//    TPRG Struct
//-----------------------------------------------------------------------------

/*

  int an,al;                    // Character width and height of the window

  int old_x,old_y;              // Coordinates before maximized
  int old_w,old_h;            // Width and height before maximized

  char path[_MAX_PATH+1];       // Path of associated file
  char filename[12+1];          // Associated file's name

  char * buffer;                // Buffer with loaded file
  int buffer_len;               // Buffer length
  int file_len;                 // File length ( < buffer_len)

  int num_lines;               // Number of source lines
  int line;                     // Current line in editor
  int column;                   // Current cursor column
  char * lptr;                  // Pointer to current line in the file
  char * vptr;                  // Pointer to first displayed line
  int first_line;            // First line displayed on screen
  int first_column;          // Horizontal scroll offset on screen

  char l[long_line+4];          // Buffer for the edited line
  int line_size;                // Original size of the edited line

  int prev_line;              // Previous line (for partial blit_screen)

  */

//-----------------------------------------------------------------------------
//      Editor variables
//-----------------------------------------------------------------------------

// Window size is (an*editor_font_width+12*big2) x (al*editor_font_height+20*big2)

char line_buffer[long_line + 4]; // Copy of the line being edited, for f_enter()

int cursor_mode = 0; // Active cursor type (0-insert/1-overwrite)

long block_state =
    0; //0-n/a 1-pivot(block_start,block_col1) 2-complete(block_start,block_col1,block_end,block_col2)

byte *block_start, *block_end; // Pointers to the start of the first and last lines
int block_col1, block_col2;    // Start and end columns in those lines

struct tprg *kprg; // Prg that owns the block

char *clipboard = NULL; // Pointer to clipboard (initially empty)
int clipboard_len;      // Clipboard content length in bytes
int clipboard_lines;    // Number of line breaks contained
int clipboard_type = 0; // 0-character block, 1-line block

int edit_block_mode = 0; // 1-Volatile block on current line, 2-multi-line

int skip_full_blit = 0; // For skipping full screen blits

byte *advance(byte *q);
byte *retreat(byte *q);

char color_cursor;

int forced_slider = 0;

//-----------------------------------------------------------------------------
//      Lexical colorizer variables
//-----------------------------------------------------------------------------

#define p_ultima 0x00  // End of file <EOF>
#define p_rem    0x7f  // Single-line comment
#define p_id     0xfd  // Identifier
#define p_num    0xfe  // Number
#define p_spc    0x100 // Whitespace
#define p_sym    0x101 // Symbol
#define p_lit    0x102 // Quoted literal
#define p_res    0x103 // Reserved keyword
#define p_pre    0x104 // Predefined identifier

void color_lex(void);

extern int color_token;    // Token read by color_lex()
extern byte *color_source; // Pointer to source, for compiling the program
extern int in_comment;     // Whether inside a comment

char colin[1024]; // Buffer for line syntax coloring

int _cached_in_comment;
char *_cached_color_source = NULL;

char *c_oldline;
int old_comment_state;
extern int numrem;

//-----------------------------------------------------------------------------
//      Program window handlers
//-----------------------------------------------------------------------------

void program1(void) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  _completo();

  if (error_number != -1 && eprg == v.prg)
    error_cursor();

  wput(ptr, w, h, w - 17, 10, 56);    // Maximize
  wput(ptr, w, h, w - 9, 10, 39);     // Up
  wput(ptr, w, h, w - 9, h - 17, 40); // Down
  wput(ptr, w, h, w - 9, h - 9, 34);  // Resize
  wput(ptr, w, h, 2, h - 9, 51);      // Left
  wput(ptr, w, h, w - 17, h - 9, 52); // Right
}

void program2(void) {
  static byte mouse_block = 0, mouse_block_x, mouse_block_y;
  int n, m, min, max;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  if (dragging == 4)
    v.redraw = 1;

  if (wmouse_x != -1 && v.state) {
    if ((mouse_b & 1) && mouse_y >= v.y + 18 * big2 && mouse_x >= v.x + 2 * big2) {
      n = v.prg->first_line + (mouse_y - (v.y + 18 * big2)) / editor_font_height;
      m = v.prg->first_column + (mouse_x - (v.x + 2 * big2)) / editor_font_width;
      if (n >= v.prg->first_line && n < v.prg->first_line + v.prg->h && m >= v.prg->first_column &&
          m < v.prg->first_column + v.prg->w) {
        if (!(prev_mouse_buttons & 1)) {
          mouse_block = 1;
          mouse_block_x = m;
          mouse_block_y = n;
        } else {
          if (mouse_block == 1 && (m != mouse_block_x || n != mouse_block_y)) {
            f_unmark();
            mouse_block = 2;
            f_mark();
          }
        }
        if (n > v.prg->num_lines)
          n = v.prg->num_lines;
        while (v.prg->line < n)
          f_down();
        while (v.prg->line > n)
          f_up();
        while (v.prg->column < m)
          f_right();
        while (v.prg->column > m)
          f_left();
        v.redraw++;
      }
    }
    if (v.redraw == 0) {
      if (mouse_block == 2) {
        f_mark();
        v.redraw++;
      }
      mouse_block = 0;
    }
    if (mouse_b & 2) {
      f_unmark();
      v.redraw++;
    }
  }

  if (wmouse_in(w - 9, 10, 9, h - 20)) { // Vertical slider
    if (wmouse_y < 18)
      mouse_graf = 7;
    else if (wmouse_y >= h - 17)
      mouse_graf = 9;
    else
      mouse_graf = 13;
  } else if (wmouse_in(2, h - 9, w - 11, 9)) { // Horizontal slider
    if (wmouse_x < 10)
      mouse_graf = 10;
    else if (wmouse_x >= w - 17)
      mouse_graf = 11;
    else
      mouse_graf = 14;
  } else if (wmouse_in(w - 9, h - 9, 9, 9)) { // Resize
    mouse_graf = 12;
  } else if (wmouse_in(w - 17, 10, 9, 9)) { // Maximize
    mouse_graf = 15;
  }

  if (mouse_graf == 15 && (mouse_b & 1) && wmouse_x != -1) {
    if (!(v.buttons & 1)) {
      wput(v.ptr, w, h, w - 17, 10, -57);
      v.buttons |= 1;
      v.redraw++;
    }
  } else if (v.buttons & 1) {
    v.buttons ^= 1;
    if (mouse_graf == 15 && wmouse_in(w - 17, 10, 9, 9)) {
      maximize();
    } else {
      wput(v.ptr, w, h, w - 17, 10, 56);
      v.redraw++;
    }
  }

  if (mouse_graf == 7 && (mouse_b & 1) && wmouse_x != -1) {
    if (!(v.buttons & 2)) {
      wput(v.ptr, w, h, w - 9, 10, -41);
      v.buttons |= 2;
    } else {
      retrace_wait();
      retrace_wait();
    }
    v.redraw++;
    write_line();
    retreat_vptr();
    retreat_lptr();
    read_line();
  } else if (v.buttons & 2) {
    wput(v.ptr, w, h, w - 9, 10, -39);
    v.buttons ^= 2;
    v.redraw++;
  }

  if (mouse_graf == 9 && (mouse_b & 1) && wmouse_x != -1) {
    if (!(v.buttons & 4)) {
      wput(v.ptr, w, h, w - 9, h - 17, -42);
      v.buttons |= 4;
    } else {
      retrace_wait();
      retrace_wait();
    }
    v.redraw++;
    write_line();
    advance_vptr();
    advance_lptr();
    read_line();
  } else if (v.buttons & 4) {
    wput(v.ptr, w, h, w - 9, h - 17, -40);
    v.buttons ^= 4;
    v.redraw++;
  }

  if (mouse_graf == 10 && (mouse_b & 1) && wmouse_x != -1) {
    if (!(v.buttons & 8)) {
      wput(v.ptr, w, h, 2, h - 9, -53);
      v.buttons |= 8;
    } else {
      retrace_wait();
      retrace_wait();
    }
    v.redraw++;
    f_left();
  } else if (v.buttons & 8) {
    wput(v.ptr, w, h, 2, h - 9, -51);
    v.buttons ^= 8;
    v.redraw++;
  }

  if (mouse_graf == 11 && (mouse_b & 1) && wmouse_x != -1) {
    if (!(v.buttons & 16)) {
      wput(v.ptr, w, h, w - 17, h - 9, -54);
      v.buttons |= 16;
    } else {
      retrace_wait();
      retrace_wait();
    }
    v.redraw++;
    f_right();
  } else if (v.buttons & 16) {
    wput(v.ptr, w, h, w - 17, h - 9, -52);
    v.buttons ^= 16;
    v.redraw++;
  }

  if (mouse_graf == 12 && (mouse_b & 1) && wmouse_x != -1)
    resize();

  if (mouse_graf == 13 && (mouse_b & 1) && wmouse_x != -1) {
    f_bop();
    f_home();
    min = 18;
    max = h - 21;
    forced_slider = wmouse_y - 1;
    if (forced_slider < min)
      forced_slider = min;
    else if (forced_slider > max)
      forced_slider = max;
    n = 1 + ((v.prg->num_lines - 1) * (forced_slider - min)) / (max - min);
    if (n < 1)
      n = 1;
    else if (n > v.prg->num_lines)
      n = v.prg->num_lines;
    while (v.prg->line > n) {
      write_line();
      retreat_lptr();
      read_line();
      retreat_vptr();
    }
    while (v.prg->line < n) {
      write_line();
      advance_lptr();
      read_line();
      advance_vptr();
    }
    v.buttons |= 128;
    v.redraw = 2;
  } else {
    if (v.buttons & 128) {
      v.buttons ^= 128;
      v.redraw = 2;
    }
  }

  if (mouse_graf == 14 && !(prev_mouse_buttons & 1) && (mouse_b & 1) && wmouse_x != -1) {
    if (!(v.buttons & 64)) {
      if (wmouse_x > get_slide_x()) {
        f_right();
        f_right();
        f_right();
        f_right();
      } else {
        f_left();
        f_left();
        f_left();
        f_left();
      }
      v.buttons ^= 64;
    }
  } else if (v.buttons & 64)
    v.buttons ^= 64;

  if (v.state)
    editor();
}
extern byte m_b;

void program3(void) {
  if (block_state && kprg == v.prg)
    block_state = 0;

  free(v.prg->buffer);
  v.prg->buffer = NULL;
  free(v.prg);
  v.prg = NULL;
}

void program0(void) {
  v.type = 102;

  v_prg->w = (vga_width / 2 - 1 - 12 * big2) / editor_font_width;
  v_prg->h = (vga_height / 2 - 32 * big2 - 1) / editor_font_height;

  v_prg->line = 1;
  v_prg->column = 1;

  v_prg->first_line = 1;
  v_prg->first_column = 1;

  v_prg->lptr = v_prg->buffer;
  v_prg->vptr = v_prg->buffer;

  v.w = (4 + 8) * big2 + editor_font_width * v_prg->w;
  v.h = (12 + 16) * big2 + editor_font_height * v_prg->h;

  if (big) {
    if (v.w & 1)
      v.w++;
    if (v.h & 1)
      v.h++;
    v.w = -v.w; // Negative signals the window should not be scaled by 2
  }

  v.prg = v_prg;
  v.title = (byte *)v_prg->filename;
  v.name = (byte *)v_prg->filename;

  v.paint_handler = program1;
  v.click_handler = program2;
  v.close_handler = program3;
}

//-----------------------------------------------------------------------------
//      Editor loop
//-----------------------------------------------------------------------------

struct tprg *edited;

/* Code editor main loop (click handler for program editor windows).
 * Processes keyboard input for text editing, block selection, search/replace,
 * cursor movement, and scrollbar interaction. Called once per frame.
 */
void editor() {
  byte *p;
  int n;

  if (error_number != -1 && eprg == v.prg) {
    if ((scan_code <= 1 || scan_code == 59) && ascii == 0 && mouse_b == 0) {
      _parcial();
      error_cursor2();
      v.redraw++;
      return;
    }
    error_number = -1;
  }

  v.prg->prev_line = v.prg->line - v.prg->first_line;

  if (edited != v.prg) { // If we were editing a different one ...
    for (n = 0; n < max_windows; n++)
      if (window[n].type == 102 && window[n].prg != NULL && window[n].prg == edited)
        break;
    if (n < max_windows) {
      wup(n);
      write_line();
      read_line();
      if (block_state == 1)
        f_mark();
      wdown(n);
    }
  }
  edited = v.prg;

  // poll_keyboard();

  delete_text_cursor();

  //-----------------------------------------------------------------------------

  if (edit_block_mode == 1) { // Single-line block edit

    if ((shift_status & 3) && !(shift_status & 12))
      switch (scan_code) {
      case 0:
        break;
      case 77:
        if (block_col2 + 1 == v.prg->column)
          block_col2++;
        else if (block_col1 == v.prg->column)
          block_col1++;
        if (block_col1 > block_col2)
          f_unmark();
        f_right();
        break;
      case 75:
        if (v.prg->column > 1) {
          if (block_col2 == v.prg->column - 1)
            block_col2--;
          else if (block_col1 == v.prg->column)
            block_col1--;
          if (block_col1 > block_col2)
            f_unmark();
          f_left();
        }
        break;
      case 82:
        f_cut_block(2); // shift+insert
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      case 83:
        f_cut_block(1); // shift+delete
        edit_block_mode = 0;
        scan_code = 0;
        break;
      case 71: // shift+home
        if (block_col1 == v.prg->column)
          block_col1 = 1;
        else {
          block_col2 = block_col1 - 1;
          block_col1 = 1;
        }
        if (block_col1 > block_col2)
          f_unmark();
        f_home();
        break;
      case 79: // shift+end
        if (block_col2 + 1 == v.prg->column)
          block_col2 = linelen(v.prg->lptr);
        else {
          block_col1 = block_col2 + 1;
          block_col2 = linelen(v.prg->lptr);
        }
        if (block_col1 > block_col2)
          f_unmark();
        f_end();
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & 4) && !(shift_status & 11)) {
      switch (scan_code) {
      case 0:
        break;
      case 45:
        f_cut_block(1); // ctrl+x
        edit_block_mode = 0;
        scan_code = 0;
        break;
      case 46:
        f_cut_block(0); // ctrl+c
        scan_code = 0;
        break;
      case 47:
        f_cut_block(2); // ctrl+v
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      default:
        f_unmark();
        break;
      }

      if (kbdFLAGS[82]) { // ctrl+ins
        kbdFLAGS[82] = 0;
        f_cut_block(0);
        scan_code = 0;
      }

    } else if (!(shift_status & 15) && ascii == 0)
      switch (scan_code) {
      case 0:
        break;
      case 83:
        f_cut_block(2); // delete
        scan_code = 0;
        edit_block_mode = 0;
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & 3) && (shift_status & 4))
      switch (scan_code) {
      case 116: // ctrl+shift+right
        if (block_col2 == v.prg->column - 1) {
          f_word_right2();
          block_end = v.prg->lptr;
          block_col2 = v.prg->column - 1;
        } else {
          f_word_right2();
          block_start = v.prg->lptr;
          block_col1 = v.prg->column;
          if (block_end == block_start) {
            if (block_col1 > block_col2) {
              block_col1 = block_col2 + 1;
              block_col2 = v.prg->column - 1;
            }
          }
        }
        if (block_end != block_start) {
          edit_block_mode = 2;
          block_col1 = 1;
          n = v.prg->column;
          v.prg->column = 1;
          f_up();
          block_end = v.prg->lptr;
          block_col2 = linelen(v.prg->lptr) + 1;
          f_down();
          v.prg->column = n;
        } else if (block_col1 > block_col2)
          f_unmark();
        scan_code = 0;
        v.redraw++;
        break;
      case 115: // ctrl+shift+left
        f_up();
        p = v.prg->lptr;
        f_down();
        if (block_col1 == v.prg->column) {
          f_word_left();
          block_start = v.prg->lptr;
          block_col1 = v.prg->column;
        } else {
          f_word_left();
          block_end = v.prg->lptr;
          block_col2 = v.prg->column - 1;
          if (block_start == block_end) {
            if (block_col1 > block_col2) {
              block_col2 = block_col1 - 1;
              block_col1 = v.prg->column;
            }
          }
        }
        if (block_end != block_start) {
          edit_block_mode = 2;
          block_col1 = 1;
          block_start = v.prg->lptr;
          block_end = p;
          block_col2 = linelen(p) + 1;
        } else if (block_col1 > block_col2)
          f_unmark();
        scan_code = 0;
        v.redraw++;
        break;
      }
    else if (scan_code)
      f_unmark();
  } // edit_block_mode==1

  //-----------------------------------------------------------------------------

  if (edit_block_mode == 2) { // Multi-line block edit

    if ((shift_status & 3) && !(shift_status & 12))
      switch (scan_code) {
      case 0:
        break;
      case 80: // shift+down
        if (block_end < v.prg->lptr) {
          block_end = v.prg->lptr;
          block_col2 = linelen(v.prg->lptr) + 1;
          f_down();
        } else if (block_start == v.prg->lptr) {
          f_down();
          block_start = v.prg->lptr;
          block_col1 = 1;
        }
        if (block_start > block_end)
          f_unmark();
        v.redraw++;
        scan_code = 0;
        break;
      case 81: // shift+page down
        if (block_end < v.prg->lptr) {
          f_page_down();
          f_up();
          block_end = v.prg->lptr;
          f_down();
          block_col2 = linelen(block_end) + 1;
        } else if (block_start == v.prg->lptr) {
          f_page_down();
          block_start = v.prg->lptr;
          block_col1 = 1;
          if (block_start > block_end) {
            block_start = advance(block_end);
            f_up();
            block_end = v.prg->lptr;
            f_down();
            block_col2 = linelen(block_end) + 1;
          }
        }
        if (block_start > block_end)
          f_unmark();
        v.redraw++;
        scan_code = 0;
        break;
      case 73: // shift+page up
        if (block_start == v.prg->lptr) {
          f_page_up();
          block_start = v.prg->lptr;
          block_col1 = 1;
        } else {
          f_page_up();
          f_up();
          block_end = v.prg->lptr;
          f_down();
          block_col2 = linelen(block_end) + 1;
          if (block_start > block_end) {
            block_end = retreat(block_start);
            block_col2 = linelen(block_end) + 1;
            block_start = v.prg->lptr;
            block_col1 = 1;
          }
        }
        if (block_start > block_end)
          f_unmark();
        v.redraw++;
        scan_code = 0;
        break;
      case 72: // shift+up
        if (block_start == v.prg->lptr) {
          f_up();
          block_start = v.prg->lptr;
          block_col1 = 1;
        } else {
          f_up();
          if (block_end == v.prg->lptr) {
            if (v.prg->line > 1) {
              f_up();
              block_end = v.prg->lptr;
              block_col2 = linelen(v.prg->lptr) + 1;
              f_down();
            } else {
              f_unmark();
            }
          }
        }
        if (block_start > block_end)
          f_unmark();
        v.redraw++;
        scan_code = 0;
        break;
      case 82:
        f_cut_block(2); // shift+insert
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      case 83:
        f_cut_block(1); // shift+delete
        edit_block_mode = 0;
        clipboard_type = 1;
        scan_code = 0;
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & 4) && !(shift_status & 11)) {
      switch (scan_code) {
      case 0:
        break;
      case 45:
        f_cut_block(1); // ctrl+x
        edit_block_mode = 0;
        clipboard_type = 1;
        scan_code = 0;
        break;
      case 46:
        f_cut_block(0); // ctrl+c
        clipboard_type = 1;
        scan_code = 0;
        break;
      case 47:
        f_cut_block(2); // ctrl+v
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      default:
        f_unmark();
        break;
      }

      if (kbdFLAGS[82]) { // ctrl+ins
        kbdFLAGS[82] = 0;
        f_cut_block(0);
        clipboard_type = 1;
        scan_code = 0;
      }

    } else if (!(shift_status & 15) && ascii == 0)
      switch (scan_code) {
      case 0:
        break;
      case 83:
        f_cut_block(2); // delete
        scan_code = 0;
        edit_block_mode = 0;
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & 3) && (shift_status & 4))
      switch (scan_code) {
      case 116: // ctrl+shift+right
        if (block_end < v.prg->lptr) {
          f_word_right2();
          f_up();
          block_end = v.prg->lptr;
          f_down();
          block_col2 = linelen(block_end) + 1;
        } else if (block_start == v.prg->lptr) {
          f_word_right2();
          block_start = v.prg->lptr;
          block_col1 = 1;
        }
        if (block_start > block_end)
          f_unmark();
        v.redraw++;
        scan_code = 0;
        break;
        break;
      case 115: // ctrl+shift+left
        if (block_start == v.prg->lptr) {
          f_word_left();
          block_start = v.prg->lptr;
          block_col1 = 1;
        } else {
          f_word_left();
          f_up();
          block_end = v.prg->lptr;
          f_down();
          block_col2 = linelen(block_end) + 1;
        }
        if (block_start > block_end)
          f_unmark();
        v.redraw++;
        scan_code = 0;
        break;
      }
    else if (scan_code)
      f_unmark();
  } // edit_block_mode==2

  //-----------------------------------------------------------------------------

  if (edit_block_mode == 0) { // Only if there is no active edit block

    if ((shift_status & 3) && !(shift_status & 12))
      switch (scan_code) {
      case 77: // shift+right
        f_unmark();
        edit_block_mode = 1;
        block_state = 2;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = v.prg->column;
        block_col2 = v.prg->column;
        f_right();
        break;
      case 75: // shift+left
        if (v.prg->column > 1) {
          f_unmark();
          edit_block_mode = 1;
          block_state = 2;
          kprg = v.prg;
          block_start = v.prg->lptr;
          block_end = v.prg->lptr;
          f_left();
          block_col1 = v.prg->column;
          block_col2 = v.prg->column;
        }
        break;
      case 80: // shift+down
        f_unmark();
        edit_block_mode = 2;
        block_state = 2;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = 1;
        block_col2 = linelen(v.prg->lptr) + 1;
        f_down();
        if (v.prg->lptr == block_end)
          f_unmark();
        break;
      case 81: // shift+page down
        f_unmark();
        edit_block_mode = 2;
        block_state = 2;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_col1 = 1;
        f_page_down();
        f_up();
        block_end = v.prg->lptr;
        f_down();
        block_col2 = linelen(block_end) + 1;
        if (block_start > block_end)
          f_unmark();
        break;
      case 72: // shift+up
        if (v.prg->line > 1) {
          f_unmark();
          edit_block_mode = 2;
          block_state = 2;
          kprg = v.prg;
          f_up();
          block_start = v.prg->lptr;
          block_end = v.prg->lptr;
          block_col1 = 1;
          block_col2 = linelen(v.prg->lptr) + 1;
        }
        break;
      case 73: // shift+page up
        edit_block_mode = 2;
        block_state = 2;
        kprg = v.prg;
        f_up();
        block_end = v.prg->lptr;
        f_down();
        block_col2 = linelen(block_end) + 1;
        f_page_up();
        block_start = v.prg->lptr;
        block_col1 = 1;
        if (v.prg->lptr == block_end)
          f_unmark();
        break;
      case 15:
        f_untab();
        break; // shift+tab
      case 82:
        f_paste_block(); // shift+insert
        f_unmark();
        break;
      case 83:
        f_cut_block(1);
        break; // shift+delete
      case 71: // shift+home
        f_unmark();
        edit_block_mode = 1;
        block_state = 2;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = 1;
        block_col2 = v.prg->column - 1;
        if (block_col1 > block_col2)
          f_unmark();
        f_home();
        break;
      case 79: // shift+end
        f_unmark();
        edit_block_mode = 1;
        block_state = 2;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = v.prg->column;
        block_col2 = linelen(v.prg->lptr);
        if (block_col1 > block_col2)
          f_unmark();
        f_end();
        break;
      }
    else if ((shift_status & 3) && (shift_status & 4))
      switch (scan_code) {
      case 116: // ctrl+shift+right
        f_unmark();
        edit_block_mode = 1;
        block_state = 2;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_col1 = v.prg->column;
        f_word_right2();
        block_end = v.prg->lptr;
        block_col2 = v.prg->column - 1;
        if (block_end != block_start) {
          edit_block_mode = 2;
          block_col1 = 1;
          n = v.prg->column;
          v.prg->column = 1;
          f_up();
          block_end = v.prg->lptr;
          block_col2 = linelen(v.prg->lptr) + 1;
          f_down();
          v.prg->column = n;
        } else if (block_col1 > block_col2)
          f_unmark();
        break;
      case 115: // ctrl+shift+left
        f_unmark();
        edit_block_mode = 1;
        block_state = 2;
        kprg = v.prg;
        block_end = v.prg->lptr;
        block_col2 = v.prg->column - 1;
        f_up();
        p = v.prg->lptr;
        f_down();
        f_word_left();
        block_start = v.prg->lptr;
        block_col1 = v.prg->column;
        if (block_end != block_start) {
          edit_block_mode = 2;
          block_col1 = 1;
          block_start = v.prg->lptr;
          block_end = p;
          block_col2 = linelen(p) + 1;
        } else if (block_col1 > block_col2)
          f_unmark();
        break;
      }
    else if (!(shift_status & 15) && ascii == 0)
      switch (scan_code) {
      case 77:
        f_right();
        break; // cursor right
      case 75:
        f_left();
        break; // cursor left
      case 80:
        f_down();
        break; // cursor down
      case 72:
        f_up();
        break; // cursor up
      case 71:
        f_home();
        break; // home
      case 79:
        f_end();
        break; // end
      case 81:
        f_page_down();
        break; // page down
      case 73:
        f_page_up();
        break; // page up
      case 82:
        f_insert();
        break; // insert
      case 83:
        f_delete_char();
        break; // delete
      }
    else if (!(shift_status & 15))
      switch (scan_code) {
      case 14:
        f_backspace();
        break; // backspace
      case 15:
        f_tab();
        break; // tab
      }
    else if ((shift_status & 4) && !(shift_status & 11))
      switch (scan_code) {
      case 14:
      case 21:
        f_delete();
        break; // ctrl+backspace,ctrl+y
      case 116:
      case 77:
        f_word_right();
        break; // ctrl+right
      case 115:
      case 75:
        f_word_left();
        break; // ctrl+left
      case 132:
      case 73:
        f_bof();
        break; // ctrl+page up
      case 118:
      case 81:
        f_eof();
        break; // ctrl+page down
      case 119:
      case 71:
        f_bop();
        break; // ctrl+home
      case 117:
      case 79:
        f_eop();
        break; // ctrl+end
      case 45:
        f_cut_block(1);
        break; // ctrl+x
      case 46:
        f_cut_block(0);
        break; // ctrl+c
      case 47:
        f_paste_block(); // ctrl+v
        f_unmark();
        break;
      }
    else if ((shift_status & 8) && !(shift_status & 7))
      switch (scan_code) {
      case 30:
        f_mark();
        break; // alt+a
      case 22:
        f_unmark();
        break; // alt+u
      case 34: // alt+g=alt+d
      case 32:
        f_cut_block(1);
        break; // alt+d
      case 46:
        if (block_state) { // alt+c
          f_cut_block(0);
          f_paste_block();
        }
        break;
      case 50:
        if (block_state) { // alt+m
          f_cut_block(1);
          f_paste_block();
        }
        break;
      }
  } // edit_block_mode==0

  if (mouse_b & 4 && wmouse_x != -1) {
    f_scroll(3);
  }

  if (mouse_b & 8 && wmouse_x != -1) {
    f_mscroll(3);
  }

  //-----------------------------------------------------------------------------

  if (!(shift_status & (4 | 8)) && ascii) {
    if (ascii == cr)
      f_enter();
    else if (ascii != 0x1b) {
      if (cursor_mode)
        f_overwrite();
      else
        f_insert_char();
    }
  }

  remove_spaces();

  check_memory(0);

  if (v.redraw && ibuf != fbuf) {
    skip_full_blit = 1;
    v.redraw = 0;
  } else {
    if (skip_full_blit) {
      v.redraw = 1;
      skip_full_blit = 0;
    }
    if (v.redraw)
      _completo();
    else
      _parcial();
    text_cursor();
    v.redraw++;
  }
}

//-----------------------------------------------------------------------------
//      Check that buffer_len>file_len+buffer_min+block_len
//-----------------------------------------------------------------------------

//  char * buffer;                // Buffer with loaded file
//  int buffer_len;               // Buffer length
//  int file_len;                 // File length ( < buffer_len)
//  char * lptr;                  // Pointer to current line in the file
//  char * vptr;                  // Pointer to first displayed line

void check_memory(int block_len) {
  byte *p;
  int ip;

  if (v.prg->buffer_len <= v.prg->file_len + block_len + buffer_min) {
    write_line();
    p = (byte *)realloc(v.prg->buffer, v.prg->file_len + block_len + buffer_grow);
    if (p != NULL) {
      ip = (byte *)p - v.prg->buffer;
      v.prg->buffer += ip;
      v.prg->lptr += ip;
      v.prg->vptr += ip;
      if (kprg == v.prg && block_state) {
        block_start += ip;
        block_end += ip;
      }
      v.prg->buffer_len = v.prg->file_len + block_len + buffer_grow;
    }
    read_line();
  }
}

//-----------------------------------------------------------------------------
//      Calculate the length of a source line
//-----------------------------------------------------------------------------

int linelen(byte *p) {
  int n = 0;
  if (p == v.prg->lptr)
    return (strlen(v.prg->l));
  else {
    while (*p != cr && p < v.prg->buffer + v.prg->file_len) {
      p++;
      n++;
    }
    return (n);
  }
}

//-----------------------------------------------------------------------------
//      Block functions
//-----------------------------------------------------------------------------

void f_mark(void) {
  int n;
  byte *s;

  remove_spaces();

  if (block_state == 2)
    if (kprg == v.prg) {
      if (in_block()) {
        block_state = 1;
        block_start = v.prg->lptr;
        if ((block_col1 = v.prg->column) > strlen(v.prg->l))
          block_col1 = strlen(v.prg->l) + 1;
      } else if (v.prg->lptr < block_start ||
                 (v.prg->lptr == block_start && v.prg->column < block_col1)) {
        block_start = v.prg->lptr;
        block_col1 = v.prg->column;
      } else {
        block_end = v.prg->lptr;
        block_col2 = v.prg->column;
        if (block_col2 > strlen(v.prg->l))
          block_col2 = strlen(v.prg->l) + 1;
      }
    } else {
      for (n = 0; n < max_windows; n++)
        if (window[n].type == 102 && window[n].prg == kprg && kprg != NULL)
          break;
      if (n < max_windows) {
        wup(n);
        f_unmark();
        v.redraw = 2;
        _completo();
        wdown(n);
        flush_window(n);
      }
      block_state = 0;
      f_mark();
    }
  else if (block_state == 1) {
    block_state = 2;
    block_end = v.prg->lptr;
    if ((block_col2 = v.prg->column) > strlen(v.prg->l))
      block_col2 = strlen(v.prg->l) + 1;
    if ((block_start > block_end) || (block_start == block_end && block_col1 > block_col2)) {
      s = block_start;
      block_start = block_end;
      block_end = s;
      n = block_col1;
      block_col1 = block_col2;
      block_col2 = n;
    }
  } else if (block_state == 0) {
    block_state = 1;
    block_start = v.prg->lptr;
    kprg = v.prg;
    if ((block_col1 = v.prg->column) > strlen(v.prg->l))
      block_col1 = strlen(v.prg->l) + 1;
  }
  v.redraw++;
}

void f_unmark(void) {
  int n;
  if (block_state && kprg != v.prg) {
    for (n = 0; n < max_windows; n++)
      if (window[n].type == 102 && window[n].prg == kprg && kprg != NULL)
        break;
    if (n < max_windows) {
      wup(n);
      f_unmark();
      v.redraw = 2;
      _completo();
      wdown(n);
      flush_window(n);
    }
  }
  block_state = 0;
  edit_block_mode = 0;
  v.redraw++;
}

int t_p;

void f_cut_block(int mode) {
  int n;
  t_p = 0; // Clipboard type -> chars by default
  if (block_state && kprg != v.prg) {
    for (n = 0; n < max_windows; n++)
      if (window[n].type == 102 && window[n].prg == kprg && kprg != NULL)
        break;
    if (n < max_windows) {
      wup(n);
      f_cut(mode);
      v.redraw = 2;
      _completo();
      wdown(n);
      flush_window(n);
    }
  } else
    f_cut(mode);
  if (mode != 2) {
    clipboard_type = t_p;
  }
}

void f_cut(int mode) { // 0-Copy, 1-Cut, 2-Delete
  int n, num_lines, num_lineas2, num_lineas3;
  byte *k1, *k2;

  if (block_state == 0)
    return;
  else if (block_state & 1)
    f_mark();
  write_line();
  if (block_col1 > linelen(block_start))
    block_col1 = linelen(block_start) + 1; // ->cr
  if (block_col2 > linelen(block_end))
    block_col2 = linelen(block_end) + 2; // ->lf

  if (block_col1 == 1 && block_col2 == linelen(block_end) + 2)
    t_p = 1; // Clipboard type -> lines

  k2 = block_end + block_col2 - 1; // Set k2

  k1 = v.prg->buffer + v.prg->file_len; // If the nonexistent trailing crlf was selected, create it
  if (k2 > k1) {
    *k1++ = cr;
    *k1++ = lf;
    *k1 = 0;
    v.prg->file_len += 2;
    v.prg->num_lines++;
  }

  k1 = block_start + block_col1 - 1; // Set k1

  n = k2 - k1; // Text length

  // Count the number of crlf in the text

  num_lines = 0;
  num_lineas2 = 0;
  num_lineas3 = 0;
  while (n--) {
    if (*k1 == cr) {
      num_lines++;
      if (k1 < v.prg->lptr)
        num_lineas2++;
      if (k1 < v.prg->vptr)
        num_lineas3++;
    }
    k1++;
  }
  k1 = block_start + block_col1 - 1;

  if (mode != 2) {
    if (clipboard != NULL)
      free(clipboard);

    if ((clipboard = (char *)malloc(clipboard_len = k2 - k1 + 1)) == NULL) {
      // TODO: Show "out of memory" error to the user (clipboard not updated)
      clipboard_len = 0;
    } else {
      memcpy(clipboard, k1, clipboard_len);
      clipboard_lines = num_lines;
    }
  }

  if (mode) {
    if (in_block()) { // lptr inside the block
      v.prg->line -= num_lineas2;
      v.prg->lptr = block_start;
      v.prg->column = block_col1;
    } else if (v.prg->lptr > block_end) { // lptr after the block
      v.prg->line -= num_lines;
      v.prg->lptr -= k2 - k1 + 1;
    } else if (v.prg->lptr == block_end && v.prg->column > block_col2) { // column after the block
      v.prg->line -= num_lines;
      v.prg->lptr = block_start;
      v.prg->column += block_col1 - (block_col2 + 1);
    }

    if (v.prg->vptr > block_start && v.prg->vptr <= block_end) {
      v.prg->first_line -= num_lineas3;
      v.prg->vptr = block_start;
    } else if (v.prg->vptr > block_end) {
      v.prg->first_line -= num_lines;
      v.prg->vptr -= k2 - k1 + 1;
    }

    if (k2 < v.prg->buffer + v.prg->file_len)
      memmove(k1, k2 + 1, (byte *)v.prg->buffer + v.prg->file_len - k2);

    v.prg->file_len -= k2 - k1 + 1;

    v.prg->num_lines -= num_lines;

    block_state = 0;
  }

  read_line();
  test_cursor2();
  v.redraw++;
}

void test_cursor2(void) {
  if (v.prg->column - v.prg->first_column >= v.prg->w)
    v.prg->first_column = v.prg->column - v.prg->w + 1;

  if (v.prg->column < v.prg->first_column)
    v.prg->first_column = v.prg->column;

  while (v.prg->line >= v.prg->first_line + v.prg->h)
    advance_vptr();

  while (v.prg->line < v.prg->first_line)
    retreat_vptr();
}

void f_paste_block(void) {
  byte *ini, *p;
  int espacios, n, col = 0;

  if (clipboard_type == 1) {
    col = v.prg->column;
    v.prg->column = 1;
  }

  if (clipboard != NULL) {
    f_unmark();
    write_line();

    check_memory(clipboard_len);

    block_state = 2;
    kprg = v.prg;
    block_start = v.prg->lptr;
    block_col1 = v.prg->column; // Set block_start

    // Fill with spaces from end of current line to cursor

    if (v.prg->column - 1 > strlen(v.prg->l))
      espacios = v.prg->column - 1 - strlen(v.prg->l);
    else
      espacios = 0;

    ini = v.prg->lptr + v.prg->column - 1 - espacios; // Where the block will be pasted

    if (ini < v.prg->buffer + v.prg->file_len)
      memmove(ini + espacios + clipboard_len, ini, (byte *)v.prg->buffer + v.prg->file_len - ini);

    memset(ini, ' ', espacios); // Paste the spaces

    memcpy(ini + espacios, clipboard, clipboard_len); // Paste the clipboard

    block_end = ini + espacios + clipboard_len - 1;
    block_col2 = 1; // Set block_end
    while (block_end != v.prg->buffer && *block_end != cr) {
      block_col2++;
      block_end--;
    }
    if (block_end != v.prg->buffer && *block_end == cr) {
      block_end += 2;
      block_col2 -= 2;
    }

    n = 0;
    p = v.prg->lptr; // If current line exceeds 1023 chars, truncate it
    while (*p != cr && p < v.prg->buffer + v.prg->file_len) {
      p++;
      n++;
    }

    if (n > 1023) {
      if (v.prg->lptr + n < v.prg->buffer + v.prg->file_len) {
        memmove(v.prg->lptr + 1023, v.prg->lptr + n,
                v.prg->buffer + v.prg->file_len - v.prg->lptr + n);
      } else {
        *(v.prg->lptr + n) = 0;
      }
      v.prg->file_len -= n - 1023;
      if (block_end > v.prg->lptr)
        block_end -= 1023 - n;
      if (block_col2 > 1023)
        block_col2 = 1023;
    }

    v.prg->num_lines += clipboard_lines;
    v.prg->file_len += espacios + clipboard_len;
    read_line();
    v.redraw++;
  }

  if (col) {
    v.prg->column = col;
  }
}

//-----------------------------------------------------------------------------
//      Editing functions
//-----------------------------------------------------------------------------

void f_right(void) {
  if (v.prg->column < long_line - 1)
    v.prg->column++;
  if (v.prg->column - v.prg->first_column == v.prg->w) {
    v.prg->first_column++;
    v.redraw++;
  }
}

void f_left(void) {
  if (v.prg->column > 1)
    v.prg->column--;
  if (v.prg->column < v.prg->first_column) {
    v.prg->first_column--;
    v.redraw++;
  }
}

void f_down(void) {
  if (v.prg->lptr + v.prg->line_size < v.prg->buffer + v.prg->file_len) {
    write_line();
    advance_lptr();
    read_line();
    if (v.prg->line - v.prg->first_line == v.prg->h) {
      advance_vptr();
      v.redraw++;
    }
  }
  if (block_state & 1)
    v.redraw++;
}

void f_up(void) {
  if (v.prg->lptr > v.prg->buffer) {
    write_line();
    retreat_lptr();
    read_line();
    if (v.prg->line < v.prg->first_line) {
      retreat_vptr();
      v.redraw++;
    }
  }
  if (block_state & 1)
    v.redraw++;
}

void f_home(void) {
  if (v.prg->first_column != 1)
    v.redraw++;
  v.prg->first_column = v.prg->column = 1;
}

void f_end(void) {
  long n;
  remove_spaces();
  v.prg->column = strlen(v.prg->l) + 1;
  n = v.prg->column - v.prg->first_column;
  if (n < 0) {
    v.prg->first_column += n;
    v.redraw++;
  } else if (n >= v.prg->w) {
    v.prg->first_column += n + 1 - v.prg->w;
    v.redraw++;
  }
}

void f_scroll(int n) {
  write_line();
  while (n--) {
    advance_vptr();
    advance_lptr();
  }
  read_line();
  v.redraw++;
}

void f_mscroll(int n) {
  write_line();
  while (n--) {
    retreat_vptr();
    retreat_lptr();
  }
  read_line();
  v.redraw++;
}

void f_page_down(void) {
  f_scroll(v.prg->h);
}

void f_page_up(void) {
  f_mscroll(v.prg->h);
}

void f_insert(void) {
  cursor_mode ^= 1;
}

void f_delete_char(void) {
  int n, n_chars;
  byte *p;
  remove_spaces();
  if (strlen(v.prg->l) < v.prg->column) { // Join two lines
    if (v.prg->lptr + v.prg->line_size + 2 <= v.prg->buffer + v.prg->file_len) {
      for (n = strlen(v.prg->l); n < v.prg->column - 1; n++)
        v.prg->l[n] = ' ';
      p = v.prg->lptr + v.prg->line_size + 2; // Start of the next line
      n = v.prg->column - 1;
      n_chars = 0;
      if (p == block_start) {
        block_start = v.prg->lptr;
        block_col1 += n;
      }
      if (p == block_end) {
        block_end = v.prg->lptr;
        block_col2 += n;
      }
      while (p < v.prg->buffer + v.prg->file_len && *p != cr) {
        n_chars++;
        if (n < long_line - 1)
          v.prg->l[n++] = *p++;
      }
      v.prg->l[n] = 0;
      v.prg->line_size += 2 + n_chars;
      write_line();
      read_line();
      v.prg->num_lines--;
      v.redraw++;
    }
  } else { // Delete a character
    for (n = v.prg->column - 1; n < strlen(v.prg->l); n++)
      v.prg->l[n] = v.prg->l[n + 1];
    if (block_start == v.prg->lptr && block_col1 > v.prg->column)
      block_col1--;
    if (block_end == v.prg->lptr) {
      if (block_col2 > v.prg->column)
        block_col2--;
      else if (block_col2 == v.prg->column) {
        if (block_start == v.prg->lptr && block_col1 == block_col2)
          block_state = 0;
        else if (block_col2 == 1) {
          retreat_lptr();
          block_end = v.prg->lptr;
          advance_lptr();
          block_col2 = linelen(block_end) + 1;
        } else
          block_col2--;
      }
    }
  }
}

void f_backspace(void) {
  remove_spaces();
  ascii = 0;
  if (strlen(v.prg->l) < v.prg->column - 1)
    f_left();
  else if (v.prg->column != 1) {
    f_left();
    f_delete_char();
  } else if (v.prg->lptr > v.prg->buffer) {
    f_up();
    f_end();
    f_delete_char();
  }
}

void f_tab(void) {
  do {
    if (cursor_mode)
      f_right();
    else {
      ascii = ' ';
      f_insert_char();
    }
  } while (((v.prg->column - 1) % tab_size) != 0 && v.prg->column < long_line - 1);
  ascii = 0;
}

void f_untab(void) {
  if (v.prg->column > 1)
    do {
      if (cursor_mode)
        f_left();
      else
        f_backspace();
    } while (((v.prg->column - 1) % tab_size) != 0);
  ascii = 0;
}

void f_overwrite(void) {
  if (v.prg->column < long_line) {
    if (strlen(v.prg->l) < v.prg->column)
      f_insert_char();
    else {
      v.prg->l[v.prg->column - 1] = ascii;
      f_right();
    }
  }
}

void f_insert_char(void) {
  int n;

  if (v.prg->column < long_line) {
    if (strlen(v.prg->l) < v.prg->column) {
      for (n = strlen(v.prg->l); n < v.prg->column - 1; n++)
        v.prg->l[n] = ' ';
      v.prg->l[n] = ascii;
      v.prg->l[n + 1] = 0;
    } else {
      if (block_start == v.prg->lptr && block_col1 >= v.prg->column)
        block_col1++;
      if (block_end == v.prg->lptr && block_col2 >= v.prg->column)
        block_col2++;
      if (strlen(v.prg->l) < long_line - 1)
        v.prg->l[strlen(v.prg->l) + 1] = 0;
      for (n = strlen(v.prg->l); n >= v.prg->column; n--)
        v.prg->l[n] = v.prg->l[n - 1];
      v.prg->l[n] = ascii;
      v.prg->l[long_line - 1] = 0;
    }
    f_right();
  }
}

void f_enter(void) {
  int n, t, lon;
  remove_spaces();

  if ((lon = strlen(v.prg->l)) < v.prg->column) { // Normal enter
    v.prg->l[lon++] = cr;
    v.prg->l[lon++] = lf;
    v.prg->l[lon] = 0;
    t = 0;
    while (v.prg->l[t] == ' ')
      t++;
    write_line();
    advance_lptr();
    read_line();
    if (v.prg->line - v.prg->first_line == v.prg->h)
      advance_vptr();
    if (lon > 2) {
      f_home();
      while (t--)
        f_right();
    }
  } else { // Enter, split the current line
    n = v.prg->column - 1;
    t = 0;
    while (v.prg->l[t] == ' ' && t < n) {
      line_buffer[t] = ' ';
      t++;
    }
    memcpy(line_buffer + t, v.prg->l + n, long_line - n);
    v.prg->l[n] = 0;
    remove_spaces();
    n = strlen(v.prg->l);
    v.prg->l[n++] = cr;
    v.prg->l[n++] = lf;
    v.prg->l[n] = 0;

    write_line();

    if (block_start == v.prg->lptr && block_col1 >= v.prg->column) {
      block_start += n;
      block_col1 = block_col1 - v.prg->column + t + 1;
    }
    if (block_end == v.prg->lptr && block_col2 >= v.prg->column) {
      block_end += n;
      block_col2 = block_col2 - v.prg->column + t + 1;
    }

    advance_lptr();
    read_line();

    memcpy(v.prg->l, line_buffer, long_line);

    if (v.prg->line - v.prg->first_line == v.prg->h)
      advance_vptr();
    f_home();
    while (t--)
      f_right();
  }
  v.redraw++;
  v.prg->num_lines++;
}

void f_delete(void) {
  if (block_start == v.prg->lptr && block_end == v.prg->lptr)
    block_state = 0;
  else if (block_start == v.prg->lptr)
    block_col1 = 1;
  else if (block_end == v.prg->lptr) {
    retreat_lptr();
    block_end = v.prg->lptr;
    advance_lptr();
    block_col2 = linelen(block_end) + 1;
  }
  v.prg->l[0] = 0;
  if (v.prg->lptr + v.prg->line_size + 2 <= v.prg->buffer + v.prg->file_len) {
    delete_line();
    read_line();
    v.prg->num_lines--;
    v.redraw++;
  }
}

void f_word_right(void) {
  int i;
  do {
    if (v.prg->column <= strlen(v.prg->l))
      f_right();
    else {
      if (v.prg->lptr + v.prg->line_size >= v.prg->buffer + v.prg->file_len)
        break;
      f_down();
      f_home();
    }
    i = 1;
    if (!lower[v.prg->l[v.prg->column - 1]])
      i = 0;
    else if (lower[v.prg->l[v.prg->column - 2]])
      i = 0;
  } while (!i);
}

void f_word_right2(void) {
  int i;
  do {
    if (v.prg->column <= strlen(v.prg->l))
      f_right();
    else {
      if (v.prg->lptr + v.prg->line_size >= v.prg->buffer + v.prg->file_len)
        break;
      f_down();
      f_home();
    }
    i = 1;
    if (lower[v.prg->l[v.prg->column - 1]])
      i = 0;
    else if (!lower[v.prg->l[v.prg->column - 2]])
      i = 0;
  } while (!i);
}

void f_word_left(void) {
  int i;
  do {
    if (v.prg->column > 1)
      f_left();
    else {
      if (v.prg->lptr == v.prg->buffer)
        break;
      f_up();
      f_end();
    }
    i = 1;
    if (v.prg->column <= linelen(v.prg->lptr) + 1) {
      if (v.prg->column > 1) {
        if (!lower[v.prg->l[v.prg->column - 1]])
          i = 0;
        else if (lower[v.prg->l[v.prg->column - 2]])
          i = 0;
      } else {
        if (!lower[v.prg->l[v.prg->column - 1]])
          i = 0;
      }
    } else
      i = 0;
  } while (!i);
}

void f_eof(void) {
  while (v.prg->lptr + v.prg->line_size < v.prg->buffer + v.prg->file_len) {
    write_line();
    advance_lptr();
    read_line();
    if (v.prg->line - v.prg->first_line == v.prg->h)
      advance_vptr();
  }
  v.redraw++;
}

void f_bof(void) {
  while (v.prg->lptr > v.prg->buffer) {
    write_line();
    retreat_lptr();
    read_line();
    if (v.prg->line < v.prg->first_line)
      retreat_vptr();
  }
  v.redraw++;
}

void f_eop(void) {
  while (v.prg->lptr + v.prg->line_size < v.prg->buffer + v.prg->file_len &&
         v.prg->line - v.prg->first_line != v.prg->h - 1) {
    write_line();
    advance_lptr();
    read_line();
  }
  if (block_state & 1)
    v.redraw++;
}

void f_bop(void) {
  write_line();
  v.prg->lptr = v.prg->vptr;
  v.prg->line = v.prg->first_line;
  read_line();
  if (block_state & 1)
    v.redraw++;
}

//-----------------------------------------------------------------------------
//      Forward and backward navigation through the file
//-----------------------------------------------------------------------------

void advance_vptr(void) {
  byte *p = v.prg->vptr;

  while (*p != cr && p < v.prg->buffer + v.prg->file_len)
    p++;
  if (*p == cr && p < v.prg->buffer + v.prg->file_len) {
    if (_cached_color_source == (char *)v.prg->vptr) {
      color_source = (byte *)_cached_color_source;
      in_comment = _cached_in_comment;
      v.prg->vptr = p + 2;
      while (color_source < v.prg->vptr)
        color_lex();
      _cached_color_source = (char *)v.prg->vptr;
      _cached_in_comment = in_comment;
    } else
      v.prg->vptr = p + 2;
    v.prg->first_line++;
  }
}

void advance_lptr(void) {
  byte *p = v.prg->lptr;
  while (*p != cr && p < v.prg->buffer + v.prg->file_len)
    p++;
  if (*p == cr && p < v.prg->buffer + v.prg->file_len) {
    v.prg->lptr = p + 2;
    v.prg->line++;
  }
}

void retreat_vptr(void) {
  byte *p = v.prg->vptr;
  if (p != v.prg->buffer) {
    p -= 2;
    while (p > v.prg->buffer && (*p != cr || p == v.prg->vptr - 2))
      p--;
    if (p == v.prg->buffer && (*p != cr || p == v.prg->vptr - 2)) {
      if (_cached_color_source == (char *)v.prg->vptr) {
        color_source = (byte *)_cached_color_source;
        in_comment = _cached_in_comment;
        v.prg->vptr = p;
        color_token = -1;
        color_source = p;
        while (color_token != p_ultima)
          color_lex();
        _cached_color_source = (char *)v.prg->vptr;
        _cached_in_comment -= (in_comment - _cached_in_comment);
      } else
        v.prg->vptr = p;
    } else {
      if (_cached_color_source == (char *)v.prg->vptr) {
        color_source = (byte *)_cached_color_source;
        in_comment = _cached_in_comment;
        v.prg->vptr = p + 2;
        color_token = -1;
        color_source = p + 2;
        while (color_token != p_ultima)
          color_lex();
        _cached_color_source = (char *)v.prg->vptr;
        _cached_in_comment -= (in_comment - _cached_in_comment);
      }
      v.prg->vptr = p + 2;
    }
    v.prg->first_line--;
  }
}

void retreat_lptr(void) {
  byte *p = v.prg->lptr;
  if (p != v.prg->buffer) {
    p -= 2;
    while (p > v.prg->buffer && (*p != cr || p == v.prg->lptr - 2))
      p--;
    if (p == v.prg->buffer && (*p != cr || p == v.prg->lptr - 2)) {
      v.prg->lptr = p;
    } else {
      v.prg->lptr = p + 2;
    }
    v.prg->line--;
  }
}

byte *advance(byte *q) {
  byte *p = q;
  while (*p != cr && p < v.prg->buffer + v.prg->file_len)
    p++;
  if (*p == cr && p < v.prg->buffer + v.prg->file_len)
    return (p + 2);
  else
    return (q);
}

byte *retreat(byte *q) {
  byte *p = q;
  if (p != v.prg->buffer) {
    p -= 2;
    while (p > v.prg->buffer && (*p != cr || p == q - 2))
      p--;
    if (p == v.prg->buffer && (*p != cr || p == q - 2))
      return (p);
    else
      return (p + 2);
  } else
    return (q);
}

//-----------------------------------------------------------------------------
//      Source code coloring function
//-----------------------------------------------------------------------------

void fill_color_line(void) { // Get the colors for the next line
  unsigned char *p = color_source;
  int i = 0;

  if (!colorizer || !strstr((const char *)v.title, ".PRG")) {
    memset(colin, ce4, 1024);
  } else
    do {
      color_lex();
      switch (color_token) {
      case p_sym:
        while (i < color_source - p)
          colin[i++] = c_sim;
        break;
      case p_res:
        while (i < color_source - p)
          colin[i++] = c_res;
        break;
      case p_pre:
        while (i < color_source - p)
          colin[i++] = c_pre;
        break;
      case p_lit:
        while (i < color_source - p)
          colin[i++] = c_lit;
        break;
      case p_num:
        while (i < color_source - p)
          colin[i++] = c_num;
        break;
      case p_rem:
        while (i < color_source - p)
          colin[i++] = c_com;
        break;
      case p_spc:
      case p_id:
        while (i < color_source - p)
          colin[i++] = ce4;
        break;
      }
    } while (color_token != p_ultima);
}

//-----------------------------------------------------------------------------
//      Editor window rendering functions
//-----------------------------------------------------------------------------

void _completo(void) {
  int width, height;
  byte *di, *si, *old_di;
  byte *s, *_kini = NULL;
  int n, _kcol1 = 0;
  int x, col0, col1;
  int w, h, _an = 0, _al = 0;
  int i;

  SDL_Rect rc;

  rc.x = 4;
  rc.y = 35;
  rc.w = v.w - 30;
  rc.h = v.h - 50;


  if (_cached_color_source != (char *)v.prg->vptr) {
    color_source = v.prg->buffer;
    in_comment = 0;
    while (color_source < v.prg->vptr)
      color_lex();
    _cached_color_source = (char *)v.prg->vptr;
    color_source = (byte *)_cached_color_source;
    _cached_in_comment = in_comment;
  } else {
    color_source = (byte *)_cached_color_source;
    in_comment = _cached_in_comment;
  }

  if (v.h < v._al) {
    _an = v.w;
    _al = v.h;
    v.w = v._an;
    v.h = v._al;
  }
  w = v.w / big2;
  h = v.h / big2;
  if (block_state && kprg == v.prg) {
    if (block_col1 > linelen(block_start))
      block_col1 = linelen(block_start) + 1;
    if (block_state & 1) {
      _kini = block_start;
      _kcol1 = block_col1;
      block_end = v.prg->lptr;
      block_col2 = v.prg->column;
    }
    if (block_col2 > linelen(block_end))
      block_col2 = linelen(block_end) + 1;
    if ((block_start > block_end) || (block_start == block_end && block_col1 > block_col2)) {
      s = block_start;
      block_start = block_end;
      block_end = s;
      n = block_col1;
      block_col1 = block_col2;
      block_col2 = n;
    }
  }

  info_bar();
  scrollbars();

  di = v.ptr + (v.w * 18 + 2) * big2; // Blit the text window
  si = v.prg->vptr;
  height = v.prg->h;

  *(v.prg->buffer + v.prg->file_len) = cr;

  do {
    old_di = di;
    n = editor_font_height;
    if (block_state == 0 || kprg != v.prg || si < block_start ||
        si > block_end) { // Outside the block
      if (si > v.prg->buffer + v.prg->file_len)
        while (n--) {
          memset(di, ce01, v.w - 12 * big2);
          di += v.w;
        }
      else
        while (n--) {
          memset(di, ce1, v.w - 12 * big2);
          di += v.w;
        }
    } else if (si > block_start && si < block_end) { // Inside the block
      while (n--) {
        memset(di, ce4, v.w - 12 * big2);
        di += v.w;
      }
    } else { // Partial, paint background character by character
      if (si == block_start)
        col0 = block_col1;
      else
        col0 = 1;
      if (si == block_end) {
        if (block_col2 > linelen(block_end))
          col1 = long_line;
        else
          col1 = block_col2;
      } else
        col1 = long_line;
      x = v.prg->first_column;
      width = v.prg->w;
      while (width--) {
        n = editor_font_height;
        if (x >= col0 && x <= col1) {
          while (n--) {
            memset(di, ce4, editor_font_width);
            di += v.w;
          }
        } else {
          while (n--) {
            memset(di, ce1, editor_font_width);
            di += v.w;
          }
        }
        di -= v.w * editor_font_height - editor_font_width;
        x++;
      }
    }
    di = old_di;

    if (height == 1 && big && (editor_font_height * v_prg->h & 1)) { // Fix odd-sized window
      if (*di == ce01)
        memset(di + v.w * editor_font_height, ce01, v.w - 24);
      else
        memset(di + v.w * editor_font_height, ce1, v.w - 24);
    }

    width = v.prg->first_column;

    if (si != v.prg->lptr) {
      i = 0;
      color_source = si;
      if (si < v.prg->buffer + v.prg->file_len)
        fill_color_line();
      else
        memset(colin, ce4, 1024);

      while (--width && *si != cr && si < v.prg->buffer + v.prg->file_len) {
        si++;
        i++;
      }
      width = v.prg->w;
      while (*si != cr && si < v.prg->buffer + v.prg->file_len && width--) {
        put_char3(di, v.w, *si, *di == ce4, colin[i++]);
        di += editor_font_width;
        si++;
      }

    } else {
      s = si;
      si = (byte *)v.prg->l;

      i = 0;
      color_source = si;
      fill_color_line();
      color_cursor = colin[v.prg->column - 1];

      width = v.prg->first_column;
      while (--width && *si) {
        si++;
        i++;
      }
      width = v.prg->w;
      while (*si && width--) {
        put_char3(di, v.w, *si, *di == ce4, colin[i++]);
        di += editor_font_width;
        si++;
      }
      width = -1;
      si = s;
    }

    while (*si != cr && si < v.prg->buffer + v.prg->file_len)
      si++;
    if (si < v.prg->buffer + v.prg->file_len)
      si += 2;
    else
      si++;

    di = old_di + v.w * editor_font_height;

  } while (--height);

  if ((block_state & 1) && kprg == v.prg) {
    block_start = _kini;
    block_col1 = _kcol1;
  }

  if (_an && _al) {
    v.w = _an;
    v.h = _al;
  }
}

//-----------------------------------------------------------------------------
//      Partial rendering of an editor window (current line only)
//-----------------------------------------------------------------------------

void _parcial(void) {
  int linea, width;
  byte *di, *si, *old_di;
  byte *s, *_kini = NULL;
  int n, _kcol1 = 0;
  int x, col0, col1;
  int w, h, _an = 0, _al = 0;
  int i;

  if (_cached_color_source != (char *)v.prg->vptr) {
    color_source = v.prg->buffer;
    in_comment = 0;
    while (color_source < v.prg->vptr)
      color_lex();
    _cached_color_source = (char *)v.prg->vptr;
    color_source = (byte *)_cached_color_source;
    _cached_in_comment = in_comment;
  } else {
    color_source = (byte *)_cached_color_source;
    in_comment = _cached_in_comment;
  }

  while (color_source < v.prg->lptr)
    color_lex();
  numrem = 0;
  color_source = (byte *)v.prg->l;
  fill_color_line();
  color_cursor = colin[v.prg->column - 1];

  if (c_oldline == (char *)v.prg->lptr && numrem != old_comment_state) {
    c_oldline = (char *)v.prg->lptr;
    old_comment_state = numrem;
    v.redraw++;
    _completo();
    return;
  }
  c_oldline = (char *)v.prg->lptr;
  old_comment_state = numrem;

  if (v.h < v._al) {
    _an = v.w;
    _al = v.h;
    v.w = v._an;
    v.h = v._al;
  }
  w = v.w / big2;
  h = v.h / big2;

  linea = v.prg->line - v.prg->first_line;

  if (block_state && kprg == v.prg) {
    if (block_col1 > linelen(block_start))
      block_col1 = linelen(block_start) + 1;
    if (block_state & 1) {
      _kini = block_start;
      _kcol1 = block_col1;
      block_end = v.prg->lptr;
      block_col2 = v.prg->column;
    }
    if (block_col2 > linelen(block_end))
      block_col2 = linelen(block_end) + 1;
    if ((block_start > block_end) || (block_start == block_end && block_col1 > block_col2)) {
      s = block_start;
      block_start = block_end;
      block_end = s;
      n = block_col1;
      block_col1 = block_col2;
      block_col2 = n;
    }
  }

  info_bar();

  old_di = di = v.ptr + (v.w * 18 + 2) * big2 + linea * v.w * editor_font_height;
  si = v.prg->lptr;

  n = editor_font_height;
  if (block_state == 0 || kprg != v.prg || si < block_start ||
      si > block_end) { // Outside the block
    while (n--) {
      memset(di, ce1, v.w - 12 * big2);
      di += v.w;
    }
  } else if (si > block_start && si < block_end) { // Inside the block
    while (n--) {
      memset(di, ce4, v.w - 12 * big2);
      di += v.w;
    }
  } else { // Partial, paint background character by character
    if (si == block_start)
      col0 = block_col1;
    else
      col0 = 1;
    if (si == block_end) {
      if (block_col2 > linelen(block_end))
        col1 = long_line;
      else
        col1 = block_col2;
    } else
      col1 = long_line;
    x = v.prg->first_column;
    width = v.prg->w;
    while (width--) {
      n = editor_font_height;
      if (x >= col0 && x <= col1) {
        while (n--) {
          memset(di, ce4, editor_font_width);
          di += v.w;
        }
      } else {
        while (n--) {
          memset(di, ce1, editor_font_width);
          di += v.w;
        }
      }
      di -= v.w * editor_font_height - editor_font_width;
      x++;
    }
  }

  di = old_di;
  si = (byte *)v.prg->l;
  i = 0;

  width = v.prg->first_column;
  while (--width && *si) {
    si++;
    i++;
  }
  width = v.prg->w;
  while (*si && width--) {
    put_char3(di, v.w, *si, *di == ce4, colin[i++]);

    di += editor_font_width;
    si++;
  }

  if ((block_state & 1) && kprg == v.prg) {
    block_start = _kini;
    block_col1 = _kcol1;
  }

  if (_an && _al) {
    v.w = _an;
    v.h = _al;
  }
}

//-----------------------------------------------------------------------------
//      Scrollbar rendering
//-----------------------------------------------------------------------------

void scrollbars(void) {
  byte *ptr = v.ptr;
  int min, max, slider;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wbox(ptr, w, h, c2, w - 9, 18, 7, h - 12 - 24); // Vertical slider
  min = 18;
  max = h - 21;
  if (v.prg->num_lines <= 1)
    slider = min;
  else
    slider = min + ((v.prg->first_line - 1) * (max - min)) / (v.prg->num_lines - 1);
  if (forced_slider) {
    slider = forced_slider;
    forced_slider = 0;
  }
  wbox(ptr, w, h, c0, w - 9, slider - 1, 7, 1);
  wbox(ptr, w, h, c0, w - 9, slider + 3, 7, 1);
  wput(ptr, w, h, w - 9, slider, 43);

  wbox(ptr, w, h, c2, 10, h - 9, w - 4 - 24, 7); // Horizontal slider
  min = 10;
  max = w - 21;
  slider = min + (v.prg->first_column * (max - min)) / (long_line - v.prg->w);
  wbox(ptr, w, h, c0, slider - 1, h - 9, 1, 7);
  wbox(ptr, w, h, c0, slider + 3, h - 9, 1, 7);
  wput(ptr, w, h, slider, h - 9, 55);
}

int get_slide_y(void) {
  int min, max;
  int h = v.h;
  if (big)
    h /= 2;
  min = 18;
  max = h - 21;
  if (v.prg->num_lines <= 1)
    return (min);
  else
    return (min + ((v.prg->first_line - 1) * (max - min)) / (v.prg->num_lines - 1));
}

int get_slide_x(void) {
  int min, max;
  int w = v.w;
  if (big)
    w /= 2;
  min = 10;
  max = w - 21;
  return (min + (v.prg->first_column * (max - min)) / (long_line - v.prg->w));
}

//-----------------------------------------------------------------------------
//      Info bar rendering
//-----------------------------------------------------------------------------

void info_bar(void) {
  char num[16];
  byte *ptr = v.ptr;
  int width;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wbox(ptr, w, h, c12, 2, 10, w - 4 - 16, 7); // Status bar
  itoa(v.prg->line, num, 10);
  wwrite_in_box(v.ptr, w, w - 19, h, 3, 10, 0, (byte *)num, c3);
  width = text_len((byte *)num) + 1;
  itoa(v.prg->column, num + 1, 10);
  num[0] = ',';
  wwrite_in_box(v.ptr, w, w - 19, h, 3 + width, 10, 0, (byte *)num, c3);
  width += text_len((byte *)num) + 1;
}

//-----------------------------------------------------------------------------
//      Enter the window resize loop
//-----------------------------------------------------------------------------

void resize_surface(void) {
  window_surface(v.w, v.h, 0);
}

void resize(void) {
  int _mx = mouse_x, _my = mouse_y; // Initial mouse coordinates
  int mx, my;                       // Snapped mouse coordinates at each moment
  int _an, _al;                     // Original an/al (in chars)
  int old_w, old_h;                 // Previous an/al
  byte *new_block;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  _an = v.prg->w;
  _al = v.prg->h;

  wput(v.ptr, w, h, w - 9, h - 9, -44);
  flush_window(0);

  do {
    read_mouse();
    mx = _mx + ((mouse_x - _mx) / editor_font_width) * editor_font_width;
    my = _my + ((mouse_y - _my) / editor_font_height) * editor_font_height;

    old_w = v.prg->w;
    old_h = v.prg->h;
    v.prg->w = _an + (mouse_x - _mx) / editor_font_width;
    v.prg->h = _al + (mouse_y - _my) / editor_font_height;
    if (v.prg->w < 4 * big2) {
      v.prg->w = 4 * big2;
      mx = _mx + (v.prg->w - _an) * editor_font_width;
    }
    if (v.prg->h < 2 * big2) {
      v.prg->h = 2 * big2;
      my = _my + (v.prg->h - _al) * editor_font_height;
    }
    if (v.prg->w > 80 * 2) {
      v.prg->w = 80 * 2;
      mx = _mx + (v.prg->w - _an) * editor_font_width;
    }
    if (v.prg->h > 50 * 2) {
      v.prg->h = 50 * 2;
      my = _my + (v.prg->h - _al) * editor_font_height;
    }

    w = v.w;
    h = v.h;

    v.w = (4 + 8) * big2 + editor_font_width * v.prg->w;
    v.h = (12 + 16) * big2 + editor_font_height * v.prg->h;

    if (big) {
      if (v.w & 1)
        v.w++;
      if (v.h & 1)
        v.h++;
    }

    if ((new_block = (byte *)realloc(v.ptr, v.w * v.h)) != NULL) {
      v.ptr = new_block;
      resize_surface();

      test_cursor();
      repaint_window();
      wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 9, v.h / big2 - 9, -44);
      on_window_moved(v.x, v.y, w, h);
      update_box(v.x, v.y, v.w > w ? v.w : w, v.h > h ? v.h : h);
    } else {
      v.prg->w = old_w;
      v.prg->h = old_h;
      v.w = w;
      v.h = h;
    }

    w = v.w / big2;
    h = v.h / big2;

    save_mouse_bg(mouse_background, mx, my, mouse_graf, 0);
    put(mx, my, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mx, my, mouse_graf, 1);

  } while (mouse_b & 1);

  wput(v.ptr, w, h, w - 9, h - 9, -34);
  v.redraw = 2;
}

//-----------------------------------------------------------------------------
// Check the cursor position
//-----------------------------------------------------------------------------

void test_cursor(void) {
  if (v.prg->column - v.prg->first_column >= v.prg->w)
    v.prg->column = v.prg->first_column + v.prg->w - 1;

  if (v.prg->line - v.prg->first_line >= v.prg->h)
    do {
      write_line();
      retreat_lptr();
      read_line();
    } while (v.prg->line >= v.prg->first_line + v.prg->h);
}

//-----------------------------------------------------------------------------
//      Maximize or restore a PRG window
//-----------------------------------------------------------------------------

void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2);

void maximize(void) {
  byte *new_block;
  int w = v.w, h = v.h;
  int _x, _y, _an, _al, _an2, _al2;
  if (big) {
    w /= 2;
    h /= 2;
  }

  _an = (vga_width - 12 * big2) / editor_font_width; // Calculate maximized size (in chars)
  _al = (vga_height - 28 * big2) / editor_font_height;
  if (_an > 100)
    _an = 100;
  if (_al > 100)
    _al = 100;

  if (v.prg->w != _an || v.prg->h != _al) { // *** Maximize ***

    v.prg->old_w = v.prg->w;
    v.prg->old_h = v.prg->h;
    v.prg->old_x = v.x;
    v.prg->old_y = v.y;

    v.prg->w = _an;
    v.prg->h = _al;

    _an2 = v.w;
    _al2 = v.h;
    v.w = (4 + 8) * big2 + editor_font_width * v.prg->w;
    v.h = (12 + 16) * big2 + editor_font_height * v.prg->h;

    if (big) {
      if (v.w & 1)
        v.w++;
      if (v.h & 1)
        v.h++;
    }

    if ((new_block = (byte *)realloc(v.ptr, v.w * v.h)) != NULL) {
      _an = _an2;
      _al = _al2;
      v.ptr = new_block;
      resize_surface();
      test_cursor();
      repaint_window();
      v.x = vga_width / 2 - v.w / 2;
      v.y = vga_height / 2 - v.h / 2;
      // place_window(1,&v.x,&v.y,v.an,v.al);
      if (exploding_windows)
        extrude(v.prg->old_x, v.prg->old_y, _an, _al, v.x, v.y, v.w, v.h);
      v.redraw = 2;
      on_window_moved(v.prg->old_x, v.prg->old_y, _an, _al);
    } else {
      v.prg->w = v.prg->old_w;
      v.prg->h = v.prg->old_h;
      v.w = _an;
      v.h = _al;
      wput(v.ptr, w, h, w - 17, 10, -56);
      v.redraw = 2;
    }

  } else { // *** Restore ***

    _an2 = v.w;
    _al2 = v.h;
    _x = v.x;
    _y = v.y;

    v.prg->w = v.prg->old_w;
    v.prg->h = v.prg->old_h;

    v.w = (4 + 8) * big2 + editor_font_width * v.prg->w;
    v.h = (12 + 16) * big2 + editor_font_height * v.prg->h;

    if (big) {
      if (v.w & 1)
        v.w++;
      if (v.h & 1)
        v.h++;
    }

    if ((new_block = (byte *)realloc(v.ptr, v.w * v.h)) != NULL) {
      v.ptr = new_block;
      resize_surface();

      test_cursor();
      repaint_window();
      v.x = v.prg->old_x;
      v.y = v.prg->old_y;
      if (v.x > vga_width - 8 * big2)
        v.x = vga_width - 32 * big2;
      if (v.y > vga_height - 8 * big2)
        v.y = vga_height - 32 * big2;
      if (exploding_windows)
        extrude(_x, _y, _an2, _al2, v.x, v.y, v.w, v.h);
      v.redraw = 2;
      on_window_moved(_x, _y, _an2, _al2);
    } else {
      v.prg->w = _an;
      v.prg->h = _al;
      v.w = _an2;
      v.h = _al2;
      wput(v.ptr, w, h, w - 17, 10, -56);
      v.redraw = 2;
    }
  }
}

//-----------------------------------------------------------------------------
//      Repaint a window (including title bar and icons)
//-----------------------------------------------------------------------------

void repaint_window(void) {
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  memset(v.ptr, c0, v.w * v.h);
  wrectangle(v.ptr, w, h, c2, 0, 0, w, h);
  wput(v.ptr, w, h, w - 9, 2, 35);

  if (v.type == 1) { // Dialogs cannot be minimized
    wgra(v.ptr, w, h, c_b_low, 2, 2, w - 12, 7);
    if (text_len(v.title) + 3 > w - 12) {
      wwrite_in_box(v.ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
      wwrite_in_box(v.ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
    } else {
      wwrite(v.ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
      wwrite(v.ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
    }
  } else {
    wput(v.ptr, w, h, w - 17, 2, 37);
    wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
    } else {
      wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
    }
  }

  call((void_return_type_t)v.paint_handler);
}

//-----------------------------------------------------------------------------
//      Determine if the cursor is inside a block
//-----------------------------------------------------------------------------

int in_block(void) {
  int col0, col1;

  if (kprg != v.prg)
    return (0);
  if (block_state & 1)
    return (1);
  else if (block_state && v.prg->lptr >= block_start && v.prg->lptr <= block_end)
    if (v.prg->lptr > block_start && v.prg->lptr < block_end)
      return (1);
    else {
      if (v.prg->lptr == block_start)
        col0 = block_col1;
      else
        col0 = 1;
      if (v.prg->lptr == block_end) {
        if (block_col2 > linelen(block_end))
          col1 = long_line;
        else
          col1 = block_col2;
      } else
        col1 = long_line;
      if (v.prg->column >= col0 && v.prg->column <= col1)
        return (1);
      else
        return (0);
    }
  else
    return (0);
}

//-----------------------------------------------------------------------------
//      Text cursor
//-----------------------------------------------------------------------------

void text_cursor(void) {
  int x, y, height, block, n, keypressed;
  byte *old_di, *di, *si, ch;

  x = (v.prg->column - v.prg->first_column) * editor_font_width;
  y = (v.prg->line - v.prg->first_line) * editor_font_height;
  old_di = v.ptr + (v.w * 18 + 2) * big2;
  old_di += x + y * v.w;

  if (cursor_mode) {
    height = editor_font_height;
    di = old_di;
  } else {
    height = editor_font_height / 2;
    di = old_di + v.w * (editor_font_height - height);
  }

  block = in_block();

  keypressed = 0;

  for (n = 0; n < 128; n++) {
    if (key(n)) {
      keypressed = 1;
      n = 128;
    }
  }

  if (*system_clock & 4 || keypressed) {
    if (block)
      do {
        memset(di, ce1, editor_font_width);
        di += v.w;
      } while (--height);
    else
      do {
        memset(di, c_y, editor_font_width);
        di += v.w;
      } while (--height);
  } else {
    if (block)
      do {
        memset(di, ce4, editor_font_width);
        di += v.w;
      } while (--height);
    else
      do {
        memset(di, ce1, editor_font_width);
        di += v.w;
      } while (--height);
    si = (byte *)v.prg->l;
    y = v.prg->column;
    x = 0;
    while (--y && si[x]) {
      x++;
    }
    if (!(ch = si[x]))
      ch = ' ';
    put_char3(old_di, v.w, ch, block, color_cursor);
  }
}

void error_cursor2(void) {
  int x, y, height;
  byte *old_di, *di, *si, ch;

  x = (v.prg->column - v.prg->first_column) * editor_font_width;
  y = (v.prg->line - v.prg->first_line) * editor_font_height;
  old_di = v.ptr + (v.w * 18 + 2) * big2;
  old_di += x + y * v.w;

  height = editor_font_height;
  di = old_di;
  do {
    memset(di, c_r_low, editor_font_width);
    di += v.w;
  } while (--height);

  si = (byte *)v.prg->l;
  y = v.prg->column;
  x = 0;
  while (--y && si[x])
    x++;
  if (!(ch = si[x]))
    ch = ' ';
  put_char2(old_di, v.w, ch, c4);

  if (cursor_mode) {
    height = editor_font_height;
    di = old_di;
  } else {
    height = editor_font_height / 2;
    di = old_di + v.w * (editor_font_height - height);
  }

  if (*system_clock & 4) {
    do {
      memset(di, c_y, editor_font_width);
      di += v.w;
    } while (--height);
  }
}

void error_cursor(void) {
  int x, y, height;
  byte *old_di, *di, *si, ch;

  x = (v.prg->column - v.prg->first_column) * editor_font_width;
  y = (v.prg->line - v.prg->first_line) * editor_font_height;
  old_di = v.ptr + (v.w * 18 + 2) * big2;
  old_di += x + y * v.w;

  height = editor_font_height;
  di = old_di;
  do {
    memset(di, c_r_low, editor_font_width);
    di += v.w;
  } while (--height);

  si = (byte *)v.prg->l;
  y = v.prg->column;
  x = 0;
  while (--y && si[x])
    x++;
  if (!(ch = si[x]))
    ch = ' ';
  put_char2(old_di, v.w, ch, c4);
}

void delete_text_cursor(void) {
  int x, y, height, block;
  byte *old_di, *di, *si, ch;

  x = (v.prg->column - v.prg->first_column) * editor_font_width;
  y = (v.prg->line - v.prg->first_line) * editor_font_height;
  di = v.ptr + (v.w * 18 + 2) * big2;
  di += x + y * v.w;
  old_di = di;
  height = editor_font_height;

  block = in_block();

  if (block)
    do {
      memset(di, ce4, editor_font_width);
      di += v.w;
    } while (--height);
  else
    do {
      memset(di, ce1, editor_font_width);
      di += v.w;
    } while (--height);

  si = (byte *)v.prg->l;
  y = v.prg->column;
  x = 0;
  while (--y && si[x]) {
    x++;
  }
  if (!(ch = si[x]))
    ch = ' ';
  put_char3(old_di, v.w, ch, block, color_cursor);
}

//-----------------------------------------------------------------------------
//      Character rendering
//-----------------------------------------------------------------------------

void put_char(byte *ptr, int w, byte c, int block) {
  int n, m;
  byte *si, color;

  si = font + c * char_size;
  //printf("%c",c);

  if (block) {
    color = ce1;
    n = editor_font_height;
    do {
      m = editor_font_width;
      do {
        if (*si)
          *ptr = color;
        si++;
        ptr++;
      } while (--m);
      ptr += w - editor_font_width;
    } while (--n);

  } else {
    //    if (in_comment>0) color=ce2; else color=ce4;
    color = ce4;
    n = editor_font_height;
    switch (editor_font_width) {
    case 6:
      do {
        if (*(si + 0))
          *(ptr + 0) = color;
        if (*(si + 1))
          *(ptr + 1) = color;
        if (*(si + 2))
          *(ptr + 2) = color;
        if (*(si + 3))
          *(ptr + 3) = color;
        if (*(si + 4))
          *(ptr + 4) = color;
        if (*(si + 5))
          *(ptr + 5) = color;
        ptr += w;
        si += 6;
      } while (--n);
      break;
    case 8:
      do {
        if (*(si + 0))
          *(ptr + 0) = color;
        if (*(si + 1))
          *(ptr + 1) = color;
        if (*(si + 2))
          *(ptr + 2) = color;
        if (*(si + 3))
          *(ptr + 3) = color;
        if (*(si + 4))
          *(ptr + 4) = color;
        if (*(si + 5))
          *(ptr + 5) = color;
        if (*(si + 6))
          *(ptr + 6) = color;
        if (*(si + 7))
          *(ptr + 7) = color;
        ptr += w;
        si += 8;
      } while (--n);
      break;
    case 9:
      do {
        if (*(si + 0))
          *(ptr + 0) = color;
        if (*(si + 1))
          *(ptr + 1) = color;
        if (*(si + 2))
          *(ptr + 2) = color;
        if (*(si + 3))
          *(ptr + 3) = color;
        if (*(si + 4))
          *(ptr + 4) = color;
        if (*(si + 5))
          *(ptr + 5) = color;
        if (*(si + 6))
          *(ptr + 6) = color;
        if (*(si + 7))
          *(ptr + 7) = color;
        if (*(si + 8))
          *(ptr + 8) = color;
        ptr += w;
        si += 9;
      } while (--n);
      break;
    }
  }
}

void put_char2(byte *ptr, int w, byte c, byte color) {
  int n, m;
  byte *si;

  si = font + c * char_size;

  n = editor_font_height;
  do {
    m = editor_font_width;
    do {
      if (*si)
        *ptr = color;

      si++;
      ptr++;

    } while (--m);

    ptr += w - editor_font_width;

  } while (--n);
}

void put_char3(byte *ptr, int w, byte c, int block, byte color) {
  int n, m;
  byte *si;

  si = font + c * char_size;

  if (block) {
    color = ce1;
    n = editor_font_height;
    do {
      m = editor_font_width;
      do {
        if (*si)
          *ptr = color;
        si++;
        ptr++;
      } while (--m);
      ptr += w - editor_font_width;
    } while (--n);

  } else {
    n = editor_font_height;
    switch (editor_font_width) {
    case 6:
      do {
        if (*(si + 0))
          *(ptr + 0) = color;
        if (*(si + 1))
          *(ptr + 1) = color;
        if (*(si + 2))
          *(ptr + 2) = color;
        if (*(si + 3))
          *(ptr + 3) = color;
        if (*(si + 4))
          *(ptr + 4) = color;
        if (*(si + 5))
          *(ptr + 5) = color;
        ptr += w;
        si += 6;
      } while (--n);
      break;
    case 8:
      do {
        if (*(si + 0))
          *(ptr + 0) = color;
        if (*(si + 1))
          *(ptr + 1) = color;
        if (*(si + 2))
          *(ptr + 2) = color;
        if (*(si + 3))
          *(ptr + 3) = color;
        if (*(si + 4))
          *(ptr + 4) = color;
        if (*(si + 5))
          *(ptr + 5) = color;
        if (*(si + 6))
          *(ptr + 6) = color;
        if (*(si + 7))
          *(ptr + 7) = color;
        ptr += w;
        si += 8;
      } while (--n);
      break;
    case 9:
      do {
        if (*(si + 0))
          *(ptr + 0) = color;
        if (*(si + 1))
          *(ptr + 1) = color;
        if (*(si + 2))
          *(ptr + 2) = color;
        if (*(si + 3))
          *(ptr + 3) = color;
        if (*(si + 4))
          *(ptr + 4) = color;
        if (*(si + 5))
          *(ptr + 5) = color;
        if (*(si + 6))
          *(ptr + 6) = color;
        if (*(si + 7))
          *(ptr + 7) = color;
        if (*(si + 8))
          *(ptr + 8) = color;
        ptr += w;
        si += 9;
      } while (--n);
      break;
    }
  }
}


//-----------------------------------------------------------------------------
//      Read a line from the program (stores it as ASCIIZ)
//-----------------------------------------------------------------------------

void read_line(void) {
  byte *p = v.prg->lptr;
  int len = 0;
  while (*p != cr && p < v.prg->buffer + v.prg->file_len) {
    p++;
    len++;
  }
  if (len)
    memcpy(v.prg->l, v.prg->lptr, len);
  v.prg->l[len] = 0;
  v.prg->line_size = len;
}

//-----------------------------------------------------------------------------
//      Write a line back to the program buffer
//-----------------------------------------------------------------------------

void write_line(void) {
  byte *ini, *fin;
  int old_lon, lon;

  remove_spaces();

  old_lon = v.prg->file_len;
  v.prg->file_len += strlen(v.prg->l) - v.prg->line_size;

  ini = v.prg->lptr + v.prg->line_size; // Where the cr,lf of this line is
  fin = v.prg->lptr + strlen(v.prg->l); // Where it should go

  if (ini < v.prg->buffer + old_lon) {
    lon = (uintptr_t)v.prg->buffer + old_lon - (uintptr_t)ini;
    memmove(fin, ini, lon);
    if (block_start > ini && block_start < ini + lon)
      block_start += fin - ini;
    if (block_end > ini && block_end < ini + lon)
      block_end += fin - ini;
  }

  ini = (byte *)v.prg->l;
  fin = v.prg->lptr;
  while (*ini)
    *fin++ = *ini++;
}

void delete_line(void) {
  byte *ini, *fin;
  int old_lon, lon;

  remove_spaces();

  old_lon = v.prg->file_len;
  v.prg->file_len += strlen(v.prg->l) - (v.prg->line_size + 2);

  ini = v.prg->lptr + (v.prg->line_size + 2); // Where the cr,lf of this line is
  fin = v.prg->lptr + strlen(v.prg->l);       // Where it should go

  if (ini < v.prg->buffer + old_lon) {
    lon = (uintptr_t)v.prg->buffer + old_lon - (uintptr_t)ini;
    memmove(fin, ini, lon);
    if (block_start >= ini && block_start < ini + lon)
      block_start += fin - ini;
    if (block_end >= ini && block_end < ini + lon)
      block_end += fin - ini;
  }

  ini = (byte *)v.prg->l;
  fin = v.prg->lptr;
  while (*ini)
    *fin++ = *ini++;
}

//-----------------------------------------------------------------------------
//      Strip trailing spaces from the current line (v.prg->l[])
//-----------------------------------------------------------------------------

void remove_spaces(void) {
  int n = strlen(v.prg->l) - 1;
  while (n >= 0)
    if (v.prg->l[n] == ' ')
      v.prg->l[n--] = 0;
    else
      n = -1;
}

//-----------------------------------------------------------------------------
//      Open a program
//-----------------------------------------------------------------------------

#define max_archivos 512 // ------------------------------- File listbox
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[max_archivos];
extern int num_taggeds;

void open_program(void) {
  FILE *f;
  int n, x;
  byte *buffer, *p;
  struct tprg *pr;
  int num;

  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), tipo[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    if ((f = fopen(full, "rb")) != NULL) {
      fclose(f);
      v_exists = 1;
    } else
      v_exists = 0;
    div_strcpy(file_list_br.list, file_list_br.item_width, input);
    file_list_br.total_items = 1;
    thumb[0].tagged = 1;
    num_taggeds = 1;
  }

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), tipo[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);

      if ((f = fopen(full, "rb")) != NULL) { // A file was selected
        fseek(f, 0, SEEK_END);
        n = ftell(f) + buffer_grow;
        if ((buffer = (byte *)malloc(n)) != NULL) {
          memset(buffer, 0, n);

          if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
            v_prg->buffer_len = n;
            div_strcpy(v_prg->filename, sizeof(v_prg->filename), input);
            div_strcpy(v_prg->path, sizeof(v_prg->path), tipo[v_type].path);
            fseek(f, 0, SEEK_SET);

            if (fread(buffer, 1, n, f) == n - buffer_grow) {
              for (p = buffer; p < buffer + n - buffer_grow; p++) // Reject binary files
                if (!*p) {
                  n = 0;
                  break;
                }

              if (n) {
                n -= buffer_grow;
                v_prg->file_len = n;
                v_prg->buffer = buffer;
                v_prg->num_lines = 1;

                x = 0;
                while (n--) {
                  if (*buffer == cr || *buffer == lf) {
                    if (*(buffer + 1) != ((*buffer) ^ (cr ^ lf))) {
                      memmove(buffer + 1, buffer, n + 1);
                      v_prg->file_len++;
                    } else if (n)
                      n--;
                    *buffer++ = cr;
                    *buffer = lf;
                    v_prg->num_lines++;
                    x = -1;
                  }

                  if (*buffer == 9) {
                    if (x >= 1020)
                      *buffer = ' ';
                    else {
                      memmove(buffer + 3, buffer, n + 1);
                      v_prg->file_len += 3;
                      x += 3;
                      *buffer++ = ' ';
                      *buffer++ = ' ';
                      *buffer++ = ' ';
                      *buffer = ' ';
                    }
                  }

                  buffer++;
                  x++;

                  if (x >= 1023) {
                    memmove(buffer + 2, buffer, n + 1);
                    *buffer++ = cr;
                    *buffer++ = lf;
                    v_prg->file_len += 2;
                    v_prg->num_lines++;
                    x = 0;
                  }
                }

                v_prg->lptr = v_prg->buffer;
                pr = v.prg;
                v.prg = v_prg;
                read_line();
                v.prg = pr;

                new_window(program0);

              } else {
                free(v_prg);
                free(buffer);
                v_text = (char *)texts[46];
                show_dialog(err0);
              }
            } else {
              free(v_prg);
              free(buffer);
              v_text = (char *)texts[44];
              show_dialog(err0);
            }
          } else {
            free(buffer);
            v_text = (char *)texts[45];
            show_dialog(err0);
          }
        } else {
          v_text = (char *)texts[45];
          show_dialog(err0);
        }
        fclose(f);
      } else {
        if (v_finished != -1) {
          v_text = (char *)texts[44];
          show_dialog(err0);
        }
        v_finished = 0;
      }
    }
  }
}
extern char user1[128], user2[128];

void program0_new(void) {
  byte *buffer;
  struct tprg *pr;
  int n;
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  struct tm *timeinfo;
  time_t dtime;

  div_strcpy(full, sizeof(full), tipo[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  if (v_finished) {
    f = fopen(full, "wb");
    // insert template
    if (f) {
      time(&dtime);
      timeinfo = localtime(&dtime);
      _splitpath(full, drive, dir, fname, ext);
      fprintf(f, "/*\n * %s%s by %s\n * (c) %d %s \n */\n\n", fname, ext, user1,
              (timeinfo->tm_year) + 1900, user2);
      fprintf(f,
              "PROGRAM %s;\n\nBEGIN\n\n//Write your code here, make something amazing!\n\nEND\n\n",
              fname);
      fclose(f);
      open_program();

      for (n = 0; n < 10; n++) {
        write_line();
        advance_lptr();
        read_line();
      }

      return;
    }
    n = buffer_grow;
    if ((buffer = (byte *)malloc(n)) != NULL) {
      memset(buffer, 0, n);
      if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
        memset(v_prg, 0, sizeof(struct tprg));
        v_prg->buffer_len = n;
        div_strcpy(v_prg->filename, sizeof(v_prg->filename), input);
        div_strcpy(v_prg->path, sizeof(v_prg->path), tipo[v_type].path);
        //        n-=buffer_grow;
        n = strlen((char *)buffer);
        v_prg->file_len = n;
        v_prg->buffer = buffer;
        v_prg->lptr = buffer;
        pr = v.prg;
        v.prg = v_prg;
        read_line();
        v.prg = pr;
        v_prg->num_lines = 1;
        new_window(program0);
        // Add the template
        div_strcpy((char *)buffer, buffer_grow, "PROGRAM yourprg;");
        read_line();
        f_enter();
        // Add the template
        div_strcat((char *)buffer, buffer_grow, "BEGIN");
        // Your code here\n\n END");
        f_enter();


      } else {
        free(buffer);
        v_text = (char *)texts[45];
        show_dialog(err0);
      }
    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
    }
  }
}

//-----------------------------------------------------------------------------
//      Save a program to disk
//-----------------------------------------------------------------------------

void save_program(void) {
  int w = v.w / big2, h = v.h / big2;
  FILE *f;

  div_strcpy(full, sizeof(full), tipo[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);
  if ((f = fopen(full, "wb")) != NULL) {
    size_t written;
    write_line();
    written = fwrite(v.prg->buffer, 1, v.prg->file_len, f);
    fclose(f);
    if (written != (size_t)v.prg->file_len) {
      remove(full); // Delete partial file
      v_text = (char *)texts[47];
      show_dialog(err0);
      return;
    }

    div_strcpy(v.prg->path, sizeof(v.prg->path), tipo[v_type].path);
    div_strcpy(v.prg->filename, sizeof(v.prg->filename), input);

    wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
    } else {
      wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
    }
    flush_window(v_window);

    if (!strcmp(input, "help.div"))
      make_helpidx();
    if (!strcmp(input, "help.idx"))
      load_index();
  } else {
    v_text = (char *)texts[47];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
//      Determine if a string has been found
//-----------------------------------------------------------------------------

int string_found(char *p, char *q, int may_min, int completa) {
  if (completa && lower[*(q - 1)])
    return (0);
  if (may_min) {
    while (*p) {
      if (*p != *q)
        return (0);
      p++;
      q++;
    }
  } else {
    while (*p) {
      if (lower[*p]) {
        if (lower[*p] != lower[*q])
          return (0);
      } else {
        if (*p != *q)
          return (0);
      }
      p++;
      q++;
    }
  }
  if (completa && lower[*q])
    return (0);
  else
    return (1);
}

//-----------------------------------------------------------------------------
//      Find text
//-----------------------------------------------------------------------------

#define y_bt 50
char buscar[32] = {0};
int may_min = 0, completa = 0;

void find_text0(void) {
  v.type = 1;
  v.title = texts[160];
  v.w = 126;
  v.h = 14 + y_bt;
  v.paint_handler = find_text1;
  v.click_handler = find_text2;

  //  strcpy(buscar,"");

  _get(161, 4, 11, v.w - 8, (byte *)buscar, 32, 0, 0);
  _button(100, 7, y_bt, 0);
  _button(101, v.w - 8, y_bt, 2);
  _flag(163, 4, y_bt - 20, &completa);
  _flag(164, 4, y_bt - 12, &may_min);

  v_accept = 0;
}

void find_text1(void) {
  _show_items();
}

void find_text2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    end_dialog = 1;
    if (buscar[0])
      v_accept = 1;
    break;
  case 2:
    end_dialog = 1;
    break;
  }
}

void find_text(void) {
  struct tprg mi_prg;
  int encontrado = 0, n;

  if (!buscar[0])
    return;

  write_line();
  read_line();
  memcpy(&mi_prg, v.prg, sizeof(struct tprg));

  f_right();

  while ((v.prg->line < v.prg->num_lines || v.prg->column <= strlen(v.prg->l)) && !encontrado) {
    if (v.prg->column > strlen(v.prg->l)) {
      if (v.prg->line < v.prg->num_lines) {
        n = v.prg->line;
        write_line();
        advance_vptr();
        advance_lptr();
        read_line();
        f_home();
      }
    } else {
      if (string_found(buscar, &v.prg->l[v.prg->column - 1], may_min, completa))
        encontrado = 1;
      else
        f_right();
    }
  }

  if (!encontrado) { // Restore variables and show info dialog
    memcpy(v.prg, &mi_prg, sizeof(struct tprg));
    v_title = (char *)texts[347];
    v_text = (char *)texts[189];
    show_dialog(info0);
  } else {
    n = strlen(buscar);
    while (n--)
      f_right();
    n = strlen(buscar);
    while (n--)
      f_left();
  }

  v.redraw = 2;
  _completo();
  text_cursor();
}

//-----------------------------------------------------------------------------
//      Replace text
//-----------------------------------------------------------------------------

void replace0(void);
void replacements0(void);

#define y_st 69
char buscar2[32] = {0};
char sustituir[32] = {0};
int may_min2 = 0, completa2 = 0;
int num_cambios;

void replace_text0(void) {
  v.type = 1;

  v.title = texts[165];
  v.w = 126;
  v.h = 14 + y_st;
  v.paint_handler = replace_text1;
  v.click_handler = replace_text2;

  //strcpy(buscar2,"");
  //strcpy(sustituir,"");

  _get(161, 4, 11, v.w - 8, (byte *)buscar2, 32, 0, 0);
  _get(162, 4, 30, v.w - 8, (byte *)sustituir, 32, 0, 0);
  _button(100, 7, y_st, 0);
  _button(101, v.w - 8, y_st, 2);
  _flag(163, 4, y_st - 20, &completa2);
  _flag(164, 4, y_st - 12, &may_min2);

  v_accept = 0;
}

void replace_text1(void) {
  _show_items();
}

void replace_text2(void) {
  _process_items();
  switch (v.active_item) {
  case 2:
    end_dialog = 1;
    if (buscar2[0])
      v_accept = 1;
    break;
  case 3:
    end_dialog = 1;
    break;
  }
}

void replace_text(void) {
  struct tprg mi_prg;
  int encontrado, n;

  if (!buscar2[0])
    return;

  write_line();
  read_line();

  v_accept = 0;
  num_cambios = 0;
  f_right();

  do {
    memcpy(&mi_prg, v.prg, sizeof(struct tprg));
    encontrado = 0;

    while ((v.prg->line < v.prg->num_lines || v.prg->column <= strlen(v.prg->l)) && !encontrado) {
      if (v.prg->column > strlen(v.prg->l)) {
        if (v.prg->line < v.prg->num_lines) {
          write_line();
          advance_vptr();
          advance_lptr();
          read_line();
          f_home();
        }
      } else {
        if (string_found(buscar2, &v.prg->l[v.prg->column - 1], may_min2, completa2))
          encontrado = 1;
        else
          f_right();
      }
    }

    if (encontrado) {
      block_state = 2;
      kprg = v.prg;
      block_start = v.prg->lptr;
      block_col1 = v.prg->column;
      block_end = v.prg->lptr;
      block_col2 = v.prg->column + strlen(buscar2) - 1;
      n = strlen(buscar2);
      while (n--)
        f_right();
      n = strlen(buscar2);
      while (n--)
        f_left();
      if (v_accept != 3) {
        v.redraw = 2;
        _completo();
        flush_window(0);
        show_dialog(replace0);
      }
      if (v_accept & 1) {
        num_cambios++;
        f_cut_block(1);
        if (clipboard != NULL)
          free(clipboard);
        clipboard = sustituir;
        clipboard_len = strlen(sustituir);
        clipboard_lines = 0;
        f_paste_block();
        f_unmark();
        clipboard = NULL;
        while (clipboard_len--)
          f_right();
      } else if (v_accept == 2) {
        f_right();
      } else if (v_accept == 4)
        encontrado = 0;
      block_state = 0;
    }
  } while (encontrado);

  if (v_accept != 4) {
    memcpy(v.prg, &mi_prg, sizeof(struct tprg));
  } // EOF
  v.redraw = 2;
  _completo();
  text_cursor();
  show_dialog(replacements0);
}

//-----------------------------------------------------------------------------
//      Replace or not?
//-----------------------------------------------------------------------------

void sustituir1(void) {
  _show_items();
}

void replace2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break; // YES
  case 1:
    v_accept = 2;
    end_dialog = 1;
    break; // NO
  case 2:
    v_accept = 3;
    end_dialog = 1;
    break; // ALL
  case 3:
    v_accept = 4;
    end_dialog = 1;
    break; // CANCEL
  }
}

void replace0(void) {
  int x2, x3, x4;
  x2 = 7 + text_len(texts[102] + 1) + 10;
  x3 = x2 + text_len(texts[103] + 1) + 10;
  x4 = x3 + text_len(texts[124] + 1) + 10;
  v.type = 1;
  v.title = texts[190];
  v.w = x4 + text_len(texts[101] + 1) + 7;
  v.h = 29;
  v.paint_handler = sustituir1;
  v.click_handler = replace2;
  _button(102, 7, v.h - 14, 0);
  _button(103, x2, v.h - 14, 0);
  _button(124, x3, v.h - 14, 0);
  _button(101, x4, v.h - 14, 0);
  v_accept = 4;
}

//-----------------------------------------------------------------------------
//      Info dialog at the end of a replacement
//-----------------------------------------------------------------------------

char sus[128];

void replacements1(void) {
  _show_items();
  wwrite(v.ptr, v.w / big2, v.h / big2, (v.w / big2) / 2, 12, 1, (byte *)sus, c3);
}
void replacements2(void) {
  _process_items();
  if (!v.active_item)
    end_dialog = 1;
}

void replacements0(void) {
  v.type = 1;
  v.title = texts[191];
  itoa(num_cambios, sus, 10);
  div_strcat(sus, sizeof(sus), (char *)texts[192]);
  v.w = text_len(texts[191]) + 28;
  v.h = 38;
  v.paint_handler = replacements1;
  v.click_handler = replacements2;
  _button(100, v.w / 2, v.h - 14, 1);
}

void open_program_for_fernando(char *nombre, char *path) {
  char wpath[_MAX_PATH];
  struct tprg *pr;
  FILE *f;
  int n, x;
  byte *buffer, *p;

  div_strcpy(full, sizeof(full), path);
  div_strcpy(input, sizeof(input), nombre);
  div_strcpy(wpath, sizeof(wpath), path);
  wpath[strlen(wpath) - strlen(nombre)] = 0;
  if ((f = fopen(full, "rb")) != NULL) { // A file was selected
    fseek(f, 0, SEEK_END);
    n = ftell(f) + buffer_grow;
    if ((buffer = (byte *)malloc(n)) != NULL) {
      if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
        v_prg->buffer_len = n;
        div_strcpy(v_prg->filename, sizeof(v_prg->filename), input);
        div_strcpy(v_prg->path, sizeof(v_prg->path), wpath); //<<<-----------
        fseek(f, 0, SEEK_SET);
        if (fread(buffer, 1, n, f) == n - buffer_grow) {
          for (p = buffer; p < buffer + n - buffer_grow; p++)
            if (!*p) {
              n = 0;
              break;
            }

          if (n) {
            n -= buffer_grow;
            v_prg->file_len = n;
            v_prg->buffer = buffer;
            v_prg->num_lines = 1;

            x = 0;
            while (n--) {
              if (*buffer == cr || *buffer == lf) {
                if (*(buffer + 1) != ((*buffer) ^ (cr ^ lf))) {
                  memmove(buffer + 1, buffer, n + 1);
                  v_prg->file_len++;
                } else if (n)
                  n--;
                *buffer++ = cr;
                *buffer = lf;
                v_prg->num_lines++;
                x = -1;
              }

              if (*buffer == 9) {
                if (x >= 1020)
                  *buffer = ' ';
                else {
                  memmove(buffer + 3, buffer, n + 1);
                  v_prg->file_len += 3;
                  x += 3;
                  *buffer++ = ' ';
                  *buffer++ = ' ';
                  *buffer++ = ' ';
                  *buffer = ' ';
                }
              }

              buffer++;
              x++;

              if (x >= 1023) {
                memmove(buffer + 2, buffer, n + 1);
                *buffer++ = cr;
                *buffer++ = lf;
                v_prg->file_len += 2;
                v_prg->num_lines++;
                x = 0;
              }
            }

            v_prg->lptr = v_prg->buffer;
            pr = v.prg;
            v.prg = v_prg;
            read_line();
            v.prg = pr;

            new_window(program0);

          } else {
            free(v_prg);
            free(buffer);
            v_text = (char *)texts[46];
            show_dialog(err0);
          }
        } else {
          free(v_prg);
          free(buffer);
          v_text = (char *)texts[44];
          show_dialog(err0);
        }
      } else {
        free(buffer);
        v_text = (char *)texts[45];
        show_dialog(err0);
      }
    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
    }
    fclose(f);
  } else {
    v_text = (char *)texts[44];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
// Process list dialog
//-----------------------------------------------------------------------------

int lp1[512];    // Line number where processes are defined
byte *lp2[512];  // Pointers to the process lines
int lp_num;      // Number of processes in the list
int lp_ini;      // First item displayed in the window
int lp_select;   // Selected item
int lp_sort = 0; // Flag indicating whether to sort the list

void create_process_list(char *buffer, int file_len) {
  byte *p, *end, *q;
  char cwork[512], cwork2[256];
  int linea = 1, n, m;

  p = (byte *)buffer;
  end = p + file_len;
  lp_num = 0;
  lp_ini = 0;
  lp_select = 0;

  do {
    while (*p != lf && p < end) {
      p++;
    }
    p++;
    linea++;
    if (p < end) {
      while (*p == ' ' && p < end)
        p++;
      if (p + 7 < end) {
        memcpy(cwork, p, 7);
        cwork[7] = 0;
        n = 6;
        do {
          cwork[n] = lower[cwork[n]];
        } while (n--);
        if ((lower[*(p + 7)] == ' ' || lower[*(p + 7)] == 0) && !strcmp(cwork, "process")) {
          p += 7;
          while (*p == ' ' && p < end)
            p++;
          if (p < end) {
            if (lp_sort) {
              q = p;
              n = 0;
              while (*q != cr && q < end) {
                cwork[n++] = *q;
                q++;
              }
              cwork[n] = 0;
              n = 0;
              while (n < lp_num) {
                if (strcmp((char *)cwork, (char *)lp2[n]) < 0)
                  break;
                n++;
              }
              if (n < lp_num) {
                memmove(&lp1[n + 1], &lp1[n], 4 * (511 - n));
                memmove(&lp2[n + 1], &lp2[n], 4 * (511 - n));
                lp1[n] = linea;
                lp2[n] = p;
              } else {
                lp1[lp_num] = linea;
                lp2[lp_num] = p;
              }
            } else {
              lp1[lp_num] = linea;
              lp2[lp_num] = p;
            }
            if (++lp_num == 512)
              break;
          } else
            break;
        }
      } else
        break;
    } else
      break;
  } while (1);


  // If the cursor is on a process name in the program, select it

  if (buffer == (char *)window[1].prg->buffer) {
    n = window[1].prg->column - 1;
    p = (byte *)window[1].prg->l;
    if (n <= strlen((char *)p)) {
      if (!lower[p[n]] && n)
        n--;
      if (lower[p[n]]) {
        while (p)
          if (lower[p[n - 1]])
            n--;
          else
            break;
        cwork[0] = 0;
        while (lower[p[n]]) {
          cwork[strlen(cwork) + 1] = 0;
          cwork[strlen(cwork)] = lower[p[n++]];
        }
        for (n = 0; n < lp_num; n++) {
          p = (byte *)strchr((char *)lp2[n], ' ');
          if (p > (byte *)strchr((char *)lp2[n], '(') || p == NULL)
            p = (byte *)strchr((char *)lp2[n], '(');
          memcpy(cwork2, lp2[n], p - lp2[n]);
          cwork2[p - lp2[n]] = 0;
          for (m = strlen(cwork2) - 1; m >= 0; m--)
            cwork2[m] = lower[cwork2[m]];
          if (!strcmp(cwork, cwork2)) {
            lp_select = n;
            lp_ini = n;
            if (lp_ini + 15 > lp_num) {
              lp_ini = lp_num - 15;
              if (lp_ini < 0)
                lp_ini = 0;
            }
            break;
          }
        }
      }
    }
  }
}

void paint_process_list(void) {
  byte *ptr = v.ptr, *p, *end;
  char cwork[512];
  int w = v.w / big2, h = v.h / big2;
  int n, m, x;

  wbox(ptr, w, h, c1, 4, 20, 128 + 132 - 10, 121); // Process listbox bounds

  end = window[1].prg->buffer + window[1].prg->file_len;

  for (m = lp_ini; m < lp_ini + 15 && m < lp_num; m++) {
    if (m == lp_select) {
      wbox(ptr, w, h, c01, 4, 20 + (m - lp_ini) * 8, 150 + 100, 9); // Process listbox fill
      x = c4;
    } else
      x = c3;
    p = lp2[m];
    n = 0;
    while (*p != cr && p < end) {
      cwork[n++] = *p;
      p++;
    }
    cwork[n] = 0;
    wwrite_in_box(ptr, w, 153 + 100, h, 5, 21 + (m - lp_ini) * 8, 0, (byte *)cwork, x);
  }
  paint_process_segment();
}

void paint_process_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 129, n;
  float x;

  wbox(ptr, w, h, c2, 123 + 132, 28, 7, max - min + 3); // Clear the slider bar

  if (lp_num <= 1)
    n = min;
  else {
    x = (float)lp_select / (float)(lp_num - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122 + 132, n, 9, 5);
  wput(ptr, w, h, 123 + 132, n + 1, -43);
}

void process_list1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  _show_items();

  wwrite(ptr, w, h, 5, 11, 0, texts[379], c1);
  wwrite(ptr, w, h, 4, 11, 0, texts[379], c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128 + 132, 123); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122 + 132, 19, 9, 123);
  wrectangle(ptr, w, h, c0, 122 + 132, 27, 9, 123 - 16);

  wput(ptr, w, h, 123 + 132, 20, -39); // Up / down button (pressed 41,42)
  wput(ptr, w, h, 123 + 132, 174 - 40, -40);

  create_process_list((char *)window[1].prg->buffer, window[1].prg->file_len);
  paint_process_list();
}

int lp_boton;

void process_list2(void) {
  int n;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  float x;

  _process_items();

  if (scan_code == 80 && lp_select + 1 < lp_num) {
    if (lp_ini + 15 == ++lp_select)
      lp_ini++;
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 72 && lp_select) {
    if (lp_ini == lp_select--)
      lp_ini--;
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 81) {
    for (n = 0; n < 15; n++)
      if (lp_select + 1 < lp_num) {
        if (lp_ini + 15 == ++lp_select)
          lp_ini++;
      }
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 73) {
    for (n = 0; n < 15; n++)
      if (lp_select) {
        if (lp_ini == lp_select--)
          lp_ini--;
      }
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }

  if (wmouse_in(3, 21, 128 + 132 - 9, 120) && (mouse_b & 1)) {
    n = lp_ini + (wmouse_y - 21) / 8;
    if (n < lp_num) {
      if (lp_select != n) {
        lp_select = n;
        paint_process_list();
        v.redraw = 1;
      } else if (!(prev_mouse_buttons & 1)) {
        v_accept = 1;
        end_dialog = 1;
      }
    }
  }

  if (wmouse_in(122 + 132, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, 123 + 132, 20, -41);
        lp_boton = 1;
        if (lp_select) {
          if (lp_ini == lp_select--)
            lp_ini--;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 1) {
      wput(ptr, w, h, 123 + 132, 20, -39);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp_boton == 1) {
    wput(ptr, w, h, 123 + 132, 20, -39);
    lp_boton = 0;
    v.redraw = 1;
  }


  if (wmouse_in(123 + 132, 28, 7, 105)) {
    mouse_graf = 13;
    if (lp_num > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - 28) / 104.0;
      lp_select = x * (lp_num - 1);
      if (lp_select < lp_ini)
        lp_ini = lp_select;
      if (lp_select >= lp_ini + 15)
        lp_ini = lp_select - 14;
      paint_process_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122 + 132, 93 + 40, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, 123 + 132, 94 + 40, -42);
        lp_boton = 2;
        if (lp_select + 1 < lp_num) {
          if (lp_ini + 15 == ++lp_select)
            lp_ini++;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 2) {
      wput(ptr, w, h, 123 + 132, 94 + 40, -40);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp_boton == 2) {
    wput(ptr, w, h, 123 + 132, 94 + 40, -40);
    lp_boton = 0;
    v.redraw = 1;
  }

  switch (v.active_item) {
  case 0:
    if (lp_num)
      v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    create_process_list((char *)window[1].prg->buffer, window[1].prg->file_len);
    paint_process_list();
    v.redraw = 1;
    break;
  }
}

void process_list0(void) {
  v.type = 1;
  v.title = texts[380];

  v.w = 166 + 100;
  v.h = 161;
  v.paint_handler = process_list1;
  v.click_handler = process_list2;

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _flag(337, v.w - text_len(texts[337]) - 12, 11, &lp_sort);
  v_accept = 0;
}

//-----------------------------------------------------------------------------
//  Jump to the line and column where the error was detected
//-----------------------------------------------------------------------------

void goto_error(void) {
  int m, n = 0;

  for (m = 0; m < max_windows; m++) {
    if (window[m].type == 102 && window[m].state && window[m].prg != NULL) {
      n = m;
      break;
    }
  }

  if (n)
    move(0, n); // If not window[0], bring to foreground

  if (v.foreground != 1) { // If not in foreground, bring it there
    for (m = 1; m < max_windows; m++)
      if (window[m].type && window[m].foreground == 1)
        if (windows_collide(0, m)) {
          window[m].foreground = 0;
          flush_window(m);
        }
  }

  if (error_number < 10) {
    error_line = 1;
    error_col = 1;
  }

  if (error_line > v.prg->num_lines) {
    error_line = v.prg->num_lines;
    m = 1;
  } else
    m = 0;
  if (error_line < 1)
    error_line = 1;

  f_home();

  if (v.prg->line > error_line) {
    while (v.prg->line > error_line) {
      write_line();
      retreat_lptr();
      read_line();
      retreat_vptr();
    }
  } else if (v.prg->line < error_line) {
    while (v.prg->line < error_line) {
      write_line();
      advance_lptr();
      read_line();
      advance_vptr();
    }
  }

  if (v.prg->line <= v.prg->first_line)
    retreat_vptr();
  if (v.prg->line - v.prg->first_line >= v.prg->h - 1)
    advance_vptr();

  if (m)
    f_end();
  else {
    v.prg->column = error_col;
    if (v.prg->column - v.prg->first_column >= v.prg->w - 1) {
      v.prg->first_column = v.prg->column - v.prg->w + 2;
    }
    if (v.prg->column < v.prg->first_column) {
      v.prg->first_column = v.prg->column;
      if (v.prg->first_column > 1)
        v.prg->first_column--;
    }
  }

  eprg = v.prg;
  ascii = 0;
  scan_code = 0;

  v.redraw = 2;
  _completo();
  text_cursor();
}

//-----------------------------------------------------------------------------
// Program listing print option
//-----------------------------------------------------------------------------

int fp_co = 1, fp_bl = 0;

void printprogram1(void) {
  _show_items();
}

void printprogram2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    fp_co = 1;
    fp_bl = 0;
    _show_items();
    break;
  case 3:
    fp_co = 0;
    fp_bl = 1;
    _show_items();
    break;
  }
}

void printprogram0(void) {
  v.type = 1;
  v.w = 120;
  v.h = 38 + 10;
  v.title = texts[453];
  v.paint_handler = printprogram1;
  v.click_handler = printprogram2;
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _flag(454, 4, 12, &fp_co);
  _flag(455, 4, 12 + 9, &fp_bl);
  v_accept = 0;
}

void print_program(void) {
  byte *buf;
  int lon, n;

  show_dialog(printprogram0);

  if (v_accept) {
    xchg(v_window, 0);
    write_line();
    read_line();

    if (fp_bl) { // Print the selected block

      if (!block_state) {
        v_text = (char *)texts[452];
        show_dialog(err0);
        return;
      }

      f_cut_block(0);
      v.redraw = 2;

      buf = (byte *)clipboard;
      lon = clipboard_len;

    } else { // Print the complete listing

      buf = v.prg->buffer;
      lon = v.prg->file_len;
    }

    xchg(v_window, 0);

    if (lon > 0) {
      for (n = 0; n < lon; n += 32) {
        show_progress((char *)texts[437], n, lon);
        if (n + 32 <= lon)
          fwrite(buf + n, 1, 32, stdprn);
        else
          fwrite(buf + n, 1, lon - n, stdprn);
      }

      fwrite("\xd\xa\f", 1, 3, stdprn);
      show_progress((char *)texts[437], lon, lon);
    }
  }
}
