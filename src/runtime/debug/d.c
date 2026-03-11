
//----------------------------------------------------------------------------
//      Debugger functions and error dialogs
//      Note: trace_process() is defined in i.c, not here.
//----------------------------------------------------------------------------

#include "../inter.h"
#include "../../div_string.h"

//--------------------------------------------------------------------------

int debug_active = 1; // If set to 0, the debugger and its windows
                      // can never be invoked

//--------------------------------------------------------------------------

void err0(void);
void process_list0(void);
void profile0(void);
byte *get_offset_byte(int m);
word *get_offset_word(int m);
void flush_window(int m);
void wrectangle(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
void wput(byte *dest, int dest_width, int dest_height, int x, int y, int n);
void wbox(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
int text_len(byte *ptr);
void wwrite_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x_org,
                   int y_org, int centro_org, byte *ptr, byte c);

void wwrite(byte *dest, int dest_width, int dest_height, int x, int y, int centro, byte *ptr,
            byte c);

void dread_mouse(void);
void explode(int x, int y, int w, int h);
void blit_region(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w, int h,
              int salta);

void modal_loop(void);
int mouse_in(int x, int y, int x2, int y2);
void move_window(void);
void close_window(void);
void flush_copy(void);
void update_box(int x, int y, int w, int h);
void implode(int x, int y, int w, int h);
int collides_with(int a, int x, int y, int w, int h);

void blit_region_dark(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w,
                     int h, int salta);

void wbox_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, byte c, int x, int y,
                 int w, int h);
void wput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n);
void bwput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n);

void wtexc(byte *dest, int dest_pitch, int dest_width, int dest_height, byte *p, int x, int y,
           byte w, int h, byte c);

void wtexn(byte *dest, int dest_pitch, byte *p, int x, int y, byte w, int h, byte c);
void restore_wallpaper(int x, int y, int w, int h);

void show_button(t_item *i);
void show_get(t_item *i);
void show_flag(t_item *i);
void select_button(t_item *i, int activo);

void select_get(t_item *i, int activo, int ocultar_error);
void _select_new_item(int i);
int button_status(int n);

void process_button(int n, int e);

int get_status(int n);
void process_get(int n, int e);

int flag_status(int n);

void process_flag(int n, int e);
int wmouse_in(int x, int y, int w, int h);
void get_input(int n);

void save_mouse_bg(byte *p, int x, int y, int n, int flag);
void process_graph(int id, byte *q, int van, int w, int h);

void paint_segment(void);
void include_members(int padre, int nivel, int index);
void paint_var_list(void);
void paint_segment2(void);
int get_offset(int m);
int memo(int dir);
void visualize(int valor, int object, char *str);
int _get_offset(int m);
void determine_code(void);
void f_down(void);
void paint_code(void);
void f_up(void);
void f_home(void);
void f_right(void);
void f_left(void);
int get_ip(int n);
void get_line(int n);
void paint_process_segment(void);
void paint_profile_segment(void);


#define max_procesos 2048

char combo_error[128]; // buffer for composing compound error messages

extern int process_level; // Tracks cal/ret nesting depth (for step)

extern float m_x, m_y;
extern float ffps2;
extern double game_ticks, game_frames;

int no_volcar_nada = 0;
int profiler_x, profiler_y;

int show_items_called = 0;
int get_pos = 0, get_cursor = 0; // Clock and cursor position in get fields
int superget = 0;

//--------------------------------------------------------------------------

int linea0;    // First line number in the debugger code window
byte *plinea0; // Pointer to the first line in the debugger code window

int mem1, mem2;                         // Bounds of the current statement in the mem[] array
int linea1, columna1, linea2, columna2; // Bounds of the current statement

int linea_sel; // Selected line number

int x_inicio = 54; // Initial x offset in the source code window

//--------------------------------------------------------------------------

int smouse_x, smouse_y, mouse_x = 0, mouse_y = 0, mouse_b;

int reloj_debug;
int ticks_debug;

//----------------------------------------------------------------------------

int boton = 0;
int ids_old = 0, ids_inc;

void exec_process(void);
void trace_process(void);
void inspect0(void);
void debug0(void);

byte *source = NULL, *end_source = NULL;

static int *line = NULL;
int num_sentencias;

//----------------------------------------------------------------------------
//      Debugger variables
//----------------------------------------------------------------------------

clock_t system_clock; // = clock();//(void*) 0x46c; // System clock

char get[256];

char *vnom = NULL; // Name vector (symbol name table)

#define tnone 0 // Object types in obj[]
#define tcons 1
#define tvglo 2
#define ttglo 3
#define tcglo 4
#define tvloc 5
#define ttloc 6
#define tcloc 7
#define tproc 8
#define tfunc 9
#define tsglo 10
#define tsloc 11
#define tfext 12 // External library function

#define tbglo 13 // Global byte
#define twglo 14
#define tbloc 15
#define twloc 16

#define tpigl 17 // Global int pointer (addressable ttglo)
#define tpilo 18 // Local int pointer (same)

#define tpwgl 19 // Word pointers
#define tpwlo 20
#define tpbgl 21 // Byte pointers
#define tpblo 22

#define tpcgl 23 // String pointers
#define tpclo 24
#define tpsgl 25 // Struct pointers
#define tpslo 26

struct object {
  int type;
  int name;
  int scope;
  int miembro;
  int v0, v1, v2, v3, v4, v5;
} *o = NULL;

int num_objects = 0; // Number of objects in the object table

int num_predefined;

int *usado; // Marks which objects have been included in var[]
int *visor; // Display mode used for each object

struct variables {
  int object;  // Object index in o[]
  int tab;     // Indentation (0=no member, 1, 2, ...)
  int miembro; // Member of ..., as index into var[]
  int indice;  // For arrays or structs, the displayed element index
} *var = NULL;

int num_var = 0; // Number of variables included in var[]

int var_ini;    // First variable displayed in the window
int var_select; // Selected variable

int show_const = 0, show_global = 0, show_local = 1, show_private = 1;

int pre_defined = 0, user_defined = 1;

int current_scope; // Process being inspected

//----------------------------------------------------------------------------

int iids, *ids; // Process identifiers, in execution order
int ids_ini;    // First process displayed in the window
int ids_select; // Process highlighted for info display
int ids_next;   // Next process in execution order '>'

//----------------------------------------------------------------------------

int lp1[512];    // Line numbers where processes are defined
char *lp2[512];  // Pointers to the process definition lines
int lp_num;      // Number of processes in the list
int lp_ini;      // First entry displayed in the window
int lp_select;   // Selected entry
int lp_sort = 0; // Flag: sort the list alphabetically

//----------------------------------------------------------------------------
//      Profiler variables
//----------------------------------------------------------------------------

byte c_r_low0, c_g_low0, c_b_low0;

int obj_start; // Start of the first object (&obj[0])
int obj_size;  // Size of each object (struct object)

// The block of an ID is: (mem[ID+_Bloque]-obj_start)/obj_size;

unsigned f_time[256];     // Time consumed by the different functions
unsigned frame_time[256]; // Time consumed by the different functions

//---------------------------------------------------------------------------- //      Debug initialization //----------------------------------------------------------------------------

void init_debug(void) {
  FILE *f;
  int n;

  for (n = 0; n < max_windows; n++)
    window[n].type = 0;
  if ((mouse_background = (byte *)malloc(2048)) == NULL)
    exer(1);
  init_big();

  if ((f = fopen("system/exec.dbg", "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    n = ftell(f) - 4;
    if ((o = (struct object *)malloc(n)) != NULL) {
      fseek(f, 0, SEEK_SET);
      fread(&num_objects, 4, 1, f);
      fread(&num_predefined, 4, 1, f);
      fread(&obj_start, 4, 1, f);
      fread(&obj_size, 4, 1, f);
      fread(o, 1, n, f);
      fclose(f);
      vnom = (char *)o + num_objects * sizeof(struct object) + 4;
    } else {
      fclose(f);
      exer(1);
    }
  } else {
    for (n = 0; n < max_breakpoint; n++)
      breakpoint[n].line = -1;
    debug_active = 0;
    return;
  }

  if ((f = fopen("system/exec.lin", "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    if ((line = (int *)malloc(n)) != NULL) {
      fseek(f, 0, SEEK_SET);
      fread(line, 1, n, f);
      fclose(f);
      num_sentencias = n / (6 * 4);
    } else {
      fclose(f);
      exer(1);
    }
  } else
    line = NULL;

  if (line != NULL) {
    if ((f = fopen("system/exec.pgm", "rb")) != NULL) {
      fseek(f, 0, SEEK_END);
      n = ftell(f);
      if ((source = (byte *)malloc(n)) != NULL) {
        fseek(f, 0, SEEK_SET);
        fread(source, 1, n, f);
        fclose(f);
        end_source = source + n;
      } else {
        fclose(f);
        exer(1);
      }
    } else {
      free(line);
      line = NULL;
    }
  }

  if ((var = (struct variables *)malloc(sizeof(struct variables) * num_objects)) == NULL)
    exer(1);
  if ((ids = (int *)malloc(sizeof(int) * max_procesos)) == NULL)
    exer(1);
  if ((usado = (int *)malloc(sizeof(int) * num_objects)) == NULL)
    exer(1);
  if ((visor = (int *)malloc(sizeof(int) * num_objects)) == NULL)
    exer(1);

  // Set filters for objects

  memset(visor, 0, sizeof(int) * num_objects);
  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tcglo || o[n].type == tcloc)
      visor[n] = 2;
    if (o[n].type == tcons && o[n].v1 == 1)
      visor[n] = 2;
    if ((o[n].type == tvloc || o[n].type == tvglo || o[n].type == tcons) &&
        (vnom[o[n].name] == 'a' || vnom[o[n].name] == '\xa0') && vnom[o[n].name + 1] == 'n' &&
        vnom[o[n].name + 2] == 'g')
      visor[n] = 4;
    if (o[n].type == tcons && !strcmp(vnom + o[n].name, "pi"))
      visor[n] = 4;
    if ((o[n].type == tvloc || o[n].type == tvglo) &&
        (vnom[o[n].name + 2] == 'e' ||
         (vnom[o[n].name + 2] >= '0' && vnom[o[n].name + 2] <= '9')) &&
        vnom[o[n].name] == 'i' && vnom[o[n].name + 1] == 'd')
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "caller_id"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "father"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "bigbro"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "smallbro"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "son"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "process_id"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "id_scan"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "type_scan"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "is_executed"))
      visor[n] = 1;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "is_painted"))
      visor[n] = 1;
    if (o[n].type == tvglo && !strcmp(vnom + o[n].name, "fading"))
      visor[n] = 1;
    if (o[n].type == ttglo && !strcmp(vnom + o[n].name, "argv"))
      visor[n] = 2;

    if (o[n].type == ttglo && !strcmp(vnom + o[n].name, "name") && o[n - 2].type == tsglo &&
        !strcmp(vnom + o[n - 2].name, "dirinfo"))
      visor[n] = 2;
    if (o[n].type == tvglo && !strcmp(vnom + o[n].name, "attrib") && o[n - 13].type == tsglo &&
        !strcmp(vnom + o[n - 13].name, "fileinfo"))
      visor[n] = 6;
  }

  // Initialize process block timings

  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tproc) {
      o[n].v4 = 0; // Time_exec
      o[n].v5 = 0; // Time_paint
    }
  }
  memset(f_time, 0, 256 * 4);

  for (n = 0; n < max_breakpoint; n++)
    breakpoint[n].line = -1;
}

void end_debug(void) {
  free(mouse_background);
  free(o);
  free(var);
  free(usado);
  free(ids);
  free(visor);
  free(source);
  free(line);
}
//-----------------------------------------------------------------------------
//  Initialize the font
//-----------------------------------------------------------------------------

int old_big = -1;

void init_big(void) {
  FILE *f;
  int n;
  byte *ptr;

  if (vga_width >= 640) {
    big = 1;
    big2 = 2;
  } else {
    big = 0;
    big2 = 1;
  }

  if (old_big >= 0 && old_big != big) {
    free(text_font);
    free(graf_ptr);
  }
  old_big = big;

  if (big)
    f = fopen("system/grande.fon", "rb");
  else
    f = fopen("system/pequeno.fon", "rb");
  if (f == NULL)
    exer(5);
  fseek(f, 0, SEEK_END);
  n = ftell(f);
  if ((text_font = (byte *)malloc(n)) != NULL) {
    fseek(f, 0, SEEK_SET);
    fread(text_font, 1, n, f);
    fclose(f);
  } else {
    fclose(f);
    exer(1);
  }

  if (big)
    f = fopen("system/GRAF_G.DIV", "rb");
  else
    f = fopen("system/GRAF_P.DIV", "rb");
  if (f == NULL)
    exer(6);
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
          graf_ptr += *(int *)(graf_ptr + 52) * *(int *)(graf_ptr + 56) + 68;
        } else {
          graf[*((int *)graf_ptr)] = graf_ptr + 56;
          *(word *)(graf_ptr + 56) = *(int *)(graf_ptr + 52);
          *(word *)(graf_ptr + 58) = *(int *)(graf_ptr + 56);
          *(int *)(graf_ptr + 60) = 0;
          graf_ptr += *(int *)(graf_ptr + 52) * *(int *)(graf_ptr + 56) + 64;
        }
      }
      graf_ptr = ptr;
    } else {
      fclose(f);
      exer(1);
    }
  }
}

//-----------------------------------------------------------------------------
//  Initialize colors (after palette change)
//-----------------------------------------------------------------------------

void init_colors(void) {
  find_color(0, 0, 0);
  c0 = find_col;
  find_color(63, 63, 63);
  c4 = find_col;
  c2 = average_color(c0, c4);
  c1 = average_color(c0, c2);
  c3 = average_color(c2, c4);
  c01 = average_color(c0, c1);
  c12 = average_color(c1, c2);
  c23 = average_color(c2, c3);
  c34 = average_color(c3, c4);
  find_color(63, 0, 0);
  c_r = find_col;
  find_color(0, 63, 0);
  c_g = find_col;
  find_color(0, 0, 63);
  c_b = find_col;
  find_color(32, 0, 0);
  c_r_low = find_col;
  find_color(0, 32, 0);
  c_g_low = find_col;
  find_color(0, 0, 32);
  c_b_low = find_col;
  find_color(16, 0, 0);
  c_r_low0 = find_col;
  find_color(0, 16, 0);
  c_g_low0 = find_col;
  find_color(0, 0, 16);
  c_b_low0 = find_col;
}

//-----------------------------------------------------------------------------
//      Create a dialog box (must return to the caller as-is)
//-----------------------------------------------------------------------------

void dummy_handler(void) {}

