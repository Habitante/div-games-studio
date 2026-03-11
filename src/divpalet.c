
//-----------------------------------------------------------------------------
//      Palette (DAC) operations module
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#include "fpgfile.hpp"

// File-scope palette temps (formerly globals, used as implicit params between palette functions)
static int r, g, b;

void create_ghost_vc(int m);
void create_ghost_slow(void);
void merge_palettes(void);
void rescalar(byte *si, int sian, int sial, byte *di, int dian, int dial);


// Prototypes for palette loading functions from DIVFORMA.CPP

extern int exp_Color0, exp_Color1, exp_Color2;

int fmt_load_dac_map(char *name);
int fmt_load_dac_pcx(char *name);
int fmt_load_dac_bmp(char *name);
int fmt_load_dac_jpg(char *name);
int fmt_load_dac_fnt(char *name);
int fmt_load_dac_fpg(char *name);
int fmt_load_dac_pal(char *name);

//-----------------------------------------------------------------------------
//      Module global variables
//-----------------------------------------------------------------------------

struct t_tpoints { // For ghost table creation
  int r, g, b;
  struct t_tpoints *next;
} tpoints[256];

struct t_tpoints *vcubos[512]; // For ghost table creation

byte _r, _g, _b, find_col; // Palette calculations

int find_min; // Nearest color calculation

byte paleta[768];

int num_points;

//-----------------------------------------------------------------------------
//      Find the environment colors c0..c4 for main_loop
//-----------------------------------------------------------------------------

void find_colors(void) {
  int x;
  char pal[768];
  FILE *f;

  create_dac4();

  c0 = find_color(0, 0, 0);

  c2 = find_color(colors_rgb[0], colors_rgb[1], colors_rgb[2]);
  c4 = find_color(colors_rgb[3], colors_rgb[4], colors_rgb[5]);
  c_b_low = find_color(colors_rgb[6], colors_rgb[7], colors_rgb[8]);
  ce1 = find_color(colors_rgb[9], colors_rgb[10], colors_rgb[11]);
  ce4 = find_color(colors_rgb[12], colors_rgb[13], colors_rgb[14]);
  c_y = find_color(colors_rgb[15], colors_rgb[16], colors_rgb[17]);

  c_com = find_color(colors_rgb[18], colors_rgb[19], colors_rgb[20]);
  c_sim = find_color(colors_rgb[21], colors_rgb[22], colors_rgb[23]);
  c_res = find_color(colors_rgb[24], colors_rgb[25], colors_rgb[26]);
  c_pre = find_color(colors_rgb[27], colors_rgb[28], colors_rgb[29]);
  c_num = find_color(colors_rgb[30], colors_rgb[31], colors_rgb[32]);
  c_lit = find_color(colors_rgb[33], colors_rgb[34], colors_rgb[35]);

  c1 = average_color(c0, c2);
  c3 = average_color(c2, c4);
  ce01 = average_color(c0, ce1);
  ce2 = average_color(ce1, ce4);
  c01 = average_color(c0, c1);
  c12 = average_color(c1, c2);
  c23 = average_color(c2, c3);
  c34 = average_color(c3, c4);
  c_r = find_color(63, 0, 0);
  c_g = find_color(0, 63, 0);
  c_b = find_color(0, 0, 63);
  c_r_low = find_color(32, 0, 0);
  c_g_low = find_color(0, 32, 0);
  color_c0 = c0;

  memset(pal, 0, 768);

  if ((f = fopen("help/help.fig", "rb")) != NULL) {
    fseek(f, 8, SEEK_SET);
    fread(pal, 1, 768, f);
    fclose(f);
    help_xlat[0] = 0;

    for (x = 1; x < 256; x++) {
      help_xlat[x] = find_color_not0(pal[x * 3], pal[x * 3 + 1], pal[x * 3 + 2]);
    }
  }
}

//-----------------------------------------------------------------------------
//      Ghost table creation functions
//-----------------------------------------------------------------------------

void init_ghost(void) {
  int n, m;
  byte *d = dac;

  create_dac4();

  for (n = 0; n < 512; n++) {
    vcubos[n] = NULL;
  }

  for (n = 0; n < 256; n++) {
    tpoints[n].r = *d++ * 4;
    tpoints[n].g = *d++ * 4;
    tpoints[n].b = *d++ * 4;
    m = (((int)tpoints[n].r & 224) << 1) + (((int)tpoints[n].g & 224) >> 2) +
        ((int)tpoints[n].b >> 5);

    if (vcubos[m] == NULL) {
      vcubos[m] = &tpoints[n];
      tpoints[n].next = NULL;
    } else {
      tpoints[n].next = vcubos[m];
      vcubos[m] = &tpoints[n];
    }
  }
}

//-----------------------------------------------------------------------------
//      Fast color lookup function
//      WARNING: create_dac4() must be called before this function
//-----------------------------------------------------------------------------

byte fast_find_color(byte fr, byte fg, byte fb) {
  int r3, g3, b3, vcubo;

  r = fr << 8;
  g = fg << 8;
  b = fb << 8;
  r3 = (r & 0x3800) >> 5;
  g3 = (g & 0x3800) >> 8;
  b3 = (b & 0x3800) >> 11;
  vcubo = r3 + g3 + b3;
  find_min = 65536;
  num_points = 0;

  // Distance cubes sqr(0) --------------------------------------------

  create_ghost_vc(vcubo);

  if (num_points > 1)
    goto fast_find;

  // Distance cubes sqr(1) --------------------------------------------

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

  if (num_points > 2)
    goto fast_find;

  // Distance cubes sqr(2) --------------------------------------------

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

fast_find:
  return (find_col);
}

//-----------------------------------------------------------------------------
//      Ghost table creation function
//-----------------------------------------------------------------------------

void create_ghost(int points) {
  int n, m, punto = 0;
  int r3, g3, b3, vcubo;
  byte *ptr;

  n = 255;
  do {
    ptr = dac + n * 3;
    _r = *ptr;
    _g = *(ptr + 1);
    _b = *(ptr + 2);
    ptr = dac;
    m = 0;
    do {
      r = ((int)(*ptr + _r) << 7) & 0x3f00;
      g = ((int)(*(ptr + 1) + _g) << 7) & 0x3f00;
      b = ((int)(*(ptr + 2) + _b) << 7) & 0x3f00;
      ptr += 3;

      r3 = (r & 0x3800) >> 5;
      g3 = (g & 0x3800) >> 8;
      b3 = (b & 0x3800) >> 11;
      vcubo = r3 + g3 + b3;

      find_min = 65536;
      num_points = 0;

      // Distance cubes sqr(0) --------------------------------------------

      create_ghost_vc(vcubo);

      if (num_points > 1)
        goto fast_ghost;

      // Distance cubes sqr(1) --------------------------------------------

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

      if (num_points > 2)
        goto fast_ghost;

      // Distance cubes sqr(2) --------------------------------------------

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

      if (points) {
        if ((punto++ & 2047) == 0) {
          cprintf(".");
        }
      }
    } while (++m < n);
  } while (--n);

  do {
    *(ghost + n * 256 + n) = n;
  } while (++n < 256);
  if (points) {
    cprintf(".\r\n");
  }
}

void create_ghost_vc(int m) {
  int dif;
  struct t_tpoints *p;

  if ((p = vcubos[m]) != NULL)
    do {
      num_points++;
      dif = *(int *)(color_lookup + r + (*p).r);
      dif += *(int *)(color_lookup + g + (*p).g);
      dif += *(int *)(color_lookup + b + (*p).b);
      if (dif < find_min) {
        find_min = dif;
        find_col = ((byte *)p - (byte *)tpoints) / sizeof(struct t_tpoints);
      }
    } while ((p = (*p).next) != NULL);
}

void create_ghost_slow(void) {
  int dmin, dif;
  byte *pal, *endpal, *color;

  color = dac4;
  pal = dac4;
  endpal = dac4 + 768;
  dmin = 65536;
  do {
    dif = *(int *)(color_lookup + r + *pal);
    pal++;
    dif += *(int *)(color_lookup + g + *pal);
    pal++;
    dif += *(int *)(color_lookup + b + *pal);
    pal += 4;
    if (dif < dmin) {
      dmin = dif;
      color = pal - 6;
    }
  } while (pal < endpal);
  find_col = (color - dac4) / 3;
}

//-----------------------------------------------------------------------------
//      Generate dac4[] from dac[]
//-----------------------------------------------------------------------------

void create_dac4(void) {
  int a;
  for (a = 0; a < 768; a++)
    dac4[a] = dac[a] * 4;
}

//-----------------------------------------------------------------------------
//      Find a color (r,g,b) in the palette (full search)
//      WARNING: create_dac4() must be called before this function
//-----------------------------------------------------------------------------

