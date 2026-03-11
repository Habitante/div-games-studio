
//-----------------------------------------------------------------------------
//      Hypertext system module
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"

extern int window_closing;
extern int primera_vez;

int helpidx[4096];        // Per topic: {offset, length}
int help_item;            // Which topic help is requested for
int help_len;             // Length of help_buffer
int help_w, help_h;     // Width and height of the help window
int help_l, help_lines;   // Current line, and total lines
byte help_title[128];     // Topic title
byte *help_buffer = NULL; // Buffer holding the help content
byte *h_buffer;           // Auxiliary buffer
byte *help_line;          // Pointer to the current line
byte *help_end;           // End of help_buffer
byte *div_index = NULL;   // Start of the glossary
byte *index_end;          // End of the glossary

int loaded[64], n_loaded = 0; // Loaded images, up to a maximum of 32

int determine_prg2(void);
struct tprg *old_prg;
extern int help_paint_active;

void read_line(void);
void vuelca_help(void);
void help_xref(int n, int linea);
void Print_Help(void);
void resize_help(void);
int dtoi(int m);
void arregla_linea(byte *end, int chars, int help_w);
void put_chr(byte *ptr, int w, byte c, byte color);
void put_image_line(int n, int linea, byte *di, int v_w);


//-----------------------------------------------------------------------------

// "Back to previous topic" system

int backto[64];     // Circular queue storing visited topics {n, line}
int i_back, f_back; // Start and end of the circular queue (0,2,...,62)
int a_back;         // Current topic (i_back <= a_back <= f_back)

//-----------------------------------------------------------------------------
//      Load the glossary of terms (div_index.prg)
//-----------------------------------------------------------------------------

