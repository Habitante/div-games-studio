// Debugger UI — dialog system, graphics primitives, widgets, mouse
#include "debugger_internal.h"

void dummy_handler(void) {}

void show_dialog(void_return_type_t init_handler) {
  byte *ptr;
  int x, y, w, h;
  int vtipo;

  if (!window[max_windows - 1].type) {
    memmove(&window[1].type, &v.type, sizeof(twindow) * (max_windows - 1));

    //---------------------------------------------------------------------------
    // The following values must be set by init_handler; defaults:
    //---------------------------------------------------------------------------

    v.type = 1;
    v.foreground = 1;
    v.title = (byte *)"?";
    v.paint_handler = dummy_handler;
    v.click_handler = dummy_handler;
    v.close_handler = dummy_handler;
    v.x = 0;
    v.y = 0;
    v.w = vga_width;
    v.h = vga_height;
    v.state = 0;
    v.redraw = 0;
    v.items = 0;
    v.selected_item = -1;

    call(init_handler);

    if (big) {
      v.w = v.w * 2;
      v.h = v.h * 2;
    }

    w = v.w;
    h = v.h;

    if (skip_flush) {
      x = profiler_x;
      y = profiler_y;
    } else {
      x = vga_width / 2 - w / 2;
      y = vga_height / 2 - h / 2;
    }

    v.x = x;
    v.y = y;

    if ((ptr = (byte *)malloc(w * h)) != NULL) { // Window buffer, freed in close_window

      //---------------------------------------------------------------------------
      // Send appropriate windows to background
      //---------------------------------------------------------------------------

      vtipo = v.type;
      v.type = 0;

      if (window[1].type == 1) { // Dialog over dialog
        window[1].foreground = 0;
        flush_window(1);
      }

      v.type = vtipo;

      //---------------------------------------------------------------------------
      // Initialize the window
      //---------------------------------------------------------------------------

      v.ptr = ptr;

      memset(ptr, c0, w * h);
      if (big) {
        w /= 2;
        h /= 2;
      }
      wrectangle(ptr, w, h, c2, 0, 0, w, h);
      wput(ptr, w, h, w - 9, 2, 35);
      if (!strcmp((char *)v.title, (char *)text[1]) || !strcmp((char *)v.title, (char *)text[2]))
        wbox(ptr, w, h, c_r_low, 2, 2, w - 12, 7);
      else
        wbox(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
      if (text_len(v.title) + 3 > w - 12) {
        wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
        wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
      } else {
        wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
        wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
      }

      call(v.paint_handler);

      if (big) {
        w *= 2;
        h *= 2;
      }

      do {
        dread_mouse();
      } while ((mouse_b & 1) || key(_ESC));

      explode(x, y, w, h);

      blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
      blit_partial(x, y, w, h);
      do {
        dread_mouse();
      } while (mouse_b & 1);
      modal_loop();

      //---------------------------------------------------------------------------
      // Could not open the dialog (out of memory)
      //---------------------------------------------------------------------------

    } else {
      memmove(&v.type, &window[1].type, sizeof(twindow) * (max_windows - 1));
      window[max_windows - 1].type = 0;
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

  wbox(v.ptr, w, h, c0, 1, 1, w - 2, 9);
  wrectangle(v.ptr, w, h, c0, 1, 1, w - 2, h - 2);
  wrectangle(v.ptr, w, h, c2, 0, 0, w, h);
  wput(v.ptr, w, h, w - 9, 2, 35);
  wbox(v.ptr, w, h, c_b_low, 2, 2, w - 12, 7);
  if (text_len(v.title) + 3 > w - 20) {
    wwrite_in_box(v.ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(v.ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(v.ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
    wwrite(v.ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
  }
}

//-----------------------------------------------------------------------------
//      Dialog box management - Debugger
//-----------------------------------------------------------------------------

void modal_loop(void) {
  int n, m, oldn = max_windows;
  int dialogo_invocado;
  int salir_del_dialogo = 0;

  end_dialog = 0;
  do {
    ascii = 0;
    scan_code = 0;
    poll_keyboard();
    dialogo_invocado = 0;

    //-------------------------------------------------------------------------
    // Find the window the mouse is over (n); n=max_windows if none
    //-------------------------------------------------------------------------

    if (mouse_in(v.x, v.y, v.x + v.w - 1, v.y + v.h - 1))
      n = 0;
    else
      n = max_windows;

    //-------------------------------------------------------------------------
    // If we were previously in a window we've now left,
    // repaint it (to clear any highlight)
    //-------------------------------------------------------------------------

    if (n == 0) // If we're on the title bar, also repaint the window
      if (!mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2))
        n--;

    if (n != oldn && oldn == 0)
      if (v.foreground == 1) {
        dialogo_invocado = 1;
        wmouse_x = -1;
        wmouse_y = -1;
        m = mouse_b;
        mouse_b = 0;
        call(v.click_handler);
        mouse_b = m;
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        salir_del_dialogo = 0;
      }
    oldn = max_windows;
    if (n < 0)
      n++;

    //-------------------------------------------------------------------------
    // Determine cursor shape
    //-------------------------------------------------------------------------

    if (n == max_windows)
      mouse_graf = 1;
    else if (mouse_in(v.x + 2 * big2, v.y + 2 * big2, v.x + v.w - 2 * big2, v.y + 9 * big2))
      if (mouse_x <= v.x + v.w - 10 * big2)
        mouse_graf = 2;
      else
        mouse_graf = 5;
    else
      mouse_graf = 1;

    //-------------------------------------------------------------------------
    // If we are inside a window's content area ...
    //-------------------------------------------------------------------------

    if (n == 0) {
      if (mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2)) {
        dialogo_invocado = 1;
        wmouse_x = mouse_x - v.x;
        wmouse_y = mouse_y - v.y;
        if (big) {
          wmouse_x /= 2;
          wmouse_y /= 2;
        }
        call(v.click_handler);
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        oldn = 0;
        salir_del_dialogo = 0;

      } else { // If we are on the window's control bar ...

        if (mouse_graf == 2 && (mouse_b & 1))
          move_window();

        if (mouse_graf == 5 && (mouse_b & 1)) {
          close_window();
          salir_del_dialogo = 1;
        }
        oldn = -1;
      }
    }

    //-------------------------------------------------------------------------
    //  Dialogs must always be invoked
    //-------------------------------------------------------------------------

    if (!dialogo_invocado && !salir_del_dialogo) {
      dialogo_invocado = 1;
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      mouse_b = 0;
      call(v.click_handler);
      mouse_b = m;
      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }
      salir_del_dialogo = 0;
    }

    if (end_dialog && !salir_del_dialogo) {
      close_window();
      salir_del_dialogo = 1;
    }

    //-------------------------------------------------------------------------
    // Keyboard handling
    //-------------------------------------------------------------------------

    if (key(_ESC) && !key(_L_CTRL)) {
      for (n = 0; n < v.items; n++)
        if (v.item[n].type == 2 && (v.item[n].state & 2))
          break;
      if (n == v.items) {
        close_window();
        salir_del_dialogo = 1;
      }
    }

    //-------------------------------------------------------------------------
    // End of the main loop
    //-------------------------------------------------------------------------

    if (!skip_flush) {
      flush_copy();
    }

  } while (!salir_del_dialogo);
  end_dialog = 0;

  get[0] = 0;

  do {
    dread_mouse();
  } while ((mouse_b & 1) || key(_ESC));
}

//----------------------------------------------------------------------------
//      Paint a show_dialog box
//----------------------------------------------------------------------------

void refresh_dialog(void) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;

  memset(ptr, c0, w * h);
  if (big) {
    w /= 2;
    h /= 2;
  }
  wrectangle(ptr, w, h, c2, 0, 0, w, h);
  wput(ptr, w, h, w - 9, 2, 35);
  if (!strcmp((char *)v.title, (char *)text[1]))
    wbox(ptr, w, h, c_r_low, 2, 2, w - 12, 7);
  else
    wbox(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
  if (text_len(v.title) + 3 > w - 12) {
    wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
    wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
  }

  call(v.paint_handler);
}

//-----------------------------------------------------------------------------
//      Close the active window (window 0)
//-----------------------------------------------------------------------------

void close_window(void) {
  int x, y, w, h;

  call(v.close_handler);
  if (big)
    wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
  else
    wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);
  flush_window(0);
  blit_partial(v.x, v.y, v.w, v.h);
  if (!skip_flush) {
    flush_copy();
  }
  free(v.ptr);

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;
  memmove(&v.type, &window[1].type, sizeof(twindow) * (max_windows - 1));
  update_box(x, y, w, h);

  if (v.type == 1) { // Dialog over dialog: only open the last one
    v.foreground = 1;
    flush_window(0);
  }

  do {
    dread_mouse();
  } while ((mouse_b & 1) || key(_ESC));

  implode(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Move a window
//-----------------------------------------------------------------------------

void move_window(void) {
  int ix, iy;
  int x, y, w, h;

  mouse_graf = 2;
  w = v.w;
  h = v.h;
  ix = mouse_x - v.x;
  iy = mouse_y - v.y;

  wrectangle(v.ptr, w / big2, h / big2, c4, 0, 0, w / big2, h / big2);

  do {
    x = v.x;
    y = v.y;
    v.x = mouse_x - ix;
    v.y = mouse_y - iy;
    v.type = 0;
    update_box(x, y, w, h);
    v.type = 1;
    flush_window(0);
    if (!skip_flush) {
      flush_copy();
    }
  } while (mouse_b & 1);

  wrectangle(v.ptr, w / big2, h / big2, c2, 0, 0, w / big2, h / big2);
  v.redraw = 1;
}

//-----------------------------------------------------------------------------
//	Blit a window, normal or darkened depending on foreground state
//-----------------------------------------------------------------------------

void flush_window(int m) {
  int x, y, w, h, n;
  byte *_ptr;
  int __x, _y, _an, _al;
  int salta_x, salta_y;

  x = window[m].x;
  y = window[m].y;
  w = window[m].w;
  h = window[m].h;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;

  for (n = m; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        _ptr = window[n].ptr;
        salta_x = 0;
        salta_y = 0;
        __x = window[n].x;
        _y = window[n].y;
        _an = window[n].w;
        _al = window[n].h;

        if (y > _y) {
          salta_y += y - _y;
          _ptr += _an * salta_y;
          _y = y;
          _al -= salta_y;
        }
        if (y + h < _y + _al) {
          salta_y += _y + _al - y - h;
          _al -= _y + _al - y - h;
        }
        if (x > __x) {
          salta_x += x - __x;
          _ptr += salta_x;
          __x = x;
          _an -= salta_x;
        }
        if (x + w < __x + _an) {
          salta_x += __x + _an - x - w;
          _an -= __x + _an - x - w;
        }

        if (_an > 0 && _al > 0) {
          if (window[n].foreground == 1)
            blit_region(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al, salta_x);
          else
            blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al,
                             salta_x);
        }
      }

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Blit a window to screen
//-----------------------------------------------------------------------------

void blit_region(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w, int h,
                 int salta) {
  byte *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  q = dest + y * dest_width + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width + salta;
  else
    resto_x = salta;
  long_x = w + salta - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += dest_width * salta_y + salta_x;
  resto_x += salta_x + long_x;
  w = long_x;
  do {
    memcpy(q, p, w);
    q += dest_width;
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Blit a window to screen (darkened)
//-----------------------------------------------------------------------------

void blit_region_dark(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w,
                      int h, int salta) {
  byte *q, *_ghost;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  q = dest + y * dest_width + x;

  if (c0)
    _ghost = ghost + 256 * (int)c0;
  else
    _ghost = ghost_inicial;


  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width + salta;
  else
    resto_x = salta;
  long_x = w + salta - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += dest_width * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;

  do {
    do {
      *q = *(_ghost + *p);
      p++;
      q++;
    } while (--w);
    q += dest_width - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Draw a filled box on screen
//-----------------------------------------------------------------------------

void wbox(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h) {
  wbox_in_box(dest, dest_width, dest_width, dest_height, c, x, y, w, h);
}

void wbox_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, byte c, int x, int y,
                 int w, int h) {
  byte *p;

  if (big) {
    dest_pitch *= 2;
    dest_width *= 2;
    dest_height *= 2;
    x *= 2;
    y *= 2;
    w *= 2;
    h *= 2;
  }

  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y + h > dest_height)
    h = dest_height - y;
  if (x + w > dest_width)
    w = dest_width - x;

  if (w > 0 && h > 0) {
    p = dest + y * dest_pitch + x;
    do {
      memset(p, c, w);
      p += dest_pitch;
    } while (--h);
  }
}


//-----------------------------------------------------------------------------
//      Draw a rectangle outline
//-----------------------------------------------------------------------------

void wrectangle(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h) {
  wbox(dest, dest_width, dest_height, c, x, y, w, 1);
  wbox(dest, dest_width, dest_height, c, x, y + h - 1, w, 1);
  wbox(dest, dest_width, dest_height, c, x, y + 1, 1, h - 2);
  wbox(dest, dest_width, dest_height, c, x + w - 1, y + 1, 1, h - 2);
}

//-----------------------------------------------------------------------------
//      Draw a graphic
//-----------------------------------------------------------------------------

void put(int x, int y, int n) {
  wput_in_box(screen_buffer, vga_width, vga_width, vga_height, x, y, n);
}

void wput(byte *dest, int dest_width, int dest_height, int x, int y, int n) {
  wput_in_box(dest, dest_width, dest_width, dest_height, x, y, n);
}

void wput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n) {
  int h, w;
  int block;
  byte *p, *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (big)
    if (n >= 32 || n < 0) {
      bwput_in_box(dest, dest_pitch, dest_width, dest_height, x, y, n);
      return;
    }

  if (n < 0) {
    n = -n;
    block = 1;
  } else
    block = 0;

  p = graf[n] + 8;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  q = dest + y * dest_pitch + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width;
  else
    resto_x = 0;
  if ((long_x = w - salta_x - resto_x) <= 0)
    return;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  if ((long_y = h - salta_y - resto_y) <= 0)
    return;

  p += w * salta_y + salta_x;
  q += dest_pitch * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;
  if (block)
    do {
      do {
        switch (*p) {
        case 0:
          *q = c0;
          break;
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
  else
    do {
      do {
        switch (*p) {
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
}

void bwput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y,
                  int n) {
  int h, w;
  int block;
  byte *p, *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (n < 0) {
    n = -n;
    block = 1;
  } else
    block = 0;

  p = graf[n] + 8;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  if (dest_width > 0) {
    dest_pitch *= 2;
    dest_width *= 2;
    dest_height *= 2;
    x *= 2;
    y *= 2;
  } else {
    dest_width = -dest_width;
    if (dest_pitch < 0)
      dest_pitch = -dest_pitch;
  }

  q = dest + y * dest_pitch + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width;
  else
    resto_x = 0;
  if ((long_x = w - salta_x - resto_x) <= 0)
    return;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  if ((long_y = h - salta_y - resto_y) <= 0)
    return;

  p += w * salta_y + salta_x;
  q += dest_pitch * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;

  if (block)
    do {
      do {
        switch (*p) {
        case 0:
          *q = c0;
          break;
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
  else
    do {
      do {
        switch (*p) {
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Text printing functions
//-----------------------------------------------------------------------------

int char_len(char c) {
  struct _car {
    byte w;
    word dir;
  } *car;
  car = (struct _car *)(text_font + 1);
  return (car[c].w);
}

int text_len(byte *ptr) {
  int w;

  struct _car {
    byte w;
    word dir;
  } *car;

  car = (struct _car *)(text_font + 1);
  w = 0;
  while (*ptr) {
    w += car[*ptr].w;
    ptr++;
  }

  if (big)
    w /= 2;

  if (w) {
    w--;
  }
  return (w);
}

int text_len2(byte *ptr) {
  int w;

  struct _car {
    byte w;
    word dir;
  } *car;

  car = (struct _car *)(text_font + 1);
  w = 0;
  while (*ptr) {
    w += car[*ptr].w;
    ptr++;
  }

  if (big)
    w /= 2;

  return (w - 1);
}

void wwrite(byte *dest, int dest_width, int dest_height, int x, int y, int alignment, byte *ptr,
            byte c) {
  wwrite_in_box(dest, dest_width, dest_width, dest_height, x, y, alignment, ptr, c);
}

void wwrite_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x_org,
                   int y_org, int alignment_org, byte *ptr, byte c) {
  int w, h, button, multi;
  int alignment = alignment_org, x = x_org, y = y_org;

  struct _car {
    byte w;
    word dir;
  } *car;

  byte *font;

  if (alignment & 0xFF00) {
    w = (alignment >> 8);
    h = 7;
    alignment &= 0xFF;
  } else
    w = 0;

  if (alignment >= 10) {
    alignment -= 10;
    multi = 1;
  } else
    multi = 0;

  if (*ptr == '\xd') {
    button = 1;
    ptr++;
  } else
    button = 0;

  car = (struct _car *)(text_font + 1);

  if (!w) {
    if (big && !multi) {
      w = text_len(ptr);
      h = 7;
    } else {
      font = ptr;
      w = 0;
      while (*font) {
        w += car[*font].w;
        font++;
      }
      h = *text_font;
      if (w)
        w--;
    }
  }

  font = text_font + 1025;

  switch (alignment) {
  case 0:
    break;
  case 1:
    x = x - (w >> 1);
    break;
  case 2:
    x = x - w + 1;
    break;
  case 3:
    y = y - (h >> 1);
    break;
  case 4:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case 5:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case 6:
    y = y - h + 1;
    break;
  case 7:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case 8:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }

  if (button) {
    if (c != c0) {
      wbox(dest, dest_pitch, dest_height, c2, x - 2, y - 2, w + 4, h + 4);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 3, y - 3, w + 6, h + 6);
      wrectangle(dest, dest_pitch, dest_height, c3, x - 2, y - 2, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c3, x - 2, y - 2, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c4, x - 2, y - 2, 1, 1);
      wrectangle(dest, dest_pitch, dest_height, c1, x - 1, y + h + 1, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c1, x + w + 1, y - 1, 1, h + 3);
    } else {
      wbox(dest, dest_pitch, dest_height, c1, x - 2, y - 2, w + 4, h + 4);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 3, y - 3, w + 6, h + 6);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 2, y - 2, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 2, y - 2, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c2, x - 1, y + h + 1, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c2, x + w + 1, y - 1, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c3, x + w + 1, y + h + 1, 1, 1);
    }
  }

  if (alignment_org & 0xFF00) {
    if (big && !multi) {
      w = text_len(ptr);
      h = 7;
    } else {
      font = ptr;
      w = 0;
      while (*font) {
        w += car[*font].w;
        font++;
      }
      h = *text_font;
      if (w)
        w--;
    }
    font = text_font + 1025;
    switch (alignment) {
    case 0:
      break;
    case 1:
      x = x_org - (w >> 1);
      break;
    case 2:
      x = x_org - w + 1;
      break;
    case 3:
      y = y_org - (h >> 1);
      break;
    case 4:
      x = x_org - (w >> 1);
      y = y_org - (h >> 1);
      break;
    case 5:
      x = x_org - w + 1;
      y = y_org - (h >> 1);
      break;
    case 6:
      y = y_org - h + 1;
      break;
    case 7:
      x = x_org - (w >> 1);
      y = y_org - h + 1;
      break;
    case 8:
      x = x_org - w + 1;
      y = y_org - h + 1;
      break;
    }
  }

  if (big && !multi) {
    dest_pitch *= 2;
    dest_width *= 2;
    dest_height *= 2;
    x *= 2;
    y *= 2;
    w *= 2;
    h *= 2;
  }

  if (y < dest_height && y + h > 0) {
    if (y >= 0 && y + h <= dest_height) { // Text fits entirely (y coord)
      while (*ptr && x + car[*ptr].w <= 0) {
        x = x + car[*ptr].w;
        ptr++;
      }
      if (*ptr && x < 0) {
        wtexc(dest, dest_pitch, dest_width, dest_height, font + car[*ptr].dir, x, y, car[*ptr].w, h,
              c);
        x = x + car[*ptr].w;
        ptr++;
      }
      while (*ptr && x + car[*ptr].w <= dest_width) {
        wtexn(dest, dest_pitch, font + car[*ptr].dir, x, y, car[*ptr].w, h, c);
        x = x + car[*ptr].w;
        ptr++;
      }
      if (*ptr && x < dest_width)
        wtexc(dest, dest_pitch, dest_width, dest_height, font + car[*ptr].dir, x, y, car[*ptr].w, h,
              c);
    } else {
      while (*ptr && x + car[*ptr].w <= 0) {
        x = x + car[*ptr].w;
        ptr++;
      }
      while (*ptr && x < dest_width) {
        wtexc(dest, dest_pitch, dest_width, dest_height, font + car[*ptr].dir, x, y, car[*ptr].w, h,
              c);
        x = x + car[*ptr].w;
        ptr++;
      }
    }
  }
}

void wtexn(byte *dest, int dest_pitch, byte *p, int x, int y, byte w, int h, byte c) {
  byte *q = dest + y * dest_pitch + x;
  int width = w;

  do {
    do {
      if (*p) {
        *q = c;
      }
      p++;
      q++;
    } while (--w);
    q += dest_pitch - (w = width);
  } while (--h);
}

void wtexc(byte *dest, int dest_pitch, int dest_width, int dest_height, byte *p, int x, int y,
           byte w, int h, byte c) {
  byte *q = dest + y * dest_pitch + x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width;
  else
    resto_x = 0;
  long_x = w - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += dest_pitch * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;
  do {
    do {
      if (*p) {
        *q = c;
      }
      p++;
      q++;
    } while (--w);
    q += dest_pitch - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Explode animation for a new window
//-----------------------------------------------------------------------------

void explode(int x, int y, int w, int h) {
  int n = 0, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  if (skip_flush)
    return;
  v.type = 0;
  big = 0;
  update_box(x, y, w, h);
  while (++n < 10) {
    aan = (w * n) / 10;
    aal = (h * n) / 10;
    xx = x + w / 2 - aan / 2;
    yy = y + h / 2 - aal / 2;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, aal);
    retrace_wait();
    flush_copy();
    update_box(xx, yy, aan, 1);
    update_box(xx, yy, 1, aal);
    update_box(xx + aan - 1, yy, 1, aal);
    update_box(xx, yy + aal - 1, aan, 1);
  }
  v.type = tipo;
  big = b;
}

void implode(int x, int y, int w, int h) {
  int n = 9, b = big;
  int xx, yy, aan, aal;
  if (skip_flush)
    return;
  big = 0;
  do {
    aan = (w * n) / 10;
    if (!aan)
      aan = 1;
    aal = (h * n) / 10;
    if (!aal)
      aal = 1;
    xx = x + w / 2 - aan / 2;
    yy = y + h / 2 - aal / 2;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, aal);
    flush_copy();
    update_box(xx, yy, aan, aal);
    retrace_wait();
  } while (--n);
  big = b;
}

void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
  int n = 9, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  if (skip_flush)
    return;
  v.type = 0;
  big = 0;
  update_box(x, y, w, h);
  do {
    aan = (w * n + w2 * (10 - n)) / 10;
    aal = (h * n + h2 * (10 - n)) / 10;
    xx = (x * n + x2 * (10 - n)) / 10;
    yy = (y * n + y2 * (10 - n)) / 10;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, aal);
    flush_copy();
    update_box(xx, yy, aan, aal);
    retrace_wait();
  } while (--n);
  big = b;
  v.type = tipo;
}

//-----------------------------------------------------------------------------
//      Update a screen region
//-----------------------------------------------------------------------------

void update_box(int x, int y, int w, int h) {
  int n;
  byte *_ptr;
  int __x, _y, _an, _al;
  int salta_x, salta_y;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;
  if (w <= 0 || h <= 0)
    return;

  restore_wallpaper(x, y, w, h);

  for (n = max_windows - 1; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        _ptr = window[n].ptr;
        salta_x = 0;
        salta_y = 0;
        __x = window[n].x;
        _y = window[n].y;
        _an = window[n].w;
        _al = window[n].h;

        if (y > _y) {
          salta_y += y - _y;
          _ptr += _an * salta_y;
          _y = y;
          _al -= salta_y;
        }
        if (y + h < _y + _al) {
          salta_y += _y + _al - y - h;
          _al -= _y + _al - y - h;
        }
        if (x > __x) {
          salta_x += x - __x;
          _ptr += salta_x;
          __x = x;
          _an -= salta_x;
        }
        if (x + w < __x + _an) {
          salta_x += __x + _an - x - w;
          _an -= __x + _an - x - w;
        }

        if (_an > 0 && _al > 0) {
          if (window[n].foreground == 1)
            blit_region(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al, salta_x);
          else
            blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al,
                             salta_x);
        }
      }

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//	Check if two windows collide
//-----------------------------------------------------------------------------

int windows_collide(int a, int b) {
  if (window[b].y < window[a].y + window[a].h && window[b].y + window[b].h > window[a].y &&
      window[b].x < window[a].x + window[a].w && window[b].x + window[b].w > window[a].x)
    return (1);
  else
    return (0);
}

int collides_with(int a, int x, int y, int w, int h) {
  if (y < window[a].y + window[a].h && y + h > window[a].y && x < window[a].x + window[a].w &&
      x + w > window[a].x)
    return (1);
  else
    return (0);
}

//----------------------------------------------------------------------------
//      Restore wallpaper/background under debugger windows
// TODO: Should also update the background when the game state changes.
//----------------------------------------------------------------------------

void restore_wallpaper(int x, int y, int w, int h) {
  byte *p;
  byte *t;
  int n, _an;

  if (y < 0) {
    y = 0;
  }
  if (x < 0) {
    x = 0;
  }
  if (y + h > vga_height)
    h = vga_height - y;
  if (x + w > vga_width)
    w = vga_width - x;

  if (w > 0 && h > 0) {
    p = screen_buffer + y * vga_width + x;
    t = screen_buffer_debug + y * vga_width;
    _an = w;
    do {
      n = x;
      do {
        *p = *(t + n);
        p++;
        n++;
      } while (--w);
      w = _an;
      p += vga_width - w;
      t += vga_width;
    } while (--h);
  }
}

//-----------------------------------------------------------------------------
//      Item definitions
//-----------------------------------------------------------------------------

void _button(byte *t, int x, int y, int c) {
  v.item[v.items].type = 1;
  v.item[v.items].state = 0;
  v.item[v.items].button.text = t;
  v.item[v.items].button.x = x;
  v.item[v.items].button.y = y;
  v.item[v.items].button.center = c;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _get(byte *t, int x, int y, int w, byte *buffer, int buffer_len, int r0, int r1) {
  v.item[v.items].type = 2;
  v.item[v.items].state = 0;
  v.item[v.items].get.text = t;
  v.item[v.items].get.x = x;
  v.item[v.items].get.y = y;
  v.item[v.items].get.w = w;
  v.item[v.items].get.buffer = buffer;
  v.item[v.items].get.buffer_len = buffer_len;
  v.item[v.items].get.r0 = r0;
  v.item[v.items].get.r1 = r1;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _flag(byte *t, int x, int y, int *value) {
  v.item[v.items].type = 3;
  v.item[v.items].state = 0;
  v.item[v.items].flag.text = t;
  v.item[v.items].flag.x = x;
  v.item[v.items].flag.y = y;
  v.item[v.items].flag.value = value;
  v.items++;
}

//-----------------------------------------------------------------------------
//      Item rendering
//-----------------------------------------------------------------------------

void _show_items(void) {
  int n = 0;
  show_items_called = 1;
  wbox(v.ptr, v.w / big2, v.h / big2, c2, 2, 10, v.w / big2 - 4, v.h / big2 - 12);
  while (n < v.items) {
    switch (abs(v.item[n].type)) {
    case 1:
      show_button(&v.item[n]);
      break;
    case 2:
      show_get(&v.item[n]);
      break;
    case 3:
      show_flag(&v.item[n]);
      break;
    }
    n++;
  }
  kbdFLAGS[28] = 0;
  show_items_called = 0;
}

void _show_items2(void) {
  int n = 0;
  show_items_called = 1;
  while (n < v.items) {
    switch (v.item[n].type) {
    case 1:
      show_button(&v.item[n]);
      break;
    case 2:
      show_get(&v.item[n]);
      break;
    case 3:
      show_flag(&v.item[n]);
      break;
    }
    n++;
  }
  kbdFLAGS[28] = 0;
  show_items_called = 0;
}

void show_button(t_item *i) {
  wwrite(v.ptr, v.w / big2, v.h / big2, i->button.x, i->button.y, i->button.center, i->button.text,
         c3);
  if (&v.item[v.selected_item] == i)
    select_button(i, 1);
}

void show_get(t_item *i) {
  wbox(v.ptr, v.w / big2, v.h / big2, c1, i->get.x, i->get.y + 8, i->get.w, 9);
  wwrite_in_box(v.ptr, v.w / big2, i->get.w - 1 + i->get.x, v.h / big2, i->get.x + 1, i->get.y + 9,
                0, i->get.buffer, c3);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->get.x + 1, i->get.y, 0, i->get.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->get.x, i->get.y, 0, i->get.text, c3);
  if (&v.item[v.selected_item] == i) {
    if (i->state & 2)
      select_get(i, 0, 0);
    select_get(i, 1, 0);
  }
}

void select_get(t_item *i, int activo, int ocultar_error) {
  int n;
  if (activo) {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c12, i->get.x - 1, i->get.y + 7, i->get.w + 2, 11);
    if (i->state & 2) {
      div_strcpy(get, sizeof(get), (char *)i->get.buffer);
      get_pos = strlen(get);
    }
    i->state &= 1;
  } else {
    if (i->state & 2) {
      if (*get) {
        if (i->get.r0 == i->get.r1)
          div_strcpy((char *)i->get.buffer, i->get.buffer_len, get);
        else {
          if (atoi(get) >= i->get.r0 && atoi(get) <= i->get.r1)
            itoa(atoi(get), (char *)i->get.buffer, 10);
          else if (!ocultar_error && !show_items_called) {
            div_snprintf(combo_error, sizeof(combo_error), "%s [%d..%d].", (char *)text[4],
                         i->get.r0, i->get.r1);
            text[3] = (byte *)combo_error;
            v_text = (char *)text[3];
            show_dialog(err0);
          }
        }
      }
    }

    i->state &= 1;

    if (!superget) {
      wbox(v.ptr, v.w / big2, v.h / big2, c1, i->get.x, i->get.y + 8, i->get.w, 9);
      wwrite_in_box(v.ptr, v.w / big2, i->get.w - 1 + i->get.x, v.h / big2, i->get.x + 1,
                    i->get.y + 9, 0, i->get.buffer, c3);
      wrectangle(v.ptr, v.w / big2, v.h / big2, c2, i->get.x - 1, i->get.y + 7, i->get.w + 2, 11);
    }

    for (n = 0; n < max_items; n++)
      if (i == &v.item[n])
        v.active_item = n;
  }
}

void show_flag(t_item *i) {
  if (*i->flag.value)
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, -59);
  else
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, 58);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 9, i->flag.y, 0, i->flag.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 8, i->flag.y, 0, i->flag.text, c3);
}

void select_button(t_item *i, int activo) {
  int x = i->button.x, y = i->button.y;
  int w, h;

  if (i->button.center & 0xFF00) {
    w = (i->button.center >> 8);
    h = 7;
  } else {
    w = text_len(i->button.text + 1);
    h = 7;
  }
  switch ((i->button.center & 0xFF)) {
  case 0:
    break;
  case 1:
    x = x - (w >> 1);
    break;
  case 2:
    x = x - w + 1;
    break;
  case 3:
    y = y - (h >> 1);
    break;
  case 4:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case 5:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case 6:
    y = y - h + 1;
    break;
  case 7:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case 8:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }
  if (v.ptr[(x - 4) * big2 - 1 + (y - 5) * big2 * v.w] == c12) {
    if (activo) {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c1, x - 4, y - 4, w + 8, h + 8);
    } else {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c12, x - 4, y - 4, w + 8, h + 8);
    }
  } else {
    if (activo) {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c12, x - 4, y - 4, w + 8, h + 8);
    } else {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c2, x - 4, y - 4, w + 8, h + 8);
    }
  }
}

//-----------------------------------------------------------------------------
//      Item processing
//-----------------------------------------------------------------------------

void _process_items(void) {
  int n = 0, estado;
  int asc = 0, kesc = 0, est;

  v.active_item = -1;

  if (v.selected_item != -1) {
    if (!v.state && v.type == 102) {
      asc = ascii;
      kesc = kbdFLAGS[28];
      ascii = 0;
      kbdFLAGS[28] = 0;
    } else {
      if (ascii == 9) {
        ascii = 0;
        _select_new_item(v.selected_item + 1);
      }
      if (ascii == 0x1b) { // && (v.item[v.selected_item].state&2)) {
        if (v.item[v.selected_item].type == 2) {
          asc = ascii;
          kesc = kbdFLAGS[28];
          est = v.item[v.selected_item].state;
          ascii = 0;
          if (superget)
            div_strcpy((char *)v.item[v.selected_item].get.buffer,
                       v.item[v.selected_item].get.buffer_len, "");
          div_strcpy(get, sizeof(get), (char *)v.item[v.selected_item].get.buffer);
          get_pos = strlen(get);
          select_get(&v.item[v.selected_item], 0, 1);
          select_get(&v.item[v.selected_item], 1, 1);
          if (est == v.item[v.selected_item].state) {
            ascii = asc;
            kbdFLAGS[28] = kesc;
          } else {
            v.redraw = 1;
            key(_ESC) = 0;
          }
        }
      }
    }
  }

  while (n < v.items) {
    switch (v.item[n].type) {
    case 1:
      estado = button_status(n);
      if (estado != v.item[n].state)
        process_button(n, estado);
      break;
    case 2:
      estado = get_status(n);
      if (estado != v.item[n].state || estado >= 2)
        process_get(n, estado);
      break;
    case 3:
      estado = flag_status(n);
      if (estado != v.item[n].state)
        process_flag(n, estado);
      break;
    }
    n++;
  }

  if (v.selected_item != -1) {
    if (!v.state && v.type == 102) {
      ascii = asc;
      kbdFLAGS[28] = kesc;
    }
  }
}

void _select_new_item(int i) {
  if (v.selected_item == i)
    return;
  switch (abs(v.item[v.selected_item].type)) {
  case 1:
    select_button(&v.item[v.selected_item], 0);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 0, 0);
    break;
  }
  i--;
  do {
    i++;
    if (i >= v.items)
      i = 0;
  } while (v.item[i].type >= 3 || v.item[i].type < 1);

  switch (v.item[v.selected_item = i].type) {
  case 1:
    select_button(&v.item[v.selected_item], 1);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 1, 0);
    break;
  }
  v.redraw = 1;
}

void _reselect_item(void) {
  switch (v.item[v.selected_item].type) {
  case 1:
    select_button(&v.item[v.selected_item], 0);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 0, 0);
    break;
  }
  switch (v.item[v.selected_item].type) {
  case 1:
    select_button(&v.item[v.selected_item], 1);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 1, 0);
    break;
  }
  v.redraw = 1;
}

void process_button(int n, int e) {
  if (v.item[n].state == 3 && e != 3) {
    v.active_item = n;
    kbdFLAGS[28] = 0;
    ascii = 0;
  }
  switch (e) {
  case 0:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c3);
    break;
  case 1:
    if (v.item[n].state == 2)
      v.active_item = n;
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c4);
    break;
    break;
  case 2:
    _select_new_item(n);
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c0);
    break;
  case 3:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c0);
    break;
  }
  v.item[n].state = e;
  v.redraw = 1;
}

