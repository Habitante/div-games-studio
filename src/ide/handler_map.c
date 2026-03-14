//-----------------------------------------------------------------------------
//      Map viewer window, open/save/new map, resize, and search operations
//-----------------------------------------------------------------------------

#include "handler_internal.h"

// Forward declarations for functions defined in other modules
void merge_palettes(void);
int fmt_load_dac_map(char *name);
int fmt_load_dac_pcx(char *name);
int fmt_load_dac_bmp(char *name);
int fmt_load_dac_jpg(char *name);
void create_palette(void);
void palette_action0(void);
void apply_resize(struct tmapa *map_data, int map_w, int map_h);

extern byte apply_palette[768];
extern byte *sample;
extern int num_taggeds;
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[];

//-----------------------------------------------------------------------------
//      Map window type
//-----------------------------------------------------------------------------

// A "map" type window requires the following information (whether
// loading a map or creating a new one in the program)

// v_map->filename   - These two fields will be "Map x",NULL if
// v_map->path       - the map has no corresponding file on disk yet
// v_map->map
// v_map->selection_mask
// v_map->map_width/al

void blit_mouse(void);

void map_view0(void) {
  int x, y;
  int max;

  v.type = WIN_MAP; // Map (windows of which only one can be active)
  v.mapa = v_map;

  if ((v.w = v.mapa->map_width) < 48 * big2)
    v.w = (24 + 4) * big2;
  else {
    max = vga_width / 2 - big2;
    v.w = v.w / 2 + 4 * big2;
    if (v.w > max)
      v.w = max;
  }


  if ((v.h = v.mapa->map_height) < 16 * big2)
    v.h = (8 + 12) * big2;
  else {
    max = vga_height / 2 - 9 * big2;
    v.h = v.h / 2 + 12 * big2;
    if (v.h > max)
      v.h = max;
  }

  if (big) {
    if (v.w & 1)
      v.w++;
    if (v.h & 1)
      v.h++;
    v.w = -v.w; // Negative to indicate the window should not be doubled
  }

  v.title = (byte *)v.mapa->filename;
  v.name = (byte *)v.mapa->filename;

  v.mapa->code = next_map_code++;
  v.mapa->saved = 1;

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

  v.paint_handler = map_view1;
  v.click_handler = map_view2;
  v.close_handler = map_view3;
}

void map_view1(void) {
  byte *si, *di;
  int w, h;
  int x, y, x2, y2, src_stride, dst_stride;
  int max_w, max_h;

  dst_stride = v.w;
  di = v.ptr + dst_stride * 10 * big2 + 2 * big2;

  if (v.mapa->map_width < 48 * big2)
    di += (24 * big2 - v.mapa->map_width / 2) / 2;
  if (v.mapa->map_height < 16 * big2)
    di += ((8 * big2 - v.mapa->map_height / 2) / 2) * dst_stride;

  src_stride = v.mapa->map_width;
  si = v.mapa->map;

  max_w = v.w - 4 * big2;
  max_h = v.h - 12 * big2;

  if ((w = v.mapa->map_width / 2) > max_w) {
    w = max_w;
    x = (v.mapa->zoom_cx - max_w);
    if (x < 0)
      x = 0;
    else if (x + max_w * 2 > v.mapa->map_width)
      x = v.mapa->map_width - max_w * 2;
    si += x;
  }

  if ((h = v.mapa->map_height / 2) > max_h) {
    h = max_h;
    y = (v.mapa->zoom_cy - max_h);
    if (y < 0)
      y = 0;
    else if (y + max_h * 2 > v.mapa->map_height)
      y = v.mapa->map_height - max_h * 2;
    si += y * src_stride;
  }

  if (w && h)
    for (y = 0, y2 = 0; y < h; y++, y2 += 2)
      for (x = 0, x2 = 0; x < w; x++, x2 += 2)
        *(di + dst_stride * y + x) =
            *(ghost + *(ghost + *(word *)(si + src_stride * y2 + x2)) * 256 +
              *(ghost + *(word *)(si + src_stride * (y2 + 1) + x2)));
  else {
    if (!w) {
      w++;
    }
    if (!h) {
      h++;
    }
    for (y = 0, y2 = 0; y < h; y++, y2 += 2)
      for (x = 0, x2 = 0; x < w; x++, x2 += 2)
        *(di + dst_stride * y + x) = *(si + src_stride * y2 + x2);
  }
}

int windows_collide(int, int);

void calculate_foreground(void) {
  int n, m;

  for (n = 0; n < MAX_WINDOWS; n++)
    if (window[n].type && window[n].foreground != WF_MINIMIZED) {
      window[n].foreground = WF_FOREGROUND;
      for (m = 0; m < n; m++)
        if (window[m].type) {
          if (windows_collide(n, m))
            window[n].foreground = WF_BACKGROUND;
        }
    }
}

// READ MOUSE BUTTONS

void read_mouse3(void) {}

extern int back;

#define max_texturas 1000
#define w_textura    (3 + 1) // width for 000 - 999

#define BRUSH 4
#define MAPBR 8

extern int texture_type;
extern int browser_type;
extern byte brush_fpg_path[256];
extern struct t_listboxbr texture_list_br;
extern struct t_listboxbr thumbmap_list_br;

