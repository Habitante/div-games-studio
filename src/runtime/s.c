
//----------------------------------------------------------------------------
// Sprite, text, and scroll rendering functions
//----------------------------------------------------------------------------

#include "inter.h"

#ifdef DEBUG
int get_ticks(void);
void process_paint(int id, int n);
void function_exec(int id, int n);
#endif

int line_fx, color;

void line(int x0, int y0, int x1, int y1);
void draw_box(int x, int y, int an, int al);
void draw_filled_box(int x, int y, int an, int al);
void draw_circle(int relleno, int x0, int y0, int x1, int y1);
void line_pixel(int x, int y);


//----------------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------------
void move_scroll(int plano, int x, int y);
void save_region(void);
void sp_scan_clipped(byte *p, short n, short m, short o, byte *si, int an, int x0, int y0, int x1,
                     int y1);
void sp_scan_clipped_ghost(byte *p, short n, short m, short o, byte *si, int an, int x0, int y0,
                           int x1, int y1);
void sp_scan(byte *p, short n, byte *si, int an, int x0, int y0, int x1, int y1);
void sp_scan_ghost(byte *p, short n, byte *si, int an, int x0, int y0, int x1, int y1);
void text_clipped(byte *p, int x, int y, byte an, int al);
void text_normal(byte *p, int x, int y, byte an, int al);
void paint_mode7(int n, int camara_x, int camara_y, int camara_z, int angulo);
void paint_sprite_m7(int n, int ide, int x, int y, int size, int ang);

//----------------------------------------------------------------------------
// Angle constants (in 4096-unit degrees)
//----------------------------------------------------------------------------

#define grados_0    0
#define grados_30   320
#define grados_45   512
#define grados_90   1024
#define grados_180  2048
#define grados_270  3072
#define grados_360  4096
#define grados_giro 16

//----------------------------------------------------------------------------
// Module variables
//----------------------------------------------------------------------------

int seno[grados_360 + 1], coseno[grados_360 + 1]; //m7

//----------------------------------------------------------------------------
// Fast scroll parallax lookup table
//----------------------------------------------------------------------------

tfast *fast;

static int mul_24(int a, int b) {
  uint64_t c = (uint64_t)a * (uint64_t)b;
  return (c >> 24);
}

//----------------------------------------------------------------------------
//  Muliply two 16:16 floating points
//----------------------------------------------------------------------------
static unsigned int mul_16(long a, long b) {
  uint64_t c = (uint64_t)a * (uint64_t)b;
  return c >> 16;
}


//----------------------------------------------------------------------------
// Paint scroll sprites sorted by Z order
//----------------------------------------------------------------------------

void paint_scroll_sprites(void) {
#ifdef DEBUG
  int oreloj;
#endif
  int old_ide = ide, max;

  if (post_process_scroll != NULL)
    post_process_scroll();

  for (ide = id_start; ide <= id_end; ide += iloc_len)
    mem[ide + _Painted] = 0;
  do {
#ifdef DEBUG
    oreloj = get_ticks();
#endif
    ide = 0;
    max = 0x80000000;
    for (id = id_start; id <= id_end; id += iloc_len)
      if ((mem[id + _Status] == 2 || mem[id + _Status] == 4) && mem[id + _Ctype] == 1 &&
          (mem[id + _Cnumber] == 0 || (mem[id + _Cnumber] & (1 << snum))) && !mem[id + _Painted] &&
          mem[id + _Z] > max) {
        ide = id;
        max = mem[id + _Z];
      }
    if (ide) {
      paint_sprite();
      mem[ide + _Painted] = 1;
#ifdef DEBUG
      process_paint(ide, get_ticks() - oreloj);
#endif
    }
  } while (ide);
  ide = old_ide;
}

//----------------------------------------------------------------------------
// Determine scroll window movement (automatic camera or manual)
//----------------------------------------------------------------------------

void update_scroll(int back) {
  int i, x, y, ix, iy, r;

  if ((i = (scroll + snum)->camera)) { // Check if scroll has automatic camera tracking
    if (i >= id_start && i <= id_end && mem[i] == i) {
      ix = mem[i + _X];
      iy = mem[i + _Y];
      if (mem[i + _Resolution]) {
        ix /= mem[i + _Resolution];
        iy /= mem[i + _Resolution];
      }
      x = ix + iscroll[snum].x - iscroll[snum].map1_x;
      y = iy + iscroll[snum].y - iscroll[snum].map1_y;
      r = (scroll + snum)->region1;
      if (r >= 0 && r < max_region) {
        ix = 0;
        iy = 0;
        if (x < region[r].x0)
          ix = x - region[r].x0;
        if (y < region[r].y0)
          iy = y - region[r].y0;
        if (x >= region[r].x1)
          ix = x - (region[r].x1 - 1);
        if (y >= region[r].y1)
          iy = y - (region[r].y1 - 1);
      } else { // No lock region1 defined, center camera on target
        ix -= iscroll[snum].an / 2;
        iy -= iscroll[snum].al / 2;
        ix -= iscroll[snum].map1_x;
        iy -= iscroll[snum].map1_y;
      }
      r = (scroll + snum)->region2;
      if (r >= 0 && r < max_region) {
        if (x < region[r].x0 || x >= region[r].x1) {
          if (x < region[r].x0)
            ix = x - region[r].x0;
          else
            ix = x - (region[r].x1 - 1);
        } else if ((scroll + snum)->speed > 0) {
          if (ix > (scroll + snum)->speed)
            ix = (scroll + snum)->speed;
          else if (ix < -(scroll + snum)->speed)
            ix = -(scroll + snum)->speed;
        }
        if (y < region[r].y0 || y >= region[r].y1) {
          if (y < region[r].y0)
            iy = y - region[r].y0;
          else
            iy = y - (region[r].y1 - 1);
        } else if ((scroll + snum)->speed > 0) {
          if (iy > (scroll + snum)->speed)
            iy = (scroll + snum)->speed;
          else if (iy < -(scroll + snum)->speed)
            iy = -(scroll + snum)->speed;
        }
      }

      x = iscroll[snum].map1_x;
      y = iscroll[snum].map1_y;
      move_scroll(0, ix, iy);

      if (back)
        if ((scroll + snum)->ratio) {
          ix = iscroll[snum].map1_x - x;
          iy = iscroll[snum].map1_y - y;
          move_scroll(1, ix * 100 / (scroll + snum)->ratio, iy * 100 / (scroll + snum)->ratio);
        }
    }
  } else { // Check if scroll is moved manually
    if ((scroll + snum)->x0 != iscroll[snum].map1_x ||
        (scroll + snum)->y0 != iscroll[snum].map1_y) {
      move_scroll(0, (scroll + snum)->x0 - iscroll[snum].map1_x,
                  (scroll + snum)->y0 - iscroll[snum].map1_y);
    }
    if (back)
      if ((scroll + snum)->x1 != iscroll[snum].map2_x ||
          (scroll + snum)->y1 != iscroll[snum].map2_y) {
        move_scroll(1, (scroll + snum)->x1 - iscroll[snum].map2_x,
                    (scroll + snum)->y1 - iscroll[snum].map2_y);
      }
  }
}

//----------------------------------------------------------------------------
// Scroll simple
//----------------------------------------------------------------------------

void scroll_simple(void) {
  int n, b;
#ifdef DEBUG
  int oreloj = get_ticks();
#endif
  byte *di, *si;

  update_scroll(0);

  si = iscroll[snum].sscr1;
  b = iscroll[snum].block1;
  di = screen_buffer + iscroll[snum].x + iscroll[snum].y * vga_width;

  for (n = 0; n < iscroll[snum].al; n++) {
    memcpy(di, si, iscroll[snum].an);
    di += vga_width;
    if (--b == 0)
      si -= iscroll[snum].an * (iscroll[snum].al - 1);
    else
      si += iscroll[snum].an;
  }
#ifdef DEBUG
  function_exec(252, get_ticks() - oreloj);
#endif
  paint_scroll_sprites();
}

//----------------------------------------------------------------------------
// Scroll parallax
//----------------------------------------------------------------------------

void scroll_parallax(void) {
  int n, an, b1, b2;
#ifdef DEBUG
  int oreloj = get_ticks();
#endif
  int nt, scan;
  byte *di, *si1, *si2;

  update_scroll(1); // Move both scroll planes (auto or manual)

  si1 = iscroll[snum].sscr1;
  si2 = iscroll[snum].sscr2;
  b1 = iscroll[snum].block1;
  b2 = iscroll[snum].block2;
  di = screen_buffer + iscroll[snum].x + iscroll[snum].y * vga_width;
  fast = iscroll[snum].fast;

  scan = iscroll[snum].iscan;

  for (n = 0; n < iscroll[snum].al; n++) {
    if (fast[scan].nt >= max_inc) {
      an = iscroll[snum].an;
      do {
        if (*si1)
          *di = *si1;
        else
          *di = *si2;
        di++;
        si1++;
        si2++;
      } while (--an);
    } else {
      nt = 0;
      do {
        if (fast[scan].inc[nt]) {
          if (nt & 1) {
            memcpy(di, si1, fast[scan].inc[nt]);
          } else {
            memcpy(di, si2, fast[scan].inc[nt]);
          }
          di += fast[scan].inc[nt];
          si1 += fast[scan].inc[nt];
          si2 += fast[scan].inc[nt];
        }
      } while (nt++ < fast[scan].nt);
    }
    if (++scan == iscroll[snum].al)
      scan = 0;
    di += vga_width - iscroll[snum].an;
    if (--b1 == 0)
      si1 -= iscroll[snum].an * iscroll[snum].al;
    if (--b2 == 0)
      si2 -= iscroll[snum].an * iscroll[snum].al;
  }
#ifdef DEBUG
  function_exec(252, get_ticks() - oreloj);
#endif
  paint_scroll_sprites();
}

//----------------------------------------------------------------------------
//      Set_scroll(plano,x,y) - snum
//----------------------------------------------------------------------------

