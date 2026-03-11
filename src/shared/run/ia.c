
//----------------------------------------------------------------------------
//  AI functions (and related utilities)
//----------------------------------------------------------------------------

//  Initially the following functions:
//    path_find() (main pathfinding search)
//    path_line() (determines if a straight line path is possible)
//    path_free() (fast check if a point is free)
//    new_map()  (to create maps in the program itself, similar to load_map)

#include "inter.h"

//----------------------------------------------------------------------------
//  Module data
//----------------------------------------------------------------------------

#define max_map_size 254 // 128

int find_status; // Whether the search structures have been initialized

int modo; // 0 Fast, 1 Exact

word *distancias;  // Distance table from origin to each cell
word *distancias2; // Distance table from destination to each cell
word *siguientes;  // Linked list of cells

#define dis(x)  (*(distancias + (x)))
#define dis2(x) (*(distancias2 + (x)))
#define sig(x)  (*(siguientes + (x)))

byte *map;  // Pointer to the hardness map (bytes, 0 = free)
int w, h; // Width and height of the map

#define m(x, y) (*(map + (x) + ((y) * w)))

word cx, cy, c;     // Cell currently being explored (x, y and index)
int ax, ay, bx, by; // Start and end cells
word b;             // End cell (index)
int fin;            // Whether the destination has been reached
int tile;           // Map tile size (integer, 1 or greater)
int choque_linea;   // Whether a straight line between two points is blocked

int cas_inicial; // If the first cell is occupied, temporarily clear it

//----------------------------------------------------------------------------
//  path_find(mode,file,code,tilesize,x,y,table offset,sizeof table)
//----------------------------------------------------------------------------

int init_find(void);
int calculate_vertices(int *ptr, int max_ver, int x0, int y0, int x1, int y1);
void can_go(int x0, int y0, int x1, int y1);
void path_line(void);
void path_free(void);
void expand(void);
void expand2(void);
void add(int x, int y, word bnew, word step);
void add2(word bnew, word step);

