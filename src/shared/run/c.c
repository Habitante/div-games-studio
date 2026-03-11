
//----------------------------------------------------------------------------
// Collision detection functions
//----------------------------------------------------------------------------
#include "inter.h"
//----------------------------------------------------------------------------
// Headers
//----------------------------------------------------------------------------
int check_collisions(int i, int bloque, int scroll);
void test_collision(byte *buffer, int *ptr, int x, int y, int xg, int yg, int angle, int size,
                    int flags);
void sp_rotated_p(byte *si, int w, int h, int flags);
void test_normal(byte *p, int x, int y, int w, int h, int flags);
void test_clipped(byte *p, int x, int y, int w, int h, int flags);
void test_scaled(byte *old_si, int x, int y, int w, int h, int xg, int yg, int size, int flags);
void test_rotated(byte *si, int w, int h, int flags);
void test_scanc(byte *p, short n, short m, short o, byte *si, int w, int x0, int y0, int x1,
                int y1);
void test_scan(byte *p, short n, byte *si, int w, int x0, int y0, int x1, int y1);

//----------------------------------------------------------------------------
// Module-level variables
//----------------------------------------------------------------------------

int colisiona;
int p[24];

//----------------------------------------------------------------------------
//      Out_region(id,region) - only for screen or scroll graphics
//----------------------------------------------------------------------------

void out_region(void) {
  int id, file, graph, reg, angle;
  int xg, yg, xg1, yg1, x, y, w, h;
  int *ptr, n, m;

  reg = pila[sp--];
  id = pila[sp];
  pila[sp] = 1; // Default: not in the region

  if (mem[id + _Ctype] == 2) {
    e(137);
    return;
  }

  file = mem[id + _File];
  graph = mem[id + _Graph];
  angle = mem[id + _Angle];

  if (reg < 0 || reg >= max_region) {
    e(108);
    return;
  } // Already has region[reg].(x0..y1)

  if (file < 0 || file > max_fpgs) {
    e(109);
    return;
  }

  if ((n = mem[id + _XGraph]) > 0) {
    m = mem[n];
    if (m < 1 || m > 256)
      return;
    while (angle >= 2 * pi)
      angle -= 2 * pi;
    while (angle < 0)
      angle += 2 * pi;
    mem[id + _Flags] &= 254;
    graph = ((angle + (2 * pi) / (m * 2)) * m) / (2 * pi);
    angle = 0;
    if (graph >= m)
      graph = 0;
    if ((graph = mem[n + 1 + graph]) < 0) {
      graph = -graph;
      mem[id + _Flags] |= 1;
    }
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
  if ((ptr = g[file].grf[graph]) == NULL) {
    e(121);
    return;
  }

  x = mem[id + _X];
  y = mem[id + _Y];
  if (mem[id + _Resolution] > 0) {
    x /= mem[id + _Resolution];
    y /= mem[id + _Resolution];
  }

  if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
    xg = ptr[13] / 2;
    yg = ptr[14] / 2;
  } else {
    xg = *((word *)ptr + 32);
    yg = *((word *)ptr + 33);
  }

  w = ptr[13];
  h = ptr[14];

  if (angle) {
    sp_size(&x, &y, &w, &h, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);
    w = w - x + 1;
    h = h - y + 1;
  } else if (mem[id + _Size] != 100) {
    sp_size_scaled(&x, &y, &w, &h, xg, yg, mem[id + _Size], mem[id + _Flags]);
    w = w - x + 1;
    h = h - y + 1;
  } else {
    if (mem[id + _Flags] & 1) {
      xg = w - 1 - xg;
    }
    x -= xg;
    if (mem[id + _Flags] & 2) {
      yg = h - 1 - yg;
    }
    y -= yg;
  }

  // Already has region[reg].(x0..y1) and sprite(x,y,an,al)

  if (mem[id + _Ctype] == 1) {
    for (n = 0; n < 10; n++)
      if (iscroll[n].on && (mem[id + _Cnumber] == 0 || (mem[id + _Cnumber] & (1 << n)))) {
        xg = x + iscroll[n].x - iscroll[n].map1_x; // Sprite position in the scroll
        yg = y + iscroll[n].y - iscroll[n].map1_y;

        xg1 = xg + w;
        yg1 = yg + h; // 1st intersection of xg,yg,an,al and iscroll[n]

        if (iscroll[n].x > xg)
          xg = iscroll[n].x;
        if (iscroll[n].y > yg)
          yg = iscroll[n].y;
        if (iscroll[n].x + iscroll[n].w < xg1)
          xg1 = iscroll[n].x + iscroll[n].w;
        if (iscroll[n].y + iscroll[n].h < yg1)
          yg1 = iscroll[n].y + iscroll[n].h;

        if (xg >= xg1 || yg >= yg1)
          continue; // No intersection, skip

        if (xg < region[reg].x1 && yg < region[reg].y1 && xg1 > region[reg].x0 &&
            yg1 > region[reg].y0) {
          pila[sp] = 0;
          return;
        } // It is in the region
      }

  } else if (mem[id + _Ctype] == 0) {
    if (x < region[reg].x1 && y < region[reg].y1 && x + w > region[reg].x0 &&
        y + h > region[reg].y0)
      pila[sp] = 0; // It is in the region
  }
}

