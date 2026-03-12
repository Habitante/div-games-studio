#include "global.h"
#include "div_string.h"
#include "fpgfile.hpp"

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

int create_saved_window(void_return_type_t init_handler, int nx, int ny);
int load_new_map(int nx, int ny, char *name, byte *bitmap);
void carga_programa0(void);
void carga_Fonts0(void);
void carga_help(int n, int helpal, int helpline, int x1, int x2);
void help_xref(int n, int linea);
void mixer0(void);
void open_desktop_sound(FILE *f);
void save_desktop_sound(pcminfo *mypcminfo, FILE *f);
void open_desktop_song(void);

int CDinit(void);
int get_cd_error(void);
extern short cd_playing;

// M3D_info removed (MODE8/3D map editor deleted)

extern char Load_FontPathName[256];
extern char Load_FontName[14];

//extern
struct _calc {
  char ctext[128];
  char cget[128];
  char cresult[128];
  int cint, chex;
};
extern struct _calc *pcalc;
extern struct _calc *readcalc;
void calc0(void);

extern int helpidx[4096];            // For each term {offset,length}
extern int help_item;                // Which term help is requested for
extern int help_len;                 // Length of help_buffer
extern int help_w, help_h;           // Width and height of the help window
extern int help_l, help_lines;       // Current line, and total lines
extern byte help_title[128];         // Title of the term
extern byte *help_buffer, *h_buffer; // Buffer to hold help content, auxiliary
extern byte *help_line;              // Pointer to the current line
extern byte *help_end;               // End of help_buffer;
extern int loaded[64], n_loaded;     // Loaded images, up to a maximum of 32
extern int backto[64];               // Circular queue to store consulted topics {n,line}
extern int i_back, a_back, f_back;   // Start and end of the circular queue (both 0,2,..62)

void Fonts1(void);
void Fonts2(void);
void Fonts3(void);

void Load_Font_session(FILE *file);
int Save_Font_session(FILE *file, int);
FILE *desktop;

char pathtmp[1024];

int get_file_creation_time(char *path) {
  struct stat attr;
  stat(path, &attr);
  return (int)attr.st_mtime;
}


