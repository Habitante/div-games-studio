#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "charset.h"

//-----------------------------------------------------------------------------
//  Windows-to-MSDOS character conversion table
//-----------------------------------------------------------------------------

char OEM2ANSI[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   164, 0,   0,   0,
    0,   182, 167, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   32,  33,  34,  35,  36,  37,
    38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
    57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,
    76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,
    95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
    114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 199, 252, 233, 226, 228,
    224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197, 201, 230, 198, 244, 246, 242, 251, 249,
    255, 214, 220, 248, 163, 216, 215, 131, 225, 237, 243, 250, 241, 209, 170, 186, 191, 174, 172,
    189, 188, 161, 171, 187, 95,  95,  95,  166, 166, 193, 194, 192, 169, 166, 166, 43,  43,  162,
    165, 43,  43,  45,  45,  43,  45,  43,  227, 195, 43,  43,  45,  45,  166, 45,  43,  164, 240,
    208, 202, 203, 200, 105, 205, 206, 207, 43,  43,  95,  95,  166, 204, 95,  211, 223, 212, 210,
    245, 213, 181, 254, 222, 218, 219, 217, 253, 221, 175, 180, 173, 177, 95,  190, 182, 167, 247,
    184, 176, 168, 183, 185, 179, 178, 95,  32};

fnt_header_t fnt_hdr = {"DIVFNT", 0x1A};
ifs_header_t ifs_hdr;
fnt_table_entry fnt_table[256];
ifs_table_entry ifs_table[256];
IFS ifs;

char *body_tex_buffer, *out_tex_buffer, *shadow_tex_buffer;

char *char_ptr, *ptr_body, *ptr_outline, *ptr_shadow;
char *Buffer = NULL, *Buffer2 = NULL, *Buffer3 = NULL;
char *out_buffer = NULL, *shadow_buffer = NULL;
FILE *file_ifs = NULL, *file_fnt = NULL;
short offset_y, char_h, char_w, ifs_size, incY;
unsigned short real_width, real_height, body_width, body_height, outline_width, outline_height;


void close_and_free_all() {
  if (file_ifs != NULL) {
    fclose(file_ifs);
    file_ifs = NULL;
  }
  if (file_fnt != NULL) {
    fclose(file_fnt);
    file_fnt = NULL;
  }
  if (Buffer)
    free(Buffer);
  if (Buffer2)
    free(Buffer2);
  if (Buffer3)
    free(Buffer3);
  if (shadow_buffer)
    free(shadow_buffer);
  if (out_buffer)
    free(out_buffer);
}


short init_struct() {
  long position;
  if ((file_ifs = fopen(ifs.ifsName, "rb")) == NULL)
    return (IFS_OPEN_ERROR);
  if (fread(&ifs_hdr, sizeof(ifs_hdr), 1, file_ifs) < 1)
    return (IFS_FORMAT_ERROR);
  if (strcmp((char *)ifs_hdr.id, "IFS"))
    return (IFS_FORMAT_ERROR);
  if (ifs.size_x < 8 || ifs.size_x > 128 || ifs.size_y < 8 || ifs.size_y > 128)
    return (IFS_PARAM_ERROR);

  if (ifs.size_x == 8 && ifs.size_y == 8) {
    position = ifs_hdr.offset8;
    ifs_size = 8;
  } else if (ifs.size_x <= 10 && ifs.size_y <= 10) {
    position = ifs_hdr.offset10;
    ifs_size = 10;
  } else if (ifs.size_x <= 12 && ifs.size_y <= 12) {
    position = ifs_hdr.offset12;
    ifs_size = 12;
  } else if (ifs.size_x <= 14 && ifs.size_y <= 14) {
    position = ifs_hdr.offset14;
    ifs_size = 14;
  } else {
    position = ifs_hdr.offset128;
    ifs_size = 128;
  }

  if (fseek(file_ifs, position, SEEK_SET))
    return (IFS_FORMAT_ERROR);
  if (fread(&ifs_table, sizeof(ifs_table), 1, file_ifs) < 1)
    return (IFS_FORMAT_ERROR);

  if (ifs.brightness > 4)
    ifs.brightness = 0;

  return (0);
}


