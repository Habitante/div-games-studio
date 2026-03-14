
//-----------------------------------------------------------------------------
//      Selection/block operations
//      Split from paint.c (code reorganization only, no behavioral changes)
//-----------------------------------------------------------------------------

#include "paint_internal.h"

//-----------------------------------------------------------------------------
//      Polygon fill data (file-local to paint_select.c)
//-----------------------------------------------------------------------------

#define max_int 65536

word polygon[2048]; // Up to 1024 points for polygon selection
int n_points;
int k1, k2, xmin, trans;

//-----------------------------------------------------------------------------
//      Block toolbar setup
//-----------------------------------------------------------------------------

void block_bar(int n) {
  int a;

  switch (n) {
  case 0:
    bar[0] = 101 + zoom;
    bar[1] = 121;
    for (a = 2; a < 11; a++)
      bar[a] = 1;
    bar[11] = 166;
    bar[13] = 0;
    bar[14] = 0;
    if (mode_selection < 4)
      bar[12] = 135 + mode_selection;
    else
      bar[12] = mode_selection + 169 - 4;
    draw_bar(22);
    draw_ruler();
    break;
  case 1:
    bar[0] = 101 + zoom;
    bar[1] = 121;
    for (a = 2; a < 11; a++)
      bar[a] = 1;
    bar[11] = 166; // Eyedropper
    bar[12] = 139; // Move
    bar[13] = 171; // Fx
    bar[14] = 172; // Window
    bar[15] = 0;
    bar[16] = 0;
    draw_bar(22);
    draw_ruler();
    break;
  }
}

//-----------------------------------------------------------------------------
//      Edit screen (blocks) — main selection loop
//-----------------------------------------------------------------------------

