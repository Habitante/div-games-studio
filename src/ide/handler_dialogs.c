//-----------------------------------------------------------------------------
//      Dialog boxes, listbox UI, file browser, and progress bar
//-----------------------------------------------------------------------------

#include "handler_internal.h"

///////////////////////////////////////////////////////////////////////////////
//      Palette window
///////////////////////////////////////////////////////////////////////////////

#define PIXELS_PER_COLOR 4

void palette0(void) {
  v.type = WIN_PALETTE; // Palette
  v.w = 16 * PIXELS_PER_COLOR + 3;
  v.h = 16 * PIXELS_PER_COLOR + 11;
  v.title = texts[51];
  v.name = texts[51];
  v.paint_handler = palette1;
}

void palette1(void) {
  int x, y;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, w, h, x + y * 16, 2 + x * PIXELS_PER_COLOR, 10 + y * PIXELS_PER_COLOR,
           PIXELS_PER_COLOR - 1, PIXELS_PER_COLOR - 1);
}

///////////////////////////////////////////////////////////////////////////////
//      Dialog box to enter information about the user
///////////////////////////////////////////////////////////////////////////////

char user1[128] = "";
char user2[128] = "";

void user_info1(void) {
  int w = v.w / big2, h = v.h / big2;

  _show_items();
  wput(v.ptr, w, h, 5, 10, -50);

  wwrite(v.ptr, w, h, 91, 13, 0, texts[462], c1);
  wwrite(v.ptr, w, h, 90, 13, 0, texts[462], c4);
  wwrite(v.ptr, w, h, 91, 12 + 38 + 11, 0, texts[484], c1);
  wwrite(v.ptr, w, h, 90, 12 + 38 + 11, 0, texts[484], c4);
}

void user_info2(void) {
  _process_items();
  if (v.active_item == 0) {
    if (strlen(user1) && strlen(user2)) {
      v_accept = 1;
      end_dialog = 1;
    } else {
      if (!strlen(user1)) {
        _select_new_item(1);
      } else if (!strlen(user2)) {
        _select_new_item(2);
      }
    }
  }
}

void user_info3(void) {
  FILE *f;

  if (strlen(user1) || strlen(user2)) {
    if ((f = fopen("system/user.nfo", "wb")) != NULL) {
      fwrite(user1, 1, 128, f);
      fwrite(user2, 1, 128, f);
      fclose(f);
    }
  }
}

void user_info0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[485];
  v.w = 160 + 86;
  v.h = 10 + 47 + 31;

  v.paint_handler = user_info1;
  v.click_handler = user_info2;
  v.close_handler = user_info3;

  _button(100, v.w - 8, v.h - 14, 2);

  _get(460, 4 + 86, 22, v.w - 8 - 86, (byte *)user1, 127, 0, 0);
  _get(461, 4 + 86, 22 + 19, v.w - 8 - 86, (byte *)user2, 127, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
//      Copyright Dialog Box
///////////////////////////////////////////////////////////////////////////////

extern int new_session;

void copyright0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[35];
  v.w = 232;
  v.h = 12 + 47 + 31;
  v.paint_handler = copyright1;
  v.click_handler = copyright2;
  if (copy_desktop) {
    _button(456, v.w - 11, v.h - 16, 2);    // Continue
    _button(457, 48 + 48 + 3, v.h - 16, 0); // New session
  } else {
    _button(457, v.w - 11, v.h - 16, 2); // New session
  }
}

