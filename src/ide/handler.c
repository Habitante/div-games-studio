//-----------------------------------------------------------------------------
//      Menu infrastructure, top-level menus, and window finders
//-----------------------------------------------------------------------------

#include "handler_internal.h"

//-----------------------------------------------------------------------------
//      Module variables
//-----------------------------------------------------------------------------

byte aux_palette[768];
int new_status;

void repaint_full(void);
void text_cursor(void);
void repaint_window(void);

extern int error_number;
extern void browser0(void);
extern void browser1(void);
extern void browser2(void);
extern void browser3(void);

void delete_file(char *name);
void about0(void);
void save_sound(pcminfo *mypcminfo, char *dst);
void paste_new_sounds(void);

//-----------------------------------------------------------------------------
//      Minimum handler function of type (????)
//-----------------------------------------------------------------------------

void dummy_handler(void) {}

//-----------------------------------------------------------------------------
//      Main Menu
//-----------------------------------------------------------------------------

void menu_main0(void) {
  create_menu(MENU_MAIN);
  v.paint_handler = menu_main1;
  v.click_handler = menu_main2;
  v.close_handler = menu_main3;
}

void menu_main1(void) {
  paint_menu(MENU_MAIN);
}

/* Click handler for the top menu bar.
 * Dispatches mouse clicks to the appropriate submenu (Programs, Palettes,
 * Maps, Graphics, Fonts, Sounds, System, Help) based on which item is hit.
 */
void menu_main2(void) {
  update_menu(MENU_MAIN, 1, 0);
  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    switch (v.state) {
    case 1:
      new_window(menu_programs0);
      break;

    case 2:
      new_window(menu_palettes0);
      break;

    case 3:
      new_window(menu_maps0);
      break;

    case 4:
      new_window(menu_graphics0);
      break;

    case 5:
      new_window(menu_fonts0);
      break;

    case 6:
      new_window(menu_sounds0);
      break;

    case 7:
      new_window(menu_system0);
      break;

    case 8:
      help(3);
      break;
    }
  }
}

void menu_main3(void) {
  exit_requested = 1;
}

//-----------------------------------------------------------------------------
//      Programs Menu
//-----------------------------------------------------------------------------

void print_program(void);

void menu_programs0(void) {
  create_menu(MENU_PROGRAMS);
  v.paint_handler = menu_programs1;
  v.click_handler = menu_programs2;
}

void menu_programs1(void) {
  paint_menu(MENU_PROGRAMS);
}

