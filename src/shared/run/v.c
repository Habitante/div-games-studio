
///////////////////////////////////////////////////////////////////////////////
// Video functions
///////////////////////////////////////////////////////////////////////////////

#include "inter.h"
#include "madewith.h"
#include "../../div_string.h"

///////////////////////////////////////////////////////////////////////////////
//	Declarations and module-level data
///////////////////////////////////////////////////////////////////////////////

#define CRTC_INDEX  0x3d4 //CRT Controller Index
#define CRTC_OFFSET 0x13  //CRTC offset register index
#define SC_INDEX    0x3c4 //Sequence Controller Index
#define MISC_OUTPUT 0x3c2 //Miscellaneous Output register

SDL_Surface *vga = NULL;

void snapshot(byte *p);
void blit_full_svga(byte *p);
void blit_full_320x200(byte *p);
void blit_full_modex(byte *p);
void blit_partial_svga(byte *p);
void blit_partial_320x200(byte *p);
void blit_partial_modex(byte *p);
int save_PCX(byte *mapa, int w, int h, FILE *f);
void create_ghost_vc(int m);
void create_ghost_slow(void);

// Check if SDL is already loaded

#ifndef __EMSCRIPTEN__
int is_fullscreen(SDL_Surface *surface) {
  return OSDEP_IsFullScreen(); // Return false if surface is windowed
}

int toggle_fullscreen(SDL_Surface *surface) {
  if (is_fullscreen(surface))
    fsmode = 0;
  else
    fsmode = 1;

  setup_video_mode();
  set_dac();
  return 1;
}
#endif

#define MAX_YRES 2048

short scan[MAX_YRES * 4]; // Per scanline [x,width,x,width] up to 2 blit segments

struct {
  byte dot;
  int crt[20];
} modox[5] = {

    {0xe3,
     {0x0d06, 0x3e07, 0x4109, 0xea10, 0xac11, 0xdf12, 0x0014, 0xe715, //320x240
      0x0616, 0xe317, 0}},

    {0xe3, {0x4009, 0x0014, 0xe317, 0}}, //320x400

    {0xe7,
     {0x6b00, 0x5901, 0x5a02, 0x8e03, 0x5e04, 0x8a05, 0x0d06, 0x3e07, //360x240
      0x4109, 0xea10, 0xac11, 0xdf12, 0x2d13, 0x0014, 0xe715, 0x0616, 0xe317, 0}},

    {0xe7,
     {0x6b00, 0x5901, 0x5a02, 0x8e03, 0x5e04, 0x8a05, 0x4009, 0x8810, //360x360
      0x8511, 0x6712, 0x2d13, 0x0014, 0x6d15, 0xba16, 0xe317, 0}},

    {0xe7,
     {0x6e00, 0x5d01, 0x5e02, 0x9103, 0x6204, 0x8f05, 0x6206, 0xf007, //376x282
      0x6109, 0x310f, 0x3710, 0x8911, 0x3312, 0x2f13, 0x0014, 0x3c15, 0x5c16, 0xe317, 0}}};

//-----------------------------------------------------------------------------
//      Ghost (transparency) table
//-----------------------------------------------------------------------------

struct t_tpuntos { // For building the ghost table
  int r, g, b;
  struct t_tpuntos *next;
} tpuntos[256];

struct t_tpuntos *vcubos[512]; // For building the ghost table

extern int fli_palette_update;

//----------------------------------------------------------------------------
// Function to set the palette
//----------------------------------------------------------------------------

byte dark_color;

