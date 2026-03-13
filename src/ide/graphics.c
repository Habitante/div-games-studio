
///////////////////////////////////////////////////////////////////////////////
//      Module that contains the basic (graphics) primitives
///////////////////////////////////////////////////////////////////////////////

#include "global.h"

void draw_help(int); // Drawing support


void draw_selection_box(int _x0, int _y0, int _x1, int _y1);
void draw_selection_mask(void);
void fill_inter(int w, int h);
void blit_interpolated(int w);
void fill_scan(word x, word y);
void fill_draw(void);
void copy_block(byte *d, byte *s, int w, int h);
void xchg_block(byte *d, byte *s, int w, int h);


///////////////////////////////////////////////////////////////////////////////
//      Global module variables
///////////////////////////////////////////////////////////////////////////////

word *m0, *m1;
word w_original;

extern int point_index;

///////////////////////////////////////////////////////////////////////////////
//      Functions to print text
///////////////////////////////////////////////////////////////////////////////

void writetxt(int x, int y, int alignment, byte *ptr) {
  wwrite(screen_buffer, vga_width / big2, vga_height, x, y, alignment, ptr, text_color);
}

///////////////////////////////////////////////////////////////////////////////
//      Draw a box
///////////////////////////////////////////////////////////////////////////////

void box(byte c, int x0, int y0, int x1, int y1) {
  wbox(screen_buffer, vga_width, vga_height, c, x0, y0, x1 - x0 + 1, y1 - y0 + 1);
}

///////////////////////////////////////////////////////////////////////////////
//      Draw a rectangle
///////////////////////////////////////////////////////////////////////////////

void rectangle(byte c, int x, int y, int w, int h) {
  wrectangle(screen_buffer, vga_width, vga_height, c, x, y, w, h);
}

///////////////////////////////////////////////////////////////////////////////
//      Save ( flag = 0 ) or retrieve ( flag = 1) of a virtual box
///////////////////////////////////////////////////////////////////////////////

void save_mouse_bg(byte *p, int x, int y, int n, int flag) {
  byte *q;
  int w, h;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));


  if ((x > vga_width) | (y > vga_height))
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

///////////////////////////////////////////////////////////////////////////////
//      Put a graphic
///////////////////////////////////////////////////////////////////////////////

void put(int x, int y, int n) {
  wput_in_box(screen_buffer, vga_width, vga_width, vga_height, x, y, n);
}

void put_bw(int x, int y, int n) { // Puts a contrasting graphic (mouse edition )
  int h, w;
  byte *p, *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  p = graf[n] + 8;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  if (x >= vga_width || y >= vga_height || x + w <= 0 || y + h <= 0)
    return;

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

  h = (int)c0 * 3;
  salta_x = dac[h] + dac[h + 1] + dac[h + 2];
  h = (int)c4 * 3;
  salta_x += dac[h] + dac[h + 1] + dac[h + 2];
  salta_x /= 2;

  do {
    do {
      if (*p) {
        h = (int)(*q) * 3;
        if (dac[h] + dac[h + 1] + dac[h + 2] > salta_x)
          *q = c0;
        else
          *q = c4;
      }
      p++;
      q++;
    } while (--w);
    q += vga_width - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

///////////////////////////////////////////////////////////////////////////////
//      Swap two memory blocks ( without requiring a third party)
///////////////////////////////////////////////////////////////////////////////

void memxchg(byte *d, byte *s, int n) {
  int m, x;

  if ((m = n / 4))
    do {
      x = *(int *)s;
      *(int *)s = *(int *)d;
      *(int *)d = x;
      d += 4;
      s += 4;
    } while (--m);

  if (n &= 3)
    do {
      x = *s;
      *s = *d;
      *d = x;
      d++;
      s++;
    } while (--n);
}

///////////////////////////////////////////////////////////////////////////////
//      Main routine to restore the background of an editing box
///////////////////////////////////////////////////////////////////////////////

static int zoom_region = 0;
int zoom_w, zoom_h;
byte *zoom_p, *zoom_q;

void draw_edit_background(int x, int y, int w, int h) {
  int _x0, _y0, _x1, _y1;
  int _zoom_x, _zoom_y;
  int _zoom_win_x, _zoom_win_y, _zoom_win_width, _zoom_win_height;
  int _big = big;

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

  blit_partial(x, y, w, h);

  // TODO: Improve update_box() for better window background repaint

  if (zoom_win_x || zoom_win_y)
    if (x < zoom_win_x || y < zoom_win_y || x + w > zoom_win_x + zoom_win_width ||
        y + h > zoom_win_y + zoom_win_height)
      update_box(x, y, w, h);

  if (_big) {
    big = 0;
    big2 = 1;
  }

  if (zoom_win_y) {
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c2, zoom_win_x - 2 - x,
                zoom_win_y - 2 - y, zoom_win_width + 4, 1);
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c0, zoom_win_x - 1 - x,
                zoom_win_y - 1 - y, zoom_win_width + 2, 1);
  }
  // Only draw bottom/right borders when the bitmap genuinely fits (centered),
  // not for the truncation remainder when the bitmap overflows the screen.
  if (zoom_win_y + zoom_win_height < vga_height && (map_height << zoom) < vga_height) {
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c2, zoom_win_x - 2 - x,
                zoom_win_y + zoom_win_height + 1 - y, zoom_win_width + 4, 1);
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c0, zoom_win_x - 1 - x,
                zoom_win_y + zoom_win_height - y, zoom_win_width + 2, 1);
  }

  if (zoom_win_x) {
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c2, zoom_win_x - 2 - x,
                zoom_win_y - 1 - y, 1, zoom_win_height + 2);
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c0, zoom_win_x - 1 - x,
                zoom_win_y - y, 1, zoom_win_height);
  }
  if (zoom_win_x + zoom_win_width < vga_width && (map_width << zoom) < vga_width) {
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c2,
                zoom_win_x + zoom_win_width + 1 - x, zoom_win_y - 1 - y, 1, zoom_win_height + 2);
    wbox_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, c0,
                zoom_win_x + zoom_win_width - x, zoom_win_y - y, 1, zoom_win_height);
  }

  if (_big) {
    big = 1;
    big2 = 2;
  }

  // Call zoom_map to update the corresponding box

  // 1 - Intersection between zoom and window in the framebuffer

  _x0 = (x > zoom_win_x) ? x : zoom_win_x;
  _y0 = (y > zoom_win_y) ? y : zoom_win_y;
  _x1 = (x + w < zoom_win_x + zoom_win_width) ? x + w - 1 : zoom_win_x + zoom_win_width - 1;
  _y1 = (y + h < zoom_win_y + zoom_win_height) ? y + h - 1 : zoom_win_y + zoom_win_height - 1;

  if (_x1 < _x0 || _y1 < _y0)
    return;

  // 2 - Calculate the first and last pixel of the map

  _x0 = ((_x0 - zoom_win_x) >> zoom) + zoom_x;
  _y0 = ((_y0 - zoom_win_y) >> zoom) + zoom_y;
  _x1 = ((_x1 - zoom_win_x) >> zoom) + zoom_x;
  _y1 = ((_y1 - zoom_win_y) >> zoom) + zoom_y;

  zoom_p = map + _y0 * map_width + _x0;
  zoom_q = screen_buffer + (zoom_win_y + ((_y0 - zoom_y) << zoom)) * vga_width + zoom_win_x +
           ((_x0 - zoom_x) << zoom);
  zoom_w = _x1 - _x0 + 1;
  zoom_h = _y1 - _y0 + 1;

  _zoom_x = zoom_x;
  _zoom_y = zoom_y;
  _zoom_win_x = zoom_win_x;
  _zoom_win_y = zoom_win_y;
  _zoom_win_width = zoom_win_width;
  _zoom_win_height = zoom_win_height;
  zoom_win_x = zoom_win_x + ((_x0 - zoom_x) << zoom);
  zoom_win_y = zoom_win_y + ((_y0 - zoom_y) << zoom);
  zoom_x = _x0;
  zoom_y = _y0;
  zoom_win_width = (_x1 - _x0 + 1) << zoom;
  zoom_win_height = (_y1 - _y0 + 1) << zoom;
  blit_partial(zoom_win_x, zoom_win_y, zoom_win_width, zoom_win_height);
  zoom_region = 1;
  zoom_map();
  zoom_region = 0;
  zoom_x = _zoom_x;
  zoom_y = _zoom_y;
  zoom_win_x = _zoom_win_x;
  zoom_win_y = _zoom_win_y;
  zoom_win_width = _zoom_win_width;
  zoom_win_height = _zoom_win_height;
}