extern struct _thumb_tex {
  int w, h;                    // Width and height of the thumbnail
  int real_width, real_height; // Width and height of the texture
  char *ptr;                   // ==NULL if the thumbnail has not started loading
  int status;                  // 0-Not a valid texture, 1-Loaded
  int FilePos;
  int Code;
  int is_square;
} thumb_tex[max_texturas];

extern struct _thumb_map {
  int w, h;                    // Width and height of the thumbnail
  int real_width, real_height; // Width and height of the texture
  char *ptr;                   // ==NULL if the thumbnail has not started loading
  int status;                  // 0-Not a valid texture, 1-Loaded
  int FilePos;
  int Code;
  int is_square;
} thumb_map[MAX_WINDOWS];

void M3D_create_thumbs(struct t_listboxbr *l, int prog);
int create_mapbr_thumbs(struct t_listboxbr *l);
void free_paint_thumbs(void);

extern byte *texture_color;

FILE *file_paint_fpg;

void map_view2(void) {
  int old_draw_mode = 0, n, _undo_index;
  int w = v.w, h = v.h;
  int sp_w, sp_h, ew;
  byte *sp;
  if (big) {
    w /= 2;
    h /= 2;
  }

  if ((mouse_b & MB_LEFT)) {
    read_mouse3();

    num_windows_to_create = 0;
    _undo_index = undo_index;

    map = v.mapa->map;

    map_width = v.mapa->map_width;
    map_height = v.mapa->map_height;

    selection_mask = (int *)malloc(((map_width * map_height + 31) / 32) * 4);

    if (selection_mask == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    for (n = 1; n < MAX_WINDOWS; n++)
      if (window[n].type && window[n].foreground == WF_FOREGROUND) {
        hidden[n] = 1;
        window[n].foreground = WF_BACKGROUND;
      } else
        hidden[n] = 0;

    zoom = v.mapa->zoom;
    zoom_x = v.mapa->zoom_x;
    zoom_y = v.mapa->zoom_y;
    zoom_cx = v.mapa->zoom_cx;
    zoom_cy = v.mapa->zoom_cy;

    // The following four 'if' are to avoid problems when you change (video mode?)

    if ((map_width << zoom) < vga_width) {
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

    current_map_code = v.mapa->code;
    zoom_background = 0;
    v.type = WIN_EMPTY;
    v.foreground = WF_BACKGROUND;
    v.state = 0;

    wgra(v.ptr, w, h, c1, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c0);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c2);
    } else {
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 3, 1, v.title, c0);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c2);
    }

    if (dragging == 4) {
      mouse_b = 0;
      old_draw_mode = draw_mode;
      draw_mode = 190;
    } else
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);

    highlight_background = 0;
    current_mouse = 21 + paint_cursor * 4;

    // Special initialization for the drawing program
    ew = exploding_windows;
    exploding_windows = 0;
    for (n = 0; n < max_texturas; n++)
      thumb_tex[n].ptr = NULL;
    for (n = 0; n < MAX_WINDOWS; n++)
      thumb_map[n].ptr = NULL;
    div_strcpy(full, sizeof(full), file_types[1].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), "system/brush.fpg");

    texture_color = NULL;

    texture_type = 0;
    if ((file_paint_fpg = fopen(full, "rb")) != NULL) // NOTE !!! Could provide message here
    {
      div_strcpy((char *)brush_fpg_path, sizeof(brush_fpg_path), full);
      draw_mode -= 100;
      M3D_create_thumbs(&texture_list_br, 0);
      draw_mode += 100;
      texture_type |= BRUSH; // Thumbnail type BRUSH
    }

    v.type = WIN_MAP;
    if (create_mapbr_thumbs(&thumbmap_list_br)) {
      texture_type |= MAPBR; // Thumbnail type MAPBR
    }
    v.type = WIN_EMPTY;

    do {
      zoom_map();
      need_zoom = 0;
      back = 0;
      draw_mode -= 100;
      switch (draw_mode) {
      case 0:
        edit_mode_0();
        break; // Pixels
      case 1:
        edit_mode_1();
        break; // Strokes
      case 2:
        edit_mode_2();
        break; // Lines
      case 3:
        edit_mode_3();
        break; // Lines cont.
      case 4:
        edit_mode_4();
        break; // Bezier
      case 5:
        edit_mode_5();
        break; // Bezier cont.
      case 6:
        edit_mode_6();
        break; // Boxes
      case 7:
        edit_mode_7();
        break; // Circles
      case 8:
        edit_mode_8();
        break; // Spray
      case 9:
        edit_mode_9();
        break; // Fill
      case 10:
        edit_mode_10();
        break; // Cut
      case 11:
        edit_mode_11();
        break; // Undo
      case 12:
        edit_mode_12();
        break; // Points
      case 13:
        edit_mode_13();
        break; // Text
      case 90: // Paste graphic
        sp_w = window[1].mapa->map_width;
        sp_h = window[1].mapa->map_height;
        if ((sp = (byte *)malloc(sp_w * sp_h)) != NULL) {
          memcpy(sp, window[1].mapa->map, sp_w * sp_h);
          move_selection(sp, sp_w, sp_h);
        };
        dragging = 5;
        break;
      default:
        edit_scr();
        break;
      }
      if (draw_mode < 100) {
        put_bar(2, 10, 45);
      }
      blit_mouse();
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    } while (draw_mode >= 100);
    mouse_graf = CURSOR_ARROW;

    free(selection_mask);
    selection_mask = NULL;

    exploding_windows = ew;
    if (texture_type & BRUSH)
      fclose(file_paint_fpg);
    free_paint_thumbs();
    texture_type = 0;
    browser_type = 0;

    if (draw_mode < 90)
      draw_mode += 100;
    else
      draw_mode = old_draw_mode;

    if (highlight_background) {
      highlight_background = 0;
      dac[0] = (dac[0] + 32) % 64;
      dac[1] = (dac[1] + 32) % 64;
      dac[2] = (dac[2] + 32) % 64;
      set_dac(dac);
    }

    v.mapa->zoom = zoom;
    v.mapa->zoom_x = zoom_x;
    v.mapa->zoom_y = zoom_y;
    v.mapa->zoom_cx = zoom_cx;
    v.mapa->zoom_cy = zoom_cy;
    for (n = 1; n < MAX_WINDOWS; n++)
      if (hidden[n])
        window[n].foreground = WF_FOREGROUND;
    v.foreground = WF_FOREGROUND;
    v.state = 1;
    v.type = WIN_MAP;

    calculate_foreground();

    wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
    } else {
      wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
    }

    map_view1();
    update_box(0, 0, vga_width, vga_height);
    if (undo_index != _undo_index)
      v.mapa->saved = 0;

    n = 0;
    while (n < num_windows_to_create) {
      v_map = windows_to_create[n++];
      new_window(map_view0);
    }
  }
}

