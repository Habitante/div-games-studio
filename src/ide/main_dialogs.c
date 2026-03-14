/**
 * main_dialogs.c — Dialog system, modal loop, and UI item controls.
 *
 * Split from main.c. Contains:
 * - Dialog/modal loops (dialog_loop, modal_loop)
 * - Dialog creation (show_dialog, refresh_dialog)
 * - Item builders (_button, _get, _flag)
 * - Item rendering (show_button, show_get, show_flag, select_button, select_get)
 * - Item event processing (_show_items, _process_items, _select_new_item, _reselect_item)
 * - Item status/handlers (button_status, process_button, get_status, process_get, flag_status, process_flag)
 * - Text input (get_input)
 */

#include "main_internal.h"
#include "sound.h"
#include "mixer.h"

extern int superget;

void dialog_loop(void) {
  dialogo_invocado = 0;

  poll_keyboard();

  //-------------------------------------------------------------------------
  // Find the window the mouse is over (n); n=MAX_WINDOWS if none
  //-------------------------------------------------------------------------

  if (mouse_in(v.x, v.y, v.x + v.w - 1, v.y + v.h - 1))
    n = 0;
  else
    n = MAX_WINDOWS;

  //-------------------------------------------------------------------------
  // If we were previously in a window we've now left,
  // repaint it (to clear any highlights)
  //-------------------------------------------------------------------------

  if (n == 0) // If we're now on the toolbar, also repaint the window
    if (!mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2))
      n--;

  if (n != oldn && oldn == 0)
    if (v.foreground == WF_FOREGROUND) {
      dialogo_invocado = 1;
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      mouse_b = 0;

      call((void_return_type_t)v.click_handler);
      mouse_b = m;

      partial_blits = 1;

      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }

      partial_blits = 0;
      salir_del_dialogo = 0;
    }

  oldn = MAX_WINDOWS;

  if (n < 0)
    n++;

  //-------------------------------------------------------------------------
  // Determine cursor shape
  //-------------------------------------------------------------------------

  if (n == MAX_WINDOWS)
    mouse_graf = CURSOR_ARROW;
  else if (mouse_in(v.x + 2 * big2, v.y + 2 * big2, v.x + v.w - 2 * big2, v.y + 9 * big2))
    if (mouse_x <= v.x + v.w - 10 * big2)
      mouse_graf = CURSOR_MOVE;
    else
      mouse_graf = CURSOR_CLOSE;
  else
    mouse_graf = CURSOR_ARROW;

  //-------------------------------------------------------------------------
  // If we are inside a window's content area ...
  //-------------------------------------------------------------------------

  if (n == 0)
    if (mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2)) {
      dialogo_invocado = 1;
      wmouse_x = mouse_x - v.x;
      wmouse_y = mouse_y - v.y;

      if (big) {
        wmouse_x /= 2;
        wmouse_y /= 2;
      }

      call((void_return_type_t)v.click_handler);
      partial_blits = 1;

      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }

      partial_blits = 0;
      oldn = 0;
      salir_del_dialogo = 0;

    } else { // If we are on the window's control toolbar ...

      if (mouse_graf == CURSOR_MOVE && (mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT))
        move_window();

      if (mouse_graf == CURSOR_CLOSE) {
        if (mouse_b & MB_LEFT) {
          if (big)
            wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
          else
            wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);

          partial_blits = 1;
          flush_window(0);
          partial_blits = 0;
        }
        if (!(mouse_b & MB_LEFT) && (prev_mouse_buttons & MB_LEFT)) {
          close_window();
          salir_del_dialogo = 1;
        } else if (mouse_b & MB_LEFT)
          restore_button = 3;
      }

      oldn = -1;
    }
  else if (draw_mode < 100 && (mouse_b & MB_LEFT)) {
    close_window();
    salir_del_dialogo = 1;
  }

  //-------------------------------------------------------------------------
  //  Dialogs must always be invoked
  //-------------------------------------------------------------------------

  if (!dialogo_invocado && !salir_del_dialogo) {
    dialogo_invocado = 1;
    wmouse_x = -1;
    wmouse_y = -1;
    m = mouse_b;
    mouse_b = 0;
    call((void_return_type_t)v.click_handler);
    mouse_b = m;
    partial_blits = 1;
    if (v.redraw) {
      flush_window(0);
      v.redraw = 0;
    }

    partial_blits = 0;
    salir_del_dialogo = 0;
  }

  if (end_dialog && !salir_del_dialogo) {
    close_window();
    salir_del_dialogo = 1;
  }

  //-------------------------------------------------------------------------
  // Keyboard Control
  //-------------------------------------------------------------------------

  if ((key(_ESC) && !key(_L_CTRL)) || (draw_mode < 100 && (mouse_b & MB_RIGHT))) {
    for (n = 0; n < v.items; n++)
      if (v.item[n].type == ITEM_TEXT && (v.item[n].state & 2))
        break;
    if (n == v.items) {
      close_window();
      salir_del_dialogo = 1;
    }
  }

  //-------------------------------------------------------------------------
  // End central loop
  //-------------------------------------------------------------------------

  // Alt+S DOS shell disabled (no DOS)


  flush_copy();

  if (restore_button == 3) {
    if (big)
      wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -35);
    else
      wput(v.ptr, v.w, v.h, v.w - 9, 2, -35);
    flush_window(0);
  }

  restore_button = 0;
}

