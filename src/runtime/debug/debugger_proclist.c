// Debugger Process List — process list dialog
#include "debugger_internal.h"

extern byte strlower[256];

void create_process_list(void) {
  byte *p, *q;
  char cwork[512];
  int line = 0, n;

  p = source + 1;
  lp_num = 0;
  lp_ini = 0;
  lp_select = 0;

  do {
    while (*p && p < end_source) {
      p++;
    }
    p++;
    line++;
    if (p < end_source) {
      while (*p == ' ' && p < end_source)
        p++;
      if (p + 9 < end_source) {
        memcpy(cwork, p, 9);
        cwork[9] = 0;
        if (cwork[8] == ' ')
          cwork[8] = 0;
        if (cwork[7] == ' ')
          cwork[7] = 0;
        n = strlen(cwork);
        do {
          cwork[n] = tolower(cwork[n]);
        } while (n--);
        if (!strcmp(cwork, "process") || !strcmp(cwork, "function")) {
          p += strlen(cwork);
          while (*p == ' ' && p < end_source)
            p++;
          if (p < end_source) {
            if (lp_sort) {
              q = p;
              n = 0;
              while (*q && q < end_source) {
                cwork[n++] = *q;
                q++;
              }
              cwork[n] = 0;
              n = 0;
              while (n < lp_num) {
                if (strcmp(cwork, lp2[n]) < 0)
                  break;
                n++;
              }
              if (n < lp_num) {
                memmove(&lp1[n + 1], &lp1[n], 4 * (511 - n));
                memmove(&lp2[n + 1], &lp2[n], 4 * (511 - n));
                lp1[n] = line;
                lp2[n] = (char *)p;
              } else {
                lp1[lp_num] = line;
                lp2[lp_num] = (char *)p;
              }
            } else {
              lp1[lp_num] = line;
              lp2[lp_num] = (char *)p;
            }
            if (++lp_num == 512)
              break;
          } else
            break;
        }
      } else
        break;
    } else
      break;
  } while (1);
}

void paint_process_list(void) {
  byte *ptr = v.ptr, *p;
  char cwork[512];
  int w = v.w / big2, h = v.h / big2;
  int n, m, x;

  wbox(ptr, w, h, c1, 4, 20, 128 + 32 - 10, 121); // Process listbox bounds

  for (m = lp_ini; m < lp_ini + 15 && m < lp_num; m++) {
    if (m == lp_select) {
      wbox(ptr, w, h, c01, 4, 20 + (m - lp_ini) * 8, 150, 9); // Process listbox fill
      x = c4;
    } else
      x = c3;
    p = (byte *)lp2[m];
    n = 0;
    while (*p && p < end_source) {
      cwork[n++] = *p;
      p++;
    }
    cwork[n] = 0;
    wwrite_in_box(ptr, w, 153, h, 5, 21 + (m - lp_ini) * 8, 0, (byte *)cwork, x);
  }
  paint_process_segment();
}

void paint_process_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 129, n;
  float x;

  wbox(ptr, w, h, c2, 123 + 32, 28, 7, max - min + 3); // Clear the slider bar

  if (lp_num <= 1)
    n = min;
  else {
    x = (float)lp_select / (float)(lp_num - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122 + 32, n, 9, 5);
  wput(ptr, w, h, 123 + 32, n + 1, -43);
}

void process_list1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  _show_items();

  wwrite(ptr, w, h, 5, 11, 0, text[74], c1);
  wwrite(ptr, w, h, 4, 11, 0, text[74], c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128 + 32, 123); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122 + 32, 19, 9, 123);
  wrectangle(ptr, w, h, c0, 122 + 32, 27, 9, 123 - 16);

  wput(ptr, w, h, 123 + 32, 20, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, 123 + 32, 174 - 40, -40);

  create_process_list();
  paint_process_list();
}

int lp_button;

void process_list2(void) {
  int n;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  float x;

  _process_items();

  if (scan_code == 80 && lp_select + 1 < lp_num) {
    if (lp_ini + 15 == ++lp_select)
      lp_ini++;
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 72 && lp_select) {
    if (lp_ini == lp_select--)
      lp_ini--;
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 81) {
    for (n = 0; n < 15; n++)
      if (lp_select + 1 < lp_num) {
        if (lp_ini + 15 == ++lp_select)
          lp_ini++;
      }
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 73) {
    for (n = 0; n < 15; n++)
      if (lp_select) {
        if (lp_ini == lp_select--)
          lp_ini--;
      }
    paint_process_list();
    flush_buffer();
    v.redraw = 1;
  }

  if (wmouse_in(3, 21, 128 + 32 - 9, 120) && (mouse_b & MB_LEFT)) {
    n = lp_ini + (wmouse_y - 21) / 8;
    if (n < lp_num) {
      if (lp_select != n) {
        lp_select = n;
        paint_process_list();
        v.redraw = 1;
      } else if (!(prev_mouse_buttons & MB_LEFT)) {
        v_accept = 1;
        end_dialog = 1;
      }
    }
  }

  if (wmouse_in(122 + 32, 19, 9, 9)) {
    if (mouse_b & MB_LEFT) {
      if (lp_button == 0) {
        wput(ptr, w, h, 123 + 32, 20, -41);
        lp_button = 1;
        if (lp_select) {
          if (lp_ini == lp_select--)
            lp_ini--;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_button == 1) {
      wput(ptr, w, h, 123 + 32, 20, -39);
      lp_button = 0;
      v.redraw = 1;
    }
    mouse_graf = CURSOR_SCROLL_UP;
  } else if (lp_button == 1) {
    wput(ptr, w, h, 123 + 32, 20, -39);
    lp_button = 0;
    v.redraw = 1;
  }


  if (wmouse_in(123 + 32, 28, 7, 105)) {
    mouse_graf = CURSOR_SCROLL_TRACK;
    if (lp_num > 1 && (mouse_b & MB_LEFT)) {
      x = (float)(wmouse_y - 28) / 104.0;
      lp_select = x * (lp_num - 1);
      if (lp_select < lp_ini)
        lp_ini = lp_select;
      if (lp_select >= lp_ini + 15)
        lp_ini = lp_select - 14;
      paint_process_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122 + 32, 93 + 40, 9, 9)) {
    if (mouse_b & MB_LEFT) {
      if (lp_button == 0) {
        wput(ptr, w, h, 123 + 32, 94 + 40, -42);
        lp_button = 2;
        if (lp_select + 1 < lp_num) {
          if (lp_ini + 15 == ++lp_select)
            lp_ini++;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_button == 2) {
      wput(ptr, w, h, 123 + 32, 94 + 40, -40);
      lp_button = 0;
      v.redraw = 1;
    }
    mouse_graf = CURSOR_SCROLL_DOWN;
  } else if (lp_button == 2) {
    wput(ptr, w, h, 123 + 32, 94 + 40, -40);
    lp_button = 0;
    v.redraw = 1;
  }

  switch (v.active_item) {
  case 0:
    if (lp_num)
      v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    create_process_list();
    paint_process_list();
    v.redraw = 1;
    break;
  }
}

void process_list0(void) {
  v.type = WIN_DIALOG;
  v.title = text[75];

  v.w = 166;
  v.h = 161;
  v.paint_handler = process_list1;
  v.click_handler = process_list2;

  _button(text[7], 7, v.h - 14, ALIGN_TL);
  _button(text[58], v.w - 8, v.h - 14, ALIGN_TR);
  _flag(text[76], v.w - text_len(text[76]) - 12, 11, &lp_sort);
  v_accept = 0;
}