void set_scroll(int plano, int x, int y) {
  int an, al, iscan;
  int _nx, nx, ny;
  byte *di, *si, *_si;
  int kk, nt, esdato;

  if (snum < 0 || snum > 9)
    return;

  if (plano < 0 || plano >= iscroll[snum].on)
    return;

  if (plano == 0) {
    di = iscroll[snum].sscr1 = iscroll[snum]._sscr1;
    iscroll[snum].block1 = iscroll[snum].al;
    if (!(iscroll[snum].map_flags & 1)) {
      if (x < 0)
        x = 0;
      else if (x + iscroll[snum].an >= iscroll[snum].map1_an)
        x = iscroll[snum].map1_an - iscroll[snum].an;
    }
    if (!(iscroll[snum].map_flags & 2)) {
      if (y < 0)
        y = 0;
      else if (y + iscroll[snum].al >= iscroll[snum].map1_al)
        y = iscroll[snum].map1_al - iscroll[snum].al;
    }
    iscroll[snum].map1_x = x;
    iscroll[snum].map1_y = y;
    an = iscroll[snum].map1_an;
    al = iscroll[snum].map1_al;
    si = iscroll[snum].map1;
  } else {
    di = iscroll[snum].sscr2 = iscroll[snum]._sscr2;
    iscroll[snum].block2 = iscroll[snum].al;
    if (!(iscroll[snum].map_flags & 4)) {
      if (x < 0)
        x = 0;
      else if (x + iscroll[snum].an >= iscroll[snum].map2_an)
        x = iscroll[snum].map2_an - iscroll[snum].an;
    }
    if (!(iscroll[snum].map_flags & 8)) {
      if (y < 0)
        y = 0;
      else if (y + iscroll[snum].al >= iscroll[snum].map2_al)
        y = iscroll[snum].map2_al - iscroll[snum].al;
    }
    iscroll[snum].map2_x = x;
    iscroll[snum].map2_y = y;
    an = iscroll[snum].map2_an;
    al = iscroll[snum].map2_al;
    si = iscroll[snum].map2;
  }

  if (x < 0)
    x = (an - 1) - ((-x - 1) % an);
  else
    x = x % an;

  if (y < 0)
    y = (al - 1) - ((-y - 1) % al);
  else
    y = y % al;

  si += y * an + x;

  // Copy map region (si,x,y,an,al) into scroll buffer (di,iscroll[snum].an/al)

  /* This optimization was removed
  if (x+iscroll[snum].an<=an && y+iscroll[snum].al<=al) {
    y=iscroll[snum].al; do {
      memcpy(di,si,iscroll[snum].an);
      di+=iscroll[snum].an; si+=an;
    } while (--y);
  } else {
*/

  if (iscroll[snum].on == 2 && plano == 0) {
    fast = iscroll[snum].fast;
    ny = al - y;
    _nx = an - x;
    y = iscroll[snum].al;
    iscan = 0;
    do {
      _si = si;
      nx = _nx;
      x = iscroll[snum].an;
      nt = 0; // number of segments
      fast[iscan].inc[0] = 0;
      esdato = 0;
      do {
        if ((kk = *si++)) {
          if (esdato) {
            if (nt < max_inc)
              fast[iscan].inc[nt]++;
          } else {
            nt++;
            esdato = 1;
            if (nt < max_inc)
              fast[iscan].inc[nt] = 1;
          }
        } else {
          if (esdato) {
            nt++;
            esdato = 0;
            if (nt < max_inc)
              fast[iscan].inc[nt] = 1;
          } else {
            if (nt < max_inc)
              fast[iscan].inc[nt]++;
          }
        }
        *di++ = kk;
        if (--nx == 0) {
          si -= an;
          nx = an;
        }
      } while (--x);
      fast[iscan].nt = nt;
      iscan++;
      si = _si + an;
      if (--ny == 0) {
        si -= an * al;
        ny = al;
      }
    } while (--y);
    iscroll[snum].iscan = 0;
  } else {
    ny = al - y;
    _nx = an - x;
    y = iscroll[snum].al;
    do {
      _si = si;
      nx = _nx;
      x = iscroll[snum].an;
      do {
        kk = *si++;
        *di++ = kk;
        if (--nx == 0) {
          si -= an;
          nx = an;
        }
      } while (--x);
      si = _si + an;
      if (--ny == 0) {
        si -= an * al;
        ny = al;
      }
    } while (--y);
  }

  (scroll + snum)->x0 = iscroll[snum].map1_x;
  (scroll + snum)->y0 = iscroll[snum].map1_y;
  (scroll + snum)->x1 = iscroll[snum].map2_x;
  (scroll + snum)->y1 = iscroll[snum].map2_y;
}

//----------------------------------------------------------------------------
// Recalculate the increment list for a scanline (0..vga_height-1)
//----------------------------------------------------------------------------

void process_scan(int n) { // Fully reprocess a scanline
  int in;                  // index into fast[in].{nt,inc[]}
  byte *si;
  int resta, an;
  int nt = 0, esdato = 0;

  if ((in = iscroll[snum].iscan + n) >= iscroll[snum].al)
    in -= iscroll[snum].al;
  if (n >= iscroll[snum].block1)
    n -= iscroll[snum].al;
  si = iscroll[snum].sscr1 + n * iscroll[snum].an;
  fast = iscroll[snum].fast;

  an = iscroll[snum].an;
  resta = iscroll[snum].map1_an;
  fast[in].inc[0] = 0;
  do {
    if (*si++) {
      if (esdato) {
        if (nt < max_inc)
          fast[in].inc[nt]++;
      } else {
        nt++;
        esdato = 1;
        if (nt < max_inc)
          fast[in].inc[nt] = 1;
      }
    } else {
      if (esdato) {
        nt++;
        esdato = 0;
        if (nt < max_inc)
          fast[in].inc[nt] = 1;
      } else {
        if (nt < max_inc)
          fast[in].inc[nt]++;
      }
    }
    if (--resta == 0) {
      si -= iscroll[snum].map1_an;
      resta = iscroll[snum].map1_an;
    }
  } while (--an);
  fast[in].nt = nt;
}

//----------------------------------------------------------------------------
// Shift all scanlines right by one pixel (exits on the left)
//----------------------------------------------------------------------------

void process_scan_right(void) {
  int n, in, block;
  byte *si;

  block = iscroll[snum].block1;
  si = iscroll[snum].sscr1 + iscroll[snum].an - 1; // End of the first scanline
  in = iscroll[snum].iscan;
  n = iscroll[snum].al;
  fast = iscroll[snum].fast;

  do {
    // 1. Remove the pixel on the left ...

    if (fast[in].inc[0]) { // If it starts with background
      fast[in].inc[0]--;
    } else {
      if (fast[in].inc[1]) { // If it starts with foreground
        fast[in].inc[1]--;
      }
      if (!fast[in].inc[1]) { // A pair of segments was exhausted
        fast[in].nt -= 2;
        memmove(&fast[in].inc[0], &fast[in].inc[2], (max_inc - 2) * 2);

        if (fast[in].nt < max_inc && fast[in].nt + 2 >= max_inc) {
          process_scan(iscroll[snum].al - n);
          goto cont_loop;
        }
      }
    }

    // 2. Add the pixel on the right ...

    if (*si) {               // If incoming pixel is foreground
      if (fast[in].nt & 1) { // If the segment ends with foreground
        if (fast[in].nt < max_inc)
          fast[in].inc[fast[in].nt]++;
      } else { // If it ends with background
        if (++fast[in].nt < max_inc)
          fast[in].inc[fast[in].nt] = 1;
      }
    } else {                 // If incoming pixel is background
      if (fast[in].nt & 1) { // If the segment ends with foreground
        if (++fast[in].nt < max_inc)
          fast[in].inc[fast[in].nt] = 1;
      } else { // If it ends with background
        if (fast[in].nt < max_inc)
          fast[in].inc[fast[in].nt]++;
      }
    }

cont_loop:

    if (!--block)
      si -= iscroll[snum].an * (iscroll[snum].al - 1);
    else
      si += iscroll[snum].an;

    if (++in == iscroll[snum].al)
      in = 0;

  } while (--n);
}

//----------------------------------------------------------------------------
// Shift all scanlines left by one pixel (exits on the right)
//----------------------------------------------------------------------------

void process_scan_left(void) {
  int n, in, block;
  byte *si;

  block = iscroll[snum].block1;
  si = iscroll[snum].sscr1 + iscroll[snum].an - 1; // End of the first scanline
  in = iscroll[snum].iscan;
  n = iscroll[snum].al;
  fast = iscroll[snum].fast;

  do {
    // 1. Remove the pixel on the right ...

    if (fast[in].nt < max_inc) {
      if (!--fast[in].inc[fast[in].nt])
        fast[in].nt--;
    } else {                 // Complex case
      if (fast[in].nt & 1) { // If the line ended with foreground ...
        if (!*si) {          // ... and now ends with background
          if (--fast[in].nt == max_inc - 1) {
            process_scan(iscroll[snum].al - n);
            goto cont_loop;
          }
        }
      } else {     // If the line ended with background ...
        if (*si) { // ... and now ends with foreground
          if (--fast[in].nt == max_inc - 1) {
            process_scan(iscroll[snum].al - n);
            goto cont_loop;
          }
        }
      }
    }

    // 2. Add the pixel on the left ...

    if (*(si - iscroll[snum].an + 1)) { // If incoming pixel is foreground
      if (!fast[in].inc[0]) {           // If the segment begins with foreground
        fast[in].inc[1]++;
      } else { // If it begins with background
        fast[in].nt += 2;
        if (fast[in].nt < max_inc) {
          memmove(&fast[in].inc[2], &fast[in].inc[0], (max_inc - 2) * 2);
          fast[in].inc[0] = 0;
          fast[in].inc[1] = 1;
        }
      }
    } else { // If incoming pixel is background
      fast[in].inc[0]++;
    }

cont_loop:

    if (!--block)
      si -= iscroll[snum].an * (iscroll[snum].al - 1);
    else
      si += iscroll[snum].an;

    if (++in == iscroll[snum].al)
      in = 0;

  } while (--n);
}

//----------------------------------------------------------------------------
//      Move_scroll(snum,plano,ix,iy)
//----------------------------------------------------------------------------

byte *scroll_right(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y);

byte *scroll_left(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y);

byte *scroll_down(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y);

byte *scroll_up(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y);