int get_status(int n) {
  int x = v.item[n].state;
  if (strcmp((char *)v.item[n].get.text, "")) {
    if (wmouse_in(v.item[n].get.x, v.item[n].get.y, // bit 0 "hilite"
                  v.item[n].get.w, 18))
      x |= 1;
    else
      x &= 2;
  } else {
    if (wmouse_in(v.item[n].get.x, v.item[n].get.y + 8, // bit 0 "hilite"
                  v.item[n].get.w, 10))
      x |= 1;
    else
      x &= 2;
  }
  if ((x & 1) && (mouse_b & 1)) {
    if (!(prev_mouse_buttons & 1) && (x & 2))
      x |= 4;
    x |= 2;
  }
  if ((ascii && (ascii != 0x1b) && v.selected_item == n)) { //||superget) {
    if (!(x & 2)) {
      if (ascii == 13)
        ascii = 0;
      else
        x |= 4;
    }
    x |= 2;
  }
  if (superget)
    x = 3; // x&=3;
  return (x);
}

void process_get(int n, int e) {
  int old_e;

  old_e = v.item[n].state;
  v.item[n].state = e;
  if (!(old_e & 2) && (e & 2)) {
    _select_new_item(n);
    div_strcpy(get, sizeof(get), (char *)v.item[n].get.buffer);
    get_pos = strlen(get);
  }

  if (e & 4) {
    *get = 0;
  }
  e &= 3;

  switch (v.item[n].state = e) {
  case 2:
    get_input(n);
    break;
  case 3:
    get_input(n);
    break;
  }

  switch (v.item[n].state) {
  case 2:
  case 0:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].get.x, v.item[n].get.y, 0, v.item[n].get.text,
           c3);
    break;
  case 3:
  case 1:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].get.x, v.item[n].get.y, 0, v.item[n].get.text,
           c4);
    break;
  }

  if (old_e != v.item[n].state)
    v.redraw = 1;
}

