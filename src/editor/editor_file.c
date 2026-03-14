
//-----------------------------------------------------------------------------
//      Code editor — file I/O and dialogs
//-----------------------------------------------------------------------------
//  Open/new/save, search/replace, process list, goto_error, print.
//-----------------------------------------------------------------------------

#include "editor_internal.h"
#include <time.h>

//-----------------------------------------------------------------------------
//      Open a program
//-----------------------------------------------------------------------------

void open_program(void) {
  FILE *f;
  int n, x;
  byte *buffer, *p;
  struct tprg *pr;
  int num;

  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), file_types[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
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
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);

      if ((f = fopen(full, "rb")) != NULL) { // A file was selected
        fseek(f, 0, SEEK_END);
        n = ftell(f) + BUFFER_INCREASE;
        if ((buffer = (byte *)malloc(n)) != NULL) {
          memset(buffer, 0, n);

          if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
            v_prg->buffer_len = n;
            div_strcpy(v_prg->filename, sizeof(v_prg->filename), input);
            div_strcpy(v_prg->path, sizeof(v_prg->path), file_types[v_type].path);
            fseek(f, 0, SEEK_SET);

            if (fread(buffer, 1, n, f) == n - BUFFER_INCREASE) {
              for (p = buffer; p < buffer + n - BUFFER_INCREASE; p++) // Reject binary files
                if (!*p) {
                  n = 0;
                  break;
                }

              if (n) {
                n -= BUFFER_INCREASE;
                v_prg->file_len = n;
                v_prg->buffer = buffer;
                v_prg->num_lines = 1;

                x = 0;
                while (n--) {
                  if (*buffer == cr || *buffer == lf) {
                    if (*(buffer + 1) != ((*buffer) ^ (cr ^ lf))) {
                      memmove(buffer + 1, buffer, n + 1);
                      v_prg->file_len++;
                    } else if (n)
                      n--;
                    *buffer++ = cr;
                    *buffer = lf;
                    v_prg->num_lines++;
                    x = -1;
                  }

                  if (*buffer == 9) {
                    if (x >= 1020)
                      *buffer = ' ';
                    else {
                      memmove(buffer + 3, buffer, n + 1);
                      v_prg->file_len += 3;
                      x += 3;
                      *buffer++ = ' ';
                      *buffer++ = ' ';
                      *buffer++ = ' ';
                      *buffer = ' ';
                    }
                  }

                  buffer++;
                  x++;

                  if (x >= 1023) {
                    memmove(buffer + 2, buffer, n + 1);
                    *buffer++ = cr;
                    *buffer++ = lf;
                    v_prg->file_len += 2;
                    v_prg->num_lines++;
                    x = 0;
                  }
                }

                v_prg->lptr = v_prg->buffer;
                pr = v.prg;
                v.prg = v_prg;
                read_line();
                v.prg = pr;

                new_window(program0);

              } else {
                free(v_prg);
                free(buffer);
                v_text = (char *)texts[46];
                show_dialog(err0);
              }
            } else {
              free(v_prg);
              free(buffer);
              v_text = (char *)texts[44];
              show_dialog(err0);
            }
          } else {
            free(buffer);
            v_text = (char *)texts[45];
            show_dialog(err0);
          }
        } else {
          v_text = (char *)texts[45];
          show_dialog(err0);
        }
        fclose(f);
      } else {
        if (v_finished != -1) {
          v_text = (char *)texts[44];
          show_dialog(err0);
        }
        v_finished = 0;
      }
    }
  }
}

