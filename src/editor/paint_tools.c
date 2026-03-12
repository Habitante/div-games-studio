
//-----------------------------------------------------------------------------
//      Drawing tools — interactive drawing tool modes
//      Split from paint.c (code reorganization only, no behavioral changes)
//-----------------------------------------------------------------------------

#include "paint_internal.h"

//-----------------------------------------------------------------------------
//      Icon arrays for tool selectors
//-----------------------------------------------------------------------------

int box_icons[] = {2, 1, 110, 111};

int circle_icons[] = {4, 1, 175, 176, 112, 113};

int spray_icons[] = {6, 1, 181, 182, 183, 184, 185, 186};

//-----------------------------------------------------------------------------
//      Tool helper: select rectangle mode
//-----------------------------------------------------------------------------

void select_rect(int n) {
  int r;
  r = select_icon(toolbar_x + 48 + n * 16, box_icons);
  if (r >= 0) {
    mode_rect = r;
    bar[13] = box_icons[2 + mode_rect];
    draw_bar(22);
    draw_ruler();
  }
}

//-----------------------------------------------------------------------------
//      Tool helper: select circle mode
//-----------------------------------------------------------------------------

void select_circle(int n) {
  int r;
  r = select_icon(toolbar_x + 48 + n * 16, circle_icons);
  if (r >= 0) {
    mode_circle = r;
    bar[13] = circle_icons[2 + mode_circle];
    draw_bar(22);
    draw_ruler();
  }
}

//-----------------------------------------------------------------------------
//      Tool helper: select spray mode
//-----------------------------------------------------------------------------

#define undo_spray -1
#define undo_box   4

int spray_mode = 2, clock_spray;

void select_spray(int n) {
  int r;
  r = select_icon(toolbar_x + 48 + n * 16, spray_icons);
  if (r >= 0) {
    spray_mode = r;
    bar[13] = spray_icons[2 + spray_mode];
    draw_bar(0);
    draw_ruler();
  }
}

//-----------------------------------------------------------------------------
//      Edit screen (generic editing, no draw_mode defined)
//-----------------------------------------------------------------------------