void load_index(void) {
  long len, n;
  byte *si, *di;
  FILE *f;

  if (div_index != NULL) {
    div_index = NULL;
  }

  if ((f = fopen("help/help.idx", "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if ((div_index = (byte *)malloc(len)) != NULL) {
      fseek(f, 0, SEEK_SET);
      fread(div_index, 1, len, f);
      fclose(f);
      index_end = div_index + len;
      si = di = div_index;
      do {
        if (lower[*si] && lower[*si] != ' ')
          *di++ = *si;
        else if (*si == ',') {
          *di++ = 0;
          si++;
          n = 0;
          while (*si >= '0' && *si <= '9') {
            n = n * 10 + *si - '0';
            si++;
          }
          si--;
          *(word *)di = n;
          di += 2;
        }
      } while (++si < index_end);
    }
  }
}

//-----------------------------------------------------------------------------
//      Build the hypertext index
//-----------------------------------------------------------------------------

void make_helpidx(void) {
  long m = -1, n, len;
  byte *help = NULL, *i = NULL, *help_end;
  FILE *f;

  i_back = a_back = f_back = 0; // Clear the queue

  memset(helpidx, 0, sizeof(helpidx));
  if ((f = fopen("help/help.div", "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if ((help = (byte *)malloc(len + 10)) != NULL) {
      memset(help, 0, len + 10);
      fseek(f, 0, SEEK_SET);
      fread(help, 1, len, f);
      fclose(f);
      help_end = help + len;
      i = help;
      do {
        if (*(word *)i == '{' + ('.' * 256)) {
          len = (long)(i - help);
          n = 0;
          i += 2;
          while (*i >= '0' && *i <= '9')
            n = n * 10 + *i++ - 0x30;

          helpidx[n * 2] = (long)(i + 1 - help); // Set the start offset of topic n

          if (m >= 0)
            helpidx[m * 2 + 1] = len - helpidx[m * 2]; // Set the length of the previous topic

          m = n;
        }
      } while (++i < help_end);

      if (m >= 0)
        helpidx[m * 2 + 1] = (long)(i - help) - helpidx[m * 2];
      free(help);
    }
  }
}

//-----------------------------------------------------------------------------
//      Help window handlers
//-----------------------------------------------------------------------------

int forced_helpslider = 0;

int get_helpslide_y(void) {
  return (18 + (help_l * (v.h / big2 - 21 - 18)) / (help_lines - help_h));
}

void barra_vertical(void) {
  byte *ptr = v.ptr;
  int min, max, slider;
  int w = v.w / big2, h = v.h / big2;

  wbox(ptr, w, h, c2, w - 9, 18, 7, h - 36); // Gray slider background

  min = 18;
  max = h - 21;

  if (help_lines > help_h) {
    slider = get_helpslide_y();
    if (slider > max)
      slider = max;
  } else
    slider = min;

  if (forced_helpslider) {
    slider = forced_helpslider;
    forced_helpslider = 0;
  }

  wbox(ptr, w, h, c0, w - 9, slider - 1, 7, 1);
  wbox(ptr, w, h, c0, w - 9, slider + 3, 7, 1);
  wput(ptr, w, h, w - 9, slider, 43);
}

void help1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;

  _show_items();

  wbox(ptr, w, h, c0, w - 10, 10, 9, h - 12); // Black slider background

  wput(ptr, w, h, w - 9, 10, 39);      // Up
  wput(ptr, w, h, w - 9, h - 17, 40); // Down
  wput(ptr, w, h, w - 9, h - 9, 34);  // Resize
  barra_vertical();

  wbox(ptr, w, h, c0, 2, 10 + 15, w - 12, 1);

  vuelca_help();
}

void help2(void) {
  int w = v.w / big2, h = v.h / big2;
  int n, mx, my, f2_back, max, min;
  byte *p, *si, *di, c;
  static int old_estado;

  if (v.state) {
    _process_items();

    if (v.active_item == 2 && help_item != 3)
      help_xref(3, 0);
    if (v.active_item == 3 && help_item != 4)
      help_xref(4, 0);
    if (v.active_item == 4 && help_item != 1032)
      help_xref(1032, 0);
    if (v.active_item == 5) {
      Print_Help();
      return;
    }
    if (v.active_item == 6 && help_item != 1)
      help_xref(1, 0);

    if (!help_paint_active) {
      if (scan_code == 74) { // Previous help topic (-)
        n = help_item;
        while (--n >= 0) {
          if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
            help(n);
            break;
          }
        }
        return;
      }
      if (scan_code == 78) { // Next help topic (+)
        n = help_item;
        while (++n < 2048) {
          if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
            help(n);
            break;
          }
        }
        return;
      }
    }

    if (wmouse_in(w - 9, 10, 9, h - 20)) { // Vertical slider
      if (wmouse_y < 18)
        mouse_graf = 7;
      else if (wmouse_y >= h - 17)
        mouse_graf = 9;
      else
        mouse_graf = 13;
    } else if (wmouse_in(w - 9, h - 9, 9, 9)) { // Resize
      mouse_graf = 12;
    }

    if (scan_code == 72 || (mouse_graf == 7 && (mouse_b & 1) && wmouse_x != -1)) {
      if (scan_code != 72) {
        if (!(v.buttons & 2)) {
          wput(v.ptr, w, h, w - 9, 10, -41);
          v.buttons |= 2;
        } else {
          retrace_wait();
          retrace_wait();
        }
      }
      v.redraw++;
      if (help_line != help_buffer + 1) {
        help_line--;
        while (*(--help_line))
          ;
        help_line++;
        help_l--;
      }
      vuelca_help();
    } else if (v.buttons & 2) {
      wput(v.ptr, w, h, w - 9, 10, -39);
      v.buttons ^= 2;
      v.redraw++;
    }
    if (scan_code == 73 || (mouse_b & 8 && wmouse_x != -1)) {
      for (n = 0; n < (mouse_b & 8 ? 3 : help_h); n++) {
        if (help_line != help_buffer + 1) {
          help_line--;
          while (*(--help_line))
            ;
          help_line++;
          help_l--;
        }
      }
      vuelca_help();
      v.redraw++;
    }

    if (scan_code == 80 || (mouse_graf == 9 && (mouse_b & 1) && wmouse_x != -1)) {
      if (scan_code != 80) {
        if (!(v.buttons & 4)) {
          wput(v.ptr, w, h, w - 9, h - 17, -42);
          v.buttons |= 4;
        } else {
          retrace_wait();
          retrace_wait();
        }
      }
      v.redraw++;
      if (help_l + help_h < help_lines) {
        while (*(help_line++))
          ;
        help_l++;
      }
      vuelca_help();
    } else if (v.buttons & 4) {
      wput(v.ptr, w, h, w - 9, h - 17, -40);
      v.buttons ^= 4;
      v.redraw++;
    }

    if (scan_code == 81 || (mouse_b & 4 && wmouse_x != -1)) {
      for (n = 0; n < (mouse_b & 4 ? 3 : help_h); n++) {
        if (help_l + help_h < help_lines) {
          while (*(help_line++))
            ;
          help_l++;
        }
      }
      vuelca_help();
      v.redraw++;
    }

    if (mouse_graf == 13 && (mouse_b & 1) && wmouse_x != -1) {
      min = 18;
      max = h - 21;
      forced_helpslider = wmouse_y - 1;
      if (forced_helpslider < min)
        forced_helpslider = min;
      else if (forced_helpslider > max)
        forced_helpslider = max;
      n = ((help_lines - help_h) * (forced_helpslider - min)) / (max - min);
      if (n >= help_lines - help_h)
        n = help_lines - help_h - 1;
      if (n < 0)
        n = 0;
      while (help_l > n) {
        if (help_line != help_buffer + 1) {
          help_line--;
          while (*(--help_line))
            ;
          help_line++;
          help_l--;
        }
      }
      while (help_l < n) {
        if (help_l + help_h < help_lines) {
          while (*(help_line++))
            ;
          help_l++;
        }
      }
      v.buttons |= 128;
      vuelca_help();
      v.redraw++;
    } else {
      if (v.buttons & 128) {
        v.buttons ^= 128;
        vuelca_help();
        v.redraw++;
      }
    }

    if (mouse_graf == 12 && (mouse_b & 1))
      resize_help();

    barra_vertical();

    // Check if we are over a cross-reference

    if (wmouse_x != -1 && wmouse_y >= 10 + 16) {
      mx = mouse_x - v.x - 2 * big2;
      my = (mouse_y - v.y - (10 + 16) * big2) / font_height;

      p = help_line;
      while (p + 1 < help_end && my) {
        while (*p++) {}
        my--;
      }
      si = p;
      while (*si++)
        ;

      if (p + 1 < help_end) { // We are in a line of hypertext, in mx
        n = -1;
        while (*p && mx >= 0) {
          switch (*p) {
          case 1:
            p++;
            break;
          case 2:
            p++;
            n = -1;
            break;
          case 3:
            n = dtoi(*(int *)(p + 1));
            p += 5;
            break;
          case 5:
            if (dtoi(*(int *)(p + 5)) % 100 == 0) {
              mx -= ((graf_help[dtoi(*(int *)(p + 1))].ran + font_width - 1) / font_width) *
                    font_width;
            }
            p += 9;
            break;
          case 6:
            p++;
            break;
          default:
            if (*p < 32)
              mx -= *p - 6 + font_width;
            else
              mx -= font_width;
            p++;
            break;
          }
        }

        // We are over a cross-reference in the hypertext

        if (n != -1) {
          mouse_graf = 2;
          if ((mouse_b & 1) && !(prev_mouse_buttons & 1) && old_estado) {
            if (n == 9999) { // *** Extract an example ***
              if ((di = p = (byte *)malloc(16384)) != NULL) {
                while (*si != 6)
                  si++;
                while (*si++ == 6) {
                  while ((c = *si++)) {
                    if (c >= 32)
                      *di++ = c;
                    else
                      *di++ = ' ';
                  }
                  *di++ = 13;
                  *di++ = 10;
                }
                di -= 2;

                if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
                  memset(v_prg, 0, sizeof(struct tprg));

                  v_prg->buffer_len = 16384;
                  div_strcpy(v_prg->filename, sizeof(v_prg->filename), (char *)texts[220]);
                  div_strcpy(v_prg->path, sizeof(v_prg->path), (char *)tipo[1].path);
                  v_prg->file_len = di - p;
                  v_prg->buffer = p;
                  v_prg->lptr = p;
                  v.prg = v_prg;
                  read_line();
                  v.prg = NULL;
                  v_prg->num_lines = 1;
                  n = v_prg->file_len;
                  while (n--)
                    if (*p++ == 13)
                      v_prg->num_lines++;
                  new_window(program0);
                }
              }
            } else if (helpidx[n * 2] && helpidx[n * 2 + 1]) { // *** X-Ref ***
              if (n != help_item) {
                help_xref(n, 0); // May close the window if an error occurs
              }
            }
          }
        }
      }
    }

    if (((scan_code == 14 && !(shift_status & 3)) || v.active_item == 0) && a_back != i_back) {
      a_back -= 2;
      if (a_back < 0)
        a_back = 62;
      if (backto[a_back] == help_item) {
        if (a_back != i_back) {
          backto[a_back] = help_item;
          backto[a_back + 1] = help_l;
          a_back -= 2;
          if (a_back < 0)
            a_back = 62;
          f2_back = f_back;
          help_xref(backto[a_back], backto[a_back + 1]);
          f_back = f2_back;
        } else {
          a_back = (a_back + 2) % 64;
        }
      } else {
      }
    }

    if (((scan_code == 14 && (shift_status & 3)) || v.active_item == 1) && a_back != f_back) {
      f2_back = f_back;
      help_xref(backto[a_back], backto[a_back + 1]);
      f_back = f2_back;
    }

    if (!help_paint_active) {
      if (key(_ESC) && !key(_L_CTRL)) {
        if (mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2,
                     v.y + v.h - 2 * big2)) {
          close_window();
        } else
          window_closing = 2;
      }
    }

  } else
    old_prg = NULL;
  old_estado = v.state;
}

