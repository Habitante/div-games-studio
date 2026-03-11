/**
 * DIV GAMES STUDIO 2015
 * global.h
 * 
 * Global header file
 * 
 ***/

#ifndef __GLOBAL_H
#define __GLOBAL_H

#ifndef false
#define false 0
#define true  1
#endif


///////////////////////////////////////////////////////////////////////////////
//      Global Variables
///////////////////////////////////////////////////////////////////////////////

#ifdef DEFINIR_AQUI // Defined here
#define GLOBAL_DATA
#else
#define GLOBAL_DATA extern
#endif


#ifdef DIVGIT
#include "divgit.h"
#endif


typedef void (*voidReturnType)(void);
void call(const voidReturnType func); // void funcion(void); int n=(int)funcion; call(n);


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <ctype.h>

#include "osdep.h"


#include <math.h>
#include <signal.h>

#include "divkeybo.h"
// divmap3d.hpp removed (MODE8/3D map editor deleted)

// JUDAS sound library removed — SDL2_mixer has fully replaced it.
// judas/ directory deleted; shared/divsound.h provides sound constants for runtime.

//-----------------------------------------------------------------------------
//      Constants defined at the application level
//-----------------------------------------------------------------------------


#ifdef DEBUG
#define debugprintf(...) printf(__VA_ARGS__)
#else
#define printf debugprintf
#define debugprintf(...)
#endif

#define uchar uint8_t
//unsigned char
#define byte uint8_t
//unsigned char
#define ushort uint16_t
//unsigned short
#define word uint16_t
//unsigned short

#ifndef __llvm__
#define ulong uint32_t
//unsigned int
#endif


#define dword uint32_t
//unsigned int


#define max_archivos    512 // ------------------------------- File listbox
#define w_archivo      (255)
#define max_directorios 2048
#define w_directorio   (255)


#define swap(a, b) \
  {                \
    (a) ^= (b);    \
    (b) ^= (a);    \
    (a) ^= (b);    \
  }

#define max_windows 96   // maximum No. of open windows
#define max_texts   2048 // max No. allowed texts (lenguaje.div)

#define long_line 1024 // Maximum line length in the editor

#define RES_FOR_NAME _MAX_PATH

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIV (div.c)
///////////////////////////////////////////////////////////////////////////////

void _fwrite(char *, byte *, int);
void _ffwrite(byte *Buffer, unsigned int Len, FILE *file);
void error(int);
void new_window(voidReturnType);
void show_dialog(voidReturnType);
void refresh_dialog(void);
void close_window(void);
void update_box(int, int, int, int);
void update_background(void);
void flush_window(int);
void flush_copy(void);
void place_window(int flag, int *_x, int *_y, int w, int h);
void on_window_moved(int x, int y, int w, int h);
void _get(int text_id, int x, int y, int w, byte *buf, int lon_buf, int r0, int r1);
void _button(int text_id, int x, int y, int centro);
void _flag(int text_id, int x, int y, int *variable);
void _show_items();
void _process_items();
void _select_new_item(int n);
void _reselect_item(void);
int windows_collide(int a, int b);
void maximize_window(void);
void explode(int x, int y, int w, int h);
void activate(void);
void DaniDel(char *name);

void window_surface(int w, int h, byte type);

///////////////////////////////////////////////////////////////////////////////
//     Functions exported by DIVBASIC (divbasic.c)
///////////////////////////////////////////////////////////////////////////////