void menu_programs2(void) {
  FILE *f;
  int n;

  if (find_program_window())
    update_menu(MENU_PROGRAMS, 1, 0);
  else
    update_menu(MENU_PROGRAMS, 3, 99);

  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    v_type = FT_PRG;

    switch (v.state) {
    case 1: // New ...
      v_mode = 1;
      v_text = (char *)texts[186];
      show_dialog(browser0);
      if (v_finished) {
        if (!v_exists)
          v_accept = 1;
        else {
          v_title = (char *)texts[187];
          v_text = input;
          show_dialog(accept0);
        }
        if (v_accept) {
          program0_new();
          save_program();
        }
      }
      break;

    case 2: // Open program ...
      v_mode = 0;
      v_text = (char *)texts[346];
      show_dialog(browser0);
      if (v_finished) {
        if (!v_exists) {
          v_text = (char *)texts[43];
          show_dialog(err0);
        } else {
          mouse_graf = CURSOR_BUSY;
          flush_copy();
          mouse_graf = CURSOR_ARROW;
          open_program();
        }
      }
      break;

    case 3:
      if ((n = find_program_window())) {
        v_title = (char *)texts[188];
        v_text = (char *)window[n].title;
        show_dialog(accept0);
        if (v_accept) {
          move(0, n);
          close_window();
        }
      }
      break;
      break;

    case 4:
      if ((n = find_program_window())) {
        flush_window(0);
        save_prg_buffer(n);
        mouse_graf = CURSOR_BUSY;
        flush_copy();
        mouse_graf = CURSOR_ARROW;
        wup(n);
        div_strcpy(file_types[v_type].path, sizeof(file_types[v_type].path), v.prg->path);
        div_strcpy(input, sizeof(input), v.prg->filename);
        save_program();
        wdown(n);
        v.redraw = 1;
      }
      break;

    case 5:
      if ((n = find_program_window())) {
        save_prg_buffer(n);
        v_mode = 1;
        v_text = (char *)texts[906];
        show_dialog(browser0);
        if (v_finished) {
          if (v_exists) {
            v_title = (char *)texts[187];
            v_text = input;
            show_dialog(accept0);
          } else
            v_accept = 1;
          if (v_accept) {
            mouse_graf = CURSOR_BUSY;
            flush_copy();
            mouse_graf = CURSOR_ARROW;
            wup(n);
            save_program();
            wdown(n);
            v.redraw = 1;
          }
        }
      }
      break;

    case 6:
      new_window(menu_edit0);
      break;

    case 8:
      if ((n = find_program_window())) {
        save_prg_buffer(n);
        source_ptr = window[n].prg->buffer;
        source_len = window[n].prg->file_len;
        v_window = n;
        run_mode = 0;
        div_strcpy(file_types[FT_PRG].path, sizeof(file_types[FT_PRG].path), window[n].prg->path);
        compile_program();
        if (error_number != -1) {
          goto_error();
          if (v_help)
            help(500 + error_number);
        } else if (v_help)
          help(599);
      }
      break;

    case 7:
    case 9:
      if ((n = find_program_window())) {
        save_prg_buffer(n);
        mouse_graf = CURSOR_BUSY;
        flush_copy();
        mouse_graf = CURSOR_ARROW;
        wup(n);
        div_strcpy(file_types[v_type].path, sizeof(file_types[v_type].path), v.prg->path);
        div_strcpy(input, sizeof(input), v.prg->filename);
        save_program();
        wdown(n);
        source_ptr = window[n].prg->buffer;
        source_len = window[n].prg->file_len;
        v_window = n;
        if (v.state == 7)
          run_mode = 1;
        else
          run_mode = 3;
        div_strcpy(file_types[FT_PRG].path, sizeof(file_types[FT_PRG].path), window[n].prg->path);
        compile_program();
        if (error_number != -1) {
          goto_error();
          if (v_help)
            help(500 + error_number);
          break;
        }
        return_mode = 1;
        exit_requested = 1;
      }
      break;

    case 10:
      if ((n = find_program_window())) {
        save_prg_buffer(n);
        source_ptr = window[n].prg->buffer;
        source_len = window[n].prg->file_len;
        v_window = n;
        run_mode = 2;
        delete_file("system/exec.ins");
        div_strcpy(file_types[FT_PRG].path, sizeof(file_types[FT_PRG].path), window[n].prg->path);
        compile_program();
        if (error_number != -1) {
          goto_error();
          if (v_help)
            help(500 + error_number);
        } else {
          f = fopen("system/exec.ins", "rb");
          if (f != NULL) {
            fclose(f);
            create_installation();
            delete_file("install.div");
          } else {
            v_text = (char *)texts[238];
            show_dialog(err0);
          }
        }
      }
      break;

    case 11:
      if ((n = find_program_window())) {
        v_window = n;
        print_program();
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Edit Menu
//-----------------------------------------------------------------------------

void repaint_full(void);
void f_delete(void);
void f_cut_block(memptrsize);
void f_paste_block(void);
void f_mark(void);
void f_unmark(void);

void menu_edit0(void) {
  create_menu(MENU_EDIT);
  v.paint_handler = menu_edit1;
  v.click_handler = menu_edit2;
}

void menu_edit1(void) {
  paint_menu(MENU_EDIT);
}

void menu_edit2(void) {
  int n, m;

  if (find_program_window())
    update_menu(MENU_EDIT, 1, 0);
  else
    update_menu(MENU_EDIT, 0, 99);

  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    v_type = FT_PRG;

    switch (v.state) {
    case 1: // Delete line
      if ((n = find_program_window())) {
        wup(n);
        f_delete();
        repaint_full();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 2: // Mark
      if ((n = find_program_window())) {
        wup(n);
        f_mark();
        repaint_full();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 3: // Unmark
      if ((n = find_program_window())) {
        wup(n);
        f_unmark();
        repaint_full();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 4: // Cut
      if ((n = find_program_window())) {
        wup(n);
        f_cut_block(1);
        repaint_full();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 5: // Copy
      if ((n = find_program_window())) {
        wup(n);
        f_cut_block(0);
        repaint_full();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 6: // Paste
      if ((n = find_program_window())) {
        wup(n);
        f_paste_block();
        f_unmark();
        repaint_full();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 7:
      if ((n = find_program_window())) {
        wmouse_x = -1;
        wmouse_y = -1;
        mouse_b = 0;
        call(v.click_handler);
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        move(0, n);
        if (v.foreground == WF_BACKGROUND) {
          for (m = 1; m < MAX_WINDOWS; m++)
            if (window[m].type && window[m].foreground == WF_FOREGROUND)
              if (windows_collide(0, m)) {
                window[m].foreground = WF_BACKGROUND;
                flush_window(m);
              }
          v.foreground = WF_FOREGROUND;
        }
        flush_window(0);
        write_line();
        read_line();
        show_dialog(process_list0);
        scan_code = 0;
        ascii = 0;
        if (v_accept) {
          f_bop();
          f_home();
          while (v.prg->line > lp1[lp_select]) {
            write_line();
            retreat_lptr();
            read_line();
            retreat_vptr();
          }
          while (v.prg->line < lp1[lp_select]) {
            write_line();
            advance_lptr();
            read_line();
            advance_vptr();
          }
          v.redraw = 2;
          repaint_full();
          text_cursor();
        }
      }
      break;

    case 8:
      if ((n = find_program_window())) {
        show_dialog(find_text0);
        if (v_accept) {
          move(0, n);
          if (v.foreground == WF_BACKGROUND) {
            for (m = 1; m < MAX_WINDOWS; m++)
              if (window[m].type && window[m].foreground == WF_FOREGROUND)
                if (windows_collide(0, m)) {
                  window[m].foreground = WF_BACKGROUND;
                  flush_window(m);
                }
            v.foreground = WF_FOREGROUND;
          }
          find_text();
        }
      }
      break;

    case 9:
      if ((n = find_program_window())) {
        wmouse_x = -1;
        wmouse_y = -1;
        mouse_b = 0;
        call((void_return_type_t)v.click_handler);
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        move(0, n);
        if (v.foreground == WF_BACKGROUND) {
          for (m = 1; m < MAX_WINDOWS; m++)
            if (window[m].type && window[m].foreground == WF_FOREGROUND)
              if (windows_collide(0, m)) {
                window[m].foreground = WF_BACKGROUND;
                flush_window(m);
              }
          v.foreground = WF_FOREGROUND;
          flush_window(0);
        }
        find_text();
      }
      break;

    case 10:
      if ((n = find_program_window())) {
        show_dialog(replace_text0);
        if (v_accept) {
          move(0, n);
          if (v.foreground == WF_BACKGROUND) {
            for (m = 1; m < MAX_WINDOWS; m++)
              if (window[m].type && window[m].foreground == WF_FOREGROUND)
                if (windows_collide(0, m)) {
                  window[m].foreground = WF_BACKGROUND;
                  flush_window(m);
                }
            v.foreground = WF_FOREGROUND;
          }
          replace_text();
        }
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Programs Menu Functions
//-----------------------------------------------------------------------------

void write_line(void);
void read_line(void);

void save_prg_buffer(memptrsize n) { // Update the program, save the current line
  wup(n);
  write_line();
  read_line();
  wdown(n);
}

//-----------------------------------------------------------------------------
//      Palettes Menu
//-----------------------------------------------------------------------------

void menu_palettes0(void) {
  create_menu(MENU_PALETTES);
  v.paint_handler = menu_palettes1;
  v.click_handler = menu_palettes2;
}

void menu_palettes1(void) {
  paint_menu(MENU_PALETTES);
}

void menu_palettes2(void) {
  update_menu(MENU_PALETTES, 1, 0);
  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    switch (v.state) {
    case 1:
      pal_load();
      break;
    case 2:
      pal_save_as();
      break;
    case 3:
      pal_edit();
      break;
    case 4:
      sort_palette();
      break;
    case 5:
      merge_palette();
      break;
    case 6:
      new_window(palette0);
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Maps Menu
//-----------------------------------------------------------------------------

extern char input2[32];

void browser0(void);

void menu_maps0(void) {
  create_menu(MENU_MAPS);
  v.paint_handler = menu_maps1;
  v.click_handler = menu_maps2;
}

void menu_maps1(void) {
  paint_menu(MENU_MAPS);
}

void menu_maps2(void) {
  int n, m;

  if (find_map_window())
    update_menu(MENU_MAPS, 1, 0);
  else
    update_menu(MENU_MAPS, 3, 9);

  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    v_type = FT_MAP;

    switch (v.state) {
    case 1: // New map ...
      show_dialog(new_map0);
      if (v_finished) {
        mouse_graf = CURSOR_BUSY;
        flush_copy();
        mouse_graf = CURSOR_ARROW;
        new_map(NULL);
      }
      break;

    case 2: // Open map ...
      v_mode = 0;
      v_text = (char *)texts[803];
      show_dialog(browser0);
      if (v_finished) {
        if (!v_exists) {
          v_text = (char *)texts[43];
          show_dialog(err0);
        } else {
          mouse_graf = CURSOR_BUSY;
          flush_copy();
          mouse_graf = CURSOR_ARROW;
          open_map();
        }
      }
      break;

    case 3: // Close map
      if ((n = find_and_load_map())) {
        v_title = (char *)texts[50];
        v_text = (char *)window[n].title;
        show_dialog(accept0);
        if (v_accept) {
          move(0, n);
          close_window();
        }
      }
      break;

    case 4: // Close all maps (maximized)
      v_title = (char *)texts[334];
      v_text = (char *)texts[335];
      show_dialog(accept0);
      if (v_accept) {
        for (n = 0; n < MAX_WINDOWS; n++)
          if (window[n].type == WIN_MAP && window[n].foreground != WF_MINIMIZED) {
            move(0, n);
            close_window();
            n = -1;
          }
      }
      break;

    case 5: // Save map
      if ((n = find_and_load_map())) {
        if (strchr((const char *)window[n].title, ' ') != NULL)
          goto no_tiene_nombre;
        if (window[n].mapa->path[0] == 0) {
          v_mode = 2;
          div_strcpy(input2, sizeof(input2), (char *)window[n].title);
          goto casi_no_tiene_nombre;
        }
        mouse_graf = CURSOR_BUSY;
        flush_copy();
        mouse_graf = CURSOR_ARROW;
        div_strcpy(file_types[v_type].path, sizeof(file_types[v_type].path), window[n].mapa->path);
        div_strcpy(input, sizeof(input), window[n].mapa->filename);
        save_map();
      }
      break;

    case 6: // Save map as ...
no_tiene_nombre:
      if (find_and_load_map()) {
        v_mode = 1;
casi_no_tiene_nombre:
        v_text = (char *)texts[807];
        show_dialog(browser0);
        if (v_finished) {
          if (v_exists) {
            v_title = (char *)texts[52];
            v_text = input;
            show_dialog(accept0);
          } else
            v_accept = 1;
          if (v_accept) {
            mouse_graf = CURSOR_BUSY;
            flush_copy();
            mouse_graf = CURSOR_ARROW;
            save_map();
          }
        }
      }
      break;

    case 7: // Rescale the map
      reduce_half();
      break;

    case 8: // Create search map
      map_search();
      break;

    case 9: // Edit the map
      if ((n = find_and_load_map())) {
        paint_menu(MENU_MAPS);
        v.state = 0;
        move(0, n);
        if (v.foreground == WF_BACKGROUND) {
          for (m = 1; m < MAX_WINDOWS; m++)
            if (window[m].type && window[m].foreground == WF_FOREGROUND)
              if (windows_collide(0, m)) {
                window[m].foreground = WF_BACKGROUND;
                flush_window(m);
              }
          v.foreground = WF_FOREGROUND;
        }
        mouse_b = 1;
        call((void_return_type_t)v.click_handler);
      }
      break;

    case 10:
      gen_explodes();
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      3D Maps Menu
//-----------------------------------------------------------------------------

void update_title(void) {
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
  if (text_len(v.title) + 3 > w - 20) {
    wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
    wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
  }
}

// menu_mapas3D functions removed (MODE8/3D map editor deleted)

int check_file(void) {
  char cwork[8];
  FILE *f;

  v_mode = 0;
  v_type = FT_FPG;
  v_text = (char *)texts[70];
  show_dialog(browser0);

  div_strcpy(full, sizeof(full), file_types[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  if (v_finished) {
    if ((f = fopen(full, "rb")) != NULL) { // A file was selected
      if (fread(cwork, 1, 8, f) == 8) {
        fclose(f);
        if (!strcmp(cwork, "fpg\x1a\x0d\x0a")) {
          if (!v_exists) {
            v_text = (char *)texts[43];
            show_dialog(err0);
          } else {
            return (1);
          }
        } else {
          v_text = (char *)texts[46];
          show_dialog(err0);
        }
      } else {
        fclose(f);
        v_text = (char *)texts[44];
        show_dialog(err0);
      }
    } else {
      v_text = (char *)texts[44];
      show_dialog(err0);
    }
  }

  return (0);
}

//-----------------------------------------------------------------------------
//      Window finder functions
//-----------------------------------------------------------------------------

int find_pcm_window(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_SOUND && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_mod_window(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_MUSIC && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_font_window(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_FONT && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_fpg_window(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_FPG && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

// determina_prj removed (MODE8/3D map editor deleted)

int find_map_window(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_MAP && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_and_load_map(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_MAP && window[m].state) {
      n = m;
      break;
    }
  if (n) {
    v_window = n;
    v_map = window[n].mapa;
    map = v_map->map;
    map_width = v_map->map_width;
    map_height = v_map->map_height;
  }
  return (n);
}

// determina_map_view3d removed (MODE8/3D map editor deleted)

int find_program_window(void) {
  int m, n = 0;

  for (m = 1; m < MAX_WINDOWS; m++) {
    if (window[m].type == WIN_CODE && window[m].state && window[m].prg != NULL) {
      n = m;
      break;
    }
  }
  return (v_window = n);
}

//-----------------------------------------------------------------------------
//      Graphics Menu (FPG Files)
//-----------------------------------------------------------------------------

void print_list(void);
void fpg_to_map(FPG *MiFPG);
void map_to_fpg(struct tmapa *mapa);

void menu_graphics0(void) {
  create_menu(MENU_GRAPHICS);
  v.paint_handler = menu_graphics1;
  v.click_handler = menu_graphics2;
}

void menu_graphics1(void) {
  paint_menu(MENU_GRAPHICS);
}

void menu_graphics2(void) {
  FPG *fpg_file;
  int n;

  if (find_fpg_window() && find_map_window())
    update_menu(MENU_GRAPHICS, 1, 0);
  else if (find_fpg_window())
    update_menu(MENU_GRAPHICS, 9, 9);
  else if (find_map_window())
    update_menu(MENU_GRAPHICS, 3, 8);
  else
    update_menu(MENU_GRAPHICS, 3, 9);

  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    v_type = FT_FPG;

    switch (v.state) {
    case 1: // New file
      new_file();
      break;

    case 2: // Open file
      open_file();
      break;

    case 3: // Close file
      if ((n = find_fpg_window())) {
        v_title = (char *)texts[81];
        v_text = (char *)window[n].title;
        show_dialog(accept0);
        if (v_accept) {
          free(window[n].aux);
          move(0, n);
          close_window();
        }
      }
      break;

    case 4: // Save file as...
      if ((n = find_fpg_window())) {
        v_mode = 1;
        v_text = (char *)texts[830];
        show_dialog(browser0);
        if (v_finished) {
          if (v_exists) {
            // Check that the file is not the same
            fpg_file = (FPG *)window[n].aux;
            div_strcpy(full, sizeof(full), file_types[FT_FPG].path);
            if (full[strlen(full) - 1] != '/')
              div_strcat(full, sizeof(full), "/");
            div_strcat(full, sizeof(full), input);

            if (!strcmp(full, (char *)fpg_file->current_file)) {
              v_text = (char *)texts[418];
              show_dialog(err0);
              v_accept = 0;
            } else {
              v_title = (char *)texts[82];
              v_text = input;
              show_dialog(accept0);
            }

          } else {
            v_accept = 1;
          }
          if (v_accept) {
            mouse_graf = CURSOR_BUSY;
            flush_copy();
            mouse_graf = CURSOR_ARROW;
            fpg_save(n);
          }
        }
      }
      break;

    case 5:
      show_tagged();
      break;

    case 6:
      delete_tagged();
      break;

    case 7:
      print_list();
      break;

    case 8:
      if ((n = find_fpg_window())) {
        fpg_to_map((FPG *)(window[n].aux));
      }
      break;

    case 9:
      if ((n = find_map_window())) {
        map_to_fpg(window[n].mapa);
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Fonts Menu
//-----------------------------------------------------------------------------

void menu_fonts0(void) {
  create_menu(MENU_FONTS);
  v.paint_handler = menu_fonts1;
  v.click_handler = menu_fonts2;
}

void menu_fonts1(void) {
  paint_menu(MENU_FONTS);
}

extern char *font_aux;
void get_text0(void);
void create_text(void);
void generate_fontmap(void);

void menu_fonts2(void) {
  int n;

  if (find_font_window()) {
    if (find_map_window())
      update_menu(MENU_FONTS, 1, 0);
    else
      update_menu(MENU_FONTS, 6, 6);
  } else {
    if (find_map_window())
      update_menu(MENU_FONTS, 3, 5);
    else
      update_menu(MENU_FONTS, 3, 6);
  }

  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    switch (v.state) {
    case 1:
      open_gen_font();
      break;

    case 2:
      open_font();
      break;

    case 3:
      if ((n = find_font_window())) {
        move(0, n);
        close_window();
      }
      break;

    case 4:
      if ((n = find_font_window())) {
        font_aux = (char *)(window[n].aux + RES_FOR_NAME);
        show_dialog(get_text0);
        if (v_accept)
          create_text();
      }
      break;

    case 5:
      if ((n = find_font_window())) {
        font_aux = (char *)(window[n].aux + RES_FOR_NAME);
        generate_fontmap();
      }
      break;

    case 6:
      if ((n = find_and_load_map()))
        print_fontmap();
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Sound Menu
//-----------------------------------------------------------------------------

void open_sound(void);
void open_song(void);
void open_sound_file(void);

void menu_sounds0(void) {
  create_menu(MENU_SOUNDS);
  v.paint_handler = menu_sounds1;
  v.click_handler = menu_sounds2;
}

void menu_sounds1(void) {
  paint_menu(MENU_SOUNDS);
}

void mixer0(void);
void rec_sound0(void);
void edit_sound(void);

void menu_sounds2(void) {
  pcminfo *mypcminfo;
  int n;

  if (find_pcm_window())
    update_menu(MENU_SOUNDS, 1, 0);
  else
    update_menu(MENU_SOUNDS, 3, 6);

  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    switch (v.state) {
    case 1:

      if (!sound_active) {
        if (sound_error) {
          v_text = (char *)texts[549];
          show_dialog(errhlp0);
          if (v_accept)
            help(2008);
        } else {
          v_text = (char *)texts[548];
          show_dialog(errhlp0);
          if (v_accept)
            help(2009);
        }
        return;
      } else if (!sound_active) {
        v_text = (char *)texts[575];
        show_dialog(err0);
        return;
      }

      show_dialog(rec_sound0);
      break;

    case 2:
      open_sound();
      break;

    case 3:
      if ((n = find_pcm_window())) {
        v_title = (char *)texts[338];
        v_text = (char *)window[n].title;
        show_dialog(accept0);
        if (v_accept) {
          move(0, n);
          close_window();
        }
      }
      break;

    case 4:
      if ((n = find_pcm_window())) {
        if (strchr((const char *)window[n].title, ' ') != NULL)
          goto unnamed_sound;
        mouse_graf = CURSOR_BUSY;
        flush_copy();
        mouse_graf = CURSOR_ARROW;
        mypcminfo = (pcminfo *)window[n].aux;
        div_strcpy(full, sizeof(full), mypcminfo->pathname);
        div_strcpy(input, sizeof(input), mypcminfo->name);
        save_sound(mypcminfo, full);
        wup(n);
        repaint_window();
        wdown(n);
        flush_window(n);
      }
      break;

    case 5:
unnamed_sound:
      if ((n = find_pcm_window())) {
        v_type = FT_AUDIO;
        v_mode = 1;
        v_text = (char *)texts[339];
        show_dialog(browser0);
        if (v_finished) {
          if (v_exists) {
            v_title = (char *)texts[340];
            v_text = input;
            show_dialog(accept0);
          } else
            v_accept = 1;
          if (v_accept) {
            mouse_graf = CURSOR_BUSY;
            flush_copy();
            mouse_graf = CURSOR_ARROW;
            mypcminfo = (pcminfo *)window[n].aux;
            div_strcpy(full, sizeof(full), file_types[v_type].path);
            if (full[strlen(full) - 1] != '/')
              div_strcat(full, sizeof(full), "/");
            div_strcat(full, sizeof(full), input);

            if (strchr(input, ' ') == NULL) {
              save_sound(mypcminfo, full);
              div_strcpy(mypcminfo->pathname, sizeof(mypcminfo->pathname), full);
              div_strcpy(mypcminfo->name, sizeof(mypcminfo->name), input);
              move(0, n);
              close_window();
              open_sound_file();
            } else {
              v_text = (char *)texts[47];
              show_dialog(err0);
            }
          }
        }
      }
      break;

    case 6:
      if ((n = find_pcm_window())) {
        pcminfo_aux = window[n].aux;
        v_type = FT_AUDIO;
        v_text = (char *)texts[349];

        //          judas_stopsample(0);

        show_dialog(edit_sound0);
        move(0, n);
        call((void_return_type_t)v.paint_handler);
        v.redraw = 1;
        paste_new_sounds();
      }
      break;

    case 7:
      new_window(mixer0);
      break;

    case 8:
      if (!sound_active) {
        if (sound_error) {
          v_text = (char *)texts[549];
          show_dialog(errhlp0);
          if (v_accept)
            help(2008);
        } else {
          v_text = (char *)texts[548];
          show_dialog(errhlp0);
          if (v_accept)
            help(2009);
        }
        return;
      }

      open_song();
      break;

    case 9:
      if ((n = find_mod_window())) {
        v_title = (char *)texts[513];
        v_text = (char *)window[n].title;
        show_dialog(accept0);
        if (v_accept) {
          move(0, n);
          close_window();
        }
      }
      break;
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      System Menu
//-----------------------------------------------------------------------------

void mem_info0(void);

void menu_system0(void) {
  create_menu(MENU_SYSTEM);
  v.paint_handler = menu_system1;
  v.click_handler = menu_system2;
}

void menu_system1(void) {
  paint_menu(MENU_SYSTEM);
}

void config_setup0(void);
void config_setup_end(void);
void shell(void);
void calculator(void);

void menu_system2(void) {
  update_menu(MENU_SYSTEM, 1, 0);
  if ((prev_mouse_buttons & MB_LEFT) && !(mouse_b & MB_LEFT)) {
    switch (v.state) {
    case 1:
      show_clock();
      break;
    case 2:
      show_trash();
      break;
    case 3:
      calculator();
      break;
    case 4:
      show_dialog(vid_setup0);
      break;
    case 5:
      show_dialog(wallpaper_setup0);
      break;
    case 6:
      show_dialog(config_setup0);
      config_setup_end();
      break;
    case 7:
      show_dialog(mem_info0);
      break;
    case 8:
      show_dialog(about0);
      break;
    case 9:
      v_title = (char *)texts[40];
      v_text = NULL;
      show_dialog(accept0);
      if (v_accept) {
        exit_requested = 1;
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Create a new menu
//-----------------------------------------------------------------------------

/* Build a dropdown menu window from consecutive texts[] entries.
 * The first entry is the icon label, the second is the title; subsequent
 * non-NULL entries become menu items. Calculates window dimensions.
 */
void create_menu(int menu) {
  int w;

  v.type = WIN_MENU; // Menu
  v.name = texts[menu++];
  v.title = texts[menu];
  v.state = 0;

  v.w = text_len(texts[menu++]) + MENU_WIDTH_PAD;
  v.h = MENU_HEADER_HEIGHT;

  while ((char *)texts[menu]) {
    v.h += MENU_ITEM_HEIGHT;
    w = text_len(texts[menu++]) + MENU_TEXT_MARGIN;
    if (v.w < w)
      v.w = w;
  }
}

//-----------------------------------------------------------------------------
//      Draw the active menu
//-----------------------------------------------------------------------------

void paint_menu(int menu) {
  byte *ptr = v.ptr, *p, *q;
  int w = v.w / big2, h = v.h / big2, n = 0;

  menu++;

  wbox(ptr, w, h, c2, 2, 10, w - 4, h - 12);

  while (texts[++menu]) {
    if (*(p = texts[menu]) == '-') {
      p++;
      wbox(ptr, w, h, c0, 2, MENU_ITEM_HEIGHT + n * MENU_ITEM_HEIGHT, w - 4, 1);
    }

    if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
      *q = 0;
      wwrite(ptr, w, h, 3, MENU_HEADER_HEIGHT + n * MENU_ITEM_HEIGHT, 0, p, c3);
      *q = '[';
      wwrite(ptr, w, h, w - 4, MENU_HEADER_HEIGHT + n * MENU_ITEM_HEIGHT, 2, q, c1);
      wwrite(ptr, w, h, w - 5, MENU_HEADER_HEIGHT + n * MENU_ITEM_HEIGHT, 2, q, c3);
      n++;
    } else
      wwrite(ptr, w, h, 3, MENU_HEADER_HEIGHT + n++ * MENU_ITEM_HEIGHT, 0, p, c3);
  }
}

//-----------------------------------------------------------------------------
//      Toggle selected menu options
//-----------------------------------------------------------------------------

/* Update menu state: highlight the item under the cursor and handle clicks.
 * Items in the range [min..max] are disabled (greyed out). Executes the
 * selected action on mouse-button release.
 */
void update_menu(int menu, int min, int max) {
  byte *ptr = v.ptr, *p, *q;
  int w = v.w / big2, h = v.h / big2, n;

  if (dragging == 4)
    return;

  menu++;

  if (wmouse_y >= 0)
    n = (wmouse_y - (MENU_HEADER_HEIGHT - 1)) / MENU_ITEM_HEIGHT + 1;
  else
    n = 0;

  if (n >= min && n <= max) {
    n = 0;
    mouse_graf = 16;
  }

  if (mouse_b & MB_LEFT)
    n = -n;


  if (n != v.state) {
    if (v.state) {
      wbox(ptr, w, h, c2, 2, 1 + abs(v.state) * MENU_ITEM_HEIGHT, w - 4, 8);
      p = texts[menu + abs(v.state)];
      if (*p == '-')
        p++;

      if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
        *q = 0;
        wwrite(ptr, w, h, 3, 2 + abs(v.state) * MENU_ITEM_HEIGHT, 0, p, c3);
        *q = '[';
        wwrite(ptr, w, h, w - 4, 2 + abs(v.state) * MENU_ITEM_HEIGHT, 2, q, c1);
        wwrite(ptr, w, h, w - 5, 2 + abs(v.state) * MENU_ITEM_HEIGHT, 2, q, c3);
      } else
        wwrite(ptr, w, h, 3, 2 + abs(v.state) * MENU_ITEM_HEIGHT, 0, p, c3);
    }

    if (n) {
      if (n < 0) {
        wbox(ptr, w, h, c12, 3, 2 - n * MENU_ITEM_HEIGHT, w - 6, 6);

        wbox(ptr, w, h, c1, 2, 1 - n * MENU_ITEM_HEIGHT, 1, 7);
        wbox(ptr, w, h, c1, 2, 1 - n * MENU_ITEM_HEIGHT, w - 5, 1);
        wbox(ptr, w, h, c3, w - 3, 2 - n * MENU_ITEM_HEIGHT, 1, 7);
        wbox(ptr, w, h, c3, 3, 8 - n * MENU_ITEM_HEIGHT, w - 5, 1);

        if (big) {
          *(ptr + (2 * (8 - n * MENU_ITEM_HEIGHT)) * v.w + 2 * (w - 3)) = c34;
          *(ptr + (2 * (8 - n * MENU_ITEM_HEIGHT) + 1) * v.w + 2 * (w - 3) + 1) = c34;
          *(ptr + (2 * (1 - n * MENU_ITEM_HEIGHT)) * v.w + 2 * (2)) = c01;
          *(ptr + (2 * (1 - n * MENU_ITEM_HEIGHT) + 1) * v.w + 2 * (2) + 1) = c01;
          *(ptr + (2 * (1 - n * MENU_ITEM_HEIGHT)) * v.w + 2 * (w - 3)) = c1;
          *(ptr + (2 * (1 - n * MENU_ITEM_HEIGHT) + 1) * v.w + 2 * (w - 3) + 1) = c3;
          *(ptr + (2 * (8 - n * MENU_ITEM_HEIGHT)) * v.w + 2 * (2)) = c1;
          *(ptr + (2 * (8 - n * MENU_ITEM_HEIGHT) + 1) * v.w + 2 * (2) + 1) = c3;
        }

        p = texts[menu - n];
        if (*p == '-')
          p++;

        if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
          *q = 0;
          wwrite(ptr, w, h, 4, 2 - n * MENU_ITEM_HEIGHT, 0, p, c1);
          wwrite(ptr, w, h, 3, 2 - n * MENU_ITEM_HEIGHT, 0, p, c4);
          *q = '[';
          wwrite(ptr, w, h, w - 4, 2 - n * MENU_ITEM_HEIGHT, 2, q, c1);
          wwrite(ptr, w, h, w - 5, 2 - n * MENU_ITEM_HEIGHT, 2, q, c3);
        } else {
          wwrite(ptr, w, h, 4, 2 - n * MENU_ITEM_HEIGHT, 0, p, c1);
          wwrite(ptr, w, h, 3, 2 - n * MENU_ITEM_HEIGHT, 0, p, c4);
        }
      } else {
        wbox(ptr, w, h, c2, 2, 1 + n * MENU_ITEM_HEIGHT, w - 4, 8);
        p = texts[menu + n];
        if (*p == '-')
          p++;

        if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
          *q = 0;
          wwrite(ptr, w, h, 4, 2 + n * MENU_ITEM_HEIGHT, 0, p, c1);
          wwrite(ptr, w, h, 3, 2 + n * MENU_ITEM_HEIGHT, 0, p, c4);
          *q = '[';
          wwrite(ptr, w, h, w - 4, 2 + n * MENU_ITEM_HEIGHT, 2, q, c1);
          wwrite(ptr, w, h, w - 5, 2 + n * MENU_ITEM_HEIGHT, 2, q, c3);
        } else {
          wwrite(ptr, w, h, 4, 2 + n * MENU_ITEM_HEIGHT, 0, p, c1);
          wwrite(ptr, w, h, 3, 2 + n * MENU_ITEM_HEIGHT, 0, p, c4);
        }
      }
    }

    v.state = n;
    v.redraw = 1;
  }
}