void help3(void) {
  if (help_buffer != NULL) {
    while (n_loaded) {
      n_loaded--;
      graf_help[loaded[n_loaded]].ptr = NULL;
    }
    help_buffer = NULL;
  }
}

void help0(void) { // help_item indicates which topic help is requested for
  int x;

  v.type = 102;

  v.w = (4 + 8) * big2 + font_width * help_w;
  v.h = (12 + 16) * big2 + font_height * help_h;

  if (big) {
    if (v.w & 1)
      v.w++;
    if (v.h & 1)
      v.h++;
    v.w = -v.w; // Indicate that the window should not be doubled
  }

  v.title = help_title;
  v.name = help_title;

  v.paint_handler = help1;
  v.click_handler = help2;
  v.close_handler = help3;

  _button(488, 6, 14, 0);
  x = 6 + text_len(texts[488]) + 2;
  _button(489, x, 14, 0);
  x += text_len(texts[489]) + 2 + 6;
  _button(490, x, 14, 0);
  x += text_len(texts[490]) + 2;
  _button(491, x, 14, 0);
  x += text_len(texts[491]) + 2;
  _button(492, x, 14, 0);
  x += text_len(texts[492]) + 2 + 6;
  _button(494, x, 14, 0);
  _button(493, abs(v.w) / big2 - 15, 14, 2);
}

//-----------------------------------------------------------------------------
//      Resize the help window
//-----------------------------------------------------------------------------

void resize_help(void) {
  int _mx = mouse_x, _my = mouse_y; // Initial mouse coordinates
  int my;                           // Snapped mouse coordinates at each moment
  int _al;                          // Original height in chars
  int old_h;                       // Previous height
  byte *new_block;
  int w = v.w / big2, h = v.h / big2;

  _al = help_h;

  wput(v.ptr, w, h, w - 9, h - 9, -44);
  flush_window(0);

  do {
    read_mouse();
    my = _my + ((mouse_y - _my) / font_height) * font_height;

    old_h = help_h;
    help_h = _al + (mouse_y - _my) / font_height;
    if (help_h < 4 * big2) {
      help_h = 4 * big2;
      my = _my + (help_h - _al) * font_height;
    }
    if (help_h > 50) {
      help_h = 50;
      my = _my + (help_h - _al) * font_height;
    }

    h = v.h;

    v.h = (12 + 16) * big2 + font_height * help_h;

    if (big) {
      if (v.h & 1)
        v.h++;
    }

    if ((new_block = (byte *)realloc(v.ptr, v.w * v.h)) != NULL) {
      window_surface(v.w, v.h, 0);

      if (draw_mode < 100) {
        draw_edit_background(v.x, v.y, v.w > w ? v.w : w, v.h > h ? v.h : h);
        flush_bars(1);
      }

      v.ptr = new_block;
      repaint_window();
      wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 9, v.h / big2 - 9, -44);
      on_window_moved(v.x, v.y, w, h);

      if (draw_mode >= 100) {
        update_box(v.x, v.y, v.w > w ? v.w : w, v.h > h ? v.h : h);
      }

    } else {
      help_h = old_h;
      v.h = h;
    }

    h = v.h / big2;

    save_mouse_bg(mouse_background, _mx, my, mouse_graf, 0);
    put(_mx, my, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, _mx, my, mouse_graf, 1);

  } while (mouse_b & 1);

  wput(v.ptr, w, h, w - 9, h - 9, -34);
  v.redraw = 2;
}

//-----------------------------------------------------------------------------
//      Convert an int between packed-decimal and integer, and vice versa
//-----------------------------------------------------------------------------

int itod(int n) {
  int m;
  m = (n % 10) + '0';
  m *= 256;
  n /= 10;
  m += (n % 10) + '0';
  m *= 256;
  n /= 10;
  m += (n % 10) + '0';
  m *= 256;
  n /= 10;
  return (m + n % 10 + '0');
}

int dtoi(int m) {
  int n;
  n = (m % 256) - '0';
  n *= 10;
  m /= 256;
  n += (m % 256) - '0';
  n *= 10;
  m /= 256;
  n += (m % 256) - '0';
  n *= 10;
  m /= 256;
  return (n + m % 256 - '0');
}

//-----------------------------------------------------------------------------
//      Help
//-----------------------------------------------------------------------------

int determine_help(void) {
  int m, n = -1;

  for (m = 0; m < max_windows; m++) {
    if (window[m].type == 102 && window[m].click_handler == help2) {
      n = m;
      break;
    }
  }
  return (n);
}

int determine_calc(void);

