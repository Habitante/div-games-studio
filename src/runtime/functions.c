
//----------------------------------------------------------------------------
// Internal functions code
//----------------------------------------------------------------------------

#include "inter.h"
#include "div_string.h"
#include <errno.h>

void read_mouse(void);
#include "mixer.h"
#include "divsound.h"

#ifdef __EMSCRIPTEN__
extern void is_fps(byte);
#endif

void load_pal(void);
int is_PCX(byte *buffer);
void adapt_palette(byte *ptr, int len, byte *pal, byte *xlat);
void put_screen(void);
void texn2(byte *dest, int vga_width, byte *p, int x, int y, byte w, int h);
void get_token(void);
void expres0(void);
void expres1(void);
void expres2(void);
void expres3(void);
void expres4(void);
void expres5(void);
void _encrypt(int encode, char *filename, char *key);
void _compress_file(int encode, char *filename);


extern int max_reloj;

int joy_position(int axis);

void path_find(void);
void path_line(void);
void path_free(void);

void signal_tree(int p, int s);
FILE *div_open_file(char *file);
void fade_on(void);
void fade_off(void);
void stop_scroll(void);
void kill_invisible(void);
void stop_mode7(void);

int get_ticks(void);
void function_exec(int, int);

extern int skipped[128];
extern int num_skipped;

//----------------------------------------------------------------------------
//  Fix for the /oneatx /fp5 bug in i.cpp
//----------------------------------------------------------------------------

static int new_ticks = 0, old_ticks = 0;

int get_clock(void) {
  new_ticks = OSDEP_GetTicks();
  frame_clock += (new_ticks - old_ticks);
  old_ticks = new_ticks;

  return frame_clock;
}

//----------------------------------------------------------------------------
//      Signal(process,signal)
//----------------------------------------------------------------------------

void _signal(void) {
  int i;
  bp = stack[sp - 1];
  if ((bp & 1) && bp >= id_init && bp <= id_end && bp == mem[bp]) {
    if (mem[bp + _Status])
      if (stack[sp] < 100)
        mem[bp + _Status] = stack[sp--] + 1;
      else {
        mem[bp + _Status] = stack[sp--] - 99;
        if (mem[bp + _Son])
          signal_tree(mem[bp + _Son], stack[sp + 1] - 99);
      }
    else
      stack[--sp] = 0; // Returns 0 if the process was dead
  } else {
    for (i = id_start; i <= id_end; i += iloc_len)
      if (mem[i + _Status] && mem[i + _Bloque] == bp) {
        if (stack[sp] < 100)
          mem[i + _Status] = stack[sp] + 1;
        else {
          mem[i + _Status] = stack[sp] - 99;
          if (mem[i + _Son])
            signal_tree(mem[i + _Son], stack[sp] - 99);
        }
      }
    stack[--sp] = 0;
  }
}

void signal_tree(int p, int s) {
  do {
    mem[p + _Status] = s;
    if (mem[p + _Son])
      signal_tree(mem[p + _Son], s);
    p = mem[p + _BigBro];
  } while (p);
}

//----------------------------------------------------------------------------
//      Key(key_code)
//----------------------------------------------------------------------------

void _key(void) {
  if (stack[sp] <= 0 || stack[sp] >= 128) {
    runtime_error(101);
    return;
  }
  stack[sp] = key(stack[sp]);
}

//----------------------------------------------------------------------------
//  Function to locate and open a file (pal,fpg,fnt,...)
//  This function must follow the same algorithm in F.CPP and DIVC.CPP
//----------------------------------------------------------------------------

char full[_MAX_PATH + 1];

#ifdef DEBUG

FILE *__fpopen(byte *file, char *mode) {
#ifdef DEBUG
  char fprgpath[_MAX_PATH * 2];
  FILE *f;

  div_strcpy(fprgpath, sizeof(fprgpath), prgpath);
  div_strcat(fprgpath, sizeof(fprgpath), "/");
  div_strcat(fprgpath, sizeof(fprgpath), full);

  if ((f = fopen(fprgpath, mode))) { // prgpath/file
    div_strcpy(full, sizeof(full), fprgpath);
    return f;
  }

#endif

  return NULL;
}

FILE *fpopen(byte *file, char *mode) {
  return __fpopen(file, mode);
}

#endif

FILE *open_multi(char *file, char *mode) {
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  char remote[255];

  char *ff = (char *)file;

  while (*ff != 0) {
    if (*ff == '\\')
      *ff = '/';
    ff++;
  }

  div_strcpy(full, sizeof(full), (char *)file); // full filename
#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  if ((f = fopen(full, mode))) // "paz\fixero.est"
    return f;


  if (_fullpath(full, (char *)file, _MAX_PATH) == NULL)
    return (NULL);

  _splitpath(full, drive, dir, fname, ext);

  if (strchr(ext, '.') == NULL) {
    div_strcpy(full, sizeof(full), ext);
  } else {
    div_strcpy(full, sizeof(full), strchr(ext, '.') + 1);
  }

  if (strlen(full) && file[0] != '/')
    div_strcat(full, sizeof(full), "/");

  div_strcat(full, sizeof(full), (char *)file);

  if ((f = fopen(full, mode))) // "est\paz\fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  strupr(full);

  if ((f = fopen(full, mode))) // "est\paz\fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  div_strcpy(full, sizeof(full), fname);
  div_strcat(full, sizeof(full), ext);

  if ((f = fopen(full, mode))) // "fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  strupr(full);

  if ((f = fopen(full, mode))) // "fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  strlwr(full);

  if ((f = fopen(full, mode))) // "fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  if (strchr(ext, '.') == NULL)
    div_strcpy(full, sizeof(full), ext);
  else
    div_strcpy(full, sizeof(full), strchr(ext, '.') + 1);

  if (strlen(full))
    div_strcat(full, sizeof(full), "/");

  div_strcat(full, sizeof(full), fname);
  div_strcat(full, sizeof(full), ext);

  if ((f = fopen(full, mode))) // "est\fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

  strlwr(full);

  if ((f = fopen(full, mode))) // "est\fixero.est"
    return f;

#ifdef DEBUG
  if ((f = fpopen((byte *)full, mode)))
    return f;
#endif

#ifdef ZLIB
  if (mode[0] != 'w')
    if ((f = memz_open_file((byte *)file)))
      return f;
#endif

  return NULL;
}

FILE *div_open_file(char *file) {
  FILE *f, *fe;
  char *ff = (char *)file;

#ifdef DEBUG
  printf("opening file: [%s]\n", file);
#endif

  if (strlen((const char *)file) < 1)
    return NULL;

  if (strlen((char *)file) == 0)
    return NULL;

  f = open_multi(file, "rb");

  if (!f)
    div_strcpy(full, sizeof(full), "");
  return (f);
}

//----------------------------------------------------------------------------
//  When saving a file (save*()), remove it from the packfile (if present)
//----------------------------------------------------------------------------

void packfile_del(char *file) {
  FILE *f;
  char full[_MAX_PATH + 1];
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  int n;

  if (_fullpath(full, (char *)file, _MAX_PATH) == NULL)
    return;
  _splitpath(full, drive, dir, fname, ext);

  div_strcpy(full, sizeof(full), fname);
  div_strcat(full, sizeof(full), ext);

  strupr(full);

  for (n = 0; n < npackfiles; n++)
    if (!strcmp(full, packdir[n].filename))
      break;

  if (n < npackfiles) { // If the file is in the packfile ...

    if ((f = fopen(packfile, "rb+")) != NULL) {
      div_strcpy(packdir[n].filename, sizeof(packdir[n].filename), "");
      fseek(f, 24, SEEK_SET);
      fwrite(packdir, sizeof(struct _packdir), npackfiles, f);
      fseek(f, 0, SEEK_END);
      fclose(f);
    }
  }
}

//----------------------------------------------------------------------------
//  Read a file from the packfile (into byte * packptr), returns:
//  -1 Not found, -2 Not enough memory, N File length
//----------------------------------------------------------------------------

int read_packfile(byte *file) {
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  char *ptr;
  int n;
  unsigned long len_desc;
  if (_fullpath(full, (char *)file, _MAX_PATH) == NULL)
    return (-1);
  char *ff = (char *)file;

  while (*ff != 0) {
    if (*ff == '\\')
      *ff = '/';
    ff++;
  }

  div_strcpy(full, sizeof(full), (char *)file);

  _splitpath(full, drive, dir, fname, ext);

  div_strcpy(full, sizeof(full), fname);
  div_strcat(full, sizeof(full), ext);

  strupr(full);

  for (n = 0; n < npackfiles; n++) {
    if (!strcmp(full, packdir[n].filename))
      break;
  }
  if (n < npackfiles) {
    len_desc = packdir[n].len_desc;
    if ((packptr = (byte *)malloc(len_desc)) != NULL) {
      if ((ptr = (char *)malloc(packdir[n].len)) != NULL) {
        if ((f = fopen(packfile, "rb")) == NULL) {
#ifndef DEBUG
#ifdef ZLIB
          f = memz_open_file((byte *)packfile);
#endif
#endif
        }
        if (f != NULL) {
          fseek(f, packdir[n].offset, SEEK_SET);
          fread(ptr, 1, packdir[n].len, f);
          fclose(f);
#ifdef ZLIB
          if (!uncompress(packptr, &len_desc, (byte *)ptr, packdir[n].len))
#else
          if (false)
#endif
          {
            free(ptr);
            return (packdir[n].len_desc);
          } else {
            free(ptr);
            free(packptr);
            return (-2);
          }
        } else {
          free(ptr);
          free(packptr);
          return (-1);
        }
      } else {
        free(packptr);
        return (-2);
      }
    } else
      return (-2);
  } else
    return (-1);
}


//----------------------------------------------------------------------------
//      Load_pal(file)
//----------------------------------------------------------------------------

int hacer_fade = 0;

void force_pal(void) {
  auto_adapt_palette = 0;
  if (stack[sp]) {
    load_pal();
    if (stack[sp])
      auto_adapt_palette = 1;
  }
}

void load_pal(void) {
  byte pal[FNT_GENCODE_OFFSET];
  int m, offs = 8;

  if (auto_adapt_palette) {
    runtime_error(183);
    stack[sp] = 0;
    return;
  }

  if (npackfiles) {
    m = read_packfile((byte *)&mem[stack[sp]]);
    if (m == -1)
      goto palfuera;
    if (m == -2) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    if (m <= 0) {
      stack[sp] = 0;
      runtime_error(200);
      return;
    }
    memcpy(pal, packptr, FNT_GENCODE_OFFSET);
    free(packptr);
  } else {
palfuera:
    if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
      stack[sp] = 0;
      runtime_error(102);
      return;
    } else {
      fread(pal, 1, FNT_GENCODE_OFFSET, es);
      fclose(es);
    }
  }

  if (strcmp((char *)pal, "pal\x1a\x0d\x0a")) {     // not a pal file
    if (strcmp((char *)pal, "fpg\x1a\x0d\x0a")) {   // not an fpg
      if (strcmp((char *)pal, "fnt\x1a\x0d\x0a")) { // not a fnt file

        if (strcmp((char *)pal, "map\x1a\x0d\x0a")) { // not a map file

          if (is_PCX((byte *)pal)) { // Take the PCX palette

            if (npackfiles) {
              m = read_packfile((byte *)&mem[stack[sp]]);
              if (m == -1)
                goto palfuera2;
              if (m == -2) {
                stack[sp] = 0;
                runtime_error(100);
                return;
              }
              if (m <= 0) {
                stack[sp] = 0;
                runtime_error(200);
                return;
              }
              memcpy(pal, packptr + m - PALETTE_SIZE, PALETTE_SIZE);
              free(packptr);
            } else {
palfuera2:
              if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
                stack[sp] = 0;
                runtime_error(102);
                return;
              } else {
                fseek(es, -PALETTE_SIZE, SEEK_END);
                fread(pal, 1, PALETTE_SIZE, es);
                fclose(es);
              }
            }

            for (m = 0; m < PALETTE_SIZE; m++)
              pal[m] /= 4;
            offs = 0;

          } else {
            stack[sp] = 0;
            runtime_error(103);
            return;
          }

        } else
          offs = 48;
      }
    }
  }

  for (m = 0; m < PALETTE_SIZE; m++)
    if (pal[m + offs] != palette[m])
      break;
  if (m < PALETTE_SIZE) {
    dr = dacout_r;
    dg = dacout_g;
    db = dacout_b;
    if (dr < 63 || dg < 63 || db < 63) {
      hacer_fade = 1;
      fade_off();
      sp--;
    }
    memcpy(palette, pal + offs, PALETTE_SIZE);
    apply_palette();
  }

  palette_loaded = 1;
  stack[sp] = 1;
}

void apply_palette(void) {
  byte *p, c0, c1;
  int n;

  if (process_palette != NULL) {
    process_palette();
  }

  palcrc = 0;
  for (n = 0; n < PALETTE_SIZE; n++) {
    palcrc += (int)palette[n];
    palcrc <<= 1;
  }

  memcpy(dac, palette, PALETTE_SIZE);
  init_ghost();
  create_ghost();

  find_color(0, 0, 0);
  c0 = find_col;
  find_color(63, 63, 63);
  c1 = find_col;
  p = fonts[0] + FNT_TABLE_OFFSET + sizeof(fnt_table_entry) * 256;

  for (n = 0; n < 12288; n++) {
    if (*p == last_c1) {
      *p++ = c1;
      if ((n & 7) != 7 && *p != last_c1)
        *p = c0;
    } else
      p++;
  }
  last_c1 = c1;

  update_palette();

#ifdef DEBUG
  init_colors();
  new_palette = 1;
#endif

  if (hacer_fade) {
    hacer_fade = 0;
    dacout_r = dr;
    dacout_g = dg;
    dacout_b = db;
    fade_on();
    sp--;
  }
}

//----------------------------------------------------------------------------
//      Unload_map(code)
//----------------------------------------------------------------------------

void unload_map(void) {
  if (stack[sp] < MAX_FPG_GRAPHICS || stack[sp] > MAX_FPG_GRAPHICS * 2 - 1)
    return;
  if (g[0].grf[stack[sp]] != 0) {
    free((byte *)(g[0].grf[stack[sp]]) - 1330);
    g[0].grf[stack[sp]] = 0;
  }
}

//----------------------------------------------------------------------------
//      Load_map(file) - Returns the graphic code (MAX_FPG_GRAPHICS..MAX_FPG_GRAPHICS*2-1)
//----------------------------------------------------------------------------

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

int is_PCX(byte *buffer) {
  int loes = 0;

  if (buffer[2] == 1 && buffer[3] == 8 && buffer[65] == 1)
    loes = 1;
  return (loes);
}

byte *pcxdac;

void decompress_pcx(byte *buffer, byte *mapa) {
  unsigned int con;
  unsigned int pixel = 0, pixel_line = 0;
  unsigned int last_byte, bytes_line;
  char ch, rep;
  pcx_header header;
  byte *dest;
  int map_width, map_height;

  memcpy((byte *)&header, buffer, sizeof(pcx_header));
  buffer += 128; // Start of image data

  map_width = header.xmax - header.xmin + 1;
  map_height = header.ymax - header.ymin + 1;

  memset(mapa, 0, map_width * map_height);

  last_byte = header.bytes_per_line * header.color_planes * map_height;
  bytes_line = header.bytes_per_line * header.color_planes;

  dest = mapa;

  do {
    ch = *buffer++;          // Copy one by default.
    if ((ch & 192) == 192) { // If RLE then
      rep = (ch & 63);       // rep = number of times to copy.
      ch = *buffer++;
    } else
      rep = 1;
    pixel += rep; // Bounds check.
    pixel_line += rep;
    if (pixel > last_byte) {
      rep -= pixel - last_byte;
      for (con = 0; con < rep; con++)
        *dest++ = ch;
      break;
    }
    if (pixel_line == bytes_line) {
      pixel_line = 0;
      rep -= bytes_line - map_width;
    }
    for (con = 0; con < rep; con++)
      *dest++ = ch;
  } while (1);

  for (con = 0; con < PALETTE_SIZE; con++)
    buffer[con] /= 4;
  pcxdac = buffer;

  if (!palette_loaded) {
    for (con = 0; con < PALETTE_SIZE; con++)
      if (buffer[con] != palette[con])
        break;
    if (con < PALETTE_SIZE) {
      dr = dacout_r;
      dg = dacout_g;
      db = dacout_b;
      if (dr < 63 || dg < 63 || db < 63) {
        hacer_fade = 1;
        fade_off();
        sp--;
      }
      memcpy(palette, buffer, PALETTE_SIZE);
      apply_palette();
    }
    palette_loaded = 1;
  }
}

void load_map(void) {
  int width, height, num_points, m;
  byte *ptr, *buffer;
  pcx_header header;

  if (npackfiles) {
    m = read_packfile((byte *)&mem[stack[sp]]);
    if (m == -1)
      goto mapfuera;
    if (m == -2) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    if (m <= 0) {
      stack[sp] = 0;
      runtime_error(200);
      return;
    }
    ptr = packptr;
    file_len = m;
  } else {
mapfuera:
    if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
      stack[sp] = 0;
      runtime_error(143);
      return;
    } else {
      fseek(es, 0, SEEK_END);
      file_len = ftell(es);
      if ((ptr = (byte *)malloc(file_len)) != NULL) {
        fseek(es, 0, SEEK_SET);
        fread(ptr, 1, file_len, es);
        fclose(es);
      } else {
        fclose(es);
        stack[sp] = 0;
        runtime_error(100);
        return;
      }
    }
  }

  if (!strcmp((char *)ptr, "map\x1a\x0d\x0a")) {
    if (process_map != NULL)
      process_map((char *)ptr, file_len);

    if (!palette_loaded) {
      for (m = 0; m < PALETTE_SIZE; m++)
        if (ptr[m + 48] != palette[m])
          break;
      if (m < PALETTE_SIZE) {
        dr = dacout_r;
        dg = dacout_g;
        db = dacout_b;
        if (dr < 63 || dg < 63 || db < 63) {
          hacer_fade = 1;
          fade_off();
          sp--;
        }
        memcpy(palette, ptr + 48, PALETTE_SIZE);
        apply_palette();
      }
      palette_loaded = 1;
    }

    width = *(word *)(ptr + 8);
    height = *(word *)(ptr + 10);
    num_points = *(word *)(ptr + MAP_CTRLPTS_OFFSET);

    adapt_palette(ptr + 1394 + num_points * 4, width * height, ptr + 48, NULL);

    ptr = ptr + 1394 - 64;

    *((int *)ptr + 13) = width;
    *((int *)ptr + 14) = height;
    *((int *)ptr + 15) = num_points;

    while (g[0].grf[next_map_code]) {
      if (next_map_code++ == MAX_FPG_GRAPHICS * 2 - 1)
        next_map_code = MAX_FPG_GRAPHICS;
    }
    g[0].grf[next_map_code] = (int *)ptr;
    stack[sp] = next_map_code;

  } else if (is_PCX(ptr)) {
    memcpy((byte *)&header, ptr, sizeof(pcx_header));
    width = header.xmax - header.xmin + 1;
    height = header.ymax - header.ymin + 1;
    num_points = 0;

    if ((!width && !height) || width < 0 || height < 0) {
      runtime_error(144);
      free(ptr);
      return;
    }

    buffer = (byte *)malloc(1394 + width * height);
    decompress_pcx(ptr, &buffer[1394]);

    adapt_palette(buffer + 1394, width * height, pcxdac, NULL);

    free(ptr);

    buffer = buffer + 1394 - 64;

    *((int *)buffer + 13) = width;
    *((int *)buffer + 14) = height;
    *((int *)buffer + 15) = num_points;

    while (g[0].grf[next_map_code]) {
      if (next_map_code++ == MAX_FPG_GRAPHICS * 2 - 1)
        next_map_code = MAX_FPG_GRAPHICS;
    }
    g[0].grf[next_map_code] = (int *)buffer;
    stack[sp] = next_map_code;

  } else {
    runtime_error(144);
    free(ptr);
    return;
  }

  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      New_map(width,height,center_x,center_y,color) - Returns the code
