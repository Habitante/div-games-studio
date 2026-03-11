/**
 * DIV GAMES STUDIO 2015
 * div.cpp
 * 
 * Main DIV/OS file. Program entry point
 * 
 ***/


// NOTE - To see things in the DEMO version, search for "SHARE" in the cpp files

///////////////////////////////////////////////////////////////////////////////
//      DIV Operating System
///////////////////////////////////////////////////////////////////////////////

#define DEFINIR_AQUI // DEFINED HERE - see global.h
#ifdef WIN32
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#endif

#include "global.h"
#include "div_string.h"
#include "divsound.h"
#include "divmixer.hpp"
#include "divsb.h"
#include "sysdac.h"

void heap_dump(void);
void DebugFile(char *Cadena, char *Nombre);
void save_prg_buffer(memptrsize);

void maximize(void);
void test0(void);
void calc2(void);
void load_thumbs(void);
void mostrar_mod_meters(void);
// MapperCreator2 removed (MODE8/3D map editor deleted)

///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////

void initialization(void);
void init_environment(void);
void main_loop(void);
void finalization(void);
void minimize_window(void);
void move_window(void);
int determine_prg2(void);
int determine_calc(void);
void update_dialogs(int x, int y, int w, int h);
int collides_with(int a, int x, int y, int w, int h);
void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2);
void deactivate(void);
void implode(int x, int y, int w, int h);
void move_window_complete(void);
void restore_wallpaper(int x, int y, int w, int h);
void find_best_position(int *_x, int *_y, int w, int h);
int calculate_collision(int x, int y, int w, int h);
int calculate_overlap(int a, int x, int y, int w, int h);
void check_oldpif(void);
void default_gradients(void);
void determine_units(void);
void show_button(struct t_item *i);
void show_get(struct t_item *i);
void show_flag(struct t_item *i);
void select_button(struct t_item *i, int activo);
void select_get(struct t_item *i, int activo, int ocultar_error);
int button_status(int n);
void process_button(int n, int e);
int get_status(int n);
void process_get(int n, int e);
int flag_status(int n);
void process_flag(int n, int e);
void get_input(int n);
void check_mouse(void);


extern int help_item;
extern struct tprg *old_prg;
int help_paint_active = 0;
int beta_status = 0; // 0 dialog, 1 beta.nfo, 2 dc, 3 cmp, 4 ok, 5 error

extern int superget;

///////////////////////////////////////////////////////////////////////////////
//      Global variables
///////////////////////////////////////////////////////////////////////////////

char get_buffer[long_line]; // Shared buffer (except for calculator)
char *get;
int get_cursor, get_pos; // Clock and cursor position in get fields

int window_closing = 0, window_move_pending = 0;
int cierra_rapido = 0;
int skip_window_render = 0;

byte lower[256] = "                                   #$           0123456789      "
                  " abcdefghijklmnopqrstuvwxyz    _ abcdefghijklmnopqrstuvwxyz     "
                  " ueaaaa eeeiiiaae  ooouuyou     aiou                            "
                  "                                                               ";

int show_items_called = 0;
int test_video = 0;
int restore_button = 0;

extern int error_number;
extern byte *div_index;
extern byte *index_end;

///////////////////////////////////////////////////////////////////////////////

int primera_vez = 0; // Marks first time DIV runs
extern int div_started;

#ifdef SHARE
int mostrar_demo = 1; // The first time, the demo message won't appear
#else
int mostrar_demo = 0;
#endif

int volcados_parciales = 0;

///////////////////////////////////////////////////////////////////////////////
//      Critical error handler
///////////////////////////////////////////////////////////////////////////////

unsigned c_e_0, c_e_1;
unsigned far *c_e_2;

int __far critical_error(unsigned deverr, unsigned errcode, unsigned far *devhdr) {
  c_e_0 = deverr;
  c_e_1 = errcode;
  c_e_2 = devhdr;
  return (_HARDERR_FAIL);
}

///////////////////////////////////////////////////////////////////////////////
//  Info box for demo version
///////////////////////////////////////////////////////////////////////////////

#ifdef SHARE

char exe_name[128];
int exe_cola[2];

void demo1(void) {
  int w = v.w / big2, h = v.h / big2;
  char cwork[128];

  _show_items();
  wwrite(v.ptr, w, h, w / 2, 12, 1, texts[393], c3);
  wwrite(v.ptr, w, h, w / 2, 12 + 8, 1, texts[394], c3);
  wwrite(v.ptr, w, h, w / 2, 12 + 16, 1, texts[395], c3);

  div_snprintf(cwork, sizeof(cwork), "[ %s %d/31 ]", texts[396], exe_cola[1] - 0xF31725AB);

  wrectangle(v.ptr, w, h, c1, 3, 12 + 27, w - 6, 16);
  wbox(v.ptr, w, h, average_color(c_b_low, c0), 4, 12 + 28, w - 8, 14);

  if (exe_cola[1] - 0xF31725AB > 31) {
    wbox(v.ptr, w, h, c_r_low, 4, 12 + 28, w - 8, 14);
  } else {
    wbox(v.ptr, w, h, c_b_low, 4, 12 + 28, ((exe_cola[1] - 0xF31725AB) * (w - 8)) / 31, 14);
  }

  wwrite(v.ptr, w, h, w / 2 + 1, 12 + 32, 1, cwork, c0);
  wwrite(v.ptr, w, h, w / 2, 12 + 32, 1, cwork, c4);
}

void demo2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
  if (v.active_item == 1) {
    v_accept = 1;
    end_dialog = 1;
  }
}

void demo3(void) {
  if ((exe_cola[1]) <= 31 + 0xF31725AB)
    beta_status = 4;
}

void demo0(void) {
  beta_status = v.type = 1;
  v.title = texts[392];
  v.w = 160;
  v.h = 76;
  v_accept = 0;

  v.paint_handler = demo1;
  v.click_handler = demo2;
  v.close_handler = demo3;

  if (exe_cola[1] - 0xF31725AB > 31) {
    _button(100, v.w / 2, v.h - 14, 1);
  } else {
    _button(100, v.w - 8, v.h - 14, 2);
    _button(125, 7, v.h - 14, 0);
  }
}

void chk_demo(void) {
  FILE *f;

  _dos_setfileattr(exe_name, _A_NORMAL);

  f = fopen(exe_name, "rb");
  printf("DEMO VERSION: %s %x\n", exe_name, f);

  if (f) {
    fseek(f, 0, SEEK_END);
    fseek(f, -8, SEEK_CUR);

    fread(&exe_cola[0], 1, 8, f);

    printf("0x%x\n", exe_cola[0]);

    if (exe_cola[0] == 0xDABACA2A) {
      if (exe_cola[1] - 0xF31725AB > 1024)
        beta_status = 1;
      else
        exe_cola[1]++;
      fseek(f, -4, SEEK_END);
    } else {
      fclose(f);
      f = fopen(exe_name, "ab");

      if (f) {
        fseek(f, 0, SEEK_END);
        exe_cola[0] = 0xDABACA2A;
        exe_cola[1] = 1 + 0xF31725AB;
        fwrite(&exe_cola[0], 1, 8, f);
      }
    }

    if (f)
      fclose(f);
  }
}

#endif

//-----------------------------------------------------------------------------
//  beta check (?)
//-----------------------------------------------------------------------------

// 0 - Dialog

char betaname[128];

void betatest1(void) {
  _show_items();
}

void betatest2(void) {
  _process_items();
  if ((shift_status & 4) && (shift_status & 8) && key(_Z))
    beta_status = 4;
  if (v.active_item == 1)
    end_dialog = 1;
}

void coder(byte *ptr, int len, char *clave);

void betatest0(void) {
  beta_status = v.type = 1;
  v.title = texts[399];
  v.w = 160;
  v.h = 38 + 16;

  div_strcpy(betaname, sizeof(betaname), "");
  _get(460, 4, 12, v.w - 8, (uint8_t *)betaname, 127, 0, 0);

  v.paint_handler = betatest1;
  v.click_handler = betatest2;
  _button(100, v.w / 2, v.h - 14, 1);
}

// 1 - beta.nfo

char realname[128];
int realen;

void betatest4(void) {
  FILE *f;

  f = fopen("system/beta.nfo", "rb");
  if (f == NULL) {
    beta_status = 5;
    return;
  }
  fseek(f, 4, SEEK_SET);
  realen = fread(realname, 1, 128, f);
  fclose(f);

  if (realen < 8)
    beta_status = 5;
  else
    beta_status = 2;
}

// 2 - dc

void betatest5(void) {
  int n;

  div_strcpy(full, sizeof(full), "beta");
  for (n = 0; n < strlen(betaname); n++)
    if (lower[betaname[n]])
      betaname[n] = lower[betaname[n]];
  div_strcat(full, sizeof(full), (char *)texts[398]);
  coder((byte *)realname, realen, full);
  *strchr(realname, (char)13) = 0;

  if (strlen(realname) < 8)
    beta_status = 5;
  else
    beta_status = 3;
}

// 3 - cmp

void betatest6(void) {
  int n;

  for (n = 0; n < strlen(betaname); n++)
    if (lower[betaname[n]])
      realname[n] = lower[realname[n]];
  if (strcmp(realname, betaname))
    beta_status = 5;
  else
    beta_status = 4;
}


///////////////////////////////////////////////////////////////////////////////
//      Main Program
///////////////////////////////////////////////////////////////////////////////

int safe;


void GetFree4kBlocks(void);

#ifdef MIXER
void print_init_flags(int flags) {
#define PFLAG(a)              \
  if (flags & MIX_INIT_##a) { \
    printf(#a " ");           \
  }
  PFLAG(FLAC);
  PFLAG(MOD);
  PFLAG(MP3);
  PFLAG(OGG);
  if (!flags) {
    printf("None");
  }
  printf("\n");
}
#endif

void get_error(int n);
extern uint8_t cerror[128];
void show_compile_message(byte *p);
extern int compilado;

int main(int argc, char *argv[]) {
  FILE *f;
  byte *prgbuf;
  unsigned n;
  debugprintf("Welcome to DIV\n");

  OSDEP_Init();

  atexit(free_resources);

  system_clock = &mclock;

  cpu_type = 5;

  return_mode = 0;
  exit_requested = 0;
  MustCreate = 1;
  next_order = 0;
  interpreting = 1;
  safe = 34; // Text in lower right corner
  compilemode = 0;

  if (argc > 1 && !strcmp(argv[1], "INIT"))
    interpreting = 0;
  else
    beta_status = 4;

  if (argc > 1 && !strcmp(argv[1], "TEST"))
    test_video = 1;

  if (argc > 1 && !strcmp(argv[1], "-c"))
    compilemode = 1;

  getcwd(tipo[0].path, PATH_MAX + 1);

  if (argc < 1)
    exit(0);

  _fullpath(full, argv[0], _MAX_PATH + 1);

#ifdef SHARE
  div_strcpy(exe_name, sizeof(exe_name), full);
#endif

#ifdef DIVGIT
  gitinit();
#endif

  strupr(full);
  n = strlen(full);

  while (n && full[n] != '/' && full[n] != '\\')
    n--;

  full[n] = 0;

  if (n > 0 && full[n - 1] == ':')
    div_strcat(full, sizeof(full), "/");

  _dos_setdrive((memptrsize)toupper(full[0]) - 'A' + 1, &n);

  if (cpu_type == 3)
    chdir("..");

  getcwd(tipo[1].path, PATH_MAX + 1);

  if (argc > 2 && (!strcmp(argv[2], "/safe") || !strcmp(argv[2], "/SAFE"))) {
    safe = 33;
    DaniDel("sound.cfg");
    DaniDel("system/setup.bin");
    DaniDel("system/session.dtf");
  } else {
    if ((f = fopen("system/setup.bin", "rb")) != NULL) {
      fclose(f);
      primera_vez = 0;
    } else {
      primera_vez = 1;
      mostrar_demo = 0;
    }
  }

  tipo[2].ext = "*.* *.MAP *.PCX *.BMP *.JPG *.JPE *.PNG *.GIF *.TGA *.TIF"; // Maps browser
  tipo[3].ext =
      "*.* *.PAL *.FPG *.FNT *.MAP *.BMP *.PCX *.JPG *.PNG *.GIF *.TGA *.TIF"; // Palette Browser
  tipo[4].ext = "*.FPG *.*";                                                   // FPG FILES
  tipo[5].ext = "*.FNT *.*";                                                   // FNT FILES
  tipo[6].ext = "*.IFS *.*";                                                   // IFS Font templates
  tipo[7].ext = "*.* *.7 *.WAV *.PCM *.MP3 *.OGG *.FLAC";                      // Audio files
  tipo[8].ext = "*.PRG *.*";                                                   // Program files

  tipo[9].ext = "*.* *.JPG *.PNG *.BMP *.TIF"; // wallpaper files

  tipo[10].ext = "*.PAL";       // Save Palettes
  tipo[11].ext = "*.WAV *.PCM"; // Save Audio
  tipo[12].ext = "*.PRJ";       // Save Project

  tipo[13].ext = "*.* *.13"; // unknown

  tipo[14].ext = "*.MAP *.PCX *.BMP";          // Save image
  tipo[15].ext = "*.WLD *.*";                  // 3D Map files
  tipo[16].ext = "*.* *.MOD *.S3M *.XM *.MID"; // Tracker modules

  for (n = 0; n < 24; n++) {
    tipo[n].default_choice = 0;
    tipo[n].first_visible = 0;
  }

  {
    FILE *ftest = fopen("system/lenguaje.div", "rb");
    if (ftest == NULL) {
      fprintf(stderr, "FATAL: system/lenguaje.div not found. Cannot start.\n");
      exit(1);
    }
    fclose(ftest);
  }
  initialize_texts((uint8_t *)"system/lenguaje.div");

  if (compilemode == 1) {
    init_compiler();

    compilado = 1;
    mouse_graf = 3;
    error_number = -1;

    if (argc < 3) {
      fprintf(stdout, "DIV Games Studio Compiler V2.02 - http://www.div-arena.co.uk\n");
      fprintf(stdout, "Usage: -c [program name] [output.exe]\n");
      exit(-1);
    }

    f = fopen(argv[2], "rb");

    if (f) {
      fseek(f, 0, SEEK_END);
      source_len = ftell(f);
      fseek(f, 0, SEEK_SET);
      prgbuf = (byte *)malloc(source_len + 10);
      if (prgbuf) {
        fprintf(stdout, "Loaded %zu bytes\n", fread(prgbuf, 1, source_len, f));
        source_ptr = prgbuf;
        comp();
        if (error_number >= 0) {
          get_error(500 + error_number);
          show_compile_message(cerror);
        } else {
          show_compile_message(texts[202]);
        }
        free(prgbuf);
        prgbuf = NULL;
      } else {
        printf("Out of memory\n");
        exit(-1);
      }
      fclose(f);
    } else {
      printf("Failed to open %s\n", argv[2]);
      exit(-1);
    }
    exit(0);
  }

  if (SDL_NumJoysticks() > 0)
    SDL_JoystickOpen(0);

#ifdef MIXER
  int flags = MIX_INIT_MOD | MIX_INIT_OGG | MIX_INIT_FLAC;

  int initted = Mix_Init(flags);

  if ((initted & flags) != flags) {
    printf("Mix_Init: %s\n", Mix_GetError());
    print_init_flags(initted);
  }
#endif

  OSDEP_SetCaption((char *)texts[34], "");
  SDL_ShowCursor(SDL_FALSE);

  check_mouse();

  Load_Cfgbin();

  div_strcpy(full, sizeof(full), "Be");
  div_strcat(full, sizeof(full), "ta");

  if (strstr((char *)texts[34], (char *)full) == NULL &&
      strstr((char *)texts[35], (char *)full) == NULL) {
    strupr(full);
    if (strstr((char *)texts[34], (char *)full) == NULL &&
        strstr((char *)texts[35], (char *)full) == NULL) {
      beta_status = 4;
    }
  }

  if (!interpreting) {
    textcolor(BRIGHT, WHITE, RED);
    printf("%s", texts[1]);
    textcolor(TXTRESET, WHITE, BLACK);
    printf("\n");
  }

  initialization();

  init_environment();

  /////////////////////////////////////////////////////////////////////////////

  if (beta_status == 0)
    show_dialog(betatest0);

#ifdef SHARE
  else {
    if (!interpreting) {
      chk_demo();

      if (mostrar_demo) {
        show_dialog(demo0);

        if (v_accept)
          help(2007);
      }
    }
  }
#endif

  main_loop(); // Environment


  /////////////////////////////////////////////////////////////////////////////

  mouse_graf = 3;
  flush_copy();

  if (auto_save_session || return_mode != 0)
    if (return_mode != 3)
      DownLoad_Desktop(); // If the test didn't fail

  Save_Cfgbin();

  finalization();

  finalize_texts();

#ifdef GRABADORA
  EndGrabador();
#endif


  if (return_mode == 1) {
    _dos_setdrive((memptrsize)toupper(*tipo[1].path) - 'A' + 1, &n);
    _chdir(tipo[1].path);
  } else {
    _dos_setdrive((memptrsize)toupper(*tipo[0].path) - 'A' + 1, &n);
    _chdir(tipo[0].path);
  }
  return (return_mode);
}

//-----------------------------------------------------------------------------
//      Create the titlebar
//-----------------------------------------------------------------------------

char cbs[65], cbn, cgs[65], cgn, crs[65], crn;

void create_title_bar(void) {
  int n, m;

  cbs[0] = c_b_low;
  cbs[64] = average_color(c_b_low, c0);
  n = 64;
  while (n > 1) {
    m = 0;
    while (m < 64) {
      cbs[m + n / 2] = average_color(cbs[m], cbs[m + n]);
      m += n;
    }
    n >>= 1;
  }
  for (cbn = 0, m = 0; m < 65; m++)
    if (cbs[m] != cbs[cbn])
      cbs[++cbn] = cbs[m];
  cbn++;

  cgs[0] = c12;
  cgs[64] = c01;
  n = 64;
  while (n > 1) {
    m = 0;
    while (m < 64) {
      cgs[m + n / 2] = average_color(cgs[m], cgs[m + n]);
      m += n;
    }
    n >>= 1;
  }
  for (cgn = 0, m = 0; m < 65; m++)
    if (cgs[m] != cgs[cgn])
      cgs[++cgn] = cgs[m];
  cgn++;

  crs[0] = average_color(c_r, c_r_low);
  crs[64] = average_color(c_r_low, c0);
  n = 64;
  while (n > 1) {
    m = 0;
    while (m < 64) {
      crs[m + n / 2] = average_color(crs[m], crs[m + n]);
      m += n;
    }
    n >>= 1;
  }
  for (crn = 0, m = 0; m < 65; m++)
    if (crs[m] != crs[crn])
      crs[++crn] = crs[m];
  crn++;
}

//-----------------------------------------------------------------------------
//      Draw environment
//-----------------------------------------------------------------------------

int error_code;
int nueva_sesion = 0;
void errhlp0(void);
void interr0(void);
void intmsg0(void);
void usuario0(void);
extern char user1[];
extern char user2[];

void init_environment() {
  FILE *f;
  int n;
  char cWork[256];

  if ((f = fopen("system/user.nfo", "rb")) != NULL) {
    fread(user1, 1, 128, f);
    fread(user2, 1, 128, f);
    fclose(f);
  }

  update_box(0, 0, vga_width, vga_height);
  full_redraw = 1;
  blit_screen(screen_buffer);

  if (!interpreting) {
    if (!strlen(user1) || !strlen(user2)) {
      show_dialog(usuario0);
    } else {
      show_dialog(copyright0);
    }
  }

  // If the DIV.DTF file doesn't exist or safe mode is requested

  if (CopyDesktop && !nueva_sesion && !primera_vez)
    UpLoad_Desktop();

  if (!primera_vez) {
    for (n = 0; n < max_windows; n++)
      if (window[n].click_handler == menu_principal2)
        break;
    if (n == max_windows)
      new_window(menu_principal0);
  }

  //  if (test_video) show_dialog(test0);
  // Have we come back from running a prog?
  if (interpreting) {
    flush_buffer();
    if ((f = fopen("system/exec.err", "rb")) != NULL) {
      fread(&error_code, 4, 1, f);
      if (error_code)
        show_dialog(interr0);
      else {
        fread(&v_mode, 1, 4, f);
        div_strcpy(cWork, sizeof(cWork), (char *)texts[366]);
        fread(cWork + strlen(cWork), 1, 256 - strlen(cWork), f);
        cWork[255] = 0;
        if (strlen(cWork) < 255)
          div_strcat(cWork, sizeof(cWork), "\"");
        v_text = cWork;
        show_dialog(intmsg0);
      }
      fclose(f);
    }
  }

  DaniDel("*.swp");
  _chdir("system");
  DaniDel("exec.*");
  _chdir("..");

  if (primera_vez) {
    new_window(menu_principal0);
    minimize_window();

    primera_vez = 2;
    help(2000);
    primera_vez = 0;
  }

  flush_buffer();
}

///////////////////////////////////////////////////////////////////////////////
//      Execution Error (fatal)
///////////////////////////////////////////////////////////////////////////////

// 1 - Out of memory - (a core malloc failed)
// 2 - Too many process
// 3 - Stack overflow
// 4 - DLL not found
// 5 - grande.fon / pequeño.fon not found
// 6 - graf_p.div / graf_g.div not found

void interr1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, w / 2, 12, 1, (uint8_t *)v_text, c3);
}