//-----------------------------------------------------------------------------
//      Zoom, blit map(map_width*map_height) to framebuffer according to zoom,zoom_x/y
//-----------------------------------------------------------------------------

int w, h;

void zoom_map(void) {
  int32_t n, m, c;
  byte *p, *q;
  int _big = big;

  if (!zoom_region) {
    p = map + zoom_y * map_width + zoom_x;
    q = screen_buffer;

    if ((map_width << zoom) < vga_width) { // This chunk is copied in select_zoom()
      zoom_win_width = map_width << zoom;
      zoom_win_x = (vga_width - zoom_win_width) / 2;
      q += zoom_win_x;
      w = map_width;
    } else {
      zoom_win_x = 0;
      w = vga_width >> zoom;
      zoom_win_width = w << zoom;
    }

    if ((map_height << zoom) < vga_height) {
      zoom_win_height = map_height << zoom;
      zoom_win_y = (vga_height - zoom_win_height) / 2;
      q += zoom_win_y * vga_width;
      h = map_height;
    } else {
      zoom_win_y = 0;
      h = vga_height >> zoom;
      zoom_win_height = h << zoom;
    }

    if (!zoom_background) {
      if (zoom_win_x || zoom_win_y) {
        update_background();
        full_redraw = 1;
      } else {
        if (_big) {
          big = 0;
          big2 = 1;
        }
        // Only draw border lines when the bitmap genuinely fits in the screen
        // (the centered case). Don't draw them for the truncation remainder
        // when the bitmap overflows — that strip gets filled with pixel data.
        if (zoom_win_y + zoom_win_height < vga_height && (map_height << zoom) < vga_height) {
          wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2,
               zoom_win_y + zoom_win_height + 1, zoom_win_width + 4, 1);
          wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1,
               zoom_win_y + zoom_win_height, zoom_win_width + 2, 1);
          blit_partial(0, zoom_win_y + zoom_win_height, vga_width, 2);
        }
        if (zoom_win_x + zoom_win_width < vga_width && (map_width << zoom) < vga_width) {
          wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y - 2,
               zoom_win_width + 4, 1);
          wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y - 1,
               zoom_win_width + 2, 1);
          blit_partial(zoom_win_y + zoom_win_width, 0, 2, vga_height);
        }
        if (_big) {
          big = 1;
          big2 = 2;
        }
      }
      zoom_background = 1;
    }

    if (zoom_win_x || zoom_win_y)
      blit_partial(zoom_win_x, zoom_win_y, zoom_win_width, zoom_win_height);
    else
      full_redraw = 1;

  } else {
    p = zoom_p;
    q = zoom_q;
    w = zoom_w;
    h = zoom_h;
  }

  switch (zoom) {
  case 0:
    m = h;
    do {
      memcpy(q, p, w);
      p += map_width;
      q += vga_width;
    } while (--m);
    break;

  case 1:
    m = h;
    do {
      n = w;
      do {
        c = *p;
        c += c * 256;
        *(word *)q = c;
        *(word *)(q + vga_width) = c;
        p++;
        q += 2;
      } while (--n);
      p += map_width - w;
      q += vga_width * 2 - w * 2;
    } while (--m);
    break;

  case 2:
    m = h;
    do {
      n = w;
      do {
        c = *p;
        c += c * 256;
        c += c * 65536;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q -= vga_width * 3 - 4;
        p++;
      } while (--n);
      p += map_width - w;
      q += vga_width * 4 - w * 4;
    } while (--m);
    break;

  case 3:
    m = h;
    do {
      n = w;
      do {
        c = *p;
        c += c * 256;
        c += c * 65536;
        *(int *)q = c;
        q += 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q -= 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q += 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q -= 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q += 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q -= 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q += 4;
        *(int *)q = c;
        q += vga_width;
        *(int *)q = c;
        q -= 4;
        *(int *)q = c;
        q -= vga_width * 7 - 8;
        p++;
      } while (--n);
      p += map_width - w;
      q += vga_width * 8 - w * 8;
    } while (--m);
    break;
  }

  // Fill remainder strips when window size isn't a multiple of texel size.
  // The main blit loop above renders w*h full texels, covering w<<zoom by
  // h<<zoom screen pixels. If vga_width or vga_height aren't exact multiples
  // of (1<<zoom), a thin strip at the right/bottom edge remains undrawn.
  // We fill it with partial texels (the next source pixel, repeated to fill).
  if (!zoom_region) {
    int texel = 1 << zoom;
    int drawn_w = w << zoom;
    int drawn_h = h << zoom;
    int rem_x = (zoom_win_x == 0 && (map_width << zoom) >= vga_width) ? vga_width - drawn_w : 0;
    int rem_y = (zoom_win_y == 0 && (map_height << zoom) >= vga_height) ? vga_height - drawn_h : 0;

    // Right edge: partial texel column
    if (rem_x > 0) {
      byte *rq = screen_buffer + drawn_w;
      if (zoom_x + w < map_width) {
        byte *rp = map + zoom_y * map_width + zoom_x + w;
        int row;
        for (row = 0; row < h; row++) {
          int ty;
          for (ty = 0; ty < texel; ty++) {
            memset(rq, *rp, rem_x);
            rq += vga_width;
          }
          rp += map_width;
        }
      } else {
        int row;
        for (row = 0; row < drawn_h; row++) {
          memset(rq, c0, rem_x);
          rq += vga_width;
        }
      }
    }

    // Bottom edge: partial texel row
    if (rem_y > 0) {
      byte *rq = screen_buffer + drawn_h * vga_width;
      if (zoom_y + h < map_height) {
        byte *rp = map + (zoom_y + h) * map_width + zoom_x;
        int col;
        for (col = 0; col < w; col++) {
          byte *rq2 = rq + (col << zoom);
          int ry;
          for (ry = 0; ry < rem_y; ry++) {
            memset(rq2, *rp, texel);
            rq2 += vga_width;
          }
          rp++;
        }
      } else {
        int ry;
        for (ry = 0; ry < rem_y; ry++) {
          memset(rq, c0, drawn_w);
          rq += vga_width;
        }
      }

      // Corner: remainder_x * remainder_y
      if (rem_x > 0) {
        byte *cq = screen_buffer + drawn_h * vga_width + drawn_w;
        byte cc;
        int ry;
        if (zoom_x + w < map_width && zoom_y + h < map_height)
          cc = *(map + (zoom_y + h) * map_width + zoom_x + w);
        else
          cc = c0;
        for (ry = 0; ry < rem_y; ry++) {
          memset(cq, cc, rem_x);
          cq += vga_width;
        }
      }
    }
  }

  if (!zoom_region)
    cclock = (*system_clock) >> 1;

  if (sel_status)
    switch (mode_selection) {
    case -1:
      for (n = 0; n < 512; n += 2)
        if (v.mapa->points[n] != -1) {
          draw_selection_box(v.mapa->points[n], v.mapa->points[n + 1], v.mapa->points[n],
                             v.mapa->points[n + 1]);
          if (point_index == n / 2) {
            draw_selection_box(v.mapa->points[n] - 1, v.mapa->points[n + 1] - 1,
                               v.mapa->points[n] + 1, v.mapa->points[n + 1] + 1);
          }
        }
      break;
    case 0:
      draw_selection_box(sel_x0, sel_y0, sel_x1, sel_y1);
      break;
    case 1:
      draw_selection_mask();
      break;
    case 2:
      draw_selection_mask();
      break;
    case 3:
      draw_selection_mask();
      if (sel_x1 >= 0)
        draw_selection_box(sel_x0, sel_y0, sel_x1, sel_y1);
      break;
    case 4:
      draw_selection_box(sel_x0, sel_y0, sel_x1, sel_y1);
      break;
    case 5:
      draw_selection_mask();
      break;
    }
}