short load_char(short letter) {
  long offset;
  short x, y, j, t;
  char rtbyte, error = 0;
  short pixels;

  offset = ifs_table[letter].offset;
  if (fseek(file_ifs, offset, SEEK_SET))
    error = 1;
  if (fread(&char_h, 2, 1, file_ifs) < 1)
    error = 1;
  if (fread(&offset_y, 2, 1, file_ifs) < 1)
    error = 1;
  if (fread(&char_w, 2, 1, file_ifs) < 1)
    error = 1;
  if (fread(&pixels, 2, 1, file_ifs) < 1)
    error = 1;
  if (error)
    return (IFS_READ_ERROR);
  t = char_w * 8 - pixels;
  char_h += offset_y;

  if (!char_w || !char_h)
    return 0;

  Buffer = (char *)realloc(Buffer, char_h * char_w * 8 + 10);
  if (Buffer == NULL)
    return (IFS_MEM_ERROR);
  memset(Buffer, 0, char_h * char_w * 8 + 10);

  for (y = 0; y < char_h - offset_y; y++)
    for (x = 0; x < char_w; x++) {
      if (fread(&rtbyte, 1, 1, file_ifs) < 1)
        return (IFS_READ_ERROR);
      for (j = 0; j < 8; j++)
        if (rtbyte & (1 << j))
          Buffer[((y + offset_y) * pixels + x * 8 + j) - t] = 255;
    }

  real_width = char_w = pixels;
  real_height = char_h;
  return (0);
}


short scale_char() {
  float width = char_w, height = char_h, target_w, target_h;
  float fact_x, fact_y, x, y, f;
  float x1, x2, y1, y2;
  float weight, acum;
  long pixel;
  int ix, iy, ixx, iyy;
  short cx = 0, cy = 0, flag = 0;
  char *ptr = Buffer, *p;

  target_w = (float)width * ifs.size_x / ifs_size; // target width relative to
                                                   // original size
  target_h = (float)height * ifs.size_y / ifs_size;

  if (ifs.size_x == ifs_size && ifs.size_y == ifs_size) // no scaling needed
  {
    char_ptr = Buffer;
    return (0);
  }

  fact_x = width / target_w;
  fact_y = height / target_h;

  if (fact_x >= 2.0 || fact_y >= 2.0) // scale by halves
  {
    if (fact_x - (int)fact_x == 0.0 && fact_y - (int)fact_y == 0.0)
      flag = 1;

    if (fact_x < 2.0)
      fact_x = 1.0;
    else if (fact_x < 4.0)
      fact_x = 2.0;
    else if (fact_x < 8.0)
      fact_x = 4.0;
    else if (fact_x < 16.0)
      fact_x = 8;

    if (fact_y < 2.0)
      fact_y = 1.0;
    else if (fact_y < 4.0)
      fact_y = 2.0;
    else if (fact_y < 8.0)
      fact_y = 4.0;
    else if (fact_y < 16.0)
      fact_y = 8;

    f = real_width = char_w / fact_x;
    if (f - (int)f > 0.0)
      real_width++;
    f = real_height = char_h / fact_y;
    if (f - (int)f > 0.0)
      real_height++;

    Buffer2 = (char *)realloc(Buffer2, (real_height + 3) * (real_width + 3));
    if (Buffer2 == NULL)
      return (IFS_MEM_ERROR);
    memset(Buffer2, 0, (real_height + 3) * (real_width + 3));
    ptr = Buffer2;

    for (iy = 0; iy < height; iy += fact_y) {
      for (ix = 0; ix < width; ix += fact_x) {
        pixel = 0;
        for (iyy = iy; iyy < iy + (int)fact_y; iyy++)
          for (ixx = ix; ixx < ix + (int)fact_x; ixx++)
            pixel += Buffer[iyy * char_w + ixx];
        pixel /= (fact_x * fact_y);
        if (flag) // filtrar anti-alias
          ptr[cy * real_width + cx++] = pixel > 128 ? 255 : 0;
        else
          ptr[cy * real_width + cx++] = pixel;
      }
      cx = 0;
      cy++;
    }
    if (flag) {
      char_ptr = Buffer2;
      return (0);
    }
    width = real_width;
    height = real_height;
    fact_x = (float)width / target_w;
    fact_y = (float)height / target_h;
  }

  real_width = (int)target_w;
  if (target_w - (int)target_w > 0.5)
    real_width++;
  real_height = (int)target_h;

  Buffer3 = (char *)realloc(Buffer3, (real_width + 3) * (real_height + 3));
  if (Buffer3 == NULL)
    return (IFS_MEM_ERROR);
  memset(Buffer3, 0, (real_width + 3) * (real_height + 3));
  cx = 0;
  cy = 0;
  for (y = 0.5; y <= height + 0.9; y += fact_y) {
    for (x = 0.5; x <= width + 0.9; x += fact_x) {
      ix = x;
      x1 = ix + 0.5;
      if (x - ix >= 0.5) {
        x1 = ix + 0.5;
        x2 = ix + 1.5;
      } else {
        x1 = ix - 0.5;
        x2 = ix + 0.5;
      }

      iy = y;
      y1 = iy + 0.5;
      if (y - iy >= 0.5) {
        y1 = iy + 0.5;
        y2 = iy + 1.5;
      } else {
        y1 = iy - 0.5;
        y2 = iy + 0.5;
      }
      p = ptr + (int)y1 * (int)width + (int)x1;

      weight = 2 - ((x - x1) * (x - x1) + (y - y1) * (y - y1));
      acum = weight;
      pixel = *p * weight;
      p += (int)width;

      weight = 2 - ((x - x1) * (x - x1) + (y - y2) * (y - y2));
      acum += weight;
      pixel += *p * weight;
      p -= (int)width - 1;

      weight = 2 - ((x - x2) * (x - x2) + (y - y1) * (y - y1));
      acum += weight;
      pixel += *p * weight;
      p += (int)width;

      weight = 2 - ((x - x2) * (x - x2) + (y - y2) * (y - y2));
      acum += weight;
      pixel += *p * weight;

      pixel = (float)pixel / acum;
      Buffer3[cy * real_width + cx++] = pixel > 128 ? 255 : 0;
    }
    cx = 0;
    cy++;
  }
  char_ptr = Buffer3;
  return (0);
}