void copyright1(void) {
  int w = v.w / big2, h = v.h / big2;

  _show_items();
  wput(v.ptr, w, h, 6, 11, -50);

  wrectangle(v.ptr, w, h, c1, 48 + 48, 23, 129, 19);
  wbox(v.ptr, w, h, c12, 48 + 48 + 1, 23 + 1, 129 - 2, 19 - 2);

  wwrite(v.ptr, w, h, 48 + 48 + 1, 14, 0, texts[485], c1);
  wwrite(v.ptr, w, h, 48 + 48, 14, 0, texts[485], c4);

#ifndef SHARE
  wwrite(v.ptr, w, h, 48 + 48, 45, 0, texts[36], c12);
#else
  wwrite(v.ptr, w, h, 48 + 48, 45, 0, texts[37], c12);
#endif

  wwrite(v.ptr, w, h, 96, 45 + 7, 0, texts[486], c12);

  // User info:

  wwrite_in_box(v.ptr, w, w - 9, h, 99, 25, 0, (byte *)user1, c1);
  wwrite_in_box(v.ptr, w, w - 9, h, 98, 25, 0, (byte *)user1, c23);
  wwrite_in_box(v.ptr, w, w - 9, h, 99, 25 + 8, 0, (byte *)user2, c1);
  wwrite_in_box(v.ptr, w, w - 9, h, 98, 25 + 8, 0, (byte *)user2, c23);

  wwrite(v.ptr, w, h, 48 + 48 + 1, 61, 0, texts[484], c1);
  wwrite(v.ptr, w, h, 48 + 48, 61, 0, texts[484], c4);
}

void copyright2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
  if (v.active_item == 1) {
    new_session = 1;
    end_dialog = 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
//      ERROR dialog box
///////////////////////////////////////////////////////////////////////////////

void err0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[41];
  v.w = text_len((byte *)v_text) + 8;
  v.h = 38; // Note: error_window=malloc(640*38*2)
  if (v.w > 320)
    v.w = 320;
  v.paint_handler = err1;
  v.click_handler = err2;
  _button(100, v.w / 2, v.h - 14, 1);
}

void err1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, (byte *)v_text, c3);
}

void err2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

//-----------------------------------------------------------------------------
//      Error window with 'HELP' button
//-----------------------------------------------------------------------------

void errhlp1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, (byte *)v_text, c3);
}

void errhlp2(void) {
  _process_items();
  if (v.active_item == 0) {
    end_dialog = 1;
  }
  if (v.active_item == 1) {
    v_accept = 1;
    end_dialog = 1;
  }
}

void errhlp0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[41];
  v.w = text_len((byte *)v_text) + 8;
  v.h = 38; // Note: error_window=malloc(640*38*2)
  if (v.w > 320)
    v.w = 320;
  v.paint_handler = errhlp1;
  v.click_handler = errhlp2;
  v_accept = 0;
  _button(100, 7, v.h - 14, 0);
  _button(125, v.w - 8, v.h - 14, 2);
}

//-----------------------------------------------------------------------------
//      Info Message
//-----------------------------------------------------------------------------

void info0(void) {
  err0();
  v.title = (byte *)v_title;
}

//-----------------------------------------------------------------------------
//      Test new video mode
//-----------------------------------------------------------------------------

extern int *system_clock;
int old_system_clock;

void test1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, 5, 12, 0, (byte *)v_text, c1);
  wwrite(v.ptr, w, h, 4, 12, 0, (byte *)v_text, c3);
}

void test2(void) {
  _process_items();
  if (v.active_item == 0) {
    exit_requested = 0;
    return_mode = 0;
    end_dialog = 1;
  }
  if (*system_clock > old_system_clock + 180)
    end_dialog = 1;
}

void test0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[384];
  v_text = (char *)texts[383];
  v.w = text_len((byte *)v_text) + 9;
  v.h = 38;
  v.paint_handler = test1;
  v.click_handler = test2;
  _button(100, v.w / 2, v.h - 14, 1);

  exit_requested = 1;
  return_mode = 3;
  old_system_clock = *system_clock;
}

//-----------------------------------------------------------------------------
//      OK / Cancel Dialog with or without title text
//-----------------------------------------------------------------------------

void accept0(void) {
  v.type = WIN_DIALOG;
  v.w = text_len(texts[100]) + text_len(texts[101]) + 24;
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
  v.paint_handler = accept1;
  v.click_handler = accept2;
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  v_accept = 0;
}