//-----------------------------------------------------------------------------
//      Interpolative fill toolbar sub-mode
//-----------------------------------------------------------------------------

void interpolation_mode(void) {
  int w, h;

  w = sel_mask_x1 - sel_mask_x0 + 1 + 4;
  h = sel_mask_y1 - sel_mask_y0 + 1 + 4;

  if ((m0 = (word *)malloc(w * h * 2)) != NULL) {
    if ((m1 = (word *)malloc(w * h * 2)) != NULL) {
      make_nearest_gradient();

      bar[0] = 101 + zoom;
      bar[1] = 120;
      bar[2] = 154;
      bar[3] = 0;
      draw_bar(0);
      put_bar(10, 2, 118);
      need_zoom = 1;

      memset(m0, 0, w * h * 2);
      memset(m1, 0, w * h * 2);

      do {
        draw_help(1295);
        read_mouse();
        select_zoom();
        test_mouse();

        if (((mouse_b & 1) && selected_icon == 2)) {
          fill_inter(w, h);
          need_zoom = 1;
        } else if ((mouse_b & 1) && selected_icon == 1) {
          fill_inter(w, h);
          need_zoom = 1;
          do {
            read_mouse();
          } while (mouse_b & 1);
        }

        blit_interpolated(w);

        blit_edit();
      } while (!(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
               !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

      blit_interpolated(w);

      if (key(_ESC) ||
          (mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))) {
        put_bar(2, 10, 45);
        flush_bars(0);
        put(mouse_x, mouse_y, mouse_graf);
        blit_screen(screen_buffer);
      }

      free(m1);
      free(m0);
    } else
      free(m0);
  }

  if (m0 == NULL || m1 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }

  draw_mode += 100;
}

//-----------------------------------------------------------------------------
//      Interpolation
//-----------------------------------------------------------------------------

void fill_inter(int w, int h) { // Fill a region with averaged values

  int x, y;
  word *si, *di, n0, n1, n2, n3;
  int n;

  // 1 - Shift m0 one pixel down-right

  for (y = h - 1; y > 0; y--)
    for (x = w - 1; x > 0; x--)
      *(m0 + y * w + x) = *(m0 + (y - 1) * w + x - 1);
  for (x = 0; x < w; x++) {
    *(m0 + x) = 0;
  }
  for (y = 0; y < h; y++) {
    *(m0 + y * w) = 0;
  }

  // 2 - Refresh in m0 the border pixels surrounding the region

  for (y = sel_mask_y0 - 1; y <= sel_mask_y1 + 1; y++)
    if (y >= 0 && y < map_height)
      for (x = sel_mask_x0 - 1; x <= sel_mask_x1 + 1; x++)
        if (x >= 0 && x < map_width) {
          if (is_near_selection_mask(x, y)) {
            n = *(original + (y - sel_mask_y0 + 1) * (w_original) + (x - sel_mask_x0 + 1));
            n = (dac[n * 3] + dac[n * 3 + 1] + dac[n * 3 + 2]) * 256 + 256;
            *(m0 + (y - sel_mask_y0 + 2) * w + (x - sel_mask_x0 + 2)) = n;
          } else if (!is_selection_mask(x, y)) {
            *(m0 + (y - sel_mask_y0 + 2) * w + (x - sel_mask_x0 + 2)) = 0;
          }
        }

  // Interpolate from m0 to m1

  si = m0;
  di = m1;

  for (y = 0; y < h - 1; y++) {
    for (x = 0; x < w - 1; x++) {
      if ((n0 = *si)) // Store in *(di) the average of the four *(si+?)
        if ((n1 = *(si + 1)))
          if ((n2 = *(si + w)))
            if ((n3 = *(si + w + 1)))
              *di = (n0 + n1 + n2 + n3) >> 2;
            else
              *di = (n0 + n1 + n2) / 3;
          else if ((n3 = *(si + w + 1)))
            *di = (n0 + n1 + n3) / 3;
          else
            *di = (n0 + n1) >> 1;
        else if ((n2 = *(si + w)))
          if ((n3 = *(si + w + 1)))
            *di = (n0 + n2 + n3) / 3;
          else
            *di = (n0 + n2) >> 1;
        else if ((n3 = *(si + w + 1)))
          *di = (n0 + n3) >> 1;
        else
          *di = n0;
      else if ((n1 = *(si + 1)))
        if ((n2 = *(si + w)))
          if ((n3 = *(si + w + 1)))
            *di = (n1 + n2 + n3) / 3;
          else
            *di = (n1 + n2) >> 1;
        else if ((n3 = *(si + w + 1)))
          *di = (n1 + n3) >> 1;
        else
          *di = n1;
      else if ((n2 = *(si + w)))
        if ((n3 = *(si + w + 1)))
          *di = (n2 + n3) >> 1;
        else
          *di = n2;
      else if ((n3 = *(si + w + 1)))
        *di = n3;
      else
        *di = 0;
      si++;
      di++;
    }
    si++;
    di++;
  }

  // Now interpolate from m1 to m0

  si = m1;
  di = m0;

  for (y = 0; y < h - 1; y++) {
    for (x = 0; x < w - 1; x++) {
      if ((n0 = *si)) // Store in *(di) the average of the four *(si+?)
        if ((n1 = *(si + 1)))
          if ((n2 = *(si + w)))
            if ((n3 = *(si + w + 1)))
              *di = (n0 + n1 + n2 + n3) >> 2;
            else
              *di = (n0 + n1 + n2) / 3;
          else if ((n3 = *(si + w + 1)))
            *di = (n0 + n1 + n3) / 3;
          else
            *di = (n0 + n1) >> 1;
        else if ((n2 = *(si + w)))
          if ((n3 = *(si + w + 1)))
            *di = (n0 + n2 + n3) / 3;
          else
            *di = (n0 + n2) >> 1;
        else if ((n3 = *(si + w + 1)))
          *di = (n0 + n3) >> 1;
        else
          *di = n0;
      else if ((n1 = *(si + 1)))
        if ((n2 = *(si + w)))
          if ((n3 = *(si + w + 1)))
            *di = (n1 + n2 + n3) / 3;
          else
            *di = (n1 + n2) >> 1;
        else if ((n3 = *(si + w + 1)))
          *di = (n1 + n3) >> 1;
        else
          *di = n1;
      else if ((n2 = *(si + w)))
        if ((n3 = *(si + w + 1)))
          *di = (n2 + n3) >> 1;
        else
          *di = n2;
      else if ((n3 = *(si + w + 1)))
        *di = n3;
      else
        *di = 0;

      si++;
      di++;
    }
    si++;
    di++;
  }
}

//-----------------------------------------------------------------------------
//      Select a region by flood fill (x,y,fill_dac)
//-----------------------------------------------------------------------------

void fill_select(word x, word y) {
  int _mode_fill;

  _mode_fill = mode_fill;
  mode_fill = 1;
  if ((fss = (word *)malloc(60000)) != NULL) {
    fsp = fss;
    fsp_max = fss + 30000;
    fill_scan(x, y);
    free(fss);
  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }
  mode_fill = _mode_fill;
}

//-----------------------------------------------------------------------------
//      Fill a region (containing fill_dac colors) with a color
//-----------------------------------------------------------------------------

void fill(word x, word y) {
  int a, b, c, d;

  if ((fss = (word *)malloc(60000)) != NULL) {
    fsp = fss;
    fsp_max = fss + 30000;

    memset(selection_mask, 0, ((map_width * map_height + 31) / 32) * 4);

    if (mode_fill == 2) {
      memset(fill_dac, 1, 256); // Fill up to the selected color
      fill_dac[color] = 0;
    } else {
      memset(fill_dac, 0, 256); // Fill the color at map(x,y)
      fill_dac[*(map + y * map_width + x)] = 1;
    }

    sel_mask_x0 = map_width;
    sel_mask_y0 = map_height;
    sel_mask_x1 = 0;
    sel_mask_y1 = 0;

    fill_scan(x, y);

    if (mode_fill == 3) {
      if (sel_mask_x0)
        a = sel_mask_x0 - 1;
      else
        a = sel_mask_x0;
      if (sel_mask_y0)
        b = sel_mask_y0 - 1;
      else
        b = sel_mask_y0;
      if (sel_mask_x1 < map_width - 1)
        c = sel_mask_x1 + 1;
      else
        c = sel_mask_x1;
      if (sel_mask_y1 < map_height - 1)
        d = sel_mask_y1 + 1;
      else
        d = sel_mask_y1;
      c = c - a + 1;
      d = d - b + 1;
      w_original = c;
      if ((original = (byte *)save_undo(a, b, c, d)) != NULL) {
        if (!sel_mask_x0)
          original--;
        if (!sel_mask_y0)
          original -= w_original;
        interpolation_mode();
      }
    } else {
      if (save_undo(sel_mask_x0, sel_mask_y0, sel_mask_x1 - sel_mask_x0 + 1,
                    sel_mask_y1 - sel_mask_y0 + 1))
        fill_draw();
    }

    free(fss);
  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
//      Fill algorithm (scanline-based)
//-----------------------------------------------------------------------------

void fill_scan(word x, word y) {
  int x0, x1;

nuevo_scan:

  x0 = x;
  do
    x0--;
  while (x0 >= 0 && fill_dac[*(map + y * map_width + x0)]);
  x0++;
  x1 = x;
  do
    x1++;
  while (x1 < map_width && fill_dac[*(map + y * map_width + x1)]);
  x1--;

  if (x0 < sel_mask_x0) {
    sel_mask_x0 = x0;
  }
  if (x1 > sel_mask_x1) {
    sel_mask_x1 = x1;
  }
  if (y < sel_mask_y0) {
    sel_mask_y0 = y;
  }
  if (y > sel_mask_y1) {
    sel_mask_y1 = y;
  }

  for (x = x0; x <= x1; x++)
    set_selection_mask(x, y);

  if (mode_fill == 1) {
    if (x0 > 0)
      x0--;
    if (x1 < map_width - 1)
      x1++;
  }

  x = x0;
sigue_scan_0:

  if (y > 0)
    while (x <= x1) {
      if (!is_selection_mask(x, y - 1))                 // If not already filled
        if (fill_dac[*(map + (y - 1) * map_width + x)]) // If it needs to be filled
          if (fsp < fsp_max) {                          // If there is stack space
            *fsp = x0;
            fsp++;
            *fsp = x1;
            fsp++;
            *fsp = x + 2;
            fsp++;
            *fsp = y;
            fsp++;
            *fsp = 0;
            fsp++;
            y--;
            goto nuevo_scan;
          }
      x++;
    }

  x = x0;
sigue_scan_1:

  if (y < map_height - 1)
    while (x <= x1) {
      if (!is_selection_mask(x, y + 1))                 // If not already filled
        if (fill_dac[*(map + (y + 1) * map_width + x)]) // If it needs to be filled
          if (fsp < fsp_max) {                          // If there is stack space
            *fsp = x0;
            fsp++;
            *fsp = x1;
            fsp++;
            *fsp = x + 2;
            fsp++;
            *fsp = y;
            fsp++;
            *fsp = 1;
            fsp++;
            y++;
            goto nuevo_scan;
          }
      x++;
    }

  if (fsp > fss) { // While there are unfinished scanlines
    fsp -= 2;
    y = *fsp;
    fsp--;
    x = *fsp;
    fsp--;
    x1 = *fsp;
    fsp--;
    x0 = *fsp;
    if (*(fsp + 4))
      goto sigue_scan_1;
    else
      goto sigue_scan_0;
  }
}

//-----------------------------------------------------------------------------
//      Draw the fill result
//-----------------------------------------------------------------------------

extern byte *texture_color;
byte get_color(int x, int y);

void fill_draw(void) {
  int x, y;

  if (texture_color) {
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y))
          *(map + y * map_width + x) = get_color(x, y);
  } else {
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y))
          *(map + y * map_width + x) = color;
  }
}