void path_find(void) {
  int file, code, x, y, offset, size; // Input parameters
  int *ptr;                           // Pointer to the map record

  //----------------------------------------------------------------------------

  size = pila[sp--];
  offset = pila[sp--];
  y = pila[sp--];
  x = pila[sp--];
  tile = pila[sp--];
  code = pila[sp--];
  file = pila[sp--];
  modo = pila[sp];

  pila[sp] = 0; // Default return value, until proven otherwise

  // Check offset and size bounds ...
#ifdef DIV2
  if (!validate_address(offset) || !validate_address(offset + size)) {
    e(122);
    return;
  }
#else

  if (offset < long_header || offset + size > imem_max) {
    e(e122);
    return;
  }
#endif

  // Tile size bounds

  if (tile < 1 || tile > 256) {
    e(151);
    return;
  }

  // Check file and code bounds

  if (file > max_fpgs || file < 0) {
    e(109);
    return;
  }
  if (file)
    max_grf = 1000;
  else
    max_grf = 2000;
  if (code <= 0 || code >= max_grf) {
    e(110);
    return;
  }
  if (g[file].grf == NULL) {
    e(111);
    return;
  }
  if ((ptr = g[file].grf[code]) == NULL) {
    e(121);
    return;
  }

  // Get pointer to the map, width and height

  w = ptr[13];
  h = ptr[14];
  map = (byte *)ptr + 64 + ptr[15] * 4;

  if (w < 1 || h < 1 || w > max_map_size || h > max_map_size) {
    e(152);
    return;
  }

  // Check coordinate bounds (if outside the map, return 0)

  if (x < 0 || y < 0 || x >= w * tile || y >= h * tile)
    return;
  ax = mem[id + _X];
  ay = mem[id + _Y];
  if (ax < 0 || ay < 0 || ax >= w * tile || ay >= h * tile)
    return;

  // Calculate start and end cells: m(ax,ay) and m(bx,by)

  ax /= tile;
  ay /= tile;
  bx = x / tile;
  by = y / tile;

  if (m(bx, by)) { // Case where the end cell is occupied

    x = 0; // If the end cell is not free, try an adjacent one

    if (ax < bx) {
      if (ay < by) {
        if (bx)
          if (!m(bx - 1, by))
            x = 1;
        if (!x && by)
          if (!m(bx, by - 1))
            x = 2;
        if (!x && bx < w - 1)
          if (!m(bx + 1, by))
            x = 3;
        if (!x && by < h - 1)
          if (!m(bx, by + 1))
            x = 4;
      } else {
        if (bx)
          if (!m(bx - 1, by))
            x = 1;
        if (!x && by < h - 1)
          if (!m(bx, by + 1))
            x = 4;
        if (!x && by)
          if (!m(bx, by - 1))
            x = 2;
        if (!x && bx < w - 1)
          if (!m(bx + 1, by))
            x = 3;
      }
    } else {
      if (ay < by) {
        if (bx < w - 1)
          if (!m(bx + 1, by))
            x = 3;
        if (!x && by)
          if (!m(bx, by - 1))
            x = 2;
        if (!x && by < h - 1)
          if (!m(bx, by + 1))
            x = 4;
        if (!x && bx)
          if (!m(bx - 1, by))
            x = 1;
      } else {
        if (bx < w - 1)
          if (!m(bx + 1, by))
            x = 3;
        if (!x && by < h - 1)
          if (!m(bx, by + 1))
            x = 4;
        if (!x && bx)
          if (!m(bx - 1, by))
            x = 1;
        if (!x && by)
          if (!m(bx, by - 1))
            x = 2;
      }
    }

    if (!x)
      return;

    switch (x) {
    case 1:
      bx--;
      x = bx * tile + tile - 1;
      y = by * tile + tile / 2;
      break;
    case 2:
      by--;
      x = bx * tile + tile / 2;
      y = by * tile + tile - 1;
      break;
    case 3:
      bx++;
      x = bx * tile;
      y = by * tile + tile / 2;
      break;
    case 4:
      by++;
      x = bx * tile + tile / 2;
      y = by * tile;
      break;
    }
  }

  // If the start and end cells are identical ...

  if (ax == bx && ay == by) {
    if (size < 2)
      return;
    mem[offset] = x;
    mem[offset + 1] = y; // Go directly to (x,y)
    pila[sp] = 1;
    return;
  }

  //----------------------------------------------------------------------------

  // Initialize the search system if this is the first search

  if (!find_status)
    if (init_find()) {
      e(100);
      return;
    }

  cas_inicial = m(ax, ay);
  m(ax, ay) = 0;

  // Prepare (clear) the buffers ...

  memset(distancias, 0, max_map_size * max_map_size * 2); // Distances must be cleared to track
  // which cells have already been visited in the flood fill

  cx = ax;
  cy = ay;
  c = cx + cy * max_map_size;
  b = bx + by * max_map_size;

  sig(c) = 65535; // No next cell
  dis(c) = 1;     // No real distance (1 to mark it as visited)
  fin = 0;        // No path found yet

  if (!modo)
    do {
      expand();
      c = sig(c);
      if (c == 65535)
        break;
      cy = c / max_map_size;
      cx = c % max_map_size; // Needed for bounds checking (and map access)
    } while (!fin);
  else
    do {
      expand2();
      c = sig(c);
      if (c == 65535)
        break;
      cy = c / max_map_size;
      cx = c % max_map_size; // Needed for bounds checking (and map access)
    } while (!fin);

  // If (fin==1) cell (bx,by) was reached, and the path is extracted from dis()

  if (!fin) { // No path found to bx,by
    m(ax, ay) = cas_inicial;
    return;
  }

  //----------------------------------------------------------------------------

  // Path found, now extract the vertices into the table passed as parameter

  pila[sp] = calculate_vertices(&mem[offset], size / 2, mem[id + _X], mem[id + _Y], x, y);

  m(ax, ay) = cas_inicial;
}

//----------------------------------------------------------------------------
//  Expand a cell
//----------------------------------------------------------------------------

