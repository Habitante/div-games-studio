#include "global.h"
#include "fpgfile.hpp"
#include "div_string.h"

int select_file(void);


HeadFPG drag_fpg_header;
int ret_value = 0;
char code_str[5];
char file_str[13];
char description[33];
extern char *fpg_image;
extern short *fpg_points;

extern char newdac[768];
extern int new_dac_loaded;

#define incremento_maximo 65536
extern int old_mouse_x, old_mouse_y, omx, omy, oclock;
extern int incremento;

void map_to_fpg(struct tmapa *mapa);
void get_map_graphic(struct tmapa *mapa, byte *imagen, int x, int y, int width, int height,
                     int cod);
void fpg_to_map(FPG *my_fpg);
void put_map_graphic(byte *imagen, byte *mapa, int num);
void place_map(void);
int collides_with_map(int n, int x, int y, int w, int h);
void close_fpg(char *fpg_path);

void replace_fpg_0(void);
void replace_fpg_1(void);
void replace_fpg_2(void);

void fpg_dialog1(void) {
  FPG *my_fpg = (FPG *)v.aux;
  _show_items();

  FPG_create_listbox_br(&my_fpg->list_info);
}

int has_maps(void);

void load_thumbs(void) {
  FPG *my_fpg = (FPG *)v.aux;

  if (my_fpg->thumb_on)
    FPG_create_thumbs();

  FPG_update_listbox_br(&my_fpg->list_info);
}