//-----------------------------------------------------------------------------
//      Blit the interpolated region
//-----------------------------------------------------------------------------

void blit_interpolated(int w) { // Blit the interpolated region to screen

  int x, y, n;

  for (y = sel_mask_y0; y <= sel_mask_y1; y++)
    for (x = sel_mask_x0; x <= sel_mask_x1; x++)
      if (is_selection_mask(x, y)) {
        n = *(m0 + (y - sel_mask_y0 + 1) * w + (x - sel_mask_x0 + 1));
        if (n > 256)
          n -= 256;
        else
          n = 0;
        *(map + y * map_width + x) = nearest_gradient[n / 256];
      }
}

//-----------------------------------------------------------------------------
//      Draw a bitmap selection (according to zoom,zoom_x/y,sel_mask_*)
//-----------------------------------------------------------------------------

void draw_selection_mask(void) {
  int x0, y0, x1, y1; // Intersection of the selected region and the visible zoom area
  int x, y, c;
  byte *p, g0 = c0, g4 = c4;
  word g04, g40;
  int inc32, *_sel_mask;

  x0 = zoom_x;
  if (sel_mask_x0 > x0)
    x0 = sel_mask_x0;
  y0 = zoom_y;
  if (sel_mask_y0 > y0)
    y0 = sel_mask_y0;
  x1 = zoom_x + (zoom_win_width >> zoom) - 1;
  if (sel_mask_x1 < x1)
    x1 = sel_mask_x1;
  y1 = zoom_y + (zoom_win_height >> zoom) - 1;
  if (sel_mask_y1 < y1)
    y1 = sel_mask_y1;

  // NOTE: Old direct-pointer calculation replaced by the per-zoom switch below
  // p=screen_buffer+(zoom_win_y+((y0-zoom_y)<<zoom))*vga_width+zoom_win_x+((x0-zoom_x)<<zoom);

  if (x1 >= x0 && y1 >= y0)
    switch (zoom) {
    case 0:
      for (y = y0; y <= y1; y++) {
        inc32 = y * map_width + x0;
        _sel_mask = selection_mask + (inc32 >> 5);
        inc32 &= 31;
        p = screen_buffer + (zoom_win_y + y - zoom_y) * vga_width + (zoom_win_x + x0 - zoom_x);
        c = (cclock ^ y ^ x0) & 1;
        for (x = x0; x <= x1; x++, p++, c ^= 1) {
          if (*_sel_mask) {
            if ((*_sel_mask) & (1 << inc32)) {
              if (y == 0 || x == 0 || y == map_height - 1 || x == map_width - 1) {
                if (c)
                  *p = g0;
                else
                  *p = g4;
              } else if (!is_selection_mask(x, y - 1) || !is_selection_mask(x - 1, y) ||
                         !is_selection_mask(x, y + 1) || !is_selection_mask(x + 1, y)) {
                if (c)
                  *p = g0;
                else
                  *p = g4;
              }
            }
          }
          if (!(inc32 = (inc32 + 1) & 31))
            _sel_mask++;
        }
      }
      break;

    case 1:
      if (cclock & 1) {
        g0 = c4;
        g4 = c0;
      }
      g04 = g0 + g4 * 256;
      g40 = g4 + g0 * 256;
      for (y = y0; y <= y1; y++) {
        inc32 = y * map_width + x0;
        _sel_mask = selection_mask + (inc32 >> 5);
        inc32 &= 31;
        p = screen_buffer + (zoom_win_y + (y - zoom_y) * 2) * vga_width + zoom_win_x +
            (x0 - zoom_x) * 2;
        for (x = x0; x <= x1; x++, p += 2) {
          if (*_sel_mask)
            if ((*_sel_mask) & (1 << inc32)) {
              if (y == 0 || (y != 0 && !is_selection_mask(x, y - 1))) {
                *(word *)p = g04;
                if (y == map_height - 1 || (y < map_height - 1 && !is_selection_mask(x, y + 1))) {
                  *(word *)(p + vga_width) = g40;
                } else {
                  if (x == 0 || (x != 0 && !is_selection_mask(x - 1, y))) {
                    *(p + vga_width) = g4;
                  }
                  if (x == map_width - 1 || (x < map_width - 1 && !is_selection_mask(x + 1, y))) {
                    *(p + vga_width + 1) = g0;
                  }
                }
              } else if (x == 0 || (x != 0 && !is_selection_mask(x - 1, y))) {
                *p = g0;
                *(p + vga_width) = g4;
                if (x == map_width - 1 || (x < map_width - 1 && !is_selection_mask(x + 1, y))) {
                  *(p + 1) = g4;
                  *(p + vga_width + 1) = g0;
                } else if (y == map_height - 1 ||
                           (y < map_height - 1 && !is_selection_mask(x, y + 1))) {
                  *(p + vga_width + 1) = g0;
                }
              } else if (y == map_height - 1 ||
                         (y < map_height - 1 && !is_selection_mask(x, y + 1))) {
                *(word *)(p + vga_width) = g40;
                if (x == map_width - 1 || (x < map_width - 1 && !is_selection_mask(x + 1, y))) {
                  *(p + 1) = g4;
                }
              } else if (x == map_width - 1 ||
                         (x < map_width - 1 && !is_selection_mask(x + 1, y))) {
                *(p + 1) = g4;
                *(p + vga_width + 1) = g0;
              }
            }
          if (!(inc32 = (inc32 + 1) & 31))
            _sel_mask++;
        }
      }
      break;

    case 2:
      if (cclock & 1) {
        g0 = c4;
        g4 = c0;
      }
      for (y = y0; y <= y1; y++) {
        p = screen_buffer + (zoom_win_y + (y - zoom_y) * 4) * vga_width + zoom_win_x +
            (x0 - zoom_x) * 4;
        for (x = x0; x <= x1; x++, p += 4)
          if (is_selection_mask(x, y)) {
            if (y == 0 || (y != 0 && !is_selection_mask(x, y - 1))) {
              *p = g0;
              *(p + 1) = g4;
              *(p + 2) = g0;
              *(p + 3) = g4;
            }
            if (x == 0 || (x != 0 && !is_selection_mask(x - 1, y))) {
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p -= vga_width * 3;
            }
            if (y == map_height - 1 || (y < map_height - 1 && !is_selection_mask(x, y + 1))) {
              p += vga_width * 3;
              *p = g4;
              p++;
              *p = g0;
              p++;
              *p = g4;
              p++;
              *p = g0;
              p -= vga_width * 3 + 3;
            }
            if (x == map_width - 1 || (x < map_width - 1 && !is_selection_mask(x + 1, y))) {
              p += 3;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p -= vga_width * 3 + 3;
            }
          }
      }
      break;

    case 3:
      if (cclock & 1) {
        g0 = c4;
        g4 = c0;
      }
      for (y = y0; y <= y1; y++) {
        p = screen_buffer + (zoom_win_y + (y - zoom_y) * 8) * vga_width + zoom_win_x +
            (x0 - zoom_x) * 8;
        for (x = x0; x <= x1; x++, p += 8)
          if (is_selection_mask(x, y)) {
            if (y == 0 || (y != 0 && !is_selection_mask(x, y - 1))) {
              *p = g0;
              *(p + 1) = g4;
              *(p + 2) = g0;
              *(p + 3) = g4;
              *(p + 4) = g0;
              *(p + 5) = g4;
              *(p + 6) = g0;
              *(p + 7) = g4;
            }
            if (x == 0 || (x != 0 && !is_selection_mask(x - 1, y))) {
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p -= vga_width * 7;
            }
            if (y == map_height - 1 || (y < map_height - 1 && !is_selection_mask(x, y + 1))) {
              p += vga_width * 7;
              *p = g4;
              p++;
              *p = g0;
              p++;
              *p = g4;
              p++;
              *p = g0;
              p++;
              *p = g4;
              p++;
              *p = g0;
              p++;
              *p = g4;
              p++;
              *p = g0;
              p -= vga_width * 7 + 7;
            }
            if (x == map_width - 1 || (x < map_width - 1 && !is_selection_mask(x + 1, y))) {
              p += 7;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p += vga_width;
              *p = g4;
              p += vga_width;
              *p = g0;
              p -= vga_width * 7 + 7;
            }
          }
      }
      break;
    }
}

