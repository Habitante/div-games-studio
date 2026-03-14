
//-----------------------------------------------------------------------------
// Brush/texture thumbnail infrastructure for the paint editor.
// Extracted from divmap3d.c after removing the MODE8/3D map editor.
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

#define FONDO 3

//-----------------------------------------------------------------------------
// FPG file format types (were in divmap3d.c, needed for thumbnail loading)
//-----------------------------------------------------------------------------

typedef struct {
  char magic1[3];
  char magic2[4];
  char version;
  byte pal[768];
  byte gamas[576];
} FPG_header;

typedef struct {
  int cod;
  int size;
  char descrip[32];
  char filename[12];
  int width;
  int height;
  int points;
} FPG_info;

typedef struct {
  word x;
  word y;
} FPG_points;

typedef struct {
  FPG_info info;
  FPG_points *points;
  byte *image;
} FPG_data;

//-----------------------------------------------------------------------------
// External prototypes
//-----------------------------------------------------------------------------

void paint_slider_br(struct t_listboxbr *l);

// Forward declarations for this file
void mapper_browse_fpg1(void);
void mapper_browse_fpg2(void);
void M3D_paint_listboxbr(struct t_listboxbr *l);
void M3D_show_thumb(struct t_listboxbr *l, int num);
void M3D_create_listboxbr(struct t_listboxbr *l);

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

// Path to brush FPG file (replaces m3d_edit.fpg_path)
byte brush_fpg_path[256];

int tex_sop[11] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

int scroll_x, scroll_y;
int flag_num = 0;

byte FPG_pal[768];
byte FPG_xlat[256];
int fpg_thumb_pos;

char strings[5][10];

int ceiling_height = 2048;
int floor_height = 1024;

#define max_texturas 1000
#define w_textura    (3 + 1) // 000 - 999
char m3d_fpgcodesbr[max_texturas * w_textura];
char texture_names[max_texturas * w_textura];
char background_names[max_texturas * w_textura];
int tex_count;
int bg_count;
struct t_listboxbr texture_list_br = {3 - 2, 11 - 2, m3d_fpgcodesbr, w_textura, 4, 4, 32, 32};

struct _thumb_tex {
  int w, h;
  int real_width, real_height;
  char *ptr;
  int status;
  int FilePos;
  int Code;
  int is_square;
} thumb_tex[max_texturas];

extern int old_mouse_x, old_mouse_y, omx, omy, oclock, num, incremento;

struct {
  int pos;
  int total;
} FPG_progress;

int texture_type;

struct t_listboxbr copia_br;

int t_pulsada = 1;

float zoom_level;

extern int brush_index;
extern int browser_type;
extern struct _thumb_map {
  int w, h;
  int real_width, real_height;
  char *ptr;
  int status;
  int FilePos;
  int Code;
  int is_square;
} thumb_map[];

//-----------------------------------------------------------------------------
// Comparison functions for qsort
//-----------------------------------------------------------------------------

int strcmp_sort(const void *a, const void *b) {
  return strcmp((char *)a, (char *)b);
}

int cmpcode(const void *a, const void *b) {
  return (((struct _thumb_tex *)a)->Code - ((struct _thumb_tex *)b)->Code);
}

//-----------------------------------------------------------------------------
// M3D_create_thumbs - Load FPG file and create brush thumbnails
//-----------------------------------------------------------------------------