void fpg_dialog2(void) {
  int x, y, n;
  int COD, num_points, Elemento;
  FPG *my_fpg = (FPG *)v.aux;
  FILE *fpg;
  char desc[32];

  if (new_dac_loaded) { // The FPG has a new palette
    v.redraw = 0;
    new_dac_loaded = 0;
    remap_all_files(0);
    return;
  }

  if (my_fpg->thumb_on)
    FPG_create_thumbs();

  _process_items();

  switch (v.active_item) {
  case 0:
    my_fpg->list_info.x = 3;
    my_fpg->list_info.y = 11;
    my_fpg->list_info.list = (char *)my_fpg->code_desc;
    my_fpg->list_info.total_items = my_fpg->nIndex;
    my_fpg->list_info.item_width = 38 + 2;
    my_fpg->list_info.first_visible = 0;

    if (my_fpg->thumb_on) {
      my_fpg->list_info.columns = 3;
      my_fpg->list_info.lines = 2;
      my_fpg->list_info.w = 47;
      my_fpg->list_info.h = 26;
    } else {
      my_fpg->list_info.columns = 1;
      my_fpg->list_info.lines = 6;
      my_fpg->list_info.w = 143;
      my_fpg->list_info.h = 8;
    }

    fpg_dialog1();
    v.redraw = 1;

    break;

  case 1:
    fpg_dialog1();
    v.redraw = 1;
    break;
  }

  if ((dragging == 4)) { // Released
    dragging = 5;
    free_drag = 0;

    if (window[1].mapa->fpg_code) {
      COD = window[1].mapa->fpg_code;
      memcpy(desc, window[1].mapa->description, 32);
    } else {
      memcpy(desc, window[1].title, 32);
      COD = my_fpg->last_used;

      while (my_fpg->grf_offsets[COD]) {
        COD++;
      };

      if (COD > 999)
        COD = 1;

      while (my_fpg->grf_offsets[COD]) {
        COD++;
      };

      if (COD > 999)
        COD = 1;
    }
    num_points = 0;
    for (x = 511; x >= 0; x -= 2) {
      if (window[1].mapa->points[x] != -1) {
        num_points = (x + 1) / 2;
        x = -1;
      }
    }
    fpg_add(my_fpg, COD, (char *)desc, (char *)window[1].mapa->filename, window[1].mapa->map_width,
            window[1].mapa->map_height, num_points, (char *)window[1].mapa->points,
            (char *)window[1].mapa->map, 0, 1);

    for (n = 0; n < 1000; n++)
      my_fpg->thumb[n].tagged = 0;

    call((void_return_type_t)v.paint_handler);

    v.redraw = 1;
    free_drag = 1;
  }

  if ((my_fpg->fpg_info) && (my_fpg->list_info.zone >= 10) && (mouse_b & 1)) {
    mouse_b = 0;
    Elemento = my_fpg->desc_index[(my_fpg->list_info.zone - 10) + my_fpg->list_info.first_visible];

    if ((fpg = fopen((char *)my_fpg->current_file, "rb")) == NULL) {
      // Error: file not found
      v_text = (char *)texts[43];
      show_dialog(err0);
      return;
    }
    fseek(fpg, my_fpg->grf_offsets[Elemento], SEEK_SET);
    fpg_read_image_header(&my_fpg->MiHeadFPG, fpg);
    fclose(fpg);

    if (fpg_image) {
      fpg_add(my_fpg, my_fpg->MiHeadFPG.code, (char *)my_fpg->MiHeadFPG.description,
              (char *)my_fpg->MiHeadFPG.Filename, my_fpg->MiHeadFPG.width, my_fpg->MiHeadFPG.height,
              my_fpg->MiHeadFPG.num_points, (char *)fpg_points, fpg_image, 1, 1);
    }

    if (fpg_image)
      free(fpg_image);
    if (fpg_points)
      free(fpg_points);
    v.redraw = 1;
    return;
  }

  if (dragging == 1 && my_fpg->list_info.zone >= 10 && !my_fpg->fpg_info) {
    drag_graphic = 8;
    dragging = 2;
    return;
  }

  if ((my_fpg->list_info.zone >= 10) && (mouse_b & 1 || prev_mouse_buttons & 1) &&
      (mouse_b != prev_mouse_buttons) && (dragging < 3)) {
    Elemento = (my_fpg->list_info.zone - 10) + my_fpg->list_info.first_visible;

    if (my_fpg->code_desc[Elemento][0] == 255) {
      my_fpg->code_desc[Elemento][0] = 175;
      my_fpg->thumb[Elemento].tagged = 1;
    } else {
      my_fpg->code_desc[Elemento][0] = 255;
      my_fpg->thumb[Elemento].tagged = 0;
    }
    FPG_create_listbox_br(&my_fpg->list_info);
    v.redraw = 1;
  }

  if ((dragging == 3) && (my_fpg->list_info.zone >= 10)) {
    Elemento = my_fpg->desc_index[(my_fpg->list_info.zone - 10) + my_fpg->list_info.first_visible];

    if ((fpg = fopen((char *)my_fpg->current_file, "rb")) == NULL) {
      // Error: file not found
      return;
    }
    fseek(fpg, my_fpg->grf_offsets[Elemento], SEEK_SET);
    fpg_read_header(&my_fpg->MiHeadFPG, fpg);

    map_width = my_fpg->MiHeadFPG.width;
    map_height = my_fpg->MiHeadFPG.height;

    v.mapa = (struct tmapa *)malloc(sizeof(struct tmapa));

    if (v.mapa == NULL) {
      fclose(fpg);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    v.mapa->map = (byte *)malloc(map_width * map_height);

    if (v.mapa->map == NULL) {
      fclose(fpg);
      free(v.mapa);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    v.mapa->map_width = my_fpg->MiHeadFPG.width;
    v.mapa->map_height = my_fpg->MiHeadFPG.height;

    v.mapa->has_name = 2;
    v.mapa->fpg_code = my_fpg->MiHeadFPG.code;
    memcpy(v.mapa->description, my_fpg->MiHeadFPG.description, 32);
    memset(v.mapa->filename, 0, 13);
    memcpy(v.mapa->filename, my_fpg->MiHeadFPG.Filename, 12);

    for (x = 0; x < 512; x++)
      v.mapa->points[x] = -1;
    if (my_fpg->MiHeadFPG.num_points)
      fread(v.mapa->points, my_fpg->MiHeadFPG.num_points, 4, fpg);
    fread(v.mapa->map, map_width, map_height, fpg);
    fclose(fpg);

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
    v.redraw = 1;
    return;
  } else if (dragging == 3)
    dragging = 0;

  v_pause = 1;
  FPG_update_listbox_br(&my_fpg->list_info);
  v_pause = 0;
}

void fpg_dialog3(void) {
  FPG *my_fpg = (FPG *)v.aux;
  int n;

  // Free FPG thumbnails
  for (n = 0; n < 1000; n++) {
    if (my_fpg->thumb[n].ptr != NULL)
      free(my_fpg->thumb[n].ptr);
  }
  free(v.aux);
}

extern byte color_tag;

void fpg_dialog0_new(void) {
  FPG *my_fpg;
  int n;

  v.type = 101; // Droppable
  v.w = 159;
  v.h = 72 + 5;
  v.paint_handler = fpg_dialog1;
  v.click_handler = fpg_dialog2;
  v.close_handler = fpg_dialog3;

  v.aux = v_aux;
  my_fpg = (FPG *)v.aux;

  for (n = 0; n < 1000; n++) {
    my_fpg->thumb[n].ptr = NULL;
    my_fpg->thumb[n].status = 0;
    my_fpg->thumb[n].tagged = 0;
  }

  my_fpg->list_info.created = 0;
  my_fpg->thumb_on = 0;
  fpg_create(my_fpg, full);
  // Safe copy: NombreFpg is 13 bytes, truncate to fit FPG header
  div_strcpy((char *)my_fpg->fpg_name, sizeof(my_fpg->fpg_name), input);
  v.title = my_fpg->fpg_name;
  v.name = my_fpg->fpg_name;
  my_fpg->fpg_info = 0;

  _flag(419, 4, v.h - 10, &my_fpg->thumb_on);
  _flag(108, (v.w - 5) - (8 * big2 + text_len(texts[108])), v.h - 10, &my_fpg->fpg_info);
}

void fpg_dialog0_add(void) {
  FPG *my_fpg;
  int n;
  v.type = 101; // Droppable
  v.w = 159;
  v.h = 72 + 5;
  v.paint_handler = fpg_dialog1;
  v.click_handler = fpg_dialog2;
  v.close_handler = fpg_dialog3;

  v.aux = v_aux;
  my_fpg = (FPG *)v.aux;
  for (n = 0; n < 1000; n++) {
    my_fpg->thumb[n].ptr = NULL;
    my_fpg->thumb[n].status = 0;
    my_fpg->thumb[n].tagged = 0;
  }

  my_fpg->last_used = 1;
  fpg_open(my_fpg, full);
  // Safe copy: NombreFpg is 13 bytes, truncate to fit FPG header
  div_strcpy((char *)my_fpg->fpg_name, sizeof(my_fpg->fpg_name), input);
  v.title = my_fpg->fpg_name;
  v.name = my_fpg->fpg_name;

  _flag(419, 4, v.h - 10, &my_fpg->thumb_on);
  _flag(108, (v.w - 5) - (8 * big2 + text_len(texts[108])), v.h - 10, &my_fpg->fpg_info);
}

int new_file(void) {
  v_accept = 0;
  v_mode = 1;
  v_type = 4;
  v_text = (char *)texts[69];
  show_dialog(browser0);

  div_strcpy(full, sizeof(full), file_types[v_type].path);

  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "//");

  div_strcat(full, sizeof(full), input);

  if (v_finished) {
    if (!v_exists) {
      v_accept = 1;
    } else {
      v_title = (char *)texts[82];
      v_text = input;
      show_dialog(accept0);
    }
    if (v_accept) {
      v_aux = (byte *)malloc(sizeof(FPG));

      if (v_aux == NULL) {
        v_text = (char *)texts[45];
        show_dialog(err0);
        return 0;
      }

      close_fpg(full);
      memset(v_aux, 0, sizeof(FPG));
      new_window(fpg_dialog0_new);
    } else
      return 0;

    return 1;
  }
  return 0;
}

#define MAX_FILES 512 // ------------------------------- File listbox
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[MAX_FILES];
extern int num_taggeds;

void create_palette(void);
void merge_palettes(void);
void palette_action0(void);
extern byte work_palette[768];

extern byte apply_palette[768];
extern byte *sample;

void open_file(void) {
  FPG *my_fpg;
  char cwork[8];
  FILE *f;
  int num;

  int x, sum;
  int n;
  byte pal[768];

  v_mode = 0;
  v_type = 4;
  v_text = (char *)texts[70];
  show_dialog(browser0);
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

  ////////////////////
  // *** Juanjo *** //

  n = 0; // Number of distinct palettes
  sample = NULL;
  memcpy(pal, dac, 768);

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);

      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");

      div_strcat(full, sizeof(full), input);

      if ((f = fopen(full, "rb")) == NULL) {
        v_text = (char *)texts[44];
        show_dialog(err0);
        continue;
      }

      fread(cwork, 8, 1, f);

      if (strcmp(cwork, "fpg\x1a\x0d\x0a")) {
        fclose(f);
        v_text = (char *)texts[46];
        show_dialog(err0);
        continue;
      }

      fread(dac4, 768, 1, f);
      fclose(f);

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

  if (sample != NULL) {
    create_palette();
    free(sample);
    memcpy(pal, apply_palette, 768);
  }

  // Now pal[] holds the palette of the FPGs to load

  x = 0;
  sum = 0;
  do {
    sum += abs((int)pal[x] - (int)dac[x]);
  } while (++x < 768);

  if (sum) {
    memcpy(work_palette, pal, 768);
    show_dialog(palette_action0); // Load palette?

    switch (v_accept) {
    case 0: // Cancel (don't load)
      return;
      break;

    case 1: // Remap graphics to system palette
      break;

    case 2: // Merge palettes
      memcpy(dac4, pal, 768);
      merge_palettes();
      pal_refresh(0, 1);
      break;

    case 3: // Apply new palette
      memcpy(dac4, pal, 768);
      pal_refresh(0, 1);
      break;
    }
  }

  // *** Juanjo *** //
  ////////////////////

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);

      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");

      div_strcat(full, sizeof(full), input);

      if ((f = fopen(full, "rb")) != NULL) { // A file was selected
        if (fread(cwork, 1, 8, f) == 8) {
          fclose(f);

          if (!strcmp(cwork, "fpg\x1a\x0d\x0a")) {
            v_aux = (byte *)malloc(sizeof(FPG));
            if (v_aux == NULL) {
              v_text = (char *)texts[45];
              show_dialog(err0);
              continue;
            }

            my_fpg = (FPG *)v_aux;
            my_fpg->thumb_on = 0;
            my_fpg->fpg_info = 0;
            my_fpg->list_info.created = 0;
            close_fpg(full);
            memset(v_aux, 0, sizeof(FPG));
            new_window(fpg_dialog0_add);
          } else {
            v_text = (char *)texts[46];
            show_dialog(err0);
          }
        } else {
          fclose(f);
          v_text = (char *)texts[44];
          show_dialog(err0);
        }
      } else {
        v_text = (char *)texts[44];
        show_dialog(err0);
      }
    }
  }

  create_dac4();
}

void _ffwrite(byte *Buffer, unsigned int Len, FILE *file) {
  while (Len >= 768) {
    fwrite(Buffer, 768, 1, file);
    Buffer += 768;
    Len -= 768;
  }

  fwrite(Buffer, Len, 1, file);
}

int v2convert;
char cNamev2convert[128];