void edit_mode_10(void) {
  int w, h;
  int x, y, a, b, aa, bb;
  int s; // s: State 0-wait 1st click, 1-wait 2nd click, ...
  int *m;

  block_bar(0);

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_color(2);

    if (((mouse_b & MB_LEFT) && selected_icon == 1) ||
        (scan_code == _BACKSPACE && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    } else if (scan_code == _BACKSPACE && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    switch (s) {
    case 0:
    case 1:
      select_box(12);
      break;

    case 2:
    case 3:

      if ((mouse_b & MB_LEFT) && selected_icon == 12) { // *** Move, Copy, ...
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
        sel_status = 0;
        box_to_sel_mask();
        move_selection(NULL, 0, 0);
        if (draw_mode < 100) {
          block_bar(1);
          sel_status = 1;
        }
        zoom_background = 0;
        need_zoom = 1;
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
      if ((mouse_b & MB_LEFT) && selected_icon == 13) { // *** Effects
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
        box_to_sel_mask();
        effects();
        if (draw_mode < 100) {
          block_bar(1);
          sel_status = 1;
        }
        zoom_background = 0;
        need_zoom = 1;
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
      if (((mouse_b & MB_LEFT) && selected_icon == 14) || key(_K)) { // *** Cut to window
        s = 0;
        sel_status = 0;
        block_bar(0);
        need_zoom = 1;
        cut_map();
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
      break;
    }

    test_previous();

    if ((key(_L_CTRL) || key(_R_CTRL)) && (mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS && sel_status &&
        s >= 2) {
      if (mode_selection == 0 || mode_selection == 4 || (mode_selection == 3 && s < 2)) {
        x = coord_x - (sel_x1 - sel_x0) / 2;
        y = coord_y - (sel_y1 - sel_y0) / 2;
        if (x < 0) {
          x = 0;
        }
        if (y < 0) {
          y = 0;
        }
        if (x + sel_x1 - sel_x0 >= map_width) {
          x = map_width - (sel_x1 - sel_x0) - 1;
        }
        if (y + sel_y1 - sel_y0 >= map_height) {
          y = map_height - (sel_y1 - sel_y0) - 1;
        }
        sel_x1 -= sel_x0;
        sel_x0 = x;
        sel_x1 += x;
        sel_y1 -= sel_y0;
        sel_y0 = y;
        sel_y1 += y;
      } else {
        x = coord_x - (sel_mask_x1 - sel_mask_x0) / 2;
        y = coord_y - (sel_mask_y1 - sel_mask_y0) / 2;
        if (x < 0) {
          x = 0;
        }
        if (y < 0) {
          y = 0;
        }
        if (x + sel_mask_x1 - sel_mask_x0 >= map_width)
          x = map_width - (sel_mask_x1 - sel_mask_x0) - 1;
        if (y + sel_mask_y1 - sel_mask_y0 >= map_height)
          y = map_height - (sel_mask_y1 - sel_mask_y0) - 1;
        if ((m = (int *)malloc(((map_width * map_height + 31) / 32) * 4)) != NULL) {
          memcpy(m, selection_mask, ((map_width * map_height + 31) / 32) * 4);
          memset(selection_mask, 0, ((map_width * map_height + 31) / 32) * 4);
          for (a = sel_mask_y0; a <= sel_mask_y1; a++)
            for (b = sel_mask_x0; b <= sel_mask_x1; b++) {
              aa = a;
              bb = b;
              bb += aa * map_width;
              aa = bb >> 5;
              bb &= 31;
              if (*(m + aa) & (1 << bb))
                set_selection_mask(b + x - sel_mask_x0, a + y - sel_mask_y0);
            }
          sel_mask_x1 -= sel_mask_x0;
          sel_mask_x0 = x;
          sel_mask_x1 += x;
          sel_mask_y1 -= sel_mask_y0;
          sel_mask_y0 = y;
          sel_mask_y1 += y;
          free(m);
        } else {
          v_text = (char *)texts[45];
          show_dialog(err0);
        }
      }
    } else
      switch (mode_selection) {
      case 0:
        s = edit_mode_6_box(s);
        break;
      case 1:
        s = edit_mode_6_fill(s);
        break;
      case 2:
        s = edit_mode_6_lines(s);
        break;
      case 3:
        s = edit_mode_6_boxes(s);
        break;
      case 4:
        s = edit_mode_6_box_auto(s);
        break;
      case 5:
        s = edit_mode_6_fill(s);
        break;
      }

    test_next();

    if (mode_selection == 0 || mode_selection == 4 || (mode_selection == 3 && s < 2)) {
      if (sel_x1 >= sel_x0)
        w = sel_x1 - sel_x0 + 1;
      else
        w = sel_x0 - sel_x1 + 1;
      if (sel_y1 >= sel_y0)
        h = sel_y1 - sel_y0 + 1;
      else
        h = sel_y0 - sel_y1 + 1;
      if (sel_status == 0) {
        w = 0;
        h = 0;
      }
    } else {
      if (sel_mask_x1 >= sel_mask_x0)
        w = sel_mask_x1 - sel_mask_x0 + 1;
      else
        w = sel_mask_x0 - sel_mask_x1 + 1;
      if (sel_mask_y1 >= sel_mask_y0)
        h = sel_mask_y1 - sel_mask_y0 + 1;
      else
        h = sel_mask_y0 - sel_mask_y1 + 1;
      if (sel_status == 0) {
        w = 0;
        h = 0;
      }
    }
    analyze_bar(w, h);

    blit_edit();
  } while (!exit_requested && !(mouse_b & MB_RIGHT) && !(key(_ESC) && s != 1 && s != 2) &&
           draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

  sel_status = 0;
}

//-----------------------------------------------------------------------------
//      Test previous/next selection refresh helpers
//-----------------------------------------------------------------------------

void test_previous(void) {
  // All this is added to calculate the region to refresh, instead of
  // doing a simple "need_zoom=1"

  need_zoom_x = map_width;
  need_zoom_y = map_height;
  need_zoom_width = 0;
  need_zoom_height = 0;

  if (sel_status)
    switch (mode_selection) {
    case 0:
      test_sel();
      break;
    case 1:
      test_sel_mask();
      break;
    case 2:
      test_sel_mask();
      break;
    case 3:
      test_sel();
      test_sel_mask();
      break;
    case 4:
      test_sel();
      break;
    case 5:
      test_sel_mask();
      break;
    }
}
void test_next(void) {
  int x, y, w, h;

  if (sel_status)
    switch (mode_selection) {
    case -1:
      test_sel();
      break;
    case 0:
      test_sel();
      break;
    case 1:
      test_sel_mask();
      break;
    case 2:
      test_sel_mask();
      break;
    case 3:
      test_sel();
      test_sel_mask();
      break;
    case 4:
      test_sel();
      break;
    case 5:
      test_sel_mask();
      break;
    }

  // Determine the zoom region to refresh on screen

  x = need_zoom_x;
  y = need_zoom_y;
  w = need_zoom_width - need_zoom_x + 1;
  h = need_zoom_height - need_zoom_y + 1;

  if (x < zoom_x)
    need_zoom_x = zoom_win_x - ((zoom_x - x) << zoom);
  else
    need_zoom_x = zoom_win_x + ((x - zoom_x) << zoom);
  if (y < zoom_y)
    need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
  else
    need_zoom_y = zoom_win_y + ((y - zoom_y) << zoom);
  need_zoom_width = w << zoom;
  need_zoom_height = h << zoom;

  if (need_zoom_x + need_zoom_width <= zoom_win_x || need_zoom_y + need_zoom_height <= zoom_win_y ||
      need_zoom_x >= zoom_win_x + zoom_win_width || need_zoom_y >= zoom_win_y + zoom_win_height) {
    need_zoom_width = 0;
    need_zoom_height = 0;
  } else {
    if (need_zoom_x < zoom_win_x) {
      need_zoom_width -= zoom_win_x - need_zoom_x;
      need_zoom_x = zoom_win_x;
    }
    if (need_zoom_y < zoom_win_y) {
      need_zoom_height -= zoom_win_y - need_zoom_y;
      need_zoom_y = zoom_win_y;
    }
    if (need_zoom_x + need_zoom_width > zoom_win_x + zoom_win_width)
      need_zoom_width = zoom_win_x + zoom_win_width - need_zoom_x;
    if (need_zoom_y + need_zoom_height > zoom_win_y + zoom_win_height)
      need_zoom_height = zoom_win_y + zoom_win_height - need_zoom_y;
    if (!need_zoom)
      need_zoom = -1;
  }

  // Refresh region calculated

  cclock = (*system_clock) >> 1;
}

void test_sel(void) {
  if (need_zoom_x > sel_x0) {
    need_zoom_x = sel_x0;
  }
  if (need_zoom_x > sel_x1) {
    need_zoom_x = sel_x1;
  }
  if (need_zoom_y > sel_y0) {
    need_zoom_y = sel_y0;
  }
  if (need_zoom_y > sel_y1) {
    need_zoom_y = sel_y1;
  }
  if (need_zoom_width < sel_x0) {
    need_zoom_width = sel_x0;
  }
  if (need_zoom_width < sel_x1) {
    need_zoom_width = sel_x1;
  }
  if (need_zoom_height < sel_y0) {
    need_zoom_height = sel_y0;
  }
  if (need_zoom_height < sel_y1) {
    need_zoom_height = sel_y1;
  }
}

void test_sel_mask(void) {
  if (need_zoom_x > sel_mask_x0) {
    need_zoom_x = sel_mask_x0;
  }
  if (need_zoom_x > sel_mask_x1) {
    need_zoom_x = sel_mask_x1;
  }
  if (need_zoom_y > sel_mask_y0) {
    need_zoom_y = sel_mask_y0;
  }
  if (need_zoom_y > sel_mask_y1) {
    need_zoom_y = sel_mask_y1;
  }
  if (need_zoom_width < sel_mask_x0) {
    need_zoom_width = sel_mask_x0;
  }
  if (need_zoom_width < sel_mask_x1) {
    need_zoom_width = sel_mask_x1;
  }
  if (need_zoom_height < sel_mask_y0) {
    need_zoom_height = sel_mask_y0;
  }
  if (need_zoom_height < sel_mask_y1) {
    need_zoom_height = sel_mask_y1;
  }
}

//-----------------------------------------------------------------------------
//      Copy the selected block to a new window
//-----------------------------------------------------------------------------

void cut_map(void) {
  int x, y, w, h;
  byte *p;

  // 1. Allocate memory for a tmapa struct
  if ((v_map = (struct tmapa *)malloc(sizeof(struct tmapa))) != NULL) {
    box_to_sel_mask();
    w = sel_mask_x1 - sel_mask_x0 + 1;
    h = sel_mask_y1 - sel_mask_y0 + 1;

    // 2. Allocate memory for the map
    if ((v_map->map = (byte *)malloc(w * h)) != NULL) {
      // 2b. Copy the map contents
      p = v_map->map;
      for (y = sel_mask_y0; y <= sel_mask_y1; y++)
        for (x = sel_mask_x0; x <= sel_mask_x1; x++)
          if (is_selection_mask(x, y))
            *p++ = *(map + y * map_width + x);
          else
            *p++ = 0;

      // 4. Set the remaining variables
      div_strcpy(v_map->filename, sizeof(v_map->filename), (char *)texts[136]);
      ltoa(next_code++, v_map->filename + strlen(v_map->filename), 10);
      *v_map->path = '\0';
      v_map->map_width = w;
      v_map->map_height = h;
      for (x = 0; x < 512; x += 2) {
        if (v.mapa->points[x] >= sel_mask_x0 && v.mapa->points[x] <= sel_mask_x1 &&
            v.mapa->points[x + 1] >= sel_mask_y0 && v.mapa->points[x + 1] <= sel_mask_y1) {
          v_map->points[x] = v.mapa->points[x] - sel_mask_x0;
          v_map->points[x + 1] = v.mapa->points[x + 1] - sel_mask_y0;
        } else {
          v_map->points[x] = -1;
          v_map->points[x + 1] = -1;
        }
      }
      v_map->has_name = 0; // No description by default
      v_map->fpg_code = 0;
      windows_to_create[num_windows_to_create++] = v_map;

    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
      free(v_map);
    }

  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
//      Convert simple blocks (boxes) into bitmasks
//-----------------------------------------------------------------------------

void box_to_sel_mask(void) {
  int x, y;

  if (mode_selection == 0 || mode_selection == 4) {
    sel_mask_x0 = sel_x0;
    sel_mask_y0 = sel_y0;
    sel_mask_x1 = sel_x1;
    sel_mask_y1 = sel_y1;
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        set_selection_mask(x, y);
  }
}

//-----------------------------------------------------------------------------
//      Clear (with color) a selected region
//-----------------------------------------------------------------------------

void sel_mask_delete(void) {
  int x, y;

  if (save_undo(sel_mask_x0, sel_mask_y0, sel_mask_x1 - sel_mask_x0 + 1,
                sel_mask_y1 - sel_mask_y0 + 1)) {
    if (texture_color == NULL) {
      for (y = sel_mask_y0; y <= sel_mask_y1; y++)
        for (x = sel_mask_x0; x <= sel_mask_x1; x++)
          if (is_selection_mask(x, y))
            *(map + x + y * map_width) = color;
    } else {
      for (y = sel_mask_y0; y <= sel_mask_y1; y++)
        for (x = sel_mask_x0; x <= sel_mask_x1; x++)
          if (is_selection_mask(x, y))
            *(map + x + y * map_width) = get_color(x, y);
    }
  }
}

//-----------------------------------------------------------------------------
//      Remap a selected region using the current gradient
//-----------------------------------------------------------------------------

void sel_mask_ruler(void) {
  int x, y;
  int c;

  if (save_undo(sel_mask_x0, sel_mask_y0, sel_mask_x1 - sel_mask_x0 + 1,
                sel_mask_y1 - sel_mask_y0 + 1)) {
    make_nearest_gradient();
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y)) {
          c = (memptrsize) * (map + x + y * map_width) * 3;
          c = dac[c] + dac[c + 1] + dac[c + 2];
          *(map + x + y * map_width) = nearest_gradient[c];
        }
  }
}

//-----------------------------------------------------------------------------
//	Invert a selection
//-----------------------------------------------------------------------------

void sel_mask_invert(void) {
  int x, y;
  byte p[256];

  if (save_undo(sel_mask_x0, sel_mask_y0, sel_mask_x1 - sel_mask_x0 + 1,
                sel_mask_y1 - sel_mask_y0 + 1)) {
    create_dac4();
    for (x = 0; x < 256; x++) {
      p[x] = find_color(63 - dac[x * 3], 63 - dac[x * 3 + 1], 63 - dac[x * 3 + 2]);
    }
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y))
          *(map + x + y * map_width) = p[*(map + x + y * map_width)];
  }
}

//-----------------------------------------------------------------------------
//      Create mask outline for a selection
//-----------------------------------------------------------------------------

void sel_mask_mask(void) {
  int x, y, c;
  c = 0;

  if (save_undo(sel_mask_x0, sel_mask_y0, sel_mask_x1 - sel_mask_x0 + 1,
                sel_mask_y1 - sel_mask_y0 + 1)) {
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y))
          if (!*(map + x + y * map_width)) {
            c = 0;
            if (x > 0)
              c |= *(map + x - 1 + y * map_width);
            if (c == color)
              c = 0;
            if (y > 0)
              c |= *(map + x + (y - 1) * map_width);
            if (c == color)
              c = 0;
            if (x < map_width - 1)
              c |= *(map + x + 1 + y * map_width);
            if (c == color)
              c = 0;
            if (y < map_height - 1)
              c |= *(map + x + (y + 1) * map_width);
            if (c == color)
              c = 0;
            if (c)
              *(map + x + y * map_width) = color;
          }
  }
}

//-----------------------------------------------------------------------------
//      Lighten a selection
//-----------------------------------------------------------------------------

void sel_mask_lighten(byte color_aclarar) {
  int x, y, n, a, b, c, d;
  byte p[256];

  if (save_undo(sel_mask_x0, sel_mask_y0, sel_mask_x1 - sel_mask_x0 + 1,
                sel_mask_y1 - sel_mask_y0 + 1)) {
    for (x = 0; x < 256; x++) {
      n = x * 256;
      p[x] = x;
      a = *(ghost + n + color_aclarar);
      b = *(ghost + n + a);
      c = *(ghost + n + b);
      d = *(ghost + n + c);
      if (d != p[x])
        p[x] = d;
      else if (c != p[x])
        p[x] = c;
      else if (b != p[x])
        p[x] = b;
      else if (a != p[x])
        p[x] = a;
      else
        p[x] = color_aclarar;
    }
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y))
          *(map + x + y * map_width) = p[*(map + x + y * map_width)];
  }
}