void update_palette(void) {
  word n;
  n = abs(dacout_speed); // if (n>64) n=64;

  if (now_dacout_r < dacout_r) {
    if (now_dacout_r + n < dacout_r)
      now_dacout_r += n;
    else
      now_dacout_r = dacout_r;
  } else if (now_dacout_r > dacout_r) {
    if (now_dacout_r - n > dacout_r)
      now_dacout_r -= n;
    else
      now_dacout_r = dacout_r;
  }

  if (now_dacout_g < dacout_g) {
    if (now_dacout_g + n < dacout_g)
      now_dacout_g += n;
    else
      now_dacout_g = dacout_g;
  } else if (now_dacout_g > dacout_g) {
    if (now_dacout_g - n > dacout_g)
      now_dacout_g -= n;
    else
      now_dacout_g = dacout_g;
  }

  if (now_dacout_b < dacout_b) {
    if (now_dacout_b + n < dacout_b)
      now_dacout_b += n;
    else
      now_dacout_b = dacout_b;
  } else if (now_dacout_b > dacout_b) {
    if (now_dacout_b - n > dacout_b)
      now_dacout_b -= n;
    else
      now_dacout_b = dacout_b;
  }

  n = 0;
  do {
    if (now_dacout_r > paleta[n]) {
      dac[n] = 0;
    } else {
      dac[n] = paleta[n] - now_dacout_r;
    }
    if (dac[n] > 63) {
      dac[n] = 63;
    }
    n++;
    if (now_dacout_g > paleta[n]) {
      dac[n] = 0;
    } else {
      dac[n] = paleta[n] - now_dacout_g;
    }
    if (dac[n] > 63) {
      dac[n] = 63;
    }
    n++;
    if (now_dacout_b > paleta[n]) {
      dac[n] = 0;
    } else {
      dac[n] = paleta[n] - now_dacout_b;
    }
    if (dac[n] > 63) {
      dac[n] = 63;
    }
    n++;
  } while (n < 768);

  dark_color = 0;

  if (process_active_palette != NULL)
    process_active_palette();
}
extern int splashtime;

void set_dac(void) {
#ifndef DEBUG
  if (splashtime > 0)
    return;
#endif
  if (vga == NULL)
    return;

  SDL_Color colors[512];

  int i;
  int b = 0;
  for (i = 0; i < 256; i++) {
    colors[i].r = dac[b] * 4;
    colors[i].g = dac[b + 1] * 4;
    colors[i].b = dac[b + 2] * 4;
    b += 3;
  }
  if (!OSDEP_SetPalette(vga, colors, 0, 256)) {
    printf("Failed to set palette :(\n");
  }

  retrace_wait();
}

void set_dac2(void) {
  set_dac(); // FLI palette update — same as set_dac on SDL
}

void retrace_wait(void) {}

// Blocking fade: advance palette toward target, presenting each step.
// Replaces DOS spin-wait loops that wrote VGA DAC registers directly.
void fade_wait(void) {
  while (now_dacout_r != dacout_r || now_dacout_g != dacout_g || now_dacout_b != dacout_b) {
    update_palette();
    set_dac();
    OSDEP_Flip(vga);
    SDL_Delay(16);
  }
  fading = 0;
}

//-----------------------------------------------------------------------------
//      Set Video Mode (vga_width and vga_height are defined in shared.h)
//-----------------------------------------------------------------------------

int lineal_mode;
int vesa_mode;

extern float m_x, m_y;


extern int oldticks;
void madewith(void) {
  SDL_RWops *rwops = NULL;
  SDL_Surface *mwsurface, *image;

  rwops = SDL_RWFromMem(madewithsplash, 256138);
  mwsurface = SDL_LoadBMP_RW(rwops, 1);

  SDL_BlitSurface(mwsurface, NULL, vga, NULL);
  OSDEP_Flip(mwsurface);
  SDL_FreeSurface(mwsurface);
}

void setup_video_mode(void) {
  //#ifdef STDOUTLOG
  printf("setting new video mode %d %d %p\n", vga_width, vga_height, (void *)vga);
  //#endif

  //hide the mouse
  SDL_ShowCursor(SDL_DISABLE);
  vga = NULL;
#ifdef __EMSCRIPTEN__

  if (!vga)
    vga = OSDEP_SetVideoMode(vga_width, vga_height, 8, 0);
#else

  if (fsmode == 1)
    vga = OSDEP_SetVideoMode(vga_width, vga_height, 8, 1);

  if (!vga || fsmode == 0)
    vga = OSDEP_SetVideoMode(vga_width, vga_height, 8, 0);

#endif
  printf("Set mode: %d,%d\n", vga->w, vga->h);


#ifdef STDOUTLOG
  printf("SET VIDEO MODE %p\n", (void *)vga);
#endif
  OSDEP_SetCaption("DIVDX 3.01", "");

  vesa_mode = 1;

  m_x = (float)vga_width / 2.0;
  m_y = (float)vga_height / 2.0;

  if (demo) {
    texts[max_texts].type = 0;
    texts[max_texts].centro = 4;
    texts[max_texts].y = vga_height / 2;
    texts[max_texts].x = vga_width / 2;
    texts[max_texts].font = (byte *)fonts[0];
  } else
    texts[max_texts].font = 0;

#ifndef DEBUG
  if (splashtime > 0)
    madewith();
#endif
}