void fpg_warning1(void) {
  int w = v.w / big2, h = v.h / big2;
  char cWork[70];

  _show_items();
  wwrite(v.ptr, w, h, w / 2, 12, 1, texts[172], c3);
  div_snprintf(cWork, sizeof(cWork), (char *)texts[173], cNamev2convert);
  wwrite(v.ptr, w, h, w / 2, 19, 1, (byte *)cWork, c3);
  wwrite(v.ptr, w, h, w / 2, 30, 1, texts[174], c4);
  v_accept = 0;
}

void fpg_warning2(void) {
  _process_items();

  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;

  case 1:
    v_accept = 0;
    end_dialog = 1;
    break;
  }
}
void fpg_warning0(void) {
  v.type = 1; // Dialog
  v.w = 200;
  v.h = 60;
  v.title = texts[171];
  v.paint_handler = fpg_warning1;
  v.click_handler = fpg_warning2;

  _button(123, 7, v.h - 14, 0);
  _button(119, v.w - 8, v.h - 14, 2);
}


extern byte aux_pal[768];

int remap_all_files(int vent) {
  // Ask whether to remap the FPG palette or close it

  FPG *my_fpg = (FPG *)window[vent].aux;
  byte p[768]; // to compare file palette with DAC
  FILE *f;
  int sum, x;

  if ((f = fopen((char *)my_fpg->current_file, "rb")) != NULL) {
    fseek(f, 8, SEEK_SET);
    fread(p, 1, 768, f);
    fclose(f);
    x = 0;
    sum = 0;

    do {
      sum += abs((int)dac[x] - (int)p[x]);
    } while (++x < 768);

    if (!sum)
      return (0);
  }

  div_strcpy(cNamev2convert, sizeof(cNamev2convert), (char *)window[vent].title);
  show_dialog(fpg_warning0);

  switch (v_accept) {
  case 0:
    move(0, vent);
    close_window();
    break;

  case 1:
    v_title = (char *)texts[344];
    v_text = (char *)texts[345];

    show_dialog(accept0);

    if (v_accept) {
      fpg_remap_to_pal(my_fpg);
    } else {
      move(0, vent);
      close_window();
    }
    break;
  }

  return (1);
}

// Graphic code dialog
extern int get_code_width;
extern int get_code_height;
extern char *get_code_image;
extern short get_code_p0x;
extern short get_code_p0y;
char *get_code_image_red;
int get_code_width_red;
int get_code_height_red;
void fpg_edit_code_dialog(void);

void get_code1(void) {
  char cWork[64];
  int w = v.w / big2, h = v.h / big2;
  int x, y, px, py;

  _show_items();

  wwrite(v.ptr, w, h, 4, 32, 0, texts[133], c3);
  wwrite(v.ptr, w, h, 4, 42, 0, texts[134], c3);

  div_snprintf(cWork, sizeof(cWork), "%d", get_code_width);
  wwrite(v.ptr, w, h, 34, 32, 0, (byte *)cWork, c4);
  div_snprintf(cWork, sizeof(cWork), "%d", get_code_height);
  wwrite(v.ptr, w, h, 34, 42, 0, (byte *)cWork, c4);

  wwrite(v.ptr, w, h, 64, 32, 0, texts[152], c3);
  wwrite(v.ptr, w, h, 64, 42, 0, texts[153], c3);

  if (get_code_p0x == -1) {
    get_code_p0x = get_code_width / 2;
    get_code_p0y = get_code_height / 2;
  }

  div_snprintf(cWork, sizeof(cWork), "%d", get_code_p0x);
  wwrite(v.ptr, w, h, 108, 32, 0, (byte *)cWork, c4);
  div_snprintf(cWork, sizeof(cWork), "%d", get_code_p0y);
  wwrite(v.ptr, w, h, 108, 42, 0, (byte *)cWork, c4);

  wbox(v.ptr, w, h, c1, w - (72 + 4), 12, 72, 46);
  wrectangle(v.ptr, w, h, c0, w - (72 + 4), 12, 72, 46);

  if (get_code_image_red != NULL) {
    px = ((40 * big2) + (get_code_width_red / 2));
    py = ((35 * big2) - (get_code_height_red / 2));

    for (y = 0; y < get_code_height_red; y++)
      for (x = 0; x < get_code_width_red; x++)
        v.ptr[(py + y) * v.w + ((v.w - px) + x)] = get_code_image_red[y * get_code_width_red + x];
  }
}

void get_code2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    if (atoi(code_str) != 0) {
      ret_value = 1;
      end_dialog = 1;
      v.redraw = 1;
    }
    break;

  case 1:
    end_dialog = 1;
    v.redraw = 1;
    break;
  }
}

void get_code3(void) {
  free(get_code_image_red);
}

void get_code0(void) {
  v.type = 1; // Dialog
  v.w = 180 + 25;
  v.h = 100 - 12;
  v.title = texts[68];
  v.paint_handler = get_code1;
  v.click_handler = get_code2;
  v.close_handler = get_code3;

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);

  _get(71, 4, 12, 30, (byte *)code_str, 5, 1, 999);
  _get(439, 48, 12, 64, (byte *)file_str, 12, 0, 0);
  _get(72, 4, 52, v.w - 8, (byte *)description, 32, 0, 0);
}

void fpg_edit_code_dialog(void) {
  int n, m;
  int man, mal;
  byte *temp, *temp2;
  float coefredy, coefredx, a, b;
  int x, y;

  if (atoi(code_str) == 0)
    div_strcpy(code_str, sizeof(code_str), "1");

  // Create the thumbnail

  if (get_code_width > 70 * big2 || get_code_height > 44 * big2) {
    man = get_code_width;
    mal = get_code_height;
    temp = (byte *)get_code_image;

    coefredx = (float)man / (float)(70 * 2 * big2);
    coefredy = (float)mal / (float)(44 * 2 * big2);

    if (coefredx > coefredy)
      coefredy = coefredx;
    else
      coefredx = coefredy;

    get_code_width_red = (float)man / coefredx + 0.5;
    get_code_height_red = (float)mal / coefredy + 0.5;
    get_code_width_red &= -2;
    get_code_height_red &= -2;

    if (get_code_width_red < 2)
      get_code_width_red = 2;

    if (get_code_height_red < 2)
      get_code_height_red = 2;

    if (coefredx * (float)(get_code_width_red - 1) >= (float)man)
      coefredx = (float)(man - 1) / (float)(get_code_width_red - 1);

    if (coefredy * (float)(get_code_height_red - 1) >= (float)mal)
      coefredy = (float)(mal - 1) / (float)(get_code_height_red - 1);

    if ((temp2 = (byte *)malloc(get_code_width_red * get_code_height_red)) != NULL) {
      memset(temp2, 0, get_code_width_red * get_code_height_red);
      a = (float)0.0;

      for (y = 0; y < get_code_height_red; y++) {
        b = (float)0.0;
        for (x = 0; x < get_code_width_red; x++) {
          temp2[y * get_code_width_red + x] = temp[((memptrsize)a) * man + (memptrsize)b];
          b += coefredx;
        }
        a += coefredy;
      }

      if ((get_code_image_red = (char *)malloc((get_code_width_red * get_code_height_red) / 4)) !=
          NULL) {
        for (y = 0; y < get_code_height_red; y += 2) {
          for (x = 0; x < get_code_width_red; x += 2) {
            n = *(ghost + temp2[x + y * get_code_width_red] * 256 +
                  temp2[x + 1 + y * get_code_width_red]);
            m = *(ghost + temp2[x + (y + 1) * get_code_width_red] * 256 +
                  temp2[x + 1 + (y + 1) * get_code_width_red]);
            get_code_image_red[x / 2 + (y / 2) * (get_code_width_red / 2)] = *(ghost + n * 256 + m);
          }
        }

        get_code_width_red /= 2;
        get_code_height_red /= 2;
      } else {
        get_code_image_red = NULL;
      }
      free(temp2);
    } else {
      get_code_image_red = NULL;
    }
  } else {
    get_code_width_red = get_code_width;
    get_code_height_red = get_code_height;

    if ((get_code_image_red = (char *)malloc(get_code_width_red * get_code_height_red)) != NULL) {
      memcpy(get_code_image_red, get_code_image, get_code_width_red * get_code_height_red);
    }
  }

  show_dialog(get_code0);
}