void show_dialog(voidReturnType init_handler) {
  byte *ptr;
  int x, y, w, h;
  int vtipo;

  if (!window[max_windows - 1].type) {
    memmove(&window[1].type, &v.type, sizeof(twindow) * (max_windows - 1));

    //---------------------------------------------------------------------------
    // The following values must be set by init_handler; defaults:
    //---------------------------------------------------------------------------

    v.type = 1;
    v.foreground = 1;
    v.title = (byte *)"?";
    v.paint_handler = dummy_handler;
    v.click_handler = dummy_handler;
    v.close_handler = dummy_handler;
    v.x = 0;
    v.y = 0;
    v.w = vga_width;
    v.h = vga_height;
    v.state = 0;
    v.redraw = 0;
    v.items = 0;
    v.selected_item = -1;

    call(init_handler);

    if (big) {
      v.w = v.w * 2;
      v.h = v.h * 2;
    }

    w = v.w;
    h = v.h;

    if (no_volcar_nada) {
      x = profiler_x;
      y = profiler_y;
    } else {
      x = vga_width / 2 - w / 2;
      y = vga_height / 2 - h / 2;
    }

    v.x = x;
    v.y = y;

    if ((ptr = (byte *)malloc(w * h)) != NULL) { // Window buffer, freed in close_window

      //---------------------------------------------------------------------------
      // Send appropriate windows to background
      //---------------------------------------------------------------------------

      vtipo = v.type;
      v.type = 0;

      if (window[1].type == 1) { // Dialog over dialog
        window[1].foreground = 0;
        flush_window(1);
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
      if (!strcmp((char *)v.title, (char *)text[1]) || !strcmp((char *)v.title, (char *)text[2]))
        wbox(ptr, w, h, c_r_low, 2, 2, w - 12, 7);
      else
        wbox(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
      if (text_len(v.title) + 3 > w - 12) {
        wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
        wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
      } else {
        wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
        wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
      }

      call(v.paint_handler);

      if (big) {
        w *= 2;
        h *= 2;
      }

      do {
        dread_mouse();
      } while ((mouse_b & 1) || key(_ESC));

      explode(x, y, w, h);

      blit_region(screen_buffer, vga_width, vga_height, ptr, x, y, w, h, 0);
      blit_partial(x, y, w, h);
      do {
        dread_mouse();
      } while (mouse_b & 1);
      modal_loop();

      //---------------------------------------------------------------------------
      // Could not open the dialog (out of memory)
      //---------------------------------------------------------------------------

    } else {
      memmove(&v.type, &window[1].type, sizeof(twindow) * (max_windows - 1));
      window[max_windows - 1].type = 0;
    }
  }
}

//-----------------------------------------------------------------------------
//      Repaint a window (including title bar and icons)
//-----------------------------------------------------------------------------

void repaint_window(void) {
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wbox(v.ptr, w, h, c0, 1, 1, w - 2, 9);
  wrectangle(v.ptr, w, h, c0, 1, 1, w - 2, h - 2);
  wrectangle(v.ptr, w, h, c2, 0, 0, w, h);
  wput(v.ptr, w, h, w - 9, 2, 35);
  wbox(v.ptr, w, h, c_b_low, 2, 2, w - 12, 7);
  if (text_len(v.title) + 3 > w - 20) {
    wwrite_in_box(v.ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(v.ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(v.ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
    wwrite(v.ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
  }
}

//-----------------------------------------------------------------------------
//      Dialog box management - Debugger
//-----------------------------------------------------------------------------

void modal_loop(void) {
  int n, m, oldn = max_windows;
  int dialogo_invocado;
  int salir_del_dialogo = 0;

  end_dialog = 0;
  do {
    ascii = 0;
    scan_code = 0;
    poll_keyboard();
    dialogo_invocado = 0;

    //-------------------------------------------------------------------------
    // Find the window the mouse is over (n); n=max_windows if none
    //-------------------------------------------------------------------------

    if (mouse_in(v.x, v.y, v.x + v.w - 1, v.y + v.h - 1))
      n = 0;
    else
      n = max_windows;

    //-------------------------------------------------------------------------
    // If we were previously in a window we've now left,
    // repaint it (to clear any highlight)
    //-------------------------------------------------------------------------

    if (n == 0) // If we're on the title bar, also repaint the window
      if (!mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2))
        n--;

    if (n != oldn && oldn == 0)
      if (v.foreground == 1) {
        dialogo_invocado = 1;
        wmouse_x = -1;
        wmouse_y = -1;
        m = mouse_b;
        mouse_b = 0;
        call(v.click_handler);
        mouse_b = m;
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
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

    if (n == 0) {
      if (mouse_in(v.x + 2 * big2, v.y + 10 * big2, v.x + v.w - 2 * big2, v.y + v.h - 2 * big2)) {
        dialogo_invocado = 1;
        wmouse_x = mouse_x - v.x;
        wmouse_y = mouse_y - v.y;
        if (big) {
          wmouse_x /= 2;
          wmouse_y /= 2;
        }
        call(v.click_handler);
        if (v.redraw) {
          flush_window(0);
          v.redraw = 0;
        }
        oldn = 0;
        salir_del_dialogo = 0;

      } else { // If we are on the window's control bar ...

        if (mouse_graf == 2 && (mouse_b & 1))
          move_window();

        if (mouse_graf == 5 && (mouse_b & 1)) {
          close_window();
          salir_del_dialogo = 1;
        }
        oldn = -1;
      }
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
      call(v.click_handler);
      mouse_b = m;
      if (v.redraw) {
        flush_window(0);
        v.redraw = 0;
      }
      salir_del_dialogo = 0;
    }

    if (end_dialog && !salir_del_dialogo) {
      close_window();
      salir_del_dialogo = 1;
    }

    //-------------------------------------------------------------------------
    // Keyboard handling
    //-------------------------------------------------------------------------

    if (key(_ESC) && !key(_L_CTRL)) {
      for (n = 0; n < v.items; n++)
        if (v.item[n].type == 2 && (v.item[n].state & 2))
          break;
      if (n == v.items) {
        close_window();
        salir_del_dialogo = 1;
      }
    }

    //-------------------------------------------------------------------------
    // End of the main loop
    //-------------------------------------------------------------------------

    if (!no_volcar_nada) {
      flush_copy();
    }

  } while (!salir_del_dialogo);
  end_dialog = 0;

  get[0] = 0;

  do {
    dread_mouse();
  } while ((mouse_b & 1) || key(_ESC));
}

//----------------------------------------------------------------------------
//      Paint a show_dialog box
//----------------------------------------------------------------------------

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
  if (!strcmp((char *)v.title, (char *)text[1]))
    wbox(ptr, w, h, c_r_low, 2, 2, w - 12, 7);
  else
    wbox(ptr, w, h, c_b_low, 2, 2, w - 12, 7);
  if (text_len(v.title) + 3 > w - 12) {
    wwrite_in_box(ptr, w, w - 11, h, 4, 2, 0, v.title, c1);
    wwrite_in_box(ptr, w, w - 11, h, 3, 2, 0, v.title, c4);
  } else {
    wwrite(ptr, w, h, 3 + (w - 12) / 2, 2, 1, v.title, c1);
    wwrite(ptr, w, h, 2 + (w - 12) / 2, 2, 1, v.title, c4);
  }

  call(v.paint_handler);
}

//-----------------------------------------------------------------------------
//      Close the active window (window 0)
//-----------------------------------------------------------------------------

void close_window(void) {
  int x, y, w, h;

  call(v.close_handler);
  if (big)
    wput(v.ptr, v.w / 2, v.h / 2, v.w / 2 - 9, 2, -45);
  else
    wput(v.ptr, v.w, v.h, v.w - 9, 2, -45);
  flush_window(0);
  blit_partial(v.x, v.y, v.w, v.h);
  if (!no_volcar_nada) {
    flush_copy();
  }
  free(v.ptr);

  x = v.x;
  y = v.y;
  w = v.w;
  h = v.h;
  memmove(&v.type, &window[1].type, sizeof(twindow) * (max_windows - 1));
  update_box(x, y, w, h);

  if (v.type == 1) { // Dialog over dialog: only open the last one
    v.foreground = 1;
    flush_window(0);
  }

  do {
    dread_mouse();
  } while ((mouse_b & 1) || key(_ESC));

  implode(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Move a window
//-----------------------------------------------------------------------------

void move_window(void) {
  int ix, iy;
  int x, y, w, h;

  mouse_graf = 2;
  w = v.w;
  h = v.h;
  ix = mouse_x - v.x;
  iy = mouse_y - v.y;

  wrectangle(v.ptr, w / big2, h / big2, c4, 0, 0, w / big2, h / big2);

  do {
    x = v.x;
    y = v.y;
    v.x = mouse_x - ix;
    v.y = mouse_y - iy;
    v.type = 0;
    update_box(x, y, w, h);
    v.type = 1;
    flush_window(0);
    if (!no_volcar_nada) {
      flush_copy();
    }
  } while (mouse_b & 1);

  wrectangle(v.ptr, w / big2, h / big2, c2, 0, 0, w / big2, h / big2);
  v.redraw = 1;
}

//-----------------------------------------------------------------------------
//	Blit a window, normal or darkened depending on foreground state
//-----------------------------------------------------------------------------

void flush_window(int m) {
  int x, y, w, h, n;
  byte *_ptr;
  int __x, _y, _an, _al;
  int salta_x, salta_y;

  x = window[m].x;
  y = window[m].y;
  w = window[m].w;
  h = window[m].h;

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
        _ptr = window[n].ptr;
        salta_x = 0;
        salta_y = 0;
        __x = window[n].x;
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
        if (x > __x) {
          salta_x += x - __x;
          _ptr += salta_x;
          __x = x;
          _an -= salta_x;
        }
        if (x + w < __x + _an) {
          salta_x += __x + _an - x - w;
          _an -= __x + _an - x - w;
        }

        if (_an > 0 && _al > 0) {
          if (window[n].foreground == 1)
            blit_region(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al, salta_x);
          else
            blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al, salta_x);
        }
      }

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//      Blit a window to screen
//-----------------------------------------------------------------------------

void blit_region(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w, int h,
              int salta) {
  byte *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  q = dest + y * dest_width + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width + salta;
  else
    resto_x = salta;
  long_x = w + salta - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += dest_width * salta_y + salta_x;
  resto_x += salta_x + long_x;
  w = long_x;
  do {
    memcpy(q, p, w);
    q += dest_width;
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Blit a window to screen (darkened)
//-----------------------------------------------------------------------------

void blit_region_dark(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w,
                     int h, int salta) {
  byte *q, *_ghost;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  q = dest + y * dest_width + x;

  if (c0)
    _ghost = ghost + 256 * (int)c0;
  else
    _ghost = ghost_inicial;


  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width + salta;
  else
    resto_x = salta;
  long_x = w + salta - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += dest_width * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;

  do {
    do {
      *q = *(_ghost + *p);
      p++;
      q++;
    } while (--w);
    q += dest_width - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Draw a filled box on screen
//-----------------------------------------------------------------------------

void wbox(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h) {
  wbox_in_box(dest, dest_width, dest_width, dest_height, c, x, y, w, h);
}

void wbox_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, byte c, int x, int y,
                 int w, int h) {
  byte *p;

  if (big) {
    dest_pitch *= 2;
    dest_width *= 2;
    dest_height *= 2;
    x *= 2;
    y *= 2;
    w *= 2;
    h *= 2;
  }

  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y + h > dest_height)
    h = dest_height - y;
  if (x + w > dest_width)
    w = dest_width - x;

  if (w > 0 && h > 0) {
    p = dest + y * dest_pitch + x;
    do {
      memset(p, c, w);
      p += dest_pitch;
    } while (--h);
  }
}


//-----------------------------------------------------------------------------
//      Draw a rectangle outline
//-----------------------------------------------------------------------------

void wrectangle(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h) {
  wbox(dest, dest_width, dest_height, c, x, y, w, 1);
  wbox(dest, dest_width, dest_height, c, x, y + h - 1, w, 1);
  wbox(dest, dest_width, dest_height, c, x, y + 1, 1, h - 2);
  wbox(dest, dest_width, dest_height, c, x + w - 1, y + 1, 1, h - 2);
}

//-----------------------------------------------------------------------------
//      Draw a graphic
//-----------------------------------------------------------------------------

void put(int x, int y, int n) {
  wput_in_box(screen_buffer, vga_width, vga_width, vga_height, x, y, n);
}

void wput(byte *dest, int dest_width, int dest_height, int x, int y, int n) {
  wput_in_box(dest, dest_width, dest_width, dest_height, x, y, n);
}

void wput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n) {
  int h, w;
  int block;
  byte *p, *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (big)
    if (n >= 32 || n < 0) {
      bwput_in_box(dest, dest_pitch, dest_width, dest_height, x, y, n);
      return;
    }

  if (n < 0) {
    n = -n;
    block = 1;
  } else
    block = 0;

  p = graf[n] + 8;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  q = dest + y * dest_pitch + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width;
  else
    resto_x = 0;
  if ((long_x = w - salta_x - resto_x) <= 0)
    return;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  if ((long_y = h - salta_y - resto_y) <= 0)
    return;

  p += w * salta_y + salta_x;
  q += dest_pitch * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;
  if (block)
    do {
      do {
        switch (*p) {
        case 0:
          *q = c0;
          break;
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
  else
    do {
      do {
        switch (*p) {
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
}

void bwput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y,
                  int n) {
  int h, w;
  int block;
  byte *p, *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (n < 0) {
    n = -n;
    block = 1;
  } else
    block = 0;

  p = graf[n] + 8;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  if (dest_width > 0) {
    dest_pitch *= 2;
    dest_width *= 2;
    dest_height *= 2;
    x *= 2;
    y *= 2;
  } else {
    dest_width = -dest_width;
    if (dest_pitch < 0)
      dest_pitch = -dest_pitch;
  }

  q = dest + y * dest_pitch + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width;
  else
    resto_x = 0;
  if ((long_x = w - salta_x - resto_x) <= 0)
    return;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  if ((long_y = h - salta_y - resto_y) <= 0)
    return;

  p += w * salta_y + salta_x;
  q += dest_pitch * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;

  if (block)
    do {
      do {
        switch (*p) {
        case 0:
          *q = c0;
          break;
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
  else
    do {
      do {
        switch (*p) {
        case 1:
          *q = c1;
          break;
        case 2:
          *q = c2;
          break;
        case 3:
          *q = c3;
          break;
        case 4:
          *q = c4;
          break;
        }
        p++;
        q++;
      } while (--w);
      q += dest_pitch - (w = long_x);
      p += resto_x;
    } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Text printing functions
//-----------------------------------------------------------------------------

int char_len(char c) {
  struct _car {
    byte w;
    word dir;
  } *car;
  car = (struct _car *)(text_font + 1);
  return (car[c].w);
}

int text_len(byte *ptr) {
  int w;

  struct _car {
    byte w;
    word dir;
  } *car;

  car = (struct _car *)(text_font + 1);
  w = 0;
  while (*ptr) {
    w += car[*ptr].w;
    ptr++;
  }

  if (big)
    w /= 2;

  if (w) {
    w--;
  }
  return (w);
}

int text_len2(byte *ptr) {
  int w;

  struct _car {
    byte w;
    word dir;
  } *car;

  car = (struct _car *)(text_font + 1);
  w = 0;
  while (*ptr) {
    w += car[*ptr].w;
    ptr++;
  }

  if (big)
    w /= 2;

  return (w - 1);
}

void wwrite(byte *dest, int dest_width, int dest_height, int x, int y, int centro, byte *ptr,
            byte c) {
  wwrite_in_box(dest, dest_width, dest_width, dest_height, x, y, centro, ptr, c);
}

void wwrite_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x_org,
                   int y_org, int centro_org, byte *ptr, byte c) {
  int w, h, boton, multi;
  int centro = centro_org, x = x_org, y = y_org;

  struct _car {
    byte w;
    word dir;
  } *car;

  byte *font;

  if (centro & 0xFF00) {
    w = (centro >> 8);
    h = 7;
    centro &= 0xFF;
  } else
    w = 0;

  if (centro >= 10) {
    centro -= 10;
    multi = 1;
  } else
    multi = 0;

  if (*ptr == '\xd') {
    boton = 1;
    ptr++;
  } else
    boton = 0;

  car = (struct _car *)(text_font + 1);

  if (!w) {
    if (big && !multi) {
      w = text_len(ptr);
      h = 7;
    } else {
      font = ptr;
      w = 0;
      while (*font) {
        w += car[*font].w;
        font++;
      }
      h = *text_font;
      if (w)
        w--;
    }
  }

  font = text_font + 1025;

  switch (centro) {
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

  if (boton) {
    if (c != c0) {
      wbox(dest, dest_pitch, dest_height, c2, x - 2, y - 2, w + 4, h + 4);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 3, y - 3, w + 6, h + 6);
      wrectangle(dest, dest_pitch, dest_height, c3, x - 2, y - 2, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c3, x - 2, y - 2, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c4, x - 2, y - 2, 1, 1);
      wrectangle(dest, dest_pitch, dest_height, c1, x - 1, y + h + 1, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c1, x + w + 1, y - 1, 1, h + 3);
    } else {
      wbox(dest, dest_pitch, dest_height, c1, x - 2, y - 2, w + 4, h + 4);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 3, y - 3, w + 6, h + 6);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 2, y - 2, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c0, x - 2, y - 2, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c2, x - 1, y + h + 1, w + 3, 1);
      wrectangle(dest, dest_pitch, dest_height, c2, x + w + 1, y - 1, 1, h + 3);
      wrectangle(dest, dest_pitch, dest_height, c3, x + w + 1, y + h + 1, 1, 1);
    }
  }

  if (centro_org & 0xFF00) {
    if (big && !multi) {
      w = text_len(ptr);
      h = 7;
    } else {
      font = ptr;
      w = 0;
      while (*font) {
        w += car[*font].w;
        font++;
      }
      h = *text_font;
      if (w)
        w--;
    }
    font = text_font + 1025;
    switch (centro) {
    case 0:
      break;
    case 1:
      x = x_org - (w >> 1);
      break;
    case 2:
      x = x_org - w + 1;
      break;
    case 3:
      y = y_org - (h >> 1);
      break;
    case 4:
      x = x_org - (w >> 1);
      y = y_org - (h >> 1);
      break;
    case 5:
      x = x_org - w + 1;
      y = y_org - (h >> 1);
      break;
    case 6:
      y = y_org - h + 1;
      break;
    case 7:
      x = x_org - (w >> 1);
      y = y_org - h + 1;
      break;
    case 8:
      x = x_org - w + 1;
      y = y_org - h + 1;
      break;
    }
  }

  if (big && !multi) {
    dest_pitch *= 2;
    dest_width *= 2;
    dest_height *= 2;
    x *= 2;
    y *= 2;
    w *= 2;
    h *= 2;
  }

  if (y < dest_height && y + h > 0) {
    if (y >= 0 && y + h <= dest_height) { // Text fits entirely (y coord)
      while (*ptr && x + car[*ptr].w <= 0) {
        x = x + car[*ptr].w;
        ptr++;
      }
      if (*ptr && x < 0) {
        wtexc(dest, dest_pitch, dest_width, dest_height, font + car[*ptr].dir, x, y, car[*ptr].w,
              h, c);
        x = x + car[*ptr].w;
        ptr++;
      }
      while (*ptr && x + car[*ptr].w <= dest_width) {
        wtexn(dest, dest_pitch, font + car[*ptr].dir, x, y, car[*ptr].w, h, c);
        x = x + car[*ptr].w;
        ptr++;
      }
      if (*ptr && x < dest_width)
        wtexc(dest, dest_pitch, dest_width, dest_height, font + car[*ptr].dir, x, y, car[*ptr].w,
              h, c);
    } else {
      while (*ptr && x + car[*ptr].w <= 0) {
        x = x + car[*ptr].w;
        ptr++;
      }
      while (*ptr && x < dest_width) {
        wtexc(dest, dest_pitch, dest_width, dest_height, font + car[*ptr].dir, x, y, car[*ptr].w,
              h, c);
        x = x + car[*ptr].w;
        ptr++;
      }
    }
  }
}

void wtexn(byte *dest, int dest_pitch, byte *p, int x, int y, byte w, int h, byte c) {
  byte *q = dest + y * dest_pitch + x;
  int width = w;

  do {
    do {
      if (*p) {
        *q = c;
      }
      p++;
      q++;
    } while (--w);
    q += dest_pitch - (w = width);
  } while (--h);
}

void wtexc(byte *dest, int dest_pitch, int dest_width, int dest_height, byte *p, int x, int y,
           byte w, int h, byte c) {
  byte *q = dest + y * dest_pitch + x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > dest_width)
    resto_x = x + w - dest_width;
  else
    resto_x = 0;
  long_x = w - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > dest_height)
    resto_y = y + h - dest_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += dest_pitch * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;
  do {
    do {
      if (*p) {
        *q = c;
      }
      p++;
      q++;
    } while (--w);
    q += dest_pitch - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Explode animation for a new window
//-----------------------------------------------------------------------------

void explode(int x, int y, int w, int h) {
  int n = 0, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  if (no_volcar_nada)
    return;
  v.type = 0;
  big = 0;
  update_box(x, y, w, h);
  while (++n < 10) {
    aan = (w * n) / 10;
    aal = (h * n) / 10;
    xx = x + w / 2 - aan / 2;
    yy = y + h / 2 - aal / 2;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, aal);
    retrace_wait();
    flush_copy();
    update_box(xx, yy, aan, 1);
    update_box(xx, yy, 1, aal);
    update_box(xx + aan - 1, yy, 1, aal);
    update_box(xx, yy + aal - 1, aan, 1);
  }
  v.type = tipo;
  big = b;
}

void implode(int x, int y, int w, int h) {
  int n = 9, b = big;
  int xx, yy, aan, aal;
  if (no_volcar_nada)
    return;
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
    blit_partial(xx, yy, aan, aal);
    flush_copy();
    update_box(xx, yy, aan, aal);
    retrace_wait();
  } while (--n);
  big = b;
}

void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
  int n = 9, tipo = v.type, b = big;
  int xx, yy, aan, aal;
  if (no_volcar_nada)
    return;
  v.type = 0;
  big = 0;
  update_box(x, y, w, h);
  do {
    aan = (w * n + w2 * (10 - n)) / 10;
    aal = (h * n + h2 * (10 - n)) / 10;
    xx = (x * n + x2 * (10 - n)) / 10;
    yy = (y * n + y2 * (10 - n)) / 10;
    wrectangle(screen_buffer, vga_width, vga_height, c4, xx, yy, aan, aal);
    blit_partial(xx, yy, aan, aal);
    flush_copy();
    update_box(xx, yy, aan, aal);
    retrace_wait();
  } while (--n);
  big = b;
  v.type = tipo;
}

//-----------------------------------------------------------------------------
//      Update a screen region
//-----------------------------------------------------------------------------

void update_box(int x, int y, int w, int h) {
  int n;
  byte *_ptr;
  int __x, _y, _an, _al;
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

  restore_wallpaper(x, y, w, h);

  for (n = max_windows - 1; n >= 0; n--)
    if (window[n].type)
      if (collides_with(n, x, y, w, h)) {
        _ptr = window[n].ptr;
        salta_x = 0;
        salta_y = 0;
        __x = window[n].x;
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
        if (x > __x) {
          salta_x += x - __x;
          _ptr += salta_x;
          __x = x;
          _an -= salta_x;
        }
        if (x + w < __x + _an) {
          salta_x += __x + _an - x - w;
          _an -= __x + _an - x - w;
        }

        if (_an > 0 && _al > 0) {
          if (window[n].foreground == 1)
            blit_region(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al, salta_x);
          else
            blit_region_dark(screen_buffer, vga_width, vga_height, _ptr, __x, _y, _an, _al, salta_x);
        }
      }

  blit_partial(x, y, w, h);
}

//-----------------------------------------------------------------------------
//	Check if two windows collide
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

//----------------------------------------------------------------------------
//      Restore wallpaper/background under debugger windows
// TODO: Should also update the background when the game state changes.
//----------------------------------------------------------------------------

void restore_wallpaper(int x, int y, int w, int h) {
  byte *p;
  byte *t;
  int n, _an;

  if (y < 0) {
    y = 0;
  }
  if (x < 0) {
    x = 0;
  }
  if (y + h > vga_height)
    h = vga_height - y;
  if (x + w > vga_width)
    w = vga_width - x;

  if (w > 0 && h > 0) {
    p = screen_buffer + y * vga_width + x;
    t = screen_buffer_debug + y * vga_width;
    _an = w;
    do {
      n = x;
      do {
        *p = *(t + n);
        p++;
        n++;
      } while (--w);
      w = _an;
      p += vga_width - w;
      t += vga_width;
    } while (--h);
  }
}

//-----------------------------------------------------------------------------
//      Item definitions
//-----------------------------------------------------------------------------

void _button(byte *t, int x, int y, int c) {
  v.item[v.items].type = 1;
  v.item[v.items].state = 0;
  v.item[v.items].button.text = t;
  v.item[v.items].button.x = x;
  v.item[v.items].button.y = y;
  v.item[v.items].button.center = c;
  if (v.selected_item == -1)
    v.selected_item = v.items;
  v.items++;
}

void _get(byte *t, int x, int y, int w, byte *buffer, int lon_buffer, int r0, int r1) {
  v.item[v.items].type = 2;
  v.item[v.items].state = 0;
  v.item[v.items].get.text = t;
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

void _flag(byte *t, int x, int y, int *valor) {
  v.item[v.items].type = 3;
  v.item[v.items].state = 0;
  v.item[v.items].flag.text = t;
  v.item[v.items].flag.x = x;
  v.item[v.items].flag.y = y;
  v.item[v.items].flag.valor = valor;
  v.items++;
}

//-----------------------------------------------------------------------------
//      Item rendering
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

void _show_items2(void) {
  int n = 0;
  show_items_called = 1;
  while (n < v.items) {
    switch (v.item[n].type) {
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

void show_button(t_item *i) {
  wwrite(v.ptr, v.w / big2, v.h / big2, i->button.x, i->button.y, i->button.center,
         i->button.text, c3);
  if (&v.item[v.selected_item] == i)
    select_button(i, 1);
}

void show_get(t_item *i) {
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

void select_get(t_item *i, int activo, int ocultar_error) {
  int n;
  if (activo) {
    wrectangle(v.ptr, v.w / big2, v.h / big2, c12, i->get.x - 1, i->get.y + 7, i->get.w + 2, 11);
    if (i->state & 2) {
      div_strcpy(get, sizeof(get), (char *)i->get.buffer);
      get_pos = strlen(get);
    }
    i->state &= 1;
  } else {
    if (i->state & 2) {
      if (*get) {
        if (i->get.r0 == i->get.r1)
          div_strcpy((char *)i->get.buffer, i->get.lon_buffer, get);
        else {
          if (atoi(get) >= i->get.r0 && atoi(get) <= i->get.r1)
            itoa(atoi(get), (char *)i->get.buffer, 10);
          else if (!ocultar_error && !show_items_called) {
            div_snprintf(combo_error, sizeof(combo_error), "%s [%d..%d].", (char *)text[4],
                         i->get.r0, i->get.r1);
            text[3] = (byte *)combo_error;
            v_text = (char *)text[3];
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

void show_flag(t_item *i) {
  if (*i->flag.valor)
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, -59);
  else
    wput(v.ptr, v.w / big2, v.h / big2, i->flag.x, i->flag.y, 58);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 9, i->flag.y, 0, i->flag.text, c12);
  wwrite(v.ptr, v.w / big2, v.h / big2, i->flag.x + 8, i->flag.y, 0, i->flag.text, c3);
}

void select_button(t_item *i, int activo) {
  int x = i->button.x, y = i->button.y;
  int w, h;

  if (i->button.center & 0xFF00) {
    w = (i->button.center >> 8);
    h = 7;
  } else {
    w = text_len(i->button.text + 1);
    h = 7;
  }
  switch ((i->button.center & 0xFF)) {
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
  if (v.ptr[(x - 4) * big2 - 1 + (y - 5) * big2 * v.w] == c12) {
    if (activo) {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c1, x - 4, y - 4, w + 8, h + 8);
    } else {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c12, x - 4, y - 4, w + 8, h + 8);
    }
  } else {
    if (activo) {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c12, x - 4, y - 4, w + 8, h + 8);
    } else {
      wrectangle(v.ptr, v.w / big2, v.h / big2, c2, x - 4, y - 4, w + 8, h + 8);
    }
  }
}

//-----------------------------------------------------------------------------
//      Item processing
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
                       v.item[v.selected_item].get.lon_buffer, "");
          div_strcpy(get, sizeof(get), (char *)v.item[v.selected_item].get.buffer);
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
    if (!(x & 2)) {
      if (ascii == 13)
        ascii = 0;
      else
        x |= 4;
    }
    x |= 2;
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
    div_strcpy(get, sizeof(get), (char *)v.item[n].get.buffer);
    get_pos = strlen(get);
  }

  if (e & 4) {
    *get = 0;
  }
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

void get_input(int n) {
  char cwork[512];
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
      div_strcpy((char *)v.item[v.selected_item].get.buffer, v.item[v.selected_item].get.lon_buffer,
                 "");
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
                     v.item[v.selected_item].get.lon_buffer, "");
        break;
      default:
        v.redraw = l;
        break;
      }
    } else if (ascii && char_len(ascii) > 1 && (x = strlen(get)) < v.item[n].get.lon_buffer - 1) {
      div_strcpy(cwork, sizeof(cwork), get);
      cwork[get_pos] = ascii;
      cwork[get_pos + 1] = 0;
      div_strcat(cwork, sizeof(cwork), get + get_pos);
      div_strcpy(get, sizeof(get), cwork);
      get_pos++;
      v.redraw = 1;
      get[x++] = ascii;
      get[x] = 0;
      v.redraw = 1;
    }
    break;
  }

  if (v.redraw || get_cursor != (system_clock & 4)) {
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

    if (system_clock & 4) {
      x = l + 1;
      wbox_in_box(v.ptr + (v.item[n].get.x + 1) * big2, v.w / big2, v.item[n].get.w - 2,
                  v.h / big2, c3, x - scroll, v.item[n].get.y + 9, 2, 7);
    }
  }
  get_cursor = (system_clock & 4);
}

int button_status(int n) {
  int x = v.item[n].button.x, y = v.item[n].button.y;
  int w, h, e = 0;

  if (v.item[n].button.center & 0xFF00) {
    w = (v.item[n].button.center >> 8);
    h = 7;
  } else {
    w = text_len(v.item[n].button.text + 1);
    h = 7;
  }
  switch ((v.item[n].button.center & 0xFF)) {
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

//----------------------------------------------------------------------------
//      Read mouse adapted for the interpreter
//----------------------------------------------------------------------------
void readmouse(void);

void dread_mouse(void) {
  poll_keyboard();
  int n = 0;

  mouse_x = (int)mouse->x;
  mouse_y = (int)mouse->y;

  if (mouse_x < 0) {
    mouse_x = 0;
    n++;
  } else if (mouse_x >= vga_width) {
    mouse_x = vga_width - 1;
    n++;
  }
  if (mouse_y < 0) {
    mouse_y = 0;
    n++;
  } else if (mouse_y >= vga_height) {
    mouse_y = vga_height - 1;
    n++;
  }

  if (n)
    set_mouse(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------
//      Blit the virtual screen copy to the real screen (main display)
//-----------------------------------------------------------------------------

void flush_copy(void) {
  dread_mouse();
  save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 0);
  put(mouse_x, mouse_y, mouse_graf);
  blit_screen(screen_buffer);
  save_mouse_bg(mouse_background, mouse_x, mouse_y, mouse_graf, 1);

  // **************************************************

  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 16;
  fade_wait();

  // **************************************************
}

//-----------------------------------------------------------------------------
//      Save (flag=0) or restore (flag=1) a virtual screen region
//-----------------------------------------------------------------------------

void save_mouse_bg(byte *p, int x, int y, int n, int flag) {
  byte *q;
  int w, h;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  h = *((word *)(graf[n] + 2));
  w = *((word *)graf[n]);

  x -= *((word *)(graf[n] + 4));
  y -= *((word *)(graf[n] + 6));

  if (x >= vga_width || y >= vga_height || x + w <= 0 || y + h <= 0)
    return;

  blit_partial(x, y, w, h);

  q = screen_buffer + y * vga_width + x;

  if (x < 0)
    salta_x = -x;
  else
    salta_x = 0;
  if (x + w > vga_width)
    resto_x = x + w - vga_width;
  else
    resto_x = 0;
  long_x = w - salta_x - resto_x;

  if (y < 0)
    salta_y = -y;
  else
    salta_y = 0;
  if (y + h > vga_height)
    resto_y = y + h - vga_height;
  else
    resto_y = 0;
  long_y = h - salta_y - resto_y;

  p += w * salta_y + salta_x;
  q += vga_width * salta_y + salta_x;
  resto_x += salta_x;
  w = long_x;
  do {
    do {
      if (flag)
        *q = *p;
      else
        *p = *q;
      p++;
      q++;
    } while (--w);
    q += vga_width - (w = long_x);
    p += resto_x;
  } while (--long_y);
}

//-----------------------------------------------------------------------------
//      Check if the mouse is inside a box
//-----------------------------------------------------------------------------

int mouse_in(int x, int y, int x2, int y2) {
  return (mouse_x >= x && mouse_x <= x2 && mouse_y >= y && mouse_y <= y2);
}

int wmouse_in(int x, int y, int w, int h) {
  return (wmouse_x >= x && wmouse_x <= x + w - 1 && wmouse_y >= y && wmouse_y <= y + h - 1);
}

//-----------------------------------------------------------------------------
//      ERROR dialog box
//-----------------------------------------------------------------------------

void err1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, (byte *)v_text, c3);
}

void err2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void err0(void) {
  v.type = 1;
  v.title = text[1];
  v.w = text_len((byte *)v_text) + 8;
  v.h = 38;
  v.paint_handler = err1;
  v.click_handler = err2;
  _button(text[7], v.w / 2, v.h - 14, 1);
}

//-----------------------------------------------------------------------------
//      ERROR dialog box inside a language function
//-----------------------------------------------------------------------------

char *te;

void _err1(void) {
  int w = v.w / big2, h = v.h / big2, n;
  char cwork[256];
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, text[8], c4);
  if (num_objects) {
    for (n = 0; n < num_objects; n++)
      if (o[n].type == tproc && o[n].v0 == mem[id + _Bloque])
        break;
    if (n < num_objects)
      wwrite(v.ptr, w, h, 46, 12, 0, (byte *)vnom + o[n].name, c3);
    else
      wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
  } else
    wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
  wwrite(v.ptr, w, h, 4, 12 + 8, 0, (byte *)text[10], c4);
  if (v_function >= 0)
    wwrite(v.ptr, w, h, 46, 12 + 8, 0, (byte *)fname[v_function], c3);
  else
    wwrite(v.ptr, w, h, 46, 12 + 8, 0, text[9], c3);

  wwrite(v.ptr, w, h, 4, 12 + 16, 0, (byte *)text[1], c4);
  itoa(num_error, cwork, 10);
  wwrite(v.ptr, w, h, 46, 12 + 16, 0, (byte *)cwork, c3);

  wwrite(v.ptr, w, h, 4, 12 + 24, 0, (byte *)v_text, c3);
}

void _err2(void) {
  int n;
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
  if (v.active_item == 1) {
    end_dialog = 1;
    n = 0;
    while (n < num_skipped) {
      if (omitidos[n] == num_error)
        break;
      n++;
    }
    if (n >= num_skipped && num_skipped < 128) {
      omitidos[num_skipped++] = num_error;
    }
  }
  if (v.active_item == 2) {
    if (debug_active)
      call_to_debug = 1;
    end_dialog = 1;
  }
  if (v.active_item == 3)
    exer(0);
}

void _err0(void) {
  v.type = 1;
  v.title = text[1];
  v.w = text_len((byte *)v_text) + 8;
  if (v.w < 218 + 16)
    v.w = 218 + 16;
  v.h = 38 + 16 + 8;
  v.paint_handler = _err1;
  v.click_handler = _err2;

  //  TODO: add the missing one and check that 218 is enough
  _button(text[11], 7, v.h - 14, 0);
  _button(text[12], 7 + text_len(text[11]) + 14, v.h - 14, 0);
  _button(text[13], v.w - 8 - text_len(text[14]) - 14, v.h - 14, 2);
  _button(text[14], v.w - 8, v.h - 14, 2);
}

//----------------------------------------------------------------------------
//      Error messages - Debugger version
//----------------------------------------------------------------------------

extern int ignore_errors;

void e(int text_id) {
#ifdef __EMSCRIPTEN__
  return;
#endif
  int smouse_x, smouse_y;
  int reloj_e = frame_clock, n;
  int ticks_e = ticks;
  int dr = dacout_r, dg = dacout_g, db = dacout_b;

  if (v_function == -1)
    return; // Some errors are better off ignored...

  num_error = text_id;
  te = (char *)text[text_id];
  n = 0;
  while (n < num_skipped) {
    if (omitidos[n] == num_error)
      break;
    n++;
  }
  if (ignore_errors || n < num_skipped)
    return;

  smouse_x = mouse->x;
  smouse_y = mouse->y;
  set_mouse(mouse_x, mouse_y);
  if (!v.type)
    memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 16;

  mouse_graf = 1;
  v_text = (char *)text[text_id];
  show_dialog(_err0);
  dacout_r = dr;
  dacout_g = dg;
  dacout_b = db;
  frame_clock = reloj_e;
  ticks = ticks_e;
  set_mouse(smouse_x, smouse_y);
}

//-----------------------------------------------------------------------------
//      Dialog box when a DEBUG statement is reached
//-----------------------------------------------------------------------------

void deb1(void) {
  int w = v.w / big2, h = v.h / big2, n;
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, text[8], c4);
  if (num_objects) {
    for (n = 0; n < num_objects; n++)
      if (o[n].type == tproc && o[n].v0 == mem[id + _Bloque])
        break;
    if (n < num_objects)
      wwrite(v.ptr, w, h, 46, 12, 0, (byte *)vnom + o[n].name, c3);
    else
      wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
  } else
    wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
}

void deb2(void) {
  _process_items();
  if (v.active_item == 2) {
    mem[ip - 1] = 0; // NOP out the debug statement
    end_dialog = 1;
  }
  if (v.active_item == 0) {
    if (debug_active) {
      call_to_debug = 1;
      process_stoped = id;
    }
    end_dialog = 1;
  }
  if (v.active_item == 1)
    exer(0);
}

void deb0(void) {
  v.type = 1;
  v.title = text[15];
  v.w = 190;
  v.h = 38;
  v.paint_handler = deb1;
  v.click_handler = deb2;
  _button(text[16], v.w / 2, v.h - 14, 1);
  _button(text[17], v.w - 8, v.h - 14, 2);
  _button(text[18], 7, v.h - 14, 0);
}

void deb(void) {
  int smouse_x, smouse_y;
  int reloj_e = frame_clock;
  int ticks_e = ticks;
  int dr = dacout_r, dg = dacout_g, db = dacout_b;

  smouse_x = mouse->x;
  smouse_y = mouse->y;
  set_mouse(mouse_x, mouse_y);
  if (!v.type)
    memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 16;

  mouse_graf = 1;
  show_dialog(deb0);

  dacout_r = dr;
  dacout_g = dg;
  dacout_b = db;
  frame_clock = reloj_e;
  ticks = ticks_e;
  set_mouse(smouse_x, smouse_y);
}

//----------------------------------------------------------------------------
//    Determine if all processes have been executed
//----------------------------------------------------------------------------

int procesos_ejecutados(void) {
  int max = 0x80000000, ide = 0;
  if (process_stoped)
    return (0);
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
  return (!ide);
}

//----------------------------------------------------------------------------
//      Build ids[] (process list, in execution order)
//----------------------------------------------------------------------------

void determine_ids(void) {
  int max, ide, id;

  iids = 0; // Number of processes
  ids_next = -1;

  // Mark all processes as not yet in the list

  for (ide = id_start; ide <= id_end; ide += iloc_len)
    mem[ide + _Painted] = 0;

  // First add already-executed processes to the list

  id_old = id_start;

  do {
    ide = 0;
    max = 0x80000000;
    id = id_old;
    do {
      if (mem[id + _Status] == 2 && mem[id + _Executed] && !mem[id + _Painted] &&
          mem[id + _Priority] > max) {
        ide = id;
        max = mem[id + _Priority];
      }
      if (id == id_end)
        id = id_start;
      else
        id += iloc_len;
    } while (id != id_old);
    if (ide) {
      mem[ide + _Painted] = 1;
      if (iids < max_procesos)
        ids[iids++] = ide;
    }
  } while (ide);

  // Then the stopped process, if any

  if (process_stoped) {
    mem[process_stoped + _Painted] = 1;
    if (ids_next == -1)
      ids_next = iids;
    if (iids < max_procesos)
      ids[iids++] = process_stoped;
  }

  // Then add processes yet to be executed

  do {
    ide = 0;
    max = 0x80000000;
    id = id_old;
    do {
      if (mem[id + _Status] == 2 && !mem[id + _Executed] && !mem[id + _Painted] &&
          mem[id + _Priority] > max) {
        ide = id;
        max = mem[id + _Priority];
      }
      if (id == id_end)
        id = id_start;
      else
        id += iloc_len;
    } while (id != id_old);
    if (ide) {
      mem[ide + _Painted] = 1;
      if (ids_next == -1)
        ids_next = iids;
      if (iids < max_procesos)
        ids[iids++] = ide;
    }
  } while (ide);

  // Then add remaining processes (any status)

  do {
    ide = 0;
    max = 0x80000000;
    id = id_old;
    do {
      if (mem[id + _Status] != 0 && !mem[id + _Painted] && mem[id + _Priority] > max) {
        ide = id;
        max = mem[id + _Priority];
      }
      if (id == id_end)
        id = id_start;
      else
        id += iloc_len;
    } while (id != id_old);
    if (ide) {
      mem[ide + _Painted] = 1;
      if (iids < max_procesos)
        ids[iids++] = ide;
    }
  } while (ide);
}


//----------------------------------------------------------------------------
//      Draw the process list and info about the selected process
//----------------------------------------------------------------------------

void draw_proc_list(void) {
  char msg[512];
  byte *ptr = v.ptr;
  int n, m, x;
  int w = v.w / big2, h = v.h / big2;

  wbox(ptr, w, h, c1, 4, 20, 118, 126 - 5 - 16 - 32); // Process listbox fill

  for (m = ids_ini; m < ids_ini + 9 && m < iids; m++) {
    if (m == ids_select) {
      x = c4;
      wbox(ptr, w, h, c01, 4, 20 + (m - ids_ini) * 8, 118, 9); // Selected process highlight
    } else
      x = c3;
    for (n = 0; n < num_objects; n++)
      if (o[n].type == tproc && o[n].v0 == mem[ids[m] + _Bloque])
        break;
    if (n < num_objects)
      div_strcpy(msg, sizeof(msg), (char *)vnom + o[n].name);
    else
      div_strcpy(msg, sizeof(msg), (char *)text[9]);
    div_strcat(msg, sizeof(msg), "(");
    itoa(mem[ids[m] + _Id], msg + strlen(msg), 10);
    div_strcat(msg, sizeof(msg), ")");
    wwrite_in_box(ptr, w, 90, h, 10, 21 + (m - ids_ini) * 8, 0, (byte *)msg, x);
    switch (mem[ids[m] + _Status]) {
    case 0:
      msg[0] = '-';
      break;
    case 1:
      msg[0] = 'K';
      break;
    case 2:
      msg[0] = 'A';
      break;
    case 3:
      msg[0] = 'S';
      break;
    case 4:
      msg[0] = 'F';
      break;
    default:
      msg[0] = '?';
      break;
    }
    msg[1] = 0;
    wwrite(ptr, w, h, 93, 21 + (m - ids_ini) * 8, 0, (byte *)msg, x);
    if (mem[ids[m] + _Executed])
      div_snprintf(msg, sizeof(msg), "%03d", mem[ids[m] + _Frame] + 100);
    else
      div_snprintf(msg, sizeof(msg), "%03d", mem[ids[m] + _Frame]);
    div_strcat(msg, sizeof(msg), "%");
    wwrite_in_box(ptr, w, 121, h, 101, 21 + (m - ids_ini) * 8, 0, (byte *)msg, x);
    if (m == ids_next)
      wput(ptr, w, h, 4, 21 + (m - ids_ini) * 8, 36);
  }

  wrectangle(ptr, w, h, c0, 91, 19, 1, 128 - 5 - 16 - 32);
  wrectangle(ptr, w, h, c0, 91 + 8, 19, 1, 128 - 5 - 16 - 32);

  wbox(ptr, w, h, c12, 133, 11, 119 + 304 - 256, 7); // Process identifier
  div_strcpy(msg, sizeof(msg), (char *)text[19]);
  itoa(ids[ids_select], msg + strlen(msg), 10);
  switch (mem[ids[ids_select] + _Status]) {
  case 1:
    div_strcat(msg, sizeof(msg), (char *)text[20]);
    break;
  case 2:
    div_strcat(msg, sizeof(msg), (char *)text[21]);
    break;
  case 3:
    div_strcat(msg, sizeof(msg), (char *)text[22]);
    break;
  case 4:
    div_strcat(msg, sizeof(msg), (char *)text[23]);
    break;
  default:
    div_strcat(msg, sizeof(msg), (char *)text[24]);
    break;
  }
  wwrite(ptr, w, h, 134, 11, 0, (byte *)msg, c1);
  wwrite(ptr, w, h, 133, 11, 0, (byte *)msg, c3);

  for (n = 0; n < num_objects; n++)
    if (o[n].type == tproc && o[n].v0 == mem[ids[ids_select] + _Bloque])
      break;
  if (n < num_objects)
    div_strcpy(msg, sizeof(msg), vnom + o[n].name);
  else
    div_strcpy(msg, sizeof(msg), (char *)text[9]);

  wbox(ptr, w, h, c1, 134, 20, 118 + 304 - 256, 9); // Process name box
  wwrite_in_box(ptr, w, 252, h, 135, 21, 0, (byte *)msg, c3);

  wbox(ptr, w, h, c12, 200 - 16 + 304 - 256 - 4, 47, 54 + 16 + 4, 32); // Main process variables

  if (mem[ids[ids_select] + _Resolution])
    div_snprintf(msg, sizeof(msg), "x=%f",
                 (float)mem[ids[ids_select] + _X] / (float)mem[ids[ids_select] + _Resolution]);
  else
    div_snprintf(msg, sizeof(msg), "x=%d", mem[ids[ids_select] + _X]);
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48, 1, (byte *)msg, c3);
  if (mem[ids[ids_select] + _Resolution])
    div_snprintf(msg, sizeof(msg), "y=%f",
                 (float)mem[ids[ids_select] + _Y] / (float)mem[ids[ids_select] + _Resolution]);
  else
    div_snprintf(msg, sizeof(msg), "y=%d", mem[ids[ids_select] + _Y]);
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48 + 8, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48 + 8, 1, (byte *)msg, c3);

  switch (mem[ids[ids_select] + _Ctype]) {
  case 0:
    div_strcpy(msg, sizeof(msg), (char *)text[25]);
    break;
  case 1:
    div_strcpy(msg, sizeof(msg), (char *)text[26]);
    break;
  case 2:
    div_strcpy(msg, sizeof(msg), (char *)text[27]);
    break;
  case 3:
    div_strcpy(msg, sizeof(msg), (char *)text[28]);
    break;
  default:
    div_strcpy(msg, sizeof(msg), (char *)text[29]);
    break;
  }
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48 + 16, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48 + 16, 1, (byte *)msg, c3);

  switch (mem[ids[ids_select] + _Flags] & 7) {
  case 0:
    div_strcpy(msg, sizeof(msg), (char *)text[30]);
    break;
  case 1:
    div_strcpy(msg, sizeof(msg), (char *)text[31]);
    break;
  case 2:
    div_strcpy(msg, sizeof(msg), (char *)text[32]);
    break;
  case 3:
    div_strcpy(msg, sizeof(msg), (char *)text[33]);
    break;
  case 4:
    div_strcpy(msg, sizeof(msg), (char *)text[34]);
    break;
  case 5:
    div_strcpy(msg, sizeof(msg), (char *)text[35]);
    break;
  case 6:
    div_strcpy(msg, sizeof(msg), (char *)text[36]);
    break;
  case 7:
    div_strcpy(msg, sizeof(msg), (char *)text[37]);
    break;
  }
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48 + 24, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48 + 24, 1, (byte *)msg, c3);

  wbox(ptr, w, h, c1, 134, 32, 46, 46); // Process graphic
  process_graph(ids[ids_select], ptr + 32 * big2 * v.w + 134 * big2, v.w, 46 * big2, 46 * big2);

  paint_segment();
}

//----------------------------------------------------------------------------
//      Draw a process graphic for the debugger
//----------------------------------------------------------------------------

void process_graph(int id, byte *q, int van, int w, int h) {
  int file, graph, angle;
  int x, y, n, m;
  int xr, ixr, yr, iyr, old_w;
  short xg, yg;
  int *ptr;
  byte *p;

  file = mem[id + _File];
  graph = mem[id + _Graph];
  angle = mem[id + _Angle];

  if (file < 0 || file > max_fpgs)
    return;

  if (mem[id + _Ctype] == 2 || mem[id + _Ctype] == 3)
    angle = 0;
  else if ((n = mem[id + _XGraph]) > 0) {
    m = mem[n];
    if (m < 1 || m > 256)
      return;
    while (angle >= 2 * pi)
      angle -= 2 * pi;
    while (angle < 0)
      angle += 2 * pi;
    mem[id + _Flags] &= 254;
    graph = ((angle + (2 * pi) / (m * 2)) * m) / (2 * pi);
    angle = 0;
    if (graph >= m)
      graph = 0;
    if ((graph = mem[n + 1 + graph]) < 0) {
      graph = -graph;
      mem[id + _Flags] |= 1;
    }
  }

  if (file)
    max_grf = 1000;
  else
    max_grf = 2000;
  if (graph <= 0 || graph >= max_grf)
    return;
  if (g[file].grf == NULL)
    return;
  if ((ptr = g[file].grf[graph]) == NULL)
    return;

  x = mem[id + _X];
  y = mem[id + _Y];
  if (mem[id + _Resolution] > 0) {
    x /= mem[id + _Resolution];
    y /= mem[id + _Resolution];
  }

  if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
    xg = ptr[13] / 2;
    yg = ptr[14] / 2;
  } else {
    xg = *((word *)ptr + 32);
    yg = *((word *)ptr + 33);
  }

  if (angle) {
    clipx0 = x;
    clipy0 = y;
    clipx1 = ptr[13];
    clipy1 = ptr[14];
    sp_size(&clipx0, &clipy0, &clipx1, &clipy1, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);
  } else if (mem[id + _Size] != 100) {
    clipx0 = x;
    clipy0 = y;
    clipx1 = ptr[13];
    clipy1 = ptr[14];
    sp_size_scaled(&clipx0, &clipy0, &clipx1, &clipy1, xg, yg, mem[id + _Size], mem[id + _Flags]);
  } else {
    if (mem[id + _Flags] & 1)
      clipx0 = x - (ptr[13] - 1 - xg);
    else
      clipx0 = x - xg;
    if (mem[id + _Flags] & 2)
      clipy0 = y - (ptr[14] - 1 - yg);
    else
      clipy0 = y - yg;
    clipx1 = clipx0 + ptr[13] - 1;
    clipy1 = clipy0 + ptr[14] - 1;
  }

  buffer_w = clipx1 - clipx0 + 1;
  buffer_h = clipy1 - clipy0 + 1;
  if ((buffer = (byte *)malloc(buffer_w * buffer_h)) == NULL)
    return;
  memset(buffer, 0, buffer_w * buffer_h);

  // Now paint the sprite into the buffer (clip...)

  put_collision(buffer, ptr, x, y, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);

  // window q, van, of an x al

  wbox(v.ptr, v.w / big2, v.h / big2, c0, 134, 32, w / big2, h / big2); // Set black background

  if (buffer_w <= w && buffer_h <= h) { // If it fits in the window, center it

    q += (w - buffer_w) / 2 + ((h - buffer_h) / 2) * van;
    p = buffer;
    old_w = buffer_w;
    do {
      do {
        *q++ = *p++;
      } while (--buffer_w);
      q += van - (buffer_w = old_w);
    } while (--buffer_h);

  } else { // If it doesn't fit, scale it down

    ixr = (float)(buffer_w * 256) / (float)w;
    iyr = (float)(buffer_h * 256) / (float)h;

    if (ixr > iyr) {
      yg = h - (float)(buffer_h * 256) / (float)ixr;
      q += (yg / 2) * van;
      h -= yg;
      iyr = ixr;
    } else if (iyr > ixr) {
      xg = w - (float)(buffer_w * 256) / (float)iyr;
      q += xg / 2;
      w -= xg;
      ixr = iyr;
    }
    old_w = w;
    yr = 0;

    if (w && h)
      do {
        p = buffer + (yr >> 8) * buffer_w;
        xr = 0;
        do {
          *q = *(p + (xr >> 8));
          q++;
          xr += ixr;
        } while (--w);
        yr += iyr;
        q += van - (w = old_w);
      } while (--h);
  }

  free(buffer);
}

//----------------------------------------------------------------------------
//      Debug function (invoked by pressing F12 during execution)
//----------------------------------------------------------------------------

void debug(void) {
  smouse_x = mouse->x;
  smouse_y = mouse->y;
  set_mouse(mouse_x, mouse_y);

  reloj_debug = frame_clock;
  ticks_debug = ticks;

  dr = dacout_r;
  dg = dacout_g;
  db = dacout_b;

  memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);

  new_palette = 0;
  mouse_graf = 1;
  show_dialog(debug0);

  dacout_r = dr;
  dacout_g = dg;
  dacout_b = db;

  frame_clock = reloj_debug;
  ticks = ticks_debug;

  set_mouse(smouse_x, smouse_y);
}

//----------------------------------------------------------------------------
//  Create the variable list applying the current filter
//----------------------------------------------------------------------------

int member;

void create_variable_list(void) {
  int n, nuevo, incluir;

  memset(usado, 0, sizeof(int) * num_objects);

  num_var = 0;

  // Add variables in alphabetical order to the list var[num_var]

  do {
    nuevo = 0;
    for (n = 0; n < num_objects; n++) { // Find the smallest alphabetically

      if (!pre_defined && n < num_predefined)
        continue;
      if (!user_defined && n >= num_predefined)
        continue;
      if (usado[n])
        continue;
      if (o[n].miembro)
        continue;

      switch (o[n].type) {
      case tcons:
        incluir = show_const;
        break;
      case tcglo:
      case tvglo:
      case ttglo:
      case tsglo:
      case tbglo:
      case twglo:
      case tpigl:
      case tpwgl:
      case tpbgl:
      case tpcgl:
      case tpsgl:
        incluir = show_global;
        break;
      case tcloc:
      case tvloc:
      case ttloc:
      case tsloc:
      case tbloc:
      case twloc:
      case tpilo:
      case tpwlo:
      case tpblo:
      case tpclo:
      case tpslo:
        if (o[n].scope) {
          if (o[n].scope == current_scope)
            incluir = show_private;
          else
            incluir = 0;
        } else
          incluir = show_local;
        break;
      default:
        incluir = 0;
        break;
      }
      if (incluir) {
        if (!nuevo || (nuevo && strcmp(vnom + o[n].name, vnom + o[nuevo].name) == -1)) {
          nuevo = n;
        }
      }
    }
    if (nuevo) {
      var[num_var].object = nuevo;
      var[num_var].tab = 0;
      var[num_var].miembro = 0;
      usado[nuevo] = 1;

      switch (o[nuevo].type) {
      case tpigl:
      case tpwgl:
      case tpbgl:
      case tpcgl:
      case tpsgl:
      case tpilo:
      case tpwlo:
      case tpblo:
      case tpclo:
      case tpslo:
        var[num_var].indice = -1;
        break;
      default:
        var[num_var].indice = 0;
        break;
      }
      num_var++;

      if (o[nuevo].type == tsglo || o[nuevo].type == tsloc) {
        member = nuevo + 1;
        include_members(num_var, 1, var[num_var - 1].indice);
      }
      if (o[nuevo].type == tpsgl || o[nuevo].type == tpslo) {
        member = o[nuevo].v1 + 1;
        include_members(num_var, 1, var[num_var - 1].indice);
      }
    }
  } while (nuevo);
}

void exclude_members(int padre, int nivel, int index) {
  int m;
  m = o[member].miembro;
  while (m == o[member].miembro) {
    if (index == -1)
      var[padre].object = -member;
    else
      var[padre].object = member;
    switch (o[member].type) {
    case tpigl:
    case tpwgl:
    case tpbgl:
    case tpcgl:
    case tpsgl:
    case tpilo:
    case tpwlo:
    case tpblo:
    case tpclo:
    case tpslo:
      var[padre].indice = -1;
      break;
    }
    padre++;
    if (o[member].type == tsglo || o[member].type == tsloc) {
      member++;
      exclude_members(padre, nivel + 1, index);
    } else
      member++;
  }
}

void include_members(int padre, int nivel, int index) {
  int m;

  m = o[member].miembro;

  while (m == o[member].miembro) {
    if (index == -1)
      var[num_var].object = -member;
    else
      var[num_var].object = member;
    var[num_var].tab = nivel;
    var[num_var].miembro = padre;
    usado[member] = 1;

    switch (o[member].type) {
    case tpigl:
    case tpwgl:
    case tpbgl:
    case tpcgl:
    case tpsgl:
    case tpilo:
    case tpwlo:
    case tpblo:
    case tpclo:
    case tpslo:
      var[num_var].indice = -1;
      break;
    default:
      var[num_var].indice = 0;
      break;
    }
    num_var++;

    if (o[member].type == tsglo || o[member].type == tsloc) {
      member++;
      include_members(num_var, nivel + 1, index);
    } else
      member++;
  }
}

//----------------------------------------------------------------------------
//  Inspect window
//----------------------------------------------------------------------------

int _var_ini = -1, _var_select = -1;

void change0(void);
void changestring0(void);

void inspect1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(ptr, w, h, 4, h - 14, 0, text[38], c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128 + 32 + 64, 83); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122 + 32 + 64, 19, 9, 83);
  wrectangle(ptr, w, h, c0, 122 + 32 + 64, 27, 9, 83 - 16);

  wput(ptr, w, h, 123 + 32 + 64, 20, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, 123 + 32 + 64, 134 - 40, -40);

  create_variable_list();

  if (_var_ini != -1 && num_var > 1) {
    var_ini = _var_ini;
    var_select = _var_select;

    if (var_ini + 10 > num_var && var_ini)
      var_ini = num_var - 10;
    if (var_ini < 0)
      var_ini = 0;
    if (var_select >= num_var)
      var_select = num_var - 1;
    if (var_select < var_ini)
      var_select = var_ini;
    if (var_select >= var_ini + 10)
      var_select = var_ini + 9;

  } else {
    var_ini = 0;
    var_select = 0;
  }

  paint_var_list();

  paint_segment2();
}

void paint_segment2(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 89, n;
  float x;

  wbox(ptr, w, h, c2, 123 + 32 + 64, 28, 7, 65); // Clear the slider bar

  if (num_var <= 1)
    n = min;
  else {
    x = (float)var_select / (float)(num_var - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122 + 32 + 64, n, 9, 5);
  wput(ptr, w, h, 123 + 32 + 64, n + 1, -43);
}

void inspect2(void) {
  int n, tipo;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  float x;

  _process_items();
  if (scan_code == 80 && var_select + 1 < num_var) {
    if (var_ini + 10 == ++var_select)
      var_ini++;
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 72 && var_select) {
    if (var_ini == var_select--)
      var_ini--;
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 81) {
    for (n = 0; n < 10; n++)
      if (var_select + 1 < num_var) {
        if (var_ini + 10 == ++var_select)
          var_ini++;
      }
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 73) {
    for (n = 0; n < 10; n++)
      if (var_select) {
        if (var_ini == var_select--)
          var_ini--;
      }
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }

  if (wmouse_in(3, 21, 128 + 32 + 64 - 9, 80) && (mouse_b & 1)) {
    n = var_ini + (wmouse_y - 21) / 8;
    if (n < num_var) {
      var_select = n;
      paint_var_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122 + 32 + 64, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (boton == 0) {
        wput(ptr, w, h, 123 + 32 + 64, 20, -41);
        boton = 1;
        if (var_select) {
          if (var_ini == var_select--)
            var_ini--;
          paint_var_list();
          v.redraw = 1;
        }
      }
    } else if (boton == 1) {
      wput(ptr, w, h, 123 + 32 + 64, 20, -39);
      boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (boton == 1) {
    wput(ptr, w, h, 123 + 32 + 64, 20, -39);
    boton = 0;
    v.redraw = 1;
  }

  if (wmouse_in(123 + 32 + 64, 28, 7, 65)) {
    mouse_graf = 13;
    if (num_var > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - 28) / 64.0;
      var_select = x * (num_var - 1);
      if (var_select < var_ini)
        var_ini = var_select;
      if (var_select >= var_ini + 10)
        var_ini = var_select - 9;
      paint_var_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122 + 32 + 64, 93, 9, 9)) {
    if (mouse_b & 1) {
      if (boton == 0) {
        wput(ptr, w, h, 123 + 32 + 64, 94, -42);
        boton = 2;
        if (var_select + 1 < num_var) {
          if (var_ini + 10 == ++var_select)
            var_ini++;
          paint_var_list();
          v.redraw = 1;
        }
      }
    } else if (boton == 2) {
      wput(ptr, w, h, 123 + 32 + 64, 94, -40);
      boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (boton == 2) {
    wput(ptr, w, h, 123 + 32 + 64, 94, -40);
    boton = 0;
    v.redraw = 1;
  }

  if (num_var) {
    if (var[var_select].object > 0) {
      if (scan_code == 75)
        goto dec_index;
      if (scan_code == 77)
        goto inc_index;
    }
  }

  if (scan_code == 71) { // Home
    if (var[var_select].object > 0)
      switch (o[var[var_select].object].type) {
      case tpigl:
      case tpilo:
      case tpwgl:
      case tpwlo:
      case tpbgl:
      case tpblo:
      case tpcgl:
      case tpclo:
        var[var_select].indice = -1;
        paint_var_list();
        v.redraw = 1;
        break;
      case tpsgl:
      case tpslo:
        if (var[var_select].indice > -1) {
          var[var_select].indice = -1;
          if (!var[var_select].miembro) {
            member = o[var[var_select].object].v1 + 1;
            exclude_members(var_select + 1, 1, var[var_select].indice);
            if (var_ini + 10 > num_var && var_ini)
              var_ini = num_var - 10;
            if (var_ini < 0)
              var_ini = 0;
            paint_segment2();
          }
          paint_var_list();
          paint_segment2();
          v.redraw = 1;
        }
        break;
      default:
        var[var_select].indice = 0;
        paint_var_list();
        v.redraw = 1;
        break;
      }
  }

  if (scan_code == 79) { // End
    if (var[var_select].object > 0)
      switch (o[var[var_select].object].type) {
      case tpigl:
      case tpilo:
      case tpwgl:
      case tpwlo:
      case tpbgl:
      case tpblo:
      case tpcgl:
      case tpclo:
        if (var[var_select].indice == -1) {
          tipo = o[var[var_select].object].type;
          switch (o[var[var_select].object].type) {
          case tpigl:
          case tpwgl:
          case tpbgl:
          case tpcgl:
          case tpsgl:
            o[var[var_select].object].type = tvglo;
            break;
          case tpilo:
          case tpwlo:
          case tpblo:
          case tpclo:
          case tpslo:
            o[var[var_select].object].type = tvloc;
            break;
          }
          n = memo(get_offset(var_select));
          o[var[var_select].object].type = tipo;
          if (!n)
            break; // Not if the pointer is NULL
        }
        if (o[var[var_select].object].v2 == -1) {
          var[var_select].indice = 0;
          break;
        }
        /* fall through */
      case ttglo:
      case ttloc:
      case tbglo:
      case tbloc:
      case twglo:
      case twloc:
        var[var_select].indice = o[var[var_select].object].v2 + 1;
        if (o[var[var_select].object].v3 > -1)
          var[var_select].indice *= o[var[var_select].object].v3 + 1;
        if (o[var[var_select].object].v4 > -1)
          var[var_select].indice *= o[var[var_select].object].v4 + 1;
        var[var_select].indice--;
        break;
      case tpsgl:
      case tpslo:
        if (var[var_select].indice == -1) {
          if (memo(get_offset(var_select)) == 0)
            break; // Don't show the struct if the pointer is NULL
          var[var_select].indice = 0;
          if (!var[var_select].miembro) {
            member = o[var[var_select].object].v1 + 1;
            exclude_members(var_select + 1, 1, var[var_select].indice);
            paint_segment2();
          }
          paint_var_list();
          break;
        }
        if (o[var[var_select].object].v3 == -1)
          var[var_select].indice = 0;
        else
          var[var_select].indice = o[var[var_select].object].v2 - 1;
        if (!var[var_select].miembro) {
          member = o[var[var_select].object].v1 + 1;
          exclude_members(var_select + 1, 1, var[var_select].indice);
          paint_segment2();
        }
        paint_var_list();
        break;
      case tsglo:
      case tsloc:
        var[var_select].indice = o[var[var_select].object].v2 - 1;
        break;
      }
    paint_var_list();
    v.redraw = 1;
  }

  if (num_var == 0 && v.active_item < 9)
    v.active_item = -1;
  if (var[var_select].object < 0 && v.active_item < 9)
    v.active_item = -1;

  switch (v.active_item) {
  case 0: // Change
    if (o[var[var_select].object].type != tcons) {
      if (o[var[var_select].object].type == tsglo || o[var[var_select].object].type == tsloc ||
          ((o[var[var_select].object].type == tpsgl || o[var[var_select].object].type == tpslo) &&
           var[var_select].indice >= 0)) {
        v_text = (char *)text[39];
        show_dialog(err0);
      } else if (o[var[var_select].object].type == tcglo ||
                 o[var[var_select].object].type == tcloc) {
        show_dialog(changestring0);
        paint_var_list();
        v.redraw = 1;
      } else {
        show_dialog(change0);
        paint_var_list();
        v.redraw = 1;
      }
    } else {
      v_text = (char *)text[40];
      show_dialog(err0);
    }
    break;
  case 1: // Index--
dec_index:
    switch (o[var[var_select].object].type) {
    case ttglo:
    case ttloc:
    case tsglo:
    case tsloc:
    case tbglo:
    case tbloc:
    case twglo:
    case twloc:
      if (var[var_select].indice > 0)
        var[var_select].indice--;
      paint_var_list();
      v.redraw = 1;
      break;
    case tpigl:
    case tpilo:
    case tpwgl:
    case tpwlo:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      if (var[var_select].indice > -1)
        var[var_select].indice--;
      paint_var_list();
      v.redraw = 1;
      break;
    case tpsgl:
    case tpslo:
      if (var[var_select].indice > -1) {
        var[var_select].indice--;
        if (!var[var_select].miembro) {
          member = o[var[var_select].object].v1 + 1;
          exclude_members(var_select + 1, 1, var[var_select].indice);
          paint_segment2();
        }
        paint_var_list();
        v.redraw = 1;
      }
      break;
    }
    break;
  case 2: // Index++
inc_index:
    switch (o[var[var_select].object].type) {
    case tpigl:
    case tpilo:
    case tpwgl:
    case tpwlo:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      if (var[var_select].indice == -1) {
        tipo = o[var[var_select].object].type;
        switch (o[var[var_select].object].type) {
        case tpigl:
        case tpwgl:
        case tpbgl:
        case tpcgl:
        case tpsgl:
          o[var[var_select].object].type = tvglo;
          break;
        case tpilo:
        case tpwlo:
        case tpblo:
        case tpclo:
        case tpslo:
          o[var[var_select].object].type = tvloc;
          break;
        }
        n = memo(get_offset(var_select));
        o[var[var_select].object].type = tipo;
        if (!n)
          break; // Not if the pointer is NULL
      }
      if (o[var[var_select].object].v2 == -1) {
        var[var_select].indice++;
        paint_var_list();
        v.redraw = 1;
        break;
      }
      /* fall through */
    case ttglo:
    case ttloc:
    case tbglo:
    case tbloc:
    case twglo:
    case twloc:
      n = o[var[var_select].object].v2 + 1;
      if (o[var[var_select].object].v3 > -1)
        n *= o[var[var_select].object].v3 + 1;
      if (o[var[var_select].object].v4 > -1)
        n *= o[var[var_select].object].v4 + 1;
      if (var[var_select].indice < n - 1)
        var[var_select].indice++;
      paint_var_list();
      v.redraw = 1;
      break;
    case tpsgl:
    case tpslo:
      if (var[var_select].indice == -1) {
        if (memo(get_offset(var_select)) == 0)
          break; // Don't show the struct if the pointer is NULL
      }
      if (o[var[var_select].object].v3 == -1)
        var[var_select].indice++;
      else if (var[var_select].indice < o[var[var_select].object].v2 - 1)
        var[var_select].indice++;
      if (!var[var_select].miembro) {
        member = o[var[var_select].object].v1 + 1;
        exclude_members(var_select + 1, 1, var[var_select].indice);
        paint_segment2();
      }
      paint_var_list();
      v.redraw = 1;
      break;
    case tsglo:
    case tsloc:
      if (var[var_select].indice < o[var[var_select].object].v2 - 1)
        var[var_select].indice++;
      paint_var_list();
      v.redraw = 1;
      break;
    }
    break;
  case 3: // View as angle
    if (visor[var[var_select].object] == 4)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 4;
    paint_var_list();
    v.redraw = 1;
    break;
  case 4: // View as process
    if (visor[var[var_select].object] == 3)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 3;
    paint_var_list();
    v.redraw = 1;
    break;
  case 5: // View as text
    if (visor[var[var_select].object] == 2)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 2;
    paint_var_list();
    v.redraw = 1;
    break;
  case 6: // View as boolean
    if (visor[var[var_select].object] == 1)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 1;
    paint_var_list();
    v.redraw = 1;
    break;
  case 7: // View as Hex
    if (visor[var[var_select].object] == 5)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 5;
    paint_var_list();
    v.redraw = 1;
    break;
  case 8: // View as Bin
    if (visor[var[var_select].object] == 6)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 6;
    paint_var_list();
    v.redraw = 1;
    break;
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14: // Flags
    create_variable_list();
    var_ini = 0;
    var_select = 0;
    paint_var_list();
    paint_segment2();
    v.redraw = 1;
    break;
  }
}

void inspect3(void) {
  _var_ini = var_ini;
  _var_select = var_select;
}

char titulo[256];

void inspect0(void) {
  int n, x = 50;

  v.type = 1;
  v.title = (byte *)titulo;
  div_strcpy(titulo, sizeof(titulo), (char *)text[41]);

  for (n = 0; n < num_objects; n++)
    if (o[n].type == tproc && o[n].v0 == mem[ids[ids_select] + _Bloque])
      break;
  if (n < num_objects)
    div_strcat(titulo, sizeof(titulo), vnom + o[n].name);
  else
    div_strcat(titulo, sizeof(titulo), (char *)text[9]);

  div_strcat(titulo, sizeof(titulo), "(");
  itoa(ids[ids_select], titulo + strlen(titulo), 10);
  div_strcat(titulo, sizeof(titulo), ")");

  v.w = 209 + 64;
  v.h = 121;
  v.paint_handler = inspect1;
  v.click_handler = inspect2;
  v.close_handler = inspect3;

  _button(text[42], 185 + 64, 72, 1);

  _button(text[43], 169 + 64, 92, 0);
  _button(text[44], 190 + 64, 92, 0);

  _button(text[45], x, v.h - 14, 0);
  x += text_len(text[45] + 1) + 8;
  _button(text[46], x, v.h - 14, 0);
  x += text_len(text[46] + 1) + 8;
  _button(text[47], x, v.h - 14, 0);
  x += text_len(text[47] + 1) + 8;
  _button(text[48], x, v.h - 14, 0);
  x += text_len(text[48] + 1) + 8;
  _button(text[49], x, v.h - 14, 0);
  x += text_len(text[49] + 1) + 8;
  _button(text[50], x, v.h - 14, 0);

  _flag(text[51], 4, 11, &pre_defined);
  _flag(text[52], 32 + text_len(text[51]), 11, &user_defined);

  _flag((byte *)"Const", v.w - 44, 19, &show_const);
  _flag((byte *)"Global", v.w - 44, 19 + 10, &show_global);
  _flag((byte *)"Local", v.w - 44, 19 + 20, &show_local);
  _flag((byte *)"Private", v.w - 44, 19 + 30, &show_private);
}

//----------------------------------------------------------------------------
//  Draw the variable list inside the listbox
//----------------------------------------------------------------------------

// Functions to prevent page faults

byte *capar_byte(byte *dir) {
  uintptr_t offset;
  offset = ((uintptr_t)dir - (uintptr_t)mem) / 4;
  if (validate_address(offset))
    return (dir);
  else
    return ((byte *)mem);
}

word *capar_word(word *dir) {
  uintptr_t offset;
  offset = ((uintptr_t)dir - (uintptr_t)mem) / 4;
  if (validate_address(offset))
    return (dir);
  else
    return ((word *)mem);
}

int memo(int dir) {
  return (mem[validate_address(dir)]);
}

void paint_var_list(void) {
  char msg[512];
  byte *ptr = v.ptr, x;
  int w = v.w / big2, h = v.h / big2;
  int m, tipo, fin;

  wbox(ptr, w, h, c1, 4, 20, 150 + 64, 81); // Process listbox fill

  scan_code = 0;
  ascii = 0;
  fin = var_ini + 10;

  for (m = var_ini; m < fin && m < num_var; m++) {
    if (m == var_select) {
      wbox(ptr, w, h, c01, 4, 20 + (m - var_ini) * 8, 150 + 64, 9); // Process listbox fill
      x = c4;
    } else
      x = c3;
    if (var[m].object < 0) {
      switch (o[abs(var[m].object)].type) {
      case tcons:
        div_strcpy(msg, sizeof(msg), "CONST ");
        break;
      case tvglo:
      case tvloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        break;
      case tcglo:
      case tcloc:
        div_strcpy(msg, sizeof(msg), "STRING ");
        break;
      case tbglo:
      case tbloc:
        div_strcpy(msg, sizeof(msg), "BYTE ");
        break;
      case twglo:
      case twloc:
        div_strcpy(msg, sizeof(msg), "WORD ");
        break;
      case ttglo:
      case ttloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        break;
      case tpigl:
      case tpilo:
        div_strcpy(msg, sizeof(msg), "INT POINTER ");
        break;
      case tpwgl:
      case tpwlo:
        div_strcpy(msg, sizeof(msg), "WORD POINTER ");
        break;
      case tpbgl:
      case tpblo:
        div_strcpy(msg, sizeof(msg), "BYTE POINTER ");
        break;
      case tpsgl:
      case tpslo:
        div_strcpy(msg, sizeof(msg), "STRUCT POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[o[abs(var[m].object)].v1].name);
        div_strcat(msg, sizeof(msg), vnom + o[o[abs(var[m].object)].v1].name);
        strupr(msg);
        div_strcat(msg, sizeof(msg), " ");
        break;
      case tpcgl:
      case tpclo:
        div_strcpy(msg, sizeof(msg), "STRING POINTER ");
        break;
      case tsglo:
      case tsloc:
        div_strcpy(msg, sizeof(msg), "STRUCT ");
        break;
      }
      div_strcat(msg, sizeof(msg), vnom + o[abs(var[m].object)].name);
      x = c2;
    } else
      switch (o[var[m].object].type) {
      case tcons:
        div_strcpy(msg, sizeof(msg), "CONST ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        visualize(o[var[m].object].v0, var[m].object, msg);
        break;
      case tvglo:
      case tvloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        visualize(memo(get_offset(m)), var[m].object, msg);
        break;
      case tcglo:
      case tcloc:
        div_strcpy(msg, sizeof(msg), "STRING ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        visualize(get_offset(m), var[m].object, msg);
        break;
      case tbglo:
      case tbloc:
        div_strcpy(msg, sizeof(msg), "BYTE ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v3 == -1 && o[var[m].object].v2 == 0)
          goto show_variable;
        else
          goto show_tabla;
      case twglo:
      case twloc:
        div_strcpy(msg, sizeof(msg), "WORD ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v3 == -1 && o[var[m].object].v2 == 0)
          goto show_variable;
        else
          goto show_tabla;
      case ttglo:
      case ttloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v3 == -1 && o[var[m].object].v2 == 0)
          goto show_variable;
        else
          goto show_tabla;
      case tpigl:
      case tpilo:
        div_strcpy(msg, sizeof(msg), "INT POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
      case tpwgl:
      case tpwlo:
        div_strcpy(msg, sizeof(msg), "WORD POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
      case tpbgl:
      case tpblo:
        div_strcpy(msg, sizeof(msg), "BYTE POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
      case tpsgl:
      case tpslo:
        div_strcpy(msg, sizeof(msg), "STRUCT POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[o[var[m].object].v1].name);
        strupr(msg);
        div_strcat(msg, sizeof(msg), " ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_indice;
      case tpcgl:
      case tpclo:
        div_strcpy(msg, sizeof(msg), "STRING POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
show_tabla:
        div_strcat(msg, sizeof(msg), "[");
        if (o[var[m].object].v3 == -1) { // 1-dimensional array
          itoa(var[m].indice, msg + strlen(msg), 10);
        } else if (o[var[m].object].v4 == -1) { // 2-dimensional array
          itoa(var[m].indice % (o[var[m].object].v2 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa(var[m].indice / (o[var[m].object].v2 + 1), msg + strlen(msg), 10);
        } else { // 3-dimensional array
          itoa(var[m].indice % (o[var[m].object].v2 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v2 + 1)) % (o[var[m].object].v3 + 1),
               msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v2 + 1)) / (o[var[m].object].v3 + 1),
               msg + strlen(msg), 10);
        }
        div_strcat(msg, sizeof(msg), "]");
show_variable:
        if (var[m].indice == -1) {
          tipo = o[var[m].object].type;
          switch (o[var[m].object].type) {
          case tpigl:
          case tpwgl:
          case tpbgl:
          case tpcgl:
          case tpsgl:
            o[var[m].object].type = tvglo;
            break;
          case tpilo:
          case tpwlo:
          case tpblo:
          case tpclo:
          case tpslo:
            o[var[m].object].type = tvloc;
            break;
          }
          if (!memo(get_offset(m)))
            div_strcat(msg, sizeof(msg), " = NULL");
          else
            visualize(memo(get_offset(m)), var[m].object, msg);
          o[var[m].object].type = tipo;
        } else if (o[var[m].object].type == ttglo || o[var[m].object].type == ttloc ||
                   o[var[m].object].type == tpigl || o[var[m].object].type == tpilo) {
          visualize(memo(get_offset(m)), var[m].object, msg);
        } else if (o[var[m].object].type == twglo || o[var[m].object].type == twloc ||
                   o[var[m].object].type == tpwgl || o[var[m].object].type == tpwlo) {
          visualize(*get_offset_word(m), var[m].object, msg);
        } else if (o[var[m].object].type == tbglo || o[var[m].object].type == tbloc ||
                   o[var[m].object].type == tpbgl || o[var[m].object].type == tpblo) {
          visualize(*get_offset_byte(m), var[m].object, msg);
        } else if (o[var[m].object].type == tpcgl || o[var[m].object].type == tpclo) {
          visualize(*get_offset_byte(m), var[m].object, msg);
        }
        break;
      case tsglo:
      case tsloc:
        div_strcpy(msg, sizeof(msg), "STRUCT ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v4 == -1 && o[var[m].object].v3 == 0)
          break;
show_indice:
        div_strcat(msg, sizeof(msg), "[");
        if (o[var[m].object].v4 == -1) { // 1-dimensional struct
          itoa(var[m].indice, msg + strlen(msg), 10);
        } else if (o[var[m].object].v5 == -1) { // 2-dimensional struct
          itoa(var[m].indice % (o[var[m].object].v3 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa(var[m].indice / (o[var[m].object].v3 + 1), msg + strlen(msg), 10);
        } else { // 3-dimensional struct
          itoa(var[m].indice % (o[var[m].object].v3 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v3 + 1)) % (o[var[m].object].v4 + 1),
               msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v3 + 1)) / (o[var[m].object].v4 + 1),
               msg + strlen(msg), 10);
        }
        div_strcat(msg, sizeof(msg), "]");
        break;
      }
    wwrite_in_box(ptr, w, 144 + 9 + 64, h, 10 + var[m].tab * 8, 21 + (m - var_ini) * 8, 0,
                  (byte *)msg, x);
  }

  paint_segment2();
}

//----------------------------------------------------------------------------
//  Display an object according to its visor[] mode
//----------------------------------------------------------------------------

void visualize(int valor, int object, char *str) {
  int n;
  div_strcat(str, 512, " = ");
  switch (visor[object]) {
  case 0: // Integer
    itoa(valor, str + strlen(str), 10);
    break;
  case 1: // Boolean
    if (valor & 1)
      div_strcat(str, 512, "TRUE");
    else
      div_strcat(str, 512, "FALSE");
    break;
  case 2: // Text
    if (valor >= 256) {
      div_strcat(str, 512, "\"");
      if (strlen((char *)&mem[valor]) + strlen(str) < 511) {
        div_strcat(str, 512, (char *)&mem[valor]);
      } else {
        str[strlen(str) + 256] = 0;
        memcpy(str + strlen(str), (char *)&mem[valor], 256);
      }
      div_strcat(str, 512, "\"");
    } else if (valor >= 0 && valor <= 255) {
      if (valor == 0)
        div_strcat(str, 512, "<EOL>");
      else
        div_snprintf(str + strlen(str), 512 - strlen(str), "\"%c\"", valor);
    } else
      itoa(valor, str + strlen(str), 10);
    break;
  case 3: // Process
    if (valor == id_init) {
      div_strcat(str, 512, "div_main()");
    } else if (valor) {
      for (n = 0; n < iids; n++)
        if (ids[n] == valor)
          break;
      if (n < iids) {
        for (n = 0; n < num_objects; n++)
          if (o[n].type == tproc && o[n].v0 == mem[valor + _Bloque])
            break;
        if (n < num_objects) {
          div_strcat(str, 512, (char *)vnom + o[n].name);
          div_strcat(str, 512, "(");
          n = 1;
        } else
          n = 0;
      } else
        n = 0;
      itoa(valor, str + strlen(str), 10);
      if (n)
        div_strcat(str, 512, ")");
    } else
      div_strcat(str, 512, (char *)text[53]);
    break;
  case 4: // Angle
    while (valor > pi)
      valor -= 2 * pi;
    while (valor < -pi)
      valor += 2 * pi;
    div_snprintf(str + strlen(str), 512 - strlen(str), "%.3f\xa7, %.4f rad", (float)valor / 1000.0,
                 (float)valor / radian);
    break;
  case 5: // Hex
    div_snprintf(str + strlen(str), 512 - strlen(str), "0x%X", valor);
    break;
  case 6: // Bin
    for (n = 0; n < 32; n++)
      if (valor & 0x80000000)
        break;
      else
        valor <<= 1;
    if (n < 32) {
      for (; n < 32; n++) {
        if (valor & 0x80000000)
          div_strcat(str, 512, "1");
        else
          div_strcat(str, 512, "0");
        valor <<= 1;
      }
      div_strcat(str, 512, " Bin");
    } else {
      div_strcat(str, 512, "0 Bin");
    }
    break;
  }
}

//----------------------------------------------------------------------------
//  Get the offset within mem[] for a variable in the list
//----------------------------------------------------------------------------

int get_offset(int m) {
  return (validate_address(_get_offset(m)));
}

int _get_offset(int m) {
  if (var[m].miembro == 0)
    switch (o[var[m].object].type) {
    case tvglo:
      return (o[var[m].object].v0);
    case tcglo:
      return (o[var[m].object].v0);
    case ttglo:
      if (var[m].indice >= 0)
        return (o[var[m].object].v0 + var[m].indice);
      /* fall through */
    case tpigl:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0) + var[m].indice);
      else
        return (o[var[m].object].v0);
    case tbglo:
    case tbloc:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      return (1);
    case twglo:
    case twloc:
    case tpwgl:
    case tpwlo:
      return (2);
    case tsglo:
      return (o[var[m].object].v0 + var[m].indice * o[var[m].object].v1);
    case tpsgl:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0) + var[m].indice * o[o[var[m].object].v1].v1);
      else
        return (o[var[m].object].v0);
    case tvloc:
      return (o[var[m].object].v0 + ids[ids_select]);
    case tcloc:
      return (o[var[m].object].v0 + ids[ids_select]);
    case ttloc:
      return (o[var[m].object].v0 + ids[ids_select] + var[m].indice);
    case tpilo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + ids[ids_select]) + var[m].indice);
      else
        return (o[var[m].object].v0 + ids[ids_select]);
    case tsloc:
      return (o[var[m].object].v0 + var[m].indice * o[var[m].object].v1 + ids[ids_select]);
    case tpslo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + ids[ids_select]) +
                var[m].indice * o[o[var[m].object].v1].v1);
      else
        return (o[var[m].object].v0 + ids[ids_select]);
    }
  else {
    switch (o[var[m].object].type) {
    case tvglo:
    case tvloc:
      return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    case tcglo:
    case tcloc:
      return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    case ttglo:
    case ttloc:
      return (o[var[m].object].v0 + get_offset(var[m].miembro - 1) + var[m].indice);
    case tpigl:
    case tpilo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) + var[m].indice);
      else
        return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    case tbglo:
    case tbloc:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      return (1);
    case twglo:
    case twloc:
    case tpwgl:
    case tpwlo:
      return (2);
    case tsglo:
    case tsloc:
      return (o[var[m].object].v0 + var[m].indice * o[var[m].object].v1 +
              get_offset(var[m].miembro - 1));
    case tpsgl:
    case tpslo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) +
                var[m].indice * o[o[var[m].object].v1].v1);
      else
        return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    }
  }
  return (0);
}

byte *_get_offset_byte(int m) {
  if (var[m].miembro == 0)
    switch (o[var[m].object].type) {
    case tbglo:
      return (&memb[o[var[m].object].v0 * 4 + var[m].indice]);
    case tbloc:
      return (&memb[o[var[m].object].v0 * 4 + var[m].indice + ids[ids_select] * 4]);
    case tpbgl:
    case tpcgl:
      if (var[m].indice >= 0)
        return (&memb[memo(o[var[m].object].v0) * 4 + var[m].indice]);
      else
        return (&memb[o[var[m].object].v0 * 4]);
    case tpblo:
    case tpclo:
      if (var[m].indice >= 0)
        return (&memb[memo(o[var[m].object].v0 + ids[ids_select]) * 4 + var[m].indice]);
      else
        return (&memb[o[var[m].object].v0 * 4 + ids[ids_select] * 4]);
    }
  else
    switch (o[var[m].object].type) {
    case tbglo:
    case tbloc:
    case tpcgl:
    case tpclo:
      return (&memb[o[var[m].object].v0 * 4 + var[m].indice + get_offset(var[m].miembro - 1) * 4]);
    case tpbgl:
    case tpblo:
      if (var[m].indice >= 0)
        return (
            &memb[memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) * 4 + var[m].indice]);
      else
        return (&memb[o[var[m].object].v0 * 4 + get_offset(var[m].miembro - 1) * 4]);
    }
  return (0);
}

byte *get_offset_byte(int m) {
  return (capar_byte(_get_offset_byte(m)));
}

word *_get_offset_word(int m) {
  if (var[m].miembro == 0)
    switch (o[var[m].object].type) {
    case twglo:
      return ((word *)&memb[o[var[m].object].v0 * 4 + var[m].indice * 2]);
    case twloc:
      return ((word *)&memb[o[var[m].object].v0 * 4 + var[m].indice * 2 + ids[ids_select] * 4]);
    case tpwgl:
      if (var[m].indice >= 0)
        return ((word *)&memb[memo(o[var[m].object].v0) * 4 + var[m].indice * 2]);
      else
        return ((word *)&memb[o[var[m].object].v0 * 4]);
    case tpwlo:
      if (var[m].indice >= 0)
        return ((word *)&memb[memo(o[var[m].object].v0 + ids[ids_select]) * 4 + var[m].indice * 2]);
      else
        return ((word *)&memb[o[var[m].object].v0 * 4 + ids[ids_select] * 4]);
    }
  else
    switch (o[var[m].object].type) {
    case twglo:
    case twloc:
      return ((word *)&memb[o[var[m].object].v0 * 4 + var[m].indice * 2 +
                            get_offset(var[m].miembro - 1) * 4]);
    case tpwgl:
    case tpwlo:
      if (var[m].indice >= 0)
        return ((word *)&memb[memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) * 4 +
                              var[m].indice * 2]);
      else
        return ((word *)&memb[o[var[m].object].v0 * 4 + get_offset(var[m].miembro - 1) * 4]);
    }
  return (0);
}

word *get_offset_word(int m) {
  return (capar_word(_get_offset_word(m)));
}

//-----------------------------------------------------------------------------
//      Change the value of a variable (in Inspect)
//-----------------------------------------------------------------------------

#define y_bt 34
char buscar[32];
int valor;

void change1(void) {
  _show_items();
}

void change2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    if (get_offset(var_select) == 1) {
      if (atoi(buscar) < 0 || atoi(buscar) > 255) {
        v_text = (char *)text[54];
        show_dialog(err0);
      } else {
        *get_offset_byte(var_select) = (byte)atoi(buscar);
        end_dialog = 1;
      }
    } else if (get_offset(var_select) == 2) {
      if (atoi(buscar) < 0 || atoi(buscar) > 65535) {
        v_text = (char *)text[55];
        show_dialog(err0);
      } else {
        *get_offset_word(var_select) = (word)atoi(buscar);
        end_dialog = 1;
      }
    } else {
      mem[get_offset(var_select)] = atoi(buscar);
      end_dialog = 1;
    }
    break;
  case 2:
    end_dialog = 1;
    break;
  }
}