void help(int n) {
  FILE *f;
  byte *p;
  int m, m_back;

  determine_prg2();

  if (v_window != -1) {
    old_prg = window[v_window].prg;
  } else {
    determine_calc();
    if (v_window != -1) {
      old_prg = (struct tprg *)window[v_window].aux;
    }
  }

  if ((m = determine_help()) != -1) {
    if (m)
      move(0, m);
    if (v.foreground == 2)
      maximize_window();

    if (m && v.foreground == 0) { // If it was in the background
      for (m = 1; m < max_windows; m++)
        if (window[m].type && window[m].foreground == 1)
          if (windows_collide(0, m)) {
            window[m].foreground = 0;
            flush_window(m);
          }
      v.foreground = 1;
    }

    if (help_item == n) {
      while (help_line != help_buffer + 1) {
        help_line--;
        while (*(--help_line))
          ;
        help_line++;
        help_l--;
      }
      vuelca_help();
    } else {
      help_xref(n, 0);
    }

    if (!v.state)
      activate();
    flush_window(0);

    return;
  }

  if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
    if ((f = fopen("help/help.div", "rb")) != NULL) {
      fseek(f, helpidx[n * 2], SEEK_SET);
      if ((h_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 2048)) != NULL) {
        if ((help_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 2048)) != NULL) {
          if (i_back != a_back) {
            m_back = a_back - 2;
            if (m_back < 0)
              m_back = 62;
            if (backto[m_back] == help_item && backto[m_back + 1] == -1)
              backto[m_back + 1] = help_l;
            if (backto[m_back] == n)
              a_back = m_back;
          }

          help_item = n;

          backto[a_back] = n;
          backto[a_back + 1] = -1;
          f_back = a_back = (a_back + 2) % 64;
          if (i_back == f_back)
            i_back = (i_back + 2) % 64;

          fread(h_buffer, 1, helpidx[n * 2 + 1], f);
          p = h_buffer;
          while (*p != '}')
            p++;
          *p = 0;
          div_strcpy((char *)help_title, sizeof(help_title), (char *)h_buffer);
          help_w = (vga_width - 12 * big2 - 1) / font_width;
          if (help_w > 120)
            help_w = 120;
          help_h = (vga_height / 2 - (12 + 16) * big2 - 1) / font_height;
          help_l = 0;
          if (primera_vez)
            help_h += 5;
          tabula_help(p + 1, help_buffer, helpidx[n * 2 + 1] - (p + 1 - h_buffer));
          new_window(help0);
        }
      }
      fclose(f);
    }
  }
}

//-----------------------------------------------------------------------------
//      Help in the paint program
//-----------------------------------------------------------------------------

void help_paint0(void) { // help_item indicates which topic help is requested for
  int x;

  v.type = 1;

  v.w = (4 + 8) * big2 + font_width * help_w;
  v.h = (12 + 16) * big2 + font_height * help_h;

  if (big) {
    if (v.w & 1)
      v.w++;
    if (v.h & 1)
      v.h++;
    v.w = -v.w; // Indicate that the window should not be doubled
  }

  v.title = help_title;
  v.name = help_title;

  v.paint_handler = help1;
  v.click_handler = help2;
  v.close_handler = help3;

  v.state = 1;

  _button(488, 6, 14, 0);
  x = 6 + text_len(texts[488]) + 2;
  _button(489, x, 14, 0);
  x += text_len(texts[489]) + 2 + 6;
  _button(490, x, 14, 0);
  x += text_len(texts[490]) + 2;
  _button(491, x, 14, 0);
  x += text_len(texts[491]) + 2;
  _button(492, x, 14, 0);
  x += text_len(texts[492]) + 2 + 6;
  _button(494, x, 14, 0);
  _button(493, abs(v.w) / big2 - 15, 14, 2);
}

void help_paint(memptrsize n) {
  FILE *f;
  byte *p;
  int m_back;

  if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
    if ((f = fopen("help/help.div", "rb")) != NULL) {
      fseek(f, helpidx[n * 2], SEEK_SET);
      if ((h_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 2048)) != NULL) {
        if ((help_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 2048)) != NULL) {
          if (i_back != a_back) {
            m_back = a_back - 2;
            if (m_back < 0)
              m_back = 62;
            if (backto[m_back] == help_item && backto[m_back + 1] == -1)
              backto[m_back + 1] = help_l;
            if (backto[m_back] == n)
              a_back = m_back;
          }

          help_item = n;

          backto[a_back] = n;
          backto[a_back + 1] = -1;
          f_back = a_back = (a_back + 2) % 64;
          if (i_back == f_back)
            i_back = (i_back + 2) % 64;

          fread(h_buffer, 1, helpidx[n * 2 + 1], f);
          p = h_buffer;
          while (*p != '}')
            p++;
          *p = 0;
          div_strcpy((char *)help_title, sizeof(help_title), (char *)h_buffer);
          help_w = (vga_width - 12 * big2 - 1) / font_width;
          if (help_w > 120)
            help_w = 120;
          help_h = (vga_height / 2 - (12 + 16) * big2 - 1) / font_height;
          help_l = 0;
          tabula_help(p + 1, help_buffer, helpidx[n * 2 + 1] - (p + 1 - h_buffer));
          help_paint_active = 1;
          show_dialog(help_paint0);
          help_paint_active = 0;
        }
      }
      fclose(f);
    }
  }
}
//-----------------------------------------------------------------------------
//  Get error message n into cerror[]
//-----------------------------------------------------------------------------

char cerror[128];

extern int error_line;

void get_error(int n) {
  FILE *f;
  byte *p;
  DIV_SPRINTF(cerror, "%s", (char *)texts[381]);
  p = (byte *)cerror + strlen((char *)cerror);

  if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
    if ((f = fopen("help/help.div", "rb")) != NULL) {
      fseek(f, helpidx[n * 2], SEEK_SET);
      fread(p, 1, 128 - strlen(cerror), f);
      fclose(f);
      while (*p != '}') {
        p++;
      }
      *p = 0;
    }
  }

  if (n == 502 || n == 503 || n == 504 || n == 507) {
    /* Fix overlapping sprintf: cerror is both dest and arg */
    char tmp[sizeof(cerror)];
    DIV_SPRINTF(tmp, "%s (%d).", cerror, error_line);
    DIV_STRCPY(cerror, tmp);
  }
}

//-----------------------------------------------------------------------------
//      Navigate to another topic via cross-reference
//-----------------------------------------------------------------------------