byte find_color(byte r, byte g, byte b) {
  int dmin, dif, _r, _g, _b;
  byte *pal, *endpal, *color;

  color = dac4;
  pal = dac4;
  endpal = dac4 + 768;
  dmin = 65536;
  _r = (int)r * 256;
  _g = (int)g * 256;
  _b = (int)b * 256;
  do {
    dif = *(int *)(color_lookup + _r + *pal);
    pal++;
    dif += *(int *)(color_lookup + _g + *pal);
    pal++;
    dif += *(int *)(color_lookup + _b + *pal);
    pal++;
    if (dif < dmin) {
      dmin = dif;
      color = pal - 3;
    }
  } while (pal < endpal);

  return ((color - dac4) / 3);
}

byte find_color_not0(byte r, byte g, byte b) {
  int dmin, dif, _r, _g, _b;
  byte *pal, *endpal, *color;

  color = dac4;
  pal = dac4 + 3;
  endpal = dac4 + 768;
  dmin = 65536;
  _r = (int)r * 256;
  _g = (int)g * 256;
  _b = (int)b * 256;
  do {
    dif = *(int *)(color_lookup + _r + *pal);
    pal++;
    dif += *(int *)(color_lookup + _g + *pal);
    pal++;
    dif += *(int *)(color_lookup + _b + *pal);
    pal++;
    if (dif < dmin) {
      dmin = dif;
      color = pal - 3;
    }
  } while (pal < endpal);

  return ((color - dac4) / 3);
}

//-----------------------------------------------------------------------------
//      Palette sorting function
//-----------------------------------------------------------------------------

void ord_paleta0(void) {
  int n;
  create_dac4();
  for (n = 0; n < 256; n++)
    paleta[n] = find_color(original_palette[n * 3], original_palette[n * 3 + 1],
                           original_palette[n * 3 + 2]);
}

void ord_paleta1(void) {
  byte *pal, c;
  int n;

  pal = (byte *)malloc(1024);
  if (pal == NULL)
    error(0);

  n = 0;
  do {
    pal[n * 4 + 3] = (pal[n * 4] = dac[n * 3]) + (pal[n * 4 + 1] = dac[n * 3 + 1]) +
                     (pal[n * 4 + 2] = dac[n * 3 + 2]);
  } while (++n < 256);

  r = pal[c0 * 4];
  g = pal[c0 * 4 + 1];
  b = pal[c0 * 4 + 2];
  pal[c0 * 4] = 255;
  paleta[0] = c0;

  n = 1;
  do {
    c = find_ord(pal);
    r = pal[c * 4];
    g = pal[c * 4 + 1];
    b = pal[c * 4 + 2];
    pal[c * 4] = 255;
    paleta[n] = c;
  } while (++n < 256);

  free(pal);
}

void ord_paleta2(void) {
  byte *pal, c;
  int n;

  pal = (byte *)malloc(1024);
  if (pal == NULL)
    error(0);

  n = 0;
  do {
    pal[n * 4 + 3] = (pal[n * 4] = dac[n * 3]) + (pal[n * 4 + 1] = dac[n * 3 + 1]) +
                     (pal[n * 4 + 2] = dac[n * 3 + 2]);
  } while (++n < 256);

  r = pal[c0 * 4];
  g = pal[c0 * 4 + 1];
  b = pal[c0 * 4 + 2];
  pal[c0 * 4] = 255;
  paleta[0] = c0;
  _r = r;
  _g = g;
  _b = b;

  n = 1;
  do {
    c = find_ord(pal);
    r = (int)2 * pal[c * 4] - _r;
    g = (int)2 * pal[c * 4 + 1] - _g;
    b = (int)2 * pal[c * 4 + 2] - _b;
    _r = pal[c * 4];
    _g = pal[c * 4 + 1];
    _b = pal[c * 4 + 2];
    pal[c * 4] = 255;
    paleta[n] = c;
  } while (++n < 256);

  free(pal);
}

void ord_paleta3(void) {
  byte *pal, c;
  int n;

  pal = (byte *)malloc(1024);
  if (pal == NULL)
    error(0);

  n = 0;
  do {
    pal[n * 4 + 3] = (pal[n * 4] = dac[n * 3]) + (pal[n * 4 + 1] = dac[n * 3 + 1]) +
                     (pal[n * 4 + 2] = dac[n * 3 + 2]);
  } while (++n < 256);

  r = pal[c0 * 4];
  g = pal[c0 * 4 + 1];
  b = pal[c0 * 4 + 2];
  pal[c0 * 4] = 255;
  paleta[0] = c0;

  n = 1;
  do {
    c = find_ord(pal);
    if (!(n & 15)) {
      r = 0;
      g = 0;
      b = 0;
      c = find_ord(pal);
    }
    r = pal[c * 4];
    g = pal[c * 4 + 1];
    b = pal[c * 4 + 2];
    pal[c * 4] = 255;
    paleta[n] = c;
  } while (++n < 256);

  free(pal);
}

byte find_ord(byte *dac) {
  int dmin, dif, r2, g2, b2;
  byte *pal, *endpal, *color;

  color = dac;
  pal = dac;
  endpal = dac + 1024;
  dmin = 65536;
  if (r < 0)
    r = 0;
  else if (r > 63)
    r = 63;
  if (g < 0)
    g = 0;
  else if (g > 63)
    g = 63;
  if (b < 0)
    b = 0;
  else if (b > 63)
    b = 63;
  r2 = (int)r * 256;
  g2 = (int)g * 256;
  b2 = (int)b * 256;
  do
    if (*pal != 255) {
      dif = *(int *)(color_lookup + r2 + *pal * 4);
      pal++;
      dif += *(int *)(color_lookup + g2 + *pal * 4);
      pal++;
      dif += *(int *)(color_lookup + b2 + *pal * 4);
      pal += 2;
      if (dif < dmin) {
        dmin = dif;
        color = pal - 4;
      }
    } else {
      pal += 4;
    }
  while (pal < endpal);

  return ((color - dac) / 4);
}

//-----------------------------------------------------------------------------
//      Average of two colors (exact calculation)
//-----------------------------------------------------------------------------

byte average_color(byte a, byte b) {
  create_dac4();
  return (find_color((dac[a * 3] + dac[b * 3]) / 2, (dac[a * 3 + 1] + dac[b * 3 + 1]) / 2,
                     (dac[a * 3 + 2] + dac[b * 3 + 2]) / 2));
}

//-----------------------------------------------------------------------------
//      Calculate which gradient colors are nearest to each RGB
//-----------------------------------------------------------------------------