void M3D_create_thumbs(struct t_listboxbr *l, int prog) {
  FILE *FPG_F;
  FPG_header FPG_H;
  FPG_data FPG_D;
  char cwork[4];
  int n, con;
  int m;
  int man, mal;
  byte *temp, *temp2;
  float coefredy, coefredx, a, b;
  int x, y;

  if (brush_fpg_path[0] == 0)
    return;

  if (texture_type > 3)
    fpg_thumb_pos = 0;

  thumb_tex[0].ptr = NULL;
  thumb_tex[0].status = 0;
  thumb_tex[0].Code = 0;
  div_strcpy(&texture_names[0], w_textura, "000");
  div_strcpy(&background_names[0], w_textura, "000");
  tex_count = bg_count = 1;

  div_snprintf(strings[0], sizeof(strings[0]), "%d", scroll_x);
  div_snprintf(strings[1], sizeof(strings[1]), "%d", scroll_y);
  div_snprintf(strings[2], sizeof(strings[2]), "%d", flag_num);
  div_snprintf(strings[3], sizeof(strings[3]), "%d", ceiling_height);
  div_snprintf(strings[4], sizeof(strings[4]), "%d", floor_height);

  // Free existing thumbnails
  for (n = 0; n < max_texturas; n++) {
    if (thumb_tex[n].ptr != NULL) {
      free(thumb_tex[n].ptr);
      thumb_tex[n].ptr = NULL;
    }
  }

  if (brush_fpg_path[0] == 0)
    return;
  debugprintf("Trying to load %s\n", (char *)brush_fpg_path);

  if ((FPG_F = fopen((char *)brush_fpg_path, "rb")) == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return;
  }
  fseek(FPG_F, 0, SEEK_END);
  FPG_progress.pos = 1352;
  FPG_progress.total = ftell(FPG_F) * 2;
  fseek(FPG_F, 0, SEEK_SET);

  if (fread(&FPG_H, 1, sizeof(FPG_header), FPG_F) != sizeof(FPG_header)) {
    fclose(FPG_F);
    v_text = (char *)texts[44];
    show_dialog(err0);
    return;
  }

  memcpy(FPG_pal, FPG_H.pal, 768);
  create_dac4();

  if (draw_mode < 100) {
    for (n = 0; n < 256; n++) {
      switch ((FPG_pal[n * 3] + FPG_pal[n * 3 + 1] + FPG_pal[n * 3 + 2]) / 22) {
      case 1:
        FPG_xlat[n] = c0;
        break;
      case 2:
        FPG_xlat[n] = c01;
        break;
      case 3:
        FPG_xlat[n] = c1;
        break;
      case 4:
        FPG_xlat[n] = c12;
        break;
      case 5:
        FPG_xlat[n] = c2;
        break;
      case 6:
        FPG_xlat[n] = c23;
        break;
      case 7:
        FPG_xlat[n] = c3;
        break;
      case 8:
        FPG_xlat[n] = c34;
        break;
      case 9:
        FPG_xlat[n] = c4;
        break;
      }
    }
    FPG_xlat[0] = 0;
  } else {
    for (n = 0; n < 256; n++)
      FPG_xlat[n] = fast_find_color(FPG_pal[n * 3], FPG_pal[n * 3 + 1], FPG_pal[n * 3 + 2]);
  }

  // In paint mode, start from index 0 (no empty texture slot)
  n = tex_count = bg_count = 0;
  for (;;) {
    if (fread(&(FPG_D.info), 1, sizeof(FPG_info), FPG_F) != sizeof(FPG_info)) {
      if (feof(FPG_F)) {
        l->total_items = bg_count = n;
        break;
      } else {
        fclose(FPG_F);
        if (prog)
          show_progress((char *)texts[93], FPG_progress.total, FPG_progress.total);
        v_text = (char *)texts[44];
        show_dialog(err0);
        return;
      }
    }
    fseek(FPG_F, sizeof(FPG_points) * FPG_D.info.points, SEEK_CUR);
    FPG_progress.pos += (64 + 4 * FPG_D.info.points + FPG_D.info.width * FPG_D.info.height);
    if (prog)
      show_progress((char *)texts[93], FPG_progress.pos, FPG_progress.total);

    thumb_tex[n].is_square = 0;
    for (con = 0; con < 11; con++) {
      if (FPG_D.info.width == tex_sop[con] && FPG_D.info.height == tex_sop[con]) {
        thumb_tex[n].is_square = 1;
      }
    }

    thumb_tex[n].w = FPG_D.info.width;
    thumb_tex[n].h = FPG_D.info.height;
    thumb_tex[n].Code = FPG_D.info.cod;

    DIV_SPRINTF(cwork, "%03d", FPG_D.info.cod);
    if (thumb_tex[n].is_square) {
      div_strcpy(texture_names + tex_count * w_textura, w_textura, cwork);
      tex_count++;
    }
    div_strcpy(background_names + n * w_textura, w_textura, cwork);

    thumb_tex[n].FilePos = ftell(FPG_F);

    if ((FPG_D.image = (byte *)malloc(FPG_D.info.width * FPG_D.info.height)) == NULL) {
      for (n = 0; n < max_texturas; n++) {
        if (thumb_tex[n].ptr != NULL) {
          free(thumb_tex[n].ptr);
          thumb_tex[n].ptr = NULL;
        }
      }
      fclose(FPG_F);
      if (prog)
        show_progress((char *)texts[93], FPG_progress.total, FPG_progress.total);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    if (fread(FPG_D.image, 1, FPG_D.info.width * FPG_D.info.height, FPG_F) !=
        FPG_D.info.width * FPG_D.info.height) {
      for (n = 0; n < max_texturas; n++) {
        if (thumb_tex[n].ptr != NULL) {
          free(thumb_tex[n].ptr);
          thumb_tex[n].ptr = NULL;
        }
      }
      fclose(FPG_F);
      free(FPG_D.image);
      if (prog)
        show_progress((char *)texts[93], FPG_progress.total, FPG_progress.total);
      v_text = (char *)texts[44];
      show_dialog(err0);
      return;
    }

    thumb_tex[n].ptr = (char *)FPG_D.image;

    n++;
  }

  fclose(FPG_F);

  // In paint mode, start from index 0 (no empty texture slot)
  for (con = 0; con < l->total_items; con++) {
    for (;;) {
      man = thumb_tex[con].real_width = thumb_tex[con].w;
      mal = thumb_tex[con].real_height = thumb_tex[con].h;
      temp = (byte *)thumb_tex[con].ptr;

      if (man <= 32 * big2 && mal <= 32 * big2) {
        for (n = thumb_tex[con].w * thumb_tex[con].h - 1; n >= 0; n--) {
          temp[n] = FPG_xlat[temp[n]];
        }
      } else {
        coefredx = man / ((float)32 * 2 * (float)big2);
        coefredy = mal / ((float)32 * 2 * (float)big2);
        thumb_tex[con].w = (float)man / coefredx + 0.5;
        thumb_tex[con].h = (float)mal / coefredy + 0.5;
        thumb_tex[con].w &= -2;
        thumb_tex[con].h &= -2;
        if (thumb_tex[con].w < 2)
          thumb_tex[con].w = 2;
        if (thumb_tex[con].h < 2)
          thumb_tex[con].h = 2;

        if ((temp2 = (byte *)malloc(thumb_tex[con].w * thumb_tex[con].h)) == NULL) {
          free(thumb_tex[con].ptr);
          thumb_tex[con].ptr = NULL;
          thumb_tex[con].status = 0;
          break;
        }

        memset(temp2, 0, thumb_tex[con].w * thumb_tex[con].h);
        a = (float)0.0;
        for (y = 0; y < thumb_tex[con].h; y++) {
          b = (float)0.0;
          for (x = 0; x < thumb_tex[con].w; x++) {
            temp2[y * thumb_tex[con].w + x] = temp[((int)a) * man + (int)b];
            b += coefredx;
          }
          a += coefredy;
        }

        free(thumb_tex[con].ptr);

        for (n = thumb_tex[con].w * thumb_tex[con].h - 1; n >= 0; n--) {
          temp2[n] = FPG_xlat[temp2[n]];
        }

        if ((thumb_tex[con].ptr = (char *)malloc((thumb_tex[con].w * thumb_tex[con].h) / 4)) ==
            NULL) {
          free(temp2);
          thumb_tex[con].ptr = NULL;
          thumb_tex[con].status = 0;
          break;
        }
        for (y = 0; y < thumb_tex[con].h; y += 2) {
          for (x = 0; x < thumb_tex[con].w; x += 2) {
            n = *(ghost + temp2[x + y * thumb_tex[con].w] * 256 +
                  temp2[x + 1 + y * thumb_tex[con].w]);
            m = *(ghost + temp2[x + (y + 1) * thumb_tex[con].w] * 256 +
                  temp2[x + 1 + (y + 1) * thumb_tex[con].w]);
            thumb_tex[con].ptr[x / 2 + (y / 2) * (thumb_tex[con].w / 2)] = *(ghost + n * 256 + m);
          }
        }
        thumb_tex[con].w /= 2;
        thumb_tex[con].h /= 2;
        free(temp2);
      }
      thumb_tex[con].status = 1;
      FPG_progress.pos += (FPG_progress.total - FPG_progress.pos) / (l->total_items - con);
      if (prog)
        show_progress((char *)texts[93], FPG_progress.pos, FPG_progress.total);
      break;
    }
  }

  if (FPG_progress.pos < FPG_progress.total) {
    if (prog)
      show_progress((char *)texts[93], FPG_progress.total, FPG_progress.total);
  }

  qsort(thumb_tex, l->total_items, sizeof(struct _thumb_tex), cmpcode);
  qsort(texture_names, tex_count, w_textura, strcmp_sort);
  qsort(background_names, l->total_items, w_textura, strcmp_sort);
}

//-----------------------------------------------------------------------------
// M3D_show_thumb - Display a thumbnail in the browser
//-----------------------------------------------------------------------------

#define MAPBR 8

void M3D_show_thumb(struct t_listboxbr *l, int num) {
  byte *ptr = v.ptr, c;
  int w = v.w / big2, h = v.h / big2;
  int px, py, x, y, ly, incy;
  int num_tex;
  char *p;
  struct _thumb_tex thumb_tmp[max_texturas];

  if (browser_type == MAPBR)
    memcpy(&thumb_tmp, &thumb_map, sizeof(struct _thumb_map) * MAX_WINDOWS);
  else
    memcpy(&thumb_tmp, &thumb_tex, sizeof(struct _thumb_tex) * max_texturas);

  if (num >= l->first_visible && num < l->first_visible + l->lines * l->columns) {
    if (browser_type == MAPBR) {
      num_tex = num;
    } else {
      num_tex = atoi(m3d_fpgcodesbr + num * w_textura);
      if (num_tex) {
        for (x = 0; x < max_texturas; x++) {
          if (thumb_tmp[x].Code == num_tex) {
            num_tex = x;
            break;
          }
        }
      }
    }

    px = (l->x + 1 + (l->w + 1) * ((num - l->first_visible) % l->columns)) * big2 +
         (l->w * big2 - thumb_tmp[num_tex].w) / 2;
    if ((incy = ((l->h - 8) * big2 - thumb_tmp[num_tex].h) / 2) < 0)
      incy = 0;
    py = (l->y + 1 + (l->h + 1) * ((num - l->first_visible) / l->columns)) * big2 + incy;

    ly = (l->y + (l->h + 1) * ((num - l->first_visible) / l->columns) + l->h - 8) * big2;

    wbox(ptr, w, h, c01, (px - (l->w * big2 - thumb_tmp[num_tex].w) / 2) / big2,
         (py - incy) / big2 + l->h - 8, l->w, 8);

    if (thumb_tmp[num_tex].ptr != NULL && thumb_tmp[num_tex].status) {
      if (browser_type == 4 /*BRUSH*/)
        for (y = 0; y < thumb_tmp[num_tex].h; y++) {
          for (x = 0; x < thumb_tmp[num_tex].w; x++) {
            if (py + y > ly)
              c = *(ghost + c0 * 256 + thumb_tmp[num_tex].ptr[y * thumb_tmp[num_tex].w + x]);
            else
              c = thumb_tmp[num_tex].ptr[y * thumb_tmp[num_tex].w + x];
            if (c)
              v.ptr[(py + y) * v.w + (px + x)] = c;
          }
        }
      else
        for (y = 0; y < thumb_tmp[num_tex].h; y++) {
          for (x = 0; x < thumb_tmp[num_tex].w; x++) {
            if (py + y > ly)
              v.ptr[(py + y) * v.w + (px + x)] =
                  *(ghost + c0 * 256 + thumb_tmp[num_tex].ptr[y * thumb_tmp[num_tex].w + x]);
            else
              v.ptr[(py + y) * v.w + (px + x)] =
                  thumb_tmp[num_tex].ptr[y * thumb_tmp[num_tex].w + x];
          }
        }
    }

    px -= (l->w * big2 - thumb_tmp[num_tex].w) / 2;
    py -= incy;
    px /= big2;
    py /= big2;

    if (thumb_tmp[num_tex].ptr == NULL && !thumb_tmp[num_tex].status && num != 0) {
      wput(ptr, w, h, px + (l->w - 21) / 2, py + 1, 60);
    }

    if (browser_type != MAPBR) {
      if (draw_mode < 100 && num_tex == brush_index) {
        wbox(ptr, w, h, c_b_low, px, py + l->h - 8, l->w, 8);
      }

      py += l->h - 1;
      p = l->list + l->item_width * num;

      if (l->zone - 10 == num - l->first_visible)
        x = c4;
      else
        x = c3;
      wwrite(ptr, w, h, px + l->w / 2 + 1, py, 7, (byte *)p, c0);
      wwrite(ptr, w, h, px + l->w / 2, py, 7, (byte *)p, x);
    }

    v.redraw = 1;
  }
}

//-----------------------------------------------------------------------------
// M3D_create_listboxbr - Create a thumbnail listbox browser
//-----------------------------------------------------------------------------

void M3D_create_listboxbr(struct t_listboxbr *l) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int x, y;

  if (!l->created) {
    l->slide = l->s0 = l->y + 9;
    l->s1 = l->y + (l->h * l->lines + l->lines + 1) - 12;
    l->buttons = 0;
    l->created = 1;
    l->zone = 0;
    l->first_visible = fpg_thumb_pos;
    if ((l->first_visible + (l->lines - 1) * l->columns) >= l->total_items) {
      l->first_visible = 0;
    }
  }

  wbox(ptr, w, h, c1, l->x, l->y, (l->w + 1) * l->columns, (l->h + 1) * l->lines);

  for (y = 0; y < l->lines; y++)
    for (x = 0; x < l->columns; x++)
      wrectangle(ptr, w, h, c0, l->x + (x * (l->w + 1)), l->y + (y * (l->h + 1)), l->w + 2,
                 l->h + 2);

  wrectangle(ptr, w, h, c0, l->x + (l->w + 1) * l->columns, l->y, 9, (l->h + 1) * l->lines + 1);
  wrectangle(ptr, w, h, c0, l->x + (l->w + 1) * l->columns, l->y + 8, 9,
             (l->h + 1) * l->lines - 15);
  wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + 1, -39);
  wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + (l->h + 1) * l->lines - 7, -40);

  M3D_paint_listboxbr(l);
  paint_slider_br(l);
}