//-----------------------------------------------------------------------------
//      Anti aliasing
//-----------------------------------------------------------------------------

void sel_mask_antialias(void) {
  int x, y;
  int w, h;
  byte *p;
  int cx, cy, _c;

  w = sel_mask_x1 - sel_mask_x0 + 1;
  h = sel_mask_y1 - sel_mask_y0 + 1;
  if ((p = (byte *)malloc(map_width * map_height)) != NULL) {
    if (save_undo(sel_mask_x0, sel_mask_y0, w, h)) {
      for (y = sel_mask_y0; y <= sel_mask_y1; y++)
        for (x = sel_mask_x0; x <= sel_mask_x1; x++)
          if (is_selection_mask(x, y)) {
            _c = *(map + x + y * map_width);
            if (x > 0)
              cx = *(map + x - 1 + y * map_width) * 256;
            else
              cx = _c * 256;
            if (x < map_width - 1)
              cx = *(ghost + cx + *(map + x + 1 + y * map_width));
            else
              cx = *(ghost + cx + _c);
            if (y > 0)
              cy = *(map + x + (y - 1) * map_width) * 256;
            else
              cy = _c * 256;
            if (y < map_height - 1)
              cy = *(ghost + cy + *(map + x + (y + 1) * map_width));
            else
              cy = *(ghost + cy + _c);
            cx = *(ghost + cx * 256 + cy);
            _c = *(ghost + _c * 256 + cx);
            *(p + (x - sel_mask_x0) + (y - sel_mask_y0) * w) = (byte)_c;
          }
      for (y = sel_mask_y0; y <= sel_mask_y1; y++)
        for (x = sel_mask_x0; x <= sel_mask_x1; x++)
          if (is_selection_mask(x, y)) {
            *(map + x + y * map_width) = *(p + (x - sel_mask_x0) + (y - sel_mask_y0) * w);
          }
      free(p);
    }
  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
//      Block selection mode by box (based on s=state)
//-----------------------------------------------------------------------------

int edit_mode_6_box(int s) {
  switch (s) {
  case 0:
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      s = 1;
      sel_x0 = coord_x;
      sel_y0 = coord_y;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }
    break;

  case 1:
    if (mouse_graf >= CURSOR_ON_CANVAS) {
      sel_x1 = coord_x;
      sel_y1 = coord_y;
      sel_status = 1;
      if (mouse_b & MB_LEFT) {
        if (sel_x0 > sel_x1)
          swap(sel_x0, sel_x1);
        if (sel_y0 > sel_y1)
          swap(sel_y0, sel_y1);
        s = 2;
        block_bar(1);
      } else if (mouse_b || key(_ESC)) {
        s = 0;
        sel_status = 0;
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
    } else
      sel_status = 0;
    break;

  case 2:
    sel_status = 1;
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      if (coord_x >= (sel_x0 + sel_x1 * 3) / 4)
        sel_x1 = coord_x;
      else if (coord_x <= (sel_x0 * 3 + sel_x1) / 4)
        sel_x0 = coord_x;
      if (coord_y >= (sel_y0 + sel_y1 * 3) / 4)
        sel_y1 = coord_y;
      else if (coord_y <= (sel_y0 * 3 + sel_y1) / 4)
        sel_y0 = coord_y;
    } else if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      s = 0;
      sel_status = 0;
      block_bar(0);
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }
    break;
  }
  return (s);
}