void map_view3(void) {
  free(v.mapa->map);
  free(v.mapa);
}

//-----------------------------------------------------------------------------
//      New map dialog
//-----------------------------------------------------------------------------

#define DIALOG_Y_OFFSET 67
char map_sizes[9 * 9] =
    "320x200\0 320x240\0 360x240\0 360x360\0 376x282\0 640x400\0 640x480\0 800x600\0 1024x768";
struct t_listbox map_sizes_list = {64, 19, map_sizes, 9, 5, 50};
char width_str[8], height_str[8];

void new_map0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[132];
  v.w = 126;
  v.h = 14 + DIALOG_Y_OFFSET;
  v.paint_handler = new_map1;
  v.click_handler = new_map2;
  map_sizes_list.created = 0;
  map_sizes_list.total_items = 9;
  map_width = vga_width;
  map_height = vga_height;
  itoa(map_width, width_str, 10);
  itoa(map_height, height_str, 10);
  _get(133, 4, 12, v.w - 72, (byte *)width_str, 8, 1, 32767);
  _get(134, 4, 34, v.w - 72, (byte *)height_str, 8, 1, 32767);
  _button(100, 7, DIALOG_Y_OFFSET, ALIGN_TL);
  _button(101, v.w - 8, DIALOG_Y_OFFSET, ALIGN_TR);
  v_finished = 0;
}

void new_map1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, 64, 12, 0, texts[135], c3);
  create_listbox(&map_sizes_list);
}

