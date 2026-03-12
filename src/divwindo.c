
//-----------------------------------------------------------------------------
//      Basic window drawing primitives
//-----------------------------------------------------------------------------

#include "global.h"

extern SDL_Surface *vga;

void bwput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n);
void wtexc(byte *dest, int dest_pitch, int dest_width, int dest_height, byte *p, int x, int y,
           byte w, int h, byte c);

void wtexn(byte *dest, int dest_pitch, byte *p, int x, int y, byte w, int h, byte c);


//-----------------------------------------------------------------------------
//      Draw a button on a window
//-----------------------------------------------------------------------------

// button

void draw_button(int n, int x, int y, int alignment, int color) {
  wwrite(v.ptr, v.w / big2, v.h / big2, x, y, alignment, texts[100 + n], color);
}

// Mouse button
int mouse_button_hit(int n, int x, int y, int alignment) {
  int w, h;
  int mx = wmouse_x, my = wmouse_y;

  w = text_len(texts[100 + n] + 1);
  h = 7;

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

  return (mx >= x - 3 && mx <= x + w + 3 && my >= y - 3 && my <= y + h + 3);
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
//  Title bar
//-----------------------------------------------------------------------------

char dither_pattern[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
    0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0,
    1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1,
    1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1,
    1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

extern char cbs[65], cbn, cgs[65], cgn, crs[65], crn;

void wgra(byte *dest, int dest_width, int dest_height, byte color, int x, int y, int w, int h) {
  int xx, yy, c, m;
  char *cs, cn;

  if (big) {
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

  if (color == c_r_low) {
    cn = crn;
    cs = crs;
  } else if (color == c1) {
    cn = cgn;
    cs = cgs;
  } else {
    cn = cbn;
    cs = cbs;
  }

  if (w > 0 && h > 0) {
    xx = 0;
    do {
      c = (cn - 1) * xx * 16 / w;
      m = c % 16;
      c /= 16;
      yy = 0;
      do {
        if (dither_pattern[m * 16 + (xx % 4) + (yy % 4) * 4]) {
          dest[(y + yy) * dest_width + (x + xx)] = cs[c + 1];
        } else {
          dest[(y + yy) * dest_width + (x + xx)] = cs[c];
        }
      } while (++yy < h);
    } while (++xx < w);
  }
}

//-----------------------------------------------------------------------------
//      Highlight a box, like an icon under the cursor
//-----------------------------------------------------------------------------

void wresalta_box(byte *dest, int dest_width, int dest_height, int x, int y, int w, int h) {
  byte *p;

  if (big) {
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
    p = dest + y * dest_width + x;
    x = w;
    do {
      do {
        if (*p == c2)
          *p = c1;
        else if (*p == c3)
          *p = c2;
        p++;
      } while (--w);
      p += dest_width - (w = x);
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
//      Puts a graph
//-----------------------------------------------------------------------------

void wput(byte *dest, int dest_width, int dest_height, int x, int y, int n) {
  wput_in_box(dest, dest_width, dest_width, dest_height, x, y, n);
}

void wput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n) {
  int h, w;
  int block;
  byte *p, *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (big) {
    if ((n >= 32 || n < 0) && n != 233) {
      bwput_in_box(dest, dest_pitch, dest_width, dest_height, x, y, n);
      return;
    }
  } else {
    if (n == mouse_graf && mouse_shift) {
      x = mouse_shift_x;
      y = mouse_shift_y;
    }
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
//      Dump window
//-----------------------------------------------------------------------------

void blit_region(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w, int h,
                 int salta) {
  byte *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;
  int vn = 0;

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
  } while (--long_y > 0 && q < (dest + (dest_width * dest_height)));
}

//-----------------------------------------------------------------------------
//      draw hidden (greyed) Window
//-----------------------------------------------------------------------------

void blit_region_dark(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w,
                      int h, int salta) {
  byte *q, *_ghost;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;
  int vn = 0;

  SDL_Rect trc;

  for (vn = 0; vn < max_windows; vn++) {
    if (window[vn].ptr != NULL)
      if (window[vn].ptr == p) {
        break;
      }
  }
  q = dest + y * dest_width + x;
  _ghost = ghost + 256 * (int)c0;

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


  trc.x = x;
  trc.y = y;
  trc.w = dest_width;
  trc.h = dest_height;
}

//-----------------------------------------------------------------------------
//      Text rendering functions
//-----------------------------------------------------------------------------
typedef struct {
  byte w;
  word dir;
} sscar;
// * car;

int char_len(char c) {
  sscar *car;
  car = (sscar *)(text_font + 1);
  return (car[c].w);
}

int text_len(byte *ptr) {
  int w;

  sscar *car;

  car = (sscar *)(text_font + 1);
  w = 0;
  while (*ptr) {
    w += car[*ptr].w;
    ptr++;
  }
  if (big)
    w /= 2;

  if (w)
    w--;


  return (w);
}


int text_len2(byte *ptr) {
  int w;
  sscar *car;
  car = (sscar *)(text_font + 1);
  w = 0;

  while (*ptr) {
    w += car[*ptr].w;
    ptr++;
  }

  debugprintf("text_len2 %s: %d\n", ptr, w);

  if (big)
    w /= 2;

  return (w - 1);
}

void wwrite(byte *dest, int dest_width, int dest_height, int x, int y, int alignment, byte *ptr,
            byte c) {
  wwrite_in_box(dest, dest_width, dest_width, dest_height, x, y, alignment, ptr, c);
}

extern SDL_Surface *vga;

extern struct t_listboxbr file_list_br;

void wwrite_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y,
                   int alignment, byte *ptr, byte c) {
  int w, h, button, multi;

  byte *font;

  sscar *car;

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

  car = (sscar *)(text_font + 1);

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
      if (big) {
        *(dest + (2 * y - 3) * dest_pitch * 2 + 2 * x - 4) = c3;
        *(dest + (2 * y - 4) * dest_pitch * 2 + 2 * x - 3) = c3;
        *(dest + (2 * y - 4) * dest_pitch * 2 + 2 * (x + w) + 2) = c3;
        *(dest + (2 * y - 3) * dest_pitch * 2 + 2 * (x + w) + 3) = c1;
        *(dest + (2 * (y + h) + 2) * dest_pitch * 2 + 2 * x - 4) = c3;
        *(dest + (2 * (y + h) + 3) * dest_pitch * 2 + 2 * x - 3) = c1;
      }
    } else {
      wbox(dest, dest_pitch, dest_height, c1, x - 2, y - 2, w + 4, h + 4);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 3, y - 3, w + 6, h + 6);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 2, y - 2, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 2, y - 2, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c2, x - 1, y + h + 1, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c2, x + w + 1, y - 1, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c3, x + w + 1, y + h + 1, 1, 1);

      if (big) {
        *(dest + (2 * (y + h) + 3) * dest_pitch * 2 + 2 * (x + w) + 2) = c2;
        *(dest + (2 * (y + h) + 2) * dest_pitch * 2 + 2 * (x + w) + 3) = c2;
        *(dest + (2 * y - 4) * dest_pitch * 2 + 2 * (x + w) + 2) = c0;
        *(dest + (2 * y - 3) * dest_pitch * 2 + 2 * (x + w) + 3) = c2;
        *(dest + (2 * (y + h) + 2) * dest_pitch * 2 + 2 * x - 4) = c0;
        *(dest + (2 * (y + h) + 3) * dest_pitch * 2 + 2 * x - 3) = c2;
      }
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
    if (y >= 0 && y + h <= dest_height) { // Text fits entirely (y axis)
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
      if (*p)
        *q = c;
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
      if (*p)
        *q = c;
      p++;
      q++;
    } while (--w);

    q += dest_pitch - (w = long_x);
    p += resto_x;
  } while (--long_y);
}