void edit_scr(void) {
  bar[0] = 101 + zoom;
  bar[1] = 0;
  draw_bar(0);

  do {
    read_mouse();
    select_zoom();
    test_mouse();

    // ...

    blit_edit();
  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (pixels)
//-----------------------------------------------------------------------------

void edit_mode_0(void) {
  int n;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (n = 2; n < 11; n++) {
    bar[n] = 1;
  }
  bar[11] = 166;
  bar[12] = 0;
  draw_bar(0);
  draw_ruler();

  do {
    draw_help(1295);
    edit_ruler();
    select_color(2);
    if (((mouse_b & 1) && !(prev_mouse_buttons & 1)) && mouse_graf >= 10) {
      if (!mask[*(map + coord_x + coord_y * map_width)]) {
        if (*(map + coord_x + coord_y * map_width) != color) {
          save_undo(coord_x, coord_y, 1, 1);
          *(map + coord_x + coord_y * map_width) = color;
        } else {
          save_undo(coord_x, coord_y, 1, 1);
          *(map + coord_x + coord_y * map_width) = 0;
        }
      }
    }

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    blit_edit();
  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (pencil)
//-----------------------------------------------------------------------------

void edit_mode_1(void) {
  int _x, _y, a, _a = 0, _b = 0;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = 0;

  draw_bar(0);
  draw_ruler();

  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &pencil_tool_effect);
    select_color(2);

    a = -1; // Last action performed (this iteration) - undefined

    if ((mouse_b & 1) && mouse_graf >= 10) {
      if (key(_D))
        blur_enabled = 1;

      if (_b == 0 || coord_x != _x || coord_y != _y) {
        a = 1;
        line_fx = pencil_tool_effect;
        if (_a == 1)
          line(_x, _y, coord_x, coord_y, 0);
        else
          line(coord_x, coord_y, coord_x, coord_y, 0);
      }

      blur_enabled = 0;

    } else
      a = 0;

    if (a >= 0)
      _a = a; // Save the last action

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    _x = coord_x;
    _y = coord_y;
    _b = mouse_b;
    blit_edit();
  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (lines)
//-----------------------------------------------------------------------------

void edit_mode_2(void) {
  int s;      // State 0-wait 1st click, 1-wait 2nd click
  int x0, y0; // Starting coordinates
  int w, h;
  int a;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = 0;
  bar[14] = 0;
  draw_bar(22);
  draw_ruler();

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &line_tool_effect);
    select_color(2);

    if (key(_D))
      blur_enabled = 1;

    w = -1;
    h = -1;
    back = 0;
    switch (s) {
    case 0:
      if ((mouse_b & 1) && mouse_graf >= 10) {
        s = 1;
        x0 = coord_x;
        y0 = coord_y;
        do {
          read_mouse();
        } while (mouse_b & 1);
      }
      break;
    case 1:
      if (mouse_graf >= 10) {
        line_fx = line_tool_effect;
        undo_error = 0;
        line(x0, y0, coord_x, coord_y, 1);
        if (undo_error)
          s = 0;
        else {
          w = (x0 > coord_x) ? x0 - coord_x : coord_x - x0;
          h = (y0 > coord_y) ? y0 - coord_y : coord_y - y0;
          if (mouse_b & 1) {
            s = 0;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
            back = 1;
          } else
            back = 1;
        }
      }
      break;
    }

    blur_enabled = 0;

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    analyze_bar(w + 1, h + 1);
    blit_edit();

    if (back) {
      undo_back();
      if (need_zoom == 1)
        zoom_map();
      else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
        draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
      need_zoom_width = 0;
      need_zoom_height = 0;
      need_zoom = 0;
    }

  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && s != 1) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (continuous lines)
//-----------------------------------------------------------------------------

void edit_mode_3(void) {
  int s;      // State 0-wait 1st click, 1-wait 2nd click, 2-wait 3rd click
  int x0, y0; // Starting coordinates
  int w, h;
  int a;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = 0;
  bar[14] = 0;
  draw_bar(22);
  draw_ruler();

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &box_tool_effect);
    select_color(2);

    if (key(_D))
      blur_enabled = 1;

    w = -1;
    h = -1;
    back = 0;
    switch (s) {
    case 0:
      if ((mouse_b & 1) && mouse_graf >= 10) {
        s = 1;
        x0 = coord_x;
        y0 = coord_y;
        do {
          read_mouse();
        } while (mouse_b & 1);
      }
      break;
    case 1:
    case 2:
      if (mouse_graf >= 10) {
        line_fx = box_tool_effect;
        undo_error = 0;
        line(x0, y0, coord_x, coord_y, s & 1);
        if (undo_error)
          s = 0;
        else {
          w = (x0 > coord_x) ? x0 - coord_x : coord_x - x0;
          h = (y0 > coord_y) ? y0 - coord_y : coord_y - y0;
          if (mouse_b & 1) {
            s = 2;
            x0 = coord_x;
            y0 = coord_y;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
            back = 1;
          } else
            back = 1;
        }
      }
      break;
    }

    blur_enabled = 0;

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    analyze_bar(w + 1, h + 1);
    blit_edit();

    if (back) {
      undo_back();
      if (need_zoom == 1)
        zoom_map();
      else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
        draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
      need_zoom_width = 0;
      need_zoom_height = 0;
      need_zoom = 0;
    }

  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && s != 1) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (Bezier curves)
//-----------------------------------------------------------------------------

void edit_mode_4(void) {
  int s; // State 0-wait 1st click, 1-wait 2nd click, 2-3rd..., 3-4th...
  int a;

  int x0, y0;   // Starting coordinates
  int x1, y1;   // Ending coordinates
  int _x0, _y0; // Initial slope

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = 0;
  draw_bar(0);
  draw_ruler();

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &bezier_tool_effect);
    select_color(2);

    if (key(_D))
      blur_enabled = 1;

    back = 0;
    switch (s) {
    case 0:
      if ((mouse_b & 1) && mouse_graf >= 10) {
        s = 1;
        x0 = coord_x;
        y0 = coord_y;
        do {
          read_mouse();
        } while (mouse_b & 1);
      }
      break;
    case 1:
      if (mouse_graf >= 10) {
        line_fx = bezier_tool_effect;
        undo_error = 0;
        line(x0, y0, coord_x, coord_y, 1);
        if (undo_error)
          s = 0;
        else {
          if (mouse_b & 1) {
            s = 2;
            x1 = coord_x;
            y1 = coord_y;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
          }
          back = 1;
        }
      }
      break;
    case 2:
      if (mouse_graf >= 10) {
        line_fx = bezier_tool_effect;
        undo_error = 0;
        bezier(x0, y0, x1, y1, coord_x, coord_y, coord_x, coord_y, 1);
        if (undo_error)
          s = 0;
        else {
          if (mouse_b & 1) {
            s = 3;
            _x0 = coord_x;
            _y0 = coord_y;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
          }
          back = 1;
        }
      }
      break;
    case 3:
      if (mouse_graf >= 10) {
        line_fx = bezier_tool_effect;
        undo_error = 0;
        bezier(x0, y0, x1, y1, _x0, _y0, coord_x, coord_y, 1);
        if (undo_error)
          s = 0;
        else {
          if (mouse_b & 1) {
            s = 0;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
            back = 1;
          } else
            back = 1;
        }
      }
      break;
    }

    blur_enabled = 0;

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    blit_edit();

    if (back) {
      undo_back();
      if (need_zoom == 1)
        zoom_map();
      else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
        draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
      need_zoom_width = 0;
      need_zoom_height = 0;
      need_zoom = 0;
    }

  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && s != 1) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (continuous Bezier curves)
//-----------------------------------------------------------------------------

void edit_mode_5(void) {
  int s; // State 0-wait 1st click, 1-wait 2nd click, 2-3rd...
  int a;

  int x0, y0; // First point
  int x1, y1; // Second point

  float ang;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = 0;
  draw_bar(0);
  draw_ruler();

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &polygon_tool_effect);
    select_color(2);

    if (key(_D))
      blur_enabled = 1;

    back = 0;

    switch (s) {
    case 0:
      if ((mouse_b & 1) && mouse_graf >= 10) {
        s = 1;
        x0 = coord_x;
        y0 = coord_y;
        do {
          read_mouse();
        } while (mouse_b & 1);
      }
      break;
    case 1:
      if (mouse_graf >= 10) {
        line_fx = polygon_tool_effect;
        undo_error = 0;
        line(x0, y0, coord_x, coord_y, 1);
        if (undo_error)
          s = 0;
        else {
          if (mouse_b & 1) {
            s = 2;
            x0 = x0 - coord_x;
            y0 = coord_y - y0;
            if (!x0 && !y0)
              ang = 0;
            else
              ang = atan2(y0, x0);
            x1 = coord_x;
            y1 = coord_y;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            back = 1;
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
          } else
            back = 1;
        }
      }
      break;
    case 2:
      if (mouse_graf >= 10) {
        y0 = sqrt(abs(coord_x - x1) * abs(coord_x - x1) + abs(coord_y - y1) * abs(coord_y - y1));

        if (key(0xd) || key(0x4e))
          tension++;
        if (key(0xc) || (key(0x4a) && tension))
          tension--;
        y0 = tension;

        x0 = x1 - (long)((float)cos(ang) * y0);
        y0 = y1 + (long)((float)sin(ang) * y0);
        line_fx = polygon_tool_effect;
        undo_error = 0;
        bezier(x1, y1, coord_x, coord_y, x0, y0, coord_x, coord_y, 0);
        if (undo_error)
          s = 0;
        else {
          if (mouse_b & 1) {
            x0 = bezier_x - coord_x;
            y0 = coord_y - bezier_y;
            if (!x0 && !y0)
              ang = 0;
            else
              ang = atan2(y0, x0);
            x1 = coord_x;
            y1 = coord_y;
            do {
              read_mouse();
            } while (mouse_b & 1);
          } else if (mouse_b || key(_ESC)) {
            back = 1;
            do {
              read_mouse();
            } while (mouse_b || key(_ESC));
            s = 0;
          } else
            back = 1;
        }
      }
      break;
    }

    blur_enabled = 0;

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    blit_edit();

    if (back) {
      undo_back();
      if (need_zoom == 1)
        zoom_map();
      else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
        draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
      need_zoom_width = 0;
      need_zoom_height = 0;
      need_zoom = 0;
    }

  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && s != 1) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (rectangles)
