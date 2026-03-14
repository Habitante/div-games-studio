
//-----------------------------------------------------------------------------
//      Code editor — core
//-----------------------------------------------------------------------------
//  Window handlers (program0/1/2/3), editor main loop, buffer management.
//-----------------------------------------------------------------------------

#include "editor_internal.h"

//-----------------------------------------------------------------------------
//      Editor state definitions (shared via editor_internal.h)
//-----------------------------------------------------------------------------

char line_buffer[LONG_LINE + 4]; // Copy of the line being edited, for f_enter()

int cursor_mode = 0; // Active cursor type (0-insert/1-overwrite)

long block_state =
    0; //0-n/a 1-pivot(block_start,block_col1) 2-complete(block_start,block_col1,block_end,block_col2)

byte *block_start, *block_end; // Pointers to the start of the first and last lines
int block_col1, block_col2;    // Start and end columns in those lines

struct tprg *kprg; // Prg that owns the block

char *clipboard = NULL; // Pointer to clipboard (initially empty)
int clipboard_len;      // Clipboard content length in bytes
int clipboard_lines;    // Number of line breaks contained
int clipboard_type = CLIP_CHAR; // 0-character block, 1-line block

int edit_block_mode = BMODE_NONE; // 1-Volatile block on current line, 2-multi-line

int skip_full_blit = 0; // For skipping full screen blits

char color_cursor;

int forced_slider = 0;

char colin[1024]; // Buffer for line syntax coloring

int _cached_in_comment;
char *_cached_color_source = NULL;

char *c_oldline;
int old_comment_state;

struct tprg *edited;

