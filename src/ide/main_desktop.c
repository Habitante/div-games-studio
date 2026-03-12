/**
 * main_desktop.c — Window management, desktop rendering, and animations.
 *
 * Split from main.c. Contains:
 * - Window state management (move, minimize, maximize, close, activate)
 * - Desktop rendering (update_box, flush_window, restore_wallpaper)
 * - Window placement and collision detection
 * - Window/dialog creation (new_window) and animations (explode/implode/extrude)
 * - Screen blit (flush_copy)
 */

#include "main_internal.h"
#include "sound.h"
#include "mixer.h"

void calc2(void);

int leer_mouse = 1;

//-----------------------------------------------------------------------------
//      Declares that window v has moved from the given position
//      (... to its current position v.x/y/an/al)
//-----------------------------------------------------------------------------

void on_window_moved(int x, int y, int w, int h) {
  int n, m;

  n = v.type;
  v.type = 0;
  if (draw_mode < 100) {
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
  } else
    update_box(x, y, w, h);
  v.type = n;

  if (v.type != 1) {
    for (n = 1; n < MAX_WINDOWS; n++)
      if (window[n].type && window[n].foreground == 0)

        // If a dimmed window was previously covered ...

        if (collides_with(n, x, y, w, h)) {
          window[n].foreground = 1;

          // If it's covered by other windows (foreground or background) ...

          for (m = 1; m < MAX_WINDOWS; m++)
            if (m != n && window[m].type &&
                (window[m].foreground == 1 || (window[m].foreground != 1 && m < n)))
              if (windows_collide(n, m))
                window[n].foreground = 0;

          // If we uncovered said window, bring it to the foreground

          if (window[n].foreground == 1) {
            if (n != 1) {
              move(1, n);
            }
            flush_window(1);
          }
        }

    for (n = 1; n < MAX_WINDOWS; n++)
      if (window[n].type && window[n].foreground == 1)
        if (windows_collide(0, n)) {
          window[n].foreground = 0;
          flush_window(n);
        }
  }
  flush_window(0);
}

//-----------------------------------------------------------------------------
//      Maximize the active window (window 0)
//-----------------------------------------------------------------------------

void maximize_window(void) {
  int x, y, w, h, n, m;

  if (big) {
    wput(screen_buffer, -vga_width, vga_height, v.x, v.y, -48);
    blit_partial(v.x, v.y, 14, 14);
  } else {
    wput(screen_buffer, vga_width, vga_height, v.x, v.y, -48);
    blit_partial(v.x, v.y, 7, 7);
  }
  flush_copy();

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;

  v.x = v._x;
  v.y = v._y;
  v.w = v._w_saved;
  v.h = v._h_saved;

  m = 0;
  for (n = 1; n < MAX_WINDOWS; n++)
    if (window[n].type && window[n].foreground == 1)
      if (windows_collide(0, n))
        m++;
  if (m)
    place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);

  v._x = x;
  v._y = y;
  v._w_saved = w;
  v._h_saved = h;
  v.foreground = 1;

  do {
    read_mouse();
  } while (mouse_b & 1);

  on_window_moved(x, y, w, h);

  if (v.type >= 100) {
    activate();
    flush_window(0);
  }
}

//-----------------------------------------------------------------------------
//      Minimize the active window (window 0)
//-----------------------------------------------------------------------------

