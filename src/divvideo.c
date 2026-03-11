
///////////////////////////////////////////////////////////////////////////////
//      Video Access Module
///////////////////////////////////////////////////////////////////////////////

#include "global.h"
#include "div_string.h"
#include "lib/sdlgfx/SDL_framerate.h"


#define CDEPTH 8

void snapshot(byte *p);
void blit_full_svga(byte *p);
void blit_full_320x200(byte *p);
void blit_full_modex(byte *p);
void blit_partial_svga(byte *p);
void blit_partial_320x200(byte *p);
void blit_partial_modex(byte *p);
void blit_sdl(byte *p);

///////////////////////////////////////////////////////////////////////////////
//	Declarations and module-level data
///////////////////////////////////////////////////////////////////////////////

SDL_Surface *vga;

int IsFullScreen(SDL_Surface *surface) {
  return OSDEP_IsFullScreen();
}

static int windowed_an = 0, windowed_al = 0;

void SDL_ToggleFS(SDL_Surface *surface) {
  if (IsFullScreen(surface)) {
    // Going windowed — restore saved windowed size
    if (windowed_an >= 640 && windowed_al >= 480) {
      vga_width = windowed_an;
      vga_height = windowed_al;
    } else {
      // Fallback: use setup values or sensible default
      vga_width = (VS_WIDTH >= 640) ? VS_WIDTH : 640;
      vga_height = (VS_HEIGHT >= 480) ? VS_HEIGHT : 480;
    }
    fsmode = 0;
  } else {
    // Going fullscreen — save current windowed size
    windowed_an = vga_width;
    windowed_al = vga_height;
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
  retrace_wait();
}

//-----------------------------------------------------------------------------
//      Set Video Mode (vga_width y vga_height se definen en shared.h)
//-----------------------------------------------------------------------------

int LinealMode;
int modovesa;


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

  modovesa = 1;

  set_dac(dac);
}

void setup_modex(int m) {
  printf("TODO - divvideo.cpp setup_modex.cpp\n");
}

//-----------------------------------------------------------------------------
//      Reset Video Mode
//-----------------------------------------------------------------------------

void reset_video_mode(void) {
  if (IsFullScreen(vga))
    SDL_ToggleFS(vga);
}

//-----------------------------------------------------------------------------
//      Dump buffer to vga (screen)
//-----------------------------------------------------------------------------
void vgacpy(byte *q, byte *p, int n);