short paint_outline() {
  short x, y, i, cont, cont2;
  short total_width = body_width + 4 + ifs.outline * 2;
  short total_height = body_height + 4 + ifs.outline * 2;
  short out_width = body_width + ifs.outline * 2;
  short out_height = body_height + 1 + ifs.outline * 2;
  char *tmp_buffer, *ptr, *ptr2, *pun, white = 0, black = 0, grey;

  tmp_buffer = (char *)malloc(total_width * (total_height + 1));
  out_buffer = (char *)realloc(out_buffer, out_width * out_height);
  if (tmp_buffer == NULL || out_buffer == NULL)
    return (IFS_MEM_ERROR);
  memset(tmp_buffer, 0, total_width * (total_height + 1));
  memset(out_buffer, 0, out_width * out_height);
  ptr = ptr_body;
  ptr_outline = ptr2 = tmp_buffer + ifs.outline + 2 + (ifs.outline + 2) * total_width;
  for (y = 0; y < body_height; y++) {
    memcpy(ptr2, ptr, body_width);
    ptr2 += total_width;
    ptr += body_width;
  }

  switch (ifs.brightness) {
  case 1:
  case 3:
    white = 192;
    black = 64;
    grey = 128;
    break;
  case 2:
  case 4:
    white = 64;
    black = 192;
    grey = 128;
    break;
  }

  pun = out_buffer + ifs.outline + ifs.outline * out_width;

  for (i = 1; i <= ifs.outline; i++) {
    for (y = 0; y < real_height + i * 2; y++) {
      ptr = ptr_outline - i + (y - i) * total_width;
      ptr2 = pun - i + (y - i) * out_width;
      for (x = 0; x < real_width + i * 2; x++, ptr++, ptr2++)
        if (!*ptr) {
          cont = 0;
          if (*(ptr + 1) >= 255 - i)
            cont += 1;
          if (*(ptr + total_width) >= 255 - i)
            cont += 2;
          if (*(ptr - 1) >= 255 - i)
            cont += 4;
          if (*(ptr - total_width) >= 255 - i)
            cont += 8;
          if (cont) {
            *ptr = 254 - i;
            if (!ifs.brightness)
              *ptr2 = 128;
            else if (ifs.brightness == 1 || ifs.brightness == 2)
              switch (cont) {
              case 2:
                if (*(ptr - 1 - total_width) >= 255 - i) {
                  *ptr2 = black;
                } else {
                  *ptr2 = white;
                }
                break;
              case 1:
              case 2 + 1:
              case 4 + 2 + 1:
              case 8 + 2 + 1:
                *ptr2 = white;
                break;

              case 4:
              case 8:
              case 8 + 4:
              case 8 + 4 + 1:
              case 8 + 4 + 2:
                *ptr2 = black;
                break;

              case 4 + 1:
              case 8 + 2:
              case 8 + 4 + 2 + 1:
                *ptr2 = grey;
                break;

              case 4 + 2:
              case 8 + 1:
                cont2 = 0;
                if (*(ptr - 1 - total_width) >= 255 - i)
                  cont2 += 1;
                if (*(ptr + 1 + total_width) >= 255 - i)
                  cont2 += 2;
                switch (cont2) {
                case 0:
                  if (cont == 4 + 2) {
                    if (*(ptr - 2 - 2 * total_width) >= 255 - i ||
                        *(ptr - 3 - 3 * total_width) >= 255 - i)
                      *ptr2 = black;
                    else
                      *ptr2 = grey;
                  } else if (*(ptr - 1 - 2 * total_width) < 255 - i)
                    *ptr2 = white;
                  else if (*(ptr + 2) < 255 - i)
                    *ptr2 = black;
                  else
                    *ptr2 = grey;
                  break;
                case 1:
                  *ptr2 = black;
                  break;
                case 2:
                  *ptr2 = white;
                  break;
                case 2 + 1:
                  *ptr2 = grey;
                  break;
                }
                break;
              }
            else if (ifs.brightness == 3 || ifs.brightness == 4)
              switch (cont) {
              case 1:
              case 8:
              case 8 + 1:
              case 4 + 8 + 1:
              case 8 + 2 + 1:
                *ptr2 = black;
                break;

              case 4:
              case 2:
              case 2 + 4:
              case 2 + 4 + 1:
              case 8 + 4 + 2:
                *ptr2 = white;
                break;

              case 4 + 1:
              case 8 + 2:
              case 8 + 4 + 2 + 1:
                *ptr2 = grey;
                break;

              case 1 + 2:
              case 8 + 4:
                cont2 = 0;
                if (*(ptr - 1 + total_width) >= 255 - i)
                  cont2 += 1;
                if (*(ptr + 1 - total_width) >= 255 - i)
                  cont2 += 2;
                switch (cont2) {
                case 0:
                  if (cont == 1 + 2) {
                    if (*(ptr + 2 - 2 * total_width) >= 255 - i)
                      *ptr2 = black;
                    else
                      *ptr2 = grey;
                  } else if (*(ptr + 1 - 2 * total_width) < 255 - i)
                    *ptr2 = white;
                  else if (*(ptr - 2) < 255 - i)
                    *ptr2 = black;
                  else
                    *ptr2 = grey;
                  break;
                case 1:
                  *ptr2 = white;
                  break;
                case 2:
                  *ptr2 = black;
                  break;
                case 2 + 1:
                  *ptr2 = grey;
                  break;
                }
                break;
              }
          }
        }
    }
  }

  char_ptr = out_buffer;
  real_width += (ifs.outline * 2);
  real_height += (ifs.outline * 2);
  free(tmp_buffer);
  return (0);
}