void minimize_window(void) {
  int x, y, w, h, n, m;

  wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -47);
  flush_window(0);
  if (first_run != 1)
    flush_copy();
  wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -37);
  if (v.type >= 100)
    deactivate();

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;

  v.w = (7 + 1 + text_len(v.name) + 1) * big2;
  v.h = 7 * big2;

  if (v.w != v._w_saved || v.h != v._h_saved) { // If this is the first minimize
                                                // (or the name changed ...)
    place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
  } else {
    v.x = v._x;
    v.y = v._y;
    m = 0;
    for (n = 1; n < MAX_WINDOWS; n++)
      if (window[n].type && window[n].foreground == 2)
        if (windows_collide(0, n))
          m++;
    if (m)
      place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
  }

  v._x = x;
  v._y = y;
  v._w_saved = w;
  v._h_saved = h;
  v.foreground = 2;

  do {
    read_mouse();
  } while (mouse_b & 1);

  on_window_moved(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Close the active window (window 0)
//-----------------------------------------------------------------------------

extern struct tprg *old_prg;

void close_window(void) {
  int x = 0, y = 0, w = 0, h = 0, n = 0, m = 0;

  if (v.type == 102 && window_closing == 1) {
    window_closing = 2;
    return;
  }

  if (exploding_windows) {
    v.exploding = 1;
    implode(x, y, w, h);
    v.exploding = 0;
  }

  call((void_return_type_t)v.close_handler);
  if (!quick_close) {
    if (big)
      wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
    else
      wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);
    flush_window(0);
    blit_partial(v.x, v.y, v.w, v.h);
    flush_copy();
  }

  if (v.click_handler != err2) {
    free(v.ptr);
    v.ptr = NULL;
  }

  if (v.click_handler == help2 && old_prg != NULL) {
    for (m = 1; m < MAX_WINDOWS; m++) {
      if (window[m].click_handler == program2 || window[m].click_handler == calc2) {
        if ((window[m].prg == old_prg || window[m].aux == (byte *)old_prg) &&
            window[m].foreground < 2) {
          window[m].state = 1;
          wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c_b_low, 2, 2,
               window[m].w / big2 - 20, 7);
          if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
            wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                          window[m].h / big2, 4, 2, 0, window[m].title, c1);
            wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                          window[m].h / big2, 3, 2, 0, window[m].title, c4);
          } else {
            wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                   3 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c1);
            wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                   2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c4);
          }
          flush_window(m);
          v.state = 0;
          break;
        }
      }
    }
    old_prg = NULL;
  }

  if (v.type >= 100 && v.state) { // Maps auto-deactivate on close
    for (m = 1; m < MAX_WINDOWS; m++)
      if (window[m].type == v.type && window[m].foreground < 2) {
        window[m].state = 1;
        wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c_b_low, 2, 2,
             window[m].w / big2 - 20, 7);
        if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 4, 2, 0, window[m].title, c1);
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 3, 2, 0, window[m].title, c4);
        } else {
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 3 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c1);
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c4);
        }
        flush_window(m);
        break;
      }
  }

  n = v.type;

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;
  divdelete(0);
  if (draw_mode < 100) {
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
  } else
    update_box(x, y, w, h);

  if (n == 1 || n == 7) {             // Dialogs bring hidden[] windows to foreground on close
    if (v.type == 1 || v.type == 7) { // Dialog over dialog: only show the last one
      v.foreground = 1;
      flush_window(0);
    } else if (draw_mode >= 100)
      for (n = 0; n < MAX_WINDOWS; n++)
        if (hidden[n]) {
          window[n].foreground = 1;
          flush_window(n);
        }
  } else
    for (n = 0; n < MAX_WINDOWS; n++)
      if (window[n].type && window[n].foreground == 0)
        if (collides_with(n, x, y, w, h)) {
          window[n].foreground = 1;
          for (m = 0; m < MAX_WINDOWS; m++)
            if (m != n && window[m].type &&
                (window[m].foreground == 1 || (window[m].foreground == 2 && m < n)))
              if (windows_collide(n, m))
                window[n].foreground = 0;
          if (window[n].foreground == 1) {
            if (n != 0) {
              move(0, n);
            }
            flush_window(0);
          }
        }
  do {
    read_mouse();
    poll_keyboard();
  } while ((mouse_b) || key(_ESC) || key(_T) || key(_U));

  scan_code = 0;
  ascii = 0;
  mouse_b = 0;
}

//-----------------------------------------------------------------------------
//      Move a window
//-----------------------------------------------------------------------------

void move_window(void) {
  int ix, iy, oldx, oldy, w, h, b, x, y;

  if (move_full_window) {
    move_window_complete();
    return;
  }

  mouse_graf = 2;
  w = v.w;
  h = v.h;
  oldx = v.x;
  oldy = v.y;
  ix = mouse_x - oldx;
  iy = mouse_y - oldy;
  b = big;
  big = 0;

  do {
    x = mouse_x - ix;
    y = mouse_y - iy;

    wrectangle(screen_buffer, vga_width, vga_height, c4, x, y, w, h);
    blit_partial(x, y, w, 1);
    blit_partial(x, y, 1, h);
    blit_partial(x, y + h - 1, w, 1);
    blit_partial(x + w - 1, y, 1, h);
    flush_copy();

    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(x, y, w, h);
      flush_bars(0);
      update_dialogs(x, y, w, h);
      big = 0;
    } else {
      update_box(x, y, w, 1);
      update_box(x, y, 1, h);
      update_box(x, y + h - 1, w, 1);
      update_box(x + w - 1, y, 1, h);
    }

  } while (mouse_b & 1);

  big = b;
  if (x != oldx || y != oldy) {
    v.x = x;
    v.y = y;
    on_window_moved(oldx, oldy, w, h);
    full_redraw = 1;
    flush_copy();
  }

  //---------------------------------------------------------------------------
  // Check if double-click was pressed to auto-place the window
  //---------------------------------------------------------------------------

  x = 0; // Now x indicates whether the window was auto-placed

  if (oldx == v.x && oldy == v.y) {
    if (*system_clock < double_click + 10 && *system_clock > double_click &&
        abs(double_click_x - mouse_x) < 8 && abs(double_click_y - mouse_y) < 8) {
      if (v.foreground == 2)
        place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
      else
        place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
      if (v.x == oldx && v.y == oldy) {
        v.side ^= 1;
        if (v.foreground == 2)
          place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
        else
          place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
        if (v.x == oldx && v.y == oldy)
          v.side ^= 1;
      }
      if (v.x != oldx || v.y != oldy) {
        if (exploding_windows)
          extrude(oldx, oldy, v.w, v.h, v.x, v.y, v.w, v.h);
        on_window_moved(oldx, oldy, v.w, v.h);
        x++;
      }
    } else {
      double_click = *system_clock;
      double_click_x = mouse_x;
      double_click_y = mouse_y;
    }
  }

  if (!x && v.foreground == 1)
    flush_window(0);
}