void download_desktop() {
  int iWork, x, numvent = 0, n;
  int man, mal;
  pcminfo *mypcminfo;
  modinfo *mymodinfo;

  // Write identification header
  desktop = fopen("system/session.dtf", "wb");
  n = fwrite("dtf\x1a\x0d\x0a\x0", 8, 1, desktop);
  // Save the previous resolution
  iWork = setup_file.vid_mode_height + setup_file.vid_mode_width * 10000 +
          (setup_file.vid_mode_big << 31);
  n = fwrite(&iWork, 1, 4, desktop);
  // Reserve space for the window count
  n = fwrite(&numvent, 1, 4, desktop);
  // Save palette /4
  n = fwrite(dac, 768, 1, desktop);
  // Save ghost table
  n = fwrite(ghost, 65536, 1, desktop);
  // Check and save each used window one by one
  for (x = max_windows - 1; x >= 0; x--) {
    if (window[x].type != 0 && window[x].title) {
      numvent++;
      n = fwrite(&window[x], 1, sizeof(struct twindow), desktop);
      switch (window[x].type) {
      //Window struct
      case 2: //menu
        iWork = -1;
        if (window[x].paint_handler == menu_principal1)
          iWork = 0;
        if (window[x].paint_handler == menu_programas1)
          iWork = 1;
        if (window[x].paint_handler == menu_paletas1)
          iWork = 2;
        if (window[x].paint_handler == menu_mapas1)
          iWork = 3;
        if (window[x].paint_handler == menu_graficos1)
          iWork = 4;
        if (window[x].paint_handler == menu_fuentes1)
          iWork = 5;
        if (window[x].paint_handler == menu_sonidos1)
          iWork = 6;
        if (window[x].paint_handler == menu_sistema1)
          iWork = 7;
        if (window[x].paint_handler == menu_edicion1)
          iWork = 8;
        // menu_mapas3D1 check removed (MODE8/3D map editor deleted)
        n = fwrite(&iWork, 1, 4, desktop);

        break;
      case 3: //palet
        break;
      case 4: //timer
        if (window[x].paint_handler == clock1)
          iWork = 1;
        n = fwrite(&iWork, 1, 4, desktop);
        break;
      case 5: //recycle bin
        break;
      case 8: //mixer
        break;
      case 100: //map
        // tmapa struct
        man = window[x].mapa->map_width;
        mal = window[x].mapa->map_height;
        n = fwrite(window[x].mapa, 1, sizeof(struct tmapa), desktop);
        // Graphic data
        n = fwrite((char *)window[x].mapa->map, man * mal, 1, desktop);
        break;
      case 101: //fpg
        // FPG struct
        n = fwrite(window[x].aux, 1, sizeof(FPG), desktop);
        break;
      case 102: //prg
        if (window[x].prg != NULL) {
          iWork = 0;
          n = fwrite(&iWork, 1, 4, desktop);
          n = fwrite(window[x].prg, 1, sizeof(struct tprg), desktop);
          n = fwrite(window[x].prg->buffer, 1, window[x].prg->buffer_len, desktop);
          iWork = (uintptr_t)window[x].prg->lptr - (uintptr_t)window[x].prg->buffer;
          n = fwrite(&iWork, 1, 4, desktop);
          iWork = (uintptr_t)window[x].prg->vptr - (uintptr_t)window[x].prg->buffer;
          n = fwrite(&iWork, 1, 4, desktop);
        } else {
          //if(window[x].aux!=NULL)
          if (!strcmp((char *)window[x].name, (char *)texts[83])) {
            iWork = 1;
            n = fwrite(&iWork, 1, 4, desktop);
            iWork = Save_Font_session(desktop, iWork);
          } else if (!strcmp((char *)window[x].name, (char *)texts[413])) {
            iWork = 3;
            n = fwrite(&iWork, 1, 4, desktop);
            div_strcpy(((struct _calc *)(window[x].aux))->ctext,
                       sizeof(((struct _calc *)(window[x].aux))->ctext),
                       ((struct _calc *)(window[x].aux))->cget);
            n = fwrite(window[x].aux, 1, sizeof(struct _calc), desktop);
          } else {
            iWork = 2;
            n = fwrite(&iWork, 1, 4, desktop);
            n = fwrite(&help_item, 1, 4, desktop);
            n = fwrite(&help_h, 1, 4, desktop);
            n = fwrite(&help_l, 1, 4, desktop);
          }
        }
        break;
      case 104: //fnt
        // Save to file
        n = fwrite(window[x].aux, 1, 14, desktop);
        n = fwrite(window[x].aux + 14, 1, _MAX_PATH - 14, desktop);
        break;
      case 105: //pcm
        mypcminfo = (pcminfo *)window[x].aux;
        save_desktop_sound(mypcminfo, desktop);
        break;
      // case 106 (map3d) removed (MODE8/3D map editor deleted)
      case 107: //mod
        mymodinfo = (modinfo *)window[x].aux;
        n = fwrite(mymodinfo->name, 1, 14, desktop);
        n = fwrite(mymodinfo->pathname, 1, 256, desktop);
        break;
      }
    }
    if (window[x].ptr != NULL)
      free(window[x].ptr);
  }
  fseek(desktop, 0, SEEK_SET);
  fseek(desktop, 8 + 4, SEEK_SET);
  n = fwrite(&numvent, 1, 4, desktop);
  fclose(desktop);
}

int vid_mode_changed = 0;

int previous_mode;

int can_upload_desktop() {
  char cWork[8];
  int iWork;
  vid_mode_changed = 0;
  desktop = fopen("system/session.dtf", "rb");
  if (desktop == NULL)
    return (0);
  // read the header id
  fread(cWork, 8, 1, desktop);
  // Check old resolution
  fread(&iWork, 1, 4, desktop);
  if (iWork != setup_file.vid_mode_height + setup_file.vid_mode_width * 10000 +
                   (setup_file.vid_mode_big << 31)) {
    previous_mode = iWork;
    vid_mode_changed = 1;
  }
  // Skip the window counter
  fread(cWork, 4, 1, desktop);
  // Read palette /4
  fread(dac, 768, 1, desktop);
  // Read ghost table
  fread(ghost, 65536, 1, desktop);
  fclose(desktop);
  return (1);
}

struct twindow window_aux;
struct tmapa maux;
FPG faux;
struct tprg paux;
char *baux;