void help_xref(int n, int linea) {
  FILE *f;
  byte *p;
  int m_back;

  while (n_loaded) {
    n_loaded--;
    graf_help[loaded[n_loaded]].ptr = NULL;
  }

  help_buffer = NULL;
  if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
    if ((f = fopen("help/help.div", "rb")) != NULL) {
      fseek(f, helpidx[n * 2], SEEK_SET);
      if ((h_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 2048)) != NULL) {
        if ((help_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 2048)) != NULL) {
          if (i_back != a_back) {
            m_back = a_back - 2;
            if (m_back < 0)
              m_back = 62;
            if (backto[m_back] == help_item && backto[m_back + 1] == -1)
              backto[m_back + 1] = help_l;
            if (backto[m_back] == n)
              a_back = m_back;
          }

          help_item = n;

          backto[a_back] = n;
          backto[a_back + 1] = -1;
          f_back = a_back = (a_back + 2) % 64;
          if (i_back == f_back)
            i_back = (i_back + 2) % 64;

          fread(h_buffer, 1, helpidx[n * 2 + 1], f);
          p = h_buffer;
          while (*p != '}')
            p++;
          *p = 0;
          div_strcpy((char *)help_title, sizeof(help_title), (char *)h_buffer);
          help_l = 0;
          tabula_help(p + 1, help_buffer, helpidx[n * 2 + 1] - (p + 1 - h_buffer));
          if (linea != -1)
            while (linea--) {
              while (*help_line++)
                ;
              help_l++;
            }
          repaint_window();
          flush_window(0);
        }
      }
      fclose(f);
    }
  }
  if (help_buffer == NULL)
    close_window();
}

//-----------------------------------------------------------------------------
//      Format help text for a given width (help_w)
//-----------------------------------------------------------------------------

// graf[n] pointer to width,height,x0,x1,graphic... (width..x1 are word)

int ejemplo = 0; // An example follows
int imagen = 0;  // Image code
int imagen_y;    // Current line of the image
int image_h;   // Image height in chars
int image_w;   // Image width in chars
int tipo_imagen; // 0-left, 1-center, 2-right
int restore_help_w = 0;

byte *continua_imagen(byte *di) {
  int n = 0;

  if (restore_help_w) {
    restore_help_w = 0;
    help_w += image_w;
  }
  if (ejemplo)
    *di++ = 6;
  else if (imagen) {
    *di++ = 5;
    *(int *)di = itod(imagen);
    di += 4;
    switch (tipo_imagen) {
    case 0:
      n = 0;
      break;
    case 1:
      n = help_w / 2;
      break;
    case 2:
      n = help_w;
      break;
    }
    n += imagen_y++ * 100;
    *(int *)di = itod(n);
    di += 4;
    if (imagen_y == image_h) {
      imagen = 0;
      restore_help_w = 1;
    }
  }
  return (di);
}