void move_window_complete(void) {
  int ix, iy, oldx, oldy;
  int x, y, w, h;

  mouse_graf = 2;
  w = v.w;
  h = v.h;
  oldx = v.x;
  oldy = v.y;
  ix = mouse_x - oldx;
  iy = mouse_y - oldy;

  if (v.foreground == 1)
    wrectangle(v.ptr, w / big2, h / big2, c4, 0, 0, w / big2, h / big2);

  if (v.foreground == 2)
    window_move_pending = 1;

  do {
    x = v.x;
    y = v.y;
    v.x = mouse_x - ix;
    v.y = mouse_y - iy;
    on_window_moved(x, y, w, h);
    flush_copy();
  } while (mouse_b & 1);

  if (window_move_pending) {
    window_move_pending = 0;
    flush_window(0);
  }

  if (v.foreground == 1)
    wrectangle(v.ptr, w / big2, h / big2, c2, 0, 0, w / big2, h / big2);

  //---------------------------------------------------------------------------
  // Check if you pressed double-click to auto deploy window
  //---------------------------------------------------------------------------

  x = 0; // Now x indicates whether the window was auto-placed

  if (oldx == v.x && oldy == v.y) {
    if (*system_clock < double_click + 10 && *system_clock > double_click &&
        abs(double_click_x - mouse_x) < 8 && abs(double_click_y - mouse_y) < 8) {
      if (v.foreground == 2)
        place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
      else
        place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
      if (v.x == oldx && v.y == oldy) {
        v.side ^= 1;
        if (v.foreground == 2)
          place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
        else
          place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
        if (v.x == oldx && v.y == oldy)
          v.side ^= 1;
      }
      if (v.x != oldx || v.y != oldy) {
        if (exploding_windows)
          extrude(oldx, oldy, v.w, v.h, v.x, v.y, v.w, v.h);
        on_window_moved(oldx, oldy, v.w, v.h);
        x++;
      }
    } else {
      double_click = *system_clock;
      double_click_x = mouse_x;
      double_click_y = mouse_y;
    }
  }

  if (!x && v.foreground == 1)
    flush_window(0);
}

//-----------------------------------------------------------------------------
//      updates a screen box (region)
//-----------------------------------------------------------------------------

void update_box(int x, int y, int w, int h) {
  int n = 0;
  byte *_ptr = NULL;
  int _x = 0, _y = 0, _an = 0, _al = 0;
  int salta_x = 0, salta_y = 0;


  byte *div_version;

#ifdef GIT_SHA1

  char divver[255];
  div_strcpy(divver, sizeof(divver), "DIVDX BUILD ");
  div_strcat(divver, sizeof(divver), GIT_SHA1);
  div_version = (byte *)divver;

#else
  div_version = texts[safe];
#endif


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

  if (y < vga_height && y + h > vga_height - 8 * big2 && x < vga_width &&
      x + w >= vga_width - (text_len(div_version) + 2) * big2) {
    wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, vga_width - 1 - x,
                  vga_height - 1 - y, 18, div_version, c0);
    wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, vga_width - 2 - x,
                  vga_height - 1 - y, 18, div_version, c2);
  }

  for (n = MAX_WINDOWS - 1; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
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
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1)
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            else
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                               salta_x);
          }

        } else {
          if (n == 0 && window_move_pending) {
            if (big) {
              big = 0;
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 16, v.y, v.w - 16, v.h);
              big = 1;
            } else
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 8, v.y, v.w - 8, v.h);
          } else {
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 9 * big2 - x, window[n].y - y, 10, window[n].name, c0);
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 8 * big2 - x, window[n].y - y, 10, window[n].name, c4);
          }

          if (x < window[n].x + 7 * big2) {
            if (big) {
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, -w, h, window[n].x - x,
                          window[n].y - y, 38);
            } else
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, window[n].x - x,
                          window[n].y - y, 38);
          }
        }
      }
  blit_partial(x, y, w, h);
}

void update_dialogs(int x, int y, int w, int h) {
  int n;
  byte *_ptr;
  int _x, _y, _an, _al;
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

  for (n = MAX_WINDOWS - 1; n >= 0; n--)
    if (window[n].type == 1)
      if (collides_with(n, x, y, w, h))
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
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
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1)
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            else
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                               salta_x);
          }
        }
}

//-----------------------------------------------------------------------------
//      Update a screen region, from a given window upward
//-----------------------------------------------------------------------------