//-----------------------------------------------------------------------------
//      Auto block selection mode by bounding box (based on s=state)
//-----------------------------------------------------------------------------

int edit_mode_6_box_auto(int s) {
  int x, y, n, m;
  int x0, y0, x1, y1;

  switch (s) {
  case 0:
  case 2:
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      if (*(map + coord_y * map_width + coord_x)) {
        if (s == 0 ||
            (coord_x < sel_x0 || coord_x > sel_x1 || coord_y < sel_y0 || coord_y > sel_y1)) {
          x0 = coord_x;
          x1 = coord_x;
          y0 = coord_y;
          y1 = coord_y;
          sel_x0 = map_width;
          sel_y0 = map_height;
          sel_x1 = 0;
          sel_y1 = 0;
        } else {
          x0 = sel_x0;
          x1 = sel_x1;
          y0 = sel_y0;
          y1 = sel_y1;
        }

        do {
          m = 0;

          // Expand the box ...
          n = 0;
          if (y0 > 0) {
            for (x = x0; x <= x1; x++)
              if (*(map + (y0 - 1) * map_width + x))
                n++;
            if (n) {
              y0--;
            }
            m += n;
          }
          n = 0;
          if (y1 < map_height - 1) {
            for (x = x0; x <= x1; x++)
              if (*(map + (y1 + 1) * map_width + x))
                n++;
            if (n) {
              y1++;
            }
            m += n;
          }
          n = 0;
          if (x0 > 0) {
            for (y = y0; y <= y1; y++)
              if (*(map + y * map_width + x0 - 1))
                n++;
            if (n) {
              x0--;
            }
            m += n;
          }
          n = 0;
          if (x1 < map_width - 1) {
            for (y = y0; y <= y1; y++)
              if (*(map + y * map_width + x1 + 1))
                n++;
            if (n) {
              x1++;
            }
            m += n;
          }

          // Shrink the box ...
          n = 0;
          if (y0 < y1) {
            for (x = x0; x <= x1; x++)
              if (*(map + y0 * map_width + x))
                n++;
            if (!n) {
              y0++;
              m++;
            }
          }
          n = 0;
          if (y0 < y1) {
            for (x = x0; x <= x1; x++)
              if (*(map + y1 * map_width + x))
                n++;
            if (!n) {
              y1--;
              m++;
            }
          }
          n = 0;
          if (x0 < x1) {
            for (y = y0; y <= y1; y++)
              if (*(map + y * map_width + x0))
                n++;
            if (!n) {
              x0++;
              m++;
            }
          }
          n = 0;
          if (x0 < x1) {
            for (y = y0; y <= y1; y++)
              if (*(map + y * map_width + x1))
                n++;
            if (!n) {
              x1--;
              m++;
            }
          }

        } while (m);

        if (sel_x0 == x0 && sel_x1 == x1 && sel_y0 == y0 && sel_y1 == y1) {
          if (x0 > 0) {
            sel_x0--;
          }
          if (y0 > 0) {
            sel_y0--;
          }
          if (x1 < map_width - 1) {
            sel_x1++;
          }
          if (y1 < map_height - 1) {
            sel_y1++;
          }
        } else {
          sel_x0 = x0;
          sel_y0 = y0;
          sel_x1 = x1;
          sel_y1 = y1;
        }

        s = 2;
        sel_status = 1;
        block_bar(1);

      } else {
        sel_status = 0;
        s = 0;
        block_bar(0);
      }

      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);

    } else if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      if (s == 2) {
        s = 0;
        sel_status = 0;
        block_bar(0);
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
    }
    break;
  }
  return (s);
}