int flag_status(int n) {
  int x = 0;
  if (wmouse_in(v.item[n].flag.x, v.item[n].flag.y, text_len(v.item[n].flag.text) + 10, 8))
    x = 1;
  if (x && (mouse_b & 1))
    x = 2;
  return (x);
}

void process_flag(int n, int e) {
  if (v.item[n].state == 3 && e != 3)
    v.active_item = n;
  switch (e) {
  case 0:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x + 8, v.item[n].flag.y, 0,
           v.item[n].flag.text, c3);
    break;
  case 1:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x + 8, v.item[n].flag.y, 0,
           v.item[n].flag.text, c4);
    break;
  case 2:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x + 8, v.item[n].flag.y, 0,
           v.item[n].flag.text, c4);
    if (v.item[n].state == 1) {
      v.active_item = n;
      if ((*v.item[n].flag.value = !*v.item[n].flag.value))
        wput(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x, v.item[n].flag.y, -59);
      else
        wput(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x, v.item[n].flag.y, 58);
    }
    break;
  }
  v.item[n].state = e;
  v.redraw = 1;
}

//-----------------------------------------------------------------------------
//      Input routine
//-----------------------------------------------------------------------------

void get_input(int n) {
  char cwork[512];
  int x, l, scroll;

  if (!*get)
    get_pos = 0;

  switch (ascii) {
  case 1:
  case 0x1a:
  case 0x1b:
    break;
  case 8:
    if (get_pos) {
      memmove(&get[get_pos - 1], &get[get_pos], strlen(&get[get_pos]) + 1);
      get_pos--;
    }
    if (!*get && superget)
      div_strcpy((char *)v.item[v.selected_item].get.buffer, v.item[v.selected_item].get.buffer_len,
                 "");
    v.redraw = 1;
    break;
  case 13:
    ascii = 0;
    kbdFLAGS[28] = 0;
    _select_new_item(n + 1);
    return;
  default:
    if (!(shift_status & 15) && ascii == 0) {
      l = v.redraw;
      v.redraw = 1;
      switch (scan_code) {
      case 77:
        get_pos++;
        break; // cursor right
      case 75:
        get_pos--;
        break; // cursor left
      case 71:
        get_pos = 0;
        break; // home
      case 79:
        get_pos = strlen(get);
        break; // end
      case 83:
        get[strlen(get) + 1] = 0;
        memmove(&get[get_pos], &get[get_pos + 1], strlen(&get[get_pos + 1]) + 1);
        if (!*get && superget)
          div_strcpy((char *)v.item[v.selected_item].get.buffer,
                     v.item[v.selected_item].get.buffer_len, "");
        break;
      default:
        v.redraw = l;
        break;
      }
    } else if (ascii && char_len(ascii) > 1 && (x = strlen(get)) < v.item[n].get.buffer_len - 1) {
      div_strcpy(cwork, sizeof(cwork), get);
      cwork[get_pos] = ascii;
      cwork[get_pos + 1] = 0;
      div_strcat(cwork, sizeof(cwork), get + get_pos);
      div_strcpy(get, sizeof(get), cwork);
      get_pos++;
      v.redraw = 1;
      get[x++] = ascii;
      get[x] = 0;
      v.redraw = 1;
    }
    break;
  }

  if (v.redraw || get_cursor != (system_clock & 4)) {
    v.redraw = 1;

    if (get_pos < 0)
      get_pos = 0;
    else if (get_pos > strlen(get))
      get_pos = strlen(get);

    div_strcpy(cwork, sizeof(cwork), get);
    cwork[get_pos] = 0;
    l = text_len2((byte *)cwork);
    div_strcat(cwork, sizeof(cwork), " ");
    div_strcat(cwork, sizeof(cwork), get + get_pos);

    if (l > v.item[n].get.w - 8) {
      scroll = l - (v.item[n].get.w - 8);
    } else
      scroll = 0;

    wbox(v.ptr, v.w / big2, v.h / big2, c0, v.item[n].get.x, v.item[n].get.y + 8, v.item[n].get.w,
         9);
    wwrite_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2, v.h / big2,
                  0 - scroll, v.item[n].get.y + 9, 0, (byte *)cwork, c4);

    if (system_clock & 4) {
      x = l + 1;
      wbox_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2, v.h / big2,
                  c3, x - scroll, v.item[n].get.y + 9, 2, 7);
    }
  }
  get_cursor = (system_clock & 4);
}