void update_box2(int vent, int x, int y, int w, int h) {
  int n;
  byte *_ptr;
  int _x, _y, _an, _al;
  int salta_x, salta_y;

  if (window[vent].foreground == 2) {
    update_box(x, y, w, h);
    return;
  }

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

  for (n = vent; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
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
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1)
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            else
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                               salta_x);
          }

        } else {
          if (n == 0 && window_move_pending) {
            if (big) {
              big = 0;
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 16, v.y, v.w - 16, v.h);
              big = 1;
            } else
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 8, v.y, v.w - 8, v.h);
          } else {
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 9 * big2 - x, window[n].y - y, 10, window[n].name, c0);
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 8 * big2 - x, window[n].y - y, 10, window[n].name, c4);
          }

          if (x < window[n].x + 7 * big2) {
            if (big) {
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, -w, h, window[n].x - x,
                          window[n].y - y, 38);
            } else
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, window[n].x - x,
                          window[n].y - y, 38);
          }
        }
      }

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Update the background around a zoom window (zoom_win_x,zoom_win_y,zoom_win_width,zoom_win_height)
//-----------------------------------------------------------------------------

void update_background(void) {
  int y0 = 0, y1 = vga_height - 1;
  int _big = big;

  if (zoom_win_y > 2) {
    y0 = zoom_win_y - 2;
    update_box(0, 0, vga_width, zoom_win_y - 2);
  }
  if (zoom_win_y + zoom_win_height < vga_height - 2) {
    y1 = zoom_win_y + zoom_win_height + 1;
    update_box(0, zoom_win_y + zoom_win_height + 2, vga_width,
               vga_height - 2 - zoom_win_y - zoom_win_height);
  }
  if (zoom_win_x > 2) {
    update_box(0, y0, zoom_win_x - 2, y1 - y0 + 1);
  }
  if (zoom_win_x + zoom_win_width < vga_width - 2) {
    update_box(zoom_win_x + zoom_win_width + 2, y0, vga_width - 2 - zoom_win_x - zoom_win_width,
               y1 - y0 + 1);
  }

  if (_big) {
    big = 0;
    big2 = 1;
  }

  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y - 2, zoom_win_width + 4,
       1);
  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y + zoom_win_height + 1,
       zoom_win_width + 4, 1);
  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y - 1, 1,
       zoom_win_height + 2);
  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x + zoom_win_width + 1, zoom_win_y - 1, 1,
       zoom_win_height + 2);

  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y - 1, zoom_win_width + 2,
       1);
  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y + zoom_win_height,
       zoom_win_width + 2, 1);
  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y, 1, zoom_win_height);
  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x + zoom_win_width, zoom_win_y, 1,
       zoom_win_height);

  if (_big) {
    big = 1;
    big2 = 2;
  }
}

//-----------------------------------------------------------------------------
//      Resets the bacground (wallpaper) in a given area
//-----------------------------------------------------------------------------

void restore_wallpaper(int x, int y, int w, int h) {
  byte *p;
  byte *t;
  int n, _an;

  if (y < 0)
    y = 0;
  if (x < 0)
    x = 0;
  if (y + h > vga_height)
    h = vga_height - y;
  if (x + w > vga_width)
    w = vga_width - x;

  if (w > 0 && h > 0) {
    p = screen_buffer + y * vga_width + x;
    if (wallpaper != NULL) {
      t = wallpaper_map + (y % wallpaper_height) * wallpaper_width;
      _an = w;
      do {
        n = x;
        do {
          if (n >= wallpaper_width)
            n %= wallpaper_width;
          *p = *(t + n);
          p++;
          n++;
        } while (--w);
        w = _an;
        p += vga_width - w;
        t += wallpaper_width;
        if (t == wallpaper_map + wallpaper_width * wallpaper_height)
          t = wallpaper_map;
      } while (--h);
    } else {
      do {
        memset(p, c1, w);
        p += vga_width;
      } while (--h);
    }
  }
}

//-----------------------------------------------------------------------------
//    Dumps a window, according this standard or obscured or not in the foreground
//-----------------------------------------------------------------------------

