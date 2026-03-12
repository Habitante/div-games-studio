
// NOTE: The main text rendering function is create_text()
// ShowFont is the font window (font loaded in memory), GetText is the "input text" dialog
// When calling create_text(), v.aux of ShowFont points to:
//   v.aux -> filename, v.aux+14 -> path, v.aux+RES_FOR_NAME -> FNT file data
// font_aux is NOT set to v.aux+RES_FOR_NAME before calling create_text (font_aux already points to the file)
// The string is passed to create_text via cCharsToPrint[128]

#include "global.h"
#include "div_string.h"
#include "ifs.h"

void Selcolor0(void);
void GenFont0(void);
int font_generated = 0;
void get_text0(void);
void create_test_text(char *s, char flags);


char fontsizes[8 * 8] = "8x8\0    10x10\0  12x12\0  14x14\0  16x16\0  32x32\0  64x64\0  128x128";
int fontTamanos[8] = {8, 10, 12, 14, 16, 32, 64, 128};
struct t_listbox lfontsizes = {116, 12, fontsizes, 8, 4, 41};
void Preview();
void Preview20();
void get_font();
void get_ifs();
int create_font(int GenCode);

int text1_w = 1;
int text1_h = 1;
int Text1Col = 0;
char *Text1;
int text2_w = 1;
int text2_h = 1;
int Text2Col = 0;
char *Text2;
int text3_w = 1;
int text3_h = 1;
int Text3Col = 0;
char *Text3;
char *Text01 = NULL;
char *Text02 = NULL;
char *Text03 = NULL;
char Load_FontPathName[256];
char Load_FontName[14];

char FontPathName[256];
char FontName[1024];

char FacePathName[256];
char FaceName[1024];

int font_height_gen, font_width_gen;
char font_height_str[5], font_width_str[5];
char test_string[21];
char test_string2[21];
int OutLine = 0;
int outline_dir = 0;
int highlight_dir[5] = {0, 1, 3, 2, 4};
int shadow_x = 0;
int shadow_y = 0;
int Mosaico1 = 0, Mosaico2 = 0, Mosaico3 = 0;
char char_table[256];
char cCharsToPrint[128];

int sel_color_font;
int sel_color_ok;
char *font_aux;

int spacelen;

void pal_show_text() {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h, x, y, tal = 24, tan = 41;
  if (big) {
    tan *= 2;
    tal *= 2;
  }
  for (y = 0; y < tal; y++)
    for (x = 0; x < tan; x++)
      ptr[(y * w + x) + ((big ? 86 * 2 : 86) * w + (big ? 64 * 2 : 64))] = Text1[y * tan + x];
  for (y = 0; y < tal; y++)
    for (x = 0; x < tan; x++)
      ptr[(y * w + x) + ((big ? 86 * 2 : 86) * w + (big ? 124 * 2 : 124))] = Text2[y * tan + x];
  for (y = 0; y < tal; y++)
    for (x = 0; x < tan; x++)
      ptr[(y * w + x) + ((big ? 86 * 2 : 86) * w + (big ? 4 * 2 : 4))] = Text3[y * tan + x];
  wrectangle(ptr, w / big2, h / big2, c0, 4, 86, 41, 24);
  wrectangle(ptr, w / big2, h / big2, c0, 64, 86, 41, 24);
  wrectangle(ptr, w / big2, h / big2, c0, 124, 86, 41, 24);
  v.redraw = 1;
}

void Fonts1(void) {
  int w = v.w / big2, h = v.h / big2;
  char cWork[10];
  _show_items();

  create_listbox(&lfontsizes);

  wbox(v.ptr, w, h, c0, 2, 49, w - 4, 1);     // Upper horizontal line
  wbox(v.ptr, w, h, c0, 54, 49, 1, 92 - 20);  // Left vertical line
  wbox(v.ptr, w, h, c0, 114, 49, 1, 92 - 20); // Right vertical line
  wbox(v.ptr, w, h, c0, 18 + 67, 10, 1, 39);  // Upper-middle vertical line

  wbox(v.ptr, w, h, c2, 24 - 8, 61 - 1, 16, 8); // Dark border for Outline
  wbox(v.ptr, w, h, c2, 84 - 8, 61 - 1, 16, 8); // Dark border for shadow X
  wbox(v.ptr, w, h, c2, 84 - 8, 75 - 1, 16, 8); // Dark border for shadow Y

  wwrite(v.ptr, w, h, 4 + 60 + 20, 51, 1, texts[86], c3);
  wwrite(v.ptr, w, h, 24 + 60, 61, 1, (byte *)itoa(OutLine, cWork, 10), c3);
  wput(v.ptr, w, h, 20 + 60, 75, 220 + outline_dir);

  wwrite(v.ptr, w, h, 64 + 60 + 20, 51, 1, texts[87], c3);
  wwrite(v.ptr, w, h, 84 + 60, 61, 1, (byte *)itoa(shadow_x, cWork, 10), c3);
  wwrite(v.ptr, w, h, 84 + 60, 75, 1, (byte *)itoa(shadow_y, cWork, 10), c3);

  wwrite(v.ptr, w, h, 124 - 120 + 20, 51, 1, texts[91], c3);

  wwrite(v.ptr, w, h, 4, 12, 0, texts[84], c3);
  wbox(v.ptr, w, h, c2, 4, 19, 66, 8); // Dark border for Font
  wwrite(v.ptr, w, h, 4, 19, 0, (byte *)FontName, c4);
  wwrite(v.ptr, w, h, 4, 31, 0, texts[85], c3);
  wbox(v.ptr, w, h, c2, 4, 38, 66, 8); // Dark border for Face
  wwrite(v.ptr, w, h, 4, 38, 0, (byte *)FaceName, c4);

  pal_show_text();

  wbox(v.ptr, w, h, c0, 2, h - 38 - 3, w - 4, 1); // Lower horizontal line
  wbox(v.ptr, w, h, c0, 2, h - 20, w - 4, 1);     // Lower horizontal line
}

void close_old_fnt(void) {
  int m;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 104 && !strcmp(FontName, (char *)window[m].aux)) {
      move(0, m);
      close_window();
      break;
    }
}

