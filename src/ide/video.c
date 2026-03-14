
///////////////////////////////////////////////////////////////////////////////
//      Video Access Module
///////////////////////////////////////////////////////////////////////////////

#include "global.h"
#include "div_string.h"
#include "lib/sdlgfx/SDL_framerate.h"


#define CDEPTH 8

void snapshot(byte *p);
void blit_sdl(byte *p);
void blit_partial_sdl(byte *p);

///////////////////////////////////////////////////////////////////////////////
//	Declarations and module-level data
///////////////////////////////////////////////////////////////////////////////

SDL_Surface *vga;

int is_fullscreen(SDL_Surface *surface) {
  return OSDEP_IsFullScreen();
}

static int windowed_w = 0, windowed_h = 0;

void toggle_fullscreen(SDL_Surface *surface) {
  if (is_fullscreen(surface)) {
    // Going windowed — restore saved windowed size
    if (windowed_w >= 640 && windowed_h >= 480) {
      vga_width = windowed_w;
      vga_height = windowed_h;
    } else {
      // Fallback: use setup values or sensible default
      vga_width = (VS_WIDTH >= 640) ? VS_WIDTH : 640;
      vga_height = (VS_HEIGHT >= 480) ? VS_HEIGHT : 480;
    }
    fsmode = 0;
  } else {
    // Going fullscreen — save current windowed size
    windowed_w = vga_width;
    windowed_h = vga_height;
    fsmode = 1;
  }

  setup_video_mode();
  set_dac(dac);
}


#define MAX_YRES 2048

static short scan[MAX_YRES * 4]; // Por scan [x,an,x,an] se definen hasta 2 segmentos a volcar

///////////////////////////////////////////////////////////////////////////////
//      Awaits the arrival of the vertical retrace (vsync)
///////////////////////////////////////////////////////////////////////////////

FPSmanager fpsman;

void retrace_wait(void) {
  SDL_framerateDelay(&fpsman);
}

///////////////////////////////////////////////////////////////////////////////
//      Activate a palette
///////////////////////////////////////////////////////////////////////////////


void set_dac(byte *_dac) {
  int i;
  int b = 0;
  for (i = 0; i < 256; i++) {
    colors[i].r = _dac[b] * 4;
    colors[i].g = _dac[b + 1] * 4;
    colors[i].b = _dac[b + 2] * 4;
    b += 3;
  }
  if (!OSDEP_SetPalette(vga, colors, 0, 256)) {
    printf("Failed to set palette :(\n");
  }
  full_redraw = 1; /* Palette change requires full 8->32 re-conversion */
  retrace_wait();
}

//-----------------------------------------------------------------------------
//      Set Video Mode (vga_width y vga_height se definen en shared.h)
//-----------------------------------------------------------------------------

SDL_Surface *copy_surface(SDL_Surface *source) {
  SDL_Surface *target;

  target = SDL_CreateRGBSurface(0, source->w, source->h, source->format->BitsPerPixel,
                                source->format->Rmask, source->format->Gmask, source->format->Bmask,
                                source->format->Amask);

  /*
     * I really don't understand why this is necessary. This is supposed to
     * clear the SDL_SRCALPHA flag presumably set by IMG_Load. But why wouldn't
     * blitting work otherwise?
     */
  //    SDL_SetAlpha(source, 0, 0);

  SDL_BlitSurface(source, 0, target, 0);
  return target;
}


void setup_video_mode(void) {
  Uint32 colorkey = 0;
  int vn = 0;

  SDL_initFramerate(&fpsman);
  SDL_setFramerate(&fpsman, 60);


  fprintf(stdout, "full screen: %d\n", fsmode);

  if (fsmode == 0)
    vga = OSDEP_SetVideoMode(vga_width, vga_height, CDEPTH, 0);

  else
    vga = OSDEP_SetVideoMode(vga_width, vga_height, CDEPTH, 1);

  set_dac(dac);
}

void setup_modex(int m) {
  printf("TODO - divvideo.cpp setup_modex.cpp\n");
}