void interr2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void interr0(void) {
  v.type = 1;
  v.title = texts[367];
  v.h = 38;
  switch (error_code) {
  case 1:
    v_text = (char *)texts[368];
    break;
  case 2:
    v_text = (char *)texts[369];
    break;
  case 3:
    v_text = (char *)texts[370];
    break;
  case 4:
    v_text = (char *)texts[371];
    break;
  case 5:
    v_text = (char *)texts[372];
    break;
  case 6:
    v_text = (char *)texts[373];
    break;
  }
  v.w = text_len((byte *)v_text) + 8;
  if (v.w < 120)
    v.w = 120;
  v.paint_handler = interr1;
  v.click_handler = interr2;
  _button(100, v.w / 2, v.h - 14, 1);
}

//-----------------------------------------------------------------------------
//      Return from interpreter with exit ("message" code)
//-----------------------------------------------------------------------------

void intmsg1(void) {
  char cWork[256];
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, w / 2, 12, 1, (byte *)v_text, c3);
  div_snprintf(cWork, sizeof(cWork), (char *)texts[374], v_mode);
  wwrite(v.ptr, w, h, w / 2, 12 + 8, 1, (byte *)cWork, c3);
}

void intmsg2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void intmsg0(void) {
  v.type = 1;
  v.title = texts[375];
  v.w = text_len((byte *)v_text) + 8;
  if (v.w < 120)
    v.w = 120;
  v.h = 38 + 8;
  v.paint_handler = intmsg1;
  v.click_handler = intmsg2;
  _button(100, v.w / 2, v.h - 14, 1);
}

//-----------------------------------------------------------------------------
//      Main loop DIV / OS
//-----------------------------------------------------------------------------

/*
typedef void (__interrupt __far *TIRQHandler)(void);
extern void __far __interrupt __loadds Irq0Handler(void);
extern void SetIRQVector(int n, TIRQHandler vec);
extern TIRQHandler GetIRQVector(int n);
int IntIncr  = 65536;
*/

byte rndb(void);

int old_clock, loop_count = 0;
extern int frame_clock; // clock

///////////////////////////////////////////////////////////////////////////////
//      Environment
///////////////////////////////////////////////////////////////////////////////

void mainloop(void) {
  int n, m, oldn = max_windows;
  int llamar;
  char cwork[256], *p;

  if (dragging == 3) {      // drag == 3?
    goto fin_bucle_entorno; // end loop environment
  }

  poll_keyboard();

  //-------------------------------------------------------------------------
  // Find the window the mouse is over (n); n=max_windows if none
  //-------------------------------------------------------------------------

  n = 0;

  while (n < max_windows &&
         !(window[n].type > 0 && mouse_in(window[n].x, window[n].y, window[n].x + window[n].w - 1,
                                          window[n].y + window[n].h - 1)))
    n++;

  if (n < max_windows && dragging == 4 && window[n].order == drag_source)
    dragging = 5;

  //-------------------------------------------------------------------------
  //  Drag onto the wallpaper
  //-------------------------------------------------------------------------

  if (dragging == 4 && (n == max_windows || window[n].type == 2)) {
    dragging = 5;
    free_drag = 0;
    v_title = (char *)texts[57];
    v_text = NULL;
    show_dialog(aceptar0);

    if (v_accept) {
      if (v.type == 101)
        MustCreate = 0;

      if (!new_map(NULL)) {
        if (MustCreate == 0) {
          memcpy(v_map->filename, v.mapa->filename, 13);
          new_window(mapa0);
          MustCreate = 1;
        }

        v_map = window[1].mapa;
        memcpy(v.mapa->map, v_map->map, map_width * map_height);
        v.mapa->zoom = v_map->zoom;
        v.mapa->zoom_x = v_map->zoom_x;
        v.mapa->zoom_y = v_map->zoom_y;
        v.mapa->zoom_cx = v_map->zoom_cx;
        v.mapa->zoom_cy = v_map->zoom_cy;

        for (n = 0; n < 512; n++)
          v.mapa->puntos[n] = v_map->puntos[n];

        if (v_map->TengoNombre == 1) {
          v_map->TengoNombre = 0;
          memcpy(v.mapa->filename, v_map->filename, 13);
          memcpy(v.name, v_map->filename, 13);
          memcpy(v.title, v_map->filename, 13);
          v.mapa->fpg_code = 0;
          memcpy(v.mapa->description, v_map->description, 32);
        } else if (v_map->TengoNombre == 2) {
          v.mapa->TengoNombre = 0;
          v.mapa->fpg_code = v_map->fpg_code;
          memcpy(v.mapa->description, v_map->description, 32);
          memcpy(v.mapa->filename, v_map->filename, 13);
        } else
          v.mapa->fpg_code = 0;

        call((voidReturnType)v.paint_handler);
        blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);

      } else
        MustCreate = 1;
    }
    free_drag = 1;
  }

  //-------------------------------------------------------------------------
  // If we were previously in a window we've now left,
  // repaint it (to clear any highlights)
  //-------------------------------------------------------------------------

  if (dragging != 4) {
    if (n == 0)
      if (v.foreground == 1)
        if (!mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2,
                      v.y + v.h - 2 * big2))
          n--;

    if (n != oldn && oldn == 0) {
      if (v.foreground == 1) {
        wmouse_x = -1;
        wmouse_y = -1;
        m = mouse_b;
        mouse_b = 0;
        call((voidReturnType)v.click_handler);
        mouse_b = m;

        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
      }
    }
    oldn = max_windows;

    if (n < 0)
      n++;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Determine the shape of the cursor (mouse pointer)
  ///////////////////////////////////////////////////////////////////////////

  if (n >= max_windows || n < 0) {
    mouse_graf = 1;
  } else {
    switch (window[n].foreground) {
    case 0:
      mouse_graf = 7;
      break;
    case 1:
      if (mouse_in(window[n].x + 2 * big2, window[n].y + 2 * big2,
                   window[n].x + window[n].w - 2 * big2, window[n].y + 9 * big2))
        if (mouse_x <= window[n].x + window[n].w - 18 * big2)
          mouse_graf = 2;
        else if (mouse_x <= window[n].x + window[n].w - 10 * big2)
          mouse_graf = 4;
        else
          mouse_graf = 5;
      else
        mouse_graf = 1;
      break;
    case 2:
      if (mouse_x >= window[n].x + 7 * big2)
        mouse_graf = 2;
      else
        mouse_graf = 6;
      break;
    }
  }

  //-------------------------------------------------------------------------
  // If we are on a window that is not the first ...
  //-------------------------------------------------------------------------

  if (n != 0 && n < max_windows) {
    // Bring window to position 0 if it's in the foreground or clicked

    if (window[n].foreground == 1 || (mouse_b & 1)) {
      move(0, n);
      n = 0;
    }

    if (n == 0 && v.foreground != 1) { // Clicked icon or background window
      for (m = 1; m < max_windows; m++) {
        if (window[m].type && window[m].foreground == 1) {
          if (windows_collide(0, m)) {
            window[m].foreground = 0;
            flush_window(m);
          }
        }
      }
      if (v.foreground == 0) { // If it was in the background
        if (v.type >= 100 && !v.state)
          activate(); // Activate a map

        v.foreground = 1;
        flush_window(0);
        do {
          read_mouse();
        } while (mouse_b & 1);

        prev_mouse_buttons = 0;
      }
    }

    //-------------------------------------------------------------------------
    // Activate an excludable window (type>=100) when interacted with
    //-------------------------------------------------------------------------

  } else if (n == 0 && (mouse_b & 1) && v.type >= 100 && v.foreground < 2 && !v.state &&
             mouse_graf != 4 && mouse_graf != 5) {
    activate();
    flush_window(0);
  }

  //-------------------------------------------------------------------------
  //  Drop something onto a background window
  //-------------------------------------------------------------------------

  if (n < max_windows && window[n].foreground == 0 && dragging == 4 && v.type >= 100 &&
      window[n].type != 2) {
    move(0, n);
    n = 0;

    if (v.type == 100)
      mouse_b |= 1;

    activate();
    wmouse_x = mouse_x - v.x;
    wmouse_y = mouse_y - v.y;

    if (big) {
      wmouse_x /= 2;
      wmouse_y /= 2;
    }

    call((voidReturnType)v.click_handler);

    for (m = 1; m < max_windows; m++)
      if (window[m].type && window[m].foreground == 1)
        if (windows_collide(0, m)) {
          window[m].foreground = 0;
          flush_window(m);
        }

    v.foreground = 1;
    flush_window(0);
    v.redraw = 0;
    do {
      read_mouse();
    } while (mouse_b & 1);

    goto fin_bucle_entorno; // end loop environment
  }

  //-------------------------------------------------------------------------
  // If we are within the contents of a window ...
  //-------------------------------------------------------------------------

  if (n == 0 && v.foreground == 1) {
    if (mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2)) {
      llamar = 1; // Call its click_handler

      if (v.type == 100 && dragging != 4) {
        if (dragging == 1) {
          drag_graphic = 8;
          dragging = 2;
          map_width = v.mapa->map_width;
          map_height = v.mapa->map_height;
        }

        llamar = 0;

        if ((mouse_b & 1) && !(prev_mouse_buttons & 1)) {
          if (*system_clock < double_click + 10 && *system_clock > double_click &&
              abs(double_click_x - mouse_x) < 8 && abs(double_click_y - mouse_y) < 8) {
            llamar = 1;
          } else {
            double_click = *system_clock;
            double_click_x = mouse_x;
            double_click_y = mouse_y;
          }
        }
      }

      // tipo==106 (3D map window) check removed (MODE8/3D map editor deleted)

      if (v.type >= 100 && dragging == 4) {
        if (v.type == 100)
          mouse_b |= 1;
        activate();
      }

      if (llamar) {
        wmouse_x = mouse_x - v.x;
        wmouse_y = mouse_y - v.y;

        if (big) {
          wmouse_x /= 2;
          wmouse_y /= 2;
        }

        call((voidReturnType)v.click_handler);
        volcados_parciales = 1;
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        volcados_parciales = 0;
      }

      oldn = 0;

      //-------------------------------------------------------------------------
    } else { // If we are in the control bar of the window ...
             //-------------------------------------------------------------------------
             //-----------------------------------------------------------------------
             // Move a window
             //-----------------------------------------------------------------------

      if (mouse_graf == 2 && (mouse_b & 1) && !(prev_mouse_buttons & 1)) {
        move_window();
      }

      //-----------------------------------------------------------------------
      // Close the window
      //-----------------------------------------------------------------------

      if (mouse_graf == 5) {
        if (mouse_b & 1) {
          if (big)
            wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
          else
            wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);

          flush_window(0);
        }

        if (!(mouse_b & 1) && (prev_mouse_buttons & 1)) {
          if (v.click_handler == menu_principal2) {
            if (big)
              wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
            else
              wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);

            flush_window(0);
            flush_copy();

            if (big)
              wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -35);
            else
              wput(v.ptr, v.w, v.h, v.w - 9, 2, -35);

            do {
              read_mouse();
            } while (mouse_b & 1);

            v_title = (char *)texts[40];
            v_text = NULL;
            show_dialog(aceptar0);

            if (v_accept)
              exit_requested = 1;
          } else if (v.type >= 100) {
            if (big)
              wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
            else
              wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);

            flush_window(0);
            flush_copy();

            if (big)
              wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -35);
            else
              wput(v.ptr, v.w, v.h, v.w - 9, 2, -35);

            do {
              read_mouse();
            } while (mouse_b & 1);

            switch (v.type) {
            case 100:
              v_title = (char *)texts[50];
              v_text = (char *)v.title;
              break;

            case 102:
              v_title = (char *)texts[188];
              v_text = (char *)v.title;
              break;
            }

            if (v.type == 100 || (v.type == 102 && v.prg != NULL))
              show_dialog(aceptar0);
            else
              v_accept = 1;

            if (v_accept)
              close_window();
          } else
            close_window();
        } else if (mouse_b & 1)
          restore_button = 3;
      }

      //-----------------------------------------------------------------------
      // Minimize a window
      //-----------------------------------------------------------------------

      if (mouse_graf == 4) {
        if (mouse_b & 1) {
          wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -47);
          flush_window(0);
        }

        if (!(mouse_b & 1) && (prev_mouse_buttons & 1)) {
          minimize_window();
        } else if (mouse_b & 1)
          restore_button = 1;
      }

      oldn = -1;
    }
  }

  //-------------------------------------------------------------------------
  // We are over an icon
  //-------------------------------------------------------------------------

  if (n == 0 && v.foreground == 2) {
    if (mouse_graf == 2) {
      if ((mouse_b & 1) && !(prev_mouse_buttons & 1))
        move_window();
    } else {
      if (mouse_b & 1) {
        if (big) {
          wput(screen_buffer, -vga_width, vga_height, v.x, v.y, -48);
          blit_partial(v.x, v.y, 14, 14);
        } else {
          wput(screen_buffer, vga_width, vga_height, v.x, v.y, -48);
          blit_partial(v.x, v.y, 7, 7);
        }
      }

      if (!(mouse_b & 1) && (prev_mouse_buttons & 1)) {
        maximize_window();
      } else if (mouse_b & 1)
        restore_button = 2;
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  // Type windows control timer
  ///////////////////////////////////////////////////////////////////////////

fin_bucle_entorno:

  for (m = 0; m < max_windows; m++) {
    if (m == 0 &&
        mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2) &&
        window[m].type != 107)
      continue;

    if (window[m].type == 4 || (window[m].type == 101 && window[m].foreground != 2) ||
        (window[m].type == 107 && window[m].foreground != 2) ||
        (window[m].type == 102 && window[m].state && window[m].foreground != 2)) {
      if (m) {
        wup(m);
      }

      window_closing = 1;
      wmouse_x = -1;
      wmouse_y = -1;

      switch (v.type) {
      case 101:
        load_thumbs();
        break;

      case 107:
        mostrar_mod_meters();
        break;

      default:
        call((voidReturnType)v.click_handler);
        break;
      }

      if (m) {
        wdown(m);
      }

      if (window_closing == 2) { // Close the window(m)
        move(0, m);
        close_window();
      } else if (window[m].redraw) {
        volcados_parciales = 1;
        flush_window(m);
        window[m].redraw = 0;
        volcados_parciales = 0;
      }

      window_closing = 0;
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  //  Program menu hotkeys
  ///////////////////////////////////////////////////////////////////////////

  for (m = 0; m < max_windows; m++)
    if (window[m].type == 102 && window[m].state && window[m].prg != NULL)
      break;

  if (m < max_windows && beta_status == 4) { // If a PRG ...
    n = 0;
    if (shift_status & 8)
      switch (scan_code) {
      case 33:
        n = 1;
        break; // alt+f

      case 49:
        n = 2;
        break; // alt+n

      case 19:
        n = 3;
        break; // alt+r
      }
    else if (shift_status & 4)

      switch (scan_code) {
      case 33:
        n = 1;
        break; // ctrl+f

      case 38:
        n = 2;
        break; // ctrl+l

      case 19:
        n = 3;
        break; // ctrl+r

      case 44:
        n = 9;
        break; // ctrl+z
      }
    else
      switch (scan_code) {
      case 60:
        n = 4;
        break; // f2

      case 61:
        n = 2;
        break; // f3

      case 63:
        n = 8;
        break; // f5

      case 68:
        n = 6;
        break; // f10
      }

    if (kbdFLAGS[87])
      n = 5; // f11

    if (kbdFLAGS[88])
      n = 7; // f12

    if (n) { // If a hotkey was pressed ...
      if (m) {
        wmouse_x = -1;
        wmouse_y = -1;
        mouse_b = 0;
        call((voidReturnType)v.click_handler);

        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }

        move(0, m);

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
      }

      switch (n) {
      case 1:
        show_dialog(find_text0);
        if (v_accept)
          find_text();
        break;

      case 2:
        find_text();
        break;

      case 3:
        show_dialog(replace_text0);
        if (v_accept)
          replace_text();
        break;

      case 4:
        mouse_graf = 3;
        flush_copy();
        v_type = 8;
        save_prg_buffer(0);
        div_strcpy(tipo[8].path, sizeof(tipo[8].path), v.prg->path);
        div_strcpy(input, sizeof(input), v.prg->filename);
        save_program();
        break;

      case 5:
        v_type = 8;
        save_prg_buffer(0);
        source_ptr = v.prg->buffer;
        source_len = v.prg->file_len;
        v_window = 0;
        run_mode = 0;
        compile_program();

        if (error_number != -1) {
          goto_error();
          if (v_help)
            help(500 + error_number);
        } else if (v_help)
          help(599);
        break;

      case 6:
      case 7:
        v_type = 8;
        save_prg_buffer(0);
        div_strcpy(tipo[8].path, sizeof(tipo[8].path), v.prg->path);
        div_strcpy(input, sizeof(input), v.prg->filename);
        save_program();
        source_ptr = v.prg->buffer;
        source_len = v.prg->file_len;
        v_window = 0;

        if (n == 6)
          run_mode = 1;
        else
          run_mode = 3;

        compile_program();

        if (error_number != -1) {
          goto_error();

          if (v_help)
            help(500 + error_number);

          break;
        }
        return_mode = 1;
        exit_requested = 1;
        break;

      case 8:
        v_type = 8;
        save_prg_buffer(0);

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
        }
        break;

      case 9:
        maximize();
        break;
      }
    }
  }

  //-------------------------------------------------------------------------
  // Beta verification
  //-------------------------------------------------------------------------

  if (rndb() > 200) {
    switch (beta_status) {
    case 1:
      call((voidReturnType)betatest4);
      break;

    case 2:
      call((voidReturnType)betatest5);
      break;

    case 3:
      call((voidReturnType)betatest6);
      break;

    case 5:
      exit_requested = 1;
      break;
    }
  }

  //-------------------------------------------------------------------------
  // Other hotkeys
  //-------------------------------------------------------------------------

  if (scan_code == 62) { // F4 Open PRG
    v_type = 8;
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
  }

  if (scan_code == 59) { // F1 Help
    determine_prg2();

    if (v_window != -1) {
      if (error_number != -1 && window[v_window].prg == eprg) {
        help(500 + error_number);
      } else {
        n = window[v_window].prg->column - 1;
        p = window[v_window].prg->l;

        if (n <= strlen(p)) {
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

            p = (char *)div_index;

            while (p < (char *)index_end) {
              if (!strcmp(cwork, p)) {
                help(*(word *)(p + strlen(p) + 1));
                break;
              } else
                p += strlen(p) + 3;
            }

            if (p >= (char *)index_end)
              help(3);

          } else
            help(3);

        } else
          help(3);
      }

    } else {
      determine_calc();

      if (v_window != -1) {
        help(1301);
      } else
        help(3);
    }
  }

  if ((shift_status & 8) && scan_code == 45) { // Alt-X Exit
    v_title = (char *)texts[40];
    v_text = NULL;
    show_dialog(aceptar0);

    if (v_accept)
      exit_requested = 1;
  }

  // Alt+S DOS shell disabled (no DOS)


  //-------------------------------------------------------------------------
  // End of the main loop
  //-------------------------------------------------------------------------

  flush_copy();

  if (restore_button == 1) {
    wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -37);
    flush_window(0);
  } else if (restore_button == 2) {
    if (big) {
      wput(screen_buffer, -vga_width, vga_height, v.x, v.y, -38);
      blit_partial(v.x, v.y, 14, 14);
    } else {
      wput(screen_buffer, vga_width, vga_height, v.x, v.y, -38);
      blit_partial(v.x, v.y, 7, 7);
    }
  } else if (restore_button == 3) {
    if (big)
      wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -35);
    else
      wput(v.ptr, v.w, v.h, v.w - 9, 2, -35);

    flush_window(0);
  }

  restore_button = 0;

  if (key(_ESC) && key(_L_CTRL))
    exit_requested = 1;
}