void program0_new(void) {
  byte *buffer;
  struct tprg *pr;
  int n;
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  struct tm *timeinfo;
  time_t dtime;

  div_strcpy(full, sizeof(full), file_types[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  if (v_finished) {
    f = fopen(full, "wb");
    // insert template
    if (f) {
      time(&dtime);
      timeinfo = localtime(&dtime);
      _splitpath(full, drive, dir, fname, ext);
      fprintf(f, "/*\n * %s%s by %s\n * (c) %d %s \n */\n\n", fname, ext, user1,
              (timeinfo->tm_year) + 1900, user2);
      fprintf(f,
              "PROGRAM %s;\n\nBEGIN\n\n//Write your code here, make something amazing!\n\nEND\n\n",
              fname);
      fclose(f);
      open_program();

      for (n = 0; n < 10; n++) {
        write_line();
        advance_lptr();
        read_line();
      }

      return;
    }
    n = BUFFER_INCREASE;
    if ((buffer = (byte *)malloc(n)) != NULL) {
      memset(buffer, 0, n);
      if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
        memset(v_prg, 0, sizeof(struct tprg));
        v_prg->buffer_len = n;
        div_strcpy(v_prg->filename, sizeof(v_prg->filename), input);
        div_strcpy(v_prg->path, sizeof(v_prg->path), file_types[v_type].path);
        //        n-=BUFFER_INCREASE;
        n = strlen((char *)buffer);
        v_prg->file_len = n;
        v_prg->buffer = buffer;
        v_prg->lptr = buffer;
        pr = v.prg;
        v.prg = v_prg;
        read_line();
        v.prg = pr;
        v_prg->num_lines = 1;
        new_window(program0);
        // Add the template
        div_strcpy((char *)buffer, BUFFER_INCREASE, "PROGRAM yourprg;");
        read_line();
        f_enter();
        // Add the template
        div_strcat((char *)buffer, BUFFER_INCREASE, "BEGIN");
        // Your code here\n\n END");
        f_enter();


      } else {
        free(buffer);
        v_text = (char *)texts[45];
        show_dialog(err0);
      }
    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
    }
  }
}

//-----------------------------------------------------------------------------
//      Save a program to disk
//-----------------------------------------------------------------------------

void save_program(void) {
  int w = v.w / big2, h = v.h / big2;
  FILE *f;

  div_strcpy(full, sizeof(full), file_types[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);
  if ((f = fopen(full, "wb")) != NULL) {
    size_t written;
    write_line();
    written = fwrite(v.prg->buffer, 1, v.prg->file_len, f);
    fclose(f);
    if (written != (size_t)v.prg->file_len) {
      remove(full); // Delete partial file
      v_text = (char *)texts[47];
      show_dialog(err0);
      return;
    }

    div_strcpy(v.prg->path, sizeof(v.prg->path), file_types[v_type].path);
    div_strcpy(v.prg->filename, sizeof(v.prg->filename), input);

    wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
    } else {
      wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
    }
    flush_window(v_window);

    if (!strcmp(input, "help.div"))
      make_helpidx();
    if (!strcmp(input, "help.idx"))
      load_index();
  } else {
    v_text = (char *)texts[47];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
//      Determine if a string has been found
//-----------------------------------------------------------------------------

int string_found(char *p, char *q, int may_min, int completa) {
  if (completa && lower[*(q - 1)])
    return (0);
  if (may_min) {
    while (*p) {
      if (*p != *q)
        return (0);
      p++;
      q++;
    }
  } else {
    while (*p) {
      if (lower[*p]) {
        if (lower[*p] != lower[*q])
          return (0);
      } else {
        if (*p != *q)
          return (0);
      }
      p++;
      q++;
    }
  }
  if (completa && lower[*q])
    return (0);
  else
    return (1);
}

//-----------------------------------------------------------------------------
//      Find text
//-----------------------------------------------------------------------------

#define y_bt 50
char search_text[32] = {0};
int may_min = 0, completa = 0;

void find_text1(void);
void find_text2(void);

void find_text0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[160];
  v.w = 126;
  v.h = 14 + y_bt;
  v.paint_handler = find_text1;
  v.click_handler = find_text2;

  //  strcpy(search_text,"");

  _get(161, 4, 11, v.w - 8, (byte *)search_text, 32, 0, 0);
  _button(100, 7, y_bt, 0);
  _button(101, v.w - 8, y_bt, 2);
  _flag(163, 4, y_bt - 20, &completa);
  _flag(164, 4, y_bt - 12, &may_min);

  v_accept = 0;
}

void find_text1(void) {
  _show_items();
}

void find_text2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    end_dialog = 1;
    if (search_text[0])
      v_accept = 1;
    break;
  case 2:
    end_dialog = 1;
    break;
  }
}

