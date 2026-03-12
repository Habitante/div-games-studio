/**
 * main_internal.h — Shared declarations between main.c, main_desktop.c,
 * and main_dialogs.c.
 *
 * These were file-scope globals in the original monolithic main.c.
 * Not intended for use outside the ide/main_*.c family.
 */

#ifndef MAIN_INTERNAL_H
#define MAIN_INTERNAL_H

#include "global.h"
#include "div_string.h"

/* --- Shared globals (defined in main.c) --- */

extern char get_buffer[LONG_LINE];
extern char *get;
extern int get_cursor, get_pos;

extern int window_closing, window_move_pending;
extern int quick_close;
extern int skip_window_render;

extern byte lower[256];

extern int show_items_called;
extern int test_video;
extern int restore_button;

extern int first_run;
extern int partial_blits;
extern int leer_mouse;

/* --- Title bar gradient tables (defined in main.c) --- */

extern char cbs[65], cbn, cgs[65], cgn, crs[65], crn;

/* --- Dialog loop state (defined in main.c) --- */

extern int n, m, oldn;
extern int dialogo_invocado;
extern int salir_del_dialogo;

/* --- Window array utilities (defined in main.c) --- */

void copy(int a, int b);
void xchg(int a, int b);
void move(int a, int b);
void divdelete(int a);
void addwindow(void);
void wup(int a);
void wdown(int a);

/* --- Functions in main_desktop.c --- */

void on_window_moved(int x, int y, int w, int h);
void maximize_window(void);
void minimize_window(void);
void close_window(void);
void move_window(void);
void move_window_complete(void);
void update_box(int x, int y, int w, int h);
void update_dialogs(int x, int y, int w, int h);
void update_box2(int vent, int x, int y, int w, int h);
void update_background(void);
void restore_wallpaper(int x, int y, int w, int h);
void flush_window(int m);
void place_window(int flag, int *_x, int *_y, int w, int h);
void find_best_position(int *_x, int *_y, int w, int h);
int calculate_collision(int x, int y, int w, int h);
int calculate_overlap(int a, int x, int y, int w, int h);
int windows_collide(int a, int b);
int collides_with(int a, int x, int y, int w, int h);
void flush_copy(void);
void window_surface(int w, int h, byte type);
void new_window(void_return_type_t init_handler);
void explode(int x, int y, int w, int h);
void implode(int x, int y, int w, int h);
void extrude(int x, int y, int w, int h, int x2, int y2, int w2, int h2);
void deactivate(void);
void activate(void);

/* --- Functions in main_dialogs.c --- */

void show_dialog(void_return_type_t init_handler);
void refresh_dialog(void);
void modal_loop(void);
void dialog_loop(void);
void _button(int t, int x, int y, int c);
void _get(int t, int x, int y, int w, byte *buffer, int buffer_len, int r0, int r1);
void _flag(int t, int x, int y, int *value);
void _show_items(void);
void show_button(struct t_item *i);
void select_button(struct t_item *i, int activo);
void show_get(struct t_item *i);
void select_get(struct t_item *i, int activo, int ocultar_error);
void show_flag(struct t_item *i);
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

#endif /* MAIN_INTERNAL_H */