void make_nearest_gradient(void) {
  int n, i, min_dist, dist, c;

  for (n = 0; n < 190; n++) {
    min_dist = 192;
    for (i = 0; i <= gradients[gradient].num_colors; i++) {
      c = (int)gradients[gradient].colors[i] * 3;
      dist = dac[c] + dac[c + 1] + dac[c + 2];
      if (dist > n)
        dist = dist - n;
      else
        dist = n - dist;
      if (dist < min_dist) {
        nearest_gradient[n] = c / 3;
        min_dist = dist;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//      Calculate the intermediate colors of a color gradient
//-----------------------------------------------------------------------------

void calculate_gradient(int n) {
  int a;

  if (!gradients[n].fixed) { // Fixed gradients cannot be recalculated
    switch (gradients[n].type) {
    case 0: // Linear, straightforward: colors taken in sequence
      for (a = 1; a < 32; a++)
        gradients[n].colors[a + 1] = gradients[n].colors[a] + 1;
      break;
    case 1: // Adaptive every 1 color, nothing to calculate
      break;
    case 2: // Adaptive every 2 colors, fill with averages
      for (a = 0; a < 32; a += 2)
        gradients[n].colors[a + 1] =
            average_color(gradients[n].colors[a], gradients[n].colors[a + 2]);
      break;
    case 4: // Adaptive every 4
      for (a = 0; a < 32; a += 4) {
        gradients[n].colors[a + 2] =
            average_color(gradients[n].colors[a], gradients[n].colors[a + 4]);
        gradients[n].colors[a + 1] =
            average_color(gradients[n].colors[a], gradients[n].colors[a + 2]);
        gradients[n].colors[a + 3] =
            average_color(gradients[n].colors[a + 2], gradients[n].colors[a + 4]);
      }
      break;
    case 8: // Adaptive every 8
      for (a = 0; a < 32; a += 8) {
        gradients[n].colors[a + 4] =
            average_color(gradients[n].colors[a], gradients[n].colors[a + 8]);
        gradients[n].colors[a + 2] =
            average_color(gradients[n].colors[a], gradients[n].colors[a + 4]);
        gradients[n].colors[a + 6] =
            average_color(gradients[n].colors[a + 4], gradients[n].colors[a + 8]);
        gradients[n].colors[a + 1] =
            average_color(gradients[n].colors[a], gradients[n].colors[a + 2]);
        gradients[n].colors[a + 3] =
            average_color(gradients[n].colors[a + 2], gradients[n].colors[a + 4]);
        gradients[n].colors[a + 5] =
            average_color(gradients[n].colors[a + 4], gradients[n].colors[a + 6]);
        gradients[n].colors[a + 7] =
            average_color(gradients[n].colors[a + 6], gradients[n].colors[a + 8]);
      }
      break;
    }
  }
}

int has_maps(void) {
  int m, n = -1;
  for (m = 0; m < max_windows; m++)
    if (window[m].type == 100) {
      n = m;
      break;
    }
  return (n + 1);
}

char PalName[_MAX_PATH + 1] = "";

#define max_archivos 512 // ------------------------------- File listbox
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[max_archivos];
extern int num_taggeds;

extern byte *sample;
extern byte apply_palette[768];
extern int num_colores;
void create_palette(void);

void pal_load() {
  int div_try = 0;
  int num, n;
  FILE *f;

  v_mode = 0;
  v_type = 3;
  v_text = (char *)texts[777];
  show_dialog(browser0);
  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), tipo[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    if ((f = fopen(full, "rb")) != NULL) {
      fclose(f);
      v_exists = 1;
    } else
      v_exists = 0;
    div_strcpy(file_list_br.list, file_list_br.item_width, input);
    file_list_br.total_items = 1;
    thumb[0].tagged = 1;
    num_taggeds = 1;
  }

  if (num_taggeds == 1)
    for (num = 0; num < file_list_br.total_items; num++) {
      if (thumb[num].tagged) {
        div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
        div_strcpy(full, sizeof(full), tipo[v_type].path);
        if (full[strlen(full) - 1] != '/')
          div_strcat(full, sizeof(full), "/");
        div_strcat(full, sizeof(full), input);

        if (!v_exists) {
          v_text = (char *)texts[43];
          show_dialog(err0);
        } else {
          div_strcpy(PalName, sizeof(PalName), full);

          div_try |= fmt_load_dac_fpg(PalName);
          div_try |= fmt_load_dac_fnt(PalName);
          div_try |= fmt_load_dac_pcx(PalName);
          div_try |= fmt_load_dac_bmp(PalName);
          div_try |= fmt_load_dac_map(PalName);
          div_try |= fmt_load_dac_pal(PalName);
          div_try |= fmt_load_dac_jpg(PalName);

          if (div_try) {
            if (has_maps()) {
              v_title = (char *)texts[53];
              v_text = (char *)texts[321];
              show_dialog(accept0);
            } else
              v_accept = 1;
            if (v_accept)
              pal_refresh(0, 1);
            else
              pal_refresh(1, 1);
          } else {
            v_text = (char *)texts[46];
            show_dialog(err0);
            return;
          }
        }
        break;
      }
    }
  else {
    sample = (byte *)malloc(32768);
    if (sample == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    memset(sample, 0, 32768);

    for (num = 0; num < file_list_br.total_items; num++) {
      if (thumb[num].tagged) {
        div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
        div_strcpy(full, sizeof(full), tipo[v_type].path);
        if (full[strlen(full) - 1] != '/')
          div_strcat(full, sizeof(full), "/");
        div_strcat(full, sizeof(full), input);

        div_strcpy(PalName, sizeof(PalName), full);

        div_try |= fmt_load_dac_fpg(PalName);
        div_try |= fmt_load_dac_fnt(PalName);
        div_try |= fmt_load_dac_pcx(PalName);
        div_try |= fmt_load_dac_bmp(PalName);
        div_try |= fmt_load_dac_map(PalName);
        div_try |= fmt_load_dac_pal(PalName);
        div_try |= fmt_load_dac_jpg(PalName);

        if (div_try)
          for (n = 0; n < 256; n++) {
            sample[((dac4[n * 3 + 0] & 0xFE) << 9) | ((dac4[n * 3 + 1] & 0xFE) << 4) |
                    (dac4[n * 3 + 2] >> 1)] = 1;
          }
      }
    }

    create_palette();
    memcpy(&dac4[0], &apply_palette[0], 768);
    free(sample);

    if (has_maps()) {
      v_title = (char *)texts[53];
      v_text = (char *)texts[321];
      show_dialog(accept0);
    } else
      v_accept = 1;
    if (v_accept)
      pal_refresh(0, 1);
    else
      pal_refresh(1, 1);
  }
}

void pal_save() {
  int x;
  FILE *f;
  div_strcpy(full, sizeof(full), tipo[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);
  if ((f = fopen(full, "wb")) != NULL) {
    div_strcpy(PalName, sizeof(PalName), full);
    fwrite("pal\x1a\x0d\x0a\x00\x00", 8, 1, f);
    fwrite(dac, 768, 1, f);
    for (x = 0; x < 16; x++)
      fwrite(&gradients[x], 1, 36, f);
    fclose(f);
  } else {
    v_text = (char *)texts[47];
    show_dialog(err0);
  }
}


void pal_save_as() {
  v_mode = 1;
  v_type = 10;
  v_text = (char *)texts[778];
  show_dialog(browser0);
  if (v_finished) {
    if (v_exists) {
      v_title = (char *)texts[139];
      v_text = input;
      show_dialog(accept0);
      if (v_accept)
        pal_save();
    } else {
      pal_save();
    }
  }
}

//-----------------------------------------------------------------------------
//      Refresh the entire environment after a palette change
//-----------------------------------------------------------------------------
void pal_reload_font(int window, struct twindow *vntn);

extern int Text1Anc;
extern int Text1Alt;
extern int Text1Col;
extern char *Text1;
extern int Text2Anc;
extern int Text2Alt;
extern int Text2Col;
extern char *Text2;
extern int Text3Anc;
extern int Text3Alt;
extern int Text3Col;
extern char *Text3;
extern char *Text01;
extern char *Text02;
extern char *Text03;

void pal_show_text(void);
void create_title_bar(void);

byte *t64 = NULL;

void pal_refresh(int no_tocar_mapas, int guardar_original) {
  byte *ptr, *ptrend;
  int w, h, x, sum;
  int n, m;
  FPG *MiFPG;
  byte pal[768];
  byte xlat[768];
  int tal = 24 * big2, tan = 41 * big2;

  // Check if the new palette is different

  x = 0;
  sum = 0;
  do {
    sum += abs((int)dac[x] - (int)dac4[x]);
  } while (++x < 768);
  if (!sum) {
    x = 0;
    do
      dac4[x] = dac[x] * 4;
    while (++x < 768);
    return;
  }

  // ************** Free FPG thumbnails

  for (m = 0; m < max_windows; m++)
    if (window[m].type == 101) {
      MiFPG = (FPG *)window[m].aux;

      // Free FPG thumbnails
      for (n = 0; n < 1000; n++) {
        if (MiFPG->thumb[n].ptr != NULL)
          free(MiFPG->thumb[n].ptr);
        MiFPG->thumb[n].ptr = NULL;
        MiFPG->thumb[n].status = 0;
      }
    }

  memcpy(pal, dac, 768);
  memcpy(dac, dac4, 768);

  // *** Remap existing graphics to the new palette

  create_dac4();
  xlat[0] = 0;
  for (x = 1; x < 256; x++)
    xlat[x] = find_color_not0(pal[x * 3], pal[x * 3 + 1], pal[x * 3 + 2]);

  if (!no_tocar_mapas) {
    for (n = 1; n < max_windows; n++) {
      if (window[n].type == 100) {
        ptr = window[n].mapa->map;
        sum = window[n].mapa->map_width * window[n].mapa->map_height;
        x = 0;
        do {
          *ptr = xlat[*ptr];
          ptr++;
        } while (++x < sum);
      }
    }
    ptr = undo;
    ptrend = ptr + undo_memory;
    do {
      *ptr = xlat[*ptr];
      ptr++;
    } while (ptr < ptrend);
  }

  for (n = 0; n < 256; n++)
    if (graf_help[n].ptr) {
      for (x = graf_help[n].ran * graf_help[n].ral - 1; x >= 0; x--) {
        graf_help[n].ptr[x] = xlat[graf_help[n].ptr[x]];
      }
    }

  for (n = 0; n < max_windows; n++) {
    if (window[n].type == 102 && !strcmp((char *)window[n].name, (char *)texts[83])) {
      Text1Col = xlat[Text1Col];
      Text2Col = xlat[Text2Col];
      Text3Col = xlat[Text3Col];
      if (Text01 == NULL) {
        memset(Text1, Text1Col, tan * tal + tan);
      } else {
        ptr = (byte *)Text01;
        while (ptr < (byte *)Text01 + Text1Anc * Text1Alt) {
          *ptr = xlat[*ptr];
          ptr++;
        }
        ptr = (byte *)Text1;
        while (ptr < (byte *)Text1 + tan * tal) {
          *ptr = xlat[*ptr];
          ptr++;
        }
      }
      if (Text02 == NULL) {
        memset(Text2, Text2Col, tan * tal + tan);
      } else {
        ptr = (byte *)Text02;
        while (ptr < (byte *)Text02 + Text2Anc * Text2Alt) {
          *ptr = xlat[*ptr];
          ptr++;
        }
        ptr = (byte *)Text2;
        while (ptr < (byte *)Text2 + tan * tal) {
          *ptr = xlat[*ptr];
          ptr++;
        }
      }
      if (Text03 == NULL) {
        memset(Text3, Text3Col, tan * tal + tan);
      } else {
        ptr = (byte *)Text03;
        while (ptr < (byte *)Text03 + Text3Anc * Text3Alt) {
          *ptr = xlat[*ptr];
          ptr++;
        }
        ptr = (byte *)Text3;
        while (ptr < (byte *)Text3 + tan * tal) {
          *ptr = xlat[*ptr];
          ptr++;
        }
      }
    }
  }

  x = 0;
  mouse_graf = 3;
  flush_copy();
  mouse_graf = 1;

  init_ghost();
  create_ghost(0);
  find_colors();
  zoom_move = c3;
  color = 0;

  create_title_bar();

  for (n = 1; n < max_windows; n++) {
    if (window[n].type == 104)
      pal_reload_font(n, (struct twindow *)&window[n].type);
  }

  for (n = 0; n < max_windows; n++)
    if (window[n].type) {
      wup(n);
      ptr = v.ptr;
      if (window[n].foreground == 2) {
        swap(v.w, v._an);
        swap(v.h, v._al);
      }
      w = v.w;
      h = v.h;
      memset(ptr, c0, w * h);
      if (big) {
        w /= 2;
        h /= 2;
      }
      wrectangle(ptr, w, h, c2, 0, 0, w, h);
      wput(ptr, w, h, w - 9, 2, 35);
      wput(ptr, w, h, w - 17, 2, 37);

      if (v.type >= 100 && !v.state) {
        wgra(v.ptr, w, h, c1, 2, 2, w - 20, 7);
        if (text_len(v.title) + 3 > w - 20) {
          wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c0);
          wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c2);
        } else {
          wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 3, 1, v.title, c0);
          wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c2);
        }
      } else {
        wgra(ptr, w, h, c_b_low, 2, 2, w - 20, 7);
        if (text_len(v.title) + 3 > w - 20) {
          wwrite_in_box(ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
        }
      }
      call((void_return_type_t)v.paint_handler);
      if (v.foreground == 2) {
        swap(v.w, v._an);
        swap(v.h, v._al);
      }
      wdown(n);
    }

  for (n = 4; n < 64; n += 4) { // fade_off
    for (x = 0; x < 768; x++) {
      if (pal[x] > n) {
        dac4[x] = pal[x] - n;
      } else {
        dac4[x] = 0;
      }
    }
    set_dac(dac4);
  }

  create_dac4();

  // Adapt user-defined color gradients to the new palette

  create_gradient_colors(Setupfile.gradient_config, wallpaper_gradient);

  // Restore the background wallpaper
  prepare_wallpaper();

  update_box(0, 0, vga_width, vga_height);

  blit_screen(screen_buffer);

  for (n = 60; n >= 0; n -= 4) {
    for (x = 0; x < 768; x++)
      if (dac[x] > n)
        dac4[x] = dac[x] - n;
      else
        dac4[x] = 0;
    set_dac(dac4);
  }

  for (n = 0; n < 768; n++)
    dac4[n] = dac[n] * 4;

  if (guardar_original)
    memcpy(original_palette, dac, 768);

  for (n = 1; n < max_windows; n++) {
    if (window[n].type == 101) {
      if (remap_all_files(n))
        n--;
    }
  }

  exp_Color0 = -1;
  exp_Color1 = -1;
  exp_Color2 = -1;

  if (t64 != NULL) {
    free(t64);
    t64 = NULL;
  }
}

//-----------------------------------------------------------------------------
//      Sort palette
//-----------------------------------------------------------------------------

int ordenacion = 0;

void ordena1(void) {
  int x, y;
  int w = v.w / big2, h = v.h / big2;

  _show_items();

  wbox(v.ptr, w, h, c0, 3, 11, 65 * 2 + 1, 65 * 2 + 1);

  wbox(v.ptr, w, h, c2, 2 + 66, 10, 1, 65 * 2 + 3);
  wbox(v.ptr, w, h, c2, 2, 10 + 66, 65 * 2 + 3, 1);

  ord_paleta0();
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, paleta[x + y * 16], 4 + x * 4, 12 + y * 4, 3, 3);

  ord_paleta1();
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, paleta[x + y * 16], 66 + 4 + x * 4, 12 + y * 4, 3, 3);

  ord_paleta2();
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, paleta[x + y * 16], 4 + x * 4, 66 + 12 + y * 4, 3, 3);

  ord_paleta3();
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, paleta[x + y * 16], 66 + 4 + x * 4, 66 + 12 + y * 4, 3, 3);

  switch (ordenacion) {
  case 0:
    ord_paleta0();
    wrectangle(v.ptr, w, h, c4, 2, 10, 67, 67);
    break;
  case 1:
    ord_paleta1();
    wrectangle(v.ptr, w, h, c4, 2 + 66, 10, 67, 67);
    break;
  case 2:
    ord_paleta2();
    wrectangle(v.ptr, w, h, c4, 2, 10 + 66, 67, 67);
    break;
  case 3:
    ord_paleta3();
    wrectangle(v.ptr, w, h, c4, 2 + 66, 10 + 66, 67, 67);
    break;
  }
}