void find_text(void) {
  struct tprg mi_prg;
  int encontrado = 0, n;

  if (!search_text[0])
    return;

  write_line();
  read_line();
  memcpy(&mi_prg, v.prg, sizeof(struct tprg));

  f_right();

  while ((v.prg->line < v.prg->num_lines || v.prg->column <= strlen(v.prg->l)) && !encontrado) {
    if (v.prg->column > strlen(v.prg->l)) {
      if (v.prg->line < v.prg->num_lines) {
        n = v.prg->line;
        write_line();
        advance_vptr();
        advance_lptr();
        read_line();
        f_home();
      }
    } else {
      if (string_found(search_text, &v.prg->l[v.prg->column - 1], may_min, completa))
        encontrado = 1;
      else
        f_right();
    }
  }

  if (!encontrado) { // Restore variables and show info dialog
    memcpy(v.prg, &mi_prg, sizeof(struct tprg));
    v_title = (char *)texts[347];
    v_text = (char *)texts[189];
    show_dialog(info0);
  } else {
    n = strlen(search_text);
    while (n--)
      f_right();
    n = strlen(search_text);
    while (n--)
      f_left();
  }

  v.redraw = 2;
  repaint_full();
  text_cursor();
}

//-----------------------------------------------------------------------------
//      Replace text
//-----------------------------------------------------------------------------

void replace0(void);
void replacements0(void);

#define y_st 69
char buscar2[32] = {0};
char sustituir[32] = {0};
int may_min2 = 0, completa2 = 0;
int num_cambios;

void replace_text1(void);
void replace_text2(void);

void replace_text0(void) {
  v.type = WIN_DIALOG;

  v.title = texts[165];
  v.w = 126;
  v.h = 14 + y_st;
  v.paint_handler = replace_text1;
  v.click_handler = replace_text2;

  //strcpy(buscar2,"");
  //strcpy(sustituir,"");

  _get(161, 4, 11, v.w - 8, (byte *)buscar2, 32, 0, 0);
  _get(162, 4, 30, v.w - 8, (byte *)sustituir, 32, 0, 0);
  _button(100, 7, y_st, 0);
  _button(101, v.w - 8, y_st, 2);
  _flag(163, 4, y_st - 20, &completa2);
  _flag(164, 4, y_st - 12, &may_min2);

  v_accept = 0;
}

void replace_text1(void) {
  _show_items();
}

void replace_text2(void) {
  _process_items();
  switch (v.active_item) {
  case 2:
    end_dialog = 1;
    if (buscar2[0])
      v_accept = 1;
    break;
  case 3:
    end_dialog = 1;
    break;
  }
}

void replace_text(void) {
  struct tprg mi_prg;
  int encontrado, n;

  if (!buscar2[0])
    return;

  write_line();
  read_line();

  v_accept = 0;
  num_cambios = 0;
  f_right();

  do {
    memcpy(&mi_prg, v.prg, sizeof(struct tprg));
    encontrado = 0;

    while ((v.prg->line < v.prg->num_lines || v.prg->column <= strlen(v.prg->l)) && !encontrado) {
      if (v.prg->column > strlen(v.prg->l)) {
        if (v.prg->line < v.prg->num_lines) {
          write_line();
          advance_vptr();
          advance_lptr();
          read_line();
          f_home();
        }
      } else {
        if (string_found(buscar2, &v.prg->l[v.prg->column - 1], may_min2, completa2))
          encontrado = 1;
        else
          f_right();
      }
    }

    if (encontrado) {
      block_state = 2;
      kprg = v.prg;
      block_start = v.prg->lptr;
      block_col1 = v.prg->column;
      block_end = v.prg->lptr;
      block_col2 = v.prg->column + strlen(buscar2) - 1;
      n = strlen(buscar2);
      while (n--)
        f_right();
      n = strlen(buscar2);
      while (n--)
        f_left();
      if (v_accept != 3) {
        v.redraw = 2;
        repaint_full();
        flush_window(0);
        show_dialog(replace0);
      }
      if (v_accept & 1) {
        num_cambios++;
        f_cut_block(1);
        if (clipboard != NULL)
          free(clipboard);
        clipboard = sustituir;
        clipboard_len = strlen(sustituir);
        clipboard_lines = 0;
        f_paste_block();
        f_unmark();
        clipboard = NULL;
        while (clipboard_len--)
          f_right();
      } else if (v_accept == 2) {
        f_right();
      } else if (v_accept == 4)
        encontrado = 0;
      block_state = 0;
    }
  } while (encontrado);

  if (v_accept != 4) {
    memcpy(v.prg, &mi_prg, sizeof(struct tprg));
  } // EOF
  v.redraw = 2;
  repaint_full();
  text_cursor();
  show_dialog(replacements0);
}