void Fonts2(void) {
  int w = v.w / big2, h = v.h / big2, x, y, tal = 24, tan = 41;
  float px, py;
  char cWork[10];
  if (big) {
    tan *= 2;
    tal *= 2;
  }
  _process_items();
  switch (v.active_item) {
  case 0:
    get_font();
    break;
  case 1:
    get_ifs();
    break;
  case 2:
  case 3:
    if ((atoi(font_height_str) < 16 || atoi(font_width_str) < 16) && atoi(font_height_str) != atoi(font_width_str)) {
      if (atoi(font_height_str) < 16) {
        DIV_STRCPY(font_height_str, "16");
        font_height_gen = 16;
      }
      if (atoi(font_width_str) < 16) {
        DIV_STRCPY(font_width_str, "16");
        font_width_gen = 16;
      }
      call((void_return_type_t)v.paint_handler);
      v.redraw = 1;
    }
    break;

  case 4:
    if (--OutLine < 0)
      OutLine = 0;
    wbox(v.ptr, w, h, c2, 24 - 7 + 60, 61 - 1, 14, 8);
    wwrite(v.ptr, w, h, 24 + 60, 61, 1, (byte *)itoa(OutLine, cWork, 10), c3);
    v.redraw = 1;
    break;
  case 5:
    if (++OutLine > 99)
      OutLine = 99;
    wbox(v.ptr, w, h, c2, 24 - 7 + 60, 61 - 1, 14, 8);
    wwrite(v.ptr, w, h, 24 + 60, 61, 1, (byte *)itoa(OutLine, cWork, 10), c3);
    v.redraw = 1;
    break;
  case 6:
    if (--outline_dir < 0)
      outline_dir = 4;
    wput(v.ptr, w, h, 80, 75, 220 + outline_dir);
    v.redraw = 1;
    break;
  case 7:
    if (++outline_dir > 4)
      outline_dir = 0;
    wput(v.ptr, w, h, 80, 75, 220 + outline_dir);
    v.redraw = 1;
    break;
  case 8:
    if (--shadow_x < -99)
      shadow_x = -99;
    wbox(v.ptr, w, h, c2, 84 - 7 + 60, 61 - 1, 14, 8);
    wwrite(v.ptr, w, h, 84 + 60, 61, 1, (byte *)itoa(shadow_x, cWork, 10), c3);
    v.redraw = 1;
    break;
  case 9:
    if (++shadow_x > 99)
      shadow_x = 99;
    wbox(v.ptr, w, h, c2, 84 - 7 + 60, 61 - 1, 14, 8);
    wwrite(v.ptr, w, h, 84 + 60, 61, 1, (byte *)itoa(shadow_x, cWork, 10), c3);
    v.redraw = 1;
    break;
  case 10:
    if (--shadow_y < -99)
      shadow_y = -99;
    wbox(v.ptr, w, h, c2, 84 - 7 + 60, 75 - 1, 14, 8);
    wwrite(v.ptr, w, h, 84 + 60, 75, 1, (byte *)itoa(shadow_y, cWork, 10), c3);
    v.redraw = 1;
    break;
  case 11:
    if (++shadow_y > 99)
      shadow_y = 99;
    wbox(v.ptr, w, h, c2, 84 - 7 + 60, 75 - 1, 14, 8);
    wwrite(v.ptr, w, h, 84 + 60, 75, 1, (byte *)itoa(shadow_y, cWork, 10), c3);
    v.redraw = 1;
    break;
  case 13:
    if (!strcmp(FaceName, "")) {
      v_text = (char *)texts[263];
      show_dialog(err0);
      break;
    }
    Preview();
    v.redraw = 1;
    break;
  case 14:
    if (!strcmp(FaceName, "")) {
      v_text = (char *)texts[263];
      show_dialog(err0);
      break;
    }
    font_generated = 0;
    show_dialog(GenFont0);
    if (font_generated) {
      close_old_fnt();
      div_strcpy(Load_FontName, sizeof(Load_FontName), FontName);
      DIV_STRCPY(Load_FontPathName, FontPathName);
      for (x = strlen(Load_FontPathName) + 1; x >= 0; x++) {
        if (IS_PATH_SEP(Load_FontPathName[x])) {
          Load_FontPathName[x + 1] = 0;
          break;
        }
      }
      new_window(show_font0);
    }
    v.redraw = 1;
    break;
  case 15:
    close_window();
    return;
    break;
  }

  if ((wmouse_y > 86) && (wmouse_y < 86 + 24) && (mouse_b)) {
    if ((wmouse_x > 64) && (wmouse_x < 64 + 41)) {
      _reselect_item();
      sel_color_font = Text1Col;
      show_dialog(Selcolor0);
      if (sel_color_ok) {
        Text1Col = sel_color_font;
        memset(Text1, Text1Col, tan * tal + tan);
        text1_w = 1;
        text1_h = 1;
        pal_show_text();
        wmouse_x = -1;
        if (Text01 != NULL) {
          free(Text01);
          Text01 = NULL;
        }
      }
      return;
    }
    if ((wmouse_x > 124) && (wmouse_x < 124 + 41)) {
      _reselect_item();
      sel_color_font = Text2Col;
      show_dialog(Selcolor0);
      if (sel_color_ok) {
        Text2Col = sel_color_font;
        memset(Text2, Text2Col, tan * tal + tan);
        text2_w = 1;
        text2_h = 1;
        pal_show_text();
        wmouse_x = -1;
        if (Text02 != NULL) {
          free(Text02);
          Text02 = NULL;
        }
      }
      return;
    }
    if ((wmouse_x > 4) && (wmouse_x < 4 + 41)) {
      _reselect_item();
      sel_color_font = Text3Col;
      show_dialog(Selcolor0);
      if (sel_color_ok) {
        Text3Col = sel_color_font;
        memset(Text3, Text3Col, tan * tal + tan);
        text3_w = 1;
        text3_h = 1;
        pal_show_text();
        wmouse_x = -1;
        if (Text03 != NULL) {
          free(Text03);
          Text03 = NULL;
        }
      }
      return;
    }
  }

  if (dragging == 4) {
    dragging = 5;
    free_drag = 0;
    if ((wmouse_y > 86) && (wmouse_y < 86 + 24)) {
      if ((wmouse_x > 64) && (wmouse_x < 64 + 41)) {
        if (Text1Col == 0)
          Text1Col = 1;
        if (Text01 != NULL)
          free(Text01);
        Text01 = (char *)malloc(window[1].mapa->map_width * window[1].mapa->map_height);
        if (Text01 == NULL) {
          v_text = (char *)texts[45];
          show_dialog(err0);
          text1_w = 1;
          text1_h = 1;
        } else {
          text1_w = window[1].mapa->map_width;
          text1_h = window[1].mapa->map_height;
          memcpy(Text01, window[1].mapa->map, text1_w * text1_h);
          x = 0;
          y = 0;
          for (py = 0; py < window[1].mapa->map_height;
               py += window[1].mapa->map_height / (float)tal) {
            x = 0;
            for (px = 0; px < window[1].mapa->map_width;
                 px += window[1].mapa->map_width / (float)tan)
              Text1[y * tan + x++] =
                  window[1].mapa->map[(memptrsize)py * window[1].mapa->map_width + (memptrsize)px];
            y++;
          }
        }
        pal_show_text();
      }
      if ((wmouse_x > 124) && (wmouse_x < 124 + 41)) {
        if (Text2Col == 0)
          Text2Col = 1;
        if (Text02 != NULL)
          free(Text02);
        Text02 = (char *)malloc(window[1].mapa->map_width * window[1].mapa->map_height);
        if (Text02 == NULL) {
          v_text = (char *)texts[45];
          show_dialog(err0);
          text2_w = 1;
          text2_h = 1;
        } else {
          text2_w = window[1].mapa->map_width;
          text2_h = window[1].mapa->map_height;
          memcpy(Text02, window[1].mapa->map, text2_w * text2_h);
          x = 0;
          y = 0;
          for (py = 0; py < window[1].mapa->map_height;
               py += window[1].mapa->map_height / (float)tal) {
            x = 0;
            for (px = 0; px < window[1].mapa->map_width;
                 px += window[1].mapa->map_width / (float)tan)
              Text2[y * tan + x++] =
                  window[1].mapa->map[(memptrsize)py * window[1].mapa->map_width + (memptrsize)px];
            y++;
          }
        }
        pal_show_text();
      }
      if ((wmouse_x > 4) && (wmouse_x < 4 + 41)) {
        if (Text3Col == 0)
          Text3Col = 1;
        if (Text03 != NULL)
          free(Text03);
        Text03 = (char *)malloc(window[1].mapa->map_width * window[1].mapa->map_height);
        if (Text03 == NULL) {
          v_text = (char *)texts[45];
          show_dialog(err0);
          text3_w = 1;
          text3_h = 1;
        } else {
          text3_w = window[1].mapa->map_width;
          text3_h = window[1].mapa->map_height;
          memcpy(Text03, window[1].mapa->map, text3_w * text3_h);
          x = 0;
          y = 0;
          for (py = 0; py < window[1].mapa->map_height;
               py += window[1].mapa->map_height / (float)tal) {
            x = 0;
            for (px = 0; px < window[1].mapa->map_width;
                 px += window[1].mapa->map_width / (float)tan)
              Text3[y * tan + x++] =
                  window[1].mapa->map[(memptrsize)py * window[1].mapa->map_width + (memptrsize)px];
            y++;
          }
        }
        pal_show_text();
      }
    }
    free_drag = 1;
  }

  update_listbox(&lfontsizes);
  if ((lfontsizes.zone >= 10) && (mouse_b & 1)) {
    font_width_gen = font_height_gen = fontTamanos[(lfontsizes.zone - 10) + lfontsizes.first_visible];
    DIV_SPRINTF(font_height_str, "%d", font_height_gen);
    DIV_SPRINTF(font_width_str, "%d", font_width_gen);
    call((void_return_type_t)v.paint_handler);
    v.redraw = 1;
  }
}
int FontsWindowInUse = 0;
void Fonts3(void) {
  FontsWindowInUse = 0;
  if (Text1 != NULL)
    free(Text01);
  if (Text2 != NULL)
    free(Text02);
  if (Text3 != NULL)
    free(Text03);
  free(Text1);
  free(Text2);
  free(Text3);
}