void ordena2(void) {
  int ord;
  int w = v.w / big2, h = v.h / big2;

  _process_items();

  if ((mouse_b & 1) && wmouse_in(2, 10, 132, 132)) {
    if (wmouse_x < 2 + 66)
      ord = 0;
    else
      ord = 1;
    if (wmouse_y >= 10 + 66)
      ord += 2;

    if (ord != ordenacion) {
      v.redraw = 1;
      wrectangle(v.ptr, w, h, c2, 2, 10, 65 * 2 + 3, 65 * 2 + 3);
      wbox(v.ptr, w, h, c2, 2 + 66, 10, 1, 65 * 2 + 3);
      wbox(v.ptr, w, h, c2, 2, 10 + 66, 65 * 2 + 3, 1);
      switch (ord) {
      case 0:
        wrectangle(v.ptr, w, h, c4, 2, 10, 67, 67);
        ord_paleta0();
        break;
      case 1:
        wrectangle(v.ptr, w, h, c4, 2 + 66, 10, 67, 67);
        ord_paleta1();
        break;
      case 2:
        wrectangle(v.ptr, w, h, c4, 2, 10 + 66, 67, 67);
        ord_paleta2();
        break;
      case 3:
        wrectangle(v.ptr, w, h, c4, 2 + 66, 10 + 66, 67, 67);
        ord_paleta3();
        break;
      }
      ordenacion = ord;
    }
  }

  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  }
}

void ordena0(void) {
  v.type = 1; // Dialog
  v.state = 0;
  v.w = 65 * 2 + 7;
  v.h = 65 * 2 + 31;
  v.title = texts[140];

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);

  v.paint_handler = ordena1;
  v.click_handler = ordena2;
  v_accept = 0;
}

void sort_palette(void) {
  int n;

  show_dialog(ordena0);
  if (v_accept) {
    for (n = 0; n < 256; n++) {
      dac4[n * 3] = dac[paleta[n] * 3];
      dac4[n * 3 + 1] = dac[paleta[n] * 3 + 1];
      dac4[n * 3 + 2] = dac[paleta[n] * 3 + 2];
    }
    pal_refresh(0, 0);
  }
}

//-----------------------------------------------------------------------------
//      Merge two different palettes
//-----------------------------------------------------------------------------

// byte paleta[768]

word find_ord2(byte *dac) {
  int dmin, dif, r2, g2, b2;
  byte *pal, *endpal, *color;

  color = dac;
  pal = dac;
  endpal = dac + 2048;
  dmin = 65536;
  if (r < 0)
    r = 0;
  else if (r > 63)
    r = 63;
  if (g < 0)
    g = 0;
  else if (g > 63)
    g = 63;
  if (b < 0)
    b = 0;
  else if (b > 63)
    b = 63;
  r2 = (int)r * 256;
  g2 = (int)g * 256;
  b2 = (int)b * 256;
  do
    if (*pal != 255) {
      dif = *(int *)(color_lookup + r2 + *pal * 4);
      pal++;
      dif += *(int *)(color_lookup + g2 + *pal * 4);
      pal++;
      dif += *(int *)(color_lookup + b2 + *pal * 4);
      pal += 2;
      if (dif < dmin) {
        dmin = dif;
        color = pal - 4;
      }
    } else {
      pal += 4;
    }
  while (pal < endpal);

  return ((color - dac) / 4);
}