short paint_shadow() {
  char *ptr, *ptr2;
  short x, y;
  short abs_shadow_x = abs(ifs.shadow_x), abs_shadow_y = abs(ifs.shadow_y);

  shadow_buffer =
      (char *)realloc(shadow_buffer, (real_width + abs_shadow_x + 1) * (real_height + abs_shadow_y + 1));
  if (shadow_buffer == NULL)
    return (IFS_MEM_ERROR);
  memset(shadow_buffer, 0, (real_width + abs_shadow_x + 1) * (real_height + abs_shadow_y + 1));

  if (ifs.shadow_y > 0) {
    ptr2 = shadow_buffer + abs_shadow_y * (real_width + abs_shadow_x);
  } else
    ptr2 = shadow_buffer;
  if (ifs.shadow_x > 0)
    ptr2 += abs_shadow_x;

  ptr_shadow = ptr2;

  ptr = ptr_outline;
  for (y = 0; y < outline_height; y++) {
    for (x = 0; x < outline_width; x++, ptr++, ptr2++)
      if (*ptr)
        *ptr2 = ifs.shadow_tex_color;
    ptr2 += abs_shadow_x;
  }

  char_ptr = shadow_buffer;
  real_width += abs_shadow_x;
  real_height += abs_shadow_y;
  return (0);
}