int find_fpg_window();

void show_tagged() {
  FPG *my_fpg;
  int a, x, y, Elemento, n;
  FILE *fpg;

  if (!(n = find_fpg_window()))
    return;

  my_fpg = (FPG *)window[n].aux;

  for (a = 0; a < my_fpg->nIndex; a++)

    if (my_fpg->code_desc[a][0] == 175) {
      Elemento = my_fpg->desc_index[a];

      if ((fpg = fopen((char *)my_fpg->current_file, "rb")) == NULL) {
        // Error: file not found
        return;
      }

      fseek(fpg, my_fpg->grf_offsets[Elemento], SEEK_SET);

      fpg_read_header(&my_fpg->MiHeadFPG, fpg);

      map_width = my_fpg->MiHeadFPG.width;
      map_height = my_fpg->MiHeadFPG.height;

      if (new_map(NULL)) {
        // ERROR: Out of memory
        fclose(fpg);
        return;
      }

      v.mapa->has_name = 1;
      v.mapa->fpg_code = my_fpg->MiHeadFPG.code;
      memcpy(v.mapa->description, my_fpg->MiHeadFPG.description, 32);
      memset(v.mapa->filename, 0, 13);
      memcpy(v.mapa->filename, my_fpg->MiHeadFPG.Filename, 12);

      for (x = 0; x < 512; x++)
        v.mapa->points[x] = -1;

      if (my_fpg->MiHeadFPG.num_points)
        fread(v.mapa->points, my_fpg->MiHeadFPG.num_points, 4, fpg);

      fread(v.mapa->map, map_width, map_height, fpg);
      fclose(fpg);

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

      activate();
      call((void_return_type_t)v.paint_handler);
      blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
    }
}

int fpg_delete_many(FPG *Fpg, int taggeds, int *array_del);

void delete_tagged() {
  FPG *my_fpg;
  int taggeds = 0, n, vent;
  int *array_del;
  FILE *fpg;

  if (!(vent = find_fpg_window()))
    return;

  my_fpg = (FPG *)window[vent].aux;

  for (n = 0; n < my_fpg->nIndex; n++)
    if (my_fpg->code_desc[n][0] == 175)
      taggeds++;

  if ((array_del = (int *)malloc(taggeds * 4)) == NULL)
    return;

  if ((fpg = fopen((char *)my_fpg->current_file, "rb")) == NULL) { // This should never fail
    free(array_del);
    return;
  }

  fclose(fpg);

  taggeds = 0;

  for (n = 0; n < my_fpg->nIndex; n++)
    if (my_fpg->code_desc[n][0] == 175) {
      array_del[taggeds++] = my_fpg->desc_index[n];
    }

  fpg_delete_many(my_fpg, taggeds, array_del);

  free(array_del);

  wup(vent);

  while (my_fpg->list_info.first_visible + (my_fpg->list_info.lines - 1) * my_fpg->list_info.columns +
             1 >
         my_fpg->list_info.total_items) {
    my_fpg->list_info.first_visible -= my_fpg->list_info.columns;
  }

  if (my_fpg->list_info.first_visible < 0)
    my_fpg->list_info.first_visible = 0;

  wmouse_x = -1;
  wmouse_y = -1;
  FPG_update_listbox_br(&my_fpg->list_info);
  call((void_return_type_t)v.paint_handler);
  wdown(vent);
  flush_window(vent);
}

//-----------------------------------------------------------------------------
//  Print graphic list from a file
//-----------------------------------------------------------------------------

int f_im = 1, f_ar = 0;
char n_ar[16] = "";

void printlist1(void) {
  _show_items();
}

void printlist2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    f_im = 1;
    f_ar = 0;
    _show_items();
    break;
  case 3:
    f_im = 0;
    f_ar = 1;
    _show_items();
    break;
  }
}

void printlist0(void) {
  v.type = 1;
  v.w = 120;
  v.h = 38 + 10;
  v.title = (byte *)v_title;
  v.paint_handler = printlist1;
  v.click_handler = printlist2;
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _flag(448, 4, 12, &f_im);
  _flag(449, 4, 12 + 9, &f_ar);
  _get(414, 16 + text_len(texts[449]), 12, v.w - 20 - text_len(texts[449]), (byte *)v_text, 15, 0,
       0);
  v_accept = 0;
}

void print_list(void) {
  FPG *my_fpg;
  HeadFPG cab;
  FILE *f = NULL, *g;
  int n, vent;
  int _num = 0, num = 0;
  char cwork[512], cwork2[13];
  unsigned u;

  if (!(vent = find_fpg_window()))
    return;

  if (!strlen(n_ar))
    div_strcpy(n_ar, sizeof(n_ar), (char *)texts[451]);

  v_text = n_ar;
  v_title = (char *)texts[438];

  show_dialog(printlist0);

  if (v_accept) {
    my_fpg = (FPG *)window[vent].aux;

    for (n = 0; n < 1000; n++)
      if (my_fpg->grf_offsets[n])
        num++;

    if (num > 0) {
      if (f_ar) {
        _dos_setdrive(toupper(*file_types[1].path) - 'A' + 1, &u);
        chdir(file_types[1].path);
        f = fopen(n_ar, "rb");

        if (f != NULL) {
          fclose(f);
          div_snprintf(cwork, sizeof(cwork), "%s\\%s", file_types[1].path, n_ar);
          strupr(cwork);
          v_title = (char *)texts[450];
          v_text = cwork;
          show_dialog(accept0);

          if (!v_accept)
            return;
        }

        f = fopen(n_ar, "wb");

        if (f == NULL) {
          v_text = (char *)texts[47];
          show_dialog(err0);
          return;
        }
      }

      g = fopen((char *)my_fpg->current_file, "rb");
      if (g == NULL) { // This should never happen

        if (f_ar)
          fclose(f);
        return;
      }

      for (n = 0; n < 1000; n++) {
        if (my_fpg->grf_offsets[n]) {
          show_progress((char *)texts[437], ++_num, num);
          fseek(g, my_fpg->grf_offsets[n], SEEK_SET);
          fread(&cab, 1, sizeof(cab), g);
          memset(cwork2, 0, 13);
          memcpy(cwork2, cab.Filename, 12);
          div_snprintf(cwork, sizeof(cwork), "[%03d] %s (%s, %dx%d)", cab.code, cab.description,
                       cwork2, cab.width, cab.height);
          if (f_ar) {
            fwrite(cwork, 1, strlen(cwork), f);
            fwrite("\xd\xa", 1, 2, f);
          } else {
            fwrite(cwork, 1, strlen(cwork), stdprn);
            fwrite("\xd\xa", 1, 2, stdprn);
          }
        }
      }

      fclose(g);

      if (f_ar)
        fclose(f);
      else
        fwrite("\f", 1, 1, stdprn);
    }
  }
}

//-----------------------------------------------------------------------------
//  FPG browser functions
//-----------------------------------------------------------------------------

extern int num;