//----------------------------------------------------------------------------
//      Graphic_info(file,graph,info)
//----------------------------------------------------------------------------

void graphic_info(void) {
  int info, graph, file;
  int xg, yg;
  int *ptr;

  info = pila[sp--];
  graph = pila[sp--];
  file = pila[sp];
  pila[sp] = 0;

  if (file < 0 || file > max_fpgs) {
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
  if ((ptr = g[file].grf[graph]) == NULL) {
    e(121);
    return;
  }

  if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
    xg = ptr[13] / 2;
    yg = ptr[14] / 2;
  } else {
    xg = *((word *)ptr + 32);
    yg = *((word *)ptr + 33);
  }

  switch (pila[sp + 2]) {
  case 0:
    pila[sp] = ptr[13];
    break; //g_width (original)
  case 1:
    pila[sp] = ptr[14];
    break; //g_height (original)
  case 2:
    pila[sp] = xg;
    break; //g_x_center
  case 3:
    pila[sp] = yg;
    break; //g_y_center
  default:
    e(138);
  }
}

//----------------------------------------------------------------------------
// Main language function collision(process type)
//----------------------------------------------------------------------------

// WARNING: Detects collisions between screen-screen, scroll-scroll, and
// screen-scroll sprites. Sprites on a scroll only test collision against
// the first scroll they belong to, even if visible on multiple scrolls.

void collision(void) {
  int i, bloque; // Iterates processes from _IdScan onwards; if _IdScan=0 from start
  int file, graph, angle;
  int x, y, n, m; // Current process coordinates
  int64_t xg, yg; // Center of gravity of the current process graphic
  int *ptr;

  bloque = pila[sp];
  pila[sp] = 0; // Default: no collision

  if (mem[id + _Ctype] == 2) {
    e(139);
    return;
  }

  if (mem[id + _IdScan] == 0 || bloque != mem[id + _BlScan]) {
    mem[id + _BlScan] = bloque;
    i = id_start;
  } else if (mem[id + _IdScan] > id_end) {
    pila[sp] = 0;
    return;
  } else
    i = mem[id + _IdScan];

  file = mem[id + _File];
  graph = mem[id + _Graph];
  angle = mem[id + _Angle];

  if (file < 0 || file > max_fpgs) {
    e(109);
    return;
  }

  if ((n = mem[id + _XGraph]) > 0) {
    m = mem[n];
    if (m < 1 || m > 256)
      return;
    while (angle >= 2 * pi)
      angle -= 2 * pi;
    while (angle < 0)
      angle += 2 * pi;
    mem[id + _Flags] &= 254;
    graph = ((angle + (2 * pi) / (m * 2)) * m) / (2 * pi);
    angle = 0;
    if (graph >= m)
      graph = 0;
    if ((graph = mem[n + 1 + graph]) < 0) {
      graph = -graph;
      mem[id + _Flags] |= 1;
    }
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
  if ((ptr = g[file].grf[graph]) == NULL) {
    e(121);
    return;
  }

  x = mem[id + _X];
  y = mem[id + _Y];
  if (mem[id + _Resolution] > 0) {
    x /= mem[id + _Resolution];
    y /= mem[id + _Resolution];
  }

  if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
    xg = ptr[13] / 2;
    yg = ptr[14] / 2;
  } else {
    xg = *((word *)ptr + 32);
    yg = *((word *)ptr + 33);
  }

  if (angle) {
    clipx0 = x;
    clipy0 = y;
    clipx1 = ptr[13];
    clipy1 = ptr[14];
    sp_size(&clipx0, &clipy0, &clipx1, &clipy1, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);
  } else if (mem[id + _Size] != 100) {
    clipx0 = x;
    clipy0 = y;
    clipx1 = ptr[13];
    clipy1 = ptr[14];
    sp_size_scaled(&clipx0, &clipy0, &clipx1, &clipy1, xg, yg, mem[id + _Size], mem[id + _Flags]);
  } else {
    if (mem[id + _Flags] & 1)
      clipx0 = x - (ptr[13] - 1 - xg);
    else
      clipx0 = x - xg;
    if (mem[id + _Flags] & 2)
      clipy0 = y - (ptr[14] - 1 - yg);
    else
      clipy0 = y - yg;
    clipx1 = clipx0 + ptr[13] - 1;
    clipy1 = clipy0 + ptr[14] - 1;
  }

  buffer_w = clipx1 - clipx0 + 1;
  buffer_h = clipy1 - clipy0 + 1;
  if ((buffer = (byte *)malloc(buffer_w * buffer_h)) == NULL) {
    e(100);
    return;
  }
  memset(buffer, 0, buffer_w * buffer_h);

  // Now paint the sprite into the buffer(clip...)

  put_collision(buffer, ptr, x, y, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);

  // Already has sprite region clipx0..clipy1 (if on screen)

  if (mem[id + _Ctype] == 1) {
    for (n = 0; n < 10; n++)
      if (iscroll[n].on && (mem[id + _Cnumber] == 0 || (mem[id + _Cnumber] & (1 << n)))) {
        xg = iscroll[n].x - iscroll[n].map1_x;
        yg = iscroll[n].y - iscroll[n].map1_y;
        clipx0 += xg;
        clipx1 += xg;
        clipy0 += yg;
        clipy1 += yg;
        pila[sp] = check_collisions(i, bloque, n);
        break;
      }
  } else
    pila[sp] = check_collisions(i, bloque, -1);

  free(buffer);
}