void modal_loop(void) {
  salir_del_dialogo = 0;

  end_dialog = 0;

  do {
    dialog_loop();
  } while (!salir_del_dialogo && !exit_requested);
  end_dialog = 0;

  get[0] = 0;

  wmouse_x = -1;
  wmouse_y = -1;

  do {
    read_mouse();
  } while ((mouse_b) || key(_ESC));
}

void err0(void);

void show_dialog(void_return_type_t init_handler) {
  int vtipo, _get_pos;
  byte *ptr;
  int n, m, x, y, w, h;
  uint32_t colorkey = 0;

  if (!window[MAX_WINDOWS - 1].type) {
    _get_pos = get_pos;

    if (v.type) {
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      mouse_b = 0;
      call((void_return_type_t)v.click_handler);
      mouse_b = m;
      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }
    }

    add_window();

    //---------------------------------------------------------------------------
    // The following values should define the init_handler, default values
    //---------------------------------------------------------------------------

    v.order = next_order++;
    v.type = WIN_DIALOG;
    v.foreground = WF_FOREGROUND;
    v.name = (byte *)"?";
    v.title = (byte *)"?";
    v.paint_handler = dummy_handler;
    v.click_handler = dummy_handler;
    v.close_handler = dummy_handler;
    v.x = 0;
    v.y = 0;
    v.w = vga_width;
    v.h = vga_height;
    v._w_saved = 0;
    v._h_saved = 0;
    v.state = 0;
    v.buttons = 0;
    v.redraw = 0;
    v.items = 0;
    v.selected_item = -1;
    v.prg = NULL;
    v.aux = NULL;

    call((void_return_type_t)init_handler);

    if (big) {
      if (v.w > 0) {
        v.w = v.w * 2;
        v.h = v.h * 2;
      } else
        v.w = -v.w;
    }

    w = v.w;
    h = v.h;
    x = vga_width / 2 - w / 2;
    y = vga_height / 2 - h / 2;
    v.x = x;
    v.y = y;

    if (v.click_handler == err2)
      ptr = error_window;
    else
      ptr = (byte *)malloc(w * h);

    if (ptr != NULL) { // Window buffer, freed in close_window
      memset(ptr, 0, w * h);

      window_surface(w, h, 1);

      //---------------------------------------------------------------------------
      // Send appropriate windows to the background
      //---------------------------------------------------------------------------

      vtipo = v.type;
      v.type = WIN_EMPTY; // Megabug workaround

      if (draw_mode >= 100) {
        if (window[1].type == WIN_DIALOG || window[1].type == WIN_PROGRESS) { // Dialog over dialog
          window[1].foreground = WF_BACKGROUND;
          flush_window(1);
        } else {
          for (n = 1; n < MAX_WINDOWS; n++) {
            if (window[n].type && window[n].foreground == WF_FOREGROUND) {
              hidden[n - 1] = 1;
              window[n].foreground = WF_BACKGROUND;
              flush_window(n);
            } else
              hidden[n - 1] = 0;
          }
        }
      }

      v.type = vtipo;

      //---------------------------------------------------------------------------
      // Initializes the window
      //---------------------------------------------------------------------------

      v.ptr = ptr;

      memset(ptr, c0, w * h);
      if (big) {
        w /= 2;
        h /= 2;
      }

      wrectangle(ptr, w, h, c2, 0, 0, w, h);

      wput(ptr, w, h, w - 9, 2, 35);
      if (!strcmp((char *)v.title, (char *)texts[41]) ||
          !strcmp((char *)v.title, (char *)texts[367]))
        wgra(ptr, w, h, c_r_low, 2, 2, w - 12, 7);
      else
        wgra(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
      if (text_len(v.title) + 3 > w - 12) {
        wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
        wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
      } else {
        wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
        wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
      }

      call((void_return_type_t)v.paint_handler);

      if (big) {
        w *= 2;
        h *= 2;
      }

      do {
        read_mouse();
      } while ((mouse_b & MB_LEFT) || key(_ESC));

      if (exploding_windows) {
        v.exploding = 1;
        explode(x, y, w, h);
        v.exploding = 0;
      }
      blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
      blit_partial(x, y, w, h);
      do {
        read_mouse();
      } while (mouse_b & MB_LEFT);
      modal_loop();

      get_pos = _get_pos;

      //---------------------------------------------------------------------------
      // Could not open the dialog (no memory)
      //---------------------------------------------------------------------------

    } else {
      div_delete(0);
    }
  }
}

