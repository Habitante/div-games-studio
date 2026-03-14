//-----------------------------------------------------------------------------
//      Font map generation (print_fontmap, generate_fontmap, generate_character)
//-----------------------------------------------------------------------------

#include "handler_internal.h"

extern char *font_aux;

//-----------------------------------------------------------------------------
//      Functions for generate_fontmap() / print_fontmap()
//-----------------------------------------------------------------------------

#define MAP_PIXEL(x, y) (*(map + (y) * map_width + (x)))
void show_font0(void);

extern char Load_FontPathName[256];
extern char Load_FontName[14];

void print_fontmap(void) {
  char font_path_name[256], font_name[14] = "";
  char *buffer, *di, color;
  int buffer_len, x, y, _x0, x0 = 1, y0 = 1, chars, *p;
  int gencode;
  FILE *f;

  v_mode = 1;
  v_type = FT_FNT;

  div_strcpy(font_path_name, sizeof(font_path_name), file_types[v_type].path);
  div_strcat(font_path_name, sizeof(font_path_name), "/");
  div_strcat(font_path_name, sizeof(font_path_name), font_name);

  v_text = (char *)texts[264];
  show_dialog(browser0);

  if (!v_finished)
    return;

  if (v_exists) {
    v_title = (char *)texts[75];
    v_text = (char *)texts[76];
    show_dialog(accept0);
    if (!v_accept)
      return;
  }

  div_strcpy(font_name, sizeof(font_name), input);
  DIV_STRCPY(font_path_name, file_types[v_type].path);
  if (!IS_PATH_SEP(font_path_name[strlen(font_path_name) - 1]))
    div_strcat(font_path_name, sizeof(font_path_name), "/");
  div_strcat(font_path_name, sizeof(font_path_name), input); // * font_path_name full path of the file

  buffer_len = FNT_TABLE_OFFSET + 256 * 16 + map_width * map_height; // Allocate enough memory for the FNT

  if ((buffer = (char *)malloc(buffer_len)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return; // Not enought memory, close ...
  }

  memcpy(buffer, "fnt\x1a\x0d\x0a\x00", 8);
  memcpy(buffer + 8, dac, PALETTE_SIZE);
  memcpy(buffer + 776, gradients, sizeof(gradients));
  memset(buffer + FNT_TABLE_OFFSET, 0, 256 * 16);

  di = buffer + FNT_TABLE_OFFSET + 256 * 16;
  color = *map;
  chars = 0;
  p = (int *)(buffer + FNT_TABLE_OFFSET);

  for (x = 1; x < map_width - 1; x++) {
    for (y = 1; y < map_height - 1; y++) {
      if (MAP_PIXEL(x - 1, y - 1) == color && MAP_PIXEL(x - 1, y) == color &&
          MAP_PIXEL(x, y - 1) == color && MAP_PIXEL(x, y) != color) {
        x0 = x;
        y0 = y;
      }
      if (MAP_PIXEL(x, y) != color && MAP_PIXEL(x + 1, y + 1) == color &&
          MAP_PIXEL(x + 1, y) == color && MAP_PIXEL(x, y + 1) == color) {
        if (x == x0 && y == y0) {
          *p++ = 0;
          *p++ = 0;
          *p++ = 0;
          *p++ = 0;
        } else {
          *p++ = x - x0 + 1;
          *p++ = y - y0 + 1;
          *p++ = y0 - 1;
          *p++ = di - buffer;
          for (; y0 <= y; y0++)
            for (_x0 = x0; _x0 <= x; _x0++)
              *di++ = MAP_PIXEL(_x0, y0);
        }
        if (chars++ == 256)
          goto end_bucle;
        if ((x += 2) >= map_width) {
          goto end_bucle;
        }
        y = 0;
      }
    }
  }

end_bucle:

  if (chars != 256) {
    v_text = (char *)texts[243];
    show_dialog(err0);
    free(buffer);
    return;
  }

  gencode = 0;
  if (*(int *)(buffer + FNT_TABLE_OFFSET + '0' * 16))
    gencode |= 1;
  if (*(int *)(buffer + FNT_TABLE_OFFSET + 'A' * 16))
    gencode |= 2;
  if (*(int *)(buffer + FNT_TABLE_OFFSET + 'a' * 16))
    gencode |= 4;
  if (*(int *)(buffer + FNT_TABLE_OFFSET + '?' * 16))
    gencode |= 8;
  if (*(int *)(buffer + FNT_TABLE_OFFSET + '\xa4' * 16))
    gencode |= 16;

  memcpy(buffer + FNT_GENCODE_OFFSET, &gencode, 4);

  if ((f = fopen(font_path_name, "wb")) == NULL) {
    v_text = (char *)texts[242];
    show_dialog(err0);
    free(buffer);
    return;
  }

  if (fwrite(buffer, 1, di - buffer, f) != di - buffer) {
    v_text = (char *)texts[242];
    show_dialog(err0);
    fclose(f);
    free(buffer);
    return;
  }

  fclose(f);
  free(buffer);

  // Create the window and close the old if available


  for (x = -1, y = 1; y < MAX_WINDOWS; y++)
    if (window[y].type == WIN_FONT && !strcmp((char *)window[y].title, font_name))
      x = y;

  if (x >= 0) {
    move(0, x);
    close_window();
  }

  div_strcpy(Load_FontName, sizeof(Load_FontName), font_name);
  div_strcpy(Load_FontPathName, sizeof(Load_FontPathName), font_path_name);
  new_window(show_font0);
}

//-----------------------------------------------------------------------------

void generate_fontmap(void) {
  int *p = (int *)(font_aux + FNT_TABLE_OFFSET), n, x;
  char col[256], *ptr, *FntEnd = NULL;
  int dist, mincolor, mindist, r, g, b;

  map_width = 1;
  map_height = 0;

  for (n = 0; n < 256; n++) {
    if (*(p + n * 4) == 0 || *(p + n * 4 + 1) == 0)
      map_width += 2;
    else {
      map_width += *(p + n * 4) + 1;
      if (map_height < 2 + *(p + n * 4 + 1) + *(p + n * 4 + 2))
        map_height = 2 + *(p + n * 4 + 1) + *(p + n * 4 + 2);
      FntEnd = font_aux + *(p + n * 4 + 3) + *(p + n * 4) * *(p + n * 4 + 1);
    }
  }

  must_create = 0;

  if (new_map(NULL)) {
    must_create = 1;
    return;
  }

  must_create = 1;

  if (v_map != NULL && v_map->map != NULL) {
    // Warning! Calculate "c4" as a color !=0 !=c4 that is not
    // in the font and as close to c4 as possible

    memset(col, 0, 256);
    ptr = (char *)p + 256 * 16;
    for (; ptr < FntEnd; ptr++)
      col[*ptr] = 1;
    if (FntEnd - ptr > 1024) {
      FntEnd = ptr + 1024;
    }
    r = 128;
    g = 64;
    b = 0;
    ptr = (char *)p + 256 * 16;
    for (; ptr < FntEnd; ptr++) {
      n = *ptr;
      n *= 3;
      r += dac[n];
      g += dac[n + 1];
      b += dac[n + 2];
    }
    if (r < g) {
      g = 0;
      if (r < b) {
        r = 32;
        b = 0;
      } else {
        r = 0;
        b = 32;
      }
    } else {
      r = 0;
      if (g < b) {
        g = 32;
        b = 0;
      } else {
        g = 0;
        b = 32;
      }
    }
    mindist = 9999;
    mincolor = c4;
    for (n = 1; n < 256; n++)
      if (!col[n]) {
        dist = abs(r - dac[n * 3]) + abs(g - dac[n * 3 + 1]) + abs(b - dac[n * 3 + 2]);
        if (dist < mindist) {
          mindist = dist;
          mincolor = n;
        }
      }

    memset(v_map->map, mincolor, map_width * map_height);
    for (x = 1, n = 0; n < 256; n++) {
      generate_character(v_map->map + map_width + x, *(p + n * 4), *(p + n * 4 + 1),
                         *(p + n * 4 + 2), font_aux + *(p + n * 4 + 3));
      if (*(p + n * 4) == 0 || *(p + n * 4 + 1) == 0)
        x += 2;
      else
        x += *(p + n * 4) + 1;
    }
    new_window(map_view0);
  }
}

void generate_character(byte *di, int w, int h, int inc, char *si) {
  int x, y;
  if (w == 0 || h == 0) {
    *di = 0;
  } else {
    di += inc * map_width;
    y = h;
    do {
      x = w;
      do {
        *di++ = *si++;
      } while (--x);
      di += map_width - w;
    } while (--y);
  }
}