void change0(void) {
  v.type = 1;
  v.title = text[56];
  v.w = 126;
  v.h = 14 + y_bt;
  v.paint_handler = change1;
  v.click_handler = change2;

  if (get_offset(var_select) == 1) {
    valor = *get_offset_byte(var_select);
  } else if (get_offset(var_select) == 2) {
    valor = *get_offset_word(var_select);
  } else {
    valor = memo(get_offset(var_select));
  }

  itoa(valor, buscar, 10);
  _get((byte *)text[57], 4, 11, v.w - 8, (byte *)buscar, 32, 0, 0);
  _button(text[7], 7, y_bt, 0);
  _button(text[58], v.w - 8, y_bt, 2);
}

//-----------------------------------------------------------------------------
//      Change the value of a string (in Inspect)
//-----------------------------------------------------------------------------

char enterstring[256];

void changestring1(void) {
  _show_items();
}

void changestring2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    if (strlen(enterstring) <= o[var[var_select].object].v1 + 1)
      div_strcpy((char *)&mem[get_offset(var_select)], o[var[var_select].object].v1 + 1,
                 enterstring);
    else {
      v_text = (char *)text[59];
      show_dialog(err0);
    }
    end_dialog = 1;
    break;
  case 2:
    end_dialog = 1;
    break;
  }
}