void new_map2(void) {
  int n;

  _process_items();
  update_listbox(&map_sizes_list);

  if ((mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT)) {
    if (map_sizes_list.zone >= 10) {
      n = map_sizes_list.zone - 10 + map_sizes_list.first_visible;
      *strchr(&map_sizes[n * 9], 'x') = 0;
      map_width = atoi(&map_sizes[n * 9]);
      map_height = atoi(&map_sizes[n * 9 + strlen(&map_sizes[n * 9]) + 1]);
      map_sizes[n * 9 + strlen(&map_sizes[n * 9])] = 'x';
      itoa(map_width, width_str, 10);
      itoa(map_height, height_str, 10);
      call((void_return_type_t)v.paint_handler);
      update_listbox(&map_sizes_list);
      _select_new_item(2);
      v.redraw = 1;
    }
  }

  switch (v.active_item) {
  case 2:
    end_dialog = 1;
    if (atoi(width_str) > 0 && atoi(width_str) < 32768)
      map_width = atol(width_str);
    if (atoi(height_str) > 0 && atoi(height_str) < 32768)
      map_height = atol(height_str);
    v_finished = 1;
    break;
  case 3:
    end_dialog = 1;
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//      Create a new map (return 1 on error)
///////////////////////////////////////////////////////////////////////////////

int new_map(byte *pre_buffer) {
  int n;

  // 1. Allocate memory for the tmapa struct
  if ((v_map = (struct tmapa *)malloc(sizeof(struct tmapa))) != NULL) {
    // 2. Allocate memory for the map

    if (pre_buffer != NULL)
      v_map->map = pre_buffer;
    else
      v_map->map = (byte *)malloc(map_width * map_height);

    if (v_map->map != NULL) {
      if (pre_buffer == NULL) {
        // 2b. Clear the map to zero
        memset(v_map->map, 0, map_width * map_height);
      }

      // 4. Set the remaining variables
      div_strcpy(v_map->filename, sizeof(v_map->filename), (char *)texts[136]);
      ltoa(next_code++, v_map->filename + strlen(v_map->filename), 10);
      *v_map->path = '\0';
      v_map->map_width = map_width;
      v_map->map_height = map_height;
      v_map->has_name = 0;
      v_map->fpg_code = 0;
      v_map->description[0] = 0;
      for (n = 0; n < 512; n++)
        v_map->points[n] = -1;
      if (must_create)
        new_window(map_view0);

      return (0);

    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
      free(v_map);
    }

  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }

  return (1);
}

//-----------------------------------------------------------------------------
//     Open a map disc content
//-----------------------------------------------------------------------------

int has_maps(void);
extern int load_palette;

extern byte apply_palette[768];
extern byte *sample;

void open_map(void) {
  FILE *f;
  byte *ptr;
  int x, sum;
  int n, map_type;
  byte *buffer;
  byte pal[768];
  byte palorg[768];
  byte xlat[256];
  int num, div_try;

  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), file_types[v_type].path);
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

  n = 0; // Number of distinct palettes
  sample = NULL;
  memcpy(pal, dac, 768);
  memcpy(palorg, original_palette, 768);
  memset(original_palette, 0, 768);

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_try = 0;
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);

      load_palette = 1;
      div_try |= fmt_load_dac_pcx(full);
      if (!div_try)
        div_try |= fmt_load_dac_bmp(full);
      if (!div_try)
        div_try |= fmt_load_dac_map(full);
      if (!div_try)
        div_try |= fmt_load_dac_jpg(full);
      load_palette = 0;

      if (div_try) {
        if (n++ == 0) {
          memcpy(pal, dac4, 768);
        } else {
          x = 0;
          sum = 0;
          do {
            sum += abs((int)pal[x] - (int)dac4[x]);
          } while (++x < 768);
          if (sum) {
            if (sample == NULL) {
              sample = (byte *)malloc(32768);
              if (sample == NULL)
                continue;
              memset(sample, 0, 32768);
              for (x = 0; x < 256; x++) {
                sample[((pal[x * 3 + 0] & 0xFE) << 9) | ((pal[x * 3 + 1] & 0xFE) << 4) |
                       (pal[x * 3 + 2] >> 1)] = 1;
              }
            }
            for (x = 0; x < 256; x++) {
              sample[((dac4[x * 3 + 0] & 0xFE) << 9) | ((dac4[x * 3 + 1] & 0xFE) << 4) |
                     (dac4[x * 3 + 2] >> 1)] = 1;
            }
          }
        }
      }
    }
  }

  if (sample != NULL) {
    create_palette();
    free(sample);
    memcpy(pal, apply_palette, 768);
  }

  // pal[] now contains the palette of the maps to load

  x = 0;
  sum = 0;
  do {
    sum += abs((int)pal[x] - (int)dac[x]);
  } while (++x < 768);
  memcpy(work_palette, pal, 768);

  if (sum && n == 1) {
    x = 0;
    sum = 0;
    do {
      sum += abs((int)original_palette[x] - (int)dac[x]);
    } while (++x < 768);
    memcpy(work_palette, original_palette, 768);
  }

  if (sum) {
    show_dialog(palette_action0); // Load palette?

    switch (v_accept) {
    case 0: // Cancel (don't load)
      memcpy(original_palette, palorg, 768);
      return;
    case 1: // Adapt maps to the system palette
      break;
    case 2: // Merge palettes
      memcpy(dac4, pal, 768);
      merge_palettes();
      pal_refresh(0, 1);
      break;
    case 3: // Activate the new palette
      if (sample == NULL)
        memcpy(pal, original_palette, 768);
      memcpy(dac4, pal, 768);
      pal_refresh(0, 1);
      break;
    }
  }
  memcpy(original_palette, palorg, 768);


  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);

      if ((f = fopen(full, "rb")) != NULL) { // A file was selected
        fseek(f, 0, SEEK_END);
        n = ftell(f);
        if ((buffer = (byte *)malloc(n)) != NULL) {
          fseek(f, 0, SEEK_SET);
          if (fread(buffer, 1, n, f) == n) {
            if (fmt_is_map(buffer))
              map_type = 1;
            else if (fmt_is_pcx(buffer))
              map_type = 2;
            else if (fmt_is_bmp(buffer))
              map_type = 3;
            else if (fmt_is_jpg(buffer, n))
              map_type = 4;
            else
              map_type = 0;

            if (map_type) {
              if ((v_map = (struct tmapa *)malloc(sizeof(struct tmapa))) != NULL) {
                memset(v_map, 0, sizeof(struct tmapa));

                if ((v_map->map = (byte *)malloc(map_width * map_height + map_width)) != NULL) {
                  v_map->has_name = 0; // No description by default

                  for (x = 0; x < 512; x++)
                    v_map->points[x] = -1;

                  x = 1;
                  switch (map_type) {
                  case 1:
                    fmt_load_map(buffer, v_map->map, 1);
                    break;
                  case 2:
                    fmt_load_pcx(buffer, v_map->map, 1);
                    break;
                  case 3:
                    fmt_load_bmp(buffer, v_map->map, 1);
                    break;
                  case 4:
                    x = fmt_load_jpg(buffer, v_map->map, 1, n);
                    break;
                  }

                  if (!x) {
                    free(v_map->map);
                    free(v_map);
                    free(buffer);
                    fclose(f);
                    v_text = (char *)texts[44];
                    show_dialog(err0);
                    continue;
                  }

                  // NOTE: fmt_load_map/PCX/BMP return void, so errors cannot be checked here.
                  // Only fmt_load_jpg returns a status (already handled above).

                  extern_use_buffer_map = (char *)v_map->map;

                  x = 0;
                  sum = 0;
                  do {
                    sum += abs((int)dac4[x] - (int)dac[x]);
                  } while (++x < 768);

                  if (sum) {
                    memcpy(pal, dac4, 768);
                    create_dac4();
                    for (x = 0; x < 256; x++)
                      xlat[x] = find_color(pal[x * 3], pal[x * 3 + 1], pal[x * 3 + 2]);
                    ptr = v_map->map;
                    sum = map_width * map_height;
                    x = 0;
                    do {
                      *ptr = xlat[*ptr];
                      ptr++;
                    } while (++x < sum);
                  }

                  // Set path\filename, map_width/al and create the new window

                  div_strcpy(v_map->filename, sizeof(v_map->filename), input);
                  div_strcpy(v_map->path, sizeof(v_map->path), file_types[v_type].path);
                  memcpy(v_map->description, map_description, 32);
                  v_map->has_name = 0;
                  v_map->fpg_code = fpg_code;
                  if (fpg_code)
                    v_map->has_name = 1;
                  v_map->map_width = map_width;
                  v_map->map_height = map_height;

                  new_window(map_view0);

                } else {
                  v_text = (char *)texts[45];
                  show_dialog(err0);
                  free(v_map);
                }
              } else {
                v_text = (char *)texts[45];
                show_dialog(err0);
              }
            } else {
              v_text = (char *)texts[46];
              show_dialog(err0);
            }
          } else {
            v_text = (char *)texts[44];
            show_dialog(err0);
          }
          free(buffer);
        } else {
          v_text = (char *)texts[45];
          show_dialog(err0);
        }
        fclose(f);
      } else {
        v_text = (char *)texts[44];
        show_dialog(err0);
      }
    }
  }
  create_dac4();
}