int button_status(int n) {
  int x = v.item[n].button.x, y = v.item[n].button.y;
  int w, h, e = 0;

  if (v.item[n].button.center & 0xFF00) {
    w = (v.item[n].button.center >> 8);
    h = 7;
  } else {
    w = text_len(v.item[n].button.text + 1);
    h = 7;
  }
  switch ((v.item[n].button.center & 0xFF)) {
  case 0:
    break;
  case 1:
    x = x - (w >> 1);
    break;
  case 2:
    x = x - w + 1;
    break;
  case 3:
    y = y - (h >> 1);
    break;
  case 4:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case 5:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case 6:
    y = y - h + 1;
    break;
  case 7:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case 8:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }
  if (wmouse_in(x - 3, y - 3, w + 6, h + 6))
    e = 1;
  if (e && (mouse_b & 1))
    e = 2;
  if (v.selected_item == n && kbdFLAGS[28])
    e = 3;
  return (e);
}

//----------------------------------------------------------------------------
//      Read mouse adapted for the interpreter
//----------------------------------------------------------------------------
void readmouse(void);

void dread_mouse(void) {
  poll_keyboard();
  int n = 0;

  mouse_x = (int)mouse->x;
  mouse_y = (int)mouse->y;

  if (mouse_x < 0) {
    mouse_x = 0;
    n++;
  } else if (mouse_x >= vga_width) {
    mouse_x = vga_width - 1;
    n++;
  }
  if (mouse_y < 0) {
    mouse_y = 0;
    n++;
  } else if (mouse_y >= vga_height) {
    mouse_y = vga_height - 1;
    n++;
  }

  if (n)
    set_mouse(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------
//      Blit the virtual screen copy to the real screen (main display)
//-----------------------------------------------------------------------------

void flush_copy(void) {
  dread_mouse();
  save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 0);
  put(mouse_x, mouse_y, mouse_graf);
  blit_screen(screen_buffer);
  save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 1);

  // **************************************************

  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 16;
  fade_wait();

  // **************************************************
}

//-----------------------------------------------------------------------------
//      Save (flag=0) or restore (flag=1) a virtual screen region
//-----------------------------------------------------------------------------

void save_mouse_bg(byte *p, int x, int y, int n, int flag) {
  byte *q;
  int w, h;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  if (x >= vga_width || y >= vga_height || x + w <= 0 || y + h <= 0)
    return;

  blit_partial(x, y, w, h);

  q = screen_buffer + y * vga_width + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > vga_width)
    resto_x = x + w - vga_width;
  else
    resto_x = 0;
  long_x = w - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > vga_height)
    resto_y = y + h - vga_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += vga_width * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;
  do {
    do {
      if (flag)
        *q = *p;
      else
        *p = *q;
      p++;
      q++;
    } while (--w);
    q += vga_width - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Check if the mouse is inside a box
//-----------------------------------------------------------------------------

int mouse_in(int x, int y, int x2, int y2) {
  return (mouse_x >= x && mouse_x <= x2 && mouse_y >= y && mouse_y <= y2);
}

int wmouse_in(int x, int y, int w, int h) {
  return (wmouse_x >= x && wmouse_x <= x + w - 1 && wmouse_y >= y && wmouse_y <= y + h - 1);
}