//-----------------------------------------------------------------------------
//      Draw a selection box (according to zoom,zoom_x/y)
//-----------------------------------------------------------------------------

void draw_selection_box(int _x0, int _y0, int _x1, int _y1) {
  int x0, y0, x1, y1, n;
  byte *p;

  if (_x0 > _x1)
    swap(_x0, _x1);
  if (_y0 > _y1)
    swap(_y0, _y1);

  x0 = zoom_win_x + (_x0 << zoom) - (zoom_x << zoom);
  y0 = zoom_win_y + (_y0 << zoom) - (zoom_y << zoom);
  x1 = zoom_win_x + (_x1 << zoom) - (zoom_x << zoom) + (1 << zoom) - 1;
  y1 = zoom_win_y + (_y1 << zoom) - (zoom_y << zoom) + (1 << zoom) - 1;

  if (x1 >= zoom_win_x && x0 < zoom_win_x + zoom_win_width && y1 >= zoom_win_y &&
      y0 < zoom_win_y + zoom_win_height) {
    if (y0 >= zoom_win_y) {
      if (x0 < zoom_win_x) {
        x0 = zoom_win_x;
      }
      if (x1 >= zoom_win_x + zoom_win_width) {
        x1 = zoom_win_x + zoom_win_width - 1;
      }
      n = x1 - x0 + 1;
      p = screen_buffer + vga_width * y0 + x0;
      if ((y0 ^ cclock) & 1)
        do {
          if ((uintptr_t)p & 1)
            *p = c0;
          else
            *p = c4;
          p++;
        } while (--n);
      else
        do {
          if ((uintptr_t)p & 1)
            *p = c4;
          else
            *p = c0;
          p++;
        } while (--n);
    }

    if (y1 < zoom_win_y + zoom_win_height) {
      if (x0 < zoom_win_x) {
        x0 = zoom_win_x;
      }
      if (x1 >= zoom_win_x + zoom_win_width) {
        x1 = zoom_win_x + zoom_win_width - 1;
      }
      n = x1 - x0 + 1;
      p = screen_buffer + vga_width * y1 + x0;
      if ((y1 ^ cclock) & 1)
        do {
          if ((uintptr_t)p & 1)
            *p = c0;
          else
            *p = c4;
          p++;
        } while (--n);
      else
        do {
          if ((uintptr_t)p & 1)
            *p = c4;
          else
            *p = c0;
          p++;
        } while (--n);
    }

    x0 = zoom_win_x + (_x0 << zoom) - (zoom_x << zoom);
    x1 = zoom_win_x + (_x1 << zoom) - (zoom_x << zoom) + (1 << zoom) - 1;

    if (x0 >= zoom_win_x) {
      if (y0 < zoom_win_y) {
        y0 = zoom_win_y;
      }
      if (y1 >= zoom_win_y + zoom_win_height) {
        y1 = zoom_win_y + zoom_win_height - 1;
      }
      n = y1 - y0 + 1;
      p = screen_buffer + vga_width * y0 + x0;
      if ((x0 ^ cclock) & 1)
        do {
          if (y0 & 1)
            *p = c0;
          else
            *p = c4;
          p += vga_width;
          y0++;
        } while (--n);
      else
        do {
          if (y0 & 1)
            *p = c4;
          else
            *p = c0;
          p += vga_width;
          y0++;
        } while (--n);
      y0 = zoom_win_y + (_y0 << zoom) - (zoom_y << zoom);
    }

    if (x1 < zoom_win_x + zoom_win_width) {
      if (y0 < zoom_win_y) {
        y0 = zoom_win_y;
      }
      if (y1 >= zoom_win_y + zoom_win_height) {
        y1 = zoom_win_y + zoom_win_height - 1;
      }
      n = y1 - y0 + 1;
      p = screen_buffer + vga_width * y0 + x1;
      if ((x1 ^ cclock) & 1)
        do {
          if (y0 & 1)
            *p = c0;
          else
            *p = c4;
          p += vga_width;
          y0++;
        } while (--n);
      else
        do {
          if (y0 & 1)
            *p = c4;
          else
            *p = c0;
          p += vga_width;
          y0++;
        } while (--n);
    }
  }
}