void flush_window(int m) {
  int x, y, w, h, n;
  byte *_ptr;
  int _x, _y, _an, _al;
  int salta_x, salta_y;

  SDL_Rect trc;

  if (skip_window_render)
    return;

  if (partial_blits) {
    if (window[m].type == 102 && window[m].redraw == 1 && window[m].prg != NULL) {
      window[m].redraw = 0;
      update_box2(m, window[m].x + 2 * big2, window[m].y + 10 * big2, window[m].w - 20, 7 * big2);
      update_box2(m, window[m].x + 2 * big2,
                  window[m].y + 18 * big2 +
                      (window[m].prg->line - window[m].prg->first_line) * font_height,
                  window[m].w - 12, font_height);
      if (window[m].prg->prev_line != window[m].prg->line - window[m].prg->first_line)
        update_box2(m, window[m].x + 2 * big2,
                    window[m].y + 18 * big2 + window[m].prg->prev_line * font_height,
                    window[m].w - 12, font_height);
      return;
    }
    // MapperCreator2 check removed (MODE8/3D map editor deleted)
  }
  window[m].redraw = 0;

  x = window[m].x;
  y = window[m].y;
  w = window[m].w;
  h = window[m].h;

  if (window[m].foreground == 2) {
    update_box(x, y, w, h);
    return;
  }

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
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          if (_ptr == NULL)
            return;

          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
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
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1) {
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            } else {
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                               salta_x);
            }
          }
        } else {
          wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                        window[n].x + 9 * big2 - x, window[n].y - y, 10, window[n].name, c0);
          wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                        window[n].x + 8 * big2 - x, window[n].y - y, 10, window[n].name, c4);

          if (x < window[n].x + 7 * big2) {
            if (big) {
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, -w, h, window[n].x - x,
                          window[n].y - y, 38);
            } else
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, window[n].x - x,
                          window[n].y - y, 38);
          }
        }
      }

  trc.x = window[n].x;
  trc.y = window[n].y;
  trc.w = window[n].w;
  trc.h = window[n].h;

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Window placement algorithm
//      (flag=0: icon, flag=1: window)
//-----------------------------------------------------------------------------

void place_window(int flag, int *_x, int *_y, int w, int h) {
  int n, m, x, y, new_x, old_y = *_y;
  int scanes, scan[MAX_WINDOWS];

  // First build scan[] with possible Y positions (0 + window bottom edges)

  if (flag & 1) {
    *_y = vga_height;
    scan[0] = 0;
  } else {
    *_y = -1;
    scan[0] = vga_height - h;
  }
  scanes = 1;

  for (n = 1; n < MAX_WINDOWS; n++)
    if (window[n].type) {
      if (flag & 1) {
        if ((y = window[n].y + window[n].h + 1) < vga_height) {
          x = 0;
          do {
            x++;
          } while (x < scanes && y > scan[x]);
          if (x == scanes)
            scan[scanes++] = y;
          else if (y != scan[x]) {
            memmove(&scan[x + 1], &scan[x], 4 * (MAX_WINDOWS - x - 1));
            scan[x] = y;
            scanes++;
          }
        }
      } else {
        if ((y = window[n].y - h - 1) >= 0) {
          x = 0;
          do {
            x++;
          } while (x < scanes && y < scan[x]);
          if (x == scanes)
            scan[scanes++] = y;
          else if (y != scan[x]) {
            memmove(&scan[x + 1], &scan[x], 4 * (MAX_WINDOWS - x - 1));
            scan[x] = y;
            scanes++;
          }
        }
      }
    }

  // Second ... placement algorithm ...

  if (flag & 2) {
    for (n = 0; n < scanes; n++) {
      y = scan[n];
      new_x = vga_width - w;
      do {
        x = new_x;
        for (m = 1; m < MAX_WINDOWS; m++)
          if (window[m].type && collides_with(m, x - 1, y - 1, w + 2, h + 2))
            if (new_x >= window[m].x - w)
              new_x = window[m].x - w - 1;
      } while (new_x != x && new_x >= 0);
      if (new_x >= 0) {
        *_x = new_x;
        *_y = y;
        break;
      }
    }
  } else {
    for (n = 0; n < scanes; n++) {
      y = scan[n];
      new_x = 0;
      do {
        x = new_x;
        for (m = 1; m < MAX_WINDOWS; m++)
          if (window[m].type && collides_with(m, x - 1, y - 1, w + 2, h + 2))
            if (window[m].x + window[m].w >= new_x)
              new_x = window[m].x + window[m].w + 1;
      } while (new_x != x && new_x + w <= vga_width);
      if (new_x + w <= vga_width) {
        *_x = new_x;
        *_y = y;
        break;
      }
    }
  }

  // Third, if the algorithm failed, place the window at ...

  if (flag & 1) {
    if (*_y + h > vga_height)
      find_best_position(_x, _y, w, h);
  } else {
    if (*_y < 0)
      *_y = old_y;
  }
}

//-----------------------------------------------------------------------------
//    Forced placement algorithms
//-----------------------------------------------------------------------------

#define pasos_x 16
#define pasos_y 10

void find_best_position(int *_x, int *_y, int w, int h) {
  unsigned int c, colision = -1;
  int a, b, x, y;

  for (a = 0; a <= pasos_x; a++) {
    x = (float)((vga_width - w) * a) / (float)pasos_x;
    for (b = 0; b <= pasos_y; b++) {
      y = (float)((vga_height - h) * b) / (float)pasos_y;
      c = calculate_collision(x, y, w, h);
      if (c < colision) {
        colision = c;
        *_x = x;
        *_y = y;
      }
    }
  }
}