//----------------------------------------------------------------------------
// Check collisions of sprite id (in buffer) against the rest (i..id_end)
//----------------------------------------------------------------------------

int check_collisions(int i, int bloque, int scroll) {
  int file, graph, angle;
  int *ptr, n = 0, m, j;
  int x, y, dist = 0;
  int xx, yy, w, h;
  short xg, yg, xxg, yyg;

  if (bloque == 0) { // collision(type mouse)
    if (mouse->x >= clipx0 && mouse->x <= clipx1 && mouse->y >= clipy0 && mouse->y <= clipy1) {
      if (*(buffer + buffer_w * (mouse->y - clipy0) + (mouse->x - clipx0)))
        return (id);
      else
        return (0);
    } else
      return (0);
  }

  for (; i <= id_end; i += iloc_len) {
    if (i != id && mem[i + _Bloque] == bloque && (mem[i + _Status] == 2 || mem[i + _Status] == 4) &&
        mem[i + _Ctype] < 2) {
      if (mem[i + _Ctype] == 1) { // If scroll sprite, determine which scroll (n)
        if (scroll < 0) {         // Screen vs scroll collision - first active
          for (n = 0; n < 10; n++)
            if (iscroll[n].on && (mem[i + _Cnumber] == 0 || (mem[i + _Cnumber] & (1 << n))))
              break;
          if (n == 10)
            continue;
        } else { // Scroll vs scroll collision - only if they share the 1st scroll
          if (mem[i + _Cnumber] && !(mem[i + _Cnumber] & (1 << scroll)))
            continue;
          else
            n = scroll;
        }
      }

      file = mem[i + _File];
      graph = mem[i + _Graph];
      angle = mem[i + _Angle];

      if (file < 0 || file > max_fpgs)
        continue;

      if ((j = mem[i + _XGraph]) > 0) {
        m = mem[j];
        if (m < 1 || m > 256)
          continue;
        while (angle >= 2 * pi)
          angle -= 2 * pi;
        while (angle < 0)
          angle += 2 * pi;
        mem[i + _Flags] &= 254;
        graph = ((angle + (2 * pi) / (m * 2)) * m) / (2 * pi);
        angle = 0;
        if (graph >= m)
          graph = 0;
        if ((graph = mem[j + 1 + graph]) < 0) {
          graph = -graph;
          mem[i + _Flags] |= 1;
        }
      }

      if (file)
        max_grf = 1000;
      else
        max_grf = 2000;
      if (graph <= 0 || graph >= max_grf)
        continue;
      if (g[file].grf == NULL)
        continue;
      if ((ptr = g[file].grf[graph]) == NULL)
        continue;

      x = mem[i + _X];
      y = mem[i + _Y];
      if (mem[i + _Resolution] > 0) {
        x /= mem[i + _Resolution];
        y /= mem[i + _Resolution];
      }

      if (mem[i + _Ctype] == 1) {
        x += iscroll[n].x - iscroll[n].map1_x;
        y += iscroll[n].y - iscroll[n].map1_y;
      }

      if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
        xg = ptr[13] / 2;
        yg = ptr[14] / 2;
      } else {
        xg = *((word *)ptr + 32);
        yg = *((word *)ptr + 33);
      }

      colisiona = 0;

      if (angle) {
        // Safety distance

        if (dist == 0)
          dist = (int)sqrt((buffer_w * buffer_w + buffer_h * buffer_h) / 4);
        w = ((int)sqrt(ptr[13] * ptr[13] + ptr[14] * ptr[14]) * mem[i + _Size]) / 100 + dist;

        // Actual distance

        xx = (clipx0 + clipx1) / 2 - x;
        yy = (clipy0 + clipy1) / 2 - y;
        h = (int)sqrt(xx * xx + yy * yy);

        if (h <= w) {
          xx = x;
          yy = y;
          w = ptr[13];
          h = ptr[14];
          sp_size(&xx, &yy, &w, &h, xg, yg, angle, mem[i + _Size], mem[i + _Flags]);

          if (clipx1 >= xx && clipx0 <= w)
            if (clipy1 >= yy && clipy0 < h)
              colisiona = 1;
        }

      } else if (mem[i + _Size] != 100) {
        xx = x;
        yy = y;
        w = ptr[13];
        h = ptr[14];
        sp_size_scaled(&xx, &yy, &w, &h, xg, yg, mem[i + _Size], mem[i + _Flags]);

        if (clipx1 >= xx && clipx0 <= w)
          if (clipy1 >= yy && clipy0 < h)
            colisiona = 1;

      } else {
        w = ptr[13];
        h = ptr[14];
        if (mem[i + _Flags] & 1)
          xxg = w - 1 - xg;
        else
          xxg = xg;
        xx = x - xxg;
        if (mem[i + _Flags] & 2)
          yyg = h - 1 - yg;
        else
          yyg = yg;
        yy = y - yyg;
        if (clipx1 >= xx && clipx0 < xx + w)
          if (clipy1 >= yy && clipy0 < yy + h)
            colisiona = 1;
      }

      if (colisiona == 1) {
        colisiona = 0;
        test_collision(buffer, ptr, x, y, xg, yg, angle, mem[i + _Size], mem[i + _Flags]);
        if (colisiona) {
          mem[id + _IdScan] = i + iloc_len;
          return (i);
        }
      }
    }
  }

  mem[id + _IdScan] = i;
  return (0);
}