//-----------------------------------------------------------------------------
// M3D_update_listboxbr - Update a thumbnail listbox browser
//-----------------------------------------------------------------------------

void M3D_update_listboxbr(struct t_listboxbr *l) {
  byte *ptr = v.ptr, *p;
  int w = v.w / big2, h = v.h / big2;
  int n, old_zona = l->zone, x, y;

  if (wmouse_in(l->x, l->y, (l->w + 1) * l->columns, (l->h + 1) * l->lines)) {
    l->zone = (wmouse_x - l->x) / (l->w + 1) + ((wmouse_y - l->y) / (l->h + 1)) * l->columns;
    if (l->zone >= l->total_items - l->first_visible || l->zone >= l->lines * l->columns)
      l->zone = 1;
    else
      l->zone += 10;
  } else if (wmouse_in(l->x + (l->w + 1) * l->columns, l->y, 9, 9))
    l->zone = 2;
  else if (wmouse_in(l->x + (l->w + 1) * l->columns, l->y + (l->h + 1) * l->lines - 8, 9, 9))
    l->zone = 3;
  else if (wmouse_in(l->x + (l->w + 1) * l->columns, l->y + 9, 9, (l->h + 1) * l->lines - 17))
    l->zone = 4;
  else
    l->zone = 0;

  if (browser_type != MAPBR) {
    if (old_zona != l->zone)
      if (old_zona >= 10) {
        x = l->x + 1 + ((old_zona - 10) % l->columns) * (l->w + 1);
        y = l->y + l->h + ((old_zona - 10) / l->columns) * (l->h + 1);
        p = (byte *)l->list + l->item_width * (l->first_visible + old_zona - 10);
        wwrite(ptr, w, h, x + l->w / 2, y, 7, p, c3);
        v.redraw = 1;
      }
  }

  if (l->zone == 2 && (mouse_b & MB_LEFT)) {
    if (prev_mouse_buttons & MB_LEFT) {
      retrace_wait();
      retrace_wait();
      retrace_wait();
      retrace_wait();
    }
    if (l->first_visible) {
      l->first_visible -= l->columns;
      M3D_paint_listboxbr(l);
      v.redraw = 1;
    }
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + 1, -41);
    l->buttons |= 1;
    v.redraw = 1;
  } else if (l->buttons & 1) {
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + 1, -39);
    l->buttons ^= 1;
    v.redraw = 1;
  }

  if (l->zone == 3 && (mouse_b & MB_LEFT)) {
    if (prev_mouse_buttons & MB_LEFT) {
      retrace_wait();
      retrace_wait();
      retrace_wait();
      retrace_wait();
    }
    n = l->total_items - l->first_visible;
    if (n > l->lines * l->columns) {
      l->first_visible += l->columns;
      M3D_paint_listboxbr(l);
      v.redraw = 1;
    }
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + (l->h + 1) * l->lines - 7, -42);
    l->buttons |= 2;
    v.redraw = 1;
  } else if (l->buttons & 2) {
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + (l->h + 1) * l->lines - 7, -40);
    l->buttons ^= 2;
    v.redraw = 1;
  }

  if (l->zone == 4 && (mouse_b & MB_LEFT)) {
    l->slide = wmouse_y - 1;
    if (l->slide < l->s0)
      l->slide = l->s0;
    else if (l->slide > l->s1)
      l->slide = l->s1;

    if (l->total_items > l->lines * l->columns) {
      n = (l->total_items - l->lines * l->columns + l->columns - 1) / l->columns;

      n = 0.5 + (float)(n * (l->slide - l->s0)) / (l->s1 - l->s0);

      if (n != l->first_visible / l->columns) {
        l->first_visible = n * l->columns;
        M3D_paint_listboxbr(l);
      }
    }
    paint_slider_br(l);
    v.redraw = 1;

  } else {
    if (l->total_items <= l->lines * l->columns)
      n = l->s0;
    else {
      n = (l->total_items - l->lines * l->columns + l->columns - 1) / l->columns;

      n = (l->s0 * (n - l->first_visible / l->columns) + l->s1 * (l->first_visible / l->columns)) /
          n;
    }
    if (n != l->slide) {
      l->slide = n;
      paint_slider_br(l);
      v.redraw = 1;
    }
  }

  if (browser_type != MAPBR) {
    if (old_zona != l->zone)
      if (l->zone >= 10) {
        x = l->x + 1 + ((l->zone - 10) % l->columns) * (l->w + 1);
        y = l->y + l->h + ((l->zone - 10) / l->columns) * (l->h + 1);
        p = (byte *)l->list + l->item_width * (l->first_visible + l->zone - 10);
        wwrite(ptr, w, h, x + l->w / 2, y, 7, p, c4);
        v.redraw = 1;
      }
  }

  switch (l->zone) {
  case 2:
    mouse_graf = 7;
    break;
  case 3:
    mouse_graf = 9;
    break;
  case 4:
    mouse_graf = 13;
    break;
  }
}