void move_scroll(int plano, int x, int y) {
  int ix, iy;

  if (snum < 0 || snum > 9)
    return;

  if (plano < 0 || plano >= iscroll[snum].on)
    return;

  if (plano == 0) {
    ix = iscroll[snum].map1_x;
    iy = iscroll[snum].map1_y;
    x += iscroll[snum].map1_x;
    y += iscroll[snum].map1_y;
    if (!(iscroll[snum].map_flags & 1)) {
      if (x < 0)
        x = 0;
      else if (x + iscroll[snum].an >= iscroll[snum].map1_an)
        x = iscroll[snum].map1_an - iscroll[snum].an;
    }
    if (!(iscroll[snum].map_flags & 2)) {
      if (y < 0)
        y = 0;
      else if (y + iscroll[snum].al >= iscroll[snum].map1_al)
        y = iscroll[snum].map1_al - iscroll[snum].al;
    }
  } else {
    ix = iscroll[snum].map2_x;
    iy = iscroll[snum].map2_y;
    x += iscroll[snum].map2_x;
    y += iscroll[snum].map2_y;
    if (!(iscroll[snum].map_flags & 4)) {
      if (x < 0)
        x = 0;
      else if (x + iscroll[snum].an >= iscroll[snum].map2_an)
        x = iscroll[snum].map2_an - iscroll[snum].an;
    }
    if (!(iscroll[snum].map_flags & 8)) {
      if (y < 0)
        y = 0;
      else if (y + iscroll[snum].al >= iscroll[snum].map2_al)
        y = iscroll[snum].map2_al - iscroll[snum].al;
    }
  }

  ix = x - ix;
  iy = y - iy;

  /////////////////////////////////////////////////////////////////////
  if (abs(ix) > iscroll[snum].an / 2 || abs(iy) > iscroll[snum].al / 2) {
    if (plano == 0) {
      ix += iscroll[snum].map1_x;
      iy += iscroll[snum].map1_y;
    } else {
      ix += iscroll[snum].map2_x;
      iy += iscroll[snum].map2_y;
    }
    set_scroll(plano, ix, iy);
    return;
  }
  /////////////////////////////////////////////////////////////////////

  if (plano == 0) {
    iscroll[snum].map1_x = x;
    iscroll[snum].map1_y = y;
  } else {
    iscroll[snum].map2_x = x;
    iscroll[snum].map2_y = y;
  }

  x -= ix;
  y -= iy;

  if (ix > 0)
    do {
      if (plano == 0) {
        iscroll[snum].sscr1 =
            scroll_right(iscroll[snum]._sscr1, iscroll[snum].sscr1, &iscroll[snum].block1,
                         iscroll[snum].map1, iscroll[snum].map1_an, iscroll[snum].map1_al, x++, y);
        if (iscroll[snum].on == 2) {
          process_scan_right(); // New pixel enters from the right (exits left)
        }
      } else {
        iscroll[snum].sscr2 =
            scroll_right(iscroll[snum]._sscr2, iscroll[snum].sscr2, &iscroll[snum].block2,
                         iscroll[snum].map2, iscroll[snum].map2_an, iscroll[snum].map2_al, x++, y);
      }
    } while (--ix);

  if (ix < 0)
    do {
      if (plano == 0) {
        iscroll[snum].sscr1 =
            scroll_left(iscroll[snum]._sscr1, iscroll[snum].sscr1, &iscroll[snum].block1,
                        iscroll[snum].map1, iscroll[snum].map1_an, iscroll[snum].map1_al, x--, y);
        if (iscroll[snum].on == 2) {
          process_scan_left(); // New pixel enters from the left (exits right)
        }
      } else {
        iscroll[snum].sscr2 =
            scroll_left(iscroll[snum]._sscr2, iscroll[snum].sscr2, &iscroll[snum].block2,
                        iscroll[snum].map2, iscroll[snum].map2_an, iscroll[snum].map2_al, x--, y);
      }
    } while (++ix);

  if (iy > 0)
    do {
      if (plano == 0) {
        iscroll[snum].sscr1 =
            scroll_down(iscroll[snum]._sscr1, iscroll[snum].sscr1, &iscroll[snum].block1,
                        iscroll[snum].map1, iscroll[snum].map1_an, iscroll[snum].map1_al, x, y++);
        if (iscroll[snum].on == 2) {
          if (++iscroll[snum].iscan == iscroll[snum].al)
            iscroll[snum].iscan = 0;
          process_scan(iscroll[snum].al - 1);
        }
      } else {
        iscroll[snum].sscr2 =
            scroll_down(iscroll[snum]._sscr2, iscroll[snum].sscr2, &iscroll[snum].block2,
                        iscroll[snum].map2, iscroll[snum].map2_an, iscroll[snum].map2_al, x, y++);
      }
    } while (--iy);

  if (iy < 0)
    do {
      if (plano == 0) {
        iscroll[snum].sscr1 =
            scroll_up(iscroll[snum]._sscr1, iscroll[snum].sscr1, &iscroll[snum].block1,
                      iscroll[snum].map1, iscroll[snum].map1_an, iscroll[snum].map1_al, x, y--);
        if (iscroll[snum].on == 2) {
          if (iscroll[snum].iscan-- == 0)
            iscroll[snum].iscan = iscroll[snum].al - 1;
          process_scan(0);
        }
      } else {
        iscroll[snum].sscr2 =
            scroll_up(iscroll[snum]._sscr2, iscroll[snum].sscr2, &iscroll[snum].block2,
                      iscroll[snum].map2, iscroll[snum].map2_an, iscroll[snum].map2_al, x, y--);
      }
    } while (++iy);

  if (plano) {
    (scroll + snum)->x1 = iscroll[snum].map2_x;
    (scroll + snum)->y1 = iscroll[snum].map2_y;
  } else {
    (scroll + snum)->x0 = iscroll[snum].map1_x;
    (scroll + snum)->y0 = iscroll[snum].map1_y;
  }
}

//----------------------------------------------------------------------------
// Single-direction scroll functions
//----------------------------------------------------------------------------

byte *scroll_right(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y) {
  // Scroll window buffer (_buf, iscroll[snum].an, iscroll[snum].al(+1))
  // Number of scanlines in the first block (*block)
  // Pointer to current scroll start (buf)
  // Map at (map,an,al), scroll origin coordinates (x,y)

  int n, m, b;
  byte *ret;

  x += iscroll[snum].an;

  if (x < 0)
    x = (an - 1) - ((-x - 1) % an);
  else
    x = x % an;
  if (y < 0)
    y = (al - 1) - ((-y - 1) % al);
  else
    y = y % al;

  m = al - y;
  map += y * an + x; // First pixel in the map

  buf++;
  if (buf == _buf + iscroll[snum].an * iscroll[snum].al) {
    memcpy(_buf, _buf + iscroll[snum].an * iscroll[snum].al, iscroll[snum].an);
    buf = _buf;
    *block = iscroll[snum].al;
  } else if (((int)(buf - _buf) % iscroll[snum].an) == 0) {
    memcpy(_buf, _buf + iscroll[snum].an * iscroll[snum].al, iscroll[snum].an);
    (*block)--;
  }

  ret = buf;

  buf += iscroll[snum].an - 1;
  b = *block;

  n = iscroll[snum].al;
  do {
    *buf = *map;
    if (--b == 0)
      buf -= iscroll[snum].an * (iscroll[snum].al - 1);
    else
      buf += iscroll[snum].an;
    if (--m == 0) {
      map -= an * (al - 1);
      m = al;
    } else
      map += an;
  } while (--n);

  return (ret);
}

byte *scroll_left(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y) {
  int n, m, b;
  byte *ret;

  x--;

  if (x < 0)
    x = (an - 1) - ((-x - 1) % an);
  else
    x = x % an;
  if (y < 0)
    y = (al - 1) - ((-y - 1) % al);
  else
    y = y % al;

  m = al - y;
  map += y * an + x; // First pixel in the map

  if (buf == _buf) {
    memcpy(_buf + iscroll[snum].an * iscroll[snum].al, _buf, iscroll[snum].an);
    buf = _buf + iscroll[snum].an * iscroll[snum].al - 1;
    *block = 1;
  } else if (((int)(buf - _buf) % iscroll[snum].an) == 0) {
    buf--;
    memcpy(_buf + iscroll[snum].an * iscroll[snum].al, _buf, iscroll[snum].an);
    (*block)++;
  } else
    buf--;

  ret = buf;

  b = *block;

  n = iscroll[snum].al;
  do {
    *buf = *map;
    if (--b == 0)
      buf -= iscroll[snum].an * (iscroll[snum].al - 1);
    else
      buf += iscroll[snum].an;
    if (--m == 0) {
      map -= an * (al - 1);
      m = al;
    } else
      map += an;
  } while (--n);

  return (ret);
}

byte *scroll_down(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y) {
  int n, m;
  byte *ret;

  y += iscroll[snum].al;

  if (x < 0)
    x = (an - 1) - ((-x - 1) % an);
  else
    x = x % an;
  if (y < 0)
    y = (al - 1) - ((-y - 1) % al);
  else
    y = y % al;

  m = an - x;
  map += y * an + x; // First pixel in the map

  buf += iscroll[snum].an;
  if (buf >= _buf + iscroll[snum].an * iscroll[snum].al) {
    buf -= iscroll[snum].an * iscroll[snum].al;
    *block = iscroll[snum].al;
  } else
    (*block)--;

  ret = buf;

  buf += (iscroll[snum].al - 1) * iscroll[snum].an;
  if (buf >= _buf + iscroll[snum].an * iscroll[snum].al)
    buf -= iscroll[snum].an * iscroll[snum].al;

  n = iscroll[snum].an;
  do {
    *buf++ = *map;
    if (--m == 0) {
      map -= an - 1;
      m = an;
    } else
      map++;
  } while (--n);

  return (ret);
}

byte *scroll_up(byte *_buf, byte *buf, int *block, byte *map, int an, int al, int x, int y) {
  int n, m;
  byte *ret;

  y--;

  if (x < 0)
    x = (an - 1) - ((-x - 1) % an);
  else
    x = x % an;
  if (y < 0)
    y = (al - 1) - ((-y - 1) % al);
  else
    y = y % al;

  m = an - x;
  map += y * an + x; // First pixel in the map

  buf -= iscroll[snum].an;
  if (buf < _buf) {
    buf += iscroll[snum].an * iscroll[snum].al;
    *block = 1;
  } else
    (*block)++;

  ret = buf;

  n = iscroll[snum].an;
  do {
    *buf++ = *map;
    if (--m == 0) {
      map -= an - 1;
      m = an;
    } else
      map++;
  } while (--n);

  return (ret);
}

//----------------------------------------------------------------------------
//      External function to render graphics (for the language's PUT command)
//----------------------------------------------------------------------------

