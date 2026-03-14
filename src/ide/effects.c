//-----------------------------------------------------------------------------
//      Module for creating explosions
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"

//-----------------------------------------------------------------------------
//      Definitions
//-----------------------------------------------------------------------------

#define max_pun      64 // Maximum number of control points
#define MAX_EXP_SIZE 8  // Maximum number of base explosions
#define DEEP         4
// #pragma on (check_stack)  /* Watcom-specific, not needed */

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef byte
#define byte unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef word
#define word unsigned short
#endif

//-----------------------------------------------------------------------------
//      Internal function declarations
//-----------------------------------------------------------------------------

void init_rnd(int, char *); // Initialize the random generator with an int and key
byte rndb(void);            // Get a byte from the random generator
int rnd(void);              // Get an int from the random generator

void gamma0(void);
byte exp_colores[128];
struct _gcolor exp_gama[9];

//-----------------------------------------------------------------------------
//      Tables and global variables
//-----------------------------------------------------------------------------

int cx, cy;
int exp_gamma;

struct tp {   // Structure for explosion points
  int x, y;   // Point coordinates
  int radius; // Effective radius of the point
  int force;  // Central force of the point
  int xr, yr; // Coord. * 10000
  int ix, iy; // Increments
};

struct te { // Structure for a base explosion
  struct tp p[max_pun];
  byte *ptr;
} e[MAX_EXP_SIZE];

//-----------------------------------------------------------------------------
//      Parameters for creating the explosion
//-----------------------------------------------------------------------------

int exp_ancho = 32; // 8..256
int exp_alto = 32;  // 8..256
int n_pun;          // 16..64
int n_exp;          // 1..8
int n_frames;       // 1..n
int _n_frames = 6;  // 1..n
int paso_frame;
int exp_type = 1;
int per_points;
char *exp_buffer;

//-----------------------------------------------------------------------------
//	Create the point array
//-----------------------------------------------------------------------------

void create_points(void) {
  int m, n, rx, ry;
  double ang, dist;

  for (m = 0; m < n_exp; m++) {
    for (n = 0; n < n_pun; n++) {
      ang = (double)rnd() / 10000.0;            // Angle (?)
      dist = (double)(rnd() % 10000) / 10000.0; // Distance (0 .. 0.8)

      e[m].p[n].ix = cos(ang) * 10000.0;
      e[m].p[n].iy = sin(ang) * 10000.0;

      e[m].p[n].x = cx + (int)(cos(ang) * dist * (float)(exp_ancho / 2));
      e[m].p[n].y = cy + (int)(sin(ang) * dist * (float)(exp_alto / 2));

      e[m].p[n].xr = e[m].p[n].x * 10000;
      e[m].p[n].yr = e[m].p[n].y * 10000;

      if (e[m].p[n].x < cx)
        rx = e[m].p[n].x;
      else
        rx = exp_ancho - e[m].p[n].x;
      if (e[m].p[n].y < cy)
        ry = e[m].p[n].y;
      else
        ry = exp_alto - e[m].p[n].y;
      if (rx < ry)
        e[m].p[n].radius = rx * 10000;
      else
        e[m].p[n].radius = ry * 10000;

      e[m].p[n].force = e[m].p[n].radius + rnd() % (e[m].p[n].radius) * 4;
    }
  }
}

//-----------------------------------------------------------------------------
//      Advance one frame in the explosion
//-----------------------------------------------------------------------------

void advance_points(void) {
  int m, n;
  for (m = 0; m < n_exp; m++) {
    for (n = 0; n < n_pun; n++) {
      e[m].p[n].xr += (e[m].p[n].ix * paso_frame) / 10000;
      e[m].p[n].yr += (e[m].p[n].iy * paso_frame) / 10000;
      e[m].p[n].x = e[m].p[n].xr / 10000;
      e[m].p[n].y = e[m].p[n].yr / 10000;
      if (e[m].p[n].force > e[m].p[n].radius)
        e[m].p[n].force = (int)((float)e[m].p[n].force * (float)0.86); //0.98
      e[m].p[n].radius -= paso_frame;
    }
  }
}

//-----------------------------------------------------------------------------
//	Render the explosion
//-----------------------------------------------------------------------------