//-----------------------------------------------------------------------------
//      Bitmap functions, set a pixel
//-----------------------------------------------------------------------------

void set_selection_mask(int x, int y) { // Set a pixel in the bitmap

  x += y * map_width; // Bit index in the buffer
  y = x >> 5;         // Int index in the buffer
  x &= 31;            // Bit index within the int
  *(selection_mask + y) |= 1 << x;
}

//-----------------------------------------------------------------------------
//      Bitmap functions, test a pixel
//-----------------------------------------------------------------------------

int is_selection_mask(int x, int y) { // Test a pixel in the bitmap

  x += y * map_width; // Bit index in the buffer
  y = x >> 5;         // Int index in the buffer
  x &= 31;            // Bit index within the int
  return (*(selection_mask + y) & (1 << x));
}

int is_near_selection_mask(int x, int y) {
  int c;

  if (x)
    c = is_selection_mask(x - 1, y);
  else
    c = 0;
  if (x < map_width - 1)
    c |= is_selection_mask(x + 1, y);
  if (y)
    c |= is_selection_mask(x, y - 1);
  if (y < map_height - 1)
    c |= is_selection_mask(x, y + 1);

  return (c && !is_selection_mask(x, y));
}

//-----------------------------------------------------------------------------
//      UNDO - Save the region affected by an action, before performing it
//-----------------------------------------------------------------------------