void setup_modex(int m) {}

//-----------------------------------------------------------------------------
//      Reset Video Mode
//-----------------------------------------------------------------------------

void reset_video_mode(void) {
// Emscripten has its own fullscreen controls
#ifndef __EMSCRIPTEN__
  if (is_fullscreen(vga))
    toggle_fullscreen(vga);
#endif
}

//-----------------------------------------------------------------------------
//      Dump buffer to VGA
//-----------------------------------------------------------------------------
/* Blit the 8-bit paletted framebuffer (p) to the SDL2 display surface (vga).
 * Copies row-by-row to handle pitch differences, then calls OSDEP_Flip()
 * which converts 8-bit->32-bit via palette and presents to screen.
 */
void blit_sdl(byte *p) {
  if (SDL_MUSTLOCK(vga))
    SDL_LockSurface(vga);

  byte *q = (byte *)vga->pixels;
  int vy;
  for (vy = 0; vy < vga_height; vy++) {
    memcpy(q, p, vga_width);
    p += vga_width;
    q += vga->pitch;
  }

  if (SDL_MUSTLOCK(vga))
    SDL_UnlockSurface(vga);

  OSDEP_Flip(vga);
}

long lasttick = 0;
long newtick = 0;
long nexttick = 0;
extern int game_fps;
int framecount = 0;
int recording = 0;
int tfs = 0;
extern int alt_x;

#define maxframes 30000

void blit_screen(byte *p) {
#ifndef __EMSCRIPTEN__
  if ((shift_status & 4) && (shift_status & 8) && key(_0)) {
    recording = 1;
  }
  if ((shift_status & 4) && (shift_status & 8) && key(_9)) {
    recording = 0;
  }

  if (recording) {
    framecount++;
    if (!(framecount % (fps / 12)) && framecount > 5 &&
        framecount < maxframes) { // && framecount%3==1) {
      char filename[255];
      memset(filename, 0, 255);
      div_snprintf(filename, sizeof(filename), "out%05d.bmp", framecount);
      SDL_SaveBMP(vga, filename);
    } else {
      if (framecount > maxframes)
        alt_x = 1;
    }
    //	}
  }
  // CTRL + ALT + P

  if ((shift_status & 4) && (shift_status & 8) && key(_P)) {
    snapshot(p);
    do {
      poll_keyboard();
    } while (key(_P));
  }

  {
    static uint32_t fs_cooldown = 0;
    if (shift_status & 8 && key(_ENTER) && SDL_GetTicks() - fs_cooldown > 500) {
      toggle_fullscreen(vga);
      fs_cooldown = SDL_GetTicks();
    }
  }

#endif
  if (fli_palette_update)
    retrace_wait();

  blit_sdl(p);
  if (fli_palette_update) {
    fli_palette_update = 0;
    set_dac2();
  }
  init_flush();
}

//-----------------------------------------------------------------------------
//      Snapshot
//-----------------------------------------------------------------------------

void snapshot(byte *p) {
  FILE *f;
  int n = 0;
  char cwork[128];

  do {
    div_snprintf(cwork, sizeof(cwork), "SNAP%04d.PCX", n++);
    if ((f = fopen(cwork, "rb")) != NULL)
      fclose(f);
  } while (f != NULL);

  f = fopen(cwork, "wb");
  save_PCX(p, vga_width, vga_height, f);
  fclose(f);
}

typedef struct _pcx_header {
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
  short xmin, ymin;
  short xmax, ymax;
  short hres;
  short vres;
  char palette16[48];
  char reserved;
  char color_planes;
  short bytes_per_line;
  short palette_type;
  short Hresol;
  short Vresol;
  char filler[54];
} pcx_header;