//-----------------------------------------------------------------------------
//      Returns the bounding box of a rotated and scaled graphic
//-----------------------------------------------------------------------------

void sp_size(int *x, int *y, int *xx, int *yy, int xg, int yg, int ang, int size, int flags) {
  float d0, d1, d2, d3;
  float a0, a1, a2, a3;
  float a, s;
  int n;

  a = (float)(ang / radian);
  s = (float)(size / 100.0f);

  xg = -xg;
  yg = -yg;

  d0 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a0 = a;
  else
    a0 = a + (float)atan2(-yg, xg);
  xg = xg + *xx - 1;
  d1 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a1 = a;
  else
    a1 = a + (float)atan2(-yg, xg);
  yg = yg + *yy - 1;
  d2 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a2 = a;
  else
    a2 = a + (float)atan2(-yg, xg);
  xg = xg - *xx + 1;
  d3 = (float)sqrt(xg * xg + yg * yg) * s;
  if (yg == 0 && xg == 0)
    a3 = a;
  else
    a3 = a + (float)atan2(-yg, xg);

  if (flags & 1) {
    p[0] = *x - (int)((float)cos(a0) * d0);
    p[2] = *x - (int)((float)cos(a1) * d1);
    p[4] = *x - (int)((float)cos(a2) * d2);
    p[6] = *x - (int)((float)cos(a3) * d3);
  } else {
    p[0] = *x + (int)((float)cos(a0) * d0);
    p[2] = *x + (int)((float)cos(a1) * d1);
    p[4] = *x + (int)((float)cos(a2) * d2);
    p[6] = *x + (int)((float)cos(a3) * d3);
  }

  if (flags & 2) {
    p[1] = *y - (int)(-(float)sin(a0) * d0);
    p[3] = *y - (int)(-(float)sin(a1) * d1);
    p[5] = *y - (int)(-(float)sin(a2) * d2);
    p[7] = *y - (int)(-(float)sin(a3) * d3);
  } else {
    p[1] = *y + (int)(-(float)sin(a0) * d0);
    p[3] = *y + (int)(-(float)sin(a1) * d1);
    p[5] = *y + (int)(-(float)sin(a2) * d2);
    p[7] = *y + (int)(-(float)sin(a3) * d3);
  }

  *x = 0x7fffffff;
  *y = 0x7fffffff;
  *xx = 0x80000000;
  *yy = 0x80000000;

  for (n = 0; n < 8; n++) {
    if (*x > p[n]) {
      *x = p[n];
    }
    if (*xx < p[n]) {
      *xx = p[n];
    }
    n++;
    if (*y > p[n]) {
      *y = p[n];
    }
    if (*yy < p[n]) {
      *yy = p[n];
    }
  }
}

//-----------------------------------------------------------------------------
//      Returns the bounding box of a scaled graphic
//-----------------------------------------------------------------------------

void sp_size_scaled(int *x, int *y, int *xx, int *yy, int xg, int yg, int size, int flags) {
  int x0, y0, x1, y1; // Area occupied by the sprite on the framebuffer

  if (flags & 1)
    x0 = *x - ((*xx - 1 - xg) * size) / 100;
  else
    x0 = *x - (xg * size) / 100;
  if (flags & 2)
    y0 = *y - ((*yy - 1 - yg) * size) / 100;
  else
    y0 = *y - (yg * size) / 100;

  x1 = x0 + (*xx * size) / 100 - 1;
  y1 = y0 + (*yy * size) / 100 - 1;

  if (x1 < x0) {
    x1 = x0;
  }
  if (y1 < y0) {
    y1 = y0;
  } // ***

  *x = x0;
  *y = y0;
  *xx = x1;
  *yy = y1;
}

//----------------------------------------------------------------------------
//      External function to render graphics (for collisions)
//----------------------------------------------------------------------------