void writetxt(int, int, int, byte *);
void box(byte, int, int, int, int);
void rectangle(byte, int, int, int, int);
void save_mouse_bg(byte *p, int x, int y, int n, int flag);
void put(int, int, int);
void put_bw(int, int, int);
void memxchg(byte *, byte *, int);
void draw_edit_background(int, int, int, int);
void zoom_map(void);
void fill_select(word, word);
void fill(word, word);
void interpolation_mode(void);
void set_selection_mask(int, int);
int is_selection_mask(int, int);
int is_near_selection_mask(int, int);
byte *save_undo(int, int, int, int);
int undo_back(void);
void undo_next(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVPAINT (divpaint.c)
///////////////////////////////////////////////////////////////////////////////

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
void edit_mode_10(void);
void edit_mode_11(void);
void edit_mode_12(void);
void edit_mode_13(void);

void test_mouse(void);
void blit_edit(void);
void draw_bar(int);
void put_bar(int, int, int);
void put_bar_inv(int, int, int);
void flush_bars(int);
void move_selection(byte *, int, int);
void select_zoom(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVSPRIT (divsprit.c)
///////////////////////////////////////////////////////////////////////////////

void sp_normal(byte *, int, int, int, int, int, int, int);
void sp_rotated(byte *, int, int, int, int, int, int, int, float, float);
void sp_size(int *, int *, int *, int *, int, int, float, float);
void invierte_hor(byte *, int, int);
void invierte_ver(byte *, int, int);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVVIDEO (divvideo.c)
///////////////////////////////////////////////////////////////////////////////

void retrace_wait(void);
void setup_video_mode(void);
void reset_video_mode(void);
void set_dac(byte *);
void init_flush(void);
void blit_partial(int, int, int, int);
void blit_screen(byte *);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVMOUSE (divmouse.c)
///////////////////////////////////////////////////////////////////////////////

void mouse_on(void);
void mouse_off(void);
int mouse_in(int, int, int, int);
int wmouse_in(int, int, int, int);
void set_mouse(word, word);
void set_mickeys(word);
void read_mouse(void);
void mouse_window(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVPALET (divpalet.c)
///////////////////////////////////////////////////////////////////////////////

void find_colors(void);
void init_ghost(void);
void create_ghost(int);
void create_dac4();
byte find_color(byte, byte, byte);
byte find_color_not0(byte r, byte g, byte b);
byte fast_find_color(byte fr, byte fg, byte fb);
byte find_ord(byte *);
byte average_color(byte, byte);
void make_nearest_gradient(void);
void calculate_gradient(int);

// Editing palette

void pal_edit();
void pal_load();
void pal_save_as();
void pal_refresh(int, int);
void sort_palette();      // sort palette
void merge_palette();     // merge palette
void prepare_wallpaper(); // background wallpaper setup (???)

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVSETUP (divsetup.c)
///////////////////////////////////////////////////////////////////////////////

struct _gcolor {
  byte color;   // colour within palette
  byte r, g, b; // colour properties
  byte selec;   // state : 1-Active 0-Inactive
};

// Structure used to save and load the configuration file
// SETUP.BIN

typedef struct _SetupFile {
  char Setup_Header[8];

  // Video mode
  int Vid_mode;
  int Vid_modeAncho; // Video mode width
  int Vid_modeAlto;  // Video mode height
  int Vid_modeBig;   // "Big" video mode

  // Undo system
  int Max_undo;
  int Undo_memory;

  // Directory system
  char Dir_cwd[_MAX_PATH + 1];
  char Dir_map[_MAX_PATH + 1];
  char Dir_pal[_MAX_PATH + 1];
  char Dir_fpg[_MAX_PATH + 1];
  char Dir_fnt[_MAX_PATH + 1];
  char Dir_ifs[_MAX_PATH + 1];
  char Dir_pcm[_MAX_PATH + 1];
  char Dir_prg[_MAX_PATH + 1];
  char Dir_pcms[_MAX_PATH + 1];
  char Dir_prj[_MAX_PATH + 1];
  char Dir_wld[_MAX_PATH + 1];
  char Dir_mod[_MAX_PATH + 1];

  // Wallpaper info
  char Desktop_Image[_MAX_PATH + 1];
  int Desktop_Gama;
  int Desktop_Tile;
  struct _gcolor gradient_config[9];

  char colors_rgb[12 * 3]; // r,g,b [c2,c4,c_b_low,ce1,ce4,c_y]
  int editor_font;
  int paint_cursor;
  int exploding_windows;
  int auto_save_session;
  int move_full_window;
  int tab_size;
  int colorizer;

  byte vol_fx, vol_cd, vol_ma;
  byte mut_fx, mut_cd, mut_ma;

  int mouse_ratio;
  int fullscreen;

} SetupFile;

GLOBAL_DATA byte fsmode;

GLOBAL_DATA int MustCreate;
GLOBAL_DATA SetupFile Setupfile;
GLOBAL_DATA int next_order;
GLOBAL_DATA int return_mode;

// Global variables for undo
GLOBAL_DATA int max_undos;
GLOBAL_DATA int undo_memory;

GLOBAL_DATA int interpreting;
GLOBAL_DATA int SoundError;
GLOBAL_DATA int CopyDesktop;
GLOBAL_DATA int compilemode;

int w_setup(void);
void Save_Cfgbin();
void Load_Cfgbin();

GLOBAL_DATA char ce1, ce01, ce2, ce4; // Editor colours

GLOBAL_DATA int auto_save_session, exploding_windows, move_full_window, tab_size, colorizer;
GLOBAL_DATA int editor_font, paint_cursor;
GLOBAL_DATA char colors_rgb[12 * 3];

///////////////////////////////////////////////////////////////////////////////
//                        Configuration dialogs
//                            divsetup.c
///////////////////////////////////////////////////////////////////////////////

// Video config settings
GLOBAL_DATA int VS_MODE;
GLOBAL_DATA int VS_WIDTH;
GLOBAL_DATA int VS_HEIGHT;
GLOBAL_DATA int VS_BIG;
void Vid_Setup0();

// Wallpaper configuration
void Tap_Setup0(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVWINDO (divwindo.c)
///////////////////////////////////////////////////////////////////////////////

void wgra(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
void wbox(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
void wresalta_box(byte *dest, int dest_width, int dest_height, int x, int y, int w, int h);
void wbox_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, byte c, int x, int y,
                 int w, int h);
void wrectangle(byte *dest, int dest_width, int dest_height, byte c, int x, int y, int w, int h);
void wput(byte *dest, int dest_width, int dest_height, int x, int y, int n);
void wput_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y, int n);
void blit_region(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w, int h,
              int salta);
void blit_region_dark(byte *dest, int dest_width, int dest_height, byte *p, int x, int y, int w,
                     int h, int salta);
void wwrite(byte *dest, int dest_width, int dest_height, int x, int y, int centro, byte *ptr,
            byte c);
void wwrite_in_box(byte *dest, int dest_pitch, int dest_width, int dest_height, int x, int y,
                   int centro, byte *ptr, byte c);
int text_len(byte *ptr);
int char_len(char);
void boton(int n, int x, int y, int centro, int color);
int ratonboton(int n, int x, int y, int centro);

void wline(char *ptr, int realan, int w, int h, int x0, int y0, int x1, int y1, char color);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVLENGU (divlengu.c)
///////////////////////////////////////////////////////////////////////////////

void initialize_texts(byte *fichero);
void finalize_texts(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVCDROM (divcdrom.c)
///////////////////////////////////////////////////////////////////////////////

void muestra_cd_player();
void show_clock();
void CDiv0(void);
void CDiv1(void);
void Clock0(void);
void Clock1(void);
void Bin0(void);
void Bin1(void);
void FPG0A(void);
void FPG1(void);
void PRJ0();
void PRJ1();
void PRJ2();
void PRJ3();
void ShowFont0(void);
void ShowFont1(void);

///////////////////////////////////////////////////////////////////////////////
//      Main handler, exported by DIVHANDLE (divhandl.c)
///////////////////////////////////////////////////////////////////////////////

void dummy_handler(void);

void menu_principal0(void);
void menu_principal1(void);
void menu_principal2(void);
void menu_principal3(void);
void menu_programas0(void);
void menu_programas1(void);
void menu_programas2(void);
void menu_paletas0(void);
void menu_paletas1(void);
void menu_paletas2(void);
void menu_mapas0(void);
void menu_mapas1(void);
void menu_mapas2(void);
void menu_graficos0(void);
void menu_graficos1(void);
void menu_graficos2(void);
void menu_fuentes0(void);
void menu_fuentes1(void);
void menu_fuentes2(void);
void menu_sonidos0(void);
void menu_sonidos1(void);
void menu_sonidos2(void);
void menu_sistema0(void);
void menu_sistema1(void);
void menu_sistema2(void);
void menu_edicion0(void);
void menu_edicion1(void);
void menu_edicion2(void);
// menu_mapas3D declarations removed (MODE8/3D map editor deleted)

void nuevo_mapa0(void);
void nuevo_mapa1(void);
void nuevo_mapa2(void);
void nuevo_mapa3(void);
void mapa0(void);
void mapa1(void);
void mapa2(void);
void mapa3(void);
void paleta0(void);
void paleta1(void);
void copyright0(void);
void copyright1(void);
void copyright2(void);
void copyright3(void);
void aceptar0(void);
void aceptar1(void);
void aceptar2(void);
void aceptar3(void);
void error0(void);
void error1(void);
void error2(void);
void error3(void);
void err0(void);
void err1(void);
void err2(void);
void err3(void);
void info0(void);
void browser0(void);
void browser2(void);
void MapperWarning0(void);
void MapperWarning1(void);
void MapperWarning2(void);
// MapperVisor declarations removed (MODE8/3D map editor deleted)

int new_map(byte *mapilla);
void nuevo_mapa3D(void);
void RenderToMed();
void Reducex2();
int Progress(char *titulo, int current, int total);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVFORMA (divforma.c)
///////////////////////////////////////////////////////////////////////////////

int fmt_is_map(byte *buffer);
void fmt_load_map(byte *buffer, byte *mapa, int);
int fmt_save_map(byte *mapa, FILE *f);
int fmt_is_pcx(byte *buffer);
void fmt_load_pcx(byte *buffer, byte *mapa, int);
int fmt_save_pcx(byte *mapa, FILE *f);
int fmt_is_bmp(byte *buffer);
void fmt_load_bmp(byte *buffer, byte *mapa, int);
int fmt_is_jpg(byte *buffer, int img_filesize);
int fmt_load_jpg(byte *buffer, byte *mapa, int vent, int img_filesize);
int fmt_save_bmp(byte *mapa, FILE *f);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVEDIT (divedit.c)
///////////////////////////////////////////////////////////////////////////////

void program0(void);
void program1(void);
void program2(void);
void program3(void);
void program0_new(void);
void save_program(void);
void open_program(void);
void find_text0(void);
void find_text1(void);
void find_text2(void);
void replace_text0(void);
void replace_text1(void);
void replace_text2(void);
void find_text(void);
void replace_text(void);
void repaint_window(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVASM (divasm.c)
///////////////////////////////////////////////////////////////////////////////

void memcpyb(byte *, byte *, int);
///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVEFFECT (diveffect.c)
///////////////////////////////////////////////////////////////////////////////

void GenExplodes();

///////////////////////////////////////////////////////////////////////////////
//      Global variables shared between several modules
//      GLOBAL_DATA variables are defined in DIVOS, the rest only declare them
///////////////////////////////////////////////////////////////////////////////

GLOBAL_DATA int big, big2; // big(0,1), big2(1,2)

GLOBAL_DATA byte *error_window;

GLOBAL_DATA int map_width, map_height; // Dimensions of the edited map
GLOBAL_DATA int vga_width, vga_height; // Video mode (id vga_width*1000+vga_height, e.g. 320200)
GLOBAL_DATA int vwidth, vheight;       // Window Screen Size

GLOBAL_DATA int fpg_code;            // Map code
GLOBAL_DATA char MapDescription[32]; // Map description

GLOBAL_DATA byte *wallpaper;     // Background graphic for the window main_loop
GLOBAL_DATA byte *wallpaper_map; // Start of bitmap data (wallpaper_width,wallpaper_height)
GLOBAL_DATA byte *fill_dac;      // Set to 1 for colors that may be in the fill zone
GLOBAL_DATA byte *map;           // Edited map (the drawing, not the main_loop)
GLOBAL_DATA int *selection_mask; // Edited map (bitmap for zone selection)
GLOBAL_DATA byte *ghost;         // Ghost-layering table for the edited object's palette
GLOBAL_DATA byte *dac;           // Palette of the edited object
GLOBAL_DATA byte *dac4;          // Palette with 0..255 range

GLOBAL_DATA word sel_mask_x0, sel_mask_x1, sel_mask_y0,
    sel_mask_y1; // Bounds of zone selected in selection_mask

// int r,g,b,c,d,a removed (Sprint D) — replaced with local variables or file-static in divpalet.c
// FILE *f removed (Sprint D) — all usages now use local declarations

GLOBAL_DATA byte c0, c1, c2, c3, c4, text_color; // IDE environment colors
GLOBAL_DATA byte c01, c12, c23, c34;             // Intermediate colors
GLOBAL_DATA byte c_r, c_g, c_b, c_r_low, c_g_low, c_b_low, c_y;
GLOBAL_DATA byte c_com, c_sim, c_res, c_pre, c_num, c_lit;

GLOBAL_DATA int highlight_background;

GLOBAL_DATA byte mask[256]; // Paint mask for divpaint
GLOBAL_DATA int mask_on;

extern int hotkey;

GLOBAL_DATA int wallpaper_width, wallpaper_height;

GLOBAL_DATA int mouse_x, mouse_y, mouse_b, mouse_graf; // Mouse
GLOBAL_DATA int real_mouse_x, real_mouse_y;

GLOBAL_DATA int prev_mouse_buttons; // Previous mouse button state

GLOBAL_DATA byte *color_lookup; // Squared differences table for palette color matching

GLOBAL_DATA int *system_clock, cclock, mclock;

GLOBAL_DATA byte *screen_buffer; // Virtual screen copy (IDE framebuffer)

GLOBAL_DATA SDL_Surface *screen_buffer_surface;
GLOBAL_DATA SDL_Surface *tempsurface;
GLOBAL_DATA SDL_Surface *mouse_surface;
GLOBAL_DATA SDL_Surface *wallpaper_surface;      // background
GLOBAL_DATA SDL_Surface *wallpaper_temp_surface; // surface for preview

GLOBAL_DATA uint32_t colorkey;
GLOBAL_DATA uint32_t rmask, gmask, bmask, amask;
GLOBAL_DATA byte explode_num;

GLOBAL_DATA byte *undo;    // Copies of edited map (for undo, NULL if out of memory)
GLOBAL_DATA byte *toolbar; // Paint editor toolbar buffer

GLOBAL_DATA int undo_error;

GLOBAL_DATA int coord_x, coord_y; // Coordinates on the map being edited

GLOBAL_DATA int zoom, zoom_x, zoom_y; // Active zoom in editor (zoom=0..3)

GLOBAL_DATA int zoom_win_x, zoom_win_y, zoom_win_width,
    zoom_win_height; // Zoom window in screen copy

GLOBAL_DATA int zoom_background; // Whether the zoom window background has been drawn

GLOBAL_DATA int need_zoom; // Whether the map zoom needs to be redrawn
GLOBAL_DATA int need_zoom_x, need_zoom_y, need_zoom_width, need_zoom_height;

GLOBAL_DATA int zoom_cx, zoom_cy; // Logical center of the zoom

GLOBAL_DATA int _mouse_x, _mouse_y;

GLOBAL_DATA byte color, old_color, color_c0; // Selected color(s)

//GLOBAL_DATA
struct callback_data {
  unsigned short mouse_action;
  unsigned short mouse_bx;
  unsigned short mouse_cx;
  unsigned short mouse_dx;
};

extern struct callback_data cbd;

//GLOBAL_DATA
struct tipo_regla {
  byte num_colors; // 8,16,32
  byte type;       // 0 (linear), 1-2-4-8 (adaptive every n colors)
  byte fixed;      // 0 (No), 1 (Yes)
  byte colors[33]; // Black(c0) + Up to 32 colors per (num_colors,type)
};


GLOBAL_DATA struct tipo_regla gradients[16];

GLOBAL_DATA int gradient; // Selected gradient (color range) number

GLOBAL_DATA byte nearest_gradient[190]; // Nearest gradient color for each RGB

GLOBAL_DATA int toolbar_x, toolbar_y, toolbar_width; // Paint toolbar position

GLOBAL_DATA int draw_mode; // Drawing mode (0=pencil,1=lines,2=bezier, ...) (+100 when in menu)

GLOBAL_DATA byte *mouse_background; // Buffer to save the mouse background

GLOBAL_DATA byte *graf_ptr, *graf[256]; // IDE environment graphics

//GLOBAL_DATA
struct tgraf_help {
  int offset;
  int w, h;
  int ran, ral;
  byte *ptr;
};

GLOBAL_DATA struct tgraf_help graf_help[384]; // Hypertext graphics (offset,width,height,ptr)
GLOBAL_DATA char help_xlat[256];              // Translation table for help graphics

GLOBAL_DATA byte *text_font; // Standard font, 7 pixels tall, proportional width

GLOBAL_DATA SDL_Color colors[256];

GLOBAL_DATA byte *font;                                // Font for the program editor / hypertext
GLOBAL_DATA int font_width, font_height;               // Character width and height
GLOBAL_DATA int editor_font_width, editor_font_height; // Editor character width and height
GLOBAL_DATA int char_size;                             // font_width*font_height

GLOBAL_DATA int current_mouse; // Mouse cursor graphic

//GLOBAL_DATA
struct tipo_undo {                    // Circular undo table
  int start, end, x, y, w, h, mode; // start refers to *(undo+start)
  int code;                           // Map identifier for this undo entry
}; // mode=-1 if entry unused

GLOBAL_DATA struct tipo_undo *undo_table;

GLOBAL_DATA int undo_index; // Index into undo_table[], first free element

GLOBAL_DATA int zoom_move; // Whether zoom auto-scrolls (paint) (uses c3, c4)

GLOBAL_DATA int sel_status;                     // Whether any selection is drawn on screen copy
GLOBAL_DATA int sel_x0, sel_x1, sel_y0, sel_y1; // Box-type selection bounds

// *** Fill stack layout: x0, x1, x, y, inc_y (10 bytes)

GLOBAL_DATA word *fss, *fsp, *fsp_max; // Fill stack segment, Fill stack pointer
GLOBAL_DATA byte *original;

GLOBAL_DATA int bar[16]; // Toolbar graphics

GLOBAL_DATA int mode_rect;      // Whether to draw rectangles or filled boxes
GLOBAL_DATA int mode_circle;    // Whether to draw circles or filled circles
GLOBAL_DATA int mode_fill;      // Fill algorithm mode
GLOBAL_DATA int mode_selection; // Selection mode
GLOBAL_DATA int selected_icon;  // (toolbar) -1 if none selected, 0-zoom, ...

GLOBAL_DATA int full_redraw; // Whether the entire VGA screen copy has been modified

GLOBAL_DATA byte *texts[max_texts]; // Output text strings, in translatable format

GLOBAL_DATA int wmouse_x, wmouse_y; // Mouse position within a window

#define max_items 24 // Maximum number of items in a window

//GLOBAL_DATA
struct t_item {
  int type;  // 0-none, 1-button, 2-get, 3-switch
  int state; // Item state (mouse hover, pressed, etc.)
  union {
    struct {
      byte *text;
      int x, y, center;
    } button;
    struct {
      byte *text;
      byte *buffer;
      int x, y, w, lon_buffer;
      int r0, r1;
    } get;
    struct {
      byte *text;
      int *valor;
      int x, y;
    } flag;
  };
};

//GLOBAL_DATA
struct twindow {
  int type;       // 0-none, 1-show_dialog, 2-menu, 3-palette
                  // 4-timer, 5-trash, 7-progress_bar
                  // 8-mixer
                  // 100-map (>=100 excludable objects)
                  // 101-fpg 102-prg-gen_fnt-hlp 104-fnt
                  // 105-pcm 106-map3D 107-music module
  int order;      // Window z-order
  int foreground; // 0-No, 1-Yes, 2-Minimized
  byte *name;     // Icon name
  byte *title;    // Title bar text
  voidReturnType paint_handler, click_handler, close_handler;
  int x, y, w, h;              // Window position and dimensions
  int _x, _y, _an, _al;          // Position saved when minimized
  byte *ptr;                     // Window buffer
  struct tmapa *mapa;            // Pointer to associated map struct
  struct tprg *prg;              // Pointer to associated program struct
  int redraw;                    // Needs-redraw flag
  int state;                     // Button/item state
  int buttons;                   // Pressed buttons bitmask
  byte *aux;                     // Auxiliary pointer for miscellaneous data
  struct t_item item[max_items]; // Buttons, gets, switches, etc.
  int items;                     // Number of defined items
  int active_item;               // Which item triggered an action
  int selected_item;             // Currently selected item (for keyboard)
  int side;                      // 0 Right, 1 Left (auto-place on double-click)
  int exploding;
};

GLOBAL_DATA struct twindow window[max_windows];
#define v window[0]

//GLOBAL_DATA
struct tmapa {
  char has_name;                               // New window will take the filename
  int code;                                   // Map identifier (for undo)
  int fpg_code;                               // Map code (for the FPG)
  char path[_MAX_PATH + 1];                   // Path of associated file
  char filename[255];                         // Associated filename
  byte *map;                                  // Map data
  int map_width, map_height;                  // Map dimensions (pixels)
  int zoom, zoom_x, zoom_y, zoom_cx, zoom_cy; // Edited portion of the map
  byte saved;                                 // Saved to disk flag (yes/no)
  char description[32];                       // Map description string
  short points[512];                          // Up to 256 control points (x,y)
};

struct tprg {
  int w, h;               // Width and height in characters of window
  int old_x, old_y;         // Coordinates before maximizing
  int old_w, old_h;       // Width and height before maximizing
  char path[_MAX_PATH + 1]; // Path of associated file
  char filename[255];       // Associated filename
  byte *buffer;             // Buffer with loaded file
  int buffer_len;           // Buffer length
  int file_len;             // File length ( < buffer_len)
  int num_lines;            // Number of lines in source
  int line;                 // Current line in editor
  int column;               // Current cursor column
  byte *lptr;               // Pointer to current line in file
  byte *vptr;               // Pointer to first visible line
  int first_line;           // First visible line on screen
  int first_column;         // Horizontal scroll offset
  char l[long_line + 4];    // Buffer for edited line
  int line_size;            // Original size of edited line
  int prev_line;            // Previous line (for partial redraw)
};

struct t_listbox {
  int x, y;          // Listbox position in window
  char *list;        // List pointer
  int item_width;    // Characters per item
  int visible_items; // Visible item count
  int w, h;        // Text zone width in pixels
  int first_visible; // First visible index (from 0)
  int total_items;   // Total item count (0 n/a)
  int s0, s1, slide; // Slide bar start, end, current position
  int zone;          // Selected zone
  int buttons;       // Pressed button: up(1) or down(2)
  int created;       // Whether list is already created on screen
};

struct t_listboxbr {
  int x, y;       // Listbox position in window
  char *list;     // List pointer
  int item_width; // Characters per item
  int columns;    // Number of columns in browser listbox
  int lines;      // Number of lines
  int w, h;     // Pixel size of each cell

  int first_visible; // First visible index (from 0)
  int total_items;   // Total item count (0 n/a)
  int s0, s1, slide; // Slide bar start, end, current position
  int zone;          // Selected zone
  int buttons;       // Pressed button: up(1) or down(2)
  int created;       // Whether list is already created on screen
};

// Parameters for some windows

GLOBAL_DATA char *v_title;       // Window title
GLOBAL_DATA char *v_text;        // Window text
GLOBAL_DATA byte *v_aux;         // Memory allocated before calling the creator
GLOBAL_DATA int v_mode;          // 0-open, 1-save as
GLOBAL_DATA int v_type;          // 2-map, 3-pal, 4-grf, 5-fnt, 6-ifs, 7-pcm, 8-prg
GLOBAL_DATA int v_thumb;         // 2-map, 3-pal, 4-grf, 5-fnt, 6-ifs, 7-pcm, 0-prg
GLOBAL_DATA int v_finished;      // Whether a file has been selected
GLOBAL_DATA int v_exists;        // Whether the file exists
GLOBAL_DATA int v_accept;        // Whether accept or cancel was chosen
GLOBAL_DATA int v_window;        // Target window (e.g. map to save)
GLOBAL_DATA int v_help;          // Whether help was requested
GLOBAL_DATA int v_pause;         // Whether to pause (in listboxes)
GLOBAL_DATA struct tmapa *v_map; // Pointer to a map
GLOBAL_DATA struct tprg *v_prg;  // Window buffer
GLOBAL_DATA struct tprg *eprg;   // Window buffer

GLOBAL_DATA int next_map_code;    // Code of next loaded map
GLOBAL_DATA int current_map_code; // Code of current edited map

GLOBAL_DATA int next_code; // Code for next created map

GLOBAL_DATA int exit_requested;

//GLOBAL_DATA
struct ttipo {             // Information for each file type
  char path[PATH_MAX + 1]; // Default path
  char *ext;               // Extensions e.g.: "*.MAP *.PCX *.*"
  int default_choice;      // Default extension selection
  int first_visible;       // File listbox position in open dialog
};

GLOBAL_DATA struct ttipo tipo[24]; // Paths 0-(current working directory),
                                   // 1-(path d.exe), 2-MAP, 3-PAL, 4-FPG, 5-FNT,
                                   // 6-IFS, 7-PCM, 8-PRG, 9-MAP(wallpapers),
                                   // 10-PAL(?), 11-PCM(save as,?), 12-PRJ,
                                   // 13-(generic *.*), 14-MAP(save), 15-WLD
                                   // 16-Music module

GLOBAL_DATA char drives[26 + 1];

GLOBAL_DATA char shift_status, ascii, scan_code;

GLOBAL_DATA char full[_MAX_PATH + 1];
GLOBAL_DATA char file_mask[512]; // File mask for directory listing
GLOBAL_DATA char input[512];     // Dialog box for entering the filename


GLOBAL_DATA int hidden[max_windows]; // Windows hidden when opening a dialog

GLOBAL_DATA struct tmapa *windows_to_create[max_windows];
GLOBAL_DATA int num_windows_to_create;

GLOBAL_DATA int mouse_shift; // For big mode, adjusts coordinates within the toolbar
GLOBAL_DATA int mouse_shift_x, mouse_shift_y;

GLOBAL_DATA int end_dialog;

GLOBAL_DATA int double_click, double_click_x, double_click_y; // For detecting double-click

GLOBAL_DATA int dragging, drag_x, drag_y, drag_graphic;

GLOBAL_DATA byte original_palette[768];

GLOBAL_DATA int drag_source, free_drag;

GLOBAL_DATA int menukey;

///////////////////////////////////////////////////////////////////////////////
//      Pulsed scan codes table
///////////////////////////////////////////////////////////////////////////////

GLOBAL_DATA byte kbdFLAGS[128];

#define key(x) kbdFLAGS[x]

void kbdInit(void);
void kbdReset(void);
void poll_keyboard(void);
void flush_buffer(void);

///////////////////////////////////////////////////////////////////////////////
//      Functions and structures exported by DIVFPG (divfpg.c)
///////////////////////////////////////////////////////////////////////////////

int new_file(void);
void open_file();
int RemapAllFiles(int vent);
void fpg_save(int n);
void Delete_Taggeds();
void Show_Taggeds();

//-----------------------------------------------------------------------------
//      Functions and structures exported by the trash can
//-----------------------------------------------------------------------------

void muestra_papelera(); // Show trash can

//-----------------------------------------------------------------------------
//      Functions and structures exported by the (sound) recorder
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//      Functions and structures exported by the font generator
//-----------------------------------------------------------------------------

void OpenGenFont();
void OpenFont();


// divbrow.c

void create_listbox(struct t_listbox *l);
void update_listbox(struct t_listbox *l);
void paint_listbox(struct t_listbox *l);


//-----------------------------------------------------------------------------
//      Functions and structures exported by DIVPCM
//-----------------------------------------------------------------------------

#define MOD 1
#define S3M 2
#define XM  3

typedef struct _pcminfo {
  char name[255];
  char pathname[256];
  int SoundFreq;
  int SoundBits;
  int SoundSize;
  short *SoundData;
#ifdef MIXER
  Mix_Chunk *SI;
#endif
  //        SAMPLE *sample;
  char *sample;
} pcminfo;

typedef struct _modinfo {
  char name[255];
  char pathname[256];
  int SongCode;
#ifdef MIXER
  Mix_Music *music;
#endif
} modinfo;

GLOBAL_DATA char *ExternUseBufferMap;
void PCM0(void);
void PCM1(void);
void PCM2(void);
void EditSound0(void);
void EditSound1(void);
void EditSound2(void);
extern char SoundName[255];
extern char SoundPathName[256];
extern char SongName[255];
extern char SongPathName[256];
extern byte *pcminfo_aux;
extern byte *modinfo_aux;

//-----------------------------------------------------------------------------
//      Functions and structures exported by DIVDSKTP
//-----------------------------------------------------------------------------

int Can_UpLoad_Desktop();
int UpLoad_Desktop();
void DownLoad_Desktop();
void New_DownLoad_Desktop();

//-----------------------------------------------------------------------------
//      Compiler functions
//-----------------------------------------------------------------------------

extern byte lower[256];

GLOBAL_DATA byte *source_ptr;
GLOBAL_DATA int source_len;
GLOBAL_DATA int saved_esp;

void init_compiler(void);
void compile_program(void);
void finaliza_compilador(void);

void compile(void); // Internal compiler functions
void comp(void);
void free_resources(void);
void comp_exit(void);

GLOBAL_DATA int run_mode; // 0-Compile, 1-Run, 2-Install

void goto_error(void);

//-----------------------------------------------------------------------------
//      Hypertext
//-----------------------------------------------------------------------------

void make_helpidx(void); // Create the term index
void load_index(void);   // Load the glossary
void help(int n);        // Create a hypertext window
void help0(void);
void help1(void);
void help2(void);
void help3(void);
void tabula_help(byte *si, byte *di, int lon);

//-----------------------------------------------------------------------------
//      Installation
//-----------------------------------------------------------------------------

void crear_instalacion(void);

//-----------------------------------------------------------------------------
//      Exported divedit functions
//-----------------------------------------------------------------------------

void process_list0(void);
void f_bop(void);
void f_home(void);
void write_line(void);
void read_line(void);
void retreat_lptr(void);
void retreat_vptr(void);
void advance_lptr(void);
void advance_vptr(void);

extern int lp1[512], lp_select;

//-----------------------------------------------------------------------------
//      Window movement functions
//-----------------------------------------------------------------------------

void copy(int a, int b);
void xchg(int a, int b);
void move(int a, int b);
void divdelete(int a);
void addwindow(void);
void wup(int a);
void wdown(int a);

//-----------------------------------------------------------------------------
//      Prototypes for divgama
//-----------------------------------------------------------------------------

void create_gradient_colors(struct _gcolor *gradient_config, byte *gradient_buf);

GLOBAL_DATA byte wallpaper_gradient[128];
GLOBAL_DATA byte explosion_gradient[128];

GLOBAL_DATA byte *gradient_buf;
GLOBAL_DATA struct _gcolor *gradient_config;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// M3D_info struct removed (MODE8/3D map editor deleted)

//-----------------------------------------------------------------------------
//  Structure for storing thumbnails
//-----------------------------------------------------------------------------

typedef struct {
  int w, h; // Width and height of the thumbnail
  char *ptr;  // ==NULL if the thumbnail has not started loading
  SDL_Surface *surfaceptr;
  int status;   // -1-Not an image, 0-Loaded, +N-Number of bytes read
  int filesize; // File size in bytes
  int tagged;   // Whether tagged or not (1/0)
} t_thumb;

GLOBAL_DATA int sound_freq;

//////////////////////////////////////////////////////////////////////////////
//  Video modes
//////////////////////////////////////////////////////////////////////////////

struct _modos {
  short width;
  short height;
  short mode;
};

GLOBAL_DATA struct _modos modos[32];

GLOBAL_DATA int num_modes;

GLOBAL_DATA char vga_marker[128];

GLOBAL_DATA int vesa_version;

void detect_vesa(void);

//////////////////////////////////////////////////////////////////////////////

GLOBAL_DATA int cpu_type; // 3 (386) or 5 (Pentium)

//////////////////////////////////////////////////////////////////////////////
//  Dumps debug information to a file
//////////////////////////////////////////////////////////////////////////////

void DebugInfo(char *Msg);
void DebugData(int Val);

//////////////////////////////////////////////////////////////////////////////

#endif // __GLOBAL_H
