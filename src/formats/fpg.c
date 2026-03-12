

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

char newdac[768];
int new_dac_loaded = 0;

int get_code_width;
int get_code_height;
char *get_code_image;
short get_code_p0x;
short get_code_p0y;

#define ancho_br 155
#define alto_br  72

void fpg_read_image_header(HeadFPG *MiHeadFPG, FILE *fpg) {
  if (fread(&MiHeadFPG->code, 1, 4, fpg) != 4 ||
      fread(&MiHeadFPG->length, 1, 4, fpg) != 4) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }
  memset(MiHeadFPG->description, 0, 33);
  fread(MiHeadFPG->description, 1, 32, fpg);
  fread(MiHeadFPG->Filename, 1, 12, fpg);
  if (fread(&MiHeadFPG->width, 1, 4, fpg) != 4 ||
      fread(&MiHeadFPG->height, 1, 4, fpg) != 4 ||
      fread(&MiHeadFPG->num_points, 1, 4, fpg) != 4) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }

  if (!validate_image_dimensions(MiHeadFPG->width, MiHeadFPG->height)) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }
  if (MiHeadFPG->num_points < 0 || MiHeadFPG->num_points > 256) {
    fpg_image = NULL;
    fpg_points = NULL;
    return;
  }

  fpg_image = (char *)malloc(MiHeadFPG->width * MiHeadFPG->height);
  if (fpg_image == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  if (MiHeadFPG->num_points != 0) {
    fpg_points = (short *)malloc(MiHeadFPG->num_points * 4);
    if (fpg_points == NULL) {
      free(fpg_image);
      fpg_image = NULL;
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    fread(fpg_points, MiHeadFPG->num_points * 2, 2, fpg);
  } else
    fpg_points = NULL;
  fread(fpg_image, MiHeadFPG->width * MiHeadFPG->height, 1, fpg);
}

void fpg_create(FPG *Fpg, char *Name) {
  int x;
  FILE *fpg;
  if ((fpg = fopen(Name, "wb")) == NULL)
    return;
  for (x = 0; x < 1000; x++) {
    Fpg->grf_offsets[x] = 0;
    Fpg->desc_index[x] = 0;
  }

  Fpg->nIndex = 0;
  Fpg->last_used = 0;

  Fpg->version = 0; // 32 bit fpg files

  div_strcpy((char *)Fpg->current_file, sizeof(Fpg->current_file), Name);
  fwrite("fpg\x1a\x0d\x0a\x00", 7, 1, fpg);

  fwrite(&Fpg->version, 1, 1, fpg);

  fwrite(dac, 768, 1, fpg);

  fwrite(gradients, sizeof(gradients), 1, fpg);
  fclose(fpg);

  if (Fpg->thumb_on) {
    Fpg->list_info.columns = 3;
    Fpg->list_info.lines = 2;
    Fpg->list_info.w = 47;
    Fpg->list_info.h = 26;
  } else {
    Fpg->list_info.columns = 1;
    Fpg->list_info.lines = 6;
    Fpg->list_info.w = 143;
    Fpg->list_info.h = 8;
  }

  Fpg->list_info.x = 3;
  Fpg->list_info.y = 11;
  Fpg->list_info.list = (char *)Fpg->code_desc;
  Fpg->list_info.total_items = Fpg->nIndex;
  Fpg->list_info.item_width = 38 + 2;
}

int fpg_open(FPG *Fpg, char *Name) {
  int x;
  FILE *fpg;
  HeadFPG kkhead;
  char buffer[768];

  if ((fpg = fopen(Name, "rb")) == NULL) {
    return 0;
  }

  fread(buffer, 7, 1, fpg);

  if (strcmp(buffer, "fpg\x1a\x0d\x0a")) {
    fclose(fpg);
    return 0;
  }

  fread(&Fpg->version, 1, 1, fpg);
#ifdef DEBUG
  printf("FPG version: %d\n", Fpg->version);
#endif
  for (x = 0; x < 1000; x++) {
    Fpg->grf_offsets[x] = 0;
    Fpg->desc_index[x] = 0;
  }

  Fpg->nIndex = 0;
  div_strcpy((char *)Fpg->current_file, sizeof(Fpg->current_file), Name);
  if (fread(newdac, 768, 1, fpg) != 1) {
    fclose(fpg);
    return 0;
  }
  memcpy(dac4, newdac, 768);
  new_dac_loaded = 1;
  fread((byte *)gradients, 1, sizeof(gradients), fpg);

  while (fpg_read_header(&kkhead, fpg)) {
    if (kkhead.code < 0 || kkhead.code >= 1000) {
      debugprintf("FPG: skipping graphic with out-of-range code %d\n", kkhead.code);
      fseek(fpg, kkhead.length - FPG_HEAD, SEEK_CUR);
      continue;
    }
    if (Fpg->nIndex >= 1000) {
      debugprintf("FPG: too many graphics (>1000), stopping\n");
      break;
    }
    Fpg->grf_offsets[kkhead.code] = ftell(fpg) - FPG_HEAD;
    Fpg->desc_index[Fpg->nIndex] = kkhead.code;
    div_snprintf((char *)Fpg->code_desc[Fpg->nIndex++], sizeof(Fpg->code_desc[0]), "%c %03d %s",
                 255, kkhead.code, kkhead.description);
    fseek(fpg, Fpg->grf_offsets[kkhead.code] + kkhead.length, SEEK_SET);
  }

  fpg_sort(Fpg);
  fclose(fpg);

  if (Fpg->thumb_on) {
    Fpg->list_info.columns = 3;
    Fpg->list_info.lines = 2;
    Fpg->list_info.w = 47;
    Fpg->list_info.h = 26;
  } else {
    Fpg->list_info.columns = 1;
    Fpg->list_info.lines = 6;
    Fpg->list_info.w = 143;
    Fpg->list_info.h = 8;
  }

  Fpg->list_info.x = 3;
  Fpg->list_info.y = 11;
  Fpg->list_info.list = (char *)Fpg->code_desc;
  Fpg->list_info.total_items = Fpg->nIndex;
  Fpg->list_info.item_width = 38 + 2;

  return 1;
}

void fpg_sort(FPG *Fpg) {
  int iWork, i, j;
  char cWork[38];
  for (i = 0; i < Fpg->nIndex; i++) {
    for (j = 0; j < Fpg->nIndex - 1; j++) {
      if (Fpg->desc_index[j] > Fpg->desc_index[j + 1]) {
        iWork = Fpg->desc_index[j];
        Fpg->desc_index[j] = Fpg->desc_index[j + 1];
        Fpg->desc_index[j + 1] = iWork;

        div_strcpy(cWork, sizeof(cWork), (char *)Fpg->code_desc[j]);
        div_strcpy((char *)Fpg->code_desc[j], sizeof(Fpg->code_desc[j]),
                   (char *)Fpg->code_desc[j + 1]);
        div_strcpy((char *)Fpg->code_desc[j + 1], sizeof(Fpg->code_desc[j + 1]), cWork);
      }
    }
  }
}

int fpg_read_header(HeadFPG *MiHeadFPG, FILE *fpg) {
  if (fread(&MiHeadFPG->code, 1, 4, fpg) != 4 ||
      fread(&MiHeadFPG->length, 1, 4, fpg) != 4)
    return 0;
  memset(MiHeadFPG->description, 0, 33);
  fread(MiHeadFPG->description, 1, 32, fpg);
  fread(MiHeadFPG->Filename, 1, 12, fpg);
  if (fread(&MiHeadFPG->width, 1, 4, fpg) != 4 ||
      fread(&MiHeadFPG->height, 1, 4, fpg) != 4 ||
      fread(&MiHeadFPG->num_points, 1, 4, fpg) != 4)
    return 0;
  return 1;
}

void fpg_write_header(HeadFPG *MiHeadFPG, short *points, char *imagen, FILE *fpg) {
  fwrite(&MiHeadFPG->code, 1, 4, fpg);
  fwrite(&MiHeadFPG->length, 1, 4, fpg);
  fwrite(MiHeadFPG->description, 1, 32, fpg);
  fwrite(MiHeadFPG->Filename, 1, 12, fpg);
  fwrite(&MiHeadFPG->width, 1, 4, fpg);
  fwrite(&MiHeadFPG->height, 1, 4, fpg);
  fwrite(&MiHeadFPG->num_points, 1, 4, fpg);
  if (MiHeadFPG->num_points != 0)
    fwrite(points, MiHeadFPG->num_points * 4, 1, fpg);
  fwrite(imagen, MiHeadFPG->width * MiHeadFPG->height, 1, fpg);
}

int fpg_add(FPG *Fpg, int COD, char *desc, char *filename, int Ancho, int Alto, int num_points,
            char *points, char *Imagen, int delete_old, int get_info) {
  int LONG, OLDCOD = COD, First = 1, n;
  FILE *fpg;

  while ((COD == 0) || First) {
    First = 0;
    div_snprintf(code_str, sizeof(code_str), "%d", COD);
    memcpy(description, desc, 32);
    memcpy(file_str, filename, 12);
    file_str[12] = (char)0;
    ret_value = 0;
    get_code_width = Ancho;
    get_code_height = Alto;
    get_code_image = Imagen;
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
    COD = atoi(code_str);
    Fpg->last_used = COD;
    div_snprintf(code_str, sizeof(code_str), "%d", COD);

    //**********************************************!!!!!!!!!!!!!!!!!!!!!!!
    //                memcpy(description,description,32);
    //**********************************************!!!!!!!!!!!!!!!!!!!!!!!

    if (!ret_value)
      return 1;
  }

  if (delete_old)
    fpg_delete(Fpg, OLDCOD); // Delete the old one

  if (Fpg->grf_offsets[COD] != 0)
    fpg_delete(Fpg, COD); // If overwriting an existing one

  if ((fpg = fopen((char *)Fpg->current_file, "ab")) == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return 0;
  }

  //COD

  n = 0;
  while (1) {
    if (Fpg->desc_index[n] > COD || Fpg->desc_index[n] == 0)
      break;
    n++;
  }

  memmove(&(Fpg->thumb[n + 1]), &(Fpg->thumb[n]), sizeof(t_thumb) * (999 - n));

  Fpg->thumb[n].ptr = NULL;
  Fpg->thumb[n].status = 0;

  fseek(fpg, 0, SEEK_END);
  Fpg->grf_offsets[COD] = ftell(fpg);
  LONG = FPG_HEAD + (num_points * 4) + Ancho * Alto;
  fwrite(&COD, 1, 4, fpg);
  fwrite(&LONG, 1, 4, fpg);
  fwrite(description, 1, 32, fpg);
  fwrite(file_str, 1, 12, fpg);
  fwrite(&Ancho, 1, 4, fpg);
  fwrite(&Alto, 1, 4, fpg);
  fwrite(&num_points, 1, 4, fpg);
  if (num_points != 0)
    fwrite(points, num_points * 4, 1, fpg);
  fwrite((byte *)Imagen, Ancho * Alto, 1, fpg);
  fclose(fpg);

  // Re-read file information
  if (!fpg_open(Fpg, (char *)Fpg->current_file)) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return 0;
  }

  //*******************************************************

  while (Fpg->list_info.first_visible + (Fpg->list_info.lines - 1) * Fpg->list_info.columns + 1 >
         Fpg->list_info.total_items) {
    Fpg->list_info.first_visible -= Fpg->list_info.columns;
  }

  if (Fpg->list_info.first_visible < 0)
    Fpg->list_info.first_visible = 0;

  wmouse_x = -1;
  wmouse_y = -1;
  FPG_update_listbox_br(&Fpg->list_info);
  call(v.paint_handler);

  //        paint_listbox(&Fpg->list_info);

  //*******************************************************
  v.redraw = 1;

  return 1;
}

