

#include "global.h"
#include "fpg.h"
#include "div_string.h"

void fpg_edit_code_dialog(void);
extern int ret_value;
extern char code_str[5];
extern char file_str[13];
extern char description[33];
char *fpg_image = NULL;
short *fpg_points = NULL;

char newdac[PALETTE_SIZE];
int new_dac_loaded = 0;

int get_code_width;
int get_code_height;
char *get_code_image;
short get_code_p0x;
short get_code_p0y;

#define ancho_br 155
#define alto_br  72

void fpg_read_image_header(HeadFPG *fpg_header, FILE *fpg) {
  if (fread(&fpg_header->code, 1, 4, fpg) != 4 || fread(&fpg_header->length, 1, 4, fpg) != 4) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }
  memset(fpg_header->description, 0, 33);
  fread(fpg_header->description, 1, 32, fpg);
  fread(fpg_header->filename, 1, 12, fpg);
  if (fread(&fpg_header->width, 1, 4, fpg) != 4 || fread(&fpg_header->height, 1, 4, fpg) != 4 ||
      fread(&fpg_header->num_points, 1, 4, fpg) != 4) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }

  if (!validate_image_dimensions(fpg_header->width, fpg_header->height)) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }
  if (fpg_header->num_points < 0 || fpg_header->num_points > 256) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }

  fpg_image = (char *)malloc(fpg_header->width * fpg_header->height);
  if (fpg_image == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  if (fpg_header->num_points != 0) {
    fpg_points = (short *)malloc(fpg_header->num_points * 4);
    if (fpg_points == NULL) {
      free(fpg_image);
      fpg_image = NULL;
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    fread(fpg_points, fpg_header->num_points * 2, 2, fpg);
  } else
    fpg_points = NULL;
  fread(fpg_image, fpg_header->width * fpg_header->height, 1, fpg);
}

void fpg_create(FPG *fpg_file, char *name) {
  int x;
  FILE *fpg;
  if ((fpg = fopen(name, "wb")) == NULL)
    return;
  for (x = 0; x < MAX_FPG_GRAPHICS; x++) {
    fpg_file->grf_offsets[x] = 0;
    fpg_file->desc_index[x] = 0;
  }

  fpg_file->nIndex = 0;
  fpg_file->last_used = 0;

  fpg_file->version = 0; // 32 bit fpg files

  div_strcpy((char *)fpg_file->current_file, sizeof(fpg_file->current_file), name);
  fwrite("fpg\x1a\x0d\x0a\x00", 7, 1, fpg);

  fwrite(&fpg_file->version, 1, 1, fpg);

  fwrite(dac, PALETTE_SIZE, 1, fpg);

  fwrite(gradients, sizeof(gradients), 1, fpg);
  fclose(fpg);

  if (fpg_file->thumb_on) {
    fpg_file->list_info.columns = 3;
    fpg_file->list_info.lines = 2;
    fpg_file->list_info.w = 47;
    fpg_file->list_info.h = 26;
  } else {
    fpg_file->list_info.columns = 1;
    fpg_file->list_info.lines = 6;
    fpg_file->list_info.w = 143;
    fpg_file->list_info.h = 8;
  }

  fpg_file->list_info.x = 3;
  fpg_file->list_info.y = 11;
  fpg_file->list_info.list = (char *)fpg_file->code_desc;
  fpg_file->list_info.total_items = fpg_file->nIndex;
  fpg_file->list_info.item_width = 38 + 2;
}

int fpg_open(FPG *fpg_file, char *name) {
  int x;
  FILE *fpg;
  HeadFPG kkhead;
  char buffer[PALETTE_SIZE];

  if ((fpg = fopen(name, "rb")) == NULL) {
    return 0;
  }

  fread(buffer, 7, 1, fpg);

  if (strcmp(buffer, "fpg\x1a\x0d\x0a")) {
    fclose(fpg);
    return 0;
  }

  fread(&fpg_file->version, 1, 1, fpg);
#ifdef DEBUG
  printf("FPG version: %d\n", fpg_file->version);
#endif
  for (x = 0; x < MAX_FPG_GRAPHICS; x++) {
    fpg_file->grf_offsets[x] = 0;
    fpg_file->desc_index[x] = 0;
  }

  fpg_file->nIndex = 0;
  div_strcpy((char *)fpg_file->current_file, sizeof(fpg_file->current_file), name);
  if (fread(newdac, PALETTE_SIZE, 1, fpg) != 1) {
    fclose(fpg);
    return 0;
  }
  memcpy(dac4, newdac, PALETTE_SIZE);
  new_dac_loaded = 1;
  fread((byte *)gradients, 1, sizeof(gradients), fpg);

  while (fpg_read_header(&kkhead, fpg)) {
    if (kkhead.code < 0 || kkhead.code >= MAX_FPG_GRAPHICS) {
      debugprintf("FPG: skipping graphic with out-of-range code %d\n", kkhead.code);
      fseek(fpg, kkhead.length - FPG_HEAD, SEEK_CUR);
      continue;
    }
    if (fpg_file->nIndex >= MAX_FPG_GRAPHICS) {
      debugprintf("FPG: too many graphics (>=%d), stopping\n", MAX_FPG_GRAPHICS);
      break;
    }
    fpg_file->grf_offsets[kkhead.code] = ftell(fpg) - FPG_HEAD;
    fpg_file->desc_index[fpg_file->nIndex] = kkhead.code;
    div_snprintf((char *)fpg_file->code_desc[fpg_file->nIndex++], sizeof(fpg_file->code_desc[0]), "%c %03d %s",
                 255, kkhead.code, kkhead.description);
    fseek(fpg, fpg_file->grf_offsets[kkhead.code] + kkhead.length, SEEK_SET);
  }

  fpg_sort(fpg_file);
  fclose(fpg);

  if (fpg_file->thumb_on) {
    fpg_file->list_info.columns = 3;
    fpg_file->list_info.lines = 2;
    fpg_file->list_info.w = 47;
    fpg_file->list_info.h = 26;
  } else {
    fpg_file->list_info.columns = 1;
    fpg_file->list_info.lines = 6;
    fpg_file->list_info.w = 143;
    fpg_file->list_info.h = 8;
  }

  fpg_file->list_info.x = 3;
  fpg_file->list_info.y = 11;
  fpg_file->list_info.list = (char *)fpg_file->code_desc;
  fpg_file->list_info.total_items = fpg_file->nIndex;
  fpg_file->list_info.item_width = 38 + 2;

  return 1;
}

void fpg_sort(FPG *fpg_file) {
  int iWork, i, j;
  char cWork[38];
  for (i = 0; i < fpg_file->nIndex; i++) {
    for (j = 0; j < fpg_file->nIndex - 1; j++) {
      if (fpg_file->desc_index[j] > fpg_file->desc_index[j + 1]) {
        iWork = fpg_file->desc_index[j];
        fpg_file->desc_index[j] = fpg_file->desc_index[j + 1];
        fpg_file->desc_index[j + 1] = iWork;

        div_strcpy(cWork, sizeof(cWork), (char *)fpg_file->code_desc[j]);
        div_strcpy((char *)fpg_file->code_desc[j], sizeof(fpg_file->code_desc[j]),
                   (char *)fpg_file->code_desc[j + 1]);
        div_strcpy((char *)fpg_file->code_desc[j + 1], sizeof(fpg_file->code_desc[j + 1]), cWork);
      }
    }
  }
}

int fpg_read_header(HeadFPG *fpg_header, FILE *fpg) {
  if (fread(&fpg_header->code, 1, 4, fpg) != 4 || fread(&fpg_header->length, 1, 4, fpg) != 4)
    return 0;
  memset(fpg_header->description, 0, 33);
  fread(fpg_header->description, 1, 32, fpg);
  fread(fpg_header->filename, 1, 12, fpg);
  if (fread(&fpg_header->width, 1, 4, fpg) != 4 || fread(&fpg_header->height, 1, 4, fpg) != 4 ||
      fread(&fpg_header->num_points, 1, 4, fpg) != 4)
    return 0;
  return 1;
}

void fpg_write_header(HeadFPG *fpg_header, short *points, char *image, FILE *fpg) {
  fwrite(&fpg_header->code, 1, 4, fpg);
  fwrite(&fpg_header->length, 1, 4, fpg);
  fwrite(fpg_header->description, 1, 32, fpg);
  fwrite(fpg_header->filename, 1, 12, fpg);
  fwrite(&fpg_header->width, 1, 4, fpg);
  fwrite(&fpg_header->height, 1, 4, fpg);
  fwrite(&fpg_header->num_points, 1, 4, fpg);
  if (fpg_header->num_points != 0)
    fwrite(points, fpg_header->num_points * 4, 1, fpg);
  fwrite(image, fpg_header->width * fpg_header->height, 1, fpg);
}

int fpg_add(FPG *fpg_file, int cod, char *desc, char *filename, int width, int height, int num_points,
            char *points, char *img_data, int delete_old, int get_info) {
  int length, old_cod = cod, first = 1, n;
  FILE *fpg;

  while ((cod == 0) || first) {
    first = 0;
    div_snprintf(code_str, sizeof(code_str), "%d", cod);
    memcpy(description, desc, 32);
    memcpy(file_str, filename, 12);
    file_str[12] = (char)0;
    ret_value = 0;
    get_code_width = width;
    get_code_height = height;
    get_code_image = img_data;
    if (num_points != 0) {
      get_code_p0x = *((short *)points);
      get_code_p0y = *((short *)(points + 2));
    } else {
      get_code_p0x = -1;
      get_code_p0y = -1;
    }
    if (get_info)
      fpg_edit_code_dialog();
    else
      ret_value = 1;
    cod = atoi(code_str);
    fpg_file->last_used = cod;
    div_snprintf(code_str, sizeof(code_str), "%d", cod);

    //**********************************************!!!!!!!!!!!!!!!!!!!!!!!
    //                memcpy(description,description,32);
    //**********************************************!!!!!!!!!!!!!!!!!!!!!!!

    if (!ret_value)
      return 1;
  }

  if (delete_old)
    fpg_delete(fpg_file, old_cod); // Delete the old one

  if (fpg_file->grf_offsets[cod] != 0)
    fpg_delete(fpg_file, cod); // If overwriting an existing one

  if ((fpg = fopen((char *)fpg_file->current_file, "ab")) == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return 0;
  }

  //cod

  n = 0;
  while (1) {
    if (fpg_file->desc_index[n] > cod || fpg_file->desc_index[n] == 0)
      break;
    n++;
  }

  memmove(&(fpg_file->thumb[n + 1]), &(fpg_file->thumb[n]), sizeof(t_thumb) * (MAX_FPG_GRAPHICS - 1 - n));

  fpg_file->thumb[n].ptr = NULL;
  fpg_file->thumb[n].status = 0;

  fseek(fpg, 0, SEEK_END);
  fpg_file->grf_offsets[cod] = ftell(fpg);
  length = FPG_HEAD + (num_points * 4) + width * height;
  fwrite(&cod, 1, 4, fpg);
  fwrite(&length, 1, 4, fpg);
  fwrite(description, 1, 32, fpg);
  fwrite(file_str, 1, 12, fpg);
  fwrite(&width, 1, 4, fpg);
  fwrite(&height, 1, 4, fpg);
  fwrite(&num_points, 1, 4, fpg);
  if (num_points != 0)
    fwrite(points, num_points * 4, 1, fpg);
  fwrite((byte *)img_data, width * height, 1, fpg);
  fclose(fpg);

  // Re-read file information
  if (!fpg_open(fpg_file, (char *)fpg_file->current_file)) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return 0;
  }

  //*******************************************************

  while (fpg_file->list_info.first_visible + (fpg_file->list_info.lines - 1) * fpg_file->list_info.columns + 1 >
         fpg_file->list_info.total_items) {
    fpg_file->list_info.first_visible -= fpg_file->list_info.columns;
  }

  if (fpg_file->list_info.first_visible < 0)
    fpg_file->list_info.first_visible = 0;

  wmouse_x = -1;
  wmouse_y = -1;
  FPG_update_listbox_br(&fpg_file->list_info);
  call(v.paint_handler);

  //        paint_listbox(&fpg_file->list_info);

  //*******************************************************
  v.redraw = 1;

  return 1;
}