//-----------------------------------------------------------------------------
//      Draw a line in the selection bitmask from x0,y0 .. to x1,y1
//-----------------------------------------------------------------------------

void sel_mask_line(int x0, int y0, int x1, int y1) {
  int dx, dy, a, b, d, x, y;

  if (x0 > x1) {
    x = x1;
    dx = x0 - x1;
  } else {
    x = x0;
    dx = x1 - x0;
  }
  if (y0 > y1) {
    y = y1;
    dy = y0 - y1;
  } else {
    y = y0;
    dy = y1 - y0;
  }

  if (!dx && !dy)
    set_selection_mask(x0, y0);
  else {
    if (dy <= dx) {
      if (x0 > x1) {
        set_selection_mask(x1, y1);
        x0--;
        swap(x0, x1);
        swap(y0, y1);
      }
      d = 2 * dy - dx;
      a = 2 * dy;
      b = 2 * (dy - dx);
      x = x0;
      y = y0;
      if (y0 <= y1)
        while (x < x1) {
          if (d <= 0) {
            d += a;
            x++;
          } else {
            d += b;
            x++;
            y++;
          }
          set_selection_mask(x, y);
        }
      else
        while (x < x1) {
          if (d <= 0) {
            d += a;
            x++;
          } else {
            d += b;
            x++;
            y--;
          }
          set_selection_mask(x, y);
        }
    } else {
      if (y0 > y1) {
        set_selection_mask(x1, y1);
        y0--;
        swap(x0, x1);
        swap(y0, y1);
      }
      d = 2 * dx - dy;
      a = 2 * dx;
      b = 2 * (dx - dy);
      x = x0;
      y = y0;
      if (x0 <= x1)
        while (y < y1) {
          if (d <= 0) {
            d += a;
            y++;
          } else {
            d += b;
            y++;
            x++;
          }
          set_selection_mask(x, y);
        }
      else
        while (y < y1) {
          if (d <= 0) {
            d += a;
            y++;
          } else {
            d += b;
            y++;
            x--;
          }
          set_selection_mask(x, y);
        }
    }
  }
}

//-----------------------------------------------------------------------------
//      Polygon fill algorithm for selection_mask
//-----------------------------------------------------------------------------

void find_first_x_from(int xi, int y) {
  int x0, y0, x1, y1, n, x;

  trans = 0;
  xmin = max_int;

  for (n = 0; n < n_points - 1; n++) {
    x0 = polygon[n * 2];
    y0 = polygon[n * 2 + 1];
    x1 = polygon[n * 2 + 2];
    y1 = polygon[n * 2 + 3];
    if (y0 > y1) {
      swap(x0, x1);
      swap(y0, y1);
    }
    if (y0 <= y && y1 >= y && y0 != y1) {
      x = (float)x0 + (float)((x1 - x0) * (y - y0)) / (float)(y1 - y0);
      if (x > xi && x < xmin)
        xmin = x;
    }
  }

  // Got xmin, now count k1 and k2 updating the transition flag

  if (xmin != max_int)
    for (n = 0; n < n_points - 1; n++) {
      x0 = polygon[n * 2];
      y0 = polygon[n * 2 + 1];
      x1 = polygon[n * 2 + 2];
      y1 = polygon[n * 2 + 3];
      if (y0 > y1) {
        swap(x0, x1);
        swap(y0, y1);
      }
      if (y0 <= y && y1 >= y && y0 != y1) {
        x = (float)x0 + (float)((x1 - x0) * (y - y0)) / (float)(y1 - y0);
        if (x == xmin) {
          if (y == y0) {
            k1++;
            if (k1 != k2)
              trans ^= 1;
          } else if (y == y1) {
            k2++;
            if (k1 != k2)
              trans ^= 1;
          } else
            trans ^= 1;
          while (k1 < k2 - 1) {
            k1 += 2;
          }
          while (k2 < k1 - 1) {
            k2 += 2;
          }
        }
      }
    }
}

void fill_polygon(void) {
  int y0, y1; // Start and end Y coordinates
  int x0, inside;
  int m, n;

  y0 = map_height;
  y1 = 0;

  for (n = 1; n < n_points * 2; n += 2) {
    if (polygon[n] < y0)
      y0 = polygon[n];
    if (polygon[n] > y1)
      y1 = polygon[n];
  }

  for (n = y0; n <= y1; n++) {
    x0 = -1;
    k1 = 0;
    k2 = 0;
    inside = 0;
    do {
      find_first_x_from(x0, n);
      if (xmin != max_int) {
        if (inside) {
          for (m = x0 + 1; m <= xmin; m++)
            set_selection_mask(m, n);
        }
        inside ^= trans;
        x0 = xmin;
      }
    } while (xmin != max_int);
  }
}

//-----------------------------------------------------------------------------
//      Polygon selection mode (based on s=state)
//-----------------------------------------------------------------------------