void expand(void) { // Fast version
  int n = 0;

  // Check map bounds and add the four cardinal neighbors

  if (cx)
    if (!m(cx - 1, cy)) {
      n |= 1;
      if (!dis(c - 1))
        add(cx - 1, cy, c - 1, 10);
    }
  if (cx < w - 1)
    if (!m(cx + 1, cy)) {
      n |= 2;
      if (!dis(c + 1))
        add(cx + 1, cy, c + 1, 10);
    }
  if (cy)
    if (!m(cx, cy - 1)) {
      n |= 4;
      if (!dis(c - max_map_size))
        add(cx, cy - 1, c - max_map_size, 10);
    }
  if (cy < h - 1)
    if (!m(cx, cy + 1)) {
      n |= 8;
      if (!dis(c + max_map_size))
        add(cx, cy + 1, c + max_map_size, 10);
    }

  // Now add the four diagonal neighbors

  if ((n & 5) == 5)
    if (!dis(c - 1 - max_map_size))
      if (!m(cx - 1, cy - 1))
        add(cx - 1, cy - 1, c - 1 - max_map_size, 14);
  if ((n & 9) == 9)
    if (!dis(c - 1 + max_map_size))
      if (!m(cx - 1, cy + 1))
        add(cx - 1, cy + 1, c - 1 + max_map_size, 14);
  if ((n & 6) == 6)
    if (!dis(c + 1 - max_map_size))
      if (!m(cx + 1, cy - 1))
        add(cx + 1, cy - 1, c + 1 - max_map_size, 14);
  if ((n & 10) == 10)
    if (!dis(c + 1 + max_map_size))
      if (!m(cx + 1, cy + 1))
        add(cx + 1, cy + 1, c + 1 + max_map_size, 14);
}

void expand2(void) { // Exact version
  int n = 0;

  // Check map bounds and add the four cardinal neighbors

  if (cx)
    if (!m(cx - 1, cy)) {
      n |= 1;
      if (!dis(c - 1))
        add2(c - 1, 10);
    }
  if (cx < w - 1)
    if (!m(cx + 1, cy)) {
      n |= 2;
      if (!dis(c + 1))
        add2(c + 1, 10);
    }
  if (cy)
    if (!m(cx, cy - 1)) {
      n |= 4;
      if (!dis(c - max_map_size))
        add2(c - max_map_size, 10);
    }
  if (cy < h - 1)
    if (!m(cx, cy + 1)) {
      n |= 8;
      if (!dis(c + max_map_size))
        add2(c + max_map_size, 10);
    }

  // Now add the four diagonal neighbors

  if ((n & 5) == 5)
    if (!dis(c - 1 - max_map_size))
      if (!m(cx - 1, cy - 1))
        add2(c - 1 - max_map_size, 14);
  if ((n & 9) == 9)
    if (!dis(c - 1 + max_map_size))
      if (!m(cx - 1, cy + 1))
        add2(c - 1 + max_map_size, 14);
  if ((n & 6) == 6)
    if (!dis(c + 1 - max_map_size))
      if (!m(cx + 1, cy - 1))
        add2(c + 1 - max_map_size, 14);
  if ((n & 10) == 10)
    if (!dis(c + 1 + max_map_size))
      if (!m(cx + 1, cy + 1))
        add2(c + 1 + max_map_size, 14);
}

//----------------------------------------------------------------------------
//  Add a new cell
//----------------------------------------------------------------------------

void add(int x, int y, word bnew, word step) { // Fast version
  word cdis;                                   // Distance
  word i, a;                                   // Next and previous

  dis(bnew) = dis(c) + step; // Store its distance

  i = abs(x - bx);
  a = abs(y - by);
  cdis = dis2(bnew) = (i < a) ? (i >> 2) + a : i + (a >> 2);

  a = c;
  do { // Find the insertion point for this new cell
    i = a;
    a = sig(a);
    if (a == 65535)
      break;
  } while (cdis > dis2(a));

  sig(i) = bnew;
  sig(bnew) = a;

  if (bnew == b)
    fin = 1;
}

void add2(word bnew, word step) { // Exact version
  word i, a;                      // Next and previous

  dis(bnew) = dis(c) + step; // Store its distance

  a = c;
  do { // Find the insertion point for this new cell
    i = a;
    a = sig(a);
    if (a == 65535)
      break;
  } while (dis(bnew) > dis(a));

  sig(i) = bnew;
  sig(bnew) = a;

  if (bnew == b)
    fin = 1;
}

//----------------------------------------------------------------------------
//  Initialize search structures - Returns 1 if any allocation failed
//----------------------------------------------------------------------------

int init_find(void) {
  find_status = 1;

  distancias = (word *)malloc(max_map_size * max_map_size * 2);
  if (distancias == NULL)
    return (1);
  distancias2 = (word *)malloc(max_map_size * max_map_size * 2);
  if (distancias2 == NULL) {
    free(distancias);
    return (1);
  }
  siguientes = (word *)malloc(max_map_size * max_map_size * 2);
  if (siguientes == NULL) {
    free(distancias2);
    free(distancias);
    return (1);
  }

  return (0);
}