void refresh_dialog(void) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  memset(ptr, c0, w * h);
  if (big) {
    w /= 2;
    h /= 2;
  }
  wrectangle(ptr, w, h, c2, 0, 0, w, h);
  wput(ptr, w, h, w - 9, 2, 35);
  if (!strcmp((char *)v.title, (char *)texts[41]))
    wgra(ptr, w, h, c_r_low, 2, 2, w - 12, 7);
  else
    wgra(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
  if (text_len(v.title) + 3 > w - 12) {
    wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
    wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
  }

  call((void_return_type_t)v.paint_handler);
}

void _button(int t, int x, int y, int c) {
  v.item[v.items].type = ITEM_BUTTON;
  v.item[v.items].state = 0;
  v.item[v.items].button.text = texts[t];
  v.item[v.items].button.x = x;
  v.item[v.items].button.y = y;
  v.item[v.items].button.center = c;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _get(int t, int x, int y, int w, byte *buffer, int buffer_len, int r0, int r1) {
  v.item[v.items].type = ITEM_TEXT;
  v.item[v.items].state = 0;
  v.item[v.items].get.text = texts[t];
  v.item[v.items].get.x = x;
  v.item[v.items].get.y = y;
  v.item[v.items].get.w = w;
  v.item[v.items].get.buffer = buffer;
  v.item[v.items].get.buffer_len = buffer_len;
  v.item[v.items].get.r0 = r0;
  v.item[v.items].get.r1 = r1;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _flag(int t, int x, int y, int *value) {
  v.item[v.items].type = ITEM_CHECKBOX;
  v.item[v.items].state = 0;
  v.item[v.items].flag.text = texts[t];
  v.item[v.items].flag.x = x;
  v.item[v.items].flag.y = y;
  v.item[v.items].flag.value = value;
  v.items++;
}

void _show_items(void) {
  int n = 0;
  show_items_called = 1;
  wbox(v.ptr, v.w / big2, v.h / big2, c2, 2, 10, v.w / big2 - 4, v.h / big2 - 12);
  while (n < v.items) {
    switch (abs(v.item[n].type)) {
    case 1:
      show_button(&v.item[n]);
      break;
    case 2:
      show_get(&v.item[n]);
      break;
    case 3:
      show_flag(&v.item[n]);
      break;
    }
    n++;
  }
  key(_ENTER) = 0;
  show_items_called = 0;
}

void show_button(struct t_item *i) {
  wwrite(v.ptr, v.w / big2, v.h / big2, i->button.x, i->button.y, i->button.center, i->button.text,
         c3);
  if (&v.item[v.selected_item] == i)
    select_button(i, 1);
}

void select_button(struct t_item *i, int active) {
  int x = i->button.x, y = i->button.y;
  int w, h;
  w = text_len(i->button.text + 1);
  h = 7;
  switch (i->button.center) {
  case ALIGN_TL:
    break;
  case ALIGN_TC:
    x = x - (w >> 1);
    break;
  case ALIGN_TR:
    x = x - w + 1;
    break;
  case ALIGN_ML:
    y = y - (h >> 1);
    break;
  case ALIGN_MC:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case ALIGN_MR:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case ALIGN_BL:
    y = y - h + 1;
    break;
  case ALIGN_BC:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case ALIGN_BR:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }
  if (active) {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c12, x - 4, y - 4, w + 8, h + 8);
  } else {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c2, x - 4, y - 4, w + 8, h + 8);
  }
}

