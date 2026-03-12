//-----------------------------------------------------------------------------
//      DIV Debugger - Shared Internal Definitions
//
//      This header is included by debugger.c, debugger_ui.c,
//      debugger_inspect.c, debugger_code.c, debugger_proclist.c,
//      and debugger_profile.c.
//
//      It holds every constant, type, and extern declaration that
//      the debugger translation units share.
//-----------------------------------------------------------------------------

#ifndef DEBUGGER_INTERNAL_H
#define DEBUGGER_INTERNAL_H

#include "../inter.h"
#include "../../div_string.h"

//-----------------------------------------------------------------------------
//      Constants
//-----------------------------------------------------------------------------

#define max_procesos 2048

//-----------------------------------------------------------------------------
//      Object types in the debugger symbol table (from exec.dbg)
//-----------------------------------------------------------------------------

#define tnone 0
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
#define tfext 12

#define tbglo 13
#define twglo 14
#define tbloc 15
#define twloc 16

#define tpigl 17
#define tpilo 18

#define tpwgl 19
#define tpwlo 20
#define tpbgl 21
#define tpblo 22

#define tpcgl 23
#define tpclo 24
#define tpsgl 25
#define tpslo 26

//-----------------------------------------------------------------------------
//      Types
//-----------------------------------------------------------------------------

struct object {
  int type;
  int name;
  int scope;
  int miembro;
  int v0, v1, v2, v3, v4, v5;
};

struct variables {
  int object;  // Object index in o[]
  int tab;     // Indentation (0=no member, 1, 2, ...)
  int miembro; // Member of ..., as index into var[]
  int indice;  // For arrays or structs, the displayed element index
};

//-----------------------------------------------------------------------------
//      Shared globals (defined in debugger.c)
//-----------------------------------------------------------------------------

extern char combo_error[128];

extern int skip_flush;
extern int profiler_x, profiler_y;

extern int show_items_called;
extern int get_pos, get_cursor;
extern int superget;

// Code window state
extern int line0;
extern byte *pline0;
extern int mem1, mem2;
extern int line1, col1, line2, col2;
extern int line_sel;
extern int x_start;

// Mouse state
extern int smouse_x, smouse_y, mouse_x, mouse_y, mouse_b;
extern int debug_clock;
extern int ticks_debug;

// Debug state
extern int button;
extern int ids_old, ids_inc;

extern byte *source, *end_source;
extern int *dbg_lines;
extern int num_statements;

extern clock_t system_clock;
extern char get[256];
extern char *vnom;

extern struct object *o;
extern int num_objects;
extern int num_predefined;

extern int *used;
extern int *visor;

extern struct variables *var;
extern int num_var;
extern int var_ini;
extern int var_select;

extern int show_const, show_global, show_local, show_private;
extern int pre_defined, user_defined;
extern int current_scope;

// Process list state
extern int iids, *ids;
extern int ids_ini;
extern int ids_select;
extern int ids_next;

// Process list dialog state
extern int lp1[512];
extern char *lp2[512];
extern int lp_num;
extern int lp_ini;
extern int lp_select;
extern int lp_sort;

// Profiler state
extern byte c_r_low0, c_g_low0, c_b_low0;
extern int obj_start;
extern int obj_size;
extern unsigned f_time[256];
extern unsigned frame_time[256];

// From interpreter.c / other runtime files
extern int process_level;
extern float ffps2;
extern double game_ticks, game_frames;
void exec_process(void);
void trace_process(void);

//-----------------------------------------------------------------------------
//      Functions — debugger.c (init, error dialogs, debug entry, process IDs)
//-----------------------------------------------------------------------------

void init_debug(void);
void end_debug(void);
void init_big(void);
void init_colors(void);

void err0(void);
void e(int text_id);
void deb(void);
void debug(void);

int procesos_ejecutados(void);
void determine_ids(void);
void draw_proc_list(void);
void process_graph(int id, byte *q, int van, int w, int h);