//      Unloaded with unload_map(code)
//----------------------------------------------------------------------------

void new_map(void) {
  int width, height, cx, cy, color;
  byte *ptr;

  color = stack[sp--];
  cy = stack[sp--];
  cx = stack[sp--];
  height = stack[sp--];
  width = stack[sp];
  stack[sp] = 0;

  // Check width/height/color bounds ...

  if (width < 1 || height < 1 || width > 32768 || height > 32768) {
    runtime_error(153);
    return;
  }
  if (color < 0 || color > 255) {
    runtime_error(154);
    return;
  }
  if (cx < 0 || cy < 0 || cx >= width || cy >= height) {
    runtime_error(155);
    return;
  }

  if ((ptr = (byte *)malloc(1330 + 64 + 4 + width * height)) != NULL) {
    ptr += 1330; // fix load_map/unload_map
    *((int *)ptr + 13) = width;
    *((int *)ptr + 14) = height;
    *((int *)ptr + 15) = 1; // Define one control point (the center)
    *((word *)ptr + 32) = cx;
    *((word *)ptr + 33) = cy;
    memset(ptr + 4 + 64, color, width * height);

    while (g[0].grf[next_map_code]) {
      if (next_map_code++ == MAX_FPG_GRAPHICS * 2 - 1)
        next_map_code = MAX_FPG_GRAPHICS;
    }
    g[0].grf[next_map_code] = (int *)ptr;
    stack[sp] = next_map_code;

  } else
    runtime_error(100);
}

//----------------------------------------------------------------------------
//      Load_fpg(file) - Returns the file code 0..max_fpgs
//----------------------------------------------------------------------------
#define STDOUTLOG

void load_fpg(void) {
  int num = 0, n = 0, m = 0;
  int **lst = NULL;
  byte *ptr = NULL, *ptr2 = NULL, *ptr3 = NULL;
  byte xlat[256];
  int *iptr = NULL;
  int frompak = 0;

  memset(xlat, 0, 256);

  while (num < max_fpgs) {
    if (g[num].fpg == 0) {
      break;
    }
    num++;
  }
  if (num == max_fpgs) {
    stack[sp] = 0;
    runtime_error(104);
    return;
  }
  if (num) {
    if ((lst = (int **)malloc(sizeof(int *) * MAX_FPG_GRAPHICS)) == NULL) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
  } else
    lst = g[0].grf;
  memset(lst, 0, sizeof(int *) * MAX_FPG_GRAPHICS);

  if (npackfiles) {
    m = read_packfile((byte *)&mem[stack[sp]]);
    if (m == -1)
      goto fpgfuera;
    if (m == -2) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    if (m <= 0) {
      stack[sp] = 0;
      runtime_error(200);
      return;
    }
    ptr = packptr;
    file_len = m;
    g[num].fpg = (int **)ptr;
    frompak = 1;
  } else {
fpgfuera:
#ifdef STDOUTLOG
    printf("fpg wanted is [%s]\n", (char *)&mem[stack[sp]]);
#endif
    if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
      stack[sp] = 0;
      runtime_error(105);
      return;
    } else {
      fseek(es, 0, SEEK_END);
      file_len = ftell(es);

#ifdef __EMSCRIPTEN__
      file_len = FNT_GENCODE_OFFSET;
#endif
      if ((ptr = (byte *)malloc(file_len + 8)) != NULL) {
        memset(ptr, 0, file_len + 8);
        g[num].fpg = (int **)ptr;
        fseek(es, 0, SEEK_SET);
        n = fread(ptr, 1, file_len, es);
#ifdef STDOUTLOG
        printf("ptr is %p\n", (void *)ptr);
        printf("read %d bytes of %d\n", n, file_len);
#endif

#ifndef __EMSCRIPTEN__
        fclose(es);
#endif
#ifdef STDOUTLOG
        printf("fpg pointer is %p\n", (void *)ptr);
#endif
      } else {
        fclose(es);
        stack[sp] = 0;
        runtime_error(100);
        return;
      }
    }
  }

  if (strcmp((char *)ptr, "fpg\x1a\x0d\x0a")) {
    runtime_error(106);
    free(ptr);
    return;
  }

  if (process_fpg != NULL)
    process_fpg((char *)ptr, file_len);
  if (!palette_loaded) {
    for (m = 0; m < PALETTE_SIZE; m++)
      if (ptr[m + 8] != palette[m])
        break;
    if (m < PALETTE_SIZE) {
      dr = dacout_r;
      dg = dacout_g;
      db = dacout_b;
      if (dr < 63 || dg < 63 || db < 63) {
        hacer_fade = 1;
        fade_off();
        sp--;
      }
      memcpy(palette, ptr + 8, PALETTE_SIZE);
      apply_palette();
    }
    palette_loaded = 1;
  }

  for (m = 0, n = 0; n < PALETTE_SIZE; n++) {
    m += (int)ptr[n + 8];
    m <<= 1;
  }

  if (m != palcrc) {
    xlat[0] = 0;
    for (n = 1; n < 256; n++) {
      find_color(ptr[n * 3 + 8], ptr[n * 3 + 9], ptr[n * 3 + 10]);
      xlat[n] = find_col;
    }
  }

  g[num].grf = lst;

#ifdef STDOUTLOG
  printf("num: %d ptr: %p\n", num, (void *)ptr);
#endif

#ifdef __EMSCRIPTEN__
  // do something different
  if (frompak) {
    es = fmemopen(ptr, file_len, "rb");
  }
  fseek(es, 0, SEEK_END);
  file_len = ftell(es);
  fseek(es, FNT_GENCODE_OFFSET, SEEK_SET);
  int len_ = 1;
  int num_ = 1;

  while (ftell(es) < file_len && len_ > 0 && num_ > 0) {
    int pos = ftell(es);
    byte *mptr = &ptr[pos];
    fread(&num_, 4, 1, es);
    fread(&len_, 4, 1, es);
    fseek(es, -8, SEEK_CUR);
    mptr = (byte *)malloc(len_);
    fread(mptr, 1, len_, es);
    lst[num_] = iptr = (int *)mptr;
    if (m != palcrc) {
      adapt_palette(ptr + 64 + iptr[15] * 4, iptr[13] * iptr[14], (byte *)(g[num].fpg) + 8,
                    &xlat[0]);
    }
  }
  fclose(es);
#else
  ptr += FNT_GENCODE_OFFSET; // FPG header length
  ptr2 = ptr;
  ptr3 = ptr;

  while (ptr <= (ptr2 + file_len) && *(int *)ptr3 < MAX_FPG_GRAPHICS && *(int *)ptr3 > 0) {
    int *ptr_4 = (int *)ptr3;
    int *ptr_8 = (int *)ptr3;
    int num = *ptr_4;
    int len = *(ptr_8 + 1);

    lst[num] = iptr = ptr_4;
    if (m != palcrc)
      adapt_palette(ptr + 64 + iptr[15] * 4, iptr[13] * iptr[14], (byte *)(g[num].fpg) + 8,
                    &xlat[0]);
    ptr = (byte *)&ptr2[len]; //(int*)(ptr[4]);
    ptr3 = ptr;
    ptr2 = ptr;
  }
#endif
#ifdef STDOUTLOG
  printf("fpg search ended, %p: ptr: %p\n", (void *)((byte *)g[num].fpg + file_len), (void *)ptr);
#endif
  stack[sp] = num;
  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      Start_scroll(snum,file,graf1,graf2,region,flags)
//----------------------------------------------------------------------------

void set_scroll(int plane, int x, int y);

void start_scroll(void) {
  int file, graf1, graf2, reg, s;
  int *ptr1, *ptr2, mf;

  mf = stack[sp--];
  reg = stack[sp--];
  graf2 = stack[sp--];
  graf1 = stack[sp--];
  file = stack[sp--];
  snum = stack[sp];
  stack[sp] = 0;

  if (snum < 0 || snum > 9) {
    runtime_error(107);
    return;
  }

  iscroll[snum].map_flags = mf;

  if (iscroll[snum].on) {
    stack[sp] = snum;
    stop_scroll();
  }

  if (reg >= 0 && reg < max_region) {
    iscroll[snum].x = region[reg].x0;
    iscroll[snum].y = region[reg].y0;
    iscroll[snum].w = region[reg].x1 - region[reg].x0;
    iscroll[snum].h = region[reg].y1 - region[reg].y0;
  } else {
    runtime_error(108);
    return;
  }

  if (iscroll[snum].w == 0 || iscroll[snum].h == 0) {
    runtime_error(146);
    return;
  }

  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf1 < 0 || graf1 >= max_grf) {
    runtime_error(110);
    return;
  }
  if (graf2 < 0 || graf2 >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  ptr1 = g[file].grf[graf1];
  ptr2 = g[file].grf[graf2];
  if (ptr1 == NULL && ptr2 == NULL) {
    runtime_error(112);
    return;
  }
  if (ptr1 == NULL) {
    ptr1 = ptr2;
    ptr2 = NULL;
  }

  if (ptr2 == NULL)
    s = 1;
  else
    s = 2; // Scroll type: normal(1) or parallax(2)

  iscroll[snum].map1_w = ptr1[13];
  iscroll[snum].map1_h = ptr1[14];
  iscroll[snum].map1 = (byte *)ptr1 + 64 + ptr1[15] * 4;
  if (iscroll[snum].w > iscroll[snum].map1_w)
    iscroll[snum].map_flags |= 1;
  if (iscroll[snum].h > iscroll[snum].map1_h)
    iscroll[snum].map_flags |= 2;
  if (ptr1[15] == 0) {
    iscroll[snum].map1_x = 0;
    iscroll[snum].map1_y = 0;
  } else {
    iscroll[snum].map1_x = *((word *)ptr1 + 32);
    iscroll[snum].map1_y = *((word *)ptr1 + 33);
  }
  if ((iscroll[snum]._sscr1 = (byte *)malloc(iscroll[snum].w * (iscroll[snum].h + 1))) == NULL) {
    runtime_error(100);
    return;
  }
  if ((iscroll[snum].fast = (tfast *)malloc(iscroll[snum].h * sizeof(tfast))) == NULL) {
    runtime_error(100);
    return;
  }
  iscroll[snum].sscr1 = iscroll[snum]._sscr1;
  iscroll[snum].block1 = iscroll[snum].h;
  iscroll[snum].on = s;
  set_scroll(0, iscroll[snum].map1_x, iscroll[snum].map1_y);
  iscroll[snum].on = 0; // If any error (malloc) occurs, there will be no scroll

  if (s == 2) {
    iscroll[snum].map2_w = ptr2[13];
    iscroll[snum].map2_h = ptr2[14];
    iscroll[snum].map2 = (byte *)ptr2 + 64 + ptr2[15] * 4;
    if (iscroll[snum].w > iscroll[snum].map2_w)
      iscroll[snum].map_flags |= 4;
    if (iscroll[snum].h > iscroll[snum].map2_h)
      iscroll[snum].map_flags |= 8;
    if (ptr2[15] == 0) {
      iscroll[snum].map2_x = 0;
      iscroll[snum].map2_y = 0;
    } else {
      iscroll[snum].map2_x = *((word *)ptr2 + 32);
      iscroll[snum].map2_y = *((word *)ptr2 + 33);
    }
    if ((iscroll[snum]._sscr2 = (byte *)malloc(iscroll[snum].w * (iscroll[snum].h + 1))) == NULL) {
      free(iscroll[snum]._sscr1);
      free(iscroll[snum].fast);
      runtime_error(100);
      return;
    }
    iscroll[snum].sscr2 = iscroll[snum]._sscr2;
    iscroll[snum].block2 = iscroll[snum].h;
    iscroll[snum].on = 2;
    set_scroll(1, iscroll[snum].map2_x, iscroll[snum].map2_y);
  }

  iscroll[snum].on = s; // Finally, if no errors occurred, set the scroll variable

  (scroll + snum)->x0 = iscroll[snum].map1_x;
  (scroll + snum)->y0 = iscroll[snum].map1_y;
  (scroll + snum)->x1 = iscroll[snum].map2_x;
  (scroll + snum)->y1 = iscroll[snum].map2_y;
}

//----------------------------------------------------------------------------
//      Refresh_scroll(snum)
//----------------------------------------------------------------------------

void refresh_scroll(void) {
  snum = stack[sp];
  set_scroll(0, iscroll[snum].map1_x, iscroll[snum].map1_y);
  set_scroll(1, iscroll[snum].map2_x, iscroll[snum].map2_y);
}

//----------------------------------------------------------------------------
//      Move_scroll(snum) - moves the scroll automatically or manually
//----------------------------------------------------------------------------

void update_scroll(int);

void _move_scroll(void) {
  snum = stack[sp];
  if (snum < 0 || snum > 9) {
    runtime_error(107);
    return;
  }
  if (iscroll[snum].on == 1)
    update_scroll(0);
  else if (iscroll[snum].on == 2)
    update_scroll(1);
}

//----------------------------------------------------------------------------
//      Stop_scroll(snum)
//----------------------------------------------------------------------------

void stop_scroll(void) {
  snum = stack[sp];

  if (snum < 0 || snum > 9) {
    runtime_error(107);
    return;
  }

  if (iscroll[snum].on) {
    free(iscroll[snum]._sscr1);
    iscroll[snum]._sscr1 = 0;
  }
  if (iscroll[snum].on == 2) {
    free(iscroll[snum].fast);
    free(iscroll[snum]._sscr2);
    iscroll[snum].fast = 0;
    iscroll[snum]._sscr2 = 0;
  }

  iscroll[snum].on = 0;

  kill_invisible();

  (scroll + snum)->x0 = 0;
  (scroll + snum)->y0 = 0;
  (scroll + snum)->x1 = 0;
  (scroll + snum)->y1 = 0;
}

//----------------------------------------------------------------------------
//      Kill processes in scroll or mode-7 that are no longer visible
//----------------------------------------------------------------------------

void kill_process(int);

void kill_invisible(void) {
  int i, n = 0;
  for (i = id_start; i <= id_end; i += iloc_len) {
    if (mem[i + _Status]) {
      n = 0;
      if (mem[i + _Ctype] == CTYPE_SCROLL) {
        for (n = 0; n < 10; n++) {
          if (iscroll[n].on && (!mem[i + _Cnumber] || (mem[i + _Cnumber] & (1 << n)))) {
            break;
          }
        }
      }
      if (mem[i + _Ctype] == CTYPE_MODE7) {
        for (n = 0; n < 10; n++) {
          if (im7[n].on && (!mem[i + _Cnumber] || (mem[i + _Cnumber] & (1 << n)))) {
            break;
          }
        }
      }
    }
    if (n == 10)
      kill_process(i);
  }
}

//----------------------------------------------------------------------------
//      Get_id(type)
//----------------------------------------------------------------------------

void get_id(void) {
  int i, bloque;

  bloque = stack[sp];
  if (mem[id + _IdScan] == 0 || bloque != -mem[id + _BlScan]) {
    mem[id + _BlScan] = -bloque;
    i = id_init;
  } else if (mem[id + _IdScan] > id_end) {
    stack[sp] = 0;
    return;
  } else
    i = mem[id + _IdScan];
  do {
    if (i != id && mem[i + _Bloque] == bloque &&
        (mem[i + _Status] == PROC_ALIVE || mem[i + _Status] == PROC_FROZEN)) {
      mem[id + _IdScan] = i + iloc_len;
      stack[sp] = i;
      return;
    }
    i += iloc_len;
  } while (i <= id_end);
  mem[id + _IdScan] = i;
  stack[sp] = 0;
  return;
}

//----------------------------------------------------------------------------
//      Get_disx(angle,dist)
//----------------------------------------------------------------------------

void get_disx(void) {
  angle = (float)stack[sp - 1] / radian;
  stack[sp - 1] = (int)((float)cos(angle) * stack[sp]);
  sp--;
}

//----------------------------------------------------------------------------
//      Get_disy(angle,dist)
//----------------------------------------------------------------------------

void get_disy(void) {
  angle = (float)stack[sp - 1] / radian;
  stack[sp - 1] = -(int)((float)sin(angle) * stack[sp]);
  sp--;
}

//----------------------------------------------------------------------------
//      Get_angle(id);
//----------------------------------------------------------------------------

void get_angle(void) {
  bp = stack[sp];
  x = mem[bp + _X] - mem[id + _X];
  y = mem[id + _Y] - mem[bp + _Y];
  if (!x && !y)
    stack[sp] = 0;
  else
    stack[sp] = (float)atan2(y, x) * radian;
}

//----------------------------------------------------------------------------
//      Get_dist(id);
//----------------------------------------------------------------------------

void get_dist(void) {
  int n = 1;
  bp = stack[sp];
  x = mem[bp + _X] - mem[id + _X];
  y = mem[id + _Y] - mem[bp + _Y];
  while (abs(x) + abs(y) >= 46000) {
    n *= 2;
    x /= 2;
    y /= 2;
  }
  stack[sp] = sqrt(x * x + y * y) * n;
}

//----------------------------------------------------------------------------
//      Fade(%r,%g,%b,speed)
//----------------------------------------------------------------------------