void FPG_create_thumbs(void) {
  FPG *my_fpg = (FPG *)v.aux;

  if (my_fpg->thumb_on) {
    do {
      create_thumb_FPG(&(my_fpg->list_info));
      if (num > -1) {
        if (my_fpg->thumb[num].ptr != NULL && my_fpg->thumb[num].status == 0) {
          FPG_show_thumb(&(my_fpg->list_info), num);
          break;
        } else if (my_fpg->thumb[num].ptr == NULL && my_fpg->thumb[num].status == -1) {
          FPG_show_thumb(&(my_fpg->list_info), num);
        } else
          break;
      } else
        break;
    } while (1);
  }
}

void FPG_show_thumb(struct t_listboxbr *l, int num) {
  FPG *my_fpg = (FPG *)v.aux;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int px, py, x, y, ly, incy;
  char p[40];

  if (num >= l->first_visible && num < l->first_visible + l->lines * l->columns) {
    px = (l->x + 1 + (l->w + 1) * ((num - l->first_visible) % l->columns)) * big2 +
         (l->w * big2 - my_fpg->thumb[num].w) / 2;

    if ((incy = ((l->h - 8) * big2 - my_fpg->thumb[num].h) / 2) < 0)
      incy = 0;

    py = (l->y + 1 + (l->h + 1) * ((num - l->first_visible) / l->columns)) * big2 + incy;

    ly = (l->y + (l->h + 1) * ((num - l->first_visible) / l->columns) + l->h - 8) * big2;

    if (my_fpg->thumb_on && my_fpg->thumb[num].ptr != NULL && my_fpg->thumb[num].status == 0) {
      for (y = 0; y < my_fpg->thumb[num].h; y++)
        for (x = 0; x < my_fpg->thumb[num].w; x++)

          if (py + y > ly) {
            if (!my_fpg->thumb[num].tagged)
              v.ptr[(py + y) * v.w + (px + x)] =
                  *(ghost + c0 * 256 + my_fpg->thumb[num].ptr[y * my_fpg->thumb[num].w + x]);
          } else
            v.ptr[(py + y) * v.w + (px + x)] = my_fpg->thumb[num].ptr[y * my_fpg->thumb[num].w + x];
    }

    px -= (l->w * big2 - my_fpg->thumb[num].w) / 2;
    py -= incy;
    px /= big2;
    py /= big2;

    if (my_fpg->thumb[num].ptr == NULL && my_fpg->thumb[num].status == -1) {
      wput(ptr, w, h, px + (l->w - 21) / 2, py + 1, 60);
    }

    py += l->h - 1;
    div_strcpy(p, sizeof(p), l->list + l->item_width * num);
    if (my_fpg->thumb_on)
      p[5] = 0;

    if (l->zone - 10 == num - l->first_visible)
      x = c4;
    else
      x = c3;

    if (text_len((byte *)p) < l->w - 2 && my_fpg->thumb_on) {
      wwrite(ptr, w, h, px + l->w / 2 + 1, py, 7, (byte *)p, c0);
      wwrite(ptr, w, h, px + l->w / 2, py, 7, (byte *)p, x);
    } else {
      wwrite_in_box(ptr, w, px + l->w - 1, h, px + 2, py, 6, (byte *)p, c0);
      wwrite_in_box(ptr, w, px + l->w - 1, h, px + 1, py, 6, (byte *)p, x);
    }

    v.redraw = 1;
  }
}

void FPG_paint_listbox_br(struct t_listboxbr *l) {
  FPG *my_fpg = (FPG *)v.aux;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int n, y, x;

  color_tag = c_b_low;

  for (y = 0; y < l->lines; y++)
    for (x = 0; x < l->columns; x++) {
      wbox(ptr, w, h, c1, l->x + (x * (l->w + 1)) + 1, l->y + (y * (l->h + 1)) + 1, l->w, l->h - 8);

      if (my_fpg->thumb[l->first_visible + y * l->columns + x].tagged)
        wbox(ptr, w, h, color_tag, l->x + (x * (l->w + 1)) + 1,
             l->y + (y * (l->h + 1)) + 1 + l->h - 8, l->w, 8);
      else
        wbox(ptr, w, h, c01, l->x + (x * (l->w + 1)) + 1, l->y + (y * (l->h + 1)) + 1 + l->h - 8,
             l->w, 8);
    }

  if (wmouse_in(l->x, l->y, (l->w + 1) * l->columns, (l->h + 1) * l->lines)) { // Calculate zone
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
    FPG_show_thumb(l, l->first_visible + --n);
}

void FPG_paint_slider_br(struct t_listboxbr *l) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wbox(ptr, w, h, c2, l->x + (l->w + 1) * l->columns + 1, l->y + 9, 7, (l->h + 1) * l->lines - 17);

  if (l->slide > l->s0)
    wbox(ptr, w, h, c0, l->x + (l->w + 1) * l->columns + 1, l->slide - 1, 7, 1);

  if (l->slide < l->s1)
    wbox(ptr, w, h, c0, l->x + (l->w + 1) * l->columns + 1, l->slide + 3, 7, 1);

  wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->slide, 43);
}

void FPG_create_listbox_br(struct t_listboxbr *l) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int x, y;

  if (!l->created) {
    l->slide = l->s0 = l->y + 9;
    l->s1 = l->y + (l->h * l->lines + l->lines + 1) - 12;
    l->buttons = 0;
    l->created = 1;
    l->zone = 0;
    l->first_visible = file_types[v_type].first_visible;

    if ((l->first_visible + (l->lines - 1) * l->columns) >= l->total_items)
      l->first_visible = 0;
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

  FPG_paint_listbox_br(l);
  FPG_paint_slider_br(l);
}