void accept1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  if (v_text != NULL)
    wwrite(v.ptr, w, h, w / 2, 12, 1, (byte *)v_text, c3);
}

void accept2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  }
}

//-----------------------------------------------------------------------------
//      Dialog box for handling palette when opening with v_title and/or v_text
//-----------------------------------------------------------------------------

void palette_action1(void);
void palette_action2(void);
void palette_action3(void);
int palette_option[3];
byte work_palette[768];

void palette_action0(void) {
  v.type = WIN_DIALOG;
  v.w = 160;
  v.h = 59 + 77;

  v.title = (byte *)(v_title = (char *)texts[568]);

  v.paint_handler = palette_action1;
  v.click_handler = palette_action2;
  v.close_handler = palette_action3;

  palette_option[0] = 1;
  palette_option[1] = 0;
  palette_option[2] = 0;

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _flag(569, 4, 12 + 77, &palette_option[0]);
  _flag(570, 4, 22 + 77, &palette_option[1]);
  _flag(571, 4, 32 + 77, &palette_option[2]);

  v_accept = 0;
}

byte fast_find_color(byte fr, byte fg, byte fb);
byte find_color(byte r, byte g, byte b);

void palette_action1(void) {
  int w = v.w / big2, h = v.h / big2;
  int xx = 91, x, y;
  byte c;
  byte d4[768];

  _show_items();

  wbox(v.ptr, w, h, c0, 4, 12, 65, 65 + 10);
  wbox(v.ptr, w, h, c0, xx, 12, 65, 65 + 10);

  wbox(v.ptr, w, h, c1, 5, 13, 63, 9);
  wbox(v.ptr, w, h, c1, xx + 1, 13, 63, 9);

  if (palette_option[1]) {
    wrectangle(v.ptr, w, h, c0, 4, 12, w - 8, 11);
    wbox(v.ptr, w, h, c12, 5, 13, w - 10, 9);
    wwrite(v.ptr, w, h, w / 2 + 1, 14, 1, (byte *)"+", c0);
    wwrite(v.ptr, w, h, w / 2, 14, 1, (byte *)"+", c4);
  }

  if (palette_option[0] || palette_option[1]) {
    wbox(v.ptr, w, h, c12, 5, 13, 63, 9);
    wwrite(v.ptr, w, h, 4 + 33, 14, 1, texts[576], c0);
    c = c4;
  } else
    c = c3;
  wwrite(v.ptr, w, h, 4 + 32, 14, 1, texts[576], c);

  if (palette_option[1] || palette_option[2]) {
    wbox(v.ptr, w, h, c12, xx + 1, 13, 63, 9);
    wwrite(v.ptr, w, h, xx + 33, 14, 1, texts[577], c0);
    c = c4;
  } else
    c = c3;
  wwrite(v.ptr, w, h, xx + 32, 14, 1, texts[577], c);

  memcpy(d4, dac4, 768);
  create_dac4();

  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      wbox(v.ptr, w, h, x + y * 16, 5 + x * 4, 23 + y * 4, 3, 3);
      c = fast_find_color(work_palette[(x + y * 16) * 3 + 0], work_palette[(x + y * 16) * 3 + 1],
                          work_palette[(x + y * 16) * 3 + 2]);
      wbox(v.ptr, w, h, c, xx + 1 + x * 4, 23 + y * 4, 3, 3);
    }
  }

  memcpy(dac4, d4, 768);
}