void volcadosdlp(byte *p) {
  blit_sdl(p);
  return;

  int y = 0, n;
  byte *oldp = (byte *)p;
  uint32_t x1 = vga_width, y1 = vga_height, w1 = 1, h1 = 1;
  SDL_Rect rc;
  byte *q = (byte *)vga->pixels;
  uint32_t *q32 = (uint32_t *)vga->pixels;
  SDL_LockSurface(vga);

  while (y < vga_height) {
    n = y * 4;
    if (scan[n + 1]) {
      memcpy(q + scan[n], p + scan[n], scan[n + 1]);
      if (scan[n] < x1)
        x1 = scan[n];
      if (y < y1)
        y1 = y;
      if (scan[n] + scan[n + 1] + 1 - x1 > w1)
        w1 = scan[n] + scan[n + 1] + 1 - x1;

      if (y + 1 - y1 > h1)
        h1 = (y + 1 - y1);
    }
    if (scan[n + 3]) {
      memcpy(q + scan[n + 2], p + scan[n + 2], scan[n + 3]);
      if (scan[n + 2] < x1)
        x1 = scan[n + 2];
      if (y < y1)
        y1 = y;
      if (scan[n + 2] + scan[n + 3] + 1 - x1 > w1)
        w1 = scan[n + 2] + scan[n + 3] + 1 - x1;

      if (y + 1 - y1 > h1)
        h1 = (y + 1 - y1);
    }

    q += vga->pitch;
    p += vga_width;
    y++;
  }
  SDL_UnlockSurface(vga);
  printf("changed rect: %d %d %d %d     \r", x1, y1, w1, h1);

  OSDEP_UpdateRect(vga, x1, y1, w1, h1);

  return;
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
  if ((shift_status & 4) && (shift_status & 8) && scan_code == _P)
    snapshot(p);


  if (full_redraw) {
    if (modovesa)
      blit_sdl(p);
    else
      switch (vga_width * 1000 + vga_height) {
      case 320200:
        blit_full_320x200(p);
        break;
      case 320240:
        blit_full_modex(p);
        break;
      case 320400:
        blit_full_modex(p);
        break;
      case 360240:
        blit_full_modex(p);
        break;
      case 360360:
        blit_full_modex(p);
        break;
      case 376282:
        blit_full_modex(p);
        break;
      }
  } else {
    if (modovesa)
      volcadosdlp(p);
    else
      switch (vga_width * 1000 + vga_height) {
      case 320200:
        blit_partial_320x200(p);
        break;
      case 320240:
        blit_partial_modex(p);
        break;
      case 320400:
        blit_partial_modex(p);
        break;
      case 360240:
        blit_partial_modex(p);
        break;
      case 360360:
        blit_partial_modex(p);
        break;
      case 376282:
        blit_partial_modex(p);
        break;
      }
  }

  {
    static uint32_t fs_cooldown = 0;
    if (shift_status & 8 && key(_ENTER) && SDL_GetTicks() - fs_cooldown > 500) {
      SDL_ToggleFS(vga);
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
  save_PCX(p, f);
  map_width = man;
  map_height = mal;
  fclose(f);
}

//-----------------------------------------------------------------------------
//      Dump mode 320x200
//-----------------------------------------------------------------------------

void blit_partial_320x200(byte *p) { // PARTIAL
}

void blit_full_320x200(byte *p) { // COMPLETE
}

//-----------------------------------------------------------------------------
//      SVGA DUMP
//-----------------------------------------------------------------------------

void blit_partial_svga(byte *p) {
  int y = 0, page, old_page = -1751, point, t1, t2, n;
  char *q = (char *)vga->pixels;

  if (LinealMode) {
    while (y < vga_height) {
      n = y * 4;
      if (scan[n + 1])
        memcpy(q + scan[n], p + scan[n], scan[n + 1]);
      if (scan[n + 3])
        memcpy(q + scan[n + 2], p + scan[n + 2], scan[n + 3]);
      q += vga_width;
      p += vga_width;
      y++;
    }
  } else
    while (y < vga_height) {
      n = y * 4;
      if (scan[n + 1]) {
        page = (y * vga_width + scan[n]) / 65536;
        point = (y * vga_width + scan[n]) % 65536;
        if (point + scan[n + 1] > 65536) {
          t1 = 65536 - point;
          t2 = scan[n + 1] - t1;
          memcpy(vga + point, p + scan[n], t1);
          memcpy(vga, p + scan[n] + t1, t2);
        } else {
          memcpy(vga + point, p + scan[n], scan[n + 1]);
        }
      }
      if (scan[n + 3]) {
        page = (y * vga_width + scan[n + 2]) / 65536;
        point = (y * vga_width + scan[n + 2]) % 65536;
        if (point + scan[n + 3] > 65536) {
          t1 = 65536 - point;
          t2 = scan[n + 3] - t1;
          memcpy(vga + point, p + scan[n + 2], t1);
          memcpy(vga, p + scan[n + 2] + t1, t2);
        } else {
          memcpy(vga + point, p + scan[n + 2], scan[n + 3]);
        }
      }
      p += vga_width;
      y++;
    }
}

void blit_full_svga(byte *p) {
  int cnt = vga_width * vga_height;
  int tpv = 0, ActPge = 0;

  if (LinealMode)
    memcpy(vga, p, cnt);
  else
    while (cnt > 0) {
      tpv = cnt > 65536 ? 65536 : cnt;
      memcpy(vga, p, tpv);
      p += tpv;
      cnt -= tpv;
    }
}

//-----------------------------------------------------------------------------
//      Volcado en un modo-x
//-----------------------------------------------------------------------------

void blit_partial_modex(byte *p) {}

void blit_full_modex(byte *p) {}


//-----------------------------------------------------------------------------
//      Subrutinas de blit_screen genéricas
//-----------------------------------------------------------------------------

void vgacpy(byte *q, byte *p, int n) {}

//-----------------------------------------------------------------------------
//      Select a window for subsequent dump
//-----------------------------------------------------------------------------

void init_flush(void) {
  memset(&scan[0], 0, MAX_YRES * 8);
  full_redraw = 0;
}

void blit_partial(int x, int y, int an, int al) {
  int ymax = 0, xmax = 0, n = 0, d1 = 0, d2 = 0, x2 = 0;

  if (an == vga_width && al == vga_height && x == 0 && y == 0) {
    full_redraw = 1;
    return;
  }

  if (an > 0 && al > 0 && x < vga_width && y < vga_height) {
    if (x < 0) {
      an += x;
      x = 0;
    }
    if (y < 0) {
      al += y;
      y = 0;
    }
    if (x + an > vga_width) {
      an = vga_width - x;
    }
    if (y + al > vga_height) {
      al = vga_height - y;
    }
    if (an <= 0 || al <= 0)
      return;
    xmax = x + an - 1;
    ymax = y + al - 1;

    if (!modovesa) {
      switch (vga_width * 1000 + vga_height) {
      case 320240:
      case 320400:
      case 360240:
      case 360360:
      case 376282: // Modos X
        x >>= 2;
        xmax >>= 2;
        an = xmax - x + 1;
        break;
      }
    }

    while (y <= ymax) {
      n = y * 4;
      if (scan[n + 1] == 0) { // Caso 1, el scan estaba vacío ...
        scan[n] = x;
        scan[n + 1] = an;
      } else if (scan[n + 3] == 0) {                         // Caso 2, ya hay un scan definido ...
        if (x > scan[n] + scan[n + 1] || x + an < scan[n]) { // ... hueco entre medias
          if (x > scan[n]) {
            scan[n + 2] = x;
            scan[n + 3] = an;
          } else {
            scan[n + 2] = scan[n];
            scan[n + 3] = scan[n + 1];
            scan[n] = x;
            scan[n + 1] = an;
          }
        } else { // ... no hay hueco, amplia el primer scan
          if (x < (x2 = scan[n]))
            scan[n] = x;
          if (x + an > x2 + scan[n + 1])
            scan[n + 1] = x + an - scan[n];
          else
            scan[n + 1] = x2 + scan[n + 1] - scan[n];
        }
      } else { // Caso 3, hay 2 scanes definidos ...
        if (x <= scan[n] + scan[n + 1] && x + an >= scan[n + 2]) {
          // Caso 3.1, se tapa el hueco anterior -> queda un solo scan
          if (x < scan[n])
            scan[n] = x;
          if (x + an > scan[n + 2] + scan[n + 3])
            scan[n + 1] = x + an - scan[n];
          else
            scan[n + 1] = scan[n + 2] + scan[n + 3] - scan[n];
          scan[n + 2] = 0;
          scan[n + 3] = 0;
        } else {
          if (x > scan[n] + scan[n + 1] || x + an < scan[n]) {           // No choca con 1º
            if (x > scan[n + 2] + scan[n + 3] || x + an < scan[n + 2]) { // No choca con 2º
              // Caso 3.4, el nuevo no colisiona con ninguno, se calcula el espacio
              // hasta ambos, y se fusiona con el más cercano
              if (x + an < scan[n])
                d1 = scan[n] - (x + an);
              else
                d1 = x - (scan[n] + scan[n + 1]);
              if (x + an < scan[n + 2])
                d2 = scan[n + 2] - (x + an);
              else
                d2 = x - (scan[n + 2] + scan[n + 3]);
              if (d1 <= d2) {
                // Caso 3.4.1 se fusiona con el primero
                if (x < (x2 = scan[n]))
                  scan[n] = x;
                if (x + an > x2 + scan[n + 1])
                  scan[n + 1] = x + an - scan[n];
                else
                  scan[n + 1] = x2 + scan[n + 1] - scan[n];
              } else {
                // Caso 3.4.2 se fusiona con el segundo
                if (x < (x2 = scan[n + 2]))
                  scan[n + 2] = x;
                if (x + an > x2 + scan[n + 3])
                  scan[n + 3] = x + an - scan[n + 2];
                else
                  scan[n + 3] = x2 + scan[n + 3] - scan[n + 2];
              }
            } else {
              // Caso 3.3, el nuevo colisiona con el 2º, se fusionan
              if (x < (x2 = scan[n + 2]))
                scan[n + 2] = x;
              if (x + an > x2 + scan[n + 3])
                scan[n + 3] = x + an - scan[n + 2];
              else
                scan[n + 3] = x2 + scan[n + 3] - scan[n + 2];
            }
          } else {
            // Caso 3.2, el nuevo colisiona con el 1º, se fusionan
            if (x < (x2 = scan[n]))
              scan[n] = x;
            if (x + an > x2 + scan[n + 1])
              scan[n + 1] = x + an - scan[n];
            else
              scan[n + 1] = x2 + scan[n + 1] - scan[n];
          }
        }
      }
      y++;
    }
  }
}