void put_collision(byte *buffer, int *ptr, int x, int y, int xg, int yg, int angle, int size,
                   int flags) {
  byte *si;
  int w, h; // Graphic screen dimensions
  byte *_saved_buffer;
  int _saved_width, _saved_height;
  int ix, iy;

  w = ptr[13];
  h = ptr[14];
  si = (byte *)ptr + 64 + ptr[15] * 4;

  ix = clipx0;
  iy = clipy0;

  _saved_buffer = screen_buffer;
  screen_buffer = buffer;
  _saved_width = vga_width;
  vga_width = buffer_w;
  _saved_height = vga_height;
  vga_height = buffer_h;
  clipx0 = 0;
  clipx1 -= ix - 1;
  clipy0 = 0;
  clipy1 -= iy - 1;

  if (angle) {
    for (x = 0; x < 8; x += 2) {
      p[x] -= ix;
      p[x + 1] -= iy;
    }
    sp_rotated_p(si, w, h, flags);
  } else if (size != 100) {
    x -= ix;
    y -= iy;
    sp_scaled(si, x, y, w, h, xg, yg, size, flags);
  } else {
    if (flags & 1) {
      xg = w - 1 - xg;
    }
    x -= xg + ix;
    if (flags & 2) {
      yg = h - 1 - yg;
    }
    y -= yg + iy;
    if (x >= clipx0 && x + w <= clipx1 && y >= clipy0 &&
        y + h <= clipy1) // Render unclipped sprite
      sp_normal(si, x, y, w, h, flags);
    else if (x < clipx1 && y < clipy1 && x + w > clipx0 &&
             y + h > clipy0) // Render clipped sprite
      sp_clipped(si, x, y, w, h, flags);
  }

  screen_buffer = _saved_buffer;
  vga_width = _saved_width;
  vga_height = _saved_height;
  clipx0 = ix;
  clipx1 += ix - 1;
  clipy0 = iy;
  clipy1 += iy - 1;
}

//----------------------------------------------------------------------------
// Sprite - rotated [scaled] [clipped] [mirrored] [ghost]
//----------------------------------------------------------------------------