int edit_mode_6_lines(int s) {
  static int oldx, oldy;

  switch (s) {
  case 0:
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      s = 1;
      n_points = 1;
      polygon[0] = coord_x;
      polygon[1] = coord_y;
      memset(selection_mask, 0, ((map_width * map_height + 31) / 32) * 4);
      sel_mask_x0 = sel_mask_x1 = coord_x;
      sel_mask_y0 = sel_mask_y1 = coord_y;
      set_selection_mask(coord_x, coord_y);
      oldx = coord_x;
      oldy = coord_y;
    }
    break;

  case 1:
    sel_status = 1;
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS && n_points < 1024 &&
        (polygon[n_points * 2 - 2] != coord_x || polygon[n_points * 2 - 1] != coord_y)) {
      polygon[n_points * 2] = coord_x;
      polygon[n_points * 2 + 1] = coord_y;
      n_points++;
      if (coord_x < sel_mask_x0)
        sel_mask_x0 = coord_x;
      else if (coord_x > sel_mask_x1)
        sel_mask_x1 = coord_x;
      if (coord_y < sel_mask_y0)
        sel_mask_y0 = coord_y;
      else if (coord_y > sel_mask_y1)
        sel_mask_y1 = coord_y;
      sel_mask_line(oldx, oldy, coord_x, coord_y);
      oldx = coord_x;
      oldy = coord_y;
      if (n_points > 3 && oldx >= polygon[0] - 1 && oldx <= polygon[0] + 1 &&
          oldy >= polygon[1] - 1 && oldy <= polygon[1] + 1) {
        if (oldx != polygon[0] || oldy != polygon[1]) {
          polygon[n_points * 2] = polygon[0];
          polygon[n_points * 2 + 1] = polygon[1];
          n_points++;
        }
        fill_polygon();
        s = 2;
        block_bar(1);
      }
    } else if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      if (n_points > 2) {
        polygon[n_points * 2] = polygon[0];
        polygon[n_points * 2 + 1] = polygon[1];
        n_points++;
        sel_mask_line(oldx, oldy, polygon[0], polygon[1]);
        fill_polygon();
        s = 2;
        block_bar(1);
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      } else {
        s = 0;
        sel_status = 0;
        block_bar(0);
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
    }
    break;

  case 2:
    sel_status = 1;
    if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      s = 0;
      sel_status = 0;
      block_bar(0);
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }
    break;
  }
  return (s);
}

//-----------------------------------------------------------------------------
//      Block selection mode by flood fill (based on s=state)
//-----------------------------------------------------------------------------

int edit_mode_6_fill(int s) {
  int n;

  switch (s) {
  case 0:
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      if (mode_selection == 1) {
        memset(fill_dac, 0, 256);
        fill_dac[*(map + coord_y * map_width + coord_x)] = 1;
      } else {
        memset(fill_dac, 1, 256);
        fill_dac[0] = 0;
      }
      s = 2;
      memset(fill_seeds, -1, 512);
      fill_seeds[0] = coord_x;
      fill_seeds[1] = coord_y;
      memset(selection_mask, 0, ((map_width * map_height + 31) / 32) * 4);
      sel_mask_x0 = map_width;
      sel_mask_y0 = map_height;
      sel_mask_x1 = 0;
      sel_mask_y1 = 0;
      fill_select(coord_x, coord_y);
      if (is_selection_mask(coord_x, coord_y))
        block_bar(1);
      else
        s = 0;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }
    break;

  case 2:
    sel_status = 1;
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      if (mode_selection == 1)
        fill_dac[*(map + coord_y * map_width + coord_x)] = 1;
      n = 0;
      while (n < 126 && fill_seeds[n] >= 0)
        n += 2;
      fill_seeds[n] = coord_x;
      fill_seeds[n + 1] = coord_y;
      memset(selection_mask, 0, ((map_width * map_height + 31) / 32) * 4);
      for (n = 0; n < 128; n += 2)
        if (fill_seeds[n] >= 0) {
          if (!is_selection_mask(fill_seeds[n], fill_seeds[n + 1])) {
            fill_select(fill_seeds[n], fill_seeds[n + 1]);
            if (!is_selection_mask(fill_seeds[n], fill_seeds[n + 1])) {
              s = 0;
              sel_status = 0;
              block_bar(0);
              do {
                read_mouse();
              } while (mouse_b || key(_ESC));
            }
          } else
            fill_seeds[n] = -1;
        }
    } else if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      s = 0;
      sel_status = 0;
      block_bar(0);
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }
    break;
  }
  return (s);
}

//-----------------------------------------------------------------------------
//      Block selection mode by multiple boxes (based on s=state)
//-----------------------------------------------------------------------------

int edit_mode_6_boxes(int s) {
  int x, y;

  switch (s) {
  case 0:
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      s = 1;
      sel_x0 = coord_x;
      sel_y0 = coord_y;
      memset(selection_mask, 0, ((map_width * map_height + 31) / 32) * 4);
      sel_mask_x0 = map_width;
      sel_mask_y0 = map_height;
      sel_mask_x1 = 0;
      sel_mask_y1 = 0;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }
    break;

  case 1:
    if (mouse_graf >= CURSOR_ON_CANVAS) {
      sel_x1 = coord_x;
      sel_y1 = coord_y;
      sel_status = 1;
      if (mouse_b & MB_LEFT) {
        if (sel_x0 > sel_x1)
          swap(sel_x0, sel_x1);
        if (sel_y0 > sel_y1)
          swap(sel_y0, sel_y1);
        sel_mask_x0 = sel_x0;
        sel_mask_x1 = sel_x1;
        sel_mask_y0 = sel_y0;
        sel_mask_y1 = sel_y1;
        for (y = sel_y0; y <= sel_y1; y++)
          for (x = sel_x0; x <= sel_x1; x++)
            set_selection_mask(x, y);
        sel_x1 = -1;
        s = 2;
        block_bar(1);
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      } else if (mouse_b || key(_ESC)) {
        s = 0;
        sel_status = 0;
        do {
          read_mouse();
        } while (mouse_b || key(_ESC));
      }
    } else
      sel_status = 0;
    break;

  case 2:
    sel_status = 1;
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      s = 3;
      sel_x0 = coord_x;
      sel_y0 = coord_y;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    } else if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      s = 0;
      sel_status = 0;
      block_bar(0);
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }
    break;

  case 3:
    sel_status = 1;
    if (mouse_graf >= CURSOR_ON_CANVAS) {
      sel_x1 = coord_x;
      sel_y1 = coord_y;
    } else
      sel_x1 = -1;
    if ((mouse_b & MB_LEFT) && mouse_graf >= CURSOR_ON_CANVAS) {
      if (sel_x0 > sel_x1)
        swap(sel_x0, sel_x1);
      if (sel_y0 > sel_y1)
        swap(sel_y0, sel_y1);
      if (sel_x0 < sel_mask_x0)
        sel_mask_x0 = sel_x0;
      if (sel_x1 > sel_mask_x1)
        sel_mask_x1 = sel_x1;
      if (sel_y0 < sel_mask_y0)
        sel_mask_y0 = sel_y0;
      if (sel_y1 > sel_mask_y1)
        sel_mask_y1 = sel_y1;
      for (y = sel_y0; y <= sel_y1; y++)
        for (x = sel_x0; x <= sel_x1; x++)
          set_selection_mask(x, y);
      s = 2;
      sel_x1 = -1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    } else if ((mouse_b & MB_RIGHT) || key(_ESC)) {
      sel_x1 = -1;
      s = 2;
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    }
    break;
  }
  return (s);
}