int paint_explosion(void) {
  int m, x, y, dx, dy, n;
  int dist, exp_Color, exp_Coloracum;

  show_progress((char *)texts[434], 0, exp_alto);
  for (y = 0; y < exp_alto; y++) {
    show_progress((char *)texts[434], y, exp_alto);
    poll_keyboard();
    if (key(_ESC)) {
      show_progress((char *)texts[434], exp_alto, exp_alto);
      return (1);
    }
    for (x = 0; x < exp_ancho; x++) {
      exp_Coloracum = 0;
      for (m = 0; m < n_exp; m++) {
        exp_Color = 0;
        for (n = 0; n < n_pun; n++) {
          dx = abs(x - e[m].p[n].x);
          dy = abs(y - e[m].p[n].y);
          if ((dx < e[m].p[n].radius / 10000) && (dy < e[m].p[n].radius / 10000)) {
            dist = sqrt(dx * dx + dy * dy);
            if (dist < e[m].p[n].radius / 10000)
              exp_Color += (((e[m].p[n].radius / 10000) - dist) * (e[m].p[n].force / 10000)) /
                           (e[m].p[n].radius / 10000);
          }
        }
        if (exp_Color > 255)
          exp_Color = 255;
        switch (exp_type) {
        case 0:
        case 3:
          exp_Coloracum += exp_Color;
          break;
        case 1:
        case 2:
          if (m % 2)
            exp_Coloracum -= exp_Color;
          else
            exp_Coloracum += exp_Color;
          break;
        }
      }
      switch (exp_type) {
      case 0:
      case 3:
        exp_buffer[y * exp_ancho + x] = exp_Coloracum / n_exp;
        break;
      case 1:
      case 2:
        if (exp_Coloracum > 255)
          exp_Coloracum = 255;
        if (exp_Coloracum < 0)
          exp_Coloracum = 0;
        exp_buffer[y * exp_ancho + x] = exp_Coloracum;
        break;
      }
    }
  }
  show_progress((char *)texts[434], exp_alto, exp_alto);
  for (n = 0; n < (exp_ancho * exp_alto) * per_points / 100; n++) {
    x = (rnd() % (exp_ancho - 2)) + 1;
    y = (rnd() % (exp_alto - 2)) + 1;
    if (exp_buffer[y * exp_ancho + x] > DEEP * 2)
      exp_buffer[y * exp_ancho + x] -= DEEP * 2;

    if (exp_buffer[y * exp_ancho + x - 1] > DEEP)
      exp_buffer[y * exp_ancho + x - 1] -= DEEP;

    if (exp_buffer[y * exp_ancho + x + 1] > DEEP)
      exp_buffer[y * exp_ancho + x + 1] -= DEEP;

    if (exp_buffer[(y - 1) * exp_ancho + x] > DEEP)
      exp_buffer[(y - 1) * exp_ancho + x] -= DEEP;
    if (exp_buffer[(y + 1) * exp_ancho + x] > DEEP)
      exp_buffer[(y + 1) * exp_ancho + x] -= DEEP;
  }
  return (0);
}

char frame_count_str[4];
char cexp_ancho[5];
char cexp_alto[5];
char cper_points[5];
int exp_color0 = -1, exp_color1 = -1, exp_color2 = -1;
int type_a = 1, type_b = 0, type_c = 0;
int old_type_a = 1, old_type_b = 0, old_type_c = 0;

void explode_dialog1(void) {
  int w = v.w / big2, h = v.h / big2;
  int x;

  _show_items();

  create_gradient_colors(exp_gama, exp_colores);

  wbox(v.ptr, w, h, c0, 2, 31, w - 4, 1);

  wbox(v.ptr, w, h, c0, 2, 74, w - 4, 1);

  wwrite(v.ptr, w, h, w - 70, 11, 0, texts[181], c3);
  wrectangle(v.ptr, w, h, c0, w - 70, 18, 66, 11);
  for (x = 0; x < 64; x++)
    wbox(v.ptr, w, h, exp_colores[x * 2], w - 69 + x, 18 + 1, 1, 9);
}

void selcolor0();
extern int sel_color_font;
extern int sel_color_ok;

void explode_dialog2(void) {
  int w = v.w / big2;
  int need_refresh = 0;

  _process_items();
  switch (v.active_item) {
  case 0:
    end_dialog = 1;
    v_accept = 1;
    break;
  case 1:
    end_dialog = 1;
    v_accept = 0;
    break;
  }
  if (!(type_a + type_b + type_c)) {
    type_a = old_type_a;
    type_b = old_type_b;
    type_c = old_type_c;
    need_refresh = 1;
  }
  if (type_a != old_type_a) {
    type_a = 1;
    type_b = type_c = 0;
    old_type_b = old_type_c = 0;
    need_refresh = 1;
  }
  if (type_b != old_type_b) {
    type_b = 1;
    type_a = type_c = 0;
    old_type_a = old_type_c = 0;
    need_refresh = 1;
  }
  if (type_c != old_type_c) {
    type_c = 1;
    type_a = type_b = 0;
    old_type_a = old_type_b = 0;
    need_refresh = 1;
  }
  old_type_a = type_a;
  old_type_b = type_b;
  old_type_c = type_c;
  if (wmouse_in(w - 70, 18, 66, 11) && (mouse_b & 1)) {
    gradient_buf = exp_colores;
    gradient_config = exp_gama;
    show_dialog((void_return_type_t)gamma0);
    if (v_accept)
      need_refresh = 1;
  }
  if (need_refresh) {
    call((void_return_type_t)v.paint_handler);
    v.redraw = 1;
  }
}