void palette_action2(void) {
  int need_refresh = 0;

  _process_items();

  if ((mouse_b & 1) && wmouse_in(4, 12, 65, 65 + 10))
    v.active_item = 2;
  if ((mouse_b & 1) && wmouse_in(4 + 65, 12, 65, 65 + 10))
    v.active_item = 3;
  if ((mouse_b & 1) && wmouse_in(91, 12, 65, 65 + 10))
    v.active_item = 4;

  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    palette_option[0] = 1;
    palette_option[1] = 0;
    palette_option[2] = 0;
    need_refresh = 1;
    break;
  case 3:
    palette_option[0] = 0;
    palette_option[1] = 1;
    palette_option[2] = 0;
    need_refresh = 1;
    break;
  case 4:
    palette_option[0] = 0;
    palette_option[1] = 0;
    palette_option[2] = 1;
    need_refresh = 1;
    break;
  }

  if (need_refresh) {
    call((void_return_type_t)v.paint_handler);
    v.redraw = 1;
  }
}
void palette_action3(void) {
  if (v_accept && palette_option[1])
    v_accept = 2;
  if (v_accept && palette_option[2])
    v_accept = 3;
}

//-----------------------------------------------------------------------------
//      Lists for the file open window
//-----------------------------------------------------------------------------

char files_buf[MAX_FILES * FILE_SIZE];
struct t_listbox file_list = {3, 49, files_buf, FILE_SIZE, 12, 64};

char dirs_buf[MAX_FOLDERS * FOLDER_SIZE];
struct t_listbox dir_list = {80, 49, dirs_buf, FOLDER_SIZE, 10, 64};

#define MAX_DRIVES  26
#define DRIVE_WIDTH (4 + 1)
char drives_buf[MAX_DRIVES * DRIVE_WIDTH];
struct t_listbox drive_list = {157, 49, drives_buf, DRIVE_WIDTH, 4, 28};

#define MAX_EXTENSIONS 26
#define EXT_WIDTH      (5 + 1)
char ext[MAX_EXTENSIONS * EXT_WIDTH];
struct t_listbox ext_list = {157, 97, ext, EXT_WIDTH, 4, 28};

///////////////////////////////////////////////////////////////////////////////
//      Draw listbox and slider elements
///////////////////////////////////////////////////////////////////////////////

void paint_listbox(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  int n, y;
  char *p;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wbox(ptr, w, h, c1, l->x + 1, l->y + 1, l->w - 2, l->h - 2);

  if ((n = l->total_items)) { // If there are items in the list
    n -= l->first_visible;
    p = l->list + l->item_width * l->first_visible;
    y = l->y + 2;
    if (n > l->visible_items)
      n = l->visible_items;
    do {
      wwrite_in_box(ptr + (l->x + 2) * big2, w, l->w - 4, h, 0, y, 0, (byte *)p, c3);
      p += l->item_width;
      y += 8;
    } while (--n);
  }

  if (wmouse_in(l->x, l->y, l->w - 1, l->h)) { // Calculate zone
    l->zone = ((mouse_y - v.y) - (l->y + 2) * big2) / (8 * big2);
    if (l->total_items <= l->zone || l->zone >= l->visible_items)
      l->zone = 1;
    else
      l->zone += 10;
  } else if (wmouse_in(l->x + l->w - 1, l->y, 9, 9))
    l->zone = 2;
  else if (wmouse_in(l->x + l->w - 1, l->y + l->h - 9, 9, 9))
    l->zone = 3;
  else if (wmouse_in(l->x + l->w - 1, l->y + 9, 9, l->h - 18))
    l->zone = 4;
  else
    l->zone = 0;

  if (l->zone >= 10) { // Highlight zone
    wwrite_in_box(ptr + (l->x + 2) * big2, w, l->w - 4, h, 0, l->y + 2 + (l->zone - 10) * 8, 0,
                  (byte *)l->list + l->item_width * (l->first_visible + l->zone - 10), c4);
    v.redraw = 1;
  }
}

void paint_slider(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wbox(ptr, w, h, c2, l->x + l->w, l->y + 9, 7, l->h - 18);
  if (l->slide > l->s0)
    wbox(ptr, w, h, c0, l->x + l->w, l->slide - 1, 7, 1);
  if (l->slide < l->s1)
    wbox(ptr, w, h, c0, l->x + l->w, l->slide + 3, 7, 1);
  wput(ptr, w, h, l->x + l->w, l->slide, 43);
}