void fade(void) {
  int r, g, b;
  r = stack[sp - 3];
  g = stack[sp - 2];
  b = stack[sp - 1];
  if (r < 0)
    r = 0;
  else if (r > 200)
    r = 200;
  if (g < 0)
    g = 0;
  else if (g > 200)
    g = 200;
  if (b < 0)
    b = 0;
  else if (b > 200)
    b = 200;
  dacout_r = 64 - r * 64 / 100;
  dacout_g = 64 - g * 64 / 100;
  dacout_b = 64 - b * 64 / 100;
  dacout_speed = stack[sp];

  if (now_dacout_r != dacout_r || now_dacout_g != dacout_g || now_dacout_b != dacout_b)
    fading = 1;

  sp -= 3;
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      Unload_fnt(font_code)
//----------------------------------------------------------------------------

void unload_fnt(void) {
  if (stack[sp] < 1 || stack[sp] >= max_fonts)
    return;
  if (fonts[stack[sp]] != NULL) {
    free(fonts[stack[sp]]);
    fonts[stack[sp]] = NULL;
  }
}

//----------------------------------------------------------------------------
//      Load_fnt(file) - Returns font_code
//----------------------------------------------------------------------------

void load_fnt(void) {
  byte *ptr;
  int n, w, h, nan, ifonts, m;

  for (ifonts = 1; ifonts < max_fonts; ifonts++)
    if (!fonts[ifonts])
      break;
  if (ifonts == max_fonts) {
    stack[sp] = 0;
    runtime_error(113);
    return;
  }

  if (npackfiles) {
    m = read_packfile((byte *)&mem[stack[sp]]);
    if (m == -1)
      goto fntfuera;
    if (m == -2) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    if (m <= 0) {
      stack[sp] = 0;
      runtime_error(200);
      return;
    }
    ptr = packptr;
    file_len = m;
    fonts[ifonts] = ptr;
  } else {
fntfuera:
    if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
      stack[sp] = 0;
      runtime_error(114);
      return;
    } else {
      fseek(es, 0, SEEK_END);
      file_len = ftell(es);
      if ((ptr = (byte *)malloc(file_len)) != NULL) {
        fonts[ifonts] = ptr;
        fseek(es, 0, SEEK_SET);
        fread(ptr, 1, file_len, es);
        fclose(es);
      } else {
        fclose(es);
        stack[sp] = 0;
        runtime_error(118);
        return;
      }
    }
  }

  if (strcmp((char *)ptr, "fnt\x1a\x0d\x0a")) {
    fonts[ifonts] = 0;
    runtime_error(115);
    free(ptr);
    return;
  }

  if (process_fnt != NULL)
    process_fnt((char *)ptr, file_len);
  w = 0;
  h = 0;
  nan = 0;
  fnt = (fnt_table_entry *)((byte *)ptr + FNT_TABLE_OFFSET);
  for (n = 0; n < 256; n++) {
    if (fnt[n].width) {
      w += fnt[n].width;
      nan++;
    }
    if (fnt[n].height) {
      if (fnt[n].height + fnt[n].incY > h)
        h = fnt[n].height + fnt[n].incY;
    }
  }

  ptr += 8;
  m = 0;
  for (n = 0; n < PALETTE_SIZE; n++) {
    m += (int)ptr[n];
    m <<= 1;
  }

  if (strlen((char *)&mem[stack[sp]]) < 80)
    div_strcpy(f_i[ifonts].name, sizeof(f_i[ifonts].name), (char *)&mem[stack[sp]]);
  else
    div_strcpy(f_i[ifonts].name, sizeof(f_i[ifonts].name), "");

  f_i[ifonts].len = file_len;
  f_i[ifonts].fonpal = m;
  f_i[ifonts].syspal = m;

  f_i[ifonts].width = w / nan;
  f_i[ifonts].spacing = (w / nan) / 2;
  f_i[ifonts].letter_spacing = 0;
  f_i[ifonts].height = h;
  stack[sp] = ifonts;

  if (auto_adapt_palette) {
    adapt_palette(fonts[ifonts] + FNT_TABLE_OFFSET + sizeof(fnt_table_entry) * 256,
                  f_i[ifonts].len - FNT_TABLE_OFFSET - sizeof(fnt_table_entry) * 256, fonts[ifonts] + 8, NULL);
    f_i[ifonts].syspal = palcrc;
  }

  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      Adapt a font to the system palette (uses computed "palcrc")
//----------------------------------------------------------------------------

void checkpal_font(int ifonts) {
  if (ifonts <= 0 || ifonts >= max_fonts)
    return;
  if (!fonts[ifonts])
    return;
  if (f_i[ifonts].syspal != palcrc) {
    if (f_i[ifonts].syspal != f_i[ifonts].fonpal) { // Must reload it

      if (npackfiles) {
        file_len = read_packfile((byte *)&mem[stack[sp]]);
        if (file_len == -1)
          goto fntfuera;
        if (file_len == -2)
          return;
        if (file_len <= 0)
          return;
        if (file_len != f_i[ifonts].len)
          return;
        memcpy(fonts[ifonts], packptr, file_len);
        free(packptr);
      } else {
fntfuera:
        if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL)
          return;
        else {
          fseek(es, 0, SEEK_END);
          file_len = ftell(es);
          if (file_len != f_i[ifonts].len)
            return;
          fseek(es, 0, SEEK_SET);
          fread(fonts[ifonts], 1, file_len, es);
          fclose(es);
        }
      }

      if (process_fnt != NULL)
        process_fnt((char *)fonts[ifonts], file_len);
    }

    if (f_i[ifonts].fonpal != palcrc) {
      adapt_palette(fonts[ifonts] + FNT_TABLE_OFFSET + sizeof(fnt_table_entry) * 256,
                    f_i[ifonts].len - FNT_TABLE_OFFSET - sizeof(fnt_table_entry) * 256, fonts[ifonts] + 8,
                    NULL);
    }

    f_i[ifonts].syspal = palcrc;
  }
}

//----------------------------------------------------------------------------
//      Adapt (ptr,len) where pal[] is its palette
//----------------------------------------------------------------------------

void adapt_palette(byte *ptr, int len, byte *pal, byte *xlat) {
  int n, m;
  byte _xlat[256];
  byte *endptr;

  if (auto_adapt_palette) {
    if (xlat == NULL) {
      xlat = &_xlat[0];

      for (m = 0, n = 0; n < PALETTE_SIZE; n++) {
        m += (int)pal[n];
        m <<= 1;
      }
      if (m == palcrc)
        return;

      xlat[0] = 0;

      for (n = 1; n < 256; n++) {
        find_color(pal[n * 3 + 0], pal[n * 3 + 1], pal[n * 3 + 2]);
        xlat[n] = find_col;
      }
    }

    endptr = ptr + len;
    do {
      *ptr = xlat[*ptr];
    } while (++ptr < endptr);
  }
}

//----------------------------------------------------------------------------
//      Write(font,x,y,alignment,ptr)
//----------------------------------------------------------------------------

void __write(void) {
  int f = stack[sp - 4];

  if (f < 0 || f >= max_fonts) {
    runtime_error(116);
    f = 0;
  }
  if (fonts[f] == 0) {
    runtime_error(116);
    f = 0;
  }
  x = 1;
  while (texts[x].font != NULL) {
    x++;
    if (x == MAX_TEXTS)
      break;
    if (stack[sp - 1] == texts[x].alignment && stack[sp - 2] == texts[x].y &&
        stack[sp - 3] == texts[x].x)
      break;
  }

  if (x < MAX_TEXTS) {
    texts[x].type = 0;
    texts[x].ptr = stack[sp--];
    if (stack[sp] < 0 || stack[sp] > 8) {
      runtime_error(117);
      stack[sp] = 0;
    }
    texts[x].alignment = stack[sp--];
    texts[x].y = stack[sp--];
    texts[x].x = stack[sp--];
    texts[x].font = (byte *)fonts[f];
    stack[sp] = x;
  } else {
    sp -= 4;
    stack[sp] = 0;
    runtime_error(118);
  }
}

//----------------------------------------------------------------------------
//      Write_int(font,x,y,alignment,&num)
//----------------------------------------------------------------------------

void write_int(void) {
  int f = stack[sp - 4];
  if (f < 0 || f >= max_fonts) {
    runtime_error(116);
    f = 0;
  }
  if (fonts[f] == 0) {
    runtime_error(116);
    f = 0;
  }
  x = 1;
  while (texts[x].font) {
    x++;
    if (x == MAX_TEXTS)
      break;
    if (stack[sp - 1] == texts[x].alignment && stack[sp - 2] == texts[x].y &&
        stack[sp - 3] == texts[x].x)
      break;
  }
  if (x < MAX_TEXTS) {
    texts[x].type = 1;
    texts[x].ptr = stack[sp--];
    if (stack[sp] < 0 || stack[sp] > 8) {
      runtime_error(117);
      stack[sp] = 0;
    }
    texts[x].alignment = stack[sp--];
    texts[x].y = stack[sp--];
    texts[x].x = stack[sp--];
    texts[x].font = (byte *)fonts[f];
    stack[sp] = x;
  } else {
    sp -= 4;
    stack[sp] = 0;
    runtime_error(118);
  }
}

//----------------------------------------------------------------------------
//      Delete_text(t_id or all_text)
//----------------------------------------------------------------------------

void delete_text(void) {
  x = stack[sp];
  if (x < MAX_TEXTS && x > 0)
    texts[x].font = 0;
  else if (x == 0) {
    x = 1;
    do
      texts[x++].font = 0;
    while (x < MAX_TEXTS);
  } else
    runtime_error(119);
}

//----------------------------------------------------------------------------
//      Move_text(t_id,x,y)
//----------------------------------------------------------------------------

void move_text(void) {
  x = stack[sp - 2];
  if (x < MAX_TEXTS && x > 0) {
    texts[x].x = stack[sp - 1];
    texts[x].y = stack[sp];
  } else
    runtime_error(119);
  sp -= 2;
}

//----------------------------------------------------------------------------
//      Unload_fpg(code) - Returns the file code 0..max_fpgs
//----------------------------------------------------------------------------

void unload_fpg(void) {
  int c;
  c = stack[sp];
  stack[sp] = 0;
  if (c < max_fpgs && c >= 0) {
    if (g[c].fpg != 0) {
      free(g[c].fpg);
      g[c].fpg = 0;
    } else
      runtime_error(109);
    if (g[c].grf != 0) {
      if (c) {
        free(g[c].grf);
        g[c].grf = 0;
      } else {
        memset(g[c].grf, 0, sizeof(int *) * MAX_FPG_GRAPHICS);
      }
    }
  } else
    runtime_error(109);
}

//----------------------------------------------------------------------------
//      Rand(min,max)
//----------------------------------------------------------------------------

union {
  byte b[128];
  int d[32];
} seed; // Random seed (127 bytes + PTR)

byte rnd(void) {
  byte ptr;
  ptr = seed.b[127];
  if ((seed.b[127] = (ptr + seed.b[ptr]) & 127) == 127)
    seed.b[127] = 0;
  return (seed.b[seed.b[127]] += ++ptr);
}

void divrandom(void) {
  int min, max;
  max = stack[sp--];
  min = stack[sp];
  stack[sp] = _random(min, max);
}

int _random(int min, int max) {
  int r;
  if (max < min)
    swap(max, min);
  r = (((rnd() & 127) * 256 + rnd()) * 256 + rnd()) * 256 + rnd();
  if (min < -2147483640 && max > 2147483640)
    return (r);
  else
    return ((r % (max - min + 1)) + min);
}

//----------------------------------------------------------------------------
//      Rand_seed(n)
//----------------------------------------------------------------------------

void init_rnd(int n) {
  int a;
  for (a = 0; a < 32; a++)
    seed.d[a] = n;
  for (a = 0; a < 2048; a++)
    rnd();
}

void rand_seed(void) {
  init_rnd(stack[sp]);
}

//----------------------------------------------------------------------------
//      Define_region(n,x,y,width,height)
//----------------------------------------------------------------------------

void define_region(void) {
  int n, x, y, w, h;

  h = stack[sp--];
  w = stack[sp--];
  y = stack[sp--];
  x = stack[sp--];

  n = stack[sp];

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;
  if (w < 0 || h < 0) {
    runtime_error(120);
    return;
  }

  if (n >= 0 && n < max_region) {
    region[n].x0 = x;
    region[n].y0 = y;
    region[n].x1 = x + w;
    region[n].y1 = y + h;
    stack[sp] = 1;
  } else {
    stack[sp] = 0;
    runtime_error(108);
  }
}

//----------------------------------------------------------------------------
//      Xput(file,graf,x,y,angle,size,flags,region)
//----------------------------------------------------------------------------

void _xput(void) {
  int file, graf, x, y, angle, size, flags, reg;

  reg = stack[sp--];
  flags = stack[sp--];
  size = stack[sp--];
  angle = stack[sp--];
  y = stack[sp--];
  x = stack[sp--];
  graf = stack[sp--];
  file = stack[sp];

  put_sprite(file, graf, x, y, angle, size, flags, reg, back_buffer, vga_width, vga_height);
}

//----------------------------------------------------------------------------
//      Put(file,graf,x,y)
//----------------------------------------------------------------------------

void _put(void) {
  int file, graf, x, y;

  y = stack[sp--];
  x = stack[sp--];
  graf = stack[sp--];
  file = stack[sp];

  put_sprite(file, graf, x, y, 0, 100, 0, 0, back_buffer, vga_width, vga_height);
}

//----------------------------------------------------------------------------
//      Map_xput(file,graf1,graf2,x,y,angle,size,flags)
//----------------------------------------------------------------------------

void map_xput(void) {
  int file, graf1, graf2, x, y, angle, size, flags;
  int *ptr;

  flags = stack[sp--];
  size = stack[sp--];
  angle = stack[sp--];
  y = stack[sp--];
  x = stack[sp--];
  graf2 = stack[sp--];
  graf1 = stack[sp--];
  file = stack[sp];

  if (file > max_fpgs || file < 0) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf1 <= 0 || graf1 >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }

  if ((ptr = g[file].grf[graf1]) != NULL) {
    put_sprite(file, graf2, x, y, angle, size, flags, -1, (byte *)ptr + 64 + ptr[15] * 4, ptr[13],
               ptr[14]);
  } else
    runtime_error(121);
}

//----------------------------------------------------------------------------
//      Map_put(file,graf1,graf2,x,y)
//----------------------------------------------------------------------------

void map_put(void) {
  int file, graf1, graf2, x, y;
  int *ptr;

  y = stack[sp--];
  x = stack[sp--];
  graf2 = stack[sp--];
  graf1 = stack[sp--];
  file = stack[sp];

  if (file > max_fpgs || file < 0) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf1 <= 0 || graf1 >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }

  if ((ptr = g[file].grf[graf1]) != NULL) {
    put_sprite(file, graf2, x, y, 0, 100, 0, -1, (byte *)ptr + 64 + ptr[15] * 4, ptr[13], ptr[14]);
  } else
    runtime_error(121);
}

//----------------------------------------------------------------------------
//      Map_block_copy(file,graf_dest,x_dest,y_dest,graf,x,y,width,height)
//----------------------------------------------------------------------------

void map_block_copy(void) {
  int file, grafd, xd, yd;
  int graf, x, y, w, h;
  int *ptrd, *ptr;
  byte *_saved_buffer = screen_buffer, *si;
  int _saved_width = vga_width, _saved_height = vga_height;

  h = stack[sp--];
  w = stack[sp--];
  y = stack[sp--];
  x = stack[sp--];
  graf = stack[sp--];
  yd = stack[sp--];
  xd = stack[sp--];
  grafd = stack[sp--];
  file = stack[sp];

  if (file > max_fpgs || file < 0) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (grafd <= 0 || grafd >= max_grf) {
    runtime_error(110);
    return;
  }
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }

  if ((ptrd = g[file].grf[grafd]) != NULL) {
    if ((ptr = g[file].grf[graf]) != NULL) {
      vga_width = ptrd[13];
      vga_height = ptrd[14];
      screen_buffer = (byte *)ptrd + 64 + ptrd[15] * 4;

      if (xd > 0)
        clipx0 = xd;
      else
        clipx0 = 0;
      if (yd > 0)
        clipy0 = yd;
      else
        clipy0 = 0;
      if (xd + w < vga_width)
        clipx1 = xd + w;
      else
        clipx1 = vga_width;
      if (yd + h < vga_height)
        clipy1 = yd + h;
      else
        clipy1 = vga_height;

      if (clipx0 >= vga_width || clipx1 <= 0)
        goto no;
      if (clipy0 >= vga_height || clipy1 <= 0)
        goto no;
      if (clipx0 >= clipx1 || clipy0 >= clipy1)
        goto no;

      w = ptr[13];
      h = ptr[14];
      si = (byte *)ptr + 64 + ptr[15] * 4;
      x = xd - x;
      y = yd - y;

      if (x >= clipx0 && x + w <= clipx1 && y >= clipy0 && y + h <= clipy1) // Draw sprite unclipped
        sp_normal(si, x, y, w, h, 0);
      else if (x < clipx1 && y < clipy1 && x + w > clipx0 && y + h > clipy0) // Draw sprite clipped
        sp_clipped(si, x, y, w, h, 0);

no:
      screen_buffer = _saved_buffer;
      vga_width = _saved_width;
      vga_height = _saved_height;
    } else
      runtime_error(121);
  } else
    runtime_error(121);
}

//----------------------------------------------------------------------------
//      Screen_copy(region,file,graf,x,y,width,height)
//      (scaled reverse-blit from a screen region to a graphic)
//----------------------------------------------------------------------------

void screen_copy(void) {
  int reg, file, graf;
  int w, h, divand, ald;
  int *ptr;
  int xr, ixr, yr, iyr;
  byte *old_si, *si, *di;

  ald = stack[sp--];
  divand = stack[sp--];
  yr = stack[sp--];
  xr = stack[sp--];
  graf = stack[sp--];
  file = stack[sp--];
  reg = stack[sp];

  if (reg >= 0 && reg < max_region) {
    w = region[reg].x1 - region[reg].x0;
    h = region[reg].y1 - region[reg].y0;
  } else {
    runtime_error(108);
    return;
  }

  if (file > max_fpgs || file < 0) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }

  if ((ptr = g[file].grf[graf]) == NULL) {
    runtime_error(121);
    return;
  }

  if (xr < 0)
    xr = 0;
  if (yr < 0)
    yr = 0;
  if (xr + divand > ptr[13])
    divand = ptr[13] - xr;
  if (yr + ald > ptr[14])
    ald = ptr[14] - yr;
  if (divand <= 0 || ald <= 0 || w <= 0 || h <= 0)
    return;

  di = (byte *)ptr + 64 + ptr[15] * 4 + xr + yr * ptr[13];
  old_si = screen_buffer + region[reg].x0 + region[reg].y0 * vga_width;

  ixr = (float)(w * 256) / (float)divand;
  iyr = (float)(h * 256) / (float)ald;

  w = divand;
  yr = 0;

  do {
    si = old_si + (yr >> 8) * vga_width;
    xr = 0;
    do {
      *di = *(si + (xr >> 8));
      di++;
      xr += ixr;
    } while (--w);
    yr += iyr;
    di += ptr[13] - (w = divand);
  } while (--ald);
}

//----------------------------------------------------------------------------
//      Load_screen(filename)
//----------------------------------------------------------------------------

void load_screen(void) {
  load_map(); // filename
  stack[sp + 1] = stack[sp];
  stack[sp++] = 0;
  put_screen(); // file,graf
  sp++;
  unload_map(); // graf
  stack[--sp] = 0;
}

//----------------------------------------------------------------------------
//      Put_screen(file,graf)
//----------------------------------------------------------------------------

void put_screen(void) {
  int file, graf;
  short xg, yg;
  int *ptr;

  graf = stack[sp--];
  file = stack[sp];

  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[file].grf[graf]) == NULL) {
    runtime_error(121);
    return;
  }

  if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
    xg = ptr[13] / 2;
    yg = ptr[14] / 2;
  } else {
    xg = *((word *)ptr + 32);
    yg = *((word *)ptr + 33);
  }

  memset(back_buffer, 0, vga_width * vga_height);
  put_sprite(file, graf, xg, yg, 0, 100, 0, 0, back_buffer, vga_width, vga_height);
}

//----------------------------------------------------------------------------
//      Put_pixel(x,y,color)
//----------------------------------------------------------------------------

void put_pixel(void) {
  int x, y, color;

  color = stack[sp--];
  y = stack[sp--];
  x = stack[sp];
  if (x >= 0 && y >= 0 && x < vga_width && y < vga_height) {
    *(back_buffer + x + y * vga_width) = color;
  }
}

//----------------------------------------------------------------------------
//      Get_pixel(x,y)
//----------------------------------------------------------------------------

void get_pixel(void) {
  int x, y;

  y = stack[sp--];
  x = stack[sp];
  if (x >= 0 && y >= 0 && x < vga_width && y < vga_height) {
    stack[sp] = (int)(*(back_buffer + x + y * vga_width));
  } else
    stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      Map_put_pixel(file,graf,x,y,color)
//----------------------------------------------------------------------------

void map_put_pixel(void) {
  int file, graf, x, y, color;
  int *ptr;
  byte *si;

  color = stack[sp--];
  y = stack[sp--];
  x = stack[sp--];
  graf = stack[sp--];
  file = stack[sp];

  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[file].grf[graf]) == NULL) {
    runtime_error(121);
    return;
  }

  if (x >= 0 && y >= 0 && x < ptr[13] && y < ptr[14]) {
    si = (byte *)ptr + 64 + ptr[15] * 4;
    *(si + x + y * ptr[13]) = color;
  }
}

//----------------------------------------------------------------------------
//      Map_get_pixel(file,graf,x,y)
//----------------------------------------------------------------------------

void map_get_pixel(void) {
  int file, graf, x, y;
  int *ptr;
  byte *si;

  y = stack[sp--];
  x = stack[sp--];
  graf = stack[sp--];
  file = stack[sp];

  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[file].grf[graf]) == NULL) {
    runtime_error(121);
    return;
  }

  if (x >= 0 && y >= 0 && x < ptr[13] && y < ptr[14]) {
    si = (byte *)ptr + 64 + ptr[15] * 4;
    stack[sp] = (int)(*(si + x + y * ptr[13]));
  } else
    stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      Get_point(file,graf,n,&x,&y)
//----------------------------------------------------------------------------