byte *save_undo(int x, int y, int w, int h) {
  int a, start, end; // Start and end of the saved block (in undo[])
  byte *ret = 0;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > map_width) {
    w = map_width - x;
  }
  if (y + h > map_height) {
    h = map_height - y;
  }
  if (w <= 0 || h <= 0)
    return ((byte *)-1);

  // Determine the zoom region to refresh

  if (x < zoom_x)
    need_zoom_x = zoom_win_x - ((zoom_x - x) << zoom);
  else
    need_zoom_x = zoom_win_x + ((x - zoom_x) << zoom);
  if (y < zoom_y)
    need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
  else
    need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
  need_zoom_width = w << zoom;
  need_zoom_height = h << zoom;

  if (need_zoom_x + need_zoom_width <= zoom_win_x || need_zoom_y + need_zoom_height <= zoom_win_y ||
      need_zoom_x >= zoom_win_x + zoom_win_width || need_zoom_y >= zoom_win_y + zoom_win_height) {
    need_zoom_width = 0;
    need_zoom_height = 0;
  } else {
    if (need_zoom_x < zoom_win_x) {
      need_zoom_width -= zoom_win_x - need_zoom_x;
      need_zoom_x = zoom_win_x;
    }
    if (need_zoom_y < zoom_win_y) {
      need_zoom_height -= zoom_win_y - need_zoom_y;
      need_zoom_y = zoom_win_y;
    }
    if (need_zoom_x + need_zoom_width > zoom_win_x + zoom_win_width)
      need_zoom_width = zoom_win_x + zoom_win_width - need_zoom_x;
    if (need_zoom_y + need_zoom_height > zoom_win_y + zoom_win_height)
      need_zoom_height = zoom_win_y + zoom_win_height - need_zoom_y;
    if (!need_zoom)
      need_zoom = -1;
  }

  if (undo_index)
    a = undo_index - 1;
  else
    a = max_undos - 1;

  start = 0;
  if (undo_table[a].mode != -1)
    start = undo_table[a].end;

  if (start + w * h > undo_memory)
    start = 0;

  // If an action exceeds undo_memory, don't save it.

  if ((end = start + w * h) <= undo_memory) {
    undo_table[undo_index].start = start;
    undo_table[undo_index].end = end;

    undo_table[undo_index].x = x;
    undo_table[undo_index].y = y;
    undo_table[undo_index].w = w;
    undo_table[undo_index].h = h;

    undo_table[undo_index].mode = draw_mode;

    // Invalidate any overwritten blocks

    for (a = 0; a < max_undos; a++)
      if (a != undo_index && undo_table[a].mode != -1)
        if ((undo_table[a].start < start && undo_table[a].end > start) ||
            (undo_table[a].start >= start && undo_table[a].start < end))
          undo_table[a].mode = -1;

    // Save the region that will be affected by the action.

    copy_block(undo + start, map + x + y * map_width, w, h);

    undo_table[undo_index].code = current_map_code;

    undo_index = (undo_index + 1) % max_undos;
    undo_table[undo_index].mode = -1; // Disable undo_next()

    ret = (byte *)(undo + start);

  } else {
    draw_edit_background(0, 0, vga_width, vga_height);
    flush_bars(1);
    full_redraw = 1;
    blit_screen(screen_buffer);
    v_text = (char *)texts[320];
    show_dialog(err0);
    undo_error = 1;
  }

  return (ret);
}