//-----------------------------------------------------------------------------
//      Functions — debugger_ui.c (dialog, graphics, widgets, mouse)
//-----------------------------------------------------------------------------

void dummy_handler(void);
void show_dialog(void_return_type_t init_handler);
void repaint_window(void);
void modal_loop(void);
void refresh_dialog(void);
void close_window(void);
void move_window(void);
void flush_window(int m);

void blit_region(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w, int h,
                 int salta);
void blit_region_dark(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w,
                      int h, int salta);
void wbox(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
void wbox_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, byte c, int x, int y,
                 int w, int h);
void wrectangle(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
void put(int x, int y, int n);
void wput(byte *dest, int dest_width, int dest_height, int x, int y, int n);
void wput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n);
void bwput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n);
int char_len(char c);
int text_len(byte *ptr);
int text_len2(byte *ptr);
void wwrite(byte *dest, int dest_width, int dest_height, int x, int y, int alignment, byte *ptr,
            byte c);
void wwrite_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x_org,
                   int y_org, int alignment_org, byte *ptr, byte c);
void wtexn(byte *dest, int dest_pitch, byte *p, int x, int y, byte w, int h, byte c);
void wtexc(byte *dest, int dest_pitch, int dest_width, int dest_height, byte *p, int x, int y,
           byte w, int h, byte c);

void explode(int x, int y, int w, int h);
void implode(int x, int y, int w, int h);
void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2);
void update_box(int x, int y, int w, int h);
int windows_collide(int a, int b);
int collides_with(int a, int x, int y, int w, int h);
void restore_wallpaper(int x, int y, int w, int h);

void _button(byte *t, int x, int y, int c);
void _get(byte *t, int x, int y, int w, byte *buffer, int buffer_len, int r0, int r1);
void _flag(byte *t, int x, int y, int *value);
void _show_items(void);
void _show_items2(void);
void show_button(t_item *i);
void show_get(t_item *i);
void show_flag(t_item *i);
void select_button(t_item *i, int activo);
void select_get(t_item *i, int activo, int ocultar_error);
void _process_items(void);
void _select_new_item(int i);
void _reselect_item(void);
int button_status(int n);
void process_button(int n, int e);
int get_status(int n);
void process_get(int n, int e);
int flag_status(int n);
void process_flag(int n, int e);
void get_input(int n);

void dread_mouse(void);
void flush_copy(void);
void save_mouse_bg(byte *p, int x, int y, int n, int flag);
int mouse_in(int x, int y, int x2, int y2);
int wmouse_in(int x, int y, int w, int h);

//-----------------------------------------------------------------------------
//      Functions — debugger_inspect.c (variable inspector)
//-----------------------------------------------------------------------------

void create_variable_list(void);
void exclude_members(int padre, int nivel, int index);
void include_members(int padre, int nivel, int index);
void inspect0(void);
void paint_var_list(void);
void paint_segment2(void);
int memo(int dir);
void visualize(int value, int object, char *str);
int get_offset(int m);
int _get_offset(int m);
byte *get_offset_byte(int m);
word *get_offset_word(int m);
void change0(void);
void changestring0(void);

//-----------------------------------------------------------------------------
//      Functions — debugger_code.c (source code window)
//-----------------------------------------------------------------------------

void debug0(void);
byte *change_mode(void);
void paint_segment(void);
void paint_code(void);
void determine_code(void);
void get_line(int n);
int get_ip(int n);
void f_up(void);
void f_down(void);
void f_home(void);
void f_right(void);
void f_left(void);

//-----------------------------------------------------------------------------
//      Functions — debugger_proclist.c (process list dialog)
//-----------------------------------------------------------------------------

void process_list0(void);
void paint_process_segment(void);
extern int lp_button;

//-----------------------------------------------------------------------------
//      Functions — debugger_profile.c (profiler dialog)
//-----------------------------------------------------------------------------

void profile0(void);
void function_exec(int id, int n);
void process_exec(int id, int n);
void process_paint(int id, int n);
void paint_profile_segment(void);

#endif // DEBUGGER_INTERNAL_H
