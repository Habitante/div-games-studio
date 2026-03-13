// TODO: Allow the user to configure imem_max (maximum memory / max process count)
// at runtime instead of it being a compile-time constant.

///////////////////////////////////////////////////////////////////////////////
// Libraries used
///////////////////////////////////////////////////////////////////////////////

#define DEFINE_GLOBALS_HERE

#include "inter.h"
#include "div_string.h"

#include "divsound.h"
#include "mixer.h"
#include <time.h>
#include <sched.h>

void find_packfile(void);

void InitHandler(int);
int DetectBlaster(int *, int *, int *, int *, int *);
int DetectGUS(int *, int *, int *, int *, int *);
void system_font(void);
void interpreter(void);
void create_color_lookup(void);
void exec_process(void);
void finalization(void);
void kill_process(int id);
void core_exec(void);
void core_trace(void);
int get_reloj(void);


void deb(void);
extern int ids_old;
extern int skip_flush;

void readmouse(void);

int trace_program = 0;
#ifdef __EMSCRIPTEN__
int ignore_errors = 1;
#else
int ignore_errors = 0;
#endif

int old_dump_type;
int old_restore_type;

int last_key_check, key_check, last_joy_check, joy_check; // Call screensaver
int last_mou_check, mou_check;

void function_exec(int id, int n); // Id, ciclos
void process_exec(int id, int n);  // Id, ciclos
void process_paint(int id, int n); // Id, ciclos

int get_ticks(void);

int restore_time = 0;
int blit_time = 0;

int process_level = 0; // Track call/ret nesting (for debugger step)

int nullstring[4];
int nstring = 0;

int max, max_reloj; // Process in order or _Priority and _Z
extern int alt_x;
extern int app_paused;
int splashtime = 5000; // 5 seconds
#ifdef EMSCRIPTEN
byte running = 0;
#endif

void madewith(void);


#ifdef LLPROC


int checklist = 0;
int dtemp = 0;

struct llist {
  int pid;
  struct llist *lnext;
  int next;
  struct llist *lprev;
  int prev;
  int z;
  int p;
};


struct llist *plist = NULL;

void first_process(void) {
  plist = (struct llist *)malloc(sizeof(struct llist));

  plist->next = 0;
  plist->lnext = NULL;
  plist->prev = 0;
  plist->lprev = NULL;
  plist->z = 0;
  plist->p = 0;
  plist->pid = 0;
}

void proc_list(void) {
  return;
  struct llist *f = plist;

  printf("Proc tree: \n");

  do {
    f = f->lnext;
    printf("ID: %x %d %d \n", f, f->pid, f->z);
  } while (f->lnext != NULL);
}

struct llist *find_proc(int id) {
  struct llist *ls = plist;

  do {
    if (ls->pid == id) {
      printf("found id: %d %x\n", id, ls);
      return ls;
    }
    ls = ls->lnext;
  } while (ls != NULL);

  return NULL;
}
void dirty(int did) {
  struct llist *fs = find_proc(did);
  struct llist *ff = plist;
  struct llist *pf = NULL;

  printf("fs: %x\nBefore id %d removed:\n", fs, did);

  proc_list();


  if (fs->lprev != NULL) {
    fs->lprev->lnext = fs->lnext;
    fs->lprev->next = fs->next;
  }
  if (fs->lnext != NULL) {
    fs->lnext->lprev = fs->lprev;
    fs->lnext->prev = fs->prev;
  }

  printf("After id %d removed:\n", did);


  proc_list();

  if (checklist == _Z)
    fs->z = mem[did + checklist];
  else
    fs->p = mem[did + checklist];

  printf("%c changed, Old: %d New: %d\n", ((checklist == _Z) ? 'Z' : 'P'), dtemp,
         mem[did + checklist]);

  // look for slot

  do {
    if (checklist == _Z) {
      if (ff->z > mem[did + checklist]) {
        pf = ff;
      }
    }
    if (checklist == _P) {
      if (ff->p > mem[did + checklist]) {
        printf("MATCHED P\n");
        pf = ff;
      }
    }
    ff = ff->lnext;

  } while (ff != NULL);


  if (pf == NULL) {
    pf = plist; // first
  }

  printf("pf: %x ff: %x\n", pf, ff);


  if (pf != NULL) {
    ff = pf;

    fs->lnext = ff->lnext;
    fs->next = ff->next;

    if (ff->lnext != NULL) {
      ff->lnext->lprev = fs;
      ff->lnext->prev = did;
    }


    ff->lnext = fs;
    ff->next = fs->pid;
  }
  checklist = 0;
  dtemp = 0;

  printf("DIRTY DONE\n");
  proc_list();
}


void remove_process(int remove_id) {
  printf("[Removing process] %d\n", remove_id);

  struct llist *f = find_proc(remove_id);

  if (f == NULL)
    return;

  printf("lprev %x\n", f->lprev);


  f->lprev->lnext = f->lnext;
  f->lprev->next = f->next;


  if (f->lnext != NULL) {
    f->lnext->lprev = f->lprev;
    f->lnext->prev = f->prev;
  }

  free(f);

  proc_list();
}