void show_get(struct t_item *i) {
  wbox(v.ptr, v.w / big2, v.h / big2, c1, i->get.x, i->get.y + 8, i->get.w, 9);
  wwrite_in_box(v.ptr, v.w / big2, i->get.w - 1 + i->get.x, v.h / big2, i->get.x + 1, i->get.y + 9,
                0, i->get.buffer, c3);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->get.x + 1, i->get.y, 0, i->get.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->get.x, i->get.y, 0, i->get.text, c3);
  if (&v.item[v.selected_item] == i) {
    if (i->state & 2)
      select_get(i, 0, 0);
    select_get(i, 1, 0);
  }
}

void select_get(struct t_item *i, int active, int hide_error) {
  char cWork[128];
  int n;
  if (active) {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c12, i->get.x - 1, i->get.y + 7, i->get.w + 2, 11);
    if (i->state & 2) {
      div_strcpy((char *)get, LONG_LINE, (char *)i->get.buffer);
      get_pos = strlen(get);
    }
    i->state &= 1;
  } else {
    if (i->state & 2) {
      if (*get) {
        if (i->get.r0 == i->get.r1)
          div_strcpy((char *)i->get.buffer, i->get.buffer_len + 1, get);
        else {
          if (atoi(get) >= i->get.r0 && atoi(get) <= i->get.r1)
            itoa(atoi(get), (char *)i->get.buffer, 10);
          else if (!hide_error && !show_items_called) {
            div_snprintf(cWork, sizeof(cWork), "%s [%d..%d].", texts[49], i->get.r0, i->get.r1);
            v_text = cWork;
            show_dialog(err0);
          }
        }
      }
    }

    i->state &= 1;

    if (!superget) {
      wbox(v.ptr, v.w / big2, v.h / big2, c1, i->get.x, i->get.y + 8, i->get.w, 9);
      wwrite_in_box(v.ptr, v.w / big2, i->get.w - 1 + i->get.x, v.h / big2, i->get.x + 1,
                    i->get.y + 9, 0, i->get.buffer, c3);
      wrectangle(v.ptr, v.w / big2, v.h / big2, c2, i->get.x - 1, i->get.y + 7, i->get.w + 2, 11);
    }

    for (n = 0; n < MAX_ITEMS; n++)
      if (i == &v.item[n])
        v.active_item = n;
  }
}