//-----------------------------------------------------------------------------
//      Reset Video Mode
//-----------------------------------------------------------------------------

void reset_video_mode(void) {
  if (is_fullscreen(vga))
    toggle_fullscreen(vga);
}

//-----------------------------------------------------------------------------
//      Dump buffer to vga (screen)
//-----------------------------------------------------------------------------

void blit_partial_sdl(byte *p) {
  int y = 0, n;
  int x_min = vga_width, x_max = 0, y_min = vga_height, y_max = 0;
  byte *q = (byte *)vga->pixels;

  if (SDL_MUSTLOCK(vga))
    SDL_LockSurface(vga);

  while (y < vga_height) {
    n = y * 4;
    if (scan[n + 1]) {
      memcpy(q + scan[n], p + scan[n], scan[n + 1]);
      if (scan[n] < x_min)
        x_min = scan[n];
      if (scan[n] + scan[n + 1] > x_max)
        x_max = scan[n] + scan[n + 1];
      if (y < y_min)
        y_min = y;
      y_max = y + 1;
    }
    if (scan[n + 3]) {
      memcpy(q + scan[n + 2], p + scan[n + 2], scan[n + 3]);
      if (scan[n + 2] < x_min)
        x_min = scan[n + 2];
      if (scan[n + 2] + scan[n + 3] > x_max)
        x_max = scan[n + 2] + scan[n + 3];
      if (y < y_min)
        y_min = y;
      y_max = y + 1;
    }
    q += vga->pitch;
    p += vga_width;
    y++;
  }

  if (SDL_MUSTLOCK(vga))
    SDL_UnlockSurface(vga);

  if (x_max > x_min)
    OSDEP_UpdateRect(vga, x_min, y_min, x_max - x_min, y_max - y_min);
}

void blit_sdl(byte *p) {
  int vy;

  if (SDL_MUSTLOCK(vga))
    SDL_LockSurface(vga);

  byte *q = (byte *)vga->pixels;
  for (vy = 0; vy < vga_height; vy++) {
    memcpy(q, p, vga_width);
    p += vga_width;
    q += vga->pitch;
  }

  if (SDL_MUSTLOCK(vga))
    SDL_UnlockSurface(vga);

  OSDEP_Flip(vga);
}

void blit_screen(byte *p) {
  if ((shift_status & MOD_CTRL) && (shift_status & MOD_ALT) && scan_code == _P)
    snapshot(p);


  if (full_redraw)
    blit_sdl(p);
  else
    blit_partial_sdl(p);

  {
    static uint32_t fs_cooldown = 0;
    if (shift_status & MOD_ALT && key(_ENTER) && SDL_GetTicks() - fs_cooldown > 500) {
      toggle_fullscreen(vga);
      fs_cooldown = SDL_GetTicks();
    }
  }


  init_flush();
}

void snapshot(byte *p) {
  FILE *f;
  int man = map_width, mal = map_height, n = 0;
  char cwork[128];

  do {
    div_snprintf(cwork, sizeof(cwork), "DIV_%04d.PCX", n++);
    if ((f = fopen(cwork, "rb")) != NULL)
      fclose(f);
  } while (f != NULL);

  f = fopen(cwork, "wb");
  map_width = vga_width;
  map_height = vga_height;
  fmt_save_pcx(p, f);
  map_width = man;
  map_height = mal;
  fclose(f);
}

//-----------------------------------------------------------------------------
//      Mark a window region for subsequent blit_screen
//-----------------------------------------------------------------------------

void init_flush(void) {
  memset(&scan[0], 0, MAX_YRES * 8);
  full_redraw = 0;
}