void explode_dialog3(void) {
  if (!v_accept)
    return;

  _n_frames = n_frames = atoi(frame_count_str);
  exp_ancho = atoi(cexp_ancho);
  exp_alto = atoi(cexp_alto);
  per_points = atoi(cper_points);
  if (type_a)
    exp_type = 0;
  if (type_b)
    exp_type = 1;
  if (type_c)
    exp_type = 2;
}

void explode_dialog0(void) {
  v.type = WIN_DIALOG;

  v.title = texts[300];
  v.w = 128;
  v.h = 90 + 5;
  v.paint_handler = (void_return_type_t)explode_dialog1;
  v.click_handler = (void_return_type_t)explode_dialog2;
  v.close_handler = (void_return_type_t)explode_dialog3;

  _button(100, 7, v.h - 14, ALIGN_TL);
  _button(101, v.w - 8, v.h - 14, ALIGN_TR);
  type_a = type_b = type_c = old_type_a = old_type_b = old_type_c = 0;
  if (exp_color0 == -1)
    exp_color0 = c4;
  if (exp_color1 == -1)
    exp_color1 = c2;
  if (exp_color2 == -1)
    exp_color2 = c0;
  switch (exp_type) {
  case 0:
    type_a = old_type_a = 1;
    break;
  case 1:
    type_b = old_type_b = 1;
    break;
  case 2:
    type_c = old_type_c = 1;
    break;
  }

  DIV_SPRINTF(frame_count_str, "%d", n_frames);
  DIV_SPRINTF(cexp_ancho, "%d", exp_ancho);
  DIV_SPRINTF(cexp_alto, "%d", exp_alto);
  DIV_SPRINTF(cper_points, "%d", per_points);

  _get(133, 4, 11, 21, (byte *)cexp_ancho, 5, 8, 2000);
  _get(134, 69 - 36, 11, 21, (byte *)cexp_alto, 5, 8, 2000);

  _flag(301, 4, 16 + 19, &type_a);
  _flag(302, 4, 16 + 34, &type_b);
  _flag(303, 4, 16 + 48, &type_c);

  _get(304, 69 - 24 + 30, 16 + 19, 21, (byte *)frame_count_str, 3, 1, 48);
  _get(305, 69 - 24 + 30, 16 + 38, 21, (byte *)cper_points, 3, 0, 100);

  v_accept = 0;
}

void gen_explodes() {
  int ExpDac[256];
  int n = 0, nf, y;
  int x;
  n_frames = _n_frames;
  create_dac4();

  show_dialog((void_return_type_t)explode_dialog0);
  if (!v_accept)
    return;
  for (x = 0; x < 256; x++)
    ExpDac[x] = exp_colores[x / 2];
  init_rnd(*system_clock, NULL);
  exp_buffer = (char *)malloc(exp_ancho * exp_alto);
  n_pun = 32;
  switch (exp_type) {
  case 0:
    n_exp = 4;
    break;
  case 1:
    n_exp = 3;
    break;
  case 2:
    n_exp = 5;
    break;
  case 3:
    n_exp = 1;
    break;
  }

  cx = exp_ancho / 2;
  cy = exp_alto / 2;
  nf = n_frames;
  create_points();
  n_frames *= 2;
  if (exp_ancho < exp_alto)
    paso_frame = (exp_ancho * 10000) / n_frames;
  else
    paso_frame = (exp_alto * 10000) / n_frames;
  do {
    if (paint_explosion())
      break;

    map_width = exp_ancho;
    map_height = exp_alto;

    if (new_map(NULL))
      break;

    for (x = 0; x < map_width * map_height; x++)
      v.mapa->map[x] = ExpDac[exp_buffer[x]];

    v.mapa->zoom_cx = v.mapa->map_width / 2;
    v.mapa->zoom_cy = v.mapa->map_height / 2;

    x = (v.mapa->zoom_cx - vga_width / 2);
    if (x < 0)
      x = 0;
    else if (x + vga_width > v.mapa->map_width)
      x = v.mapa->map_width - vga_width;
    y = (v.mapa->zoom_cy - vga_height / 2);
    if (y < 0)
      y = 0;
    else if (y + vga_height > v.mapa->map_height)
      y = v.mapa->map_height - vga_height;
    v.mapa->zoom = 0;
    v.mapa->zoom_x = x;
    v.mapa->zoom_y = y;

    for (n = 0; n < 512; n++)
      v.mapa->points[n] = -1;
    v.mapa->fpg_code = 0;
    call((void_return_type_t)v.paint_handler);
    blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);

    advance_points();
    n_frames -= 2;
  } while (n_frames);

  free(exp_buffer);
}