//-----------------------------------------------------------------------------
//      Draw listbox
//-----------------------------------------------------------------------------

void create_listbox(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  if (!l->created) {
    if (!l->w)
      l->w = (l->item_width - 1) * 6 + 3;
    if (!l->h)
      l->h = l->visible_items * 8 + 3;
    l->slide = l->s0 = l->y + 9;
    l->s1 = l->y + l->h - 12;
    l->buttons = 0;
    l->created = 1;
    l->zone = 0;
    if (l == &file_list) {
      l->first_visible = file_types[v_type].first_visible;
      if ((l->first_visible + l->visible_items) > l->total_items) {
        l->first_visible = 0;
      }
    } else
      l->first_visible = 0;
  }

  wrectangle(ptr, w, h, c0, l->x, l->y, l->w, l->h);
  wbox(ptr, w, h, c1, l->x + 1, l->y + 1, l->w - 2, l->h - 2);
  wrectangle(ptr, w, h, c0, l->x + l->w - 1, l->y, 9, l->h);
  wrectangle(ptr, w, h, c0, l->x + l->w - 1, l->y + 8, 9, l->h - 16);
  wput(ptr, w, h, l->x + l->w, l->y + 1, -39);
  wput(ptr, w, h, l->x + l->w, l->y + l->h - 8, -40);

  paint_listbox(l);
  paint_slider(l);
}

//-----------------------------------------------------------------------------
//      Update a listbox, depending on mouse movements
//-----------------------------------------------------------------------------