/* IDE main loop (desktop environment).
 * Repeatedly calls mainloop() to pump events, manage windows, and render
 * the desktop until exit_requested is set (quit requested).
 */
void main_loop(void) {
  div_started = 1;
  int n, m, oldn = max_windows;

  int llamar;

  char cwork[256], *p;

  do {
    mainloop();
  } while (!exit_requested);

  do {
    read_mouse();
  } while (mouse_b & 1);

  printf("env end\n");
}

//-----------------------------------------------------------------------------
//  Shell to the operating system
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//    Secondary loop DIV / OS ( management of a dialog box )
//-----------------------------------------------------------------------------

int n, m, oldn = max_windows;
int dialogo_invocado;
int salir_del_dialogo = 0;


void dialog_loop(void) {
  dialogo_invocado = 0;

  poll_keyboard();

  //-------------------------------------------------------------------------
  // Find the window the mouse is over (n); n=max_windows if none
  //-------------------------------------------------------------------------

  if (mouse_in(v.x, v.y, v.x + v.w - 1, v.y + v.h - 1))
    n = 0;
  else
    n = max_windows;

  //-------------------------------------------------------------------------
  // If we were previously in a window we've now left,
  // repaint it (to clear any highlights)
  //-------------------------------------------------------------------------

  if (n == 0) // If we're now on the toolbar, also repaint the window
    if (!mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2))
      n--;

  if (n != oldn && oldn == 0)
    if (v.foreground == 1) {
      dialogo_invocado = 1;
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      mouse_b = 0;

      call((voidReturnType)v.click_handler);
      mouse_b = m;

      volcados_parciales = 1;

      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }

      volcados_parciales = 0;
      salir_del_dialogo = 0;
    }

  oldn = max_windows;

  if (n < 0)
    n++;

  //-------------------------------------------------------------------------
  // Determine cursor shape
  //-------------------------------------------------------------------------

  if (n == max_windows)
    mouse_graf = 1;
  else if (mouse_in(v.x + 2 * big2, v.y + 2 * big2, v.x + v.w - 2 * big2, v.y + 9 * big2))
    if (mouse_x <= v.x + v.w - 10 * big2)
      mouse_graf = 2;
    else
      mouse_graf = 5;
  else
    mouse_graf = 1;

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

      call((voidReturnType)v.click_handler);
      volcados_parciales = 1;

      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }

      volcados_parciales = 0;
      oldn = 0;
      salir_del_dialogo = 0;

    } else { // If we are on the window's control toolbar ...

      if (mouse_graf == 2 && (mouse_b & 1) && !(prev_mouse_buttons & 1))
        move_window();

      if (mouse_graf == 5) {
        if (mouse_b & 1) {
          if (big)
            wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
          else
            wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);

          volcados_parciales = 1;
          flush_window(0);
          volcados_parciales = 0;
        }
        if (!(mouse_b & 1) && (prev_mouse_buttons & 1)) {
          close_window();
          salir_del_dialogo = 1;
        } else if (mouse_b & 1)
          restore_button = 3;
      }

      oldn = -1;
    }
  else if (draw_mode < 100 && (mouse_b & 1)) {
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
    call((voidReturnType)v.click_handler);
    mouse_b = m;
    volcados_parciales = 1;
    if (v.redraw) {
      flush_window(0);
      v.redraw = 0;
    }

    volcados_parciales = 0;
    salir_del_dialogo = 0;
  }

  if (end_dialog && !salir_del_dialogo) {
    close_window();
    salir_del_dialogo = 1;
  }

  //-------------------------------------------------------------------------
  // Keyboard Control
  //-------------------------------------------------------------------------

  if ((key(_ESC) && !key(_L_CTRL)) || (draw_mode < 100 && (mouse_b & 2))) {
    for (n = 0; n < v.items; n++)
      if (v.item[n].type == 2 && (v.item[n].state & 2))
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

//-----------------------------------------------------------------------------
//      Declares that window v has moved from the given position
//      (... to its current position v.x/y/an/al)
//-----------------------------------------------------------------------------

void on_window_moved(int x, int y, int w, int h) {
  int n, m;

  n = v.type;
  v.type = 0;
  if (draw_mode < 100) {
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
  } else
    update_box(x, y, w, h);
  v.type = n;

  if (v.type != 1) {
    for (n = 1; n < max_windows; n++)
      if (window[n].type && window[n].foreground == 0)

        // If a dimmed window was previously covered ...

        if (collides_with(n, x, y, w, h)) {
          window[n].foreground = 1;

          // If it's covered by other windows (foreground or background) ...

          for (m = 1; m < max_windows; m++)
            if (m != n && window[m].type &&
                (window[m].foreground == 1 || (window[m].foreground != 1 && m < n)))
              if (windows_collide(n, m))
                window[n].foreground = 0;

          // If we uncovered said window, bring it to the foreground

          if (window[n].foreground == 1) {
            if (n != 1) {
              move(1, n);
            }
            flush_window(1);
          }
        }

    for (n = 1; n < max_windows; n++)
      if (window[n].type && window[n].foreground == 1)
        if (windows_collide(0, n)) {
          window[n].foreground = 0;
          flush_window(n);
        }
  }
  flush_window(0);
}

//-----------------------------------------------------------------------------
//      Maximize the active window (window 0)
//-----------------------------------------------------------------------------

void maximize_window(void) {
  int x, y, w, h, n, m;

  if (big) {
    wput(screen_buffer, -vga_width, vga_height, v.x, v.y, -48);
    blit_partial(v.x, v.y, 14, 14);
  } else {
    wput(screen_buffer, vga_width, vga_height, v.x, v.y, -48);
    blit_partial(v.x, v.y, 7, 7);
  }
  flush_copy();

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;

  v.x = v._x;
  v.y = v._y;
  v.w = v._an;
  v.h = v._al;

  m = 0;
  for (n = 1; n < max_windows; n++)
    if (window[n].type && window[n].foreground == 1)
      if (windows_collide(0, n))
        m++;
  if (m)
    place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);

  v._x = x;
  v._y = y;
  v._an = w;
  v._al = h;
  v.foreground = 1;

  do {
    read_mouse();
  } while (mouse_b & 1);

  on_window_moved(x, y, w, h);

  if (v.type >= 100) {
    activate();
    flush_window(0);
  }
}

//-----------------------------------------------------------------------------
//      Minimize the active window (window 0)
//-----------------------------------------------------------------------------

void minimize_window(void) {
  int x, y, w, h, n, m;

  wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -47);
  flush_window(0);
  if (primera_vez != 1)
    flush_copy();
  wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -37);
  if (v.type >= 100)
    deactivate();

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;

  v.w = (7 + 1 + text_len(v.name) + 1) * big2;
  v.h = 7 * big2;

  if (v.w != v._an || v.h != v._al) { // If this is the first minimize
                                        // (or the name changed ...)
    place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
  } else {
    v.x = v._x;
    v.y = v._y;
    m = 0;
    for (n = 1; n < max_windows; n++)
      if (window[n].type && window[n].foreground == 2)
        if (windows_collide(0, n))
          m++;
    if (m)
      place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
  }

  v._x = x;
  v._y = y;
  v._an = w;
  v._al = h;
  v.foreground = 2;

  do {
    read_mouse();
  } while (mouse_b & 1);

  on_window_moved(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Close the active window (window 0)
//-----------------------------------------------------------------------------

void close_window(void) {
  int x = 0, y = 0, w = 0, h = 0, n = 0, m = 0;

  if (v.type == 102 && window_closing == 1) {
    window_closing = 2;
    return;
  }

  if (exploding_windows) {
    v.exploding = 1;
    implode(x, y, w, h);
    v.exploding = 0;
  }

  call((voidReturnType)v.close_handler);
  if (!cierra_rapido) {
    if (big)
      wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
    else
      wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);
    flush_window(0);
    blit_partial(v.x, v.y, v.w, v.h);
    flush_copy();
  }

  if (v.click_handler != err2) {
    free(v.ptr);
    v.ptr = NULL;
  }

  if (v.click_handler == help2 && old_prg != NULL) {
    for (m = 1; m < max_windows; m++) {
      if (window[m].click_handler == program2 || window[m].click_handler == calc2) {
        if ((window[m].prg == old_prg || window[m].aux == (byte *)old_prg) &&
            window[m].foreground < 2) {
          window[m].state = 1;
          wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c_b_low, 2, 2,
               window[m].w / big2 - 20, 7);
          if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
            wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                          window[m].h / big2, 4, 2, 0, window[m].title, c1);
            wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                          window[m].h / big2, 3, 2, 0, window[m].title, c4);
          } else {
            wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                   3 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c1);
            wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                   2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c4);
          }
          flush_window(m);
          v.state = 0;
          break;
        }
      }
    }
    old_prg = NULL;
  }

  if (v.type >= 100 && v.state) { // Maps auto-deactivate on close
    for (m = 1; m < max_windows; m++)
      if (window[m].type == v.type && window[m].foreground < 2) {
        window[m].state = 1;
        wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c_b_low, 2, 2,
             window[m].w / big2 - 20, 7);
        if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 4, 2, 0, window[m].title, c1);
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 3, 2, 0, window[m].title, c4);
        } else {
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 3 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c1);
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c4);
        }
        flush_window(m);
        break;
      }
  }

  n = v.type;

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;
  divdelete(0);
  if (draw_mode < 100) {
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
  } else
    update_box(x, y, w, h);

  if (n == 1 || n == 7) {             // Dialogs bring hidden[] windows to foreground on close
    if (v.type == 1 || v.type == 7) { // Dialog over dialog: only show the last one
      v.foreground = 1;
      flush_window(0);
    } else if (draw_mode >= 100)
      for (n = 0; n < max_windows; n++)
        if (hidden[n]) {
          window[n].foreground = 1;
          flush_window(n);
        }
  } else
    for (n = 0; n < max_windows; n++)
      if (window[n].type && window[n].foreground == 0)
        if (collides_with(n, x, y, w, h)) {
          window[n].foreground = 1;
          for (m = 0; m < max_windows; m++)
            if (m != n && window[m].type &&
                (window[m].foreground == 1 || (window[m].foreground == 2 && m < n)))
              if (windows_collide(n, m))
                window[n].foreground = 0;
          if (window[n].foreground == 1) {
            if (n != 0) {
              move(0, n);
            }
            flush_window(0);
          }
        }
  do {
    read_mouse();
    poll_keyboard();
  } while ((mouse_b) || key(_ESC) || key(_T) || key(_U));

  scan_code = 0;
  ascii = 0;
  mouse_b = 0;
}