int calculate_collision(int x, int y, int w, int h) {
  int n, c, colision = 0;

  for (n = 1; n < MAX_WINDOWS; n++) {
    if (window[n].type) {
      if ((c = calculate_overlap(n, x, y, w, h))) {
        if (window[n].foreground)
          colision += vga_width * vga_height + c;
        else
          colision += vga_width * vga_height / 4 + c;
      }
    }
  }
  return (colision);
}

int calculate_overlap(int a, int x, int y, int w, int h) {
  int n, m, _an = 0, _al = 0;

  if (y < window[a].y + window[a].h && y + h > window[a].y) {
    if (x < window[a].x + window[a].w && x + w > window[a].x) {
      n = y + h;
      m = window[a].y + window[a].h;
      if (n < m)
        m = n;
      if (window[a].y > y)
        n = window[a].y;
      else
        n = y;
      _an = m - n + 1;

      n = x + w;
      m = window[a].x + window[a].w;
      if (n < m)
        m = n;
      if (window[a].x > x)
        n = window[a].x;
      else
        n = x;
      _al = m - n + 1;

      return (_an * _al);
    }
  }
  return (0);
}

//-----------------------------------------------------------------------------
//    Check if two windows collide
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

//-----------------------------------------------------------------------------
//      Dump the virtual copy of the real screen (main display)
//-----------------------------------------------------------------------------

void flush_copy(void) {
  if (leer_mouse)
    read_mouse();
  if (draw_mode < 100) {
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
    put(mouse_shift_x, mouse_shift_y, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
  } else {
    save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 0);
    put(mouse_x, mouse_y, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 1);
  }
}

void window_surface(int w, int h, byte type) {}


//-----------------------------------------------------------------------------
//      Create a new window
//-----------------------------------------------------------------------------