void texture_body() {
  char *ptr;
  short ix, iy, xx, yy;
  float fact_x, fact_y, x, y;

  if (!ifs.body_tex_w || !ifs.body_tex_h)
    return;

  ptr = ptr_body + incY * body_width;

  if (ifs.body_tex_w == 1 && ifs.body_tex_h == 1) // texture=1 color
  {
    for (iy = 0; iy < body_height - incY; iy++)
      for (ix = 0; ix < body_width; ix++, ptr++)
        if (*ptr)
          *ptr = ifs.body_tex_color;
    return;
  }

  if (ifs.body_tex_mode == 1) // scale texture
  {
    fact_x = (float)ifs.body_tex_w / body_width;
    fact_y = (float)ifs.body_tex_h / (body_height - incY);

    for (y = 0, yy = 0; yy < body_height - incY; y += fact_y, yy++)
      for (x = 0, xx = 0; xx < body_width; x += fact_x, xx++, ptr++)
        if (*ptr)
          *ptr = body_tex_buffer[(int)y * ifs.body_tex_w + (int)x];
  } else // tiled texture
    for (yy = 0, iy = 0; yy < body_height - incY; yy++) {
      for (xx = 0, ix = 0; xx < body_width; xx++, ptr++) {
        if (*ptr)
          *ptr = body_tex_buffer[iy * ifs.body_tex_w + ix];
        if (++ix >= ifs.body_tex_w)
          ix = 0;
      }
      if (++iy >= ifs.body_tex_h)
        iy = 0;
    }
}


void texture_outline() {
  char *ptr, color;
  short x, y, ix, iy;
  float fact_x, fact_y, fx, fy;

  ptr = ptr_outline + incY * outline_width;

  if (ifs.outline_tex_w < 2 && ifs.outline_tex_h < 2) {
    for (y = 0; y < outline_height - incY; y++)
      for (x = 0; x < outline_width; x++, ptr++)
        if (*ptr) {
          if (*ptr < 128)
            *ptr = ifs.shadows[ifs.outline_tex_color];
          else if (*ptr > 128)
            *ptr = ifs.highlights[ifs.outline_tex_color];
          else
            *ptr = ifs.outline_tex_color;
        }
    return;
  }

  if (ifs.outline_tex_mode == 1) // scale texture
  {
    fact_x = (float)ifs.outline_tex_w / outline_width;
    fact_y = (float)ifs.outline_tex_h / (outline_height - incY);

    for (y = 0, fy = 0; y < outline_height - incY; y++, fy += fact_y)
      for (x = 0, fx = 0; x < outline_width; x++, fx += fact_x, ptr++)
        if (*ptr) {
          color = out_tex_buffer[(int)fy * ifs.outline_tex_w + (int)fx];
          if (*ptr < 128)
            *ptr = ifs.shadows[color];
          else if (*ptr > 128)
            *ptr = ifs.highlights[color];
          else
            *ptr = color;
        }

  } else
    for (y = 0, iy = 0; y < outline_height - incY; y++) // tiled texture
    {
      for (x = 0, ix = 0; x < outline_width; x++, ptr++) {
        if (*ptr) {
          color = out_tex_buffer[iy * ifs.outline_tex_w + ix];
          if (*ptr < 128)
            *ptr = ifs.shadows[color];
          else if (*ptr > 128)
            *ptr = ifs.highlights[color];
          else
            *ptr = color;
        }
        if (++ix >= ifs.outline_tex_w)
          ix = 0;
      }
      if (++iy >= ifs.outline_tex_h)
        iy = 0;
    }
}