//-----------------------------------------------------------------------------
//      Save a map
//
// Input: file_types[v_type].path, input
//        map, dac, map_width, map_height
//        v_window
//-----------------------------------------------------------------------------

void save_map(void) {
  int e, map_type;
  int w = window[v_window].w, h = window[v_window].h;
  char filename[255];
  FILE *f;

  if (big) {
    w /= 2;
    h /= 2;
  }


  div_strcpy(full, sizeof(full), file_types[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  if (strchr(input, ' ') == NULL) {
    if ((f = fopen(full, "wb")) != NULL) { // A file was selected
      div_strcpy(filename, sizeof(filename), input);
      strupr(filename);

      if (!strcmp(strchr(filename, '.'), ".PCX"))
        map_type = 1;
      else if (!strcmp(strchr(filename, '.'), ".BMP"))
        map_type = 2;
      else

        map_type = 0;

      switch (map_type) {
      case 0:
        e = fmt_save_map(map, f);
        break;
      case 1:
        e = fmt_save_pcx(map, f);
        break;
      case 2:
        e = fmt_save_bmp(map, f);
        break;
      }
      if (e == 2) {
        v_text = (char *)texts[48];
        show_dialog(err0);
      }

      if (e || ferror(f)) {
        fclose(f);
        remove(full); // Delete partial file on write failure
      } else {
        fclose(f);
      }

    } else {
      v_text = (char *)texts[47];
      show_dialog(err0);
      e = 1;
    }
  } else {
    v_text = (char *)texts[47];
    show_dialog(err0);
    e = 1;
  }

  if (!e) { // Update the path and name of the saved map/window
    div_strcpy(window[v_window].mapa->path, sizeof(window[v_window].mapa->path),
               file_types[v_type].path);
    div_strcpy(window[v_window].mapa->filename, sizeof(window[v_window].mapa->filename), input);
    wgra(window[v_window].ptr, w, h, c_b_low, 2, 2, w - 20, 7);
    if (text_len(window[v_window].title) + 3 > w - 20) {
      wwrite_in_box(window[v_window].ptr, w, w - 19, h, 4, 2, 0, window[v_window].title, c1);
      wwrite_in_box(window[v_window].ptr, w, w - 19, h, 3, 2, 0, window[v_window].title, c4);
    } else {
      wwrite(window[v_window].ptr, w, h, 3 + (w - 20) / 2, 2, 1, window[v_window].title, c1);
      wwrite(window[v_window].ptr, w, h, 2 + (w - 20) / 2, 2, 1, window[v_window].title, c4);
    }
    flush_window(v_window);
  }
}

//-----------------------------------------------------------------------------
//      Create a new downscaled map by rendering and optionally convert to grayscale
//      Used to create a hardness map
//-----------------------------------------------------------------------------
char width_input[5];
char height_input[5];
int grayscale = 0;
int percentages = 0;

void resize1() {
  int w = v.w / big2, h = v.h / big2, n;
  char cWork[5];
  n = find_and_load_map();
  if (!n)
    return;
  _show_items();
  wwrite(v.ptr, w, h, 4, 11, 0, texts[133], c3);
  div_snprintf(cWork, sizeof(cWork), "%d", window[n].mapa->map_width);
  wwrite(v.ptr, w, h, 4, 20, 0, (byte *)cWork, c3);

  wwrite(v.ptr, w, h, 4, 30, 0, texts[134], c3);
  div_snprintf(cWork, sizeof(cWork), "%d", window[n].mapa->map_height);
  wwrite(v.ptr, w, h, 4, 39, 0, (byte *)cWork, c3);
  v_accept = 0;
}
void resize2() {
  _process_items();
  switch (v.active_item) {
  case 4:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 5:
    v_accept = 0;
    end_dialog = 1;
    break;
  }
}
void resize3() {
  map_width = atoi(width_input);
  map_height = atoi(height_input);
}

void resize0() {
  v.type = WIN_DIALOG; // Window type 1 = dialog

  v.title = texts[63];
  v.w = 126 + 50;
  v.h = 49 + 18;
  v.paint_handler = resize1;
  v.click_handler = resize2;
  v.close_handler = resize3;

  div_snprintf(width_input, sizeof(width_input), "%d", map_width);
  div_snprintf(height_input, sizeof(height_input), "%d", map_height);

  _get(133, 42, 11, 30, (byte *)width_input, 5, 1, 9999);
  _get(134, 42, 30, 30, (byte *)height_input, 5, 1, 9999);
  percentages = 0;
  grayscale = 0;
  _flag(95, 90, 19, &percentages);
  _flag(96, 90, 38, &grayscale);
  _button(100, 7, v.h - 14, ALIGN_TL);
  _button(101, v.w - 8, v.h - 14, ALIGN_TR);
}

void reduce_half() {
  char *temp_buffer;
  int n, p1, p2, p3, p4, map_w, map_h, c;
  float x, y;
  struct tmapa *map_data;
  float incx, incy;
  int fx, fy;
  char lut[190];
  int Colors[9], min_dist, i, dist;
  n = find_and_load_map();
  if (!n)
    return;
  map_data = window[n].mapa;

  if ((map_data->map_width < 2) || (map_data->map_height < 2))
    return;

  map_w = map_data->map_width;
  map_h = map_data->map_height;

  show_dialog(resize0);
  if (!v_accept)
    return;

  if (percentages) {
    map_width = (map_width * map_w) / 100;
    map_height = (map_height * map_h) / 100;
  }

  if (map_width > map_w || map_height > map_h) {
    apply_resize(map_data, map_w, map_h);
    return;
  }

  // Generate grayscale table
  if (grayscale) {
    Colors[0] = c0;
    Colors[2] = c1;
    Colors[4] = c2;
    Colors[6] = c3;
    Colors[8] = c4;
    Colors[1] = average_color(c0, c1);
    Colors[3] = average_color(c1, c2);
    Colors[5] = average_color(c2, c3);
    Colors[7] = average_color(c3, c4);

    for (n = 0; n < 190; n++) {
      min_dist = 192;
      for (i = 0; i < 9; i++) {
        c = (memptrsize)Colors[i] * 3;
        dist = dac[c] + dac[c + 1] + dac[c + 2];
        if (dist > n)
          dist = dist - n;
        else
          dist = n - dist;
        if (dist < min_dist) {
          lut[n] = c / 3;
          min_dist = dist;
        }
      }
    }
  }
  // End grayscale table
  incy = (float)map_h / (float)map_height;
  incx = (float)map_w / (float)map_width;
  temp_buffer = (char *)malloc(map_width * map_height);

  if (temp_buffer == NULL) { // Added by Dani ...
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  y = (float)0;
  for (fy = 0; fy < map_height; fy++) {
    show_progress((char *)texts[92], fy, map_height);
    x = (float)0;
    for (fx = 0; fx < map_width; fx++) {
      if (fx == (map_width - 1)) {
        p1 = map_data->map[(memptrsize)y * map_w + (memptrsize)x - 1];
        p2 = map_data->map[(memptrsize)y * map_w + (memptrsize)x];
      } else {
        p1 = map_data->map[(memptrsize)y * map_w + (memptrsize)x];
        p2 = map_data->map[(memptrsize)y * map_w + (memptrsize)x + 1];
      }

      p1 = ghost[p1 + p2 * 256];

      if (fy == (map_height - 1)) {
        p3 = map_data->map[(memptrsize)y * map_w + (memptrsize)x - map_w - 1];
        p4 = map_data->map[(memptrsize)y * map_w + (memptrsize)x - map_w];
      } else {
        p3 = map_data->map[(memptrsize)y * map_w + (memptrsize)x + map_w];
        p4 = map_data->map[(memptrsize)y * map_w + (memptrsize)x + map_w + 1];
      }

      p3 = ghost[p3 + p4 * 256];
      p1 = ghost[p1 + p3 * 256];

      // grayscale=1: Convert the image to grayscale

      if (grayscale) {
        p1 = dac[p1 * 3] + dac[p1 * 3 + 1] + dac[p1 * 3 + 1];
        temp_buffer[fy * map_width + fx] = lut[p1];
      } else
        temp_buffer[fy * map_width + fx] = p1;
      x += incx;
    }
    y += incy;
  }
  show_progress((char *)texts[92], map_height, map_height);

  if (new_map((byte *)temp_buffer)) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(temp_buffer);
    return;
  }

  v.mapa->zoom_cx = v.mapa->map_width / 2;
  v.mapa->zoom_cy = v.mapa->map_height / 2;

  fx = (v.mapa->zoom_cx - vga_width / 2);
  if (fx < 0)
    fx = 0;
  else if (fx + vga_width > v.mapa->map_width)
    fx = v.mapa->map_width - vga_width;
  fy = (v.mapa->zoom_cy - vga_height / 2);
  if (fy < 0)
    fy = 0;
  else if (fy + vga_height > v.mapa->map_height)
    fy = v.mapa->map_height - vga_height;
  v.mapa->zoom = 0;
  v.mapa->zoom_x = fx;
  v.mapa->zoom_y = fy;

  for (n = 0; n < 512; n++)
    v.mapa->points[n] = -1;
  v.mapa->fpg_code = 0;
  call((void_return_type_t)v.paint_handler);
  blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
}

//-----------------------------------------------------------------------------
//      Create a search map of up to 128x128 tiles
//      Used to create a hardness map
//-----------------------------------------------------------------------------

char tile_label[32];
char tile_str[5];

char color_str[5];
int minimums = 0;
int mintile, maxtile;
int itile, icolor;

void map_search1() {
  _show_items();
}

void map_search2() {
  _process_items();
  switch (v.active_item) {
  case 3:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 4:
    v_accept = 0;
    end_dialog = 1;
    break;
  }
}

void map_search3() {
  itile = atoi(tile_str);
  icolor = atoi(color_str);
}

void map_search0() {
  v.type = WIN_DIALOG;
  v.title = texts[400];
  v.w = 72 + 40;
  v.h = 49 + 18 + 9;
  v.paint_handler = map_search1;
  v.click_handler = map_search2;
  v.close_handler = map_search3;

  div_snprintf(tile_str, sizeof(tile_str), "%d", mintile);
  div_snprintf(color_str, sizeof(color_str), "%d", color);

  DIV_SPRINTF(tile_label, "%s [%d..%d]", texts[401], mintile, maxtile);
  texts[546] = (byte *)&tile_label[0];

  _get(546, 4, 11, 64 + 40, (byte *)tile_str, 5, mintile, maxtile);
  _get(402, 4, 30, 64 + 40, (byte *)color_str, 5, 0, 255);

  _flag(403, 4, 49, &minimums);

  _button(100, 7, v.h - 14, ALIGN_TL);
  _button(101, v.w - 8, v.h - 14, ALIGN_TR);
}

#define max_map_size 254 // 128

void map_search() {
  int map_w, map_h;
  int c, n, m, x, y;
  byte *ma;

  if (!find_and_load_map())
    return;

  map_w = map_width;
  map_h = map_height;
  ma = map; // Original map ma(map_w,map_h)

  if (map_w > map_h) { // Get the tile size limits
    mintile = (map_w + max_map_size - 1) / max_map_size;
    if (map_h < 256)
      maxtile = map_h;
    else
      maxtile = 256;
  } else {
    mintile = (map_h + max_map_size - 1) / max_map_size;
    if (map_w < 256)
      maxtile = map_w;
    else
      maxtile = 256;
  }
  if (mintile > 256)
    mintile = 256;
  if (maxtile < mintile)
    maxtile = 256;

  v_accept = 0;
  show_dialog(map_search0);

  if (!v_accept)
    return;

  map_width = (map_width + itile - 1) / itile; // Size of the new map
  map_height = (map_height + itile - 1) / itile;
  if (map_width > max_map_size)
    map_width = max_map_size;
  if (map_height > max_map_size)
    map_height = max_map_size;

  if ((map = (byte *)malloc(map_width * map_height)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

  for (y = 0; y < map_height * itile; y += itile) {
    show_progress((char *)texts[404], y, map_height * itile);
    for (x = 0; x < map_width * itile; x += itile) {
      c = 0; // 0/c4
      for (n = 0; n < itile; n++) {
        for (m = 0; m < itile; m++) {
          if (x + m < map_w && y + n < map_h) {
            if (*(ma + x + m + (y + n) * map_w) == icolor)
              c++;
          }
        }
      }
      if (minimums) {
        if (c < (itile * itile) / 5)
          c = 0;
        else
          c = c4;
      } else if (c)
        c = c4;
      *(map + (x / itile) + (y / itile) * map_width) = c;
    }
  }
  show_progress((char *)texts[404], map_height * itile, map_height * itile);

  if (new_map(map)) {
    free(map);
    return;
  }

  call((void_return_type_t)v.paint_handler);
  blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
}

//-----------------------------------------------------------------------------

void apply_resize(struct tmapa *map_data, int map_w, int map_h) {
  char *temp_buffer;
  char saved_dac[768];
  int n, p1, p2, p3, p4;
  int x, y, z, t, x0, y0, ant, min, max;
  float incx, incy;
  float fx, fy, cx, cy, rx, ry;
  float pp1, pp2, pp3, pp4;
  int r, b, g, c, regen = 0;
  FILE *rgb_tab = NULL;
  char *rgb_table = NULL;
  char lut[190];
  int Colors[9], min_dist, i, dist;

  // Generate grayscale table
  if (grayscale) {
    Colors[0] = c0;
    Colors[2] = c1;
    Colors[4] = c2;
    Colors[6] = c3;
    Colors[8] = c4;
    Colors[1] = average_color(c0, c1);
    Colors[3] = average_color(c1, c2);
    Colors[5] = average_color(c2, c3);
    Colors[7] = average_color(c3, c4);

    for (n = 0; n < 190; n++) {
      min_dist = 192;
      for (i = 0; i < 9; i++) {
        c = (memptrsize)Colors[i] * 3;
        dist = dac[c] + dac[c + 1] + dac[c + 2];
        if (dist > n)
          dist = dist - n;
        else
          dist = n - dist;
        if (dist < min_dist) {
          lut[n] = c / 3;
          min_dist = dist;
        }
      }
    }
  }
  // End grayscale table
  else
  // Generate RGB lookup table with error margin of 1
  {
    rgb_table = (char *)malloc(32768);
    if (rgb_table == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    if ((rgb_tab = fopen("RGB_TAB.TMP", "rb")) == NULL)
      regen = 1;
    else {
      fread(saved_dac, 1, 768, rgb_tab);
      for (x = 0; x < 768; x++)
        regen += abs(dac[x] - saved_dac[x]);
    }
    if (regen) {
      for (x = 0; x < 32; x++) {
        show_progress((char *)texts[93], x, 32);
        for (y = 0; y < 32; y++)
          for (z = 0; z < 32; z++) {
            min = 32 * 32 * 32;
            ant = 0;
            for (t = 0; t < 256; t++) {
              max = (x - (dac[t * 3] / 2)) * (x - (dac[t * 3] / 2)) +
                    (y - (dac[t * 3 + 1] / 2)) * (y - (dac[t * 3 + 1] / 2)) +
                    (z - (dac[t * 3 + 2] / 2)) * (z - (dac[t * 3 + 2] / 2));
              if (max < min) {
                min = max;
                ant = t;
              }
            }
            rgb_table[x * 1024 + y * 32 + z] = ant;
          }
      }
      show_progress((char *)texts[93], 32, 32);
      if (rgb_tab)
        fclose(rgb_tab);
      rgb_tab = fopen("RGB_TAB.TMP", "wb");
      fwrite(dac, 1, 768, rgb_tab);
      fwrite(rgb_table, 1, 32768, rgb_tab);
      fclose(rgb_tab);
    } else {
      fread(rgb_table, 1, 32768, rgb_tab);
      fclose(rgb_tab);
    }
  }
  // End RGB lookup table

  incy = (float)(map_h - 1) / (float)map_height;
  incx = (float)(map_w - 1) / (float)map_width;
  temp_buffer = (char *)malloc(map_width * map_height);
  if (temp_buffer == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    if (!grayscale)
      free(rgb_table);
    return;
  }
  fy = (float)0.5;
  for (y = 0; y < map_height; y++) {
    show_progress((char *)texts[92], y, map_height);

    y0 = (memptrsize)fy;
    cy = ((float)y0 + (float)0.5);
    if (((fy - cy) < 0))
      ry = cy - 1.0;
    else
      ry = cy;

    fx = (float)0.5;
    for (x = 0; x < map_width; x++) {
      x0 = (memptrsize)fx;
      cx = ((float)x0 + (float)0.5);
      if (((fx - cx) < 0))
        rx = cx - 1.0;
      else
        rx = cx;

      if (fx > rx)
        pp1 = (fx - rx);
      else
        pp1 = (rx - fx);
      if (fy > ry)
        pp1 += (fy - ry);
      else
        pp1 += (ry - fy);
      pp1 = (2.0 - pp1) / 2.0;
      p1 = map_data->map[(memptrsize)ry * map_w + (memptrsize)rx];
      r = dac[p1 * 3] * pp1;
      g = dac[p1 * 3 + 1] * pp1;
      b = dac[p1 * 3 + 2] * pp1;

      if (fx > (rx + 1.0))
        pp2 = (fx - (rx + 1.0));
      else
        pp2 = ((rx + 1.0) - fx);
      if (fy > ry)
        pp2 += (fy - ry);
      else
        pp2 += (ry - fy);
      pp2 = (2.0 - pp2) / 2.0;
      p2 = map_data->map[(memptrsize)ry * map_w + (memptrsize)(rx + 1.0)];
      r += dac[p2 * 3] * pp2;
      g += dac[p2 * 3 + 1] * pp2;
      b += dac[p2 * 3 + 2] * pp2;

      if (fx > rx)
        pp3 = (fx - rx);
      else
        pp3 = (rx - fx);
      if (fy > ry + 1.0)
        pp3 += (fy - (ry + 1.0));
      else
        pp3 += ((ry + 1.0) - fy);
      pp3 = (2.0 - pp3) / 2.0;
      p3 = map_data->map[(memptrsize)(ry + 1.0) * map_w + (memptrsize)rx];
      r += dac[p3 * 3] * pp3;
      g += dac[p3 * 3 + 1] * pp3;
      b += dac[p3 * 3 + 2] * pp3;

      if (fx > rx + 1.0)
        pp4 = (fx - (rx + 1.0));
      else
        pp4 = ((rx + 1.0) - fx);
      if (fy > ry + 1.0)
        pp4 += (fy - (ry + 1.0));
      else
        pp4 += ((ry + 1.0) - fy);
      pp4 = (2.0 - pp4) / 2.0;
      p4 = map_data->map[(memptrsize)(ry + 1.0) * map_w + (memptrsize)(rx + 1.0)];
      r += dac[p4 * 3] * pp4;
      g += dac[p4 * 3 + 1] * pp4;
      b += dac[p4 * 3 + 2] * pp4;

      r /= (pp1 + pp2 + pp3 + pp4);
      g /= (pp1 + pp2 + pp3 + pp4);
      b /= (pp1 + pp2 + pp3 + pp4);

      if (grayscale) {
        if (r + g + b > 189)
          temp_buffer[y * map_width + x] = lut[189];
        else
          temp_buffer[y * map_width + x] = lut[r + g + b];
      } else
        temp_buffer[y * map_width + x] = rgb_table[(r >> 1) * 1024 + (g >> 1) * 32 + (b >> 1)];

      fx += incx;
    }
    fy += incy;
  }
  show_progress((char *)texts[92], map_height, map_height);

  if (new_map((byte *)temp_buffer)) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(temp_buffer);
    if (!grayscale)
      free(rgb_table);
    return;
  }

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

  if (!grayscale)
    free(rgb_table);
}