//-----------------------------------------------------------------------------
//      Move a window
//-----------------------------------------------------------------------------

void move_window(void) {
  int ix, iy, oldx, oldy, w, h, b, x, y;

  if (move_full_window) {
    move_window_complete();
    return;
  }

  mouse_graf = 2;
  w = v.w;
  h = v.h;
  oldx = v.x;
  oldy = v.y;
  ix = mouse_x - oldx;
  iy = mouse_y - oldy;
  b = big;
  big = 0;

  do {
    x = mouse_x - ix;
    y = mouse_y - iy;

    wrectangle(screen_buffer, vga_width, vga_height, c4, x, y, w, h);
    blit_partial(x, y, w, 1);
    blit_partial(x, y, 1, h);
    blit_partial(x, y + h - 1, w, 1);
    blit_partial(x + w - 1, y, 1, h);
    flush_copy();

    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(x, y, w, h);
      flush_bars(0);
      update_dialogs(x, y, w, h);
      big = 0;
    } else {
      update_box(x, y, w, 1);
      update_box(x, y, 1, h);
      update_box(x, y + h - 1, w, 1);
      update_box(x + w - 1, y, 1, h);
    }

  } while (mouse_b & 1);

  big = b;
  if (x != oldx || y != oldy) {
    v.x = x;
    v.y = y;
    on_window_moved(oldx, oldy, w, h);
    full_redraw = 1;
    flush_copy();
  }

  //---------------------------------------------------------------------------
  // Check if double-click was pressed to auto-place the window
  //---------------------------------------------------------------------------

  x = 0; // Now x indicates whether the window was auto-placed

  if (oldx == v.x && oldy == v.y) {
    if (*system_clock < double_click + 10 && *system_clock > double_click &&
        abs(double_click_x - mouse_x) < 8 && abs(double_click_y - mouse_y) < 8) {
      if (v.foreground == 2)
        place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
      else
        place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
      if (v.x == oldx && v.y == oldy) {
        v.side ^= 1;
        if (v.foreground == 2)
          place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
        else
          place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
        if (v.x == oldx && v.y == oldy)
          v.side ^= 1;
      }
      if (v.x != oldx || v.y != oldy) {
        if (exploding_windows)
          extrude(oldx, oldy, v.w, v.h, v.x, v.y, v.w, v.h);
        on_window_moved(oldx, oldy, v.w, v.h);
        x++;
      }
    } else {
      double_click = *system_clock;
      double_click_x = mouse_x;
      double_click_y = mouse_y;
    }
  }

  if (!x && v.foreground == 1)
    flush_window(0);
}

void move_window_complete(void) {
  int ix, iy, oldx, oldy;
  int x, y, w, h;

  mouse_graf = 2;
  w = v.w;
  h = v.h;
  oldx = v.x;
  oldy = v.y;
  ix = mouse_x - oldx;
  iy = mouse_y - oldy;

  if (v.foreground == 1)
    wrectangle(v.ptr, w / big2, h / big2, c4, 0, 0, w / big2, h / big2);

  if (v.foreground == 2)
    window_move_pending = 1;

  do {
    x = v.x;
    y = v.y;
    v.x = mouse_x - ix;
    v.y = mouse_y - iy;
    on_window_moved(x, y, w, h);
    flush_copy();
  } while (mouse_b & 1);

  if (window_move_pending) {
    window_move_pending = 0;
    flush_window(0);
  }

  if (v.foreground == 1)
    wrectangle(v.ptr, w / big2, h / big2, c2, 0, 0, w / big2, h / big2);

  //---------------------------------------------------------------------------
  // Check if you pressed double-click to auto deploy window
  //---------------------------------------------------------------------------

  x = 0; // Now x indicates whether the window was auto-placed

  if (oldx == v.x && oldy == v.y) {
    if (*system_clock < double_click + 10 && *system_clock > double_click &&
        abs(double_click_x - mouse_x) < 8 && abs(double_click_y - mouse_y) < 8) {
      if (v.foreground == 2)
        place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
      else
        place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
      if (v.x == oldx && v.y == oldy) {
        v.side ^= 1;
        if (v.foreground == 2)
          place_window(v.side * 2 + 0, &v.x, &v.y, v.w, v.h);
        else
          place_window(v.side * 2 + 1, &v.x, &v.y, v.w, v.h);
        if (v.x == oldx && v.y == oldy)
          v.side ^= 1;
      }
      if (v.x != oldx || v.y != oldy) {
        if (exploding_windows)
          extrude(oldx, oldy, v.w, v.h, v.x, v.y, v.w, v.h);
        on_window_moved(oldx, oldy, v.w, v.h);
        x++;
      }
    } else {
      double_click = *system_clock;
      double_click_x = mouse_x;
      double_click_y = mouse_y;
    }
  }

  if (!x && v.foreground == 1)
    flush_window(0);
}

//-----------------------------------------------------------------------------
//      updates a screen box (region)
//-----------------------------------------------------------------------------

void update_box(int x, int y, int w, int h) {
  int n = 0;
  byte *_ptr = NULL;
  int _x = 0, _y = 0, _an = 0, _al = 0;
  int salta_x = 0, salta_y = 0;


  byte *div_version;

#ifdef GIT_SHA1

  char divver[255];
  div_strcpy(divver, sizeof(divver), "DIVDX BUILD ");
  div_strcat(divver, sizeof(divver), GIT_SHA1);
  div_version = (byte *)divver;

#else
  div_version = texts[safe];
#endif


  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;
  if (w <= 0 || h <= 0)
    return;

  restore_wallpaper(x, y, w, h);

  if (y < vga_height && y + h > vga_height - 8 * big2 && x < vga_width &&
      x + w >= vga_width - (text_len(div_version) + 2) * big2) {
    wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, vga_width - 1 - x,
                  vga_height - 1 - y, 18, div_version, c0);
    wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, vga_width - 2 - x,
                  vga_height - 1 - y, 18, div_version, c2);
  }

  for (n = max_windows - 1; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
          _y = window[n].y;
          _an = window[n].w;
          _al = window[n].h;

          if (y > _y) {
            salta_y += y - _y;
            _ptr += _an * salta_y;
            _y = y;
            _al -= salta_y;
          }
          if (y + h < _y + _al) {
            salta_y += _y + _al - y - h;
            _al -= _y + _al - y - h;
          }
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1)
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            else
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                              salta_x);
          }

        } else {
          if (n == 0 && window_move_pending) {
            if (big) {
              big = 0;
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 16, v.y, v.w - 16, v.h);
              big = 1;
            } else
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 8, v.y, v.w - 8, v.h);
          } else {
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 9 * big2 - x, window[n].y - y, 10, window[n].name, c0);
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 8 * big2 - x, window[n].y - y, 10, window[n].name, c4);
          }

          if (x < window[n].x + 7 * big2) {
            if (big) {
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, -w, h, window[n].x - x,
                          window[n].y - y, 38);
            } else
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, window[n].x - x,
                          window[n].y - y, 38);
          }
        }
      }
  blit_partial(x, y, w, h);
}

void update_dialogs(int x, int y, int w, int h) {
  int n;
  byte *_ptr;
  int _x, _y, _an, _al;
  int salta_x, salta_y;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;
  if (w <= 0 || h <= 0)
    return;

  for (n = max_windows - 1; n >= 0; n--)
    if (window[n].type == 1)
      if (collides_with(n, x, y, w, h))
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
          _y = window[n].y;
          _an = window[n].w;
          _al = window[n].h;

          if (y > _y) {
            salta_y += y - _y;
            _ptr += _an * salta_y;
            _y = y;
            _al -= salta_y;
          }
          if (y + h < _y + _al) {
            salta_y += _y + _al - y - h;
            _al -= _y + _al - y - h;
          }
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1)
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            else
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                              salta_x);
          }
        }
}

//-----------------------------------------------------------------------------
//      Update a screen region, from a given window upward
//-----------------------------------------------------------------------------

void update_box2(int vent, int x, int y, int w, int h) {
  int n;
  byte *_ptr;
  int _x, _y, _an, _al;
  int salta_x, salta_y;

  if (window[vent].foreground == 2) {
    update_box(x, y, w, h);
    return;
  }

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;
  if (w <= 0 || h <= 0)
    return;

  for (n = vent; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
          _y = window[n].y;
          _an = window[n].w;
          _al = window[n].h;

          if (y > _y) {
            salta_y += y - _y;
            _ptr += _an * salta_y;
            _y = y;
            _al -= salta_y;
          }
          if (y + h < _y + _al) {
            salta_y += _y + _al - y - h;
            _al -= _y + _al - y - h;
          }
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1)
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            else
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                              salta_x);
          }

        } else {
          if (n == 0 && window_move_pending) {
            if (big) {
              big = 0;
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 16, v.y, v.w - 16, v.h);
              big = 1;
            } else
              wrectangle(screen_buffer, vga_width, vga_height, c4, v.x + 8, v.y, v.w - 8, v.h);
          } else {
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 9 * big2 - x, window[n].y - y, 10, window[n].name, c0);
            wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                          window[n].x + 8 * big2 - x, window[n].y - y, 10, window[n].name, c4);
          }

          if (x < window[n].x + 7 * big2) {
            if (big) {
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, -w, h, window[n].x - x,
                          window[n].y - y, 38);
            } else
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, window[n].x - x,
                          window[n].y - y, 38);
          }
        }
      }

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Update the background around a zoom window (zoom_win_x,zoom_win_y,zoom_win_width,zoom_win_height)
//-----------------------------------------------------------------------------

void update_background(void) {
  int y0 = 0, y1 = vga_height - 1;
  int _big = big;

  if (zoom_win_y > 2) {
    y0 = zoom_win_y - 2;
    update_box(0, 0, vga_width, zoom_win_y - 2);
  }
  if (zoom_win_y + zoom_win_height < vga_height - 2) {
    y1 = zoom_win_y + zoom_win_height + 1;
    update_box(0, zoom_win_y + zoom_win_height + 2, vga_width,
               vga_height - 2 - zoom_win_y - zoom_win_height);
  }
  if (zoom_win_x > 2) {
    update_box(0, y0, zoom_win_x - 2, y1 - y0 + 1);
  }
  if (zoom_win_x + zoom_win_width < vga_width - 2) {
    update_box(zoom_win_x + zoom_win_width + 2, y0, vga_width - 2 - zoom_win_x - zoom_win_width,
               y1 - y0 + 1);
  }

  if (_big) {
    big = 0;
    big2 = 1;
  }

  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y - 2, zoom_win_width + 4,
       1);
  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y + zoom_win_height + 1,
       zoom_win_width + 4, 1);
  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x - 2, zoom_win_y - 1, 1,
       zoom_win_height + 2);
  wbox(screen_buffer, vga_width, vga_height, c2, zoom_win_x + zoom_win_width + 1, zoom_win_y - 1, 1,
       zoom_win_height + 2);

  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y - 1, zoom_win_width + 2,
       1);
  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y + zoom_win_height,
       zoom_win_width + 2, 1);
  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x - 1, zoom_win_y, 1, zoom_win_height);
  wbox(screen_buffer, vga_width, vga_height, c0, zoom_win_x + zoom_win_width, zoom_win_y, 1,
       zoom_win_height);

  if (_big) {
    big = 1;
    big2 = 2;
  }
}

//-----------------------------------------------------------------------------
//      Resets the bacground (wallpaper) in a given area
//-----------------------------------------------------------------------------

void restore_wallpaper(int x, int y, int w, int h) {
  byte *p;
  byte *t;
  int n, _an;

  if (y < 0)
    y = 0;
  if (x < 0)
    x = 0;
  if (y + h > vga_height)
    h = vga_height - y;
  if (x + w > vga_width)
    w = vga_width - x;

  if (w > 0 && h > 0) {
    p = screen_buffer + y * vga_width + x;
    if (wallpaper != NULL) {
      t = wallpaper_map + (y % wallpaper_height) * wallpaper_width;
      _an = w;
      do {
        n = x;
        do {
          if (n >= wallpaper_width)
            n %= wallpaper_width;
          *p = *(t + n);
          p++;
          n++;
        } while (--w);
        w = _an;
        p += vga_width - w;
        t += wallpaper_width;
        if (t == wallpaper_map + wallpaper_width * wallpaper_height)
          t = wallpaper_map;
      } while (--h);
      /*
      t=wallpaper+(y&127)*128;
      _an=an;
      do {
        n=x;
        do {
          *p++=*(t+(n++&127));
        } while (--an);
        an=_an; p+=vga_width-an;
        if ((t+=128)==wallpaper+128*128) t=wallpaper;
      } while (--al);
      */
    } else {
      do {
        memset(p, c1, w);
        p += vga_width;
      } while (--h);
    }
  }
}

//-----------------------------------------------------------------------------
//    Dumps a window, according this standard or obscured or not in the foreground
//-----------------------------------------------------------------------------

void flush_window(int m) {
  int x, y, w, h, n;
  byte *_ptr;
  int _x, _y, _an, _al;
  int salta_x, salta_y;

  SDL_Rect trc;

  if (skip_window_render)
    return;

  if (volcados_parciales) {
    if (window[m].type == 102 && window[m].redraw == 1 && window[m].prg != NULL) {
      window[m].redraw = 0;
      update_box2(m, window[m].x + 2 * big2, window[m].y + 10 * big2, window[m].w - 20, 7 * big2);
      update_box2(m, window[m].x + 2 * big2,
                  window[m].y + 18 * big2 +
                      (window[m].prg->line - window[m].prg->first_line) * font_height,
                  window[m].w - 12, font_height);
      if (window[m].prg->prev_line != window[m].prg->line - window[m].prg->first_line)
        update_box2(m, window[m].x + 2 * big2,
                    window[m].y + 18 * big2 + window[m].prg->prev_line * font_height,
                    window[m].w - 12, font_height);
      return;
    }
    // MapperCreator2 check removed (MODE8/3D map editor deleted)
  }
  window[m].redraw = 0;

  x = window[m].x;
  y = window[m].y;
  w = window[m].w;
  h = window[m].h;

  if (window[m].foreground == 2) {
    update_box(x, y, w, h);
    return;
  }

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > vga_width)
    w = vga_width - x;
  if (y + h > vga_height)
    h = vga_height - y;

  for (n = m; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        if (window[n].foreground < 2) {
          _ptr = window[n].ptr;
          if (_ptr == NULL)
            return;

          salta_x = 0;
          salta_y = 0;
          _x = window[n].x;
          _y = window[n].y;
          _an = window[n].w;
          _al = window[n].h;

          if (y > _y) {
            salta_y += y - _y;
            _ptr += _an * salta_y;
            _y = y;
            _al -= salta_y;
          }
          if (y + h < _y + _al) {
            salta_y += _y + _al - y - h;
            _al -= _y + _al - y - h;
          }
          if (x > _x) {
            salta_x += x - _x;
            _ptr += salta_x;
            _x = x;
            _an -= salta_x;
          }
          if (x + w < _x + _an) {
            salta_x += _x + _an - x - w;
            _an -= _x + _an - x - w;
          }

          if (_an > 0 && _al > 0) {
            if (window[n].foreground == 1) {
              blit_region(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al, salta_x);
            } else {
              blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, _x, _y, _an, _al,
                              salta_x);
            }
          }
        } else {
          wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                        window[n].x + 9 * big2 - x, window[n].y - y, 10, window[n].name, c0);
          wwrite_in_box(screen_buffer + y * vga_width + x, vga_width, w, h,
                        window[n].x + 8 * big2 - x, window[n].y - y, 10, window[n].name, c4);

          if (x < window[n].x + 7 * big2) {
            if (big) {
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, -w, h, window[n].x - x,
                          window[n].y - y, 38);
            } else
              wput_in_box(screen_buffer + y * vga_width + x, vga_width, w, h, window[n].x - x,
                          window[n].y - y, 38);
          }
        }
      }

  trc.x = window[n].x;
  trc.y = window[n].y;
  trc.w = window[n].w;
  trc.h = window[n].h;

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Window placement algorithm
//      (flag=0: icon, flag=1: window)
//-----------------------------------------------------------------------------

