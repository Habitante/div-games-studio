
//-----------------------------------------------------------------------------
//      Drawing module — core state, primitives, UI framework
//      (Tools in paint_tools.c, selection in paint_select.c)
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#include "paint_internal.h"


//-----------------------------------------------------------------------------
//      Drawing program toolbars
//-----------------------------------------------------------------------------

struct paint_toolbar toolbars[10];

//-----------------------------------------------------------------------------
//      Constants
//-----------------------------------------------------------------------------

#define COLOR_RULER_X 96 // Color gradient start position on the edit screen
#define max_int       65536

extern int quick_close;
int determine_help(void);
extern int help_paint_active;

int back; // Whether an undo is needed after blit_screen

//-----------------------------------------------------------------------------
//      Global variables for this module
//-----------------------------------------------------------------------------

int zoom_dx, zoom_dy, zoom_sx, zoom_sy;
int zoom_speed = 4; // higher -> slower, lower -> faster
int tab_cycling = 0;

int hotkey = 1; // Whether hotkeys are enabled (disabled during text toolbar input)

int point_index = 0;

//-----------------------------------------------------------------------------

int line_fx = 16; // Drawing mode for the line

int pencil_tool_effect = 16, line_tool_effect = 16, box_tool_effect = 16, bezier_tool_effect = 16,
    polygon_tool_effect = 16;
int filled_tool_effect = 16, circle_tool_effect = 16, spray_tool_effect = 4, text_tool_effect = 16;

int fill_seeds[128]; // Fill seed points (x,y), up to 64 different (-1,?) -> n/a

int blur_enabled = 0;

//-----------------------------------------------------------------------------
//      Brush-related variables
//-----------------------------------------------------------------------------

// The brush is a bitmap of intensities between 0 (0%) and 16 (100%)

byte default_brush[16] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

byte *brush = &default_brush[0];
int brush_w = 1, brush_h = 1;

int brush_type = 0; // 0-b/w, 1-grayscale
int brush_index = 0;

//-----------------------------------------------------------------------------
//      Texture-related variables
//-----------------------------------------------------------------------------

byte *texture_color = NULL; // When NULL, draw with solid color
int texture_w = 2, texture_h = 2;
int texture_x = 0, texture_y = 0;

//-----------------------------------------------------------------------------
//      Get color from the texture
//-----------------------------------------------------------------------------

void remove_texture(void) {
  if (texture_color != NULL) {
    texture_color = NULL;
  }
}

byte get_color(int x, int y) {
  return (
      *(texture_color + (x + texture_x) % texture_w + ((y + texture_y) % texture_h) * texture_w));
}

//-----------------------------------------------------------------------------
//      Edit screen (generic editing, no draw_mode defined)
//-----------------------------------------------------------------------------

void test_mouse_box(int a, int b, int c, int d) {
  if (mouse_in(a, b, a + c * big2 - 1, b + d * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = a + (mouse_shift_x - a) / 2;
      mouse_y = b + (mouse_shift_y - b) / 2;
    }
    if (mouse_in(a + 2, b + 2, a + c - 10, b + 9))
      mouse_graf = 2;
    else if (mouse_in(a + c - 9, b + 2, a + c - 2, b + 9))
      mouse_graf = 5;
    else
      mouse_graf = 1;
  } else if (mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width * big2 - 1,
                      toolbar_y + 19 * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = toolbar_x + (mouse_shift_x - toolbar_x) / 2;
      mouse_y = toolbar_y + (mouse_shift_y - toolbar_y) / 2;
    }
    if (mouse_in(toolbar_x, toolbar_y, toolbar_x + 9, toolbar_y + 9))
      mouse_graf = 2;
    else if (mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))
      mouse_graf = 5;
    else
      mouse_graf = 1;
  } else
    mouse_graf = 1;
}

void test_mouse_box2(int a, int b, int c, int d) {
  if (mouse_in(a, b, a + c * big2 - 1, b + d * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = a + (mouse_shift_x - a) / 2;
      mouse_y = b + (mouse_shift_y - b) / 2;
    }
    if (mouse_in(a, b + 2, a + c - 10, b + 9))
      mouse_graf = 2;
    else if (mouse_in(a + c - 9, b + 2, a + c - 2, b + 9))
      mouse_graf = 5;
    else
      mouse_graf = 1;
  } else if (mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width * big2 - 1,
                      toolbar_y + 19 * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = toolbar_x + (mouse_shift_x - toolbar_x) / 2;
      mouse_y = toolbar_y + (mouse_shift_y - toolbar_y) / 2;
    }
    if (mouse_in(toolbar_x, toolbar_y, toolbar_x + 9, toolbar_y + 9))
      mouse_graf = 2;
    else if (mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))
      mouse_graf = 5;
    else
      mouse_graf = 1;
  } else if (mouse_in(zoom_win_x, zoom_win_y, zoom_win_x + zoom_win_width - 1,
                      zoom_win_y + zoom_win_height - 1))
    mouse_graf = current_mouse - zoom;
  else
    mouse_graf = 1;
}

void test_mouse(void) {
  if (mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width * big2 - 1,
               toolbar_y + 19 * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = toolbar_x + (mouse_shift_x - toolbar_x) / 2;
      mouse_y = toolbar_y + (mouse_shift_y - toolbar_y) / 2;
    }
    if (mouse_in(toolbar_x, toolbar_y, toolbar_x + 9, toolbar_y + 9))
      mouse_graf = 2;
    else if (mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))
      mouse_graf = 5;
    else
      mouse_graf = 1;
  } else if (mouse_in(zoom_win_x, zoom_win_y, zoom_win_x + zoom_win_width - 1,
                      zoom_win_y + zoom_win_height - 1))
    mouse_graf = current_mouse - zoom;
  else
    mouse_graf = 1;
}

//-----------------------------------------------------------------------------
//      Variables shared with paint_tools.c (defined here, extern in header)
//-----------------------------------------------------------------------------

int bezier_x, bezier_y;
int tension = 16;
int text_bar_active = 0; // Prevent background highlight toggle with 'b' key

int fill_icons[] = {4, 1, 114, 115, 116, 118, 1};

// NOTE: edit_scr, edit_mode_0..9, edit_mode_11..13 moved to paint_tools.c
// NOTE: block_bar, edit_mode_10, selection/block ops moved to paint_select.c


//-----------------------------------------------------------------------------
//      Zoom a region, without undo
//-----------------------------------------------------------------------------

void zoom_region(int x, int y, int w, int h) {
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
    return;

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
}


//-----------------------------------------------------------------------------
//      Draw a Bezier curve
//-----------------------------------------------------------------------------

void bezier(int x0, int y0, int x1, int y1, int _x0, int _y0, int _x1, int _y1, int inc0) {
  float t, t2, t3, it, it2, it3;
  int n = 0;
  int a, b;
  int p[66];
  int x, y, _x, _y;

  x = _x = x0;
  y = _y = y0;

  do {
    t = (float)n / 32;
    it = 1 - t;
    t2 = t * t;
    t3 = t2 * t;
    it2 = it * it;
    it3 = it2 * it;
    a = it3 * x0 + 3 * t * it2 * _x0 + 3 * t2 * it * _x1 + t3 * x1;
    b = it3 * y0 + 3 * t * it2 * _y0 + 3 * t2 * it * _y1 + t3 * y1;
    p[n * 2] = a;
    p[n * 2 + 1] = b;
    if (a < x) {
      x = a;
    }
    if (a > _x) {
      _x = a;
    }
    if (b < y) {
      y = b;
    }
    if (b > _y) {
      _y = b;
    }
  } while (n++ < 32);

  if (save_undo(x - brush_w / 2, y - brush_h / 2, _x - x + 1 + brush_w, _y - y + 1 + brush_h)) {
    if (inc0)
      line_pixel(p[0], p[1]);
    n = 1;
    do {
      if (p[n * 2] != p[n * 2 - 2] || p[n * 2 + 1] != p[n * 2 - 1])
        line(p[n * 2 - 2], p[n * 2 - 1], p[n * 2], p[n * 2 + 1], -1);
    } while (n++ < 32);

    bezier_x = p[48];
    bezier_y = p[49];
  }
}

//-----------------------------------------------------------------------------
//      Draw a line from x0,y0 .. to x1,y1
//      'inc0' indicates whether to draw the first pixel of the line
//-----------------------------------------------------------------------------