void put_sprite(int file, int graph, int x, int y, int angle, int size, int flags, int reg,
                byte *cop, int copan, int copal) {
  int *ptr;
  byte *si;
  int an, al; // Graphic width and height on screen
  int xg, yg;
  byte *_saved_buffer = screen_buffer;
  int _saved_width = vga_width, _saved_height = vga_height;

  if (file > max_fpgs || file < 0) {
    e(109);
    return;
  }
  if (file)
    max_grf = 1000;
  else
    max_grf = 2000;
  if (graph <= 0 || graph >= max_grf) {
    e(110);
    return;
  }
  if (g[file].grf == NULL) {
    e(111);
    return;
  }

  if (reg < 0 || reg >= max_region) {
    clipx0 = 0;
    clipx1 = copan;
    clipy0 = 0;
    clipy1 = copal;
  } else {
    clipx0 = region[reg].x0;
    clipx1 = region[reg].x1;
    clipy0 = region[reg].y0;
    clipy1 = region[reg].y1;
    if (clipx0 < 0) {
      clipx0 = 0;
    }
    if (clipx1 > copan) {
      clipx1 = copan;
    }
    if (clipx0 >= copan || clipx1 <= 0)
      return;
    if (clipy0 < 0) {
      clipy0 = 0;
    }
    if (clipy1 > copal) {
      clipy1 = copal;
    }
    if (clipy0 >= copal || clipy1 <= 0)
      return;
  }

  if ((ptr = g[file].grf[graph]) != NULL) {
    screen_buffer = cop;
    vga_width = copan;
    vga_height = copal;

    an = ptr[13];
    al = ptr[14];
    si = (byte *)ptr + 64 + ptr[15] * 4;

    if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
      xg = ptr[13] / 2;
      yg = ptr[14] / 2;
    } else {
      xg = *((word *)ptr + 32);
      yg = *((word *)ptr + 33);
    }

    if (angle) {
      sp_rotated(si, x, y, an, al, xg, yg, angle, size, flags);
    } else if (size != 100) {
      sp_scaled(si, x, y, an, al, xg, yg, size, flags);
    } else {
      if (flags & 1) {
        xg = an - 1 - xg;
      }
      x -= xg;
      if (flags & 2) {
        yg = al - 1 - yg;
      }
      y -= yg;
      if (x >= clipx0 && x + an <= clipx1 && y >= clipy0 &&
          y + al <= clipy1) // Fully visible sprite
        sp_normal(si, x, y, an, al, flags);
      else if (x < clipx1 && y < clipy1 && x + an > clipx0 && y + al > clipy0) // Clipped sprite
        sp_clipped(si, x, y, an, al, flags);
      x0s = x;
      x1s = x + an - 1;
      y0s = y;
      y1s = y + al - 1;
    }

    screen_buffer = _saved_buffer;
    vga_width = _saved_width;
    vga_height = _saved_height;

  } else
    e(121);
}

//----------------------------------------------------------------------------
//      Render a process's sprite
//----------------------------------------------------------------------------

/* Render the sprite for process 'ide': looks up its graphic from the FPG,
 * applies animation tables (_XGraph), clipping region, and coordinate
 * transforms, then dispatches to sp_normal/sp_clipped/sp_scaled/sp_rotated
 * depending on angle, scale, and clipping. Color 0 is always transparent.
 */
void paint_sprite(void) { // Render a sprite (if visible), using mem[ide+ ... ]

  int *ptr;
  byte *si;
  int x, y, an, al; // Graphic width and height on screen
  int xg, yg;
  int n, m;

  if (mem[ide + _File] > max_fpgs || mem[ide + _File] < 0)
    return;
  if ((n = mem[ide + _XGraph]) > 0) {
    m = mem[n];
    if (m < 1 || m > 256)
      return;
    while (mem[ide + _Angle] >= 2 * pi)
      mem[ide + _Angle] -= 2 * pi;
    while (mem[ide + _Angle] < 0)
      mem[ide + _Angle] += 2 * pi;
    mem[ide + _Flags] &= 254;
    mem[ide + _Graph] = ((mem[ide + _Angle] + (2 * pi) / (m * 2)) * m) / (2 * pi);
    if (mem[ide + _Graph] >= m)
      mem[ide + _Graph] = 0;
    if ((mem[ide + _Graph] = mem[n + 1 + mem[ide + _Graph]]) < 0) {
      mem[ide + _Graph] = -mem[ide + _Graph];
      mem[ide + _Flags] |= 1;
    }
  }
  if (mem[ide + _File])
    max_grf = 1000;
  else
    max_grf = 2000;
  if (mem[ide + _Graph] <= 0 || mem[ide + _Graph] >= max_grf)
    return;
  if (g[mem[ide + _File]].grf == NULL)
    return;

  x = mem[ide + _Region];

  if (mem[ide + _Ctype] == 1 && iscroll[snum].on > 0) {
    clipx0 = iscroll[snum].x;
    clipx1 = iscroll[snum].x + iscroll[snum].an;
    clipy0 = iscroll[snum].y;
    clipy1 = iscroll[snum].y + iscroll[snum].al;
  } else if (x < 0 || x >= max_region) {
    clipx0 = 0;
    clipx1 = vga_width;
    clipy0 = 0;
    clipy1 = vga_height;
  } else {
    clipx0 = region[x].x0;
    clipx1 = region[x].x1;
    clipy0 = region[x].y0;
    clipy1 = region[x].y1;
  }

  if ((ptr = g[mem[ide + _File]].grf[mem[ide + _Graph]]) != NULL) {
    x = mem[ide + _X];
    y = mem[ide + _Y];
    if (mem[ide + _Resolution] > 0) {
      x /= mem[ide + _Resolution];
      y /= mem[ide + _Resolution];
    }

    if (mem[ide + _Ctype] == 1) {
      x += iscroll[snum].x - iscroll[snum].map1_x;
      y += iscroll[snum].y - iscroll[snum].map1_y;
    }

    an = ptr[13];
    al = ptr[14];
    si = (byte *)ptr + 64 + ptr[15] * 4;

    if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
      xg = ptr[13] / 2;
      yg = ptr[14] / 2;
    } else {
      xg = *((word *)ptr + 32);
      yg = *((word *)ptr + 33);
    }

    if (putsprite != NULL) {
      putsprite(si, x, y, an, al, xg, yg, mem[ide + _Angle], mem[ide + _Size], mem[ide + _Flags]);
    } else if (mem[ide + _Angle] && mem[ide + _XGraph] <= 0) {
      sp_rotated(si, x, y, an, al, xg, yg, mem[ide + _Angle], mem[ide + _Size], mem[ide + _Flags]);
      save_region();
    } else if (mem[ide + _Size] != 100) {
      sp_scaled(si, x, y, an, al, xg, yg, mem[ide + _Size], mem[ide + _Flags]);
      save_region();
    } else {
      if (mem[ide + _Flags] & 1) {
        xg = an - 1 - xg;
      }
      x -= xg;
      if (mem[ide + _Flags] & 2) {
        yg = al - 1 - yg;
      }
      y -= yg;
      if (x >= clipx0 && x + an <= clipx1 && y >= clipy0 &&
          y + al <= clipy1) // Fully visible sprite
        sp_normal(si, x, y, an, al, mem[ide + _Flags]);
      else if (x < clipx1 && y < clipy1 && x + an > clipx0 && y + al > clipy0) // Clipped sprite
        sp_clipped(si, x, y, an, al, mem[ide + _Flags]);
      x0s = x;
      x1s = x + an - 1;
      y0s = y;
      y1s = y + al - 1;
      save_region();
    }
  }
}

void save_region(void) {
  if (mem[ide + _Ctype] == 0) {
    if (clipx0 > x0s) {
      x0s = clipx0;
    }
    if (clipx1 < x1s) {
      x1s = clipx1;
    }
    if (clipy0 > y0s) {
      y0s = clipy0;
    }
    if (clipy1 < y1s) {
      y1s = clipy1;
    }
    if (x1s < x0s || y1s < y0s)
      return;
    mem[ide + _x0] = x0s;
    mem[ide + _y0] = y0s;
    mem[ide + _x1] = x1s;
    mem[ide + _y1] = y1s;
  }
}

//----------------------------------------------------------------------------
// Sprite - [mirrored] [ghost]
//----------------------------------------------------------------------------

void sp_normal(byte *p, int x, int y, int an, int al, int flags) {
  byte *q = screen_buffer + y * vga_width + x;
  int ancho = an;

  switch (flags & 7) {
  case 0: //--
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p++;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
    } while (--al);
    break;
  case 1: //h-
    p += an - 1;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p--;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
      p += an * 2;
    } while (--al);
    break;
  case 2: //-v
    p += (al - 1) * an;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p++;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
      p -= an * 2;
    } while (--al);
    break;
  case 3: //hv
    p += al * an - 1;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p--;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
    } while (--al);
    break;
  case 4: //-- Ghost
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p++;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
    } while (--al);
    break;
  case 5: //h- Ghost
    p += an - 1;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p--;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
      p += an * 2;
    } while (--al);
    break;
  case 6: //-v Ghost
    p += (al - 1) * an;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p++;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
      p -= an * 2;
    } while (--al);
    break;
  case 7: //hv Ghost
    p += al * an - 1;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p--;
        q++;
      } while (--an);
      q += vga_width - (an = ancho);
    } while (--al);
    break;
  }
}

//----------------------------------------------------------------------------
// Sprite - clipped [mirrored] [ghost]
//----------------------------------------------------------------------------

void sp_clipped(byte *p, int x, int y, int an, int al, int flags) {
  byte *q = screen_buffer + y * vga_width + x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x < clipx0)
    salta_x = clipx0 - x;
  else
    salta_x = 0;
  if (x + an > clipx1)
    resto_x = x + an - clipx1;
  else
    resto_x = 0;
  long_x = an - salta_x - resto_x;

  if (y < clipy0)
    salta_y = clipy0 - y;
  else
    salta_y = 0;
  if (y + al > clipy1)
    resto_y = y + al - clipy1;
  else
    resto_y = 0;
  long_y = al - salta_y - resto_y;

  switch (flags & 7) {
  case 0: //--
    p += an * salta_y + salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    an = long_x;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p++;
        q++;
      } while (--an);
      q += vga_width - (an = long_x);
      p += resto_x;
    } while (--long_y);
    break;
  case 1: //h-
    p += an * salta_y + an - 1 - salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p--;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p += an + long_x;
    } while (--long_y);
    break;
  case 2: //-v
    p += (al - 1) * an - an * salta_y + salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p++;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p += resto_x - an * 2;
    } while (--long_y);
    break;
  case 3: //hv
    p += al * an - 1 - an * salta_y - salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        if (*p) {
          *q = *p;
        }
        p--;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p -= resto_x;
    } while (--long_y);
    break;
  case 4: //-- Ghost
    p += an * salta_y + salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    an = long_x;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p++;
        q++;
      } while (--an);
      q += vga_width - (an = long_x);
      p += resto_x;
    } while (--long_y);
    break;
  case 5: //h- Ghost
    p += an * salta_y + an - 1 - salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p--;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p += an + long_x;
    } while (--long_y);
    break;
  case 6: //-v Ghost
    p += (al - 1) * an - an * salta_y + salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p++;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p += resto_x - an * 2;
    } while (--long_y);
    break;
  case 7: //hv Ghost
    p += al * an - 1 - an * salta_y - salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        *q = ghost[(*p << 8) + *q];
        p--;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p -= resto_x;
    } while (--long_y);
    break;
  }
}

//----------------------------------------------------------------------------
// Sprite - scaled [clipped] [mirrored] [ghost]
//----------------------------------------------------------------------------