//-----------------------------------------------------------------------------
//      Toolbar for move, copy, rotate, scale, etc.
//-----------------------------------------------------------------------------

void move_selection(byte *sp, int w, int h) {
  int x, y;
  int _x, _y;
  int bar_width, _al;
  int xg, yg;
  byte *p;
  int _coord_x = 0, _coord_y = 0;
  int _mouse_graf;

  int ghost = 0, block = 0;
  float size = 1, ang = 0, old_float;
  int s = 0; // Status 0-Normal, 1-Rotating_a, 2-Rotating_b, 3-Scaling_a, 4-...

  byte num[5];

  if (sp == NULL) {
    w = sel_mask_x1 - sel_mask_x0 + 1;
    h = sel_mask_y1 - sel_mask_y0 + 1;
    if ((sp = (byte *)malloc(w * h)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    memset(sp, 0, w * h); // Create the sprite in sp
    for (y = sel_mask_y0; y <= sel_mask_y1; y++)
      for (x = sel_mask_x0; x <= sel_mask_x1; x++)
        if (is_selection_mask(x, y))
          *(sp + x - sel_mask_x0 + (y - sel_mask_y0) * w) = *(map + x + y * map_width);
  }

  bar[0] = 101 + zoom;
  bar[1] = 121;
  bar[2] = 105 + ghost;
  bar[3] = 122 + block;
  bar[4] = 125;
  bar[5] = 124;
  bar[6] = 126;
  bar[7] = 127;
  bar[8] = 129;
  bar[9] = 0;
  draw_bar(0);
  put_bar_inv(10, 2, 139);

  do {
    draw_help(1295);
    read_mouse();
    select_zoom();
    test_mouse();

    if (s == 0) {
      if (((mouse_b & MB_LEFT) && selected_icon == 1) ||
          (scan_code == _BACKSPACE && !key(_L_SHIFT) && !key(_R_SHIFT))) {
        undo_back();
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      } else if (scan_code == _BACKSPACE && (key(_L_SHIFT) || key(_R_SHIFT))) {
        undo_next();
        need_zoom = 1;
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 2) {
        ghost ^= 1;
        bar[2] = 105 + ghost;
        draw_bar(0);
        put_bar_inv(10, 2, 139);
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 3) {
        block ^= 1;
        bar[3] = 122 + block;
        draw_bar(0);
        put_bar_inv(10, 2, 139);
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 4) {
        flip_horizontal(sp, w, h);
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 5) {
        flip_vertical(sp, w, h);
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 6) {
        s = 1;
        bar[1] = 0;
        bar[2] = 0;
        draw_bar(22);
        put_bar_inv(10, 2, 126);
        old_float = ang;
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 7) {
        s = 3;
        bar[1] = 0;
        bar[2] = 0;
        draw_bar(22);
        put_bar_inv(10, 2, 127);
        old_float = size;
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }

      if ((mouse_b & MB_LEFT) && selected_icon == 8) {
        box_to_sel_mask();
        sel_mask_delete();
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);
      }
    }

    move_zoom();
    move_bar();

    if (key(_H)) {
      blit_screen(screen_buffer);
      do {
        poll_keyboard();
      } while (key(_H));
    }

    bar_coords();

    if (s != 2 && s != 4) {
      _coord_x = coord_x;
      _coord_y = coord_y;
    }

    if (s == 1 || s == 2) {
      _x = coord_x - _coord_x;
      _y = _coord_y - coord_y;
      if (!_x && !_y)
        ang = 0;
      else
        ang = atan2(_y, _x);
      xg = ang * 57.295779761;
      if (xg < 0)
        xg += 360;
      wbox(toolbar, vga_width / big2, 19, c2, toolbar_width - 23, 2, 21, 15);
      p = screen_buffer;
      screen_buffer = toolbar;
      text_color = c3;
      num[4] = 0;
      num[3] = '\xa7';
      num[2] = xg % 10 + 48;
      num[1] = (xg / 10) % 10 + 48;
      num[0] = (xg / 100) % 10 + 48;
      writetxt(toolbar_width - 21, 6, 0, num);
      screen_buffer = p;
    }

    if (s == 3 || s == 4) {
      _x = coord_x + coord_y - _coord_x - _coord_y;
      if ((size = 1 + (float)_x / 100) < 0)
        size = 0;
      xg = size * 100;
      wbox(toolbar, vga_width / big2, 19, c2, toolbar_width - 23, 2, 21, 15);
      p = screen_buffer;
      screen_buffer = toolbar;
      text_color = c3;
      num[4] = 0;
      num[3] = '%';
      num[2] = xg % 10 + 48;
      num[1] = (xg / 10) % 10 + 48;
      num[0] = (xg / 100) % 10 + 48;
      writetxt(toolbar_width - 22, 6, 0, num);
      screen_buffer = p;
    }

    if (mouse_in(toolbar_x + 48, toolbar_y + 2, toolbar_x + toolbar_width - 2, toolbar_y + 17)) {
      selected_icon = (mouse_x - toolbar_x - 48) / 16;
      if (bar[selected_icon] > 1) {
        w_highlight_box(toolbar, vga_width / big2, 19, 48 + selected_icon * 16, 2, 15, 15);
      } else
        selected_icon = -1;
    } else
      selected_icon = -1;

    if ((_mouse_graf = mouse_graf) >= 10) {
      if (_coord_x >= 0 && _coord_y >= 0 && _coord_x < map_width && _coord_y < map_height) {
        xg = w / 2;
        yg = h / 2;
        if (size == 1 && ang == 0) {
          if (save_undo(_coord_x - xg, _coord_y - yg, w, h))
            sp_normal(sp, _coord_x, _coord_y, w, h, xg, yg, block * 2 + ghost);
          else
            draw_mode = 110;
        } else {
          bar_width = w;
          _al = h;
          _x = _coord_x;
          _y = _coord_y;
          sp_size(&_x, &_y, &bar_width, &_al, xg, yg, ang, size);
          if (save_undo(_x, _y, bar_width, _al))
            sp_rotated(sp, _coord_x, _coord_y, w, h, xg, yg, block * 2 + ghost, ang, size);
          else
            draw_mode = 110;
        }
      }
      if (s == 2 || s == 4)
        _mouse_graf = CURSOR_ARROW;
      else
        _mouse_graf = CURSOR_MOVE;
    }

    zoom_map();
    flush_bars(0);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, _mouse_graf, 0);
    put(mouse_shift_x, mouse_shift_y, _mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, _mouse_graf, 1);

    if (mouse_graf >= CURSOR_ON_CANVAS)
      switch (s) {
      case 0:
        if (mouse_b & MB_LEFT)
          do {
            read_mouse();
          } while (mouse_b);
        else
          undo_back();
        break;
      case 1:
        undo_back();
        if (mouse_b & MB_LEFT) {
          do {
            read_mouse();
          } while (mouse_b);
          s = 2;
        }
        if (mouse_b & MB_RIGHT || key(_ESC)) {
          bar[1] = 121;
          bar[2] = 105 + ghost;
          draw_bar(0);
          put_bar_inv(10, 2, 139);
          do {
            read_mouse();
          } while (mouse_b || key(_ESC));
          s = 0;
          ang = old_float;
        }
        break;
      case 2:
        undo_back();
        if (mouse_b || key(_ESC)) {
          if (!(mouse_b & MB_LEFT))
            ang = old_float;
          bar[1] = 121;
          bar[2] = 105 + ghost;
          draw_bar(0);
          put_bar_inv(10, 2, 139);
          do {
            read_mouse();
          } while (mouse_b || key(_ESC));
          s = 0;
        }
        break;
      case 3:
        undo_back();
        if (mouse_b & MB_LEFT) {
          do {
            read_mouse();
          } while (mouse_b);
          s = 4;
        }
        if (mouse_b & MB_RIGHT || key(_ESC)) {
          bar[1] = 121;
          bar[2] = 105 + ghost;
          draw_bar(0);
          put_bar_inv(10, 2, 139);
          do {
            read_mouse();
          } while (mouse_b || key(_ESC));
          s = 0;
          size = old_float;
        }
        break;
      case 4:
        undo_back();
        if (mouse_b || key(_ESC)) {
          if (!(mouse_b & MB_LEFT))
            size = old_float;
          bar[1] = 121;
          bar[2] = 105 + ghost;
          draw_bar(0);
          put_bar_inv(10, 2, 139);
          do {
            read_mouse();
          } while (mouse_b || key(_ESC));
          s = 0;
        }
        break;
      }

    if (selected_icon >= 0)
      put_bar(48 + selected_icon * 16, 2, bar[selected_icon]);

    select_mode();

    if (scan_code == _TAB) {
      if (!tab_cycling) {
        tab_cycling = 1;
        if (key(_L_SHIFT) || key(_R_SHIFT))
          change_map(0);
        else
          change_map(1);
      }
    } else
      tab_cycling = 0;

  } while (!exit_requested && !(mouse_b & MB_RIGHT) && !(key(_ESC) && !s) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

  if (key(_ESC) ||
      (mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))) {
    put_bar(2, 10, 45);
    flush_bars(0);
    put(mouse_x, mouse_y, mouse_graf);
    blit_screen(screen_buffer);
  }

  free(sp);
}