//----------------------------------------------------------------------------
//  Calculate all vertices the path must pass through, from (x1,y1) to (x0,y0)
//  Returns the number of vertices, or 0 if there are too many ...
//----------------------------------------------------------------------------

int calculate_vertices(int *ptr, int max_ver, int x0, int y0, int x1, int y1) {
  int *p = ptr + max_ver * 2; // Fill from last to first, then memmove
  int num = max_ver;          // Count of vertices inserted into *ptr
  int d;                      // Distance from current cell to start
  int x, y;                   // Next point
  int xx, yy;                 // Current point (the last one reachable safely)
  int newdir = 0;             // Temp (direction to the next cell)
  int dir;                    // Available directions ...
  int cas;                    // Current cell (bx+by*max_map_size)
  int n;                      // Simple counter
  int nextx[8], nexty[8];     // The next eight cells (ix,iy from bx,by)

  if (num < 1)
    return (0);
  *(--p) = y1;
  *(--p) = x1;
  num--; // Insert the last vertex

  x = bx * tile + tile / 2;
  y = by * tile + tile / 2;
  cas = bx + by * max_map_size;
  fin = 0;

  do {
    do {
      d = dis(cas); // Get the next (bx,by) and (x,y)

      if (d > 14 * 8) {
        xx = bx;
        yy = by;

        for (n = 0; n < 8; n++) {
          dir = 0;
          if (xx)
            if (dis(cas - 1)) {
              dir |= 1;
              if (dis(cas - 1) <= d) {
                d = dis(cas - 1);
                newdir = 1;
              }
            }
          if (xx < w - 1)
            if (dis(cas + 1)) {
              dir |= 2;
              if (dis(cas + 1) <= d) {
                d = dis(cas + 1);
                newdir = 2;
              }
            }
          if (yy)
            if (dis(cas - max_map_size)) {
              dir |= 4;
              if (dis(cas - max_map_size) <= d) {
                d = dis(cas - max_map_size);
                newdir = 3;
              }
            }
          if (yy < h - 1)
            if (dis(cas + max_map_size)) {
              dir |= 8;
              if (dis(cas + max_map_size) <= d) {
                d = dis(cas + max_map_size);
                newdir = 4;
              }
            }

          if ((dir & 5) == 5)
            if (dis(cas - 1 - max_map_size))
              if (dis(cas - 1 - max_map_size) <= d) {
                d = dis(cas - 1 - max_map_size);
                newdir = 5;
              }
          if ((dir & 9) == 9)
            if (dis(cas - 1 + max_map_size))
              if (dis(cas - 1 + max_map_size) <= d) {
                d = dis(cas - 1 + max_map_size);
                newdir = 6;
              }
          if ((dir & 6) == 6)
            if (dis(cas + 1 - max_map_size))
              if (dis(cas + 1 - max_map_size) <= d) {
                d = dis(cas + 1 - max_map_size);
                newdir = 7;
              }
          if ((dir & 10) == 10)
            if (dis(cas + 1 + max_map_size))
              if (dis(cas + 1 + max_map_size) <= d) {
                d = dis(cas + 1 + max_map_size);
                newdir = 8;
              }

          switch (newdir) {
          case 1:
            xx--;
            cas += -1;
            break;
          case 2:
            xx++;
            cas += 1;
            break;
          case 3:
            yy--;
            cas += -max_map_size;
            break;
          case 4:
            yy++;
            cas += max_map_size;
            break;
          case 5:
            xx--;
            yy--;
            cas += -1 - max_map_size;
            break;
          case 6:
            xx--;
            yy++;
            cas += -1 + max_map_size;
            break;
          case 7:
            xx++;
            yy--;
            cas += 1 - max_map_size;
            break;
          case 8:
            xx++;
            yy++;
            cas += 1 + max_map_size;
            break;
          }

          nextx[n] = xx - bx;
          nexty[n] = yy - by;
        }

        can_go(x1, y1, x + nextx[7] * tile, y + nexty[7] * tile);
        if (!choque_linea) {
          bx += nextx[7];
          by += nexty[7];
        } else {
          can_go(x1, y1, x + nextx[3] * tile, y + nexty[3] * tile);
          if (!choque_linea) {
            bx += nextx[3];
            by += nexty[3];
          } else {
            can_go(x1, y1, x + nextx[1] * tile, y + nexty[1] * tile);
            if (!choque_linea) {
              bx += nextx[1];
              by += nexty[1];
            } else {
              can_go(x1, y1, x + nextx[0] * tile, y + nexty[0] * tile);
              bx += nextx[0];
              by += nexty[0];
            }
          }
        }

        xx = x;
        yy = y; // from x1,y1 to xx,yy it CAN always reach
        x = bx * tile + tile / 2;
        y = by * tile + tile / 2;
        cas = bx + by * max_map_size;

      } else {
        xx = x;
        yy = y;
        dir = 0; // from x1,y1 to xx,yy it CAN always reach

        if (bx)
          if (dis(cas - 1)) {
            dir |= 1;
            if (dis(cas - 1) <= d) {
              d = dis(cas - 1);
              newdir = 1;
            }
          }
        if (bx < w - 1)
          if (dis(cas + 1)) {
            dir |= 2;
            if (dis(cas + 1) <= d) {
              d = dis(cas + 1);
              newdir = 2;
            }
          }
        if (by)
          if (dis(cas - max_map_size)) {
            dir |= 4;
            if (dis(cas - max_map_size) <= d) {
              d = dis(cas - max_map_size);
              newdir = 3;
            }
          }
        if (by < h - 1)
          if (dis(cas + max_map_size)) {
            dir |= 8;
            if (dis(cas + max_map_size) <= d) {
              d = dis(cas + max_map_size);
              newdir = 4;
            }
          }

        if ((dir & 5) == 5)
          if (dis(cas - 1 - max_map_size))
            if (dis(cas - 1 - max_map_size) <= d) {
              d = dis(cas - 1 - max_map_size);
              newdir = 5;
            }
        if ((dir & 9) == 9)
          if (dis(cas - 1 + max_map_size))
            if (dis(cas - 1 + max_map_size) <= d) {
              d = dis(cas - 1 + max_map_size);
              newdir = 6;
            }
        if ((dir & 6) == 6)
          if (dis(cas + 1 - max_map_size))
            if (dis(cas + 1 - max_map_size) <= d) {
              d = dis(cas + 1 - max_map_size);
              newdir = 7;
            }
        if ((dir & 10) == 10)
          if (dis(cas + 1 + max_map_size))
            if (dis(cas + 1 + max_map_size) <= d) {
              d = dis(cas + 1 + max_map_size);
              newdir = 8;
            }

        switch (newdir) {
        case 1:
          bx--;
          cas += -1;
          x -= tile;
          break;
        case 2:
          bx++;
          cas += 1;
          x += tile;
          break;
        case 3:
          by--;
          cas += -max_map_size;
          y -= tile;
          break;
        case 4:
          by++;
          cas += max_map_size;
          y += tile;
          break;
        case 5:
          bx--;
          by--;
          cas += -1 - max_map_size;
          x -= tile;
          y -= tile;
          break;
        case 6:
          bx--;
          by++;
          cas += -1 + max_map_size;
          x -= tile;
          y += tile;
          break;
        case 7:
          bx++;
          by--;
          cas += 1 - max_map_size;
          x += tile;
          y -= tile;
          break;
        case 8:
          bx++;
          by++;
          cas += 1 + max_map_size;
          x += tile;
          y += tile;
          break;
        }

        can_go(x1, y1, x, y); // sets choque_linea=0/1
      }

      if (bx == ax && by == ay) {
        fin = 1;
        break;
      }

    } while (!choque_linea);

    if (choque_linea) { // Add a new vertex to the path
      if (!num)
        return (0);
      *(--p) = yy;
      *(--p) = xx;
      num--;
      x1 = xx;
      y1 = yy;
    }

  } while (!fin);

  if (x != x0 || y != y0) { // Is it necessary to go to the center of the first cell?
    can_go(x1, y1, x0, y0);
    if (choque_linea) {
      if (!num)
        return (0);
      *(--p) = y;
      *(--p) = x;
      num--; // Unlikely case, but just in case ...
    }
  }

  if (num) {
    memmove((byte *)ptr, (byte *)p, (max_ver - num) * 8);
  }
  return (max_ver - num);
}