//-----------------------------------------------------------------------------

void edit_mode_6(void) {
  int s;      // State 0-wait 1st click, 1-wait 2nd click
  int x0, y0; // Starting coordinates
  int w, h;
  int a;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = box_icons[2 + mode_rect];
  bar[14] = 0;
  bar[15] = 0;
  draw_bar(22);
  draw_ruler();

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &filled_tool_effect);
    select_color(2);

    w = map_width * 2;
    h = 0;
    back = 0;
    switch (s) {
    case 0:
      if ((mouse_b & 1) && mouse_graf >= 10) {
        s = 1;
        x0 = coord_x;
        y0 = coord_y;
        do {
          read_mouse();
        } while (mouse_b & 1);
      }
      break;
    case 1:
      if (mouse_graf >= 10) {
        line_fx = filled_tool_effect;
        w = coord_x - x0;
        h = coord_y - y0;
        if (key(_L_CTRL) || key(_R_CTRL)) {
          if (abs(w) > abs(h)) {
            if (h < 0)
              h = -abs(w);
            else
              h = abs(w);
          } else {
            if (w < 0)
              w = -abs(h);
            else
              w = abs(h);
          }
          undo_error = 0;
          draw_box(x0, y0, x0 + w, y0 + h);
          if (undo_error)
            s = 0;
        } else {
          undo_error = 0;
          draw_box(x0, y0, coord_x, coord_y);
          if (undo_error)
            s = 0;
        }
        if (mouse_b & 1) {
          s = 0;
          do {
            read_mouse();
          } while (mouse_b & 1);
        } else if (mouse_b || key(_ESC)) {
          do {
            read_mouse();
          } while (mouse_b || key(_ESC));
          s = 0;
          back = 1;
        } else
          back = 1;
      }
      break;
    }

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    select_rect(13);

    if (w == map_width * 2)
      analyze_bar(0, 0);
    else
      analyze_bar(abs(w) + 1, abs(h) + 1);
    blit_edit();

    if (back) {
      undo_back();
      if (need_zoom == 1)
        zoom_map();
      else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
        draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
      need_zoom_width = 0;
      need_zoom_height = 0;
      need_zoom = 0;
    }

  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && s != 1) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (circles)
