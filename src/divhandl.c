
//-----------------------------------------------------------------------------
//      Module that contains the window handler code
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#include "divsound.h"
#include "fpgfile.hpp"
#include "divsb.h"


///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////
void create_menu(int menu);
void paint_menu(int menu);
void update_menu(int menu, int min, int max);
int find_program_window(void);
void save_prg_buffer(memptrsize n);
int find_map_window(void);
void open_map(void);
int find_and_load_map(void);
void save_map(void);
void map_search(void);
void print_fontmap(void);
void generate_character(byte *di, int an, int al, int inc, char *si);
void AplieX(struct tmapa *MiMap, int man, int mal);


//-----------------------------------------------------------------------------
//      Module variables
//-----------------------------------------------------------------------------

byte AuxPal[768];
int status_nuevo;

void _completo(void);
void text_cursor(void);
void repaint_window(void);
int check_file(void);

extern int numero_error;
extern void browser0(void);
extern void browser1(void);
extern void browser2(void);
extern void browser3(void);

void errhlp0(void);

void DaniDel(char *name);
void about0(void);
void map_save(void);

void SaveSound(pcminfo *mypcminfo, char *dst);
void PasteNewSounds(void);

void fusionar_paletas(void);
int fmt_load_dac_map(char *name);
int fmt_load_dac_pcx(char *name);
int fmt_load_dac_bmp(char *name);
int fmt_load_dac_jpg(char *name);
int fmt_load_dac_fnt(char *name);
int fmt_load_dac_fpg(char *name);
int fmt_load_dac_pal(char *name);
void crear_paleta(void);

extern byte apply_palette[768];
extern byte *muestra;

//-----------------------------------------------------------------------------
//      Minimum handler function of type (????)
//-----------------------------------------------------------------------------

void dummy_handler(void) {}

//-----------------------------------------------------------------------------
//      Main Menu
//-----------------------------------------------------------------------------

void menu_principal0(void) {
  create_menu(750);
  v.paint_handler = menu_principal1;
  v.click_handler = menu_principal2;
  v.close_handler = menu_principal3;
}

void menu_principal1(void) {
  paint_menu(750);
}

/* Click handler for the top menu bar.
 * Dispatches mouse clicks to the appropriate submenu (Programs, Palettes,
 * Maps, Graphics, Fonts, Sounds, System, Help) based on which item is hit.
 */
void menu_principal2(void) {
  update_menu(750, 1, 0);
  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    switch (v.state) {
    case 1:
      new_window(menu_programas0);
      break;

    case 2:
      new_window(menu_paletas0);
      break;

    case 3:
      new_window(menu_mapas0);
      break;

    case 4:
      new_window(menu_graficos0);
      break;

    case 5:
      new_window(menu_fuentes0);
      break;

    case 6:
      new_window(menu_sonidos0);
      break;

    case 7:
      new_window(menu_sistema0);
      break;

    case 8:
      help(3);
      break;
    }
  }
}

void menu_principal3(void) {
  exit_requested = 1;
}

//-----------------------------------------------------------------------------
//      Programs Menu
//-----------------------------------------------------------------------------

void Print_Program(void);

void menu_programas0(void) {
  create_menu(900);
  v.paint_handler = menu_programas1;
  v.click_handler = menu_programas2;
}

void menu_programas1(void) {
  paint_menu(900);
}

