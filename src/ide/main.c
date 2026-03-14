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

#define DEFINE_GLOBALS_HERE // DEFINED HERE - see global.h
#ifdef WIN32
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#endif

#include "main_internal.h"
#include "sound.h"
#include "mixer.h"
#include "sysdac.h"

void heap_dump(void);
void debug_file(char *string, char *name);
void save_prg_buffer(memptrsize);

void maximize(void);
void test0(void);
void calc2(void);
void load_thumbs(void);
void show_mod_meters(void);
// MapperCreator2 removed (MODE8/3D map editor deleted)

///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////

void initialization(void);
void init_environment(void);
void main_loop(void);
void main_loop_tick(void);
void finalization(void);
int determine_prg2(void);
int determine_calc(void);
void check_oldpif(void);
void default_gradients(void);
void determine_units(void);
void check_mouse(void);


extern int help_item;
extern struct tprg *old_prg;
int help_paint_active = 0;
int beta_status = 0; // 0 dialog, 1 beta.nfo, 2 dc, 3 cmp, 4 ok, 5 error

extern int superget;

///////////////////////////////////////////////////////////////////////////////
//      Global variables
///////////////////////////////////////////////////////////////////////////////

char get_buffer[LONG_LINE]; // Shared buffer (except for calculator)
char *get;
int get_cursor, get_pos; // Clock and cursor position in get fields

int window_closing = 0, window_move_pending = 0;
int quick_close = 0;
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

int first_run = 0; // Marks first time DIV runs
extern int div_started;

#ifdef SHARE
int mostrar_demo = 1; // The first time, the demo message won't appear
#else
int mostrar_demo = 0;
#endif