//-----------------------------------------------------------------------------
// M3D_paint_listboxbr - Paint all thumbnails in the listbox
//-----------------------------------------------------------------------------

void M3D_paint_listboxbr(struct t_listboxbr *l) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int n, y, x;

  for (y = 0; y < l->lines; y++)
    for (x = 0; x < l->columns; x++) {
      wbox(ptr, w, h, c1, l->x + (x * (l->w + 1)) + 1, l->y + (y * (l->h + 1)) + 1, l->w, l->h - 8);
      wbox(ptr, w, h, c01, l->x + (x * (l->w + 1)) + 1, l->y + (y * (l->h + 1)) + 1 + l->h - 8,
           l->w, 8);
    }

  if (wmouse_in(l->x, l->y, (l->w + 1) * l->columns, (l->h + 1) * l->lines)) {
    l->zone = ((mouse_x - v.x) / big2 - l->x) / (l->w + 1) +
              (((mouse_y - v.y) / big2 - l->y) / (l->h + 1)) * l->columns;
    if (l->zone >= l->total_items - l->first_visible || l->zone >= l->lines * l->columns)
      l->zone = 1;
    else
      l->zone += 10;
  } else if (wmouse_in(l->x + (l->w + 1) * l->columns, l->y, 9, 9))
    l->zone = 2;
  else if (wmouse_in(l->x + (l->w + 1) * l->columns, l->y + (l->h + 1) * l->lines - 8, 9, 9))
    l->zone = 3;
  else if (wmouse_in(l->x + (l->w + 1) * l->columns, l->y + 9, 9, (l->h + 1) * l->lines - 17))
    l->zone = 4;
  else
    l->zone = 0;

  n = l->total_items - l->first_visible;
  if (n > l->lines * l->columns)
    n = l->lines * l->columns;
  while (n > 0)
    M3D_show_thumb(l, l->first_visible + --n);
}