int upload_desktop() {
  int iWork, iWork2, iWork3, x, numvent;
  FILE *f;

  int dtime = get_file_creation_time("system/session.dtf");

  desktop = fopen("system/session.dtf", "rb");
  if (desktop == NULL)
    return (0);


  fseek(desktop, 8 + 4, SEEK_SET);
  fread(&numvent, 1, 4, desktop);
  fseek(desktop, 8 + 4 + 4 + 768 + 65536, SEEK_SET);
  // Load each of the windows one by one
  for (x = 0; x < numvent; x++) {
    // Window struct data
    fread(&window_aux, 1, sizeof(struct twindow), desktop);
    switch (window_aux.type) {
    case 2: //menu
      fread(&iWork, 1, 4, desktop);
      switch (iWork) {
      case 0:
        create_saved_window(menu_principal0, window_aux.x, window_aux.y);
        break;
      case 1:
        create_saved_window(menu_programas0, window_aux.x, window_aux.y);
        break;
      case 2:
        create_saved_window(menu_paletas0, window_aux.x, window_aux.y);
        break;
      case 3:
        create_saved_window(menu_mapas0, window_aux.x, window_aux.y);
        break;
      case 4:
        create_saved_window(menu_graficos0, window_aux.x, window_aux.y);
        break;
      case 5:
        create_saved_window(menu_fuentes0, window_aux.x, window_aux.y);
        break;
      case 6:
        create_saved_window(menu_sonidos0, window_aux.x, window_aux.y);
        break;
      case 7:
        create_saved_window(menu_sistema0, window_aux.x, window_aux.y);
        break;
      case 8:
        create_saved_window(menu_edicion0, window_aux.x, window_aux.y);
        break;
      // case 9 (menu_mapas3D0) removed (MODE8/3D map editor deleted)
      default:
        break;
      }
      if (!interpreting)
        update_box(0, 0, vga_width, vga_height);
      break;
    case 3:
      create_saved_window(palette0, window_aux.x, window_aux.y);
      if (!interpreting)
        update_box(0, 0, vga_width, vga_height);
      break;
    case 4: //timer
      fread(&iWork, 1, 4, desktop);
      switch (iWork) {
      case 0:
        break; // removed: CD player
        break;
      case 1: //Clock
        create_saved_window(clock0, window_aux.x, window_aux.y);
        break;
      }
      if (!interpreting)
        update_box(0, 0, vga_width, vga_height);
      break;
    case 5: //recycle bin
      create_saved_window(bin0, window_aux.x, window_aux.y);
      if (!interpreting)
        update_box(0, 0, vga_width, vga_height);
      break;
    case 8: //mixer
      create_saved_window(mixer0, window_aux.x, window_aux.y);
      if (!interpreting)
        update_box(0, 0, vga_width, vga_height);
      break;
    case 100: //map
      // tmapa struct
      fread(&maux, 1, sizeof(struct tmapa), desktop);
      baux = (char *)malloc(maux.map_width * maux.map_height);
      if (baux == NULL) {
        fseek(desktop, maux.map_width * maux.map_height, SEEK_CUR);
        if (!interpreting)
          update_box(0, 0, vga_width, vga_height);
        break;
      }
      fread(baux, maux.map_width, maux.map_height, desktop);
      map_width = maux.map_width;
      map_height = maux.map_height;
      if (load_new_map(window_aux.x, window_aux.y, maux.filename, (byte *)baux)) {
        free(baux);
        if (!interpreting)
          update_box(0, 0, vga_width, vga_height);
        break;
      }
      v.mapa->has_name = maux.has_name;
      v.mapa->code = maux.code;
      v.mapa->fpg_code = maux.fpg_code;
      memcpy((char *)v.mapa->path, (char *)maux.path, _MAX_PATH + 1);
      memcpy((char *)v.mapa->filename, (char *)maux.filename, 255);
      v.mapa->zoom = maux.zoom;
      v.mapa->zoom_x = maux.zoom_x;
      v.mapa->zoom_y = maux.zoom_y;
      v.mapa->zoom_cx = maux.zoom_cx;
      v.mapa->zoom_cy = maux.zoom_cy;
      v.mapa->saved = maux.saved;
      memcpy((char *)v.mapa->description, (char *)maux.description, 32);
      memcpy((char *)v.mapa->points, (char *)maux.points, 512 * 2);
      // Graphic data
      call((void_return_type_t)v.paint_handler);
      blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
      if (!interpreting)
        update_box(0, 0, vga_width, vga_height);
      break;
    case 101: //fpg
      // FPG struct
      fread(&faux, 1, sizeof(FPG), desktop);
      div_strcpy(input, sizeof(input), (char *)faux.fpg_name);
      div_strcpy(full, sizeof(full), (char *)faux.current_file);
      if ((f = fopen(full, "rb")) != NULL) {
        fclose(f);
        v_aux = (byte *)malloc(sizeof(FPG));
        if (v_aux != NULL) {
          memcpy(v_aux, &faux, sizeof(FPG));
          create_saved_window(fpg_dialog0_add, window_aux.x, window_aux.y);
          blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
          if (!interpreting)
            update_box(0, 0, vga_width, vga_height);
        }
      }
      break;
    case 102: //prg
      fread(&iWork, 1, 4, desktop);
      if (iWork == 0) {
        if ((v_prg = (struct tprg *)malloc(sizeof(struct tprg))) != NULL) {
          fread(v_prg, 1, sizeof(struct tprg), desktop);
          v_prg->buffer = (byte *)malloc(v_prg->buffer_len);
          fread(v_prg->buffer, 1, v_prg->buffer_len, desktop);

          fread(&iWork, 1, 4, desktop);
          v_prg->lptr = v_prg->buffer + iWork;
          fread(&iWork, 1, 4, desktop);
          v_prg->vptr = v_prg->buffer + iWork;

          create_saved_window(carga_programa0, window_aux.x, window_aux.y);

          blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
          if (!interpreting)
            update_box(0, 0, vga_width, vga_height);
        }
        // check if prg on disk is newer than session
        div_strcpy(pathtmp, sizeof(pathtmp), v_prg->path);
        div_strcat(pathtmp, sizeof(pathtmp), "/");
        div_strcat(pathtmp, sizeof(pathtmp), v_prg->filename);

        if (dtime < get_file_creation_time(&pathtmp[0])) {
          v_title = v_prg->filename;
          v_text = "File on disk is newer, reload?";
          show_dialog(accept0);

          if (v_accept) {
            div_strcpy(file_types[0].path, sizeof(file_types[0].path), v_prg->path);
            div_strcpy(input, sizeof(input), v_prg->filename);

            // close old prg
            close_window();
            v_finished = 1;

            // load replacement prg
            open_program();
          }
        }
        break;
      } else {
        if (iWork == 1)
          create_saved_window(carga_Fonts0, window_aux.x, window_aux.y);
        else {
          if (iWork == 2) {
            fread(&iWork, 1, 4, desktop);
            fread(&iWork2, 1, 4, desktop);
            fread(&iWork3, 1, 4, desktop);
            carga_help(iWork, iWork2, iWork3, window_aux.x, window_aux.y);
          } else {
            if (iWork == 3) {
              readcalc = (struct _calc *)malloc(sizeof(struct _calc));
              pcalc = NULL;
              if (readcalc != NULL) {
                fread(readcalc, 1, sizeof(struct _calc), desktop);
                create_saved_window(calc0, window_aux.x, window_aux.y);
              } else {
                fseek(desktop, sizeof(struct _calc), SEEK_CUR);
              }
            } else {
              v_text = (char *)texts[45];
              show_dialog(err0);
            }
          }
        }
      }
      break;
    case 104: //fnt
      // Load from file
      fread(Load_FontName, 1, 14, desktop);
      fread(Load_FontPathName, 1, _MAX_PATH - 14, desktop);

      debugprintf("fontname [%s], fontpathname [%s]\n", Load_FontName, Load_FontPathName);
      fflush(stdout);

      if ((f = fopen(Load_FontPathName, "rb")) != NULL) {
        fclose(f);
        create_saved_window(show_font0, window_aux.x, window_aux.y);
        blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);
        if (!interpreting)
          update_box(0, 0, vga_width, vga_height);
      }
      break;
    case 105: //pcm
      open_desktop_sound(desktop);
      break;
    // case 106 (map3d) removed (MODE8/3D map editor deleted)
    case 107: //mod
      fread(song_name, 1, 14, desktop);
      fread(song_path_name, 1, 256, desktop);
      if ((f = fopen(song_path_name, "rb")) != NULL) {
        fclose(f);
        open_desktop_song();
      }
      break;
    }
  }
  fclose(desktop);
  if (interpreting)
    update_box(0, 0, vga_width, vga_height);
  return (1);
}