void place_window(int flag, int *_x, int *_y, int w, int h) {
  int n, m, x, y, new_x, old_y = *_y;
  int scanes, scan[max_windows];

  // First build scan[] with possible Y positions (0 + window bottom edges)

  if (flag & 1) {
    *_y = vga_height;
    scan[0] = 0;
  } else {
    *_y = -1;
    scan[0] = vga_height - h;
  }
  scanes = 1;

  for (n = 1; n < max_windows; n++)
    if (window[n].type) {
      if (flag & 1) {
        if ((y = window[n].y + window[n].h + 1) < vga_height) {
          x = 0;
          do {
            x++;
          } while (x < scanes && y > scan[x]);
          if (x == scanes)
            scan[scanes++] = y;
          else if (y != scan[x]) {
            memmove(&scan[x + 1], &scan[x], 4 * (max_windows - x - 1));
            scan[x] = y;
            scanes++;
          }
        }
      } else {
        if ((y = window[n].y - h - 1) >= 0) {
          x = 0;
          do {
            x++;
          } while (x < scanes && y < scan[x]);
          if (x == scanes)
            scan[scanes++] = y;
          else if (y != scan[x]) {
            memmove(&scan[x + 1], &scan[x], 4 * (max_windows - x - 1));
            scan[x] = y;
            scanes++;
          }
        }
      }
    }

  // Second ... placement algorithm ...

  if (flag & 2) {
    for (n = 0; n < scanes; n++) {
      y = scan[n];
      new_x = vga_width - w;
      do {
        x = new_x;
        for (m = 1; m < max_windows; m++)
          if (window[m].type && collides_with(m, x - 1, y - 1, w + 2, h + 2))
            if (new_x >= window[m].x - w)
              new_x = window[m].x - w - 1;
      } while (new_x != x && new_x >= 0);
      if (new_x >= 0) {
        *_x = new_x;
        *_y = y;
        break;
      }
    }
  } else {
    for (n = 0; n < scanes; n++) {
      y = scan[n];
      new_x = 0;
      do {
        x = new_x;
        for (m = 1; m < max_windows; m++)
          if (window[m].type && collides_with(m, x - 1, y - 1, w + 2, h + 2))
            if (window[m].x + window[m].w >= new_x)
              new_x = window[m].x + window[m].w + 1;
      } while (new_x != x && new_x + w <= vga_width);
      if (new_x + w <= vga_width) {
        *_x = new_x;
        *_y = y;
        break;
      }
    }
  }

  // Third, if the algorithm failed, place the window at ...

  if (flag & 1) {
    if (*_y + h > vga_height)
      find_best_position(_x, _y, w, h);
  } else {
    if (*_y < 0)
      *_y = old_y;
  }
}

//-----------------------------------------------------------------------------
//    Forced placement algorithms
//-----------------------------------------------------------------------------

#define pasos_x 16
#define pasos_y 10

void find_best_position(int *_x, int *_y, int w, int h) {
  unsigned int c, colision = -1;
  int a, b, x, y;

  for (a = 0; a <= pasos_x; a++) {
    x = (float)((vga_width - w) * a) / (float)pasos_x;
    for (b = 0; b <= pasos_y; b++) {
      y = (float)((vga_height - h) * b) / (float)pasos_y;
      c = calculate_collision(x, y, w, h);
      if (c < colision) {
        colision = c;
        *_x = x;
        *_y = y;
      }
    }
  }
}

int calculate_collision(int x, int y, int w, int h) {
  int n, c, colision = 0;

  for (n = 1; n < max_windows; n++) {
    if (window[n].type) {
      if ((c = calculate_overlap(n, x, y, w, h))) {
        if (window[n].foreground)
          colision += vga_width * vga_height + c;
        else
          colision += vga_width * vga_height / 4 + c;
      }
    }
  }
  return (colision);
}

int calculate_overlap(int a, int x, int y, int w, int h) {
  int n, m, _an = 0, _al = 0;

  if (y < window[a].y + window[a].h && y + h > window[a].y) {
    if (x < window[a].x + window[a].w && x + w > window[a].x) {
      n = y + h;
      m = window[a].y + window[a].h;
      if (n < m)
        m = n;
      if (window[a].y > y)
        n = window[a].y;
      else
        n = y;
      _an = m - n + 1;

      n = x + w;
      m = window[a].x + window[a].w;
      if (n < m)
        m = n;
      if (window[a].x > x)
        n = window[a].x;
      else
        n = x;
      _al = m - n + 1;

      return (_an * _al);
    }
  }
  return (0);
}

//-----------------------------------------------------------------------------
//    Check if two windows collide
//-----------------------------------------------------------------------------

int windows_collide(int a, int b) {
  if (window[b].y < window[a].y + window[a].h && window[b].y + window[b].h > window[a].y &&
      window[b].x < window[a].x + window[a].w && window[b].x + window[b].w > window[a].x)

    return (1);
  else
    return (0);
}

int collides_with(int a, int x, int y, int w, int h) {
  if (y < window[a].y + window[a].h && y + h > window[a].y && x < window[a].x + window[a].w &&
      x + w > window[a].x)

    return (1);
  else
    return (0);
}

//-----------------------------------------------------------------------------
//      Dump the virtual copy of the real screen (main display)
//-----------------------------------------------------------------------------

int leer_mouse = 1;

void flush_copy(void) {
  if (leer_mouse)
    read_mouse();
  if (draw_mode < 100) {
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 0);
    put(mouse_shift_x, mouse_shift_y, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_shift_x, mouse_shift_y, mouse_graf, 1);
  } else {
    save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 0);
    put(mouse_x, mouse_y, mouse_graf);
    blit_screen(screen_buffer);
    save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 1);
  }
}

void window_surface(int w, int h, byte type) {}


//-----------------------------------------------------------------------------
//      Create a new window
//-----------------------------------------------------------------------------

void new_window(voidReturnType init_handler) {
  byte *ptr;
  int n, m, om, x, y, w, h;
  int vtipo;
  uint32_t colorkey = 0;
  v.exploding = 0;

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  if (!window[max_windows - 1].type) {
    if (v.type) {
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      om = prev_mouse_buttons;
      mouse_b = 0;
      prev_mouse_buttons = 0;
      call((voidReturnType)v.click_handler);
      mouse_b = m;
      prev_mouse_buttons = om;
      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }
    }

    addwindow();

    //---------------------------------------------------------------------------
    // The following values should define the init_handler, default values:
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
    v._an = 0;
    v._al = 0;
    v.state = 0;
    v.buttons = 0;
    v.redraw = 0;
    v.items = 0;
    v.selected_item = -1;
    v.prg = NULL;
    v.aux = NULL;
    v.ptr = NULL;
    call(init_handler);

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

    if (primera_vez == 2) { // The help window (first time)
      y = x = vga_width / 2 - w / 2;
    } else if (v.type == 1 || v.type == 7) { // Dialogs are centered
      x = vga_width / 2 - w / 2;
      y = vga_height / 2 - h / 2;
    } else
      place_window(v.side * 2 + 1, &x, &y, w, h);

    v.x = x;
    v.y = y;

    //---------------------------------------------------------------------------
    // Check that if it's a map, no other map is already active
    //---------------------------------------------------------------------------

    if (v.type >= 100) {
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

          if (v.type == 102 && (window[m].prg != NULL || window[m].click_handler == calc2) &&
              window[m].type == 102) { // Erase cursor
            wup(m);
            call((voidReturnType)v.paint_handler);
            wdown(m);
          }
          vtipo = v.type;
          v.type = 0;
          flush_window(m);
          v.type = vtipo;
          break;
        }
    }

    //---------------------------------------------------------------------------
    // Check that if it's a menu, it hasn't already been created
    //---------------------------------------------------------------------------

    n = 0;
    if (v.type == 2 || v.type == 3 || v.type == 4 || v.type == 5 || v.type == 8) {
      for (m = 1; m < max_windows; m++)
        if (window[m].type == 2 || window[m].type == 3 || window[m].type == 4 ||
            window[m].type == 5 || window[m].type == 8)
          if (window[m].click_handler == v.click_handler)
            n = m;
    }

    if (!n)
      ptr = (byte *)malloc(w * h);
    else
      ptr = NULL;

    if (ptr != NULL) { // Window buffer, freed in close_window
      window_surface(w, h, 0);

      //---------------------------------------------------------------------------
      // Send appropriate windows to the background
      //---------------------------------------------------------------------------

      if (v.type == 1 || v.type == 7) { // Dialogs/progress bars hide all windows
        vtipo = v.type;
        v.type = 0;
        if (window[1].type == 1 || window[1].type == 7) { // Dialog over dialog
          window[1].foreground = 0;
          flush_window(1);
        } else
          for (n = 1; n < max_windows; n++)
            if (window[n].type && window[n].foreground == 1) {
              hidden[n - 1] = 1;
              window[n].foreground = 0;
              flush_window(n);
            } else
              hidden[n - 1] = 0;
      } else {
        vtipo = v.type;
        v.type = 0;
        for (n = 1; n < max_windows; n++) {
          if (window[n].type && window[n].foreground == 1) {
            if (windows_collide(0, n)) {
              if (n == 1) {
                wup(1);
                wmouse_x = -1;
                wmouse_y = -1;
                m = mouse_b;
                om = prev_mouse_buttons;
                mouse_b = 0;
                prev_mouse_buttons = 0;
                call((voidReturnType)v.click_handler);
                mouse_b = m;
                prev_mouse_buttons = om;
                v.foreground = 0;
                flush_window(0);
                v.redraw = 0;
                wdown(1);
              } else {
                window[n].foreground = 0;
                flush_window(n);
              }
            }
          }
        }
      }
      v.type = vtipo;

      //---------------------------------------------------------------------------
      // Initialize the window
      //---------------------------------------------------------------------------

      v.ptr = ptr;

      memset(ptr, c0, w * h);
      if (big) {
        w /= 2;
        h /= 2;
      }

      wrectangle(ptr, w, h, c2, 0, 0, w, h);
      wput(ptr, w, h, w - 9, 2, 35);

      if (v.type == 1) { // Dialogs can't be minimized
        wgra(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
        if (text_len(v.title) + 3 > w - 12) {
          wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
        }
      } else if (v.type == 7) { // Progress bar
        wgra(ptr, w, h, c_b_low, 2, 2, w - 4, 7);
        if (text_len(v.title) + 3 > w - 4) {
          wwrite_in_box(ptr, w, w - 3, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 3, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 4) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 4) / 2, 2, 1, v.title, c4);
        }
      } else {
        wput(ptr, w, h, w - 17, 2, 37);
        wgra(ptr, w, h, c_b_low, 2, 2, w - 20, 7);
        if (text_len(v.title) + 3 > w - 20) {
          wwrite_in_box(ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
          wwrite_in_box(ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
        } else {
          wwrite(ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
          wwrite(ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
        }
      }

      call((voidReturnType)v.paint_handler);

      if (big) {
        w *= 2;
        h *= 2;
      }

      if (primera_vez != 1) {
        do {
          read_mouse();
        } while ((mouse_b & 1) || key(_ESC));
        if (exploding_windows) {
          v.exploding = 1;
          explode(x, y, w, h);
          v.exploding = 0;
        }
        blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
        blit_partial(x, y, w, h);
      }

      //---------------------------------------------------------------------------
      // Could not open the window (no memory or duplicate menu)
      //---------------------------------------------------------------------------

    } else {
      if (n && window[n].foreground == 2) {
        divdelete(0);
        move(0, n - 1);
        maximize_window();
      } else if (n) {
        if (window[n].foreground == 0) {
          divdelete(0);
          move(0, n - 1);
          for (m = 1; m < max_windows; m++)
            if (window[m].type && window[m].foreground == 1)
              if (windows_collide(0, m)) {
                window[m].foreground = 0;
                flush_window(m);
              }
          v.foreground = 1;
          flush_window(0);
        } else { // When the requested menu is already in the foreground, highlight it
          divdelete(0);
          move(0, n - 1);
          wrectangle(v.ptr, v.w / big2, v.h / big2, c4, 0, 0, v.w / big2, v.h / big2);
          init_flush();
          flush_window(0);
          retrace_wait();
          flush_copy();
          wrectangle(v.ptr, v.w / big2, v.h / big2, c2, 0, 0, v.w / big2, v.h / big2);
          v.redraw = 1;
          retrace_wait();
          retrace_wait();
          retrace_wait();
          retrace_wait();
        }
      } else {
        divdelete(0);
      }
    }
  }
}

//-----------------------------------------------------------------------------
//      Explode animation for a new window
//-----------------------------------------------------------------------------

void init_flush(void);

void explode(int x, int y, int w, int h) {
  int n = 0, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  v.type = 0;
  big = 0;
  if (draw_mode < 100) {
    if (b)
      big = 1;
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
    big = 0;
  } else
    update_box(x, y, w, h);
  while (++n < 10) {
    aan = (w * n) / 10;
    aal = (h * n) / 10;
    xx = x + w / 2 - aan / 2;
    yy = y + h / 2 - aal / 2;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, 1);
    blit_partial(xx, yy, 1, aal);
    blit_partial(xx, yy + aal - 1, aan, 1);
    blit_partial(xx + aan - 1, yy, 1, aal);
    explode_num = n;
    retrace_wait();
    flush_copy();
    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(xx, yy, aan, aal);
      flush_bars(0);
      update_dialogs(xx, yy, aan, aal);
      big = 0;
    } else {
      update_box(xx, yy, aan, 1);
      update_box(xx, yy, 1, aal);
      update_box(xx, yy + aal - 1, aan, 1);
      update_box(xx + aan - 1, yy, 1, aal);
    }
  }
  v.type = tipo;
  big = b;
}

void implode(int x, int y, int w, int h) {
  int n = 9, b = big;
  int xx = 0, yy = 0, aan = 0, aal = 0;
  big = 0;
  do {
    aan = (w * n) / 10;
    if (!aan)
      aan = 1;
    aal = (h * n) / 10;
    if (!aal)
      aal = 1;
    xx = x + w / 2 - aan / 2;
    yy = y + h / 2 - aal / 2;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, 1);
    blit_partial(xx, yy, 1, aal);
    blit_partial(xx, yy + aal - 1, aan, 1);
    blit_partial(xx + aan - 1, yy, 1, aal);
    explode_num = n;
    flush_copy();
    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(xx, yy, aan, aal);
      flush_bars(0);
      update_dialogs(xx, yy, aan, aal);
      big = 0;
    } else {
      update_box(xx, yy, aan, 1);
      update_box(xx, yy, 1, aal);
      update_box(xx, yy + aal - 1, aan, 1);
      update_box(xx + aan - 1, yy, 1, aal);
    }
    retrace_wait();
  } while (--n);
  big = b;
}

void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
  int n = 9, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  v.type = 0;
  big = 0;
  if (draw_mode < 100) {
    if (b)
      big = 1;
    draw_edit_background(x, y, w, h);
    flush_bars(0);
    update_dialogs(x, y, w, h);
    big = 0;
  } else
    update_box(x, y, w, h);
  do {
    aan = (w * n + w2 * (10 - n)) / 10;
    aal = (h * n + h2 * (10 - n)) / 10;
    xx = (x * n + x2 * (10 - n)) / 10;
    yy = (y * n + y2 * (10 - n)) / 10;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, 1);
    blit_partial(xx, yy, 1, aal);
    blit_partial(xx, yy + aal - 1, aan, 1);
    blit_partial(xx + aan - 1, yy, 1, aal);
    if (primera_vez != 1)
      flush_copy();

    if (draw_mode < 100) {
      if (b)
        big = 1;
      draw_edit_background(xx, yy, aan, aal);
      flush_bars(0);
      update_dialogs(xx, yy, aan, aal);
      big = 0;
    } else {
      update_box(xx, yy, aan, 1);
      update_box(xx, yy, 1, aal);
      update_box(xx, yy + aal - 1, aan, 1);
      update_box(xx + aan - 1, yy, 1, aal);
    }

    if (primera_vez != 1)
      retrace_wait();

  } while (--n);
  big = b;
  v.type = tipo;
}