void show_flag(struct t_item *i) {
  if (*i->flag.value)
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, -59);
  else
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, 58);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 9, i->flag.y, 0, i->flag.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 8, i->flag.y, 0, i->flag.text, c3);
}

void _process_items(void) {
  int n = 0, state;
  int asc = 0, kesc = 0, est;

  v.active_item = -1;

  if (v.selected_item != -1) {
    if (!v.state && v.type == WIN_CODE) {
      asc = ascii;
      kesc = key(_ENTER);
      ascii = 0;
      key(_ENTER) = 0;
    } else {
      if (ascii == ASCII_TAB) {
        ascii = 0;
        _select_new_item(v.selected_item + 1);
      }
      if (ascii == ASCII_ESC) { // && (v.item[v.selected_item].state&2)) {
        if (v.item[v.selected_item].type == ITEM_TEXT) {
          asc = ascii;
          kesc = key(_ENTER);
          est = v.item[v.selected_item].state;
          ascii = 0;
          if (superget)
            div_strcpy((char *)v.item[v.selected_item].get.buffer,
                       v.item[v.selected_item].get.buffer_len + 1, "");
          div_strcpy((char *)get, LONG_LINE, (char *)v.item[v.selected_item].get.buffer);
          get_pos = strlen(get);
          select_get(&v.item[v.selected_item], 0, 1);
          select_get(&v.item[v.selected_item], 1, 1);
          if (est == v.item[v.selected_item].state) {
            ascii = asc;
            key(_ENTER) = kesc;
          } else {
            v.redraw = 1;
            key(_ESC) = 0;
          }
        }
      }
    }
  }

  while (n < v.items) {
    switch (v.item[n].type) {
    case 1:
      state = button_status(n);
      if (state != v.item[n].state)
        process_button(n, state);
      break;
    case 2:
      state = get_status(n);
      if (state != v.item[n].state || state >= 2)
        process_get(n, state);
      break;
    case 3:
      state = flag_status(n);
      if (state != v.item[n].state)
        process_flag(n, state);
      break;
    }
    n++;
  }

  if (v.selected_item != -1) {
    if (!v.state && v.type == WIN_CODE) {
      ascii = asc;
      key(_ENTER) = kesc;
    }
  }
}

void _select_new_item(int i) {
  if (v.selected_item == i)
    return;
  switch (abs(v.item[v.selected_item].type)) {
  case 1:
    select_button(&v.item[v.selected_item], 0);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 0, 0);
    break;
  }
  i--;
  do {
    i++;
    if (i >= v.items)
      i = 0;
  } while (v.item[i].type >= ITEM_CHECKBOX || v.item[i].type < ITEM_BUTTON);

  switch (v.item[v.selected_item = i].type) {
  case 1:
    select_button(&v.item[v.selected_item], 1);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 1, 0);
    break;
  }
  v.redraw = 1;
}

void _reselect_item(void) {
  switch (v.item[v.selected_item].type) {
  case 1:
    select_button(&v.item[v.selected_item], 0);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 0, 0);
    break;
  }
  switch (v.item[v.selected_item].type) {
  case 1:
    select_button(&v.item[v.selected_item], 1);
    break;
  case 2:
    select_get(&v.item[v.selected_item], 1, 0);
    break;
  }
  v.redraw = 1;
}