//-----------------------------------------------------------------------------
//      Load new window (1 on Error)
//-----------------------------------------------------------------------------

int create_saved_window(void_return_type_t init_handler, int nx, int ny) {
  byte *ptr;
  int n, m, x, y, w, h;
  int vtipo;

  uint32_t colorkey = 0;

  if (!window[max_windows - 1].type) {
    addwindow();

    //---------------------------------------------------------------------------
    // The following values must be set by init_handler, defaults:
    //---------------------------------------------------------------------------

    v.order = next_order++;
    v.type = 0;
    v.foreground = 1;
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

    //---------------------------------------------------------------------------
    // Window placement algorithm ...
    //---------------------------------------------------------------------------

    if (!vid_mode_changed) {
      x = nx;
      y = ny;
    } else
      place_window(v.side * 2 + 1, &x, &y, w, h);

    v.x = x;
    v.y = y;

    //---------------------------------------------------------------------------
    // If it's a map, check that no other is already active
    //---------------------------------------------------------------------------

    if (vid_mode_changed) {
      if (v.type >= 100 && window_aux.foreground != 2) {
        v.state = 1; // Activate it
        for (m = 1; m < max_windows; m++)
          if (window[m].type == v.type && window[m].state) {
            window[m].state = 0;
            wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c1, 2, 2,
                 window[m].w / big2 - 20, 7);
            if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
              wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                            window[m].h / big2, 4, 2, 0, window[m].title, c0);
              wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                            window[m].h / big2, 3, 2, 0, window[m].title, c2);
            } else {
              wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                     2 + (window[m].w / big2 - 20) / 2, 3, 1, window[m].title, c0);
              wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                     2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c2);
            }
            vtipo = v.type;
            v.type = 0;
            flush_window(m);
            v.type = vtipo;
            break;
          }
      }
    }

    //---------------------------------------------------------------------------
    // Check that if it's a menu it hasn't already been created
    //---------------------------------------------------------------------------
    if ((ptr = (byte *)malloc(w * h)) != NULL) { // Window buffer, freed in close_window

      //---------------------------------------------------------------------------
      // Send overlapping windows to background as needed
      //---------------------------------------------------------------------------
      vtipo = v.type;
      v.type = 0;

      if (!vid_mode_changed) {
        swap(v.w, window_aux.w);
        swap(v.h, window_aux.h);
        for (n = 1; n < max_windows; n++) {
          if (window[n].type && window[n].foreground == 1) {
            if (windows_collide(0, n)) {
              window[n].foreground = 0;
              flush_window(n);
            }
          }
        }
        swap(v.w, window_aux.w);
        swap(v.h, window_aux.h);
      } else {
        for (n = 1; n < max_windows; n++) {
          if (window[n].type && window[n].foreground == 1) {
            if (windows_collide(0, n)) {
              window[n].foreground = 0;
              flush_window(n);
            }
          }
        }
      }
      v.type = vtipo;

      //---------------------------------------------------------------------------
      // Initialize the window
      //---------------------------------------------------------------------------

      v.ptr = ptr;

      window_surface(w, h, 0);

      memset(ptr, c0, w * h);
      if (big) {
        w /= 2;
        h /= 2;
      }

      wrectangle(ptr, w, h, c2, 0, 0, w, h);

      wput(ptr, w, h, w - 9, 2, 35);
      wput(ptr, w, h, w - 17, 2, 37);
      wgra(ptr, w, h, c_b_low, 2, 2, w - 20, 7);
      if (text_len(v.title) + 3 > w - 20) {
        wwrite_in_box(ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
        wwrite_in_box(ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
      } else {
        wwrite(ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
        wwrite(ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
      }

      call((void_return_type_t)v.paint_handler);

      if (!vid_mode_changed) {
        v.foreground = window_aux.foreground;
        v.state = window_aux.state;
        v.w = window_aux.w;
        v.h = window_aux.h;
        v._x = window_aux._x;
        v._y = window_aux._y;
        v._w_saved = window_aux._w_saved;
        v._h_saved = window_aux._h_saved;
      }

      if (v.type >= 100 && v.state == 0) {
        wgra(v.ptr, w, h, c1, 2, 2, w - 20, 7);
        if (text_len(v.title) + 3 > w - 20) {
          wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c0);
          wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c2);
        } else {
          wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 3, 1, v.title, c0);
          wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c2);
        }
      }

      if (big) {
        w *= 2;
        h *= 2;
      }

      if (!interpreting && exploding_windows) {
        if (v.foreground == 2) {
          v.exploding = 1;
          explode(v.x, v.y, v.w, v.h);
          v.exploding = 0;
        } else {
          v.exploding = 1;
          explode(x, y, w, h);
          v.exploding = 0;
        }
      }

      if (v.foreground != 2) {
        if (v.foreground == 1)
          blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
        else
          blit_region_dark(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
        blit_partial(x, y, w, h);
      } else {
        flush_window(0);
        blit_partial(v.x, v.y, v.w, v.h);
      }

      //---------------------------------------------------------------------------
      // Could not open window (out of memory or duplicate menu)
      //---------------------------------------------------------------------------

    } else {
      divdelete(0);
      return (1);
    }

  } else
    return (1);

  return (0);
}