int t_p;

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

  repaint_full();

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

  if (dragging == DRAG_DROPPING)
    v.redraw = 1;

  if (wmouse_x != -1 && v.state) {
    if ((mouse_b & MB_LEFT) && mouse_y >= v.y + 18 * big2 && mouse_x >= v.x + 2 * big2) {
      n = v.prg->first_line + (mouse_y - (v.y + 18 * big2)) / editor_font_height;
      m = v.prg->first_column + (mouse_x - (v.x + 2 * big2)) / editor_font_width;
      if (n >= v.prg->first_line && n < v.prg->first_line + v.prg->h && m >= v.prg->first_column &&
          m < v.prg->first_column + v.prg->w) {
        if (!(prev_mouse_buttons & MB_LEFT)) {
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
    if (mouse_b & MB_RIGHT) {
      f_unmark();
      v.redraw++;
    }
  }

  if (wmouse_in(w - 9, 10, 9, h - 20)) { // Vertical slider
    if (wmouse_y < 18)
      mouse_graf = CURSOR_SCROLL_UP;
    else if (wmouse_y >= h - 17)
      mouse_graf = CURSOR_SCROLL_DOWN;
    else
      mouse_graf = CURSOR_SCROLL_TRACK;
  } else if (wmouse_in(2, h - 9, w - 11, 9)) { // Horizontal slider
    if (wmouse_x < 10)
      mouse_graf = CURSOR_SCROLL_LEFT;
    else if (wmouse_x >= w - 17)
      mouse_graf = CURSOR_SCROLL_RIGHT;
    else
      mouse_graf = CURSOR_HSCROLL_TRACK;
  } else if (wmouse_in(w - 9, h - 9, 9, 9)) { // Resize
    mouse_graf = CURSOR_GRIP;
  } else if (wmouse_in(w - 17, 10, 9, 9)) { // Maximize
    mouse_graf = CURSOR_MAXIMIZE;
  }

  if (mouse_graf == CURSOR_MAXIMIZE&& (mouse_b & MB_LEFT) && wmouse_x != -1) {
    if (!(v.buttons & 1)) {
      wput(v.ptr, w, h, w - 17, 10, -57);
      v.buttons |= 1;
      v.redraw++;
    }
  } else if (v.buttons & 1) {
    v.buttons ^= 1;
    if (mouse_graf == CURSOR_MAXIMIZE&& wmouse_in(w - 17, 10, 9, 9)) {
      maximize();
    } else {
      wput(v.ptr, w, h, w - 17, 10, 56);
      v.redraw++;
    }
  }

  if (mouse_graf == CURSOR_SCROLL_UP&& (mouse_b & MB_LEFT) && wmouse_x != -1) {
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

  if (mouse_graf == CURSOR_SCROLL_DOWN&& (mouse_b & MB_LEFT) && wmouse_x != -1) {
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

  if (mouse_graf == CURSOR_SCROLL_LEFT&& (mouse_b & MB_LEFT) && wmouse_x != -1) {
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

  if (mouse_graf == CURSOR_SCROLL_RIGHT&& (mouse_b & MB_LEFT) && wmouse_x != -1) {
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

  if (mouse_graf == CURSOR_GRIP&& (mouse_b & MB_LEFT) && wmouse_x != -1)
    resize();

  if (mouse_graf == CURSOR_SCROLL_TRACK&& (mouse_b & MB_LEFT) && wmouse_x != -1) {
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

  if (mouse_graf == CURSOR_HSCROLL_TRACK&& !(prev_mouse_buttons & MB_LEFT) && (mouse_b & MB_LEFT) && wmouse_x != -1) {
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

void program3(void) {
  if (block_state && kprg == v.prg)
    block_state = BLOCK_NONE;

  free(v.prg->buffer);
  v.prg->buffer = NULL;
  free(v.prg);
  v.prg = NULL;
}

void program0(void) {
  v.type = WIN_CODE;

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

/* Code editor main loop (click handler for program editor windows).
 * Processes keyboard input for text editing, block selection, search/replace,
 * cursor movement, and scrollbar interaction. Called once per frame.
 */
void editor() {
  byte *p;
  int n;

  if (error_number != -1 && eprg == v.prg) {
    if ((scan_code <= 1 || scan_code == _F1) && ascii == 0 && mouse_b == 0) {
      repaint_partial();
      error_cursor2();
      v.redraw++;
      return;
    }
    error_number = -1;
  }

  v.prg->prev_line = v.prg->line - v.prg->first_line;

  if (edited != v.prg) { // If we were editing a different one ...
    for (n = 0; n < MAX_WINDOWS; n++)
      if (window[n].type == WIN_CODE && window[n].prg != NULL && window[n].prg == edited)
        break;
    if (n < MAX_WINDOWS) {
      wup(n);
      write_line();
      read_line();
      if (block_state == BLOCK_PIVOT)
        f_mark();
      wdown(n);
    }
  }
  edited = v.prg;

  // poll_keyboard();

  delete_text_cursor();

  //-----------------------------------------------------------------------------

  if (edit_block_mode == BMODE_CHAR) { // Single-line block edit

    if ((shift_status & MOD_SHIFT) && !(shift_status & (MOD_CTRL | MOD_ALT)))
      switch (scan_code) {
      case 0:
        break;
      case _RIGHT:
        if (block_col2 + 1 == v.prg->column)
          block_col2++;
        else if (block_col1 == v.prg->column)
          block_col1++;
        if (block_col1 > block_col2)
          f_unmark();
        f_right();
        break;
      case _LEFT:
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
      case _INS:
        f_cut_block(2); // shift+insert
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      case _DEL:
        f_cut_block(1); // shift+delete
        edit_block_mode = BMODE_NONE;
        scan_code = 0;
        break;
      case _HOME: // shift+home
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
      case _END: // shift+end
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
    else if ((shift_status & MOD_CTRL) && !(shift_status & (MOD_SHIFT | MOD_ALT))) {
      switch (scan_code) {
      case 0:
        break;
      case _X:
        f_cut_block(1); // ctrl+x
        edit_block_mode = BMODE_NONE;
        scan_code = 0;
        break;
      case _C:
        f_cut_block(0); // ctrl+c
        scan_code = 0;
        break;
      case _V:
        f_cut_block(2); // ctrl+v
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      default:
        f_unmark();
        break;
      }

      if (key(_INS)) { // ctrl+ins
        key(_INS) = 0;
        f_cut_block(0);
        scan_code = 0;
      }

    } else if (!(shift_status & (MOD_SHIFT | MOD_CTRL | MOD_ALT)) && ascii == 0)
      switch (scan_code) {
      case 0:
        break;
      case _DEL:
        f_cut_block(2); // delete
        scan_code = 0;
        edit_block_mode = BMODE_NONE;
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & MOD_SHIFT) && (shift_status & MOD_CTRL))
      switch (scan_code) {
      case _CTRL_RIGHT: // ctrl+shift+right
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
          edit_block_mode = BMODE_LINE;
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
      case _CTRL_LEFT: // ctrl+shift+left
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
          edit_block_mode = BMODE_LINE;
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

  if (edit_block_mode == BMODE_LINE) { // Multi-line block edit

    if ((shift_status & MOD_SHIFT) && !(shift_status & (MOD_CTRL | MOD_ALT)))
      switch (scan_code) {
      case 0:
        break;
      case _DOWN: // shift+down
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
      case _PGDN: // shift+page down
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
      case _PGUP: // shift+page up
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
      case _UP: // shift+up
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
      case _INS:
        f_cut_block(2); // shift+insert
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      case _DEL:
        f_cut_block(1); // shift+delete
        edit_block_mode = BMODE_NONE;
        clipboard_type = CLIP_LINE;
        scan_code = 0;
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & MOD_CTRL) && !(shift_status & (MOD_SHIFT | MOD_ALT))) {
      switch (scan_code) {
      case 0:
        break;
      case _X:
        f_cut_block(1); // ctrl+x
        edit_block_mode = BMODE_NONE;
        clipboard_type = CLIP_LINE;
        scan_code = 0;
        break;
      case _C:
        f_cut_block(0); // ctrl+c
        clipboard_type = CLIP_LINE;
        scan_code = 0;
        break;
      case _V:
        f_cut_block(2); // ctrl+v
        f_paste_block();
        f_unmark();
        scan_code = 0;
        break;
      default:
        f_unmark();
        break;
      }

      if (key(_INS)) { // ctrl+ins
        key(_INS) = 0;
        f_cut_block(0);
        clipboard_type = CLIP_LINE;
        scan_code = 0;
      }

    } else if (!(shift_status & (MOD_SHIFT | MOD_CTRL | MOD_ALT)) && ascii == 0)
      switch (scan_code) {
      case 0:
        break;
      case _DEL:
        f_cut_block(2); // delete
        scan_code = 0;
        edit_block_mode = BMODE_NONE;
        break;
      default:
        f_unmark();
        break;
      }
    else if ((shift_status & MOD_SHIFT) && (shift_status & MOD_CTRL))
      switch (scan_code) {
      case _CTRL_RIGHT: // ctrl+shift+right
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
      case _CTRL_LEFT: // ctrl+shift+left
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

  if (edit_block_mode == BMODE_NONE) { // Only if there is no active edit block

    if ((shift_status & MOD_SHIFT) && !(shift_status & (MOD_CTRL | MOD_ALT)))
      switch (scan_code) {
      case _RIGHT: // shift+right
        f_unmark();
        edit_block_mode = BMODE_CHAR;
        block_state = BLOCK_COMPLETE;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = v.prg->column;
        block_col2 = v.prg->column;
        f_right();
        break;
      case _LEFT: // shift+left
        if (v.prg->column > 1) {
          f_unmark();
          edit_block_mode = BMODE_CHAR;
          block_state = BLOCK_COMPLETE;
          kprg = v.prg;
          block_start = v.prg->lptr;
          block_end = v.prg->lptr;
          f_left();
          block_col1 = v.prg->column;
          block_col2 = v.prg->column;
        }
        break;
      case _DOWN: // shift+down
        f_unmark();
        edit_block_mode = BMODE_LINE;
        block_state = BLOCK_COMPLETE;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = 1;
        block_col2 = linelen(v.prg->lptr) + 1;
        f_down();
        if (v.prg->lptr == block_end)
          f_unmark();
        break;
      case _PGDN: // shift+page down
        f_unmark();
        edit_block_mode = BMODE_LINE;
        block_state = BLOCK_COMPLETE;
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
      case _UP: // shift+up
        if (v.prg->line > 1) {
          f_unmark();
          edit_block_mode = BMODE_LINE;
          block_state = BLOCK_COMPLETE;
          kprg = v.prg;
          f_up();
          block_start = v.prg->lptr;
          block_end = v.prg->lptr;
          block_col1 = 1;
          block_col2 = linelen(v.prg->lptr) + 1;
        }
        break;
      case _PGUP: // shift+page up
        edit_block_mode = BMODE_LINE;
        block_state = BLOCK_COMPLETE;
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
      case _TAB:
        f_untab();
        break; // shift+tab
      case _INS:
        f_paste_block(); // shift+insert
        f_unmark();
        break;
      case _DEL:
        f_cut_block(1);
        break; // shift+delete
      case _HOME: // shift+home
        f_unmark();
        edit_block_mode = BMODE_CHAR;
        block_state = BLOCK_COMPLETE;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_end = v.prg->lptr;
        block_col1 = 1;
        block_col2 = v.prg->column - 1;
        if (block_col1 > block_col2)
          f_unmark();
        f_home();
        break;
      case _END: // shift+end
        f_unmark();
        edit_block_mode = BMODE_CHAR;
        block_state = BLOCK_COMPLETE;
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
    else if ((shift_status & MOD_SHIFT) && (shift_status & MOD_CTRL))
      switch (scan_code) {
      case _CTRL_RIGHT: // ctrl+shift+right
        f_unmark();
        edit_block_mode = BMODE_CHAR;
        block_state = BLOCK_COMPLETE;
        kprg = v.prg;
        block_start = v.prg->lptr;
        block_col1 = v.prg->column;
        f_word_right2();
        block_end = v.prg->lptr;
        block_col2 = v.prg->column - 1;
        if (block_end != block_start) {
          edit_block_mode = BMODE_LINE;
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
      case _CTRL_LEFT: // ctrl+shift+left
        f_unmark();
        edit_block_mode = BMODE_CHAR;
        block_state = BLOCK_COMPLETE;
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
          edit_block_mode = BMODE_LINE;
          block_col1 = 1;
          block_start = v.prg->lptr;
          block_end = p;
          block_col2 = linelen(p) + 1;
        } else if (block_col1 > block_col2)
          f_unmark();
        break;
      }
    else if (!(shift_status & (MOD_SHIFT | MOD_CTRL | MOD_ALT)) && ascii == 0)
      switch (scan_code) {
      case _RIGHT:
        f_right();
        break; // cursor right
      case _LEFT:
        f_left();
        break; // cursor left
      case _DOWN:
        f_down();
        break; // cursor down
      case _UP:
        f_up();
        break; // cursor up
      case _HOME:
        f_home();
        break; // home
      case _END:
        f_end();
        break; // end
      case _PGDN:
        f_page_down();
        break; // page down
      case _PGUP:
        f_page_up();
        break; // page up
      case _INS:
        f_insert();
        break; // insert
      case _DEL:
        f_delete_char();
        break; // delete
      }
    else if (!(shift_status & (MOD_SHIFT | MOD_CTRL | MOD_ALT)))
      switch (scan_code) {
      case _BACKSPACE:
        f_backspace();
        break; // backspace
      case _TAB:
        f_tab();
        break; // tab
      }
    else if ((shift_status & MOD_CTRL) && !(shift_status & (MOD_SHIFT | MOD_ALT)))
      switch (scan_code) {
      case _BACKSPACE:
      case _Y:
        f_delete();
        break; // ctrl+backspace,ctrl+y
      case _CTRL_RIGHT:
      case _RIGHT:
        f_word_right();
        break; // ctrl+right
      case _CTRL_LEFT:
      case _LEFT:
        f_word_left();
        break; // ctrl+left
      case _CTRL_PGUP:
      case _PGUP:
        f_bof();
        break; // ctrl+page up
      case 118:
      case _PGDN:
        f_eof();
        break; // ctrl+page down
      case 119:
      case _HOME:
        f_bop();
        break; // ctrl+home
      case 117:
      case _END:
        f_eop();
        break; // ctrl+end
      case _X:
        f_cut_block(1);
        break; // ctrl+x
      case _C:
        f_cut_block(0);
        break; // ctrl+c
      case _V:
        f_paste_block(); // ctrl+v
        f_unmark();
        break;
      }
    else if ((shift_status & MOD_ALT) && !(shift_status & (MOD_SHIFT | MOD_CTRL)))
      switch (scan_code) {
      case _A:
        f_mark();
        break; // alt+a
      case _U:
        f_unmark();
        break; // alt+u
      case _G: // alt+g=alt+d
      case _D:
        f_cut_block(1);
        break; // alt+d
      case _C:
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

  if (mouse_b & MB_SCROLL_DOWN&& wmouse_x != -1) {
    f_scroll(3);
  }

  if (mouse_b & MB_SCROLL_UP&& wmouse_x != -1) {
    f_mscroll(3);
  }

  //-----------------------------------------------------------------------------

  if (!(shift_status & (MOD_CTRL | MOD_ALT)) && ascii) {
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
      repaint_full();
    else
      repaint_partial();
    text_cursor();
    v.redraw++;
  }
}

//-----------------------------------------------------------------------------
//      Check that buffer_len>file_len+buffer_min+block_len
//-----------------------------------------------------------------------------

void check_memory(int block_len) {
  byte *p;
  int ip;

  if (v.prg->buffer_len <= v.prg->file_len + block_len + buffer_min) {
    write_line();
    p = (byte *)realloc(v.prg->buffer, v.prg->file_len + block_len + BUFFER_INCREASE);
    if (p != NULL) {
      ip = (byte *)p - v.prg->buffer;
      v.prg->buffer += ip;
      v.prg->lptr += ip;
      v.prg->vptr += ip;
      if (kprg == v.prg && block_state) {
        block_start += ip;
        block_end += ip;
      }
      v.prg->buffer_len = v.prg->file_len + block_len + BUFFER_INCREASE;
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