//-----------------------------------------------------------------------------
//      Replace or not?
//-----------------------------------------------------------------------------

void sustituir1(void) {
  _show_items();
}

void replace2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break; // YES
  case 1:
    v_accept = 2;
    end_dialog = 1;
    break; // NO
  case 2:
    v_accept = 3;
    end_dialog = 1;
    break; // ALL
  case 3:
    v_accept = 4;
    end_dialog = 1;
    break; // CANCEL
  }
}

void replace0(void) {
  int x2, x3, x4;
  x2 = 7 + text_len(texts[102] + 1) + 10;
  x3 = x2 + text_len(texts[103] + 1) + 10;
  x4 = x3 + text_len(texts[124] + 1) + 10;
  v.type = WIN_DIALOG;
  v.title = texts[190];
  v.w = x4 + text_len(texts[101] + 1) + 7;
  v.h = 29;
  v.paint_handler = sustituir1;
  v.click_handler = replace2;
  _button(102, 7, v.h - 14, 0);
  _button(103, x2, v.h - 14, 0);
  _button(124, x3, v.h - 14, 0);
  _button(101, x4, v.h - 14, 0);
  v_accept = 4;
}

//-----------------------------------------------------------------------------
//      Info dialog at the end of a replacement
//-----------------------------------------------------------------------------

char sus[128];

void replacements1(void) {
  _show_items();
  wwrite(v.ptr, v.w / big2, v.h / big2, (v.w / big2) / 2, 12, 1, (byte *)sus, c3);
}
void replacements2(void) {
  _process_items();
  if (!v.active_item)
    end_dialog = 1;
}

void replacements0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[191];
  itoa(num_cambios, sus, 10);
  div_strcat(sus, sizeof(sus), (char *)texts[192]);
  v.w = text_len(texts[191]) + 28;
  v.h = 38;
  v.paint_handler = replacements1;
  v.click_handler = replacements2;
  _button(100, v.w / 2, v.h - 14, 1);
}