void tabula_help(byte *si, byte *di, int lon) {
  byte *old_di2, *end = si + lon, c, *di2, *ptr;
  int ultimo_cr = 1, ultimo_cr_real = 1, tex;
  int estado = 0, chars, chars2, nchars, help_an2;
  int x, y;
  FILE *f;

  *di++ = 0;

  chars = 0;
  help_lines = 1;
  n_loaded = 0;

ini_tabulador:

  while (si < end) {
    c = *si++;

    // Characters 0..31 are replaced with '?'

    if (c < 32 && c != 13 && c != 10)
      *(si - 1) = c = '?';

    // Comments

    if (c == '#' && ultimo_cr_real && si < end) {
      while (si < end && *si != '\n')
        si++;
      if (si < end)
        si++; // skip LF
      goto ini_tabulador;
    }
    if (si >= end)
      break;

    // Line breaks

    if (c == 13 || c == 10) {
      if (c == 13 && si < end && *si == 10)
        si++; // skip LF after CR
      if (ejemplo) {
        ultimo_cr_real = 1;
        ultimo_cr = 2;
        *di++ = 0;
        help_lines++;
        di = continua_imagen(di);
        c = 0;
        chars = 0;
      } else {
        if (ultimo_cr == 1) {
          *di++ = 0;
          help_lines++;
          di = continua_imagen(di);
          ultimo_cr = 2;
        }
        if (si >= end || *si == 13 || *si == 10)
          c = 0;
        else if (!ultimo_cr)
          c = ' ';
        else
          c = 0;
        ultimo_cr_real = 1;
      }
    } else
      ultimo_cr_real = 0;

    // While chars or line breaks keep coming

    if (c != 0 || !ultimo_cr) {
      if (c) { // If a character arrives

        // Command handling

        if (c == '{') {
          switch (*si) {
          case '#': // X-ref
            estado = 0;
            si++;
            while (*si >= '0' && *si <= '9') {
              estado *= 10;
              estado += *si - '0';
              si++;
            }
            estado = -estado;
            si++;
            if (estado == -9999)
              ejemplo = 1;
            *di++ = 3;
            *(int *)di = itod(-estado);
            di += 4;
            continue;

          case '{': // '{'
            c = '{';
            si += 2;
            break;

          case '}': // '}'
            c = '}';
            si += 2;
            break;

          case 'b': // {br} — forced line break
            if (si[1] == 'r' && si[2] == '}') {
              si += 3;
              *di++ = 0;
              help_lines++;
              ultimo_cr = 1;
              chars = 0;
              if (*si == '\r')
                si++; // skip CR if present
              if (*si == '\n')
                si++; // skip LF
              continue;
            }
            // else fall through to default (bold text)
            c = 1;
            estado = 1;
            chars--;
            break;

          case '/': // Horizontal rule
            si += 2;
            if (imagen) {
              if (!ultimo_cr) {
                *di++ = 0;
                help_lines++;
                ultimo_cr = 1;
                chars = 0;
              }
              do {
                di = continua_imagen(di);
                *di++ = 0;
                help_lines++;
                ultimo_cr = 1;
                chars = 0;
              } while (imagen);
            }
            if (!ultimo_cr) {
              *di++ = 0;
              help_lines++;
              ultimo_cr = 1;
              chars = 0;
            }
            if (restore_help_w) {
              restore_help_w = 0;
              help_w += image_w;
            }
            *di++ = 4;
            *di++ = 0;
            help_lines++;
            ultimo_cr = 1;
            chars = 0;
            continue;

          case '+': // Image
            if (imagen) {
              if (!ultimo_cr) {
                *di++ = 0;
                help_lines++;
                ultimo_cr = 1;
                chars = 0;
              }
              do {
                di = continua_imagen(di);
                *di++ = 0;
                help_lines++;
                ultimo_cr = 1;
                chars = 0;
              } while (imagen);
            }
            if (!ultimo_cr) {
              *di++ = 0;
              help_lines++;
              ultimo_cr = 1;
              chars = 0;
            }
            if (restore_help_w) {
              restore_help_w = 0;
              help_w += image_w;
            }
            imagen = 0;
            si++;
            while (*si >= '0' && *si <= '9') {
              imagen *= 10;
              imagen += *si - '0';
              si++;
            }
            si++;
            tipo_imagen = 0;
            while (*si >= '0' && *si <= '9') {
              tipo_imagen *= 10;
              tipo_imagen += *si - '0';
              si++;
            }
            si++;
            if (imagen < 384 && graf_help[imagen].offset) {
              if ((f = fopen("help/help.fig", "rb")) != NULL) {
                if ((ptr = (byte *)malloc(graf_help[imagen].w * graf_help[imagen].h)) != NULL) {
                  fseek(f, graf_help[imagen].offset, SEEK_SET);
                  fread(ptr, graf_help[imagen].w, graf_help[imagen].h, f);
                  fclose(f);
                  for (x = graf_help[imagen].w * graf_help[imagen].h - 1; x >= 0; x--) {
                    ptr[x] = help_xlat[ptr[x]];
                  }

                  if (vga_width < 640 && graf_help[imagen].w > 300) {
                    for (y = 0; y < graf_help[imagen].h - 1; y += 2) {
                      for (x = 0; x < graf_help[imagen].w - 1; x += 2) {
                        ptr[(y / 2) * (graf_help[imagen].w / 2) + x / 2] =
                            *(ghost + ptr[y * graf_help[imagen].w + x] * 256 +
                              ptr[(y + 1) * graf_help[imagen].w + x + 1]);
                      }
                    }
                    graf_help[imagen].ran = graf_help[imagen].w / 2;
                    graf_help[imagen].ral = graf_help[imagen].h / 2;
                  } else {
                    graf_help[imagen].ran = graf_help[imagen].w;
                    graf_help[imagen].ral = graf_help[imagen].h;
                  }

                  graf_help[imagen].ptr = ptr;
                  loaded[n_loaded++] = imagen;
                  imagen_y = 0;
                  image_h = (graf_help[imagen].ral + font_height - 1) / font_height;
                  image_w = (graf_help[imagen].ran + font_width - 1) / font_width;
                  help_w -= image_w;
                  if (help_w <= 20)
                    tipo_imagen = 1;

                  do {
                    di = continua_imagen(di);
                    if (tipo_imagen == 1) {
                      *di++ = 0;
                      help_lines++;
                      ultimo_cr = 1;
                      chars = 0;
                    }
                  } while (tipo_imagen == 1 && imagen);

                } else {
                  fclose(f);
                  imagen = 0;
                }
              } else
                imagen = 0;
            } else
              imagen = 0;

            continue;

          case '-': // End image
            si += 2;
            if (ejemplo) {
              ejemplo = 0;
              ultimo_cr = 1;
              if (*(di - 1) == 6)
                di--;
            } else if (imagen) {
              if (!ultimo_cr) {
                *di++ = 0;
                help_lines++;
                ultimo_cr = 1;
                chars = 0;
              }
              do {
                di = continua_imagen(di);
                *di++ = 0;
                help_lines++;
                ultimo_cr = 1;
                chars = 0;
              } while (imagen);
            }
            continue;

          case '@': // Text from lenguaje.div
            tex = 0;
            si++;
            while (*si >= '0' && *si <= '9') {
              tex *= 10;
              tex += *si - '0';
              si++;
            }
            si++;
            if (*texts[tex] == '-') {
              memcpy(di, texts[tex] + 1, strlen((char *)texts[tex] + 1) + 1);
            } else {
              memcpy(di, texts[tex], strlen((char *)texts[tex]) + 1);
            }
            di += strlen((char *)di);
            continue;

          default: // Bold
            c = 1;
            estado = 1;
            chars--;
            break;
          }
        } else if (c == '}') {
          c = 2;
          chars--;
          estado = 0;
        }

        chars++;

        if (!ejemplo && chars == help_w + 1) {
          chars2 = chars;
          help_an2 = help_w; // Fix the line that was truncated

          nchars = chars = 0;
          *di = c;
          old_di2 = di + 1;

          while (*di != ' ') {
            if (*di == 1) { // *** 4 cannot appear here ***
              estado = 0;
              chars--;
              chars2++;
            } else if (*di == 2) {
              estado = 1;
              chars--;
              chars2++;
            } else if (*di == 3) {
              estado = 0;
              chars -= 5;
              chars2 += 5;
            }
            di--;
            chars++;
            nchars++;
            chars2--;
          }

          di2 = di; // Where the previous line ends

          if (estado == 0) { // Normal text is being split
            if (imagen) {
              memmove(di + 10, di + 1, nchars);
              *di++ = 0;
              help_lines++;
              di = continua_imagen(di);
              di = old_di2 + 9;
            } else {
              if (restore_help_w) {
                restore_help_w = 0;
                help_w += image_w;
              }
              *di++ = 0;
              help_lines++;
              di = old_di2;
            }
          } else if (estado == 1) { // In the middle of bold text
            if (imagen) {
              memmove(di + 12, di + 1, nchars);
              *di++ = 2;
              *di++ = 0;
              help_lines++;
              di = continua_imagen(di);
              *di = 1;
              di = old_di2 + 11;
            } else {
              if (restore_help_w) {
                restore_help_w = 0;
                help_w += image_w;
              }
              memmove(di + 3, di + 1, nchars);
              *di++ = 2;
              *di++ = 0;
              help_lines++;
              *di = 1;
              di = old_di2 + 2;
            }
          } else { // In the middle of an x-ref
            if (imagen) {
              memmove(di + 16, di + 1, nchars);
              *di++ = 2;
              *di++ = 0;
              help_lines++;
              di = continua_imagen(di);
              *di++ = 3;
              *(int *)di = itod(-estado);
              di = old_di2 + 15;
            } else {
              if (restore_help_w) {
                restore_help_w = 0;
                help_w += image_w;
              }
              memmove(di + 7, di + 1, nchars);
              *di++ = 2;
              *di++ = 0;
              help_lines++;
              *di++ = 3;
              *(int *)di = itod(-estado);
              di = old_di2 + 6;
            }
          }

          if (!ejemplo)
            arregla_linea(di2, chars2, help_an2);

        } else {
          *di = c;
          di++;
          ultimo_cr = 0;
        }

      } else {
        chars = 0;
        help_lines++;

        if (estado == 0) {
          *di++ = 0;
          di = continua_imagen(di);
        } else if (estado == 1) {
          *di++ = 2;
          *di++ = 0;
          di = continua_imagen(di);
          *di++ = 1;
        } else {
          *di++ = 2;
          *di++ = 0;
          di = continua_imagen(di);
          *di++ = 3;
          *(int *)di = -estado;
          di += 4;
        }

        if (!ultimo_cr)
          ultimo_cr = 1;
      }
    }
  }

  if (restore_help_w) {
    help_w += image_w;
    restore_help_w = 0;
  }
  help_line = help_buffer + 1;
  help_end = di;
  help_lines--;
}