void merge_palette(void) {
  int div_try = 0;

  v_mode = 0;
  v_type = 3;
  v_text = (char *)texts[781];
  show_dialog(browser0);

  if (v_finished) {
    if (!v_exists) {
      v_text = (char *)texts[43];
      show_dialog(err0);
    } else {
      div_strcpy(full, sizeof(full), tipo[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);
      div_strcpy(PalName, sizeof(PalName), full);

      div_try |= fmt_load_dac_fpg(PalName);
      div_try |= fmt_load_dac_fnt(PalName);
      div_try |= fmt_load_dac_pcx(PalName);
      div_try |= fmt_load_dac_bmp(PalName);
      div_try |= fmt_load_dac_map(PalName);
      div_try |= fmt_load_dac_pal(PalName);
      div_try |= fmt_load_dac_jpg(PalName);

      if (!div_try) {
        v_text = (char *)texts[46];
        show_dialog(err0);
        return;
      }

      mouse_graf = 3;
      blit_screen(screen_buffer);

      merge_palettes();

      pal_refresh(0, 1);
    }
  }
  create_dac4();
}

// Takes two palettes (one in dac, one in dac4) and returns the merged result in dac4

void merge_palettes(void) {
  byte pal[2048]; // Two palettes in R,G,B,sum(RGB) format
  word paleta[512];
  int dist[512];
  int n, c, min, cmin = 0;

  // Old palette in dac, new palette in dac4

  n = 0;
  do {
    pal[n * 4 + 3] = (pal[n * 4] = dac[n * 3]) + (pal[n * 4 + 1] = dac[n * 3 + 1]) +
                     (pal[n * 4 + 2] = dac[n * 3 + 2]);
  } while (++n < 256);

  n = 0;
  do {
    pal[1024 + n * 4 + 3] = (pal[1024 + n * 4] = dac4[n * 3]) +
                            (pal[1024 + n * 4 + 1] = dac4[n * 3 + 1]) +
                            (pal[1024 + n * 4 + 2] = dac4[n * 3 + 2]);
  } while (++n < 256);

  // pal[2048] prepared with { R,G,B,SUM }

  r = 0;
  g = 0;
  b = 0;
  pal[c0 * 4] = 255;
  paleta[0] = c0;
  n = 1;
  do {
    c = find_ord2(pal);
    r = pal[c * 4];
    g = pal[c * 4 + 1];
    b = pal[c * 4 + 2];
    pal[c * 4] = 255;
    paleta[n] = c;
  } while (++n < 512);

  // Restore pal

  for (n = 0; n < 512; n++) {
    pal[n * 4] = pal[n * 4 + 3] - (pal[n * 4 + 1] + pal[n * 4 + 2]);
  }

  // paleta[512] with colors sorted

  for (n = 1; n < 511; n++) {
    dist[n] = *(int *)(color_lookup + pal[paleta[n] * 4] * 256 + pal[paleta[n + 1] * 4] * 4);
    dist[n] +=
        *(int *)(color_lookup + pal[paleta[n] * 4 + 1] * 256 + pal[paleta[n + 1] * 4 + 1] * 4);
    dist[n] +=
        *(int *)(color_lookup + pal[paleta[n] * 4 + 2] * 256 + pal[paleta[n + 1] * 4 + 2] * 4);
  }

  // dist[] with distances between all consecutive colors

  c = 511;
  while (c > 255) {
    min = 64 * 64 * 64;
    for (n = 1; n < c; n++) { // Find the minimum distance
      if (dist[n] < min) {
        cmin = n;
        min = dist[n];
      }
    }

    // paleta[cmin],paleta[cmin+1] are the colors to merge

    r = (pal[paleta[cmin] * 4] + pal[paleta[cmin + 1] * 4]) / 2;
    g = (pal[paleta[cmin] * 4 + 1] + pal[paleta[cmin + 1] * 4 + 1]) / 2;
    b = (pal[paleta[cmin] * 4 + 2] + pal[paleta[cmin + 1] * 4 + 2]) / 2;

    memmove(&paleta[cmin], &paleta[cmin + 1], sizeof(paleta[0]) * (c - cmin));
    memmove(&dist[cmin], &dist[cmin + 1], sizeof(dist[0]) * (c - cmin));

    pal[paleta[cmin] * 4] = r;
    pal[paleta[cmin] * 4 + 1] = g;
    pal[paleta[cmin] * 4 + 2] = b;

    if (cmin < c - 1) {
      n = cmin;
      dist[n] = *(int *)(color_lookup + pal[paleta[n] * 4] * 256 + pal[paleta[n + 1] * 4] * 4);
      dist[n] +=
          *(int *)(color_lookup + pal[paleta[n] * 4 + 1] * 256 + pal[paleta[n + 1] * 4 + 1] * 4);
      dist[n] +=
          *(int *)(color_lookup + pal[paleta[n] * 4 + 2] * 256 + pal[paleta[n + 1] * 4 + 2] * 4);
    }

    if (cmin > 1) {
      n = cmin - 1;
      dist[n] = *(int *)(color_lookup + pal[paleta[n] * 4] * 256 + pal[paleta[n + 1] * 4] * 4);
      dist[n] +=
          *(int *)(color_lookup + pal[paleta[n] * 4 + 1] * 256 + pal[paleta[n + 1] * 4 + 1] * 4);
      dist[n] +=
          *(int *)(color_lookup + pal[paleta[n] * 4 + 2] * 256 + pal[paleta[n + 1] * 4 + 2] * 4);
    }

    c--;
  }

  for (n = 0; n < 256; n++) {
    dac4[n * 3] = pal[paleta[n] * 4];
    dac4[n * 3 + 1] = pal[paleta[n] * 4 + 1];
    dac4[n * 3 + 2] = pal[paleta[n] * 4 + 2];
  }
}

//-----------------------------------------------------------------------------
//  Create a palette from a color sample
//
//  Input: sample[]
//      Pointer to an RGB table (32x32x32 bytes) with values
//      0 or 1 (whether that color is present in the sample or not)
//
//  Output: apply_palette[] sample[]
//      The resulting palette; in the original table, values of 1 have been
//      replaced with the corresponding palette color index
//-----------------------------------------------------------------------------

byte *sample; // 32768
byte apply_palette[768];
int num_colores;
extern int load_palette;

word new_find_ord(byte *dac) {
  int dmin, dif, r2, g2, b2;
  byte *pal, *endpal, *color;

  color = dac;
  pal = dac;
  endpal = dac + num_colores * 4;
  dmin = 65536;
  if (r < 0)
    r = 0;
  else if (r > 63)
    r = 63;
  if (g < 0)
    g = 0;
  else if (g > 63)
    g = 63;
  if (b < 0)
    b = 0;
  else if (b > 63)
    b = 63;
  r2 = (int)r * 256;
  g2 = (int)g * 256;
  b2 = (int)b * 256;
  do
    if (*pal < 128) {
      dif = *(int *)(color_lookup + r2 + *pal * 4);
      pal++;
      dif += *(int *)(color_lookup + g2 + *pal * 4);
      pal++;
      dif += *(int *)(color_lookup + b2 + *pal * 4);
      pal += 2;
      if (dif < dmin) {
        dmin = dif;
        color = pal - 4;
      }
    } else {
      pal += 4;
    }
  while (pal < endpal);

  return ((color - dac) / 4);
}

void create_palette(void) {
  byte *pal; // Color list in R,G,B,sum(RGB) format
  word *paleta;
  int *dist;
  int rr, gg, bb, n;
  int c, min, cmin = 0;
  byte col;

  // Count colors and generate pal[]

  if ((pal = (byte *)malloc(32768 * 4)) == NULL)
    return;

  sample[0] = 1;                        // Black is always included
  sample[(31 * 32 + 31) * 32 + 31] = 1; // White is always included

  for (num_colores = 0, n = 0, rr = 0; rr < 32; rr++)
    for (gg = 0; gg < 32; gg++)
      for (bb = 0; bb < 32; bb++, n++) {
        if (sample[n]) {
          pal[num_colores * 4] = rr;
          pal[num_colores * 4 + 1] = gg;
          pal[num_colores * 4 + 2] = bb;
          num_colores++;
        }
      }

  if ((paleta = (word *)malloc(num_colores * 2 + 10)) == NULL) {
    free(pal);
    return;
  }
  if ((dist = (int *)malloc(num_colores * 4 + 10)) == NULL) {
    free(paleta);
    free(pal);
    return;
  }

  // pal[num_colores] prepared with { R,G,B,0 }

  r = 0;
  g = 0;
  b = 0;
  pal[0] += 128;
  paleta[0] = 0;
  n = 1;
  do {
    if (!load_palette)
      if ((n & 127) == 0)
        show_progress((char *)texts[497], n * 2, num_colores * 4 - 256);
    c = new_find_ord(pal);
    r = pal[c * 4];
    g = pal[c * 4 + 1];
    b = pal[c * 4 + 2];
    pal[c * 4] += 128;
    paleta[n] = c;
  } while (++n < num_colores);

  // Restore pal

  for (n = 0; n < num_colores; n++) {
    if (pal[n * 4] >= 128)
      pal[n * 4] -= 128;
  }

  // paleta[num_colores] with colors sorted

  for (n = 0; n < num_colores - 1; n++) {
    dist[n] = *(int *)(color_lookup + pal[paleta[n] * 4] * 256 + pal[paleta[n + 1] * 4] * 4);
    dist[n] +=
        *(int *)(color_lookup + pal[paleta[n] * 4 + 1] * 256 + pal[paleta[n + 1] * 4 + 1] * 4);
    dist[n] +=
        *(int *)(color_lookup + pal[paleta[n] * 4 + 2] * 256 + pal[paleta[n + 1] * 4 + 2] * 4);
  }

  // dist[] with distances between all consecutive colors

  c = num_colores - 1;
  while (c > 255) {
    if (!load_palette)
      if ((c & 127) == 0)
        show_progress((char *)texts[497], num_colores * 2 + num_colores - c, num_colores * 4 - 256);

    min = 64 * 64 * 64;
    for (n = 0; n < c; n++) { // Find the minimum distance
      if (dist[n] < min) {
        cmin = n;
        min = dist[n];
      }
    }

    // paleta[cmin+1] is the color to eliminate

    if (c - cmin > 0) {
      memmove(&paleta[cmin + 1], &paleta[cmin + 2], sizeof(word) * (c - cmin));
      memmove(&dist[cmin + 1], &dist[cmin + 2], sizeof(int) * (c - cmin));
    }

    if (cmin < c - 1) {
      n = cmin;
      dist[n] = *(int *)(color_lookup + pal[paleta[n] * 4] * 256 + pal[paleta[n + 1] * 4] * 4);
      dist[n] +=
          *(int *)(color_lookup + pal[paleta[n] * 4 + 1] * 256 + pal[paleta[n + 1] * 4 + 1] * 4);
      dist[n] +=
          *(int *)(color_lookup + pal[paleta[n] * 4 + 2] * 256 + pal[paleta[n + 1] * 4 + 2] * 4);
    }

    c--;
  }

  for (n = 0; n <= c; n++) {
    apply_palette[n * 3 + 0] = pal[paleta[n] * 4 + 0];
    apply_palette[n * 3 + 1] = pal[paleta[n] * 4 + 1];
    apply_palette[n * 3 + 2] = pal[paleta[n] * 4 + 2];
  }

  for (; n <= 255; n++) {
    apply_palette[n * 3 + 0] = 0;
    apply_palette[n * 3 + 1] = 0;
    apply_palette[n * 3 + 2] = 0;
  }

  free(dist);
  free(paleta);
  free(pal);

  for (n = 0; n < 768; n++)
    dac4[n] = apply_palette[n] * 4;

  for (n = 0, c = 0, rr = 0; rr < 32; rr++)
    for (gg = 0; gg < 32; gg++)
      for (bb = 0; bb < 32; bb++, n++)
        if (sample[n]) {
          if (!load_palette)
            if (((c++) & 127) == 0)
              show_progress((char *)texts[497], num_colores * 3 - 256 + c, num_colores * 4 - 256);
          sample[n] = find_color(rr, gg, bb);
        }

  create_dac4();

  for (n = 0; n < 768; n++)
    if ((col = apply_palette[n]))
      apply_palette[n] = col * 2 + 1;

  if (!load_palette)
    show_progress((char *)texts[497], num_colores * 4 - 256, num_colores * 4 - 256);
}


//-----------------------------------------------------------------------------
//      Prepare the wallpaper for a specific palette
//-----------------------------------------------------------------------------

extern byte wallpaper_gradient[128];

byte cwallpaper[256];
extern SDL_Surface *vga;

void prepare_wallpaper(void) {
  FILE *f;
  int x, lon, tap_w, tap_h;
  byte *p, *q;
  byte *temp, *temp2;
  byte pal[768];
  byte old_dac[768];
  byte old_dac4[768];
  int n;

  if ((f = fopen(Setupfile.Desktop_Image, "rb")) == NULL)
    return;
  fseek(f, 0, SEEK_END);
  lon = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (wallpaper != NULL) {
    free(wallpaper);
    wallpaper = NULL;
  }

  if ((temp2 = (byte *)malloc(lon)) == NULL) {
    fclose(f);
    return;
  }
  if (fread(temp2, 1, lon, f) != lon) {
    fclose(f);
    free(temp2);
    return;
  }
  fclose(f);

  tap_w = map_width;
  tap_h = map_height;
  if (fmt_is_map(temp2))
    x = 1;
  else if (fmt_is_pcx(temp2))
    x = 2;
  else if (fmt_is_bmp(temp2))
    x = 3;
  else if (fmt_is_jpg(temp2, lon))
    x = 4;
  else
    x = 0;
  swap(map_width, tap_w);
  swap(map_height, tap_h);

  if (!x) {
    free(temp2);
    return;
  }

  if ((temp = (byte *)malloc(tap_w * tap_h + tap_w)) == NULL) {
    free(temp2);
    return;
  }

  swap(map_width, tap_w);
  swap(map_height, tap_h);

  memcpy(old_dac4, dac4, 768);
  memcpy(old_dac, dac, 768);

  n = load_palette;
  load_palette = 1;
  switch (x) {
  case 1:
    fmt_load_map(temp2, temp, 0);
    break;
  case 2:
    fmt_load_pcx(temp2, temp, 0);
    break;
  case 3:
    fmt_load_bmp(temp2, temp, 0);
    break;
  case 4:
    fmt_load_jpg(temp2, temp, 0, lon);
    break;
  }
  swap(map_width, tap_w);
  swap(map_height, tap_h);
  load_palette = n;

  free(temp2);
  memcpy(pal, dac4, 768);
  create_dac4();

  if (!Setupfile.Desktop_Gama) { // If the file is displayed in color
    for (x = 0, p = pal; x < 256; x++, p += 3)
      cwallpaper[x] = fast_find_color(*p, *(p + 1), *(p + 2));
    p = temp;
    q = p + tap_w * tap_h;
    do
      *p = cwallpaper[*p];
    while (++p < q); // Remap to the current palette
  } else {
    for (x = 0, p = pal; x < 256; x++, p += 3)
      cwallpaper[x] = wallpaper_gradient[(*p + *(p + 1) + *(p + 2)) * 2 / 3];
    p = temp;
    q = p + tap_w * tap_h;
    do
      *p = cwallpaper[*p];
    while (++p < q);
  }

  if (Setupfile.Desktop_Tile) {
    wallpaper_map = wallpaper = temp;
    wallpaper_width = tap_w;
    wallpaper_height = tap_h;
  } else {
    if ((p = (byte *)malloc(vga_width * vga_height)) == NULL) {
      free(temp);
      wallpaper = NULL;
      return;
    }
    rescalar(temp, tap_w, tap_h, p, vga_width, vga_height);
    free(temp);
    wallpaper_width = vga_width;
    wallpaper_height = vga_height;
    wallpaper_map = wallpaper = p;
  }

  memcpy(dac, old_dac, 768);
  memcpy(dac4, old_dac4, 768);
}

void rescalar(byte *si, int sian, int sial, byte *di, int dian, int dial) {
  float xr, yr;
  float ix, iy;
  int x, y;
  byte *s, *d;

  ix = (float)sian / (float)dian;
  iy = (float)sial / (float)dial;

  for (y = 0, yr = 0; y < dial; y++, yr += iy) {
    s = si + (int)yr * sian;
    d = di + y * dian;
    for (x = 0, xr = 0; x < dian; x++, xr += ix) {
      *di++ = *(s + (int)xr);
    }
  }
}

//-----------------------------------------------------------------------------
//      Palette editing functions
//-----------------------------------------------------------------------------

char Valores[72 * 1];
struct t_listbox lRed = {132, 63, Valores, 1, 9, 1};
struct t_listbox lGre = {132 + 11, 63, Valores, 1, 9, 1};
struct t_listbox lBlu = {132 + 22, 63, Valores, 1, 9, 1};

byte SelColor = 0;
byte Retorno = 0;
int Degradar = 0, Intercambiar = 0, Copiar = 0;
int wDegradar = 0, wIntercambiar = 0, wCopiar = 0;

void pal_interpolate1(void) {
  int x, y;
  int w = v.w / big2, h = v.h / big2;
  char cWork[20];

  _show_items();

  wbox(v.ptr, w, h, c0, 2, 10, 128, 128);

  wwrite(v.ptr, w, h, 147, 11, 0, texts[141], c3);
  div_snprintf(cWork, sizeof(cWork), "%03d", SelColor);
  wwrite(v.ptr, w, h, 145, 11, 2, (byte *)cWork, c3);

  wwrite(v.ptr, w, h, 147, 19, 0, texts[142], c3);
  div_snprintf(cWork, sizeof(cWork), " %02X", SelColor);
  wwrite(v.ptr, w, h, 145, 19, 2, (byte *)cWork, c3);

  wwrite(v.ptr, w, h, 147, 63 - 21, 0, texts[143], c3);
  div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3]);
  wwrite(v.ptr, w, h, 140, 63 - 21, 2, (byte *)cWork, c3);

  wwrite(v.ptr, w, h, 147, 63 - 14, 0, texts[144], c3);
  div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3 + 1]);
  wwrite(v.ptr, w, h, 140, 63 - 14, 2, (byte *)cWork, c3);

  wwrite(v.ptr, w, h, 147, 63 - 7, 0, texts[145], c3);
  div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3 + 2]);
  wwrite(v.ptr, w, h, 140, 63 - 7, 2, (byte *)cWork, c3);

  wbox(v.ptr, w, h, c0, 132, 26, 31, 42 - 27);
  wbox(v.ptr, w, h, SelColor, 133, 27, 29, 42 - 29);
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, (y * 16 + x), x * 8 + 2, y * 8 + 10, 7, 7);

  wrectangle(v.ptr, w, h, c4, (SelColor % 16) * 8 + 1, (SelColor / 16) * 8 + 9, 9, 9);
  if (dac[SelColor * 3] * dac[SelColor * 3 + 1] * dac[SelColor * 3 + 2] < (32 * 32 * 32))
    wbox(v.ptr, w, h, c4, (SelColor % 16) * 8 + 4, (SelColor / 16) * 8 + 12, 3, 3);
  else
    wbox(v.ptr, w, h, c0, (SelColor % 16) * 8 + 4, (SelColor / 16) * 8 + 12, 3, 3);

  lRed.created = 0;
  lRed.total_items = 72;
  lBlu.created = 0;
  lBlu.total_items = 72;
  lGre.created = 0;
  lGre.total_items = 72;

  create_listbox(&lRed);
  create_listbox(&lBlu);
  create_listbox(&lGre);

  lRed.first_visible = 63 - dac[SelColor * 3];
  lGre.first_visible = 63 - dac[SelColor * 3 + 1];
  lBlu.first_visible = 63 - dac[SelColor * 3 + 2];

  update_listbox(&lRed);
  update_listbox(&lBlu);
  update_listbox(&lGre);
}