void insert_process(int insert_id) {
  // create new struct to insert into list

  struct llist *pnew = (struct llist *)malloc(sizeof(struct llist));


  struct llist *ln = plist->lnext;

  pnew->z = 0;
  pnew->p = 0;

  plist->lnext = pnew;


  // this id
  pnew->pid = insert_id;

  // first is now this
  plist->next = insert_id;
  plist->lnext = pnew;

  pnew->lnext = ln;
  pnew->lprev = plist;

  pnew->prev = plist->pid;

  pnew->next = 0;

  if (ln != NULL) {
    pnew->next = ln->pid;
    ln->lprev = pnew;
    ln->prev = insert_id;
  }

  printf("New process id: %d %x %x\n", insert_id, pnew, pnew->lprev);

  proc_list();
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Critical error handler
///////////////////////////////////////////////////////////////////////////////

unsigned c_e_0, c_e_1;
unsigned far *c_e_2;

int __far critical_error(unsigned deverr, unsigned errcode, unsigned far *devhdr) {
  c_e_0 = deverr;
  c_e_1 = errcode;
  c_e_2 = devhdr;
  return (_HARDERR_IGNORE);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Main program
///////////////////////////////////////////////////////////////////////////////

void GetFree4kBlocks(void);
int DOSalloc4k(void);
int DPMIalloc4k(void);

#ifdef __WATCOMC__
#pragma aux DOSalloc4k = "mov bx,0100h", "mov ax,0100h", "int 31h", "mov ecx,0", "jc short done", \
            "mov bx,dx", "mov ax,0006h", "int 31h", "shl ecx,16", "mov cx,dx",                    \
            "done:", modify[ax bx dx] value[ecx];

#pragma aux DPMIalloc4k = "mov cx,1000h", "mov bx,0000h", "mov ax,0501h", "int 31h", "mov edx,0", \
            "jc short done", "shrd edx,ebx,16", "mov dx,cx",                                      \
            "done:", modify[ax bx cx si di] value[edx];
#endif /* __WATCOMC__ */

//-----------------------------------------------------------------------------
// Initialise (setup)
//-----------------------------------------------------------------------------

void check_mouse(void);
int _mouse_x, _mouse_y;

extern int find_status;

#include "sysdac.h"
time_t dtime;
memptrsize stack_pool[65535];

void initialization(void) {
  int n;

  for (n = 0; n < 65535; n++)
    stack_pool[n] = 0;

  mouse = (struct _mouse *)&mem[HEADER_LENGTH];
  scroll = (struct _scroll *)&mem[HEADER_LENGTH + 14];
  m7 = (struct _m7 *)&mem[HEADER_LENGTH + 14 + 10 * 10];
  joy = (struct _joy *)&mem[HEADER_LENGTH + 14 + 10 * 10 + 10 * 7];
  setup = (struct _setup *)&mem[HEADER_LENGTH + 14 + 10 * 10 + 10 * 7 + 8];
  // net = (struct _net *)&mem[HEADER_LENGTH + 14 + 10 * 10 + 10 * 7 + 8 + 11];
  // m8 pointer removed (MODE8 deleted) - memory layout preserved for compatibility
  dirinfo = (struct _dirinfo *)&mem[HEADER_LENGTH + 14 + 10 * 10 + 10 * 7 + 8 + 11];
  fileinfo = (struct _fileinfo *)&mem[HEADER_LENGTH + 14 + 10 * 10 + 10 * 7 + 8 + 11 + 1026];
  video_modes =
      (struct _video_modes *)&mem[HEADER_LENGTH + 14 + 10 * 10 + 10 * 7 + 8 + 11 + 1026 + 146];
  iloc = mem[2];              // Start of local variables
  iloc_len = mem[6] + mem[5]; // Length of local (public and private)
  iloc_pub_len = mem[6];      // Length of local public variables
  private_start = iloc_pub_len;
  imem = mem[8]; // End of code, locals, and texts (compiled length)

  if (iloc_len & 1) {
    iloc_len++;
  }
  if (!(imem & 1)) {
    imem++;
  }

  if ((screen_buffer = (byte *)malloc(vga_width * vga_height)) == NULL)
    exer(1);
  memset(screen_buffer, 0, vga_width * vga_height);

  if ((back_buffer = (byte *)malloc(vga_width * (vga_height + 1))) == NULL)
    exer(1);
  memset(back_buffer, 0, vga_width * vga_height);

  memset(divmalloc, 0, sizeof(divmalloc));

#ifdef DEBUG
  if ((screen_buffer_debug = (byte *)malloc(vga_width * vga_height)) == NULL)
    exer(1);
#endif

  if ((ghost_inicial = (byte *)malloc(65536 + 512)) == NULL)
    exer(1);

  ghost = (byte *)((uintptr_t)(ghost_inicial + 512)); //&0xFFFFFF00);

  create_color_lookup();

  // Create the first two processes: init and main

  process_count = 1;
  id_init = imem;
  imem += iloc_len;
  id_start = id_end = imem;

// setup linked list
#ifdef LLPROC
  first_process();

  insert_process(id_start);
#endif

  mem[iloc + _x1] = -1;

  memcpy(&mem[id_init], &mem[iloc], iloc_pub_len << 2); // *** Init
  mem[id_init + _Id] = id_init;
  mem[id_init + _IdScan] = id_init;
  mem[id_init + _Status] = 2;

  memcpy(&mem[id_start], &mem[iloc], iloc_pub_len << 2); // *** Main
  mem[id_start + _Id] = id_start;
  mem[id_start + _IdScan] = id_start;
  mem[id_start + _Status] = 2;
  mem[id_start + _IP] = mem[1];

  mem[id_init + _Son] = id_start;
  mem[id_start + _Father] = id_init;

  for (n = 0; n < max_region; n++) {
    region[n].x0 = 0;
    region[n].y0 = 0;
    region[n].x1 = vga_width;
    region[n].y1 = vga_height;
  }

  memset(g, 0, sizeof(g));

  if ((g[0].grf = (int **)malloc(sizeof(int *) * 2000)) == NULL)
    exer(1);
  memset(g[0].grf, 0, sizeof(int *) * 2000);
  next_map_code = 1000;

  memset(fonts, 0, sizeof(fonts));
  memset(texts, 0, sizeof(texts));
  memset(drawing, 0, sizeof(drawing));
  memset(video_modes, 0, 12 * 32);

  system_font();

  time(&dtime);

  init_rnd(dtime);

  detect_vesa();

  for (n = 0; n < num_video_modes; n++) {
    if (video_modes[n].width == vga_width && video_modes[n].height == vga_height) {
      break;
    }
  }

  vvga_w = vga_width;
  vvga_h = vga_height;


  setup_video_mode();
  _mouse_x = mouse->x;
  _mouse_y = mouse->y;

  memset(&palette[0], 0, 768);
  memset(&dac[0], 0, 768);
  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 8;
  now_dacout_r = 64;
  now_dacout_g = 64;
  now_dacout_b = 64;
  palette_loaded = 0;

  set_dac();
  dirinfo->files = 0;
  memset(dirinfo->name, 0, 1025 * 4);

  mouse->z = -512;
  mouse->x = vga_width / 2;
  mouse->y = vga_height / 2;
  mouse->size = 100;
  mouse->speed = 2;

  check_mouse(); // mouse->cursor = 1-No mouse, 0-Mouse present

  for (n = 0; n < 10; n++) { // Initially there is no scroll or mode-7 system
    (scroll + n)->z = 512;
    (scroll + n)->ratio = 200;
    (scroll + n)->region1 = -1;
    (scroll + n)->region2 = -1;

    (m7 + n)->z = 256;
    (m7 + n)->height = 32;
    (m7 + n)->distance = 64;
    (m7 + n)->focus = 256;

    // m8 initialization removed (MODE8 deleted)
    (m7 + n)->height = 32;
  }

  ticks = 0;
  frame_clock = 0;
  last_clock = 0;
  fractional_clock = clock_interval = 1000.0 / 24.0;
  game_fps = dfps = 24;
  max_frame_skips = 0;
#ifdef __EMSCRIPTEN__
  max_frame_skips = 0;
#endif


  joy_timeout = 0;

  num_skipped = 0;

#ifdef DEBUG
  debugger_step = 0;
  call_to_debug = 0;
  process_stoped = 0;
#endif

  skip_blit = 0;
  blits_skipped = 0;

  init_sin_cos(); // Sine and cosine tables for mode-7

  memcpy(palette, system_dac, 768);
  apply_palette();

  auto_adapt_palette = 0; // Until force_pal is called...

#ifdef DEBUG
#ifndef __EMSCRIPTEN__
  init_debug();
  new_palette = 0;
  new_mode = 0;
#endif
#endif

  div_strcpy(packfile, sizeof(packfile), "");
  npackfiles = 0;

#ifndef DEBUG
  find_packfile();
#endif

  init_flush();

  ss_time = 3000;
  ss_time_counter = 0;
  ss_status = 1;
  apply_palette_flag = 0;

  memset(tabfiles, 0, 32 * 4);

  for (n = 0; n < 128; n++) {
    sounds[n].smp = 0;
    songs[n].ptr = NULL;
  }

  sound_init();
#ifdef JUDAS
  if (judascfg_device != DEV_NOSOUND)
    set_init_mixer();
#endif

  find_status = 0;
}

//-----------------------------------------------------------------------------
//  Create the squared-difference color lookup table
//-----------------------------------------------------------------------------

void create_color_lookup(void) {
  int a, b;

  if ((color_lookup = (byte *)malloc(16384)) == NULL)
    exer(1);

  a = 0;
  do {
    b = 0;
    do {
      *(int *)(color_lookup + a * 4 * 64 + b * 4) = (a > b) ? (a - b) * (a - b) : (b - a) * (b - a);
    } while (++b < 64);
  } while (++a < 64);
}

//-----------------------------------------------------------------------------
//      Load the system font
//-----------------------------------------------------------------------------

#include "06x08.h"

byte *sys06x08 = NULL;

void system_font(void) {
  int n, m;
  int *ptr;
  byte *si, *di, x;

  if ((sys06x08 = (byte *)malloc(12288)) == NULL)
    exer(1);

  si = (byte *)_06x08;
  di = sys06x08;

  for (n = 0; n < 1536; n++) {
    x = *si++;
    for (m = 0; m < 8; m++) {
      if (x & 128)
        *di++ = 1;
      else
        *di++ = 0;
      x *= 2;
    }
  }

  n = 1356 + sizeof(fnt_table_entry) * 256 + 12288;

  if ((fonts[0] = (byte *)malloc(n)) == NULL)
    exer(1);

  memset(fonts[0], 0, n);
  memcpy(fonts[0] + n - 12288, sys06x08, 12288);

  ptr = (int *)(fonts[0] + 1356);
  for (n = 0; n < 256; n++) {
    *ptr++ = 6;
    *ptr++ = 8;
    *ptr++ = 0;
    *ptr++ = 1356 + sizeof(fnt_table_entry) * 256 + n * 48;
  }
  last_c1 = 1;

  f_i[0].width = 6;
  f_i[0].spacing = 3;
  f_i[0].letter_spacing = 0;
  f_i[0].height = 8;
}

//-----------------------------------------------------------------------------
// Multi-stack system for functions (DIV 2)
//-----------------------------------------------------------------------------

// Functions:
//      save_stack(id,sp,mem[id+_SP]); // In lfrm and lfrf of functions
//      load_stack(id);                 // In exec_process/trace_process
//      update_stack(id,value);        // Set a value at the end of the stack (rtf)

// * (int *) mem[id+_SP] = {SP1,SP2,DATA...}
int stacks = 0;

void save_stack(int id, int sp1, int sp2) {
  int n;
  memptrsize *p;
  p = (memptrsize *)malloc((sp2 - sp1 + 3) * sizeof(memptrsize));
  stacks = 0;
  while (stacks < 65535) {
    if (stack_pool[stacks] > 0)
      stacks++;
    else
      break;
  }
  printf("using stack: %d\n", stacks);
  stack_pool[stacks] = (memptrsize)(uintptr_t)p;

  if (p != NULL) {
    mem[id + _SP] = stacks;
    p[0] = sp1;
    p[1] = sp2;
    for (n = 0; n <= sp2 - sp1; n++)
      p[n + 2] = stack[sp1 + n];
  } else
    mem[id + _SP] = 0;
}

/* Restore a process's saved execution stack from its heap-allocated backup.
 * Called when resuming a process in exec_process(). The backup (created by
 * save_stack) stores [sp_low, sp_high, data...]; this copies data back
 * into stack[], sets sp to sp_high, and frees the backup.
 */
void load_stack(int id) {
  int n;
  int32_t *p;
  if (mem[id + _SP]) {
    p = (int32_t *)(uintptr_t)stack_pool[mem[id + _SP]];

    for (n = 0; n <= p[1] - p[0]; n++)
      stack[p[0] + n] = p[n + 2];

    //    free(stack_pool[mem[id+_SP]]);
    stack_pool[mem[id + _SP]] = 0;
    mem[id + _SP] = 0;
    sp = p[1];
    free(p);
  } else
    sp = 0;
}

void update_stack(int id, int value) {
  int32_t *p;
  if (mem[id + _SP]) {
    p = (int32_t *)(uintptr_t)stack_pool[mem[id + _SP]];
    p[p[1] - p[0] + 2] = value;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Interpret the generated code
///////////////////////////////////////////////////////////////////////////////

void mainloop(void) {
#ifdef EMSCRIPTEN
  if (!(process_count && !(kbdFLAGS[_ESC] && kbdFLAGS[_L_CTRL]) && !alt_x)) {
    fprintf(stdout, "Program finished. Ending.\n");
    emscripten_cancel_main_loop();
    finalization();
    return;
  }
#endif

#ifndef DEBUG
  if (splashtime > 0) {
    if (OSDEP_GetTicks() < splashtime) {
      poll_keyboard();
      return;
    } else {
      splashtime = 0;
      setup_video_mode();
    }
  }
#endif

  frame_start();

#ifdef DEBUG
  if (kbdFLAGS[_F12] || trace_program) {
    trace_program = 0;
    if (debug_active)
      call_to_debug = 1;
  }
#endif

  old_dump_type = dump_type;
  old_restore_type = restore_type;
  do {
#ifdef DEBUG
    if (call_to_debug) {
      call_to_debug = 0;
      debug();
    }
#endif
    exec_process();
  } while (ide);
  frame_end();
#ifdef EMSCRIPTEN
#endif
}

/* VM entry point: initializes the runtime, then runs the main game loop
 * until all processes are dead or the user presses Ctrl+Esc / Alt+X.
 * Calls finalization() on exit to free all runtime resources.
 */
void interpreter(void) {
  initialization();
#ifndef DEBUG
#endif

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(mainloop, 0, 1);
#else
  while (process_count && !(kbdFLAGS[_ESC] && kbdFLAGS[_L_CTRL]) && !alt_x) {
    mainloop();
  }
  finalization();
#endif
}

#ifdef __EMSCRIPTEN__
void is_fps(byte f) {}
#endif
//-----------------------------------------------------------------------------
// Execute the next process
//-----------------------------------------------------------------------------
#ifdef DEBUG
int profile_clock;
#endif

#ifndef DEBUG
inline
#endif

    void
    exec_process(void) {

  ide = 0;
  max = 0x80000000;

#ifdef DEBUG

  profile_clock = get_ticks();

  if (process_stoped) {
    id = ide = process_stoped;
    process_stoped = 0;
    goto continue_process;
  }

#endif

  id = id_old;

  do {
    if (mem[id + _Status] == 2 && !mem[id + _Executed] && mem[id + _Priority] > max) {
      ide = id;
      max = mem[id + _Priority];
    }

    if (id == id_end)
      id = id_start;
    else
      id += iloc_len;

  } while (id != id_old);


  if (ide) {
    if (mem[ide + _Frame] >= 100) {
      mem[ide + _Frame] -= 100;
      mem[ide + _Executed] = 1;
    } else {
      id = ide;
      ip = mem[id + _IP];
      load_stack(id);

#ifdef DEBUG
continue_process:
#endif

      max_reloj = get_reloj() + max_process_time;

      core_exec();

      id = ide;

      // DLL callbacks
      if (post_process != NULL)
        post_process();
    }
  }
}

//-----------------------------------------------------------------------------
//  Interpreter core execution loop
//-----------------------------------------------------------------------------

int oo; // For internal kernel use

void core_exec() {
  do {
    switch ((byte)mem[ip++]) {
#include "debug/kernel.inc"
    }
  } while (1);

next_process1:
  mem[ide + _Executed] = 1;
next_process2:;
}

//-----------------------------------------------------------------------------
// Execute the next instruction of the next process (single-step trace)
//-----------------------------------------------------------------------------

#ifdef DEBUG

void trace_process(void) {
  profile_clock = get_ticks();

  ide = 0;
  max = 0x80000000;

  if (process_stoped) {
    id = ide = process_stoped;
    process_stoped = 0;
    goto continue_process;
  }


  id = id_old;
  do {
    if (mem[id + _Status] == 2 && !mem[id + _Executed] && mem[id + _Priority] > max) {
      ide = id;
      max = mem[id + _Priority];
    }

    if (id == id_end)
      id = id_start;
    else
      id += iloc_len;

  } while (id != id_old);

  id_old = ide;

  if (ide) {
    if (mem[ide + _Frame] >= 100) {
      mem[ide + _Frame] -= 100;
      mem[ide + _Executed] = 1;
    } else {
      id = ide;
      ip = mem[id + _IP];
      load_stack(id);

continue_process:

      max_reloj = get_reloj() + max_process_time;

      core_trace();
    }
  }

  id = ide;
}

//-----------------------------------------------------------------------------
//  Interpreter core execution loop, when tracing
//-----------------------------------------------------------------------------

void core_trace(void) {
  switch ((byte)mem[ip++]) {
#define TRACE
#include "debug/kernel.inc"
  }

  process_stoped = id;
  return;

next_process1:
  mem[ide + _Executed] = 1;
next_process2:

  if (post_process != NULL)
    post_process();
}

#endif // DEBUG

///////////////////////////////////////////////////////////////////////////////
// Start a frame prepares timer variables turns and looks
///////////////////////////////////////////////////////////////////////////////

float ffps = 24.0f;

#ifdef DEBUG

float ffps2 = 0.0f;
int overall_clock = 0;
double game_ticks = 0.0f;
double game_frames = 0.0f;

#endif


/* Begin-of-frame housekeeping: polls input, pauses when app loses focus,
 * eliminates dead processes (Status==1), updates the 10 user timers,
 * calculates FPS, enforces frame timing / frame-skip, and resets per-frame
 * flags so every live process is eligible for execution and painting.
 */
void frame_start(void) {
  int n, old_clock;

#ifdef DEBUG
  int profile_clock;
#endif

  ascii = 0;
  scan_code = 0;
  poll_keyboard();

  // Pause while app has lost focus (alt-tab, minimize)
  while (app_paused && !alt_x) {
    SDL_Delay(50);
    poll_keyboard();
  }

  // Control screensaver

  if (ss_status && ss_frame != NULL) {
    if (get_reloj() > ss_time_counter) {
      if (ss_init != NULL)
        ss_init();

      ss_exit = 0;

      do {
        key_check = 0;

        for (n = 0; n < 128; n++)
          if (key(n))
            key_check++;

        if (key_check != last_key_check)
          ss_exit = 1;

        readmouse();

        mou_check = mouse->x + mouse->y + mouse->left + mouse->right + mouse->middle;

        if (mou_check != last_mou_check)
          ss_exit = 1;

        if (joy_status) {
          read_joy();
          joy_check = joy->button1 + joy->button2 + joy->left + joy->right + joy->up + joy->down;

          if (joy_check != last_joy_check)
            ss_exit = 3;
        }

        ss_frame();
        full_redraw = 1;
        if (buffer_to_video != NULL)
          buffer_to_video();
        else
          blit_screen((byte *)screen_buffer);
      } while (!ss_exit);

      if (ss_end != NULL)
        ss_end();

      memcpy(screen_buffer, back_buffer, vga_width * vga_height);
      blit_partial(0, 0, vga_width, vga_height);
      ss_time_counter = get_reloj() + ss_time;
    }
  }

#ifdef DEBUG
  profile_clock = get_ticks();
#endif

  // Eliminate dead processes

  for (ide = id_start; ide <= id_end; ide += iloc_len)
    if (mem[ide + _Status] == 1)
      kill_process(ide);

#ifdef DEBUG
  function_exec(255, get_ticks() - profile_clock);
#endif

#ifdef DEBUG
  profile_clock = get_ticks();
#endif

  for (max = 0; max < 10; max++) {
    if (otimer[max] != timer(max)) {
      mtimer[max] = timer(max) * 10;
    }
    mtimer[max] += (get_reloj() - last_clock);
    timer(max) = mtimer[max] / 10;
    otimer[max] = timer(max);
  }


  if (get_reloj() > last_clock) {
    ffps = (ffps * 9.0f + 1000.0f / (float)(get_reloj() - last_clock)) / 10.0f;
    fps = (int)(ffps + 0.5f);
  }

  last_clock = get_reloj();

#ifdef DEBUG
  if (overall_clock) {
    game_ticks += (double)(get_ticks() - overall_clock);
    game_frames += 1;

    if (ffps2 > 0)
      ffps2 = (ffps2 * 3.0f + (double)4600.0 / (game_ticks / game_frames)) / 4.0f;
    else
      ffps2 = (double)4600.0 / (game_ticks / game_frames);
  }
  function_exec(255, get_ticks() - profile_clock);
#endif

  if (get_reloj() > (fractional_clock +
                     clock_interval / 3)) { // Allow consuming up to one third of the next frame
    if (blits_skipped < max_frame_skips) {
      blits_skipped++;
      skip_blit = 1;
      fractional_clock += clock_interval;
    } else {
      fractional_clock = (float)get_reloj() + clock_interval;
      blits_skipped = 0;
      skip_blit = 0;
    }
  } else {
    n = 0;
    old_clock = get_reloj();

#ifndef EMSCRIPTEN
    if (old_clock < (int)fractional_clock) {
      do {
#ifdef WIN32
        SDL_Delay(((int)fractional_clock - old_clock) - 1);
#else
        sched_yield();
#endif
      } while (get_reloj() < (int)fractional_clock); // TO keep FPS
    }
#else
    do {
      retrace_wait();
    } while (get_reloj() < (int)fractional_clock);
#endif
    blits_skipped = 0;
    skip_blit = 0;
    fractional_clock += clock_interval;
  }

#ifdef DEBUG
  overall_clock = profile_clock = get_ticks();
#endif

  // Mark all proceeses as "not executed"

  for (ide = id_start; ide <= id_end; ide += iloc_len)
    mem[ide + _Executed] = 0;

  // Set maximum priority for executing processes by _Priority

  id_old = id_start; // Next process to execute

  // Position mouse variables

  readmouse();

  // Read joystick(s)

  if (joy_status == 1 && joy_timeout >= 6) {
    joy->button1 = 0;
    joy->button2 = 0;
    joy->button3 = 0;
    joy->button4 = 0;
    joy->left = 0;
    joy->right = 0;
    joy->up = 0;
    joy->down = 0;
    joy_status = 0;
  } else if (joy_status) {
    read_joy();
    joy_check = joy->button1 + joy->button2 + joy->left + joy->right + joy->up + joy->down;

    if (joy_check != last_joy_check) {
      last_joy_check = joy_check;
      ss_time_counter = get_reloj() + ss_time;
    }
  }

  key_check = 0;
  for (n = 0; n < 128; n++)
    if (key(n))
      key_check++;

  if (key_check != last_key_check) {
    last_key_check = key_check;
    ss_time_counter = get_reloj() + ss_time;
  }

  mou_check = mouse->x + mouse->y + mouse->left + mouse->right + mouse->middle;

  if (mou_check != last_mou_check) {
    last_mou_check = mou_check;
    ss_time_counter = get_reloj() + ss_time;
  }

#ifdef DEBUG
  function_exec(255, get_ticks() - profile_clock);
#endif
}

//-----------------------------------------------------------------------------
// Ends a frame and print graphics
//-----------------------------------------------------------------------------

void frame_end(void) {
  int mouse_drawn = 0, texts_drawn = 0, drawings_drawn = 0;
  int mouse_x0 = 0, mouse_x1 = 0, mouse_y0 = 0, mouse_y1 = 0;
  int n = 0, mode7_id, scroll_id, otheride, retrace_pending = 0;
  char buf[255];

#ifdef DEBUG
  int profile_clock;
#endif

#ifdef __EMSCRIPTEN__
  div_snprintf(buf, sizeof(buf), "$('#fps').text(\"FPS: %d/%d (max frameskip: %d)\");", fps, dfps,
               max_frame_skips);
  emscripten_run_script(buf);
#endif

  // If the user modified mouse.x or mouse.y, reposition the mouse accordingly

  if (!skip_blit) {
    // Restore framebuffer regions outside scroll/mode7 areas from the clean backup

#ifdef DEBUG
    profile_clock = get_ticks();
#endif

    if (restore_type == 0 || restore_type == 1) {
      if (!iscroll[0].on || iscroll[0].x || iscroll[0].y || iscroll[0].w != vga_width ||
          iscroll[0].h != vga_height) {
        if (background_to_buffer != NULL)
          background_to_buffer();
        else {
          if (old_restore_type == 0)
            restore((byte *)screen_buffer, (byte *)back_buffer);
          else
            memcpy(screen_buffer, back_buffer, vga_width * vga_height);
        }
      }
    }

    if (pre_process_buffer != NULL)
      pre_process_buffer();

#ifdef DEBUG
    if (debugger_step) {
      function_exec(253, restore_time);
      game_ticks -= get_ticks() - profile_clock;
      game_ticks += restore_time;
    } else {
      n = get_ticks() - profile_clock;
      function_exec(253, n);

      if (!restore_time)
        restore_time = n;
      else
        restore_time = (restore_time * 3 + n) / 4;
    }
    profile_clock = get_ticks();
#endif

#ifdef DEBUG
    if (n) {
      function_exec(251, get_ticks() - profile_clock);
      profile_clock = get_ticks();
    }
#endif

    // Render sprites sorted by _Z (higher Z is rendered first/behind)

    for (ide = id_start; ide <= id_end; ide += iloc_len) {
      mem[ide + _Executed] = 0; // Not executed
      mem[ide + _x1] = -1;      // No blit_screen region
    }

    for (n = 0; n < 10; n++) {
      im7[n].painted = 0;
      iscroll[n].painted = 0;
    }

#ifdef DEBUG
    function_exec(255, get_ticks() - profile_clock);
#endif

    do {
#ifdef DEBUG
      profile_clock = get_ticks();
#endif

      ide = 0;
      mode7_id = 0;
      scroll_id = 0;
      otheride = 0;
      max = 0x80000000;

      for (id = id_start; id <= id_end; id += iloc_len) {
        if ((mem[id + _Status] == 2 || mem[id + _Status] == 4) && mem[id + _Ctype] == 0 &&
            !mem[id + _Executed] && mem[id + _Z] > max) {
          ide = id;
          max = mem[id + _Z];
        }
      }

      for (n = 0; n < 10; n++) {
        if (im7[n].on && (m7 + n)->z >= max && !im7[n].painted) {
          mode7_id = n + 1;
          max = (m7 + n)->z;
        }
      }

      for (n = 0; n < 10; n++) {
        if (iscroll[n].on && (scroll + n)->z >= max && !iscroll[n].painted) {
          scroll_id = n + 1;
          max = (scroll + n)->z;
        }
      }

      if (text_z >= max && !texts_drawn) {
        max = text_z;
        otheride = 1;
      }

      if (mouse->z >= max && !mouse_drawn) {
        max = mouse->z;
        otheride = 2;
      }

      if (draw_z >= max && !drawings_drawn) {
        max = draw_z;
        otheride = 3;
      }

      if (otheride) {
        if (otheride == 1) {
          for (n = 0; n < MAX_TEXTS; n++)
            if (texts[n].font)
              break;

          if (n < MAX_TEXTS) {
            memb[nullstring[0] * 4] = 0; // The "floating" text is never shown
            memb[nullstring[1] * 4] = 0;
            memb[nullstring[2] * 4] = 0;
            memb[nullstring[3] * 4] = 0;
            paint_texts(0);
#ifdef DEBUG
            function_exec(250, get_ticks() - profile_clock);
#endif
          }
          texts_drawn = 1;
        } else if (otheride == 2) {
          readmouse();
          x1s = -1;
          v_function = -1; // No errors (don't show?)
          put_sprite(mouse->file, mouse->graph, mouse->x, mouse->y, mouse->angle, mouse->size,
                     mouse->flags, mouse->region, screen_buffer, vga_width, vga_height);
          mouse_x0 = x0s;
          mouse_x1 = x1s;
          mouse_y0 = y0s;
          mouse_y1 = y1s;
          mouse_drawn = 1;
#ifdef DEBUG
          function_exec(255, get_ticks() - profile_clock);
#endif
        } else if (otheride == 3) {
          for (n = 0; n < max_drawings; n++)
            if (drawing[n].type)
              break;

          if (n < max_drawings) {
            paint_drawings();
#ifdef DEBUG
            function_exec(250, get_ticks() - profile_clock);
#endif
          }
          drawings_drawn = 1;
        }
      } else if (scroll_id) {
        iscroll[snum = scroll_id - 1].painted = 1;

        if (iscroll[snum].on == 1)
          scroll_simple();
        else if (iscroll[snum].on == 2)
          scroll_parallax();
      } else if (mode7_id) {
        paint_m7(mode7_id - 1);
        im7[mode7_id - 1].painted = 1;
      } else if (ide) {
        if (mem[ide + _Graph] > 0 || mem[ide + _XGraph] > 0) {
          paint_sprite();
#ifdef DEBUG
          process_paint(ide, get_ticks() - profile_clock);
#endif
        }
        mem[ide + _Executed] = 1;
      }
    } while (ide || mode7_id || scroll_id || otheride);

    if (demo)
      paint_texts(MAX_TEXTS);

    if (post_process_buffer != NULL)
      post_process_buffer();

    // If a fade is in progress, continue it

    if (now_dacout_r != dacout_r || now_dacout_g != dacout_g || now_dacout_b != dacout_b) {
      update_palette();
      set_dac();
      fading = 1;
      retrace_pending = 1;
    } else {
      if (apply_palette_flag) {
        update_palette();
        set_dac();
        retrace_pending = 1;
        apply_palette_flag--;
      }
      fading = 0;
    }

#ifdef DEBUG
    profile_clock = get_ticks();

    if (debugger_step) {
      function_exec(254, blit_time);
      game_ticks += blit_time;
    } else {
#endif

      if (!retrace_pending && vsync)
        retrace_wait();

      if (buffer_to_video != NULL) {
        buffer_to_video();
      } else {
        if (old_dump_type) {
          full_redraw = 1;
          blit_screen((byte *)screen_buffer);
        } else {
          full_redraw = 0;

          // Add this frame's blit regions to the previous frame's restore list

          for (n = id_start; n <= id_end; n += iloc_len)
            if (mem[n + _x1] != -1)
              blit_partial(mem[n + _x0], mem[n + _y0], mem[n + _x1] - mem[n + _x0] + 1,
                           mem[n + _y1] - mem[n + _y0] + 1);
          for (n = 0; n < 10; n++) {
            if (im7[n].on)
              blit_partial(im7[n].x, im7[n].y, im7[n].w, im7[n].h);

            if (iscroll[n].on)
              blit_partial(iscroll[n].x, iscroll[n].y, iscroll[n].w, iscroll[n].h);
          }

          if (mouse_x1 != -1)
            blit_partial(mouse_x0, mouse_y0, mouse_x1 - mouse_x0 + 1, mouse_y1 - mouse_y0 + 1);

          for (n = 0; n < MAX_TEXTS; n++)
            if (texts[n].font && texts[n].w)
              blit_partial(texts[n].x0, texts[n].y0, texts[n].w, texts[n].h);

          // Perform a partial blit_screen

          blit_screen((byte *)screen_buffer);
        }

        if (dump_type == 0 || restore_type == 0) { // Set up restore regions for the next frame

          for (n = id_start; n <= id_end; n += iloc_len)
            if (mem[n + _x1] != -1)
              blit_partial(mem[n + _x0], mem[n + _y0], mem[n + _x1] - mem[n + _x0] + 1,
                           mem[n + _y1] - mem[n + _y0] + 1);
          for (n = 0; n < 10; n++) {
            if (im7[n].on)
              blit_partial(im7[n].x, im7[n].y, im7[n].w, im7[n].h);

            if (iscroll[n].on)
              blit_partial(iscroll[n].x, iscroll[n].y, iscroll[n].w, iscroll[n].h);
          }
          if (mouse_x1 != -1)
            blit_partial(mouse_x0, mouse_y0, mouse_x1 - mouse_x0 + 1, mouse_y1 - mouse_y0 + 1);
          for (n = 0; n < MAX_TEXTS + 1; n++)
            if (texts[n].font && texts[n].w)
              blit_partial(texts[n].x0, texts[n].y0, texts[n].w, texts[n].h);
        }
      }

#ifdef DEBUG
      n = get_ticks() - profile_clock;
      function_exec(254, n);

      if (!blit_time)
        blit_time = n;
      else
        blit_time = (blit_time * 3 + n) / 4;
    }
#endif

  } // skip blit_screen
}

///////////////////////////////////////////////////////////////////////////////
// Remove a process
///////////////////////////////////////////////////////////////////////////////

void kill_process(int id) {
  int id2;

#ifdef LLPROC
  remove_process(id);
#endif

  mem[id + _Status] = 0;
  process_count--;

  if ((id2 = mem[id + _Father])) {
    if (mem[id2 + _Son] == id)
      mem[id2 + _Son] = mem[id + _BigBro];
    if (mem[id + _FCount] > 0 && mem[id2 + _Status] == 3) {
      mem[id2 + _Executed] = 0;
      mem[id2 + _Status] = 2;
    }
  }

  if ((id2 = mem[id + _BigBro]))
    mem[id2 + _SmallBro] = mem[id + _SmallBro];
  if ((id2 = mem[id + _SmallBro]))
    mem[id2 + _BigBro] = mem[id + _BigBro];
  if ((id2 = mem[id + _Son])) {
    do {
      mem[id2 + _Father] = id_init;
      mem[id2 + _Caller] = 0;

      if (mem[id2 + _BigBro] == 0) {
        if (mem[id_init + _Son] != 0) {
          mem[id2 + _BigBro] = mem[id_init + _Son];
          mem[mem[id_init + _Son] + _SmallBro] = id2;
        }
        id2 = 0;
      } else
        id2 = mem[id2 + _BigBro];
    } while (id2);

    mem[id_init + _Son] = mem[id + _Son];
  }

  while (mem[id_end + _Status] == 0 && id_end > id_start) {
    id_end -= iloc_len;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Finalise
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void closefiles(void); // close fake fmemopen'd files
#endif

void finalization(void) {
  int newmapcount = 0;
  int snum = 0;

  dacout_r = 64;
  dacout_g = 64;
  dacout_b = 64;
  dacout_speed = 4;
  fade_wait();

  reset_video_mode();

  kbd_reset();

#ifdef DEBUG
  if (text_font != NULL) {
    free(text_font);
  }
  if (graf_ptr != NULL) {
    free(graf_ptr);
  }
#endif

  // free any new_map ptrs if any
  for (newmapcount = 1000; newmapcount < 2000; newmapcount++) {
    if (g[0].grf[newmapcount] != 0) {
      free((byte *)(g[0].grf[newmapcount]) - 1330);
      g[0].grf[newmapcount] = 0;
    }
  }
  // Free fpg 0
  free(g[0].grf);

  // Free screen ram
  free(screen_buffer);
  free(back_buffer);

  // Free div mem
  free(mem);

  // Free scroll mem
  for (snum = 0; snum < 10; snum++) {
    if (iscroll[snum]._sscr1 != 0)
      free(iscroll[snum]._sscr1);
    if (iscroll[snum]._sscr2 != 0)
      free(iscroll[snum]._sscr2);
    if (iscroll[snum].fast != 0)
      free(iscroll[snum].fast);
  }

  // Stop sounds
  for (snum = 0; snum < 128; snum++) {
    sound_unload(snum);
  }

  // Free ghost table
  free(ghost_inicial);

  // Free quads
  free(color_lookup);

  // Free system font
  free(fonts[0]);
  free(sys06x08);
#ifdef MIXER
  Mix_CloseAudio();
  Mix_Quit();
#endif

#ifdef DEBUG
  // Free debug window
  end_debug();
  free(screen_buffer_debug);
#endif

#ifdef WIN32

  closefiles();

  OSDEP_Quit();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Runtime error
///////////////////////////////////////////////////////////////////////////////

void exer(int e) {
#ifdef DEBUG

  FILE *f;

  if (e) {
    if ((f = fopen("system/exec.err", "wb")) != NULL) {
      fwrite(&e, 4, 1, f);
      fclose(f);
    }
  }

#else

  if (e) {
    printf("Error: ");

    switch (e) {
    case 1:
      fprintf(stdout, "Out of memory!");
      break;
    case 2:
      fprintf(stdout, "Too many process!");
      break;
    case 3:
      fprintf(stdout, "Stack overflow!");
      break;
    case 4:
      fprintf(stdout, "DLL not found!");
      break;
    case 5:
      fprintf(stdout, "System font file missed!");
      break;
    case 6:
      fprintf(stdout, "System graphic file missed!");
      break;
    default:
      fprintf(stdout, "Internal error!");
      break;
    }
  }

#endif


  reset_video_mode();

  kbd_reset();

#if defined(DOS) || defined(WIN32)
  _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
#endif

  chdir(divpath);

#ifdef STDOUTLOG
  printf("exited %d\n", e);
#endif

#ifdef WIN32
  closefiles();
#endif

#ifdef EMSCRIPTEN
  emscripten_cancel_main_loop();
#endif

  exit(26);
}

//////////////////////////////////////////////////////////////////////////////
//      Non-critical error messages (non debug versions)
//////////////////////////////////////////////////////////////////////////////

#ifndef DEBUG

void e(int text_id) {
  int n = 0;

  if (v_function == -1)
    return;

  while (n < num_skipped) {
    if (skipped[n] == text_id)
      break;
    n++;
  }

  if (ignore_errors || n < num_skipped)
    return;

  if (v_function >= 0) {
    printf("Error %d (%s) %s\nn", text_id, fname[v_function], text[text_id]);
  } else {
    printf("Error %d %s\n\n", text_id, text[text_id]);
  }
  reset_video_mode();

  kbd_reset();

#if defined(DOS) || defined(WIN32)
  _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
#endif
  chdir(divpath);

  exit(26);
}

#endif

//////////////////////////////////////////////////////////////////////////////
//  Main Program
//////////////////////////////////////////////////////////////////////////////

#ifdef ZLIB
extern int datastartpos;
extern char exebin[255];
#endif


int main(int argc, char *argv[]) {
  FILE *f, *fsf;
  int a = 0;
  byte *ptr;
  byte *dp;
  char DIV_VER = ' ';
  char *jschar;

  char buf[255];

  unsigned long len, len_descomp;
  int mimem[10], n, i;
  uint32_t stubsize = 602;
  uint32_t winstubsize = 11984;
  uint32_t div1stubsize = 8819;
  uint32_t exesize = 0;
  uint32_t datsize = 0;
  uint32_t exestart = 0;
  uint32_t datstart = 0;

  screen_buffer = NULL;
  back_buffer = NULL;

#ifdef DEBUG
  screen_buffer_debug = NULL;
#endif

#ifndef EMSCRIPTEN
  atexit(OSDEP_Quit);
#endif

  OSDEP_Init();

  remove("DEBUGSRC.TXT");

  getcwd(divpath, PATH_MAX + 1);


  if (true) {
    f = fopen(argv[0], "rb");

    memset(buf, 0, 55);

    if (f) {
      fseek(f, -2, SEEK_END);
      fread(buf, 1, 2, f);
      buf[2] = 0;

      if (!strcmp(buf, "DX")) {
        fseek(f, -10, SEEK_END);
        fread(&exesize, 4, 1, f);
        fread(&datsize, 4, 1, f);
      }
      if (exesize == 0)
        fclose(f);
    }
  }


#ifndef DEBUG
#ifndef __EMSCRIPTEN__
#ifndef DROID
  if (argc < 2 && exesize == 0) {
    printf("DIV2015 Run time library - version 2.02a - http://div-arena.co.uk\n");
    printf("Error: Needs a DIV executable to load.\n");

    _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
    chdir(divpath);

    exit(26);
  }
#endif
#endif
#endif
  vga_width = argc; // To remove a warning (argc unused?)


  vga_width = 320;
  vga_height = 200;
  clock_interval = 1000.0 / 24.0; // 24 fps
  max_frame_skips = 0;            // 0 skips

#ifdef __EMSCRIPTEN__

  max_frame_skips = 0;


  f = fopen(HTML_EXE, "rb");
  printf("FILE: %s %x\n", HTML_EXE, f);

#else

#ifdef DROID

  if (argc > 1 && exesize == 0) {
    if ((f = fopen(argv[1], "rb")) == NULL) {
#ifndef DEBUG
      printf("Error: Needs a DIV executable to load.\n");
#endif

      _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
      chdir(divpath);

      exit(26);
    }
  }
//#endif
#endif

  if (argc == 1) {
    chdir("resources");
    f = fopen("EXEC.EXE", "rb");
  } else {
    f = fopen(argv[1], "rb");
  }

  if (exesize > 0) {
    if (f)
      fclose(f);
    f = fopen(argv[0], "rb");
  }

  if (!f) {

#ifndef DEBUG
    printf("Error: Needs a DIV executable to load!\n");
#endif
    chdir(divpath);
    exit(26);
  }
#endif

#ifdef DEBUG
  // check for full screen
  fsf = fopen("system/exec.fs", "rb");

  if (fsf) {
    fread(&fsmode, 1, 1, fsf);
    fclose(fsf);
  }

  fsf = fopen("system/exec.path", "rb");
  if (fsf) {
    fgets(prgpath, _MAX_PATH, fsf);
    fclose(fsf);
  }
  initialize_texts((byte *)"system/lenguaje.int");
#else
  initialize_texts((byte *)argv[0]);
#endif

  // check if div1 or div2 exe
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  if (exesize > 0)
    exestart = len - exesize - datsize - 10;

#ifdef ZLIB
  if (datsize > 0) {
    datastartpos = exestart + exesize;
    div_strcpy(exebin, sizeof(exebin), argv[0]);
  }
#endif

  fseek(f, 0x2 + exestart, SEEK_SET);
  fread(&DIV_VER, 1, 1, f);
  switch (DIV_VER) {
  case 'j':
    break;
  case 's':
    fclose(f);
    exit(26);
    break;
  case 144:
    stubsize = winstubsize;

    break;
  default:
    printf("Unknown div code, aborting\n");
    fclose(f);
    exit(26);
    break;
  }
  if (DIV_VER != 'D')
    len -= stubsize - 4 * 10;

  fseek(f, 0 + exestart, SEEK_SET);

  if (DIV_VER == 'D') {
    printf("Cannot load DIV1 exe (yet!)\n");
    fclose(f);
    exit(26);

    fseek(f, div1stubsize, SEEK_SET);
    fread(mimem, 4, 10, f);

#ifdef PRGDUMP
    for (a = 0; a < 10; a++) {}
#endif
    iloc_len = (mimem[5] + mimem[6]);

    if (iloc_len & 1)
      iloc_len++;

    if (mimem[3] > 0) {
      imem_max = mimem[8] + mimem[3] * (iloc_len) + iloc_len + 2;
    } else {
      imem_max = mimem[8] + 128 * (iloc_len) + iloc_len + 2;
      if (imem_max < 256 * 1024)
        imem_max = 256 * 1024;
      if (imem_max > 1024 * 1024)
        imem_max = 1024 * 1024;
    }

    dp = (byte *)malloc(len);
    mem = (int *)malloc(4 * len); //imem_max+1032*5+16*1025+3);
    mem = (int *)((((uintptr_t)mem + 3) / 4) * 4);
    memset(mem, 0, ((((uintptr_t)mem + 3) / 4) * 4));
    memb = (byte *)mem;
    memw = (word *)mem;
    fseek(f, div1stubsize, SEEK_SET);
    fread(mem, 1, len - div1stubsize, f);

#ifdef DUMP_PRG
    dump(len - div1stubsize);
#endif

    kbd_init();


    interpreter();
  } else {
    fseek(f, stubsize + exestart, SEEK_SET);
    fread(mimem, 4, 10, f);

    iloc_len = (mimem[5] + mimem[6]);

    if (iloc_len & 1)
      iloc_len++;

#ifdef __EMSCRIPTEN__
#define MEM_MULTI 2
#else
#define MEM_MULTI 2
#endif

    if (mimem[3] > 0) {
      imem_max = mimem[8] + mimem[3] * (iloc_len) + iloc_len + 2;
    } else {
      imem_max = mimem[8] + 128 * (iloc_len) + iloc_len + 2;
      if (imem_max < MEM_MULTI * 256 * 1024)
        imem_max = MEM_MULTI * 256 * 1024;
      if (imem_max > MEM_MULTI * 1024 * 1024)
        imem_max = MEM_MULTI * 1024 * 1024;
    }


    if ((mem = (int *)malloc(4 * imem_max + 1032 * 5 + 16 * 1025 + 3)) != NULL) {
      mem = (int *)((((uintptr_t)mem + 3) / 4) * 4);

      filenames = (char *)&mem[imem_max + 258 * 5]; // Buffer of 16*1025 for dirinfo[].name

      memset(mem, 0, 4 * imem_max + 1032 * 5);
      // To add strings (on the fly?) "in the air"
      nullstring[0] = imem_max + 1 + 258 * 0;
      mem[nullstring[0] - 1] = 0xDAD00402;
      nullstring[1] = imem_max + 1 + 258 * 1;
      mem[nullstring[1] - 1] = 0xDAD00402;
      nullstring[2] = imem_max + 1 + 258 * 2;
      mem[nullstring[2] - 1] = 0xDAD00402;
      nullstring[3] = imem_max + 1 + 258 * 3;
      mem[nullstring[3] - 1] = 0xDAD00402;
      memcpy(mem, mimem, 40);

      if ((ptr = (byte *)malloc(len)) != NULL) {
        fread(ptr, 1, len, f);
        fclose(f);

        len_descomp = mem[9];
#ifdef ZLIB
        if (!uncompress((unsigned char *)&mem[9], &len_descomp, ptr, len))
#else
        if (false)
#endif
        {
#ifdef DUMP_BYTECODE
          FILE *m = fopen("exec.m", "wb");
          if (m) {
            fwrite((uint32_t *)mem, 1, len_descomp + 80, m);
            fclose(m);
          }
#endif
          free(ptr);

          if ((mem[0] & 128) == 128) {
            trace_program = 1;
            mem[0] -= 128;
          }
          if ((mem[0] & 512) == 512) {
            ignore_errors = 1;
            mem[0] -= 512;
          }
          demo = 0;
          if ((mem[0] & 1024) == 1024) {
#ifndef DEBUG
            demo = 1;
#endif
            mem[0] -= 1024;
          }

          i = imem_max + 258 * 4;
          if ((_argc = argc - 1) > 10)
            _argc = 10;
          for (n = 0; n < _argc && n < argc - 1; n++) {
            memcpy(&mem[i], argv[n + 1], strlen(argv[n + 1]) + 1);
            _argv(n) = i;
            i += (strlen(argv[n + 1]) + 4) / 4;
          }
          for (; n < 10; n++)
            _argv(n) = 0;

          memb = (byte *)mem;
          memw = (word *)mem;

          if (mem[0] != 0 && mem[0] != 1) {
#ifndef DEBUG
            printf("Error: Needs a DIV executable to load.\n");
#endif

            _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
            chdir(divpath);

            exit(26);
          }

          kbd_init();
#ifdef DEBUG
          printf("Looking for joysticks\n");
#endif
          if (OSDEP_NumJoysticks() > 0) {
            divjoy = (OSDEP_Joystick *)OSDEP_JoystickOpen(0);
            joy_status = 1;

#ifdef DEBUG
            printf("Joyname:    %s\n", OSDEP_JoystickName(0));
            printf("NumAxes: %d: Numhats: %d : NumButtons: %d\n", OSDEP_JoystickNumAxes(0),
                   OSDEP_JoystickNumHats(0), OSDEP_JoystickNumButtons(0));
#endif
            if (OSDEP_JoystickNumHats(0) == 0) {
              OSDEP_JoystickClose(divjoy);
              divjoy = NULL;
            }

          } else {
            joy_status = 0;
          }


          interpreter();
#ifndef __EMSCRIPTEN__
          _dos_setdrive((int)toupper(*divpath) - 'A' + 1, &divnum);
          chdir(divpath);

          exit(26); // Exit without clearing screen
#endif
        } else {
          free(ptr);
          exer(1);
        }

      } else {
        fclose(f);
        exer(1);
      }

    } else {
      fclose(f);
      exer(1);
    }
  }
  return 0;
}

int is_pak(FILE *f, char *name) {
  char head[8];
  int n, nfiles;
  int prg_id = 0;
  int id[3];
  char buf[255];

  fread(head, 1, 8, f);
  fread(id, 4, 3, f);
  fread(&nfiles, 4, 1, f);

  if (!strcmp(head, "dat\x1a\x0d\x0a") && nfiles > 0) {
    if (prg_id == id[0] || prg_id == id[1] || prg_id == id[2]) {
      packdir = (struct _packdir *)malloc(nfiles * sizeof(struct _packdir));
      if (packdir != NULL) {
        if (fread(packdir, sizeof(struct _packdir), nfiles, f) == nfiles) {
          for (n = 0; n < nfiles; n++) {
            div_strcpy(buf, sizeof(buf), packdir[n].filename);
          }

          div_strcpy(packfile, sizeof(packfile), name);
          npackfiles = nfiles;
        }
      }

      return 1;
    }
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  Search the packfile (for installed games)
//////////////////////////////////////////////////////////////////////////////

void find_packfile(void) {
#ifdef ZLIB
#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

  unzFile *zip = NULL;
  unz_file_info fileInfo;
  char szCurrentFileName[UNZ_MAXFILENAMEINZIP + 1];
#endif

  FILE *f;
  int n, m;
  struct find_t fileinfo;
  int prg_id = 0;

  for (n = 0; n < 9; n++) {
    prg_id <<= 1;
    prg_id ^= mem[n];
  }

  m = _dos_findfirst("*.*", _A_NORMAL, &fileinfo);
  while (m == 0) {
    f = fopen(fileinfo.name, "rb");
    if (f != NULL) {
      if (is_pak(f, fileinfo.name)) {
        fclose(f);
        break;
      }
      fclose(f);
    }
    m = _dos_findnext(&fileinfo);
  }

#ifdef ZLIB
  // look for paks in embedded zip
  if (datastartpos > 0) {
    zip = unzOpen(exebin);
    fprintf(stdout, "%x\n", (unsigned int)(uintptr_t)zip);
    // found zip. search for .pak files
    if (unzGoToFirstFile(zip) == UNZ_OK) {
      do {
        if (unzGetCurrentFileInfo(zip, &fileInfo, szCurrentFileName, sizeof(szCurrentFileName) - 1,
                                  NULL, 0, NULL, 0) == UNZ_OK) {
          fprintf(stdout, "found file: %s\n", szCurrentFileName);
          f = memz_open_file((unsigned char *)szCurrentFileName);
          fprintf(stdout, "TESTING %s\n", szCurrentFileName);
          if (f) {
            if (is_pak(f, szCurrentFileName)) {
              fprintf(stdout, "FOUND PAK IN ZIP %s\n", packfile);
            }
            fclose(f);
          }
        }
      } while (unzGoToNextFile(zip) != UNZ_END_OF_LIST_OF_FILE);
    }
    unzClose(zip);
  }
#endif
}

//////////////////////////////////////////////////////////////////////////////
//  Dumps information to a file
//////////////////////////////////////////////////////////////////////////////

void debug_info(char *msg) {
  FILE *f;
  if ((f = fopen("DEBUGSRC.TXT", "a")) != NULL) {
    fprintf(f, "%s\n", msg);
    fclose(f);
  }
}

void debug_data(int val) {
  FILE *f;
  if ((f = fopen("DEBUGSRC.TXT", "a")) != NULL) {
    fprintf(f, "%d\n", val);
    fclose(f);
  }
}

void GetFree4kBlocks(void) {}

//////////////////////////////////////////////////////////////////////////////