//-----------------------------------------------------------------------------
// MapperBrowseFPG - Brush/texture selection dialog
//-----------------------------------------------------------------------------

extern int m_maximo;
extern struct t_listboxbr thumbmap_list_br;

#define BRUSH 4

void mapper_browse_fpg0(void) {
  v.type = WIN_DIALOG;
  v.w = 147 - 4;
  v.h = 147 - 4;
  if (texture_type > 3) {
    if (browser_type == BRUSH) {
      v.title = texts[572];
      v.name = texts[572];
    } else {
      v.title = texts[573];
      v.name = texts[573];
    }
  } else {
    v.title = texts[433];
    v.name = texts[433];
  }

  v.paint_handler = mapper_browse_fpg1;
  v.click_handler = mapper_browse_fpg2;

  if (draw_mode < 100) {
    if (browser_type == MAPBR) {
      memcpy(&copia_br, &texture_list_br, sizeof(texture_list_br));
      memcpy(&texture_list_br, &thumbmap_list_br, sizeof(texture_list_br));
      texture_list_br.total_items = m_maximo;
    } else {
      memcpy(m3d_fpgcodesbr, background_names, max_texturas * w_textura);
      texture_list_br.total_items = bg_count;
    }
  } else {
    if (texture_type == FONDO) {
      memcpy(m3d_fpgcodesbr, background_names, max_texturas * w_textura);
      texture_list_br.total_items = bg_count;
    } else {
      memcpy(m3d_fpgcodesbr, texture_names, max_texturas * w_textura);
      texture_list_br.total_items = tex_count;
    }
  }

  num = 0;

  texture_list_br.created = 0;
  texture_list_br.columns = 4;
  texture_list_br.w = 32;
  texture_list_br.h = 32;
  texture_list_br.lines = 4;

  v_finished = 0;
  t_pulsada = 1;
}

void mapper_browse_fpg1(void) {
  _show_items();

  M3D_create_listboxbr(&texture_list_br);
}

void mapper_browse_fpg2(void) {
  int old_pincel;

  if (!key(_T) && !key(_U))
    t_pulsada = 0;

  _process_items();

  M3D_update_listboxbr(&texture_list_br);

  if ((mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT)) {
    if (texture_list_br.zone >= 10) {
      // Paint mode only (draw_mode<100) - 3D map mode removed
      if (browser_type == BRUSH) {
        old_pincel = brush_index;
        brush_index = texture_list_br.first_visible + texture_list_br.zone - 10;
        M3D_show_thumb(&texture_list_br, old_pincel);
        M3D_show_thumb(&texture_list_br, brush_index);
        brush_index = old_pincel;
      }
      fpg_thumb_pos = 0;
      v_finished = 1;
      end_dialog = 1;
    }
  }

  if (!t_pulsada) {
    if (browser_type == BRUSH) {
      if (key(_T))
        end_dialog = 1;
    } else if (browser_type == MAPBR) {
      if (key(_U))
        end_dialog = 1;
    }
  }
}

void mapper_browse_fpg3(void) {}
