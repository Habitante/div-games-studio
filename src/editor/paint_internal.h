
//-----------------------------------------------------------------------------
//      Shared header for paint.c, paint_tools.c, paint_select.c
//-----------------------------------------------------------------------------

#pragma once

#include "global.h"
#include "div_string.h"

//-----------------------------------------------------------------------------
//      Module-global variables defined in paint.c, shared across files
//-----------------------------------------------------------------------------

extern int back;
extern int line_fx;
extern int blur_enabled;
extern int hotkey;
extern int point_index;
extern int tab_cycling;

extern int pencil_tool_effect, line_tool_effect, box_tool_effect, bezier_tool_effect,
    polygon_tool_effect;
extern int filled_tool_effect, circle_tool_effect, spray_tool_effect, text_tool_effect;

extern int fill_seeds[128];

extern byte default_brush[16];
extern byte *brush;
extern int brush_w, brush_h;
extern int brush_type;
extern int brush_index;

extern byte *texture_color;
extern int texture_w, texture_h;
extern int texture_x, texture_y;

extern int bezier_x, bezier_y;
extern int tension;

extern int text_bar_active;
extern int color256;
extern int b_pulsada;

extern int fill_icons[];

//-----------------------------------------------------------------------------
//      Toolbar struct (defined in paint.c)
//-----------------------------------------------------------------------------

struct paint_toolbar {
  int on;
  byte *ptr;
  int x, y, w, h;
};

extern struct paint_toolbar toolbars[10];

//-----------------------------------------------------------------------------
//      Functions defined in paint.c, called from other paint files
//-----------------------------------------------------------------------------

void remove_texture(void);
byte get_color(int x, int y);
void test_mouse_box(int a, int b, int c, int d);
void test_mouse_box2(int a, int b, int c, int d);
void test_mouse(void);
void zoom_region(int x, int y, int w, int h);
void draw_bar(int bar_width);
void draw_ruler(void);
void draw_help(int n);
void edit_ruler(void);
void select_color(int n);
void select_fx(int n, int *effect);
void line(int x0, int y0, int x1, int y1, int inc0);
void line0(int x0, int y0, int x1, int y1, int inc0);
void analyze_bar(int w, int h);
void bezier(int x0, int y0, int x1, int y1, int _x0, int _y0, int _x1, int _y1, int inc0);
int select_icon(int icon_x, int *icons);
void draw_box(int x0, int y0, int x1, int y1);
void draw_circle(int x0, int y0, int x1, int y1, int filled);
void _line_pixel(int x, int y);
void line_pixel(int x, int y);
void circle_scanline_filled(int x, int y, int w);
void circle_scanline(int x, int y, int w);
void pixel(byte *p);
void pixel_without_mask(byte *p);
void select_fill(int n);
void select_box(int n);
int editable(int *n);
void eyedropper(void);
void move_zoom(void);
void move_bar(void);
void bar_coords(void);
void select_mode(void);
void change_map(int forward);
void blit_mouse(void);
void blit_mouse_a(void);
void blit_mouse_b(void);
void adjust_box(int *a, int *b, int *c, int *d);
void paint_mask_window(byte *p, int c, int d);
void paint_color_window(byte *p, int c, int d);
void paint_window_colors2(byte *p, int c, int d, int col);
void color_up(void);
void color_down(void);
void flush_bar(byte *p, int real_w, int x, int y, int w, int h);
void flush_bar_darkened(byte *p, int real_w, int x, int y, int w, int h);
void flush_bars(int darkened);
void put_bar(int x, int y, int n);
void put_bar_inv(int x, int y, int n);
int new_bar(int w, int h);
void select_zoom(void);
void blit_edit(void);
void select_mask(int n);
void zoom_map2(void);
void draw_ruler_selection(byte *p, int c, int d, int x, int y);
int editable_selection(int *n, int x, int y);

//-----------------------------------------------------------------------------
//      Functions defined in paint_tools.c
//-----------------------------------------------------------------------------

void edit_scr(void);
void edit_mode_0(void);
void edit_mode_1(void);
void edit_mode_2(void);
void edit_mode_3(void);
void edit_mode_4(void);
void edit_mode_5(void);
void edit_mode_6(void);
void edit_mode_7(void);
void edit_mode_8(void);
void edit_mode_9(void);
void edit_mode_11(void);
void edit_mode_12(void);
void edit_mode_13(void);
void write_char(int x, int y, byte c);
void write_char2(int x, int y, byte *si, int font_width, int font_height);

//-----------------------------------------------------------------------------
//      Functions defined in paint_select.c
//-----------------------------------------------------------------------------

void block_bar(int n);
void edit_mode_10(void);
void test_previous(void);
void test_next(void);
void test_sel(void);
void test_sel_mask(void);
void cut_map(void);
void box_to_sel_mask(void);
void sel_mask_delete(void);
void sel_mask_ruler(void);
void sel_mask_invert(void);
void sel_mask_mask(void);
void sel_mask_lighten(byte color_aclarar);
void sel_mask_antialias(void);
void sel_mask_line(int x0, int y0, int x1, int y1);
void fill_polygon(void);
void move_selection(byte *sp, int w, int h);
void effects(void);
int edit_mode_6_box(int s);
int edit_mode_6_box_auto(int s);
int edit_mode_6_lines(int s);
int edit_mode_6_fill(int s);
int edit_mode_6_boxes(int s);