int fpg_remap_to_pal(FPG *Fpg) {
  struct gradient_rule CopiaReglas[16];
  HeadFPG other_header;
  char ActualPath[_MAX_PATH + 14];
  char *other_image;
  short *other_points = NULL;
  byte tmp[768];
  int x;
  FILE *fpg;
  FILE *Oldfpg;
  int y;
  byte color_lut[256];

  // Temporary file name
  div_strcpy(ActualPath, sizeof(ActualPath), (char *)Fpg->current_file);
  for (x = strlen(ActualPath); x >= 0; x--)
    if (IS_PATH_SEP(ActualPath[x]))
      x = -1;
    else
      ActualPath[x] = 0;
  div_strcat(ActualPath, sizeof(ActualPath), "_DIV_.FPG");
  delete_file(ActualPath);

  if ((fpg = fopen((char *)Fpg->current_file, "rb")) == NULL)
    return 0;
  if ((Oldfpg = fopen((char *)ActualPath, "wb")) == NULL) {
    fclose(fpg);
    return 0;
  }

  // Copy graphic header
  fread(tmp, 8, 1, fpg);
  fwrite(tmp, 8, 1, Oldfpg);
  fread(tmp, 768, 1, fpg);
  create_dac4();
  for (x = 0; x < 256; x++)
    color_lut[x] = find_color(tmp[x * 3], tmp[x * 3 + 1], tmp[x * 3 + 2]);
  fwrite(dac, 768, 1, Oldfpg);
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
  delete_file((char *)Fpg->current_file);
  rename(ActualPath, (char *)Fpg->current_file);
  if (!fpg_open(Fpg, (char *)Fpg->current_file))
    return 0;
  new_dac_loaded = 0;
  return 1;
}
#define BUFFERCOPYLEN 4096
void close_fpg(char *fpg_path);
void fpg_save(int n) {
  int w = window[n].w / big2, h = window[n].h / big2;
  FPG *Fpg = (FPG *)window[n].aux;
  FILE *FileOrg, *FileDest;
  int Lengt;
  char *Buffer;
  Buffer = (char *)malloc(BUFFERCOPYLEN);
  if (Buffer == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  div_strcpy(full, sizeof(full), file_types[4].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  FileOrg = fopen((char *)Fpg->current_file, "rb");
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

  div_strcpy((char *)Fpg->current_file, sizeof(Fpg->current_file), full);
  div_strcpy((char *)Fpg->fpg_name, sizeof(Fpg->fpg_name), input);

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

int fpg_delete(FPG *Fpg,
               int COD) // Trash, overwrite existing entry, or when changing a graphic's code
{
  struct gradient_rule CopiaReglas[16];
  HeadFPG other_header;
  char ActualPath[_MAX_PATH + 14];
  char *other_image;
  short *other_points = NULL;
  byte tmp[768];
  int x, len, n;
  FILE *fpg;
  FILE *Oldfpg;

  debugprintf("Deleting map %d\n", COD);

  // Temporary file name
  div_strcpy(ActualPath, sizeof(ActualPath), (char *)Fpg->current_file);
  for (x = strlen(ActualPath); x >= 0; x--)
    if (IS_PATH_SEP(ActualPath[x]))
      x = -1;
    else
      ActualPath[x] = 0;
  div_strcat(ActualPath, sizeof(ActualPath), "_DIV_.FPG");
  delete_file(ActualPath);

  if ((fpg = fopen((char *)Fpg->current_file, "rb")) == NULL)
    return 0;
  if ((Oldfpg = fopen(ActualPath, "wb")) == NULL) {
    fclose(fpg);
    return 0;
  }

  n = 0;
  while (1) {
    if (Fpg->desc_index[n] == COD || Fpg->desc_index[n] == 0)
      break;
    n++;
  }
  debugprintf("found COD at index: %d\n", n);

  if (Fpg->thumb[n].ptr != NULL)
    free(Fpg->thumb[n].ptr);
  memmove(&(Fpg->thumb[n]), &(Fpg->thumb[n + 1]), sizeof(t_thumb) * (999 - n));

  mouse_graf = 3;
  fseek(fpg, 0, SEEK_END);
  len = ftell(fpg);
  fseek(fpg, 0, SEEK_SET);
  show_progress((char *)texts[436], 0, len);

  // Copy graphic header
  fread(tmp, 8, 1, fpg);
  fwrite(tmp, 8, 1, Oldfpg);
  fread(tmp, 768, 1, fpg);
  fwrite(tmp, 768, 1, Oldfpg);
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

    if (other_header.code != COD) {
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

  delete_file((char *)Fpg->current_file);

  debugprintf("MOVE %s to %s\n", ActualPath, (char *)Fpg->current_file);


  rename(ActualPath, (char *)Fpg->current_file);

  if (!fpg_open(Fpg, (char *)Fpg->current_file)) {
    return 0;
  }
  return 1;
}

int fpg_delete_many(FPG *Fpg, int taggeds, int *array_del) {
  struct gradient_rule CopiaReglas[16];
  HeadFPG other_header;
  char ActualPath[_MAX_PATH + 14];
  char *other_image;
  short *other_points = NULL;
  byte tmp[768];
  int x, y, len, n;
  FILE *fpg;
  FILE *Oldfpg;

  div_strcpy(ActualPath, sizeof(ActualPath), (char *)Fpg->current_file); // Temporary file name
  for (x = strlen(ActualPath); x >= 0; x--)
    if (IS_PATH_SEP(ActualPath[x]))
      x = -1;
    else
      ActualPath[x] = 0;
  div_strcat(ActualPath, sizeof(ActualPath), "_DIV_.FPG");
  delete_file(ActualPath);

  if ((fpg = fopen((char *)Fpg->current_file, "rb")) == NULL)
    return 0;
  if ((Oldfpg = fopen(ActualPath, "wb")) == NULL) {
    fclose(fpg);
    return 0;
  }

  mouse_graf = 3;
  fseek(fpg, 0, SEEK_END);
  len = ftell(fpg);
  fseek(fpg, 0, SEEK_SET);
  show_progress((char *)texts[436], 0, len);

  fread(tmp, 8, 1, fpg); // Copy graphic header
  fwrite(tmp, 8, 1, Oldfpg);
  fread(tmp, 768, 1, fpg);
  fwrite(tmp, 768, 1, Oldfpg);
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
        if (Fpg->desc_index[n] == other_header.code || Fpg->desc_index[n] == 0)
          break;
        n++;
      }

      if (Fpg->thumb[n].ptr != NULL)
        free(Fpg->thumb[n].ptr);
      memmove(&(Fpg->thumb[n]), &(Fpg->thumb[n + 1]), sizeof(t_thumb) * (999 - n));

      memmove(&(Fpg->desc_index[n]), &(Fpg->desc_index[n + 1]), sizeof(int) * (999 - n));
    }

    // *************************

    free(other_image);

    if (other_header.num_points != 0)
      free(other_points);
  }

  fclose(Oldfpg);
  fclose(fpg);

  show_progress((char *)texts[436], len, len);

  delete_file((char *)Fpg->current_file);
  rename(ActualPath, (char *)Fpg->current_file);

  if (!fpg_open(Fpg, (char *)Fpg->current_file))
    return 0;
  return 1;
}