void line0(int x0, int y0, int x1, int y1, int inc0) {
  int dx = 0, dy = 0, a = 0, b = 0, d = 0, x = 0, y = 0;
  int64_t unded = 1;

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

  if (inc0 == -1)
    inc0 = 0;
  else
    unded =
        (intptr_t)save_undo(x - brush_w / 2, y - brush_h / 2, dx + 1 + brush_w, dy + 1 + brush_h);

  if (unded) {
    if (!dx && !dy)
      line_pixel(x0, y0);
    else {
      if (inc0)
        line_pixel(x0, y0);
      if (dy <= dx) {
        if (x0 > x1) {
          line_pixel(x1, y1);
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
        if (y0 > y1) {
          line_pixel(x1, y1);
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
      }
    }
  }
}

// Extended version that also draws bottom-to-top and right-to-left

void line(int x0, int y0, int x1, int y1, int inc0) {
  int dx = 0, dy = 0, a = 0, b = 0, d = 0, x = 0, y = 0;
  int64_t unded = 1;

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

  if (inc0 == -1)
    inc0 = 0;
  else
    unded =
        (intptr_t)save_undo(x - brush_w / 2, y - brush_h / 2, dx + 1 + brush_w, dy + 1 + brush_h);

  if (unded) {
    if (!dx && !dy)
      line_pixel(x0, y0);
    else {
      if (inc0)
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
}

//-----------------------------------------------------------------------------
//      Draw a pixel, based on line_fx and color
//-----------------------------------------------------------------------------

int color256;

void pixel(byte *p) {
  byte a, b, c, d;
  int n;

  if (mask_on)
    if (mask[*p])
      return;

  switch (line_fx) {
  case 0: // Remove a color
    if (*p == color)
      *p = 0;
    break;

  case 1:
    n = (memptrsize)*p * 256;
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

  case 2:
    n = (memptrsize)*p * 256;
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

  case 3:
    n = (memptrsize)*p * 256;
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

  case 4: // *** 25% ****
    n = (memptrsize)*p * 256;
    a = *(ghost + n + color);
    b = *(ghost + n + a);
    if (b != *p)
      *p = b;
    else if (a != *p)
      *p = a;
    else
      *p = color;
    break;

  case 5:
    n = (memptrsize)*p * 256;
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

  case 6:
    n = (memptrsize)*p * 256;
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

  case 7:
    n = (memptrsize)*p * 256;
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

  case 8: // *** 50% ****
    n = (memptrsize)*p * 256;
    a = *(ghost + n + color);
    if (a != *p)
      *p = a;
    else
      *p = color;
    break;

  case 9:
    n = (memptrsize)*p * 256;
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

  case 10:
    n = (memptrsize)*p * 256;
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

  case 11:
    n = (memptrsize)*p * 256;
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

  case 12: // *** 75% ****
    n = (memptrsize)*p * 256;
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

  case 13:
    n = (memptrsize)*p * 256;
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

  case 14:
    n = (memptrsize)*p * 256;
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

  case 15:
    n = (memptrsize)*p * 256;
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

  case 16: // Set a color
    *p = color;
    break;
  }
}

void pixel_sin_mask(byte *p) {
  byte a, b, c, d;
  int n;

  switch (line_fx) {
  case 0: // Remove a color
    if (*p == color)
      *p = 0;
    break;

  case 1:
    n = (memptrsize)*p * 256;
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

  case 2:
    n = (memptrsize)*p * 256;
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

  case 3:
    n = (memptrsize)*p * 256;
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

  case 4: // *** 25% ****
    n = (memptrsize)*p * 256;
    a = *(ghost + n + color);
    b = *(ghost + n + a);
    if (b != *p)
      *p = b;
    else if (a != *p)
      *p = a;
    else
      *p = color;
    break;

  case 5:
    n = (memptrsize)*p * 256;
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

  case 6:
    n = (memptrsize)*p * 256;
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

  case 7:
    n = (memptrsize)*p * 256;
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

  case 8: // *** 50% ****
    n = (memptrsize)*p * 256;
    a = *(ghost + n + color);
    if (a != *p)
      *p = a;
    else
      *p = color;
    break;

  case 9:
    n = (memptrsize)*p * 256;
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

  case 10:
    n = (memptrsize)*p * 256;
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

  case 11:
    n = (memptrsize)*p * 256;
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

  case 12: // *** 75% ****
    n = (memptrsize)*p * 256;
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

  case 13:
    n = (memptrsize)*p * 256;
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

  case 14:
    n = (memptrsize)*p * 256;
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

  case 15:
    n = (memptrsize)*p * 256;
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

  case 16: // Set a color
    *p = color;
    break;
  }
}

void line_pixel(int x, int y) { // Brush
  int xx = 0, yy = 0, _line_fx = line_fx;
  byte *q, *p, col;

  x -= brush_w / 2;
  y -= brush_h / 2;

  if (texture_color == NULL) {
    if (brush_type) {
      if (x >= 0 && y >= 0 && x + brush_w <= map_width && y + brush_h <= map_height &&
          !blur_enabled) {
        q = map + x + y * map_width;
        p = brush;
        if (mask_on) {
          do {
            do {
              if ((line_fx = *p) && *q != color) {
                pixel(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        } else {
          do {
            do {
              if ((line_fx = *p) && *q != color) {
                pixel_sin_mask(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        }
      } else {
        do {
          do {
            if ((line_fx = brush[xx + yy * brush_w])) {
              _line_pixel(x + xx, y + yy);
            }
          } while (++xx < brush_w);
          xx = 0;
        } while (++yy < brush_h);
      }
    } else {
      if (x >= 0 && y >= 0 && x + brush_w <= map_width && y + brush_h <= map_height &&
          !blur_enabled) {
        q = map + x + y * map_width;
        p = brush;
        if (mask_on) {
          do {
            do {
              if (*p && *q != color) {
                pixel(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        } else {
          do {
            do {
              if (*p && *q != color) {
                pixel_sin_mask(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        }
      } else {
        do {
          do {
            if (brush[xx + yy * brush_w]) {
              _line_pixel(x + xx, y + yy);
            }
          } while (++xx < brush_w);
          xx = 0;
        } while (++yy < brush_h);
      }
    }
  } else {
    col = color;
    if (brush_type) {
      if (x >= 0 && y >= 0 && x + brush_w <= map_width && y + brush_h <= map_height &&
          !blur_enabled) {
        q = map + x + y * map_width;
        p = brush;
        if (mask_on) {
          do {
            do {
              color = get_color(x + xx, y + yy);
              if ((line_fx = *p) && *q != color) {
                pixel(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        } else {
          do {
            do {
              color = get_color(x + xx, y + yy);
              if ((line_fx = *p) && *q != color) {
                pixel_sin_mask(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        }
      } else {
        do {
          do {
            if ((line_fx = brush[xx + yy * brush_w])) {
              color = get_color(x + xx, y + yy);
              _line_pixel(x + xx, y + yy);
            }
          } while (++xx < brush_w);
          xx = 0;
        } while (++yy < brush_h);
      }
    } else {
      if (x >= 0 && y >= 0 && x + brush_w <= map_width && y + brush_h <= map_height &&
          !blur_enabled) {
        q = map + x + y * map_width;
        p = brush;
        if (mask_on) {
          do {
            do {
              color = get_color(x + xx, y + yy);
              if (*p && *q != color) {
                pixel(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        } else {
          do {
            do {
              color = get_color(x + xx, y + yy);
              if (*p && *q != color) {
                pixel_sin_mask(q);
              }
              q++;
              p++;
            } while (++xx < brush_w);
            xx = 0;
            q -= brush_w - map_width;
          } while (++yy < brush_h);
        }
      } else {
        do {
          do {
            if (brush[xx + yy * brush_w]) {
              color = get_color(x + xx, y + yy);
              _line_pixel(x + xx, y + yy);
            }
          } while (++xx < brush_w);
          xx = 0;
        } while (++yy < brush_h);
      }
    }
    color = col;
  }
  line_fx = _line_fx;
}

void _line_pixel(int x, int y) { // A single pixel (for line_pixel, spray and writes)
  byte c1, c2, _color = color;

  if (x >= 0 && y >= 0 && x < map_width && y < map_height) {
    if (blur_enabled) {
      if (x < map_width - 1 && x > 0)
        c1 = *(ghost + *(map + x + y * map_width - 1) * 256 + *(map + x + y * map_width + 1));
      else
        c1 = *(map + x + y * map_width);
      if (y < map_height - 1 && y > 0)
        c2 = *(ghost + *(map + x + (y - 1) * map_width) * 256 + *(map + x + (y + 1) * map_width));
      else
        c2 = *(map + x + y * map_width);
      _color = color;
      color = *(ghost + (memptrsize)c1 * 256 + c2);
      pixel(map + x + y * map_width);
      color = _color;
    } else
      pixel(map + x + y * map_width);
  }
}

//-----------------------------------------------------------------------------
//      Draw a box, based on mode_rect, line_fx and color
//-----------------------------------------------------------------------------

void draw_box(int x0, int y0, int x1, int y1) {
  int w, h;

  if (x0 > x1)
    swap(x0, x1);
  w = x1 - x0 + 1;
  if (y0 > y1)
    swap(y0, y1);
  h = y1 - y0 + 1;

  if (save_undo(x0 - brush_w / 2, y0 - brush_h / 2, w + brush_w, h + brush_h)) {
    if (mode_rect) {
      do {
        circle_scanline_filled(x0, y0++, w);
      } while (--h);
    } else {
      circle_scanline(x0, y0, w);
      circle_scanline(x0, y0 + h - 1, w);
      h -= 2;
      y0++;
      if (h > 0)
        do {
          line_pixel(x0, y0);
          line_pixel(x0 + w - 1, y0++);
        } while (--h);
    }
  }
}

//-----------------------------------------------------------------------------
//      Draw a circle, based on mode_circle, line_fx and color
//-----------------------------------------------------------------------------

void draw_circle(int x0, int y0, int x1, int y1, int filled) {
  int p[2048];   // Points on the circumference
  double cx, rx; // Center and radius of the circumference
  int w, h;      // Width and height
  double y, ymed, nsin;
  int n, xa, xb, ya, yb;

  if (x0 > x1)
    swap(x0, x1);
  w = x1 - x0 + 1;
  if (y0 > y1)
    swap(y0, y1);
  h = y1 - y0 + 1;
  if (h > 1024)
    return;

  if (save_undo(x0 - brush_w / 2, y0 - brush_h / 2, w + brush_w, h + brush_h)) {
    rx = (double)(w - 1) / 2.0;
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
    if (h > 2 && w > 2 && p[0] == p[4] && p[0] + 1 < p[1]) {
      p[0] = p[4] + 1;
      p[1] = p[5] - 1;
    }
    if (h == 2) {
      p[0] = x0;
      p[1] = x1;
      p[4] = x0;
      p[5] = x1;
    }

    if (filled) {
      do {
        yb = p[--n];
        ya = p[--n];
        xb = p[--n];
        xa = p[--n];
        circle_scanline_filled(xa, ya, xb - xa + 1);
        if (yb != ya)
          circle_scanline_filled(xa, yb, xb - xa + 1);
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
            circle_scanline(xa, ya, p[n - 4] - xa);
            circle_scanline(p[n - 3] + 1, ya, xb - p[n - 3]);
            if (yb != ya) {
              circle_scanline(xa, yb, p[n - 4] - xa);
              circle_scanline(p[n - 3] + 1, yb, xb - p[n - 3]);
            }
          }
        } else {
          circle_scanline(xa, ya, xb - xa + 1);
          if (yb != ya)
            circle_scanline(xa, yb, xb - xa + 1);
        }
      } while (n);
    }
  }
}

void circle_scanline(int x, int y, int w) {
  line_pixel(x, y);
  line(x, y, x + w - 1, y, -1);
}

void circle_scanline_filled(int x, int y, int w) {
  byte *p, col;
  if (texture_color == NULL) {
    if (y >= 0 && y < map_height && x < map_width && x + w > 0) {
      if (x < 0) {
        w += x;
        x = 0;
      }
      if (x + w > map_width) {
        w = map_width - x;
      }
      p = map + y * map_width + x;
      do
        pixel(p++);
      while (--w);
    }
  } else {
    col = color;
    if (y >= 0 && y < map_height && x < map_width && x + w > 0) {
      if (x < 0) {
        w += x;
        x = 0;
      }
      if (x + w > map_width) {
        w = map_width - x;
      }
      p = map + y * map_width + x;
      do {
        color = get_color(x++, y);
        pixel(p++);
      } while (--w);
    }
    color = col;
  }
}

//-----------------------------------------------------------------------------
//      Detect color gradient interaction within an editing toolbar
//-----------------------------------------------------------------------------

void edit_ruler(void) {
  int n;

  read_mouse();
  select_zoom();
  test_mouse();
  if ((key(_L_SHIFT) || key(_R_SHIFT)) && (mouse_b & 1) && mouse_graf >= 10) {
    color = *(map + coord_y * map_width + coord_x);
    remove_texture();
    draw_ruler();
    mouse_b = 0;
  }

  if ((mouse_b & 1) && mouse_in(toolbar_x + COLOR_RULER_X, toolbar_y,
                                toolbar_x + COLOR_RULER_X + 127, toolbar_y + 18)) {
    if (editable(&n))
      gradients[gradient].colors[n] = color;
    else
      color = gradients[gradient].colors[n];
    remove_texture();
    draw_ruler();
  }

  if ((mouse_b & 1) && selected_icon == 11) {
    do {
      read_mouse();
    } while (mouse_b & 1);
    eyedropper();
    draw_ruler();
    do {
      read_mouse();
    } while (mouse_b || key(_ESC));
  }
}


//-----------------------------------------------------------------------------
//      Eyedropper (pick a color from screen) for any toolbar
//-----------------------------------------------------------------------------

void eyedropper(void) {
  byte *_toolbar, *_saved_buffer;
  int _bar[16];
  int _toolbar_width;
  int col = color;
  byte num[8];

  if ((_toolbar = (byte *)malloc(vga_width * 19 * big2)) != NULL) {
    memcpy(_toolbar, toolbar, vga_width * 19 * big2);
    memcpy(_bar, bar, 16 * 4);
    _toolbar_width = toolbar_width;

    bar[0] = 101 + zoom;
    bar[1] = 1;
    bar[2] = 1;
    bar[3] = 1;
    bar[4] = 0;
    draw_bar(0);
    put_bar_inv(10, 2, 166);
    need_zoom = 1;

    do {
      draw_help(1295);
      read_mouse();
      select_zoom();
      test_mouse();

      if ((mouse_b & 1) && mouse_graf >= 10) {
        col = *(map + coord_x + coord_y * map_width);
        color = col;
        mouse_b = 2;
        remove_texture();
      }

      if (mouse_graf >= 10) {
        col = *(map + coord_x + coord_y * map_width);
        _saved_buffer = screen_buffer;
        screen_buffer = toolbar;
        wbox(toolbar, vga_width / big2, 19, col, 64, 2, 15, 15);
        wbox(toolbar, vga_width / big2, 19, c2, 80, 2, 31, 15);
        text_color = c4;
        num[2] = col % 10 + 48;
        num[1] = (col / 10) % 10 + 48;
        num[0] = (col / 100) % 10 + 48;
        div_strcpy((char *)&num[3], sizeof(num) - 3, "Dec");
        writetxt(109, 3, 2, num);
        num[1] = (col % 16 > 9) ? col % 16 + 55 : col % 16 + 48;
        num[0] = (col / 16 > 9) ? col / 16 + 55 : col / 16 + 48;
        div_strcpy((char *)&num[2], sizeof(num) - 2, "Hex");
        writetxt(109, 10, 2, num);
        screen_buffer = _saved_buffer;
      } else {
        _saved_buffer = screen_buffer;
        screen_buffer = toolbar;
        wbox(toolbar, vga_width / big2, 19, c1, 64, 2, 15, 15);
        wbox(toolbar, vga_width / big2, 19, c2, 80, 2, 31, 15);
        screen_buffer = _saved_buffer;
      }

      blit_edit();
    } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
             !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

    if (key(_ESC) ||
        (mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))) {
      put_bar(2, 10, 45);
      flush_bars(0);
      put(mouse_x, mouse_y, mouse_graf);
      blit_screen(screen_buffer);
    }

    memcpy(toolbar, _toolbar, vga_width * 19 * big2);
    memcpy(bar, _bar, 16 * 4);
    toolbar_width = _toolbar_width;
    free(_toolbar);
  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }

  zoom_background = 0;
  need_zoom = 1;
}

//-----------------------------------------------------------------------------
//      Check if the cursor is over an editable color in the gradient
//      Returns the edited color index (from the current gradient) in the parameter
//-----------------------------------------------------------------------------

int editable(int *n) {
  int r = 4;

  switch (gradients[gradient].num_colors) {
  case 8:
    r = 16;
    break;
  case 16:
    r = 8;
    break;
  case 32:
    r = 4;
    break;
  }
  *n = (mouse_x - toolbar_x - COLOR_RULER_X) / r + 1;

  if (!gradients[gradient].fixed) {
    if (mouse_y >= toolbar_y + 11 && mouse_y <= toolbar_y + 18 &&
        mouse_x >= toolbar_x + COLOR_RULER_X && mouse_x < toolbar_x + COLOR_RULER_X + 128) {
      switch (gradients[gradient].type) {
      case 0:
        if (*n == 1)
          r = 1;
        else
          r = 0;
        break;
      case 1:
        r = 1;
        break;
      case 2:
        if (*n % 2 == 0)
          r = 1;
        else
          r = 0;
        break;
      case 4:
        if (*n % 4 == 0)
          r = 1;
        else
          r = 0;
        break;
      case 8:
        if (*n % 8 == 0)
          r = 1;
        else
          r = 0;
        break;
      }
      return (r);
    } else
      return (0);
  } else
    return (0);
}

//-----------------------------------------------------------------------------
//      Check if the cursor is over an editable color in the gradient
//      Returns the edited color index (from the current gradient) in the parameter
//      - Version for the gradient in the select_color window -
//-----------------------------------------------------------------------------

int editable_selection(int *n, int x, int y) {
  int r = 4;

  switch (gradients[gradient].num_colors) {
  case 8:
    r = 16;
    break;
  case 16:
    r = 8;
    break;
  case 32:
    r = 4;
    break;
  }
  *n = (mouse_x - x) / r + 1;

  if (!gradients[gradient].fixed) {
    if (mouse_y >= y + 9 && mouse_y < y + 16 && mouse_x >= x && mouse_x < x + 128) {
      switch (gradients[gradient].type) {
      case 0:
        if (*n == 1)
          r = 1;
        else
          r = 0;
        break;
      case 1:
        r = 1;
        break;
      case 2:
        if (*n % 2 == 0)
          r = 1;
        else
          r = 0;
        break;
      case 4:
        if (*n % 4 == 0)
          r = 1;
        else
          r = 0;
        break;
      case 8:
        if (*n % 8 == 0)
          r = 1;
        else
          r = 0;
        break;
      }
      return (r);
    } else
      return (0);
  } else
    return (0);
}

//-----------------------------------------------------------------------------
//      Draw the color gradient within the editing toolbar
//-----------------------------------------------------------------------------

void draw_ruler(void) {
  int x, y;
  int n, med, a;
  byte *p, *ptr;
  int s0 = 4, s1 = 0;


  if (highlight_background) {
    dac[0] = (dac[0] + 32) % 64;
    dac[1] = (dac[1] + 32) % 64;
    dac[2] = (dac[2] + 32) % 64;
  }
  calculate_gradient(gradient);
  if (highlight_background) {
    dac[0] = (dac[0] + 32) % 64;
    dac[1] = (dac[1] + 32) % 64;
    dac[2] = (dac[2] + 32) % 64;
  }

  p = screen_buffer;
  screen_buffer = toolbar;

  switch (gradients[gradient].num_colors) {
  case 8:
    s0 = 16;
    s1 = 5;
    break;
  case 16:
    s0 = 8;
    s1 = 1;
    break;
  case 32:
    s0 = 4;
    s1 = -1;
    break;
  }

  for (a = COLOR_RULER_X; a < COLOR_RULER_X + 128; a += s0) {
    n = gradients[gradient].colors[(a - COLOR_RULER_X) / s0 + 1];
    wbox(toolbar, vga_width / big2, 19, n, a, 2, s0 - 1, 15);
    wbox(toolbar, vga_width / big2, 19, c0, a + s0 - 1, 2, 1, 15);
    if (n == color) {
      wbox(toolbar, vga_width / big2, 19, c4, a, 2, s0 - 1, 1);
      wbox(toolbar, vga_width / big2, 19, c0, a, 3, s0 - 1, 1);
    }
    if (!gradients[gradient].fixed) {
      if (gradients[gradient].type) {
        if (((a - COLOR_RULER_X) / s0 + 1) % gradients[gradient].type == 0) {
          wbox(toolbar, vga_width / big2, 19, c0, a, 10, s0 - 1, 1);
          wbox(toolbar, vga_width / big2, 19, c2, a, 11, s0 - 1, 6);
          wput(toolbar, vga_width / big2, 19, a + s1, 11, 32);
          wbox(toolbar, vga_width / big2, 19, c0, a - 1, 11, 1, 6);
          wbox(toolbar, vga_width / big2, 19, c0, a + s0 - 1, 11, 1, 6);
        }
      } else if (a == COLOR_RULER_X) {
        wbox(toolbar, vga_width / big2, 19, c0, a, 10, s0 - 1, 1);
        wbox(toolbar, vga_width / big2, 19, c2, a, 11, s0 - 1, 6);
        wput(toolbar, vga_width / big2, 19, a + s1, 11, 32);
        wbox(toolbar, vga_width / big2, 19, c0, a - 1, 11, 1, 6);
        wbox(toolbar, vga_width / big2, 19, c0, a + s0 - 1, 11, 1, 6);
      }
    }
  }

  wbox(toolbar, vga_width / big2, 19, color, 80, 2, 7, 8); // Color box, triggers select_color()

  if (texture_color == NULL || draw_mode == 0) {
    n = (memptrsize)c1 * 3;
    med = dac[n] + dac[n + 1] + dac[n + 2];
    n = color * 3;
    if (dac[n] + dac[n + 1] + dac[n + 2] > med)
      n = color;
    else
      n = c1;
    wrectangle(toolbar, vga_width / big2, 19, n, 80, 2, 7, 8);
  } else {
    ptr = toolbar + 2 * big2 * vga_width + 80 * big2;
    if (big) {
      for (y = 0; y < 16; y++)
        for (x = 0; x < 14; x++) {
          ptr[x + y * vga_width] = get_color(x, y);
        }
    } else {
      for (y = 0; y < 8; y++)
        for (x = 0; x < 7; x++) {
          ptr[x + y * vga_width] = get_color(x, y);
        }
    }
  }

  // Texture color selection

  wbox(toolbar, vga_width / big2, 19, c2, 80, 2 + 9, 7, 6);

  wbox(toolbar, vga_width / big2, 19, c3, 80 + 1, 2 + 10, 1, 4); // 'U'
  wbox(toolbar, vga_width / big2, 19, c3, 80 + 5, 2 + 10, 1, 4);
  wbox(toolbar, vga_width / big2, 19, c3, 80 + 1, 2 + 13, 5, 1);

  // Color 0 rectangle

  wrectangle(toolbar, vga_width / big2, 19, c1, 88, 2, 7, 8);
  wbox(toolbar, vga_width / big2, 19, 0, 88 + 1, 2 + 1, 5, 6);

  // Brush selection

  wbox(toolbar, vga_width / big2, 19, c2, 88, 2 + 9, 7, 6);

  wbox(toolbar, vga_width / big2, 19, c3, 88 + 1, 2 + 10, 5, 1); // 'T'
  wbox(toolbar, vga_width / big2, 19, c3, 88 + 3, 2 + 10, 1, 4);

  screen_buffer = p;
}

//-----------------------------------------------------------------------------
//      Draw the color gradient within the color selection window
//-----------------------------------------------------------------------------

void draw_ruler_selection(byte *p, int c, int d, int x, int y) {
  int n, s0 = 4, s1 = 0, a;

  switch (gradients[gradient].num_colors) {
  case 8:
    s0 = 16;
    s1 = 5;
    break;
  case 16:
    s0 = 8;
    s1 = 1;
    break;
  case 32:
    s0 = 4;
    s1 = -1;
    break;
  }

  for (a = x; a < x + 128; a += s0) {
    n = gradients[gradient].colors[(a - x) / s0 + 1];
    wbox(p, c, d, n, a, y, s0 - 1, 15);
    wbox(p, c, d, c0, a + s0 - 1, y, 1, 15);
    if (n == color) {
      wbox(p, c, d, c4, a, y, s0 - 1, 1);
      wbox(p, c, d, c0, a, y + 1, s0 - 1, 1);
    }
    if (!gradients[gradient].fixed) {
      if (gradients[gradient].type) {
        if (((a - x) / s0 + 1) % gradients[gradient].type == 0) {
          wbox(p, c, d, c0, a, y + 8, s0 - 1, 1);
          wbox(p, c, d, c2, a, y + 9, s0 - 1, 6);
          wput(p, c, d, a + s1, y + 9, 32);
          wbox(p, c, d, c0, a - 1, y + 9, 1, 6);
          wbox(p, c, d, c0, a + s0 - 1, y + 9, 1, 6);
        }
      } else if (a == x) {
        wbox(p, c, d, c0, a, y + 8, s0 - 1, 1);
        wbox(p, c, d, c2, a, y + 9, s0 - 1, 6);
        wput(p, c, d, a + s1, y + 9, 32);
        wbox(p, c, d, c0, a - 1, y + 9, 1, 6);
        wbox(p, c, d, c0, a + s0 - 1, y + 9, 1, 6);
      }
    }
  }

  wbox(p, c, d, c2, x + 128, y, 47, 7);
  switch (gradients[gradient].num_colors) {
  case 8:
    wwrite(p, c, d, x + 128 + 23, y, 1, texts[310], c3);
    break;
  case 16:
    wwrite(p, c, d, x + 128 + 23, y, 1, texts[311], c3);
    break;
  case 32:
    wwrite(p, c, d, x + 128 + 23, y, 1, texts[312], c3);
    break;
  }

  wbox(p, c, d, c2, x + 128 + 48, y, 47, 7);
  switch (gradients[gradient].fixed) {
  case 0:
    wwrite(p, c, d, x + 128 + 71, y, 1, texts[313], c3);
    break;
  case 1:
    wwrite(p, c, d, x + 128 + 71, y, 1, texts[314], c3);
    break;
  }

  wbox(p, c, d, c2, x + 128, y + 8, 95, 7);
  switch (gradients[gradient].type) {
  case 0:
    wwrite(p, c, d, x + 128 + 48, y + 8, 1, texts[315], c3);
    break;
  case 1:
    wwrite(p, c, d, x + 128 + 48, y + 8, 1, texts[316], c3);
    break;
  case 2:
    wwrite(p, c, d, x + 128 + 48, y + 8, 1, texts[317], c3);
    break;
  case 4:
    wwrite(p, c, d, x + 128 + 48, y + 8, 1, texts[318], c3);
    break;
  case 8:
    wwrite(p, c, d, x + 128 + 48, y + 8, 1, texts[319], c3);
    break;
  }
}

//-----------------------------------------------------------------------------
//      Help within the drawing editor
//-----------------------------------------------------------------------------

extern struct tprg *old_prg;
void help_paint(memptrsize);

void draw_help(int n) {
  int m = 0;

  if (!key(_L_SHIFT) && !key(_R_SHIFT) && key(_F1)) {
    // If there is a help window, remove it

    if ((m = determine_help()) > 0) {
      v.foreground = 1;
      v.state = 1;
      v.type = 100;
      move(0, m);
      call(v.close_handler);
      if (v.click_handler == help2 && old_prg != NULL) {
        for (m = 1; m < MAX_WINDOWS; m++) {
          if (window[m].click_handler == program2) {
            if (window[m].prg == old_prg && window[m].foreground < 2) {
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
              v.state = 0;
              break;
            }
          }
        }
        old_prg = NULL;
      }
      divdelete(0);
      zoom_background = 0;
      v.type = 0;
      v.foreground = 0;
      v.state = 0;
      highlight_background = 0;
      zoom_map();
      need_zoom = 0;
    }

    // Create a help dialog...

    help_paint(n);
  }
}

//-----------------------------------------------------------------------------
//    Preview texture with the X key
//-----------------------------------------------------------------------------

void zoom_map2(void) {
  int w, h;
  int n, m, c, x, y;
  byte *p, *q = screen_buffer;

  p = map + zoom_y * map_width + zoom_x;

  if ((map_width << zoom) < vga_width) { // Copy of this chunk in select_zoom()
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

  if (zoom_win_x || zoom_win_y)
    blit_partial(zoom_win_x, zoom_win_y, zoom_win_width, zoom_win_height);
  else
    full_redraw = 1;

  x = zoom_x;
  y = zoom_y;

  switch (zoom) {
  case 0:
    m = h;
    do {
      n = w;
      do {
        *q++ = *(
            ghost + *p * 256 +
            texture_color[(x + texture_x) % texture_w + ((y + texture_y) % texture_h) * texture_w]);
        x++;
        p++;
      } while (--n);
      y++;
      x -= w;
      p += map_width - w;
      q += vga_width - w;
    } while (--m);
    break;

  case 1:
    m = h;
    do {
      n = w;
      do {
        c = *(
            ghost + *p * 256 +
            texture_color[(x + texture_x) % texture_w + ((y + texture_y) % texture_h) * texture_w]);
        c += c * 256;
        *(word *)q = c;
        *(word *)(q + vga_width) = c;
        q += 2;
        x++;
        p++;
      } while (--n);
      y++;
      x -= w;
      p += map_width - w;
      q += vga_width * 2 - w * 2;
    } while (--m);
    break;

  case 2:
    m = h;
    do {
      n = w;
      do {
        c = *(
            ghost + *p * 256 +
            texture_color[(x + texture_x) % texture_w + ((y + texture_y) % texture_h) * texture_w]);
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
        x++;
        p++;
      } while (--n);
      y++;
      x -= w;
      p += map_width - w;
      q += vga_width * 4 - w * 4;
    } while (--m);
    break;

  case 3:
    m = h;
    do {
      n = w;
      do {
        c = *(
            ghost + *p * 256 +
            texture_color[(x + texture_x) % texture_w + ((y + texture_y) % texture_h) * texture_w]);
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
        x++;
        p++;
      } while (--n);
      y++;
      x -= w;
      p += map_width - w;
      q += vga_width * 8 - w * 8;
    } while (--m);
    break;
  }

  // Fill remainder strips for partial texels (same logic as zoom_map()).
  // zoom_map2() is the texture-preview variant (X key), so apply the
  // ghost+texture blend to the extra edge pixels.
  {
    int texel = 1 << zoom;
    int drawn_w = w << zoom;
    int drawn_h = h << zoom;
    int rem_x = (zoom_win_x == 0 && (map_width << zoom) >= vga_width) ? vga_width - drawn_w : 0;
    int rem_y = (zoom_win_y == 0 && (map_height << zoom) >= vga_height) ? vga_height - drawn_h : 0;

    // Right edge
    if (rem_x > 0) {
      byte *rq = screen_buffer + drawn_w;
      if (zoom_x + w < map_width) {
        byte *rp = map + zoom_y * map_width + zoom_x + w;
        int src_x = zoom_x + w;
        int src_y = zoom_y;
        int row;
        for (row = 0; row < h; row++) {
          int ty;
          byte pc = *(ghost + *rp * 256 +
                      texture_color[(src_x + texture_x) % texture_w +
                                    ((src_y + texture_y) % texture_h) * texture_w]);
          for (ty = 0; ty < texel; ty++) {
            memset(rq, pc, rem_x);
            rq += vga_width;
          }
          rp += map_width;
          src_y++;
        }
      } else {
        int row;
        for (row = 0; row < drawn_h; row++) {
          memset(rq, c0, rem_x);
          rq += vga_width;
        }
      }
    }

    // Bottom edge
    if (rem_y > 0) {
      byte *rq = screen_buffer + drawn_h * vga_width;
      if (zoom_y + h < map_height) {
        byte *rp = map + (zoom_y + h) * map_width + zoom_x;
        int src_y = zoom_y + h;
        int col;
        for (col = 0; col < w; col++) {
          byte *rq2 = rq + (col << zoom);
          int src_x = zoom_x + col;
          byte pc = *(ghost + *rp * 256 +
                      texture_color[(src_x + texture_x) % texture_w +
                                    ((src_y + texture_y) % texture_h) * texture_w]);
          int ry;
          for (ry = 0; ry < rem_y; ry++) {
            memset(rq2, pc, texel);
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

      // Corner
      if (rem_x > 0) {
        byte *cq = screen_buffer + drawn_h * vga_width + drawn_w;
        byte cc = c0;
        int ry;
        if (zoom_x + w < map_width && zoom_y + h < map_height) {
          int src_x = zoom_x + w;
          int src_y = zoom_y + h;
          byte raw = *(map + src_y * map_width + src_x);
          cc = *(ghost + raw * 256 +
                 texture_color[(src_x + texture_x) % texture_w +
                               ((src_y + texture_y) % texture_h) * texture_w]);
        }
        for (ry = 0; ry < rem_y; ry++) {
          memset(cq, cc, rem_x);
          cq += vga_width;
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
//      Blit from virtual screen copy to the real screen (in edit mode)
//-----------------------------------------------------------------------------

int b_pulsada;

void blit_edit(void) {
  int mx, my;

  if (key(_X) && texture_color != NULL) {
    zoom_region(0, 0, 32768, 32768);
    zoom_map2();
    blit_mouse();
    do {
      mx = mouse_x;
      my = mouse_y;
      read_mouse();
      if (mx != mouse_x || my != mouse_y) {
        texture_x += mx - mouse_x;
        texture_y += my - mouse_y;
        if (texture_x < 0)
          texture_x += texture_w;
        if (texture_y < 0)
          texture_y += texture_h;
        zoom_region(0, 0, 32768, 32768);
        zoom_map2();
        blit_mouse();
      }
    } while (key(_X));
    zoom_region(0, 0, 32768, 32768);
  }

  move_zoom();
  move_bar();

  if (need_zoom == 1)
    zoom_map();
  else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
    draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
  need_zoom_width = 0;
  need_zoom_height = 0;
  need_zoom = 0;

  bar_coords();

  if (mouse_in(toolbar_x + 48, toolbar_y + 2, toolbar_x + toolbar_width - 2, toolbar_y + 17)) {
    selected_icon = (mouse_x - toolbar_x - 48) / 16;
    if (bar[selected_icon] > 1) {
      wresalta_box(toolbar, vga_width / big2, 19, 48 + selected_icon * 16, 2, 15, 15);
    } else
      selected_icon = -1;
  } else
    selected_icon = -1;

  blit_mouse();

  if (selected_icon >= 0)
    put_bar(48 + selected_icon * 16, 2, bar[selected_icon]);

  select_mode();

  if (scan_code == _TAB) {
    if (!tab_cycling) {
      tab_cycling = 1;
      if (key(_L_SHIFT) || key(_R_SHIFT))
        change_map(0);
      else
        change_map(1);
    }
  } else
    tab_cycling = 0;

  if (key(_B) && !(text_bar_active && sel_status == 1)) {
    if (!b_pulsada) {
      b_pulsada = 1;
      highlight_background ^= 1;
      dac[0] = (dac[0] + 32) % 64;
      dac[1] = (dac[1] + 32) % 64;
      dac[2] = (dac[2] + 32) % 64;
      set_dac(dac);
    }
  } else
    b_pulsada = 0;
}

//-----------------------------------------------------------------------------
//      Select another toolbar, from the edit screen
//-----------------------------------------------------------------------------

int icons_mode[] = {14, 2, 177, 160, 161, 178, 162, 179, 163, 164, 180, 165, 131, 100, 191, 190};

void select_mode(void) {
  int r;

  if (!key(_L_SHIFT) && !key(_R_SHIFT)) {
    if (key(_F2)) {
      draw_mode = 101;
      return;
    }
    if (key(_F3)) {
      draw_mode = 102;
      return;
    }
    if (key(_F4)) {
      draw_mode = 103;
      return;
    }
    if (key(_F5)) {
      draw_mode = 104;
      return;
    }
    if (key(_F6)) {
      draw_mode = 105;
      return;
    }
    if (key(_F7)) {
      draw_mode = 106;
      return;
    }
    if (key(_F8)) {
      draw_mode = 107;
      return;
    }
    if (key(_F9)) {
      draw_mode = 108;
      return;
    }
    if (key(_F10)) {
      draw_mode = 109;
      return;
    }
    if (key(_F11)) {
      draw_mode = 110;
      return;
    }
    if (key(_F12)) {
      draw_mode = 111;
      return;
    }
  } else {
    if (key(_F1)) {
      draw_mode = 112;
      return;
    }
    if (key(_F2)) {
      draw_mode = 113;
      return;
    }
    if (key(_F3)) {
      draw_mode = 100;
      return;
    }
  }

  if (mode_selection == -1)
    icons_mode[12] = 131;
  else if (mode_selection < 4)
    icons_mode[12] = 131 + mode_selection;
  else
    icons_mode[12] = mode_selection + 167 - 4;
  r = select_icon(toolbar_x + 10, icons_mode);
  if (r >= 0)
    draw_mode = r + 100;
}

//-----------------------------------------------------------------------------
//      FX selection, from an editing toolbar
//-----------------------------------------------------------------------------

void select_fx(int n, int *effect) {
  int icon_x = toolbar_x + 48 + n * 16;
  int a, b, c, d;
  int ix, iy, wait = 0;
  byte *p;

  if (mouse_in(icon_x, toolbar_y + 2, icon_x + 15, toolbar_y + 17) && (mouse_b & 1)) {
    c = 88 * big2;
    d = 27 * big2;
    if ((p = (byte *)malloc(c * d)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    flush_bars(0);

    a = toolbar_x + (icon_x - toolbar_x) * big2 - (c - 16 * big2) / 2;
    if (toolbar_y > vga_height / 2 - 9 * big2)
      b = toolbar_y - d - 1;
    else
      b = toolbar_y + 19 * big2 + 1;
    if (a < toolbar_x)
      a = toolbar_x;
    if (a + c > toolbar_x + toolbar_width * big2)
      a = toolbar_x + toolbar_width * big2 - c;
    adjust_box(&a, &b, &c, &d);
    c /= big2;
    d /= big2;
    do {
      read_mouse();
    } while (mouse_b & 1);

    do {
      read_mouse();
      test_mouse_box(a, b, c, d);
      wait = 0;

      wrectangle(p, c, d, c2, 0, 0, c, d);
      wbox(p, c, d, c0, 1, 1, c - 2, d - 2);
      wgra(p, c, d, c_b_low, 2, 2, c - 12, 7);
      wput(p, c, d, c - 9, 2, 35);

      wput(p, c, d, 2, 10, 33);
      wbox(p, c, d, c1, 12, 19, 64, 4);
      wbox(p, c, d, c4, 12, 19, *effect * 4, 4);

      if (mouse_in(a + 2, b + 18, a + 9, b + 25)) {
        mouse_graf = 4;
        if (mouse_b & 1) {
          wput(p, c, d, 2, 18, -47);
          wait = 1;
          if (*effect > 1)
            (*effect)--;
        }
      } else if (mouse_in(a + 79, b + 18, a + 86, b + 25)) {
        mouse_graf = 6;
        if (mouse_b & 1) {
          wput(p, c, d, 79, 18, -48);
          wait = 1;
          if (*effect < 16)
            (*effect)++;
        }
      } else if (mouse_in(a + 2, b + 10, a + 86, b + 25)) {
        if (mouse_x < a + 12)
          ix = 12;
        else if (mouse_x > a + 72)
          ix = 72;
        else
          ix = mouse_x - a;
        iy = (ix - 8) / 4;
        ix = 11 + iy * 4;
        wbox(p, c, d, c4, ix, 18, 1, 1);
        wbox(p, c, d, c4, ix, 23, 1, 1);
        if (mouse_b & 1) {
          *effect = iy;
          wait = 2;
        }
      } else if (mouse_in(a + 2, b + 2, a + c - 10, b + 9)) {
        ix = mouse_shift_x - a;
        iy = mouse_shift_y - b;
        wrectangle(p, c, d, c4, 0, 0, c, d);
        while (mouse_b & 1) {
          save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
          put(mouse_shift_x, mouse_shift_y, mouse_graf);
          blit_screen(screen_buffer);
          save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
          draw_edit_background(a, b, c * big2, d * big2);
          flush_bars(0);
          read_mouse();
          a = mouse_shift_x - ix;
          b = mouse_shift_y - iy;
          blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
          blit_partial(a, b, c * big2, d * big2);
        }
        wrectangle(p, c, d, c2, 0, 0, c, d);
      }

      if (wait) {
        wbox(p, c, d, c1, 12, 19, 64, 4);
        wbox(p, c, d, c4, 12, 19, *effect * 4, 4);
      }

      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);

      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
      put(mouse_shift_x, mouse_shift_y, mouse_graf);
      blit_screen(screen_buffer);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);

      if (wait == 1) {
        wput(p, c, d, 2, 18, -37);
        wput(p, c, d, 79, 18, -38);
        do {
          read_mouse();
        } while (mouse_b & 1);
      }

    } while ((!mouse_b || wait) && !key(_ESC));

    if (!mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width - 1, toolbar_y + 18) ||
        !(mouse_b & 1) || mouse_in(icon_x, toolbar_y + 2, icon_x + 15, toolbar_y + 17)) {
      wput(p, c, d, c - 9, 2, -45);
      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
      put(mouse_shift_x, mouse_shift_y, mouse_graf);
      blit_screen(screen_buffer);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }

    draw_edit_background(a, b, c * big2, d * big2);
    flush_bars(0);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
    put(mouse_shift_x, mouse_shift_y, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
    free(p);
  }
}

//-----------------------------------------------------------------------------
//      Fill algorithm selection, from an editing toolbar
//-----------------------------------------------------------------------------

void select_fill(int n) {
  int r;
  r = select_icon(toolbar_x + 48 + n * 16, fill_icons);
  if (r >= 0) {
    mode_fill = r;
    bar[n] = fill_icons[2 + mode_fill];
    put_bar(48 + n * 16, 2, bar[n]);
  }
}

//-----------------------------------------------------------------------------
//      Block selection algorithm, from an editing toolbar
//-----------------------------------------------------------------------------

int icons_box[] = {6, 1, 135, 136, 137, 138, 169, 170};

void select_box(int n) {
  int r;
  r = select_icon(toolbar_x + 48 + n * 16, icons_box);
  if (r >= 0) {
    mode_selection = r;
    block_bar(0);
  }
}

//-----------------------------------------------------------------------------
//      Select from any icon list, in an editing toolbar
//-----------------------------------------------------------------------------

int select_icon(int icon_x, int *icons) {
  int a, b, c, d;
  int num, col, fil;
  int r = -1, ix, iy;
  byte *p;

  if (mouse_in(icon_x, toolbar_y + 2, icon_x + 15, toolbar_y + 17) && (mouse_b & 1)) {
    num = *icons++;
    col = *icons++;
    fil = (num + col - 1) / col;
    c = (col * 16 + 3) * big2;
    d = (fil * 16 + 11) * big2;
    if ((p = (byte *)malloc(c * d)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return (-1);
    }

    flush_bars(0);

    a = toolbar_x + (icon_x - toolbar_x) * big2 - (col - 1) * 8 * big2 - 2 * big2;
    if (toolbar_y > vga_height / 2 - 9 * big2)
      b = toolbar_y - d - 1;
    else
      b = toolbar_y + 19 * big2 + 1;
    if (a < toolbar_x)
      a = toolbar_x;
    if (a + c > toolbar_x + toolbar_width * big2)
      a = toolbar_x + toolbar_width * big2 - c;
    adjust_box(&a, &b, &c, &d);
    c /= big2;
    d /= big2;
    do {
      read_mouse();
    } while (mouse_b & 1);

    do {
      read_mouse();
      test_mouse_box(a, b, c, d);

      wrectangle(p, c, d, c2, 0, 0, c, d);
      wbox(p, c, d, c0, 1, 1, c - 2, d - 2);
      wgra(p, c, d, c_b_low, 2, 2, c - 12, 7);
      wput(p, c, d, c - 9, 2, 35);

      r = 0;
      while (r < num) {
        wput(p, c, d, 2 + (r % col) * 16, 10 + (r / col) * 16, *icons++);
        r++;
      }
      icons -= num;

      if (mouse_in(a + 2, b + 10, a + c - 2, b + d - 2)) {
        r = ((mouse_y - b - 10) / 16) * col + (mouse_x - a - 2) / 16;
        ix = 2 + (r % col) * 16;
        iy = 10 + (r / col) * 16;
        wresalta_box(p, c, d, ix, iy, 15, 15);
      } else if (mouse_in(a + 2, b + 2, a + c - 10, b + 9)) {
        ix = mouse_shift_x - a;
        iy = mouse_shift_y - b;
        wrectangle(p, c, d, c4, 0, 0, c, d);
        while (mouse_b & 1) {
          save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
          put(mouse_shift_x, mouse_shift_y, mouse_graf);
          blit_screen(screen_buffer);
          save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
          draw_edit_background(a, b, c * big2, d * big2);
          flush_bars(0);
          read_mouse();
          a = mouse_shift_x - ix;
          b = mouse_shift_y - iy;
          blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
          blit_partial(a, b, c * big2, d * big2);
        }
        wrectangle(p, c, d, c2, 0, 0, c, d);
      }

      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);

      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
      put(mouse_shift_x, mouse_shift_y, mouse_graf);
      blit_screen(screen_buffer);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);

    } while (!mouse_b && !key(_ESC));

    if ((mouse_b & 1) && mouse_in(a + 2, b + 10, a + c - 2, b + d - 2)) {
      r = ((mouse_y - b - 10) / 16) * col + (mouse_x - a - 2) / 16;
    } else
      r = -1;

    if (!mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width - 1, toolbar_y + 18) ||
        !(mouse_b & 1) || mouse_in(icon_x, toolbar_y + 2, icon_x + 15, toolbar_y + 17)) {
      wput(p, c, d, c - 9, 2, -45);
      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
      put(mouse_shift_x, mouse_shift_y, mouse_graf);
      blit_screen(screen_buffer);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }

    draw_edit_background(a, b, c * big2, d * big2);
    flush_bars(0);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
    put(mouse_shift_x, mouse_shift_y, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);

    free(p);
  }
  return (r);
}

//-----------------------------------------------------------------------------
//      Blit the screen (and toolbar), positioning the mouse if mouse_graf>=10
//-----------------------------------------------------------------------------

void blit_mouse() {
  blit_mouse_a();
  blit_mouse_b();
}

void blit_mouse_a(void) {
  int moux, mouy;
  moux = zoom_win_x + ((mouse_x - zoom_win_x) & (-(1 << zoom)));
  mouy = zoom_win_y + ((mouse_y - zoom_win_y) & (-(1 << zoom)));
  if (mouse_graf < 10) {
    flush_bars(0);
  } else {
    save_mouse_bg(mouse_background, moux, mouy, mouse_graf, 0);
    put_bw(moux, mouy, mouse_graf);
    flush_bars(0);
  }
}

void blit_mouse_b(void) {
  int moux, mouy;
  moux = zoom_win_x + ((mouse_x - zoom_win_x) & (-(1 << zoom)));
  mouy = zoom_win_y + ((mouse_y - zoom_win_y) & (-(1 << zoom)));
  if (mouse_graf < 10) {
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
    put(mouse_shift_x, mouse_shift_y, mouse_graf);
  }
  blit_screen(screen_buffer);
  if (mouse_graf < 10)
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
  else
    save_mouse_bg(mouse_background, moux, mouy, mouse_graf, 1);
}

//-----------------------------------------------------------------------------
//      Mask selection window
//-----------------------------------------------------------------------------

void select_mask(int n) {
  int x, y, col, oldcol, ix, iy;
  int a, b, c, d, i;
  int done;
  byte *p;

  if ((key(_M) && hotkey) ||
      (mouse_in(toolbar_x + 48 + n * 16, toolbar_y + 2, toolbar_x + 55 + n * 16, toolbar_y + 17) &&
       (mouse_b & 2))) {
    c = (128 + 3) * big2;
    d = (128 + 11 + 8) * big2;
    a = toolbar_x;
    if (toolbar_y > vga_height / 2 - 8)
      b = toolbar_y - d - 1;
    else
      b = toolbar_y + 19 * big2 + 1;
    if ((p = (byte *)malloc(c * d)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    adjust_box(&a, &b, &c, &d);
    c /= big2;
    d /= big2;
    do {
      read_mouse();
    } while (mouse_b || key(_M));

    done = 0;
    col = color;

    do {
      read_mouse();
      test_mouse_box2(a, b, c, d);

      oldcol = col;
      col = color;

      // Move the window

      if (mouse_in(a + 2, b + 2, a + c - 10, b + 9)) {
        ix = mouse_shift_x - a;
        iy = mouse_shift_y - b;
        if (mouse_b & 1) {
          wrectangle(p, c, d, c4, 0, 0, c, d);
          while (mouse_b & 1) {
            read_mouse();
            a = mouse_shift_x - ix;
            b = mouse_shift_y - iy;
            blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
            blit_partial(a, b, c * big2, d * big2);
            save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
            put(mouse_shift_x, mouse_shift_y, mouse_graf);
            blit_screen(screen_buffer);
            save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
            draw_edit_background(a, b, c * big2, d * big2);
            flush_bars(0);
          }
          wrectangle(p, c, d, c2, 0, 0, c, d);
          test_mouse_box(a, b, c, d);
        }
      }

      // Determine the color we are over

      if (mouse_in(a + 2, b + 10, a + 128 + 1, b + 128 + 9)) {
        col = (mouse_x - a - 2) / 8 + ((mouse_y - b - 10) / 8) * 16;
        if (mouse_b & 1) {
          if (col != oldcol || !(prev_mouse_buttons & 1))
            mask[col] ^= 1;
          draw_ruler();
        }
      }

      // Invert / Clear

      if ((mouse_b & 1) && mouse_in(a + 2, b + d - 9, a + c - 2, b + d - 2)) {
        if (mouse_x < a + c / 2) {
          for (i = 0; i < 256; i++)
            mask[i] ^= 1;
        } else {
          memset(mask, 0, 256);
        }
        do {
          read_mouse();
        } while (mouse_b);
      }

      // Pick a color from the screen

      if ((mouse_b & 1) && !mouse_in(a, b, a + c - 1, b + d - 1)) {
        if (mouse_graf >= 10) {
          mask[*(map + coord_y * map_width + coord_x)] = 1;
          draw_ruler();
        } else
          done = 1;
      }

      blit_mouse_a();
      paint_mask_window(p, c, d);
      x = 1 + (col % 16) * 8;
      y = 9 + (col / 16) * 8;
      wrectangle(p, c, d, c4, x, y, 9, 9);
      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);
      blit_mouse_b();

      x = 1 + (col % 16) * 8;
      y = 9 + (col / 16) * 8;
      wrectangle(p, c, d, c0, x, y, 9, 9);
      blit_partial(a + x * big2, b + y * big2, 9 * big2, 9 * big2);

    } while (!(mouse_b & 2) && !key(_ESC) && !done && !key(_M) && !exit_requested &&
             !((mouse_b & 1) && mouse_in(a + c - 9, b + 2, a + c - 2, b + 9)));

    if (!mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width - 1, toolbar_y + 18) ||
        !(mouse_b & 1) || key(_M) ||
        mouse_in(toolbar_x + 48 + n * 16, toolbar_y + 2, toolbar_x + 57 + n * 16, toolbar_y + 17)) {
      blit_mouse_a();
      wput(p, c, d, c - 9, 2, -45);
      x = 1 + (col % 16) * 8;
      y = 9 + (col / 16) * 8;
      wrectangle(p, c, d, c4, x, y, 9, 9);
      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);
      blit_mouse_b();
      do {
        read_mouse();
      } while (mouse_b || key(_ESC) || key(_M));
    }

    draw_edit_background(a, b, c * big2, d * big2);
    test_mouse();
    blit_mouse();
    free(p);
  }

  mask_on = 0;
  for (i = 0; i < 256; i++)
    if (mask[i])
      mask_on = 1;
}

//-----------------------------------------------------------------------------
//      Draw the mask selection window
//-----------------------------------------------------------------------------

void paint_mask_window(byte *p, int c, int d) {
  int n, x, y;

  wrectangle(p, c, d, c2, 0, 0, c, d); // Window
  wbox(p, c, d, c0, 1, 1, c - 2, d - 2);
  wgra(p, c, d, c_b_low, 2, 2, c - 12, 7);
  wput(p, c, d, c - 9, 2, 35);

  // Box and palette

  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(p, c, d, y * 16 + x, 2 + x * 8, 10 + y * 8, 7, 7);

  for (n = 0; n < 256; n++)
    if (mask[n]) {
      x = (memptrsize)c2 * 3;
      x = dac[x] + dac[x + 1] + dac[x + 2];
      y = (memptrsize)n * 3;
      y = dac[y] + dac[y + 1] + dac[y + 2];
      if (y >= x)
        wbox(p, c, d, c0, 4 + (n % 16) * 8, 12 + (n / 16) * 8, 3, 3);
      else
        wbox(p, c, d, c4, 4 + (n % 16) * 8, 12 + (n / 16) * 8, 3, 3);
    }

  wbox(p, c, d, c2, 2, d - 9, c - 4, 7);
  wbox(p, c, d, c0, c / 2, d - 9, 1, 7);

  wwrite(p, c, d, c / 4, d - 9, 1, texts[306], c3);
  wwrite(p, c, d, c * 3 / 4, d - 9, 1, texts[307], c3);
}

//-----------------------------------------------------------------------------
//      Palette color selection, from an editing toolbar
//-----------------------------------------------------------------------------

#define max_texturas 1000
#define w_textura    (3 + 1) // 000 - 999

#define BRUSH 4
#define MAPBR 8

extern int texture_type;
extern int tex_count;
extern int bg_count;
extern int fpg_thumb_pos;
extern byte brush_fpg_path[256];
extern char m3d_fpgcodesbr[max_texturas * w_textura];
extern struct t_listboxbr texture_list_br;
extern struct _thumb_tex {
  int w, h;                    // Width and height of the thumbnail
  int real_width, real_height; // Width and height of the texture
  char *ptr;                   // ==NULL if the thumbnail has not started loading
  int status;                  // 0-Not a valid texture, 1-Loaded
  int FilePos;
  int Code;
  int is_square;
} thumb_tex[max_texturas];

extern FILE *file_paint_fpg;

extern struct t_listboxbr copia_br;

int m_maximo = 0;
struct t_listboxbr thumbmap_list_br = {3 - 2, 11 - 2, NULL, 0, 4, 4, 32, 32};
int browser_type = 0;

struct _thumb_map {            // Brush map thumbnails
  int w, h;                    // Width and height of the thumbnail
  int real_width, real_height; // Width and height of the texture
  char *ptr;                   // ==NULL if the thumbnail has not started loading
  int status;                  // 0-Not a valid texture, 1-Loaded
  int FilePos;
  int Code;
  int is_square;
} thumb_map[MAX_WINDOWS];

void mapper_browse_fpg0(void);

void free_paint_thumbs(void) {
  int n;

  for (n = 0; n < max_texturas; n++) {
    if (thumb_tex[n].ptr != NULL) {
      free(thumb_tex[n].ptr);
      thumb_tex[n].ptr = NULL;
    }
  }
  for (n = 0; n < MAX_WINDOWS; n++) {
    if (thumb_map[n].ptr != NULL) {
      free(thumb_map[n].ptr);
      thumb_map[n].ptr = NULL;
    }
  }
}

int create_mapbr_thumbs(struct t_listboxbr *l) {
  int n, con;
  int m;
  int man, mal;
  byte *temp, *temp2;
  float coefredy, coefredx, a, b;
  int x, y;

  fpg_thumb_pos = 0;

  n = m_maximo = 0;
  for (con = 0; con < MAX_WINDOWS; con++) {
    if (window[con].type == 100) {
      thumb_map[n].w = (int)window[con].mapa->map_width;
      thumb_map[n].h = (int)window[con].mapa->map_height;
      thumb_map[n].ptr = (char *)window[con].mapa->map;
      thumb_map[n].Code = con;
      n++;
    }
  }
  l->total_items = m_maximo = n;

  for (con = 0; con < l->total_items; con++) {
    man = thumb_map[con].real_width = thumb_map[con].w;
    mal = thumb_map[con].real_height = thumb_map[con].h;
    temp = (byte *)thumb_map[con].ptr;

    if (man <= 32 * big2 && mal <= 32 * big2) // The graphic is kept as-is
    {
      if ((thumb_map[con].ptr = (char *)malloc(man * mal)) == NULL) {
        thumb_map[con].ptr = NULL;
        thumb_map[con].status = 0;
        continue;
      }
      memcpy(thumb_map[con].ptr, temp, man * mal);
    } else // Create the thumbnail
    {
      coefredx = man / ((float)32 * 2 * (float)big2);
      coefredy = mal / ((float)32 * 2 * (float)big2);
      if (coefredx < coefredy)
        coefredx = coefredy;
      else
        coefredy = coefredx;
      thumb_map[con].w = (float)man / coefredx + 0.5;
      thumb_map[con].h = (float)mal / coefredy + 0.5;
      thumb_map[con].w &= -2;
      thumb_map[con].h &= -2;
      if (thumb_map[con].w < 2)
        thumb_map[con].w = 2;
      if (thumb_map[con].h < 2)
        thumb_map[con].h = 2;

      if (coefredx * (float)(thumb_map[con].w - 1) >= (float)man)
        coefredx = (float)(man - 1) / (float)(thumb_map[con].w - 1);
      if (coefredy * (float)(thumb_map[con].h - 1) >= (float)mal)
        coefredy = (float)(mal - 1) / (float)(thumb_map[con].h - 1);

      if ((temp2 = (byte *)malloc(thumb_map[con].w * thumb_map[con].h)) == NULL) {
        thumb_map[con].ptr = NULL;
        thumb_map[con].status = 0;
        continue;
      }

      memset(temp2, 0, thumb_map[con].w * thumb_map[con].h);

      a = (float)0.0;
      for (y = 0; y < thumb_map[con].h; y++) {
        b = (float)0.0;
        for (x = 0; x < thumb_map[con].w; x++) {
          temp2[y * thumb_map[con].w + x] = temp[((memptrsize)a) * man + (memptrsize)b];
          b += coefredx;
        }
        a += coefredy;
      }

      if ((thumb_map[con].ptr = (char *)malloc((thumb_map[con].w * thumb_map[con].h) / 4)) ==
          NULL) {
        free(temp2);
        thumb_map[con].ptr = NULL;
        thumb_map[con].status = 0;
        break;
      }

      for (y = 0; y < thumb_map[con].h; y += 2) {
        for (x = 0; x < thumb_map[con].w; x += 2) {
          n = *(ghost + temp2[x + y * thumb_map[con].w] * 256 +
                temp2[x + 1 + y * thumb_map[con].w]);
          m = *(ghost + temp2[x + (y + 1) * thumb_map[con].w] * 256 +
                temp2[x + 1 + (y + 1) * thumb_map[con].w]);
          thumb_map[con].ptr[x / 2 + (y / 2) * (thumb_map[con].w / 2)] = *(ghost + n * 256 + m);
        }
      }
      thumb_map[con].w /= 2;
      thumb_map[con].h /= 2;
      free(temp2);
    }
    thumb_map[con].status = 1;
  }

  return (1);
}

void select_color(int n) { // Icon number as parameter
  int x, y, col, oldcol, ix, iy;
  int a, b, c, d;
  int done, needs_redraw;
  int _gradient, button;
  byte reg[33];
  byte pal[768], xchg[256];
  byte *p;
  int num_tex, tex_cod;
  byte *temp;
  int man, mal;

  if ((texture_type & 4) &&
      ((key(_T) && hotkey) || (mouse_in(toolbar_x + 56 + n * 16, toolbar_y + 11,
                                        toolbar_x + 62 + n * 16, toolbar_y + 17) &&
                               (mouse_b & 1)))) {
    browser_type = BRUSH;
    show_dialog(mapper_browse_fpg0);

    num_tex = texture_list_br.first_visible + texture_list_br.zone - 10; // Position in browser
    tex_cod = atoi(m3d_fpgcodesbr + num_tex * w_textura);                // Code at that position

    if (thumb_tex[num_tex].Code == 0 || !v_finished)
      return;

    man = thumb_tex[num_tex].real_width;
    mal = thumb_tex[num_tex].real_height;

    if ((temp = (byte *)malloc(man * mal)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
    } else {
      fseek(file_paint_fpg, thumb_tex[num_tex].FilePos, SEEK_SET);

      if (fread(temp, 1, man * mal, file_paint_fpg) != man * mal) {
        free(temp);
        v_text = (char *)texts[44];
        show_dialog(err0);
      } else {
        if (brush != &default_brush[0])
          free(brush);
        brush_type = 0;
        brush_index = num_tex;
        brush = temp;
        temp = NULL;
        brush_w = man;
        brush_h = mal;

        fseek(file_paint_fpg, 8, SEEK_SET);
        memcpy(pal, dac, 768);
        fread(pal, 1, 768, file_paint_fpg);

        // Brush in {brush, man x mal, pal}

        for (n = 0; n < 256; n++)
          xchg[n] = (pal[n * 3] + pal[n * 3 + 1] + pal[n * 3 + 2] + 3) / 12;
        brush_type = 0;
        for (n = 0; n < man * mal; n++)
          if ((brush[n] = xchg[brush[n]]))
            if (brush[n] != 16)
              brush_type = 1;
      }
    }
  }

  if ((texture_type & 8) &&
      ((key(_U) && hotkey) || (mouse_in(toolbar_x + 56 - 8 + n * 16, toolbar_y + 11,
                                        toolbar_x + 62 - 8 + n * 16, toolbar_y + 17) &&
                               (mouse_b & 1)))) {
    browser_type = MAPBR;
    show_dialog(mapper_browse_fpg0);
    if (v_finished) {
      num_tex = thumb_map[texture_list_br.first_visible + texture_list_br.zone - 10].Code;
      texture_color = window[num_tex].mapa->map;
      texture_w = window[num_tex].mapa->map_width;
      texture_h = window[num_tex].mapa->map_height;
      texture_x = 0;
      texture_y = 0;
    }
    memcpy(&texture_list_br, &copia_br, sizeof(texture_list_br));
    draw_ruler();
  }

  if ((key(_C) && hotkey) ||
      (mouse_in(toolbar_x + 48 + n * 16, toolbar_y + 2, toolbar_x + 55 + n * 16, toolbar_y + 10) &&
       (mouse_b & 1))) {
    c = (128 + 3 + 32 + 64 + 8) * big2;
    d = (128 + 3 + 18 + 8) * big2;
    a = toolbar_x;
    if (toolbar_y > vga_height / 2 - 8)
      b = toolbar_y - d - 1;
    else
      b = toolbar_y + 19 * big2 + 1;
    if ((p = (byte *)malloc(c * d)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    adjust_box(&a, &b, &c, &d);
    c /= big2;
    d /= big2;

    done = 0;
    needs_redraw = 1;
    _gradient = -1;
    button = -1;
    col = color;

    do {
      read_mouse();
    } while (mouse_b || key(_C));

    do {
      read_mouse();
      test_mouse_box(a, b, c, d);

      oldcol = col;
      col = color;

      // Move the window

      if (mouse_in(a + 2, b + 2, a + c - 10, b + 9)) {
        ix = mouse_shift_x - a;
        iy = mouse_shift_y - b;
        if (mouse_b & 1) {
          wrectangle(p, c, d, c4, 0, 0, c, d);
          while (mouse_b & 1) {
            read_mouse();
            a = mouse_shift_x - ix;
            b = mouse_shift_y - iy;
            blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
            blit_partial(a, b, c * big2, d * big2);
            save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
            put(mouse_shift_x, mouse_shift_y, mouse_graf);
            blit_screen(screen_buffer);
            save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
            draw_edit_background(a, b, c * big2, d * big2);
            flush_bars(0);
          }
          wrectangle(p, c, d, c2, 0, 0, c, d);
          needs_redraw = 1;
          test_mouse_box(a, b, c, d);
        }
      }

      if (mouse_in(a + 10, b + 10, a + 128 + 9, b + 128 + 9)) {
        col = (mouse_x - a - 10) / 8 + ((mouse_y - b - 10) / 8) * 16;
        if (mouse_b & 1) {
          color = col;
          remove_texture();
          draw_ruler();
          needs_redraw = 1;
        } else if (col != oldcol && needs_redraw != 1)
          needs_redraw = 2;
      } else if (col != oldcol && needs_redraw != 1)
        needs_redraw = 2;

      if ((mouse_b & 1) && mouse_in(a + 170, b + 10, a + 233, b + 128 + 9)) {
        gradient = (mouse_y - b - 10) / 8;
        draw_ruler();
        needs_redraw = 1;
      }

      if ((mouse_b & 1) && mouse_in(a + 138, b + 140, a + 138 + 95, b + 140 + 15)) {
        if (mouse_y >= b + 132 + 16)
          x = 2;
        else if (mouse_x < a + 138 + 48)
          x = 0;
        else
          x = 1;

        if (gradient != _gradient || x != button) {
          _gradient = gradient;
          button = x;
          switch (gradients[gradient].num_colors) {
          case 32:
            for (x = 0; x <= 32; x++)
              reg[x] = gradients[gradient].colors[x];
            break;
          case 16:
            for (x = 0; x <= 32; x += 2)
              reg[x] = gradients[gradient].colors[x / 2];
            for (x = 0; x < 32; x += 2)
              reg[x + 1] = average_color(reg[x], reg[x + 2]);
            break;
          case 8:
            for (x = 0; x <= 32; x += 4)
              reg[x] = gradients[gradient].colors[x / 4];
            for (x = 0; x < 32; x += 4) {
              reg[x + 2] = average_color(reg[x], reg[x + 4]);
              reg[x + 1] = average_color(reg[x], reg[x + 2]);
              reg[x + 3] = average_color(reg[x + 2], reg[x + 4]);
            }
            break;
          }
          reg[1] = gradients[gradient].colors[1];
        }

        if (button == 2) {
          switch (gradients[gradient].type) {
          case 0:
            gradients[gradient].type = 1;
            break;
          case 1:
            gradients[gradient].type = 2;
            break;
          case 2:
            gradients[gradient].type = 4;
            break;
          case 4:
            gradients[gradient].type = 8;
            break;
          case 8:
            gradients[gradient].type = 0;
            break;
          }
          gradients[gradient].fixed = 0;
        } else if (button == 0) {
          switch (gradients[gradient].num_colors) {
          case 8:
            gradients[gradient].num_colors = 16;
            break;
          case 16:
            gradients[gradient].num_colors = 32;
            break;
          case 32:
            gradients[gradient].num_colors = 8;
            break;
          }
          gradients[gradient].fixed = 0;
        } else
          gradients[gradient].fixed ^= 1;

        if (button == 0 || button == 2)
          switch (gradients[gradient].num_colors) {
          case 8:
            for (x = 2; x <= 8; x++)
              gradients[gradient].colors[x] = reg[x * 4];
            gradients[gradient].colors[1] = reg[1];
            break;
          case 16:
            for (x = 2; x <= 16; x++)
              gradients[gradient].colors[x] = reg[x * 2];
            gradients[gradient].colors[1] = reg[1];
            break;
          case 32:
            for (x = 2; x <= 32; x++)
              gradients[gradient].colors[x] = reg[x];
            gradients[gradient].colors[1] = reg[1];
            break;
          }

        draw_ruler();
        needs_redraw = 1;
        do
          read_mouse();
        while (mouse_b & 1);
      } else if (!mouse_in(a + 138, b + 140, a + 138 + 95, b + 140 + 15))
        button = -1;

      if (editable_selection(&x, a + 10, b + 128 + 12))
        mouse_graf = 2;
      else if (!mouse_in(a, b, a + c - 1, b + d - 1) &&
               mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width - 1, toolbar_y + 18)) {
        if (mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))
          mouse_graf = 5;
        else if (mouse_in(toolbar_x, toolbar_y, toolbar_x + 9, toolbar_y + 9) || editable(&x))
          mouse_graf = 2;
      }

      // Blit: 0-Nothing (mouse), 1-Color info, 2-Entire window.

      if (needs_redraw == 1) {
        needs_redraw = 0;
        flush_bars(0);
        paint_color_window(p, c, d);
        paint_window_colors2(p, c, d, col);
        x = 9 + (col % 16) * 8;
        y = 9 + (col / 16) * 8;
        wrectangle(p, c, d, c4, x, y, 9, 9);
        blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
        blit_partial(a, b, c * big2, d * big2);
      } else if (needs_redraw == 2) {
        needs_redraw = 0;
        paint_window_colors2(p, c, d, col);
        x = 9 + (col % 16) * 8;
        y = 9 + (col / 16) * 8;
        wrectangle(p, c, d, c4, x, y, 9, 9);
        blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
        blit_partial(a + x * big2, b + y * big2, 9 * big2, 9 * big2);
        blit_partial(a + 138 * big2, b + 10 * big2, 31 * big2, 128 * big2);
      }

      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
      put(mouse_shift_x, mouse_shift_y, mouse_graf);
      blit_screen(screen_buffer);
      save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);

      x = 9 + (col % 16) * 8;
      y = 9 + (col / 16) * 8;
      wrectangle(p, c, d, c0, x, y, 9, 9);
      blit_partial(a + x * big2, b + y * big2, 9 * big2, 9 * big2);

      if ((mouse_b & 1) && mouse_in(a + 10, b + 132 + 8, a + 137, b + 147 + 8)) {
        if (editable_selection(&x, a + 10, b + 132 + 8))
          gradients[gradient].colors[x] = color;
        else {
          color = gradients[gradient].colors[x];
          remove_texture();
        }
        draw_ruler();
        needs_redraw = 1;
      }

      if ((mouse_b & 1) && !mouse_in(a, b, a + c - 1, b + d - 1)) {
        if (mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width - 1, toolbar_y + 18)) {
          if (mouse_in(toolbar_x + COLOR_RULER_X, toolbar_y, toolbar_x + COLOR_RULER_X + 127,
                       toolbar_y + 18)) {
            if (editable(&x))
              gradients[gradient].colors[x] = color;
            else {
              color = gradients[gradient].colors[x];
              remove_texture();
            }
            draw_ruler();
            needs_redraw = 1;
          } else if (mouse_in(toolbar_x + COLOR_RULER_X - 8, toolbar_y,
                              toolbar_x + COLOR_RULER_X - 1, toolbar_y + 18)) {
            if (texture_color != NULL) {
              remove_texture();
              if (color != 0) {
                color_c0 = color;
                color = 0;
              }
            } else {
              if (color == 0)
                color = color_c0;
              else {
                color_c0 = color;
                color = 0;
              }
            }
            draw_ruler();
            do {
              read_mouse();
            } while (mouse_b);
            needs_redraw = 1;
          } else
            done = 1;
        } else
          done = 1;
      }

    } while (!(mouse_b & 2) && !key(_ESC) && !done && !key(_C) && !exit_requested &&
             !((mouse_b & 1) && mouse_in(a + c - 9, b + 2, a + c - 2, b + 9)));

    if (!mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width - 1, toolbar_y + 18) ||
        !(mouse_b & 1) || key(_C) ||
        mouse_in(toolbar_x + 48 + n * 16, toolbar_y + 2, toolbar_x + 57 + n * 16, toolbar_y + 17)) {
      blit_mouse_a();
      wput(p, c, d, c - 9, 2, -45);
      x = 9 + (col % 16) * 8;
      y = 9 + (col / 16) * 8;
      wrectangle(p, c, d, c4, x, y, 9, 9);
      blit_region(screen_buffer, vga_width, vga_height, p, a, b, c * big2, d * big2, 0);
      blit_partial(a, b, c * big2, d * big2);
      blit_mouse_b();
      do {
        read_mouse();
      } while (mouse_b || key(_ESC) || key(_C));
    }

    draw_edit_background(a, b, c * big2, d * big2);
    test_mouse();
    blit_mouse();
    free(p);
  } else if ((ascii == '0' && hotkey) || (mouse_in(toolbar_x + 56 + n * 16, toolbar_y + 2,
                                                   toolbar_x + 62 + n * 16, toolbar_y + 10) &&
                                          (mouse_b & 1))) {
    if (texture_color != NULL) {
      remove_texture();
      if (color != 0) {
        color_c0 = color;
        color = 0;
      }
    } else {
      if (color == 0)
        color = color_c0;
      else {
        color_c0 = color;
        color = 0;
      }
    }
    draw_ruler();
    do {
      read_mouse();
    } while (mouse_b);
  } else if (scan_code == _S && hotkey) {
    if (key(_L_SHIFT) || key(_R_SHIFT))
      gradient = (gradient + 1) % 16;
    else
      color_up();
    draw_ruler();
  } else if (scan_code == _W && hotkey) {
    if (key(_L_SHIFT) || key(_R_SHIFT)) {
      if (!gradient--)
        gradient = 15;
    } else
      color_down();
    draw_ruler();
  } else if ((shift_status & 4) && hotkey) {
    if ((key(_RIGHT) && scan_code) || scan_code == _P)
      color_up();
    if ((key(_LEFT) && scan_code) || scan_code == _O)
      color_down();
    if (key(_DOWN) || scan_code == _A) {
      gradient = (gradient + 1) % 16;
      do {
        poll_keyboard();
      } while (key(_DOWN));
    }
    if (key(_UP) || scan_code == _Q) {
      if (!gradient--) {
        gradient = 15;
      }
      do {
        poll_keyboard();
      } while (key(_UP));
    }
    draw_ruler();
  } else
    select_mask(n);
}

//-----------------------------------------------------------------------------
//  Animation (cycle maps with TAB and shift-TAB)
//-----------------------------------------------------------------------------

void change_map(int forward) {
  int n, old = 0;

  if (back) {
    undo_back();
    if (need_zoom == 1)
      zoom_map();
    else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
      draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
    need_zoom_width = 0;
    need_zoom_height = 0;
    need_zoom = 0;
    back = 0;
  }

  v.mapa->zoom = zoom;
  v.mapa->zoom_x = zoom_x;
  v.mapa->zoom_y = zoom_y;
  v.mapa->zoom_cx = zoom_cx;
  v.mapa->zoom_cy = zoom_cy;
  v.type = 100;
  call(v.paint_handler);
  copy(-1, 0);

  if (forward) {
    for (n = 1; n < MAX_WINDOWS; n++) {
      if (window[n].type == 100 && window[n].mapa->map_width == map_width &&
          window[n].mapa->map_height == map_height && window[n].foreground != 2) {
        copy(old, n);
        old = n;
      }
    }
  } else {
    for (n = MAX_WINDOWS - 1; n > 0; n--) {
      if (window[n].type == 100 && window[n].mapa->map_width == map_width &&
          window[n].mapa->map_height == map_height && window[n].foreground != 2) {
        copy(old, n);
        old = n;
      }
    }
  }

  copy(old, -1);

  for (n = 0; n < MAX_WINDOWS; n++) {
    if (thumb_map[n].ptr != NULL) {
      free(thumb_map[n].ptr);
      thumb_map[n].ptr = NULL;
    }
  }
  create_mapbr_thumbs(&thumbmap_list_br);

  map = v.mapa->map;
  current_map_code = v.mapa->code;
  v.type = 0;
  zoom_background = 0;
  need_zoom = 1;
}

//-----------------------------------------------------------------------------
//      Move the currently selected color one shade up or down
//-----------------------------------------------------------------------------

void color_up(void) {
  int n, c = 0;
  for (n = 1; n <= gradients[gradient].num_colors; n++)
    if (gradients[gradient].colors[n] == color)
      c = n;
  if (c) {
    if (c < gradients[gradient].num_colors)
      color = gradients[gradient].colors[c + 1];
  } else
    color = gradients[gradient].colors[1];
  remove_texture();
}
void color_down(void) {
  int n, c = 0;
  for (n = gradients[gradient].num_colors; n >= 1; n--)
    if (gradients[gradient].colors[n] == color)
      c = n;
  if (c) {
    if (c > 1)
      color = gradients[gradient].colors[c - 1];
  } else
    color = gradients[gradient].colors[gradients[gradient].num_colors];
  remove_texture();
}

//-----------------------------------------------------------------------------
//      Draw the color selection window
//-----------------------------------------------------------------------------

void paint_color_window(byte *p, int c, int d) {
  int x, y;
  char num[8];

  wrectangle(p, c, d, c2, 0, 0, c, d); // Window
  wbox(p, c, d, c0, 1, 1, c - 2, d - 2);
  wgra(p, c, d, c_b_low, 2, 2, c - 12, 7);
  wput(p, c, d, c - 9, 2, 35);

  // Box and palette

  wbox(p, c, d, c2, 2, 10, 7, 145);
  wbox(p, c, d, c2, 9, 138, 224, 1);
  wbox(p, c, d, c2, 138, 10, 31, 128);
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(p, c, d, y * 16 + x, 10 + x * 8, 10 + y * 8, 7, 7);

  x = (memptrsize)c2 * 3;
  x = dac[x] + dac[x + 1] + dac[x + 2];
  y = (memptrsize)color * 3;
  y = dac[y] + dac[y + 1] + dac[y + 2];

  if (y >= x)
    wbox(p, c, d, c0, 12 + (color % 16) * 8, 12 + (color / 16) * 8, 3, 3);
  else
    wbox(p, c, d, c4, 12 + (color % 16) * 8, 12 + (color / 16) * 8, 3, 3);

  wbox(p, c, d, c2, 138, 10, 31, 63);

  // Gradients

  for (y = 0; y < 16; y++) {
    wbox(p, c, d, c1, 170, 10 + y * 8, 4, 7);
    switch (gradients[y].num_colors) {
    case 32:
      for (x = 0; x < 32; x++) {
        wbox(p, c, d, gradients[y].colors[x + 1], 175 + x, 10 + y * 8, 1, 7);
      }
      break;
    case 16:
      for (x = 0; x < 16; x++) {
        wbox(p, c, d, gradients[y].colors[x + 1], 175 + x * 2, 10 + y * 8, 2, 7);
      }
      break;
    case 8:
      for (x = 0; x < 8; x++) {
        wbox(p, c, d, gradients[y].colors[x + 1], 175 + x * 4, 10 + y * 8, 4, 7);
      }
      break;
    }
    wbox(p, c, d, c2, 208, 10 + y * 8, 11, 7);
    x = gradients[y].num_colors;
    num[2] = 0;
    num[1] = x % 10 + 48;
    num[0] = (x / 10) % 10 + 48;
    wwrite(p, c, d, 209, 10 + y * 8, 0, (byte *)num, c3);
    wbox(p, c, d, c2, 220, 10 + y * 8, 6, 7);
    x = gradients[y].type;
    num[1] = 0;
    num[0] = x + 48;
    wwrite(p, c, d, 221, 10 + y * 8, 0, (byte *)num, c3);
    wbox(p, c, d, c2, 227, 10 + y * 8, 6, 7);
    if (!gradients[y].fixed)
      wwrite(p, c, d, 228, 10 + y * 8, 0, (byte *)"E", c3);
    else
      wwrite(p, c, d, 228, 10 + y * 8, 0, (byte *)"F", c3);
  }
  wput(p, c, d, 170, 10 + gradient * 8, 36);

  wrectangle(p, c, d, c0, 139, 65 + 8, 9, 65);
  wrectangle(p, c, d, c0, 149, 65 + 8, 9, 65);
  wrectangle(p, c, d, c0, 159, 65 + 8, 9, 65);

  draw_ruler_selection(p, c, d, 10, 128 + 12);
}

//-----------------------------------------------------------------------------
//	Color window info panel for the selected color
//-----------------------------------------------------------------------------

void paint_window_colors2(byte *p, int c, int d, int col) {
  byte num[8];
  int x;

  wrectangle(p, c, d, c0, 139, 26, 29, 23);
  wbox(p, c, d, col, 140, 19 + 8, 27, 21);

  wbox(p, c, d, c2, 138, 10, 31, 16);
  num[2] = col % 10 + 48;
  num[1] = (col / 10) % 10 + 48;
  num[0] = (col / 100) % 10 + 48;
  div_strcpy((char *)&num[3], sizeof(num) - 3, "Dec");
  wwrite(p, c, d, 167, 11, 2, num, c4);
  num[1] = (col % 16 > 9) ? col % 16 + 55 : col % 16 + 48;
  num[0] = (col / 16 > 9) ? col / 16 + 55 : col / 16 + 48;
  div_strcpy((char *)&num[2], sizeof(num) - 2, "Hex");
  wwrite(p, c, d, 167, 19, 2, num, c4);

  wbox(p, c, d, c1, 140, 66 + 8, 7, 63);
  if ((x = dac[col * 3]))
    wbox(p, c, d, c_r, 140, 129 - x + 8, 7, x);
  wbox(p, c, d, c1, 150, 66 + 8, 7, 63);
  if ((x = dac[col * 3 + 1]))
    wbox(p, c, d, c_g, 150, 129 - x + 8, 7, x);
  wbox(p, c, d, c1, 160, 66 + 8, 7, 63);
  if ((x = dac[col * 3 + 2]))
    wbox(p, c, d, c_b, 160, 129 - x + 8, 7, x);

  wbox(p, c, d, c2, 138, 41 + 8, 31, 23);
  num[1] = dac[col * 3] % 10 + 48;
  num[0] = dac[col * 3] / 10 + 48;
  div_strcpy((char *)&num[2], sizeof(num) - 2, " Red");
  wwrite(p, c, d, 139, 42 + 8, 0, num, c3);
  num[1] = dac[col * 3 + 1] % 10 + 48;
  num[0] = dac[col * 3 + 1] / 10 + 48;
  div_strcpy((char *)&num[2], sizeof(num) - 2, " Grn.");
  wwrite(p, c, d, 139, 50 + 8, 0, num, c3);
  num[1] = dac[col * 3 + 2] % 10 + 48;
  num[0] = dac[col * 3 + 2] / 10 + 48;
  div_strcpy((char *)&num[2], sizeof(num) - 2, " Blue");
  wwrite(p, c, d, 139, 58 + 8, 0, num, c3);
}

//-----------------------------------------------------------------------------
//      Adjust a box (a,b,c,d) to fit entirely within the screen
//-----------------------------------------------------------------------------

void adjust_box(int *a, int *b, int *c, int *d) {
  if (*a < 0)
    *a = 0;
  else if (*a + *c > vga_width)
    *a = vga_width - *c;
  if (*b < 0)
    *b = 0;
  else if (*b + *d > vga_height)
    *b = vga_height - *d;
}

//-----------------------------------------------------------------------------
//      Detects the move_bar icon click
//-----------------------------------------------------------------------------

void move_bar(void) {
  int barx, bary;

  if ((mouse_b & 1) && mouse_in(toolbar_x, toolbar_y, toolbar_x + 9, toolbar_y + 9)) {
    wrectangle(toolbar, vga_width / big2, vga_height, c4, 0, 0, toolbar_width, 19);
    barx = toolbar_x - mouse_shift_x;
    bary = toolbar_y - mouse_shift_y;
    do {
      read_mouse();
      draw_edit_background(toolbar_x - 4, toolbar_y - big2, toolbar_width * big2 + 4, 20 * big2);
      toolbar_x = mouse_shift_x + barx;
      toolbar_y = mouse_shift_y + bary;
      bar_coords();
      flush_bars(0);
      put(mouse_x, mouse_y, 2);
      blit_screen(screen_buffer);
    } while (mouse_b & 1);
    zoom_map();

    wrectangle(toolbar, vga_width / big2, vga_height, c2, 0, 0, toolbar_width, 19);
    draw_edit_background(toolbar_x - 4, toolbar_y - big2, toolbar_width * big2 + 4, 20 * big2);
    flush_bars(0);
  }
}

//-----------------------------------------------------------------------------
//      Detect the zoom icon as the second toolbar icon
//-----------------------------------------------------------------------------

int icons_zoom[] = {4, 1, 101, 102, 103, 104};

void select_zoom(void) {
  int r = 0, z = 0;

  if (mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width * big2 - 1,
               toolbar_y + 19 * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = toolbar_x + (mouse_shift_x - toolbar_x) / 2;
      mouse_y = toolbar_y + (mouse_shift_y - toolbar_y) / 2;
    } else if (!big)
      mouse_shift = 1;
  }

  if (mouse_b & 8 && zoom > 0) {
    zoom--;
    r = 0;
    if (!mouse_shift) {
      zoom_cx = coord_x;
      zoom_cy = coord_y;
    }
  } else if (mouse_b & 4 && zoom < 3) {
    zoom++;
    r = 0;
    if (!mouse_shift) {
      zoom_cx = coord_x;
      zoom_cy = coord_y;
    }
  } else if ((z = (key(_Z) && hotkey))) {
    zoom = (zoom + 1) % 4;
    r = 0;
    if (!mouse_shift) {
      zoom_cx = coord_x;
      zoom_cy = coord_y;
    }
  } else {
    r = select_icon(toolbar_x + 48, icons_zoom);
    if (r >= 0)
      zoom = r;
  }

  if (mouse_x >= zoom_win_x + zoom_win_width || mouse_x < zoom_win_x ||
      mouse_y >= zoom_win_y + zoom_win_height || mouse_y < zoom_win_y)
    z = 0;

  if (r >= 0) {
    bar[0] = 101 + zoom;
    put_bar(48, 2, bar[0]);
    zoom_background = 0;

    if ((map_width << zoom) < vga_width) { // Copy of this chunk in zoom_map
      zoom_win_width = map_width << zoom;
      zoom_win_x = (vga_width - zoom_win_width) / 2;
    } else {
      zoom_win_x = 0;
      zoom_win_width = (vga_width >> zoom) << zoom;
    }

    if ((map_height << zoom) < vga_height) {
      zoom_win_height = map_height << zoom;
      zoom_win_y = (vga_height - zoom_win_height) / 2;
    } else {
      zoom_win_y = 0;
      zoom_win_height = (vga_height >> zoom) << zoom;
    }

    // Use ceiling division for visible columns/rows to account for partial
    // texels when window size isn't a multiple of the texel size (1<<zoom).
    {
      int texel = 1 << zoom;
      int vis_w = (vga_width + texel - 1) >> zoom;
      int vis_h = (vga_height + texel - 1) >> zoom;

      zoom_x = zoom_cx - vis_w / 2;
      if (zoom_x <= 0)
        zoom_x = 0;
      else if (zoom_x + vis_w > map_width) {
        zoom_x = map_width - vis_w;
        if (zoom_x < 0)
          zoom_x = 0;
      }

      zoom_y = zoom_cy - vis_h / 2;
      if (zoom_y < 0)
        zoom_y = 0;
      else if (zoom_y + vis_h > map_height) {
        zoom_y = map_height - vis_h;
        if (zoom_y < 0)
          zoom_y = 0;
      }
    }

    if (z && !mouse_shift) {
      mouse_x = zoom_win_x + (coord_x - zoom_x) * (1 << zoom);
      mouse_y = zoom_win_y + (coord_y - zoom_y) * (1 << zoom);
      set_mouse(mouse_x, mouse_y);
      mouse_shift_x = mouse_x;
      mouse_shift_y = mouse_y;
    }

    do {
      read_mouse();
    } while ((mouse_b & 1) || (key(_Z) && hotkey));

    need_zoom = 1;
  }

  if (mouse_in(toolbar_x, toolbar_y, toolbar_x + toolbar_width * big2 - 1,
               toolbar_y + 19 * big2 - 1)) {
    if (big && !mouse_shift) {
      mouse_shift = 1;
      mouse_x = toolbar_x + (mouse_shift_x - toolbar_x) / 2;
      mouse_y = toolbar_y + (mouse_shift_y - toolbar_y) / 2;
    }
  }

  zoom = zoom % 4;
}

//-----------------------------------------------------------------------------
//      Scroll the zoomed area when approaching its edge
//-----------------------------------------------------------------------------

void move_zoom(void) {
  int n, m = 0;

  if ((mouse_b & 1) && mouse_in(toolbar_x + 26, toolbar_y + 2, toolbar_x + 47, toolbar_y + 17)) {
    if (zoom_move == c3)
      zoom_move = c1;
    else
      zoom_move = c3;
    do {
      read_mouse();
    } while (mouse_b & 1);
  }

  if (zoom_move == c3) {
    // Use ceiling division for visible columns/rows: accounts for partial
    // texels when the window size isn't a multiple of the texel size (1<<zoom).
    int texel = 1 << zoom;
    int vis_w = (vga_width + texel - 1) >> zoom;
    int vis_h = (vga_height + texel - 1) >> zoom;

    n = zoom_x;
    // Check bitmap overflow using actual bitmap size, not truncated zoom_win_width.
    // On DOS, screen sizes were always multiples of 8 so these were equivalent;
    // with SDL2 free-resize they can differ, blocking scroll.
    if (zoom_win_x == 0 && (map_width << zoom) >= vga_width) {
      if (real_mouse_x < 0) {
        zoom_x -= (-real_mouse_x) >> zoom;
        if (zoom_x < 0)
          zoom_x = 0;
        if (n != zoom_x)
          m |= 1;
      } else if (real_mouse_x >= vga_width) {
        zoom_x += (real_mouse_x - vga_width + 1) >> zoom;
        if (zoom_x > map_width - vis_w)
          zoom_x = map_width - vis_w;
        if (zoom_x < 0)
          zoom_x = 0;
        if (n != zoom_x)
          m |= 1;
      }
    }
    n = zoom_y;
    if (zoom_win_y == 0 && (map_height << zoom) >= vga_height) {
      if (real_mouse_y < 0) {
        zoom_y -= (-real_mouse_y) >> zoom;
        if (zoom_y < 0)
          zoom_y = 0;
        if (n != zoom_y)
          m |= 2;
      } else if (real_mouse_y >= vga_height) {
        zoom_y += (real_mouse_y - vga_height + 1) >> zoom;
        if (zoom_y > map_height - vis_h)
          zoom_y = map_height - vis_h;
        if (zoom_y < 0)
          zoom_y = 0;
        if (n != zoom_y)
          m |= 2;
      }
    }
    if (m & 1)
      zoom_cx = zoom_x + (vga_width / 2) / (1 << zoom);
    if (m & 2)
      zoom_cy = zoom_y + (vga_height / 2) / (1 << zoom);
    if (m)
      need_zoom = 1;
  }
}

//-----------------------------------------------------------------------------
//      Draws the toolbar (with fixed sections: 'move','back','mode' and 'coords')
//-----------------------------------------------------------------------------

void draw_bar(int bar_width) {
  int n; // Number of icons

  draw_edit_background(toolbar_x - 4, toolbar_y, toolbar_width * big2 + 4, 19 * big2);

  n = 0;
  while (bar[n])
    n++;
  toolbar_width = 2 + 8 + 38 + n * 16 + 1 + bar_width;
  memset(toolbar, c0, vga_width * 19 * big2);
  wrectangle(toolbar, vga_width / big2, 19, c2, 0, 0, toolbar_width, 19);
  wgra(toolbar, vga_width / big2, 19, c_b_low, 2, 2, 7, 7);

  put_bar(2, 10, 35);
  n = 0;
  while (bar[n]) {
    if (bar[n] > 1)
      put_bar(48 + n * 16, 2, bar[n]);
    n++;
  }
  switch (draw_mode) {
  case 0:
    put_bar_inv(10, 2, 177);
    break;
  case 1:
    put_bar_inv(10, 2, 160);
    break;
  case 2:
    put_bar_inv(10, 2, 161);
    break;
  case 3:
    put_bar_inv(10, 2, 178);
    break;
  case 4:
    put_bar_inv(10, 2, 162);
    break;
  case 5:
    put_bar_inv(10, 2, 179);
    break;
  case 6:
    put_bar_inv(10, 2, 163);
    break;
  case 7:
    put_bar_inv(10, 2, 164);
    break;
  case 8:
    put_bar_inv(10, 2, 180);
    break;
  case 9:
    put_bar_inv(10, 2, 165);
    break;
  case 10:
    if (mode_selection < 4) {
      put_bar_inv(10, 2, 131 + mode_selection);
    } else {
      put_bar_inv(10, 2, mode_selection + 167 - 4);
    }
    break;
  case 11:
    put_bar_inv(10, 2, 100);
    break;
  case 12:
    put_bar_inv(10, 2, 191);
    break;
  case 13:
    put_bar_inv(10, 2, 190);
    break;
  }
}

//-----------------------------------------------------------------------------
//      Print coordinates on the second toolbar icon area
//-----------------------------------------------------------------------------

void bar_coords(void) {
  byte *p;
  byte num[5];

  wbox(toolbar, vga_width / big2, vga_height, c2, 26, 2, 21, 15);
  p = screen_buffer;
  screen_buffer = toolbar;
  text_color = zoom_move;
  num[4] = 0;

  if (coord_x >= 0 && coord_x < map_width) {
    num[3] = coord_x % 10 + 48;
    num[2] = (coord_x / 10) % 10 + 48;
    num[1] = (coord_x / 100) % 10 + 48;
    num[0] = (coord_x / 1000) % 10 + 48;
    writetxt(27, 3, 0, num);
  } else
    writetxt(27, 3, 0, (byte *)"000?");

  if (coord_y >= 0 && coord_y < map_height) {
    num[3] = coord_y % 10 + 48;
    num[2] = (coord_y / 10) % 10 + 48;
    num[1] = (coord_y / 100) % 10 + 48;
    num[0] = (coord_y / 1000) % 10 + 48;
    writetxt(27, 10, 0, num);
  } else
    writetxt(27, 10, 0, (byte *)"000?");

  screen_buffer = p;
}

//-----------------------------------------------------------------------------
//      Print width and height (lines, boxes, ...) on the toolbar
//-----------------------------------------------------------------------------

void analyze_bar(int w, int h) {
  byte *p;
  byte num[5];

  wbox(toolbar, vga_width / big2, vga_height, c2, toolbar_width - 23, 2, 21, 15);

  p = screen_buffer;
  screen_buffer = toolbar;
  text_color = c4;
  num[4] = 0;
  num[3] = w % 10 + 48;
  num[2] = (w / 10) % 10 + 48;
  num[1] = (w / 100) % 10 + 48;
  num[0] = (w / 1000) % 10 + 48;
  writetxt(toolbar_width - 22, 3, 0, num);
  num[3] = h % 10 + 48;
  num[2] = (h / 10) % 10 + 48;
  num[1] = (h / 100) % 10 + 48;
  num[0] = (h / 1000) % 10 + 48;
  writetxt(toolbar_width - 22, 10, 0, num);
  screen_buffer = p;
}

//-----------------------------------------------------------------------------
//      Flush the toolbar (based on toolbar_x/y and toolbar_width)
//-----------------------------------------------------------------------------

int new_bar(int w, int h) {
  int n;

  for (n = 0; n < 10; n++) {
    if (!toolbars[n].on)
      break;
  }
  if (n == 10)
    return (-1);

  if ((toolbars[n].ptr = (byte *)malloc(w * big2 * h * big2)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return (-1);
  }

  toolbars[n].on = 1;
  toolbars[n].w = w;
  toolbars[n].h = h;

  w *= big2;
  h *= big2;
  toolbars[n].x = toolbar_x;
  if (toolbar_y > vga_height / 2 - 9 * big2)
    toolbars[n].y = toolbar_y - h - 1;
  else
    toolbars[n].y = toolbar_y + 19 * big2 + 1;
  adjust_box(&toolbars[n].x, &toolbars[n].y, &w, &h);
  w /= big2;
  h /= big2;

  wrectangle(toolbars[n].ptr, w, h, c2, 0, 0, w, h);
  wbox(toolbars[n].ptr, w, h, c0, 1, 1, w - 2, h - 2);
  wgra(toolbars[n].ptr, w, h, c_b_low, 2, 2, w - 12, 7);
  wput(toolbars[n].ptr, w, h, w - 9, 2, 35);

  return (0);
}

void flush_bars(int darkened) {
  int n;

  // The main toolbar is treated specially, it is always active

  if (!darkened)
    flush_bar(toolbar, vga_width, toolbar_x, toolbar_y, toolbar_width, 19);
  else
    flush_bar_darkened(toolbar, vga_width, toolbar_x, toolbar_y, toolbar_width, 19);

  for (n = 0; n < 10; n++)
    if (toolbars[n].on) {
      if (!darkened) {
        flush_bar(toolbars[n].ptr, toolbars[n].w, toolbars[n].x, toolbars[n].y, toolbars[n].w,
                  toolbars[n].h);
      } else {
        flush_bar_darkened(toolbars[n].ptr, toolbars[n].w, toolbars[n].x, toolbars[n].y,
                           toolbars[n].w, toolbars[n].h);
      }
    }
}

void flush_bar(byte *p, int real_w, int x, int y, int w, int h) {
  byte *q;
  int skip_x, copy_w, remain_x;
  int skip_y, copy_h, remain_y;

  if (big) {
    w *= 2;
    h *= 2;
  }

  blit_partial(x, y, w, h);

  q = screen_buffer + y * vga_width + x;

  if (x < 0)
    skip_x = -x;
  else
    skip_x = 0;
  if (x + w > vga_width)
    remain_x = x + w - vga_width;
  else
    remain_x = 0;
  copy_w = w - skip_x - remain_x;

  remain_x += real_w - w;

  if (y < 0)
    skip_y = -y;
  else
    skip_y = 0;
  if (y + h > vga_height)
    remain_y = y + h - vga_height;
  else
    remain_y = 0;
  copy_h = h - skip_y - remain_y;

  p += vga_width * skip_y + skip_x;
  q += vga_width * skip_y + skip_x;
  remain_x += skip_x;
  w = copy_w;
  do {
    do {
      *q++ = *p++;
    } while (--w);
    q += vga_width - (w = copy_w);
    p += remain_x;
  } while (--copy_h);
}

void flush_bar_darkened(byte *p, int real_w, int x, int y, int w, int h) {
  byte *q;
  int skip_x, copy_w, remain_x;
  int skip_y, copy_h, remain_y;

  if (big) {
    w *= 2;
    h *= 2;
  }

  blit_partial(x, y, w, h);

  q = screen_buffer + y * vga_width + x;

  if (x < 0)
    skip_x = -x;
  else
    skip_x = 0;
  if (x + w > vga_width)
    remain_x = x + w - vga_width;
  else
    remain_x = 0;
  copy_w = w - skip_x - remain_x;

  remain_x += real_w - w;

  if (y < 0)
    skip_y = -y;
  else
    skip_y = 0;
  if (y + h > vga_height)
    remain_y = y + h - vga_height;
  else
    remain_y = 0;
  copy_h = h - skip_y - remain_y;

  p += vga_width * skip_y + skip_x;
  q += vga_width * skip_y + skip_x;
  remain_x += skip_x;
  w = copy_w;
  do {
    do {
      *q++ = *(ghost + c0 * 256 + *p++);
    } while (--w);
    q += vga_width - (w = copy_w);
    p += remain_x;
  } while (--copy_h);
}

//-----------------------------------------------------------------------------
//      Place a graphic on the editing toolbar
//-----------------------------------------------------------------------------

void put_bar(int x, int y, int n) {
  wput_in_box(toolbar, vga_width / big2, vga_width / big2, 19, x, y, -n);
}

void put_bar_inv(int x, int y, int n) {
  wput_in_box(toolbar, vga_width / big2, vga_width / big2, 19, x, y,
              -n); // TODO: Highlight inversion does not render correctly
  wresalta_box(toolbar, vga_width / big2, 19, x, y, *((word *)graf[n]) / big2,
               *((word *)(graf[n] + 2)) / big2);
}