void sp_scaled(byte *old_si, int x, int y, int an, int al, int xg, int yg, int size, int flags) {
  int salta_x, long_x, resto_x; // In screen coordinates
  int salta_y, long_y, resto_y;
  int xr, ixr, yr, iyr, old_xr, old_an;
  byte *si, *di, c;

  if (flags & 1)
    x0s = x - ((an - 1 - xg) * size) / 100;
  else
    x0s = x - (xg * size) / 100;
  if (flags & 2)
    y0s = y - ((al - 1 - yg) * size) / 100;
  else
    y0s = y - (yg * size) / 100;
  x1s = x0s + (an * size) / 100 - 1;
  y1s = y0s + (al * size) / 100 - 1;

  if (x1s < x0s || y1s < y0s)
    return;

  ixr = (float)(an * 256) / (float)(x1s - x0s + 1);
  iyr = (float)(al * 256) / (float)(y1s - y0s + 1);

  if (x1s < clipx0 || y1s < clipy0 || x0s >= clipx1 || y0s >= clipy1)
    return;

  di = screen_buffer + y0s * vga_width + x0s;

  if (x0s < clipx0)
    salta_x = clipx0 - x0s;
  else
    salta_x = 0;
  if (x1s >= clipx1)
    resto_x = x1s - clipx1 + 1;
  else
    resto_x = 0;
  long_x = (an * size) / 100 - salta_x - resto_x;

  if (y0s < clipy0)
    salta_y = clipy0 - y0s;
  else
    salta_y = 0;
  if (y1s >= clipy1)
    resto_y = y1s - clipy1 + 1;
  else
    resto_y = 0;
  long_y = (al * size) / 100 - salta_y - resto_y;

  if (flags & 1) {
    xr = an * 256 - salta_x * ixr - 1;
    ixr = -ixr;
  } else
    xr = salta_x * ixr;
  if (flags & 2) {
    yr = al * 256 - salta_y * iyr - 1;
    iyr = -iyr;
  } else
    yr = salta_y * iyr;

  old_xr = xr;
  old_an = an;
  di += vga_width * salta_y + salta_x;
  an = long_x;

  if (flags & 4)
    do {
      si = old_si + (yr >> 8) * old_an;
      do {
        c = *(si + (xr >> 8));
        *di = ghost[(c << 8) + *di];
        di++;
        xr += ixr;
      } while (--an);
      yr += iyr;
      xr = old_xr;
      di += vga_width - (an = long_x);
    } while (--long_y);
  else
    do {
      si = old_si + (yr >> 8) * old_an;
      do {
        if ((c = *(si + (xr >> 8))))
          *di = c;
        di++;
        xr += ixr;
      } while (--an);
      yr += iyr;
      xr = old_xr;
      di += vga_width - (an = long_x);
    } while (--long_y);
}

//----------------------------------------------------------------------------
// Sprite - rotated [scaled] [clipped] [mirrored] [ghost]
//----------------------------------------------------------------------------

void sp_rotated(byte *si, int x, int y, int an, int al, int xg, int yg, int ang, int size,
                int flags) {
  float d0, d1, d2, d3;
  float a0, a1, a2, a3;
  float a, s;
  int p[24];
  int h, hmin, hmax; // Minimum and maximum height
  int n, l0 = 0, l1; // Side 0 and side 1 (indices into p[])

  int hmax0, hmax1;
  union {
    int32_t l;
    int16_t w[2];
  } x0, x1, g0x = {0}, g1x = {0}, g0y = {0}, g1y = {0};
  int ix0 = 0, ix1 = 0, ig0x = 0, ig1x = 0, ig0y = 0, ig1y = 0, kk;
  byte *ptrcopia;

  a = (float)ang / radian;
  s = (float)size / 100;
  xg = -xg;
  yg = -yg;

  d0 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a0 = a;
  else
    a0 = a + (float)atan2(-yg, xg);
  xg = xg + an - 1;
  d1 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a1 = a;
  else
    a1 = a + (float)atan2(-yg, xg);
  yg = yg + al - 1;
  d2 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a2 = a;
  else
    a2 = a + (float)atan2(-yg, xg);
  xg = xg - an + 1;
  d3 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a3 = a;
  else
    a3 = a + (float)atan2(-yg, xg);

  if (flags & 1) {
    p[0] = x - (int)((float)cos(a0) * d0);
    p[2] = x - (int)((float)cos(a1) * d1);
    p[4] = x - (int)((float)cos(a2) * d2);
    p[6] = x - (int)((float)cos(a3) * d3);
  } else {
    p[0] = x + (int)((float)cos(a0) * d0);
    p[2] = x + (int)((float)cos(a1) * d1);
    p[4] = x + (int)((float)cos(a2) * d2);
    p[6] = x + (int)((float)cos(a3) * d3);
  }

  if (flags & 2) {
    p[1] = y - (int)(-(float)sin(a0) * d0);
    p[3] = y - (int)(-(float)sin(a1) * d1);
    p[5] = y - (int)(-(float)sin(a2) * d2);
    p[7] = y - (int)(-(float)sin(a3) * d3);
  } else {
    p[1] = y + (int)(-(float)sin(a0) * d0);
    p[3] = y + (int)(-(float)sin(a1) * d1);
    p[5] = y + (int)(-(float)sin(a2) * d2);
    p[7] = y + (int)(-(float)sin(a3) * d3);
  }

  memcpy(&p[8], &p[0], sizeof(p[0]) * 8);
  memcpy(&p[16], &p[0], sizeof(p[0]) * 8);

  x0s = hmin = 32767;
  x1s = hmax = -32768;
  n = 8;
  do {
    if (p[n] < x0s)
      x0s = p[n];
    if (p[n] > x1s)
      x1s = p[n];
    if (p[n + 1] < hmin) {
      l0 = n;
      hmin = p[n + 1];
    }
    if (p[n + 1] > hmax)
      hmax = p[n + 1];
    n += 2;
  } while (n < 16);
  y0s = hmin;
  y1s = hmax;

  l1 = l0;
  hmax0 = hmax1 = hmin;
  ptrcopia = screen_buffer + hmin * vga_width;

  h = hmin;
  do {
    if (h < hmax) {
      while (h == hmax0) {
        if ((hmax0 = p[l0 - 1]) != h) {
          x0.l = p[l0] << 16;
          ix0 = ((p[l0 - 2] << 16) - x0.l) / (hmax0 - h);
          if (ix0 < 0)
            x0.l += 65535;
          switch (l0 & 6) {
          case 0:
            g0y.l = 0;
            ig0y = (al << 16) / (hmax0 - h);
            g0x.l = 0;
            ig0x = 0;
            break;
          case 2:
            g0x.l = ((an - 1) << 16) + 65535;
            ig0x = (an << 16) / (h - hmax0);
            g0y.l = 0;
            ig0y = 0;
            break;
          case 4:
            g0y.l = ((al - 1) << 16) + 65535;
            ig0y = (al << 16) / (h - hmax0);
            g0x.l = ((an - 1) << 16) + 65535;
            ig0x = 0;
            break;
          case 6:
            g0x.l = 0;
            ig0x = (an << 16) / (hmax0 - h);
            g0y.l = ((al - 1) << 16) + 65535;
            ig0y = 0;
            break;
          }
        }
        l0 -= 2;
      }
      while (h == hmax1) {
        if ((hmax1 = p[l1 + 3]) != h) {
          x1.l = p[l1] << 16;
          ix1 = ((p[l1 + 2] << 16) - x1.l) / (hmax1 - h);
          if (ix1 < 0)
            x1.l += 65535;
          switch (l1 & 6) {
          case 0:
            g1x.l = 0;
            ig1x = (an << 16) / (hmax1 - h);
            g1y.l = 0;
            ig1y = 0;
            break;
          case 2:
            g1y.l = 0;
            ig1y = (al << 16) / (hmax1 - h);
            g1x.l = ((an - 1) << 16) + 65535;
            ig1x = 0;
            break;
          case 4:
            g1x.l = ((an - 1) << 16) + 65535;
            ig1x = (an << 16) / (h - hmax1);
            g1y.l = ((al - 1) << 16) + 65535;
            ig1y = 0;
            break;
          case 6:
            g1y.l = ((al - 1) << 16) + 65535;
            ig1y = (al << 16) / (h - hmax1);
            g1x.l = 0;
            ig1x = 0;
            break;
          }
        }
        l1 += 2;
      }
    } else {
      if (h == hmax0) {
        g0x.l = 0;
        g0y.l = 0;
        ig0x = 0;
        ig0y = 0;
        x0.l = p[l0] << 16;
        ix0 = 0;
        switch (l0 & 6) {
        case 2:
          g0x.l = ((an - 1) << 16) + 65535;
          break;
        case 4:
          g0y.l = ((al - 1) << 16) + 65535;
          g0x.l = ((an - 1) << 16) + 65535;
          break;
        case 6:
          g0y.l = ((al - 1) << 16) + 65535;
          break;
        }
      }
      if (h == hmax1) {
        g1x.l = 0;
        g1y.l = 0;
        ig1x = 0;
        ig1y = 0;
        x1.l = p[l1] << 16;
        ix1 = 0;
        switch (l1 & 6) {
        case 2:
          g1x.l = ((an - 1) << 16) + 65535;
          break;
        case 4:
          g1x.l = ((an - 1) << 16) + 65535;
          g1y.l = ((al - 1) << 16) + 65535;
          break;
        case 6:
          g1y.l = ((al - 1) << 16) + 65535;
          break;
        }
      }
    }

    if ((flags & 3) == 1 || (flags & 3) == 2) {
      kk = x0.l;
      x0.l = x1.l;
      x1.l = kk;
      kk = g0x.l;
      g0x.l = g1x.l;
      g1x.l = kk;
      kk = g0y.l;
      g0y.l = g1y.l;
      g1y.l = kk;
    }

    if (h < clipy1 && h >= clipy0 && x0.w[1] < clipx1 && x1.w[1] >= clipx0 && x1.w[1] > x0.w[1]) {
      if (x0.w[1] < clipx0) {
        if (x1.w[1] >= clipx1) {
          if (flags & 4)
            sp_scan_clipped_ghost(ptrcopia + clipx0, x1.w[1] - x0.w[1], clipx1 - clipx0 - 1,
                                  clipx0 - x0.w[1], si, an, g0x.l, g0y.l, g1x.l, g1y.l);
          else
            sp_scan_clipped(ptrcopia + clipx0, x1.w[1] - x0.w[1], clipx1 - clipx0 - 1,
                            clipx0 - x0.w[1], si, an, g0x.l, g0y.l, g1x.l, g1y.l);
        } else {
          if (flags & 4)
            sp_scan_clipped_ghost(ptrcopia + clipx0, x1.w[1] - x0.w[1], x1.w[1] - clipx0,
                                  clipx0 - x0.w[1], si, an, g0x.l, g0y.l, g1x.l, g1y.l);
          else
            sp_scan_clipped(ptrcopia + clipx0, x1.w[1] - x0.w[1], x1.w[1] - clipx0,
                            clipx0 - x0.w[1], si, an, g0x.l, g0y.l, g1x.l, g1y.l);
        }
      } else if (x1.w[1] >= clipx1) {
        if (flags & 4)
          sp_scan_clipped_ghost(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], clipx1 - 1 - x0.w[1], 0, si,
                                an, g0x.l, g0y.l, g1x.l, g1y.l);
        else
          sp_scan_clipped(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], clipx1 - 1 - x0.w[1], 0, si, an,
                          g0x.l, g0y.l, g1x.l, g1y.l);
      } else {
        if (flags & 4)
          sp_scan_ghost(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], si, an, g0x.l, g0y.l, g1x.l, g1y.l);
        else
          sp_scan(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], si, an, g0x.l, g0y.l, g1x.l, g1y.l);
      }
    }

    if ((flags & 3) == 1 || (flags & 3) == 2) {
      kk = x0.l;
      x0.l = x1.l;
      x1.l = kk;
      kk = g0x.l;
      g0x.l = g1x.l;
      g1x.l = kk;
      kk = g0y.l;
      g0y.l = g1y.l;
      g1y.l = kk;
    }

    x0.l += ix0;
    x1.l += ix1;
    g0x.l += ig0x;
    g1x.l += ig1x;
    g0y.l += ig0y;
    g1y.l += ig1y;

    ptrcopia += vga_width;

  } while (h++ < hmax);
}