//-----------------------------------------------------------------------------
//      FX - effects on a selected block
//-----------------------------------------------------------------------------

void effects(void) {
  bar[0] = 101 + zoom;
  bar[1] = 121;
  bar[2] = 140;
  bar[3] = 194;
  bar[4] = 128;
  bar[5] = 142;
  bar[6] = 143;
  bar[7] = 150;
  bar[8] = 0;
  draw_bar(0);
  put_bar_inv(10, 2, 171);
  need_zoom = 1;

  do {
    draw_help(1295);
    read_mouse();
    select_zoom();
    test_mouse();

    if (((mouse_b & MB_LEFT) && selected_icon == 1) ||
        (scan_code == _BACKSPACE && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    } else if (scan_code == _BACKSPACE && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    if ((mouse_b & MB_LEFT) && selected_icon == 2) { // Remap to gradient
      sel_mask_ruler();
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }

    if ((mouse_b & MB_LEFT) && selected_icon == 3) { // Invert
      sel_mask_invert();
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }

    if ((mouse_b & MB_LEFT) && selected_icon == 4) { // Mask
      sel_mask_mask();
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }

    if ((mouse_b & MB_LEFT) && selected_icon == 5) { // Lighten
      sel_mask_lighten(c4);
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }

    if ((mouse_b & MB_LEFT) && selected_icon == 6) { // Darken
      sel_mask_lighten(c0);
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }

    if ((mouse_b & MB_LEFT) && selected_icon == 7) { // Anti-alias
      sel_mask_antialias();
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
    }

    need_zoom_x = map_width;
    need_zoom_y = map_height;
    need_zoom_width = 0;
    need_zoom_height = 0;
    test_next();

    blit_edit();
  } while (!exit_requested && !(mouse_b & MB_RIGHT) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

  if (key(_ESC) ||
      (mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18))) {
    put_bar(2, 10, 45);
    flush_bars(0);
    put(mouse_x, mouse_y, mouse_graf);
    blit_screen(screen_buffer);
  }
}