struct pcx_struct {
  pcx_header header;
  unsigned char far *cimage;
  unsigned char palette[3 * 256];
  unsigned char far *image;
  int clength;
};

int save_PCX(byte *mapa, int w, int h, FILE *f) {
  byte p[768];
  int x;
  byte *cbuffer;
  struct pcx_struct pcx;
  int ptr = 0;
  int cptr = 0;
  int Desborde = 0;
  char ActPixel;
  char cntPixel = 0;
  char Paletilla = 12;

  pcx.header.manufacturer = 10;
  pcx.header.version = 5;
  pcx.header.encoding = 1;
  pcx.header.bits_per_pixel = 8;
  pcx.header.xmin = 0;
  pcx.header.ymin = 0;
  pcx.header.xmax = w - 1;
  pcx.header.ymax = h - 1;
  pcx.header.hres = w;
  pcx.header.vres = h;
  pcx.header.color_planes = 1;
  pcx.header.bytes_per_line = w;
  pcx.header.palette_type = 0;
  pcx.header.Hresol = w;
  pcx.header.Vresol = h;

  if ((cbuffer = (unsigned char *)malloc(w * h * 2)) == NULL)
    return (1);

  ActPixel = mapa[ptr];
  while (ptr < w * h) {
    while ((mapa[ptr] == ActPixel) && (ptr < w * h)) {
      cntPixel++;
      Desborde++;
      ptr++;
      if (Desborde == w) {
        Desborde = 0;
        break;
      }
      if (cntPixel == 63)
        break;
    }
    if (cntPixel == 1) {
      if (ActPixel > 63)
        cbuffer[cptr++] = 193;
      cbuffer[cptr++] = ActPixel;
    } else {
      cbuffer[cptr++] = 192 + cntPixel;
      cbuffer[cptr++] = ActPixel;
    }
    ActPixel = mapa[ptr];
    cntPixel = 0;
  }

  fwrite(&pcx.header, 1, sizeof(pcx_header), f);
  fwrite(cbuffer, 1, cptr, f);
  fwrite(&Paletilla, 1, 1, f);
  for (x = 0; x < 768; x++)
    p[x] = paleta[x] * 4;
  fwrite(p, 1, 768, f);
  free(cbuffer);
  return (0);
}

int save_MAP(byte *mapa, int w, int h, FILE *f) {
  int y;
  char cwork[32] = "";
  char gradients[576];

  fwrite("map\x1a\x0d\x0a\x00\x00", 8, 1, f); // +000 Header and version
  fwrite(&w, 2, 1, f);                        // +008 Width
  fwrite(&h, 2, 1, f);                        // +010 Height
  y = 1;
  fwrite(&y, 4, 1, f); // +012 Code

  fwrite(cwork, 32, 1, f);   // +016 Description
  fwrite(paleta, 768, 1, f); // +048 Palette

  for (y = 0; y < 16; y++) {
    gradients[y * 36] = 16;
    gradients[y * 36 + 1] = 0;
    gradients[y * 36 + 2] = 0;
    gradients[y * 36 + 3] = 0;
    memset(&gradients[y * 36 + 4], y * 16, 32);
  }
  fwrite(gradients, 1, sizeof(gradients), f); // +816 Color rules

  y = 0;
  fwrite(&y, 2, 1, f); // +1392 Number of control points
  fwrite(mapa, w * h, 1, f);
  return (0);
}

//-----------------------------------------------------------------------------
//      Partial background restore to the framebuffer copy
//-----------------------------------------------------------------------------

void restore(byte *q, byte *p) {
  int y = 0, n = 0;
  if (vga_width < 640 && vga_height > 200) { // Modo-X
    while (y < vga_height) {
      n = y * 4;
      if (scan[n + 1])
        memcpy(q + scan[n] * 4, p + scan[n] * 4, scan[n + 1] * 4);
      if (scan[n + 3])
        memcpy(q + scan[n + 2] * 4, p + scan[n + 2] * 4, scan[n + 3] * 4);
      q += vga_width;
      p += vga_width;
      y++;
    }
  } else {
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
  }
}

