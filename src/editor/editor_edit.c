
//-----------------------------------------------------------------------------
//      Code editor — edit operations
//-----------------------------------------------------------------------------
//  Block operations, cursor movement, navigation helpers, line I/O.
//-----------------------------------------------------------------------------

#include "editor_internal.h"

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
      for (n = 0; n < MAX_WINDOWS; n++)
        if (window[n].type == WIN_CODE && window[n].prg == kprg && kprg != NULL)
          break;
      if (n < MAX_WINDOWS) {
        wup(n);
        f_unmark();
        v.redraw = 2;
        repaint_full();
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
    for (n = 0; n < MAX_WINDOWS; n++)
      if (window[n].type == WIN_CODE && window[n].prg == kprg && kprg != NULL)
        break;
    if (n < MAX_WINDOWS) {
      wup(n);
      f_unmark();
      v.redraw = 2;
      repaint_full();
      wdown(n);
      flush_window(n);
    }
  }
  block_state = 0;
  edit_block_mode = 0;
  v.redraw++;
}

void f_cut_block(int mode) {
  int n;
  t_p = 0; // Clipboard type -> chars by default
  if (block_state && kprg != v.prg) {
    for (n = 0; n < MAX_WINDOWS; n++)
      if (window[n].type == WIN_CODE && window[n].prg == kprg && kprg != NULL)
        break;
    if (n < MAX_WINDOWS) {
      wup(n);
      f_cut(mode);
      v.redraw = 2;
      repaint_full();
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
//      Cursor movement functions
//-----------------------------------------------------------------------------

void f_right(void) {
  if (v.prg->column < LONG_LINE - 1)
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
        if (n < LONG_LINE - 1)
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
  } while (((v.prg->column - 1) % tab_size) != 0 && v.prg->column < LONG_LINE - 1);
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
  if (v.prg->column < LONG_LINE) {
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

  if (v.prg->column < LONG_LINE) {
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
      if (strlen(v.prg->l) < LONG_LINE - 1)
        v.prg->l[strlen(v.prg->l) + 1] = 0;
      for (n = strlen(v.prg->l); n >= v.prg->column; n--)
        v.prg->l[n] = v.prg->l[n - 1];
      v.prg->l[n] = ascii;
      v.prg->l[LONG_LINE - 1] = 0;
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
    memcpy(line_buffer + t, v.prg->l + n, LONG_LINE - n);
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

    memcpy(v.prg->l, line_buffer, LONG_LINE);

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
        while (color_token != p_end_of_file)
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
        while (color_token != p_end_of_file)
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
//      Line I/O — read/write lines between buffer and edit line
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

void remove_spaces(void) {
  int n = strlen(v.prg->l) - 1;
  while (n >= 0)
    if (v.prg->l[n] == ' ')
      v.prg->l[n--] = 0;
    else
      n = -1;
}