//-----------------------------------------------------------------------------

void edit_mode_7(void) {
  int s;      // State 0-wait 1st click, 1-wait 2nd click
  int x0, y0; // Starting coordinates
  int w, h;
  int a;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = circle_icons[2 + mode_circle];
  bar[14] = 0;
  bar[15] = 0;
  draw_bar(22);
  draw_ruler();

  s = 0;
  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &circle_tool_effect);
    select_color(2);

    w = map_width * 2;
    h = 0;
    back = 0;
    switch (s) {
    case 0:
      if ((mouse_b & 1) && mouse_graf >= 10) {
        s = 1;
        x0 = coord_x;
        y0 = coord_y;
        do {
          read_mouse();
        } while (mouse_b & 1);
      }
      break;
    case 1:
      if (mouse_graf >= 10) {
        line_fx = circle_tool_effect;
        if (mode_circle < 2) {
          w = coord_x - x0;
          h = coord_y - y0;
          if (key(_L_CTRL) || key(_R_CTRL)) {
            if (abs(w) > abs(h)) {
              if (h < 0)
                h = -abs(w);
              else
                h = abs(w);
            } else {
              if (w < 0)
                w = -abs(h);
              else
                w = abs(h);
            }
            undo_error = 0;
            draw_circle(x0, y0, x0 + w, y0 + h, mode_circle & 1);
            if (undo_error)
              s = 0;
          } else {
            undo_error = 0;
            draw_circle(x0, y0, coord_x, coord_y, mode_circle & 1);
            if (undo_error)
              s = 0;
          }
        } else {
          w = coord_x - x0;
          h = coord_y - y0;
          if (key(_L_CTRL) || key(_R_CTRL)) {
            if (abs(w) > abs(h)) {
              if (h < 0)
                h = -abs(w);
              else
                h = abs(w);
            } else {
              if (w < 0)
                w = -abs(h);
              else
                w = abs(h);
            }
            undo_error = 0;
            draw_circle(x0 - w, y0 - w, x0 + w, y0 + w, mode_circle & 1);
            if (undo_error)
              s = 0;
          } else {
            undo_error = 0;
            draw_circle(x0 - w, y0 - h, x0 + w, y0 + h, mode_circle & 1);
            if (undo_error)
              s = 0;
          }
        }
        if (mouse_b & 1) {
          s = 0;
          do {
            read_mouse();
          } while (mouse_b & 1);
        } else if (mouse_b || key(_ESC)) {
          do {
            read_mouse();
          } while (mouse_b || key(_ESC));
          s = 0;
          back = 1;
        } else
          back = 1;
      }
      break;
    }

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    select_circle(13);

    if (w == map_width * 2)
      analyze_bar(0, 0);
    else
      analyze_bar(abs(w) + 1, abs(h) + 1);
    blit_edit();

    if (back) {
      undo_back();
      if (need_zoom == 1)
        zoom_map();
      else if (need_zoom == -1 && need_zoom_width > 0 && need_zoom_height > 0)
        draw_edit_background(need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height);
      need_zoom_width = 0;
      need_zoom_height = 0;
      need_zoom = 0;
    }

  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && s != 1) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (spray)