//----------------------------------------------------------------------------
// Render a rotated and scaled sprite scanline, with clipping
//----------------------------------------------------------------------------

void sp_scan_clipped(byte *p, short n, short m, short o, byte *si, int an, int x0, int y0, int x1,
                     int y1) {
  union {
    int32_t l;
    int16_t w[2];
  } x, y;
  byte c;

  x.l = x0;
  y.l = y0;
  x0 = (x1 - x0) / n;
  y0 = (y1 - y0) / n;

  if (o) {
    x.l += x0 * o;
    y.l += y0 * o;
  }

  do {
    if ((c = *(si + x.w[1] + y.w[1] * an))) {
      *p = c;
    }
    p++;
    x.l += x0;
    y.l += y0;
  } while (m--);
}

void sp_scan_clipped_ghost(byte *p, short n, short m, short o, byte *si, int an, int x0, int y0,
                           int x1, int y1) {
  union {
    int32_t l;
    int16_t w[2];
  } x, y;
  byte c;

  x.l = x0;
  y.l = y0;
  x0 = (x1 - x0) / n;
  y0 = (y1 - y0) / n;

  if (o) {
    x.l += x0 * o;
    y.l += y0 * o;
  }

  do {
    c = *(si + x.w[1] + y.w[1] * an);
    *p = ghost[(c << 8) + *p];
    p++;
    x.l += x0;
    y.l += y0;
  } while (m--);
}

//----------------------------------------------------------------------------
// Render a rotated and scaled sprite scanline, without clipping
//----------------------------------------------------------------------------

void sp_scan(byte *p, short n, byte *si, int an, int x0, int y0, int x1, int y1) {
  union {
    int32_t l;
    int16_t w[2];
  } x, y;
  byte c;

  x.l = x0;
  y.l = y0;
  x0 = (x1 - x0) / n;
  y0 = (y1 - y0) / n;

  do {
    if ((c = *(si + x.w[1] + y.w[1] * an))) {
      *p = c;
    }
    p++;
    x.l += x0;
    y.l += y0;
  } while (n--);
}

void sp_scan_ghost(byte *p, short n, byte *si, int an, int x0, int y0, int x1, int y1) {
  union {
    int32_t l;
    int16_t w[2];
  } x, y;
  byte c;

  x.l = x0;
  y.l = y0;
  x0 = (x1 - x0) / n;
  y0 = (y1 - y0) / n;

  do {
    c = *(si + x.w[1] + y.w[1] * an);
    *p = ghost[(c << 8) + *p];
    p++;
    x.l += x0;
    y.l += y0;
  } while (n--);
}

//----------------------------------------------------------------------------
// Drawings
//----------------------------------------------------------------------------

void paint_drawings(void) {
  int x, y, an, al;
  int n = 0;

  do
    if (drawing[n].type) {
      if (drawing[n].x0 < drawing[n].x1) {
        x = drawing[n].x0;
        an = drawing[n].x1 - x + 1;
      } else {
        x = drawing[n].x1;
        an = drawing[n].x0 - x + 1;
      }
      if (drawing[n].y0 < drawing[n].y1) {
        y = drawing[n].y0;
        al = drawing[n].y1 - y + 1;
      } else {
        y = drawing[n].y1;
        al = drawing[n].y0 - y + 1;
      }

      clipx0 = region[drawing[n].region].x0;
      clipy0 = region[drawing[n].region].y0;
      clipx1 = region[drawing[n].region].x1;
      clipy1 = region[drawing[n].region].y1;

      if (x + an <= clipx0)
        continue;
      if (x >= clipx1)
        continue;
      if (y + al <= clipy0)
        continue;
      if (y >= clipy1)
        continue;

      line_fx = drawing[n].porcentaje;
      color = drawing[n].color;

      switch (drawing[n].type) {
      case 1:
        line(drawing[n].x0, drawing[n].y0, drawing[n].x1, drawing[n].y1);
        break;
      case 2:
        draw_box(x, y, an, al);
        break;
      case 3:
        draw_filled_box(x, y, an, al);
        break;
      case 4:
        draw_circle(0, x, y, x + an - 1, y + al - 1);
        break;
      case 5:
        draw_circle(1, x, y, x + an - 1, y + al - 1);
        break;
      }
    }
  while (++n < max_drawings);
}

//-----------------------------------------------------------------------------
//      Draw a rectangle using mode_rect, line_fx, and color
//-----------------------------------------------------------------------------

void draw_box(int x, int y, int an, int al) {
  line(x, y, x + an - 1, y);
  line(x, y + al - 1, x + an - 1, y + al - 1);
  al -= 2;
  y++;
  if (al > 0)
    do {
      line_pixel(x, y);
      line_pixel(x + an - 1, y++);
    } while (--al);
}

void draw_filled_box(int x, int y, int an, int al) {
  do {
    line(x, y, x + an - 1, y);
    y++;
  } while (--al);
}

//-----------------------------------------------------------------------------
//      Draw a circle (using line_fx and color)
//-----------------------------------------------------------------------------

void draw_circle(int relleno, int x0, int y0, int x1, int y1) {
  int p[2048];   // Points on the circumference
  double cx, rx; // Center and radius of the circle
  int an, al;    // Width and height
  double y, ymed, nsin;
  int n, xa, xb, ya, yb;

  an = x1 - x0 + 1;
  al = y1 - y0 + 1;
  if (al > 1024)
    return;

  rx = (double)(an - 1) / 2.0;
  cx = rx + x0 + 0.5;
  y = (double)y0 + 0.5;
  ymed = (double)(y1 + y0 + 1) / 2.0;

  n = 0;
  do {
    nsin = 1 - (y - y0) / (ymed - y0);
    xb = cx + cos(asin(nsin)) * rx;
    xa = x0 + (x1 - xb);
    ya = y;
    yb = y1 - (ya - y0);
    if (!(n && p[n - 2] == ya)) {
      p[n++] = xa;
      p[n++] = xb;
      p[n++] = ya;
      p[n++] = yb;
    }
  } while (y++ < ymed);

  if (n > 4)
    if (p[n - 1] == p[n - 6])
      n -= 4;
  if (al > 2 && an > 2 && p[0] == p[4] && p[0] + 1 < p[1]) {
    p[0] = p[4] + 1;
    p[1] = p[5] - 1;
  }
  if (al == 2) {
    p[0] = x0;
    p[1] = x1;
    p[4] = x0;
    p[5] = x1;
  }

  if (relleno) {
    do {
      yb = p[--n];
      ya = p[--n];
      xb = p[--n];
      xa = p[--n];
      line(xa, ya, xb, ya);
      if (yb != ya)
        line(xa, yb, xb, yb);
    } while (n);
  } else {
    do {
      yb = p[--n];
      ya = p[--n];
      xb = p[--n];
      xa = p[--n];
      if (n) {
        if (p[n - 3] + 1 >= xb) {
          line_pixel(xa, ya);
          if (xb != xa)
            line_pixel(xb, ya);
          if (yb != ya) {
            line_pixel(xa, yb);
            if (xb != xa)
              line_pixel(xb, yb);
          }
        } else {
          line(xa, ya, p[n - 4], ya);
          line(p[n - 3] + 1, ya, xb, ya);
          if (yb != ya) {
            line(xa, yb, p[n - 4], yb);
            line(p[n - 3] + 1, yb, xb, yb);
          }
        }
      } else {
        line(xa, ya, xb, ya);
        if (yb != ya)
          line(xa, yb, xb, yb);
      }
    } while (n);
  }
}

//----------------------------------------------------------------------------
// Graphics primitive functions
//----------------------------------------------------------------------------

void line(int x0, int y0, int x1, int y1) {
  int dx, dy, a, b, d, x, y;

  if (x0 > x1) {
    x = x1;
    dx = x0 - x1;
  } else {
    x = x0;
    dx = x1 - x0;
  }
  if (y0 > y1) {
    y = y1;
    dy = y0 - y1;
  } else {
    y = y0;
    dy = y1 - y0;
  }

  if (!dx && !dy)
    line_pixel(x0, y0);
  else {
    line_pixel(x0, y0);
    if (dy <= dx) {
      if (x0 <= x1) {
        d = 2 * dy - dx;
        a = 2 * dy;
        b = 2 * (dy - dx);
        x = x0;
        y = y0;
        if (y0 <= y1)
          while (x < x1) {
            if (d <= 0) {
              d += a;
              x++;
            } else {
              d += b;
              x++;
              y++;
            }
            line_pixel(x, y);
          }
        else
          while (x < x1) {
            if (d <= 0) {
              d += a;
              x++;
            } else {
              d += b;
              x++;
              y--;
            }
            line_pixel(x, y);
          }
      } else {
        d = 2 * dy - dx;
        a = 2 * dy;
        b = 2 * (dy - dx);
        x = x0;
        y = y0;
        if (y0 <= y1)
          while (x > x1) {
            if (d <= 0) {
              d += a;
              x--;
            } else {
              d += b;
              x--;
              y++;
            }
            line_pixel(x, y);
          }
        else
          while (x > x1) {
            if (d <= 0) {
              d += a;
              x--;
            } else {
              d += b;
              x--;
              y--;
            }
            line_pixel(x, y);
          }
      }
    } else {
      if (y0 <= y1) {
        d = 2 * dx - dy;
        a = 2 * dx;
        b = 2 * (dx - dy);
        x = x0;
        y = y0;
        if (x0 <= x1)
          while (y < y1) {
            if (d <= 0) {
              d += a;
              y++;
            } else {
              d += b;
              y++;
              x++;
            }
            line_pixel(x, y);
          }
        else
          while (y < y1) {
            if (d <= 0) {
              d += a;
              y++;
            } else {
              d += b;
              y++;
              x--;
            }
            line_pixel(x, y);
          }
      } else {
        d = 2 * dx - dy;
        a = 2 * dx;
        b = 2 * (dx - dy);
        x = x0;
        y = y0;
        if (x0 <= x1)
          while (y > y1) {
            if (d <= 0) {
              d += a;
              y--;
            } else {
              d += b;
              y--;
              x++;
            }
            line_pixel(x, y);
          }
        else
          while (y > y1) {
            if (d <= 0) {
              d += a;
              y--;
            } else {
              d += b;
              y--;
              x--;
            }
            line_pixel(x, y);
          }
      }
    }
  }
}