//-----------------------------------------------------------------------------
//      Blit in 320x200 mode
//-----------------------------------------------------------------------------

void blit_partial_320x200(byte *p) {
  int y = 0, n;
  byte *q = (byte *)vga->pixels;

#ifdef GRABADORA
  record_screen(p);
#endif

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
}

void blit_full_320x200(byte *p) {
#ifdef GRABADORA
  record_screen(p);
#endif
  memcpy(vga, p, vga_width * vga_height);
}

//-----------------------------------------------------------------------------
//      Blit in SVGA mode
//-----------------------------------------------------------------------------

void blit_partial_svga(byte *p) {}

void blit_full_svga(byte *p) {}

//-----------------------------------------------------------------------------
//      Blit in Mode-X
//-----------------------------------------------------------------------------

void blit_partial_modex(byte *p) {}

void blit_full_modex(byte *p) {}

//-----------------------------------------------------------------------------
//      Generic blit_screen subroutines
//-----------------------------------------------------------------------------

void vgacpy(byte *q, byte *p, int n) {
  int m;

  m = n >> 2;
  while (m--) {
    *(int *)q = *p + 256 * (*(p + 4) + 256 * (*(p + 8) + 256 * (*(p + 12))));
    q += 4;
    p += 16;
  }

  n &= 3;
  while (n--) {
    *q = *p;
    q++;
    p += 4;
  }
}

//-----------------------------------------------------------------------------
//      Mark a window region for subsequent blit_screen
//-----------------------------------------------------------------------------

void init_flush(void) {
#ifndef DROID
  memset(&scan[0], 0, MAX_YRES * sizeof(short));
#endif
  full_redraw = 0;
}