void texture_shadow() {
  char *ptr;
  short x, y, ix, iy;
  float fact_x, fact_y, fx, fy;
  short abs_shadow_x = abs(ifs.shadow_x);

  if (ifs.shadow_tex_w < 2 && ifs.shadow_tex_h < 2)
    return;

  ptr = ptr_shadow + incY * (outline_width + abs_shadow_x);

  if (ifs.shadow_tex_mode == 1) // scale texture
  {
    fact_x = (float)ifs.shadow_tex_w / outline_width;
    fact_y = (float)ifs.shadow_tex_h / (outline_height - incY);

    for (y = 0, fy = 0; y < outline_height - incY; y++, fy += fact_y) {
      for (x = 0, fx = 0; x < outline_width; x++, fx += fact_x, ptr++)
        if (*ptr)
          *ptr = shadow_tex_buffer[(int)fy * ifs.shadow_tex_w + (int)fx];
      ptr += abs_shadow_x;
    }
  } else
    for (y = 0, iy = 0; y < outline_height - incY; y++) // tiled texture
    {
      for (x = 0, ix = 0; x < outline_width; x++, ptr++) {
        if (*ptr)
          *ptr = shadow_tex_buffer[iy * ifs.shadow_tex_w + ix];
        if (++ix >= ifs.shadow_tex_w)
          ix = 0;
      }
      if (++iy >= ifs.shadow_tex_h)
        iy = 0;
      ptr += abs_shadow_x;
    }
}


void merge_outline_with_body(void) { // Adds the body to the outline
  char *ptr, *ptr2, c, color, real_color;
  short x, y, n;

  ptr = ptr_body - body_width - 1;
  ptr2 = ptr_outline + ifs.outline - 1 + (ifs.outline - 1) * outline_width;
  for (y = -1; y <= body_height; y++) {
    for (x = -1; x <= body_width; x++, ptr++, ptr2++) {
      if (x >= 0 && y >= 0 && x < body_width && y < body_height) {
        if (*ptr) {
          *ptr2 = *ptr;
          continue;
        }
      }
      if ((real_color = *ptr2)) {
        color = 0;
        n = 0;
        if (y < body_height && y >= 0) {
          if (x > 0)
            if ((c = *(ptr - 1))) {
              color = c;
              n++;
            }
          if (x < body_width - 1)
            if ((c = *(ptr + 1))) {
              if (color)
                color = *(ghost + color * 256 + c);
              else
                color = c;
              n++;
            }
        }
        if (x < body_width && x >= 0) {
          if (y < body_height - 1)
            if ((c = *(ptr + body_width))) {
              if (color)
                color = *(ghost + color * 256 + c);
              else
                color = c;
              n++;
            }
          if (y > 0)
            if ((c = *(ptr - body_width))) {
              if (color)
                color = *(ghost + color * 256 + c);
              else
                color = c;
              n++;
            }
        }
        switch (n) {
        case 0:
          break;
        case 1:
          color = *(ghost + real_color * 256 + color);
          real_color = *(ghost + real_color * 256 + color);
          break;
        case 2:
          real_color = *(ghost + real_color * 256 + color);
          break;
        case 3:
        case 4:
          real_color = *(ghost + real_color * 256 + color);
          real_color = *(ghost + real_color * 256 + color);
          break;
        }
        *ptr2 = real_color;
      }
    }
    ptr -= 2;
    ptr2 += ifs.outline * 2 - 2;
  }
}


void merge_shadow_with_rest() {
  char *ptr, *ptr2;
  short x, y;
  short abs_shadow_x = abs(ifs.shadow_x), abs_shadow_y = abs(ifs.shadow_y);

  ptr = ptr_outline;

  if (ifs.shadow_y < 0) {
    ptr2 = shadow_buffer + abs_shadow_y * real_width;
  } else
    ptr2 = shadow_buffer;
  if (ifs.shadow_x < 0)
    ptr2 += abs_shadow_x;

  for (y = 0; y < outline_height; y++) {
    for (x = 0; x < outline_width; x++, ptr++, ptr2++)
      if (*ptr)
        *ptr2 = *ptr;
    ptr2 += abs_shadow_x;
  }
}