void open_program_external(char *nombre, char *path) {
  char wpath[_MAX_PATH];
  struct tprg *pr;
  FILE *f;
  int n, x;
  byte *buffer, *p;

  div_strcpy(full, sizeof(full), path);
  div_strcpy(input, sizeof(input), nombre);
  div_strcpy(wpath, sizeof(wpath), path);
  wpath[strlen(wpath) - strlen(nombre)] = 0;
  if ((f = fopen(full, "rb")) != NULL) { // A file was selected
    fseek(f, 0, SEEK_END);
    n = ftell(f) + BUFFER_INCREASE;
    if ((buffer = (byte *)malloc(n)) != NULL) {
      if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
        v_prg->buffer_len = n;
        div_strcpy(v_prg->filename, sizeof(v_prg->filename), input);
        div_strcpy(v_prg->path, sizeof(v_prg->path), wpath); //<<<-----------
        fseek(f, 0, SEEK_SET);
        if (fread(buffer, 1, n, f) == n - BUFFER_INCREASE) {
          for (p = buffer; p < buffer + n - BUFFER_INCREASE; p++)
            if (!*p) {
              n = 0;
              break;
            }

          if (n) {
            n -= BUFFER_INCREASE;
            v_prg->file_len = n;
            v_prg->buffer = buffer;
            v_prg->num_lines = 1;

            x = 0;
            while (n--) {
              if (*buffer == cr || *buffer == lf) {
                if (*(buffer + 1) != ((*buffer) ^ (cr ^ lf))) {
                  memmove(buffer + 1, buffer, n + 1);
                  v_prg->file_len++;
                } else if (n)
                  n--;
                *buffer++ = cr;
                *buffer = lf;
                v_prg->num_lines++;
                x = -1;
              }

              if (*buffer == 9) {
                if (x >= 1020)
                  *buffer = ' ';
                else {
                  memmove(buffer + 3, buffer, n + 1);
                  v_prg->file_len += 3;
                  x += 3;
                  *buffer++ = ' ';
                  *buffer++ = ' ';
                  *buffer++ = ' ';
                  *buffer = ' ';
                }
              }

              buffer++;
              x++;

              if (x >= 1023) {
                memmove(buffer + 2, buffer, n + 1);
                *buffer++ = cr;
                *buffer++ = lf;
                v_prg->file_len += 2;
                v_prg->num_lines++;
                x = 0;
              }
            }

            v_prg->lptr = v_prg->buffer;
            pr = v.prg;
            v.prg = v_prg;
            read_line();
            v.prg = pr;

            new_window(program0);

          } else {
            free(v_prg);
            free(buffer);
            v_text = (char *)texts[46];
            show_dialog(err0);
          }
        } else {
          free(v_prg);
          free(buffer);
          v_text = (char *)texts[44];
          show_dialog(err0);
        }
      } else {
        free(buffer);
        v_text = (char *)texts[45];
        show_dialog(err0);
      }
    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
    }
    fclose(f);
  } else {
    v_text = (char *)texts[44];
    show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
// Process list dialog
//-----------------------------------------------------------------------------

int lp1[512];    // Line number where processes are defined
byte *lp2[512];  // Pointers to the process lines
int lp_num;      // Number of processes in the list
int lp_ini;      // First item displayed in the window
int lp_select;   // Selected item
int lp_sort = 0; // Flag indicating whether to sort the list

void create_process_list(char *buffer, int file_len) {
  byte *p, *end, *q;
  char cwork[512], cwork2[256];
  int line = 1, n, m;

  p = (byte *)buffer;
  end = p + file_len;
  lp_num = 0;
  lp_ini = 0;
  lp_select = 0;

  do {
    while (*p != lf && p < end) {
      p++;
    }
    p++;
    line++;
    if (p < end) {
      while (*p == ' ' && p < end)
        p++;
      if (p + 7 < end) {
        memcpy(cwork, p, 7);
        cwork[7] = 0;
        n = 6;
        do {
          cwork[n] = lower[cwork[n]];
        } while (n--);
        if ((lower[*(p + 7)] == ' ' || lower[*(p + 7)] == 0) && !strcmp(cwork, "process")) {
          p += 7;
          while (*p == ' ' && p < end)
            p++;
          if (p < end) {
            if (lp_sort) {
              q = p;
              n = 0;
              while (*q != cr && q < end) {
                cwork[n++] = *q;
                q++;
              }
              cwork[n] = 0;
              n = 0;
              while (n < lp_num) {
                if (strcmp((char *)cwork, (char *)lp2[n]) < 0)
                  break;
                n++;
              }
              if (n < lp_num) {
                memmove(&lp1[n + 1], &lp1[n], 4 * (511 - n));
                memmove(&lp2[n + 1], &lp2[n], 4 * (511 - n));
                lp1[n] = line;
                lp2[n] = p;
              } else {
                lp1[lp_num] = line;
                lp2[lp_num] = p;
              }
            } else {
              lp1[lp_num] = line;
              lp2[lp_num] = p;
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


  // If the cursor is on a process name in the program, select it

  if (buffer == (char *)window[1].prg->buffer) {
    n = window[1].prg->column - 1;
    p = (byte *)window[1].prg->l;
    if (n <= strlen((char *)p)) {
      if (!lower[p[n]] && n)
        n--;
      if (lower[p[n]]) {
        while (p)
          if (lower[p[n - 1]])
            n--;
          else
            break;
        cwork[0] = 0;
        while (lower[p[n]]) {
          cwork[strlen(cwork) + 1] = 0;
          cwork[strlen(cwork)] = lower[p[n++]];
        }
        for (n = 0; n < lp_num; n++) {
          p = (byte *)strchr((char *)lp2[n], ' ');
          if (p > (byte *)strchr((char *)lp2[n], '(') || p == NULL)
            p = (byte *)strchr((char *)lp2[n], '(');
          memcpy(cwork2, lp2[n], p - lp2[n]);
          cwork2[p - lp2[n]] = 0;
          for (m = strlen(cwork2) - 1; m >= 0; m--)
            cwork2[m] = lower[cwork2[m]];
          if (!strcmp(cwork, cwork2)) {
            lp_select = n;
            lp_ini = n;
            if (lp_ini + 15 > lp_num) {
              lp_ini = lp_num - 15;
              if (lp_ini < 0)
                lp_ini = 0;
            }
            break;
          }
        }
      }
    }
  }
}

void paint_process_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 129, n;
  float x;

  wbox(ptr, w, h, c2, 123 + 132, 28, 7, max - min + 3); // Clear the slider bar

  if (lp_num <= 1)
    n = min;
  else {
    x = (float)lp_select / (float)(lp_num - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122 + 132, n, 9, 5);
  wput(ptr, w, h, 123 + 132, n + 1, -43);
}

void paint_process_list(void) {
  byte *ptr = v.ptr, *p, *end;
  char cwork[512];
  int w = v.w / big2, h = v.h / big2;
  int n, m, x;

  wbox(ptr, w, h, c1, 4, 20, 128 + 132 - 10, 121); // Process listbox bounds

  end = window[1].prg->buffer + window[1].prg->file_len;

  for (m = lp_ini; m < lp_ini + 15 && m < lp_num; m++) {
    if (m == lp_select) {
      wbox(ptr, w, h, c01, 4, 20 + (m - lp_ini) * 8, 150 + 100, 9); // Process listbox fill
      x = c4;
    } else
      x = c3;
    p = lp2[m];
    n = 0;
    while (*p != cr && p < end) {
      cwork[n++] = *p;
      p++;
    }
    cwork[n] = 0;
    wwrite_in_box(ptr, w, 153 + 100, h, 5, 21 + (m - lp_ini) * 8, 0, (byte *)cwork, x);
  }
  paint_process_segment();
}

void process_list1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  _show_items();

  wwrite(ptr, w, h, 5, 11, 0, texts[379], c1);
  wwrite(ptr, w, h, 4, 11, 0, texts[379], c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128 + 132, 123); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122 + 132, 19, 9, 123);
  wrectangle(ptr, w, h, c0, 122 + 132, 27, 9, 123 - 16);

  wput(ptr, w, h, 123 + 132, 20, -39); // Up / down button (pressed 41,42)
  wput(ptr, w, h, 123 + 132, 174 - 40, -40);

  create_process_list((char *)window[1].prg->buffer, window[1].prg->file_len);
  paint_process_list();
}

int lp_boton;

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

  if (wmouse_in(3, 21, 128 + 132 - 9, 120) && (mouse_b & 1)) {
    n = lp_ini + (wmouse_y - 21) / 8;
    if (n < lp_num) {
      if (lp_select != n) {
        lp_select = n;
        paint_process_list();
        v.redraw = 1;
      } else if (!(prev_mouse_buttons & 1)) {
        v_accept = 1;
        end_dialog = 1;
      }
    }
  }

  if (wmouse_in(122 + 132, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, 123 + 132, 20, -41);
        lp_boton = 1;
        if (lp_select) {
          if (lp_ini == lp_select--)
            lp_ini--;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 1) {
      wput(ptr, w, h, 123 + 132, 20, -39);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp_boton == 1) {
    wput(ptr, w, h, 123 + 132, 20, -39);
    lp_boton = 0;
    v.redraw = 1;
  }


  if (wmouse_in(123 + 132, 28, 7, 105)) {
    mouse_graf = 13;
    if (lp_num > 1 && (mouse_b & 1)) {
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

  if (wmouse_in(122 + 132, 93 + 40, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, 123 + 132, 94 + 40, -42);
        lp_boton = 2;
        if (lp_select + 1 < lp_num) {
          if (lp_ini + 15 == ++lp_select)
            lp_ini++;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 2) {
      wput(ptr, w, h, 123 + 132, 94 + 40, -40);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp_boton == 2) {
    wput(ptr, w, h, 123 + 132, 94 + 40, -40);
    lp_boton = 0;
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
    create_process_list((char *)window[1].prg->buffer, window[1].prg->file_len);
    paint_process_list();
    v.redraw = 1;
    break;
  }
}

void process_list0(void) {
  v.type = WIN_DIALOG;
  v.title = texts[380];

  v.w = 166 + 100;
  v.h = 161;
  v.paint_handler = process_list1;
  v.click_handler = process_list2;

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _flag(337, v.w - text_len(texts[337]) - 12, 11, &lp_sort);
  v_accept = 0;
}

//-----------------------------------------------------------------------------
//  Jump to the line and column where the error was detected
//-----------------------------------------------------------------------------

void goto_error(void) {
  int m, n = 0;

  for (m = 0; m < MAX_WINDOWS; m++) {
    if (window[m].type == WIN_CODE && window[m].state && window[m].prg != NULL) {
      n = m;
      break;
    }
  }

  if (n)
    move(0, n); // If not window[0], bring to foreground

  if (v.foreground != WF_FOREGROUND) { // If not in foreground, bring it there
    for (m = 1; m < MAX_WINDOWS; m++)
      if (window[m].type && window[m].foreground == WF_FOREGROUND)
        if (windows_collide(0, m)) {
          window[m].foreground = WF_BACKGROUND;
          flush_window(m);
        }
  }

  if (error_number < 10) {
    error_line = 1;
    error_col = 1;
  }

  if (error_line > v.prg->num_lines) {
    error_line = v.prg->num_lines;
    m = 1;
  } else
    m = 0;
  if (error_line < 1)
    error_line = 1;

  f_home();

  if (v.prg->line > error_line) {
    while (v.prg->line > error_line) {
      write_line();
      retreat_lptr();
      read_line();
      retreat_vptr();
    }
  } else if (v.prg->line < error_line) {
    while (v.prg->line < error_line) {
      write_line();
      advance_lptr();
      read_line();
      advance_vptr();
    }
  }

  if (v.prg->line <= v.prg->first_line)
    retreat_vptr();
  if (v.prg->line - v.prg->first_line >= v.prg->h - 1)
    advance_vptr();

  if (m)
    f_end();
  else {
    v.prg->column = error_col;
    if (v.prg->column - v.prg->first_column >= v.prg->w - 1) {
      v.prg->first_column = v.prg->column - v.prg->w + 2;
    }
    if (v.prg->column < v.prg->first_column) {
      v.prg->first_column = v.prg->column;
      if (v.prg->first_column > 1)
        v.prg->first_column--;
    }
  }

  eprg = v.prg;
  ascii = 0;
  scan_code = 0;

  v.redraw = 2;
  repaint_full();
  text_cursor();
}

//-----------------------------------------------------------------------------
// Program listing print option
//-----------------------------------------------------------------------------

int fp_co = 1, fp_bl = 0;

void printprogram1(void) {
  _show_items();
}

void printprogram2(void) {
  _process_items();
  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    fp_co = 1;
    fp_bl = 0;
    _show_items();
    break;
  case 3:
    fp_co = 0;
    fp_bl = 1;
    _show_items();
    break;
  }
}

void printprogram0(void) {
  v.type = WIN_DIALOG;
  v.w = 120;
  v.h = 38 + 10;
  v.title = texts[453];
  v.paint_handler = printprogram1;
  v.click_handler = printprogram2;
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);
  _flag(454, 4, 12, &fp_co);
  _flag(455, 4, 12 + 9, &fp_bl);
  v_accept = 0;
}

void print_program(void) {
  byte *buf;
  int lon, n;

  show_dialog(printprogram0);

  if (v_accept) {
    xchg(v_window, 0);
    write_line();
    read_line();

    if (fp_bl) { // Print the selected block

      if (!block_state) {
        v_text = (char *)texts[452];
        show_dialog(err0);
        return;
      }

      f_cut_block(0);
      v.redraw = 2;

      buf = (byte *)clipboard;
      lon = clipboard_len;

    } else { // Print the complete listing

      buf = v.prg->buffer;
      lon = v.prg->file_len;
    }

    xchg(v_window, 0);

    if (lon > 0) {
      for (n = 0; n < lon; n += 32) {
        show_progress((char *)texts[437], n, lon);
        if (n + 32 <= lon)
          fwrite(buf + n, 1, 32, stdprn);
        else
          fwrite(buf + n, 1, lon - n, stdprn);
      }

      fwrite("\xd\xa\f", 1, 3, stdprn);
      show_progress((char *)texts[437], lon, lon);
    }
  }
}
