#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "ifs.h"

//-----------------------------------------------------------------------------
//  Tabla de conversión de chars Windows a chars MSDOS
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

char *char_ptr, *ptrBody, *ptrOutline, *ptrShadow;
char *Buffer = NULL, *Buffer2 = NULL, *Buffer3 = NULL;
char *outBuffer = NULL, *shadowBuffer = NULL;
FILE *file_ifs = NULL, *file_fnt = NULL;
short despY, Alto, Ancho, ifs_size, incY;
unsigned short anchoreal, altoreal, anchoBody, altoBody, outline_width, altoOutline;


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
  if (shadowBuffer)
    free(shadowBuffer);
  if (outBuffer)
    free(outBuffer);
}


short initStruct() {
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


short load_char(short letra) {
  long offset;
  short x, y, j, t;
  char rtbyte, error = 0;
  short pixels;

  offset = ifs_table[letra].offset;
  if (fseek(file_ifs, offset, SEEK_SET))
    error = 1;
  if (fread(&Alto, 2, 1, file_ifs) < 1)
    error = 1;
  if (fread(&despY, 2, 1, file_ifs) < 1)
    error = 1;
  if (fread(&Ancho, 2, 1, file_ifs) < 1)
    error = 1;
  if (fread(&pixels, 2, 1, file_ifs) < 1)
    error = 1;
  if (error)
    return (IFS_READ_ERROR);
  t = Ancho * 8 - pixels;
  Alto += despY;

  if (!Ancho || !Alto)
    return 0;

  Buffer = (char *)realloc(Buffer, Alto * Ancho * 8 + 10);
  if (Buffer == NULL)
    return (IFS_MEM_ERROR);
  memset(Buffer, 0, Alto * Ancho * 8 + 10);

  for (y = 0; y < Alto - despY; y++)
    for (x = 0; x < Ancho; x++) {
      if (fread(&rtbyte, 1, 1, file_ifs) < 1)
        return (IFS_READ_ERROR);
      for (j = 0; j < 8; j++)
        if (rtbyte & (1 << j))
          Buffer[((y + despY) * pixels + x * 8 + j) - t] = 255;
    }

  anchoreal = Ancho = pixels;
  altoreal = Alto;
  return (0);
}


short escalar() {
  float width = Ancho, height = Alto, ancho2, alto2;
  float factX, factY, x, y, f;
  float x1, x2, y1, y2;
  float porcien, acum;
  long pixel;
  int ix, iy, ixx, iyy;
  short cx = 0, cy = 0, flag = 0;
  char *ptr = Buffer, *p;

  ancho2 = (float)width * ifs.size_x / ifs_size; // width real en relación
                                                // al tamaño original
  alto2 = (float)height * ifs.size_y / ifs_size;

  if (ifs.size_x == ifs_size && ifs.size_y == ifs_size) // no hace falta escalar
  {
    char_ptr = Buffer;
    return (0);
  }

  factX = width / ancho2;
  factY = height / alto2;

  if (factX >= 2.0 || factY >= 2.0) // escalar por medias
  {
    if (factX - (int)factX == 0.0 && factY - (int)factY == 0.0)
      flag = 1;

    if (factX < 2.0)
      factX = 1.0;
    else if (factX < 4.0)
      factX = 2.0;
    else if (factX < 8.0)
      factX = 4.0;
    else if (factX < 16.0)
      factX = 8;

    if (factY < 2.0)
      factY = 1.0;
    else if (factY < 4.0)
      factY = 2.0;
    else if (factY < 8.0)
      factY = 4.0;
    else if (factY < 16.0)
      factY = 8;

    f = anchoreal = Ancho / factX;
    if (f - (int)f > 0.0)
      anchoreal++;
    f = altoreal = Alto / factY;
    if (f - (int)f > 0.0)
      altoreal++;

    Buffer2 = (char *)realloc(Buffer2, (altoreal + 3) * (anchoreal + 3));
    if (Buffer2 == NULL)
      return (IFS_MEM_ERROR);
    memset(Buffer2, 0, (altoreal + 3) * (anchoreal + 3));
    ptr = Buffer2;

    for (iy = 0; iy < height; iy += factY) {
      for (ix = 0; ix < width; ix += factX) {
        pixel = 0;
        for (iyy = iy; iyy < iy + (int)factY; iyy++)
          for (ixx = ix; ixx < ix + (int)factX; ixx++)
            pixel += Buffer[iyy * Ancho + ixx];
        pixel /= (factX * factY);
        if (flag) // filtrar anti-alias
          ptr[cy * anchoreal + cx++] = pixel > 128 ? 255 : 0;
        else
          ptr[cy * anchoreal + cx++] = pixel;
      }
      cx = 0;
      cy++;
    }
    if (flag) {
      char_ptr = Buffer2;
      return (0);
    }
    width = anchoreal;
    height = altoreal;
    factX = (float)width / ancho2;
    factY = (float)height / alto2;
  }

  anchoreal = (int)ancho2;
  if (ancho2 - (int)ancho2 > 0.5)
    anchoreal++;
  altoreal = (int)alto2;

  Buffer3 = (char *)realloc(Buffer3, (anchoreal + 3) * (altoreal + 3));
  if (Buffer3 == NULL)
    return (IFS_MEM_ERROR);
  memset(Buffer3, 0, (anchoreal + 3) * (altoreal + 3));
  cx = 0;
  cy = 0;
  for (y = 0.5; y <= height + 0.9; y += factY) {
    for (x = 0.5; x <= width + 0.9; x += factX) {
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

      porcien = 2 - ((x - x1) * (x - x1) + (y - y1) * (y - y1));
      acum = porcien;
      pixel = *p * porcien;
      p += (int)width;

      porcien = 2 - ((x - x1) * (x - x1) + (y - y2) * (y - y2));
      acum += porcien;
      pixel += *p * porcien;
      p -= (int)width - 1;

      porcien = 2 - ((x - x2) * (x - x2) + (y - y1) * (y - y1));
      acum += porcien;
      pixel += *p * porcien;
      p += (int)width;

      porcien = 2 - ((x - x2) * (x - x2) + (y - y2) * (y - y2));
      acum += porcien;
      pixel += *p * porcien;

      pixel = (float)pixel / acum;
      Buffer3[cy * anchoreal + cx++] = pixel > 128 ? 255 : 0;
    }
    cx = 0;
    cy++;
  }
  char_ptr = Buffer3;
  return (0);
}


short paint_outline() {
  short x, y, i, cont, cont2;
  short anchoTotal = anchoBody + 4 + ifs.outline * 2;
  short altoTotal = altoBody + 4 + ifs.outline * 2;
  short anchoOut = anchoBody + ifs.outline * 2;
  short altoOut = altoBody + 1 + ifs.outline * 2;
  char *tmpBuffer, *ptr, *ptr2, *pun, blanco = 0, negro = 0, gris;

  tmpBuffer = (char *)malloc(anchoTotal * (altoTotal + 1));
  outBuffer = (char *)realloc(outBuffer, anchoOut * altoOut);
  if (tmpBuffer == NULL || outBuffer == NULL)
    return (IFS_MEM_ERROR);
  memset(tmpBuffer, 0, anchoTotal * (altoTotal + 1));
  memset(outBuffer, 0, anchoOut * altoOut);
  ptr = ptrBody;
  ptrOutline = ptr2 = tmpBuffer + ifs.outline + 2 + (ifs.outline + 2) * anchoTotal;
  for (y = 0; y < altoBody; y++) {
    memcpy(ptr2, ptr, anchoBody);
    ptr2 += anchoTotal;
    ptr += anchoBody;
  }

  switch (ifs.brightness) {
  case 1:
  case 3:
    blanco = 192;
    negro = 64;
    gris = 128;
    break;
  case 2:
  case 4:
    blanco = 64;
    negro = 192;
    gris = 128;
    break;
  }

  pun = outBuffer + ifs.outline + ifs.outline * anchoOut;

  for (i = 1; i <= ifs.outline; i++) {
    for (y = 0; y < altoreal + i * 2; y++) {
      ptr = ptrOutline - i + (y - i) * anchoTotal;
      ptr2 = pun - i + (y - i) * anchoOut;
      for (x = 0; x < anchoreal + i * 2; x++, ptr++, ptr2++)
        if (!*ptr) {
          cont = 0;
          if (*(ptr + 1) >= 255 - i)
            cont += 1;
          if (*(ptr + anchoTotal) >= 255 - i)
            cont += 2;
          if (*(ptr - 1) >= 255 - i)
            cont += 4;
          if (*(ptr - anchoTotal) >= 255 - i)
            cont += 8;
          if (cont) {
            *ptr = 254 - i;
            if (!ifs.brightness)
              *ptr2 = 128;
            else if (ifs.brightness == 1 || ifs.brightness == 2)
              switch (cont) {
              case 2:
                if (*(ptr - 1 - anchoTotal) >= 255 - i) {
                  *ptr2 = negro;
                } else {
                  *ptr2 = blanco;
                }
                break;
              case 1:
              case 2 + 1:
              case 4 + 2 + 1:
              case 8 + 2 + 1:
                *ptr2 = blanco;
                break;

              case 4:
              case 8:
              case 8 + 4:
              case 8 + 4 + 1:
              case 8 + 4 + 2:
                *ptr2 = negro;
                break;

              case 4 + 1:
              case 8 + 2:
              case 8 + 4 + 2 + 1:
                *ptr2 = gris;
                break;

              case 4 + 2:
              case 8 + 1:
                cont2 = 0;
                if (*(ptr - 1 - anchoTotal) >= 255 - i)
                  cont2 += 1;
                if (*(ptr + 1 + anchoTotal) >= 255 - i)
                  cont2 += 2;
                switch (cont2) {
                case 0:
                  if (cont == 4 + 2) {
                    if (*(ptr - 2 - 2 * anchoTotal) >= 255 - i ||
                        *(ptr - 3 - 3 * anchoTotal) >= 255 - i)
                      *ptr2 = negro;
                    else
                      *ptr2 = gris;
                  } else if (*(ptr - 1 - 2 * anchoTotal) < 255 - i)
                    *ptr2 = blanco;
                  else if (*(ptr + 2) < 255 - i)
                    *ptr2 = negro;
                  else
                    *ptr2 = gris;
                  break;
                case 1:
                  *ptr2 = negro;
                  break;
                case 2:
                  *ptr2 = blanco;
                  break;
                case 2 + 1:
                  *ptr2 = gris;
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
                *ptr2 = negro;
                break;

              case 4:
              case 2:
              case 2 + 4:
              case 2 + 4 + 1:
              case 8 + 4 + 2:
                *ptr2 = blanco;
                break;

              case 4 + 1:
              case 8 + 2:
              case 8 + 4 + 2 + 1:
                *ptr2 = gris;
                break;

              case 1 + 2:
              case 8 + 4:
                cont2 = 0;
                if (*(ptr - 1 + anchoTotal) >= 255 - i)
                  cont2 += 1;
                if (*(ptr + 1 - anchoTotal) >= 255 - i)
                  cont2 += 2;
                switch (cont2) {
                case 0:
                  if (cont == 1 + 2) {
                    if (*(ptr + 2 - 2 * anchoTotal) >= 255 - i)
                      *ptr2 = negro;
                    else
                      *ptr2 = gris;
                  } else if (*(ptr + 1 - 2 * anchoTotal) < 255 - i)
                    *ptr2 = blanco;
                  else if (*(ptr - 2) < 255 - i)
                    *ptr2 = negro;
                  else
                    *ptr2 = gris;
                  break;
                case 1:
                  *ptr2 = blanco;
                  break;
                case 2:
                  *ptr2 = negro;
                  break;
                case 2 + 1:
                  *ptr2 = gris;
                  break;
                }
                break;
              }
          }
        }
    }
  }

  char_ptr = outBuffer;
  anchoreal += (ifs.outline * 2);
  altoreal += (ifs.outline * 2);
  free(tmpBuffer);
  return (0);
}


short paint_shadow() {
  char *ptr, *ptr2;
  short x, y;
  short absSombraX = abs(ifs.shadow_x), absSombraY = abs(ifs.shadow_y);

  shadowBuffer =
      (char *)realloc(shadowBuffer, (anchoreal + absSombraX + 1) * (altoreal + absSombraY + 1));
  if (shadowBuffer == NULL)
    return (IFS_MEM_ERROR);
  memset(shadowBuffer, 0, (anchoreal + absSombraX + 1) * (altoreal + absSombraY + 1));

  if (ifs.shadow_y > 0) {
    ptr2 = shadowBuffer + absSombraY * (anchoreal + absSombraX);
  } else
    ptr2 = shadowBuffer;
  if (ifs.shadow_x > 0)
    ptr2 += absSombraX;

  ptrShadow = ptr2;

  ptr = ptrOutline;
  for (y = 0; y < altoOutline; y++) {
    for (x = 0; x < outline_width; x++, ptr++, ptr2++)
      if (*ptr)
        *ptr2 = ifs.shadow_tex_color;
    ptr2 += absSombraX;
  }

  char_ptr = shadowBuffer;
  anchoreal += absSombraX;
  altoreal += absSombraY;
  return (0);
}


void texturarBody() {
  char *ptr;
  short ix, iy, xx, yy;
  float factX, factY, x, y;

  if (!ifs.body_tex_w || !ifs.body_tex_h)
    return;

  ptr = ptrBody + incY * anchoBody;

  if (ifs.body_tex_w == 1 && ifs.body_tex_h == 1) // textura=1 color
  {
    for (iy = 0; iy < altoBody - incY; iy++)
      for (ix = 0; ix < anchoBody; ix++, ptr++)
        if (*ptr)
          *ptr = ifs.body_tex_color;
    return;
  }

  if (ifs.body_tex_mode == 1) // escalar textura
  {
    factX = (float)ifs.body_tex_w / anchoBody;
    factY = (float)ifs.body_tex_h / (altoBody - incY);

    for (y = 0, yy = 0; yy < altoBody - incY; y += factY, yy++)
      for (x = 0, xx = 0; xx < anchoBody; x += factX, xx++, ptr++)
        if (*ptr)
          *ptr = body_tex_buffer[(int)y * ifs.body_tex_w + (int)x];
  } else // textura en tile
    for (yy = 0, iy = 0; yy < altoBody - incY; yy++) {
      for (xx = 0, ix = 0; xx < anchoBody; xx++, ptr++) {
        if (*ptr)
          *ptr = body_tex_buffer[iy * ifs.body_tex_w + ix];
        if (++ix >= ifs.body_tex_w)
          ix = 0;
      }
      if (++iy >= ifs.body_tex_h)
        iy = 0;
    }
}


void texturarOutline() {
  char *ptr, color;
  short x, y, ix, iy;
  float factX, factY, fx, fy;

  ptr = ptrOutline + incY * outline_width;

  if (ifs.outline_tex_w < 2 && ifs.outline_tex_h < 2) {
    for (y = 0; y < altoOutline - incY; y++)
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

  if (ifs.outline_tex_mode == 1) // escalar textura
  {
    factX = (float)ifs.outline_tex_w / outline_width;
    factY = (float)ifs.outline_tex_h / (altoOutline - incY);

    for (y = 0, fy = 0; y < altoOutline - incY; y++, fy += factY)
      for (x = 0, fx = 0; x < outline_width; x++, fx += factX, ptr++)
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
    for (y = 0, iy = 0; y < altoOutline - incY; y++) // textura en tile
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


void texturarSombra() {
  char *ptr;
  short x, y, ix, iy;
  float factX, factY, fx, fy;
  short absSombraX = abs(ifs.shadow_x);

  if (ifs.shadow_tex_w < 2 && ifs.shadow_tex_h < 2)
    return;

  ptr = ptrShadow + incY * (outline_width + absSombraX);

  if (ifs.shadow_tex_mode == 1) // escalar textura
  {
    factX = (float)ifs.shadow_tex_w / outline_width;
    factY = (float)ifs.shadow_tex_h / (altoOutline - incY);

    for (y = 0, fy = 0; y < altoOutline - incY; y++, fy += factY) {
      for (x = 0, fx = 0; x < outline_width; x++, fx += factX, ptr++)
        if (*ptr)
          *ptr = shadow_tex_buffer[(int)fy * ifs.shadow_tex_w + (int)fx];
      ptr += absSombraX;
    }
  } else
    for (y = 0, iy = 0; y < altoOutline - incY; y++) // textura en tile
    {
      for (x = 0, ix = 0; x < outline_width; x++, ptr++) {
        if (*ptr)
          *ptr = shadow_tex_buffer[iy * ifs.shadow_tex_w + ix];
        if (++ix >= ifs.shadow_tex_w)
          ix = 0;
      }
      if (++iy >= ifs.shadow_tex_h)
        iy = 0;
      ptr += absSombraX;
    }
}


void unirOutlineConBody(void) { // Le añade el body al outline
  char *ptr, *ptr2, c, color, realcolor;
  short x, y, n;

  ptr = ptrBody - anchoBody - 1;
  ptr2 = ptrOutline + ifs.outline - 1 + (ifs.outline - 1) * outline_width;
  for (y = -1; y <= altoBody; y++) {
    for (x = -1; x <= anchoBody; x++, ptr++, ptr2++) {
      if (x >= 0 && y >= 0 && x < anchoBody && y < altoBody) {
        if (*ptr) {
          *ptr2 = *ptr;
          continue;
        }
      }
      if ((realcolor = *ptr2)) {
        color = 0;
        n = 0;
        if (y < altoBody && y >= 0) {
          if (x > 0)
            if ((c = *(ptr - 1))) {
              color = c;
              n++;
            }
          if (x < anchoBody - 1)
            if ((c = *(ptr + 1))) {
              if (color)
                color = *(ghost + color * 256 + c);
              else
                color = c;
              n++;
            }
        }
        if (x < anchoBody && x >= 0) {
          if (y < altoBody - 1)
            if ((c = *(ptr + anchoBody))) {
              if (color)
                color = *(ghost + color * 256 + c);
              else
                color = c;
              n++;
            }
          if (y > 0)
            if ((c = *(ptr - anchoBody))) {
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
          color = *(ghost + realcolor * 256 + color);
          realcolor = *(ghost + realcolor * 256 + color);
          break;
        case 2:
          realcolor = *(ghost + realcolor * 256 + color);
          break;
        case 3:
        case 4:
          realcolor = *(ghost + realcolor * 256 + color);
          realcolor = *(ghost + realcolor * 256 + color);
          break;
        }
        *ptr2 = realcolor;
      }
    }
    ptr -= 2;
    ptr2 += ifs.outline * 2 - 2;
  }
}


void unirSombraConResto() {
  char *ptr, *ptr2;
  short x, y;
  short absSombraX = abs(ifs.shadow_x), absSombraY = abs(ifs.shadow_y);

  ptr = ptrOutline;

  if (ifs.shadow_y < 0) {
    ptr2 = shadowBuffer + absSombraY * anchoreal;
  } else
    ptr2 = shadowBuffer;
  if (ifs.shadow_x < 0)
    ptr2 += absSombraX;

  for (y = 0; y < altoOutline; y++) {
    for (x = 0; x < outline_width; x++, ptr++, ptr2++)
      if (*ptr)
        *ptr2 = *ptr;
    ptr2 += absSombraX;
  }
}

int jorge_create_font(int GenCode) {
  char error = 0, stop;
  short x, ret;
  short j;

  Buffer = NULL;
  Buffer2 = NULL;
  Buffer3 = NULL;
  outBuffer = NULL;
  shadowBuffer = NULL;
  if ((ret = initStruct())) {
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
  fwrite(&GenCode, 1, 4, file_fnt);

  if (fwrite(fnt_table, sizeof(fnt_table), 1, file_fnt) < 1) {
    error = 1;
  }

  for (x = 0, error = 0; x < 256 && !error; x++) {
    show_progress((char *)texts[217], x, 256);

    fnt_table[x].width = fnt_table[x].height = 0;
    fnt_table[x].offset = 0;
    if (ifs.table[x]) {
      anchoreal = 0;
      altoreal = 0;
      incY = 0;
      if (x != 0) {
        if ((ret = load_char(x))) {
          close_and_free_all();
          return (ret);
        }
        if (Alto && Ancho) {
          if ((ret = escalar())) {
            show_progress((char *)texts[217], 256, 256);
            close_and_free_all();
            return (ret);
          }
          ptrOutline = ptrBody = char_ptr;
          outline_width = anchoBody = anchoreal;
          altoOutline = altoBody = altoreal;

          stop = 0;
          for (incY = 0; !stop && incY < altoreal; incY++)
            for (j = 0; !stop && j < anchoreal; j++)
              if (char_ptr[incY * anchoreal + j])
                stop = 1;
          incY--;
          if (ifs.outline) {
            if ((ret = paint_outline())) {
              show_progress((char *)texts[217], 256, 256);
              close_and_free_all();
              return (ret);
            }
            ptrOutline = char_ptr;
            outline_width = anchoreal;
            altoOutline = altoreal;
            texturarBody();
            texturarOutline();
            unirOutlineConBody();
          } else {
            texturarBody();
          }
          if (ifs.shadow_x || ifs.shadow_y) {
            if ((ret = paint_shadow())) {
              show_progress((char *)texts[217], 256, 256);
              close_and_free_all();
              return (ret);
            }
            texturarSombra();
            unirSombraConResto();
          }
        }

        if (anchoreal <= 0 || incY >= altoreal) {
          fnt_table[x].width = 0;
          fnt_table[x].height = 0;
          fnt_table[x].incY = 0;
          fnt_table[x].offset = 0;
        } else {
          fnt_table[x].width = anchoreal;
          fnt_table[x].height = altoreal - incY;
          fnt_table[x].incY = incY;
          fnt_table[x].offset = ftell(file_fnt);
          if (fwrite(char_ptr + incY * anchoreal, fnt_table[x].width * fnt_table[x].height, 1,
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

void get_char_size(int WhatChar, int *width, int *height) {
  FILE *fichFnt;
  *width = 4;
  *height = 1;

  if ((fichFnt = fopen("PREVIEW.FNT", "rb")) == NULL)
    return;
  fseek(fichFnt, 8 + 768 + sizeof(gradients) + 4, SEEK_SET);
  if (fread(fnt_table, sizeof(fnt_table), 1, fichFnt) < 1) {
    fclose(fichFnt);
    return;
  }
  *height = fnt_table[WhatChar].incY + fnt_table[WhatChar].height;
  if (WhatChar == 32)
    *width = fnt_table[WhatChar].width / 2;
  else
    *width = fnt_table[WhatChar].width + 1;
  fclose(fichFnt);
  return;
}
void get_char_size_buffer(int WhatChar, int *width, int *height, char *buffer) {
  *width = 4;
  *height = 1;

  memcpy(fnt_table, buffer + 8 + 768 + sizeof(gradients) + 4, sizeof(fnt_table));
  *height = fnt_table[WhatChar].incY + fnt_table[WhatChar].height;
  *width = fnt_table[WhatChar].width + 1;
  return;
}
int show_char(int WhatChar, int cx, int cy, char *ptr, int w) {
  int y, iy;
  FILE *fichFnt;
  char *rawBuffer;
  if ((fichFnt = fopen("PREVIEW.FNT", "rb")) == NULL)
    return 4;
  fseek(fichFnt, 8 + 768 + sizeof(gradients) + 4, SEEK_SET);
  if (fread(fnt_table, sizeof(fnt_table), 1, fichFnt) < 1) {
    fclose(fichFnt);
    return 4;
  }
  if (fnt_table[WhatChar].width && fnt_table[WhatChar].height) {
    rawBuffer = (char *)malloc(fnt_table[WhatChar].width * fnt_table[WhatChar].height);
    if (rawBuffer == NULL) {
      fclose(fichFnt);
      if (WhatChar == 32)
        return (fnt_table[WhatChar].width / 2);
      else
        return (fnt_table[WhatChar].width + 1);
    }
    if (fseek(fichFnt, fnt_table[WhatChar].offset, SEEK_SET)) {
      free(rawBuffer);
      fclose(fichFnt);
      if (WhatChar == 32)
        return (fnt_table[WhatChar].width / 2);
      else
        return (fnt_table[WhatChar].width + 1);
    }
    if (fread(rawBuffer, fnt_table[WhatChar].width * fnt_table[WhatChar].height, 1, fichFnt) < 1) {
      free(rawBuffer);
      fclose(fichFnt);
      if (WhatChar == 32)
        return (fnt_table[WhatChar].width / 2);
      else
        return (fnt_table[WhatChar].width + 1);
    }
    iy = fnt_table[WhatChar].incY;
    for (y = 0; y < fnt_table[WhatChar].height; y++)
      memcpy(ptr + ((cy + iy) * w + cx) + y * w, rawBuffer + y * fnt_table[WhatChar].width,
             fnt_table[WhatChar].width);

    fclose(fichFnt);
    free(rawBuffer);
    if (WhatChar == 32)
      return (fnt_table[WhatChar].width / 2);
    else
      return (fnt_table[WhatChar].width + 1);
  } else
    fclose(fichFnt);
  if (WhatChar == 32)
    return (fnt_table[WhatChar].width / 2);
  else
    return (fnt_table[WhatChar].width + 1);
}

int show_char_buffer(int WhatChar, int cx, int cy, char *ptr, int w, char *buffer) {
  int y, iy, x, c;
  char *rawBuffer;
  memcpy(fnt_table, buffer + 8 + 768 + sizeof(gradients) + 4, sizeof(fnt_table));
  if (fnt_table[WhatChar].width && fnt_table[WhatChar].height) {
    rawBuffer = (char *)malloc(fnt_table[WhatChar].width * fnt_table[WhatChar].height);
    if (rawBuffer == NULL) {
      if (WhatChar == 32)
        return (fnt_table[WhatChar].width / 2);
      else
        return (fnt_table[WhatChar].width + 1);
    }
    memcpy(rawBuffer, buffer + fnt_table[WhatChar].offset,
           fnt_table[WhatChar].width && fnt_table[WhatChar].height);
    iy = fnt_table[WhatChar].incY;
    for (y = 0; y < fnt_table[WhatChar].height; y++) {
      for (x = 0; x < fnt_table[WhatChar].width; x++) {
        if ((c = rawBuffer[y * fnt_table[WhatChar].width + x])) {
          ptr[(cy + iy) * w + cx + x + y * w] = c;
        }
      }
    }
    free(rawBuffer);
    if (WhatChar == 32)
      return (fnt_table[WhatChar].width / 2);
    else
      return (fnt_table[WhatChar].width + 1);
  }
  return (fnt_table[WhatChar].width + 1);
}

void convert_fnt_to_pal(char *buffer) {
  char DacFnt[768];
  int acum = 0, x, a, b;
  byte xlat[256];
  memcpy(DacFnt, buffer + 8, 768);

  for (x = 0; x < 768; x++)
    if (dac[x] != DacFnt[x])
      acum = 1;
  if (!acum)
    return;

  create_dac4();
  xlat[0] = 0;
  for (x = 1; x < 256; x++)
    xlat[x] = find_color_not0(DacFnt[x * 3], DacFnt[x * 3 + 1], DacFnt[x * 3 + 2]);

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