void Fonts0(void) {
  int tal = 24, tan = 41;
  FILE *f;

  memset(test_string, 0, 21);
  memcpy(test_string, texts[73], 20);
  if (big) {
    tan *= 2;
    tal *= 2;
  }
  v.type = 102;
  v.w = 169;
  v.h = 179 - 17;
  v.paint_handler = Fonts1;
  v.click_handler = Fonts2;
  v.close_handler = Fonts3;
  v.title = texts[83];
  v.name = texts[83];
  lfontsizes.created = 0;
  lfontsizes.total_items = 8;

  text1_w = 1;
  text1_h = 1;
  Text1Col = c23;
  Text1 = (char *)malloc(tan * tal * 10);
  if (Text1 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memset(Text1, Text1Col, tan * tal * 10);

  text2_w = 1;
  text2_h = 1;
  Text2Col = c1;
  Text2 = (char *)malloc(tan * tal * 10);
  if (Text2 == NULL) {
    free(Text1);
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memset(Text2, Text2Col, tan * tal * 10);

  text3_w = 1;
  text3_h = 1;
  Text3Col = c4;
  Text3 = (char *)malloc(tan * tal * 10);
  if (Text3 == NULL) {
    free(Text1);
    free(Text2);
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memset(Text3, Text3Col, tan * tal * 10);
  font_height_gen = 8;
  font_width_gen = 8;
  OutLine = 0;
  outline_dir = 0;
  shadow_x = 0;
  shadow_y = 0;
  Mosaico1 = 0;
  Mosaico2 = 0;
  Mosaico3 = 0;

  DIV_STRCPY(FontName, (char *)texts[90]);
  DIV_STRCPY(FontPathName, file_types[5].path);
  DIV_STRCAT(FontPathName, "/");
  DIV_STRCAT(FontPathName, FontName);

  DIV_STRCPY(FaceName, "NORMAL.IFS");
  DIV_STRCPY(FacePathName, file_types[6].path);
  DIV_STRCAT(FacePathName, "/");
  DIV_STRCAT(FacePathName, FaceName);

  if ((f = fopen(FacePathName, "rb")) != NULL)
    fclose(f);
  else {
    div_strcpy(file_types[6].path, sizeof(file_types[6].path), file_types[1].path);
    div_strcat(file_types[6].path, sizeof(file_types[6].path), "/IFS");
    DIV_STRCPY(FacePathName, file_types[6].path);
    DIV_STRCAT(FacePathName, "/NORMAL.IFS");
    if ((f = fopen(FacePathName, "rb")) != NULL)
      fclose(f);
    else {
      DIV_STRCPY(FaceName, "");
      DIV_STRCPY(FacePathName, "");
      div_strcpy(file_types[6].path, sizeof(file_types[6].path), file_types[1].path);
    }
  }

  DIV_SPRINTF(font_height_str, "%d", font_height_gen);
  DIV_SPRINTF(font_width_str, "%d", font_width_gen);

  _button(121, 74, 16, 0);                              //15
  _button(121, 74, 35, 0);                              //16
  _get(133, 88, 11, 19, (byte *)font_width_str, 4, 16, 128); //1
  _get(134, 88, 30, 19, (byte *)font_height_str, 4, 16, 128);  //2

  _button(111, 7 + 60, 61, 0);  //4
  _button(110, 37 + 60, 61, 0); //5
  _button(111, 7 + 60, 75, 0);  //6 //129
  _button(110, 37 + 60, 75, 0); //7
  _button(111, 67 + 60, 61, 0); //8
  _button(110, 97 + 60, 61, 0); //9
  _button(111, 67 + 60, 75, 0); //10
  _button(110, 97 + 60, 75, 0); //11

  _get(80, 4, v.h - (39), v.w - 16 - text_len(texts[113] + 1), (byte *)test_string, 21, 0, 0);
  _button(113, v.w - 8, v.h - 14 - 18, 2); //14

  _button(120, 7, v.h - 14, 0);       //13
  _button(119, v.w - 8, v.h - 14, 2); //12

  _flag(112, 64, 112, &Mosaico1);
  _flag(112, 124, 112, &Mosaico2);
  _flag(112, 4, 112, &Mosaico3);
}

void open_gen_font() {
  if (!FontsWindowInUse) {
    FontsWindowInUse = 1;
    new_window(Fonts0);
  }
}

int create_font(int GenCode) {
  int x;
  byte _c0;
  FILE *file;
  file = fopen(FacePathName, "rb");
  if (file == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return 0;
  } else
    fclose(file);
  if (GenCode != 255) {
    for (x = 0; x < 256; x++)
      char_table[x] = 0;

    if (GenCode & NUM)
      for (x = '0'; x < '9' + 1; x++)
        char_table[x] = 1;

    if (GenCode & MAY)
      for (x = 'A'; x < 'Z' + 1; x++)
        char_table[x] = 1;

    if (GenCode & MIN)
      for (x = 'a'; x < 'z' + 1; x++)
        char_table[x] = 1;
    if (GenCode & SIM)
      for (x = 32; x < '0'; x++)
        char_table[x] = 1;
    if (GenCode & SIM)
      for (x = '9' + 1; x < 'A'; x++)
        char_table[x] = 1;
    if (GenCode & SIM)
      for (x = 'Z' + 1; x < 'a'; x++)
        char_table[x] = 1;
    if (GenCode & SIM)
      for (x = 'z' + 1; x < 128; x++)
        char_table[x] = 1;
    if (GenCode & EXT)
      for (x = 0; x < 32; x++)
        char_table[x] = 1;
    if (GenCode & EXT)
      for (x = 128; x < 256; x++)
        char_table[x] = 1;
  }

  div_strcpy(ifs.ifsName, sizeof(ifs.ifsName), FacePathName);
  div_strcpy(ifs.fntName, sizeof(ifs.fntName), FontPathName);

  font_width_gen = atoi(font_width_str);
  font_height_gen = atoi(font_height_str);
  ifs.size_x = font_width_gen;
  ifs.size_y = font_height_gen;

  ifs.outline = OutLine;
  ifs.brightness = highlight_dir[outline_dir];
  ifs.shadow_x = shadow_x;
  ifs.shadow_y = shadow_y;

  ifs.body_tex_w = text3_w;
  ifs.body_tex_h = text3_h;
  ifs.body_tex_color = Text3Col;
  ifs.body_tex_mode = !Mosaico3;

  ifs.shadow_tex_w = text2_w;
  ifs.shadow_tex_h = text2_h;
  ifs.shadow_tex_color = Text2Col;
  ifs.shadow_tex_mode = !Mosaico2;

  ifs.outline_tex_w = text1_w;
  ifs.outline_tex_h = text1_h;
  ifs.outline_tex_color = Text1Col;
  ifs.outline_tex_mode = !Mosaico1;

  memcpy(ifs.table, char_table, 256);

  ifs.highlights[0] = 0; // Transparent color stays unchanged
  ifs.shadows[0] = 0;

  _c0 = find_color_not0(dac[0], dac[1], dac[2]);

  for (x = 1; x < 256; x++) {
    ifs.highlights[x] = average_color(x, average_color(x, c4));
    if (ifs.highlights[x] == 0)
      ifs.highlights[x] = _c0;
  }

  for (x = 1; x < 256; x++) {
    ifs.shadows[x] = average_color(x, average_color(x, c0));
    if (ifs.shadows[x] == 0)
      ifs.shadows[x] = _c0;
  }

  body_tex_buffer = Text03;
  shadow_tex_buffer = Text02;
  out_tex_buffer = Text01;
  if (jorge_create_font(GenCode)) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return 0;
  }
  return 1;
}

void get_font() {
  int w = v.w / big2, h = v.h / big2;

  v_mode = 1;
  v_type = 5;

  DIV_STRCPY(FontPathName, file_types[v_type].path);
  DIV_STRCAT(FontPathName, "/");
  DIV_STRCAT(FontPathName, FontName);

  v_text = (char *)texts[264];
  show_dialog(browser0);
  if (v_finished) {
    if (v_exists) {
      v_title = (char *)texts[75];
      v_text = (char *)texts[76];
      show_dialog(accept0);
      if (!v_accept)
        return;
    }
    DIV_STRCPY(FontName, input);
    DIV_STRCPY(FontPathName, file_types[v_type].path);
    if (!IS_PATH_SEP(FontPathName[strlen(FontPathName) - 1]))
      DIV_STRCAT(FontPathName, "/");
    DIV_STRCAT(FontPathName, input);
    wbox(v.ptr, w, h, c2, 4, 19, 66, 8); // Dark border for Font
    wwrite(v.ptr, w, h, 4, 19, 0, (byte *)FontName, c4);
    v.redraw = 1;
  }
}

//-----------------------------------------------------------------------------

#define max_archivos 512 // ------------------------------- File listbox
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[max_archivos];
extern int num_taggeds;

void get_ifs() {
  int w = v.w / big2, h = v.h / big2;
  int num;
  FILE *f;

  v_mode = 0;
  v_type = 6;
  v_text = (char *)texts[77];
  show_dialog(browser0);
  if (v_finished) {
    if (!v_exists) {
      v_text = (char *)texts[43];
      show_dialog(err0);
    } else {
      if (!num_taggeds) {
        DIV_STRCPY(full, file_types[v_type].path);
        if (full[strlen(full) - 1] != '/')
          DIV_STRCAT(full, "/");
        DIV_STRCAT(full, input);
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

      for (num = 0; num < file_list_br.total_items; num++) {
        if (thumb[num].tagged) {
          DIV_STRCPY(input, file_list_br.list + file_list_br.item_width * num);
          DIV_STRCPY(FaceName, input);
          DIV_STRCPY(FacePathName, file_types[v_type].path);
          if (!IS_PATH_SEP(FacePathName[strlen(FacePathName) - 1]))
            DIV_STRCAT(FacePathName, "/");
          DIV_STRCAT(FacePathName, input);
        }
      }
      wbox(v.ptr, w, h, c2, 4, 38, 66, 8); // Dark border for Face
      wwrite(v.ptr, w, h, 4, 38, 0, (byte *)FaceName, c4);
      v.redraw = 1;
    }
  }
}

//-----------------------------------------------------------------------------

byte OldColor;
byte PrevColor;

void Selcolor1(void) {
  int x, y;
  int w = v.w / big2, h = v.h / big2;

  sel_color_ok = 0;
  _show_items();
  wbox(v.ptr, w, h, c0, 2, 10, 128, 128);
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, (y * 16 + x), x * 8 + 2, y * 8 + 10, 7, 7);
  wrectangle(v.ptr, w, h, c4, (sel_color_font % 16) * 8 + 1, (sel_color_font / 16) * 8 + 9, 9, 9);
  if (dac[sel_color_font * 3] * dac[sel_color_font * 3 + 1] * dac[sel_color_font * 3 + 2] <
      (32 * 32 * 32))
    wbox(v.ptr, w, h, c4, (sel_color_font % 16) * 8 + 4, (sel_color_font / 16) * 8 + 12, 3, 3);
  else
    wbox(v.ptr, w, h, c0, (sel_color_font % 16) * 8 + 4, (sel_color_font / 16) * 8 + 12, 3, 3);
  PrevColor = OldColor = sel_color_font;
}

void Selcolor2(void) {
  int w = v.w / big2, h = v.h / big2;
  byte cColor;

  _process_items();
  switch (v.active_item) {
  case 0:
    end_dialog = 1;
    sel_color_ok = 1;
    break;
  case 1:
    end_dialog = 1;
    sel_color_ok = 0;
    break;
  }

  if ((wmouse_y > 10) && (wmouse_y < 138) && (wmouse_x > 2) && (wmouse_x < 130)) {
    cColor = ((wmouse_y - 10) / 8) * 16 + ((wmouse_x - 2) / 8);
    if (cColor != OldColor) {
      wrectangle(v.ptr, w, h, c0, (OldColor % 16) * 8 + 1, (OldColor / 16) * 8 + 9, 9, 9);
      wrectangle(v.ptr, w, h, c4, (cColor % 16) * 8 + 1, (cColor / 16) * 8 + 9, 9, 9);
      v.redraw = 1;
      OldColor = cColor;
    }
    if (mouse_b) {
      if (sel_color_font != cColor) {
        wbox(v.ptr, w, h, sel_color_font, (sel_color_font % 16) * 8 + 4,
             (sel_color_font / 16) * 8 + 12, 3, 3);
        sel_color_font = cColor;
        if (dac[sel_color_font * 3] + dac[sel_color_font * 3 + 1] + dac[sel_color_font * 3 + 2] <
            (32 + 32 + 32))
          wbox(v.ptr, w, h, c4, (sel_color_font % 16) * 8 + 4, (sel_color_font / 16) * 8 + 12, 3,
               3);
        else
          wbox(v.ptr, w, h, c0, (sel_color_font % 16) * 8 + 4, (sel_color_font / 16) * 8 + 12, 3,
               3);
        v.redraw = 1;
      }
    }
  } else {
    wrectangle(v.ptr, w, h, c0, (OldColor % 16) * 8 + 1, (OldColor / 16) * 8 + 9, 9, 9);
    wrectangle(v.ptr, w, h, c4, (sel_color_font % 16) * 8 + 1, (sel_color_font / 16) * 8 + 9, 9, 9);
    OldColor = sel_color_font;
    v.redraw = 1;
  }
}

void Selcolor3(void) {
  if (!sel_color_ok)
    sel_color_font = PrevColor;
}

void Selcolor0(void) {
  v.type = 1; // Dialog
  v.w = 131;
  v.h = 157;
  v.title = texts[78];
  v.paint_handler = Selcolor1;
  v.click_handler = Selcolor2;
  v.close_handler = Selcolor3;
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
}

int TamaY = 0, TamaX = 0;

void Preview1() {
  int can = v.w, cal = v.h;
  byte *ptr = v.ptr;
  int init = 0, x, y, len;
  int bancho, balto;
  char *MyBuffer, *BuffAux;
  _show_items();
  BuffAux = MyBuffer = (char *)malloc(TamaX * TamaY);
  if (MyBuffer == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memset(MyBuffer, c2, TamaX * TamaY);
  init = 0;
  for (x = 0; x < strlen(test_string); x++) {
    len = show_char(test_string[x], init, 0, MyBuffer, TamaX);
    if (len <= 1)
      len = spacelen;
    init += len;
  }

  ptr += 11 * big2 * can + 2 * big2;

  if ((can - 4 * big2) >= TamaX) { // Narrower than window: center in window
    bancho = TamaX;
    ptr += ((can - 4 * big2) - TamaX) / 2 + 1;
  } else { // Wider than window: center the buffer
    bancho = (can - 4 * big2);
    BuffAux += (TamaX - (can - 4 * big2)) / 2;
  }

  if ((cal - 29 * big2) >= TamaY) { // Shorter than window: center in window
    balto = TamaY;
    ptr += (((cal - 29 * big2) - TamaY) / 2) * can;
  } else { // Taller than window: center the buffer
    balto = (cal - 29 * big2);
    BuffAux += ((TamaY - (cal - 29 * big2)) / 2) * TamaX;
  }

  for (y = 0; y < balto; y++)
    for (x = 0; x < bancho; x++)
      if (*(BuffAux + y * TamaX + x))
        *(ptr + y * can + x) = *(BuffAux + y * TamaX + x);

  free(MyBuffer);
}

void Preview2() {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void Preview0() {
  int x, fan, _fal = 0, fal, cnt, TX, TY;

  TamaX = 0;
  TamaY = 0;

  spacelen = 0;
  cnt = 0;
  for (x = 0; x < 255; x += 2) {
    get_char_size(x, &fan, &fal);
    if (fan > 1) {
      cnt++;
      spacelen += fan;
    }
    if (_fal < fal)
      _fal = fal;
  }
  if (cnt)
    spacelen = (spacelen / cnt) / 2;
  else
    spacelen = 1;

  for (x = 0; x < strlen(test_string); x++) {
    get_char_size(test_string[x], &fan, &fal);
    if (fan <= 1)
      fan = spacelen;
    TamaX += fan;
    if (TamaY < fal)
      TamaY = fal;
  }
  if (TamaY == 0)
    TamaY = _fal;

  v.type = 1; // Dialog
  if (big) {
    TX = TamaX / 2;
    TY = TamaY / 2;
  } else {
    TX = TamaX;
    TY = TamaY;
  }
  v.w = TX + 6;
  if (v.w < 60)
    v.w = 60;
  if (v.w > 320)
    v.w = 320;
  v.h = TY + 32;
  if (v.h < 32)
    v.h = 32;
  if (v.h > 200)
    v.h = 200;
  v.paint_handler = Preview1;
  v.click_handler = Preview2;
  v.title = texts[80];
  _button(100, v.w / 2, v.h - 14, 1);
}

void Preview() {
  int x;
  char saved_font_path[256];
  char saved_char_table[256];

  DIV_STRCPY(saved_font_path, FontPathName);
  memcpy(saved_char_table, char_table, 256);

  memset(char_table, 0, 256);
  for (x = 0; x < strlen(test_string); x++)
    char_table[test_string[x]] = 1;
  DIV_STRCPY(FontPathName, "PREVIEW.FNT");

  font_width_gen = atoi(font_width_str);
  font_height_gen = atoi(font_height_str);

  if (!create_font(255))
    return;
  show_dialog(Preview0);
  delete_file("PREVIEW.FNT");

  memcpy(char_table, saved_char_table, 256);
  DIV_STRCPY(FontPathName, saved_font_path);
}

void Preview_2() {
  int x;
  char saved_font_path[256];
  char saved_char_table[256];

  DIV_STRCPY(saved_font_path, FontPathName);
  memcpy(saved_char_table, char_table, 256);

  create_test_text(test_string2, v.aux[1352 + RES_FOR_NAME]);

  memset(char_table, 0, 256);
  for (x = 0; x < strlen(test_string2); x++)
    char_table[test_string2[x]] = 1;
  show_dialog(Preview20);

  memcpy(char_table, saved_char_table, 256);
  DIV_STRCPY(FontPathName, saved_font_path);
}

int GenFontbotones[5] = {1, 1, 1, 1, 0};
int GenFontLe = 0;
void GenFont1(void) {
  int w = v.w / big2, h = v.h / big2;
  char cWork[4];

  _show_items();
  GenFontLe = 0;

  font_width_gen = atoi(font_width_str);
  font_height_gen = atoi(font_height_str);

  wwrite(v.ptr, w, h, 4, 12, 0, texts[84], c3);
  wwrite(v.ptr, w, h, 4, 20, 0, (byte *)FontName, c4);

  wwrite(v.ptr, w, h, 4, 36, 0, texts[133], c3);
  DIV_SPRINTF(cWork, "%03d", font_width_gen);
  wwrite(v.ptr, w, h, 45, 36, 0, (byte *)cWork, c4);

  wwrite(v.ptr, w, h, 4, 44, 0, texts[134], c3);
  DIV_SPRINTF(cWork, "%03d", font_height_gen);
  wwrite(v.ptr, w, h, 45, 44, 0, (byte *)cWork, c4);

  wwrite(v.ptr, w, h, 4, 60, 0, texts[86], c3);
  if (OutLine)
    wwrite(v.ptr, w, h, 45, 60, 0, texts[88], c4);
  else
    wwrite(v.ptr, w, h, 45, 60, 0, texts[89], c4);

  wwrite(v.ptr, w, h, 4, 68, 0, texts[87], c3);
  if (shadow_x || shadow_y)
    wwrite(v.ptr, w, h, 45, 68, 0, texts[88], c4);
  else
    wwrite(v.ptr, w, h, 45, 68, 0, texts[89], c4);
  wrectangle(v.ptr, w, h, c0, w - 68, 10, 1, h - 30); //28

  wrectangle(v.ptr, w, h, c0, 2, h - 20, w - 4, 1);
}
int GenFontRet = 0;
void GenFont2(void) {
  _process_items();
  switch (v.active_item) {
  case 5:
    GenFontRet = 1;
    end_dialog = 1;
    break;
  case 6:
    end_dialog = 1;
    break;
  }
}
void GenFont3(void) {
  int GenCode = 0;
  if (GenFontRet) {
    if (GenFontbotones[0])
      GenCode += NUM;
    if (GenFontbotones[1])
      GenCode += MAY;
    if (GenFontbotones[2])
      GenCode += MIN;
    if (GenFontbotones[3])
      GenCode += SIM;
    if (GenFontbotones[4])
      GenCode += EXT;
    font_generated = create_font(GenCode);
  }
}
void GenFont0(void) {
  v.type = 1; // Dialog
  v.w = 169 - 26;
  v.h = 103;
  v.title = texts[852];
  v.paint_handler = GenFont1;
  v.click_handler = GenFont2;
  v.close_handler = GenFont3;

  _flag(114, 82, 15, &GenFontbotones[0]);
  _flag(115, 82, 29, &GenFontbotones[1]);
  _flag(116, 82, 43, &GenFontbotones[2]);
  _flag(117, 82, 57, &GenFontbotones[3]);
  _flag(118, 82, 71, &GenFontbotones[4]);
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  GenFontRet = 0;
}
void Preview21() {
  int can = v.w, cal = v.h;
  byte *ptr = v.ptr;
  int init, x, y, len;
  int bancho, balto;
  char *MyBuffer, *BuffAux;
  _show_items();
  BuffAux = MyBuffer = (char *)malloc(TamaX * TamaY);
  if (MyBuffer == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memset(MyBuffer, c2, TamaX * TamaY);

  init = 0;
  for (x = 0; x < strlen(test_string2); x++) {
    len = show_char_buffer(test_string2[x], init, 0, MyBuffer, TamaX, font_aux);
    if (len <= 1)
      len = 0;
    init += len;
  }

  ptr += 11 * big2 * can + 2 * big2;

  if ((can - 4 * big2) >= TamaX) { // Narrower than window: center in window
    bancho = TamaX;
    ptr += ((can - 4 * big2) - TamaX) / 2 + 1;
  } else { // Wider than window: center the buffer
    bancho = (can - 4 * big2);
    BuffAux += (TamaX - (can - 4 * big2)) / 2;
  }

  if ((cal - 29 * big2) >= TamaY) { // Shorter than window: center in window
    balto = TamaY;
    ptr += (((cal - 29 * big2) - TamaY) / 2) * can;
  } else { // Taller than window: center the buffer
    balto = (cal - 29 * big2);
    BuffAux += ((TamaY - (cal - 29 * big2)) / 2) * TamaX;
  }

  for (y = 0; y < balto; y++)
    for (x = 0; x < bancho; x++)
      if (*(BuffAux + y * TamaX + x))
        *(ptr + y * can + x) = *(BuffAux + y * TamaX + x);

  free(MyBuffer);
}


void Preview22() {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void Preview20() {
  int x, fan, _fal = 0, fal, cnt, TX, TY;

  TamaX = 0;
  TamaY = 0;

  spacelen = 0;
  cnt = 0;
  for (x = 0; x < 255; x += 2) {
    get_char_size_buffer(x, &fan, &fal, font_aux);
    if (fan > 1) {
      cnt++;
      spacelen += fan;
    }
    if (_fal < fal)
      _fal = fal;
  }
  if (cnt)
    spacelen = (spacelen / cnt) / 2;
  else
    spacelen = 1;

  for (x = 0; x < strlen(test_string2); x++) {
    get_char_size_buffer(test_string2[x], &fan, &fal, font_aux);
    if (fan <= 1)
      fan = 0;
    TamaX += fan;
    if (TamaY < fal)
      TamaY = fal;
  }
  if (TamaY == 0)
    TamaY = _fal;
  if (TamaX == 0)
    TamaX = 1;
  v.type = 1; // Dialog

  if (big) {
    TX = TamaX / 2;
    TY = TamaY / 2;
  } else {
    TX = TamaX;
    TY = TamaY;
  }
  v.w = TX + 6;
  if (v.w < 60)
    v.w = 60;
  if (v.w > 320)
    v.w = 320;
  v.h = TY + 32;
  if (v.h < 32)
    v.h = 32;
  if (v.h > 200)
    v.h = 200;
  v.paint_handler = Preview21;
  v.click_handler = Preview22;
  v.title = texts[98];
  _button(100, v.w / 2, v.h - 14, 1);
}
/////////////////////////////////////////////////////////////////////////////

char pletras[5 * 8 + 1] = "01234567ABCDEFGHabcdefgh!?$*/+()\xa5\xa4\x87\xa0\x82\xa1\xa2\xa3";

void create_test_text(char *s, char flags) {
  int numbits = 0;
  int numchar = 0, x;

  *s = 0;

  if (flags & NUM)
    numbits++;
  if (flags & MAY)
    numbits++;
  if (flags & MIN)
    numbits++;
  if (flags & SIM)
    numbits++;
  if (flags & EXT)
    numbits++;

  switch (numbits) {
  case 0:
    return;
  case 1:
    numchar = 8;
    break;
  case 2:
    numchar = 4;
    break;
  case 3:
    numchar = 3;
    break;
  case 4:
    numchar = 2;
    break;
  case 5:
    numchar = 2;
    break;
  }

  for (x = 0; x < 5; x++) {
    if (flags & (1 << x)) {
      s[strlen(s) + numchar] = 0;
      memcpy(s + strlen(s), &pletras[x * 8], numchar);
    }
  }
}

void show_font1(void) {
  int w = v.w / big2, h = v.h / big2;
  int ancho_w = 101, alto_w = 22;
  int width, height, pos;
  char cWork[32];

  int n, m, init, x, y, len;
  int fan, _fal = 0, fal, cnt;
  byte *temp, *temp2;
  char saved_char_table[256];
  float coefredy, coefredx, a, b;

  wbox(v.ptr, w, h, c2, 2, 10, w - 4, h - 12);
  wbox(v.ptr, w, h, c1, 2, 10, ancho_w, alto_w);
  wbox(v.ptr, w, h, c0, 2, 32, ancho_w, 1);

  DIV_STRCPY(cWork, "");
  if (v.aux[1352 + RES_FOR_NAME] & NUM)
    DIV_STRCAT(cWork, (char *)texts[166]);
  if (v.aux[1352 + RES_FOR_NAME] & MAY)
    DIV_STRCAT(cWork, (char *)texts[167]);
  if (v.aux[1352 + RES_FOR_NAME] & MIN)
    DIV_STRCAT(cWork, (char *)texts[168]);
  if (v.aux[1352 + RES_FOR_NAME] & SIM)
    DIV_STRCAT(cWork, (char *)texts[169]);
  if (v.aux[1352 + RES_FOR_NAME] & EXT)
    DIV_STRCAT(cWork, (char *)texts[170]);

  wwrite(v.ptr, w, h, w / 2 + 2, 34, 1, (byte *)cWork, c4);

  memcpy(saved_char_table, char_table, 256);

  create_test_text(test_string2, v.aux[1352 + RES_FOR_NAME]);

  memset(char_table, 0, 256);
  for (x = 0; x < strlen(test_string2); x++)
    char_table[test_string2[x]] = 1;

  TamaX = 0;
  TamaY = 0;

  spacelen = 0;
  cnt = 0;
  for (x = 0; x < 255; x += 2) {
    get_char_size_buffer(x, &fan, &fal, (char *)&v.aux[RES_FOR_NAME]);
    if (fan > 1) {
      cnt++;
      spacelen += fan;
    }
    if (_fal < fal)
      _fal = fal;
  }
  if (cnt)
    spacelen = (spacelen / cnt) / 2;
  else
    spacelen = 1;

  for (x = 0; x < strlen(test_string2); x++) {
    get_char_size_buffer(test_string2[x], &fan, &fal, (char *)&v.aux[RES_FOR_NAME]);
    if (fan <= 1)
      fan = 0;
    TamaX += fan;
    if (TamaY < fal)
      TamaY = fal + 1;
  }
  if (TamaX == 0)
    TamaX = 1;
  if (TamaY == 0)
    TamaY = _fal;

  memcpy(char_table, saved_char_table, 256);

  if ((temp = (byte *)malloc(TamaX * TamaY)) == NULL)
    return;

  memset(temp, c1, TamaX * TamaY);

  init = 0;
  for (x = 0; x < strlen(test_string2); x++) {
    len = show_char_buffer(test_string2[x], init, 0, (char *)temp, TamaX,
                           (char *)&v.aux[RES_FOR_NAME]);
    if (len <= 1)
      len = 0;
    init += len;
  }

  if (TamaX > ancho_w * big2 || TamaY > alto_w * big2) {
    // Create the thumbnail reduction
    coefredx = TamaX / ((float)ancho_w * (float)big2);
    coefredy = TamaY / ((float)alto_w * (float)big2);
    if (coefredx > coefredy)
      coefredy = coefredx;
    else
      coefredx = coefredy;
    width = (float)TamaX / coefredx + 0.5;
    height = (float)TamaY / coefredy + 0.5;

    width *= 2;
    height *= 2;
    coefredx /= 2;
    coefredy /= 2;

    if ((temp2 = (byte *)malloc(width * height)) == NULL) {
      free(temp);
      return;
    }

    a = coefredy / 2.0;
    for (y = 0; y < height; y++) {
      b = coefredx / 2.0;
      for (x = 0; x < width; x++) {
        temp2[y * width + x] = temp[((memptrsize)a) * TamaX + (memptrsize)b];
        b += coefredx;
      }
      a += coefredy;
    }

    pos = ((big2 * 2) + (big2 * 10) * v.w) + ((ancho_w * big2) - (width / 2)) / 2 +
          (((alto_w * big2) - (height / 2)) / 2) * v.w;

    for (y = 0; y < height; y += 2) {
      for (x = 0; x < width; x += 2) {
        n = *(ghost + temp2[x + y * width] * 256 + temp2[x + 1 + y * width]);
        m = *(ghost + temp2[x + (y + 1) * width] * 256 + temp2[x + 1 + (y + 1) * width]);
        v.ptr[x / 2 + (y / 2) * v.w + pos] = *(ghost + n * 256 + m);
      }
    }
    free(temp2);
  } else {
    width = TamaX;
    height = TamaY;
    pos = ((big2 * 2) + (big2 * 10) * v.w) + ((ancho_w * big2) - width) / 2 +
          (((alto_w * big2) - height) / 2) * v.w;

    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
        v.ptr[y * v.w + x + pos] = temp[y * width + x];
      }
    }
  }

  free(temp);
}

void ShowFont2(void) {
  if (!(mouse_b & 1) && (prev_mouse_buttons & 1) && wmouse_x != -1) {
    prev_mouse_buttons = 0;
    font_aux = (char *)v.aux + RES_FOR_NAME;
    Preview_2();
  }
}

void ShowFont3(void) {
  free(v.aux);
}

void show_font0(void) {
  FILE *file;
  int Length;
  v.type = 104;
  v.w = 105;
  v.h = 44;

  v.paint_handler = show_font1;
  v.click_handler = ShowFont2;
  v.close_handler = ShowFont3;
  file = fopen(Load_FontPathName, "rb");
  if (file == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return;
  }
  fseek(file, 0, SEEK_END);
  Length = ftell(file);
  fseek(file, 0, SEEK_SET);
  v.aux = (byte *)malloc(Length + RES_FOR_NAME);
  if (v.aux == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

  // v.aux -> filename, v.aux+14 -> path, v.aux+RES_FOR_NAME -> FNT file data

  memset(v.aux, 0, RES_FOR_NAME);
  memcpy(v.aux, Load_FontName, strlen(Load_FontName));
  memcpy(v.aux + 14, Load_FontPathName, strlen(Load_FontPathName)); //Full path
  fread(v.aux + RES_FOR_NAME, Length, 1, file);
  convert_fnt_to_pal((char *)v.aux + RES_FOR_NAME);
  fclose(file);
  v.title = v.aux;
  v.name = v.aux;
}

void pal_reload_font(int vn, struct twindow *vntn) {
  FILE *file;
  int Length;
  vn = vn;
  DIV_STRCPY(Load_FontPathName, (char *)vntn->aux + 14);
  DIV_STRCPY(Load_FontName, (char *)vntn->aux);
  free(vntn->aux);
  file = fopen(Load_FontPathName, "rb");
  if (file == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return;
  }
  fseek(file, 0, SEEK_END);
  Length = ftell(file);
  fseek(file, 0, SEEK_SET);
  vntn->aux = (byte *)malloc(Length + RES_FOR_NAME);
  if (vntn->aux == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memset(vntn->aux, 0, RES_FOR_NAME);
  memcpy(vntn->aux, Load_FontName, strlen(Load_FontName));
  memcpy(vntn->aux + 14, Load_FontPathName, strlen(Load_FontPathName)); //Full path
  fread(vntn->aux + RES_FOR_NAME, Length, 1, file);
  convert_fnt_to_pal((char *)vntn->aux + RES_FOR_NAME);
  fclose(file);
}

#define max_archivos 512 // ------------------------------- File listbox
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[max_archivos];
extern int num_taggeds;

void open_font(void) {
  char cwork[8];
  FILE *f;
  int num;

  v_mode = 0;
  v_type = 5;

  DIV_STRCPY(Load_FontPathName, file_types[v_type].path);
  DIV_STRCAT(Load_FontPathName, "/");
  DIV_STRCAT(Load_FontPathName, Load_FontName);

  v_text = (char *)texts[74];
  show_dialog(browser0);
  if (!v_finished)
    return;

  if (!num_taggeds) {
    DIV_STRCPY(full, file_types[v_type].path);
    if (full[strlen(full) - 1] != '/')
      DIV_STRCAT(full, "/");
    DIV_STRCAT(full, input);
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

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      DIV_STRCPY(input, file_list_br.list + file_list_br.item_width * num);
      DIV_STRCPY(full, file_types[v_type].path);
      if (full[strlen(full) - 1] != '/')
        DIV_STRCAT(full, "/");
      DIV_STRCAT(full, input);

      if ((f = fopen(full, "rb")) != NULL) { // A file was selected
        if (fread(cwork, 1, 8, f) == 8) {
          fclose(f);
          if (!strcmp(cwork, "fnt\x1a\x0d\x0a")) {
            if (!v_exists) {
              v_text = (char *)texts[43];
              show_dialog(err0);
            } else {
              div_strcpy(Load_FontName, sizeof(Load_FontName), input);
              DIV_STRCPY(Load_FontPathName, full);
              new_window(show_font0);
            }
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
}

/////////////////////////////////////////////////////////////////////////////
void create_text();

void GetText1(void) {
  _show_items();
}

void GetText2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    end_dialog = 1;
    v_accept = 1;
    break;
  case 2:
    end_dialog = 1;
    v_accept = 0;
    break;
  }
}
void get_text0(void) {
  v.type = 1;
  v.w = 220 + 6;
  v.h = 64 - 14;
  v.title = texts[855] + 1;
  v.name = texts[855] + 1;

  v.paint_handler = GetText1;
  v.click_handler = GetText2;
  DIV_STRCPY(cCharsToPrint, "");
  _get(161, 4, 12, v.w - 8, (byte *)cCharsToPrint, 128, 0, 0);
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  v_accept = 0;
}

void create_text() {
  int n, y, x, TamaX = 0, TamaY = 0, fan, _fal = 0, fal, init, cnt;

  // 1 - Determine the space width

  if (!strlen(cCharsToPrint))
    return;

  spacelen = 0;
  cnt = 0;
  for (x = 0; x < 255; x += 2) {
    get_char_size_buffer(x, &fan, &fal, font_aux);
    if (fan != 1) {
      cnt++;
      spacelen += fan;
    }
    if (_fal < fal)
      _fal = fal;
  }
  spacelen = (spacelen / cnt) / 2;

  // 2 - Calculate the text dimensions in TamaX x TamaY

  for (x = 0; x < strlen(cCharsToPrint); x++) {
    get_char_size_buffer(cCharsToPrint[x], &fan, &fal, font_aux);
    if (fan == 1)
      fan = spacelen;
    TamaX += fan;
    if (TamaY < fal)
      TamaY = fal;
  }
  if (TamaY == 0)
    TamaY = _fal;

  // 3 - Create a map of that size

  map_width = TamaX;
  map_height = TamaY;

  if (new_map(NULL))
    return;

  // 4 - Use show_char_buffer() to render the text into the map

  init = 0;
  for (x = 0; x < strlen(cCharsToPrint); x++)
    if ((cnt = show_char_buffer(cCharsToPrint[x], init, 0, (char *)v.mapa->map, TamaX, font_aux)) !=
        1)
      init += cnt;
    else
      init += spacelen;

  // 5 - Set the map zoom

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

  // 6 - Initialize control points and graphic code

  for (n = 0; n < 512; n++)
    v.mapa->points[n] = -1;

  v.mapa->fpg_code = 0;

  // 7 - Call the paint handler to refresh the window and blit it

  call((void_return_type_t)v.paint_handler);
  blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
}

int Save_Font_session(FILE *file, int n) {
  n += fwrite(test_string, 1, 21, file);

  n += fwrite(&text1_w, 1, 4, file);
  n += fwrite(&text1_h, 1, 4, file);
  n += fwrite(&Text1Col, 1, 4, file);
  if (!Text01)
    Text01 = (char *)malloc(text1_w * text1_h);
  n += fwrite(Text01, 1, text1_w * text1_h, file);

  n += fwrite(&text2_w, 1, 4, file);
  n += fwrite(&text2_h, 1, 4, file);
  n += fwrite(&Text2Col, 1, 4, file);
  if (!Text02)
    Text02 = (char *)malloc(text1_w * text1_h);
  n += fwrite(Text02, 1, text2_w * text2_h, file);

  n += fwrite(&text3_w, 1, 4, file);
  n += fwrite(&text3_h, 1, 4, file);
  n += fwrite(&Text3Col, 1, 4, file);
  if (!Text03)
    Text03 = (char *)malloc(text1_w * text1_h);
  n += fwrite(Text03, 1, text3_w * text3_h, file);

  n += fwrite(&font_height_gen, 1, 4, file);
  n += fwrite(&font_width_gen, 1, 4, file);
  n += fwrite(&OutLine, 1, 4, file);
  n += fwrite(&outline_dir, 1, 4, file);
  n += fwrite(&shadow_x, 1, 4, file);
  n += fwrite(&shadow_y, 1, 4, file);

  n += fwrite(&Mosaico1, 1, 4, file);
  n += fwrite(&Mosaico2, 1, 4, file);
  n += fwrite(&Mosaico3, 1, 4, file);

  n += fwrite(FontName, 1, 14, file);
  n += fwrite(FontPathName, 1, 256, file);

  n += fwrite(FaceName, 1, 14, file);
  n += fwrite(FacePathName, 1, 256, file);

  return (n);
}


void Load_Font_session(FILE *file) {
  int tal = 24, tan = 41;
  float px, py;
  int x, y;

  FontsWindowInUse = 1;

  memset(test_string, 0, 21);
  fread(test_string, 21, 1, file);

  if (big) {
    tan *= 2;
    tal *= 2;
  }
  v.type = 102;
  v.w = 169;
  v.h = 179 - 17;
  v.paint_handler = Fonts1;
  v.click_handler = Fonts2;
  v.close_handler = Fonts3;
  v.title = texts[83];
  v.name = texts[83];
  lfontsizes.created = 0;
  lfontsizes.total_items = 8;

  //---------------------------------------------------------------
  fread(&text1_w, 1, 4, file);
  fread(&text1_h, 1, 4, file);
  fread(&Text1Col, 1, 4, file);

  Text1 = (char *)malloc(tan * tal * 10);
  if (Text1 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  Text01 = (char *)malloc(text1_w * text1_h);
  if (Text01 == NULL) {
    free(Text01);
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  fread(Text01, text1_w * text1_h, 1, file);

  if (text1_w * text1_h == 1)
    memset(Text1, Text1Col, tan * tal * 10);
  else {
    y = 0;
    for (py = 0; py < text1_h; py += text1_h / (float)tal) {
      x = 0;
      for (px = 0; px < text1_w; px += text1_w / (float)tan)
        Text1[y * tan + x++] = Text01[(memptrsize)py * text1_w + (memptrsize)px];
      y++;
    }
  }
  //---------------------------------------------------------------
  fread(&text2_w, 1, 4, file);
  fread(&text2_h, 1, 4, file);
  fread(&Text2Col, 1, 4, file);

  Text2 = (char *)malloc(tan * tal * 10);
  if (Text2 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(Text1);
    free(Text01);
    return;
  }
  Text02 = (char *)malloc(text2_w * text2_h);
  if (Text02 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(Text1);
    free(Text01);
    free(Text2);
    return;
  }
  fread(Text02, text2_w * text2_h, 1, file);
  if (text2_w * text2_h == 1)
    memset(Text2, Text2Col, tan * tal * 10);
  else {
    y = 0;
    for (py = 0; py < text2_h; py += text2_h / (float)tal) {
      x = 0;
      for (px = 0; px < text2_w; px += text2_w / (float)tan)
        Text2[y * tan + x++] = Text02[(memptrsize)py * text2_w + (memptrsize)px];
      y++;
    }
  }
  //---------------------------------------------------------------
  fread(&text3_w, 1, 4, file);
  fread(&text3_h, 1, 4, file);
  fread(&Text3Col, 1, 4, file);

  Text3 = (char *)malloc(tan * tal * 10);
  if (Text3 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(Text1);
    free(Text01);
    free(Text2);
    free(Text02);
    return;
  }
  Text03 = (char *)malloc(text3_w * text3_h);
  if (Text03 == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(Text1);
    free(Text01);
    free(Text2);
    free(Text02);
    free(Text3);
    return;
  }
  fread(Text03, text3_w * text3_h, 1, file);
  if (text3_w * text3_h == 1)
    memset(Text3, Text3Col, tan * tal * 10);
  else {
    y = 0;
    for (py = 0; py < text3_h; py += text3_h / (float)tal) {
      x = 0;
      for (px = 0; px < text3_w; px += text3_w / (float)tan)
        Text3[y * tan + x++] = Text03[(memptrsize)py * text3_w + (memptrsize)px];
      y++;
    }
  }
  //---------------------------------------------------------------

  fread(&font_height_gen, 1, 4, file);
  fread(&font_width_gen, 1, 4, file);

  fread(&OutLine, 1, 4, file);
  fread(&outline_dir, 1, 4, file);

  fread(&shadow_x, 1, 4, file);
  fread(&shadow_y, 1, 4, file);

  fread(&Mosaico1, 1, 4, file);
  fread(&Mosaico2, 1, 4, file);
  fread(&Mosaico3, 1, 4, file);

  fread(FontName, 14, 1, file);
  fread(FontPathName, 256, 1, file);

  fread(FaceName, 14, 1, file);
  fread(FacePathName, 256, 1, file);

  DIV_SPRINTF(font_height_str, "%d", font_height_gen);
  DIV_SPRINTF(font_width_str, "%d", font_width_gen);

  _button(121, 74, 16, 0);
  _button(121, 74, 35, 0);
  _get(133, 88, 11, 19, (byte *)font_width_str, 4, 16, 128);
  _get(134, 88, 30, 19, (byte *)font_height_str, 4, 16, 128);

  _button(111, 7 + 60, 61, 0);  //4
  _button(110, 37 + 60, 61, 0); //5
  _button(111, 7 + 60, 75, 0);  //6 //129
  _button(110, 37 + 60, 75, 0); //7
  _button(111, 67 + 60, 61, 0); //8
  _button(110, 97 + 60, 61, 0); //9
  _button(111, 67 + 60, 75, 0); //10
  _button(110, 97 + 60, 75, 0); //11

  _get(80, 4, v.h - (39), v.w - 16 - text_len(texts[113] + 1), (byte *)test_string, 21, 0, 0);
  _button(113, v.w - 8, v.h - 14 - 18, 2); //14

  _button(120, 7, v.h - 14, 0);       //13
  _button(119, v.w - 8, v.h - 14, 2); //12

  _flag(112, 64, 112, &Mosaico1);
  _flag(112, 124, 112, &Mosaico2);
  _flag(112, 4, 112, &Mosaico3);
}