void new_window(void_return_type_t init_handler) {
  byte *ptr;
  int n, m, om, x, y, w, h;
  int vtipo;
  uint32_t colorkey = 0;
  v.exploding = 0;

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  if (!window[MAX_WINDOWS - 1].type) {
    if (v.type) {
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      om = prev_mouse_buttons;
      mouse_b = 0;
      prev_mouse_buttons = 0;
      call((void_return_type_t)v.click_handler);
      mouse_b = m;
      prev_mouse_buttons = om;
      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }
    }

    addwindow();

    //---------------------------------------------------------------------------
    // The following values should define the init_handler, default values:
    //---------------------------------------------------------------------------

    v.order = next_order++;
    v.type = 0;
    v.foreground = 1;
    v.name = (byte *)"?";
    v.title = (byte *)"?";
    v.paint_handler = dummy_handler;
    v.click_handler = dummy_handler;
    v.close_handler = dummy_handler;
    v.x = 0;
    v.y = 0;
    v.w = vga_width;
    v.h = vga_height;
    v._w_saved = 0;
    v._h_saved = 0;
    v.state = 0;
    v.buttons = 0;
    v.redraw = 0;
    v.items = 0;
    v.selected_item = -1;
    v.prg = NULL;
    v.aux = NULL;
    v.ptr = NULL;
    call(init_handler);

    if (big) {
      if (v.w > 0) {
        v.w = v.w * 2;
        v.h = v.h * 2;
      } else
        v.w = -v.w;
    }

    w = v.w;
    h = v.h;

    //---------------------------------------------------------------------------
    // Window placement algorithm ...
    //---------------------------------------------------------------------------

    if (first_run == 2) { // The help window (first time)
      y = x = vga_width / 2 - w / 2;
    } else if (v.type == 1 || v.type == 7) { // Dialogs are centered
      x = vga_width / 2 - w / 2;
      y = vga_height / 2 - h / 2;
    } else
      place_window(v.side * 2 + 1, &x, &y, w, h);

    v.x = x;
    v.y = y;

    //---------------------------------------------------------------------------
    // Check that if it's a map, no other map is already active
    //---------------------------------------------------------------------------

    if (v.type >= 100) {
      v.state = 1; // Activate it
      for (m = 1; m < MAX_WINDOWS; m++)
        if (window[m].type == v.type && window[m].state) {
          window[m].state = 0;
          wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c1, 2, 2,
               window[m].w / big2 - 20, 7);
          if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
            wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                          window[m].h / big2, 4, 2, 0, window[m].title, c0);
            wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                          window[m].h / big2, 3, 2, 0, window[m].title, c2);
          } else {
            wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                   2 + (window[m].w / big2 - 20) / 2, 3, 1, window[m].title, c0);
            wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                   2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c2);
          }

          if (v.type == 102 && (window[m].prg != NULL || window[m].click_handler == calc2) &&
              window[m].type == 102) { // Erase cursor
            wup(m);
            call((void_return_type_t)v.paint_handler);
            wdown(m);
          }
          vtipo = v.type;
          v.type = 0;
          flush_window(m);
          v.type = vtipo;
          break;
        }
    }

    //---------------------------------------------------------------------------
    // Check that if it's a menu, it hasn't already been created
    //---------------------------------------------------------------------------

    n = 0;
    if (v.type == 2 || v.type == 3 || v.type == 4 || v.type == 5 || v.type == 8) {
      for (m = 1; m < MAX_WINDOWS; m++)
        if (window[m].type == 2 || window[m].type == 3 || window[m].type == 4 ||
            window[m].type == 5 || window[m].type == 8)
          if (window[m].click_handler == v.click_handler)
            n = m;
    }

    if (!n)
      ptr = (byte *)malloc(w * h);
    else
      ptr = NULL;

    if (ptr != NULL) { // Window buffer, freed in close_window
      window_surface(w, h, 0);

      //---------------------------------------------------------------------------
      // Send appropriate windows to the background
      //---------------------------------------------------------------------------

      if (v.type == 1 || v.type == 7) { // Dialogs/progress bars hide all windows
        vtipo = v.type;
        v.type = 0;
        if (window[1].type == 1 || window[1].type == 7) { // Dialog over dialog
          window[1].foreground = 0;
          flush_window(1);
        } else
          for (n = 1; n < MAX_WINDOWS; n++)
            if (window[n].type && window[n].foreground == 1) {
              hidden[n - 1] = 1;
              window[n].foreground = 0;
              flush_window(n);
            } else
              hidden[n - 1] = 0;
      } else {
        vtipo = v.type;
        v.type = 0;
        for (n = 1; n < MAX_WINDOWS; n++) {
          if (window[n].type && window[n].foreground == 1) {
            if (windows_collide(0, n)) {
              if (n == 1) {
                wup(1);
                wmouse_x = -1;
                wmouse_y = -1;
                m = mouse_b;
                om = prev_mouse_buttons;
                mouse_b = 0;
                prev_mouse_buttons = 0;
                call((void_return_type_t)v.click_handler);
                mouse_b = m;
                prev_mouse_buttons = om;
                v.foreground = 0;
                flush_window(0);
                v.redraw = 0;
                wdown(1);
              } else {
                window[n].foreground = 0;
                flush_window(n);
              }
            }
          }
        }
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

      if (v.type == 1) { // Dialogs can't be minimized
        wgra(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
        if (text_len(v.title) + 3 > w - 12) {
          wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
        }
      } else if (v.type == 7) { // Progress bar
        wgra(ptr, w, h, c_b_low, 2, 2, w - 4, 7);
        if (text_len(v.title) + 3 > w - 4) {
          wwrite_in_box(ptr, w, w - 3, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 3, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 4) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 4) / 2, 2, 1, v.title, c4);
        }
      } else {
        wput(ptr, w, h, w - 17, 2, 37);
        wgra(ptr, w, h, c_b_low, 2, 2, w - 20, 7);
        if (text_len(v.title) + 3 > w - 20) {
          wwrite_in_box(ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
        }
      }

      call((void_return_type_t)v.paint_handler);

      if (big) {
        w *= 2;
        h *= 2;
      }

      if (first_run != 1) {
        do {
          read_mouse();
        } while ((mouse_b & 1) || key(_ESC));
        if (exploding_windows) {
          v.exploding = 1;
          explode(x, y, w, h);
          v.exploding = 0;
        }
        blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
        blit_partial(x, y, w, h);
      }

      //---------------------------------------------------------------------------
      // Could not open the window (no memory or duplicate menu)
      //---------------------------------------------------------------------------

    } else {
      if (n && window[n].foreground == 2) {
        divdelete(0);
        move(0, n - 1);
        maximize_window();
      } else if (n) {
        if (window[n].foreground == 0) {
          divdelete(0);
          move(0, n - 1);
          for (m = 1; m < MAX_WINDOWS; m++)
            if (window[m].type && window[m].foreground == 1)
              if (windows_collide(0, m)) {
                window[m].foreground = 0;
                flush_window(m);
              }
          v.foreground = 1;
          flush_window(0);
        } else { // When the requested menu is already in the foreground, highlight it
          divdelete(0);
          move(0, n - 1);
          wrectangle(v.ptr, v.w / big2, v.h / big2, c4, 0, 0, v.w / big2, v.h / big2);
          init_flush();
          flush_window(0);
          retrace_wait();
          flush_copy();
          wrectangle(v.ptr, v.w / big2, v.h / big2, c2, 0, 0, v.w / big2, v.h / big2);
          v.redraw = 1;
          retrace_wait();
          retrace_wait();
          retrace_wait();
          retrace_wait();
        }
      } else {
        divdelete(0);
      }
    }
  }
}

//-----------------------------------------------------------------------------
//      Explode animation for a new window
//-----------------------------------------------------------------------------

void init_flush(void);