void blit_partial(int x, int y, int w, int h) {
  int ymax = 0, xmax = 0, n = 0, d1 = 0, d2 = 0, x2 = 0;

  if (w == vga_width && h == vga_height && x == 0 && y == 0) {
    full_redraw = 1;
    return;
  }

  if (w > 0 && h > 0 && x < vga_width && y < vga_height) {
    if (x < 0) {
      w += x;
      x = 0;
    }
    if (y < 0) {
      h += y;
      y = 0;
    }
    if (x + w > vga_width) {
      w = vga_width - x;
    }
    if (y + h > vga_height) {
      h = vga_height - y;
    }
    if (w <= 0 || h <= 0)
      return;
    xmax = x + w - 1;
    ymax = y + h - 1;

    while (y <= ymax) {
      n = y * 4;
      if (scan[n + 1] == 0) { // Caso 1, el scan estaba vacío ...
        scan[n] = x;
        scan[n + 1] = w;
      } else if (scan[n + 3] == 0) {                        // Caso 2, ya hay un scan definido ...
        if (x > scan[n] + scan[n + 1] || x + w < scan[n]) { // ... hueco entre medias
          if (x > scan[n]) {
            scan[n + 2] = x;
            scan[n + 3] = w;
          } else {
            scan[n + 2] = scan[n];
            scan[n + 3] = scan[n + 1];
            scan[n] = x;
            scan[n + 1] = w;
          }
        } else { // ... no hay hueco, amplia el primer scan
          if (x < (x2 = scan[n]))
            scan[n] = x;
          if (x + w > x2 + scan[n + 1])
            scan[n + 1] = x + w - scan[n];
          else
            scan[n + 1] = x2 + scan[n + 1] - scan[n];
        }
      } else { // Caso 3, hay 2 scanes definidos ...
        if (x <= scan[n] + scan[n + 1] && x + w >= scan[n + 2]) {
          // Caso 3.1, se tapa el hueco anterior -> queda un solo scan
          if (x < scan[n])
            scan[n] = x;
          if (x + w > scan[n + 2] + scan[n + 3])
            scan[n + 1] = x + w - scan[n];
          else
            scan[n + 1] = scan[n + 2] + scan[n + 3] - scan[n];
          scan[n + 2] = 0;
          scan[n + 3] = 0;
        } else {
          if (x > scan[n] + scan[n + 1] || x + w < scan[n]) {           // No choca con 1º
            if (x > scan[n + 2] + scan[n + 3] || x + w < scan[n + 2]) { // No choca con 2º
              // Caso 3.4, el nuevo no colisiona con ninguno, se calcula el spacing
              // hasta ambos, y se fusiona con el más cercano
              if (x + w < scan[n])
                d1 = scan[n] - (x + w);
              else
                d1 = x - (scan[n] + scan[n + 1]);
              if (x + w < scan[n + 2])
                d2 = scan[n + 2] - (x + w);
              else
                d2 = x - (scan[n + 2] + scan[n + 3]);
              if (d1 <= d2) {
                // Caso 3.4.1 se fusiona con el primero
                if (x < (x2 = scan[n]))
                  scan[n] = x;
                if (x + w > x2 + scan[n + 1])
                  scan[n + 1] = x + w - scan[n];
                else
                  scan[n + 1] = x2 + scan[n + 1] - scan[n];
              } else {
                // Caso 3.4.2 se fusiona con el segundo
                if (x < (x2 = scan[n + 2]))
                  scan[n + 2] = x;
                if (x + w > x2 + scan[n + 3])
                  scan[n + 3] = x + w - scan[n + 2];
                else
                  scan[n + 3] = x2 + scan[n + 3] - scan[n + 2];
              }
            } else {
              // Caso 3.3, el nuevo colisiona con el 2º, se fusionan
              if (x < (x2 = scan[n + 2]))
                scan[n + 2] = x;
              if (x + w > x2 + scan[n + 3])
                scan[n + 3] = x + w - scan[n + 2];
              else
                scan[n + 3] = x2 + scan[n + 3] - scan[n + 2];
            }
          } else {
            // Caso 3.2, el nuevo colisiona con el 1º, se fusionan
            if (x < (x2 = scan[n]))
              scan[n] = x;
            if (x + w > x2 + scan[n + 1])
              scan[n + 1] = x + w - scan[n];
            else
              scan[n + 1] = x2 + scan[n + 1] - scan[n];
          }
        }
      }
      y++;
    }
  }
}