void FPG_update_listbox_br(struct t_listboxbr *l) {
  FPG *my_fpg = (FPG *)v.aux;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int n, old_zona = l->zone, x, y;
  char p[40];

  if (wmouse_in(l->x, l->y, (l->w + 1) * l->columns, (l->h + 1) * l->lines)) { // Calculate zone
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

  if (old_zona != l->zone)
    if (old_zona >= 10) { // Unhighlight zone
      x = l->x + 1 + ((old_zona - 10) % l->columns) * (l->w + 1);
      y = l->y + l->h + ((old_zona - 10) / l->columns) * (l->h + 1);
      div_strcpy(p, sizeof(p), l->list + l->item_width * (l->first_visible + old_zona - 10));

      if (my_fpg->thumb_on)
        p[5] = 0;

      if (text_len((byte *)p) < l->w - 2 && my_fpg->thumb_on) {
        wwrite(ptr, w, h, x + l->w / 2, y, 7, (byte *)p, c3);
      } else {
        wwrite_in_box(ptr, w, x + l->w - 1, h, x + 1, y, 6, (byte *)p, c3);
      }
      v.redraw = 1;
    }

  if ((mouse_b & 8 && wmouse_x != -1) ||
      (l->zone == 2 &&
       ((mouse_b & 1) || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))))) {
    if (mouse_b & 8 || !v_pause || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))) {
      if ((prev_mouse_buttons & 1) && !v_pause) {
        retrace_wait();
        retrace_wait();
      }
      //---
      if (l->first_visible) {
        l->first_visible -= l->columns;
        FPG_paint_listbox_br(l);
        v.redraw = 1;
      }
      //---
    }
    //---
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + 1, -41);
    l->buttons |= 1;
    v.redraw = 1;
  } else if (l->buttons & 1) {
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + 1, -39);
    l->buttons ^= 1;
    v.redraw = 1;
  }

  if ((mouse_b & 4 && wmouse_x != -1) ||
      (l->zone == 3 &&
       ((mouse_b & 1) || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))))) {
    if (mouse_b & 4 || !v_pause || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))) {
      if ((prev_mouse_buttons & 1) && !v_pause) {
        retrace_wait();
        retrace_wait();
      }
      //---
      n = l->total_items - l->first_visible;

      if (n > l->lines * l->columns) {
        l->first_visible += l->columns;
        FPG_paint_listbox_br(l);
        v.redraw = 1;
      }
      //---
    }
    //---
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + (l->h + 1) * l->lines - 7, -42);

    l->buttons |= 2;
    v.redraw = 1;
  } else if (l->buttons & 2) {
    wput(ptr, w, h, l->x + (l->w + 1) * l->columns + 1, l->y + (l->h + 1) * l->lines - 7, -40);
    l->buttons ^= 2;
    v.redraw = 1;
  }

  if (l->zone == 4 && (mouse_b & 1)) {
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
        FPG_paint_listbox_br(l);
      }
    }

    FPG_paint_slider_br(l);
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
      FPG_paint_slider_br(l);
      v.redraw = 1;
    }
  }

  if (old_zona != l->zone)
    if (l->zone >= 10) { // Highlight zone
      x = l->x + 1 + ((l->zone - 10) % l->columns) * (l->w + 1);
      y = l->y + l->h + ((l->zone - 10) / l->columns) * (l->h + 1);
      div_strcpy(p, sizeof(p), l->list + l->item_width * (l->first_visible + l->zone - 10));

      if (my_fpg->thumb_on)
        p[5] = 0;

      if (text_len((byte *)p) < l->w - 2 && my_fpg->thumb_on) {
        wwrite(ptr, w, h, x + l->w / 2, y, 7, (byte *)p, c4);
      } else {
        wwrite_in_box(ptr, w, x + l->w - 1, h, x + 1, y, 6, (byte *)p, c4);
      }
      v.redraw = 1;
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

void create_thumb_FPG(struct t_listboxbr *l) {
  FPG *my_fpg = (FPG *)v.aux;
  int estado = 0, n, m;
  int man, mal;
  FILE *f;
  byte *temp, *temp2;
  float coefredy, coefredx, a, b;
  int x, y;

  num = -1;

  if ((n = abs(old_mouse_x - mouse_x) + abs(old_mouse_y - mouse_y) + mouse_b * 10)) {
    incremento = (float)incremento / ((float)n / 20.0 + 1.0);
    incremento /= 256;
    incremento *= 256;

    if (incremento < 512)
      incremento = 512;
  }

  if (ascii) {
    incremento = 512;
  } else if (incremento < incremento_maximo)
    incremento += 512;

  if (l->total_items) {
    num = l->first_visible;

    do {
      if (my_fpg->thumb[num].ptr == NULL && my_fpg->thumb[num].status > -1) {
        if (mouse_b == 0)
          estado = 1;
        break;
      }

      if (my_fpg->thumb[num].ptr != NULL && my_fpg->thumb[num].status > 0) {
        estado = 2;
        break;
      }

      if (++num == l->total_items)
        num = 0;

    } while (num != l->first_visible);

    if (estado == 0) {
      num = -1;
      return;
    }

    if (estado == 1) { // Start reading a new thumbnail

      if ((f = fopen((char *)my_fpg->current_file, "rb")) != NULL) {
        fseek(f, my_fpg->grf_offsets[my_fpg->desc_index[num]], SEEK_SET);
        fpg_read_header(&(my_fpg->MiHeadFPG), f);
        fseek(f, my_fpg->MiHeadFPG.num_points * 4, SEEK_CUR);
        my_fpg->thumb[num].w = my_fpg->MiHeadFPG.width;
        my_fpg->thumb[num].h = my_fpg->MiHeadFPG.height;
        my_fpg->thumb[num].filesize = my_fpg->MiHeadFPG.width * my_fpg->MiHeadFPG.height;

        if (my_fpg->thumb[num].filesize <= 2048)
          incremento = 2048;

        if ((my_fpg->thumb[num].ptr = (char *)malloc(my_fpg->thumb[num].filesize)) != NULL) {
          if (my_fpg->thumb[num].filesize > incremento) {
            if (fread(my_fpg->thumb[num].ptr, 1, incremento, f) == incremento) {
              my_fpg->thumb[num].status = incremento;
            } else {
              free(my_fpg->thumb[num].ptr);
              my_fpg->thumb[num].ptr = NULL;
              estado = 0;
              my_fpg->thumb[num].status = -1;
            }
          } else {
            if (fread(my_fpg->thumb[num].ptr, 1, my_fpg->thumb[num].filesize, f) ==
                my_fpg->thumb[num].filesize) {
              my_fpg->thumb[num].status = my_fpg->thumb[num].filesize;
            } else {
              free(my_fpg->thumb[num].ptr);
              my_fpg->thumb[num].ptr = NULL;
              estado = 0;
              my_fpg->thumb[num].status = -1;
            }
          }

          fclose(f);
        } else {
          fclose(f);
          estado = 0;
          my_fpg->thumb[num].status = -1;
        }
      } else {
        estado = 0;
        my_fpg->thumb[num].status = -1;
      }
      return;
    } else if (estado == 2 && my_fpg->thumb[num].status !=
                                  my_fpg->thumb[num].filesize) { // Continue reading a thumbnail

      if ((f = fopen((char *)my_fpg->current_file, "rb")) != NULL) {
        fseek(f, my_fpg->grf_offsets[my_fpg->desc_index[num]], SEEK_SET);
        fpg_read_header(&(my_fpg->MiHeadFPG), f);
        fseek(f, my_fpg->MiHeadFPG.num_points * 4, SEEK_CUR);
        fseek(f, my_fpg->thumb[num].status, SEEK_CUR);

        if (my_fpg->thumb[num].filesize - my_fpg->thumb[num].status > incremento) {
          if (fread(&(my_fpg->thumb[num].ptr)[my_fpg->thumb[num].status], 1, incremento, f) ==
              incremento) {
            my_fpg->thumb[num].status += incremento;
          } else {
            free(my_fpg->thumb[num].ptr);
            my_fpg->thumb[num].ptr = NULL;
            estado = 0;
            my_fpg->thumb[num].status = -1;
          }
        } else {
          if (fread(&(my_fpg->thumb[num].ptr)[my_fpg->thumb[num].status], 1,
                    my_fpg->thumb[num].filesize - my_fpg->thumb[num].status,
                    f) == my_fpg->thumb[num].filesize - my_fpg->thumb[num].status) {
            my_fpg->thumb[num].status = my_fpg->thumb[num].filesize;
          } else {
            free(my_fpg->thumb[num].ptr);
            my_fpg->thumb[num].ptr = NULL;
            estado = 0;
            my_fpg->thumb[num].status = -1;
          }
        }

        fclose(f);
      } else {
        estado = 0;
        my_fpg->thumb[num].status = -1;
      }

      return;
    }

    // Now create the thumbnail if the file is fully loaded
    if (estado == 2 && my_fpg->thumb[num].status == my_fpg->thumb[num].filesize &&
        abs(old_mouse_x - mouse_x) + abs(old_mouse_y - mouse_y) + mouse_b + ascii == 0) {
      my_fpg->thumb[num].status = 0;
      man = my_fpg->thumb[num].w;
      mal = my_fpg->thumb[num].h;
      temp = (byte *)my_fpg->thumb[num].ptr;

      // Create the thumbnail
      if (man > 47 * big2 || mal > 26 * big2) {
        coefredx = (float)man / (float)(47 * 2 * big2);
        coefredy = (float)mal / (float)(26 * 2 * big2);

        if (coefredx > coefredy)
          coefredy = coefredx;
        else
          coefredx = coefredy;

        my_fpg->thumb[num].w = (float)man / coefredx + 0.5;
        my_fpg->thumb[num].h = (float)mal / coefredy + 0.5;
        my_fpg->thumb[num].w &= -2;
        my_fpg->thumb[num].h &= -2;

        if (my_fpg->thumb[num].w < 2)
          my_fpg->thumb[num].w = 2;

        if (my_fpg->thumb[num].h < 2)
          my_fpg->thumb[num].h = 2;

        if (coefredx * (float)(my_fpg->thumb[num].w - 1) >= (float)man)
          coefredx = (float)(man - 1) / (float)(my_fpg->thumb[num].w - 1);

        if (coefredy * (float)(my_fpg->thumb[num].h - 1) >= (float)mal)
          coefredy = (float)(mal - 1) / (float)(my_fpg->thumb[num].h - 1);

        if ((temp2 = (byte *)malloc(my_fpg->thumb[num].w * my_fpg->thumb[num].h)) != NULL) {
          memset(temp2, 0, my_fpg->thumb[num].w * my_fpg->thumb[num].h);
          a = (float)0.0;

          for (y = 0; y < my_fpg->thumb[num].h; y++) {
            b = (float)0.0;

            for (x = 0; x < my_fpg->thumb[num].w; x++) {
              temp2[y * my_fpg->thumb[num].w + x] = temp[((memptrsize)a) * man + (memptrsize)b];
              b += coefredx;
            }
            a += coefredy;
          }

          free(temp);

          if ((my_fpg->thumb[num].ptr =
                   (char *)malloc((my_fpg->thumb[num].w * my_fpg->thumb[num].h) / 4)) != NULL) {
            for (y = 0; y < my_fpg->thumb[num].h; y += 2) {
              for (x = 0; x < my_fpg->thumb[num].w; x += 2) {
                n = *(ghost + temp2[x + y * my_fpg->thumb[num].w] * 256 +
                      temp2[x + 1 + y * my_fpg->thumb[num].w]);
                m = *(ghost + temp2[x + (y + 1) * my_fpg->thumb[num].w] * 256 +
                      temp2[x + 1 + (y + 1) * my_fpg->thumb[num].w]);
                my_fpg->thumb[num].ptr[x / 2 + (y / 2) * (my_fpg->thumb[num].w / 2)] =
                    *(ghost + n * 256 + m);
              }
            }

            my_fpg->thumb[num].w /= 2;
            my_fpg->thumb[num].h /= 2;
          } else {
            my_fpg->thumb[num].ptr = NULL;
            my_fpg->thumb[num].status = -1;
          }
          free(temp2);
        } else {
          my_fpg->thumb[num].ptr = NULL;
          my_fpg->thumb[num].status = -1;
        }
      }
    }
  }

  if (oclock < *system_clock) {
    old_mouse_x = omx;
    old_mouse_y = omy;
    omx = mouse_x;
    omy = mouse_y;
    oclock = *system_clock;
  }
}

//-----------------------------------------------------------------------------
//  Functions for importing maps into FPG
//-----------------------------------------------------------------------------

void map_to_fpg(struct tmapa *mapa) {
  int x, y;
  int width, height;
  int pos;
  int cod = 1;
  byte *imagen;

  if ((imagen = (byte *)malloc(mapa->map_width * mapa->map_height)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

  memcpy(imagen, mapa->map, mapa->map_width * mapa->map_height);

  if (!select_file()) {
    free(imagen);
    return;
  }

  for (y = 0; y < mapa->map_height; y++) {
    for (x = 0; x < mapa->map_width; x++) {
      pos = y * mapa->map_width + x;

      if (imagen[pos]) {
        for (width = 0; width < mapa->map_width - x; width++) {
          if (!imagen[pos + width])
            break;
          imagen[pos + width] = 0;
        }

        imagen[pos] = 1;
        for (height = 0; height < mapa->map_height - y; height++) {
          if (!imagen[pos + height * mapa->map_width])
            break;

          imagen[pos + height * mapa->map_width] = 0;
        }

        if ((width - 2) * (height - 2) <= 0) {
          free(imagen);
          v_text = (char *)texts[551];
          show_dialog(err0);
          return;
        }

        get_map_graphic(mapa, imagen, x + 1, y + 1, width - 2, height - 2, cod++);
      }
    }
  }

  free(imagen);
}

void get_map_graphic(struct tmapa *mapa, byte *imagen, int x, int y, int width, int height,
                     int cod) {
  FPG *my_fpg = (FPG *)v.aux;
  int pos, index = 0;
  int scan_x, scan_y;
  byte *buffer;
  char str_desc[40];
  char str_file[13];

  if ((buffer = (byte *)malloc(width * height)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

  for (scan_y = 0; scan_y < height; scan_y++) {
    for (scan_x = 0; scan_x < width; scan_x++) {
      pos = (y + scan_y) * mapa->map_width + x + scan_x;
      buffer[index++] = imagen[pos];
      imagen[pos] = 0;
    }

    pos = (y + scan_y) * mapa->map_width + x + scan_x;
    imagen[pos] = 0;
  }

  pos = (y + scan_y) * mapa->map_width + x;
  memset(&imagen[pos], 0, width + 1);

  while (my_fpg->grf_offsets[cod])
    cod++;

  DIV_STRCPY(str_file, (char *)my_fpg->fpg_name);

  if (strchr(str_file, '.'))
    *strchr(str_file, '.') = '\0';

  if (strlen(str_file) > 5)
    str_file[5] = (char)0;

  { /* Overlapping sprintf fix: str_file is both dest and arg */
    char tmp[sizeof(str_file)];
    DIV_SPRINTF(tmp, "%s%d.MAP", strupr(str_file), cod);
    DIV_STRCPY(str_file, tmp);
  }
  DIV_SPRINTF(str_desc, "%s%d%s%s", texts[499], cod, texts[500], mapa->filename);

  fpg_add(my_fpg, cod, str_desc, str_file, width, height, 0, NULL, (char *)buffer, 0, 0);

  free(buffer);
}

//-----------------------------------------------------------------------------
//  Functions for exporting maps from FPG
//-----------------------------------------------------------------------------

struct {
  int x, y, w, h;
} lgraf[1000];

int lnum = 0;
int lmapan, lmapal;

void fpg_to_map(FPG *my_fpg) {
  int num;
  FILE *fpg;
  HeadFPG MiHeadFPG;
  byte *fpg_image;
  byte *fpg_map;

  lmapan = vga_width;
  lmapal = vga_height;

  if ((fpg = fopen((char *)my_fpg->current_file, "rb")) == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return;
  }

  num = 0;
  while (my_fpg->desc_index[num]) {
    fseek(fpg, my_fpg->grf_offsets[my_fpg->desc_index[num]], SEEK_SET);
    fseek(fpg, 52, SEEK_CUR);

    fread(&MiHeadFPG.width, 1, 4, fpg);
    fread(&MiHeadFPG.height, 1, 4, fpg);

    lgraf[num].w = MiHeadFPG.width + 2;
    lgraf[num].h = MiHeadFPG.height + 2;

    if (lmapan < lgraf[num].w)
      lmapan = lgraf[num].w;
    num++;
  }

  for (lnum = 0; lnum < num; lnum++) {
    place_map();

    if (lmapal < lgraf[lnum].y + lgraf[lnum].h) {
      lmapal = lgraf[lnum].y + lgraf[lnum].h;
    }
  }

  if ((fpg_map = (byte *)malloc(lmapan * lmapal)) == NULL) {
    fclose(fpg);
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

  memset(fpg_map, 0, lmapan * lmapal);

  num = 0;

  while (my_fpg->desc_index[num]) {
    fseek(fpg, my_fpg->grf_offsets[my_fpg->desc_index[num]], SEEK_SET);
    fseek(fpg, 60, SEEK_CUR);

    fread(&MiHeadFPG.num_points, 1, 4, fpg);

    if (MiHeadFPG.num_points != 0)
      fseek(fpg, MiHeadFPG.num_points * 4, SEEK_CUR);

    if ((fpg_image = (byte *)malloc((lgraf[num].w - 2) * (lgraf[num].h - 2))) == NULL) {
      fclose(fpg);
      free(fpg_map);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    if (fread(fpg_image, 1, (lgraf[num].w - 2) * (lgraf[num].h - 2), fpg) !=
        (lgraf[num].w - 2) * (lgraf[num].h - 2)) {
      fclose(fpg);
      free(fpg_map);
      free(fpg_image);
      v_text = (char *)texts[44];
      show_dialog(err0);
      return;
    }

    put_map_graphic(fpg_image, fpg_map, num);
    free(fpg_image);
    num++;
  }

  fclose(fpg);

  map_width = lmapan;
  map_height = lmapal;

  if (new_map(fpg_map)) {
    free(fpg_map);
    return;
  }
  call((void_return_type_t)v.paint_handler);
  v.redraw = 1;
}

void put_map_graphic(byte *imagen, byte *mapa, int num) {
  int x, y;
  int pos_im, pos_ma;

  // Top and bottom border lines
  memset(&mapa[lgraf[num].y * lmapan + lgraf[num].x], c4, lgraf[num].w);
  memset(&mapa[(lgraf[num].y + lgraf[num].h - 1) * lmapan + lgraf[num].x], c4, lgraf[num].w);

  for (y = 0; y < lgraf[num].h - 2; y++) {
    mapa[(y + lgraf[num].y + 1) * lmapan + lgraf[num].x] = c4; // Left column

    for (x = 0; x < lgraf[num].w - 2; x++) {
      pos_im = y * (lgraf[num].w - 2) + x;
      pos_ma = (y + lgraf[num].y + 1) * lmapan + x + lgraf[num].x + 1;
      mapa[pos_ma] = imagen[pos_im]; // Graphic data
    }

    mapa[(y + lgraf[num].y + 1) * lmapan + x + lgraf[num].x + 1] = c4; // Right column
  }
}

//-----------------------------------------------------------------------------
//      Placement algorithm for an image in the MAP
//-----------------------------------------------------------------------------

void place_map(void) { // Place graphic lnum

  int n, m, x, y, new_x;
  int scans, scan[1000];

  lgraf[lnum].x = 0;
  lgraf[lnum].y = lmapal;
  scan[0] = 0;
  scans = 1;

  // First create in scan[] a list of possible heights (0 + window bottoms)
  for (n = 0; n < lnum; n++) {
    y = lgraf[n].y + lgraf[n].h + 1;

    x = 0;

    do {
      x++;
    } while (x < scans && y > scan[x]);

    if (x == scans)
      scan[scans++] = y;
    else if (y != scan[x]) {
      memmove(&scan[x + 1], &scan[x], 4 * (1000 - x - 1));
      scan[x] = y;
      scans++;
    }
  }

  // Second ... placement algorithm ...
  for (n = 0; n < scans; n++) {
    y = scan[n];
    new_x = 0;
    do {
      x = new_x;
      for (m = 0; m < lnum; m++) {
        if (collides_with_map(m, x - 1, y - 1, lgraf[lnum].w + 2, lgraf[lnum].h + 2)) {
          if (lgraf[m].x + lgraf[m].w >= new_x)
            new_x = lgraf[m].x + lgraf[m].w + 1;
        }
      }
    } while (new_x != x && new_x + lgraf[lnum].w <= lmapan);

    if (new_x + lgraf[lnum].w <= lmapan) {
      lgraf[lnum].x = new_x;
      lgraf[lnum].y = y;
      break;
    }
  }
}

int collides_with_map(int n, int x, int y, int w, int h) {
  if (y < lgraf[n].y + lgraf[n].h && y + h > lgraf[n].y && x < lgraf[n].x + lgraf[n].w &&
      x + w > lgraf[n].x)
    return (1);
  else
    return (0);
}

//-----------------------------------------------------------------------------
//      Close a duplicate FPG
//-----------------------------------------------------------------------------

void close_fpg(char *fpg_path) {
  FPG *my_fpg;
  int m;

  for (m = 0; m < max_windows; m++) {
    my_fpg = (FPG *)window[m].aux;
    if (window[m].type == 101) {
      if (!strcmp(fpg_path, (char *)my_fpg->current_file)) {
        move(0, m);
        close_window();
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//      Select a file to open or overwrite
//-----------------------------------------------------------------------------

int select_file(void) {
  FPG *my_fpg;

  v_accept = 0;
  v_mode = 0;
  v_type = 4;
  v_text = (char *)texts[69];
  show_dialog(browser0);

  div_strcpy(full, sizeof(full), file_types[v_type].path);

  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");

  div_strcat(full, sizeof(full), input);

  if (v_finished) {
    if (!v_exists) {
      v_accept = 1;
    } else {
      v_title = (char *)texts[82];
      v_text = input;
      show_dialog(replace_fpg_0);
    }

    if (v_accept == 1) {
      v_aux = (byte *)malloc(sizeof(FPG));

      if (v_aux == NULL) {
        v_text = (char *)texts[45];
        show_dialog(err0);
        return 0;
      }
      close_fpg(full);
      memset(v_aux, 0, sizeof(FPG));
      new_window(fpg_dialog0_new);
    } else if (v_accept == 2) {
      v_aux = (byte *)malloc(sizeof(FPG));

      if (v_aux == NULL) {
        v_text = (char *)texts[45];
        show_dialog(err0);
        return 0;
      }

      my_fpg = (FPG *)v_aux;
      my_fpg->thumb_on = 0;
      my_fpg->fpg_info = 0;
      my_fpg->list_info.created = 0;
      close_fpg(full);
      memset(v_aux, 0, sizeof(FPG));
      new_window(fpg_dialog0_add);
    } else
      return 0;

    return 1;
  }
  return 0;
}

//-----------------------------------------------------------------------------
//      Replace or add to FPG
//-----------------------------------------------------------------------------

void replace_fpg_1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();

  if (v_text != NULL)
    wwrite(v.ptr, w, h, w / 2, 12, 1, (byte *)v_text, c3);
}

void replace_fpg_2(void) {
  _process_items();

  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    v_accept = 2;
    end_dialog = 1;
    break;
  case 2:
    v_accept = 3;
    end_dialog = 1;
    break;
  }
}

void replace_fpg_0(void) {
  int x2, x3;

  v.type = 1;
  v.w = text_len(texts[510]) + text_len(texts[511]) + text_len(texts[101]) + 16;
  x2 = 7 + text_len(texts[510] + 1) + 10;
  x3 = x2 + text_len(texts[511] + 1) + 10;

  if (v_title != NULL) {
    v.title = (byte *)v_title;
    if (text_len((byte *)v_title) + 14 > v.w)
      v.w = text_len((byte *)v_title) + 14;
  }

  if (v_text != NULL) {
    v.h = 38;
    if (text_len((byte *)v_text) + 6 > v.w)
      v.w = text_len((byte *)v_text) + 6;
  } else
    v.h = 29;

  v.paint_handler = replace_fpg_1;
  v.click_handler = replace_fpg_2;

  _button(510, 7, v.h - 14, 0);  // Replace
  _button(511, x2, v.h - 14, 0); // Add
  _button(101, x3, v.h - 14, 0); // Cancel

  v_accept = 3;
}