void explode(int x, int y, int w, int h) {
  int n = 0, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  v.type = 0;
  big = 0;
  if (draw_mode < 100) {
    if (b)
      big = 1;
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
    big = 0;
  } else
    update_box(x, y, w, h);
  while (++n < 10) {
    aan = (w * n) / 10;
    aal = (h * n) / 10;
    xx = x + w / 2 - aan / 2;
    yy = y + h / 2 - aal / 2;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, 1);
    blit_partial(xx, yy, 1, aal);
    blit_partial(xx, yy + aal - 1, aan, 1);
    blit_partial(xx + aan - 1, yy, 1, aal);
    explode_num = n;
    retrace_wait();
    flush_copy();
    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(xx, yy, aan, aal);
      flush_bars(0);
      update_dialogs(xx, yy, aan, aal);
      big = 0;
    } else {
      update_box(xx, yy, aan, 1);
      update_box(xx, yy, 1, aal);
      update_box(xx, yy + aal - 1, aan, 1);
      update_box(xx + aan - 1, yy, 1, aal);
    }
  }
  v.type = tipo;
  big = b;
}

void implode(int x, int y, int w, int h) {
  int n = 9, b = big;
  int xx = 0, yy = 0, aan = 0, aal = 0;
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
    blit_partial(xx, yy, aan, 1);
    blit_partial(xx, yy, 1, aal);
    blit_partial(xx, yy + aal - 1, aan, 1);
    blit_partial(xx + aan - 1, yy, 1, aal);
    explode_num = n;
    flush_copy();
    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(xx, yy, aan, aal);
      flush_bars(0);
      update_dialogs(xx, yy, aan, aal);
      big = 0;
    } else {
      update_box(xx, yy, aan, 1);
      update_box(xx, yy, 1, aal);
      update_box(xx, yy + aal - 1, aan, 1);
      update_box(xx + aan - 1, yy, 1, aal);
    }
    retrace_wait();
  } while (--n);
  big = b;
}

void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
  int n = 9, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  v.type = 0;
  big = 0;
  if (draw_mode < 100) {
    if (b)
      big = 1;
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
    big = 0;
  } else
    update_box(x, y, w, h);
  do {
    aan = (w * n + w2 * (10 - n)) / 10;
    aal = (h * n + h2 * (10 - n)) / 10;
    xx = (x * n + x2 * (10 - n)) / 10;
    yy = (y * n + y2 * (10 - n)) / 10;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, 1);
    blit_partial(xx, yy, 1, aal);
    blit_partial(xx, yy + aal - 1, aan, 1);
    blit_partial(xx + aan - 1, yy, 1, aal);
    if (first_run != 1)
      flush_copy();

    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(xx, yy, aan, aal);
      flush_bars(0);
      update_dialogs(xx, yy, aan, aal);
      big = 0;
    } else {
      update_box(xx, yy, aan, 1);
      update_box(xx, yy, 1, aal);
      update_box(xx, yy + aal - 1, aan, 1);
      update_box(xx + aan - 1, yy, 1, aal);
    }

    if (first_run != 1)
      retrace_wait();

  } while (--n);
  big = b;
  v.type = tipo;
}

//-----------------------------------------------------------------------------
//      Deactivate and activate functions
//-----------------------------------------------------------------------------

void deactivate(void) { // Minimize: deactivate
  int m;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  if (v.state) {
    v.state = 0;
    wgra(v.ptr, w, h, c1, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c0);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c2);
    } else {
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 3, 1, v.title, c0);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c2);
    }
    for (m = 1; m < MAX_WINDOWS; m++)
      if (window[m].type == v.type && window[m].foreground < 2) {
        window[m].state = 1;
        wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c_b_low, 2, 2,
             window[m].w / big2 - 20, 7);
        if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 4, 2, 0, window[m].title, c1);
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 3, 2, 0, window[m].title, c4);
        } else {
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 3 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c1);
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c4);
        }
        flush_window(m);
        break;
      }
    if (v.type == 102)
      call((void_return_type_t)v.paint_handler); // Erase cursor
  }
}

/* Activate the front window: highlight its title bar as focused.
 * NOTE: Called from multiple places, not just maximize — see original div.cpp.
 */
void activate(void) {
  int m;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  v.state = 1;
  wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
  if (text_len(v.title) + 3 > w - 20) {
    wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
    wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
  }
  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == v.type && window[m].state) {
      window[m].state = 0;
      wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c1, 2, 2, window[m].w / big2 - 20,
           7);
      if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
        wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                      window[m].h / big2, 4, 2, 0, window[m].title, c0);
        wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                      window[m].h / big2, 3, 2, 0, window[m].title, c2);
      } else {
        wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
               2 + (window[m].w / big2 - 20) / 2, 3, 1, window[m].title, c0);
        wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
               2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c2);
      }
      if (v.type == 102 && window[m].type == 102) { // Erase cursor
        wup(m);
        call((void_return_type_t)v.paint_handler);
        wdown(m);
      }
      flush_window(m);
      break;
    }
}