//-----------------------------------------------------------------------------
//      Justify a line (generate correct spacing)
//-----------------------------------------------------------------------------

void arregla_linea(byte *end, int chars, int help_w) {
  byte *ini, *i, *_i;
  int espacios, error, chesp;
  float nuevo_esp, espacio_acum;

  ini = end - 1;
  while (*ini)
    ini--;

  _i = ini + 1;
  espacios = 0;
  while (_i < end)
    if (*_i == ' ')
      _i++;
    else
      break;
  i = _i;
  while (i < end) {
    if (*i == ' ') {
      espacios++;
      i++;
    } else if (*i == 1 || *i == 2 || *i == 6)
      i++;
    else if (*i == 3)
      i += 5;
    else if (*i == 5)
      i += 9;
    else
      i++;
  }

  if (espacios) {
    error = (help_w + 1 - chars) * font_width;
    if (error) {
      nuevo_esp = (float)font_width + (float)error / (float)espacios;
      if (nuevo_esp > font_width + 25)
        nuevo_esp = font_width + 25;
      espacio_acum = 0;

      i = _i;
      while (i < end) {
        if (*i == ' ') {
          espacio_acum += nuevo_esp;
          chesp = espacio_acum;
          espacio_acum -= chesp;
          if (chesp != font_width)
            *i = chesp - font_width + 6;
          i++;
        } else if (*i == 1 || *i == 2 || *i == 6)
          i++;
        else if (*i == 3)
          i += 5;
        else if (*i == 5)
          i += 9;
        else
          i++;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//      Render the hypertext window
//-----------------------------------------------------------------------------

void vuelca_help(void) {
  byte *si, *di, *old_di;
  int h, n, ch;
  byte color = c0, color2 = average_color(c0, c_b_low);

  wbox(v.ptr, v.w / big2, v.h / big2, c12, 2, 10 + 16, v.w / big2 - 12, v.h / big2 - 12 - 16);

  di = v.ptr + (v.w * (10 + 16) + 2) * big2; // Blit the text window
  si = help_line;
  h = help_h;

  while (h-- && si < help_end) {
    old_di = di;
    while (*si) {
      if (*si < 7)
        switch (*si) {
        case 1:
          color = c3;
          break;
        case 2:
          color = c0;
          break;
        case 3:
          color = c4;
          si += 4;
          break;
        case 4:
          color = c0;
          n = help_w;
          while (n--) {
            put_chr(di, v.w, '*', color);
            di += font_width;
          }
          break;
        case 5:
          n = dtoi(*(int *)(si + 5));
          put_image_line(dtoi(*(int *)(si + 1)), n / 100, old_di + (n % 100) * font_width, v.w);
          if (n % 100 == 0) {
            n = dtoi(*(int *)(si + 1));
            di = old_di + ((graf_help[n].ran + font_width - 1) / font_width) * font_width;
          }
          si += 8;
          break;
        case 6:
          di += font_width * 2;
          si++;
          ch = 0;
          while (*si && ch++ < help_w - 2) {
            put_chr(di, v.w, *si, color2);
            di += font_width;
            si++;
          }
          while (*si)
            si++;
          si--;
        }
      else if (*si < 32) {
        di += *si - 6 + font_width;
      } else {
        put_chr(di, v.w, *si, color);
        di += font_width;
      }
      si++;
    }
    si++;
    di = old_di + v.w * font_height;
  }
}

//-----------------------------------------------------------------------------
//      Render a single line of an image
//-----------------------------------------------------------------------------

void put_image_line(int n, int linea, byte *di, int v_w) {
  int w, h, _an, c;
  byte *si;

  if (!graf_help[n].offset)
    return;

  _an = graf_help[n].ran;
  h = graf_help[n].ral - linea * font_height;
  if (h > font_height)
    h = font_height;
  si = graf_help[n].ptr + linea * font_height * _an;

  do {
    w = _an;
    do {
      if ((c = *si++))
        *di = c;
      di++;
    } while (--w);
    di += v_w - _an;
  } while (--h);
}

//-----------------------------------------------------------------------------
//      Render a single character
//-----------------------------------------------------------------------------

void put_chr(byte *ptr, int w, byte c, byte color) {
  int n, m;
  byte *si;

  si = font + c * font_width * font_height;
  n = font_height;
  do {
    m = font_width;
    do {
      if (*si)
        *ptr = color;
      si++;
      ptr++;
    } while (--m);
    ptr += w - font_width;
  } while (--n);
}

//-----------------------------------------------------------------------------
//  Format a help topic for printing
//-----------------------------------------------------------------------------

#define ancho_ayuda 70

void tabula_help2(byte *si, byte *di, int lon) {
  byte *old_di, *end = si + lon, c;
  int ultimo_cr = 1, ultimo_cr_real = 1, chars = 0, n;

  *di++ = 0;

ini_tabulador:

  while (si < end) {
    c = *si++;

    // Characters 0..31 are replaced with '?'

    if (c < 32 && c != 13 && c != 10)
      *(si - 1) = c = '?';

    // Comments

    if (c == '#' && ultimo_cr_real && si < end) {
      while (si < end && *si != '\n')
        si++;
      if (si < end)
        si++; // skip LF
      goto ini_tabulador;
    }

    if (si >= end)
      break;

    // Line breaks

    if (c == 13 || c == 10) {
      if (c == 13 && si < end && *si == 10)
        si++; // skip LF after CR
      if (ejemplo) {
        ultimo_cr_real = 1;
        ultimo_cr = 2;
        *di++ = 0;
        c = 0;
        chars = 0;
      } else {
        if (ultimo_cr == 1) {
          *di++ = 0;
          ultimo_cr = 2;
        }
        if (si >= end || *si == 13 || *si == 10)
          c = 0;
        else if (!ultimo_cr)
          c = ' ';
        else
          c = 0;
        ultimo_cr_real = 1;
      }
    } else
      ultimo_cr_real = 0;

    // While chars or line breaks keep coming

    if (c != 0 || !ultimo_cr) {
      if (c) { // If a character arrives

        // Command handling

        if (c == '{') {
          switch (*si) {
          case '#': // X-ref
            n = 0;
            si++;
            while (*si >= '0' && *si <= '9') {
              n *= 10;
              n += *si - '0';
              si++;
            }
            si++;
            if (n == 9999)
              ejemplo = 1;
            continue;

          case '{': // '{'
            c = '{';
            si += 2;
            break;

          case '}': // '}'
            c = '}';
            si += 2;
            break;

          case 'b': // {br} — forced line break
            if (si[1] == 'r' && si[2] == '}') {
              si += 3;
              *di++ = 0;
              ultimo_cr = 1;
              chars = 0;
              if (*si == '\r')
                si++; // skip CR if present
              if (*si == '\n')
                si++; // skip LF
              continue;
            }
            // else fall through to default (bold/ignored in print)
            continue;

          case '/': // Horizontal rule
            si += 2;
            if (!ultimo_cr) {
              *di++ = 0;
              ultimo_cr = 1;
              chars = 0;
            }
            memcpy(di,
                   "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
                   "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
                   "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
                   "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4",
                   71);
            di += strlen((char *)di) + 1;
            ultimo_cr = 1;
            chars = 0;
            continue;

          case '+': // Image
            if (!ultimo_cr) {
              *di++ = 0;
              ultimo_cr = 1;
              chars = 0;
            }
            si++;
            while (*si >= '0' && *si <= '9') {
              si++;
            }
            si++;
            while (*si >= '0' && *si <= '9') {
              si++;
            }
            si++;
            continue;

          case '-': // End image
            si += 2;
            if (ejemplo) {
              ejemplo = 0;
              ultimo_cr = 1;
            }
            continue;

          case '@': // Text from lenguaje.div
            n = 0;
            si++;
            while (*si >= '0' && *si <= '9') {
              n *= 10;
              n += *si - '0';
              si++;
            }
            si++;
            if (*texts[n] == '-') {
              memcpy(di, texts[n] + 1, strlen((char *)texts[n] + 1) + 1);
            } else {
              memcpy(di, texts[n], strlen((char *)texts[n]) + 1);
            }
            di += strlen((char *)di);
            continue;

          default: // Bold
            continue;
          }
        } else if (c == '}') {
          continue;
        }

        chars++;

        if (!ejemplo && chars == ancho_ayuda + 1) {
          chars = 0;
          *di = c;
          old_di = di + 1;
          while (*di != ' ') {
            di--;
            chars++;
          }
          *di++ = 0;
          di = old_di;
        } else {
          *di = c;
          di++;
          ultimo_cr = 0;
        }

      } else {
        chars = 0;
        *di++ = 0;
        if (!ultimo_cr)
          ultimo_cr = 1;
      }
    }
  }

  help_end = di;
}

//-----------------------------------------------------------------------------
//  Print a help page option
//-----------------------------------------------------------------------------

char h_ar[16] = "";
extern int f_ar;

void printlist0(void);

void Print_Help(void) {
  byte *print_buffer, *p, *_help_end;
  int n;
  char cwork[512];
  unsigned u;
  FILE *f, *g;

  if (!strlen(h_ar))
    div_strcpy(h_ar, sizeof(h_ar), (char *)texts[495]);

  v_text = h_ar;
  v_title = (char *)texts[496];
  show_dialog(printlist0);

  if (v_accept) {
    if (f_ar) {
      _dos_setdrive(toupper(*tipo[1].path) - 'A' + 1, &u);
      chdir(tipo[1].path);
      g = fopen(h_ar, "rb");
      if (g != NULL) {
        fclose(g);
        DIV_SPRINTF(cwork, "%s/%s", tipo[1].path, h_ar);
        strupr(cwork);
        v_title = (char *)texts[450];
        v_text = cwork;
        show_dialog(aceptar0);
        if (!v_accept)
          return;
      }
      g = fopen(h_ar, "wb");
      if (g == NULL) {
        v_text = (char *)texts[47];
        show_dialog(err0);
        return;
      }
    } else
      g = stdprn;

    if ((f = fopen("help/help.div", "rb")) != NULL) {
      fseek(f, helpidx[help_item * 2], SEEK_SET);
      if ((h_buffer = (byte *)malloc(helpidx[help_item * 2 + 1] + 2048)) != NULL) {
        if ((print_buffer = (byte *)malloc(helpidx[help_item * 2 + 1] + 2048)) != NULL) {
          fread(h_buffer, 1, helpidx[help_item * 2 + 1], f);
          p = h_buffer;
          while (*p != '}')
            p++;
          *p = 0;

          _help_end = help_end;
          tabula_help2(p + 1, print_buffer, helpidx[help_item * 2 + 1] - (p + 1 - h_buffer));

          // Print the help page title

          fwrite("\xd\xa", 1, 2, g);
          for (n = 0; n < (ancho_ayuda - strlen((char *)help_title) - 4) / 2; n++)
            fwrite(" ", 1, 1, g);
          fwrite("[ ", 1, 2, g);
          fwrite(help_title, 1, strlen((char *)help_title), g);
          fwrite(" ]"
                 "\xd\xa\xd\xa\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
                 "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
                 "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
                 "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4",
                 1, 76, g);

          p = print_buffer;

          while (p < help_end) {
            Progress((char *)texts[437], p - print_buffer, help_end - print_buffer);
            fwrite(p, 1, strlen((char *)p), g);
            fwrite("\xd\xa", 1, 2, g);
            p += strlen((char *)p) + 1;
          }

          Progress((char *)texts[437], help_end - print_buffer, help_end - print_buffer);

          help_end = _help_end;
        }
      }
      fclose(f);
    }

    if (f_ar)
      fclose(g);
    else
      fwrite("\f", 1, 1, stdprn);
  }
}

//-----------------------------------------------------------------------------