//----------------------------------------------------------------------------
// Draw a pixel with opacity level line_fx (0..15)
//----------------------------------------------------------------------------

void line_pixel(int x, int y) {
  byte a, b, c, d;
  int n, color256;
  byte *p;

  if (x >= clipx0 && y >= clipy0 && x < clipx1 && y < clipy1) {
    p = screen_buffer + x + y * vga_width;

    switch (line_fx) {
    case 0:
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      c = *(ghost + n + b);
      d = *(ghost + n + c);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 1:
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      c = *(ghost + n + b);
      if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 2:
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      c = *(ghost + n + b);
      d = *(ghost + b + c * 256);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 3: // *** 25% ****
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 4:
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      c = *(ghost + a + b * 256);
      d = *(ghost + b + c * 256);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 5:
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      c = *(ghost + a + b * 256);
      if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 6:
      n = (int)*p * 256;
      a = *(ghost + n + color);
      b = *(ghost + n + a);
      c = *(ghost + a + b * 256);
      d = *(ghost + a + c * 256);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 7: // *** 50% ****
      n = (int)*p * 256;
      a = *(ghost + n + color);
      if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 8:
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      c = *(ghost + a + b * 256);
      d = *(ghost + a + c * 256);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 9:
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      c = *(ghost + a + b * 256);
      if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 10:
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      c = *(ghost + a + b * 256);
      d = *(ghost + b + c * 256);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 11: // *** 75% ****
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 12:
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      c = *(ghost + color256 + b);
      d = *(ghost + b + c * 256);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 13:
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      c = *(ghost + color256 + b);
      if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 14:
      n = (int)*p * 256;
      color256 = color * 256;
      a = *(ghost + n + color);
      b = *(ghost + color256 + a);
      c = *(ghost + color256 + b);
      d = *(ghost + color256 + c);
      if (d != *p)
        *p = d;
      else if (c != *p)
        *p = c;
      else if (b != *p)
        *p = b;
      else if (a != *p)
        *p = a;
      else
        *p = color;
      break;

    case 15: // Solid color
      *p = color;
      break;
    }
  }
}


//----------------------------------------------------------------------------
// Text rendering
//----------------------------------------------------------------------------

void text_out(char *ptr, int x, int y) {
  int an, al;
  byte *ptr2;

  ptr2 = (byte *)ptr;
  an = 0;
  while (*ptr2) {
    if (fnt[*ptr2].ancho == 0) {
      an += f_i[0].espacio;
      ptr2++;
    } else {
      an += fnt[*ptr2++].ancho;
    }
  }

  fnt = (TABLAFNT *)((byte *)fonts[0] + 1356);

  al = f_i[0].alto;
  while (*ptr && x + fnt[*ptr].ancho <= 0)
    if (fnt[*ptr].ancho == 0) {
      x += f_i[0].espacio;
      ptr++;
    } else {
      x = x + fnt[*ptr].ancho;
      ptr++;
    }
  while (*ptr && x < vga_width)
    if (fnt[*ptr].ancho == 0) {
      x += f_i[0].espacio;
      ptr++;
    } else {
      text_clipped(fonts[0] + fnt[*ptr].offset, x, y + fnt[*ptr].incY, fnt[*ptr].ancho,
                   fnt[*ptr].alto);
      x = x + fnt[*ptr].ancho;
      ptr++;
    }
}

void checkpal_font(int ifonts);

void paint_texts(int n) { // E: texts[]

  int x, y, an, al;
  int fuente;

  byte *ptr = NULL, *ptr2;
  byte numero[32];

  do
    if (texts[n].font) {
      if (n == max_texts) {
        if (reloj & 64)
          break;
        else
          ptr = text[91];
      } else
        switch (texts[n].type) {
        case 0:
          ptr = (byte *)&mem[texts[n].ptr];
          break;
        case 1:
          if (mem[texts[n].ptr] < 0) {
            numero[0] = '-';
            ltoa(-mem[texts[n].ptr], (char *)numero + 1, 10);
          } else
            ltoa(mem[texts[n].ptr], (char *)numero, 10);
          ptr = numero;
          break;
        }

      texts[n].an = 0; // Initialize the blit region for this text

      x = texts[n].x; // X of first character
      y = texts[n].y; // Y

      fuente = 0;
      while (fuente < max_fonts) {
        if (texts[n].font == fonts[fuente])
          break;
        fuente++;
      }
      if (fuente == max_fonts)
        continue;

      checkpal_font(fuente);

      fnt = (TABLAFNT *)((byte *)texts[n].font + 1356);

      al = f_i[fuente].alto;

      ptr2 = ptr;
      an = 0;
      while (*ptr2) {
        if (fnt[*ptr2].ancho == 0) {
          an += f_i[fuente].espacio;
          ptr2++;
        } else
          an += fnt[*ptr2++].ancho;
      }

      switch (texts[n].centro) {
      case 0:
        break;
      case 1:
        x = x - (an >> 1);
        break;
      case 2:
        x = x - an;
        break;
      case 3:
        y = y - (al >> 1);
        break;
      case 4:
        x = x - (an >> 1);
        y = y - (al >> 1);
        break;
      case 5:
        x = x - an;
        y = y - (al >> 1);
        break;
      case 6:
        y = y - al;
        break;
      case 7:
        x = x - (an >> 1);
        y = y - al;
        break;
      case 8:
        x = x - an;
        y = y - al;
        break;
      }

      if (y < vga_height && y + al > 0) {
        texts[n].x0 = x;
        texts[n].y0 = y;
        texts[n].an = an;
        texts[n].al = al;

        if (y >= 0 && y + al <= vga_height) { // Text fits entirely (y coordinate)

          while (*ptr && x + fnt[*ptr].ancho <= 0) {
            if (fnt[*ptr].ancho == 0) {
              x += f_i[fuente].espacio;
              ptr++;
            } else {
              x = x + fnt[*ptr].ancho;
              ptr++;
            }
          }

          if (*ptr && x < 0) {
            if (fnt[*ptr].ancho == 0) {
              x += f_i[fuente].espacio;
              ptr++;
            } else {
              text_clipped(texts[n].font + fnt[*ptr].offset, x, y + fnt[*ptr].incY, fnt[*ptr].ancho,
                           fnt[*ptr].alto);
              x = x + fnt[*ptr].ancho;
              ptr++;
            }
          }

          while (*ptr && x + fnt[*ptr].ancho <= vga_width) {
            if (fnt[*ptr].ancho == 0) {
              x += f_i[fuente].espacio;
              ptr++;
            } else {
              text_normal(texts[n].font + fnt[*ptr].offset, x, y + fnt[*ptr].incY, fnt[*ptr].ancho,
                          fnt[*ptr].alto);
              x = x + fnt[*ptr].ancho;
              ptr++;
            }
          }

          if (*ptr && x < vga_width) {
            if (fnt[*ptr].ancho == 0) {
              x += f_i[fuente].espacio;
              ptr++;
            } else
              text_clipped(texts[n].font + fnt[*ptr].offset, x, y + fnt[*ptr].incY, fnt[*ptr].ancho,
                           fnt[*ptr].alto);
          }

        } else {
          while (*ptr && x + fnt[*ptr].ancho <= 0)
            if (fnt[*ptr].ancho == 0) {
              x += f_i[fuente].espacio;
              ptr++;
            } else {
              x = x + fnt[*ptr].ancho;
              ptr++;
            }

          while (*ptr && x < vga_width)
            if (fnt[*ptr].ancho == 0) {
              x += f_i[fuente].espacio;
              ptr++;
            } else {
              text_clipped(texts[n].font + fnt[*ptr].offset, x, y + fnt[*ptr].incY, fnt[*ptr].ancho,
                           fnt[*ptr].alto);
              x = x + fnt[*ptr].ancho;
              ptr++;
            }
        }
      }
    }
  while (++n < max_texts);
}

void text_normal(byte *p, int x, int y, byte an, int al) {
  byte *q = screen_buffer + y * vga_width + x;
  int ancho = an;

  do {
    do {
      if (*p) {
        *q = *p;
      }
      p++;
      q++;
    } while (--an);
    q += vga_width - (an = ancho);
  } while (--al);
}

void text_clipped(byte *p, int x, int y, byte an, int al) {
  byte *q = screen_buffer + y * vga_width + x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + an > vga_width)
    resto_x = x + an - vga_width;
  else
    resto_x = 0;
  long_x = an - salta_x - resto_x;

  if (long_x <= 0)
    return;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + al > vga_height)
    resto_y = y + al - vga_height;
  else
    resto_y = 0;
  long_y = al - salta_y - resto_y;

  if (long_y <= 0)
    return;

  p += an * salta_y + salta_x;
  q += vga_width * salta_y + salta_x;
  resto_x += salta_x;
  an = long_x;
  do {
    do {
      if (*p) {
        *q = *p;
      }
      p++;
      q++;
    } while (--an);
    q += vga_width - (an = long_x);
    p += resto_x;
  } while (--long_y);
}

//----------------------------------------------------------------------------
// Render the mode-7 window
//----------------------------------------------------------------------------

void paint_sprites_m7(int n, int cx, int cy, float ang);

void paint_m7(int n) {
  int x, y;
#ifdef DEBUG
  int oreloj = get_ticks();
#endif
  int id = (m7 + n)->camera;
  int height = (m7 + n)->height;
  int distance = (m7 + n)->distance;
  int angle = mem[id + _Angle];

  if (!im7[n].on || !id || id != mem[id])
    return;

  while (mem[id + _Angle] > 2 * pi)
    mem[id + _Angle] -= 2 * pi;
  while (mem[id + _Angle] < 0)
    mem[id + _Angle] += 2 * pi;

  angle = (mem[id + _Angle] * 4096) / (2 * pi);

  if (mem[id + _Resolution] > 0) {
    x = (((mem[id + _X] * 256) / mem[id + _Resolution]) * 256 -
         get_distx(mem[id + _Angle], distance * 65536));
    y = -(((mem[id + _Y] * 256) / mem[id + _Resolution]) * 256 -
          get_disty(mem[id + _Angle], distance * 65536));
  } else {
    x = (mem[id + _X] * 65536 - get_distx(mem[id + _Angle], distance * 65536));
    y = -(mem[id + _Y] * 65536 - get_disty(mem[id + _Angle], distance * 65536));
  }

  paint_mode7(n, x, height * 16384, y, angle); //<<14

  if (post_process_m7 != NULL)
    post_process_m7();

#ifdef DEBUG
  function_exec(251, get_ticks() - oreloj);
#endif

  paint_sprites_m7(n, x, -y, (float)((float)mem[id + _Angle] / radian));
}