int create_font_charset(int gen_code) {
  char error = 0, stop;
  short x, ret;
  short j;

  Buffer = NULL;
  Buffer2 = NULL;
  Buffer3 = NULL;
  out_buffer = NULL;
  shadow_buffer = NULL;
  if ((ret = init_struct())) {
    close_and_free_all();
    return (ret);
  }


  if ((file_fnt = fopen(ifs.fntName, "wb")) == NULL) {
    close_and_free_all();
    return (IFS_OPEN_ERROR);
  }
  fwrite("fnt\x1a\x0d\x0a\x00\x00", 8, 1, file_fnt);
  fwrite(dac, 768, 1, file_fnt);
  fwrite(gradients, sizeof(gradients), 1, file_fnt);
  fwrite(&gen_code, 1, 4, file_fnt);

  if (fwrite(fnt_table, sizeof(fnt_table), 1, file_fnt) < 1) {
    error = 1;
  }

  for (x = 0, error = 0; x < 256 && !error; x++) {
    show_progress((char *)texts[217], x, 256);

    fnt_table[x].width = fnt_table[x].height = 0;
    fnt_table[x].offset = 0;
    if (ifs.table[x]) {
      real_width = 0;
      real_height = 0;
      incY = 0;
      if (x != 0) {
        if ((ret = load_char(x))) {
          close_and_free_all();
          return (ret);
        }
        if (char_h && char_w) {
          if ((ret = scale_char())) {
            show_progress((char *)texts[217], 256, 256);
            close_and_free_all();
            return (ret);
          }
          ptr_outline = ptr_body = char_ptr;
          outline_width = body_width = real_width;
          outline_height = body_height = real_height;

          stop = 0;
          for (incY = 0; !stop && incY < real_height; incY++)
            for (j = 0; !stop && j < real_width; j++)
              if (char_ptr[incY * real_width + j])
                stop = 1;
          incY--;
          if (ifs.outline) {
            if ((ret = paint_outline())) {
              show_progress((char *)texts[217], 256, 256);
              close_and_free_all();
              return (ret);
            }
            ptr_outline = char_ptr;
            outline_width = real_width;
            outline_height = real_height;
            texture_body();
            texture_outline();
            merge_outline_with_body();
          } else {
            texture_body();
          }
          if (ifs.shadow_x || ifs.shadow_y) {
            if ((ret = paint_shadow())) {
              show_progress((char *)texts[217], 256, 256);
              close_and_free_all();
              return (ret);
            }
            texture_shadow();
            merge_shadow_with_rest();
          }
        }

        if (real_width <= 0 || incY >= real_height) {
          fnt_table[x].width = 0;
          fnt_table[x].height = 0;
          fnt_table[x].incY = 0;
          fnt_table[x].offset = 0;
        } else {
          fnt_table[x].width = real_width;
          fnt_table[x].height = real_height - incY;
          fnt_table[x].incY = incY;
          fnt_table[x].offset = ftell(file_fnt);
          if (fwrite(char_ptr + incY * real_width, fnt_table[x].width * fnt_table[x].height, 1,
                     file_fnt) < 1)
            error = 1;
        }
      }
    }
  }

  if (error) {
    show_progress((char *)texts[217], 256, 256);
    close_and_free_all();
    return (IFS_WRITE_ERROR);
  }

  fseek(file_fnt, 8 + 768 + sizeof(gradients) + 4, SEEK_SET);
  fwrite(&fnt_table, sizeof(fnt_table), 1, file_fnt);

  show_progress((char *)texts[217], 256, 256);
  close_and_free_all();
  return (0);
}