void sp_rotated_p(byte *si, int w, int h, int flags) {
  int scan_y, hmin, hmax; // Minimum and maximum height
  int n, l0 = 0, l1; // Side 0 and side 1 (p[] indices)

  int hmax0, hmax1;
  union {
    int l;
    short w[2];
  } x0, x1, g0x = {0}, g1x = {0}, g0y = {0}, g1y = {0};
  int ix0 = 0, ix1 = 0, ig0x = 0, ig1x = 0, ig0y = 0, ig1y = 0, kk;
  byte *ptrcopia;

  memcpy(&p[8], &p[0], sizeof(p[0]) * 8);
  memcpy(&p[16], &p[0], sizeof(p[0]) * 8);

  hmin = 32767;
  hmax = -32768;
  n = 8;
  do {
    if (p[n + 1] < hmin) {
      l0 = n;
      hmin = p[n + 1];
    }
    if (p[n + 1] > hmax)
      hmax = p[n + 1];
    n += 2;
  } while (n < 16);

  l1 = l0;
  hmax0 = hmin;
  hmax1 = hmin;
  ptrcopia = screen_buffer + hmin * vga_width;

  scan_y = hmin;
  do {
    if (scan_y < hmax) {
      while (scan_y == hmax0) {
        if ((hmax0 = p[l0 - 1]) != scan_y) {
          x0.l = p[l0] << 16;
          ix0 = ((p[l0 - 2] << 16) - x0.l) / (hmax0 - scan_y);
          if (ix0 < 0)
            x0.l += 65535;
          switch (l0 & 6) {
          case 0:
            g0y.l = 0;
            ig0y = (h << 16) / (hmax0 - scan_y);
            g0x.l = 0;
            ig0x = 0;
            break;
          case 2:
            g0x.l = ((w - 1) << 16) + 65535;
            ig0x = (w << 16) / (scan_y - hmax0);
            g0y.l = 0;
            ig0y = 0;
            break;
          case 4:
            g0y.l = ((h - 1) << 16) + 65535;
            ig0y = (h << 16) / (scan_y - hmax0);
            g0x.l = ((w - 1) << 16) + 65535;
            ig0x = 0;
            break;
          case 6:
            g0x.l = 0;
            ig0x = (w << 16) / (hmax0 - scan_y);
            g0y.l = ((h - 1) << 16) + 65535;
            ig0y = 0;
            break;
          }
        }
        l0 -= 2;
      }
      while (scan_y == hmax1) {
        if ((hmax1 = p[l1 + 3]) != scan_y) {
          x1.l = p[l1] << 16;
          ix1 = ((p[l1 + 2] << 16) - x1.l) / (hmax1 - scan_y);
          if (ix1 < 0)
            x1.l += 65535;
          switch (l1 & 6) {
          case 0:
            g1x.l = 0;
            ig1x = (w << 16) / (hmax1 - scan_y);
            g1y.l = 0;
            ig1y = 0;
            break;
          case 2:
            g1y.l = 0;
            ig1y = (h << 16) / (hmax1 - scan_y);
            g1x.l = ((w - 1) << 16) + 65535;
            ig1x = 0;
            break;
          case 4:
            g1x.l = ((w - 1) << 16) + 65535;
            ig1x = (w << 16) / (scan_y - hmax1);
            g1y.l = ((h - 1) << 16) + 65535;
            ig1y = 0;
            break;
          case 6:
            g1y.l = ((h - 1) << 16) + 65535;
            ig1y = (h << 16) / (scan_y - hmax1);
            g1x.l = 0;
            ig1x = 0;
            break;
          }
        }
        l1 += 2;
      }
    } else {
      if (scan_y == hmax0) {
        g0x.l = 0;
        g0y.l = 0;
        ig0x = 0;
        ig0y = 0;
        x0.l = p[l0] << 16;
        ix0 = 0;
        switch (l0 & 6) {
        case 2:
          g0x.l = ((w - 1) << 16) + 65535;
          break;
        case 4:
          g0y.l = ((h - 1) << 16) + 65535;
          g0x.l = ((w - 1) << 16) + 65535;
          break;
        case 6:
          g0y.l = ((h - 1) << 16) + 65535;
          break;
        }
      }
      if (scan_y == hmax1) {
        g1x.l = 0;
        g1y.l = 0;
        ig1x = 0;
        ig1y = 0;
        x1.l = p[l1] << 16;
        ix1 = 0;
        switch (l1 & 6) {
        case 2:
          g1x.l = ((w - 1) << 16) + 65535;
          break;
        case 4:
          g1x.l = ((w - 1) << 16) + 65535;
          g1y.l = ((h - 1) << 16) + 65535;
          break;
        case 6:
          g1y.l = ((h - 1) << 16) + 65535;
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

    if (scan_y < clipy1 && scan_y >= clipy0 && x0.w[1] < clipx1 && x1.w[1] >= clipx0 && x1.w[1] > x0.w[1])
      sp_scan(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], si, w, g0x.l, g0y.l, g1x.l, g1y.l);

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

  } while (scan_y++ < hmax);
}

//----------------------------------------------------------------------------
//      External function to test graphics (for collisions)
//----------------------------------------------------------------------------

void test_collision(byte *buffer, int *ptr, int x, int y, int xg, int yg, int angle, int size,
                    int flags) {
  byte *si;
  int w, h; // Graphic screen dimensions
  byte *_saved_buffer;
  int _saved_width, _saved_height;
  int ix, iy;

  w = ptr[13];
  h = ptr[14];
  si = (byte *)ptr + 64 + ptr[15] * 4;

  ix = clipx0;
  iy = clipy0;

  _saved_buffer = screen_buffer;
  screen_buffer = buffer;
  _saved_width = vga_width;
  vga_width = buffer_w;
  _saved_height = vga_height;
  vga_height = buffer_h;
  clipx0 = 0;
  clipx1 -= ix - 1;
  clipy0 = 0;
  clipy1 -= iy - 1;

  if (angle) {
    for (x = 0; x < 8; x += 2) {
      p[x] -= ix;
      p[x + 1] -= iy;
    }
    test_rotated(si, w, h, flags);
  } else if (size != 100) {
    x -= ix;
    y -= iy;
    test_scaled(si, x, y, w, h, xg, yg, size, flags);
  } else {
    if (flags & 1) {
      xg = w - 1 - xg;
    }
    x -= xg + ix;
    if (flags & 2) {
      yg = h - 1 - yg;
    }
    y -= yg + iy;
    if (x >= clipx0 && x + w <= clipx1 && y >= clipy0 && y + h <= clipy1) // Test unclipped sprite
      test_normal(si, x, y, w, h, flags);
    else if (x < clipx1 && y < clipy1 && x + w > clipx0 && y + h > clipy0) // Test clipped sprite
      test_clipped(si, x, y, w, h, flags);
  }

  screen_buffer = _saved_buffer;
  vga_width = _saved_width;
  vga_height = _saved_height;
  clipx0 = ix;
  clipx1 += ix - 1;
  clipy0 = iy;
  clipy1 += iy - 1;
}

//----------------------------------------------------------------------------
// Test sprite - [mirrored] [ghost]
//----------------------------------------------------------------------------

void test_normal(byte *p, int x, int y, int w, int h, int flags) {
  byte *q = screen_buffer + y * vga_width + x;
  int width = w;

  switch (flags & 3) {
  case 0: //--
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p++;
        q++;
      } while (--w);
      q += vga_width - (w = width);
    } while (--h);
    break;
  case 1: //h-
    p += w - 1;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p--;
        q++;
      } while (--w);
      q += vga_width - (w = width);
      p += w * 2;
    } while (--h);
    break;
  case 2: //-v
    p += (h - 1) * w;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p++;
        q++;
      } while (--w);
      q += vga_width - (w = width);
      p -= w * 2;
    } while (--h);
    break;
  case 3: //hv
    p += h * w - 1;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p--;
        q++;
      } while (--w);
      q += vga_width - (w = width);
    } while (--h);
    break;
  }
}

//----------------------------------------------------------------------------
// Test sprite - clipped [mirrored] [ghost]
//----------------------------------------------------------------------------

