
//-----------------------------------------------------------------------------
//      Code editor — rendering
//-----------------------------------------------------------------------------
//  Syntax coloring, full/partial window painting, scrollbars, info bar,
//  cursor display, character rendering, window resize/maximize.
//-----------------------------------------------------------------------------

#include "editor_internal.h"

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
    } while (color_token != p_end_of_file);
}

//-----------------------------------------------------------------------------
//      Full editor window rendering
//-----------------------------------------------------------------------------

void repaint_full(void) {
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

  if (v.h < v._h_saved) {
    _an = v.w;
    _al = v.h;
    v.w = v._w_saved;
    v.h = v._h_saved;
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
          col1 = LONG_LINE;
        else
          col1 = block_col2;
      } else
        col1 = LONG_LINE;
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

void repaint_partial(void) {
  int line, width;
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
    repaint_full();
    return;
  }
  c_oldline = (char *)v.prg->lptr;
  old_comment_state = numrem;

  if (v.h < v._h_saved) {
    _an = v.w;
    _al = v.h;
    v.w = v._w_saved;
    v.h = v._h_saved;
  }
  w = v.w / big2;
  h = v.h / big2;

  line = v.prg->line - v.prg->first_line;

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

  old_di = di = v.ptr + (v.w * 18 + 2) * big2 + line * v.w * editor_font_height;
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
        col1 = LONG_LINE;
      else
        col1 = block_col2;
    } else
      col1 = LONG_LINE;
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
  slider = min + (v.prg->first_column * (max - min)) / (LONG_LINE - v.prg->w);
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
  return (min + (v.prg->first_column * (max - min)) / (LONG_LINE - v.prg->w));
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
//      Window resize
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

  } while (mouse_b & MB_LEFT);

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

  if (v.type == WIN_DIALOG) { // Dialogs cannot be minimized
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
          col1 = LONG_LINE;
        else
          col1 = block_col2;
      } else
        col1 = LONG_LINE;
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