void get_char_size(int what_char, int *width, int *height) {
  FILE *fnt_file;
  *width = 4;
  *height = 1;

  if ((fnt_file = fopen("PREVIEW.FNT", "rb")) == NULL)
    return;
  fseek(fnt_file, 8 + 768 + sizeof(gradients) + 4, SEEK_SET);
  if (fread(fnt_table, sizeof(fnt_table), 1, fnt_file) < 1) {
    fclose(fnt_file);
    return;
  }
  *height = fnt_table[what_char].incY + fnt_table[what_char].height;
  if (what_char == 32)
    *width = fnt_table[what_char].width / 2;
  else
    *width = fnt_table[what_char].width + 1;
  fclose(fnt_file);
  return;
}
void get_char_size_buffer(int what_char, int *width, int *height, char *buffer) {
  *width = 4;
  *height = 1;

  memcpy(fnt_table, buffer + 8 + 768 + sizeof(gradients) + 4, sizeof(fnt_table));
  *height = fnt_table[what_char].incY + fnt_table[what_char].height;
  *width = fnt_table[what_char].width + 1;
  return;
}
int show_char(int what_char, int cx, int cy, char *ptr, int w) {
  int y, iy;
  FILE *fnt_file;
  char *raw_buffer;
  if ((fnt_file = fopen("PREVIEW.FNT", "rb")) == NULL)
    return 4;
  fseek(fnt_file, 8 + 768 + sizeof(gradients) + 4, SEEK_SET);
  if (fread(fnt_table, sizeof(fnt_table), 1, fnt_file) < 1) {
    fclose(fnt_file);
    return 4;
  }
  if (fnt_table[what_char].width && fnt_table[what_char].height) {
    raw_buffer = (char *)malloc(fnt_table[what_char].width * fnt_table[what_char].height);
    if (raw_buffer == NULL) {
      fclose(fnt_file);
      if (what_char == 32)
        return (fnt_table[what_char].width / 2);
      else
        return (fnt_table[what_char].width + 1);
    }
    if (fseek(fnt_file, fnt_table[what_char].offset, SEEK_SET)) {
      free(raw_buffer);
      fclose(fnt_file);
      if (what_char == 32)
        return (fnt_table[what_char].width / 2);
      else
        return (fnt_table[what_char].width + 1);
    }
    if (fread(raw_buffer, fnt_table[what_char].width * fnt_table[what_char].height, 1, fnt_file) < 1) {
      free(raw_buffer);
      fclose(fnt_file);
      if (what_char == 32)
        return (fnt_table[what_char].width / 2);
      else
        return (fnt_table[what_char].width + 1);
    }
    iy = fnt_table[what_char].incY;
    for (y = 0; y < fnt_table[what_char].height; y++)
      memcpy(ptr + ((cy + iy) * w + cx) + y * w, raw_buffer + y * fnt_table[what_char].width,
             fnt_table[what_char].width);

    fclose(fnt_file);
    free(raw_buffer);
    if (what_char == 32)
      return (fnt_table[what_char].width / 2);
    else
      return (fnt_table[what_char].width + 1);
  } else
    fclose(fnt_file);
  if (what_char == 32)
    return (fnt_table[what_char].width / 2);
  else
    return (fnt_table[what_char].width + 1);
}

int show_char_buffer(int what_char, int cx, int cy, char *ptr, int w, char *buffer) {
  int y, iy, x, c;
  char *raw_buffer;
  memcpy(fnt_table, buffer + 8 + 768 + sizeof(gradients) + 4, sizeof(fnt_table));
  if (fnt_table[what_char].width && fnt_table[what_char].height) {
    raw_buffer = (char *)malloc(fnt_table[what_char].width * fnt_table[what_char].height);
    if (raw_buffer == NULL) {
      if (what_char == 32)
        return (fnt_table[what_char].width / 2);
      else
        return (fnt_table[what_char].width + 1);
    }
    memcpy(raw_buffer, buffer + fnt_table[what_char].offset,
           fnt_table[what_char].width * fnt_table[what_char].height);
    iy = fnt_table[what_char].incY;
    for (y = 0; y < fnt_table[what_char].height; y++) {
      for (x = 0; x < fnt_table[what_char].width; x++) {
        if ((c = raw_buffer[y * fnt_table[what_char].width + x])) {
          ptr[(cy + iy) * w + cx + x + y * w] = c;
        }
      }
    }
    free(raw_buffer);
    if (what_char == 32)
      return (fnt_table[what_char].width / 2);
    else
      return (fnt_table[what_char].width + 1);
  }
  return (fnt_table[what_char].width + 1);
}

void convert_fnt_to_pal(char *buffer) {
  char fnt_dac[768];
  int acum = 0, x, a, b;
  byte xlat[256];
  memcpy(fnt_dac, buffer + 8, 768);

  for (x = 0; x < 768; x++)
    if (dac[x] != fnt_dac[x])
      acum = 1;
  if (!acum)
    return;

  create_dac4();
  xlat[0] = 0;
  for (x = 1; x < 256; x++)
    xlat[x] = find_color_not0(fnt_dac[x * 3], fnt_dac[x * 3 + 1], fnt_dac[x * 3 + 2]);

  memcpy(fnt_table, buffer + 8 + 768 + sizeof(gradients) + 4, sizeof(fnt_table));

  for (x = 0; x < 256; x++) {
    if (fnt_table[x].width && fnt_table[x].height)
      for (a = 0; a < fnt_table[x].height; a++)
        for (b = 0; b < fnt_table[x].width; b++) {
          acum = *(buffer + fnt_table[x].offset + a * fnt_table[x].width + b);
          *(buffer + fnt_table[x].offset + a * fnt_table[x].width + b) = xlat[acum];
        }
  }
  memcpy(buffer + 8, dac, 768);
}