//----------------------------------------------------------------------------
// Render mode-7 sprites (scroll snum)
//----------------------------------------------------------------------------

void paint_sprites_m7(int n, int cx, int cy, float ang) { // Takes the camera position
  int factor;
#ifdef DEBUG
  int oreloj;
#endif
  int cx2 = cx / 4096, cy2 = cy / 4096, dx, dy;
  int ide, id, max, distmax, old_z = 0;
  int altura, h, anchura, porcen;
  float a;

  factor = (((m7 + n)->focus * im7[n].an * 16) / 320);

  // Pre-calculate _Dist1 and _Dist2 for each mode-7 object

  for (id = id_start; id <= id_end; id += iloc_len) {
    if ((mem[id + _Status] == 2 || mem[id + _Status] == 4) && mem[id + _Ctype] == 2 &&
        (mem[id + _Cnumber] == 0 || (mem[id + _Cnumber] & (1 << n)))) {
      if (mem[id + _Resolution] > 0) {
        dx = (mem[id + _X] * 16) / mem[id + _Resolution] - cx2;
        dy = cy2 - (mem[id + _Y] * 16) / mem[id + _Resolution];
      } else {
        dx = mem[id + _X] * 16 - cx2;
        dy = cy2 - mem[id + _Y] * 16;
      }
      if ((!dx && !dy) || abs(dx) + abs(dy) > 2800 * 16)
        mem[id + _Dist1] = 0;
      else {
        mem[id + _Dist1] = sqrt(dx * dx + dy * dy);
        a = ang - atan2(dy, dx);
        mem[id + _Dist2] = (float)mem[id + _Dist1] * sin(a);
        mem[id + _Dist1] = (float)mem[id + _Dist1] * cos(a);
      }
    } else
      mem[id + _Dist1] = 0;
  }

  // Render visible objects within mode-7

  for (ide = id_start; ide <= id_end; ide += iloc_len)
    mem[ide + _Painted] = 0;

  do {
#ifdef DEBUG
    oreloj = get_ticks();
#endif
    ide = 0;
    max = 0x80000000;
    for (id = id_start; id <= id_end; id += iloc_len) {
      if (mem[id + _Ctype] == 2 && mem[id + _Dist1] && !mem[id + _Painted]) {
        if (mem[id + _Dist1] > max || (mem[id + _Dist1] == max && mem[id + _Z] > old_z)) {
          ide = id;
          max = mem[id + _Dist1];
          distmax = mem[id + _Dist2];
          old_z = mem[id + _Z];
        }
      }
    }

    if (ide) {
      if (max >= 32) { // Don't render objects at the camera's own coordinates

        h = (m7 + n)->height - mem[ide + _Height];
        altura = (h * (max - factor)) / (max);                // en pix/4
        altura = im7[n].y + im7[n].al / 2 + (h - altura) / 4; // en pix
        altura += (m7 + n)->horizon - im7[n].al / 2;

        h += 1000;
        porcen = (h * (max - factor)) / (max);                // en pix/4
        porcen = im7[n].y + im7[n].al / 2 + (h - porcen) / 4; // en pix
        porcen += (m7 + n)->horizon - im7[n].al / 2 - altura;

        porcen = mem[ide + _Size] * porcen / 1000;

        anchura = vga_width / 2 + (factor * distmax) / (max * 16);

        a = ((float)mem[ide + _Angle] / radian) - ang;
        h = (a * 4096.0) / 6.2831853;
        h += 1024;
        while (h < 0)
          h += 4096;
        while (h >= 4096)
          h -= 4096;

        paint_sprite_m7(n, ide, anchura, altura, porcen, h);
#ifdef DEBUG
        process_paint(ide, get_ticks() - oreloj);
#endif

        // *** Debug: white pixel at the object's base ***
        // if (altura<vga_height && altura>=0 && anchura<vga_width && anchura>=0) {
        //   *(screen_buffer+altura*vga_width+anchura)=127;
        // }
      }
      mem[ide + _Painted] = 1;
    }

  } while (ide);
}

//----------------------------------------------------------------------------
// Render a sprite (mem[ide]) in mode-7 (if it is in front of the camera)
//----------------------------------------------------------------------------

void paint_sprite_m7(int n, int ide, int x, int y, int size, int ang) {
  int *ptr;
  byte *si;
  int an, al; // Graphic width and height on screen
  int xg, yg;
  int m, p;

  if (mem[ide + _File] > max_fpgs || mem[ide + _File] < 0)
    return;
  if ((p = mem[ide + _XGraph]) > 0) {
    m = mem[p];
    if (m < 1 || m > 256)
      return;
    // ang is the viewing angle of the graphic (0..4096), m is the number of partitions
    mem[ide + _Flags] &= 254;
    mem[ide + _Graph] = ((ang + 4096 / (m * 2)) * m) / 4096;
    if (mem[ide + _Graph] >= m)
      mem[ide + _Graph] = 0;
    if ((mem[ide + _Graph] = mem[p + 1 + mem[ide + _Graph]]) < 0) {
      mem[ide + _Graph] = -mem[ide + _Graph];
      mem[ide + _Flags] |= 1;
    }
  }
  if (mem[ide + _File])
    max_grf = 1000;
  else
    max_grf = 2000;
  if (mem[ide + _Graph] <= 0 || mem[ide + _Graph] >= max_grf)
    return;
  if (g[mem[ide + _File]].grf == NULL)
    return;

  clipx0 = im7[n].x;
  clipx1 = im7[n].x + im7[n].an;
  clipy0 = im7[n].y;
  clipy1 = im7[n].y + im7[n].al;

  if ((ptr = g[mem[ide + _File]].grf[mem[ide + _Graph]]) != NULL) {
    an = ptr[13];
    al = ptr[14];                        // Graphic width and height
    si = (byte *)ptr + 64 + ptr[15] * 4; // Start of graphic data

    if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
      xg = ptr[13] / 2;
      yg = ptr[14] - 1;
    } else {
      xg = *((word *)ptr + 32);
      yg = *((word *)ptr + 33);
    }

    sp_scaled(si, x, y, an, al, xg, yg, size, mem[ide + _Flags]);
  }
}

//----------------------------------------------------------------------------
// Get_distx/y(angle,dist)
//----------------------------------------------------------------------------

int get_distx(int a, int d) {
  angulo = (float)a / radian;
  return ((int)((float)cos(angulo) * d));
}

int get_disty(int a, int d) {
  angulo = (float)a / radian;
  return (-(int)((float)sin(angulo) * d));
}

//----------------------------------------------------------------------------
// Render the map in perspective (mode-7 floor)
//----------------------------------------------------------------------------

void paint_mode7(int n, int camara_x, int camara_y, int camara_z, int angulo) {
  int y, u, du, vv, dv, pos_x, pos_y;
  int ancho_m, alto_m, ancho_e, alto_e;
  int proyeccion_y, sint, cost;
  int mediox_modo7 = im7[n].an >> 1;
  int mediox_modo7_16;
  int distancia = ((im7[n].an * (m7 + n)->focus) / 320) << 16;
  int divisor;
  byte *di = screen_buffer + im7[n].y * vga_width + im7[n].x, *di_end, color = (m7 + n)->color;

  mediox_modo7_16 = mediox_modo7 << 16;

  ancho_m = im7[n].map_width - 1;
  alto_m = im7[n].map_height - 1;
  ancho_e = im7[n].ext_an - 1;
  alto_e = im7[n].ext_al - 1;

  if (!camara_y)
    return;

  for (y = 0; y < im7[n].al; y++, di += vga_width - im7[n].an) {
    divisor = (m7 + n)->horizon - y - 1;
    if (camara_y > 0) {
      if (divisor >= 0) {
        di += im7[n].an;
        continue;
      }
    } else {
      if (divisor <= 0) {
        di += im7[n].an;
        continue;
      }
    }
    proyeccion_y = -camara_y / divisor;

    sint = mul_24(seno[angulo], proyeccion_y);
    cost = mul_24(coseno[angulo], proyeccion_y);

    u = camara_x + mul_16(distancia, cost) + mul_16(sint, -mediox_modo7_16);
    du = sint;

    vv = camara_z + mul_16(distancia, sint) - mul_16(cost, -mediox_modo7_16);
    dv = -cost;
    vv = -vv;
    dv = -dv;

    di_end = di + im7[n].an;

    if (im7[n].ext != NULL)
      do {
        pos_x = u >> 16;
        pos_y = vv >> 16;
        if (pos_x > ancho_m || pos_x < 0 || pos_y > alto_m || pos_y < 0) {
          *di = *(im7[n].ext + (pos_y & alto_e) * im7[n].ext_an + (pos_x & ancho_e));
        } else {
          *di = *(im7[n].map + pos_y * im7[n].map_width + pos_x);
        }
        u += du;
        vv += dv;
      } while (++di < di_end);
    else
      do {
        pos_x = u >> 16;
        pos_y = vv >> 16;
        if (pos_x > ancho_m || pos_x < 0 || pos_y > alto_m || pos_y < 0) {
          *di = color;
        } else {
          *di = *(im7[n].map + pos_y * im7[n].map_width + pos_x);
        }
        u += du;
        vv += dv;
      } while (++di < di_end);
  }
}


//----------------------------------------------------------------------------
// Initialize the sine and cosine lookup tables
//----------------------------------------------------------------------------

void init_sin_cos() {
  int i;

  for (i = 0; i < grados_90; i++) { // Generate the sine and cosine tables
    seno[i] = 256 * sin((float)i * 1131 / 737280) * 65536;
    seno[grados_180 - i] = seno[i];
    seno[grados_180 + i] = -seno[i];
    seno[grados_360 - i] = -seno[i];
    coseno[grados_90 - i] = seno[i];
    coseno[grados_90 + i] = -seno[i];
    coseno[grados_270 - i] = -seno[i];
    coseno[grados_270 + i] = seno[i];
  }
  seno[grados_0] = 0;
  seno[grados_90] = 256 * 65536;
  seno[grados_180] = 0;
  seno[grados_270] = -256 * 65536;
  seno[grados_360] = 0;
  coseno[grados_0] = 256 * 65536;
  coseno[grados_90] = 0;
  coseno[grados_180] = -256 * 65536;
  coseno[grados_270] = 0;
  coseno[grados_360] = 256 * 65536;
}

//----------------------------------------------------------------------------