int fpg_remap_to_pal(FPG *fpg_file) {
  struct gradient_rule CopiaReglas[16];
  HeadFPG other_header;
  char ActualPath[_MAX_PATH + 14];
  char *other_image;
  short *other_points = NULL;
  byte tmp[PALETTE_SIZE];
  int x;
  FILE *fpg;
  FILE *Oldfpg;
  int y;
  byte color_lut[256];

  // Temporary file name
  div_strcpy(ActualPath, sizeof(ActualPath), (char *)fpg_file->current_file);
  for (x = strlen(ActualPath); x >= 0; x--)
    if (IS_PATH_SEP(ActualPath[x]))
      x = -1;
    else
      ActualPath[x] = 0;
  div_strcat(ActualPath, sizeof(ActualPath), "_DIV_.FPG");
  delete_file(ActualPath);

  if ((fpg = fopen((char *)fpg_file->current_file, "rb")) == NULL)
    return 0;
  if ((Oldfpg = fopen((char *)ActualPath, "wb")) == NULL) {
    fclose(fpg);
    return 0;
  }

  // Copy graphic header
  fread(tmp, 8, 1, fpg);
  fwrite(tmp, 8, 1, Oldfpg);
  fread(tmp, PALETTE_SIZE, 1, fpg);
  create_dac4();
  for (x = 0; x < 256; x++)
    color_lut[x] = find_color(tmp[x * 3], tmp[x * 3 + 1], tmp[x * 3 + 2]);
  fwrite(dac, PALETTE_SIZE, 1, Oldfpg);
  fread(CopiaReglas, 1, sizeof(CopiaReglas), fpg);
  fwrite(CopiaReglas, sizeof(CopiaReglas), 1, Oldfpg);
  while (fpg_read_header(&other_header, fpg)) {
    if (!validate_image_dimensions(other_header.width, other_header.height) ||
        other_header.num_points < 0 || other_header.num_points > 256) {
      fclose(fpg);
      fclose(Oldfpg);
      return 0;
    }
    other_image = (char *)malloc(other_header.width * other_header.height);
    if (other_image == NULL) {
      fclose(fpg);
      fclose(Oldfpg);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return 0;
    }
    // Check memory
    if (other_header.num_points != 0) {
      other_points = (short *)malloc(other_header.num_points * 4);
      if (other_points == NULL) {
        fclose(fpg);
        fclose(Oldfpg);
        free(other_image);
        v_text = (char *)texts[45];
        show_dialog(err0);
        return 0;
      }
      fread(other_points, other_header.num_points * 2, 2, fpg);
    }
    fread(other_image, other_header.width * other_header.height, 1, fpg);
    for (y = 0; y < other_header.width * other_header.height; y++)
      other_image[y] = color_lut[other_image[y]];

    fpg_write_header(&other_header, other_points, other_image, Oldfpg);

    free(other_image);
    if (other_header.num_points != 0)
      free(other_points);
  }
  fclose(Oldfpg);
  fclose(fpg);
  delete_file((char *)fpg_file->current_file);
  rename(ActualPath, (char *)fpg_file->current_file);
  if (!fpg_open(fpg_file, (char *)fpg_file->current_file))
    return 0;
  new_dac_loaded = 0;
  return 1;
}
#define BUFFERCOPYLEN 4096
void close_fpg(char *fpg_path);
void fpg_save(int n) {
  int w = window[n].w / big2, h = window[n].h / big2;
  FPG *fpg_file = (FPG *)window[n].aux;
  FILE *FileOrg, *FileDest;
  int Lengt;
  char *Buffer;
  Buffer = (char *)malloc(BUFFERCOPYLEN);
  if (Buffer == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  div_strcpy(full, sizeof(full), file_types[FT_FPG].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  FileOrg = fopen((char *)fpg_file->current_file, "rb");
  if (FileOrg == NULL) {
    free(Buffer);
    return;
  }

  FileDest = fopen(full, "wb");
  if (FileDest == NULL) {
    free(Buffer);
    fclose(FileOrg);
    return;
  }
  fseek(FileOrg, 0, SEEK_END);
  Lengt = ftell(FileOrg);
  fseek(FileOrg, 0, SEEK_SET);
  while (Lengt) {
    if (Lengt >= BUFFERCOPYLEN) {
      fread(Buffer, BUFFERCOPYLEN, 1, FileOrg);
      fwrite(Buffer, BUFFERCOPYLEN, 1, FileDest);
      Lengt -= BUFFERCOPYLEN;
    } else {
      fread(Buffer, Lengt, 1, FileOrg);
      fwrite(Buffer, Lengt, 1, FileDest);
      Lengt = 0;
    }
  }

  close_fpg(full);

  div_strcpy((char *)fpg_file->current_file, sizeof(fpg_file->current_file), full);
  div_strcpy((char *)fpg_file->fpg_name, sizeof(fpg_file->fpg_name), input);

  wgra(window[n].ptr, w, h, c_b_low, 2, 2, w - 20, 7);
  if (text_len(window[n].title) + 3 > w - 20) {
    wwrite_in_box(window[n].ptr, w, w - 19, h, 4, 2, 0, window[n].title, c1);
    wwrite_in_box(window[n].ptr, w, w - 19, h, 3, 2, 0, window[n].title, c4);
  } else {
    wwrite(window[n].ptr, w, h, 3 + (w - 20) / 2, 2, 1, window[n].title, c1);
    wwrite(window[n].ptr, w, h, 2 + (w - 20) / 2, 2, 1, window[n].title, c4);
  }
  flush_window(n);

  fclose(FileOrg);
  fclose(FileDest);
  free(Buffer);
}

int fpg_delete(FPG *fpg_file,
               int cod) // Trash, overwrite existing entry, or when changing a graphic's code
{
  struct gradient_rule CopiaReglas[16];
  HeadFPG other_header;
  char ActualPath[_MAX_PATH + 14];
  char *other_image;
  short *other_points = NULL;
  byte tmp[PALETTE_SIZE];
  int x, len, n;
  FILE *fpg;
  FILE *Oldfpg;

  debugprintf("Deleting map %d\n", cod);

  // Temporary file name
  div_strcpy(ActualPath, sizeof(ActualPath), (char *)fpg_file->current_file);
  for (x = strlen(ActualPath); x >= 0; x--)
    if (IS_PATH_SEP(ActualPath[x]))
      x = -1;
    else
      ActualPath[x] = 0;
  div_strcat(ActualPath, sizeof(ActualPath), "_DIV_.FPG");
  delete_file(ActualPath);

  if ((fpg = fopen((char *)fpg_file->current_file, "rb")) == NULL)
    return 0;
  if ((Oldfpg = fopen(ActualPath, "wb")) == NULL) {
    fclose(fpg);
    return 0;
  }

  n = 0;
  while (1) {
    if (fpg_file->desc_index[n] == cod || fpg_file->desc_index[n] == 0)
      break;
    n++;
  }
  debugprintf("found cod at index: %d\n", n);

  if (fpg_file->thumb[n].ptr != NULL)
    free(fpg_file->thumb[n].ptr);
  memmove(&(fpg_file->thumb[n]), &(fpg_file->thumb[n + 1]), sizeof(t_thumb) * (MAX_FPG_GRAPHICS - 1 - n));

  mouse_graf = CURSOR_BUSY;
  fseek(fpg, 0, SEEK_END);
  len = ftell(fpg);
  fseek(fpg, 0, SEEK_SET);
  show_progress((char *)texts[436], 0, len);

  // Copy graphic header
  fread(tmp, 8, 1, fpg);
  fwrite(tmp, 8, 1, Oldfpg);
  fread(tmp, PALETTE_SIZE, 1, fpg);
  fwrite(tmp, PALETTE_SIZE, 1, Oldfpg);
  fread(CopiaReglas, 1, sizeof(CopiaReglas), fpg);
  fwrite(CopiaReglas, sizeof(CopiaReglas), 1, Oldfpg);

  while (fpg_read_header(&other_header, fpg)) {
    show_progress((char *)texts[436], ftell(fpg), len);

    if (!validate_image_dimensions(other_header.width, other_header.height) ||
        other_header.num_points < 0 || other_header.num_points > 256) {
      show_progress((char *)texts[436], len, len);
      fclose(fpg);
      fclose(Oldfpg);
      return 0;
    }
    other_image = (char *)malloc(other_header.width * other_header.height);
    if (other_image == NULL) {
      show_progress((char *)texts[436], len, len);
      fclose(fpg);
      fclose(Oldfpg);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return 0;
    }
    // Check memory
    if (other_header.num_points != 0) {
      other_points = (short *)malloc(other_header.num_points * 4);
      if (other_points == NULL) {
        show_progress((char *)texts[436], len, len);
        fclose(fpg);
        fclose(Oldfpg);
        free(other_image);
        v_text = (char *)texts[45];
        show_dialog(err0);
        return 0;
      }
      fread(other_points, other_header.num_points * 2, 2, fpg);
    }
    fread(other_image, other_header.width * other_header.height, 1, fpg);

    // *************************

    if (other_header.code != cod) {
      fpg_write_header(&other_header, other_points, other_image, Oldfpg);
    }

    // *************************

    free(other_image);

    if (other_header.num_points != 0)
      free(other_points);
  }
  fclose(Oldfpg);
  fclose(fpg);

  show_progress((char *)texts[436], len, len);

  delete_file((char *)fpg_file->current_file);

  debugprintf("MOVE %s to %s\n", ActualPath, (char *)fpg_file->current_file);


  rename(ActualPath, (char *)fpg_file->current_file);

  if (!fpg_open(fpg_file, (char *)fpg_file->current_file)) {
    return 0;
  }
  return 1;
}

int fpg_delete_many(FPG *fpg_file, int taggeds, int *array_del) {
  struct gradient_rule CopiaReglas[16];
  HeadFPG other_header;
  char ActualPath[_MAX_PATH + 14];
  char *other_image;
  short *other_points = NULL;
  byte tmp[PALETTE_SIZE];
  int x, y, len, n;
  FILE *fpg;
  FILE *Oldfpg;

  div_strcpy(ActualPath, sizeof(ActualPath), (char *)fpg_file->current_file); // Temporary file name
  for (x = strlen(ActualPath); x >= 0; x--)
    if (IS_PATH_SEP(ActualPath[x]))
      x = -1;
    else
      ActualPath[x] = 0;
  div_strcat(ActualPath, sizeof(ActualPath), "_DIV_.FPG");
  delete_file(ActualPath);

  if ((fpg = fopen((char *)fpg_file->current_file, "rb")) == NULL)
    return 0;
  if ((Oldfpg = fopen(ActualPath, "wb")) == NULL) {
    fclose(fpg);
    return 0;
  }

  mouse_graf = CURSOR_BUSY;
  fseek(fpg, 0, SEEK_END);
  len = ftell(fpg);
  fseek(fpg, 0, SEEK_SET);
  show_progress((char *)texts[436], 0, len);

  fread(tmp, 8, 1, fpg); // Copy graphic header
  fwrite(tmp, 8, 1, Oldfpg);
  fread(tmp, PALETTE_SIZE, 1, fpg);
  fwrite(tmp, PALETTE_SIZE, 1, Oldfpg);
  fread(CopiaReglas, 1, sizeof(CopiaReglas), fpg);
  fwrite(CopiaReglas, sizeof(CopiaReglas), 1, Oldfpg);

  while (fpg_read_header(&other_header, fpg)) {
    show_progress((char *)texts[436], ftell(fpg), len);

    if (!validate_image_dimensions(other_header.width, other_header.height) ||
        other_header.num_points < 0 || other_header.num_points > 256) {
      show_progress((char *)texts[436], len, len);
      fclose(fpg);
      fclose(Oldfpg);
      return 0;
    }
    other_image = (char *)malloc(other_header.width * other_header.height);

    if (other_image == NULL) {
      show_progress((char *)texts[436], len, len);
      fclose(fpg);
      fclose(Oldfpg);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return 0;
    }

    if (other_header.num_points != 0) { // Check memory
      other_points = (short *)malloc(other_header.num_points * 4);
      if (other_points == NULL) {
        show_progress((char *)texts[436], len, len);
        fclose(fpg);
        fclose(Oldfpg);
        free(other_image);
        v_text = (char *)texts[45];
        show_dialog(err0);
        return 0;
      }
      fread(other_points, other_header.num_points * 2, 2, fpg);
    }

    fread(other_image, other_header.width * other_header.height, 1, fpg);

    // *************************

    for (y = 0; y < taggeds; y++) {
      if (array_del[y] == other_header.code)
        break;
    }

    if (y == taggeds) {
      fpg_write_header(&other_header, other_points, other_image, Oldfpg);
    } else {
      n = 0;
      while (1) {
        if (fpg_file->desc_index[n] == other_header.code || fpg_file->desc_index[n] == 0)
          break;
        n++;
      }

      if (fpg_file->thumb[n].ptr != NULL)
        free(fpg_file->thumb[n].ptr);
      memmove(&(fpg_file->thumb[n]), &(fpg_file->thumb[n + 1]), sizeof(t_thumb) * (MAX_FPG_GRAPHICS - 1 - n));

      memmove(&(fpg_file->desc_index[n]), &(fpg_file->desc_index[n + 1]), sizeof(int) * (MAX_FPG_GRAPHICS - 1 - n));
    }

    // *************************

    free(other_image);

    if (other_header.num_points != 0)
      free(other_points);
  }

  fclose(Oldfpg);
  fclose(fpg);

  show_progress((char *)texts[436], len, len);

  delete_file((char *)fpg_file->current_file);
  rename(ActualPath, (char *)fpg_file->current_file);

  if (!fpg_open(fpg_file, (char *)fpg_file->current_file))
    return 0;
  return 1;
}