//-----------------------------------------------------------------------------
//      Create ( it must return to the caller as is) a dialog
//-----------------------------------------------------------------------------


void show_dialog(voidReturnType init_handler) {
  int vtipo, _get_pos;
  byte *ptr;
  int n, m, x, y, w, h;
  uint32_t colorkey = 0;

  if (!window[max_windows - 1].type) {
    _get_pos = get_pos;

    if (v.type) {
      wmouse_x = -1;
      wmouse_y = -1;
      m = mouse_b;
      mouse_b = 0;
      call((voidReturnType)v.click_handler);
      mouse_b = m;
      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }
    }

    addwindow();

    //---------------------------------------------------------------------------
    // The following values should define the init_handler, default values
    //---------------------------------------------------------------------------

    v.order = next_order++;
    v.type = 1;
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
    v._an = 0;
    v._al = 0;
    v.state = 0;
    v.buttons = 0;
    v.redraw = 0;
    v.items = 0;
    v.selected_item = -1;
    v.prg = NULL;
    v.aux = NULL;

    call((voidReturnType)init_handler);

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
      v.type = 0; // Megabug workaround

      if (draw_mode >= 100) {
        if (window[1].type == 1 || window[1].type == 7) { // Dialog over dialog
          window[1].foreground = 0;
          flush_window(1);
        } else {
          for (n = 1; n < max_windows; n++) {
            if (window[n].type && window[n].foreground == 1) {
              hidden[n - 1] = 1;
              window[n].foreground = 0;
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

      call((voidReturnType)v.paint_handler);

      if (big) {
        w *= 2;
        h *= 2;
      }

      do {
        read_mouse();
      } while ((mouse_b & 1) || key(_ESC));

      if (exploding_windows) {
        v.exploding = 1;
        explode(x, y, w, h);
        v.exploding = 0;
      }
      blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
      blit_partial(x, y, w, h);
      do {
        read_mouse();
      } while (mouse_b & 1);
      modal_loop();

      get_pos = _get_pos;

      //---------------------------------------------------------------------------
      // Could not open the dialog (no memory)
      //---------------------------------------------------------------------------

    } else {
      divdelete(0);
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

  call((voidReturnType)v.paint_handler);
}

//-----------------------------------------------------------------------------
//      Program initialization
//-----------------------------------------------------------------------------

void init_lexcolor(void);
void end_lexcolor(void);

/* IDE startup: allocate all core buffers and load essential resources.
 * Sets up video mode, keyboard, help index, palette tables, UI fonts,
 * icon graphics, sound system, and the paint toolbar.
 */
void initialization(void) {
  FILE *f;
  int n, a;
  byte *ptr, *ptr2;

  detect_vesa();

  printf("Num modes: %d (%d %d)\n", num_modes, vga_width, vga_height);

  for (n = 0; n < num_modes; n++) {
    if (modos[n].width == vga_width && modos[n].height == vga_height) {
      break;
    }
  }
  /*
	if (n==num_modes) {
		VS_WIDTH=vga_width  =Setupfile.Vid_modeAncho=640; // Video mode
		VS_HEIGHT =vga_height  =Setupfile.Vid_modeAlto=480;
		VS_BIG  =big     =Setupfile.Vid_modeBig=1;
		editor_font      =Setupfile.editor_font=3;
		big2=big+1;
	}
*/
  kbdInit();


  if (!interpreting) {
    printf("%s (%d,%d)\n\n", vga_marker, vga_width, vga_height); // *** User info only ***
    check_oldpif();
  }

  make_helpidx(); // *** Create the hypertext index ***
  load_index();   // *** Load the hypertext glossary ***

  if (!interpreting) {
    printf("%s", (char *)texts[6]); // *** Init graphics buffers ***
  }

  undo = (byte *)malloc(undo_memory);
  undo_table = (struct tipo_undo *)malloc(sizeof(struct tipo_undo) * max_undos);

  for (n = 0; n < max_windows; n++) {
    window[n].type = 0;
    window[n].side = 0;
  }

  mouse_background = (byte *)malloc(1024 * big2);
  screen_buffer = (byte *)malloc(vga_width * vga_height + 6) + 6;
  dac = (byte *)malloc(768);
  dac4 = (byte *)malloc(768);
  color_lookup = (byte *)malloc(16384);

  ghost = (byte *)malloc(65536); // 256*256 combinations

  toolbar = (byte *)malloc(vga_width * 19 * big2);
  fill_dac = (byte *)malloc(256);
  error_window = (byte *)malloc(640 * 38 * 2);

  if (mouse_background == NULL || screen_buffer == NULL || dac == NULL || dac4 == NULL ||
      color_lookup == NULL || ghost == NULL || toolbar == NULL || undo == NULL ||
      undo_table == NULL || fill_dac == NULL || error_window == NULL)
    error(0);

  if (big)
    f = fopen("system/grande.fon", "rb");
  else
    f = fopen("system/pequeno.fon", "rb");

  if (f == NULL)
    error(0);

  fseek(f, 0, SEEK_END);
  n = ftell(f);

  if ((text_font = (byte *)malloc(n)) != NULL) {
    fseek(f, 0, SEEK_SET);
    fread(text_font, 1, n, f);
    fclose(f);
  } else {
    fclose(f);
    error(0);
  }

  switch (editor_font) {
  case 0:
    f = fopen("system/SYS06X08.BIN", "rb");
    font_width = 6;
    font_height = 8;
    break;

  case 1:
    f = fopen("system/SYS08X08.BIN", "rb");
    font_width = 8;
    font_height = 8;
    break;

  case 2:
    f = fopen("system/SYS08X11.BIN", "rb");
    font_width = 8;
    font_height = 11;
    break;

  case 3:
    f = fopen("system/SYS09X16.BIN", "rb");
    font_width = 9;
    font_height = 16;
    break;
  }
  char_size = font_width * font_height;


  if (f == NULL)
    error(0);

  fseek(f, 0, SEEK_END);
  n = ftell(f);

  if ((font = (byte *)malloc(n)) != NULL) {
    fseek(f, 0, SEEK_SET);
    fread(font, 1, n, f);
    fclose(f);
  } else {
    fclose(f);
    error(0);
  }


  editor_font_height = font_height;
  editor_font_width = font_width;

#ifdef IMAGE
  mouse_surface = IMG_Load("system/cursor.png");
#endif


  f = fopen("system/tab_cuad.div", "rb");
  fread(color_lookup, 4, 4096, f);
  fclose(f);

  if (big)
    f = fopen("system/GRAF_G.DIV", "rb");
  else
    f = fopen("system/GRAF_P.DIV", "rb");

  if (f == NULL)
    error(0);

  else {
    fseek(f, 0, SEEK_END);

    n = ftell(f) - 1352;

    if (n > 0 && (graf_ptr = (byte *)malloc(n)) != NULL) {
      memset(graf, 0, sizeof(graf));
      ptr = graf_ptr;
      fseek(f, 1352, SEEK_SET);
      fread(graf_ptr, 1, n, f);
      fclose(f);

      while (graf_ptr < ptr + n && *((int *)graf_ptr) < 256) {
        if (*(int *)(graf_ptr + 60)) {
          graf[*((int *)graf_ptr)] = graf_ptr + 60;
          *(word *)(graf_ptr + 60) = *(int *)(graf_ptr + 52);
          *(word *)(graf_ptr + 62) = *(int *)(graf_ptr + 56);
          graf_ptr += *(word *)(graf_ptr + 60) * *(word *)(graf_ptr + 62) + 68;
        } else {
          graf[*((int *)graf_ptr)] = graf_ptr + 56;
          *(word *)(graf_ptr + 58) = *(int *)(graf_ptr + 56);
          *(word *)(graf_ptr + 56) = *(int *)(graf_ptr + 52);
          *(int *)(graf_ptr + 60) = 0;
          graf_ptr += *(word *)(graf_ptr + 56) * *(word *)(graf_ptr + 58) + 64;
        }
      }
      graf_ptr = ptr;
    } else {
      fclose(f);
      error(0);
    }
  }

  // HYPERLINK

  // *** Initialize graf_help[384].offset/an/al/ptr=0

  if ((f = fopen("help/help.fig", "rb")) == NULL)
    error(0);
  else {
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    if ((ptr2 = (byte *)malloc(n)) != NULL) {
      memset(graf_help, 0, sizeof(graf_help));
      ptr = ptr2;
      fseek(f, 0, SEEK_SET);
      fread(ptr2, 1, n, f);
      fclose(f);
      ptr2 += 1352;

      while (ptr2 < ptr + n && *((int *)ptr2) < 384) {
        graf_help[*(int *)ptr2].w = *(int *)(ptr2 + 52);
        graf_help[*(int *)ptr2].h = *(int *)(ptr2 + 56);
        graf_help[*(int *)ptr2].offset = (ptr2 - ptr) + 64 + 4 * (*(int *)(ptr2 + 60));
        ptr2 += *(int *)(ptr2 + 52) * *(int *)(ptr2 + 56) + 64 + 4 * (*(int *)(ptr2 + 60));
      }
      free(ptr);
    } else {
      fclose(f);
      error(0);
    }
  }

  if (!interpreting) {
    printf("%s", (char *)texts[10]);
  } // *** Load edited objects ***

  if (auto_save_session || interpreting)
    CopyDesktop = Can_UpLoad_Desktop();

  if (!CopyDesktop) { // Load common palette
    if (!interpreting) {
      cprintf("%s", (char *)texts[11]);
    } // *** Palette calculations ***
    memcpy(dac, system_dac, 768);
    init_ghost();
    create_ghost(1);
  }

  if (!interpreting) {
    printf("%s", (char *)texts[12]);
  } // *** Miscellaneous ***
  find_colors();
  memset(screen_buffer, c0, vga_width * vga_height);
  zoom = 0;
  zoom_x = 0;
  zoom_y = 0;
  zoom_cx = vga_width / 2;
  zoom_cy = vga_height / 2;
  zoom_move = c3;
  toolbar_x = 8 * big2;
  toolbar_y = vga_height - 27 * big2;
  gradient = 0;
  current_mouse = 21;
  sel_status = 0;
  undo_index = 0;
  for (a = 0; a < max_undos; a++)
    undo_table[a].mode = -1;
  color = 0;
  default_gradients();
  draw_mode = 101;
  mode_fill = 0;
  mode_circle = 0;
  mode_rect = 1;
  mode_selection = 0;
  v.type = 0;
  init_flush();
  next_code = 1;
  mouse_shift = 0;
  memcpy(original_palette, dac, 768);
  drag_source = 0;
  free_drag = 1;
  memset(mask, 0, 256);
  mask_on = 0;
  v_pause = 0;
  get = get_buffer;

  determine_units();

  init_compiler(); // *** Compiler *** spaces in lower[] set to 00
  init_lexcolor();

  if (!interpreting) {
    printf("%s", (char *)texts[13]);
  }

  create_gradient_colors(Setupfile.gradient_config, wallpaper_gradient);

  wallpaper = NULL;

  mouse_x = vga_width / 2;
  mouse_y = vga_height / 2;
  _mouse_x = mouse_x;
  _mouse_y = mouse_y;
  mouse_graf = 1;
  set_mouse(mouse_x, mouse_y); // set_mickeys(8);

  dragging = 0;

  update_box(0, 0, vga_width, vga_height);

  create_title_bar();

  setup_video_mode();
  set_dac(dac);
  //read_mouse();
  prepare_wallpaper();
  full_redraw = 1;
  blit_screen(screen_buffer);

  sound_init();
  if (SoundActive)
    set_init_mixer();
}

//-----------------------------------------------------------------------------
//      Return from program
//-----------------------------------------------------------------------------

/* IDE shutdown: free all buffers allocated by initialization().
 * Restores video mode, shuts down the syntax colorizer and keyboard.
 */
void finalization(void) {
  if (undo != NULL) {
    free(undo);
    undo = NULL;
  }
  if (graf_ptr != NULL) {
    free(graf_ptr);
    graf_ptr = NULL;
  }

  if (text_font != NULL) {
    free(text_font);
    graf_ptr = NULL;
  }

  free(font);

  free(wallpaper);
  free(fill_dac);
  free(toolbar);
  free(ghost);
  free(color_lookup);
  free(dac4);
  free(dac);
  free(screen_buffer - 6);
  free(mouse_background);
  free(undo_table);


  if (return_mode == 0 || return_mode == 3)
    reset_video_mode();

  end_lexcolor();

  kbdReset();
}

///////////////////////////////////////////////////////////////////////////////
//      Create default colour rules
///////////////////////////////////////////////////////////////////////////////

void default_gradients(void) {
  int a;

  for (a = 0; a < 16; a++) {
    gradients[a].num_colors = 16;
    gradients[a].type = 0;
    gradients[a].fixed = 0;
    gradients[a].colors[1] = a * 16;
    calculate_gradient(a);
  }
}

//-----------------------------------------------------------------------------
//      Function to determine the present drives (in string drives[])
//-----------------------------------------------------------------------------

void determine_units(void) {
#ifdef WIN32
  int n, m, uni = 0;
  DWORD cchBuffer;
  char *driveStrings;
  int driveType;
  char driveTypeString[255];
  uint64_t freeSpace;

  // Find out how big a buffer we need
  cchBuffer = GetLogicalDriveStrings(0, NULL);

  driveStrings = (char *)malloc((cchBuffer + 1) * sizeof(char));
  if (driveStrings == NULL) {
    return;
  }

  // Fetch all drive strings
  GetLogicalDriveStrings(cchBuffer, driveStrings);

  // Loop until we find the final '\0'
  // driveStrings is a double null terminated list of null terminated strings)
  while (*driveStrings) {
    // Dump drive information
    driveType = GetDriveType(driveStrings);

    switch (driveType) {
    case DRIVE_FIXED:
      div_strcpy(driveTypeString, sizeof(driveTypeString), "Hard disk");
      break;

    case DRIVE_CDROM:
      div_strcpy(driveTypeString, sizeof(driveTypeString), "CD/DVD");
      break;

    case DRIVE_REMOVABLE:
      div_strcpy(driveTypeString, sizeof(driveTypeString), "Removable");
      break;

    case DRIVE_REMOTE:
      div_strcpy(driveTypeString, sizeof(driveTypeString), "Network");
      break;

    default:
      div_strcpy(driveTypeString, sizeof(driveTypeString), "Unknown");
      break;
    }

    // Move to next drive string
    // +1 is to move past the null at the end of the string.
    drives[uni++] = driveStrings[0];
    driveStrings += lstrlen(driveStrings) + 1;
  }

  free(driveStrings);

  return;
#endif
}

//-----------------------------------------------------------------------------
//      Generic routine to write buffer to file
//-----------------------------------------------------------------------------

void _fwrite(char *s, byte *buf, int n) {
  FILE *es;
  es = fopen(s, "wb");
  fwrite(buf, 1, n, es);
  fclose(es);
}

//-----------------------------------------------------------------------------
//      Generic function to error and exit to system
//-----------------------------------------------------------------------------

void error(int n) {
  debugprintf("WHOOPS!\n");
  finalization();
  printf((char *)texts[14], n);
  printf("\n");
  exit(0);
}

//-----------------------------------------------------------------------------
//      Debug system
//-----------------------------------------------------------------------------

struct _meminfo {
  unsigned data1;
  unsigned data2;
  unsigned data3;
  unsigned data4;
  unsigned data5;
  unsigned data6;
  unsigned data7;
  unsigned data8;
  unsigned data9;
} MemInfo;

char MemoriaLibre[100];

int MaxMemUsed = 0;

int GetHeapFree(int RetUsed) {
  return 0;
}

char *GetMemoryFree() {
  return "";
}

void DebugFile(char *Cadena, char *Nombre) {
  FILE *Debug;

  Debug = fopen("DEBUG.TXT", "ab");
  fwrite(Cadena, 1, strlen(Cadena), Debug);
  fwrite("(", 1, 1, Debug);
  fwrite(Nombre, 1, strlen(Nombre), Debug);
  fwrite(") ", 1, 2, Debug);
  fwrite(GetMemoryFree(), 1, strlen(GetMemoryFree()), Debug);
  fwrite(")\n", 1, 2, Debug);
  fclose(Debug);
}

//-----------------------------------------------------------------------------
//      Items definition
//-----------------------------------------------------------------------------

void _button(int t, int x, int y, int c) {
  v.item[v.items].type = 1;
  v.item[v.items].state = 0;
  v.item[v.items].button.text = texts[t];
  v.item[v.items].button.x = x;
  v.item[v.items].button.y = y;
  v.item[v.items].button.center = c;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _get(int t, int x, int y, int w, byte *buffer, int lon_buffer, int r0, int r1) {
  v.item[v.items].type = 2;
  v.item[v.items].state = 0;
  v.item[v.items].get.text = texts[t];
  v.item[v.items].get.x = x;
  v.item[v.items].get.y = y;
  v.item[v.items].get.w = w;
  v.item[v.items].get.buffer = buffer;
  v.item[v.items].get.lon_buffer = lon_buffer;
  v.item[v.items].get.r0 = r0;
  v.item[v.items].get.r1 = r1;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _flag(int t, int x, int y, int *valor) {
  v.item[v.items].type = 3;
  v.item[v.items].state = 0;
  v.item[v.items].flag.text = texts[t];
  v.item[v.items].flag.x = x;
  v.item[v.items].flag.y = y;
  v.item[v.items].flag.valor = valor;
  v.items++;
}

//-----------------------------------------------------------------------------
//      Print (show) items
//-----------------------------------------------------------------------------

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
  kbdFLAGS[28] = 0;
  show_items_called = 0;
}

void show_button(struct t_item *i) {
  wwrite(v.ptr, v.w / big2, v.h / big2, i->button.x, i->button.y, i->button.center,
         i->button.text, c3);
  if (&v.item[v.selected_item] == i)
    select_button(i, 1);
}

void select_button(struct t_item *i, int activo) {
  int x = i->button.x, y = i->button.y;
  int w, h;
  w = text_len(i->button.text + 1);
  h = 7;
  switch (i->button.center) {
  case 0:
    break;
  case 1:
    x = x - (w >> 1);
    break;
  case 2:
    x = x - w + 1;
    break;
  case 3:
    y = y - (h >> 1);
    break;
  case 4:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case 5:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case 6:
    y = y - h + 1;
    break;
  case 7:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case 8:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }
  if (activo) {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c12, x - 4, y - 4, w + 8, h + 8);
  } else {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c2, x - 4, y - 4, w + 8, h + 8);
  }
}

void show_get(struct t_item *i) {
  wbox(v.ptr, v.w / big2, v.h / big2, c1, i->get.x, i->get.y + 8, i->get.w, 9);
  wwrite_in_box(v.ptr, v.w / big2, i->get.w - 1 + i->get.x, v.h / big2, i->get.x + 1,
                i->get.y + 9, 0, i->get.buffer, c3);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->get.x + 1, i->get.y, 0, i->get.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->get.x, i->get.y, 0, i->get.text, c3);
  if (&v.item[v.selected_item] == i) {
    if (i->state & 2)
      select_get(i, 0, 0);
    select_get(i, 1, 0);
  }
}

void select_get(struct t_item *i, int activo, int ocultar_error) {
  char cWork[128];
  int n;
  if (activo) {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c12, i->get.x - 1, i->get.y + 7, i->get.w + 2, 11);
    if (i->state & 2) {
      div_strcpy((char *)get, long_line, (char *)i->get.buffer);
      get_pos = strlen(get);
    }
    i->state &= 1;
  } else {
    if (i->state & 2) {
      if (*get) {
        if (i->get.r0 == i->get.r1)
          div_strcpy((char *)i->get.buffer, i->get.lon_buffer + 1, get);
        else {
          if (atoi(get) >= i->get.r0 && atoi(get) <= i->get.r1)
            itoa(atoi(get), (char *)i->get.buffer, 10);
          else if (!ocultar_error && !show_items_called) {
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
      wrectangle(v.ptr, v.w / big2, v.h / big2, c2, i->get.x - 1, i->get.y + 7, i->get.w + 2,
                 11);
    }

    for (n = 0; n < max_items; n++)
      if (i == &v.item[n])
        v.active_item = n;
  }
}

void show_flag(struct t_item *i) {
  if (*i->flag.valor)
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, -59);
  else
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, 58);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 9, i->flag.y, 0, i->flag.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 8, i->flag.y, 0, i->flag.text, c3);
}

//-----------------------------------------------------------------------------
//      Process items
//-----------------------------------------------------------------------------

void _process_items(void) {
  int n = 0, estado;
  int asc = 0, kesc = 0, est;

  v.active_item = -1;

  if (v.selected_item != -1) {
    if (!v.state && v.type == 102) {
      asc = ascii;
      kesc = kbdFLAGS[28];
      ascii = 0;
      kbdFLAGS[28] = 0;
    } else {
      if (ascii == 9) {
        ascii = 0;
        _select_new_item(v.selected_item + 1);
      }
      if (ascii == 0x1b) { // && (v.item[v.selected_item].state&2)) {
        if (v.item[v.selected_item].type == 2) {
          asc = ascii;
          kesc = kbdFLAGS[28];
          est = v.item[v.selected_item].state;
          ascii = 0;
          if (superget)
            div_strcpy((char *)v.item[v.selected_item].get.buffer,
                       v.item[v.selected_item].get.lon_buffer + 1, "");
          div_strcpy((char *)get, long_line, (char *)v.item[v.selected_item].get.buffer);
          get_pos = strlen(get);
          select_get(&v.item[v.selected_item], 0, 1);
          select_get(&v.item[v.selected_item], 1, 1);
          if (est == v.item[v.selected_item].state) {
            ascii = asc;
            kbdFLAGS[28] = kesc;
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
      estado = button_status(n);
      if (estado != v.item[n].state)
        process_button(n, estado);
      break;
    case 2:
      estado = get_status(n);
      if (estado != v.item[n].state || estado >= 2)
        process_get(n, estado);
      break;
    case 3:
      estado = flag_status(n);
      if (estado != v.item[n].state)
        process_flag(n, estado);
      break;
    }
    n++;
  }

  if (v.selected_item != -1) {
    if (!v.state && v.type == 102) {
      ascii = asc;
      kbdFLAGS[28] = kesc;
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
  } while (v.item[i].type >= 3 || v.item[i].type < 1);

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
  case 0:
    break;
  case 1:
    x = x - (w >> 1);
    break;
  case 2:
    x = x - w + 1;
    break;
  case 3:
    y = y - (h >> 1);
    break;
  case 4:
    x = x - (w >> 1);
    y = y - (h >> 1);
    break;
  case 5:
    x = x - w + 1;
    y = y - (h >> 1);
    break;
  case 6:
    y = y - h + 1;
    break;
  case 7:
    x = x - (w >> 1);
    y = y - h + 1;
    break;
  case 8:
    x = x - w + 1;
    y = y - h + 1;
    break;
  }
  if (wmouse_in(x - 3, y - 3, w + 6, h + 6))
    e = 1;
  if (e && (mouse_b & 1))
    e = 2;
  if (v.selected_item == n && kbdFLAGS[28])
    e = 3;
  return (e);
}

void process_button(int n, int e) {
  if (v.item[n].state == 3 && e != 3) {
    v.active_item = n;
    kbdFLAGS[28] = 0;
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
  if ((x & 1) && (mouse_b & 1)) {
    if (!(prev_mouse_buttons & 1) && (x & 2))
      x |= 4;
    x |= 2;
  }
  if ((ascii && (ascii != 0x1b) && v.selected_item == n)) { //||superget) {
    // MapperCreator2 digit filter removed (MODE8/3D map editor deleted)
    {
      if (!(x & 2)) {
        if (ascii == 13)
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
    div_strcpy((char *)get, long_line, (char *)v.item[n].get.buffer);
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
  if (x && (mouse_b & 1))
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
      if ((*v.item[n].flag.valor = !*v.item[n].flag.valor))
        wput(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x, v.item[n].flag.y, -59);
      else
        wput(v.ptr, v.w / big2, v.h / big2, v.item[n].flag.x, v.item[n].flag.y, 58);
    }
    break;
  }
  v.item[n].state = e;
  v.redraw = 1;
}

//-----------------------------------------------------------------------------
//      Input routine
//-----------------------------------------------------------------------------

int text_len2(byte *ptr);

void get_input(int n) {
  char cwork[long_line];
  int x, l, scroll;

  if (!*get)
    get_pos = 0;

  switch (ascii) {
  case 1:
  case 0x1a:
  case 0x1b:
    break;
  case 8:
    if (get_pos) {
      memmove(&get[get_pos - 1], &get[get_pos], strlen(&get[get_pos]) + 1);
      get_pos--;
    }
    if (!*get && superget)
      div_strcpy((char *)v.item[v.selected_item].get.buffer,
                 v.item[v.selected_item].get.lon_buffer + 1, "");
    v.redraw = 1;
    break;
  case 13:
    ascii = 0;
    kbdFLAGS[28] = 0;
    _select_new_item(n + 1);
    return;
  default:
    if (!(shift_status & 15) && ascii == 0) {
      l = v.redraw;
      v.redraw = 1;
      switch (scan_code) {
      case 77:
        get_pos++;
        break; // cursor right
      case 75:
        get_pos--;
        break; // cursor left
      case 71:
        get_pos = 0;
        break; // home
      case 79:
        get_pos = strlen(get);
        break; // end
      case 83:
        get[strlen(get) + 1] = 0;
        memmove(&get[get_pos], &get[get_pos + 1], strlen(&get[get_pos + 1]) + 1);
        if (!*get && superget)
          div_strcpy((char *)v.item[v.selected_item].get.buffer,
                     v.item[v.selected_item].get.lon_buffer + 1, "");
        break;
      default:
        v.redraw = l;
        break;
      }
    } else if (ascii && char_len(ascii) > 1 && (x = strlen(get)) < v.item[n].get.lon_buffer - 1) {
      // MapperCreator2 digit filter removed (MODE8/3D map editor deleted)
      {
        div_strcpy(cwork, sizeof(cwork), get);
        cwork[get_pos] = ascii;
        cwork[get_pos + 1] = 0;
        div_strcat(cwork, sizeof(cwork), get + get_pos);
        div_strcpy(get, long_line, cwork);
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

    wbox(v.ptr, v.w / big2, v.h / big2, c0, v.item[n].get.x, v.item[n].get.y + 8,
         v.item[n].get.w, 9);
    wwrite_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2,
                  v.h / big2, 0 - scroll, v.item[n].get.y + 9, 0, (byte *)cwork, c4);

    if (*system_clock & 4) {
      x = l + 1;
      wbox_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2,
                  v.h / big2, c3, x - scroll, v.item[n].get.y + 9, 2, 7);
    }
  }
  get_cursor = (*system_clock & 4);
}

//-----------------------------------------------------------------------------
//      Deactivate and activate functions
//-----------------------------------------------------------------------------

void deactivate(void) { // Minimize: deactivate
  int m;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  if (v.state) {
    v.state = 0;
    wgra(v.ptr, w, h, c1, 2, 2, w - 20, 7);
    if (text_len(v.title) + 3 > w - 20) {
      wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c0);
      wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c2);
    } else {
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 3, 1, v.title, c0);
      wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c2);
    }
    for (m = 1; m < max_windows; m++)
      if (window[m].type == v.type && window[m].foreground < 2) {
        window[m].state = 1;
        wgra(window[m].ptr, window[m].w / big2, window[m].h / big2, c_b_low, 2, 2,
             window[m].w / big2 - 20, 7);
        if (text_len(window[m].title) + 3 > window[m].w / big2 - 20) {
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 4, 2, 0, window[m].title, c1);
          wwrite_in_box(window[m].ptr, window[m].w / big2, window[m].w / big2 - 19,
                        window[m].h / big2, 3, 2, 0, window[m].title, c4);
        } else {
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 3 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c1);
          wwrite(window[m].ptr, window[m].w / big2, window[m].h / big2,
                 2 + (window[m].w / big2 - 20) / 2, 2, 1, window[m].title, c4);
        }
        flush_window(m);
        break;
      }
    if (v.type == 102)
      call((voidReturnType)v.paint_handler); // Erase cursor
  }
}