void changestring0(void) {
  v.type = 1;
  v.title = text[60];
  v.w = 226;
  v.h = 14 + y_bt;
  v.paint_handler = changestring1;
  v.click_handler = changestring2;
  if (strlen((char *)&mem[get_offset(var_select)]) < 256)
    div_strcpy(enterstring, sizeof(enterstring), (char *)&mem[get_offset(var_select)]);
  else
    div_strcpy(enterstring, sizeof(enterstring), "");
  _get((byte *)text[61], 4, 11, v.w - 8, (byte *)enterstring, 256, 0, 0);
  _button(text[7], 7, y_bt, 0);
  _button(text[58], v.w - 8, y_bt, 2);
}

//----------------------------------------------------------------------------
//	Main debugging window
//----------------------------------------------------------------------------

void debug1(void) {
  char msg[256];
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2, n;

  wbox(ptr, w, h, c2, 2, 10, w - 4, h - 12);
  wbox(ptr, w, h, c12, 2, 10, w - 4, 144 - 16 - 32 - 10);

  _show_items2();
  itoa(procesos, msg, 10);
  div_strcat(msg, sizeof(msg), "/");
  itoa((imem_max - id_start) / iloc_len, msg + strlen(msg), 10);

  div_strcat(msg, sizeof(msg), (char *)text[62]);
  wwrite(ptr, w, h, 4, 11, 0, (byte *)msg, c1);
  wwrite(ptr, w, h, 3, 11, 0, (byte *)msg, c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128, 128 - 5 - 16 - 32); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122, 19, 9, 128 - 5 - 16 - 32);
  wrectangle(ptr, w, h, c0, 122, 27, 9, 112 - 5 - 16 - 32);

  wrectangle(ptr, w, h, c0, 133, 19, 120 + 304 - 256, 11); // Process name box

  wrectangle(ptr, w, h, c0, 133, 31, 48, 48); // Graphic box

  wput(ptr, w, h, 123, 20, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, 123, 139 - 5 - 16 - 32, -40);

  determine_ids();

  for (n = 0; n < iids; n++)
    if (ids[n] == ids_old)
      break;
  if (n < iids) { // If the previously selected process is still in the list ...
    ids_select = n;
    ids_ini = ids_select - ids_inc;
    if (ids_ini && ids_ini + 13 > iids)
      ids_ini = iids - 13;
    if (ids_ini < 0)
      ids_ini = 0;
  } else {
    if (ids_next + 12 <= iids) {
      if (ids_next)
        ids_ini = ids_next - 1;
      else
        ids_ini = 0;
    } else if ((ids_ini = iids - 13) < 0)
      ids_ini = 0;
    ids_select = ids_next;
  }

  draw_proc_list();
  determine_code();

  wbox(ptr, w, h, c0, 1, 144 - 16 - 32, w - 2, 1);
  wrectangle(ptr, w, h, c0, 47, 146 - 16 - 32, w - 50, 43 + 16 + 32);
}