void get_point(void) {
  int file, graf, n, dx, dy;
  int *ptr;
  short *p;

  dy = stack[sp--];
  dx = stack[sp--];
  n = stack[sp--];
  graf = stack[sp--];
  file = stack[sp];
  stack[sp] = 0;

  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[file].grf[graf]) == NULL) {
    runtime_error(121);
    return;
  }

  if (n >= 0 || n < ptr[15]) {
    p = (short *)&ptr[16];
    mem[dx] = p[n * 2];
    mem[dy] = p[n * 2 + 1];
  }
}

//----------------------------------------------------------------------------
//      Clear_screen()
//----------------------------------------------------------------------------

void clear_screen(void) {
  memset(back_buffer, 0, vga_width * vga_height);
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      Save(file,offset,length)
//----------------------------------------------------------------------------

#ifdef DEBUG // Version with debugger.

FILE *open_save_file(byte *file) {
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  f = open_multi((char *)file, "wb");
  return f;
}

#else // Release version.

FILE *open_save_file(byte *file) {
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  char *ff = (char *)file;

  packfile_del((char *)file);

  while (*ff != 0) {
    if (*ff == '\\')
      *ff = '/';
    ff++;
  }

  printf("Looking for save file: %s\n", (char *)file);

  f = open_multi((char *)file, "wb");
  return f;
}

#endif

void save(void) {
  int offset, lon;
  int llon;

  if (unit_size < 1)
    unit_size = 1;

  lon = stack[sp--];
  offset = stack[sp--];
  lon = lon * unit_size;

  if (!validate_address(offset) || !validate_address(offset + lon)) {
    stack[sp] = 0;
    runtime_error(122);
    return;
  }
  es = open_save_file((byte *)&mem[stack[sp]]);
  if (es == NULL) {
    stack[sp] = 0;
    runtime_error(123);
    return;
  }

  llon = (int)fwrite(&mem[offset], 1, lon, es);

  fclose(es);

  if (lon != llon) //*unit_size)
    runtime_error(124);

  max_reloj += get_clock() - old_clock;
}

void _save(void) {
  int offset, lon;
  int llon;

  if (unit_size < 1)
    unit_size = 1;
  lon = stack[sp--];
  offset = stack[sp--];
  if (offset < HEADER_LENGTH || offset + lon > imem_max) {
    stack[sp] = 0;
    runtime_error(122);
    return;
  }
  es = open_save_file((byte *)&mem[stack[sp]]);
  if (es == NULL) {
    stack[sp] = 0;
    // Save failure is silently ignored (file may be on read-only media)
    return;
  }
  //  if (fwrite(&mem[offset],unit_size,lon,es)!=lon) runtime_error(124);
  llon = fwrite(&mem[offset], unit_size, lon, es);
  fclose(es);

  if (llon != lon)
    runtime_error(124);
  max_reloj += get_clock() - old_clock;
}


//----------------------------------------------------------------------------
//      Load(file,offset)
//----------------------------------------------------------------------------

void load(void) {
  int offset = 0, lon = 0;
  int fbytes = 0;

  if (unit_size < 1)
    unit_size = 1;

  offset = stack[sp--];
  if (!validate_address(offset)) {
    stack[sp] = 0;
    runtime_error(125);
    return;
  }
  //fprintf(stdout, "loading data from: %s\n",(byte*)&mem[stack[sp]]);

  if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
    // if not found, check pak
    // this way files override paks
    lon = read_packfile((byte *)&mem[stack[sp]]);

    if (lon > 0) {
      if (!validate_address(offset + lon)) {
        stack[sp] = 0;
        runtime_error(125);
        return;
      }
      memcpy(&mem[offset], packptr, lon);
      max_reloj += get_clock() - old_clock;
      return;
    }

    stack[sp] = 0;
#ifdef DEBUG
    runtime_error(126);
#endif
    return;
  }

  //fprintf(stdout, "File loaded: %s\n", full);

  fseek(es, 0, SEEK_END);
  lon = ftell(es); ///4;
  printf("file len: %ld\n", ftell(es));
  fseek(es, 0, SEEK_SET);
  if (!validate_address(offset + lon)) {
    stack[sp] = 0;
    runtime_error(125);
    return;
  }
  lon = lon / unit_size;
  fbytes = fread(&mem[offset], unit_size, lon, es);
  if (fbytes != lon) {
    //fprintf(stdout,"Bytes read: %d bytes wanted: %d len: %d unit_size: %d\n",fbytes, lon*unit_size, lon, unit_size);
    runtime_error(127);
  }
  fclose(es);
  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      Set_mode(mode)
//----------------------------------------------------------------------------

void set_mode(void) {
  int n;

#ifdef DEBUG
  if (v.type)
    new_mode = 1; // Notify the debugger of a video mode change
#endif

  vga_width = stack[sp] / 1000;
  vga_height = stack[sp] % 1000;
  //	printf("Tring to set mode %dx%d\n",vga_width,vga_height);

  // nonsense ?
  for (n = 0; n < num_video_modes; n++) {
    if (stack[sp] == video_modes[n].mode) {
      vga_width = video_modes[n].width;
      vga_height = video_modes[n].height;
      break;
    }
  }


  vvga_w = vga_width;
  vvga_h = vga_height;


  dacout_r = 64;
  dacout_g = 64;
  dacout_b = 64;
  dacout_speed = 8;
  fade_wait();

  if (screen_buffer != NULL) {
    free(screen_buffer);
    screen_buffer = NULL;
  }

  if (back_buffer != NULL) {
    free(back_buffer);
    back_buffer = NULL;
  }

#ifdef DEBUG
  if (screen_buffer_debug != NULL) {
    free(screen_buffer_debug);
    screen_buffer_debug = NULL;
  }
#endif

  if ((screen_buffer = (byte *)malloc(vga_width * vga_height)) == NULL)
    exer(1);
  memset(screen_buffer, 0, vga_width * vga_height);

  if ((back_buffer = (byte *)malloc(vga_width * vga_height)) == NULL)
    exer(1);
  memset(back_buffer, 0, vga_width * vga_height);

#ifdef DEBUG
  if ((screen_buffer_debug = (byte *)malloc(vga_width * vga_height)) == NULL)
    exer(1);
  memset(screen_buffer_debug, 0, vga_width * vga_height);
#endif

  if (set_video_mode != NULL) {
    set_video_mode();
  } else {
    setup_video_mode();
  }
  OSDEP_SetWindowSize(vga_width, vga_height);

  dacout_speed = 0;
  set_dac();

  for (n = 0; n < 10; n++) {
    if (iscroll[n].on) {
      stack[sp] = n;
      stop_scroll();
      iscroll[n].on = 0;
    }
    if (im7[n].on) {
      stack[sp] = n;
      stop_mode7();
      iscroll[n].on = 0;
    }
  }
  stack[sp] = 0;

  region[0].x0 = 0;
  region[0].y0 = 0;
  region[0].x1 = vga_width;
  region[0].y1 = vga_height;

#ifdef DEBUG
  init_big();
#endif

  fade_on();
  sp--;

  if (vwidth < vga_width || vheight < vga_height) {
    vwidth = vga_width;
    vheight = vga_height;
  }
}

//----------------------------------------------------------------------------
//      Load_pcm(file,loop)
//----------------------------------------------------------------------------

void load_pcm(void) {
  int loop, m;
  char *ptr;

  loop = stack[sp--];

  if (npackfiles) {
    m = read_packfile((byte *)&mem[stack[sp]]);
    if (m == -1)
      goto pcmfuera;
    if (m == -2) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    if (m <= 0) {
      stack[sp] = 0;
      runtime_error(200);
      return;
    }
    ptr = (char *)packptr;
    file_len = m;
  } else {
pcmfuera:
    if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
      stack[sp] = -1;
      runtime_error(128);
      return;
    } else {
      fseek(es, 0, SEEK_END);
      file_len = ftell(es);
      if ((ptr = (char *)malloc(file_len)) != NULL) {
        fseek(es, 0, SEEK_SET);
        fread(ptr, 1, file_len, es);
        fclose(es);
      } else {
        fclose(es);
        stack[sp] = 0;
        runtime_error(100);
        return;
      }
    }
  }

  stack[sp] = sound_load(ptr, file_len, loop);

  free(ptr);

  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      Unload_pcm(sound_id)
//----------------------------------------------------------------------------

void unload_pcm(void) {
  sound_unload(stack[sp]);
}

//----------------------------------------------------------------------------
//      Sound(sound_id,volume,frequency)
//----------------------------------------------------------------------------

void _sound(void) {
  int vol, fre;
  fre = stack[sp--];
  vol = stack[sp--];
  if (vol < 0)
    vol = 0;
  else if (vol > 511)
    vol = 511;
  if (fre < 8)
    fre = 8;
  if (fre) {
#ifdef MIXER
    stack[sp] = sound_play(stack[sp], vol, fre) + 1;
//	printf("New sound on channel %d\n",stack[sp]);
#else
    stack[sp] = 0;
#endif
  }
  // if (stack[sp]==-1) runtime_error(129);
}

//----------------------------------------------------------------------------
//      Stop_sound(channel_id)
//----------------------------------------------------------------------------

extern int music_channels;

void stop_sound(void) {
#ifdef MIXER
  int x;
  if (stack[sp] == -1) {
    for (x = 0; x < CHANNELS; x++)
      sound_stop(x);
  } else {
    sound_stop(stack[sp] - 1);
  }
#endif
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      Change_sound(channel_id,volume,frequency)
//----------------------------------------------------------------------------

void change_sound(void) {
  int vol, fre;
  fre = stack[sp--];
  vol = stack[sp--];
  if (vol < 0)
    vol = 0;
  else if (vol > 511)
    vol = 511;
  if (fre < 8)
    fre = 8;
  sound_change(stack[sp] - 1, vol, fre);
}

//----------------------------------------------------------------------------
//      Change_channel(channel_id,volume,panning)
//----------------------------------------------------------------------------

void change_channel(void) {
  int vol, pan;
  pan = stack[sp--];
  vol = stack[sp--];
  if (vol < 0)
    vol = 0;
  else if (vol > 511)
    vol = 511;
  if (pan < 0)
    pan = 0;
  else if (pan > 255)
    pan = 255;
  sound_change_channel(stack[sp] - 1, vol, pan);
}

//----------------------------------------------------------------------------
//      load_song(file,loop)
//----------------------------------------------------------------------------

void load_song(void) {
  int loop, m;
  char *ptr;

  loop = stack[sp--];
  if (npackfiles) {
    m = read_packfile((byte *)&mem[stack[sp]]);
    if (m == -1)
      goto songfuera;
    if (m == -2) {
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    if (m <= 0) {
      stack[sp] = 0;
      runtime_error(200);
      return;
    }
    ptr = (char *)packptr;
    file_len = m;
  } else {
songfuera:
    if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
      stack[sp] = -1;
      runtime_error(167);
      return;
    } else {
      fseek(es, 0, SEEK_END);
      file_len = ftell(es);
      if ((ptr = (char *)malloc(file_len)) != NULL) {
        fseek(es, 0, SEEK_SET);
        fread(ptr, 1, file_len, es);
        fclose(es);
      } else {
        fclose(es);
        stack[sp] = 0;
        runtime_error(100);
        return;
      }
    }
  }
  stack[sp] = sound_load_song(ptr, file_len, loop);

  free(ptr);

  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      unload_song(song_id)
//----------------------------------------------------------------------------

void unload_song(void) {
  sound_unload_song(stack[sp]);
}

//----------------------------------------------------------------------------
//      song(song_id)
//----------------------------------------------------------------------------

void song(void) {
  sound_play_song(stack[sp]);
}

//----------------------------------------------------------------------------
//      stop_song()
//----------------------------------------------------------------------------

void stop_song(void) {
  sound_stop_song();
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      set_song_pos(pattern)
//----------------------------------------------------------------------------

void set_song_pos(void) {
  sound_set_song_pos(stack[sp]);
}

//----------------------------------------------------------------------------
//      get_song_pos()
//----------------------------------------------------------------------------

void get_song_pos(void) {
  stack[++sp] = sound_get_song_pos();
}

//----------------------------------------------------------------------------
//      get_song_line()
//----------------------------------------------------------------------------

void get_song_line(void) {
  stack[++sp] = sound_get_song_line();
}

//----------------------------------------------------------------------------
//      is_playing_sound(channel_id)
//----------------------------------------------------------------------------

void is_playing_sound(void) {
  stack[sp] = sound_is_playing(stack[sp] - 1);
}

//----------------------------------------------------------------------------
//      is_playing_song()
//----------------------------------------------------------------------------

void is_playing_song(void) {
  stack[++sp] = sound_is_playing_song();
}

//----------------------------------------------------------------------------
//      Set_fps(fps, max frame skips)
//----------------------------------------------------------------------------
void runtime_main_loop(void);

void set_fps(void) {
  max_frame_skips = stack[sp--];
  if (max_frame_skips < 0)
    max_frame_skips = 0;
  if (max_frame_skips > 10)
    max_frame_skips = 10;
  if (stack[sp] < 4)
    stack[sp] = 4;
  if (stack[sp] > 999)
    stack[sp] = 999;
  dfps = stack[sp];
  clock_interval = 1000.0 / (double)stack[sp];
}

//----------------------------------------------------------------------------
//      Start_fli("file",x,y) returns number of frames
//----------------------------------------------------------------------------

void start_fli(void) {
  int x, y;
  y = stack[sp--];
  x = stack[sp--];

#ifdef USE_FLI
  if ((es = div_open_file((char *)&mem[stack[sp]])) == NULL) {
    stack[sp] = 0;
    runtime_error(147);
  } else {
    fclose(es);
    stack[sp] = fli_start(full, (char *)back_buffer, vga_width, vga_height, x, y);
    if (stack[sp] == 0)
      runtime_error(130);
  }
#endif

  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      Frame_fli() returns 0-end of fli, 1-continues
//----------------------------------------------------------------------------

void frame_fli(void) {
  stack[++sp] = fli_next_frame();
}

//----------------------------------------------------------------------------
//      End_fli()
//----------------------------------------------------------------------------

void end_fli(void) {
#ifdef USE_FLI
  fli_end();
#endif
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      Reset_fli()
//----------------------------------------------------------------------------

void reset_fli(void) {
#ifdef USE_FLI
  fli_reset();
#endif
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      System("command")
//----------------------------------------------------------------------------

void _system(void) {
  char cwork[256];
  unsigned n;
#ifdef STDOUTLOG
  printf("system call not implemented yet\n");
#endif

  return;

  if (system(NULL)) {
    if (!strcmp(strupr((char *)&mem[stack[sp]]), "COMMAND.COM")) {
      getcwd(cwork, 256);
      sound_end();
      sound_init();
      set_mixer();
      _dos_setdrive((int)toupper(*cwork) - 'A' + 1, &n);
      chdir(cwork);
      setup_video_mode();
      set_dac();
      set_mouse(mouse->x, mouse->y);
      read_mouse();
      full_redraw = 1;
    } else {
      system((char *)&mem[stack[sp]]);
    }
  }
}

//----------------------------------------------------------------------------
//      Fget_dist(x0,y0,x1,y1);
//----------------------------------------------------------------------------

void fget_dist(void) {
  int x0, y0, x1, y1, n = 1;
  y1 = stack[sp--];
  x1 = stack[sp--];
  y0 = stack[sp--];
  x0 = stack[sp];
  x0 = abs(x1 - x0);
  y0 = abs(y1 - y0);
  while (x0 + y0 >= 46000) {
    n *= 2;
    x0 /= 2;
    y0 /= 2;
  }
  stack[sp] = sqrt(x0 * x0 + y0 * y0) * n;
}

//----------------------------------------------------------------------------
//      Fget_angle(x0,y0,x1,y1);
//----------------------------------------------------------------------------


void fget_angle(void) {
  int x0, y0, x1, y1;
  y1 = stack[sp--];
  x1 = stack[sp--];
  y0 = stack[sp--];
  x0 = stack[sp];
  x0 = x1 - x0;
  y0 = y0 - y1;
  if (!x0 && !y0)
    stack[sp] = 0;
  else
    stack[sp] = (float)atan2(y0, x0) * radian;
}

// CD function stubs removed (CDDA deleted)

//----------------------------------------------------------------------------
//      Start_mode7(n,file,graf1,graf2,region,horizon)
//----------------------------------------------------------------------------

void start_mode7(void) {
  int n, m, file, graf1, graf2, reg;
  int *ptr1, *ptr2;

  m = stack[sp--];
  reg = stack[sp--];
  graf2 = stack[sp--];
  graf1 = stack[sp--];
  file = stack[sp--];
  n = stack[sp];
  stack[sp] = 0;

  if (n < 0 || n > 9) {
    runtime_error(131);
    return;
  }
  (m7 + n)->horizon = m;
  im7[n].map = NULL;
  im7[n].ext = NULL;

  if (im7[n].on) {
    stack[sp] = n;
    stop_mode7();
  }

  if (reg >= 0 && reg < max_region) {
    im7[n].x = region[reg].x0;
    im7[n].y = region[reg].y0;
    im7[n].w = region[reg].x1 - region[reg].x0;
    im7[n].h = region[reg].y1 - region[reg].y0;
  } else {
    runtime_error(108);
    return;
  }

  if (im7[n].w == 0 || im7[n].h == 0) {
    runtime_error(146);
    return;
  }

  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf1 < 0 || graf1 >= max_grf) {
    runtime_error(110);
    return;
  }
  if (graf2 < 0 || graf2 >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  ptr1 = g[file].grf[graf1];
  ptr2 = g[file].grf[graf2];
  if (ptr1 == NULL && ptr2 == NULL) {
    runtime_error(132);
    return;
  }
  if (ptr1 == NULL) {
    ptr1 = ptr2;
    ptr2 = NULL;
  }

  im7[n].map_width = ptr1[13];
  im7[n].map_height = ptr1[14];
  im7[n].map = (byte *)ptr1 + 64 + ptr1[15] * 4;

  if (ptr2 != NULL) {
    im7[n].ext_w = ptr2[13];
    im7[n].ext_h = ptr2[14];
  } else
    im7[n].ext_w = 0;

  switch (im7[n].ext_w) {
  case 1:
  case 2:
  case 4:
  case 8:
  case 16:
  case 32:
  case 64:
  case 128:
  case 256:
  case 512:
  case 1024:
  case 2048:
  case 4096:
  case 8192:
    break;
  default:
    im7[n].ext_w = 0;
  }

  switch (im7[n].ext_h) {
  case 1:
  case 2:
  case 4:
  case 8:
  case 16:
  case 32:
  case 64:
  case 128:
  case 256:
  case 512:
  case 1024:
  case 2048:
  case 4096:
  case 8192:
    break;
  default:
    im7[n].ext_h = 0;
  }

  if (im7[n].ext_w && im7[n].ext_h)
    im7[n].ext = (byte *)ptr2 + 64 + ptr2[15] * 4;
  else
    im7[n].ext = NULL;

  im7[n].on = 1; // Finally, if no errors occurred, set the m7 variable
}

//----------------------------------------------------------------------------
//      Stop_mode7(n)
//----------------------------------------------------------------------------

void stop_mode7(void) {
  int n = stack[sp];

  if (n < 0 || n > 9) {
    runtime_error(131);
    return;
  }
  im7[n].on = 0;
  im7[n].map = NULL;
  im7[n].ext = NULL;
  kill_invisible();
}

//----------------------------------------------------------------------------
//      Advance(dist)
//----------------------------------------------------------------------------

void advance(void) {
  mem[id + _X] += get_distx(mem[id + _Angle], stack[sp]);
  mem[id + _Y] += get_disty(mem[id + _Angle], stack[sp]);
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      X_Advance(angle,dist)
//----------------------------------------------------------------------------

void x_advance(void) {
  int distancia = stack[sp--];

  mem[id + _X] += get_distx(stack[sp], distancia);
  mem[id + _Y] += get_disty(stack[sp], distancia);
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      Abs(value)
//----------------------------------------------------------------------------

void _abs(void) {
  stack[sp] = abs(stack[sp]);
}

//----------------------------------------------------------------------------
//      Fade_On()
//----------------------------------------------------------------------------

void fade_on(void) {
  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 8;
  stack[++sp] = 0;
  if (now_dacout_r != dacout_r || now_dacout_g != dacout_g || now_dacout_b != dacout_b)
    fading = 1;
}

//----------------------------------------------------------------------------
//      Fade_Off()
//----------------------------------------------------------------------------

void fade_off(void) {
  dacout_r = 64;
  dacout_g = 64;
  dacout_b = 64;
  dacout_speed = 8;
  fade_wait();
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      Sqrt(n)
//----------------------------------------------------------------------------

void _sqrt(void) {
  int x = abs(stack[sp]);
  if (x >= 0)
    stack[sp] = sqrt(x);
  else
    stack[sp] = 999999999;
}

//----------------------------------------------------------------------------
//      Pow(n,m)
//----------------------------------------------------------------------------

void _pow(void) {
  int n, m;
  m = stack[sp--];
  n = stack[sp];
  if (m > 1) {
    m--;
    do {
      stack[sp] *= n;
    } while (--m);
  } else if (m == 0) {
    stack[sp] = 1;
  } else if (m < 0) {
    stack[sp] = 0;
  }
}

//----------------------------------------------------------------------------
//      Near_angle(angle1,angle2,angle_inc) - returns "new_angle1"
//----------------------------------------------------------------------------

void near_angle(void) {
  int a1, a2, i;
  i = abs(stack[sp--]);
  a2 = stack[sp--];
  a1 = stack[sp];
  while (a1 < a2 - pi)
    a1 += 2 * pi;
  while (a1 > a2 + pi)
    a1 -= 2 * pi;
  if (a1 < a2) {
    a1 += i;
    if (a1 > a2)
      a1 = a2;
  } else {
    a1 -= i;
    if (a1 < a2)
      a1 = a2;
  }
  stack[sp] = a1;
}

//----------------------------------------------------------------------------
//      Let_me_alone()
//----------------------------------------------------------------------------

void let_me_alone(void) {
  int i;
  for (i = id_start; i <= id_end; i += iloc_len)
    if (i != id && mem[i + _Status])
      mem[i + _Status] = PROC_KILLED;
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      Exit("message",error_level)
//----------------------------------------------------------------------------

void _exit_dos(void) {
#ifdef DEBUG
  FILE *f;
#endif
  reset_video_mode();
  kbd_reset();

#ifdef DEBUG
  if ((f = fopen("system/exec.err", "wb")) != NULL) {
    fwrite("\x0\x0\x0\x0", 4, 1, f);
    fwrite(&stack[sp], 4, 1, f);
    fwrite(&mem[stack[sp - 1]], 1, strlen((char *)(&mem[stack[sp - 1]])) + 1, f);
    fclose(f);
  }
#else
  printf("%s\n", (char *)&mem[stack[sp - 1]]);
#endif

  _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
  chdir(divpath);

#ifdef WIN32
  {
    extern void closefiles(void);
    closefiles();
  }
#endif

  exit(stack[sp]);
}

//----------------------------------------------------------------------------
//      Roll_palette(start_color,num_colors,inc)
//----------------------------------------------------------------------------

void roll_palette(void) {
  int c, n, i, x, color;
  char pal[PALETTE_SIZE];

  i = stack[sp--];
  n = abs(stack[sp--]);
  c = abs(stack[sp]) % 256;
  if (n + c > 256)
    n = 256 - c;
  for (x = c; x < c + n; x++) {
    color = x + i - c;
    while (color < 0)
      color += n;
    while (color >= n)
      color -= n;
    color += c;
    memcpy(&pal[color * 3], &palette[x * 3], 3);
  }
  memcpy(&palette[c * 3], &pal[c * 3], n * 3);
  if (!apply_palette_flag)
    apply_palette_flag = 1;
}

//----------------------------------------------------------------------------
//      Get_real_point(n,&x,&y)
//----------------------------------------------------------------------------

void get_real_point(void) {
  int x, y, w, h, xg, yg;
  int n, dx, dy, px, py;
  int *ptr;
  float ang, dis;
  short *p;

  dy = stack[sp--];
  dx = stack[sp--];
  n = stack[sp];

  if (mem[id + _File] > max_fpgs || mem[id + _File] < 0) {
    runtime_error(109);
    return;
  }
  if (mem[id + _File])
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (mem[id + _Graph] <= 0 || mem[id + _Graph] >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[mem[id + _File]].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[mem[id + _File]].grf[mem[id + _Graph]]) == NULL) {
    runtime_error(121);
    return;
  }

  if (n >= 0 || n < ptr[15]) {
    p = (short *)&ptr[16];
    px = p[n * 2];
    py = p[n * 2 + 1];

    x = mem[id + _X];
    y = mem[id + _Y];
    if (mem[id + _Resolution] > 0) {
      x /= mem[id + _Resolution];
      y /= mem[id + _Resolution];
    }

    w = ptr[13];
    h = ptr[14];

    if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
      xg = ptr[13] / 2;
      yg = ptr[14] / 2;
    } else {
      xg = *((word *)ptr + 32);
      yg = *((word *)ptr + 33);
    }

    if (mem[id + _Angle] != 0) {
      px -= xg;
      py -= yg;
      if (!px && !py) {
        px = x;
        py = y;
      } else {
        if (!px) {
          if (py > 0)
            ang = -1.5708;
          else
            ang = 1.5708;
        } else
          ang = atan2(-py, px);
        ang += ((float)mem[id + _Angle]) / radian;
        dis = sqrt(px * px + py * py) * mem[id + _Size] / 100;
        if (mem[id + _Flags] & SPRITE_HFLIP)
          px = x - cos(ang) * dis;
        else
          px = x + cos(ang) * dis;
        if (mem[id + _Flags] & SPRITE_VFLIP)
          py = y + sin(ang) * dis;
        else
          py = y - sin(ang) * dis;
      }
    } else if (mem[id + _Size] != 100) {
      if (mem[id + _Flags] & SPRITE_HFLIP)
        px = x + (xg - px) * mem[id + _Size] / 100;
      else
        px = x + (px - xg) * mem[id + _Size] / 100;
      if (mem[id + _Flags] & SPRITE_VFLIP)
        py = y + (yg - py) * mem[id + _Size] / 100;
      else
        py = y + (py - yg) * mem[id + _Size] / 100;
    } else {
      if (mem[id + _Flags] & SPRITE_HFLIP)
        px = x + xg - px;
      else
        px += x - xg;
      if (mem[id + _Flags] & SPRITE_VFLIP)
        py = y + yg - py;
      else
        py += y - yg;
    }
    if (mem[id + _Resolution] > 0) {
      px *= mem[id + _Resolution];
      py *= mem[id + _Resolution];
    }
    mem[dx] = px;
    mem[dy] = py;
  } else
    runtime_error(133);
}

//----------------------------------------------------------------------------
//      Get_joy_button(button 0..3)
//----------------------------------------------------------------------------

#define GAME_PORT  0x201
#define TIMER_PORT 0x40
#define TIME_OUT   2000

void get_joy_button(void) {
  // SDL joypad
  if (divjoy && joy_status) {
    stack[sp] = OSDEP_JoystickGetButton(divjoy, stack[sp]);
  } else {
    stack[sp] = 0;
  }
}

//----------------------------------------------------------------------------
//      Get_joy_position(axis 0..3)
//----------------------------------------------------------------------------

int ej[4] = {-1, -1, -1, -1};

void get_joy_position(void) {
  if (stack[sp] < 0 || stack[sp] > 3) {
    stack[sp] = 0;
    runtime_error(134);
    return;
  }

  stack[sp] = joy_position(stack[sp]);
}

int joy_position(int axis) {
  return OSDEP_JoystickGetAxis(divjoy, axis) / 100;
}

//----------------------------------------------------------------------------
//      Read_joy() - Joystick reading with auto-calibration - INTERNAL
//----------------------------------------------------------------------------

int joy_cx = 0, joy_cy = 0, joy_x0, joy_x1, joy_y0, joy_y1, init_joy = 0;

void read_joy(void) {
  if (!divjoy || !joy_status) {
    joy->button1 = 0;
    joy->button2 = 0;
    joy->button3 = 0;
    joy->button4 = 0;
    joy->left = 0;
    joy->right = 0;
    joy->up = 0;
    joy->down = 0;
    return;
  }

  // Read button states
  joy->button1 = OSDEP_JoystickGetButton(divjoy, 0) ? 1 : 0;
  joy->button2 = OSDEP_JoystickGetButton(divjoy, 1) ? 1 : 0;
  joy->button3 = OSDEP_JoystickGetButton(divjoy, 2) ? 1 : 0;
  joy->button4 = OSDEP_JoystickGetButton(divjoy, 3) ? 1 : 0;

  // Read axes (-32768..32767) and apply dead zone (~30%)
  int x = OSDEP_JoystickGetAxis(divjoy, 0);
  int y = OSDEP_JoystickGetAxis(divjoy, 1);

  if (x > 9830) {
    joy->left = 0;
    joy->right = 1;
  } else if (x < -9830) {
    joy->left = 1;
    joy->right = 0;
  } else {
    joy->left = 0;
    joy->right = 0;
  }

  if (y > 9830) {
    joy->up = 0;
    joy->down = 1;
  } else if (y < -9830) {
    joy->up = 1;
    joy->down = 0;
  } else {
    joy->up = 0;
    joy->down = 0;
  }
}
//----------------------------------------------------------------------------
//      Convert_palette(file,graph,&apply_palette)
//----------------------------------------------------------------------------

void convert_palette(void) {
  int file, graf, pal_ofs;
  int *ptr, n;
  byte *si;

  pal_ofs = stack[sp--];
  graf = stack[sp--];
  file = stack[sp];

  if (!validate_address(pal_ofs) || !validate_address(pal_ofs + 256)) {
    runtime_error(136);
    return;
  }
  if (file < 0 || file > max_fpgs) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graf <= 0 || graf >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[file].grf[graf]) == NULL) {
    runtime_error(121);
    return;
  }

  n = ptr[13] * ptr[14];
  si = (byte *)ptr + 64 + ptr[15] * 4;
  do {
    *si = (byte)mem[pal_ofs + *si];
    si++;
  } while (--n);
}

//----------------------------------------------------------------------------
//      Reset_sound()
//----------------------------------------------------------------------------

void reset_sound(void) {
  sound_reset();
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      Set_volume()
//----------------------------------------------------------------------------

void set_master_volume(word);
void set_voc_volume(word);
void set_cd_volume(word);

void set_volume(void) {
  if (setup->master < 0)
    setup->master = 0;
  if (setup->master > 15)
    setup->master = 15;
  if (setup->sound_fx < 0)
    setup->sound_fx = 0;
  if (setup->sound_fx > 15)
    setup->sound_fx = 15;
  if (setup->cd_audio < 0)
    setup->cd_audio = 0;
  if (setup->cd_audio > 15)
    setup->cd_audio = 15;
  set_master_volume(setup->master);
  set_voc_volume(setup->sound_fx);
  set_cd_volume(setup->cd_audio);
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      Set_color(color,r,g,b)
//----------------------------------------------------------------------------

void set_color(void) {
  int color, r, g, b;

  b = abs(stack[sp--]) % 64;
  g = abs(stack[sp--]) % 64;
  r = abs(stack[sp--]) % 64;
  color = abs(stack[sp]) % 256;

  palette[color * 3] = r;
  palette[color * 3 + 1] = g;
  palette[color * 3 + 2] = b;

  if (!apply_palette_flag)
    apply_palette_flag = 1;
}

//----------------------------------------------------------------------------
//      Find_color(r,g,b) Returns a palette color (1..255)
//----------------------------------------------------------------------------

void _find_color(void) {
  int r, g, b;

  b = abs(stack[sp--]) % 64;
  g = abs(stack[sp--]) % 64;
  r = abs(stack[sp]) % 64;

  find_color(r, g, b);
  stack[sp] = find_col;
}

//----------------------------------------------------------------------------
//      String functions
//----------------------------------------------------------------------------

void _strchar(void) { // char("0") -> 48
  if ((unsigned)stack[sp] > 255)
    stack[sp] = (int)memb[stack[sp] * 4];
}

void _strcpy(void) {
  if ((mem[stack[sp - 1] - 1] & 0xFFF00000) != 0xDAD00000) {
    sp--;
    runtime_error(164);
    return;
  }
  if ((unsigned)stack[sp] > 255)
    if ((mem[stack[sp - 1] - 1] & 0xFFFFF) + 1 < strlen((char *)&mem[stack[sp]])) {
      sp--;
      runtime_error(140);
      return;
    }
  if ((unsigned)stack[sp] > 255)
    memmove((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]],
            strlen((char *)&mem[stack[sp]]) + 1);
  else
    mem[stack[sp - 1]] = stack[sp];
  sp--;
}

void _strcat(void) {
  int n;
  if ((mem[stack[sp - 1] - 1] & 0xFFF00000) != 0xDAD00000) {
    sp--;
    runtime_error(164);
    return;
  }
  if ((unsigned)stack[sp] > 255)
    n = strlen((char *)&mem[stack[sp]]);
  else
    n = 1;
  if ((mem[stack[sp - 1] - 1] & 0xFFFFF) + 1 < strlen((char *)&mem[stack[sp - 1]]) + n) {
    sp--;
    runtime_error(140);
    return;
  }
  if ((unsigned)stack[sp] > 255) {
    char *dst = (char *)&mem[stack[sp - 1]];
    int dlen = strlen(dst);
    memmove(dst + dlen, (char *)&mem[stack[sp]], strlen((char *)&mem[stack[sp]]) + 1);
  } else {
    char *dst = (char *)&mem[stack[sp - 1]];
    int dlen = strlen(dst);
    dst[dlen] = (char)stack[sp];
    dst[dlen + 1] = 0;
  }
  sp--;
}

void _strlen(void) {
  if ((unsigned)stack[sp] > 255)
    stack[sp] = strlen((char *)&mem[stack[sp]]);
  else
    stack[sp] = 1;
}

void _strcmp(void) {
  if ((unsigned)stack[sp - 1] > 255) {
    if ((unsigned)stack[sp] > 255) {
      stack[sp - 1] = strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]);
    } else {
      stack[sp - 1] = strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]);
    }
  } else {
    if ((unsigned)stack[sp] > 255) {
      stack[sp - 1] = strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]);
    } else {
      stack[sp - 1] = strcmp((char *)&stack[sp - 1], (char *)&stack[sp]);
    }
  }
  sp--;
}

void _strchr(void) { // e.g.: strchr(string,"aeiou") -> -1 Not found, N Position
  char *p;
  if ((unsigned)stack[sp] > 255)
    p = strpbrk((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]);
  else
    p = strchr((char *)&mem[stack[sp - 1]], (char)stack[sp]);
  if (p != NULL) {
    stack[sp - 1] = (int)(p - (char *)&mem[stack[sp - 1]]);
  } else
    stack[sp - 1] = -1;
  sp--;
}

void _strstr(void) {
  char *p;
  if ((unsigned)stack[sp] > 255)
    p = strstr((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]);
  else
    p = strchr((char *)&mem[stack[sp - 1]], (char)stack[sp]);
  if (p != NULL) {
    stack[sp - 1] = (int)(p - (char *)&mem[stack[sp - 1]]);
  } else
    stack[sp - 1] = -1;
  sp--;
}

void __strset(void) {
  int n;
  if ((mem[stack[sp - 1] - 1] & 0xFFF00000) != 0xDAD00000) {
    sp--;
    runtime_error(164);
    return;
  }
  n = (mem[stack[sp - 1] - 1] & 0xFFFFF) + 1;
  if ((unsigned)stack[sp] > 255)
    memset((char *)&mem[stack[sp - 1]], (char)mem[stack[sp]], n);
  else
    memset((char *)&mem[stack[sp - 1]], (char)stack[sp], n);
  sp--;
}

byte strupper[270] = "                                                                "
                     " ABCDEFGHIJKLMNOPQRSTUVWXYZ      ABCDEFGHIJKLMNOPQRSTUVWXYZ     "
                     "   A AA EEEIII A   O OUUY       AIOU                            "
                     "                                                                ";

void __strupr(void) {
  int n;
  if ((unsigned)stack[sp] > 255) {
    n = strlen((char *)&mem[stack[sp]]);
    while (n--) {
      if (strupper[memb[stack[sp] * 4 + n]] != ' ')
        memb[stack[sp] * 4 + n] = strupper[memb[stack[sp] * 4 + n]];
    }
  } else {
    if (strupper[(char)stack[sp]] != ' ')
      stack[sp] = (int)strupper[(char)stack[sp]];
  }
}

byte strlower[260] = "                                                                "
                     " abcdefghijklmnopqrstuvwxyz      abcdefghijklmnopqrstuvwxyz     "
                     "                                                                "
                     "                                                                ";

void __strlwr(void) {
  int n;
  if ((unsigned)stack[sp] > 255) {
    n = strlen((char *)&mem[stack[sp]]);
    while (n--) {
      if (strlower[memb[stack[sp] * 4 + n]] != ' ')
        memb[stack[sp] * 4 + n] = strlower[memb[stack[sp] * 4 + n]];
    }
  } else {
    if (strlower[(char)stack[sp]] != ' ')
      stack[sp] = (int)strlower[(char)stack[sp]];
  }
}

void str_del_beg(char *s, int n) {
  int len = strlen(s);
  if (n > 0) {
    if (n >= len)
      *s = 0;
    else
      memmove(s, s + n, len + 1 - n);
  } else if (n < 0) {
    memmove(s - n, s, len + 1);
    memset(s, ' ', -n);
  }
}

void str_del_end(char *s, int n) {
  int len = strlen(s);
  if (n > 0) {
    if (n >= len)
      *s = 0;
    else
      s[len - n] = 0;
  } else if (n < 0) {
    n = len - n;
    for (; len < n; len++)
      s[len] = ' ';
    s[len] = 0;
  }
}

void _strdel(void) { // (string,n,m) delete <n> chars from start and <m> from end
  int m = stack[sp--];
  int n = stack[sp--];

  if ((mem[stack[sp] - 1] & 0xFFF00000) != 0xDAD00000) {
    runtime_error(164);
    return;
  }

  if ((mem[stack[sp] - 1] & 0xFFFFF) + 1 < strlen((char *)&mem[stack[sp]]) - n - m) {
    runtime_error(140);
    return;
  }

  if (n > m) { // Delete from start first
    str_del_beg((char *)&mem[stack[sp]], n);
    str_del_end((char *)&mem[stack[sp]], m);
  } else { // Delete from end first
    str_del_end((char *)&mem[stack[sp]], m);
    str_del_beg((char *)&mem[stack[sp]], n);
  }
}

//----------------------------------------------------------------------------
//  Structure sort / shuffle function (name,field,mode)
//----------------------------------------------------------------------------

byte xlat_rnd[256];
int offset_key;

int sort0(const void *a, const void *b) {
  return ((*((int *)a + offset_key)) - (*((int *)b + offset_key)));
}

int sort1(const void *a, const void *b) {
  return ((*((int *)b + offset_key)) - (*((int *)a + offset_key)));
}

int sort2(const void *a, const void *b) {
  return (strcmp((char *)a + offset_key * 4, (char *)b + offset_key * 4));
}

int sort3(const void *a, const void *b) {
  return (-strcmp((char *)a + offset_key * 4, (char *)b + offset_key * 4));
}

int sort4(const void *a, const void *b) {
  return (strcmp((char *)&mem[*((char *)a + offset_key)], (char *)&mem[*((char *)b + offset_key)]));
}

int sort5(const void *a, const void *b) {
  return (
      -strcmp((char *)&mem[*((char *)a + offset_key)], (char *)&mem[*((char *)b + offset_key)]));
}

int unsort00(byte *a, byte *b) {
  return (
      (int)(xlat_rnd[*(a + offset_key * 4)] ^ (xlat_rnd[*(a + 1 + offset_key * 4)] / 2) ^
            (xlat_rnd[*(a + 2 + offset_key * 4)] / 4) ^ (xlat_rnd[*(a + 3 + offset_key * 4)] / 8)) -
      (int)(xlat_rnd[*(b + offset_key * 4)] ^ (xlat_rnd[*(b + 1 + offset_key * 4)] / 2) ^
            (xlat_rnd[*(b + 2 + offset_key * 4)] / 4) ^ (xlat_rnd[*(b + 3 + offset_key * 4)] / 8)));
}


int unsort0(const void *a, const void *b) {
  return unsort00((byte *)a, (byte *)b);
}

int strcmp_sort(const void *a, const void *b) {
  return strcmp((char *)a, (char *)b);
}

void sort(void) {
  int tipo_key;
  int offset, size, numreg, modo;

  modo = stack[sp--];
  tipo_key = stack[sp--];
  offset_key = stack[sp--];
  numreg = stack[sp--];
  size = stack[sp--];
  offset = stack[sp];

  if (modo < 0 || modo > 1) {
    for (modo = 0; modo < 256; modo++)
      xlat_rnd[modo] = rnd();
    qsort(&mem[offset], numreg, size * 4, unsort0);
  } else
    switch (tipo_key) {
    case 0:
      if (modo)
        qsort(&mem[offset], numreg, size * 4, sort1);
      else
        qsort(&mem[offset], numreg, size * 4, sort0);
      break;
    case 1:
      if (modo)
        qsort(&mem[offset], numreg, size * 4, sort3);
      else
        qsort(&mem[offset], numreg, size * 4, sort2);
      break;
    case 2:
      if (modo)
        qsort(&mem[offset], numreg, size * 4, sort5);
      else
        qsort(&mem[offset], numreg, size * 4, sort4);
      break;
    }
  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
// File functions (in addition to load/save)
//----------------------------------------------------------------------------

/*
function 122 int fopen(0,0)         // Similar to C (filename,"rwa+") (always binary mode)
function 123 int fclose(0)          // Similar to C (handle) but if handle is 0, does fcloseall
function 124 int fread(0,0,0)       // Similar to C (&buffer,length(in ints),handle) but with a single int length
function 125 int fwrite(0,0,0)      // Inverse of fread (&buffer,length,handle)
function 126 int fseek(0,0,0)       // Identical to C (handle,position,mode) (mode seek_set/cur/end)
function 127 int ftell(0)           // Identical to C (handle)
function 128 int filelength(0)      // Length of an open file (handle)
function 129 int flush()            // Flush write buffers and return number of open files

function 130 int get_dirinfo(0,0)   // Read a directory ("dir\*.pr?",_hidden+_system+_subdir) into the dirinfo(files,name[]) struct and return "files" (count)
function 131 string get_fileinfo(0) // Fill the fileinfo struct (fullpath (or cwd), drive, dir, name, ext, size, date, time, attrib) and return pointer to "fullpath" or "cwd"

function 132 int getdrive()         // Returns current drive (1-A, 2-B, ...) [C: get_drive()]
function 133 int setdrive(0)        // Set current drive (same) [C: set_drive()]
function 134 int chdir(0)           // Change current directory ("..")
function 135 int mkdir(0)           // Create a new directory (from cwd)
function 136 int remove(0)          // Delete files or directories (wildcards accepted) (auto rmdir)
function 137 int disk_free(0)       // Free space on a drive (in KBs)
function 138 int memory_free()      // Available memory (in KBs)
function 139 int ignore_error(0)    // Ignore a runtime error (number)
*/

//----------------------------------------------------------------------------
//      fopen((filename,"rwa+") (always binary mode)
//----------------------------------------------------------------------------

void _fopen(void) { // Search for the file, as it may have been included in the installation
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  char modo[128];
  int n, x;
  FILE *f = NULL;

  div_strcpy(modo, sizeof(modo), (char *)&mem[stack[sp--]]);
  div_strcpy(full, sizeof(full), (char *)&mem[stack[sp]]);
  for (n = 0; n < strlen(modo); n++)
    if (modo[n] != 'r' && modo[n] != 'w' && modo[n] != 'a' && modo[n] != '+')
      break;
  if (n < strlen(modo)) {
    stack[sp] = 0;
    runtime_error(166);
  }
  div_strcat(modo, sizeof(modo), "b");

  packfile_del(full);

#ifdef DEBUG
  // check for file in prg dir
  f = __fpopen((byte *)full, modo);
#endif

  if (f == NULL) {
    if ((f = fopen(full, modo)) == NULL) { // "paz\fixero.est"
      if (_fullpath(full, (char *)&mem[stack[sp]], _MAX_PATH) == NULL) {
        stack[sp] = 0;
        return;
      }
      _splitpath(full, drive, dir, fname, ext);
      if (strchr(ext, '.') == NULL)
        div_strcpy(full, sizeof(full), ext);
      else
        div_strcpy(full, sizeof(full), strchr(ext, '.') + 1);
      if (strlen(full) && memb[stack[sp] * 4] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), (char *)&mem[stack[sp]]);
      if ((f = fopen(full, modo)) == NULL) { // "est\paz\fixero.est"
        div_strcpy(full, sizeof(full), fname);
        div_strcat(full, sizeof(full), ext);
        if ((f = fopen(full, modo)) == NULL) { // "fixero.est"
          if (strchr(ext, '.') == NULL)
            div_strcpy(full, sizeof(full), ext);
          else
            div_strcpy(full, sizeof(full), strchr(ext, '.') + 1);
          if (strlen(full))
            div_strcat(full, sizeof(full), "/");
          div_strcat(full, sizeof(full), fname);
          div_strcat(full, sizeof(full), ext);
          f = fopen(full, modo); // "est\fixero.est"
        }
      }
    }
  }

  if (f) {
    for (x = 0; x < 32; x++)
      if (tabfiles[x] == 0)
        break;
    if (x == 32) {
      fclose(f);
      stack[sp] = 0;
      runtime_error(169);
    } else {
      tabfiles[x] = f;
      stack[sp] = x * 2 + 1;
    }
  } else {
    stack[sp] = 0;
    if (errno == EMFILE) {
      runtime_error(169);
    }
  }
}

//----------------------------------------------------------------------------
//      fclose(handle) (handle==0 -> fcloseall)
//----------------------------------------------------------------------------

void _fclose(void) {
  int n;

  if (stack[sp] == 0) {
    stack[sp] = 0; //fcloseall();
    if (stack[sp] == EOF)
      stack[sp] = 0;
    memset(tabfiles, 0, 32 * 4);
  } else {
    if (!(stack[sp] & 1) || stack[sp] < 1 || stack[sp] > 63) {
#ifdef DEBUG
      runtime_error(170);
#else
      stack[sp] = 0;
#endif
      return;
    }
    n = stack[sp] / 2;
    if (tabfiles[n] == 0) {
      runtime_error(170);
      return;
    }
    stack[sp] = fclose((FILE *)(tabfiles[n]));
    tabfiles[n] = 0;
    if (!stack[sp])
      stack[sp] = 1;
    else
      stack[sp] = 0;
  }
}

//----------------------------------------------------------------------------
//      fread(&buffer, length(in unit_size), handle)
//----------------------------------------------------------------------------

void _fread(void) {
  int offset, lon, handle, n;
  FILE *f;

  if (unit_size < 1)
    unit_size = 1;

  handle = stack[sp--];
  lon = stack[sp--];
  offset = stack[sp];

  if (!(handle & 1) || handle < 1 || handle > 63) {
    runtime_error(170);
    return;
  }
  if (tabfiles[handle / 2] == 0) {
    runtime_error(170);
    return;
  }
  f = (FILE *)tabfiles[handle / 2];
  if (!validate_address(offset) || !validate_address(offset + (lon * unit_size) / 4)) {
    stack[sp] = 0;
    runtime_error(125);
    return;
  }
  n = fread(&mem[offset], 1, unit_size * lon, f); // Bytes read
  if ((n + unit_size - 1) / unit_size < lon) {
    stack[sp] = 0;
    runtime_error(127);
  } else {
    if (n / unit_size < lon)
      memset(&memb[offset * 4 + n], 0, lon * unit_size - n);
    stack[sp] = 1;
  }
  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      fwrite(&buffer,length(in unit_size),handle)
//----------------------------------------------------------------------------

void _fwrite(void) {
  int offset, lon, handle;
  FILE *f;

  if (unit_size < 1)
    unit_size = 1;

  handle = stack[sp--];
  lon = stack[sp--];
  offset = stack[sp];

  if (!(handle & 1) || handle < 1 || handle > 63) {
    runtime_error(170);
    return;
  }
  if (tabfiles[handle / 2] == 0) {
    runtime_error(170);
    return;
  }
  f = (FILE *)tabfiles[handle / 2];
  if (!validate_address(offset) || !validate_address(offset + (lon * unit_size) / 4)) {
    stack[sp] = 0;
    runtime_error(122);
    return;
  }
  if (fwrite(&mem[offset], unit_size, lon, f) != lon) {
    stack[sp] = 0;
    runtime_error(124);
  } else
    stack[sp] = 1;
  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      fseek(handle,offset,where)
//----------------------------------------------------------------------------

void _fseek(void) {
  int handle, offset, where;
  FILE *f;

  if (unit_size < 1)
    unit_size = 1;

  where = stack[sp--];
  offset = stack[sp--] * unit_size;
  handle = stack[sp];

  if (!(handle & 1) || handle < 1 || handle > 63) {
    runtime_error(170);
    return;
  }
  if (tabfiles[handle / 2] == 0) {
    runtime_error(170);
    return;
  }
  f = (FILE *)tabfiles[handle / 2];

  fseek(f, offset, where);
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      ftell(handle)
//----------------------------------------------------------------------------

void _ftell(void) {
  if (unit_size < 1)
    unit_size = 1;

  if (!(stack[sp] & 1) || stack[sp] < 1 || stack[sp] > 63) {
    runtime_error(170);
    return;
  }
  if (tabfiles[stack[sp] / 2] == 0) {
    runtime_error(170);
    return;
  }
  stack[sp] = (int)(ftell((FILE *)tabfiles[stack[sp] / 2]) + unit_size - 1) / unit_size;
}

//----------------------------------------------------------------------------
//      filelength(handle)
//----------------------------------------------------------------------------

void __filelength(void) {
  int pos, len;

  if (unit_size < 1)
    unit_size = 1;

  if (!(stack[sp] & 1) || stack[sp] < 1 || stack[sp] > 63) {
    runtime_error(170);
    return;
  }
  if (tabfiles[stack[sp] / 2] == 0) {
    runtime_error(170);
    return;
  }

  pos = ftell((FILE *)tabfiles[stack[sp] / 2]);
  fseek((FILE *)tabfiles[stack[sp] / 2], 0, SEEK_END);
  len = (ftell((FILE *)tabfiles[stack[sp] / 2]) + unit_size - 1) / unit_size;
  fseek((FILE *)tabfiles[stack[sp] / 2], pos, SEEK_SET);
  stack[sp] = len;
}

//----------------------------------------------------------------------------
//      flush()
//----------------------------------------------------------------------------

void flush(void) {
  stack[++sp] = 0; //flushall()-numfiles;
}

//----------------------------------------------------------------------------
//      get_dirinfo(mask, attributes)
//----------------------------------------------------------------------------

// function 130 int get_dirinfo(0,0)
// Read a directory ("dir\*.pr?",_hidden+_system+_subdir)
// into the dirinfo(files,name[]) struct and return "files" (count)

void get_dirinfo(void) {
  //char * filenames (in mem[imem_max+258*5])
  //dirinfo->files
  //dirinfo->name[]

  unsigned rc;
  struct find_t ft;
  int x = 0;
  int flags;

  flags = stack[sp--];
  memset(dirinfo->name, 0, 1025 * 4);

  rc = _dos_findfirst((char *)&mem[stack[sp]], flags, &ft);
  while (!rc) {
    div_strcpy(&filenames[x * 16], 16, ft.name);
    dirinfo->name[x] = imem_max + 258 * 5 + x * 4;
    rc = _dos_findnext(&ft);
    x++;
  }

  qsort(filenames, x, 16, strcmp_sort);

  dirinfo->files = stack[sp] = x;
}

//----------------------------------------------------------------------------
//      get_fileinfo(filename)
//----------------------------------------------------------------------------

#define YEAR(t)   (((t & 0xFE00) >> 9) + 1980)
#define MONTH(t)  ((t & 0x01E0) >> 5)
#define DAY(t)    (t & 0x001F)
#define HOUR(t)   ((t & 0xF800) >> 11)
#define MINUTE(t) ((t & 0x07E0) >> 5)
#define SECOND(t) ((t & 0x001F) << 1)

void get_fileinfo(void) {
  unsigned rc;
  struct find_t ft;

  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  char filename[_MAX_PATH + 1];

  if (unit_size < 1)
    unit_size = 1;

  div_strcpy(filename, sizeof(filename), (char *)&mem[stack[sp]]);
  stack[sp] = 0;

  rc = _dos_findfirst(filename, _A_NORMAL | _A_SYSTEM | _A_HIDDEN | _A_SUBDIR, &ft);
  if (rc)
    return;
  if (_fullpath(full, filename, _MAX_PATH) == NULL)
    return;
  strupr(full);
  _splitpath(full, drive, dir, fname, ext);

  div_strcpy(fileinfo->fullpath, sizeof(fileinfo->fullpath), full);
  fileinfo->drive = (int)drive[0] - 64;
  div_strcpy(fileinfo->dir, sizeof(fileinfo->dir), dir);
  div_strcpy(fileinfo->name, sizeof(fileinfo->name), fname);
  div_strcpy(fileinfo->ext, sizeof(fileinfo->ext), ext);
  fileinfo->size = (ft.size + unit_size - 1) / unit_size;
  fileinfo->day = DAY(ft.wr_date);
  fileinfo->month = MONTH(ft.wr_date);
  fileinfo->year = YEAR(ft.wr_date);
  fileinfo->hour = HOUR(ft.wr_time);
  fileinfo->min = MINUTE(ft.wr_time);
  fileinfo->sec = SECOND(ft.wr_time);
  fileinfo->attrib = ft.attrib;

  stack[sp] = 1;
}

//----------------------------------------------------------------------------
//      get_drive() returns drive number (1-A, 2-B, 3-C, ...)
//----------------------------------------------------------------------------

void get_drive(void) {
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      set_drive(drive)
//----------------------------------------------------------------------------

void set_drive(void) {
  unsigned int total;
  _dos_setdrive(stack[sp], &total);
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      chdir(directory) 1 - success, 0 - error
//----------------------------------------------------------------------------

void div_chdir(void) {
  if (chdir((char *)&mem[stack[sp]]))
    stack[sp] = 0;
  else
    stack[sp] = 1;
}

//----------------------------------------------------------------------------
//      mkdir(directory)
//----------------------------------------------------------------------------

void _mkdir(void) {
  char *buffer;
  char cwork[256];
  int x;

  buffer = (char *)&mem[stack[sp]];

  if (strlen(buffer))
    if (IS_PATH_SEP(buffer[strlen(buffer) - 1]))
      buffer[strlen(buffer) - 1] = 0;

  for (x = 0; x < strlen(buffer); x++) {
    if (x > 0 && buffer[x - 1] == ':')
      continue;
    if (IS_PATH_SEP(buffer[x])) {
      div_strcpy(cwork, sizeof(cwork), buffer);
      cwork[x] = 0;
      __mkdir(cwork);
    }
  }
  __mkdir(buffer);

  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      remove(filename)
//----------------------------------------------------------------------------

void remove_file(void) {
  unsigned rc;
  struct find_t ft;
  int x;
  char cwork1[_MAX_PATH + 1];
  char cwork2[_MAX_PATH + 1];
  char cwork3[_MAX_PATH + 1];

  div_strcpy(cwork2, sizeof(cwork2), (char *)&mem[stack[sp]]);
  stack[sp] = 0;

  for (x = strlen(cwork2) - 1;; x--) {
    if (x == -1) {
      cwork2[0] = 0;
      break;
    }
    if (IS_PATH_SEP(cwork2[x])) {
      cwork2[x + 1] = 0;
      break;
    }
  }

  rc = _dos_findfirst((char *)&mem[stack[sp]], _A_NORMAL | _A_SYSTEM | _A_HIDDEN, &ft);
  while (!rc) {
    div_strcpy(cwork3, sizeof(cwork3), cwork2);
    div_strcat(cwork3, sizeof(cwork3), ft.name);
    if (_fullpath(cwork1, cwork3, _MAX_PATH) == NULL)
      div_strcpy(cwork1, sizeof(cwork1), ft.name);
    _dos_setfileattr(cwork1, _A_NORMAL);
    remove(cwork1);
    rc = _dos_findnext(&ft);
  }

  rc = _dos_findfirst((char *)&mem[stack[sp]], _A_SUBDIR, &ft);
  while (!rc) {
    div_strcpy(cwork3, sizeof(cwork3), cwork2);
    div_strcat(cwork3, sizeof(cwork3), ft.name);
    if (_fullpath(cwork1, cwork3, _MAX_PATH) == NULL)
      div_strcpy(cwork1, sizeof(cwork1), ft.name);
    _dos_setfileattr(cwork1, _A_SUBDIR);
    rmdir(cwork1);
    rc = _dos_findnext(&ft);
  }
}

//----------------------------------------------------------------------------
// Memory and system functions
//----------------------------------------------------------------------------

/*
function 137 int disk_free(0)                      // Free space on a (drive), in bytes
function 138 int memory_free()                     // Available memory (in KBs)
function 139 int ignore_error(0)                   // Ignore a runtime error (number)
*/

typedef struct _meminfo {
  unsigned largest_available_block; // Largest available block
  unsigned max_unlocked_pages;      // Max unlocked pages
  unsigned largest_lockable_page;   // Largest lockable page
  unsigned linear_address_space;    // Linear address space
  unsigned free_pages_available;    // Number of free pages available
  unsigned free_physical_pages;     // Number of free physical pages
  unsigned total_physical_pages;    // Total physical pages
  unsigned free_linear_space;       // Free linear address space
  unsigned page_file_size;          // Page file size
  unsigned reservado[3];            // Reserved
} meminfo;

int mem_get_heap_free() {
  return 65535;
}

void get_free_mem(meminfo *meminfo_ptr) {}

//----------------------------------------------------------------------------
//      disk_free(drive)
//----------------------------------------------------------------------------

void disk_free(void) {
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      memory_free()
//----------------------------------------------------------------------------

void memory_free(void) {
  stack[++sp] = 0;
}

//----------------------------------------------------------------------------
//      ignore_error(number)
//----------------------------------------------------------------------------

void ignore_error(void) {
  int n;
  n = 0;
  while (n < num_skipped) {
    if (skipped[n] == stack[sp])
      break;
    n++;
  }
  if (n >= num_skipped && num_skipped < 127) {
    skipped[num_skipped++] = stack[sp];
  } else if (num_skipped == 127)
    runtime_error(168);
  stack[sp] = 0;
}

//----------------------------------------------------------------------------
// Math functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//      sin(angle) Returns the sine of an angle given in thousandths
//----------------------------------------------------------------------------

#define pi     180000
#define radian 57295.77951

void _sin(void) {
  float angle = (float)stack[sp] / radian;
  stack[sp] = (int)((float)sin(angle) * 1000);
}

void _asin(void) {
  float seno = (float)stack[sp] / 1000.0;
  if (stack[sp] < -1000 || stack[sp] > 1000) {
    stack[sp] = 0;
    runtime_error(171);
    return;
  }
  stack[sp] = (int)((float)asin(seno) * radian);
}

//----------------------------------------------------------------------------
//      cos(angle) Returns the cosine of an angle given in thousandths
//----------------------------------------------------------------------------

void _cos(void) {
  float angle = (float)stack[sp] / radian;
  stack[sp] = (int)((float)cos(angle) * 1000);
}

void _acos(void) {
  float coseno = (float)stack[sp] / 1000.0;
  if (stack[sp] < -1000 || stack[sp] > 1000) {
    stack[sp] = 0;
    runtime_error(171);
    return;
  }
  stack[sp] = (int)((float)acos(coseno) * radian);
}

//----------------------------------------------------------------------------
//      tan(angle) Returns the tangent of an angle given in thousandths
//----------------------------------------------------------------------------

void _tan(void) {
  float angle;

  while (stack[sp] >= 180000)
    stack[sp] -= 360000;
  while (stack[sp] <= -180000)
    stack[sp] += 360000;

  if (stack[sp] == 90000) {
    stack[sp] = 0x7FFFFFFF;
    return;
  }
  if (stack[sp] == -90000) {
    stack[sp] = 0x80000000;
    return;
  }

  angle = (float)stack[sp] / radian;
  stack[sp] = (int)((float)tan(angle) * 1000);
}

void _atan(void) {
  float tangente;

  tangente = (float)stack[sp] / 1000.0;
  stack[sp] = (int)((float)atan(tangente) * radian);
}

void _atan2(void) {
  float x, y;

  x = (float)stack[sp--];
  y = (float)stack[sp];
  stack[sp] = (int)((float)atan2(y, x) * radian);
}

//----------------------------------------------------------------------------
//      draw(type, color, x0, y0, x1, y1)
//----------------------------------------------------------------------------

#define tipo_mayor 5 // 1-Line 2-Rectangle 3-Filled Rect. 4-Circle 5-Filled Circle

void draw(void) {
  int x;

  x = 0;
  while (drawing[x].type && x < max_drawings)
    x++;
  if (x == max_drawings) {
    sp -= 7;
    stack[sp] = 0;
    runtime_error(172);
    return;
  }

  drawing[x].y1 = stack[sp--];
  drawing[x].x1 = stack[sp--];
  drawing[x].y0 = stack[sp--];
  drawing[x].x0 = stack[sp--];
  drawing[x].region = stack[sp--];
  drawing[x].opacity = stack[sp--];
  drawing[x].color = stack[sp--];
  drawing[x].type = stack[sp];

  if (drawing[x].type < 1 || drawing[x].type > tipo_mayor) {
    drawing[x].type = 0;
    runtime_error(173);
  }
  if (drawing[x].color < 0 || drawing[x].color > 255) {
    drawing[x].type = 0;
    runtime_error(154);
  }
  if (drawing[x].opacity < 0 || drawing[x].opacity > 15) {
    drawing[x].type = 0;
    runtime_error(174);
  }
  if (drawing[x].region < 0 || drawing[x].region >= max_region) {
    drawing[x].type = 0;
    runtime_error(108);
  }

  if (drawing[x].type)
    stack[sp] = x * 2 + 1;
  else
    stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      delete_draw(draw_id)
//----------------------------------------------------------------------------

void delete_draw(void) {
  int draw_id;

  if ((draw_id = stack[sp]) == -1) {
    for (draw_id = 0; draw_id < max_drawings; draw_id++) {
      drawing[draw_id].type = 0;
    }

  } else {
    if ((draw_id & 1) == 0 || draw_id < 1 || draw_id > max_drawings * 2 - 1) {
      stack[sp] = 0;
      runtime_error(175);
      return;
    }
    draw_id /= 2;

    if (drawing[draw_id].type == 0) {
      stack[sp] = 0;
      runtime_error(175);
      return;
    }

    drawing[draw_id].type = 0;
  }
}

//----------------------------------------------------------------------------
//      move_draw(draw_id, color, opacity, x0, y0, x1, y1)
//----------------------------------------------------------------------------

void move_draw(void) {
  int draw_id;

  draw_id = stack[sp - 6];

  if ((draw_id & 1) == 0 || draw_id < 1 || draw_id > max_drawings * 2 - 1) {
    sp -= 6;
    stack[sp] = 0;
    runtime_error(175);
    return;
  }
  draw_id /= 2;

  if (drawing[draw_id].type == 0) {
    stack[sp] = 0;
    runtime_error(175);
    return;
  }

  drawing[draw_id].y1 = stack[sp--];
  drawing[draw_id].x1 = stack[sp--];
  drawing[draw_id].y0 = stack[sp--];
  drawing[draw_id].x0 = stack[sp--];
  drawing[draw_id].opacity = stack[sp--];
  drawing[draw_id].color = stack[sp--];
  stack[sp] = 0;

  if (drawing[draw_id].color < 0 || drawing[draw_id].color > 255) {
    drawing[draw_id].type = 0;
    runtime_error(154);
  }
  if (drawing[draw_id].opacity < 0 || drawing[draw_id].opacity > 15) {
    drawing[draw_id].type = 0;
    runtime_error(174);
  }
}


//----------------------------------------------------------------------------
//      Save_map/pcx(file,graph,"filename.pcx") 1-Success 0-Error
//----------------------------------------------------------------------------

int save_PCX(byte *mapa, int w, int h, FILE *f);
int save_MAP(byte *mapa, int w, int h, FILE *f);

void save_mapcx(int tipo) {
  int file, graph;
  int *ptr;
  int w, h;
  byte *buffer;
  char cwork[256];
  FILE *f;

  div_strcpy(cwork, sizeof(cwork), (char *)&mem[stack[sp--]]);
  graph = stack[sp--];
  file = stack[sp];
  stack[sp] = 0;

  if (file > max_fpgs || file < 0) {
    runtime_error(109);
    return;
  }
  if (file)
    max_grf = MAX_FPG_GRAPHICS;
  else
    max_grf = MAX_FPG_GRAPHICS * 2;
  if (graph <= 0 || graph >= max_grf) {
    runtime_error(110);
    return;
  }
  if (g[file].grf == NULL) {
    runtime_error(111);
    return;
  }
  if ((ptr = g[file].grf[graph]) == NULL) {
    runtime_error(121);
    return;
  }

  w = ptr[13];
  h = ptr[14];
  buffer = (byte *)ptr + 64 + ptr[15] * 4;

  if ((f = open_save_file((byte *)cwork)) == NULL) {
    runtime_error(123);
    return;
  }
  if (tipo) {
    if (save_PCX(buffer, w, h, f)) {
      fclose(f);
      runtime_error(100);
      return;
    }
  } else {
    if (save_MAP(buffer, w, h, f)) {
      fclose(f);
      runtime_error(100);
      return;
    }
  }
  fclose(f);
  stack[sp] = 1;

  max_reloj += get_clock() - old_clock;
}

//----------------------------------------------------------------------------
//      Write_in_map(font,text,alignment) - Returns the map code
//      Unloaded with unload_map(code)
//----------------------------------------------------------------------------

void write_in_map(void) {
  int alignment, texts;
  int cx, cy, w, h;
  int font_index;

  byte *ptr, *ptr2;

  alignment = stack[sp--];
  texts = stack[sp--];
  font_index = stack[sp];

  if (font_index < 0 || font_index >= max_fonts) {
    runtime_error(116);
    return;
  }
  if (fonts[font_index] == 0) {
    runtime_error(116);
    return;
  }

  if (alignment < 0 || alignment > 8) {
    runtime_error(117);
    return;
  }

  checkpal_font(font_index);

  ptr = (byte *)&mem[texts];

  fnt = (fnt_table_entry *)((byte *)fonts[font_index] + FNT_TABLE_OFFSET);
  h = f_i[font_index].height;

  ptr2 = ptr;
  w = 0;
  while (*ptr2) {
    if (fnt[*ptr2].width == 0) {
      w += f_i[font_index].spacing;
      ptr2++;
    } else
      w += fnt[*ptr2++].width;
  }

  cx = 0;
  cy = 0;

  switch (alignment) {
  case 0:
    break;
  case 1:
    cx = (w >> 1);
    break;
  case 2:
    cx = w - 1;
    break;
  case 3:
    cy = (h >> 1);
    break;
  case 4:
    cx = (w >> 1);
    cy = (h >> 1);
    break;
  case 5:
    cx = w - 1;
    cy = (h >> 1);
    break;
  case 6:
    cy = h - 1;
    break;
  case 7:
    cx = (w >> 1);
    cy = h - 1;
    break;
  case 8:
    cx = w - 1;
    cy = h - 1;
    break;
  }

  ptr2 = ptr;

  if ((ptr = (byte *)malloc(1330 + 64 + 4 + w * h)) != NULL) {
    ptr += 1330; // fix load_map/unload_map
    *((int *)ptr + 13) = w;
    *((int *)ptr + 14) = h;
    *((int *)ptr + 15) = 1; // Define one control point (the center)
    *((word *)ptr + 32) = cx;
    *((word *)ptr + 33) = cy;
    memset(ptr + 4 + 64, 0, w * h);

    while (g[0].grf[next_map_code]) {
      if (next_map_code++ == MAX_FPG_GRAPHICS * 2 - 1)
        next_map_code = MAX_FPG_GRAPHICS;
    }
    g[0].grf[next_map_code] = (int *)ptr;
    stack[sp] = next_map_code;

  } else
    runtime_error(100);

  cx = 0; // Draw the text (ptr2) into ptr+68 (an*al)

  while (*ptr2 && cx + fnt[*ptr2].width <= w) {
    if (fnt[*ptr2].width == 0) {
      cx += f_i[font_index].spacing;
      ptr2++;
    } else {
      texn2(ptr + 68, w, fonts[font_index] + fnt[*ptr2].offset, cx, fnt[*ptr2].incY,
            fnt[*ptr2].width, fnt[*ptr2].height);
      cx = cx + fnt[*ptr2].width;
      ptr2++;
    }
  }
}

void texn2(byte *dest, int vga_width, byte *p, int x, int y, byte w, int h) {
  byte *q = dest + y * vga_width + x;
  int width = w;

  do {
    do {
      if (*p) {
        *q = *p;
      }
      p++;
      q++;
    } while (--w);
    q += vga_width - (w = width);
  } while (--h);
}

//----------------------------------------------------------------------------
//      calculate(math expression) Returns 0 on error, otherwise the result
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//    Expression evaluator (pointer passed in `expression')
//    If successful, returns token=p_num and token_number=n
//-----------------------------------------------------------------------------

enum tokens {
  p_inicio,
  p_ultimo,
  p_error,
  p_num,
  p_abrir,
  p_cerrar,
  p_add,
  p_sub,
  p_mul,
  p_div,
  p_mod,
  p_shl,
  p_shr,
  p_xor,
  p_or,
  p_and,
  p_not,
  p_sqrt,
  p_neg
};

int token;           // From the enum above
double token_number; // When token==p_num
char *expression;    // Pointer to the ASCIIZ expression

struct { // Holds the parsed expression
  int token;
  double number;
} expressions[64];

int num_expressions; // Number of elements stored in expressions[]

double do_evaluate(void);

void do_calculate(void) {
  double result;
  token = p_inicio;        // No token initially
  num_expressions = 0;     // Initialize expression counter
  get_token();             // Get the first token
  expres0();               // Parse the expression
  if (token == p_ultimo) { // Expression parsed successfully
    result = do_evaluate();
    if (token != p_error) { // Evaluated successfully
      token = p_num;
      token_number = result;
    }
  } else
    token = p_error;
}

double do_evaluate(void) {
  double stack[64];
  int sp = 0, n = 0;

  do {
    switch (expressions[n].token) {
    case p_num:
      stack[++sp] = expressions[n].number;
      break;
    case p_or:
      stack[sp - 1] = (double)((int)stack[sp - 1] | (int)stack[sp]);
      sp--;
      break;
    case p_xor:
      stack[sp - 1] = (double)((int)stack[sp - 1] ^ (int)stack[sp]);
      sp--;
      break;
    case p_and:
      stack[sp - 1] = (double)((int)stack[sp - 1] & (int)stack[sp]);
      sp--;
      break;
    case p_add:
      stack[sp - 1] += stack[sp];
      sp--;
      break;
    case p_sub:
      stack[sp - 1] -= stack[sp];
      sp--;
      break;
    case p_mul:
      stack[sp - 1] *= stack[sp];
      sp--;
      break;
    case p_div:
      if (stack[sp] == 0.0) {
        token = p_error;
        n = num_expressions;
      } else {
        stack[sp - 1] /= stack[sp];
        sp--;
      }
      break;
    case p_mod:
      if ((int)stack[sp] == 0) {
        token = p_error;
        n = num_expressions;
      } else {
        stack[sp - 1] = (double)((int)stack[sp - 1] % (int)stack[sp]);
        sp--;
      }
      break;
    case p_neg:
      stack[sp] = -stack[sp];
      break;
    case p_not:
      stack[sp] = (double)((int)stack[sp] ^ -1);
      break;
    case p_shr:
      stack[sp - 1] = (double)((int)stack[sp - 1] >> (int)stack[sp]);
      sp--;
      break;
    case p_shl:
      stack[sp - 1] = (double)((int)stack[sp - 1] << (int)stack[sp]);
      sp--;
      break;
    case p_sqrt:
      if (stack[sp] < 0) {
        token = p_error;
        n = num_expressions;
      } else {
        if (stack[sp] < 2147483648)
          stack[sp] = sqrt(stack[sp]);
        else {
          token = p_error;
          n = num_expressions;
        }
      }
      break;

    default:
      token = p_error;
      n = num_expressions;
      break;
    }
  } while (++n < num_expressions);

  if (sp != 1)
    token = p_error;

  return (stack[sp]);
}

void expres0() { // xor or and
  int p;
  expres1();
  while ((p = token) >= p_xor && p <= p_and) {
    get_token();
    expres1();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres1() { // << >>
  int p;
  expres2();
  while ((p = token) >= p_shl && p <= p_shr) {
    get_token();
    expres2();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres2() { // + -
  int p;
  expres3();
  while ((p = token) >= p_add && p <= p_sub) {
    get_token();
    expres3();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres3() { // * / %
  int p;
  expres4();
  while ((p = token) >= p_mul && p <= p_mod) {
    get_token();
    expres4();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres4() { // sign !
  int p;

  while ((p = token) == p_add) {
    get_token();
  }

  if (p == p_not || p == p_sub) {
    if (p == p_sub)
      p = p_neg;
    get_token();
    expres4();
    expressions[num_expressions].token = p;
    num_expressions++;
  } else
    expres5();
}

void expres5() {
  if (token == p_abrir) {
    get_token();
    expres0();
    if (token != p_cerrar) {
      token = p_error;
      return;
    } else
      get_token();
  } else if (token == p_sqrt) {
    get_token();
    expres5();
    expressions[num_expressions].token = p_sqrt;
    num_expressions++;
  } else if (token == p_num) {
    expressions[num_expressions].token = p_num;
    expressions[num_expressions].number = token_number;
    num_expressions++;
    get_token();
  } else {
    token = p_error;
  }
}

double get_num(void);

void get_token(void) {
  char cwork[32];
  int n;

  if (token != p_error) {
reget_token:
    switch (*expression++) {
    case ' ':
      goto reget_token;
    case 0:
      token = p_ultimo;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      token = p_num;
      expression--;
      token_number = get_num();
      break;
    case '(':
      token = p_abrir;
      break;
    case ')':
      token = p_cerrar;
      break;
    case '+':
      token = p_add;
      break;
    case '-':
      token = p_sub;
      break;
    case '*':
      token = p_mul;
      break;
    case '/':
      token = p_div;
      break;
    case '%':
      token = p_mod;
      break;
    case '<':
      if (*expression++ == '<')
        token = p_shl;
      else
        token = p_error;
      break;
    case '>':
      if (*expression++ == '>')
        token = p_shr;
      else
        token = p_error;
      break;
    case '^':
      if (*expression == '^')
        expression++;
      token = p_xor;
      break;
    case '|':
      if (*expression == '|')
        expression++;
      token = p_or;
      break;
    case '&':
      if (*expression == '&')
        expression++;
      token = p_and;
      break;
    case '!':
      token = p_not;
      break;
    default:
      if (tolower(*(expression - 1)) >= 'a' && tolower(*(expression - 1)) <= 'z') {
        n = 1;
        cwork[0] = tolower(*(expression - 1));
        while (n < 31 && tolower(*expression) >= 'a' && tolower(*expression) <= 'z') {
          cwork[n++] = tolower(*expression++);
        }
        cwork[n] = 0;
        if (!strcmp(cwork, "mod"))
          token = p_mod;
        else if (!strcmp(cwork, "not"))
          token = p_not;
        else if (!strcmp(cwork, "xor"))
          token = p_xor;
        else if (!strcmp(cwork, "or"))
          token = p_or;
        else if (!strcmp(cwork, "and"))
          token = p_and;
        else if (!strcmp(cwork, "sqrt"))
          token = p_sqrt;
        else if (!strcmp(cwork, "pi")) {
          token = p_num;
          token_number = 3.14159265359;
        } else
          token = p_error;
      } else {
        token = p_error;
      }
      break;
    }
  }
}

double get_num(void) { // Read the number at *expression (double in hex or dec)
  double x = 0;
  double dec = 10;

  if (*expression == '0' && tolower(*(expression + 1)) == 'x') { // Hex numbers

    expression += 2;
    if ((*expression >= '0' && *expression <= '9') ||
        (tolower(*expression) >= 'a' && tolower(*expression) <= 'f')) {
      do {
        if (*expression >= '0' && *expression <= '9')
          x = x * 16 + *expression++ - 0x30;
        else
          x = x * 16 + tolower(*expression++) - 'a' + 10;
      } while ((*expression >= '0' && *expression <= '9') ||
               (tolower(*expression) >= 'a' && tolower(*expression) <= 'f'));
    } else
      token = p_error;

  } else {
    while (*expression >= '0' && *expression <= '9') {
      x *= 10;
      x += (*expression - '0');
      expression++;
    }
    if (*expression == '.') {
      expression++;
      if (*expression < '0' || *expression > '9')
        token = p_error;
      else
        do {
          x += (double)(*expression - '0') / dec;
          dec *= 10;
          expression++;
        } while (*expression >= '0' && *expression <= '9');
    }
  }

  return (x);
}

void calculate(void) {
  expression = (char *)&mem[stack[sp]];
  do_calculate();
  if (token == p_num)
    stack[sp] = (int)token_number;
  else
    stack[sp] = 0;
}

//----------------------------------------------------------------------------
//      itoa(int) Returns the string
//----------------------------------------------------------------------------

extern int nullstring[4];
extern int nstring;

void __itoa(void) {
  itoa(stack[sp], (char *)&mem[nullstring[nstring]], 10);
  stack[sp] = nullstring[nstring];
  nstring = ((nstring + 1) & 3);
}

//----------------------------------------------------------------------------
//      malloc(elements) - Returns 0 or mem index (odd)
//----------------------------------------------------------------------------

// TODO: STRING variables inside malloc'd blocks lack the 0xDAD... sentinel
// marker, so the runtime cannot distinguish them from raw integer data.

void _malloc(void) {
  byte *p;
  int con = 0;

  while (con < 256 && divmalloc[con].ptr)
    con++;

  if (con == 256) {
    stack[sp] = 0;
    runtime_error(179);
    return;
  }

  if (stack[sp] < 1) {
    stack[sp] = 0;
    runtime_error(181);
    return;
  }

  divmalloc[con].ptr = (byte *)malloc(stack[sp] * 4 + 4 + 3);

  if (!divmalloc[con].ptr) {
    stack[sp] = 0;
    runtime_error(100);
    return;
  }

  memset(divmalloc[con].ptr, 0, stack[sp] * 4 + 4 + 3);

  p = (byte *)((((uintptr_t)divmalloc[con].ptr + 3) / 4) * 4);

  divmalloc[con].imem1 = ((uintptr_t)p - (uintptr_t)mem) / 4;

  if (!(divmalloc[con].imem1 & 1))
    divmalloc[con].imem1++;

  divmalloc[con].imem2 = divmalloc[con].imem1 + stack[sp];

  stack[sp] = divmalloc[con].imem1;
}

//----------------------------------------------------------------------------
//      free(mem index)
//----------------------------------------------------------------------------

void _free(void) {
  int con = 0;

  while (con < 256 && divmalloc[con].imem1 != stack[sp])
    con++;

  if (con == 256 || stack[sp] == 0) {
    stack[sp] = 0;
    runtime_error(180);
    return;
  }

  free(divmalloc[con].ptr);

  divmalloc[con].ptr = NULL;
  divmalloc[con].imem1 = 0;
  divmalloc[con].imem2 = 0;

  stack[sp] = 1;
}

//----------------------------------------------------------------------------
//      encode(offset, size, key) Returns 0 - 1
//----------------------------------------------------------------------------

void init_rnd_coder(int n, char *key);
byte rndb(void);

void encode(void) {
  int offset, size, key;
  int n;
  byte *ptr;

  key = stack[sp--];
  size = stack[sp--];
  offset = stack[sp];

  if (!validate_address(offset) || !validate_address(offset + size)) {
    stack[sp] = 0;
    runtime_error(182);
    return;
  }
  stack[sp] = 1;

  init_rnd_coder(size + 33, (char *)&mem[key]);
  ptr = (byte *)&mem[offset];
  for (n = 0; n < size * 4; n++) {
    ptr[n] ^= rndb();
  }
}

//----------------------------------------------------------------------------
//      encode_file(file, key) Returns 0 - 1
//----------------------------------------------------------------------------

void encode_file(int encode) {
  unsigned rc;
  struct find_t ft;
  int x;
  char cwork1[_MAX_PATH + 1];
  char cwork2[_MAX_PATH + 1];
  char cwork3[_MAX_PATH + 1];
  byte *name, *key;

  key = (byte *)&mem[stack[sp--]];
  name = (byte *)&mem[stack[sp]];

  stack[sp] = 1;

  div_strcpy(cwork2, sizeof(cwork2), (char *)name);
  for (x = strlen(cwork2) - 1;; x--) {
    if (x == -1) {
      cwork2[0] = 0;
      break;
    }
    if (IS_PATH_SEP(cwork2[x])) {
      cwork2[x + 1] = 0;
      break;
    }
  }

  rc = _dos_findfirst((char *)name, _A_NORMAL, &ft);
  while (!rc) {
    div_strcpy(cwork3, sizeof(cwork3), cwork2);
    div_strcat(cwork3, sizeof(cwork3), ft.name);
    if (_fullpath(cwork1, cwork3, _MAX_PATH) == NULL)
      div_strcpy(cwork1, sizeof(cwork1), ft.name);
    _dos_setfileattr(cwork1, _A_NORMAL);
    _encrypt(encode, cwork1, (char *)key);
    rc = _dos_findnext(&ft);
  }

  max_reloj += get_clock() - old_clock;
}

void _encrypt(int encode, char *filename, char *key) {
  char full[_MAX_PATH + 1];
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  int n, size;
  byte *ptr, *p;
  FILE *f;

  if ((f = fopen(filename, "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if ((ptr = (byte *)malloc(size)) != NULL) {
      fseek(f, 0, SEEK_SET);
      if (fread(ptr, 1, size, f) == size) {
        fclose(f);
      } else {
        fclose(f);
        free(ptr);
        stack[sp] = 0;
        runtime_error(127);
        return;
      }
    } else {
      fclose(f);
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
  } else {
    stack[sp] = 0;
    runtime_error(105);
    return;
  }

  if (encode) {
    if (!strcmp((char *)ptr, "dj!\x1a\x0d\x0a\xff"))
      return;
    else
      p = ptr;
  } else {
    if (strcmp((char *)ptr, "dj!\x1a\x0d\x0a\xff"))
      return;
    else {
      p = ptr + 8;
      size -= 8;
    }
  }

  init_rnd_coder(size + 1133, key);
  for (n = 0; n < size; n++)
    p[n] ^= rndb();

  _splitpath(filename, drive, dir, fname, ext);
  div_strcpy(ext, sizeof(ext), "dj!");
  _makepath(full, drive, dir, fname, ext);

  if (rename(filename, full)) {
    stack[sp] = 0;
    free(ptr);
    runtime_error(105);
    return;
  }

  if ((f = open_save_file((byte *)filename)) == NULL) {
    rename(full, filename);
    free(ptr);
    stack[sp] = 0;
    runtime_error(105);
    return;
  }

  if (encode) {
    if (fwrite("dj!\x1a\x0d\x0a\xff", 1, 8, f) != 8) {
      fclose(f);
      remove(filename);
      rename(full, filename);
      free(ptr);
      stack[sp] = 0;
      runtime_error(105);
      return;
    }
  }

  if (fwrite(p, 1, size, f) != size) {
    fclose(f);
    remove(filename);
    rename(full, filename);
    free(ptr);
    stack[sp] = 0;
    runtime_error(105);
    return;
  }

  // If everything went well ...

  fclose(f);
  free(ptr);
  remove(full);
}

//----------------------------------------------------------------------------
//      compress(file) Returns 0 - 1
//----------------------------------------------------------------------------

int divcompress(unsigned char *dest, unsigned long *destLen, unsigned char *source,
                unsigned long sourceLen);

void _compress(int encode) {
  unsigned rc;
  struct find_t ft;
  int x;
  char cwork1[_MAX_PATH + 1];
  char cwork2[_MAX_PATH + 1];
  char cwork3[_MAX_PATH + 1];
  byte *name;

  name = (byte *)&mem[stack[sp]];

  stack[sp] = 1;

  div_strcpy(cwork2, sizeof(cwork2), (char *)name);
  for (x = strlen(cwork2) - 1;; x--) {
    if (x == -1) {
      cwork2[0] = 0;
      break;
    }
    if (IS_PATH_SEP(cwork2[x])) {
      cwork2[x + 1] = 0;
      break;
    }
  }

  rc = _dos_findfirst((char *)name, _A_NORMAL, &ft);
  while (!rc) {
    div_strcpy(cwork3, sizeof(cwork3), cwork2);
    div_strcat(cwork3, sizeof(cwork3), ft.name);
    if (_fullpath(cwork1, cwork3, _MAX_PATH) == NULL)
      div_strcpy(cwork1, sizeof(cwork1), ft.name);
    _dos_setfileattr(cwork1, _A_NORMAL);
    _compress_file(encode, cwork1);
    rc = _dos_findnext(&ft);
  }

  max_reloj += get_clock() - old_clock;
}

void _compress_file(int encode, char *filename) {
  char full[_MAX_PATH + 1];
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  unsigned long size, size2;
  byte *ptr, *ptr_dest;
  FILE *f;

  if ((f = fopen(filename, "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if ((ptr = (byte *)malloc(size)) != NULL) {
      fseek(f, 0, SEEK_SET);
      if (fread(ptr, 1, size, f) == size) {
        fclose(f);
      } else {
        fclose(f);
        free(ptr);
        stack[sp] = 0;
        runtime_error(127);
        return;
      }
    } else {
      fclose(f);
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
  } else {
    stack[sp] = 0;
    runtime_error(105);
    return;
  }

  if (encode) {
    if (!strcmp((char *)ptr, "zx!\x1a\x0d\x0a\xff"))
      return;
    size2 = size + size / 100 + 256;
    if ((ptr_dest = (byte *)malloc(size2)) == NULL) {
      free(ptr);
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
#ifdef ZLIB
    if (compress(ptr_dest, &size2, ptr, size))
#else
    if (false)
#endif
    {
      free(ptr_dest);
      free(ptr);
      stack[sp] = 0;
      runtime_error(100);
      return;
    }

    // If no space is saved, leave the file uncompressed

    if (size2 >= size - 12) {
      free(ptr_dest);
      free(ptr);
      return;
    }

  } else {
    if (strcmp((char *)ptr, "zx!\x1a\x0d\x0a\xff"))
      return;
    size2 = *(int *)(ptr + 8);
    if ((ptr_dest = (byte *)malloc(size2)) == NULL) {
      free(ptr);
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
#ifdef ZLIB
    if (uncompress(ptr_dest, &size2, ptr + 12, size - 12))
#else
    if (true)
#endif
    {
      free(ptr_dest);
      free(ptr);
      stack[sp] = 0;
      runtime_error(100);
      return;
    }
    size2 = *(int *)(ptr + 8);
  }

  free(ptr);

  _splitpath(filename, drive, dir, fname, ext);
  div_strcpy(ext, sizeof(ext), "ZX!");
  _makepath(full, drive, dir, fname, ext);

  if (rename(filename, full)) {
    stack[sp] = 0;
    free(ptr_dest);
    runtime_error(105);
    return;
  }

  if ((f = open_save_file((byte *)filename)) == NULL) {
    rename(full, filename);
    free(ptr_dest);
    stack[sp] = 0;
    runtime_error(105);
    return;
  }

  if (encode) {
    if (fwrite("zx!\x1a\x0d\x0a\xff", 1, 8, f) != 8) {
      fclose(f);
      remove(filename);
      rename(full, filename);
      free(ptr_dest);
      stack[sp] = 0;
      runtime_error(105);
      return;
    }
    if (fwrite(&size, 1, 4, f) != 4) {
      fclose(f);
      remove(filename);
      rename(full, filename);
      free(ptr_dest);
      stack[sp] = 0;
      runtime_error(105);
      return;
    }
  }

  if (fwrite(ptr_dest, 1, size2, f) != size2) {
    fclose(f);
    remove(filename);
    rename(full, filename);
    free(ptr_dest);
    stack[sp] = 0;
    runtime_error(105);
    return;
  }

  // If everything went well ...

  fclose(f);
  free(ptr_dest);
  remove(full);
}

//----------------------------------------------------------------------------

// write ... printf, fprintf, itoatoi, calculate, ...
// text mode functions ... ???

//----------------------------------------------------------------------------
// WARNING: Validates a memory address against mem[] bounds and malloc'd blocks.
// Does not cover all malloc'd ranges — may miss dynamically allocated regions.
//----------------------------------------------------------------------------

int validate_address(int dir) {
  int n;

  if (dir > 0 && dir <= imem_max)
    return (dir);
  else {
    for (n = 0; n < 256; n++) {
      if (dir >= divmalloc[n].imem1 && dir <= divmalloc[n].imem2)
        return (dir);
    }
    return (0);
  }
}

//----------------------------------------------------------------------------
// Main switch
//----------------------------------------------------------------------------

#ifdef DEBUG
extern int f_time[256]; // Time consumed by each function
#endif

/* Built-in function dispatcher: called by the 'lfun' opcode. Reads the
 * function code from mem[ip], then dispatches via a large switch to the
 * corresponding runtime function (signal, collision, load_fpg, sound, etc.).
 * Arguments are popped from stack[] by each handler; results pushed back.
 */
void function(void) {
#ifdef DEBUG
  int oticks = get_ticks();
#endif

  old_clock = get_clock();

  switch (v_function = (byte)mem[ip++]) {
  case 0:
    _signal();
    break;
  case 1:
    _key();
    break;
  case 2:
    load_pal();
    break;
  case 3:
    load_fpg();
    break;
  case 4:
    start_scroll();
    break;
  case 5:
    stop_scroll();
    break;
  case 6:
    out_region();
    break;
  case 7:
    graphic_info();
    break;
  case 8:
    collision();
    break;
  case 9:
    get_id();
    break;
  case 10:
    get_disx();
    break;
  case 11:
    get_disy();
    break;
  case 12:
    get_angle();
    break;
  case 13:
    get_dist();
    break;
  case 14:
    fade();
    break;
  case 15:
    load_fnt();
    break;
  case 16:
    __write();
    break;
  case 17:
    write_int();
    break;
  case 18:
    delete_text();
    break;
  case 19:
    move_text();
    break;
  case 20:
    unload_fpg();
    break;
  case 21:
    divrandom();
    break;
  case 22:
    define_region();
    break;
  case 23:
    _xput();
    break;
  case 24:
    _put();
    break;
  case 25:
    put_screen();
    break;
  case 26:
    map_xput();
    break;
  case 27:
    map_put();
    break;
  case 28:
    put_pixel();
    break;
  case 29:
    get_pixel();
    break;
  case 30:
    map_put_pixel();
    break;
  case 31:
    map_get_pixel();
    break;
  case 32:
    get_point();
    break;
  case 33:
    clear_screen();
    break;
  case 34:
    save();
    break;
  case 35:
    load();
    break;
  case 36:
    set_mode();
    break;
  case 37:
    load_pcm();
    break;
  case 38:
    unload_pcm();
    break;
  case 39:
    _sound();
    break;
  case 40:
    stop_sound();
    break;
  case 41:
    change_sound();
    break;
  case 42:
    set_fps();
    break;
  case 43:
    start_fli();
    break;
  case 44:
    frame_fli();
    break;
  case 45:
    end_fli();
    break;
  case 46:
    reset_fli();
    break;
  case 47:
    _system();
    break;
  case 48:
    refresh_scroll();
    break;
  case 49:
    fget_dist();
    break;
  case 50:
    fget_angle();
    break;
  case 51:
    sp--;
    break; // play_cd removed (CDDA deleted)
  case 52:
    stack[++sp] = 0;
    break; // stop_cd removed (CDDA deleted)
  case 53:
    stack[++sp] = 0;
    break; // is_playing_cd removed (CDDA deleted)
  case 54:
    start_mode7();
    break;
  case 55:
    stop_mode7();
    break;
  case 56:
    advance();
    break;
  case 57:
    _abs();
    break;
  case 58:
    fade_on();
    break;
  case 59:
    fade_off();
    break;
  case 60:
    rand_seed();
    break;
  case 61:
    _sqrt();
    break;
  case 62:
    _pow();
    break;
  case 63:
    map_block_copy();
    break;
  case 64:
    _move_scroll();
    break;
  case 65:
    near_angle();
    break;
  case 66:
    let_me_alone();
    break;
  case 67:
    _exit_dos();
    break;
  case 68:
    roll_palette();
    break;
  case 69:
    get_real_point();
    break;
  case 70:
    get_joy_button();
    break;
  case 71:
    get_joy_position();
    break;
  case 72:
    convert_palette();
    break;
  case 73:
    load_map();
    break;
  case 74:
    reset_sound();
    break;
  case 75:
    unload_map();
    break;
  case 76:
    unload_fnt();
    break;
  case 77:
    set_volume();
    break;
  case 78:
    set_color();
    break;

  case 79:
    x_advance();
    break;
  case 80:
    _strchar();
    break;
  case 81:
    path_find();
    break;
  case 82:
    path_line();
    break;
  case 83:
    path_free();
    break;
  case 84:
    new_map();
    break;

  case 85:
    _strcpy();
    break;
  case 86:
    _strcat();
    break;
  case 87:
    _strlen();
    break;
  case 88:
    _strcmp();
    break;
  case 89:
    _strchr();
    break;
  case 90:
    _strstr();
    break;
  case 91:
    __strset();
    break;
  case 92:
    __strupr();
    break;
  case 93:
    __strlwr();
    break;
  case 94:
    _strdel();
    break;
  case 95:
    screen_copy();
    break;
  case 96:
    sort();
    break;
  case 97:
    load_song();
    break;
  case 98:
    unload_song();
    break;
  case 99:
    song();
    break;
  case 100:
    stop_song();
    break;
  case 101:
    set_song_pos();
    break;
  case 102:
    get_song_pos();
    break;
  case 103:
    get_song_line();
    break;
  case 104:
    is_playing_sound();
    break;
  case 105:
    is_playing_song();
    break;
  case 106:
    _fopen();
    break;
  case 107:
    _fclose();
    break;
  case 108:
    _fread();
    break;
  case 109:
    _fwrite();
    break;
  case 110:
    _fseek();
    break;
  case 111:
    _ftell();
    break;
  case 112:
    __filelength();
    break;
  case 113:
    flush();
    break;
  case 114:
    get_dirinfo();
    break;
  case 115:
    get_fileinfo();
    break;
  case 116:
    get_drive();
    break;
  case 117:
    set_drive();
    break;
  case 118:
    div_chdir();
    break;
  case 119:
    _mkdir();
    break;
  case 120:
    remove_file();
    break;
  case 121:
    disk_free();
    break;
  case 122:
    memory_free();
    break;
  case 123:
    ignore_error();
    break;
  case 124:
    save_mapcx(1);
    break;
  case 125:
    _sin();
    break;
  case 126:
    _cos();
    break;
  case 127:
    _tan();
    break;
  case 128:
    _asin();
    break;
  case 129:
    _acos();
    break;
  case 130:
    _atan();
    break;
  case 131:
    _atan2();
    break;
  case 132:
    draw();
    break;
  case 133:
    delete_draw();
    break;
  case 134:
    move_draw();
    break;
  case 135:
    save_mapcx(0);
    break;
  case 136:
    write_in_map();
    break;
  case 137:
    calculate();
    break;
  case 138:
    __itoa();
    break;
  case 139:
    change_channel();
    break;
  case 140:
    _malloc();
    break;
  case 141:
    _free();
    break;
  case 142:
    encode();
    break;
  case 143:
    encode_file(1);
    break;
  case 144:
    encode_file(0);
    break;
  case 145:
    _compress(1);
    break;
  case 146:
    _compress(0);
    break;
  case 147:
    _find_color();
    break;
  case 148:
    load_screen();
    break;
  case 149:
    force_pal();
    break;
  }

#ifdef DEBUG
  function_exec(v_function, get_ticks() - oticks);
#endif
}

// To add a function:
// In ltobj.def, in the switch above, and in fname (and docs: add.prg and help)

char *fname[] = {
    "signal", "key", "load_pal", "load_fpg", "start_scroll", "stop_scroll", "out_region",
    "graphic_info", "collision", "get_id", "get_distx", "get_disty", "get_angle", "get_dist",
    "fade", "load_fnt", "write", "write_int", "delete_text", "move_text", "unload_fpg", "rand",
    "define_region", "xput", "put", "put_screen", "map_xput", "map_put", "put_pixel", "get_pixel",
    "map_put_pixel", "map_get_pixel", "get_point", "clear_screen", "save", "load", "set_mode",
    "load_pcm/wav", "unload_pcm/wav", "sound", "stop_sound", "change_sound", "set_fps", "start_fli",
    "frame_fli", "end_fli", "reset_fli", "system", "refresh_scroll", "fget_dist", "fget_angle", "",
    "", "", // Removed CD Music functions
    "start_mode7", "stop_mode7", "advance", "abs", "fade_on", "fade_off", "rand_seed", "sqrt",
    "pow", "map_block_copy", "move_scroll", "near_angle", "let_me_alone", "exit", "roll_palette",
    "get_real_point", "get_joy_button", "get_joy_position", "convert_palette", "load_map/pcx",
    "reset_sound", "unload_map/pcx", "unload_fnt", "set_volume",

    // New functions added for DIV 2.0

    "set_color", "x_advance", "char", "path_find", "path_line", "path_free", "new_map", "strcpy",
    "strcat", "strlen", "strcmp", "strchr", "strstr", "strset", "strupr", "strlwr", "strdel",
    "screen_copy", "sort", "load_song", "unload_song", "song", "stop_song", "set_song_pos",
    "get_song_pos", "get_song_line", "is_playing_sound", "is_playing_song", "fopen", "fclose",
    "fread", "fwrite", "fseek", "ftell", "filelength", "flush", "get_dirinfo", "get_fileinfo",
    "getdrive", "setdrive", "chdir", "mkdir", "remove", "disk_free", "memory_free", "ignore_error",
    "save_pcx", "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "draw", "delete_draw",
    "move_draw", "save_map", "write_in_map", "calculate", "itoa", "change_channel", "malloc",
    "free", "encode", "encode_file", "decode_file", "compress_file", "uncompress_file",
    "find_color", "load_screen", "force_pal"};