//----------------------------------------------------------------------------
//  Determine if a straight line path is possible (pathline/calculate_vertices)
//----------------------------------------------------------------------------

void can_go(int x0, int y0, int x1, int y1) {
  int tilesize;
  int dx, dy, a, b, d, x, y;

  choque_linea = 0;
  tilesize = tile;

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

  if (dx || dy) {
    if (dy <= dx) {
      if (x0 > x1) {
        if (m(x1 / tilesize, y1 / tilesize))
          choque_linea = 1;
        x0--;
        swap(x0, x1);
        swap(y0, y1);
      }
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
          if (m(x / tilesize, y / tilesize))
            choque_linea = 1;
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
          if (m(x / tilesize, y / tilesize))
            choque_linea = 1;
        }
    } else {
      if (y0 > y1) {
        if (m(x1 / tilesize, y1 / tilesize))
          choque_linea = 1;
        y0--;
        swap(x0, x1);
        swap(y0, y1);
      }
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
          if (m(x / tilesize, y / tilesize))
            choque_linea = 1;
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
          if (m(x / tilesize, y / tilesize))
            choque_linea = 1;
        }
    }
  }
}

//----------------------------------------------------------------------------
//  path_line(file,code,tilesize,x,y)
//----------------------------------------------------------------------------

void path_line(void) {
  int file, code, x, y;
  int *ptr;

  y = pila[sp--];
  x = pila[sp--];
  tile = pila[sp--];
  code = pila[sp--];
  file = pila[sp];

  pila[sp] = 0; // Default return value, until proven otherwise

  if (tile < 1 || tile > 256) {
    e(151);
    return;
  } // Tile size bounds

  // Check file and code bounds

  if (file > max_fpgs || file < 0) {
    e(109);
    return;
  }
  if (file)
    max_grf = 1000;
  else
    max_grf = 2000;
  if (code <= 0 || code >= max_grf) {
    e(110);
    return;
  }
  if (g[file].grf == NULL) {
    e(111);
    return;
  }
  if ((ptr = g[file].grf[code]) == NULL) {
    e(121);
    return;
  }

  // Get pointer to the map, width and height

  w = ptr[13];
  h = ptr[14];
  map = (byte *)ptr + 64 + ptr[15] * 4;
  if (w < 1 || h < 1 || w > max_map_size || h > max_map_size) {
    e(152);
    return;
  }

  // Check coordinate bounds (if outside the map, return 0)

  if (x < 0 || y < 0 || x >= w * tile || y >= h * tile)
    return;
  ax = mem[id + _X];
  ay = mem[id + _Y];
  if (ax < 0 || ay < 0 || ax >= w * tile || ay >= h * tile)
    return;

  // Determine if there is a clear path from (ax,ay) to (x,y)

  can_go(ax, ay, x, y);
  if (!choque_linea)
    pila[sp] = 1;
}