void update_listbox(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int n, old_zona = l->zone;

  if (!l->h)
    l->h = l->visible_items * 8 + 3;

  if (wmouse_in(l->x, l->y, l->w - 1, l->h)) { // Calculate zone
    l->zone = ((mouse_y - v.y) - (l->y + 2) * big2) / (8 * big2);
    if (l->total_items <= l->zone || l->zone >= l->visible_items)
      l->zone = 1;
    else
      l->zone += 10;
  } else if (wmouse_in(l->x + l->w - 1, l->y, 9, 9))
    l->zone = 2;
  else if (wmouse_in(l->x + l->w - 1, l->y + l->h - 9, 9, 9))
    l->zone = 3;
  else if (wmouse_in(l->x + l->w - 1, l->y + 9, 9, l->h - 18))
    l->zone = 4;
  else
    l->zone = 0;

  if (old_zona != l->zone)
    if (old_zona >= 10) { // Unhighlight zone
      wwrite_in_box(ptr + (l->x + 2) * big2, w, l->w - 4, h, 0, l->y + 2 + (old_zona - 10) * 8, 0,
                    (byte *)l->list + l->item_width * (l->first_visible + old_zona - 10), c3);
      v.redraw = 1;
    }

  if ((mouse_b & 8 && l->zone > 0) ||
      (l->zone == 2 &&
       ((mouse_b & 1) || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))))) {
    if (!v_pause || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))) {
      if ((prev_mouse_buttons & 1) && !v_pause) {
        retrace_wait();
        retrace_wait();
      }
      if (l->first_visible) {
        l->first_visible--;
        paint_listbox(l);
        v.redraw = 1;
      }
    }
    wput(ptr, w, h, l->x + l->w, l->y + 1, -41);
    l->buttons |= 1;
    v.redraw = 1;
  } else if (l->buttons & 1) {
    wput(ptr, w, h, l->x + l->w, l->y + 1, -39);
    l->buttons ^= 1;
    v.redraw = 1;
  }

  if ((mouse_b & 4 && l->zone > 0) ||
      (l->zone == 3 &&
       ((mouse_b & 1) || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))))) {
    if (!v_pause || (v_pause && !(mouse_b & 1) && (prev_mouse_buttons & 1))) {
      if ((prev_mouse_buttons & 1) && !v_pause) {
        retrace_wait();
        retrace_wait();
      }
      n = l->total_items - l->first_visible;
      if (n > l->visible_items) {
        l->first_visible++;
        paint_listbox(l);
        v.redraw = 1;
      }
    }
    wput(ptr, w, h, l->x + l->w, l->y + l->h - 8, -42);
    l->buttons |= 2;
    v.redraw = 1;
  } else if (l->buttons & 2) {
    wput(ptr, w, h, l->x + l->w, l->y + l->h - 8, -40);
    l->buttons ^= 2;
    v.redraw = 1;
  }

  if (l->zone == 4 && (mouse_b & 1)) {
    l->slide = wmouse_y - 1;
    if (l->slide < l->s0)
      l->slide = l->s0;
    else if (l->slide > l->s1)
      l->slide = l->s1;
    if (l->total_items > l->visible_items) {
      n = l->total_items - l->visible_items;
      n = 0.5 + (float)(n * (l->slide - l->s0)) / (l->s1 - l->s0);
      if (n != l->first_visible) {
        l->first_visible = n;
        paint_listbox(l);
      }
    }
    paint_slider(l);
    v.redraw = 1;
  } else {
    if (l->total_items <= l->visible_items)
      n = l->s0;
    else {
      n = l->total_items - l->visible_items;
      n = (l->s0 * (n - l->first_visible) + l->s1 * (l->first_visible)) / n;
    }
    if (n != l->slide) {
      l->slide = n;
      paint_slider(l);
      v.redraw = 1;
    }
  }

  if (old_zona != l->zone)
    if (l->zone >= 10) { // Highlight zone
      wwrite_in_box(ptr + (l->x + 2) * big2, w, l->w - 4, h, 0, l->y + 2 + (l->zone - 10) * 8, 0,
                    (byte *)l->list + l->item_width * (l->first_visible + l->zone - 10), c4);
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

//-----------------------------------------------------------------------------
//     Analyzes a file name (for input + enter, accept or double-click)
//-----------------------------------------------------------------------------

void create_listbox_br(struct t_listboxbr *l);
void print_path_br(void);
void open_dir_br(void);
extern struct t_listboxbr file_list_br;
extern struct t_listbox dir_list_br;
extern t_thumb thumb[MAX_FILES];
extern int num_taggeds;

void analyze_input(void) {
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  struct find_t fileinfo;
  unsigned n;

  // Remove blank name
  // Note: overlapping memmove to shift chars left (not a buffer overflow risk)
  while (strchr(input, ' '))
    memmove(strchr(input, ' '), strchr(input, ' ') + 1, strlen(strchr(input, ' ') + 1) + 1);

  if (strlen(input) == 0)
    div_strcpy(input, sizeof(input), file_mask);
  else if ((n = strlen(input)) > 1)
    if (input[n - 1] == '.' && input[n - 2] == '.')
      div_strcat(input, sizeof(input), "/");

  if (strchr(input, ':') != NULL)
    if (strchr(drives, toupper(*(strchr(input, ':') - 1))) == NULL) {
      div_strcpy(input, sizeof(input), file_mask);
      return;
    }

  if (_fullpath(full, input, _MAX_PATH) != NULL) {
    strupr(full);
    div_strcpy(input, sizeof(input), file_mask);
    _splitpath(full, drive, dir, fname, ext);
    _dos_setdrive(drive[0] - 'A' + 1, &n);
    //    getcwd(full,PATH_MAX);
    if (true || (full[0] == drive[0])) {
      if (strlen(dir) > 1)
        if (IS_PATH_SEP(dir[strlen(dir) - 1]))
          dir[strlen(dir) - 1] = 0;
      if (!strlen(dir) || !chdir(dir)) {
        div_strcpy(input, sizeof(input), fname);
        div_strcat(input, sizeof(input), ext);
        if (strlen(input)) {
          // FILE NAME PROCESSING LOGIC

          // if it has wildcards
          //   if it matches no files, use it as the new mask
          //   if it matches one file, expand its name into input (editing)
          //   if it matches more than one, use it as the new mask
          // if it has no wildcards
          //   if the file doesn't exist ... (depends on open/save mode)
          //   if it exists as a file ... (depends on open/save mode)
          //   if it exists as a directory, change to that directory

          // (the other difference between open/save is that on entering save
          // status_abrir=1 and input="")


          if (strchr(input, '*') != NULL || strchr(input, '?') != NULL) { // Wildcards

            if (!_dos_findfirst(input, _A_NORMAL, &fileinfo)) { // If any... ?

              if (_dos_findnext(&fileinfo)) { // if there is ONLY one ...
                div_strcpy(input, sizeof(input), fileinfo.name);

              } else {
                DIV_STRCPY(file_mask, input); // if there are MORE than one ...
                printf("input: %s\n", input);
              }
            } else { // if none were found ...

              // if it had no extension and the mask does ...
              if (strchr(input, '.') == NULL && strchr(file_mask, '.') != NULL) {
                div_strcat(input, sizeof(input),
                           strchr(file_mask, '.')); // append the mask extension

                if (!_dos_findfirst(input, _A_NORMAL, &fileinfo)) { // if any found ...
                  if (_dos_findnext(&fileinfo)) {                   // if there is ONLY one
                    div_strcpy(input, sizeof(input), fileinfo.name);
                  } else
                    DIV_STRCPY(file_mask, input); // if there are MORE than one
                } else
                  DIV_STRCPY(file_mask, input);
              } else
                DIV_STRCPY(file_mask, input);
            }

          } else // No wildcards ...
            if (_dos_findfirst(input, _A_SUBDIR, &fileinfo) == 0)
              if (fileinfo.attrib & 16) {
                debugprintf("chdir %s\n", input);
                chdir(input);
                div_strcpy(input, sizeof(input), file_mask);
              } else {
                v_finished = 1;
                v_exists = 1;
              } // File found
            else if (strchr(input, '.') == NULL && strchr(file_mask, '.') != NULL) {
              div_strcat(input, sizeof(input), strchr(file_mask, '.'));
              if (!_dos_findfirst(input, _A_NORMAL, &fileinfo))
                if (_dos_findnext(&fileinfo)) {
                  div_strcpy(input, sizeof(input), fileinfo.name);
                } else {
                  DIV_STRCPY(file_mask, input);
                  printf("2584 input %s\n", input);
                }
              else if (strchr(input, '*') != NULL || strchr(input, '?') != NULL) {
                DIV_STRCPY(file_mask, input);
              } else {
                v_finished = 1;
                v_exists = 0;
              } // File not found (with appended extension)
            } else {
              v_finished = 1;
              v_exists = 0;
            } // File not found (without appended extension)
        } else {
          div_strcpy(input, sizeof(input), file_mask); // Keep the old mask
        }
        getcwd(file_types[v_type].path, PATH_MAX + 1);
        print_path_br();

        file_list.created = 0;
        dir_list.created = 0;
        file_types[v_type].first_visible = 0;
        open_dir_br();
        create_listbox(&file_list);
        create_listbox(&dir_list);
      } else
        _dos_setdrive(file_types[v_type].path[0] - 'A' + 1, &n);
    }
  } else
    div_strcpy(input, sizeof(input), file_mask);

  call((void_return_type_t)v.paint_handler);
  v.redraw = 1;
}

//-----------------------------------------------------------------------------
// Progress indicator for an operation
//-----------------------------------------------------------------------------

int progress_init = 0;
int progress_total;
int progress_current;
char *progress_title;

void progress1() {
  int w = v.w, h = v.h;
  byte *ptr = v.ptr;
  char cwork[5];
  if (big) {
    w /= 2;
    h /= 2;
  }
  wbox(ptr, w, h, c2, 2, 10, w - 4, h - 12);

  wbox(ptr, w, h, c1, 4, 12, w - 8, h - 16);

  wbox(ptr, w, h, c_r_low, 4, 12, progress_current * (w - 8) / progress_total, h - 16);
  wrectangle(ptr, w, h, c0, 4, 12, w - 8, h - 16);
  div_snprintf((char *)cwork, sizeof(cwork), "%d%c", progress_current * 100 / progress_total, '%');
  wwrite(ptr, w, h, 4 + (w - 8) / 2, (14 + (h - 16) / 2) - 2, 4, (byte *)cwork, c3);
}

void progress2() {
  int w = v.w, h = v.h;
  byte *ptr = v.ptr;
  char cwork[5];
  if (big) {
    w /= 2;
    h /= 2;
  }
  wbox(ptr, w, h, c2, 2, 10, w - 4, h - 12);

  wbox(ptr, w, h, c1, 4, 12, w - 8, h - 16);

  wbox(ptr, w, h, c_r_low, 4, 12, progress_current * (w - 8) / progress_total, h - 16);
  wrectangle(ptr, w, h, c0, 4, 12, w - 8, h - 16);
  div_snprintf(cwork, sizeof(cwork), "%d%c", progress_current * 100 / progress_total, '%');
  wwrite(ptr, w, h, 4 + (w - 8) / 2, (14 + (h - 16) / 2) - 2, 4, (byte *)cwork, c3);
}
void progress0() {
  v.type = WIN_PROGRESS;
  v.w = 100;
  v.h = 28;
  v.paint_handler = progress1;
  v.click_handler = progress2;
  v.title = (byte *)progress_title;
  v.name = (byte *)progress_title;
}

int show_progress(char *title, int current, int total) {
  if (!progress_init) { // Initialize the progress window
    mouse_graf = 3;
    flush_copy();
    progress_init = 1;
    progress_current = current;
    progress_title = title;
    progress_total = total;
    new_window(progress0);
    flush_window(0);
    flush_copy();
  } else {
    progress_current = current;
    if (progress_current >= progress_total) { // Finalize the progress window
      mouse_graf = 1;
      progress_init = 0;
      close_window();
      flush_copy();
      return 0;
    } else { // Progress window update loop
      v = window[0];
      progress2();
      flush_window(0);
      flush_copy();
    }
  }
  return (1);
}

//-----------------------------------------------------------------------------
//      About Dialog Box
//-----------------------------------------------------------------------------

void about1(void) {
  int x;
  int w = v.w / big2, h = v.h / big2;
  _show_items();

  x = w / 2;

  wwrite(v.ptr, w, h, x, 11 + 8 * 0, 1, texts[464], c3);
  wwrite(v.ptr, w, h, x, 11 + 8 * 1, 1, texts[465], c3);
  wwrite(v.ptr, w, h, x, 11 + 8 * 2, 1, texts[466], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 3, 1, texts[467], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 4, 1, texts[468], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 5, 1, texts[469], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 6, 1, texts[470], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 7, 1, texts[471], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 8, 1, texts[472], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 9, 1, texts[473], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 10, 1, texts[474], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 11, 1, texts[475], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 12, 1, texts[476], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 13, 1, texts[477], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 14, 1, texts[478], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 15, 1, texts[479], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 16, 1, texts[480], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 17, 1, texts[481], c0);
  wwrite(v.ptr, w, h, x, 11 + 8 * 18, 1, texts[482], c3);
  wwrite(v.ptr, w, h, x, 11 + 8 * 19, 1, texts[483], c4);
}

void about2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void about0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[885];
  v.w = 232;
  v.h = 188;
  v.paint_handler = about1;
  v.click_handler = about2;
  _button(100, v.w / 2, v.h - 14, 1);
}