void test_clipped(byte *p, int x, int y, int w, int h, int flags) {
  byte *q = screen_buffer + y * vga_width + x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x < clipx0)
    salta_x = clipx0 - x;
  else
    salta_x = 0;
  if (x + w > clipx1)
    resto_x = x + w - clipx1;
  else
    resto_x = 0;
  long_x = w - salta_x - resto_x;

  if (y < clipy0)
    salta_y = clipy0 - y;
  else
    salta_y = 0;
  if (y + h > clipy1)
    resto_y = y + h - clipy1;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  switch (flags & 3) {
  case 0: //--
    p += w * salta_y + salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    w = long_x;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p++;
        q++;
      } while (--w);
      q += vga_width - (w = long_x);
      p += resto_x;
    } while (--long_y);
    break;
  case 1: //h-
    p += w * salta_y + w - 1 - salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p--;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p += w + long_x;
    } while (--long_y);
    break;
  case 2: //-v
    p += (h - 1) * w - w * salta_y + salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
        p++;
        q++;
      } while (--salta_x);
      q += vga_width - (salta_x = long_x);
      p += resto_x - w * 2;
    } while (--long_y);
    break;
  case 3: //hv
    p += h * w - 1 - w * salta_y - salta_x;
    q += vga_width * salta_y + salta_x;
    resto_x += salta_x;
    salta_x = long_x;
    do {
      do {
        if (*p && *q) {
          colisiona = 1;
          return;
        }
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
// Test sprite - scaled [clipped] [mirrored] [ghost]
//----------------------------------------------------------------------------

void test_scaled(byte *old_si, int x, int y, int w, int h, int xg, int yg, int size, int flags) {
  int x0, y0, x1, y1;           // Area occupied by the sprite on the framebuffer
  int salta_x, long_x, resto_x; // Screen-relative
  int salta_y, long_y, resto_y;
  int xr, ixr, yr, iyr, old_xr, old_w;
  byte *si, *di;

  if (flags & 1)
    x0 = x - ((w - 1 - xg) * size) / 100;
  else
    x0 = x - (xg * size) / 100;
  if (flags & 2)
    y0 = y - ((h - 1 - yg) * size) / 100;
  else
    y0 = y - (yg * size) / 100;
  x1 = x0 + (w * size) / 100 - 1;
  y1 = y0 + (h * size) / 100 - 1;

  if (x1 < x0 || y1 < y0)
    return;

  ixr = (float)(w * 256) / (float)(x1 - x0 + 1);
  iyr = (float)(h * 256) / (float)(y1 - y0 + 1);

  if (x1 < clipx0 || y1 < clipy0 || x0 >= clipx1 || y0 >= clipy1)
    return;

  di = screen_buffer + y0 * vga_width + x0;

  if (x0 < clipx0)
    salta_x = clipx0 - x0;
  else
    salta_x = 0;
  if (x1 >= clipx1)
    resto_x = x1 - clipx1 + 1;
  else
    resto_x = 0;
  long_x = (w * size) / 100 - salta_x - resto_x;

  if (y0 < clipy0)
    salta_y = clipy0 - y0;
  else
    salta_y = 0;
  if (y1 >= clipy1)
    resto_y = y1 - clipy1 + 1;
  else
    resto_y = 0;
  long_y = (h * size) / 100 - salta_y - resto_y;

  if (flags & 1) {
    xr = w * 256 - salta_x * ixr - 1;
    ixr = -ixr;
  } else
    xr = salta_x * ixr;
  if (flags & 2) {
    yr = h * 256 - salta_y * iyr - 1;
    iyr = -iyr;
  } else
    yr = salta_y * iyr;

  old_xr = xr;
  old_w = w;
  di += vga_width * salta_y + salta_x;
  w = long_x;

  do {
    si = old_si + (yr >> 8) * old_w;
    do {
      if (*(si + (xr >> 8)) && *di) {
        colisiona = 1;
        return;
      }
      di++;
      xr += ixr;
    } while (--w);
    yr += iyr;
    xr = old_xr;
    di += vga_width - (w = long_x);
  } while (--long_y);
}

//----------------------------------------------------------------------------
// Test sprite - rotated [scaled] [clipped] [mirrored] [ghost]
//----------------------------------------------------------------------------

void test_rotated(byte *si, int w, int h, int flags) {
  int scan_y, hmin, hmax; // Minimum and maximum height
  int n, l0 = 0, l1; // Side 0 and side 1 (p[] indices)

  int hmax0, hmax1;
  union {
    int l;
    short w[2];
  } x0, x1, g0x = {0}, g1x = {0}, g0y = {0}, g1y = {0};
  int ix0 = 0, ix1 = 0, ig0x = 0, ig1x = 0, ig0y = 0, ig1y = 0, kk;
  byte *ptrcopia;

  memcpy(&p[8], &p[0], sizeof(p[0]) * 8);
  memcpy(&p[16], &p[0], sizeof(p[0]) * 8);

  hmin = 32767;
  hmax = -32768;
  n = 8;
  do {
    if (p[n + 1] < hmin) {
      l0 = n;
      hmin = p[n + 1];
    }
    if (p[n + 1] > hmax)
      hmax = p[n + 1];
    n += 2;
  } while (n < 16);

  l1 = l0;
  hmax0 = hmin;
  hmax1 = hmin;
  ptrcopia = screen_buffer + hmin * vga_width;

  scan_y = hmin;
  do {
    if (scan_y < hmax) {
      while (scan_y == hmax0) {
        if ((hmax0 = p[l0 - 1]) != scan_y) {
          x0.l = p[l0] << 16;
          ix0 = ((p[l0 - 2] << 16) - x0.l) / (hmax0 - scan_y);
          if (ix0 < 0)
            x0.l += 65535;
          switch (l0 & 6) {
          case 0:
            g0y.l = 0;
            ig0y = (h << 16) / (hmax0 - scan_y);
            g0x.l = 0;
            ig0x = 0;
            break;
          case 2:
            g0x.l = ((w - 1) << 16) + 65535;
            ig0x = (w << 16) / (scan_y - hmax0);
            g0y.l = 0;
            ig0y = 0;
            break;
          case 4:
            g0y.l = ((h - 1) << 16) + 65535;
            ig0y = (h << 16) / (scan_y - hmax0);
            g0x.l = ((w - 1) << 16) + 65535;
            ig0x = 0;
            break;
          case 6:
            g0x.l = 0;
            ig0x = (w << 16) / (hmax0 - scan_y);
            g0y.l = ((h - 1) << 16) + 65535;
            ig0y = 0;
            break;
          }
        }
        l0 -= 2;
      }
      while (scan_y == hmax1) {
        if ((hmax1 = p[l1 + 3]) != scan_y) {
          x1.l = p[l1] << 16;
          ix1 = ((p[l1 + 2] << 16) - x1.l) / (hmax1 - scan_y);
          if (ix1 < 0)
            x1.l += 65535;
          switch (l1 & 6) {
          case 0:
            g1x.l = 0;
            ig1x = (w << 16) / (hmax1 - scan_y);
            g1y.l = 0;
            ig1y = 0;
            break;
          case 2:
            g1y.l = 0;
            ig1y = (h << 16) / (hmax1 - scan_y);
            g1x.l = ((w - 1) << 16) + 65535;
            ig1x = 0;
            break;
          case 4:
            g1x.l = ((w - 1) << 16) + 65535;
            ig1x = (w << 16) / (scan_y - hmax1);
            g1y.l = ((h - 1) << 16) + 65535;
            ig1y = 0;
            break;
          case 6:
            g1y.l = ((h - 1) << 16) + 65535;
            ig1y = (h << 16) / (scan_y - hmax1);
            g1x.l = 0;
            ig1x = 0;
            break;
          }
        }
        l1 += 2;
      }
    } else {
      if (scan_y == hmax0) {
        g0x.l = 0;
        g0y.l = 0;
        ig0x = 0;
        ig0y = 0;
        x0.l = p[l0] << 16;
        ix0 = 0;
        switch (l0 & 6) {
        case 2:
          g0x.l = ((w - 1) << 16) + 65535;
          break;
        case 4:
          g0y.l = ((h - 1) << 16) + 65535;
          g0x.l = ((w - 1) << 16) + 65535;
          break;
        case 6:
          g0y.l = ((h - 1) << 16) + 65535;
          break;
        }
      }
      if (scan_y == hmax1) {
        g1x.l = 0;
        g1y.l = 0;
        ig1x = 0;
        ig1y = 0;
        x1.l = p[l1] << 16;
        ix1 = 0;
        switch (l1 & 6) {
        case 2:
          g1x.l = ((w - 1) << 16) + 65535;
          break;
        case 4:
          g1x.l = ((w - 1) << 16) + 65535;
          g1y.l = ((h - 1) << 16) + 65535;
          break;
        case 6:
          g1y.l = ((h - 1) << 16) + 65535;
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

    if (scan_y < clipy1 && scan_y >= clipy0 && x0.w[1] < clipx1 && x1.w[1] >= clipx0 && x1.w[1] > x0.w[1]) {
      if (x0.w[1] < clipx0) {
        if (x1.w[1] >= clipx1)
          test_scanc(ptrcopia + clipx0, x1.w[1] - x0.w[1], clipx1 - clipx0 - 1, clipx0 - x0.w[1],
                     si, w, g0x.l, g0y.l, g1x.l, g1y.l);
        else
          test_scanc(ptrcopia + clipx0, x1.w[1] - x0.w[1], x1.w[1] - clipx0, clipx0 - x0.w[1], si,
                     w, g0x.l, g0y.l, g1x.l, g1y.l);
      } else if (x1.w[1] >= clipx1)
        test_scanc(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], clipx1 - 1 - x0.w[1], 0, si, w, g0x.l,
                   g0y.l, g1x.l, g1y.l);
      else
        test_scan(ptrcopia + x0.w[1], x1.w[1] - x0.w[1], si, w, g0x.l, g0y.l, g1x.l, g1y.l);
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

  } while (scan_y++ < hmax);
}

void test_scanc(byte *p, short n, short m, short o, byte *si, int w, int x0, int y0, int x1,
                int y1) {
  union {
    int l;
    short w[2];
  } x, y;

  x.l = x0;
  y.l = y0;
  x0 = (x1 - x0) / n;
  y0 = (y1 - y0) / n;

  if (o) {
    x.l += x0 * o;
    y.l += y0 * o;
  }

  do {
    if (*p && *(si + x.w[1] + y.w[1] * w)) {
      colisiona = 1;
      return;
    }
    p++;
    x.l += x0;
    y.l += y0;
  } while (m--);
}

void test_scan(byte *p, short n, byte *si, int w, int x0, int y0, int x1, int y1) {
  union {
    int l;
    short w[2];
  } x, y;

  x.l = x0;
  y.l = y0;
  x0 = (x1 - x0) / n;
  y0 = (y1 - y0) / n;

  do {
    if (*p && *(si + x.w[1] + y.w[1] * w)) {
      colisiona = 1;
      return;
    }
    p++;
    x.l += x0;
    y.l += y0;
  } while (n--);
}