//----------------------------------------------------------------------------
//  path_free(file,code,tilesize,x,y)
//----------------------------------------------------------------------------

void path_free(void) {
  int file, code, x, y;
  int *ptr;

  y = pila[sp--];
  x = pila[sp--];
  tile = pila[sp--];
  code = pila[sp--];
  file = pila[sp];

  pila[sp] = 0; // Default return value, until proven otherwise

  if (tile < 1 || tile > 256) {
    e(151);
    return;
  } // Tile size bounds

  // Check file and code bounds

  if (file > max_fpgs || file < 0) {
    e(109);
    return;
  }
  if (file)
    max_grf = 1000;
  else
    max_grf = 2000;
  if (code <= 0 || code >= max_grf) {
    e(110);
    return;
  }
  if (g[file].grf == NULL) {
    e(111);
    return;
  }
  if ((ptr = (int *)g[file].grf[code]) == NULL) {
    e(121);
    return;
  }

  // Get pointer to the map, width and height

  w = ptr[13];
  h = ptr[14];
  map = (byte *)ptr + 64 + ptr[15] * 4;
  if (w < 1 || h < 1 || w > max_map_size || h > max_map_size) {
    e(152);
    return;
  }
  // Check coordinate bounds (if outside the map, return 0)
  if (x < 0 || y < 0 || x >= w * tile || y >= h * tile)
    return;

  // Determine if the destination cell is free

  if (!m(x / tile, y / tile))
    pila[sp] = 1;
}

//----------------------------------------------------------------------------