//-----------------------------------------------------------------------------

extern int frame_clock;

void init_rnd(int n, char *clave);
byte rndb();
int rnd();

void edit_mode_8(void) {
  byte col;
  int n, m, x, y;
  int u = undo_spray, ux, uy; // Number of saved actions and in which box

  clock_spray = -1;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (n = 2; n < 11; n++) {
    bar[n] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = spray_icons[2 + spray_mode];
  bar[14] = 0;
  draw_bar(0);
  draw_ruler();
  init_rnd(0, "");

  do {
    draw_help(1295);
    edit_ruler();
    select_color(2);
    select_fx(12, &spray_tool_effect);

    if (key(_D))
      blur_enabled = 1;

    if ((mouse_b & 1) && mouse_graf >= 10) {
      if (clock_spray != -1) {
        if (u == undo_spray || abs(coord_x - ux) > undo_box || abs(coord_y - uy) > undo_box) {
          u = 0;
          ux = coord_x;
          uy = coord_y;
          save_undo(coord_x - brush_w / 2 - undo_box, coord_y - brush_h / 2 - undo_box,
                    brush_w + undo_box * 2 + 1, brush_h + undo_box * 2 + 1);
        } else {
          zoom_region(coord_x - brush_w / 2, coord_y - brush_h / 2, brush_w * 2 + 1,
                      brush_h * 2 + 1);
          u++;
        }

        m = brush_w * brush_h * (frame_clock - clock_spray);
        clock_spray = frame_clock;

        if (texture_color == NULL) {
          while (m--) {
            if (rndb() < (8 << spray_mode)) {
              x = rnd() % brush_w;
              y = rnd() % brush_h;
              if (brush[x + y * brush_w]) {
                line_fx = spray_tool_effect;
                _line_pixel(coord_x + x - brush_w / 2, coord_y + y - brush_h / 2);
              }
            }
          }
        } else {
          col = color;
          while (m--) {
            if (rndb() < (8 << spray_mode)) {
              x = rnd() % brush_w;
              y = rnd() % brush_h;
              if (brush[x + y * brush_w]) {
                line_fx = spray_tool_effect;
                color = get_color(coord_x + x - brush_w / 2, coord_y + y - brush_h / 2);
                _line_pixel(coord_x + x - brush_w / 2, coord_y + y - brush_h / 2);
              }
            }
          }
          color = col;
        }
      } else
        clock_spray = frame_clock;

    } else
      clock_spray = -1;

    blur_enabled = 0;

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    n = spray_mode;
    select_spray(13);
    if (n != spray_mode)
      u = undo_spray;

    blit_edit();
  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (fills)
//-----------------------------------------------------------------------------

void edit_mode_9(void) {
  int a;

  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = fill_icons[2 + mode_fill];
  bar[13] = 0;
  draw_bar(0);
  draw_ruler();

  do {
    draw_help(1295);
    edit_ruler();
    select_fill(12);
    select_color(2);

    if ((mouse_b & 1) && !(prev_mouse_buttons & 1) && mouse_graf >= 10) {
      if (!mask[*(map + coord_y * map_width + coord_x)] &&
          (*(map + coord_y * map_width + coord_x) != color || mode_fill == 3 ||
           texture_color != NULL)) {
        fill(coord_x, coord_y);
      }
    }

    if (((mouse_b & 1) && selected_icon == 1) ||
        (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      undo_back();
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    blit_edit();

  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Edit screen (undo)
//-----------------------------------------------------------------------------

void edit_mode_11(void) {
  bar[0] = 101 + zoom;
  bar[1] = 153;
  bar[2] = 121;
  bar[3] = 120;
  bar[4] = 154;
  bar[5] = 0;
  draw_bar(0);

  do {
    draw_help(1295);
    read_mouse();
    select_zoom();
    test_mouse();

    if (mouse_b & 1)
      switch (selected_icon) {
      case 1:
        undo_back();
        break;
      case 2:
        undo_back();
        do {
          read_mouse();
        } while (mouse_b & 1);
        break;
      case 3:
        undo_next();
        do {
          read_mouse();
        } while (mouse_b & 1);
        break;
      case 4:
        undo_next();
        break;
      }
    if (scan_code == 14 && !key(_L_SHIFT) && !key(_R_SHIFT))
      undo_back();
    else if (scan_code == 14 && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    blit_edit();
  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));
}

//-----------------------------------------------------------------------------
//      Text
//-----------------------------------------------------------------------------

// TODO: Known bug -- screen refresh fails when combining undo with backspace during text deletion

int find_font_window(void);
void get_char_size_buffer(int WhatChar, int *width, int *height, char *buffer);
int show_char_buffer(int WhatChar, int cx, int cy, char *ptr, int w, char *buffer);

void edit_mode_12(void) {
  int spacelen, cnt, x, char_w, char_h, max_char_w, max_char_h;
  int ms = mode_selection;
  int a;
  int tx = -256, ty = -256; // Text start position (for enter/newline)
  byte *font, *buffer;
  int text_len = 0;
  byte char_widths[256];

  mode_selection = 0;
  bar[0] = 101 + zoom;
  bar[1] = 121;
  for (a = 2; a < 11; a++) {
    bar[a] = 1;
  }
  bar[11] = 166;
  bar[12] = 119;
  bar[13] = 0;
  draw_bar(0);
  draw_ruler();

  if ((x = find_font_window())) {
    // Pointer to font

    font = window[x].aux + RES_FOR_NAME;

    // Determine spacelen

    spacelen = 0;
    cnt = 0;
    for (x = 0; x < 255; x++) {
      get_char_size_buffer(x, &char_w, &char_h, (char *)font);
      if (char_w != 1) {
        cnt++;
        spacelen += char_w;
      }
    }
    spacelen = (spacelen / cnt) / 2;

    // Determine max width and height

    max_char_w = 0;
    max_char_h = 0;
    for (x = 0; x < 255; x++) {
      get_char_size_buffer(x, &char_w, &char_h, (char *)font);
      if (char_w == 1)
        char_w = spacelen;
      if (max_char_w < char_w)
        max_char_w = char_w;
      if (max_char_h < char_h)
        max_char_h = char_h;
    }

    // Allocate memory for the buffer

    if ((buffer = (byte *)malloc(max_char_w * max_char_h)) != NULL) {
      memset(buffer, 0, max_char_w * max_char_h);
    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
      font = NULL;
    }

  } else
    font = NULL; // Will draw with the editor's built-in font

  text_bar_active = 1;

  do {
    draw_help(1295);
    edit_ruler();
    select_fx(12, &text_tool_effect);
    select_color(2);

    test_previous();
    if (!hotkey)
      poll_keyboard();

    if (font == NULL) {
      // Write with the system font

      if (mouse_graf >= 10 && (mouse_b & 1) && !key(_SPC)) {
        sel_x0 = coord_x - font_width / 2;
        sel_y0 = coord_y - font_height / 2;
        sel_x1 = sel_x0 + font_width - 1;
        sel_y1 = sel_y0 + font_height - 1;
        sel_status = 1;
        hotkey = 0;
        tx = sel_x0;
        ty = sel_y0;
      } else if (((mouse_b & 2) || key(_ESC)) && sel_status) {
        sel_status = 0;
        hotkey = 1;
        do {
          read_mouse();
        } while ((mouse_b & 2) || key(_ESC));
      }
      if (sel_status == 1) {
        if (ascii == 13) {
          ty += font_height;
          sel_x0 = tx;
          sel_y0 = ty;
          sel_x1 = sel_x0 + font_width - 1;
          sel_y1 = sel_y0 + font_height - 1;
        } else if (scan_code == 14) {
          if (sel_x0 != tx) {
            if (undo_back()) {
              test_previous();
              sel_x0 -= font_width;
              sel_x1 -= font_width;
            }
          }
        } else if (ascii && scan_code != 15 && scan_code != 1 && text_len < 256) {
          if (save_undo(sel_x0, sel_y0, font_width, font_height)) {
            test_previous(); // To counteract the effect of save_undo()
            line_fx = text_tool_effect;
            write_char(sel_x0, sel_y0, ascii);
            sel_x0 += font_width;
            sel_x1 += font_width;
          }
        }
      }
    } else {
      // Write with the selected font

      if (mouse_graf >= 10 && (mouse_b & 1)) {
        sel_x0 = coord_x - spacelen / 2;
        sel_y0 = coord_y - max_char_h / 2;
        sel_x1 = sel_x0 + spacelen - 1;
        sel_y1 = sel_y0 + max_char_h - 1;
        sel_status = 1;
        hotkey = 0;
        tx = sel_x0;
        ty = sel_y0;
        text_len = 0;
      } else if (((mouse_b & 2) || key(_ESC)) && sel_status) {
        sel_status = 0;
        hotkey = 1;
        do {
          read_mouse();
        } while ((mouse_b & 2) || key(_ESC));
      }
      if (sel_status == 1) {
        if (ascii == 13) {
          ty += max_char_h;
          sel_x0 = tx;
          sel_y0 = ty;
          sel_x1 = sel_x0 + spacelen - 1;
          sel_y1 = sel_y0 + max_char_h - 1;
          text_len = 0;
        } else if (scan_code == 14) {
          if (sel_x0 != tx) {
            if (undo_back()) {
              test_previous();
              sel_x0 -= char_widths[--text_len];
              sel_x1 -= char_widths[text_len];
            }
          }
        } else if (ascii && scan_code != 15 && scan_code != 1 && text_len < 256) {
          get_char_size_buffer(ascii, &char_w, &char_h, (char *)font);
          if (char_w == 1) {
            if (save_undo(sel_x0, sel_y1, 1, 1)) {
              test_previous();
              sel_x0 += spacelen;
              sel_x1 += spacelen;
              char_widths[text_len++] = spacelen;
            }
          } else {
            memset(buffer, 0, max_char_w * max_char_h);
            if (save_undo(sel_x0, sel_y0, char_w, max_char_h)) {
              test_previous(); // To counteract the effect of save_undo()
              show_char_buffer(ascii, 0, 0, (char *)buffer, max_char_w, (char *)font);
              line_fx = text_tool_effect;
              write_char2(sel_x0, sel_y0, buffer, max_char_w, max_char_h);
              sel_x0 += char_w;
              sel_x1 += char_w;
              char_widths[text_len++] = char_w;
            }
          }
        }
      }
    }

    test_next();

    if (((mouse_b & 1) && selected_icon == 1) ||
        ((scan_code == 14 && hotkey) && !key(_L_SHIFT) && !key(_R_SHIFT))) {
      if (sel_x0 != tx) {
        if (undo_back()) {
          if (font == NULL) {
            sel_x0 -= font_width;
            sel_x1 -= font_width;
          } else {
            sel_x0 -= char_widths[--text_len];
            sel_x1 -= char_widths[text_len];
          }
        }
      } else
        undo_back();
      need_zoom = 1;
      do {
        read_mouse();
      } while (mouse_b & 1);
    } else if ((scan_code == 14 && hotkey) && (key(_L_SHIFT) || key(_R_SHIFT))) {
      undo_next();
      need_zoom = 1;
    }

    blit_edit();
  } while (!exit_requested && !(mouse_b & 2) && !(key(_ESC) && hotkey) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

  text_bar_active = 0;

  hotkey = 1;
  sel_status = 0;
  mode_selection = ms;
  if (font != NULL)
    free(buffer);
}

//-----------------------------------------------------------------------------
//      Write a system font character onto a map (using 'color')
//-----------------------------------------------------------------------------

void write_char(int x, int y, byte c) {
  int n, m;
  byte *si;
  byte col;

  if (texture_color == NULL) {
    si = font + c * font_width * font_height;
    n = font_height;
    do {
      m = font_width;
      do {
        if (*si)
          _line_pixel(x, y);
        si++;
        x++;
      } while (--m);
      x -= font_width;
      y++;
    } while (--n);
  } else {
    col = color;
    si = font + c * font_width * font_height;
    n = font_height;
    do {
      m = font_width;
      do {
        color = get_color(x, y);
        if (*si)
          _line_pixel(x, y);
        si++;
        x++;
      } while (--m);
      x -= font_width;
      y++;
    } while (--n);
    color = col;
  }
}

void write_char2(int x, int y, byte *si, int font_width, int font_height) {
  int n, m;
  byte c = color, *g;

  if (texture_color != NULL) {
    n = font_height;
    do {
      m = font_width;
      do {
        if (*si) {
          color = *(ghost + (memptrsize)get_color(x, y) * 256 + *si);
          _line_pixel(x, y);
        }
        si++;
        x++;
      } while (--m);
      x -= font_width;
      y++;
    } while (--n);
  } else if (!c) {
    n = font_height;
    do {
      m = font_width;
      do {
        if (*si) {
          color = *si;
          _line_pixel(x, y);
        }
        si++;
        x++;
      } while (--m);
      x -= font_width;
      y++;
    } while (--n);
  } else {
    g = ghost + (memptrsize)c * 256;
    n = font_height;
    do {
      m = font_width;
      do {
        if (*si) {
          color = *(g + *si);
          _line_pixel(x, y);
        }
        si++;
        x++;
      } while (--m);
      x -= font_width;
      y++;
    } while (--n);
  }
  color = c;
}

//-----------------------------------------------------------------------------
//      Control points
//-----------------------------------------------------------------------------

void edit_mode_13(void) {
  byte *p;
  int m;
  char num[8];
  int m_s = mode_selection;

  bar[0] = 101 + zoom;
  bar[1] = 192;
  bar[2] = 1;
  bar[3] = 193;
  bar[4] = 0;
  bar[5] = 0;
  draw_bar(22);
  mode_selection = -1;

  do {
    draw_help(1295);
    read_mouse();
    select_zoom();
    test_mouse();

    switch (scan_code) {
    case 0x4A:
      point_index--;
      break; // -
    case 0x4E:
      point_index++;
      break; // +
    case 0x49:
      point_index -= 10;
      break; // PgUp
    case 0x51:
      point_index += 10;
      break; // PgDn
    case 0x47:
      point_index = 0;
      break;   // Home
    case 0x4F: // End
      for (m = 0; m < 512; m += 2)
        if (v.mapa->points[m] != -1)
          point_index = m / 2;
      break;
    }

    if (mouse_b & 1)
      switch (selected_icon) {
      case 1:
        point_index--;
        do {
          read_mouse();
        } while (mouse_b & 1);
        break;
      case 3:
        point_index++;
        do {
          read_mouse();
        } while (mouse_b & 1);
        break;
      }

    if (point_index < 0)
      point_index = 0;
    if (point_index > 255)
      point_index = 255;

    need_zoom_x = map_width;
    need_zoom_y = map_height;
    need_zoom_width = 0;
    need_zoom_height = 0;

    for (m = 0; m < 512; m += 2)
      if (v.mapa->points[m] != -1) {
        sel_x0 = v.mapa->points[m] - 1;
        sel_x1 = v.mapa->points[m] + 1;
        sel_y0 = v.mapa->points[m + 1] - 1;
        sel_y1 = v.mapa->points[m + 1] + 1;
        test_sel();
      }

    if (scan_code == 0x53) { // Delete
      v.mapa->points[point_index * 2] = -1;
      v.mapa->points[point_index * 2 + 1] = -1;
    }

    if (mouse_graf >= 10 && (mouse_b & 1)) {
      if ((v.mapa->points[point_index * 2] == coord_x) &
          (v.mapa->points[point_index * 2 + 1] == coord_y)) {
        v.mapa->points[point_index * 2] = -1;
        v.mapa->points[point_index * 2 + 1] = -1;
      } else {
        v.mapa->points[point_index * 2] = coord_x;
        v.mapa->points[point_index * 2 + 1] = coord_y;
      }
      do
        read_mouse();
      while (mouse_b & 1);
      sel_x0 = coord_x - 1;
      sel_x1 = coord_x + 1;
      sel_y0 = coord_y - 1;
      sel_y1 = coord_y + 1;
      test_sel();
    }

    sel_status = 1;
    test_next();

    if (v.mapa->points[point_index * 2] != -1)
      analyze_bar(v.mapa->points[point_index * 2], v.mapa->points[point_index * 2 + 1]);
    else {
      wbox(toolbar, vga_width / big2, vga_height, c2, toolbar_width - 23, 2, 21, 15);
      p = screen_buffer;
      screen_buffer = toolbar;
      text_color = c4;
      writetxt(toolbar_width - 22, 3, 0, (byte *)"????");
      writetxt(toolbar_width - 22, 10, 0, (byte *)"????");
      screen_buffer = p;
    }

    wbox(toolbar, vga_width / big2, 19, c2, 48 + 2 * 16, 2, 15, 15);

    num[3] = 0;
    num[2] = 48 + point_index % 10;
    num[1] = 48 + (point_index / 10) % 10;
    num[0] = 48 + (point_index / 100) % 10;

    wwrite(toolbar, vga_width / big2, 19, 56 + 2 * 16, 6, 1, (byte *)num, c1);
    wwrite(toolbar, vga_width / big2, 19, 55 + 2 * 16, 6, 1, (byte *)num, c4);

    blit_edit();

  } while (!exit_requested && !(mouse_b & 2) && !key(_ESC) && draw_mode < 100 &&
           !(mouse_b && mouse_in(toolbar_x, toolbar_y + 10, toolbar_x + 9, toolbar_y + 18)));

  sel_status = 0;
  mode_selection = m_s;
}