int button_status(int n) {
  int x = v.item[n].button.x, y = v.item[n].button.y;
  int w, h, e = 0;
  w = text_len(v.item[n].button.text + 1);
  h = 7;
  switch (v.item[n].button.center) {
  case ALIGN_TL:
    break;
  case ALIGN_TC:
    x = x - (w >> 1);
    break;
  case ALIGN_TR:
    x = x - w + 1;
    break;
  case ALIGN_ML:
    y = y - (h >> 1);
    break;
  case ALIGN_MC:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case ALIGN_MR:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case ALIGN_BL:
    y = y - h + 1;
    break;
  case ALIGN_BC:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case ALIGN_BR:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }
  if (wmouse_in(x - 3, y - 3, w + 6, h + 6))
    e = 1;
  if (e && (mouse_b & MB_LEFT))
    e = 2;
  if (v.selected_item == n && key(_ENTER))
    e = 3;
  return (e);
}

void process_button(int n, int e) {
  if (v.item[n].state == 3 && e != 3) {
    v.active_item = n;
    key(_ENTER) = 0;
    ascii = 0;
  }
  switch (e) {
  case 0:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c3);
    break;
  case 1:
    if (v.item[n].state == 2)
      v.active_item = n;
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c4);
    break;
    break;
  case 2:
    _select_new_item(n);
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c0);
    break;
  case 3:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].button.x, v.item[n].button.y,
           v.item[n].button.center, v.item[n].button.text, c0);
    break;
  }
  v.item[n].state = e;
  v.redraw = 1;
}

int get_status(int n) {
  int x = v.item[n].state;
  if (strcmp((char *)v.item[n].get.text, "")) {
    if (wmouse_in(v.item[n].get.x, v.item[n].get.y, // bit 0 "hilite"
                  v.item[n].get.w, 18))
      x |= 1;
    else
      x &= 2;
  } else {
    if (wmouse_in(v.item[n].get.x, v.item[n].get.y + 8, // bit 0 "hilite"
                  v.item[n].get.w, 10))
      x |= 1;
    else
      x &= 2;
  }
  if ((x & 1) && (mouse_b & MB_LEFT)) {
    if (!(prev_mouse_buttons & MB_LEFT) && (x & 2))
      x |= 4;
    x |= 2;
  }
  if ((ascii && (ascii != ASCII_ESC) && v.selected_item == n)) { //||superget) {
    // MapperCreator2 digit filter removed (MODE8/3D map editor deleted)
    {
      if (!(x & 2)) {
        if (ascii == ASCII_ENTER)
          ascii = 0;
        else
          x |= 4;
      }
      x |= 2;
    }
  }
  if (superget)
    x = 3; // x&=3;
  return (x);
}

void process_get(int n, int e) {
  int old_e;

  old_e = v.item[n].state;
  v.item[n].state = e;
  if (!(old_e & 2) && (e & 2)) {
    _select_new_item(n);
    div_strcpy((char *)get, LONG_LINE, (char *)v.item[n].get.buffer);
    get_pos = strlen(get);
  }

  if (e & 4)
    *get = 0;
  e &= 3;

  switch (v.item[n].state = e) {
  case 2:
    get_input(n);
    break;
  case 3:
    get_input(n);
    break;
  }

  switch (v.item[n].state) {
  case 2:
  case 0:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].get.x, v.item[n].get.y, 0, v.item[n].get.text,
           c3);
    break;
  case 3:
  case 1:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].get.x, v.item[n].get.y, 0, v.item[n].get.text,
           c4);
    break;
  }

  if (old_e != v.item[n].state)
    v.redraw = 1;
}

int flag_status(int n) {
  int x = 0;
  if (wmouse_in(v.item[n].flag.x, v.item[n].flag.y, text_len(v.item[n].flag.text) + 10, 8))
    x = 1;
  if (x && (mouse_b & MB_LEFT))
    x = 2;
  return (x);
}