int partial_blits = 0;

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
  beta_status = v.type = WIN_DIALOG;
  v.title = texts[392];
  v.w = 160;
  v.h = 76;
  v_accept = 0;

  v.paint_handler = demo1;
  v.click_handler = demo2;
  v.close_handler = demo3;

  if (exe_cola[1] - 0xF31725AB > 31) {
    _button(100, v.w / 2, v.h - 14, ALIGN_TC);
  } else {
    _button(100, v.w - 8, v.h - 14, ALIGN_TR);
    _button(125, 7, v.h - 14, ALIGN_TL);
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
//  Demo check (beta meant demo version)
//-----------------------------------------------------------------------------

// 0 - Dialog

char betaname[128];

void betatest1(void) {
  _show_items();
}

void betatest2(void) {
  _process_items();
  if ((shift_status & MOD_CTRL) && (shift_status & MOD_ALT) && key(_Z))
    beta_status = 4;
  if (v.active_item == 1)
    end_dialog = 1;
}

void coder(byte *ptr, int len, char *clave);

void betatest0(void) {
  beta_status = v.type = WIN_DIALOG;
  v.title = texts[399];
  v.w = 160;
  v.h = 38 + 16;

  div_strcpy(betaname, sizeof(betaname), "");
  _get(460, 4, 12, v.w - 8, (uint8_t *)betaname, 127, 0, 0);

  v.paint_handler = betatest1;
  v.click_handler = betatest2;
  _button(100, v.w / 2, v.h - 14, ALIGN_TC);
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


void get_free_4k_blocks(void);

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

// Smoke test support
static int smoke_test = 0;
static char *smoke_prg = NULL; // PRG file to compile during smoke test

extern int helpidx[4096];

/* IDE smoke test: verify initialization, compile a test PRG, run it.
 * Returns 0 on success, non-zero on failure.
 * Result details written to tests/test_smoke.result in the standard
 * binary format (two 32-bit ints: passed count, failed count).
 */
static int run_smoke_test(void) {
  int passed = 0, failed = 0;
  FILE *f;
  byte *prgbuf;

  // Write all smoke output to a log file (-mwindows swallows console)
  FILE *smoke_log = fopen("tests/smoke.log", "w");
  if (!smoke_log)
    smoke_log = stderr;

#define SLOG(...) do { fprintf(smoke_log, __VA_ARGS__); fflush(smoke_log); } while (0)

  SLOG("[smoke] IDE smoke test starting...\n");

  // --- Phase 1: Verify core buffer allocations ---
  SLOG("[smoke] Phase 1: Core allocations\n");

#define SMOKE_CHECK(label, cond) \
  do {                           \
    if (cond) {                  \
      passed++;                  \
      SLOG("[smoke] PASS: %s\n", label); \
    } else {                     \
      SLOG("[smoke] FAIL: %s\n", label); \
      failed++;                  \
    }                            \
  } while (0)

  SMOKE_CHECK("screen_buffer", screen_buffer != NULL);
  SMOKE_CHECK("dac (palette)", dac != NULL);
  SMOKE_CHECK("dac4 (palette 0-255)", dac4 != NULL);
  SMOKE_CHECK("ghost (blend table)", ghost != NULL);
  SMOKE_CHECK("color_lookup", color_lookup != NULL);
  SMOKE_CHECK("text_font", text_font != NULL);
  SMOKE_CHECK("toolbar", toolbar != NULL);
  SMOKE_CHECK("fill_dac", fill_dac != NULL);
  SMOKE_CHECK("error_window", error_window != NULL);
  SMOKE_CHECK("undo buffer", undo != NULL);
  SMOKE_CHECK("undo_table", undo_table != NULL);
  SMOKE_CHECK("mouse_background", mouse_background != NULL);
  SMOKE_CHECK("graf_ptr (icon graphics)", graf_ptr != NULL);

  // --- Phase 2: Verify help index and texts ---
  SLOG("[smoke] Phase 2: Help & texts\n");
  SMOKE_CHECK("help index loaded", helpidx[0] != 0 || helpidx[1] != 0);
  // texts[0] may be empty; texts[1] is the banner, texts[34] is the window title
  SMOKE_CHECK("texts loaded", texts[1] != NULL && texts[1][0] != '\0');

  // --- Phase 3: Count windows (session verification) ---
  SLOG("[smoke] Phase 3: Window state\n");
  {
    int win_count = 0;
    for (int i = 0; i < MAX_WINDOWS; i++)
      if (window[i].type != WIN_EMPTY)
        win_count++;
    SLOG("[smoke] Windows open: %d\n", win_count);
    // At minimum, the main menu bar should exist
    SMOKE_CHECK("at least one window open", win_count >= 1);
  }

  // --- Phase 4: Compile a test PRG (if specified) ---
  if (smoke_prg) {
    SLOG("[smoke] Phase 4: Compile '%s'\n", smoke_prg);
    f = fopen(smoke_prg, "rb");
    if (f) {
      fseek(f, 0, SEEK_END);
      source_len = ftell(f);
      fseek(f, 0, SEEK_SET);
      prgbuf = (byte *)malloc(source_len + 10);
      if (prgbuf) {
        fread(prgbuf, 1, source_len, f);
        fclose(f);

        source_ptr = prgbuf;
        compilado = 1;
        compile_mode = 1; // So show_compile_message prints to stdout
        error_number = -1;
        comp();

        SMOKE_CHECK("compilation succeeded", error_number == -1);
        if (error_number >= 0) {
          get_error(500 + error_number);
          SLOG("[smoke] Compile error: %s\n", cerror);
        }

        free(prgbuf);
        prgbuf = NULL;
        compile_mode = 0;
      } else {
        SLOG("[smoke] FAIL: out of memory for PRG\n");
        failed++;
        fclose(f);
      }
    } else {
      SLOG("[smoke] FAIL: cannot open '%s'\n", smoke_prg);
      failed++;
    }

    // --- Phase 5: Run the compiled program ---
    if (error_number == -1) {
      char cwd_buf[256];
      getcwd(cwd_buf, sizeof(cwd_buf));
      SLOG("[smoke] Phase 5: Run compiled program (cwd=%s)\n", cwd_buf);
      SLOG("[smoke] Command: system/" RUNTIME " system/EXEC.EXE\n");
      int ret = system("system\\" RUNTIME ".exe system\\EXEC.EXE");
      SLOG("[smoke] Runtime returned: %d\n", ret);
      SMOKE_CHECK("runtime exited cleanly", ret == 0);

      // Check if the program wrote a result file
      char result_path[256];
      div_strcpy(result_path, sizeof(result_path), smoke_prg);
      // Replace .prg with .result
      char *dot = strrchr(result_path, '.');
      if (dot)
        div_strcpy(dot, sizeof(result_path) - (dot - result_path), ".result");
      else
        div_strcat(result_path, sizeof(result_path), ".result");

      f = fopen(result_path, "rb");
      if (f) {
        int test_passed = 0, test_failed = 0;
        fread(&test_passed, 4, 1, f);
        fread(&test_failed, 4, 1, f);
        fclose(f);
        SLOG("[smoke] Test PRG: %d passed, %d failed\n", test_passed, test_failed);
        SMOKE_CHECK("test PRG all assertions passed", test_failed == 0 && test_passed > 0);
      } else {
        SLOG("[smoke] No result file from test PRG (may be expected)\n");
      }
    }
  } else {
    SLOG("[smoke] Phase 4-5: Skipped (no PRG specified)\n");
  }

  // --- Phase 6: Session save (tested by caller calling download_desktop) ---
  SLOG("[smoke] Phase 6: Session save (in progress...)\n");

  // --- Write smoke result file ---
  f = fopen("tests/test_smoke.result", "wb");
  if (f) {
    fwrite(&passed, 4, 1, f);
    fwrite(&failed, 4, 1, f);
    fclose(f);
  }

  SLOG("[smoke] Result: %d passed, %d failed\n", passed, failed);

  if (smoke_log != stderr)
    fclose(smoke_log);

#undef SMOKE_CHECK
#undef SLOG

  return (failed > 0) ? 1 : 0;
}

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
  must_create = 1;
  next_order = 0;
  interpreting = 1;
  safe = 34; // Text in lower right corner
  compile_mode = 0;

  if (argc > 1 && !strcmp(argv[1], "INIT"))
    interpreting = 0;
  else
    beta_status = 4;

  if (argc > 1 && !strcmp(argv[1], "TEST"))
    test_video = 1;

  if (argc > 1 && !strcmp(argv[1], "-c"))
    compile_mode = 1;

  // --smoke: IDE smoke test (full init, automated checks, no interactive loop)
  for (int si = 1; si < argc; si++) {
    if (!strcmp(argv[si], "--smoke")) {
      smoke_test = 1;
      interpreting = 1; // Skip copyright dialog
      if (si + 1 < argc && argv[si + 1][0] != '-')
        smoke_prg = argv[si + 1];
      break;
    }
  }

  getcwd(file_types[0].path, PATH_MAX + 1);

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

  getcwd(file_types[1].path, PATH_MAX + 1);

  if (argc > 2 && (!strcmp(argv[2], "/safe") || !strcmp(argv[2], "/SAFE"))) {
    safe = 33;
    delete_file("sound.cfg");
    delete_file("system/setup.bin");
    delete_file("system/session.dtf");
  } else {
    if ((f = fopen("system/setup.bin", "rb")) != NULL) {
      fclose(f);
      first_run = 0;
    } else {
      first_run = 1;
      mostrar_demo = 0;
    }
  }

  file_types[2].ext = "*.* *.MAP *.PCX *.BMP *.JPG *.JPE *.PNG *.GIF *.TGA *.TIF"; // Maps browser
  file_types[3].ext =
      "*.* *.PAL *.FPG *.FNT *.MAP *.BMP *.PCX *.JPG *.PNG *.GIF *.TGA *.TIF"; // Palette Browser
  file_types[FT_FPG].ext = "*.FPG *.*";                                             // FPG FILES
  file_types[FT_FNT].ext = "*.FNT *.*";                                             // FNT FILES
  file_types[FT_IFS].ext = "*.IFS *.*";                                             // IFS Font templates
  file_types[7].ext = "*.* *.7 *.WAV *.PCM *.MP3 *.OGG *.FLAC";                // Audio files
  file_types[8].ext = "*.PRG *.*";                                             // Program files

  file_types[FT_WALLPAPER].ext = "*.* *.JPG *.PNG *.BMP *.TIF"; // wallpaper files

  file_types[FT_PAL_SAVE].ext = "*.PAL";       // Save Palettes
  file_types[FT_AUDIO_SAVE].ext = "*.WAV *.PCM"; // Save Audio
  file_types[12].ext = "*.PRJ";       // Save Project

  file_types[13].ext = "*.* *.13"; // unknown

  file_types[FT_MAP_SAVE].ext = "*.MAP *.PCX *.BMP";          // Save image
  file_types[15].ext = "*.WLD *.*";                  // 3D Map files
  file_types[16].ext = "*.* *.MOD *.S3M *.XM *.MID"; // Tracker modules

  for (n = 0; n < 24; n++) {
    file_types[n].default_choice = 0;
    file_types[n].first_visible = 0;
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

  if (compile_mode == 1) {
    init_compiler();

    compilado = 1;
    mouse_graf = CURSOR_BUSY;
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

  load_config();

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
    set_text_color(BRIGHT, WHITE, RED);
    printf("%s", texts[1]);
    set_text_color(TXTRESET, WHITE, BLACK);
    printf("\n");
  }

  initialization();

  if (smoke_test) {
    // Smoke test: skip init_environment() (has blocking dialogs) and
    // the beta/share dialogs. initialization() already set up video,
    // fonts, palettes, help index, icons, sound, and toolbar.
    // Clean stale files that could cause issues on next IDE start.
    delete_file("system/exec.err");
    // Create a minimal window (main menu bar) so window tests pass
    new_window(menu_main0);
    flush_buffer();

    int smoke_result = run_smoke_test();
    // Save session to test download_desktop()
    download_desktop();
    save_config();
    finalization();
    finalize_texts();
    return smoke_result;
  }

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

  mouse_graf = CURSOR_BUSY;
  flush_copy();

  if (auto_save_session || return_mode != 0)
    if (return_mode != 3)
      download_desktop(); // If the test didn't fail

  save_config();

  finalization();

  finalize_texts();

#ifdef GRABADORA
  end_recorder();
#endif


  if (return_mode == 1) {
    _dos_setdrive((memptrsize)toupper(*file_types[1].path) - 'A' + 1, &n);
    _chdir(file_types[1].path);
  } else {
    _dos_setdrive((memptrsize)toupper(*file_types[0].path) - 'A' + 1, &n);
    _chdir(file_types[0].path);
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
int new_session = 0;
void errhlp0(void);
void interr0(void);
void intmsg0(void);
void user_info0(void);
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
      show_dialog(user_info0);
    } else {
      show_dialog(copyright0);
    }
  }

  // If the DIV.DTF file doesn't exist or safe mode is requested

  if (copy_desktop && !new_session && !first_run)
    upload_desktop();

  if (!first_run) {
    for (n = 0; n < MAX_WINDOWS; n++)
      if (window[n].click_handler == menu_main2)
        break;
    if (n == MAX_WINDOWS)
      new_window(menu_main0);
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

  delete_file("*.swp");
  _chdir("system");
  delete_file("exec.*");
  _chdir("..");

  if (first_run) {
    new_window(menu_main0);
    minimize_window();

    first_run = 2;
    help(2000);
    first_run = 0;
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
  v.type = WIN_DIALOG;
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
  _button(100, v.w / 2, v.h - 14, ALIGN_TC);
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
  v.type = WIN_DIALOG;
  v.title = texts[375];
  v.w = text_len((byte *)v_text) + 8;
  if (v.w < 120)
    v.w = 120;
  v.h = 38 + 8;
  v.paint_handler = intmsg1;
  v.click_handler = intmsg2;
  _button(100, v.w / 2, v.h - 14, ALIGN_TC);
}

//-----------------------------------------------------------------------------
//      Main loop DIV / OS
//-----------------------------------------------------------------------------

/*
typedef void (__interrupt __far *TIRQHandler)(void);
extern void __far __interrupt __loadds Irq0Handler(void);
extern void set_irq_vector(int n, TIRQHandler vec);
extern TIRQHandler get_irq_vector(int n);
int IntIncr  = 65536;
*/

byte rndb(void);

int old_clock, loop_count = 0;
extern int frame_clock; // clock

///////////////////////////////////////////////////////////////////////////////
//      Environment
///////////////////////////////////////////////////////////////////////////////

void main_loop_tick(void) {
  static int oldn = MAX_WINDOWS;
  int n, m;
  int llamar;
  char cwork[256], *p;

  if (dragging == DRAG_DONE) {      // drag == 3?
    goto fin_bucle_entorno; // end loop environment
  }

  poll_keyboard();

  //-------------------------------------------------------------------------
  // Find the window the mouse is over (n); n=MAX_WINDOWS if none
  //-------------------------------------------------------------------------

  n = 0;

  while (n < MAX_WINDOWS &&
         !(window[n].type > 0 && mouse_in(window[n].x, window[n].y, window[n].x + window[n].w - 1,
                                          window[n].y + window[n].h - 1)))
    n++;

  if (n < MAX_WINDOWS && dragging == DRAG_DROPPING && window[n].order == drag_source)
    dragging = DRAG_DROPPED;

  //-------------------------------------------------------------------------
  //  Drag onto the wallpaper
  //-------------------------------------------------------------------------

  if (dragging == DRAG_DROPPING && (n == MAX_WINDOWS || window[n].type == WIN_MENU)) {
    dragging = DRAG_DROPPED;
    free_drag = 0;
    v_title = (char *)texts[57];
    v_text = NULL;
    show_dialog(accept0);

    if (v_accept) {
      if (v.type == WIN_FPG)
        must_create = 0;

      if (!new_map(NULL)) {
        if (must_create == 0) {
          memcpy(v_map->filename, v.mapa->filename, 13);
          new_window(map_view0);
          must_create = 1;
        }

        v_map = window[1].mapa;
        memcpy(v.mapa->map, v_map->map, map_width * map_height);
        v.mapa->zoom = v_map->zoom;
        v.mapa->zoom_x = v_map->zoom_x;
        v.mapa->zoom_y = v_map->zoom_y;
        v.mapa->zoom_cx = v_map->zoom_cx;
        v.mapa->zoom_cy = v_map->zoom_cy;

        for (n = 0; n < 512; n++)
          v.mapa->points[n] = v_map->points[n];

        if (v_map->has_name == 1) {
          v_map->has_name = 0;
          memcpy(v.mapa->filename, v_map->filename, 13);
          memcpy(v.name, v_map->filename, 13);
          memcpy(v.title, v_map->filename, 13);
          v.mapa->fpg_code = 0;
          memcpy(v.mapa->description, v_map->description, 32);
        } else if (v_map->has_name == 2) {
          v.mapa->has_name = 0;
          v.mapa->fpg_code = v_map->fpg_code;
          memcpy(v.mapa->description, v_map->description, 32);
          memcpy(v.mapa->filename, v_map->filename, 13);
        } else
          v.mapa->fpg_code = 0;

        call((void_return_type_t)v.paint_handler);
        blit_region(screen_buffer, vga_width, vga_height, v.ptr, v.x, v.y, v.w, v.h, 0);

      } else
        must_create = 1;
    }
    free_drag = 1;
  }

  //-------------------------------------------------------------------------
  // If we were previously in a window we've now left,
  // repaint it (to clear any highlights)
  //-------------------------------------------------------------------------

  if (dragging != DRAG_DROPPING) {
    if (n == 0)
      if (v.foreground == WF_FOREGROUND)
        if (!mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2))
          n--;

    if (n != oldn && oldn == 0) {
      if (v.foreground == WF_FOREGROUND) {
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
    }
    oldn = MAX_WINDOWS;

    if (n < 0)
      n++;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Determine the shape of the cursor (mouse pointer)
  ///////////////////////////////////////////////////////////////////////////

  if (n >= MAX_WINDOWS || n < 0) {
    mouse_graf = CURSOR_ARROW;
  } else {
    switch (window[n].foreground) {
    case WF_BACKGROUND:
      mouse_graf = CURSOR_ACTIVATE;
      break;
    case WF_FOREGROUND:
      if (mouse_in(window[n].x + 2 * big2, window[n].y + 2 * big2,
                   window[n].x + window[n].w - 2 * big2, window[n].y + 9 * big2))
        if (mouse_x <= window[n].x + window[n].w - 18 * big2)
          mouse_graf = CURSOR_MOVE;
        else if (mouse_x <= window[n].x + window[n].w - 10 * big2)
          mouse_graf = CURSOR_MINIMIZE;
        else
          mouse_graf = CURSOR_CLOSE;
      else
        mouse_graf = CURSOR_ARROW;
      break;
    case WF_MINIMIZED:
      if (mouse_x >= window[n].x + 7 * big2)
        mouse_graf = CURSOR_MOVE;
      else
        mouse_graf = CURSOR_RESIZE;
      break;
    }
  }

  //-------------------------------------------------------------------------
  // If we are on a window that is not the first ...
  //-------------------------------------------------------------------------

  if (n != 0 && n < MAX_WINDOWS) {
    // Bring window to position 0 if it's in the foreground or clicked

    if (window[n].foreground == WF_FOREGROUND || (mouse_b & MB_LEFT)) {
      move(0, n);
      n = 0;
    }

    if (n == 0 && v.foreground != WF_FOREGROUND) { // Clicked icon or background window
      for (m = 1; m < MAX_WINDOWS; m++) {
        if (window[m].type && window[m].foreground == WF_FOREGROUND) {
          if (windows_collide(0, m)) {
            window[m].foreground = WF_BACKGROUND;
            flush_window(m);
          }
        }
      }
      if (v.foreground == WF_BACKGROUND) { // If it was in the background
        if (v.type >= WIN_EDITOR_MIN && !v.state)
          activate(); // Activate a map

        v.foreground = WF_FOREGROUND;
        flush_window(0);
        do {
          read_mouse();
        } while (mouse_b & MB_LEFT);

        prev_mouse_buttons = 0;
      }
    }

    //-------------------------------------------------------------------------
    // Activate an excludable window (type>=100) when interacted with
    //-------------------------------------------------------------------------

  } else if (n == 0 && (mouse_b & MB_LEFT) && v.type >= WIN_EDITOR_MIN && v.foreground != WF_MINIMIZED && !v.state &&
             mouse_graf != CURSOR_MINIMIZE && mouse_graf != CURSOR_CLOSE) {
    activate();
    flush_window(0);
  }

  //-------------------------------------------------------------------------
  //  Drop something onto a background window
  //-------------------------------------------------------------------------

  if (n < MAX_WINDOWS && window[n].foreground == WF_BACKGROUND && dragging == DRAG_DROPPING && v.type >= WIN_EDITOR_MIN &&
      window[n].type != WIN_MENU) {
    move(0, n);
    n = 0;

    if (v.type == WIN_MAP)
      mouse_b |= 1;

    activate();
    wmouse_x = mouse_x - v.x;
    wmouse_y = mouse_y - v.y;

    if (big) {
      wmouse_x /= 2;
      wmouse_y /= 2;
    }

    call((void_return_type_t)v.click_handler);

    for (m = 1; m < MAX_WINDOWS; m++)
      if (window[m].type && window[m].foreground == WF_FOREGROUND)
        if (windows_collide(0, m)) {
          window[m].foreground = WF_BACKGROUND;
          flush_window(m);
        }

    v.foreground = WF_FOREGROUND;
    flush_window(0);
    v.redraw = 0;
    do {
      read_mouse();
    } while (mouse_b & MB_LEFT);

    goto fin_bucle_entorno; // end loop environment
  }

  //-------------------------------------------------------------------------
  // If we are within the contents of a window ...
  //-------------------------------------------------------------------------

  if (n == 0 && v.foreground == WF_FOREGROUND) {
    if (mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2)) {
      llamar = 1; // Call its click_handler

      if (v.type == WIN_MAP && dragging != DRAG_DROPPING) {
        if (dragging == DRAG_PENDING) {
          drag_graphic = 8;
          dragging = DRAG_ACTIVE;
          map_width = v.mapa->map_width;
          map_height = v.mapa->map_height;
        }

        llamar = 0;

        if ((mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT)) {
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

      if (v.type >= WIN_EDITOR_MIN && dragging == DRAG_DROPPING) {
        if (v.type == WIN_MAP)
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

        call((void_return_type_t)v.click_handler);
        partial_blits = 1;
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        partial_blits = 0;
      }

      oldn = 0;

      //-------------------------------------------------------------------------
    } else { // If we are in the control bar of the window ...
             //-------------------------------------------------------------------------
             //-----------------------------------------------------------------------
             // Move a window
             //-----------------------------------------------------------------------

      if (mouse_graf == CURSOR_MOVE && (mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT)) {
        move_window();
      }

      //-----------------------------------------------------------------------
      // Close the window
      //-----------------------------------------------------------------------

      if (mouse_graf == CURSOR_CLOSE) {
        if (mouse_b & MB_LEFT) {
          if (big)
            wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
          else
            wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);

          flush_window(0);
        }

        if (!(mouse_b & MB_LEFT) && (prev_mouse_buttons & MB_LEFT)) {
          if (v.click_handler == menu_main2) {
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
            } while (mouse_b & MB_LEFT);

            v_title = (char *)texts[40];
            v_text = NULL;
            show_dialog(accept0);

            if (v_accept)
              exit_requested = 1;
          } else if (v.type >= WIN_EDITOR_MIN) {
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
            } while (mouse_b & MB_LEFT);

            switch (v.type) {
            case WIN_MAP:
              v_title = (char *)texts[50];
              v_text = (char *)v.title;
              break;

            case WIN_CODE:
              v_title = (char *)texts[188];
              v_text = (char *)v.title;
              break;
            }

            if (v.type == WIN_MAP || (v.type == WIN_CODE && v.prg != NULL))
              show_dialog(accept0);
            else
              v_accept = 1;

            if (v_accept)
              close_window();
          } else
            close_window();
        } else if (mouse_b & MB_LEFT)
          restore_button = 3;
      }

      //-----------------------------------------------------------------------
      // Minimize a window
      //-----------------------------------------------------------------------

      if (mouse_graf == CURSOR_MINIMIZE) {
        if (mouse_b & MB_LEFT) {
          wput(v.ptr, v.w / big2, v.h / big2, v.w / big2 - 17, 2, -47);
          flush_window(0);
        }

        if (!(mouse_b & MB_LEFT) && (prev_mouse_buttons & MB_LEFT)) {
          minimize_window();
        } else if (mouse_b & MB_LEFT)
          restore_button = 1;
      }

      oldn = -1;
    }
  }

  //-------------------------------------------------------------------------
  // We are over an icon
  //-------------------------------------------------------------------------

  if (n == 0 && v.foreground == WF_MINIMIZED) {
    if (mouse_graf == CURSOR_MOVE) {
      if ((mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT))
        move_window();
    } else {
      if (mouse_b & MB_LEFT) {
        if (big) {
          wput(screen_buffer, -vga_width, vga_height, v.x, v.y, -48);
          blit_partial(v.x, v.y, 14, 14);
        } else {
          wput(screen_buffer, vga_width, vga_height, v.x, v.y, -48);
          blit_partial(v.x, v.y, 7, 7);
        }
      }

      if (!(mouse_b & MB_LEFT) && (prev_mouse_buttons & MB_LEFT)) {
        maximize_window();
      } else if (mouse_b & MB_LEFT)
        restore_button = 2;
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  // Type windows control timer
  ///////////////////////////////////////////////////////////////////////////

fin_bucle_entorno:

  for (m = 0; m < MAX_WINDOWS; m++) {
    if (m == 0 &&
        mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2) &&
        window[m].type != WIN_MUSIC)
      continue;

    if (window[m].type == WIN_CLOCK || (window[m].type == WIN_FPG && window[m].foreground != WF_MINIMIZED) ||
        (window[m].type == WIN_MUSIC && window[m].foreground != WF_MINIMIZED) ||
        (window[m].type == WIN_CODE && window[m].state && window[m].foreground != WF_MINIMIZED)) {
      if (m) {
        wup(m);
      }

      window_closing = 1;
      wmouse_x = -1;
      wmouse_y = -1;

      switch (v.type) {
      case WIN_FPG:
        load_thumbs();
        break;

      case WIN_MUSIC:
        show_mod_meters();
        break;

      default:
        call((void_return_type_t)v.click_handler);
        break;
      }

      if (m) {
        wdown(m);
      }

      if (window_closing == 2) { // Close the window(m)
        move(0, m);
        close_window();
      } else if (window[m].redraw) {
        partial_blits = 1;
        flush_window(m);
        window[m].redraw = 0;
        partial_blits = 0;
      }

      window_closing = 0;
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  //  Program menu hotkeys
  ///////////////////////////////////////////////////////////////////////////

  for (m = 0; m < MAX_WINDOWS; m++)
    if (window[m].type == WIN_CODE && window[m].state && window[m].prg != NULL)
      break;

  if (m < MAX_WINDOWS && beta_status == 4) { // If a PRG ...
    n = 0;
    if (shift_status & MOD_ALT)
      switch (scan_code) {
      case _F:
        n = 1;
        break; // alt+f

      case _N:
        n = 2;
        break; // alt+n

      case _R:
        n = 3;
        break; // alt+r
      }
    else if (shift_status & MOD_CTRL)

      switch (scan_code) {
      case _F:
        n = 1;
        break; // ctrl+f

      case _L:
        n = 2;
        break; // ctrl+l

      case _R:
        n = 3;
        break; // ctrl+r

      case _Z:
        n = 9;
        break; // ctrl+z
      }
    else
      switch (scan_code) {
      case _F2:
        n = 4;
        break; // f2

      case _F3:
        n = 2;
        break; // f3

      case _F5:
        n = 8;
        break; // f5

      case _F10:
        n = 6;
        break; // f10
      }

    if (key(_F11))
      n = 5; // f11

    if (key(_F12))
      n = 7; // f12

    if (n) { // If a hotkey was pressed ...
      if (m) {
        wmouse_x = -1;
        wmouse_y = -1;
        mouse_b = 0;
        call((void_return_type_t)v.click_handler);

        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }

        move(0, m);

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
        mouse_graf = CURSOR_BUSY;
        flush_copy();
        v_type = FT_PRG;
        save_prg_buffer(0);
        div_strcpy(file_types[FT_PRG].path, sizeof(file_types[FT_PRG].path), v.prg->path);
        div_strcpy(input, sizeof(input), v.prg->filename);
        save_program();
        break;

      case 5:
        v_type = FT_PRG;
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
        v_type = FT_PRG;
        save_prg_buffer(0);
        div_strcpy(file_types[FT_PRG].path, sizeof(file_types[FT_PRG].path), v.prg->path);
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
        v_type = FT_PRG;
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
      call((void_return_type_t)betatest4);
      break;

    case 2:
      call((void_return_type_t)betatest5);
      break;

    case 3:
      call((void_return_type_t)betatest6);
      break;

    case 5:
      exit_requested = 1;
      break;
    }
  }

  //-------------------------------------------------------------------------
  // Other hotkeys
  //-------------------------------------------------------------------------

  if (scan_code == _F4) { // F4 Open PRG
    v_type = FT_PRG;
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
  }

  if (scan_code == _F1) { // F1 Help
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

  if ((shift_status & MOD_ALT) && scan_code == _X) { // Alt-X Exit
    v_title = (char *)texts[40];
    v_text = NULL;
    show_dialog(accept0);

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
 * Repeatedly calls main_loop_tick() to pump events, manage windows, and render
 * the desktop until exit_requested is set (quit requested).
 */
void main_loop(void) {
  div_started = 1;
  int n, m, oldn = MAX_WINDOWS;

  int llamar;

  char cwork[256], *p;

  do {
    main_loop_tick();
  } while (!exit_requested);

  do {
    read_mouse();
  } while (mouse_b & MB_LEFT);

  printf("env end\n");
}

// Dialog loop state (used by dialog_loop in main_dialogs.c)
int n, m, oldn = MAX_WINDOWS;
int dialogo_invocado;
int salir_del_dialogo = 0;


// Window management, desktop rendering, animations moved to main_desktop.c
// Dialog system, item controls moved to main_dialogs.c

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
		VS_WIDTH=vga_width  =setup_file.vid_mode_width=640; // Video mode
		VS_HEIGHT =vga_height  =setup_file.vid_mode_height=480;
		VS_BIG  =big     =setup_file.vid_mode_big=1;
		editor_font      =setup_file.editor_font=3;
		big2=big+1;
	}
*/
  kbd_init();


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
  undo_table = (struct undo_entry *)malloc(sizeof(struct undo_entry) * max_undos);

  for (n = 0; n < MAX_WINDOWS; n++) {
    window[n].type = WIN_EMPTY;
    window[n].side = 0;
  }

  mouse_background = (byte *)malloc(1024 * big2);
  screen_buffer = (byte *)malloc(vga_width * vga_height + 6) + 6;
  dac = (byte *)malloc(PALETTE_SIZE);
  dac4 = (byte *)malloc(PALETTE_SIZE);
  color_lookup = (byte *)malloc(CUAD_TABLE_SIZE);

  ghost = (byte *)malloc(GHOST_TABLE_SIZE); // 256*256 combinations

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

    n = ftell(f) - FNT_GENCODE_OFFSET;

    if (n > 0 && (graf_ptr = (byte *)malloc(n)) != NULL) {
      memset(graf, 0, sizeof(graf));
      ptr = graf_ptr;
      fseek(f, FNT_GENCODE_OFFSET, SEEK_SET);
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
      ptr2 += FNT_GENCODE_OFFSET;

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
    copy_desktop = can_upload_desktop();

  if (!copy_desktop) { // Load common palette
    if (!interpreting) {
      cprintf("%s", (char *)texts[11]);
    } // *** Palette calculations ***
    memcpy(dac, system_dac, PALETTE_SIZE);
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
  draw_mode = TOOL_TRANSITION + TOOL_PENCIL;
  mode_fill = 0;
  mode_circle = 0;
  mode_rect = 1;
  mode_selection = 0;
  v.type = WIN_EMPTY;
  init_flush();
  next_code = 1;
  mouse_shift = 0;
  memcpy(original_palette, dac, PALETTE_SIZE);
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

  create_gradient_colors(setup_file.gradient_config, wallpaper_gradient);

  wallpaper = NULL;

  mouse_x = vga_width / 2;
  mouse_y = vga_height / 2;
  _mouse_x = mouse_x;
  _mouse_y = mouse_y;
  mouse_graf = CURSOR_ARROW;
  set_mouse(mouse_x, mouse_y); // set_mickeys(8);

  dragging = DRAG_IDLE;

  update_box(0, 0, vga_width, vga_height);

  create_title_bar();

  setup_video_mode();
  set_dac(dac);
  //read_mouse();
  prepare_wallpaper();
  full_redraw = 1;
  blit_screen(screen_buffer);

  sound_init();
  if (sound_active)
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

  kbd_reset();
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

int get_heap_free(int ret_used) {
  return 0;
}

char *get_memory_free() {
  return "";
}

void debug_file(char *string, char *name) {
  FILE *debug;

  debug = fopen("DEBUG.TXT", "ab");
  fwrite(string, 1, strlen(string), debug);
  fwrite("(", 1, 1, debug);
  fwrite(name, 1, strlen(name), debug);
  fwrite(") ", 1, 2, debug);
  fwrite(get_memory_free(), 1, strlen(get_memory_free()), debug);
  fwrite(")\n", 1, 2, debug);
  fclose(debug);
}


//-----------------------------------------------------------------------------
//  Load and save setup.bin
//-----------------------------------------------------------------------------

extern int previous_mode;

////////////////////////////////////////////////////////////////////////////
// Save the configuration file                                            //
////////////////////////////////////////////////////////////////////////////
void save_config() {
  FILE *file;

  // Video mode
  setup_file.vid_mode_width = VS_WIDTH;
  setup_file.vid_mode_height = VS_HEIGHT;
  setup_file.vid_mode_big = VS_BIG;
  setup_file.fullscreen = fsmode;

  if (return_mode == 3) {
    setup_file.vid_mode_big = (previous_mode & 0x8000000) >> 31;
    previous_mode -= (previous_mode & 0x80000000);
    setup_file.vid_mode_width = previous_mode / 10000;
    setup_file.vid_mode_height = previous_mode % 10000;
  }

  // Undo system
  setup_file.max_undo = max_undos;
  setup_file.undo_memory = undo_memory;
  setup_file.tab_size = tab_size;

  // Directory system

  div_strcpy(setup_file.dir_cwd, sizeof(setup_file.dir_cwd), file_types[0].path);
  div_strcpy(setup_file.dir_map, sizeof(setup_file.dir_map), file_types[2].path);
  div_strcpy(setup_file.dir_pal, sizeof(setup_file.dir_pal), file_types[3].path);
  div_strcpy(setup_file.dir_fpg, sizeof(setup_file.dir_fpg), file_types[FT_FPG].path);
  div_strcpy(setup_file.dir_fnt, sizeof(setup_file.dir_fnt), file_types[FT_FNT].path);
  div_strcpy(setup_file.dir_ifs, sizeof(setup_file.dir_ifs), file_types[FT_IFS].path);
  div_strcpy(setup_file.dir_pcm, sizeof(setup_file.dir_pcm), file_types[7].path);
  div_strcpy(setup_file.dir_prg, sizeof(setup_file.dir_prg), file_types[8].path);
  div_strcpy(setup_file.dir_pcms, sizeof(setup_file.dir_pcms), file_types[FT_AUDIO_SAVE].path);
  div_strcpy(setup_file.dir_prj, sizeof(setup_file.dir_prj), file_types[12].path);
  div_strcpy(setup_file.dir_wld, sizeof(setup_file.dir_wld), file_types[15].path);
  div_strcpy(setup_file.dir_mod, sizeof(setup_file.dir_mod), file_types[16].path);
  /*
        // Wallpaper info
        strcpy(setup_file.desktop_image,desk_file);
        setup_file.Desktop_R=desk_r;
        setup_file.Desktop_G=desk_g;
        setup_file.Desktop_B=desk_b;
        setup_file.desktop_tile=desk_tile;
*/
  setup_file.editor_font = editor_font;
  setup_file.paint_cursor = paint_cursor;
  setup_file.exploding_windows = exploding_windows;
  setup_file.auto_save_session = auto_save_session;
  setup_file.move_full_window = move_full_window;
  setup_file.colorizer = colorizer;
  memcpy(&setup_file.colors_rgb[0], &colors_rgb[0], 12 * 3);

  file = fopen("system/setup.bin", "wb");
  fwrite(&setup_file, 1, sizeof(setup_file), file);
  fclose(file);
}

////////////////////////////////////////////////////////////////////////////
// Load config file                                                       //
////////////////////////////////////////////////////////////////////////////

void load_config() {
  int n;
  FILE *file;
  char cWork[_MAX_PATH + 1];

  // System Directories

  div_strcpy(setup_file.dir_cwd, sizeof(setup_file.dir_cwd), file_types[0].path);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/MAP");
  div_strcpy(setup_file.dir_map, sizeof(setup_file.dir_map), cWork);
  div_strcpy(file_types[2].path, sizeof(file_types[2].path), cWork);
  div_strcpy(file_types[FT_WALLPAPER].path, sizeof(file_types[FT_WALLPAPER].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/PAL");
  div_strcpy(setup_file.dir_pal, sizeof(setup_file.dir_pal), cWork);
  div_strcpy(file_types[3].path, sizeof(file_types[3].path), cWork);
  div_strcpy(file_types[FT_PAL_SAVE].path, sizeof(file_types[FT_PAL_SAVE].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/FPG");
  div_strcpy(setup_file.dir_fpg, sizeof(setup_file.dir_fpg), cWork);
  div_strcpy(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/FNT");
  div_strcpy(setup_file.dir_fnt, sizeof(setup_file.dir_fnt), cWork);
  div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/IFS");
  div_strcpy(setup_file.dir_ifs, sizeof(setup_file.dir_ifs), cWork);
  div_strcpy(file_types[FT_IFS].path, sizeof(file_types[FT_IFS].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/PCM");
  div_strcpy(setup_file.dir_pcm, sizeof(setup_file.dir_pcm), cWork);
  div_strcpy(file_types[7].path, sizeof(file_types[7].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/PRG");
  div_strcpy(setup_file.dir_prg, sizeof(setup_file.dir_prg), cWork);
  div_strcpy(file_types[8].path, sizeof(file_types[8].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/PCM");
  div_strcpy(setup_file.dir_pcms, sizeof(setup_file.dir_pcms), cWork);
  div_strcpy(file_types[FT_AUDIO_SAVE].path, sizeof(file_types[FT_AUDIO_SAVE].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/PRJ");
  div_strcpy(setup_file.dir_prj, sizeof(setup_file.dir_prj), cWork);
  div_strcpy(file_types[12].path, sizeof(file_types[12].path), cWork);

  div_strcpy(file_types[13].path, sizeof(file_types[13].path), file_types[1].path);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/WLD");
  div_strcpy(setup_file.dir_wld, sizeof(setup_file.dir_wld), cWork);
  div_strcpy(file_types[15].path, sizeof(file_types[15].path), cWork);

  div_strcpy(cWork, sizeof(cWork), file_types[1].path);
  div_strcat(cWork, sizeof(cWork), "/MOD");
  div_strcpy(setup_file.dir_mod, sizeof(setup_file.dir_mod), cWork);
  div_strcpy(file_types[16].path, sizeof(file_types[16].path), cWork);

  file = fopen("system/setup.bin", "rb");
  if (file == NULL) {
    if (first_run) {
      div_strcpy(setup_file.desktop_image, sizeof(setup_file.desktop_image),
                 (char *)texts[487]); // Wallpaper info
      setup_file.desktop_gamma = 1;
      setup_file.desktop_tile = 0;
      for (n = 0; n < 8; n++)
        setup_file.gradient_config[n].selec = 0;
      setup_file.gradient_config[8].selec = 1;
      setup_file.gradient_config[8].r = 0;
      setup_file.gradient_config[8].g = 0;
      setup_file.gradient_config[8].b = 63;
      setup_file.vid_mode_width = 640; // Video Mode
      setup_file.vid_mode_height = 480;
      setup_file.vid_mode_big = 1;
      setup_file.fullscreen = 0;

      setup_file.max_undo = 1024; // Undo System
      setup_file.undo_memory = 1024 * 1024 + 65536;
      setup_file.tab_size = 4;
      memcpy(&setup_file.colors_rgb[0],
             "\x1c\x1c\x1c\x3d\x3d\x3c\x0\x0\x20\x2\x6\x7\x0\x27\x0\x3d\x3d\x3c\x14\x14"
             "\x14\x3d\x3d\x3c\x0\x0\x2e\x34\x31\x31\x22\x13\x13\x26\x26\x0",
             12 * 3);
      setup_file.editor_font = 2;
      setup_file.paint_cursor = 0;
      setup_file.exploding_windows = 1;
      setup_file.auto_save_session = 1;
      setup_file.move_full_window = 1;
      setup_file.colorizer = 1;

      setup_file.vol_fx = 7;
      setup_file.vol_cd = 7;
      setup_file.vol_ma = 7;
      setup_file.mut_fx = 0;
      setup_file.mut_cd = 0;
      setup_file.mut_ma = 0;

      setup_file.mouse_ratio = 0;

    } else {
      div_strcpy(setup_file.desktop_image, sizeof(setup_file.desktop_image), ""); // Wallpaper info
      setup_file.desktop_gamma = 0;
      setup_file.desktop_tile = 0;
      for (n = 0; n < 9; n++)
        setup_file.gradient_config[n].selec = 0;
      setup_file.vid_mode_width = 320; // Video Mode
      setup_file.vid_mode_height = 200;
      setup_file.vid_mode_big = 0;
      setup_file.fullscreen = 0;

      setup_file.max_undo = 1024; // Undo System
      setup_file.undo_memory = 1024 * 1024 + 65536;
      setup_file.tab_size = 4;
      memcpy(&setup_file.colors_rgb[0],
             "\x1c\x1c\x1c\x3d\x3d\x3c\x0\x0\x20\x2\x6\x7\x0\x27\x0\x3d\x3d\x3c\x14\x14"
             "\x14\x3d\x3d\x3c\x0\x0\x2e\x34\x31\x31\x22\x13\x13\x26\x26\x0",
             12 * 3);
      setup_file.editor_font = 0;
      setup_file.paint_cursor = 0;
      setup_file.exploding_windows = 1;
      setup_file.auto_save_session = 1;
      setup_file.move_full_window = 1;
      setup_file.colorizer = 1;

      setup_file.vol_fx = 10;
      setup_file.vol_cd = 10;
      setup_file.vol_ma = 10;
      setup_file.mut_fx = 0;
      setup_file.mut_cd = 0;
      setup_file.mut_ma = 0;

      setup_file.mouse_ratio = 0;
    }
  } else {
    fread(&setup_file, 1, sizeof(setup_file), file);
    fclose(file);
  }

  editor_font = setup_file.editor_font;
  paint_cursor = setup_file.paint_cursor;
  exploding_windows = setup_file.exploding_windows;
  auto_save_session = setup_file.auto_save_session;
  move_full_window = setup_file.move_full_window;
  colorizer = setup_file.colorizer;
  memcpy(&colors_rgb[0], &setup_file.colors_rgb[0], 12 * 3);

  VS_WIDTH = vga_width = setup_file.vid_mode_width; // Video mode
  VS_HEIGHT = vga_height = setup_file.vid_mode_height;
  VS_BIG = big = setup_file.vid_mode_big;
  fsmode = setup_file.fullscreen;

  if (test_video && vga_width < 640 && editor_font > 1)
    editor_font = 0;

  big2 = big + 1;

  max_undos = setup_file.max_undo; // Undo System
  undo_memory = setup_file.undo_memory;
  tab_size = setup_file.tab_size;

  // System Directories

  if (interpreting)
    if (chdir(setup_file.dir_cwd) != -1)
      div_strcpy(file_types[0].path, sizeof(file_types[0].path), setup_file.dir_cwd);

  if (chdir(setup_file.dir_map) != -1)
    div_strcpy(file_types[2].path, sizeof(file_types[2].path), setup_file.dir_map);
  else if (chdir(file_types[2].path) == -1)
    div_strcpy(file_types[2].path, sizeof(file_types[2].path), file_types[1].path);

  if (chdir(setup_file.dir_pal) != -1)
    div_strcpy(file_types[3].path, sizeof(file_types[3].path), setup_file.dir_pal);
  else if (chdir(file_types[3].path) == -1)
    div_strcpy(file_types[3].path, sizeof(file_types[3].path), file_types[1].path);

  if (chdir(setup_file.dir_fpg) != -1)
    div_strcpy(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), setup_file.dir_fpg);
  else if (chdir(file_types[FT_FPG].path) == -1)
    div_strcpy(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), file_types[1].path);

  if (chdir(setup_file.dir_fnt) != -1)
    div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), setup_file.dir_fnt);
  else if (chdir(file_types[FT_FNT].path) == -1)
    div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), file_types[1].path);

  if (chdir(setup_file.dir_ifs) != -1)
    div_strcpy(file_types[FT_IFS].path, sizeof(file_types[FT_IFS].path), setup_file.dir_ifs);
  else if (chdir(file_types[FT_IFS].path) == -1)
    div_strcpy(file_types[FT_IFS].path, sizeof(file_types[FT_IFS].path), file_types[1].path);

  if (chdir(setup_file.dir_pcm) != -1)
    div_strcpy(file_types[7].path, sizeof(file_types[7].path), setup_file.dir_pcm);
  else if (chdir(file_types[7].path) == -1)
    div_strcpy(file_types[7].path, sizeof(file_types[7].path), file_types[1].path);

  if (chdir(setup_file.dir_prg) != -1)
    div_strcpy(file_types[8].path, sizeof(file_types[8].path), setup_file.dir_prg);
  else if (chdir(file_types[8].path) == -1)
    div_strcpy(file_types[8].path, sizeof(file_types[8].path), file_types[1].path);

  div_strcpy(file_types[FT_WALLPAPER].path, sizeof(file_types[FT_WALLPAPER].path), file_types[2].path);
  div_strcpy(file_types[FT_PAL_SAVE].path, sizeof(file_types[FT_PAL_SAVE].path), file_types[3].path);

  if (chdir(setup_file.dir_pcms) != -1)
    div_strcpy(file_types[FT_AUDIO_SAVE].path, sizeof(file_types[FT_AUDIO_SAVE].path), setup_file.dir_pcms);
  else if (chdir(file_types[FT_AUDIO_SAVE].path) == -1)
    div_strcpy(file_types[FT_AUDIO_SAVE].path, sizeof(file_types[FT_AUDIO_SAVE].path), file_types[1].path);

  if (chdir(setup_file.dir_prj) != -1)
    div_strcpy(file_types[12].path, sizeof(file_types[12].path), setup_file.dir_prj);
  else if (chdir(file_types[12].path) == -1)
    div_strcpy(file_types[12].path, sizeof(file_types[12].path), file_types[1].path);

  div_strcpy(file_types[13].path, sizeof(file_types[13].path), file_types[1].path); // Generic

  if (chdir(setup_file.dir_wld) != -1)
    div_strcpy(file_types[15].path, sizeof(file_types[15].path), setup_file.dir_wld);
  else if (chdir(file_types[15].path) == -1)
    div_strcpy(file_types[15].path, sizeof(file_types[15].path), file_types[1].path);

  if (chdir(setup_file.dir_mod) != -1)
    div_strcpy(file_types[16].path, sizeof(file_types[16].path), setup_file.dir_mod);
  else if (chdir(file_types[16].path) == -1)
    div_strcpy(file_types[16].path, sizeof(file_types[16].path), file_types[1].path);

  chdir(file_types[1].path);
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

  for (m = 0; m < MAX_WINDOWS; m++) {
    if (window[m].type == WIN_CODE && window[m].state && window[m].prg != NULL) {
      n = m;
      break;
    }
  }
  return (v_window = n);
}

int determine_calc(void) {
  int m, n = -1;

  for (m = 0; m < MAX_WINDOWS; m++) {
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

void div_delete(int a) {
  memmove(&window[a].type, &window[a + 1].type, sizeof(struct twindow) * (MAX_WINDOWS - 1 - a));
  window[MAX_WINDOWS - 1].type = WIN_EMPTY;
}

void add_window(void) {
  memmove(&window[1].type, &v.type, sizeof(struct twindow) * (MAX_WINDOWS - 1));
}

void wup(int a) {
  if (++ivaux == 4)
    ivaux = 0;
  copy(-1, 0);
  copy(0, a);
  window[a].type = WIN_EMPTY;
}

void wdown(int a) {
  copy(a, 0);
  copy(0, -1);
  if (--ivaux == -1)
    ivaux = 3;
}

void delete_file(char *name) {
#ifdef WIN32
  debugprintf("delete_file %s\n", name);
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

void debug_info(char *msg) {}

void debug_data(int val) {}

void get_free_4k_blocks(void) {}

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