/* Activate the front window: highlight its title bar as focused.
 * NOTE: Called from multiple places, not just maximize — see original div.cpp.
 */
void activate(void) {
  int m;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  v.state = 1;
  wgra(v.ptr, w, h, c_b_low, 2, 2, w - 20, 7);
  if (text_len(v.title) + 3 > w - 20) {
    wwrite_in_box(v.ptr, w, w - 19, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(v.ptr, w, w - 19, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(v.ptr, w, h, 3 + (w - 20) / 2, 2, 1, v.title, c1);
    wwrite(v.ptr, w, h, 2 + (w - 20) / 2, 2, 1, v.title, c4);
  }
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
      if (v.type == 102 && window[m].type == 102) { // Erase cursor
        wup(m);
        call((voidReturnType)v.paint_handler);
        wdown(m);
      }
      flush_window(m);
      break;
    }
}

//-----------------------------------------------------------------------------
//  Load and save setup.bin
//-----------------------------------------------------------------------------

extern int modo_anterior;

////////////////////////////////////////////////////////////////////////////
// Save the configuration file                                            //
////////////////////////////////////////////////////////////////////////////
void Save_Cfgbin() {
  FILE *file;

  // Video mode
  Setupfile.Vid_modeAncho = VS_WIDTH;
  Setupfile.Vid_modeAlto = VS_HEIGHT;
  Setupfile.Vid_modeBig = VS_BIG;
  Setupfile.fullscreen = fsmode;

  if (return_mode == 3) {
    Setupfile.Vid_modeBig = (modo_anterior & 0x8000000) >> 31;
    modo_anterior -= (modo_anterior & 0x80000000);
    Setupfile.Vid_modeAncho = modo_anterior / 10000;
    Setupfile.Vid_modeAlto = modo_anterior % 10000;
  }

  // Undo system
  Setupfile.Max_undo = max_undos;
  Setupfile.Undo_memory = undo_memory;
  Setupfile.tab_size = tab_size;

  // Directory system

  div_strcpy(Setupfile.Dir_cwd, sizeof(Setupfile.Dir_cwd), tipo[0].path);
  div_strcpy(Setupfile.Dir_map, sizeof(Setupfile.Dir_map), tipo[2].path);
  div_strcpy(Setupfile.Dir_pal, sizeof(Setupfile.Dir_pal), tipo[3].path);
  div_strcpy(Setupfile.Dir_fpg, sizeof(Setupfile.Dir_fpg), tipo[4].path);
  div_strcpy(Setupfile.Dir_fnt, sizeof(Setupfile.Dir_fnt), tipo[5].path);
  div_strcpy(Setupfile.Dir_ifs, sizeof(Setupfile.Dir_ifs), tipo[6].path);
  div_strcpy(Setupfile.Dir_pcm, sizeof(Setupfile.Dir_pcm), tipo[7].path);
  div_strcpy(Setupfile.Dir_prg, sizeof(Setupfile.Dir_prg), tipo[8].path);
  div_strcpy(Setupfile.Dir_pcms, sizeof(Setupfile.Dir_pcms), tipo[11].path);
  div_strcpy(Setupfile.Dir_prj, sizeof(Setupfile.Dir_prj), tipo[12].path);
  div_strcpy(Setupfile.Dir_wld, sizeof(Setupfile.Dir_wld), tipo[15].path);
  div_strcpy(Setupfile.Dir_mod, sizeof(Setupfile.Dir_mod), tipo[16].path);
  /*
        // Wallpaper info
        strcpy(Setupfile.Desktop_Image,desk_file);
        Setupfile.Desktop_R=desk_r;
        Setupfile.Desktop_G=desk_g;
        Setupfile.Desktop_B=desk_b;
        Setupfile.Desktop_Tile=desk_tile;
*/
  Setupfile.editor_font = editor_font;
  Setupfile.paint_cursor = paint_cursor;
  Setupfile.exploding_windows = exploding_windows;
  Setupfile.auto_save_session = auto_save_session;
  Setupfile.move_full_window = move_full_window;
  Setupfile.colorizer = colorizer;
  memcpy(&Setupfile.colors_rgb[0], &colors_rgb[0], 12 * 3);

  file = fopen("system/setup.bin", "wb");
  fwrite(&Setupfile, 1, sizeof(Setupfile), file);
  fclose(file);
}

////////////////////////////////////////////////////////////////////////////
// Load config file                                                       //
////////////////////////////////////////////////////////////////////////////

void Load_Cfgbin() {
  int n;
  FILE *file;
  char cWork[_MAX_PATH + 1];

  // System Directories

  div_strcpy(Setupfile.Dir_cwd, sizeof(Setupfile.Dir_cwd), tipo[0].path);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/MAP");
  div_strcpy(Setupfile.Dir_map, sizeof(Setupfile.Dir_map), cWork);
  div_strcpy(tipo[2].path, sizeof(tipo[2].path), cWork);
  div_strcpy(tipo[9].path, sizeof(tipo[9].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/PAL");
  div_strcpy(Setupfile.Dir_pal, sizeof(Setupfile.Dir_pal), cWork);
  div_strcpy(tipo[3].path, sizeof(tipo[3].path), cWork);
  div_strcpy(tipo[10].path, sizeof(tipo[10].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/FPG");
  div_strcpy(Setupfile.Dir_fpg, sizeof(Setupfile.Dir_fpg), cWork);
  div_strcpy(tipo[4].path, sizeof(tipo[4].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/FNT");
  div_strcpy(Setupfile.Dir_fnt, sizeof(Setupfile.Dir_fnt), cWork);
  div_strcpy(tipo[5].path, sizeof(tipo[5].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/IFS");
  div_strcpy(Setupfile.Dir_ifs, sizeof(Setupfile.Dir_ifs), cWork);
  div_strcpy(tipo[6].path, sizeof(tipo[6].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/PCM");
  div_strcpy(Setupfile.Dir_pcm, sizeof(Setupfile.Dir_pcm), cWork);
  div_strcpy(tipo[7].path, sizeof(tipo[7].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/PRG");
  div_strcpy(Setupfile.Dir_prg, sizeof(Setupfile.Dir_prg), cWork);
  div_strcpy(tipo[8].path, sizeof(tipo[8].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/PCM");
  div_strcpy(Setupfile.Dir_pcms, sizeof(Setupfile.Dir_pcms), cWork);
  div_strcpy(tipo[11].path, sizeof(tipo[11].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/PRJ");
  div_strcpy(Setupfile.Dir_prj, sizeof(Setupfile.Dir_prj), cWork);
  div_strcpy(tipo[12].path, sizeof(tipo[12].path), cWork);

  div_strcpy(tipo[13].path, sizeof(tipo[13].path), tipo[1].path);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/WLD");
  div_strcpy(Setupfile.Dir_wld, sizeof(Setupfile.Dir_wld), cWork);
  div_strcpy(tipo[15].path, sizeof(tipo[15].path), cWork);

  div_strcpy(cWork, sizeof(cWork), tipo[1].path);
  div_strcat(cWork, sizeof(cWork), "/MOD");
  div_strcpy(Setupfile.Dir_mod, sizeof(Setupfile.Dir_mod), cWork);
  div_strcpy(tipo[16].path, sizeof(tipo[16].path), cWork);

  file = fopen("system/setup.bin", "rb");
  if (file == NULL) {
    if (primera_vez) {
      div_strcpy(Setupfile.Desktop_Image, sizeof(Setupfile.Desktop_Image),
                 (char *)texts[487]); // Wallpaper info
      Setupfile.Desktop_Gama = 1;
      Setupfile.Desktop_Tile = 0;
      for (n = 0; n < 8; n++)
        Setupfile.gradient_config[n].selec = 0;
      Setupfile.gradient_config[8].selec = 1;
      Setupfile.gradient_config[8].r = 0;
      Setupfile.gradient_config[8].g = 0;
      Setupfile.gradient_config[8].b = 63;
      Setupfile.Vid_modeAncho = 640; // Video Mode
      Setupfile.Vid_modeAlto = 480;
      Setupfile.Vid_modeBig = 1;
      Setupfile.fullscreen = 0;

      Setupfile.Max_undo = 1024; // Undo System
      Setupfile.Undo_memory = 1024 * 1024 + 65536;
      Setupfile.tab_size = 4;
      memcpy(&Setupfile.colors_rgb[0],
             "\x1c\x1c\x1c\x3d\x3d\x3c\x0\x0\x20\x2\x6\x7\x0\x27\x0\x3d\x3d\x3c\x14\x14"
             "\x14\x3d\x3d\x3c\x0\x0\x2e\x34\x31\x31\x22\x13\x13\x26\x26\x0",
             12 * 3);
      Setupfile.editor_font = 2;
      Setupfile.paint_cursor = 0;
      Setupfile.exploding_windows = 1;
      Setupfile.auto_save_session = 1;
      Setupfile.move_full_window = 1;
      Setupfile.colorizer = 1;

      Setupfile.vol_fx = 7;
      Setupfile.vol_cd = 7;
      Setupfile.vol_ma = 7;
      Setupfile.mut_fx = 0;
      Setupfile.mut_cd = 0;
      Setupfile.mut_ma = 0;

      Setupfile.mouse_ratio = 0;

    } else {
      div_strcpy(Setupfile.Desktop_Image, sizeof(Setupfile.Desktop_Image), ""); // Wallpaper info
      Setupfile.Desktop_Gama = 0;
      Setupfile.Desktop_Tile = 0;
      for (n = 0; n < 9; n++)
        Setupfile.gradient_config[n].selec = 0;
      Setupfile.Vid_modeAncho = 320; // Video Mode
      Setupfile.Vid_modeAlto = 200;
      Setupfile.Vid_modeBig = 0;
      Setupfile.fullscreen = 0;

      Setupfile.Max_undo = 1024; // Undo System
      Setupfile.Undo_memory = 1024 * 1024 + 65536;
      Setupfile.tab_size = 4;
      memcpy(&Setupfile.colors_rgb[0],
             "\x1c\x1c\x1c\x3d\x3d\x3c\x0\x0\x20\x2\x6\x7\x0\x27\x0\x3d\x3d\x3c\x14\x14"
             "\x14\x3d\x3d\x3c\x0\x0\x2e\x34\x31\x31\x22\x13\x13\x26\x26\x0",
             12 * 3);
      Setupfile.editor_font = 0;
      Setupfile.paint_cursor = 0;
      Setupfile.exploding_windows = 1;
      Setupfile.auto_save_session = 1;
      Setupfile.move_full_window = 1;
      Setupfile.colorizer = 1;

      Setupfile.vol_fx = 10;
      Setupfile.vol_cd = 10;
      Setupfile.vol_ma = 10;
      Setupfile.mut_fx = 0;
      Setupfile.mut_cd = 0;
      Setupfile.mut_ma = 0;

      Setupfile.mouse_ratio = 0;
    }
  } else {
    fread(&Setupfile, 1, sizeof(Setupfile), file);
    fclose(file);
  }

  editor_font = Setupfile.editor_font;
  paint_cursor = Setupfile.paint_cursor;
  exploding_windows = Setupfile.exploding_windows;
  auto_save_session = Setupfile.auto_save_session;
  move_full_window = Setupfile.move_full_window;
  colorizer = Setupfile.colorizer;
  memcpy(&colors_rgb[0], &Setupfile.colors_rgb[0], 12 * 3);

  VS_WIDTH = vga_width = Setupfile.Vid_modeAncho; // Video mode
  VS_HEIGHT = vga_height = Setupfile.Vid_modeAlto;
  VS_BIG = big = Setupfile.Vid_modeBig;
  fsmode = Setupfile.fullscreen;

  if (test_video && vga_width < 640 && editor_font > 1)
    editor_font = 0;

  big2 = big + 1;

  max_undos = Setupfile.Max_undo; // Undo System
  undo_memory = Setupfile.Undo_memory;
  tab_size = Setupfile.tab_size;

  // System Directories

  if (interpreting)
    if (chdir(Setupfile.Dir_cwd) != -1)
      div_strcpy(tipo[0].path, sizeof(tipo[0].path), Setupfile.Dir_cwd);

  if (chdir(Setupfile.Dir_map) != -1)
    div_strcpy(tipo[2].path, sizeof(tipo[2].path), Setupfile.Dir_map);
  else if (chdir(tipo[2].path) == -1)
    div_strcpy(tipo[2].path, sizeof(tipo[2].path), tipo[1].path);

  if (chdir(Setupfile.Dir_pal) != -1)
    div_strcpy(tipo[3].path, sizeof(tipo[3].path), Setupfile.Dir_pal);
  else if (chdir(tipo[3].path) == -1)
    div_strcpy(tipo[3].path, sizeof(tipo[3].path), tipo[1].path);

  if (chdir(Setupfile.Dir_fpg) != -1)
    div_strcpy(tipo[4].path, sizeof(tipo[4].path), Setupfile.Dir_fpg);
  else if (chdir(tipo[4].path) == -1)
    div_strcpy(tipo[4].path, sizeof(tipo[4].path), tipo[1].path);

  if (chdir(Setupfile.Dir_fnt) != -1)
    div_strcpy(tipo[5].path, sizeof(tipo[5].path), Setupfile.Dir_fnt);
  else if (chdir(tipo[5].path) == -1)
    div_strcpy(tipo[5].path, sizeof(tipo[5].path), tipo[1].path);

  if (chdir(Setupfile.Dir_ifs) != -1)
    div_strcpy(tipo[6].path, sizeof(tipo[6].path), Setupfile.Dir_ifs);
  else if (chdir(tipo[6].path) == -1)
    div_strcpy(tipo[6].path, sizeof(tipo[6].path), tipo[1].path);

  if (chdir(Setupfile.Dir_pcm) != -1)
    div_strcpy(tipo[7].path, sizeof(tipo[7].path), Setupfile.Dir_pcm);
  else if (chdir(tipo[7].path) == -1)
    div_strcpy(tipo[7].path, sizeof(tipo[7].path), tipo[1].path);

  if (chdir(Setupfile.Dir_prg) != -1)
    div_strcpy(tipo[8].path, sizeof(tipo[8].path), Setupfile.Dir_prg);
  else if (chdir(tipo[8].path) == -1)
    div_strcpy(tipo[8].path, sizeof(tipo[8].path), tipo[1].path);

  div_strcpy(tipo[9].path, sizeof(tipo[9].path), tipo[2].path);
  div_strcpy(tipo[10].path, sizeof(tipo[10].path), tipo[3].path);

  if (chdir(Setupfile.Dir_pcms) != -1)
    div_strcpy(tipo[11].path, sizeof(tipo[11].path), Setupfile.Dir_pcms);
  else if (chdir(tipo[11].path) == -1)
    div_strcpy(tipo[11].path, sizeof(tipo[11].path), tipo[1].path);

  if (chdir(Setupfile.Dir_prj) != -1)
    div_strcpy(tipo[12].path, sizeof(tipo[12].path), Setupfile.Dir_prj);
  else if (chdir(tipo[12].path) == -1)
    div_strcpy(tipo[12].path, sizeof(tipo[12].path), tipo[1].path);

  div_strcpy(tipo[13].path, sizeof(tipo[13].path), tipo[1].path); // Generic

  if (chdir(Setupfile.Dir_wld) != -1)
    div_strcpy(tipo[15].path, sizeof(tipo[15].path), Setupfile.Dir_wld);
  else if (chdir(tipo[15].path) == -1)
    div_strcpy(tipo[15].path, sizeof(tipo[15].path), tipo[1].path);

  if (chdir(Setupfile.Dir_mod) != -1)
    div_strcpy(tipo[16].path, sizeof(tipo[16].path), Setupfile.Dir_mod);
  else if (chdir(tipo[16].path) == -1)
    div_strcpy(tipo[16].path, sizeof(tipo[16].path), tipo[1].path);

  chdir(tipo[1].path);
}

//-----------------------------------------------------------------------------
//  Check for mouse driver
//  Not needed for SDL port
//-----------------------------------------------------------------------------

void check_mouse(void) {}


//-----------------------------------------------------------------------------
//  Determines a prg for help with F1
//-----------------------------------------------------------------------------

int determine_prg2(void) {
  int m, n = -1;

  for (m = 0; m < max_windows; m++) {
    if (window[m].type == 102 && window[m].state && window[m].prg != NULL) {
      n = m;
      break;
    }
  }
  return (v_window = n);
}

int determine_calc(void) {
  int m, n = -1;

  for (m = 0; m < max_windows; m++) {
    if (window[m].click_handler == calc2 && window[m].state) {
      n = m;
      break;
    }
  }
  return (v_window = n);
}

///////////////////////////////////////////////////////////////////////////////
//  Window movement
///////////////////////////////////////////////////////////////////////////////

struct twindow vaux[4];
int ivaux = 0;

void copy(int a, int b) {
  if (a == b)
    return;

  if (a == -1) {
    memcpy(&vaux[ivaux].type, &window[b].type, sizeof(struct twindow));
  } else if (b == -1) {
    memcpy(&window[a].type, &vaux[ivaux].type, sizeof(struct twindow));
  } else {
    memcpy(&window[a].type, &window[b].type, sizeof(struct twindow));
  }
}

void xchg(int a, int b) {
  copy(-1, a);
  copy(a, b);
  copy(b, -1);
}

void move(int a, int b) {
  if (a < b) {
    copy(-1, b);
    memmove(&window[a + 1].type, &window[a].type, sizeof(struct twindow) * (b - a));
    copy(a, -1);
  } else if (b < a) {
    copy(-1, b);
    memmove(&window[b].type, &window[b + 1].type, sizeof(struct twindow) * (a - b));
    copy(a, -1);
  }
}

void divdelete(int a) {
  memmove(&window[a].type, &window[a + 1].type, sizeof(struct twindow) * (max_windows - 1 - a));
  window[max_windows - 1].type = 0;
}

void addwindow(void) {
  memmove(&window[1].type, &v.type, sizeof(struct twindow) * (max_windows - 1));
}

void wup(int a) {
  if (++ivaux == 4)
    ivaux = 0;
  copy(-1, 0);
  copy(0, a);
  window[a].type = 0;
}

void wdown(int a) {
  copy(a, 0);
  copy(0, -1);
  if (--ivaux == -1)
    ivaux = 3;
}

void DaniDel(char *name) {
#ifdef WIN32
  debugprintf("DaniDel %s\n", name);
  remove(name);
  return;
#endif

  unsigned rc;
  struct find_t ft;
  int x;
  char cwork1[_MAX_PATH + 1];
  char cwork2[_MAX_PATH + 1];
  char cwork3[_MAX_PATH + 1];

  div_strcpy(cwork2, sizeof(cwork2), name);
  for (x = strlen(cwork2) - 1;; x--) {
    if (x == -1) {
      cwork2[0] = 0;
      break;
    }
    if (IS_PATH_SEP(cwork2[x])) {
      cwork2[x + 1] = 0;
      break;
    }
  }

  rc = _dos_findfirst(name, _A_NORMAL, &ft);
  while (!rc) {
    div_strcpy(cwork3, sizeof(cwork3), cwork2);
    div_strcat(cwork3, sizeof(cwork3), ft.name);
    if (_fullpath(cwork1, cwork3, _MAX_PATH) == NULL)
      div_strcpy(cwork1, sizeof(cwork1), ft.name);
    _dos_setfileattr(cwork1, _A_NORMAL);
    remove(cwork1);
    rc = _dos_findnext(&ft);
  }
}

//////////////////////////////////////////////////////////////////////////////
//  Dumps information in a file
//////////////////////////////////////////////////////////////////////////////

void DebugInfo(char *Msg) {}

void DebugData(int Val) {}

void GetFree4kBlocks(void) {}

//-----------------------------------------------------------------------------
//  Check_oldpif - If it finds an invalid DIV 1 PIF -> delete it
//  (removes the start menu shortcut to DIV Games Studio 1)
//-----------------------------------------------------------------------------

void check_oldpif(void) {
  unsigned n;
  byte pif[1024];
  FILE *f;
  char winpath[128];
  char cwork[256], *name;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  if ((f = fopen("SYSTEM/PATHINFO.INI", "rb")) == NULL)
    return;
  fread(winpath, 1, 128, f);
  fclose(f);

  snprintf(cwork, sizeof(cwork), "%s/DIVGAM~1.PIF", winpath);

  if ((f = fopen(cwork, "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (n <= 1024) {
      memset(pif, 0, 1024);
      fread(pif, 1, n, f);
      fclose(f);

      name = (char *)&pif[0x273]; // Icon path

      _splitpath(name, drive, dir, fname, ext);
      strupr(fname);
      strupr(ext);

      if (!strcmp(fname, "DIV") && !strcmp(ext, ".ICO")) { // It's the DIV 1 PIF

        name = (char *)&pif[0x24];

        if ((f = fopen(name, "rb")) != NULL) {
          fseek(f, 0, SEEK_END);
          n = ftell(f);
          fclose(f);

          if (n > 1024 * 1024)
            remove(cwork); // If it's not the DIV 1 EXE, delete the PIF

        } else
          remove(cwork); // If the EXE doesn't exist, also delete the PIF
      }

    } else
      fclose(f);
  }
}