void blit_partial(int x, int y, int w, int h) {
  int ymax, xmax, n, d1, d2, x2;

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

    if (!vesa_mode) {
      switch (vga_width * 1000 + vga_height) {
      case 320240:
      case 320400:
      case 360240:
      case 360360:
      case 376282: // Mode-X modes
        x >>= 2;
        xmax >>= 2;
        w = xmax - x + 1;
        break;
      }
    }

    while (y <= ymax) {
      n = y * 4;
      if (scan[n + 1] == 0) { // Case 1: scanline was empty ...
        scan[n] = x;
        scan[n + 1] = w;
      } else if (scan[n + 3] == 0) { // Case 2: one segment already defined ...
        if (x > scan[n] + scan[n + 1] || x + w < scan[n]) { // ... gap in between
          if (x > scan[n]) {
            scan[n + 2] = x;
            scan[n + 3] = w;
          } else {
            scan[n + 2] = scan[n];
            scan[n + 3] = scan[n + 1];
            scan[n] = x;
            scan[n + 1] = w;
          }
        } else { // ... no gap, extend the first segment
          if (x < (x2 = scan[n]))
            scan[n] = x;
          if (x + w > x2 + scan[n + 1])
            scan[n + 1] = x + w - scan[n];
          else
            scan[n + 1] = x2 + scan[n + 1] - scan[n];
        }
      } else { // Case 3: two segments already defined ...
        if (x <= scan[n] + scan[n + 1] && x + w >= scan[n + 2]) {
          // Case 3.1: fills the gap -> merges into a single segment
          if (x < scan[n])
            scan[n] = x;
          if (x + w > scan[n + 2] + scan[n + 3])
            scan[n + 1] = x + w - scan[n];
          else
            scan[n + 1] = scan[n + 2] + scan[n + 3] - scan[n];
          scan[n + 2] = 0;
          scan[n + 3] = 0;
        } else {
          if (x > scan[n] + scan[n + 1] || x + w < scan[n]) {           // No overlap with 1st
            if (x > scan[n + 2] + scan[n + 3] || x + w < scan[n + 2]) { // No overlap with 2nd
              // Case 3.4: no overlap with either, calculate distance
              // to both and merge with the nearest
              if (x + w < scan[n])
                d1 = scan[n] - (x + w);
              else
                d1 = x - (scan[n] + scan[n + 1]);
              if (x + w < scan[n + 2])
                d2 = scan[n + 2] - (x + w);
              else
                d2 = x - (scan[n + 2] + scan[n + 3]);
              if (d1 <= d2) {
                // Case 3.4.1: merge with the first segment
                if (x < (x2 = scan[n]))
                  scan[n] = x;
                if (x + w > x2 + scan[n + 1])
                  scan[n + 1] = x + w - scan[n];
                else
                  scan[n + 1] = x2 + scan[n + 1] - scan[n];
              } else {
                // Case 3.4.2: merge with the second segment
                if (x < (x2 = scan[n + 2]))
                  scan[n + 2] = x;
                if (x + w > x2 + scan[n + 3])
                  scan[n + 3] = x + w - scan[n + 2];
                else
                  scan[n + 3] = x2 + scan[n + 3] - scan[n + 2];
              }
            } else {
              // Case 3.3: overlaps with the 2nd segment, merge them
              if (x < (x2 = scan[n + 2]))
                scan[n + 2] = x;
              if (x + w > x2 + scan[n + 3])
                scan[n + 3] = x + w - scan[n + 2];
              else
                scan[n + 3] = x2 + scan[n + 3] - scan[n + 2];
            }
          } else {
            // Case 3.2: overlaps with the 1st segment, merge them
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

//-----------------------------------------------------------------------------
//      Functions for building the ghost (transparency) table
//-----------------------------------------------------------------------------

void init_ghost(void) {
  int n, m;
  byte *d = paleta;

  for (n = 0; n < 768; n++)
    dac4[n] = paleta[n] * 4;

  for (n = 0; n < 512; n++)
    vcubos[n] = NULL;

  for (n = 0; n < 256; n++) {
    tpuntos[n].r = *d++ * 4;
    tpuntos[n].g = *d++ * 4;
    tpuntos[n].b = *d++ * 4;
    m = (((int)tpuntos[n].r & 224) << 1) + (((int)tpuntos[n].g & 224) >> 2) +
        ((int)tpuntos[n].b >> 5);

    if (vcubos[m] == NULL) {
      vcubos[m] = &tpuntos[n];
      tpuntos[n].next = NULL;
    } else {
      tpuntos[n].next = vcubos[m];
      vcubos[m] = &tpuntos[n];
    }
  }
}

//-----------------------------------------------------------------------------
//      Function for building the ghost table
//-----------------------------------------------------------------------------

int rr, gg, bb;
int num_puntos;

/* Build the 256x256 ghost (transparency) lookup table. For every pair of
 * palette indices (n, m), ghost[n*256+m] = the palette index closest to
 * the average of colors n and m, producing a 50% alpha-blend effect.
 * Uses a voxel-cube spatial index for fast nearest-color search.
 */
void create_ghost(void) {
  int n, m;
  int r3, g3, b3, vcubo;
  byte *ptr;

  n = 255;
  do {
    ptr = paleta + n * 3;
    _r = *ptr;
    _g = *(ptr + 1);
    _b = *(ptr + 2);
    ptr = paleta;
    m = 0;
    do {
      rr = ((int)(*ptr + _r) << 7) & 0x3f00;
      gg = ((int)(*(ptr + 1) + _g) << 7) & 0x3f00;
      bb = ((int)(*(ptr + 2) + _b) << 7) & 0x3f00;
      ptr += 3;

      r3 = (rr & 0x3800) >> 5;
      g3 = (gg & 0x3800) >> 8;
      b3 = (bb & 0x3800) >> 11;
      vcubo = r3 + g3 + b3;

      find_min = 65536;
      num_puntos = 0;

      // Distance cubes sqr(0) ------------------------------------------------

      create_ghost_vc(vcubo);

      if (num_puntos > 1)
        goto fast_ghost;

      // Distance cubes sqr(1) ------------------------------------------------

      if (r3 > 0)
        create_ghost_vc(vcubo - 64);
      if (r3 < 7 * 64)
        create_ghost_vc(vcubo + 64);
      if (g3 > 0)
        create_ghost_vc(vcubo - 8);
      if (g3 < 7 * 8)
        create_ghost_vc(vcubo + 8);
      if (b3 > 0)
        create_ghost_vc(vcubo - 1);
      if (b3 < 7)
        create_ghost_vc(vcubo + 1);

      if (num_puntos > 2)
        goto fast_ghost;

      // Distance cubes sqr(2) ------------------------------------------------

      if (r3 > 0) {
        if (g3 > 0)
          create_ghost_vc(vcubo - 64 - 8);
        else {
          if (g3 < 7 * 8)
            create_ghost_vc(vcubo - 64 + 8);
        }
        if (b3 > 0)
          create_ghost_vc(vcubo - 64 - 1);
        else {
          if (b3 < 7)
            create_ghost_vc(vcubo - 64 + 1);
        }
      } else if (r3 < 7 * 64) {
        if (g3 > 0)
          create_ghost_vc(vcubo + 64 - 8);
        else {
          if (g3 < 7 * 8)
            create_ghost_vc(vcubo + 64 + 8);
        }
        if (b3 > 0)
          create_ghost_vc(vcubo + 64 - 1);
        else {
          if (b3 < 7)
            create_ghost_vc(vcubo + 64 + 1);
        }
      }
      if (g3 > 0) {
        if (b3 > 0)
          create_ghost_vc(vcubo - 8 - 1);
        else {
          if (b3 < 7)
            create_ghost_vc(vcubo - 8 + 1);
        }
      } else if (g3 < 7 * 8) {
        if (b3 > 0)
          create_ghost_vc(vcubo + 8 - 1);
        else {
          if (b3 < 7)
            create_ghost_vc(vcubo + 8 + 1);
        }
      }

      if (find_min == 65536)
        create_ghost_slow();

fast_ghost:
      *(ghost + n * 256 + m) = find_col;
      *(ghost + m * 256 + n) = find_col;


    } while (++m < n);
  } while (--n);

  do {
    *(ghost + n * 256 + n) = n;
  } while (++n < 256);

#ifdef DIV2
  memcpy(ghost_inicial, ghost, 256);

  n = 0;
  ptr = ghost;
  do {
    *ptr++ = n++;
  } while (n < 256);
#endif
}

void create_ghost_vc(int m) {
  int dif;
  struct t_tpuntos *p;

  if ((p = vcubos[m]) != NULL)
    do {
      num_puntos++;
      dif = *(int *)(color_lookup + rr + (*p).r);
      dif += *(int *)(color_lookup + gg + (*p).g);
      dif += *(int *)(color_lookup + bb + (*p).b);
      if (dif < find_min) {
        find_min = dif;
        find_col = ((byte *)p - (byte *)tpuntos) / sizeof(struct t_tpuntos);
      }
    } while ((p = (*p).next) != NULL);
}

void create_ghost_slow(void) {
  int dmin, dif;
  byte *pal, *endpal, *color = NULL;

  pal = dac4;
  endpal = dac4 + 768;
  dmin = 65536;
  do {
    dif = *(int *)(color_lookup + rr + *pal);
    pal++;
    dif += *(int *)(color_lookup + gg + *pal);
    pal++;
    dif += *(int *)(color_lookup + bb + *pal);
    pal += 4;
    if (dif < dmin) {
      dmin = dif;
      color = pal - 6;
    }
  } while (pal < endpal);
  find_col = (color - dac4) / 3;
}

void find_color(byte r, byte g, byte b) { // Find a color (excluding index 0)

  int dmin, dif;
  byte *pal, *endpal, *color = NULL;

  pal = paleta + 3;
  endpal = paleta + 768;
  dmin = 65536;
  do {
    if (((pal - paleta) / 3) == last_c1)
      pal += 3;
    dif = (int)(r - *pal) * (int)(r - *pal);
    pal++;
    dif += (int)(g - *pal) * (int)(g - *pal);
    pal++;
    dif += (int)(b - *pal) * (int)(b - *pal);
    pal++;
    if (dif < dmin) {
      dmin = dif;
      color = pal - 3;
    }
  } while (pal < endpal);
  find_col = (color - paleta) / 3;
}

byte average_color(byte a, byte b) {
  find_color((paleta[a * 3] + paleta[b * 3]) / 2, (paleta[a * 3 + 1] + paleta[b * 3 + 1]) / 2,
             (paleta[a * 3 + 2] + paleta[b * 3 + 2]) / 2);
  return (find_col);
}

//----------------------------------------------------------------------------