void menu_programas2(void) {
  FILE *f;
  int n;

  if (find_program_window())
    update_menu(900, 1, 0);
  else
    update_menu(900, 3, 99);

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    v_type = 8;

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
          show_dialog(aceptar0);
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
          mouse_graf = 3;
          flush_copy();
          mouse_graf = 1;
          open_program();
        }
      }
      break;

    case 3:
      if ((n = find_program_window())) {
        v_title = (char *)texts[188];
        v_text = (char *)window[n].title;
        show_dialog(aceptar0);
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
        mouse_graf = 3;
        flush_copy();
        mouse_graf = 1;
        wup(n);
        div_strcpy(tipo[v_type].path, sizeof(tipo[v_type].path), v.prg->path);
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
            show_dialog(aceptar0);
          } else
            v_accept = 1;
          if (v_accept) {
            mouse_graf = 3;
            flush_copy();
            mouse_graf = 1;
            wup(n);
            save_program();
            wdown(n);
            v.redraw = 1;
          }
        }
      }
      break;

    case 6:
      new_window(menu_edicion0);
      break;

    case 8:
      if ((n = find_program_window())) {
        save_prg_buffer(n);
        source_ptr = window[n].prg->buffer;
        source_len = window[n].prg->file_len;
        v_window = n;
        run_mode = 0;
        div_strcpy(tipo[8].path, sizeof(tipo[8].path), window[n].prg->path);
        compile_program();
        if (numero_error != -1) {
          goto_error();
          if (v_help)
            help(500 + numero_error);
        } else if (v_help)
          help(599);
      }
      break;

    case 7:
    case 9:
      if ((n = find_program_window())) {
        save_prg_buffer(n);
        mouse_graf = 3;
        flush_copy();
        mouse_graf = 1;
        wup(n);
        div_strcpy(tipo[v_type].path, sizeof(tipo[v_type].path), v.prg->path);
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
        div_strcpy(tipo[8].path, sizeof(tipo[8].path), window[n].prg->path);
        compile_program();
        if (numero_error != -1) {
          goto_error();
          if (v_help)
            help(500 + numero_error);
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
        DaniDel("system/exec.ins");
        div_strcpy(tipo[8].path, sizeof(tipo[8].path), window[n].prg->path);
        compile_program();
        if (numero_error != -1) {
          goto_error();
          if (v_help)
            help(500 + numero_error);
        } else {
          f = fopen("system/exec.ins", "rb");
          if (f != NULL) {
            fclose(f);
            crear_instalacion();
            DaniDel("install.div");
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
        Print_Program();
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Edit Menu
//-----------------------------------------------------------------------------

void _completo(void);
void f_delete(void);
void f_cut_block(memptrsize);
void f_paste_block(void);
void f_mark(void);
void f_unmark(void);

void menu_edicion0(void) {
  create_menu(950);
  v.paint_handler = menu_edicion1;
  v.click_handler = menu_edicion2;
}

void menu_edicion1(void) {
  paint_menu(950);
}

void menu_edicion2(void) {
  int n, m;

  if (find_program_window())
    update_menu(950, 1, 0);
  else
    update_menu(950, 0, 99);

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    v_type = 8;

    switch (v.state) {
    case 1: // Delete line
      if ((n = find_program_window())) {
        wup(n);
        f_delete();
        _completo();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 2: // Mark
      if ((n = find_program_window())) {
        wup(n);
        f_mark();
        _completo();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 3: // Unmark
      if ((n = find_program_window())) {
        wup(n);
        f_unmark();
        _completo();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 4: // Cut
      if ((n = find_program_window())) {
        wup(n);
        f_cut_block(1);
        _completo();
        v.redraw = 2;
        wdown(n);
        flush_window(n);
      }
      break;

    case 5: // Copy
      if ((n = find_program_window())) {
        wup(n);
        f_cut_block(0);
        _completo();
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
        _completo();
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
        if (v.foreground == 0) {
          for (m = 1; m < max_windows; m++)
            if (window[m].type && window[m].foreground == 1)
              if (windows_collide(0, m)) {
                window[m].foreground = 0;
                flush_window(m);
              }
          v.foreground = 1;
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
          _completo();
          text_cursor();
        }
      }
      break;

    case 8:
      if ((n = find_program_window())) {
        show_dialog(find_text0);
        if (v_accept) {
          move(0, n);
          if (v.foreground == 0) {
            for (m = 1; m < max_windows; m++)
              if (window[m].type && window[m].foreground == 1)
                if (windows_collide(0, m)) {
                  window[m].foreground = 0;
                  flush_window(m);
                }
            v.foreground = 1;
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
        call((voidReturnType)v.click_handler);
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        move(0, n);
        if (v.foreground == 0) {
          for (m = 1; m < max_windows; m++)
            if (window[m].type && window[m].foreground == 1)
              if (windows_collide(0, m)) {
                window[m].foreground = 0;
                flush_window(m);
              }
          v.foreground = 1;
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
          if (v.foreground == 0) {
            for (m = 1; m < max_windows; m++)
              if (window[m].type && window[m].foreground == 1)
                if (windows_collide(0, m)) {
                  window[m].foreground = 0;
                  flush_window(m);
                }
            v.foreground = 1;
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
//      Palette Menu
//-----------------------------------------------------------------------------

void menu_paletas0(void) {
  create_menu(775);
  v.paint_handler = menu_paletas1;
  v.click_handler = menu_paletas2;
}

void menu_paletas1(void) {
  paint_menu(775);
}

void menu_paletas2(void) {
  update_menu(775, 1, 0);
  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
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
      new_window(paleta0);
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Maps Menu
//-----------------------------------------------------------------------------

extern char input2[32];

void browser0(void);

void menu_mapas0(void) {
  create_menu(800);
  v.paint_handler = menu_mapas1;
  v.click_handler = menu_mapas2;
}

void menu_mapas1(void) {
  paint_menu(800);
}

void menu_mapas2(void) {
  int n, m;

  if (find_map_window())
    update_menu(800, 1, 0);
  else
    update_menu(800, 3, 9);

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    v_type = 2;

    switch (v.state) {
    case 1: // New map ...
      show_dialog(nuevo_mapa0);
      if (v_finished) {
        mouse_graf = 3;
        flush_copy();
        mouse_graf = 1;
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
          mouse_graf = 3;
          flush_copy();
          mouse_graf = 1;
          open_map();
        }
      }
      break;

    case 3: // Close map
      if ((n = find_and_load_map())) {
        v_title = (char *)texts[50];
        v_text = (char *)window[n].title;
        show_dialog(aceptar0);
        if (v_accept) {
          move(0, n);
          close_window();
        }
      }
      break;

    case 4: // Close all maps (maximized)
      v_title = (char *)texts[334];
      v_text = (char *)texts[335];
      show_dialog(aceptar0);
      if (v_accept) {
        for (n = 0; n < max_windows; n++)
          if (window[n].type == 100 && window[n].foreground != 2) {
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
        mouse_graf = 3;
        flush_copy();
        mouse_graf = 1;
        div_strcpy(tipo[v_type].path, sizeof(tipo[v_type].path), window[n].mapa->path);
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
            show_dialog(aceptar0);
          } else
            v_accept = 1;
          if (v_accept) {
            mouse_graf = 3;
            flush_copy();
            mouse_graf = 1;
            save_map();
          }
        }
      }
      break;

    case 7: // Rescale the map
      Reducex2();
      break;

    case 8: // Create search map
      map_search();
      break;

    case 9: // Edit the map
      if ((n = find_and_load_map())) {
        paint_menu(800);
        v.state = 0;
        move(0, n);
        if (v.foreground == 0) {
          for (m = 1; m < max_windows; m++)
            if (window[m].type && window[m].foreground == 1)
              if (windows_collide(0, m)) {
                window[m].foreground = 0;
                flush_window(m);
              }
          v.foreground = 1;
        }
        mouse_b = 1;
        call((voidReturnType)v.click_handler);
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

void actualiza_titulo(void) {
  int an = v.an, al = v.al;
  if (big) {
    an /= 2;
    al /= 2;
  }

  wgra(v.ptr, an, al, c_b_low, 2, 2, an - 20, 7);
  if (text_len(v.title) + 3 > an - 20) {
    wwrite_in_box(v.ptr, an, an - 19, al, 4, 2, 0, v.title, c1);
    wwrite_in_box(v.ptr, an, an - 19, al, 3, 2, 0, v.title, c4);
  } else {
    wwrite(v.ptr, an, al, 3 + (an - 20) / 2, 2, 1, v.title, c1);
    wwrite(v.ptr, an, al, 2 + (an - 20) / 2, 2, 1, v.title, c4);
  }
}

// menu_mapas3D functions removed (MODE8/3D map editor deleted)

int check_file(void) {
  char cwork[8];
  FILE *f;

  v_mode = 0;
  v_type = 4;
  v_text = (char *)texts[70];
  show_dialog(browser0);

  div_strcpy(full, sizeof(full), tipo[v_type].path);
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

int find_pcm_window(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 105 && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_mod_window(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 107 && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_font_window(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 104 && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_fpg_window(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 101 && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

// determina_prj removed (MODE8/3D map editor deleted)

int find_map_window(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 100 && window[m].state) {
      n = m;
      break;
    }
  return (n);
}

int find_and_load_map(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++)
    if (window[m].type == 100 && window[m].state) {
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

// determina_mapa3d removed (MODE8/3D map editor deleted)

int find_program_window(void) {
  int m, n = 0;

  for (m = 1; m < max_windows; m++) {
    if (window[m].type == 102 && window[m].state && window[m].prg != NULL) {
      n = m;
      break;
    }
  }
  return (v_window = n);
}

//-----------------------------------------------------------------------------
//      Graphic Menu (FPG Files)
//-----------------------------------------------------------------------------

void Print_List(void);
void FPGtoMAP(FPG *MiFPG);
void MAPtoFPG(struct tmapa *mapa);

void menu_graficos0(void) {
  create_menu(825);
  v.paint_handler = menu_graficos1;
  v.click_handler = menu_graficos2;
}

void menu_graficos1(void) {
  paint_menu(825);
}

void menu_graficos2(void) {
  FPG *Fpg;
  int n;

  if (find_fpg_window() && find_map_window())
    update_menu(825, 1, 0);
  else if (find_fpg_window())
    update_menu(825, 9, 9);
  else if (find_map_window())
    update_menu(825, 3, 8);
  else
    update_menu(825, 3, 9);

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    v_type = 4;

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
        show_dialog(aceptar0);
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
            Fpg = (FPG *)window[n].aux;
            div_strcpy(full, sizeof(full), tipo[4].path);
            if (full[strlen(full) - 1] != '/')
              div_strcat(full, sizeof(full), "/");
            div_strcat(full, sizeof(full), input);

            if (!strcmp(full, (char *)Fpg->ActualFile)) {
              v_text = (char *)texts[418];
              show_dialog(err0);
              v_accept = 0;
            } else {
              v_title = (char *)texts[82];
              v_text = input;
              show_dialog(aceptar0);
            }

          } else {
            v_accept = 1;
          }
          if (v_accept) {
            mouse_graf = 3;
            flush_copy();
            mouse_graf = 1;
            fpg_save(n);
          }
        }
      }
      break;

    case 5:
      Show_Taggeds();
      break;

    case 6:
      Delete_Taggeds();
      break;

    case 7:
      Print_List();
      break;

    case 8:
      if ((n = find_fpg_window())) {
        FPGtoMAP((FPG *)(window[n].aux));
      }
      break;

    case 9:
      if ((n = find_map_window())) {
        MAPtoFPG(window[n].mapa);
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//      Fonts Menu
//-----------------------------------------------------------------------------

void menu_fuentes0(void) {
  create_menu(850);
  v.paint_handler = menu_fuentes1;
  v.click_handler = menu_fuentes2;
}

void menu_fuentes1(void) {
  paint_menu(850);
}

extern char *FntAux;
void GetText0(void);
void CreateText(void);
void generate_fontmap(void);

void menu_fuentes2(void) {
  int n;

  if (find_font_window()) {
    if (find_map_window())
      update_menu(850, 1, 0);
    else
      update_menu(850, 6, 6);
  } else {
    if (find_map_window())
      update_menu(850, 3, 5);
    else
      update_menu(850, 3, 6);
  }

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    switch (v.state) {
    case 1:
      OpenGenFont();
      break;

    case 2:
      OpenFont();
      break;

    case 3:
      if ((n = find_font_window())) {
        move(0, n);
        close_window();
      }
      break;

    case 4:
      if ((n = find_font_window())) {
        FntAux = (char *)(window[n].aux + RES_FOR_NAME);
        show_dialog(GetText0);
        if (v_accept)
          CreateText();
      }
      break;

    case 5:
      if ((n = find_font_window())) {
        FntAux = (char *)(window[n].aux + RES_FOR_NAME);
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
//      Functions for generate_fontmap() / print_fontmap()
//-----------------------------------------------------------------------------

#define mapa(x, y) (*(map + (y) * map_width + (x)))
void ShowFont0(void);

extern char Load_FontPathName[256];
extern char Load_FontName[14];

void print_fontmap(void) {
  char FontPathName[256], FontName[14] = "";
  char *buffer, *di, color;
  int buffer_len, x, y, _x0, x0 = 1, y0 = 1, chars, *p;
  int gencode;
  FILE *f;

  v_mode = 1;
  v_type = 5;

  div_strcpy(FontPathName, sizeof(FontPathName), tipo[v_type].path);
  div_strcat(FontPathName, sizeof(FontPathName), "/");
  div_strcat(FontPathName, sizeof(FontPathName), FontName);

  v_text = (char *)texts[264];
  show_dialog(browser0);

  if (!v_finished)
    return;

  if (v_exists) {
    v_title = (char *)texts[75];
    v_text = (char *)texts[76];
    show_dialog(aceptar0);
    if (!v_accept)
      return;
  }

  div_strcpy(FontName, sizeof(FontName), input);
  DIV_STRCPY(FontPathName, tipo[v_type].path);
  if (!IS_PATH_SEP(FontPathName[strlen(FontPathName) - 1]))
    div_strcat(FontPathName, sizeof(FontPathName), "/");
  div_strcat(FontPathName, sizeof(FontPathName), input); // * FontPathName full path of the file

  buffer_len = 1356 + 256 * 16 + map_width * map_height; // Allocate enough memory for the FNT

  if ((buffer = (char *)malloc(buffer_len)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return; // Not enought memory, close ...
  }

  memcpy(buffer, "fnt\x1a\x0d\x0a\x00", 8);
  memcpy(buffer + 8, dac, 768);
  memcpy(buffer + 776, gradients, sizeof(gradients));
  memset(buffer + 1356, 0, 256 * 16);

  di = buffer + 1356 + 256 * 16;
  color = *map;
  chars = 0;
  p = (int *)(buffer + 1356);

  for (x = 1; x < map_width - 1; x++) {
    for (y = 1; y < map_height - 1; y++) {
      if (mapa(x - 1, y - 1) == color && mapa(x - 1, y) == color && mapa(x, y - 1) == color &&
          mapa(x, y) != color) {
        x0 = x;
        y0 = y;
      }
      if (mapa(x, y) != color && mapa(x + 1, y + 1) == color && mapa(x + 1, y) == color &&
          mapa(x, y + 1) == color) {
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
              *di++ = mapa(_x0, y0);
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
  if (*(int *)(buffer + 1356 + '0' * 16))
    gencode |= 1;
  if (*(int *)(buffer + 1356 + 'A' * 16))
    gencode |= 2;
  if (*(int *)(buffer + 1356 + 'a' * 16))
    gencode |= 4;
  if (*(int *)(buffer + 1356 + '?' * 16))
    gencode |= 8;
  if (*(int *)(buffer + 1356 + '\xa4' * 16))
    gencode |= 16;

  memcpy(buffer + 1352, &gencode, 4);

  if ((f = fopen(FontPathName, "wb")) == NULL) {
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


  for (x = -1, y = 1; y < max_windows; y++)
    if (window[y].type == 104 && !strcmp((char *)window[y].title, FontName))
      x = y;

  if (x >= 0) {
    move(0, x);
    close_window();
  }

  div_strcpy(Load_FontName, sizeof(Load_FontName), FontName);
  div_strcpy(Load_FontPathName, sizeof(Load_FontPathName), FontPathName);
  new_window(ShowFont0);
}

//-----------------------------------------------------------------------------

void generate_fontmap(void) {
  int *p = (int *)(FntAux + 1356), n, x;
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
      FntEnd = FntAux + *(p + n * 4 + 3) + *(p + n * 4) * *(p + n * 4 + 1);
    }
  }

  MustCreate = 0;

  if (new_map(NULL)) {
    MustCreate = 1;
    return;
  }

  MustCreate = 1;

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
                         *(p + n * 4 + 2), FntAux + *(p + n * 4 + 3));
      if (*(p + n * 4) == 0 || *(p + n * 4 + 1) == 0)
        x += 2;
      else
        x += *(p + n * 4) + 1;
    }
    new_window(mapa0);
  }
}

void generate_character(byte *di, int an, int al, int inc, char *si) {
  int x, y;
  if (an == 0 || al == 0) {
    *di = 0;
  } else {
    di += inc * map_width;
    y = al;
    do {
      x = an;
      do {
        *di++ = *si++;
      } while (--x);
      di += map_width - an;
    } while (--y);
  }
}

//-----------------------------------------------------------------------------
//      Sound Menu
//-----------------------------------------------------------------------------

void OpenSound(void);
void OpenSong(void);
void OpenSoundFile(void);

void menu_sonidos0(void) {
  create_menu(925);
  v.paint_handler = menu_sonidos1;
  v.click_handler = menu_sonidos2;
}

void menu_sonidos1(void) {
  paint_menu(925);
}

void mixer0(void);
void RecSound0(void);
void editar_sonido(void);

void menu_sonidos2(void) {
  pcminfo *mypcminfo;
  int n;

  if (find_pcm_window())
    update_menu(925, 1, 0);
  else
    update_menu(925, 3, 6);

  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    switch (v.state) {
    case 1:

      if (!SoundActive) {
        if (SoundError) {
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
      } else if (!SoundActive) {
        v_text = (char *)texts[575];
        show_dialog(err0);
        return;
      }

      show_dialog(RecSound0);
      break;

    case 2:
      OpenSound();
      break;

    case 3:
      if ((n = find_pcm_window())) {
        v_title = (char *)texts[338];
        v_text = (char *)window[n].title;
        show_dialog(aceptar0);
        if (v_accept) {
          move(0, n);
          close_window();
        }
      }
      break;

    case 4:
      if ((n = find_pcm_window())) {
        if (strchr((const char *)window[n].title, ' ') != NULL)
          goto no_tiene_nombre_sonido;
        mouse_graf = 3;
        flush_copy();
        mouse_graf = 1;
        mypcminfo = (pcminfo *)window[n].aux;
        div_strcpy(full, sizeof(full), mypcminfo->pathname);
        div_strcpy(input, sizeof(input), mypcminfo->name);
        SaveSound(mypcminfo, full);
        wup(n);
        repaint_window();
        wdown(n);
        flush_window(n);
      }
      break;

    case 5:
no_tiene_nombre_sonido:
      if ((n = find_pcm_window())) {
        v_type = 7;
        v_mode = 1;
        v_text = (char *)texts[339];
        show_dialog(browser0);
        if (v_finished) {
          if (v_exists) {
            v_title = (char *)texts[340];
            v_text = input;
            show_dialog(aceptar0);
          } else
            v_accept = 1;
          if (v_accept) {
            mouse_graf = 3;
            flush_copy();
            mouse_graf = 1;
            mypcminfo = (pcminfo *)window[n].aux;
            div_strcpy(full, sizeof(full), tipo[v_type].path);
            if (full[strlen(full) - 1] != '/')
              div_strcat(full, sizeof(full), "/");
            div_strcat(full, sizeof(full), input);

            if (strchr(input, ' ') == NULL) {
              SaveSound(mypcminfo, full);
              div_strcpy(mypcminfo->pathname, sizeof(mypcminfo->pathname), full);
              div_strcpy(mypcminfo->name, sizeof(mypcminfo->name), input);
              move(0, n);
              close_window();
              OpenSoundFile();
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
        v_type = 7;
        v_text = (char *)texts[349];

        //          judas_stopsample(0);

        show_dialog(EditSound0);
        move(0, n);
        call((voidReturnType)v.paint_handler);
        v.redraw = 1;
        PasteNewSounds();
      }
      break;

    case 7:
      new_window(mixer0);
      break;

    case 8:
      if (!SoundActive) {
        if (SoundError) {
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

      OpenSong();
      break;

    case 9:
      if ((n = find_mod_window())) {
        v_title = (char *)texts[513];
        v_text = (char *)window[n].title;
        show_dialog(aceptar0);
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

void MemInfo0(void);

void menu_sistema0(void) {
  create_menu(875);
  v.paint_handler = menu_sistema1;
  v.click_handler = menu_sistema2;
}

void menu_sistema1(void) {
  paint_menu(875);
}

void Cfg_Setup0(void);
void Cfg_Setup_end(void);
void shell(void);
void calculator(void);

void menu_sistema2(void) {
  update_menu(875, 1, 0);
  if ((prev_mouse_buttons & 1) && !(mouse_b & 1)) {
    switch (v.state) {
    case 1:
      show_clock();
      break;
    case 2:
      muestra_papelera();
      break;
    case 3:
      calculator();
      break;
    case 4:
      show_dialog(Vid_Setup0);
      break;
    case 5:
      show_dialog(Tap_Setup0);
      break;
    case 6:
      show_dialog(Cfg_Setup0);
      Cfg_Setup_end();
      break;
    case 7:
      show_dialog(MemInfo0);
      break;
    case 8:
      show_dialog(about0);
      break;
    case 9:
      v_title = (char *)texts[40];
      v_text = NULL;
      show_dialog(aceptar0);
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
  int an;

  v.type = 2; // Menu
  v.name = texts[menu++];
  v.title = texts[menu];
  v.state = 0;

  v.an = text_len(texts[menu++]) + 23;
  v.al = 11;

  while ((char *)texts[menu]) {
    v.al += 9;
    an = text_len(texts[menu++]) + 7;
    if (v.an < an)
      v.an = an;
  }
}

//-----------------------------------------------------------------------------
//      Draw the active menu
//-----------------------------------------------------------------------------

void paint_menu(int menu) {
  byte *ptr = v.ptr, *p, *q;

  int an = v.an / big2, al = v.al / big2, n = 0;

  menu++;

  wbox(ptr, an, al, c2, 2, 10, an - 4, al - 12);

  while (texts[++menu]) {
    if (*(p = texts[menu]) == '-') {
      p++;
      wbox(ptr, an, al, c0, 2, 9 + n * 9, an - 4, 1);
    }

    if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
      *q = 0;
      wwrite(ptr, an, al, 3, 11 + n * 9, 0, p, c3);
      *q = '[';
      wwrite(ptr, an, al, an - 4, 11 + n * 9, 2, q, c1);
      wwrite(ptr, an, al, an - 5, 11 + n * 9, 2, q, c3);
      n++;
    } else
      wwrite(ptr, an, al, 3, 11 + n++ * 9, 0, p, c3);
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
  int an = v.an / big2, al = v.al / big2, n;

  if (dragging == 4)
    return;

  menu++;

  if (wmouse_y >= 0)
    n = (wmouse_y - 10) / 9 + 1;
  else
    n = 0;

  if (n >= min && n <= max) {
    n = 0;
    mouse_graf = 16;
  }

  if (mouse_b & 1)
    n = -n;


  if (n != v.state) {
    if (v.state) {
      wbox(ptr, an, al, c2, 2, 1 + abs(v.state) * 9, an - 4, 8);
      p = texts[menu + abs(v.state)];
      if (*p == '-')
        p++;

      if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
        *q = 0;
        wwrite(ptr, an, al, 3, 2 + abs(v.state) * 9, 0, p, c3);
        *q = '[';
        wwrite(ptr, an, al, an - 4, 2 + abs(v.state) * 9, 2, q, c1);
        wwrite(ptr, an, al, an - 5, 2 + abs(v.state) * 9, 2, q, c3);
      } else
        wwrite(ptr, an, al, 3, 2 + abs(v.state) * 9, 0, p, c3);
    }

    if (n) {
      if (n < 0) {
        wbox(ptr, an, al, c12, 3, 2 - n * 9, an - 6, 6);

        wbox(ptr, an, al, c1, 2, 1 - n * 9, 1, 7);
        wbox(ptr, an, al, c1, 2, 1 - n * 9, an - 5, 1);
        wbox(ptr, an, al, c3, an - 3, 2 - n * 9, 1, 7);
        wbox(ptr, an, al, c3, 3, 8 - n * 9, an - 5, 1);

        if (big) {
          *(ptr + (2 * (8 - n * 9)) * v.an + 2 * (an - 3)) = c34;
          *(ptr + (2 * (8 - n * 9) + 1) * v.an + 2 * (an - 3) + 1) = c34;
          *(ptr + (2 * (1 - n * 9)) * v.an + 2 * (2)) = c01;
          *(ptr + (2 * (1 - n * 9) + 1) * v.an + 2 * (2) + 1) = c01;
          *(ptr + (2 * (1 - n * 9)) * v.an + 2 * (an - 3)) = c1;
          *(ptr + (2 * (1 - n * 9) + 1) * v.an + 2 * (an - 3) + 1) = c3;
          *(ptr + (2 * (8 - n * 9)) * v.an + 2 * (2)) = c1;
          *(ptr + (2 * (8 - n * 9) + 1) * v.an + 2 * (2) + 1) = c3;
        }

        p = texts[menu - n];
        if (*p == '-')
          p++;

        if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
          *q = 0;
          wwrite(ptr, an, al, 4, 2 - n * 9, 0, p, c1);
          wwrite(ptr, an, al, 3, 2 - n * 9, 0, p, c4);
          *q = '[';
          wwrite(ptr, an, al, an - 4, 2 - n * 9, 2, q, c1);
          wwrite(ptr, an, al, an - 5, 2 - n * 9, 2, q, c3);
        } else {
          wwrite(ptr, an, al, 4, 2 - n * 9, 0, p, c1);
          wwrite(ptr, an, al, 3, 2 - n * 9, 0, p, c4);
        }
      } else {
        wbox(ptr, an, al, c2, 2, 1 + n * 9, an - 4, 8);
        p = texts[menu + n];
        if (*p == '-')
          p++;

        if ((q = (byte *)strchr((const char *)p, '[')) != NULL) {
          *q = 0;
          wwrite(ptr, an, al, 4, 2 + n * 9, 0, p, c1);
          wwrite(ptr, an, al, 3, 2 + n * 9, 0, p, c4);
          *q = '[';
          wwrite(ptr, an, al, an - 4, 2 + n * 9, 2, q, c1);
          wwrite(ptr, an, al, an - 5, 2 + n * 9, 2, q, c3);
        } else {
          wwrite(ptr, an, al, 4, 2 + n * 9, 0, p, c1);
          wwrite(ptr, an, al, 3, 2 + n * 9, 0, p, c4);
        }
      }
    }

    v.state = n;
    v.redraw = 1;
  }
}

//-----------------------------------------------------------------------------
//      Map window type
//-----------------------------------------------------------------------------

// A "map" type window requires the following information (whether
// loading a map or creating a new one in the program)

// v_map->filename   - These two fields will be "Map x",NULL if
// v_map->path       - the map has no corresponding file on disk yet
// v_map->map
// v_map->selection_mask
// v_map->map_width/al

void blit_mouse(void);

void mapa0(void) {
  int x, y;
  int max;

  v.type = 100; // Map (windows of which only one can be active)
  v.mapa = v_map;

  if ((v.an = v.mapa->map_width) < 48 * big2)
    v.an = (24 + 4) * big2;
  else {
    max = vga_width / 2 - big2;
    v.an = v.an / 2 + 4 * big2;
    if (v.an > max)
      v.an = max;
  }


  if ((v.al = v.mapa->map_height) < 16 * big2)
    v.al = (8 + 12) * big2;
  else {
    max = vga_height / 2 - 9 * big2;
    v.al = v.al / 2 + 12 * big2;
    if (v.al > max)
      v.al = max;
  }

  if (big) {
    if (v.an & 1)
      v.an++;
    if (v.al & 1)
      v.al++;
    v.an = -v.an; // Negative to indicate the window should not be doubled
  }

  v.title = (byte *)v.mapa->filename;
  v.name = (byte *)v.mapa->filename;

  v.mapa->code = next_map_code++;
  v.mapa->saved = 1;

  v.mapa->zoom_cx = v.mapa->map_width / 2;
  v.mapa->zoom_cy = v.mapa->map_height / 2;
  x = (v.mapa->zoom_cx - vga_width / 2);
  if (x < 0)
    x = 0;
  else if (x + vga_width > v.mapa->map_width)
    x = v.mapa->map_width - vga_width;
  y = (v.mapa->zoom_cy - vga_height / 2);
  if (y < 0)
    y = 0;
  else if (y + vga_height > v.mapa->map_height)
    y = v.mapa->map_height - vga_height;
  v.mapa->zoom = 0;
  v.mapa->zoom_x = x;
  v.mapa->zoom_y = y;

  v.paint_handler = mapa1;
  v.click_handler = mapa2;
  v.close_handler = mapa3;
}

void mapa1(void) {
  byte *si, *di;
  int an, al;
  int x, y, x2, y2, ansi, andi;
  int maxan, maxal;

  andi = v.an;
  di = v.ptr + andi * 10 * big2 + 2 * big2;

  if (v.mapa->map_width < 48 * big2)
    di += (24 * big2 - v.mapa->map_width / 2) / 2;
  if (v.mapa->map_height < 16 * big2)
    di += ((8 * big2 - v.mapa->map_height / 2) / 2) * andi;

  ansi = v.mapa->map_width;
  si = v.mapa->map;

  maxan = v.an - 4 * big2;
  maxal = v.al - 12 * big2;

  if ((an = v.mapa->map_width / 2) > maxan) {
    an = maxan;
    x = (v.mapa->zoom_cx - maxan);
    if (x < 0)
      x = 0;
    else if (x + maxan * 2 > v.mapa->map_width)
      x = v.mapa->map_width - maxan * 2;
    si += x;
  }

  if ((al = v.mapa->map_height / 2) > maxal) {
    al = maxal;
    y = (v.mapa->zoom_cy - maxal);
    if (y < 0)
      y = 0;
    else if (y + maxal * 2 > v.mapa->map_height)
      y = v.mapa->map_height - maxal * 2;
    si += y * ansi;
  }

  if (an && al)
    for (y = 0, y2 = 0; y < al; y++, y2 += 2)
      for (x = 0, x2 = 0; x < an; x++, x2 += 2)
        *(di + andi * y + x) = *(ghost + *(ghost + *(word *)(si + ansi * y2 + x2)) * 256 +
                                 *(ghost + *(word *)(si + ansi * (y2 + 1) + x2)));
  else {
    if (!an) {
      an++;
    }
    if (!al) {
      al++;
    }
    for (y = 0, y2 = 0; y < al; y++, y2 += 2)
      for (x = 0, x2 = 0; x < an; x++, x2 += 2)
        *(di + andi * y + x) = *(si + ansi * y2 + x2);
  }
}

int windows_collide(int, int);

void calculate_foreground(void) {
  int n, m;

  for (n = 0; n < max_windows; n++)
    if (window[n].type && window[n].foreground != 2) {
      window[n].foreground = 1;
      for (m = 0; m < n; m++)
        if (window[m].type) {
          if (windows_collide(n, m))
            window[n].foreground = 0;
        }
    }
}

// READ MOUSE BUTTONS

void read_mouse3(void) {}

extern int back;

#define max_texturas 1000
#define an_textura   (3 + 1) // width for 000 - 999

#define BRUSH 4
#define MAPBR 8

extern int TipoTex;
extern int TipoBrowser;
extern byte brush_fpg_path[256];
extern struct t_listboxbr ltexturasbr;
extern struct t_listboxbr lthumbmapbr;

extern struct _thumb_tex {
  int an, al;         // Width and height of the thumbnail
  int RealAn, RealAl; // Width and height of the texture
  char *ptr;          // ==NULL if the thumbnail has not started loading
  int status;         // 0-Not a valid texture, 1-Loaded
  int FilePos;
  int Code;
  int Cuad;
} thumb_tex[max_texturas];

extern struct _thumb_map {
  int an, al;         // Width and height of the thumbnail
  int RealAn, RealAl; // Width and height of the texture
  char *ptr;          // ==NULL if the thumbnail has not started loading
  int status;         // 0-Not a valid texture, 1-Loaded
  int FilePos;
  int Code;
  int Cuad;
} thumb_map[max_windows];

void M3D_create_thumbs(struct t_listboxbr *l, int prog);
int create_mapbr_thumbs(struct t_listboxbr *l);
void FreePaintThumbs(void);

extern byte *textura_color;

FILE *FilePaintFPG;

void mapa2(void) {
  int old_draw_mode = 0, n, _undo_index;
  int an = v.an, al = v.al;
  int sp_an, sp_al, ew;
  byte *sp;
  if (big) {
    an /= 2;
    al /= 2;
  }

  if ((mouse_b & 1)) {
    read_mouse3();

    num_windows_to_create = 0;
    _undo_index = undo_index;

    map = v.mapa->map;

    map_width = v.mapa->map_width;
    map_height = v.mapa->map_height;

    selection_mask = (int *)malloc(((map_width * map_height + 31) / 32) * 4);

    if (selection_mask == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    for (n = 1; n < max_windows; n++)
      if (window[n].type && window[n].foreground == 1) {
        hidden[n] = 1;
        window[n].foreground = 0;
      } else
        hidden[n] = 0;

    zoom = v.mapa->zoom;
    zoom_x = v.mapa->zoom_x;
    zoom_y = v.mapa->zoom_y;
    zoom_cx = v.mapa->zoom_cx;
    zoom_cy = v.mapa->zoom_cy;

    // The following four 'if' are to avoid problems when you change (video mode?)

    if ((map_width << zoom) < vga_width) {
      zoom_win_width = map_width << zoom;
      zoom_win_x = (vga_width - zoom_win_width) / 2;
    } else {
      zoom_win_x = 0;
      zoom_win_width = (vga_width >> zoom) << zoom;
    }

    if ((map_height << zoom) < vga_height) {
      zoom_win_height = map_height << zoom;
      zoom_win_y = (vga_height - zoom_win_height) / 2;
    } else {
      zoom_win_y = 0;
      zoom_win_height = (vga_height >> zoom) << zoom;
    }

    zoom_x = zoom_cx - (zoom_win_width / 2) / (1 << zoom);
    if (zoom_x <= 0)
      zoom_x = 0;
    else if (zoom_x + zoom_win_width / (1 << zoom) > map_width) {
      zoom_x = map_width - zoom_win_width / (1 << zoom);
      if (zoom_x < 0)
        zoom_x = 0;
    }

    zoom_y = zoom_cy - (zoom_win_height / 2) / (1 << zoom);
    if (zoom_y < 0)
      zoom_y = 0;
    else if (zoom_y + zoom_win_height / (1 << zoom) > map_height) {
      zoom_y = map_height - zoom_win_height / (1 << zoom);
      if (zoom_y < 0)
        zoom_y = 0;
    }

    current_map_code = v.mapa->code;
    zoom_background = 0;
    v.type = 0;
    v.foreground = 0;
    v.state = 0;

    wgra(v.ptr, an, al, c1, 2, 2, an - 20, 7);
    if (text_len(v.title) + 3 > an - 20) {
      wwrite_in_box(v.ptr, an, an - 19, al, 4, 2, 0, v.title, c0);
      wwrite_in_box(v.ptr, an, an - 19, al, 3, 2, 0, v.title, c2);
    } else {
      wwrite(v.ptr, an, al, 2 + (an - 20) / 2, 3, 1, v.title, c0);
      wwrite(v.ptr, an, al, 2 + (an - 20) / 2, 2, 1, v.title, c2);
    }

    if (dragging == 4) {
      mouse_b = 0;
      old_draw_mode = draw_mode;
      draw_mode = 190;
    } else
      do {
        read_mouse();
      } while (mouse_b & 1);

    highlight_background = 0;
    current_mouse = 21 + paint_cursor * 4;

    // Special initialization for the drawing program
    ew = exploding_windows;
    exploding_windows = 0;
    for (n = 0; n < max_texturas; n++)
      thumb_tex[n].ptr = NULL;
    for (n = 0; n < max_windows; n++)
      thumb_map[n].ptr = NULL;
    div_strcpy(full, sizeof(full), tipo[1].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), "system/brush.fpg");

    textura_color = NULL;

    TipoTex = 0;
    if ((FilePaintFPG = fopen(full, "rb")) != NULL) // NOTE !!! Could provide message here
    {
      div_strcpy((char *)brush_fpg_path, sizeof(brush_fpg_path), full);
      draw_mode -= 100;
      M3D_create_thumbs(&ltexturasbr, 0);
      draw_mode += 100;
      TipoTex |= BRUSH; // Thumbnail type BRUSH
    }

    v.type = 100;
    if (create_mapbr_thumbs(&lthumbmapbr)) {
      TipoTex |= MAPBR; // Thumbnail type MAPBR
    }
    v.type = 0;

    do {
      zoom_map();
      need_zoom = 0;
      back = 0;
      draw_mode -= 100;
      switch (draw_mode) {
      case 0:
        edit_mode_0();
        break; // Pixels
      case 1:
        edit_mode_1();
        break; // Strokes
      case 2:
        edit_mode_2();
        break; // Lines
      case 3:
        edit_mode_3();
        break; // Lines cont.
      case 4:
        edit_mode_4();
        break; // Bezier
      case 5:
        edit_mode_5();
        break; // Bezier cont.
      case 6:
        edit_mode_6();
        break; // Boxes
      case 7:
        edit_mode_7();
        break; // Circles
      case 8:
        edit_mode_8();
        break; // Spray
      case 9:
        edit_mode_9();
        break; // Fill
      case 10:
        edit_mode_10();
        break; // Cut
      case 11:
        edit_mode_11();
        break; // Undo
      case 12:
        edit_mode_12();
        break; // Points
      case 13:
        edit_mode_13();
        break; // Text
      case 90: // Paste graphic
        sp_an = window[1].mapa->map_width;
        sp_al = window[1].mapa->map_height;
        if ((sp = (byte *)malloc(sp_an * sp_al)) != NULL) {
          memcpy(sp, window[1].mapa->map, sp_an * sp_al);
          move_selection(sp, sp_an, sp_al);
        };
        dragging = 5;
        break;
      default:
        edit_scr();
        break;
      }
      if (draw_mode < 100) {
        put_bar(2, 10, 45);
      }
      blit_mouse();
      do {
        read_mouse();
      } while (mouse_b || key(_ESC));
    } while (draw_mode >= 100);
    mouse_graf = 1;

    free(selection_mask);
    selection_mask = NULL;

    exploding_windows = ew;
    if (TipoTex & BRUSH)
      fclose(FilePaintFPG);
    FreePaintThumbs();
    TipoTex = 0;
    TipoBrowser = 0;

    if (draw_mode < 90)
      draw_mode += 100;
    else
      draw_mode = old_draw_mode;

    if (highlight_background) {
      highlight_background = 0;
      dac[0] = (dac[0] + 32) % 64;
      dac[1] = (dac[1] + 32) % 64;
      dac[2] = (dac[2] + 32) % 64;
      set_dac(dac);
    }

    v.mapa->zoom = zoom;
    v.mapa->zoom_x = zoom_x;
    v.mapa->zoom_y = zoom_y;
    v.mapa->zoom_cx = zoom_cx;
    v.mapa->zoom_cy = zoom_cy;
    for (n = 1; n < max_windows; n++)
      if (hidden[n])
        window[n].foreground = 1;
    v.foreground = 1;
    v.state = 1;
    v.type = 100;

    calculate_foreground();

    wgra(v.ptr, an, al, c_b_low, 2, 2, an - 20, 7);
    if (text_len(v.title) + 3 > an - 20) {
      wwrite_in_box(v.ptr, an, an - 19, al, 4, 2, 0, v.title, c1);
      wwrite_in_box(v.ptr, an, an - 19, al, 3, 2, 0, v.title, c4);
    } else {
      wwrite(v.ptr, an, al, 3 + (an - 20) / 2, 2, 1, v.title, c1);
      wwrite(v.ptr, an, al, 2 + (an - 20) / 2, 2, 1, v.title, c4);
    }

    mapa1();
    update_box(0, 0, vga_width, vga_height);
    if (undo_index != _undo_index)
      v.mapa->saved = 0;

    n = 0;
    while (n < num_windows_to_create) {
      v_map = windows_to_create[n++];
      new_window(mapa0);
    }
  }
}

void mapa3(void) {
  free(v.mapa->map);
  free(v.mapa);
}

///////////////////////////////////////////////////////////////////////////////
//      Palette window
///////////////////////////////////////////////////////////////////////////////

#define pixelxcolor 4

void paleta0(void) {
  v.type = 3; // Palette
  v.an = 16 * pixelxcolor + 3;
  v.al = 16 * pixelxcolor + 11;
  v.title = texts[51];
  v.name = texts[51];
  v.paint_handler = paleta1;
}

void paleta1(void) {
  int x, y;
  int an = v.an, al = v.al;
  if (big) {
    an /= 2;
    al /= 2;
  }
  for (y = 0; y < 16; y++)
    for (x = 0; x < 16; x++)
      wbox(v.ptr, an, al, x + y * 16, 2 + x * pixelxcolor, 10 + y * pixelxcolor, pixelxcolor - 1,
           pixelxcolor - 1);
}

///////////////////////////////////////////////////////////////////////////////
//      Dialog box to enter information about the user
///////////////////////////////////////////////////////////////////////////////

char user1[128] = "";
char user2[128] = "";

void usuario1(void) {
  int an = v.an / big2, al = v.al / big2;

  _show_items();
  wput(v.ptr, an, al, 5, 10, -50);

  wwrite(v.ptr, an, al, 91, 13, 0, texts[462], c1);
  wwrite(v.ptr, an, al, 90, 13, 0, texts[462], c4);
  wwrite(v.ptr, an, al, 91, 12 + 38 + 11, 0, texts[484], c1);
  wwrite(v.ptr, an, al, 90, 12 + 38 + 11, 0, texts[484], c4);
}

void usuario2(void) {
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

void usuario3(void) {
  FILE *f;

  if (strlen(user1) || strlen(user2)) {
    if ((f = fopen("system/user.nfo", "wb")) != NULL) {
      fwrite(user1, 1, 128, f);
      fwrite(user2, 1, 128, f);
      fclose(f);
    }
  }
}

void usuario0(void) {
  v.type = 1;
  v.title = texts[485];
  v.an = 160 + 86;
  v.al = 10 + 47 + 31;

  v.paint_handler = usuario1;
  v.click_handler = usuario2;
  v.close_handler = usuario3;

  _button(100, v.an - 8, v.al - 14, 2);

  _get(460, 4 + 86, 22, v.an - 8 - 86, (byte *)user1, 127, 0, 0);
  _get(461, 4 + 86, 22 + 19, v.an - 8 - 86, (byte *)user2, 127, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
//      Copyright Dialog Box
///////////////////////////////////////////////////////////////////////////////

extern int nueva_sesion;

void copyright0(void) {
  v.type = 1;
  v.title = texts[35];
  v.an = 232;
  v.al = 12 + 47 + 31;
  v.paint_handler = copyright1;
  v.click_handler = copyright2;
  if (CopyDesktop) {
    _button(456, v.an - 11, v.al - 16, 2);   // Continue
    _button(457, 48 + 48 + 3, v.al - 16, 0); // New session
  } else {
    _button(457, v.an - 11, v.al - 16, 2); // New session
  }
}

void copyright1(void) {
  int an = v.an / big2, al = v.al / big2;

  _show_items();
  wput(v.ptr, an, al, 6, 11, -50);

  wrectangle(v.ptr, an, al, c1, 48 + 48, 23, 129, 19);
  wbox(v.ptr, an, al, c12, 48 + 48 + 1, 23 + 1, 129 - 2, 19 - 2);

  wwrite(v.ptr, an, al, 48 + 48 + 1, 14, 0, texts[485], c1);
  wwrite(v.ptr, an, al, 48 + 48, 14, 0, texts[485], c4);

#ifndef SHARE
  wwrite(v.ptr, an, al, 48 + 48, 45, 0, texts[36], c12);
#else
  wwrite(v.ptr, an, al, 48 + 48, 45, 0, texts[37], c12);
#endif

  wwrite(v.ptr, an, al, 96, 45 + 7, 0, texts[486], c12);

  // User info:

  wwrite_in_box(v.ptr, an, an - 9, al, 99, 25, 0, (byte *)user1, c1);
  wwrite_in_box(v.ptr, an, an - 9, al, 98, 25, 0, (byte *)user1, c23);
  wwrite_in_box(v.ptr, an, an - 9, al, 99, 25 + 8, 0, (byte *)user2, c1);
  wwrite_in_box(v.ptr, an, an - 9, al, 98, 25 + 8, 0, (byte *)user2, c23);

  wwrite(v.ptr, an, al, 48 + 48 + 1, 61, 0, texts[484], c1);
  wwrite(v.ptr, an, al, 48 + 48, 61, 0, texts[484], c4);
}

void copyright2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
  if (v.active_item == 1) {
    nueva_sesion = 1;
    end_dialog = 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
//      ERROR dialog box
///////////////////////////////////////////////////////////////////////////////

void err0(void) {
  v.type = 1;
  v.title = texts[41];
  v.an = text_len((byte *)v_text) + 8;
  v.al = 38; // Note: error_window=malloc(640*38*2)
  if (v.an > 320)
    v.an = 320;
  v.paint_handler = err1;
  v.click_handler = err2;
  _button(100, v.an / 2, v.al - 14, 1);
}

void err1(void) {
  int an = v.an / big2, al = v.al / big2;
  _show_items();
  wwrite(v.ptr, an, al, 4, 12, 0, (byte *)v_text, c3);
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
  int an = v.an / big2, al = v.al / big2;
  _show_items();
  wwrite(v.ptr, an, al, 4, 12, 0, (byte *)v_text, c3);
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
  v.type = 1;
  v.title = texts[41];
  v.an = text_len((byte *)v_text) + 8;
  v.al = 38; // Note: error_window=malloc(640*38*2)
  if (v.an > 320)
    v.an = 320;
  v.paint_handler = errhlp1;
  v.click_handler = errhlp2;
  v_accept = 0;
  _button(100, 7, v.al - 14, 0);
  _button(125, v.an - 8, v.al - 14, 2);
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
  int an = v.an / big2, al = v.al / big2;
  _show_items();
  wwrite(v.ptr, an, al, 5, 12, 0, (byte *)v_text, c1);
  wwrite(v.ptr, an, al, 4, 12, 0, (byte *)v_text, c3);
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
  v.type = 1;
  v.title = texts[384];
  v_text = (char *)texts[383];
  v.an = text_len((byte *)v_text) + 9;
  v.al = 38;
  v.paint_handler = test1;
  v.click_handler = test2;
  _button(100, v.an / 2, v.al - 14, 1);

  exit_requested = 1;
  return_mode = 3;
  old_system_clock = *system_clock;
}

//-----------------------------------------------------------------------------
//      OK / Cancel Dialog with or without title text
//-----------------------------------------------------------------------------

void aceptar0(void) {
  v.type = 1;
  v.an = text_len(texts[100]) + text_len(texts[101]) + 24;
  if (v_title != NULL) {
    v.title = (byte *)v_title;
    if (text_len((byte *)v_title) + 14 > v.an)
      v.an = text_len((byte *)v_title) + 14;
  }
  if (v_text != NULL) {
    v.al = 38;
    if (text_len((byte *)v_text) + 6 > v.an)
      v.an = text_len((byte *)v_text) + 6;
  } else
    v.al = 29;
  v.paint_handler = aceptar1;
  v.click_handler = aceptar2;
  _button(100, 7, v.al - 14, 0);
  _button(101, v.an - 8, v.al - 14, 2);
  v_accept = 0;
}

void aceptar1(void) {
  int an = v.an / big2, al = v.al / big2;
  _show_items();
  if (v_text != NULL)
    wwrite(v.ptr, an, al, an / 2, 12, 1, (byte *)v_text, c3);
}

void aceptar2(void) {
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

void TratarPaleta1(void);
void TratarPaleta2(void);
void TratarPaleta3(void);
int OpcPal[3];
byte paltratar[768];

void TratarPaleta0(void) {
  v.type = 1;
  v.an = 160;
  v.al = 59 + 77;

  v.title = (byte *)(v_title = (char *)texts[568]);

  v.paint_handler = TratarPaleta1;
  v.click_handler = TratarPaleta2;
  v.close_handler = TratarPaleta3;

  OpcPal[0] = 1;
  OpcPal[1] = 0;
  OpcPal[2] = 0;

  _button(100, 7, v.al - 14, 0);
  _button(101, v.an - 8, v.al - 14, 2);
  _flag(569, 4, 12 + 77, &OpcPal[0]);
  _flag(570, 4, 22 + 77, &OpcPal[1]);
  _flag(571, 4, 32 + 77, &OpcPal[2]);

  v_accept = 0;
}

byte fast_find_color(byte fr, byte fg, byte fb);
byte find_color(byte r, byte g, byte b);

void TratarPaleta1(void) {
  int an = v.an / big2, al = v.al / big2;
  int xx = 91, x, y;
  byte c;
  byte d4[768];

  _show_items();

  wbox(v.ptr, an, al, c0, 4, 12, 65, 65 + 10);
  wbox(v.ptr, an, al, c0, xx, 12, 65, 65 + 10);

  wbox(v.ptr, an, al, c1, 5, 13, 63, 9);
  wbox(v.ptr, an, al, c1, xx + 1, 13, 63, 9);

  if (OpcPal[1]) {
    wrectangle(v.ptr, an, al, c0, 4, 12, an - 8, 11);
    wbox(v.ptr, an, al, c12, 5, 13, an - 10, 9);
    wwrite(v.ptr, an, al, an / 2 + 1, 14, 1, (byte *)"+", c0);
    wwrite(v.ptr, an, al, an / 2, 14, 1, (byte *)"+", c4);
  }

  if (OpcPal[0] || OpcPal[1]) {
    wbox(v.ptr, an, al, c12, 5, 13, 63, 9);
    wwrite(v.ptr, an, al, 4 + 33, 14, 1, texts[576], c0);
    c = c4;
  } else
    c = c3;
  wwrite(v.ptr, an, al, 4 + 32, 14, 1, texts[576], c);

  if (OpcPal[1] || OpcPal[2]) {
    wbox(v.ptr, an, al, c12, xx + 1, 13, 63, 9);
    wwrite(v.ptr, an, al, xx + 33, 14, 1, texts[577], c0);
    c = c4;
  } else
    c = c3;
  wwrite(v.ptr, an, al, xx + 32, 14, 1, texts[577], c);

  memcpy(d4, dac4, 768);
  create_dac4();

  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      wbox(v.ptr, an, al, x + y * 16, 5 + x * 4, 23 + y * 4, 3, 3);
      c = fast_find_color(paltratar[(x + y * 16) * 3 + 0], paltratar[(x + y * 16) * 3 + 1],
                          paltratar[(x + y * 16) * 3 + 2]);
      wbox(v.ptr, an, al, c, xx + 1 + x * 4, 23 + y * 4, 3, 3);
    }
  }

  memcpy(dac4, d4, 768);
}

void TratarPaleta2(void) {
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
    OpcPal[0] = 1;
    OpcPal[1] = 0;
    OpcPal[2] = 0;
    need_refresh = 1;
    break;
  case 3:
    OpcPal[0] = 0;
    OpcPal[1] = 1;
    OpcPal[2] = 0;
    need_refresh = 1;
    break;
  case 4:
    OpcPal[0] = 0;
    OpcPal[1] = 0;
    OpcPal[2] = 1;
    need_refresh = 1;
    break;
  }

  if (need_refresh) {
    call((voidReturnType)v.paint_handler);
    v.redraw = 1;
  }
}
void TratarPaleta3(void) {
  if (v_accept && OpcPal[1])
    v_accept = 2;
  if (v_accept && OpcPal[2])
    v_accept = 3;
}

//-----------------------------------------------------------------------------
//      Lists for the file open window
//-----------------------------------------------------------------------------

char archivo[max_archivos * an_archivo];
struct t_listbox larchivos = {3, 49, archivo, an_archivo, 12, 64};

char directorio[max_directorios * an_directorio];
struct t_listbox ldirectorios = {80, 49, directorio, an_directorio, 10, 64};

#define max_unidades 26
#define an_unidad    (4 + 1)
char unidad[max_unidades * an_unidad];
struct t_listbox lunidades = {157, 49, unidad, an_unidad, 4, 28};

#define max_ext 26
#define an_ext  (5 + 1)
char ext[max_ext * an_ext];
struct t_listbox lext = {157, 97, ext, an_ext, 4, 28};

///////////////////////////////////////////////////////////////////////////////
//      Draw listbox and slider elements
///////////////////////////////////////////////////////////////////////////////

void paint_listbox(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int an = v.an, al = v.al;
  int n, y;
  char *p;
  if (big) {
    an /= 2;
    al /= 2;
  }

  wbox(ptr, an, al, c1, l->x + 1, l->y + 1, l->an - 2, l->al - 2);

  if ((n = l->total_items)) { // If there are items in the list
    n -= l->first_visible;
    p = l->list + l->item_width * l->first_visible;
    y = l->y + 2;
    if (n > l->visible_items)
      n = l->visible_items;
    do {
      wwrite_in_box(ptr + (l->x + 2) * big2, an, l->an - 4, al, 0, y, 0, (byte *)p, c3);
      p += l->item_width;
      y += 8;
    } while (--n);
  }

  if (wmouse_in(l->x, l->y, l->an - 1, l->al)) { // Calculate zone
    l->zone = ((mouse_y - v.y) - (l->y + 2) * big2) / (8 * big2);
    if (l->total_items <= l->zone || l->zone >= l->visible_items)
      l->zone = 1;
    else
      l->zone += 10;
  } else if (wmouse_in(l->x + l->an - 1, l->y, 9, 9))
    l->zone = 2;
  else if (wmouse_in(l->x + l->an - 1, l->y + l->al - 9, 9, 9))
    l->zone = 3;
  else if (wmouse_in(l->x + l->an - 1, l->y + 9, 9, l->al - 18))
    l->zone = 4;
  else
    l->zone = 0;

  if (l->zone >= 10) { // Highlight zone
    wwrite_in_box(ptr + (l->x + 2) * big2, an, l->an - 4, al, 0, l->y + 2 + (l->zone - 10) * 8, 0,
                  (byte *)l->list + l->item_width * (l->first_visible + l->zone - 10), c4);
    v.redraw = 1;
  }
}

void paint_slider(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int an = v.an, al = v.al;
  if (big) {
    an /= 2;
    al /= 2;
  }

  wbox(ptr, an, al, c2, l->x + l->an, l->y + 9, 7, l->al - 18);
  if (l->slide > l->s0)
    wbox(ptr, an, al, c0, l->x + l->an, l->slide - 1, 7, 1);
  if (l->slide < l->s1)
    wbox(ptr, an, al, c0, l->x + l->an, l->slide + 3, 7, 1);
  wput(ptr, an, al, l->x + l->an, l->slide, 43);
}

//-----------------------------------------------------------------------------
//      Draw listbox
//-----------------------------------------------------------------------------

void create_listbox(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int an = v.an, al = v.al;
  if (big) {
    an /= 2;
    al /= 2;
  }

  if (!l->created) {
    if (!l->an)
      l->an = (l->item_width - 1) * 6 + 3;
    if (!l->al)
      l->al = l->visible_items * 8 + 3;
    l->slide = l->s0 = l->y + 9;
    l->s1 = l->y + l->al - 12;
    l->buttons = 0;
    l->created = 1;
    l->zone = 0;
    if (l == &larchivos) {
      l->first_visible = tipo[v_type].first_visible;
      if ((l->first_visible + l->visible_items) > l->total_items) {
        l->first_visible = 0;
      }
    } else
      l->first_visible = 0;
  }

  wrectangle(ptr, an, al, c0, l->x, l->y, l->an, l->al);
  wbox(ptr, an, al, c1, l->x + 1, l->y + 1, l->an - 2, l->al - 2);
  wrectangle(ptr, an, al, c0, l->x + l->an - 1, l->y, 9, l->al);
  wrectangle(ptr, an, al, c0, l->x + l->an - 1, l->y + 8, 9, l->al - 16);
  wput(ptr, an, al, l->x + l->an, l->y + 1, -39);
  wput(ptr, an, al, l->x + l->an, l->y + l->al - 8, -40);

  paint_listbox(l);
  paint_slider(l);
}

//-----------------------------------------------------------------------------
//      Update a listbox, depending on mouse movements
//-----------------------------------------------------------------------------

void update_listbox(struct t_listbox *l) {
  byte *ptr = v.ptr;
  int an = v.an / big2, al = v.al / big2;
  int n, old_zona = l->zone;

  if (!l->al)
    l->al = l->visible_items * 8 + 3;

  if (wmouse_in(l->x, l->y, l->an - 1, l->al)) { // Calculate zone
    l->zone = ((mouse_y - v.y) - (l->y + 2) * big2) / (8 * big2);
    if (l->total_items <= l->zone || l->zone >= l->visible_items)
      l->zone = 1;
    else
      l->zone += 10;
  } else if (wmouse_in(l->x + l->an - 1, l->y, 9, 9))
    l->zone = 2;
  else if (wmouse_in(l->x + l->an - 1, l->y + l->al - 9, 9, 9))
    l->zone = 3;
  else if (wmouse_in(l->x + l->an - 1, l->y + 9, 9, l->al - 18))
    l->zone = 4;
  else
    l->zone = 0;

  if (old_zona != l->zone)
    if (old_zona >= 10) { // Unhighlight zone
      wwrite_in_box(ptr + (l->x + 2) * big2, an, l->an - 4, al, 0, l->y + 2 + (old_zona - 10) * 8,
                    0, (byte *)l->list + l->item_width * (l->first_visible + old_zona - 10), c3);
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
    wput(ptr, an, al, l->x + l->an, l->y + 1, -41);
    l->buttons |= 1;
    v.redraw = 1;
  } else if (l->buttons & 1) {
    wput(ptr, an, al, l->x + l->an, l->y + 1, -39);
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
    wput(ptr, an, al, l->x + l->an, l->y + l->al - 8, -42);
    l->buttons |= 2;
    v.redraw = 1;
  } else if (l->buttons & 2) {
    wput(ptr, an, al, l->x + l->an, l->y + l->al - 8, -40);
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
      wwrite_in_box(ptr + (l->x + 2) * big2, an, l->an - 4, al, 0, l->y + 2 + (l->zone - 10) * 8, 0,
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
extern struct t_listboxbr larchivosbr;
extern struct t_listbox ldirectoriosbr;
extern t_thumb thumb[max_archivos];
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
        getcwd(tipo[v_type].path, PATH_MAX + 1);
        print_path_br();

        larchivos.created = 0;
        ldirectorios.created = 0;
        tipo[v_type].first_visible = 0;
        open_dir_br();
        create_listbox(&larchivos);
        create_listbox(&ldirectorios);
      } else
        _dos_setdrive(tipo[v_type].path[0] - 'A' + 1, &n);
    }
  } else
    div_strcpy(input, sizeof(input), file_mask);

  call((voidReturnType)v.paint_handler);
  v.redraw = 1;
}

//-----------------------------------------------------------------------------
//      New map dialog
//-----------------------------------------------------------------------------

#define y_nm 67
char mapsizes[9 * 9] =
    "320x200\0 320x240\0 360x240\0 360x360\0 376x282\0 640x400\0 640x480\0 800x600\0 1024x768";
struct t_listbox lmapsizes = {64, 19, mapsizes, 9, 5, 50};
char mancho[8], malto[8];

void nuevo_mapa0(void) {
  v.type = 1;
  v.title = texts[132];
  v.an = 126;
  v.al = 14 + y_nm;
  v.paint_handler = nuevo_mapa1;
  v.click_handler = nuevo_mapa2;
  lmapsizes.created = 0;
  lmapsizes.total_items = 9;
  map_width = vga_width;
  map_height = vga_height;
  itoa(map_width, mancho, 10);
  itoa(map_height, malto, 10);
  _get(133, 4, 12, v.an - 72, (byte *)mancho, 8, 1, 32767);
  _get(134, 4, 34, v.an - 72, (byte *)malto, 8, 1, 32767);
  _button(100, 7, y_nm, 0);
  _button(101, v.an - 8, y_nm, 2);
  v_finished = 0;
}

void nuevo_mapa1(void) {
  int an = v.an / big2, al = v.al / big2;
  _show_items();
  wwrite(v.ptr, an, al, 64, 12, 0, texts[135], c3);
  create_listbox(&lmapsizes);
}

void nuevo_mapa2(void) {
  int n;

  _process_items();
  update_listbox(&lmapsizes);

  if ((mouse_b & 1) && !(prev_mouse_buttons & 1)) {
    if (lmapsizes.zone >= 10) {
      n = lmapsizes.zone - 10 + lmapsizes.first_visible;
      *strchr(&mapsizes[n * 9], 'x') = 0;
      map_width = atoi(&mapsizes[n * 9]);
      map_height = atoi(&mapsizes[n * 9 + strlen(&mapsizes[n * 9]) + 1]);
      mapsizes[n * 9 + strlen(&mapsizes[n * 9])] = 'x';
      itoa(map_width, mancho, 10);
      itoa(map_height, malto, 10);
      call((voidReturnType)v.paint_handler);
      update_listbox(&lmapsizes);
      _select_new_item(2);
      v.redraw = 1;
    }
  }

  switch (v.active_item) {
  case 2:
    end_dialog = 1;
    if (atoi(mancho) > 0 && atoi(mancho) < 32768)
      map_width = atol(mancho);
    if (atoi(malto) > 0 && atoi(malto) < 32768)
      map_height = atol(malto);
    v_finished = 1;
    break;
  case 3:
    end_dialog = 1;
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//      Create a new map (return 1 on error)
///////////////////////////////////////////////////////////////////////////////

int new_map(byte *mapilla) {
  int n;

  // 1. Allocate memory for the tmapa struct
  if ((v_map = (struct tmapa *)malloc(sizeof(struct tmapa))) != NULL) {
    // 2. Allocate memory for the map

    if (mapilla != NULL)
      v_map->map = mapilla;
    else
      v_map->map = (byte *)malloc(map_width * map_height);

    if (v_map->map != NULL) {
      if (mapilla == NULL) {
        // 2b. Clear the map to zero
        memset(v_map->map, 0, map_width * map_height);
      }

      // 4. Set the remaining variables
      div_strcpy(v_map->filename, sizeof(v_map->filename), (char *)texts[136]);
      ltoa(next_code++, v_map->filename + strlen(v_map->filename), 10);
      *v_map->path = '\0';
      v_map->map_width = map_width;
      v_map->map_height = map_height;
      v_map->TengoNombre = 0;
      v_map->fpg_code = 0;
      v_map->description[0] = 0;
      for (n = 0; n < 512; n++)
        v_map->puntos[n] = -1;
      if (MustCreate)
        new_window(mapa0);

      return (0);

    } else {
      v_text = (char *)texts[45];
      show_dialog(err0);
      free(v_map);
    }

  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }

  return (1);
}

//-----------------------------------------------------------------------------
//     Open a map disc content
//-----------------------------------------------------------------------------

int hay_mapas(void);
extern int cargar_paleta;

void open_map(void) {
  FILE *f;
  byte *ptr;
  int x, sum;
  int n, tipomapa;
  byte *buffer;
  byte pal[768];
  byte palorg[768];
  byte xlat[256];
  int num, div_try;

  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), tipo[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    if ((f = fopen(full, "rb")) != NULL) {
      fclose(f);
      v_exists = 1;
    } else
      v_exists = 0;
    div_strcpy(larchivosbr.list, larchivosbr.item_width, input);
    larchivosbr.total_items = 1;
    thumb[0].tagged = 1;
    num_taggeds = 1;
  }

  n = 0; // Number of distinct palettes
  muestra = NULL;
  memcpy(pal, dac, 768);
  memcpy(palorg, original_palette, 768);
  memset(original_palette, 0, 768);

  for (num = 0; num < larchivosbr.total_items; num++) {
    if (thumb[num].tagged) {
      div_try = 0;
      div_strcpy(input, sizeof(input), larchivosbr.list + larchivosbr.item_width * num);
      div_strcpy(full, sizeof(full), tipo[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);

      cargar_paleta = 1;
      div_try |= fmt_load_dac_pcx(full);
      if (!div_try)
        div_try |= fmt_load_dac_bmp(full);
      if (!div_try)
        div_try |= fmt_load_dac_map(full);
      if (!div_try)
        div_try |= fmt_load_dac_jpg(full);
      cargar_paleta = 0;

      if (div_try) {
        if (n++ == 0) {
          memcpy(pal, dac4, 768);
        } else {
          x = 0;
          sum = 0;
          do {
            sum += abs((int)pal[x] - (int)dac4[x]);
          } while (++x < 768);
          if (sum) {
            if (muestra == NULL) {
              muestra = (byte *)malloc(32768);
              if (muestra == NULL)
                continue;
              memset(muestra, 0, 32768);
              for (x = 0; x < 256; x++) {
                muestra[((pal[x * 3 + 0] & 0xFE) << 9) | ((pal[x * 3 + 1] & 0xFE) << 4) |
                        (pal[x * 3 + 2] >> 1)] = 1;
              }
            }
            for (x = 0; x < 256; x++) {
              muestra[((dac4[x * 3 + 0] & 0xFE) << 9) | ((dac4[x * 3 + 1] & 0xFE) << 4) |
                      (dac4[x * 3 + 2] >> 1)] = 1;
            }
          }
        }
      }
    }
  }

  if (muestra != NULL) {
    crear_paleta();
    free(muestra);
    memcpy(pal, apply_palette, 768);
  }

  // pal[] now contains the palette of the maps to load

  x = 0;
  sum = 0;
  do {
    sum += abs((int)pal[x] - (int)dac[x]);
  } while (++x < 768);
  memcpy(paltratar, pal, 768);

  if (sum && n == 1) {
    x = 0;
    sum = 0;
    do {
      sum += abs((int)original_palette[x] - (int)dac[x]);
    } while (++x < 768);
    memcpy(paltratar, original_palette, 768);
  }

  if (sum) {
    show_dialog(TratarPaleta0); // Load palette?

    switch (v_accept) {
    case 0: // Cancel (don't load)
      memcpy(original_palette, palorg, 768);
      return;
    case 1: // Adapt maps to the system palette
      break;
    case 2: // Merge palettes
      memcpy(dac4, pal, 768);
      fusionar_paletas();
      pal_refresh(0, 1);
      break;
    case 3: // Activate the new palette
      if (muestra == NULL)
        memcpy(pal, original_palette, 768);
      memcpy(dac4, pal, 768);
      pal_refresh(0, 1);
      break;
    }
  }
  memcpy(original_palette, palorg, 768);


  for (num = 0; num < larchivosbr.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), larchivosbr.list + larchivosbr.item_width * num);
      div_strcpy(full, sizeof(full), tipo[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);

      if ((f = fopen(full, "rb")) != NULL) { // A file was selected
        fseek(f, 0, SEEK_END);
        n = ftell(f);
        if ((buffer = (byte *)malloc(n)) != NULL) {
          fseek(f, 0, SEEK_SET);
          if (fread(buffer, 1, n, f) == n) {
            if (fmt_is_map(buffer))
              tipomapa = 1;
            else if (fmt_is_pcx(buffer))
              tipomapa = 2;
            else if (fmt_is_bmp(buffer))
              tipomapa = 3;
            else if (fmt_is_jpg(buffer, n))
              tipomapa = 4;
            else
              tipomapa = 0;

            if (tipomapa) {
              if ((v_map = (struct tmapa *)malloc(sizeof(struct tmapa))) != NULL) {
                memset(v_map, 0, sizeof(struct tmapa));

                if ((v_map->map = (byte *)malloc(map_width * map_height + map_width)) != NULL) {
                  v_map->TengoNombre = 0; // No description by default

                  for (x = 0; x < 512; x++)
                    v_map->puntos[x] = -1;

                  x = 1;
                  switch (tipomapa) {
                  case 1:
                    fmt_load_map(buffer, v_map->map, 1);
                    break;
                  case 2:
                    fmt_load_pcx(buffer, v_map->map, 1);
                    break;
                  case 3:
                    fmt_load_bmp(buffer, v_map->map, 1);
                    break;
                  case 4:
                    x = fmt_load_jpg(buffer, v_map->map, 1, n);
                    break;
                  }

                  if (!x) {
                    free(v_map->map);
                    free(v_map);
                    free(buffer);
                    fclose(f);
                    v_text = (char *)texts[44];
                    show_dialog(err0);
                    continue;
                  }

                  // NOTE: fmt_load_map/PCX/BMP return void, so errors cannot be checked here.
                  // Only fmt_load_jpg returns a status (already handled above).

                  ExternUseBufferMap = (char *)v_map->map;

                  x = 0;
                  sum = 0;
                  do {
                    sum += abs((int)dac4[x] - (int)dac[x]);
                  } while (++x < 768);

                  if (sum) {
                    memcpy(pal, dac4, 768);
                    create_dac4();
                    for (x = 0; x < 256; x++)
                      xlat[x] = find_color(pal[x * 3], pal[x * 3 + 1], pal[x * 3 + 2]);
                    ptr = v_map->map;
                    sum = map_width * map_height;
                    x = 0;
                    do {
                      *ptr = xlat[*ptr];
                      ptr++;
                    } while (++x < sum);
                  }

                  // Set path\filename, map_width/al and create the new window

                  div_strcpy(v_map->filename, sizeof(v_map->filename), input);
                  div_strcpy(v_map->path, sizeof(v_map->path), tipo[v_type].path);
                  memcpy(v_map->description, MapDescription, 32);
                  v_map->TengoNombre = 0;
                  v_map->fpg_code = fpg_code;
                  if (fpg_code)
                    v_map->TengoNombre = 1;
                  v_map->map_width = map_width;
                  v_map->map_height = map_height;

                  new_window(mapa0);

                } else {
                  v_text = (char *)texts[45];
                  show_dialog(err0);
                  free(v_map);
                }
              } else {
                v_text = (char *)texts[45];
                show_dialog(err0);
              }
            } else {
              v_text = (char *)texts[46];
              show_dialog(err0);
            }
          } else {
            v_text = (char *)texts[44];
            show_dialog(err0);
          }
          free(buffer);
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
  }
  create_dac4();
}

// abrir_mapa3d removed (MODE8/3D map editor deleted)

//-----------------------------------------------------------------------------
//      Save a map
//
// Input: tipo[v_type].path, input
//        map, dac, map_width, map_height
//        v_window
//-----------------------------------------------------------------------------

void save_map(void) {
  int e, tipomapa;
  int an = window[v_window].an, al = window[v_window].al;
  char filename[255];
  FILE *f;

  if (big) {
    an /= 2;
    al /= 2;
  }


  div_strcpy(full, sizeof(full), tipo[v_type].path);
  if (full[strlen(full) - 1] != '/')
    div_strcat(full, sizeof(full), "/");
  div_strcat(full, sizeof(full), input);

  if (strchr(input, ' ') == NULL) {
    if ((f = fopen(full, "wb")) != NULL) { // A file was selected
      div_strcpy(filename, sizeof(filename), input);
      strupr(filename);

      if (!strcmp(strchr(filename, '.'), ".PCX"))
        tipomapa = 1;
      else if (!strcmp(strchr(filename, '.'), ".BMP"))
        tipomapa = 2;
      else

        tipomapa = 0;

      switch (tipomapa) {
      case 0:
        e = fmt_save_map(map, f);
        break;
      case 1:
        e = fmt_save_pcx(map, f);
        break;
      case 2:
        e = fmt_save_bmp(map, f);
        break;
      }
      if (e == 2) {
        v_text = (char *)texts[48];
        show_dialog(err0);
      }

      if (e || ferror(f)) {
        fclose(f);
        remove(full); // Delete partial file on write failure
      } else {
        fclose(f);
      }

    } else {
      v_text = (char *)texts[47];
      show_dialog(err0);
      e = 1;
    }
  } else {
    v_text = (char *)texts[47];
    show_dialog(err0);
    e = 1;
  }

  if (!e) { // Update the path and name of the saved map/window
    div_strcpy(window[v_window].mapa->path, sizeof(window[v_window].mapa->path), tipo[v_type].path);
    div_strcpy(window[v_window].mapa->filename, sizeof(window[v_window].mapa->filename), input);
    wgra(window[v_window].ptr, an, al, c_b_low, 2, 2, an - 20, 7);
    if (text_len(window[v_window].title) + 3 > an - 20) {
      wwrite_in_box(window[v_window].ptr, an, an - 19, al, 4, 2, 0, window[v_window].title, c1);
      wwrite_in_box(window[v_window].ptr, an, an - 19, al, 3, 2, 0, window[v_window].title, c4);
    } else {
      wwrite(window[v_window].ptr, an, al, 3 + (an - 20) / 2, 2, 1, window[v_window].title, c1);
      wwrite(window[v_window].ptr, an, al, 2 + (an - 20) / 2, 2, 1, window[v_window].title, c4);
    }
    flush_window(v_window);
  }
}

//-----------------------------------------------------------------------------
//      Create a new downscaled map by rendering and optionally convert to grayscale
//      Used to create a hardness map
//-----------------------------------------------------------------------------
char cAncho[5];
char cAlto[5];
int Gris = 0;
int Porcentajes = 0;

void Tamnio1() {
  int an = v.an / big2, al = v.al / big2, n;
  char cWork[5];
  n = find_and_load_map();
  if (!n)
    return;
  _show_items();
  wwrite(v.ptr, an, al, 4, 11, 0, texts[133], c3);
  div_snprintf(cWork, sizeof(cWork), "%d", window[n].mapa->map_width);
  wwrite(v.ptr, an, al, 4, 20, 0, (byte *)cWork, c3);

  wwrite(v.ptr, an, al, 4, 30, 0, texts[134], c3);
  div_snprintf(cWork, sizeof(cWork), "%d", window[n].mapa->map_height);
  wwrite(v.ptr, an, al, 4, 39, 0, (byte *)cWork, c3);
  v_accept = 0;
}
void Tamnio2() {
  _process_items();
  switch (v.active_item) {
  case 4:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 5:
    v_accept = 0;
    end_dialog = 1;
    break;
  }
}
void Tamnio3() {
  map_width = atoi(cAncho);
  map_height = atoi(cAlto);
}

void Tamnio0() {
  v.type = 1; // Window type 1 = dialog

  v.title = texts[63];
  v.an = 126 + 50;
  v.al = 49 + 18;
  v.paint_handler = Tamnio1;
  v.click_handler = Tamnio2;
  v.close_handler = Tamnio3;

  div_snprintf(cAncho, sizeof(cAncho), "%d", map_width);
  div_snprintf(cAlto, sizeof(cAlto), "%d", map_height);

  _get(133, 42, 11, 30, (byte *)cAncho, 5, 1, 9999);
  _get(134, 42, 30, 30, (byte *)cAlto, 5, 1, 9999);
  Porcentajes = 0;
  Gris = 0;
  _flag(95, 90, 19, &Porcentajes);
  _flag(96, 90, 38, &Gris);
  _button(100, 7, v.al - 14, 0);
  _button(101, v.an - 8, v.al - 14, 2);
}

void Reducex2() {
  char *bufferaux;
  int n, p1, p2, p3, p4, man, mal, c;
  float x, y;
  struct tmapa *MiMap;
  float incx, incy;
  int fx, fy;
  char Tabla[190];
  int Colors[9], min_dist, i, dist;
  n = find_and_load_map();
  if (!n)
    return;
  MiMap = window[n].mapa;

  if ((MiMap->map_width < 2) || (MiMap->map_height < 2))
    return;

  man = MiMap->map_width;
  mal = MiMap->map_height;

  show_dialog(Tamnio0);
  if (!v_accept)
    return;

  if (Porcentajes) {
    map_width = (map_width * man) / 100;
    map_height = (map_height * mal) / 100;
  }

  if (map_width > man || map_height > mal) {
    AplieX(MiMap, man, mal);
    return;
  }

  // Generate grayscale table
  if (Gris) {
    Colors[0] = c0;
    Colors[2] = c1;
    Colors[4] = c2;
    Colors[6] = c3;
    Colors[8] = c4;
    Colors[1] = average_color(c0, c1);
    Colors[3] = average_color(c1, c2);
    Colors[5] = average_color(c2, c3);
    Colors[7] = average_color(c3, c4);

    for (n = 0; n < 190; n++) {
      min_dist = 192;
      for (i = 0; i < 9; i++) {
        c = (memptrsize)Colors[i] * 3;
        dist = dac[c] + dac[c + 1] + dac[c + 2];
        if (dist > n)
          dist = dist - n;
        else
          dist = n - dist;
        if (dist < min_dist) {
          Tabla[n] = c / 3;
          min_dist = dist;
        }
      }
    }
  }
  // End grayscale table
  incy = (float)mal / (float)map_height;
  incx = (float)man / (float)map_width;
  bufferaux = (char *)malloc(map_width * map_height);

  if (bufferaux == NULL) { // Added by Dani ...
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  y = (float)0;
  for (fy = 0; fy < map_height; fy++) {
    Progress((char *)texts[92], fy, map_height);
    x = (float)0;
    for (fx = 0; fx < map_width; fx++) {
      if (fx == (map_width - 1)) {
        p1 = MiMap->map[(memptrsize)y * man + (memptrsize)x - 1];
        p2 = MiMap->map[(memptrsize)y * man + (memptrsize)x];
      } else {
        p1 = MiMap->map[(memptrsize)y * man + (memptrsize)x];
        p2 = MiMap->map[(memptrsize)y * man + (memptrsize)x + 1];
      }

      p1 = ghost[p1 + p2 * 256];

      if (fy == (map_height - 1)) {
        p3 = MiMap->map[(memptrsize)y * man + (memptrsize)x - man - 1];
        p4 = MiMap->map[(memptrsize)y * man + (memptrsize)x - man];
      } else {
        p3 = MiMap->map[(memptrsize)y * man + (memptrsize)x + man];
        p4 = MiMap->map[(memptrsize)y * man + (memptrsize)x + man + 1];
      }

      p3 = ghost[p3 + p4 * 256];
      p1 = ghost[p1 + p3 * 256];

      // Gris=1: Convert the image to grayscale

      if (Gris) {
        p1 = dac[p1 * 3] + dac[p1 * 3 + 1] + dac[p1 * 3 + 1];
        bufferaux[fy * map_width + fx] = Tabla[p1];
      } else
        bufferaux[fy * map_width + fx] = p1;
      x += incx;
    }
    y += incy;
  }
  Progress((char *)texts[92], map_height, map_height);

  if (new_map((byte *)bufferaux)) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(bufferaux);
    return;
  }

  v.mapa->zoom_cx = v.mapa->map_width / 2;
  v.mapa->zoom_cy = v.mapa->map_height / 2;

  fx = (v.mapa->zoom_cx - vga_width / 2);
  if (fx < 0)
    fx = 0;
  else if (fx + vga_width > v.mapa->map_width)
    fx = v.mapa->map_width - vga_width;
  fy = (v.mapa->zoom_cy - vga_height / 2);
  if (fy < 0)
    fy = 0;
  else if (fy + vga_height > v.mapa->map_height)
    fy = v.mapa->map_height - vga_height;
  v.mapa->zoom = 0;
  v.mapa->zoom_x = fx;
  v.mapa->zoom_y = fy;

  for (n = 0; n < 512; n++)
    v.mapa->puntos[n] = -1;
  v.mapa->fpg_code = 0;
  call((voidReturnType)v.paint_handler);
  wvolcado(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.an, v.al, 0);
}

//-----------------------------------------------------------------------------
//      Create a search map of up to 128x128 tiles
//      Used to create a hardness map
//-----------------------------------------------------------------------------

char ctiletext[32];
char ctile[5];

char ccolor[5];
int minimos = 0;
int mintile, maxtile;
int itile, icolor;

void map_search1() {
  _show_items();
}

void map_search2() {
  _process_items();
  switch (v.active_item) {
  case 3:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 4:
    v_accept = 0;
    end_dialog = 1;
    break;
  }
}

void map_search3() {
  itile = atoi(ctile);
  icolor = atoi(ccolor);
}

void map_search0() {
  v.type = 1;
  v.title = texts[400];
  v.an = 72 + 40;
  v.al = 49 + 18 + 9;
  v.paint_handler = map_search1;
  v.click_handler = map_search2;
  v.close_handler = map_search3;

  div_snprintf(ctile, sizeof(ctile), "%d", mintile);
  div_snprintf(ccolor, sizeof(ccolor), "%d", color);

  DIV_SPRINTF(ctiletext, "%s [%d..%d]", texts[401], mintile, maxtile);
  texts[546] = (byte *)&ctiletext[0];

  _get(546, 4, 11, 64 + 40, (byte *)ctile, 5, mintile, maxtile);
  _get(402, 4, 30, 64 + 40, (byte *)ccolor, 5, 0, 255);

  _flag(403, 4, 49, &minimos);

  _button(100, 7, v.al - 14, 0);
  _button(101, v.an - 8, v.al - 14, 2);
}

#define max_map_size 254 // 128

void map_search() {
  int man, mal;
  int c, n, m, x, y;
  byte *ma;

  if (!find_and_load_map())
    return;

  man = map_width;
  mal = map_height;
  ma = map; // Original map ma(man,mal)

  if (man > mal) { // Get the tile size limits
    mintile = (man + max_map_size - 1) / max_map_size;
    if (mal < 256)
      maxtile = mal;
    else
      maxtile = 256;
  } else {
    mintile = (mal + max_map_size - 1) / max_map_size;
    if (man < 256)
      maxtile = man;
    else
      maxtile = 256;
  }
  if (mintile > 256)
    mintile = 256;
  if (maxtile < mintile)
    maxtile = 256;

  v_accept = 0;
  show_dialog(map_search0);

  if (!v_accept)
    return;

  map_width = (map_width + itile - 1) / itile; // Size of the new map
  map_height = (map_height + itile - 1) / itile;
  if (map_width > max_map_size)
    map_width = max_map_size;
  if (map_height > max_map_size)
    map_height = max_map_size;

  if ((map = (byte *)malloc(map_width * map_height)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

  for (y = 0; y < map_height * itile; y += itile) {
    Progress((char *)texts[404], y, map_height * itile);
    for (x = 0; x < map_width * itile; x += itile) {
      c = 0; // 0/c4
      for (n = 0; n < itile; n++) {
        for (m = 0; m < itile; m++) {
          if (x + m < man && y + n < mal) {
            if (*(ma + x + m + (y + n) * man) == icolor)
              c++;
          }
        }
      }
      if (minimos) {
        if (c < (itile * itile) / 5)
          c = 0;
        else
          c = c4;
      } else if (c)
        c = c4;
      *(map + (x / itile) + (y / itile) * map_width) = c;
    }
  }
  Progress((char *)texts[404], map_height * itile, map_height * itile);

  if (new_map(map)) {
    free(map);
    return;
  }

  call((voidReturnType)v.paint_handler);
  wvolcado(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.an, v.al, 0);
}

//-----------------------------------------------------------------------------
// Progress indicator for an operation
//-----------------------------------------------------------------------------

int ProgressInit = 0;
int ProgressTotal;
int ProgressCurrent;
char *ProgressTitle;

void Progress1() {
  int an = v.an, al = v.al;
  byte *ptr = v.ptr;
  char cwork[5];
  if (big) {
    an /= 2;
    al /= 2;
  }
  wbox(ptr, an, al, c2, 2, 10, an - 4, al - 12);

  wbox(ptr, an, al, c1, 4, 12, an - 8, al - 16);

  wbox(ptr, an, al, c_r_low, 4, 12, ProgressCurrent * (an - 8) / ProgressTotal, al - 16);
  wrectangle(ptr, an, al, c0, 4, 12, an - 8, al - 16);
  div_snprintf((char *)cwork, sizeof(cwork), "%d%c", ProgressCurrent * 100 / ProgressTotal, '%');
  wwrite(ptr, an, al, 4 + (an - 8) / 2, (14 + (al - 16) / 2) - 2, 4, (byte *)cwork, c3);
}

void Progress2() {
  int an = v.an, al = v.al;
  byte *ptr = v.ptr;
  char cwork[5];
  if (big) {
    an /= 2;
    al /= 2;
  }
  wbox(ptr, an, al, c2, 2, 10, an - 4, al - 12);

  wbox(ptr, an, al, c1, 4, 12, an - 8, al - 16);

  wbox(ptr, an, al, c_r_low, 4, 12, ProgressCurrent * (an - 8) / ProgressTotal, al - 16);
  wrectangle(ptr, an, al, c0, 4, 12, an - 8, al - 16);
  div_snprintf(cwork, sizeof(cwork), "%d%c", ProgressCurrent * 100 / ProgressTotal, '%');
  wwrite(ptr, an, al, 4 + (an - 8) / 2, (14 + (al - 16) / 2) - 2, 4, (byte *)cwork, c3);
}
void Progress0() {
  v.type = 7;
  v.an = 100;
  v.al = 28;
  v.paint_handler = Progress1;
  v.click_handler = Progress2;
  v.title = (byte *)ProgressTitle;
  v.name = (byte *)ProgressTitle;
}

int Progress(char *titulo, int current, int total) {
  if (!ProgressInit) { // Initialize the progress window
    mouse_graf = 3;
    flush_copy();
    ProgressInit = 1;
    ProgressCurrent = current;
    ProgressTitle = titulo;
    ProgressTotal = total;
    new_window(Progress0);
    flush_window(0);
    flush_copy();
  } else {
    ProgressCurrent = current;
    if (ProgressCurrent >= ProgressTotal) { // Finalize the progress window
      mouse_graf = 1;
      ProgressInit = 0;
      close_window();
      flush_copy();
      return 0;
    } else { // Progress window update loop
      v = window[0];
      Progress2();
      flush_window(0);
      flush_copy();
    }
  }
  return (1);
}

//-----------------------------------------------------------------------------

void AplieX(struct tmapa *MiMap, int man, int mal) {
  char *bufferaux;
  char DacAux[768];
  int n, p1, p2, p3, p4;
  int x, y, z, t, x0, y0, ant, min, max;
  float incx, incy;
  float fx, fy, cx, cy, rx, ry;
  float pp1, pp2, pp3, pp4;
  int r, b, g, c, regen = 0;
  FILE *rgb_tab = NULL;
  char *rgb_table = NULL;
  char Tabla[190];
  int Colors[9], min_dist, i, dist;

  // Generate grayscale table
  if (Gris) {
    Colors[0] = c0;
    Colors[2] = c1;
    Colors[4] = c2;
    Colors[6] = c3;
    Colors[8] = c4;
    Colors[1] = average_color(c0, c1);
    Colors[3] = average_color(c1, c2);
    Colors[5] = average_color(c2, c3);
    Colors[7] = average_color(c3, c4);

    for (n = 0; n < 190; n++) {
      min_dist = 192;
      for (i = 0; i < 9; i++) {
        c = (memptrsize)Colors[i] * 3;
        dist = dac[c] + dac[c + 1] + dac[c + 2];
        if (dist > n)
          dist = dist - n;
        else
          dist = n - dist;
        if (dist < min_dist) {
          Tabla[n] = c / 3;
          min_dist = dist;
        }
      }
    }
  }
  // End grayscale table
  else
  // Generate RGB lookup table with error margin of 1
  {
    rgb_table = (char *)malloc(32768);
    if (rgb_table == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    if ((rgb_tab = fopen("RGB_TAB.TMP", "rb")) == NULL)
      regen = 1;
    else {
      fread(DacAux, 1, 768, rgb_tab);
      for (x = 0; x < 768; x++)
        regen += abs(dac[x] - DacAux[x]);
    }
    if (regen) {
      for (x = 0; x < 32; x++) {
        Progress((char *)texts[93], x, 32);
        for (y = 0; y < 32; y++)
          for (z = 0; z < 32; z++) {
            min = 32 * 32 * 32;
            ant = 0;
            for (t = 0; t < 256; t++) {
              max = (x - (dac[t * 3] / 2)) * (x - (dac[t * 3] / 2)) +
                    (y - (dac[t * 3 + 1] / 2)) * (y - (dac[t * 3 + 1] / 2)) +
                    (z - (dac[t * 3 + 2] / 2)) * (z - (dac[t * 3 + 2] / 2));
              if (max < min) {
                min = max;
                ant = t;
              }
            }
            rgb_table[x * 1024 + y * 32 + z] = ant;
          }
      }
      Progress((char *)texts[93], 32, 32);
      if (rgb_tab)
        fclose(rgb_tab);
      rgb_tab = fopen("RGB_TAB.TMP", "wb");
      fwrite(dac, 1, 768, rgb_tab);
      fwrite(rgb_table, 1, 32768, rgb_tab);
      fclose(rgb_tab);
    } else {
      fread(rgb_table, 1, 32768, rgb_tab);
      fclose(rgb_tab);
    }
  }
  // End RGB lookup table

  incy = (float)(mal - 1) / (float)map_height;
  incx = (float)(man - 1) / (float)map_width;
  bufferaux = (char *)malloc(map_width * map_height);
  if (bufferaux == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    if (!Gris)
      free(rgb_table);
    return;
  }
  fy = (float)0.5;
  for (y = 0; y < map_height; y++) {
    Progress((char *)texts[92], y, map_height);

    y0 = (memptrsize)fy;
    cy = ((float)y0 + (float)0.5);
    if (((fy - cy) < 0))
      ry = cy - 1.0;
    else
      ry = cy;

    fx = (float)0.5;
    for (x = 0; x < map_width; x++) {
      x0 = (memptrsize)fx;
      cx = ((float)x0 + (float)0.5);
      if (((fx - cx) < 0))
        rx = cx - 1.0;
      else
        rx = cx;

      if (fx > rx)
        pp1 = (fx - rx);
      else
        pp1 = (rx - fx);
      if (fy > ry)
        pp1 += (fy - ry);
      else
        pp1 += (ry - fy);
      pp1 = (2.0 - pp1) / 2.0;
      p1 = MiMap->map[(memptrsize)ry * man + (memptrsize)rx];
      r = dac[p1 * 3] * pp1;
      g = dac[p1 * 3 + 1] * pp1;
      b = dac[p1 * 3 + 2] * pp1;

      if (fx > (rx + 1.0))
        pp2 = (fx - (rx + 1.0));
      else
        pp2 = ((rx + 1.0) - fx);
      if (fy > ry)
        pp2 += (fy - ry);
      else
        pp2 += (ry - fy);
      pp2 = (2.0 - pp2) / 2.0;
      p2 = MiMap->map[(memptrsize)ry * man + (memptrsize)(rx + 1.0)];
      r += dac[p2 * 3] * pp2;
      g += dac[p2 * 3 + 1] * pp2;
      b += dac[p2 * 3 + 2] * pp2;

      if (fx > rx)
        pp3 = (fx - rx);
      else
        pp3 = (rx - fx);
      if (fy > ry + 1.0)
        pp3 += (fy - (ry + 1.0));
      else
        pp3 += ((ry + 1.0) - fy);
      pp3 = (2.0 - pp3) / 2.0;
      p3 = MiMap->map[(memptrsize)(ry + 1.0) * man + (memptrsize)rx];
      r += dac[p3 * 3] * pp3;
      g += dac[p3 * 3 + 1] * pp3;
      b += dac[p3 * 3 + 2] * pp3;

      if (fx > rx + 1.0)
        pp4 = (fx - (rx + 1.0));
      else
        pp4 = ((rx + 1.0) - fx);
      if (fy > ry + 1.0)
        pp4 += (fy - (ry + 1.0));
      else
        pp4 += ((ry + 1.0) - fy);
      pp4 = (2.0 - pp4) / 2.0;
      p4 = MiMap->map[(memptrsize)(ry + 1.0) * man + (memptrsize)(rx + 1.0)];
      r += dac[p4 * 3] * pp4;
      g += dac[p4 * 3 + 1] * pp4;
      b += dac[p4 * 3 + 2] * pp4;

      r /= (pp1 + pp2 + pp3 + pp4);
      g /= (pp1 + pp2 + pp3 + pp4);
      b /= (pp1 + pp2 + pp3 + pp4);

      if (Gris) {
        if (r + g + b > 189)
          bufferaux[y * map_width + x] = Tabla[189];
        else
          bufferaux[y * map_width + x] = Tabla[r + g + b];
      } else
        bufferaux[y * map_width + x] = rgb_table[(r >> 1) * 1024 + (g >> 1) * 32 + (b >> 1)];

      fx += incx;
    }
    fy += incy;
  }
  Progress((char *)texts[92], map_height, map_height);

  if (new_map((byte *)bufferaux)) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    free(bufferaux);
    if (!Gris)
      free(rgb_table);
    return;
  }

  v.mapa->zoom_cx = v.mapa->map_width / 2;
  v.mapa->zoom_cy = v.mapa->map_height / 2;

  x = (v.mapa->zoom_cx - vga_width / 2);
  if (x < 0)
    x = 0;
  else if (x + vga_width > v.mapa->map_width)
    x = v.mapa->map_width - vga_width;
  y = (v.mapa->zoom_cy - vga_height / 2);
  if (y < 0)
    y = 0;
  else if (y + vga_height > v.mapa->map_height)
    y = v.mapa->map_height - vga_height;
  v.mapa->zoom = 0;
  v.mapa->zoom_x = x;
  v.mapa->zoom_y = y;

  for (n = 0; n < 512; n++)
    v.mapa->puntos[n] = -1;
  v.mapa->fpg_code = 0;
  call((voidReturnType)v.paint_handler);
  wvolcado(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.an, v.al, 0);

  if (!Gris)
    free(rgb_table);
}

//-----------------------------------------------------------------------------
//      About Dialog Box
//-----------------------------------------------------------------------------

void about1(void) {
  int x;
  int an = v.an / big2, al = v.al / big2;
  _show_items();

  x = an / 2;

  wwrite(v.ptr, an, al, x, 11 + 8 * 0, 1, texts[464], c3);
  wwrite(v.ptr, an, al, x, 11 + 8 * 1, 1, texts[465], c3);
  wwrite(v.ptr, an, al, x, 11 + 8 * 2, 1, texts[466], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 3, 1, texts[467], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 4, 1, texts[468], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 5, 1, texts[469], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 6, 1, texts[470], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 7, 1, texts[471], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 8, 1, texts[472], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 9, 1, texts[473], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 10, 1, texts[474], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 11, 1, texts[475], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 12, 1, texts[476], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 13, 1, texts[477], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 14, 1, texts[478], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 15, 1, texts[479], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 16, 1, texts[480], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 17, 1, texts[481], c0);
  wwrite(v.ptr, an, al, x, 11 + 8 * 18, 1, texts[482], c3);
  wwrite(v.ptr, an, al, x, 11 + 8 * 19, 1, texts[483], c4);
}

void about2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void about0(void) {
  v.type = 1;
  v.title = texts[885];
  v.an = 232;
  v.al = 188;
  v.paint_handler = about1;
  v.click_handler = about2;
  _button(100, v.an / 2, v.al - 14, 1);
}