void paint_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 129 - 16 - 32, n;
  float x;

  wbox(ptr, w, h, c2, 123, 28, 7, 105 - 16 - 32); // Clear the slider bar

  if (iids == 1)
    n = min;
  else {
    x = (float)ids_select / (float)(iids - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122, n, 9, 5);
  wput(ptr, w, h, 123, n + 1, -43);
}

byte *change_mode(void) {
  new_mode = 0;
  free(v.ptr);
  v.w = 304 * big2;
  v.h = (146 + 46) * big2;
  if ((v.ptr = (byte *)malloc(v.w * v.h)) == NULL)
    exer(1);
  if (v.x < 0) {
    v.x = 0;
  }
  if (v.y < 0) {
    v.y = 0;
  }
  if (v.x + v.w > vga_width)
    v.x = vga_width - v.w;
  if (v.y + v.h > vga_height)
    v.y = vga_height - v.h;
  repaint_window();
  call(v.paint_handler);
  v.redraw = 1;
  full_redraw = 1;
  return (v.ptr);
}

void debug2(void) {
  int n, m, iip; //,id;
  float x;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;

  _process_items();

  if (no_volcar_nada)
    goto next_frame;

  if (!(shift_status & 15) && ascii == 0) {
    if (scan_code == 80) {
      f_down();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 72) {
      f_up();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 81) {
      for (n = 0; n < 11; n++)
        f_down();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 73) {
      for (n = 0; n < 11; n++)
        f_up();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 71) {
      f_home();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 77) {
      f_right();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 75) {
      f_left();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }

  } else if (shift_status & 15) { // (Shift || Control || Alt)

    if (kbdFLAGS[80]) {
      kbdFLAGS[80] = 0;
      if (ids_select + 1 < iids) {
        if (ids_ini + 9 == ++ids_select)
          ids_ini++;
        draw_proc_list();
        v.redraw = 1;
      }
    }
    if (kbdFLAGS[72]) {
      kbdFLAGS[72] = 0;
      if (ids_select) {
        if (ids_ini == ids_select--)
          ids_ini--;
        draw_proc_list();
        v.redraw = 1;
      }
    }
    if (scan_code == 81 || scan_code == 118) {
      for (n = 0; n < 9; n++)
        if (ids_select + 1 < iids) {
          if (ids_ini + 9 == ++ids_select)
            ids_ini++;
        }
      draw_proc_list();
      v.redraw = 1;
    }
    if (scan_code == 73 || scan_code == 132) {
      for (n = 0; n < 9; n++)
        if (ids_select) {
          if (ids_ini == ids_select--)
            ids_ini--;
        }
      draw_proc_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(3, 21, 128 - 9, 120 - 16 - 32) && (mouse_b & 1)) {
    n = ids_ini + (wmouse_y - 21) / 8;
    if (n < iids) {
      ids_select = n;
      draw_proc_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (boton == 0) {
        wput(ptr, w, h, 123, 20, -41);
        boton = 1;
        if (ids_select) {
          if (ids_ini == ids_select--)
            ids_ini--;
          draw_proc_list();
          v.redraw = 1;
        }
      }
    } else if (boton == 1) {
      wput(ptr, w, h, 123, 20, -39);
      boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (boton == 1) {
    wput(ptr, w, h, 123, 20, -39);
    boton = 0;
    v.redraw = 1;
  }

  if (wmouse_in(123, 28, 7, 105 - 16 - 32)) {
    mouse_graf = 13;
    if (iids > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - 28) / (104.0 - 16.0 - 32.0);
      ids_select = x * (iids - 1);
      if (ids_select < ids_ini)
        ids_ini = ids_select;
      if (ids_select >= ids_ini + 9)
        ids_ini = ids_select - 8;
      draw_proc_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122, 133 - 16 - 32, 9, 9)) {
    if (mouse_b & 1) {
      if (boton == 0) {
        wput(ptr, w, h, 123, 134 - 16 - 32, -42);
        boton = 2;
        if (ids_select + 1 < iids) {
          if (ids_ini + 9 == ++ids_select)
            ids_ini++;
          draw_proc_list();
          v.redraw = 1;
        }
      }
    } else if (boton == 2) {
      wput(ptr, w, h, 123, 134 - 16 - 32, -40);
      boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (boton == 2) {
    wput(ptr, w, h, 123, 134 - 16 - 32, -40);
    boton = 0;
    v.redraw = 1;
  }

  if ((mouse_b & 1) && wmouse_in(48 + 5, 147 - 16 - 32, w - 52 - 5, 41 + 16 + 32)) {
    linea_sel = linea0 + (wmouse_y - (147 - 16 - 32)) / 8;
    if (linea_sel == linea0 + 11)
      linea_sel = linea0 + 10;
    paint_code();
    v.redraw = 1;
  }

  if (scan_code == _F4)
    goto go_here;
  if (scan_code == _F5)
    goto goto_proc;
  if (scan_code == _F6)
    goto exec_proc;
  if (scan_code == _F7)
    goto inspect_proc;
  if (scan_code == _F8)
    goto trace_proc;
  if (scan_code == _F9)
    goto set_break;
  if (scan_code == _F10)
    goto step_proc;
  if (scan_code == 32 || kbdFLAGS[_F11])
    goto profile_window;
  if (scan_code == 33 || kbdFLAGS[_F12])
    goto next_frame;

  switch (v.active_item) {
  case 0: // Next frame
next_frame:
    ids_old = ids[ids_select];
    ids_inc = ids_select - ids_ini;
    do {
      exec_process();
      if (new_mode)
        ptr = change_mode();
      if (call_to_debug) {
        call(v.paint_handler);
        v.redraw = 1;
        break;
      }
    } while (ide);
    if (call_to_debug) {
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      break;
    }
    if (procesos) {
      frame_clock = reloj_debug;
      ticks = ticks_debug;
      set_mouse(smouse_x, smouse_y);
      blits_skipped = 0;
      skip_blit = 0;
      dacout_r = dr;
      dacout_g = dg;
      dacout_b = db;
      debugger_step = 1;
      frame_end();
      frame_start();
      debugger_step = 0;
      reloj_debug = frame_clock;
      ticks_debug = ticks;
      smouse_x = mouse->x;
      smouse_y = mouse->y;
      set_mouse(mouse_x, mouse_y);
      memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
      call(v.paint_handler);
      for (n = 0; n < iids; n++)
        if (ids[n] == ids_old)
          break;
      if (n < iids) { // If the previously selected process is still in the list ...
        ids_select = n;
        ids_ini = ids_select - ids_inc;
        if (ids_ini && ids_ini + 13 > iids)
          ids_ini = iids - 13;
        if (ids_ini < 0)
          ids_ini = 0;
        draw_proc_list();
        determine_code();
      }
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      dread_mouse();
      _process_items();
      v.redraw = 1;
      full_redraw = 1;
      if (no_volcar_nada)
        show_dialog(profile0);
    } else
      end_dialog = 1;
    break;
  case 1: // Goto
goto_proc:
    show_dialog(process_list0);
    //int linea0;     // First line number in the debugger code window
    //byte * plinea0; // Pointer to the first line in the debugger code window
    //int linea_sel; // Selected line number
    if (v_accept) {
      x_inicio = 54;
      while (linea0 > lp1[lp_select]) {
        linea0--;
        plinea0--;
        do {
          plinea0--;
        } while (*plinea0);
        plinea0++;
      }
      while (linea0 < lp1[lp_select]) {
        linea0++;
        plinea0 += strlen((char *)plinea0) + 1;
      }
      linea_sel = linea0;
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    break;
  case 2: // Breakpoint
set_break:
    for (n = 0; n < max_breakpoint; n++)
      if (breakpoint[n].line == linea_sel)
        break;
    if (n < max_breakpoint) { // Deactivate a breakpoint
      breakpoint[n].line = -1;
      mem[abs(breakpoint[n].offset)] = breakpoint[n].code;
      paint_code();
      v.redraw = 1;
    } else {
      for (n = 0; n < max_breakpoint; n++)
        if (breakpoint[n].line == -1)
          break;
      if (n < max_breakpoint) {
        if ((m = get_ip(linea_sel)) >= 0) {
          breakpoint[n].line = linea_sel;
          breakpoint[n].offset = m;
          breakpoint[n].code = mem[m];
          mem[m] = ldbg;
          paint_code();
          v.redraw = 1;
        }
      } else {
        v_text = (char *)text[63];
        show_dialog(err0);
      }
    }
    break;
  case 3: // Go here!
go_here:
    for (n = 0; n < max_breakpoint; n++)
      if (breakpoint[n].line == -1)
        break;
    if (n < max_breakpoint) {
      if ((m = get_ip(linea_sel)) >= 0) {
        breakpoint[n].code = linea_sel;
        do {
          trace_process();
          if (new_mode)
            ptr = change_mode();
          if (call_to_debug) {
            call(v.paint_handler);
            v.redraw = 1;
            break;
          }
        } while (ide && ((ip >= mem1 && ip <= mem2) || mem[ip] == lasp || mem[ip] == lasiasp ||
                         mem[ip] == lcarasiasp || mem[ip] == lfunasp));
        if (call_to_debug) {
          full_redraw = 1;
          call_to_debug = 0;
          if (new_palette) {
            new_palette = 0;
            repaint_window();
          }
          break;
        }
        breakpoint[n].line = breakpoint[n].code;
        breakpoint[n].offset = -m;
        breakpoint[n].code = mem[m];
        mem[m] = ldbg;
        end_dialog = 1;
      }
    } else {
      v_text = (char *)text[63];
      show_dialog(err0);
    }
    break;
  case 4: // Trace
trace_proc:
    do {
      trace_process();
      if (new_mode)
        ptr = change_mode();
      if (call_to_debug) {
        call(v.paint_handler);
        v.redraw = 1;
        break;
      }
    } while (ide && ((ip >= mem1 && ip <= mem2) || mem[ip] == lasp || mem[ip] == lasiasp ||
                     mem[ip] == lcarasiasp || mem[ip] == lfunasp));
    if (call_to_debug) {
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      break;
    }
    if (procesos) {
      if (procesos_ejecutados()) {
        frame_clock = reloj_debug;
        ticks = ticks_debug;
        set_mouse(smouse_x, smouse_y);
        blits_skipped = 0;
        skip_blit = 0;
        dacout_r = dr;
        dacout_g = dg;
        dacout_b = db;
        debugger_step = 1;
        frame_end();
        frame_start();
        debugger_step = 0;
        reloj_debug = frame_clock;
        ticks_debug = ticks;
        smouse_x = mouse->x;
        smouse_y = mouse->y;
        set_mouse(mouse_x, mouse_y);
        memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
        full_redraw = 1;
      } else if (new_palette)
        set_dac();
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      ids_old = -1;
      call(v.paint_handler);
      v.redraw = 1;
    } else
      end_dialog = 1;
    break;
  case 5: // Step
step_proc:

    // If it calls a "function", do a Go Here! at the end

    for (iip = mem1; iip < mem2; iip++) {
      // WARNING: To detect if the next statement calls a function, we scan
      // for the 'lcal' opcode (value 26). This is imprecise because a
      // constant integer with the same value could cause a false match.

      if (mem[iip] == lcal)
        if (memo(mem[iip + 1]) == ltyp && memo(mem[iip + 1] + 2) == lnop) {
          for (n = 0; n < max_breakpoint; n++)
            if (breakpoint[n].line == -1)
              break;
          if (n < max_breakpoint) {
            breakpoint[n].offset = mem2 + 1;
            if (mem[breakpoint[n].offset] == lasp)
              breakpoint[n].offset++;
            breakpoint[n].line = 0;
            get_line(breakpoint[n].offset);
            breakpoint[n].code = mem[breakpoint[n].offset];
            mem[breakpoint[n].offset] = ldbg;
            end_dialog = 1;
            break;
          }
        }
    } // If no breakpoints left, it will do a Trace.

    process_level = 0;
    do {
      trace_process();
      if (new_mode)
        ptr = change_mode();
      if (call_to_debug) {
        call(v.paint_handler);
        v.redraw = 1;
        break;
      }
    } while (ide && ((ip >= mem1 && ip <= mem2) || mem[ip] == lasp || mem[ip] == lasiasp ||
                     mem[ip] == lcarasiasp || mem[ip] == lfunasp || process_level > 0));
    if (call_to_debug) {
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      break;
    }
    if (procesos) {
      if (procesos_ejecutados()) {
        frame_clock = reloj_debug;
        ticks = ticks_debug;
        set_mouse(smouse_x, smouse_y);
        blits_skipped = 0;
        skip_blit = 0;
        dacout_r = dr;
        dacout_g = dg;
        dacout_b = db;
        debugger_step = 1;
        frame_end();
        frame_start();
        debugger_step = 0;
        reloj_debug = frame_clock;
        ticks_debug = ticks;
        smouse_x = mouse->x;
        smouse_y = mouse->y;
        set_mouse(mouse_x, mouse_y);
        memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
        full_redraw = 1;
      } else if (new_palette)
        set_dac();
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      ids_old = -1;
      call(v.paint_handler);
      v.redraw = 1;
    } else
      end_dialog = 1;
    break;
  case 6: // Inspect
inspect_proc:
    current_scope = mem[ids[ids_select] + _Bloque];
    show_dialog(inspect0);
    draw_proc_list();
    v.redraw = 1;
    break;

  case 7: // Profile
profile_window:
    show_dialog(profile0);
    break;
  case 8: // Ex. Process
exec_proc:
    exec_process();
    if (new_mode)
      ptr = change_mode();
    if (call_to_debug) {
      call(v.paint_handler);
      v.redraw = 1;
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
    }
    if (procesos) {
      if (procesos_ejecutados()) {
        frame_clock = reloj_debug;
        ticks = ticks_debug;
        set_mouse(smouse_x, smouse_y);
        blits_skipped = 0;
        skip_blit = 0;
        dacout_r = dr;
        dacout_g = dg;
        dacout_b = db;
        debugger_step = 1;
        frame_end();
        frame_start();
        debugger_step = 0;
        reloj_debug = frame_clock;
        ticks_debug = ticks;
        smouse_x = mouse->x;
        smouse_y = mouse->y;
        set_mouse(mouse_x, mouse_y);
        memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
        full_redraw = 1;
      } else if (new_palette)
        set_dac();
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      ids_old = -1;
      call(v.paint_handler);
      v.redraw = 1;
    } else
      end_dialog = 1;
    break;
  }
}

void debug3(void) {
  ids_old = ids[ids_select];
  ids_inc = ids_select - ids_ini;
}

void debug0(void) {
  v.type = 1;
  v.title = text[64];
  v.w = 304;
  v.h = 146 + 46; // TODO: Keep in sync with change_mode dialog dimensions
  v.paint_handler = debug1;
  v.click_handler = debug2;
  v.close_handler = debug3;

  _button(text[65], v.w - 8, 102 - 18, 2);
  _button(text[66], 7 + 17, v.h - 14 - 15 * 4, 35 * 256 + 1);
  _button(text[67], 7 + 17, v.h - 14 - 15 * 3, 35 * 256 + 1);
  _button(text[68], 7 + 17, v.h - 14 - 15 * 2, 35 * 256 + 1);
  _button(text[69], 7 + 17, v.h - 14 - 15 * 1, 35 * 256 + 1);
  _button(text[70], 7 + 17, v.h - 14 - 15 * 0, 35 * 256 + 1);
  _button(text[71], v.w - 40, 35, 1);
  _button(text[72], 136, 102 - 18, 0);
  _button(text[73],
          (v.w - 8 - text_len((byte *)"\15Sgte. Frame") + 136 + text_len((byte *)"\15Perfiles")) /
              2,
          102 - 18, 1);
}

//----------------------------------------------------------------------------
//  Functions that paint source code in the debugger window
//----------------------------------------------------------------------------

void get_line(int n) { // From an IP address, get the statement position
  int x = 0;
  if (line == NULL)
    return;
  for (x = 0; x < num_sentencias; x++)
    if (n >= line[x * 6] && n <= line[x * 6 + 1])
      break;
  if (x < num_sentencias) {
    mem1 = line[x * 6];
    mem2 = line[x * 6 + 1];
    linea1 = line[x * 6 + 2] - 1;
    columna1 = line[x * 6 + 3];
    linea2 = line[x * 6 + 4] - 1;
    columna2 = line[x * 6 + 5];
  } else {
    mem1 = line[0];
    mem2 = line[1];
    linea1 = line[2] - 1;
    columna1 = line[3];
    linea2 = line[4] - 1;
    columna2 = line[5];
  }
}

int get_ip(int n) { // From a line number, get the IP address of the statement
  int x = 0;
  if (line == NULL)
    return (0);
  for (x = 0; x < num_sentencias; x++)
    if (n == line[x * 6 + 2] - 1)
      break;
  if (x < num_sentencias)
    return (line[x * 6]);
  else
    return (-1);
}

void determine_code(void) { // Determine what to show for "ids_next"
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2, l;
  char msg[256];

  if (line == NULL)
    return;

  wbox(ptr, w, h, c1, 4, 147 - 16 - 32, 41, 8); // Clear the space for writing the ID
  wrectangle(ptr, w, h, c0, 3, 146 - 16 - 32, 43, 10);
  itoa(ids[ids_next], msg, 10);
  wwrite_in_box(ptr, w, 46, h, 24, 148 - 16 - 32, 1, (byte *)msg, c0);
  wwrite_in_box(ptr, w, 46, h, 23, 148 - 16 - 32, 1, (byte *)msg, c34);

  if (process_stoped)
    get_line(ip);
  else
    get_line(mem[ids[ids_next] + _IP]); // Get line/column/mem 1/2

  l = linea0 = linea1 - 3;
  if (l < 0)
    l = linea0 = 0;

  plinea0 = source + 1;
  while (l--)
    plinea0 += strlen((char *)plinea0) + 1;

  linea_sel = linea1;

  paint_code();
}

void paint_code(void) { // Paint the source code
  byte *p = plinea0, c;
  int n, x, l = linea0;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;

  if (line == NULL)
    return;

  wbox(ptr, w, h, c1, 48 + 5, 147 - 16 - 32, w - 52 - 5, 41 + 16 + 32); // Clear the window

  wbox(ptr, w, h, c12, 48, 147 - 16 - 32, 5, 41 + 16 + 32);

  wrectangle(ptr, w, h, c0, 47 + 5, 146 - 16 - 32, 1, 43 + 16 + 32);

  for (n = 0; n < 11; n++, l++) {
    if (p >= end_source)
      break;
    if (l == linea_sel)
      wbox(ptr, w, h, c0, 48 + 5, 147 - 16 - 32 + (linea_sel - linea0) * 8, w - 52 - 5, 9);
    for (x = 0; x < max_breakpoint; x++)
      if (breakpoint[x].line == l)
        wbox(ptr, w, h, c_r_low, 48 + 5, 148 - 16 - 32 + (l - linea0) * 8, w - 52 - 5, 7);
    if (l == linea1) {
      wput(ptr, w, h, 48, 148 - 16 - 32 + n * 8, 36);
      c = *(p + columna1);
      *(p + columna1) = 0;
      x = x_inicio - 54;
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p, c3);
      if (text_len(p)) {
        x += text_len(p) + 1;
      }
      *(p + columna1) = c;
      if (l == linea2) {
        c = *(p + columna2 + 1);
        *(p + columna2 + 1) = 0;
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x + 1, 148 - 16 - 32 + n * 8, 0,
                      p + columna1, c0);
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + columna1,
                      c4);
        x += text_len(p + columna1) + 1;
        *(p + columna2 + 1) = c;
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0,
                      p + columna2 + 1, c3);
      } else {
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x + 1, 148 - 16 - 32 + n * 8, 0,
                      p + columna1, c0);
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + columna1,
                      c4);
      }
    } else if (l == linea2) {
      c = *(p + columna2 + 1);
      *(p + columna2 + 1) = 0;
      x = x_inicio - 54;
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x + 1, 148 - 16 - 32 + n * 8, 0, p, c0);
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p, c4);
      x += text_len(p) + 1;
      *(p + columna2 + 1) = c;
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + columna2 + 1,
                    c3);
    } else if (l > linea1 && l < linea2) {
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x_inicio - 54 + 1, 148 - 16 - 32 + n * 8, 0,
                    p, c0);
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x_inicio - 54, 148 - 16 - 32 + n * 8, 0, p,
                    c4);
    } else {
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x_inicio - 54, 148 - 16 - 32 + n * 8, 0, p,
                    c3);
    }
    p += strlen((char *)p) + 1;
  }
}