//-----------------------------------------------------------------------------
//      UNDO - Restore the region affected by the last action.
//-----------------------------------------------------------------------------

int undo_back(void) {
  int x, y, w, h;
  int a;

  if (undo_index)
    a = undo_index - 1;
  else
    a = max_undos - 1;

  if (undo_table[a].mode != -1 && undo_table[a].code == current_map_code) {
    undo_index = a;
    retrace_wait();

    x = undo_table[a].x;
    y = undo_table[a].y;
    w = undo_table[a].w;
    h = undo_table[a].h;

    xchg_block(undo + undo_table[a].start, map + x + y * map_width, w, h);

    // Determine the zoom region to refresh

    if (x < zoom_x)
      need_zoom_x = zoom_win_x - ((zoom_x - x) << zoom);
    else
      need_zoom_x = zoom_win_x + ((x - zoom_x) << zoom);
    if (y < zoom_y)
      need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
    else
      need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
    need_zoom_width = w << zoom;
    need_zoom_height = h << zoom;

    if (need_zoom_x + need_zoom_width <= zoom_win_x ||
        need_zoom_y + need_zoom_height <= zoom_win_y ||
        need_zoom_x >= zoom_win_x + zoom_win_width || need_zoom_y >= zoom_win_y + zoom_win_height) {
      need_zoom_width = 0;
      need_zoom_height = 0;
    } else {
      if (need_zoom_x < zoom_win_x) {
        need_zoom_width -= zoom_win_x - need_zoom_x;
        need_zoom_x = zoom_win_x;
      }
      if (need_zoom_y < zoom_win_y) {
        need_zoom_height -= zoom_win_y - need_zoom_y;
        need_zoom_y = zoom_win_y;
      }
      if (need_zoom_x + need_zoom_width > zoom_win_x + zoom_win_width)
        need_zoom_width = zoom_win_x + zoom_win_width - need_zoom_x;
      if (need_zoom_y + need_zoom_height > zoom_win_y + zoom_win_height)
        need_zoom_height = zoom_win_y + zoom_win_height - need_zoom_y;
      if (!need_zoom)
        need_zoom = -1;
    }
    return (1);
  } else
    return (0);
}

//-----------------------------------------------------------------------------
//      UNDO - Redo the last action undone with undo_back()
//-----------------------------------------------------------------------------

void undo_next(void) {
  int x, y, w, h;

  if (undo_table[undo_index].mode != -1 && undo_table[undo_index].code == current_map_code) {
    retrace_wait();

    x = undo_table[undo_index].x;
    y = undo_table[undo_index].y;
    w = undo_table[undo_index].w;
    h = undo_table[undo_index].h;

    xchg_block(undo + undo_table[undo_index].start, map + x + y * map_width, w, h);

    // Determine the zoom region to refresh

    if (x < zoom_x)
      need_zoom_x = zoom_win_x - ((zoom_x - x) << zoom);
    else
      need_zoom_x = zoom_win_x + ((x - zoom_x) << zoom);
    if (y < zoom_y)
      need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
    else
      need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
    need_zoom_width = w << zoom;
    need_zoom_height = h << zoom;

    if (need_zoom_x + need_zoom_width <= zoom_win_x ||
        need_zoom_y + need_zoom_height <= zoom_win_y ||
        need_zoom_x >= zoom_win_x + zoom_win_width || need_zoom_y >= zoom_win_y + zoom_win_height) {
      need_zoom_width = 0;
      need_zoom_height = 0;
    } else {
      if (need_zoom_x < zoom_win_x) {
        need_zoom_width -= zoom_win_x - need_zoom_x;
        need_zoom_x = zoom_win_x;
      }
      if (need_zoom_y < zoom_win_y) {
        need_zoom_height -= zoom_win_y - need_zoom_y;
        need_zoom_y = zoom_win_y;
      }
      if (need_zoom_x + need_zoom_width > zoom_win_x + zoom_win_width)
        need_zoom_width = zoom_win_x + zoom_win_width - need_zoom_x;
      if (need_zoom_y + need_zoom_height > zoom_win_y + zoom_win_height)
        need_zoom_height = zoom_win_y + zoom_win_height - need_zoom_y;
      if (!need_zoom)
        need_zoom = -1;
    }

    undo_index = (undo_index + 1) % max_undos;
  }
}

//-----------------------------------------------------------------------------
//      Copy a block from the map to memory
//-----------------------------------------------------------------------------

void copy_block(byte *d, byte *s, int w, int h) {
  do {
    memcpy(d, s, w);
    d += w;
    s += map_width;
  } while (--h);
}

//-----------------------------------------------------------------------------
//      Swap a block between the map and memory
//-----------------------------------------------------------------------------

void xchg_block(byte *d, byte *s, int w, int h) {
  do {
    memxchg(d, s, w);
    d += w;
    s += map_width;
  } while (--h);
}