//-----------------------------------------------------------------------------
//      Load a new map (1 on Error)
//-----------------------------------------------------------------------------

int load_new_map(int nx, int ny, char *name, byte *bitmap) {
  int n;

  //1. Allocate memory for a tmapa struct
  if ((v_map = (struct tmapa *)malloc(sizeof(struct tmapa))) != NULL) {
    memset(v_map, 0, sizeof(struct tmapa));

    // 2. Set the map pointer
    v_map->map = bitmap;

    //4. Set the remaining variables
    memcpy((char *)v_map->filename, (char *)name, 255);
    *v_map->path = '\0';
    v_map->map_width = map_width;
    v_map->map_height = map_height;
    v_map->has_name = 0;
    v_map->fpg_code = 0;
    v_map->description[0] = 0;
    for (n = 0; n < 512; n++)
      v_map->points[n] = -1;
    create_saved_window(map_view0, nx, ny);

    return (0);

  } else {
    v_text = (char *)texts[45];
    show_dialog(err0);
  }

  return (1);
}

// nuevo_mapa3d_carga removed (MODE8/3D map editor deleted)

void test_cursor(void);

void carga_programa0(void) {
  v.type = 102;

  v.prg = v_prg;

  if (v.prg->w < 4 * big2)
    v.prg->w = 4 * big2;
  if (v.prg->h < 2 * big2)
    v.prg->h = 2 * big2;

  v.w = (4 + 8) * big2 + editor_font_width * v_prg->w;
  v.h = (12 + 16) * big2 + editor_font_height * v_prg->h;

  if (v.w > vga_width) {
    v.prg->w = (vga_width - 12 * big2) / editor_font_width; // Calculate maximized size (in chars)
    v.w = (4 + 8) * big2 + editor_font_width * v.prg->w;
    window_aux.w = v.w;
  }

  if (v.h > vga_height) {
    v.prg->h = (vga_height - 28 * big2) / editor_font_height;
    v.h = (12 + 16) * big2 + editor_font_height * v.prg->h;
    window_aux.h = v.h;
  }

  if (big) {
    if (v.w & 1)
      v.w++;
    if (v.h & 1)
      v.h++;
    v.w = -v.w; // Negative signals that the window should not be doubled
  }

  v.title = (byte *)v_prg->filename;
  v.name = (byte *)v_prg->filename;

  v.paint_handler = program1;
  v.click_handler = program2;
  v.close_handler = program3;
  v.redraw = 2;

  test_cursor();
}