void process_flag(int n, int e) {
  if (v.item[n].state == 3 && e != 3)
    v.active_item = n;
  switch (e) {
  case 0:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x + 8, v.item[n].flag.y, 0,
           v.item[n].flag.text, c3);
    break;
  case 1:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x + 8, v.item[n].flag.y, 0,
           v.item[n].flag.text, c4);
    break;
  case 2:
    wwrite(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x + 8, v.item[n].flag.y, 0,
           v.item[n].flag.text, c4);
    if (v.item[n].state == 1) {
      v.active_item = n;
      if ((*v.item[n].flag.value = !*v.item[n].flag.value))
        wput(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x, v.item[n].flag.y, -59);
      else
        wput(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x, v.item[n].flag.y, 58);
    }
    break;
  }
  v.item[n].state = e;
  v.redraw = 1;
}

int text_len2(byte *ptr);

void get_input(int n) {
  char cwork[LONG_LINE];
  int x, l, scroll;

  if (!*get)
    get_pos = 0;

  switch (ascii) {
  case 1:
  case 0x1a:
  case ASCII_ESC:
    break;
  case ASCII_BACKSPACE:
    if (get_pos) {
      memmove(&get[get_pos - 1], &get[get_pos], strlen(&get[get_pos]) + 1);
      get_pos--;
    }
    if (!*get && superget)
      div_strcpy((char *)v.item[v.selected_item].get.buffer,
                 v.item[v.selected_item].get.buffer_len + 1, "");
    v.redraw = 1;
    break;
  case ASCII_ENTER:
    ascii = 0;
    key(_ENTER) = 0;
    _select_new_item(n + 1);
    return;
  default:
    if (!(shift_status & (MOD_SHIFT | MOD_CTRL | MOD_ALT)) && ascii == 0) {
      l = v.redraw;
      v.redraw = 1;
      switch (scan_code) {
      case _RIGHT:
        get_pos++;
        break; // cursor right
      case _LEFT:
        get_pos--;
        break; // cursor left
      case _HOME:
        get_pos = 0;
        break; // home
      case _END:
        get_pos = strlen(get);
        break; // end
      case _DEL:
        get[strlen(get) + 1] = 0;
        memmove(&get[get_pos], &get[get_pos + 1], strlen(&get[get_pos + 1]) + 1);
        if (!*get && superget)
          div_strcpy((char *)v.item[v.selected_item].get.buffer,
                     v.item[v.selected_item].get.buffer_len + 1, "");
        break;
      default:
        v.redraw = l;
        break;
      }
    } else if (ascii && char_len(ascii) > 1 && (x = strlen(get)) < v.item[n].get.buffer_len - 1) {
      // MapperCreator2 digit filter removed (MODE8/3D map editor deleted)
      {
        div_strcpy(cwork, sizeof(cwork), get);
        cwork[get_pos] = ascii;
        cwork[get_pos + 1] = 0;
        div_strcat(cwork, sizeof(cwork), get + get_pos);
        div_strcpy(get, LONG_LINE, cwork);
        get_pos++;
        v.redraw = 1;
      }
    }
    break;
  }

  if (v.redraw || get_cursor != (*system_clock & 4)) {
    v.redraw = 1;

    if (get_pos < 0)
      get_pos = 0;
    else if (get_pos > strlen(get))
      get_pos = strlen(get);

    div_strcpy(cwork, sizeof(cwork), get);
    cwork[get_pos] = 0;
    l = text_len2((byte *)cwork);
    div_strcat(cwork, sizeof(cwork), " ");
    div_strcat(cwork, sizeof(cwork), get + get_pos);

    if (l > v.item[n].get.w - 8) {
      scroll = l - (v.item[n].get.w - 8);
    } else
      scroll = 0;

    wbox(v.ptr, v.w / big2, v.h / big2, c0, v.item[n].get.x, v.item[n].get.y + 8, v.item[n].get.w,
         9);
    wwrite_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2, v.h / big2,
                  0 - scroll, v.item[n].get.y + 9, 0, (byte *)cwork, c4);

    if (*system_clock & 4) {
      x = l + 1;
      wbox_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2, v.h / big2,
                  c3, x - scroll, v.item[n].get.y + 9, 2, 7);
    }
  }
  get_cursor = (*system_clock & 4);
}