void pal_interpolate2(void) {
  int n = 0;
  static int ax = 2, ay = 10;
  byte cColor, Tocado = 0, Dentro = 0;
  char cWork[20];
  static byte Accion = 0;
  static byte OldColor = 0;
  static byte sRed = 0, sGre = 0, sBlu = 0;
  byte bWork;
  float fR, fG, fB, fIR, fIG, fIB;
  byte cIni, cFin;
  int w = v.w / big2, h = v.h / big2;

  _process_items();
  v_pause = 1;
  update_listbox(&lRed);
  update_listbox(&lBlu);
  update_listbox(&lGre);
  v_pause = 1;
  if (Accion)
    mouse_graf = 2;
  if ((wmouse_y > 10) && (wmouse_y < 138) && (wmouse_x > 2) && (wmouse_x < 130)) {
    cColor = ((wmouse_y - 10) / 8) * 16 + ((wmouse_x - 2) / 8);
    if (cColor != OldColor) {
      wrectangle(v.ptr, w, h, c0, (OldColor % 16) * 8 + 1, (OldColor / 16) * 8 + 9, 9, 9);
      wbox(v.ptr, w, h, cColor, 133, 27, 29, 42 - 29);

      wbox(v.ptr, w, h, c2, 130, 11, 29, 15);

      wwrite(v.ptr, w, h, 147, 11, 0, texts[141], c3);
      div_snprintf(cWork, sizeof(cWork), "%03d", cColor);
      wwrite(v.ptr, w, h, 145, 11, 2, (byte *)cWork, c3);
      wwrite(v.ptr, w, h, 147, 19, 0, texts[142], c3);
      div_snprintf(cWork, sizeof(cWork), " %02X", cColor);
      wwrite(v.ptr, w, h, 145, 19, 2, (byte *)cWork, c3);

      wbox(v.ptr, w, h, c2, 130, 63 - 21, 25, 20);
      wwrite(v.ptr, w, h, 147, 63 - 21, 0, texts[143], c3);
      div_snprintf(cWork, sizeof(cWork), "%02d", dac[cColor * 3]);
      wwrite(v.ptr, w, h, 140, 63 - 21, 2, (byte *)cWork, c3);
      wwrite(v.ptr, w, h, 147, 63 - 14, 0, texts[144], c3);
      div_snprintf(cWork, sizeof(cWork), "%02d", dac[cColor * 3 + 1]);
      wwrite(v.ptr, w, h, 140, 63 - 14, 2, (byte *)cWork, c3);
      wwrite(v.ptr, w, h, 147, 63 - 7, 0, texts[145], c3);
      div_snprintf(cWork, sizeof(cWork), "%02d", dac[cColor * 3 + 2]);
      wwrite(v.ptr, w, h, 140, 63 - 7, 2, (byte *)cWork, c3);
      ax = ((wmouse_x - 2) / 8);
      ay = ((wmouse_y - 10) / 8);
      wrectangle(v.ptr, w, h, c4, (cColor % 16) * 8 + 1, (cColor / 16) * 8 + 9, 9, 9);
      lRed.first_visible = 63 - dac[cColor * 3];
      lGre.first_visible = 63 - dac[cColor * 3 + 1];
      lBlu.first_visible = 63 - dac[cColor * 3 + 2];
      Dentro = 1;
      v.redraw = 1;
      OldColor = cColor;
    }
    if ((mouse_b) && (cColor != SelColor)) {
      memcpy(paleta, dac, 768);
      switch (Accion) {
      case 1:
        //Gradient fill
        if (SelColor > cColor) {
          cIni = cColor;
          cFin = SelColor;
        } else {
          cIni = SelColor;
          cFin = cColor;
        }
        fR = dac[cIni * 3];
        fG = dac[cIni * 3 + 1];
        fB = dac[cIni * 3 + 2];
        fIR = (dac[cFin * 3] - fR) / (cFin - cIni);
        fIG = (dac[cFin * 3 + 1] - fG) / (cFin - cIni);
        fIB = (dac[cFin * 3 + 2] - fB) / (cFin - cIni);

        for (n = cIni; n < cFin; n++) {
          dac[n * 3] = fR;
          fR += fIR;
        }
        for (n = cIni; n < cFin; n++) {
          dac[n * 3 + 1] = fG;
          fG += fIG;
        }
        for (n = cIni; n < cFin; n++) {
          dac[n * 3 + 2] = fB;
          fB += fIB;
        }

        find_colors();
        refresh_dialog();
        wrectangle(v.ptr, w, h, c0, (SelColor % 16) * 8 + 1, (SelColor / 16) * 8 + 9, 9, 9);
        wrectangle(v.ptr, w, h, c4, (cColor % 16) * 8 + 1, (cColor / 16) * 8 + 9, 9, 9);
        set_dac(dac);
        break;
      case 2:
        //Copy
        dac[cColor * 3] = dac[SelColor * 3];
        dac[cColor * 3 + 1] = dac[SelColor * 3 + 1];
        dac[cColor * 3 + 2] = dac[SelColor * 3 + 2];
        find_colors();
        refresh_dialog();
        wrectangle(v.ptr, w, h, c0, (SelColor % 16) * 8 + 1, (SelColor / 16) * 8 + 9, 9, 9);
        wrectangle(v.ptr, w, h, c4, (cColor % 16) * 8 + 1, (cColor / 16) * 8 + 9, 9, 9);
        set_dac(dac);
        break;
      case 3:
        //Swap
        bWork = dac[SelColor * 3];
        dac[SelColor * 3] = dac[cColor * 3];
        dac[cColor * 3] = bWork;
        bWork = dac[SelColor * 3 + 1];
        dac[SelColor * 3 + 1] = dac[cColor * 3 + 1];
        dac[cColor * 3 + 1] = bWork;
        bWork = dac[SelColor * 3 + 2];
        dac[SelColor * 3 + 2] = dac[cColor * 3 + 2];
        dac[cColor * 3 + 2] = bWork;
        find_colors();
        refresh_dialog();
        wrectangle(v.ptr, w, h, c0, (SelColor % 16) * 8 + 1, (SelColor / 16) * 8 + 9, 9, 9);
        wrectangle(v.ptr, w, h, c4, (cColor % 16) * 8 + 1, (cColor / 16) * 8 + 9, 9, 9);
        set_dac(dac);
        break;
      }
      wbox(v.ptr, w, h, SelColor, (SelColor % 16) * 8 + 4, (SelColor / 16) * 8 + 12, 3, 3);
      SelColor = cColor;
      if (dac[SelColor * 3] * dac[SelColor * 3 + 1] * dac[SelColor * 3 + 2] < (32 * 32 * 32))
        wbox(v.ptr, w, h, c4, (SelColor % 16) * 8 + 4, (SelColor / 16) * 8 + 12, 3, 3);
      else
        wbox(v.ptr, w, h, c0, (SelColor % 16) * 8 + 4, (SelColor / 16) * 8 + 12, 3, 3);
      if (Accion) {
        Degradar = 0;
        Copiar = 0;
        Intercambiar = 0;
        Accion = 0;
        call((void_return_type_t)v.paint_handler);
      }
      v.redraw = 1;
    }
  } else {
    if (OldColor != SelColor) {
      wrectangle(v.ptr, w, h, c0, (OldColor % 16) * 8 + 1, (OldColor / 16) * 8 + 9, 9, 9);
      wrectangle(v.ptr, w, h, c4, (SelColor % 16) * 8 + 1, (SelColor / 16) * 8 + 9, 9, 9);
      wbox(v.ptr, w, h, SelColor, 133, 27, 29, 42 - 29);
      OldColor = SelColor;

      wbox(v.ptr, w, h, c2, 130, 11, 29, 15);
      wwrite(v.ptr, w, h, 147, 11, 0, texts[141], c3);
      div_snprintf(cWork, sizeof(cWork), "%03d", SelColor);
      wwrite(v.ptr, w, h, 145, 11, 2, (byte *)cWork, c3);
      wwrite(v.ptr, w, h, 147, 19, 0, texts[142], c3);
      div_snprintf(cWork, sizeof(cWork), " %02X", SelColor);
      wwrite(v.ptr, w, h, 145, 19, 2, (byte *)cWork, c3);
      wbox(v.ptr, w, h, c2, 130, 63 - 21, 25, 20);

      wwrite(v.ptr, w, h, 147, 63 - 21, 0, texts[143], c3);
      div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3]);
      wwrite(v.ptr, w, h, 140, 63 - 21, 2, (byte *)cWork, c3);
      wwrite(v.ptr, w, h, 147, 63 - 14, 0, texts[144], c3);
      div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3 + 1]);
      wwrite(v.ptr, w, h, 140, 63 - 14, 2, (byte *)cWork, c3);
      wwrite(v.ptr, w, h, 147, 63 - 7, 0, texts[145], c3);
      div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3 + 2]);
      wwrite(v.ptr, w, h, 140, 63 - 7, 2, (byte *)cWork, c3);
      lRed.first_visible = 63 - dac[SelColor * 3];
      lGre.first_visible = 63 - dac[SelColor * 3 + 1];
      lBlu.first_visible = 63 - dac[SelColor * 3 + 2];
      wbox(v.ptr, w, h, SelColor, 133, 27, 29, 42 - 29);
      v.redraw = 1;
    }
  }

  //Scrollbars
  if (lRed.zone != sRed) {
    sRed = lRed.zone;
    v.redraw = 1;
  }
  if (lGre.zone != sGre) {
    sGre = lGre.zone;
    v.redraw = 1;
  }
  if (lBlu.zone != sBlu) {
    sBlu = lBlu.zone;
    v.redraw = 1;
  }

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    Tocado = 0;
    switch (lRed.zone) {
    case 2:
      do {
        read_mouse();
      } while (mouse_b);
      /* fall through */
    case 3:
      do {
        read_mouse();
      } while (mouse_b);
      /* fall through */
    case 4:
      dac[SelColor * 3] = 63 - lRed.first_visible;
      Tocado = 1;
      break;
    }
    switch (lGre.zone) {
    case 2:
      do {
        read_mouse();
      } while (mouse_b);
      /* fall through */
    case 3:
      do {
        read_mouse();
      } while (mouse_b);
      /* fall through */
    case 4:
      dac[SelColor * 3 + 1] = 63 - lGre.first_visible;
      Tocado = 1;
      break;
    }
    switch (lBlu.zone) {
    case 2:
      do {
        read_mouse();
      } while (mouse_b);
      /* fall through */
    case 3:
      do {
        read_mouse();
      } while (mouse_b);
      /* fall through */
    case 4:
      dac[SelColor * 3 + 2] = 63 - lBlu.first_visible;
      Tocado = 1;
      break;
    }
    if (Tocado) {
      Degradar = 0;
      Copiar = 0;
      Intercambiar = 0;
      Accion = 0;
      call((void_return_type_t)v.paint_handler);
      lRed.first_visible = 63 - dac[SelColor * 3];
      lGre.first_visible = 63 - dac[SelColor * 3 + 1];
      lBlu.first_visible = 63 - dac[SelColor * 3 + 2];
      find_colors();
      refresh_dialog();
      set_dac(dac);
      v.redraw = 1;
    }
  }

  if (Degradar && !wDegradar) {
    if (wIntercambiar)
      Intercambiar = 0;
    if (wCopiar)
      Copiar = 0;
    call((void_return_type_t)v.paint_handler);
    Accion = 1;
  } else if (!Degradar && Accion == 1)
    Accion = 0;
  if (Intercambiar && !wIntercambiar) {
    if (wDegradar)
      Degradar = 0;
    if (wCopiar)
      Copiar = 0;
    call((void_return_type_t)v.paint_handler);
    Accion = 2;
  } else if (!Intercambiar && Accion == 2)
    Accion = 0;

  if (Copiar && !wCopiar) {
    if (wDegradar)
      Degradar = 0;
    if (wIntercambiar)
      Intercambiar = 0;
    Accion = 3;
    call((void_return_type_t)v.paint_handler);
  } else if (!Copiar && Accion == 3)
    Accion = 0;

  switch (v.active_item) {
  case 0:
    Retorno = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    //Undo
    memcpy(dac, paleta, 768);

    find_colors();
    refresh_dialog();
    set_dac(dac);
    Degradar = 0;
    Copiar = 0;
    Intercambiar = 0;
    Accion = 0;
    call((void_return_type_t)v.paint_handler);

    boton(4, w / 2, h - 13, 1, c0);

    //Refresh values and scrollbars
    wbox(v.ptr, w, h, c2, 130, 63 - 21, 25, 20);
    wwrite(v.ptr, w, h, 147, 63 - 21, 0, texts[143], c3);
    div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3]);
    wwrite(v.ptr, w, h, 140, 63 - 21, 2, (byte *)cWork, c3);
    wwrite(v.ptr, w, h, 147, 63 - 14, 0, texts[144], c3);
    div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3 + 1]);
    wwrite(v.ptr, w, h, 140, 63 - 14, 2, (byte *)cWork, c3);
    wwrite(v.ptr, w, h, 147, 63 - 7, 0, texts[145], c3);
    div_snprintf(cWork, sizeof(cWork), "%02d", dac[SelColor * 3 + 2]);
    wwrite(v.ptr, w, h, 140, 63 - 7, 2, (byte *)cWork, c3);
    lRed.first_visible = 63 - dac[SelColor * 3];
    lGre.first_visible = 63 - dac[SelColor * 3 + 1];
    lBlu.first_visible = 63 - dac[SelColor * 3 + 2];

    v.redraw = 1;
    break;
  }
  wDegradar = Degradar;
  wIntercambiar = Intercambiar;
  wCopiar = Copiar;
}