extern struct t_listbox lfontsizes;

void carga_Fonts0(void) {
  Load_Font_session(desktop);
}

void vuelca_help(void);
void barra_vertical(void);

void carga_help(int n, int helpal, int helpline, int x1, int x2) {
  FILE *f;
  byte *p;
  int m_back;

  if (helpidx[n * 2] && helpidx[n * 2 + 1]) {
    if ((f = fopen("help/help.div", "rb")) != NULL) {
      fseek(f, helpidx[n * 2], SEEK_SET);
      if ((h_buffer = (byte *)malloc(helpidx[n * 2 + 1])) != NULL) {
        if ((help_buffer = (byte *)malloc(helpidx[n * 2 + 1] + 1024)) != NULL) {
          if (i_back != a_back) {
            m_back = a_back - 2;
            if (m_back < 0)
              m_back = 62;
            if (backto[m_back] == help_item && backto[m_back + 1] == -1)
              backto[m_back + 1] = help_l;
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
          help_h = (vga_height / 2 - 12 * big2 - 1) / font_height;
          help_h = helpal;

          help_l = 0;
          tabula_help(p + 1, help_buffer, helpidx[n * 2 + 1] - (p + 1 - h_buffer));
          create_saved_window(help0, x1, x2);

          for (n = 0; n < helpline; n++) {
            if (help_l + help_h < help_lines) {
              while (*(help_line++))
                ;
              help_l++;
            }
          }

          if (v.foreground == 2) {
            swap(v.w, v._w_saved);
            swap(v.h, v._h_saved);
          }
          vuelca_help();
          barra_vertical();
          if (v.foreground == 2) {
            swap(v.w, v._w_saved);
            swap(v.h, v._h_saved);
          }

          flush_window(0);
        }
        free(h_buffer);
      }
      fclose(f);
    }
  }
}