void f_home(void) {
  x_inicio = 54;
}

void f_right(void) {
  x_inicio -= 6;
}

void f_left(void) {
  if (x_inicio < 54)
    x_inicio += 6;
}

void f_up(void) {
  if (linea_sel) {
    linea_sel--;
    if (linea_sel < linea0) {
      linea0--;
      plinea0--;
      do {
        plinea0--;
      } while (*plinea0);
      plinea0++;
    }
  }
}

void f_down(void) {
  byte *p;
  int n;

  n = linea_sel - linea0 + 1;
  p = (byte *)plinea0;
  while (n-- && p < end_source)
    p += strlen((char *)p) + 1;
  if (p < end_source) {
    linea_sel++;
    if (linea_sel == linea0 + 11) {
      linea0++;
      plinea0 += strlen((char *)plinea0) + 1;
    }
  }
}

//-----------------------------------------------------------------------------
// Process list dialog
//-----------------------------------------------------------------------------

extern byte strlower[256];

void create_process_list(void) {
  byte *p, *q;
  char cwork[512];
  int linea = 0, n;

  p = source + 1;
  lp_num = 0;
  lp_ini = 0;
  lp_select = 0;

  do {
    while (*p && p < end_source) {
      p++;
    }
    p++;
    linea++;
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
                lp1[n] = linea;
                lp2[n] = (char *)p;
              } else {
                lp1[lp_num] = linea;
                lp2[lp_num] = (char *)p;
              }
            } else {
              lp1[lp_num] = linea;
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

  if (wmouse_in(3, 21, 128 + 32 - 9, 120) && (mouse_b & 1)) {
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

  if (wmouse_in(122 + 32, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, 123 + 32, 20, -41);
        lp_boton = 1;
        if (lp_select) {
          if (lp_ini == lp_select--)
            lp_ini--;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 1) {
      wput(ptr, w, h, 123 + 32, 20, -39);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp_boton == 1) {
    wput(ptr, w, h, 123 + 32, 20, -39);
    lp_boton = 0;
    v.redraw = 1;
  }


  if (wmouse_in(123 + 32, 28, 7, 105)) {
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

  if (wmouse_in(122 + 32, 93 + 40, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, 123 + 32, 94 + 40, -42);
        lp_boton = 2;
        if (lp_select + 1 < lp_num) {
          if (lp_ini + 15 == ++lp_select)
            lp_ini++;
          paint_process_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 2) {
      wput(ptr, w, h, 123 + 32, 94 + 40, -40);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp_boton == 2) {
    wput(ptr, w, h, 123 + 32, 94 + 40, -40);
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
    create_process_list();
    paint_process_list();
    v.redraw = 1;
    break;
  }
}

void process_list0(void) {
  v.type = 1;
  v.title = text[75];

  v.w = 166;
  v.h = 161;
  v.paint_handler = process_list1;
  v.click_handler = process_list2;

  _button(text[7], 7, v.h - 14, 0);
  _button(text[58], v.w - 8, v.h - 14, 2);
  _flag(text[76], v.w - text_len(text[76]) - 12, 11, &lp_sort);
  v_accept = 0;
}

//-----------------------------------------------------------------------------
// Profile - Execution time profiling
//-----------------------------------------------------------------------------

//int lp1[512];     // Object number for each process
//int lp_num;       // Number of process blocks in the list
//int lp_ini;       // First block displayed in the window
//int lp_select;    // Selected process block

//int obj_start; // Start of the first object (&obj[0])
//int obj_size;  // Size of each object (struct object)
//(The block of an ID is: (mem[ID+_Bloque]-obj_start)/obj_size;)

//unsigned f_time[256]; // Time consumed by the different functions
unsigned f_time_total;
unsigned f_exec_total;
unsigned f_paint_total;
unsigned f_max;

int lp2_num;
int lp2_ini;
int lp2_select; //???

//----------------------------------------------------------------------------

unsigned int get_tick(void);

#ifdef __WATCOMC__
#pragma aux get_tick = "mov ecx,10h" \
                       "db 0fh,032h" parm[] value[eax] modify[eax ecx edx]
#endif

// "rdmsr"
// "mov cl,4"
// "shrd eax,edx,cl"

void reset_tick(void);

#ifdef __WATCOMC__
#pragma aux reset_tick = "mov ecx,10h" \
                         "xor edx,edx" \
                         "xor eax,eax" \
                         "db 0fh,030h" parm[] modify[ecx eax edx]
#endif

// "wrmsr"

#define IntIncr (unsigned int)(11932 / 256)

unsigned int get_ticks(void) {
  return OSDEP_GetTicks();
}

void function_exec(int id, int n) { // Number, cycles
  if (n > 0)
    f_time[id] += n;
}

void process_exec(int id, int n) { // Id, cycles
  if (!debug_active)
    return;
  if (n > 0)
    o[(mem[id + _Bloque] - obj_start) / obj_size].v4 += n;
}

void process_paint(int id, int n) { // Id, cycles
  if (!debug_active)
    return;
  if (n > 0)
    o[(mem[id + _Bloque] - obj_start) / obj_size].v5 += n;
}

//----------------------------------------------------------------------------

#define lpy   (24 + 15) // Listbox with process profiles
#define lpnum 6
#define lpal  10

#define lp2y   (86 + 15) // Listbox with process profiles
#define lp2num 6
#define lp2al  10
#define lp2esp 98

int absolut = 0;

void create_profile_list(void) {
  int n;

  lp_num = 0;
  lp_ini = 0;
  lp_select = 0; // Processes
  lp2_num = 0;
  lp2_ini = 0;
  lp2_select = 0; // Functions

  f_exec_total = 0;
  f_paint_total = 0;
  f_max = 0;

  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tproc) {
      f_exec_total += o[n].v4;
      f_paint_total += o[n].v5;
      if (o[n].v4 > f_max)
        f_max = o[n].v4;
      if (o[n].v5 > f_max)
        f_max = o[n].v5;
    }
  }

  for (n = 0; n < 240; n++) {
    if (f_time[n] > f_max)
      f_max = f_time[n];
  }

  f_exec_total += f_time[255];
  f_paint_total += f_time[254];
  f_paint_total += f_time[253];
  f_paint_total += f_time[252];
  f_paint_total += f_time[251];
  f_paint_total += f_time[250];

  if (f_time[255] > f_max)
    f_max = f_time[255];
  if (f_time[254] > f_max)
    f_max = f_time[254];
  if (f_time[253] > f_max)
    f_max = f_time[253];
  if (f_time[252] > f_max)
    f_max = f_time[252];
  if (f_time[251] > f_max)
    f_max = f_time[251];
  if (f_time[250] > f_max)
    f_max = f_time[250];

  f_time_total = f_exec_total + f_paint_total;

  if (f_time_total == 0) {
    f_time_total = 1;
    f_max = 1;
  }

  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tproc && (o[n].v4 > f_time_total / 10000 || o[n].v5 > f_time_total / 10000)) {
      lp1[lp_num++] = n;
    }
  }

  for (n = 0; n < 240; n++) {
    if (f_time[n] > f_time_total / 10000) {
      lp2[lp2_num++] = (char *)n;
    }
  }
}