void pal_interpolate0(void) {
  v.type = 1; // Dialog
  v.w = 220 - 46 - 7;
  v.h = 163 + 24 - 16;
  v.title = texts[138];
  v.paint_handler = pal_interpolate1;
  v.click_handler = pal_interpolate2;

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _button(104, v.w / 2, v.h - 14, 1);

  _flag(105, 4, 143, &Degradar);
  _flag(106, v.w / 2 - (8 * big2 + text_len(texts[106])) / 2, 143, &Intercambiar);
  _flag(107, (v.w - 5) - (8 * big2 + text_len(texts[107])), 143, &Copiar);
}

//-----------------------------------------------------------------------------
//	Palette menu functions
//-----------------------------------------------------------------------------

void pal_edit() {
  int n;
  byte DacAux[768];
  Retorno = 0;
  memcpy(DacAux, dac, 768);
  memcpy(paleta, dac, 768);
  show_dialog(pal_interpolate0);
  if (!Retorno) {
    memcpy(dac, DacAux, 768);
    find_colors();
    set_dac(dac);
  } else {
    for (n = 0; n < 768; n++)
      if (DacAux[n] != dac[n])
        break;
    if (n < 768) {
      if (has_maps()) {
        v_title = (char *)texts[53];
        v_text = (char *)texts[321];
        show_dialog(accept0);
      } else
        v_accept = 1;
      memcpy(dac4, dac, 768);
      memcpy(dac, DacAux, 768);
      if (v_accept)
        pal_refresh(0, 1);
      else
        pal_refresh(1, 1);
    }
  }
}