void paint_profile_list(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int m, x, porcen, porcen2;
  char cwork[256];

  wbox(ptr, w, h, c1, 4, lpy, w - 16, lpnum * lpal - 1); // Process listbox bounds
  for (m = lp_ini; m < lp_ini + lpnum && m < lp_num; m++) {
    if (m == lp_select) {
      wbox(ptr, w, h, c01, 4, lpy + (m - lp_ini) * lpal, w - 16 - 130,
           lpal - 1); // Process listbox fill
      x = c4;
    } else
      x = c3;
    wrectangle(ptr, w, h, c0, w - 12 - 130, lpy - 1 + (m - lp_ini) * lpal, 66,
               lpal + 1); // Bar bounds

    wbox(ptr, w, h, c_g_low0, w - 11 - 130, lpy + (m - lp_ini) * lpal, 64,
         lpal - 1); // Execution

    porcen = (unsigned)(((double)o[lp1[m]].v4 * (double)10000.0) / (double)f_time_total);
    porcen2 = (unsigned)(((double)o[lp1[m]].v4 * (double)10000.0) / (double)f_max);
    wbox(ptr, w, h, c_g_low, w - 11 - 130, lpy + (m - lp_ini) * lpal, (64 * porcen2) / 10000,
         lpal - 1);
    div_snprintf(cwork, sizeof(cwork), "%d.%02d%c", porcen / 100, porcen % 100, '%');
    if (absolut) {
      if (game_frames > 0)
        div_snprintf(cwork, sizeof(cwork), "%d", (int)((o[lp1[m]].v4 * 100) / game_frames));
      else
        div_snprintf(cwork, sizeof(cwork), "0");
    }

    wwrite(ptr, w, h, w - 10 - 130 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork,
           c_g_low0);
    wwrite(ptr, w, h, w - 11 - 130 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork, c34);

    wbox(ptr, w, h, c_r_low0, w - 11 - 65, lpy + (m - lp_ini) * lpal, 64, lpal - 1); // Rendering

    porcen = (unsigned)(((double)o[lp1[m]].v5 * (double)10000.0) / (double)f_time_total);
    porcen2 = (unsigned)(((double)o[lp1[m]].v5 * (double)10000.0) / (double)f_max);
    wbox(ptr, w, h, c_r_low, w - 11 - 65, lpy + (m - lp_ini) * lpal, (64 * porcen2) / 10000,
         lpal - 1);
    div_snprintf(cwork, sizeof(cwork), "%d.%02d%c", porcen / 100, porcen % 100, '%');
    if (absolut) {
      if (game_frames > 0)
        div_snprintf(cwork, sizeof(cwork), "%d", (int)((o[lp1[m]].v5 * 100) / game_frames));
      else
        div_snprintf(cwork, sizeof(cwork), "0");
    }

    wwrite(ptr, w, h, w - 10 - 65 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork,
           c_r_low0);
    wwrite(ptr, w, h, w - 11 - 65 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork, c34);

    wwrite_in_box(ptr, w, w - 13 - 131, h, 5, lpy + 1 + (m - lp_ini) * lpal, 0,
                  (byte *)vnom + o[lp1[m]].name, x);
    wbox(ptr, w, h, c0, 4, lpy + (m - lp_ini) * lpal + lpal - 1, w - 16, 1);
  }

  wbox(ptr, w, h, c1, 4, lp2y, w - lp2esp - 16, lp2num * lp2al - 1); // Process listbox bounds
  for (m = lp2_ini; m < lp2_ini + lp2num && m < lp2_num; m++) {
    if (m == lp2_select) {
      wbox(ptr, w, h, c01, 4, lp2y + (m - lp2_ini) * lp2al, w - lp2esp - 16 - 65,
           lp2al - 1); // Process listbox fill
      x = c4;
    } else
      x = c3;
    wrectangle(ptr, w, h, c0, w - lp2esp - 12 - 65, lp2y - 1 + (m - lp2_ini) * lp2al, 1,
               lp2al + 1); // Bar bounds

    wbox(ptr, w, h, c_g_low0, w - lp2esp - 11 - 65, lp2y + (m - lp2_ini) * lp2al, 64,
         lp2al - 1); // Execution

    porcen =
        (unsigned)(((double)f_time[(uintptr_t)lp2[m]] * (double)10000.0) / (double)f_time_total);
    porcen2 = (unsigned)(((double)f_time[(uintptr_t)lp2[m]] * (double)10000.0) / (double)f_max);
    wbox(ptr, w, h, c_g_low, w - lp2esp - 11 - 65, lp2y + (m - lp2_ini) * lp2al,
         (64 * porcen2) / 10000, lpal - 1);
    div_snprintf(cwork, sizeof(cwork), "%d.%02d%c", porcen / 100, porcen % 100, '%');
    if (absolut) {
      if (game_frames > 0)
        div_snprintf(cwork, sizeof(cwork), "%d",
                     (int)((f_time[(uintptr_t)lp2[m]] * 100) / game_frames));
      else
        div_snprintf(cwork, sizeof(cwork), "0");
    }

    wwrite(ptr, w, h, w - lp2esp - 10 - 33, lp2y + 1 + (m - lp2_ini) * lp2al, 1, (byte *)cwork,
           c_g_low0);
    wwrite(ptr, w, h, w - lp2esp - 11 - 33, lp2y + 1 + (m - lp2_ini) * lp2al, 1, (byte *)cwork,
           c34);

    wwrite_in_box(ptr, w, w - lp2esp - 13 - 66, h, 5, lp2y + 1 + (m - lp2_ini) * lp2al, 0,
                  (byte *)fname[(uintptr_t)lp2[m]], x);
    wbox(ptr, w, h, c0, 4, lp2y + (m - lp2_ini) * lp2al + lp2al - 1, w - lp2esp - 16, 1);
  }

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 0 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_g_low0, w - lp2esp - 1, lp2al * 0 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[255] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[255] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_g_low, w - lp2esp - 1, lp2al * 0 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[77], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[77],
                   (int)((f_time[255] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[77]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 0, 1, (byte *)cwork, c_g_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 0, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 1 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 1 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[254] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[254] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 1 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[78], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[78],
                   (int)((f_time[254] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[78]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 1, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 1, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 2 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 2 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[253] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[253] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 2 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[79], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[79],
                   (int)((f_time[253] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[79]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 2, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 2, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 3 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 3 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[252] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[252] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 3 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "Scroll %d.%02d%c", porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "Scroll %d", (int)((f_time[252] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "Scroll 0");
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 3, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 3, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 4 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 4 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[251] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[251] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 4 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[80], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[80],
                   (int)((f_time[251] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[80]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 4, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 4, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 5 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 5 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[250] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[250] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 5 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[81], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[81],
                   (int)((f_time[250] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[81]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 5, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 5, 1, (byte *)cwork, c34);

  paint_profile_segment();
}

void paint_profile_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = lpy + 7, max = lpy + lpnum * lpal - 13, n;
  float x;

  wbox(ptr, w, h, c2, w - 11, lpy + 8, 7, max - min + 3); // Clear the slider bar
  if (lp_num <= 1)
    n = min;
  else {
    x = (float)lp_select / (float)(lp_num - 1);
    n = min * (1 - x) + max * x;
  }
  wrectangle(ptr, w, h, c0, w - 12, n, 9, 5);
  wput(ptr, w, h, w - 11, n + 1, -43);

  min = lp2y + 7;
  max = lp2y + lp2num * lp2al - 13, n;

  wbox(ptr, w, h, c2, w - lp2esp - 11, lp2y + 8, 7, max - min + 3); // Clear the slider bar
  if (lp2_num <= 1)
    n = min;
  else {
    x = (float)lp2_select / (float)(lp2_num - 1);
    n = min * (1 - x) + max * x;
  }
  wrectangle(ptr, w, h, c0, w - lp2esp - 12, n, 9, 5);
  wput(ptr, w, h, w - lp2esp - 11, n + 1, -43);
}

void profile1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  char cwork[256];
  int n;
  unsigned x;

  _show_items();

  create_profile_list();

  wrectangle(ptr, w, h, c0, 3, 12, w - 6, 11); // Overall speed %
  wbox(ptr, w, h, c_b_low0, 4, 13, w - 8, 9);
  x = (unsigned)(ffps2 * 100.0);
  if (x > 10000)
    x = 10000;
  if (game_frames) {
    n = (int)(((double)f_time_total / game_frames) * 50.0);
    if (n > 10000)
      n = 10000;
    else if (n < 0)
      n = 0;
    x = 10000 - n;
  } else
    x = 0;
  wbox(ptr, w, h, c_b_low, 4, 13, ((w - 8) * x) / 10000, 9);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[82], x / 100, x % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s = %d", text[83],
                   (int)(((double)(f_exec_total + f_paint_total) / game_frames) * 100.0));
    else
      div_snprintf(cwork, sizeof(cwork), "%s = 0", text[83]);
  }
  wwrite(ptr, w, h, w / 2, 14, 1, (byte *)cwork, c_b_low0);
  wwrite(ptr, w, h, w / 2, 14, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, 3, 11 + 14, w / 2 - 4, 11); // Execution %
  wbox(ptr, w, h, c_g_low0, 4, 12 + 14, w / 2 - 6, 9);
  x = (unsigned)(((double)f_exec_total * (double)10000.0) / (double)f_time_total);
  wbox(ptr, w, h, c_g_low, 4, 12 + 14, ((w / 2 - 6) * x) / 10000, 9);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[84], x / 100, x % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[84],
                   (int)((f_exec_total * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s = 0", text[84]);
  }
  wwrite(ptr, w, h, 4 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c_g_low0);
  wwrite(ptr, w, h, 3 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w / 2 + 1, 11 + 14, w / 2 - 4, 11); // Rendering %
  wbox(ptr, w, h, c_r_low0, w / 2 + 2, 12 + 14, w / 2 - 6, 9);
  x = (unsigned)(((double)f_paint_total * (double)10000.0) / (double)f_time_total);
  wbox(ptr, w, h, c_r_low, w / 2 + 2, 12 + 14, ((w / 2 - 6) * x) / 10000, 9);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[85], x / 100, x % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[85],
                   (int)((f_paint_total * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s = 0", text[85]);
  }
  wwrite(ptr, w, h, w / 2 + 3 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w / 2 + 2 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, 3, lpy - 1, w - 6, lpal + 1); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 3, lpy - 1, w - 6, lpnum * lpal + 1);
  wrectangle(ptr, w, h, c0, w - 12, lpy - 1, 9, lpnum * lpal + 1);
  wrectangle(ptr, w, h, c0, w - 12, lpy + 7, 9, lpnum * lpal + 1 - 16);
  wput(ptr, w, h, w - 11, lpy, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -40);

  wrectangle(ptr, w, h, c0, 3, lp2y - 1, w - lp2esp - 6, lp2al + 1); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 3, lp2y - 1, w - lp2esp - 6, lp2num * lp2al + 1);
  wrectangle(ptr, w, h, c0, w - lp2esp - 12, lp2y - 1, 9, lp2num * lp2al + 1);
  wrectangle(ptr, w, h, c0, w - lp2esp - 12, lp2y + 7, 9, lp2num * lp2al + 1 - 16);
  wput(ptr, w, h, w - lp2esp - 11, lp2y, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -40);

  paint_profile_list();
}

int lp2_boton;

void profile2(void) {
  int n;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  float x;

  _process_items();

  if (no_volcar_nada) {
    no_volcar_nada = 0;
    return;
  }

  if (wmouse_in(3, lpy, w - 15, lpnum * lpal) && (mouse_b & 1)) {
    n = lp_ini + (wmouse_y - lpy) / lpal;
    if (n < lp_num) {
      if (lp_select != n) {
        lp_select = n;
        paint_profile_list();
        v.redraw = 1;
      }
    }
  }

  if (wmouse_in(w - 12, lpy - 1, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, w - 11, lpy, -41);
        lp_boton = 1;
        if (lp_select) {
          if (lp_ini == lp_select--)
            lp_ini--;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 1) {
      wput(ptr, w, h, w - 11, lpy, -39);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp_boton == 1) {
    wput(ptr, w, h, w - 11, lpy, -39);
    lp_boton = 0;
    v.redraw = 1;
  }


  if (wmouse_in(w - 11, lpy + 8, 7, lpnum * lpal - 17)) {
    mouse_graf = 13;
    if (lp_num > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - lpy - 8) / (float)(lpnum * lpal - 18);
      lp_select = x * (lp_num - 1);
      if (lp_select < lp_ini)
        lp_ini = lp_select;
      if (lp_select >= lp_ini + lpnum)
        lp_ini = lp_select - lpnum + 1;
      paint_profile_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(w - 12, lpy + lpnum * lpal - 9, 9, 9)) {
    if (mouse_b & 1) {
      if (lp_boton == 0) {
        wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -42);
        lp_boton = 2;
        if (lp_select + 1 < lp_num) {
          if (lp_ini + lpnum == ++lp_select)
            lp_ini++;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp_boton == 2) {
      wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -40);
      lp_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp_boton == 2) {
    wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -40);
    lp_boton = 0;
    v.redraw = 1;
  }

  // Functions listbox

  if (wmouse_in(3, lp2y, w - lp2esp - 15, lp2num * lp2al) && (mouse_b & 1)) {
    n = lp2_ini + (wmouse_y - lp2y) / lp2al;
    if (n < lp2_num) {
      if (lp2_select != n) {
        lp2_select = n;
        paint_profile_list();
        v.redraw = 1;
      }
    }
  }

  if (wmouse_in(w - lp2esp - 12, lp2y - 1, 9, 9)) {
    if (mouse_b & 1) {
      if (lp2_boton == 0) {
        wput(ptr, w, h, w - lp2esp - 11, lp2y, -41);
        lp2_boton = 1;
        if (lp2_select) {
          if (lp2_ini == lp2_select--)
            lp2_ini--;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp2_boton == 1) {
      wput(ptr, w, h, w - lp2esp - 11, lp2y, -39);
      lp2_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp2_boton == 1) {
    wput(ptr, w, h, w - lp2esp - 11, lp2y, -39);
    lp2_boton = 0;
    v.redraw = 1;
  }


  if (wmouse_in(w - lp2esp - 11, lp2y + 8, 7, lp2num * lp2al - 17)) {
    mouse_graf = 13;
    if (lp2_num > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - lp2y - 8) / (float)(lp2num * lp2al - 18);
      lp2_select = x * (lp2_num - 1);
      if (lp2_select < lp2_ini)
        lp2_ini = lp2_select;
      if (lp2_select >= lp2_ini + lp2num)
        lp2_ini = lp2_select - lp2num + 1;
      paint_profile_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(w - lp2esp - 12, lp2y + lp2num * lp2al - 9, 9, 9)) {
    if (mouse_b & 1) {
      if (lp2_boton == 0) {
        wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -42);
        lp2_boton = 2;
        if (lp2_select + 1 < lp2_num) {
          if (lp2_ini + lp2num == ++lp2_select)
            lp2_ini++;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp2_boton == 2) {
      wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -40);
      lp2_boton = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp2_boton == 2) {
    wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -40);
    lp2_boton = 0;
    v.redraw = 1;
  }

  if (scan_code == 33 || kbdFLAGS[_F12])
    goto profiler_next_frame;

  switch (v.active_item) {
  case 0:
profiler_next_frame:
    no_volcar_nada = 1;
    profiler_x = v.x;
    profiler_y = v.y;
    end_dialog = 1;
    break;
  case 1:
    for (n = 0; n < num_objects; n++) {
      if (o[n].type == tproc) {
        o[n].v4 = 0; // Time_exec
        o[n].v5 = 0; // Time_paint
      }
    }
    memset(f_time, 0, 256 * 4);
    ffps2 = 0.0f;
    game_ticks = 0.0f;
    game_frames = 0.0f;
    call(v.paint_handler);
    v.redraw = 1;
    break;
  case 2:
    end_dialog = 1;
    break;
  case 3:
    call(v.paint_handler);
    v.redraw = 1;
    break;
  }
}

void profile0(void) {
  v.type = 1;
  v.title = text[86];

  v.w = 256;
  v.h = 165 + 15;
  v.paint_handler = profile1;
  v.click_handler = profile2;

  _button(text[87], 7, v.h - 14, 0);
  _button(text[88], 7 + 3 + text_len((byte *)"\15 Siguiente Frame "), v.h - 14, 0);
  _button(text[89], v.w - 8, v.h - 14, 2);
  _flag(text[90],
        7 + 3 + text_len((byte *)"\15 Siguiente Frame ") + 3 + text_len((byte *)"\15 Comenzar "),
        v.h - 14, &absolut);
}

//----------------------------------------------------------------------------
