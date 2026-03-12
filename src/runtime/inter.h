
#ifdef DEFINE_GLOBALS_HERE
#define GLOBAL
#else
#define GLOBAL extern
#endif

#ifndef false
#define false 0
#define true  1
#endif


//#include "include.div"

#define INTERPRETE

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////


//#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <ctype.h>
//#include <direct.h>
#include <signal.h>
#include <time.h>
#include "osdep.h"

typedef void (*void_return_type_t)(void);
void call(const void_return_type_t func); // void funcion(void); int n=(int)funcion; call(n);


#include <math.h>

#include "keyboard.h"
#include "divfli.h"

#ifdef ZLIB
#include <zlib.h>
#endif

#include "divsound.h"

///////////////////////////////////////////////////////////////////////////////
// defines
///////////////////////////////////////////////////////////////////////////////


#ifdef DEBUG
#define debugprintf(...) printf(__VA_ARGS__)
#else
#define printf(...)
// debugprintf
#define debugprintf(...)
#endif

#ifdef _MSC_VER
#pragma check_stack(off)
#endif

#define DPMI_INT 0x31

#define uchar  unsigned char
#define byte   unsigned char
#define ushort unsigned short
#define word   unsigned short
#define ulong  unsigned int
#define dword  unsigned int

#define pi     180000
#define radian 57295.77951

///////////////////////////////////////////////////////////////////////////////
//  Prototypes
///////////////////////////////////////////////////////////////////////////////

// Interpreter (i.cpp)

void error(word);
void exer(int);
void mouse_window(void);

// Video (v.cpp)

void update_palette(void);
void set_dac(void);
void retrace_wait(void);
void fade_wait(void);
void setup_video_mode(void);
void reset_video_mode(void);
void init_flush(void);
void blit_partial(int, int, int, int);
void blit_screen(byte *);
void restore(byte *, byte *);
void init_ghost(void);
void create_ghost(void);
void find_color(byte, byte, byte);

// Sprites (s.cpp)

void scroll_simple(void);
void scroll_parallax(void);
void put_sprite(int file, int graph, int x, int y, int size, int angle, int flags, int reg, byte *,
                int, int);
void paint_sprite(void);
void sp_normal(byte *p, int x, int y, int w, int h, int flags);
void sp_clipped(byte *p, int x, int y, int w, int h, int flags);
void sp_scaled(byte *, int, int, int, int, int, int, int, int);
void sp_rotated(byte *, int, int, int, int, int, int, int, int, int);
void sp_scan(byte *p, short n, byte *si, int w, int x0, int y0, int x1, int y1);
void paint_texts(int n);
void paint_drawings(void);
void init_sin_cos(void);
void paint_m7(int);
int get_distx(int a, int d);
int get_disty(int a, int d);

// Functions (f.cpp)

void function(void);
void apply_palette(void);
void init_rnd(int);

// Collision routines (c.cpp)

void out_region(void);
void graphic_info(void);
void collision(void);
void sp_size_scaled(int *x, int *y, int *xx, int *yy, int xg, int yg, int size, int flags);
void put_collision(byte *buffer, int *ptr, int x, int y, int xg, int yg, int angle, int size,
                   int flags);
void sp_size(int *x, int *y, int *xx, int *yy, int xg, int yg, int ang, int size, int flags);

// Debug (d.cpp)

#ifdef DEBUG
void debug(void);
extern int debug_active;
#endif

///////////////////////////////////////////////////////////////////////////////
//      Functions exported by DIVLENGU (divlengu.cpp)
///////////////////////////////////////////////////////////////////////////////

void initialize_texts(byte *filename);
void finalize_texts(void);

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

#define max_exp         512  // Maximum number of elements in an expression
#define EVAL_STACK_SIZE 2048 // Execution stack length

#define swap(a, b) \
  {                \
    a ^= b;        \
    b ^= a;        \
    a ^= b;        \
  }

//-----------------------------------------------------------------------------
// Mnemonic/Opcode/Operands (EML code generation, "*" = "not yet used")
//-----------------------------------------------------------------------------

#define lnop 0  // *            No operation
#define lcar 1  // value        Push constant onto stack
#define lasi 2  //              Pop value and offset, store value at [offset]
#define lori 3  //              Logical OR
#define lxor 4  //              XOR, exclusive OR
#define land 5  //              Logical AND, condition operator
#define ligu 6  //              Equal, logical comparison operator
#define ldis 7  //              Not equal, true if 2 values differ
#define lmay 8  //              Greater than, signed comparison
#define lmen 9  //              Less than, signed comparison
#define lmei 10 //              Less than or equal
#define lmai 11 //              Greater than or equal
#define ladd 12 //              Add two constants
#define lsub 13 //              Subtract, binary operation
#define lmul 14 //              Multiply
#define ldiv 15 //              Integer division
#define lmod 16 //              Modulo, division remainder
#define lneg 17 //              Negate, flip sign of a constant
#define lptr 18 //              Pointer, pop offset and push [offset]
#define lnot 19 //              Bitwise NOT, bit by bit
#define laid 20 //              Add id to constant on stack
#define lcid 21 //              Push id onto stack
#define lrng 22 // offset, len  Perform range check
#define ljmp 23 // offset       Jump to a mem[] address
#define ljpf 24 // offset       Jump if false to an address
#define lfun 25 // code         Call an internal function, e.g. signal()
#define lcal 26 // offset       Create a new process in the program
#define lret 27 // num_par      Process self-termination
#define lasp 28 //              Discard a stacked value
#define lfrm 29 // num_par      Suspend process execution for this frame
#define lcbp 30 // num_par      Initialize local parameter pointer
#define lcpa 31 //              Pop offset, read parameter [offset] and bp++
#define ltyp 32 // block        Set current process type (for collisions)
#define lpri 33 // offset       Jump to address and load private variables
#define lcse 34 // offset       If switch != expression, jump to offset
#define lcsr 35 // offset       If switch not in range, jump to offset
#define lshr 36 //              Shift right (bitwise)
#define lshl 37 //              Shift left (bitwise)
#define lipt 38 //              Pre-increment and pointer
#define lpti 39 //              Pointer and post-increment
#define ldpt 40 //              Pre-decrement and pointer
#define lptd 41 //              Pointer and post-decrement
#define lada 42 //              Add-assign
#define lsua 43 //              Sub-assign
#define lmua 44 //              Mul-assign
#define ldia 45 //              Div-assign
#define lmoa 46 //              Mod-assign
#define lana 47 //              And-assign
#define lora 48 //              Or-assign
#define lxoa 49 //              Xor-assign
#define lsra 50 //              Shr-assign
#define lsla 51 //              Shl-assign
#define lpar 52 // num_par_pri  Set the number of private parameters
#define lrtf 53 // num_par      Process self-termination, returns a value
#define lclo 54 // offset       Clone current process
#define lfrf 55 // num_par      Pseudo-frame (frame at percentage, frame(100)==frame)
#define limp 56 // offset text  Import an external DLL
#define lext 57 // code         Call an external function
#define lchk 58 //              Validate an identifier
#define ldbg 59 //              Invoke the debugger

// Instructions added for optimization (DIV 2.0)

#define lcar2      60
#define lcar3      61
#define lcar4      62
#define lasiasp    63
#define lcaraid    64
#define lcarptr    65
#define laidptr    66
#define lcaraidptr 67
#define lcaraidcpa 68
#define laddptr    69
#define lfunasp    70
#define lcaradd    71
#define lcaraddptr 72
#define lcarmul    73
#define lcarmuladd 74
#define lcarasiasp 75
#define lcarsub    76
#define lcardiv    77

// Instructions added for string handling

#define lptrchr 78 // Pointer, pop (index, offset) and push [offset+byte index]
#define lasichr 79 // Pop (value, index, offset) and store value at [offset+byte index]
#define liptchr 80 // Pre-increment and pointer
#define lptichr 81 // Pointer and post-increment
#define ldptchr 82 // Pre-decrement and pointer
#define lptdchr 83 // Pointer and post-decrement
#define ladachr 84 // Add-assign
#define lsuachr 85 // Sub-assign
#define lmuachr 86 // Mul-assign
#define ldiachr 87 // Div-assign
#define lmoachr 88 // Mod-assign
#define lanachr 89 // And-assign
#define lorachr 90 // Or-assign
#define lxoachr 91 // Xor-assign
#define lsrachr 92 // Shr-assign
#define lslachr 93 // Shl-assign
#define lcpachr 94 // Pop offset, read parameter [offset] and bp++

#define lstrcpy 95  // Pop si, di, and do strcpy(mem[di],[si]) (leave di on stack)
#define lstrfix 96  // Extend a string before inserting a char into it
#define lstrcat 97  // Concatenate two strings (operates like strcpy)
#define lstradd 98  // Add two strings into a temporary and push pointer to it
#define lstrdec 99  // Add or remove characters from a string
#define lstrsub 100 // Remove characters from a string (-=)
#define lstrlen 101 // Replace a string with its length
#define lstrigu 102 // String equality comparison
#define lstrdis 103 // Strings not equal
#define lstrmay 104 // String greater than
#define lstrmen 105 // String less than
#define lstrmei 106 // String greater than or equal
#define lstrmai 107 // String less than or equal
#define lcpastr 108 // Load a parameter into a string

// Instructions added for word handling

#define lptrwor 109 // Pointer, pop (index, offset) and push [offset+word index]
#define lasiwor 110 // Pop (value, index, offset) and store value at [offset+word index]
#define liptwor 111 // Pre-increment and pointer
#define lptiwor 112 // Pointer and post-increment
#define ldptwor 113 // Pre-decrement and pointer
#define lptdwor 114 // Pointer and post-decrement
#define ladawor 115 // Add-assign
#define lsuawor 116 // Sub-assign
#define lmuawor 117 // Mul-assign
#define ldiawor 118 // Div-assign
#define lmoawor 119 // Mod-assign
#define lanawor 120 // And-assign
#define lorawor 121 // Or-assign
#define lxoawor 122 // Xor-assign
#define lsrawor 123 // Shr-assign
#define lslawor 124 // Shl-assign
#define lcpawor 125 // Pop offset, read parameter [offset] and bp++

// Miscellaneous

#define lnul 126 // Check that a pointer is not NULL

//----------------------------------------------------------------------------- // Global variables of the programs //-----------------------------------------------------------------------------

#define long_header 9 // Header length at the start of programs

GLOBAL int imem_max; // Main memory of the target machine

struct _mouse { // x1
  int x, y, z, file, graph, angle, size, flags, region, left, middle, right, cursor, speed;
};

GLOBAL struct _mouse *mouse;

struct _scroll { // x10
  int z, camera, ratio, speed, region1, region2, x0, y0, x1, y1;
};

GLOBAL struct _scroll *scroll;

struct _m7 { // x10
  int z, camera, height, distance, horizon, focus, color;
};

GLOBAL struct _m7 *m7;

struct _joy { // x1
  int button1, button2, button3, button4;
  int left, right, up, down;
};

GLOBAL struct _joy *joy;

struct _setup { // x1
  int card, port, irq, dma, dma2;
  int master, sound_fx, cd_audio;
  int mixer, mixrate, mixmode;
};

GLOBAL struct _setup *setup;

struct _net { // x1
  int device, com, speed;
  int phone, init_string;
  int tones, server, num_players;
  int act_players;
};

GLOBAL struct _net *net;

// struct _m8 removed (MODE8 deleted)

struct _dirinfo { // x1
  int files;
  int name[1025];
};

GLOBAL char *filenames;

GLOBAL struct _dirinfo *dirinfo;

struct _fileinfo { // x1
  int fullpath_fix;
  char fullpath[256]; // Full path name
  int drive;          // Disk drive
  int dir_fix;
  char dir[256]; // Directory
  int name_fix;
  char name[12]; // File name
  int ext_fix;
  char ext[8]; // Extension
  int size;    // Size (in bytes)
  int day;     // Day
  int month;   // Month
  int year;    // Year
  int hour;    // Hour
  int min;     // Minute
  int sec;     // Second
  int attrib;  // Attributes
};

GLOBAL struct _fileinfo *fileinfo;

struct _video_modes { // x100
  int width;          // Width of the video mode
  int height;         // Height of the video mode
  int mode;           // Video mode number
};

GLOBAL struct _video_modes *video_modes;

// WARNING: end_struct offset must match the initialization in i.c and div.h

#define end_struct long_header + 14 + 10 * 10 + 10 * 7 + 8 + 11 + 9 + 10 * 4 + 1026 + 146 + 32 * 3

#define timer(x) mem[end_struct + x]

GLOBAL int mtimer[10];
GLOBAL int otimer[10];

// text_z = -256
#define text_z mem[end_struct + 10]

#define fading           mem[end_struct + 11]
#define shift_status     mem[end_struct + 12]
#define ascii            mem[end_struct + 13]
#define scan_code        mem[end_struct + 14]
#define joy_filter       mem[end_struct + 15]
#define joy_status       mem[end_struct + 16]
#define restore_type     mem[end_struct + 17]
#define dump_type        mem[end_struct + 18]
#define max_process_time mem[end_struct + 19]
#define fps              mem[end_struct + 20]
#define _argc            mem[end_struct + 21]
#define _argv(x)         mem[end_struct + 22 + x]
#define channel(x)       mem[end_struct + 32 + x]
#define vsync            mem[end_struct + 64]

// draw_z = -255
#define draw_z          mem[end_struct + 65]
#define num_video_modes mem[end_struct + 66]
#define unit_size       mem[end_struct + 67]

GLOBAL int joy_timeout;

void read_joy(void);

//-----------------------------------------------------------------------------
// Sprite system local variables (the first ones are not public)
//-----------------------------------------------------------------------------

#define _Id       0  //Used to validate external accesses
#define _IdScan   1  //Scan index for iterating other processes (e.g. collisions)
#define _Bloque   2  //Process type identifier (for collisions)
#define _BlScan   3  //Last process type scanned in the last iteration
#define _Status   4  //State (0 dead, 1 killed, 2 alive, 3 asleep, 4 frozen)
#define _NumPar   5  //Number of process parameters
#define _Param    6  //Pointer to parameters passed to the process (on stack)
#define _IP       7  //Instruction pointer (next after previous frame)
#define _SP       8  //Stack pointer (process stack pointer)
#define _Executed 9  //Whether the process has already executed this frame
#define _Painted  10 //Whether the process has already been painted

// The following 2 variables are dual-purpose, depending on mode 7 or mode 8

#define _Dist1 11 //Distance 1, for mode 7
#define _Dist2 12 //Distance 2, for mode 7

// _M8_Object removed (MODE8 deleted)
#define _Old_Ctype 12 //Previous _Ctype

#define _Frame  13 //Frame progress of the process (frame(n))
#define _x0     14 //Bounding box occupied by the sprite
#define _y0     15 // each time it is painted, used for
#define _x1     16 // blit_screen and background restoration
#define _y1     17 // (dump_type==0 and restore_background==0)
#define _FCount 18 //Function call count (to skip returns during frame)
#define _Caller 19 //ID of the calling process or function (0 if kernel)

#define _Father     20 //Parent process id (0 if none)
#define _Son        21 //Last child created (0 if none)
#define _SmallBro   22 //Younger sibling id (0 if none)
#define _BigBro     23 //Older sibling id (0 if none)
#define _Priority   24 //Process priority (positive or negative)
#define _Ctype      25 //Coordinate type: screen, parallax, or mode 7
#define _X          26 //X coordinate (of the graphic's center of gravity)
#define _Y          27 //Y coordinate (same)
#define _Z          28 //Z coordinate (rendering priority)
#define _Graph      29 //Graphic code (corresponds to files)
#define _Flags      30 //Horizontal and vertical mirror flags
#define _Size       31 //Graphic size (percentage)
#define _Angle      32 //Graphic rotation angle (0 = normal)
#define _Region     33 //Clipping region for the graphic
#define _File       34 //FPG containing the process graphics
#define _XGraph     35 //Pointer to table: num_graphics,graphic_angle_0,...
#define _Height     36 //Process height in mode 7 (pix/4)
#define _Cnumber    37 //Which scroll or m7 will display the graphic
#define _Resolution 38 //Coordinate resolution for x,y of this process
// _Radius, _M8_Wall, _M8_Sector, _M8_NextSector, _M8_Step removed (MODE8 deleted)

//-----------------------------------------------------------------------------
//  Target machine memory
//-----------------------------------------------------------------------------

GLOBAL int
    pila[EVAL_STACK_SIZE + max_exp + 64]; // Expression evaluation (compilation and execution)

GLOBAL int *mem, imem, iloc, iloc_pub_len, iloc_len;
GLOBAL byte *memb;
GLOBAL word *memw;

//-----------------------------------------------------------------------------
// Global variables for interpretation - PROCESS VARIABLES
//-----------------------------------------------------------------------------

GLOBAL int private_start; // Start of private variables (running process)

GLOBAL int ip; // Program pointer (instruction pointer)

GLOBAL int sp; // Stack pointer

GLOBAL int bp; // Auxiliary stack pointer (base pointer)

GLOBAL int id_init; // Start of the init process (parent of all)

GLOBAL int id_start; // Start of the first process (its locals and privates)

GLOBAL int id_end; // Start of the last process created so far

GLOBAL int id_max;

GLOBAL int id_old; // Tracks which process is currently being executed

GLOBAL int process_count; // Number of living processes in the program

GLOBAL int ide, id; // Current process being processed

GLOBAL int id2; // Extra identifier for process calls (cal)

//-----------------------------------------------------------------------------
// Global variables for file handle management
//-----------------------------------------------------------------------------

GLOBAL int numfiles; // Number of open files when interpreter started

GLOBAL FILE *tabfiles[32]; // Table of open handles (0 = free)

//-----------------------------------------------------------------------------
// Global variables for interpretation - GRAPHICS VARIABLES
//-----------------------------------------------------------------------------

GLOBAL int vga_width, vga_height; // Physical screen dimensions
GLOBAL int vwidth, vheight;       // Screen window dimensions
GLOBAL int vvga_w, vvga_h;        // Physical screen dimensions (legacy)

GLOBAL byte fsmode;

GLOBAL char prgpath[_MAX_PATH];

GLOBAL byte *screen_buffer; // Virtual screen copy (working framebuffer)

GLOBAL byte *back_buffer; // Second copy, sprite background outside scroll

#ifdef DEBUG
GLOBAL byte *screen_buffer_debug; // Third copy, debug only (dialogs)
#endif

GLOBAL byte palette[768]; // Current program palette

GLOBAL int palcrc; // CRC of the current program palette

GLOBAL int auto_adapt_palette; // Auto-adapt loaded files to the active palette

GLOBAL byte dac[768]; // Active palette on screen

GLOBAL byte dac4[768]; // Palette multiplied by 4

GLOBAL int dacout_r, dacout_g, dacout_b, dacout_speed; // Fade: amount to subtract and speed

GLOBAL int now_dacout_r, now_dacout_g, now_dacout_b; // Current state of dac[]

GLOBAL int palette_loaded; // Whether a palette has been loaded

GLOBAL int apply_palette_flag; // Whether the palette needs to be applied

GLOBAL byte *color_lookup;

//-----------------------------------------------------------------------------
// Output texts, in translatable format
//-----------------------------------------------------------------------------

#define max_system_texts 256 // Max number of allowed texts (lenguaje.div)

GLOBAL byte *text[max_system_texts];
GLOBAL int num_error;

//-----------------------------------------------------------------------------
// Graphic files (*.FPG from DIV)
//-----------------------------------------------------------------------------

typedef struct _t_g { // Structure for an FPG file
  int **fpg;          // File loaded in memory
  int **grf;          // Pointers to graphics (g[n].grf[000..999])
} t_g;

// The first FPG can hold up to 2000 graphics; from 1000 onward they are
// graphics loaded with load_map (codes 1000..1999)

GLOBAL int next_map_code, max_grf;

#define max_fpgs 64 // Maximum number of loaded FPGs

GLOBAL t_g g[max_fpgs]; // Array of FPG files

//-----------------------------------------------------------------------------
// Generic variables used by multiple functions
//-----------------------------------------------------------------------------

GLOBAL FILE *es; // File reading during interpretation (fpg, voc, ...)

GLOBAL int file_len; // File reading during interpretation

GLOBAL word *ptr; // General pointer for a runtime malloc

GLOBAL int x, y; // Generic coordinates for use in internal functions

GLOBAL float angle; // Generic angle for use in internal functions

//-----------------------------------------------------------------------------
// Display region system
//-----------------------------------------------------------------------------

#define max_region 32 // Maximum number of defined regions

typedef struct _t_region { // Clipping zones, relative to screen
  int x0, x1;
  int y0, y1;
} t_region;

GLOBAL t_region region[max_region]; // Array of regions

GLOBAL int clipx0, clipx1, clipy0, clipy1; // Clipping region

//-----------------------------------------------------------------------------
// Font system (*.FNT files generated with DIV)
//-----------------------------------------------------------------------------

typedef struct _fnt_table_entry {
  int width;
  int height;
  int incY;
  int offset;
} fnt_table_entry;

typedef struct _fnt_info {
  int width;          // Average font width
  int spacing;        // Space character width in pixels
  int letter_spacing; // Inter-character spacing (in addition to glyph width)
  int height;         // Maximum font height
  int fonpal;         // CRC of its palette
  int syspal;         // CRC of the palette it is adapted to
  int len;            // FNT file length
  char name[80];      // FNT file name
} fnt_info;

#define max_fonts 32 // Maximum number of fonts at runtime

GLOBAL byte *fonts[max_fonts]; // Fonts loaded at runtime (0=not loaded)

GLOBAL fnt_table_entry *fnt;

GLOBAL fnt_info f_i[max_fonts];

//-----------------------------------------------------------------------------
// Text rendering system
//-----------------------------------------------------------------------------

#define max_texts 256 // Maximum number of texts at runtime

typedef struct _t_text_display {
  int type;      // Text type: 0=normal, 1=&variable
  byte *font;    // Pointer to font (byte h,car[256].an/.dir,data...)
  int x, y;      // Text coordinates
  int ptr;       // Text content
  int alignment; // Alignment type: 0=left, 1=centered, ...
  int region;    // Clipping region
  int x0, y0;    // Region occupied by the text
  int w, h;      // For partial screen blits
} t_text;

GLOBAL t_text texts[max_texts + 1];

//-----------------------------------------------------------------------------
// Drawing primitives system
//-----------------------------------------------------------------------------

#define max_drawings 16384 // Maximum number of primitives at runtime

typedef struct _t_drawing {
  int type;    // Primitive type: 0=n/a, 1=line, ...
  int color;   // Primitive color
  int opacity; // 0=minimum ... 15=opaque
  int region;  // Clipping region
  int x0, y0;  // Top-left coordinate of the primitive
  int x1, y1;  // Bottom-right coordinate of the primitive
} t_drawing;

GLOBAL t_drawing drawing[max_drawings];

//-----------------------------------------------------------------------------
// Partial blit system (games without scroll) - Now implemented
//-----------------------------------------------------------------------------

GLOBAL int full_redraw; // Whether the entire VGA copy has been modified
                        // Currently always kept at 1

// Now implemented; the variable controlling the blit_screen type
// is the global dump_type, accessible by programs

//-----------------------------------------------------------------------------
// Mode 7 system - Internal struct
//-----------------------------------------------------------------------------

struct _im7 {
  int on, painted;
  int x, y, w, h;
  byte *map, *ext;
  int map_width, map_height;
  int ext_w, ext_h;
};

GLOBAL struct _im7 im7[10];

//-----------------------------------------------------------------------------
// Parallax scroll system - Internal struct
//-----------------------------------------------------------------------------

#define max_inc 32

typedef struct _tfast { // Increment table for the foreground plane
  int nt;               // 0..max_inc-1 number of spans, >=max_inc overflow
  short inc[max_inc];   // Skip,data,skip,data,...
} tfast;                // Up to 1024x768

struct _iscroll { // x10
  int on, painted;
  int x, y, w, h;
  byte *_sscr1, *sscr1;
  byte *_sscr2, *sscr2;
  byte *map1, *map2;
  int map1_w, map1_h, map2_w, map2_h;
  int map_flags;
  int map1_x, map1_y, map2_x, map2_y;
  int block1, block2;
  int iscan;
  tfast *fast;
};

// Usage of the fast table:
//   iscroll[snum].fast=(tfast*)malloc(iscroll[snum].al*sizeof(tfast));
//   fast=iscroll[snum].fast;
//   fast[n].nt; // iscroll[snum].iscan;

GLOBAL struct _iscroll iscroll[10];

GLOBAL int snum; // Indicates which scroll is being processed

///////////////////////////////////////////////////////////////////////////////
// Dynamic system memory - Internal Struct
///////////////////////////////////////////////////////////////////////////////

struct _divmalloc {
  byte *ptr;
  memptrsize imem1;
  memptrsize imem2;
};

GLOBAL struct _divmalloc divmalloc[256];

///////////////////////////////////////////////////////////////////////////////
// Variables for palette control
///////////////////////////////////////////////////////////////////////////////

GLOBAL byte *ghost;         // Ghost blending table
GLOBAL byte *ghost_inicial; // First 256 half-blends of the ghost table

GLOBAL byte _r, _g, _b, find_col; // Palette calculations (ghost table)
GLOBAL int find_min;

GLOBAL byte last_c1; // Last system font color (in loaded palette)

///////////////////////////////////////////////////////////////////////////////
// Frame limiter
///////////////////////////////////////////////////////////////////////////////

GLOBAL int frame_clock; // Speed limiter (framerate)
GLOBAL int ticks;

GLOBAL int old_clock; // For time (timing) functions
GLOBAL int last_clock;
GLOBAL double fractional_clock, clock_interval;
GLOBAL int game_fps;

GLOBAL int max_frame_skips; // Ma number of skipped frames
GLOBAL int dfps;
GLOBAL SDL_Joystick *divjoy;
GLOBAL int skip_blit, blits_skipped;

//////////////////////////////////////////////////////////////////////////////
// Collision detection buffer
//////////////////////////////////////////////////////////////////////////////

GLOBAL byte *buffer;           // Collision Buffer
GLOBAL int buffer_w, buffer_h; // Width & Height of buffer

//////////////////////////////////////////////////////////////////////////////
// FPG Graphic files format
//////////////////////////////////////////////////////////////////////////////

// char     8   header "fpg\x1a\x0d\x0a\x00\x00"
// char   768   dac (palette)
// char 16*36   color gradients
// --------------------- 1 ---------------------------
// int      1   graphic code (000-999)
// int      1   graphic length including header
// char    32   graphic description
// char    12   source file name it was created from
// int      1   width
// int      1   height
// int      1   n - number of control points
// short  2*n   points {x,y}
// char  w*h    graphic data
// --------------------- N ---------------------------

#ifdef DEBUG

//////////////////////////////////////////////////////////////////////////////
//      Include prototypes and variables related to the debugger
//////////////////////////////////////////////////////////////////////////////

#define v           window[0]
#define max_items   24 // Maximum number of items in a window
#define max_windows 8  // Maximum number of windows

GLOBAL int big, big2; // big(0,1), big2(1,2)
GLOBAL int mouse_graf;

GLOBAL byte c0, c1, c2, c3, c4, text_color; // Main loop colors
GLOBAL byte c01, c12, c23, c34;             // Intermediate colors
GLOBAL byte c_r, c_g, c_b, c_r_low, c_g_low, c_b_low;

GLOBAL byte *mouse_background; // Buffer to save the mouse background

GLOBAL byte *graf_ptr, *graf[256]; // Main loop graphics
GLOBAL byte *text_font;            // Standard font, 7 pixels tall, proportional width

GLOBAL int wmouse_x, wmouse_y; // Mouse position within a window
GLOBAL int prev_mouse_buttons;

typedef struct _t_item {
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
      int x, y, w, buffer_len;
      int r0, r1;
    } get;
    struct {
      byte *text;
      int *value;
      int x, y;
    } flag;
  };
} t_item;

typedef struct _twindow {
  int type;       // 0-n/a, 1-dialog
  int foreground; // 1-yes 0-no (darkened)
  byte *title;    // Title bar text
  void_return_type_t paint_handler, click_handler, close_handler;
  int x, y, w, h; // Window position and dimensions
  byte *ptr;      // Window buffer
  int state;
  int redraw;             // Needs-redraw flag
  t_item item[max_items]; // Buttons, gets, switches, etc.
  int items;              // Number of defined items
  int active_item;        // When an item triggers an effect
  int selected_item;      // Currently selected item (for keyboard)
} twindow;

GLOBAL twindow window[max_windows];

struct t_listbox {
  int x, y;          // Listbox position in window
  char *list;        // List pointer
  int item_width;    // Characters per item
  int visible_items; // Visible item count
  int w, h;          // Text zone width in pixels
  int first_visible; // First visible index (from 0)
  int total_items;   // Total item count (0 n/a)
  int s0, s1, slide; // Slide bar start, end, current position
  int zone;          // Selected zone
  int buttons;       // Pressed button: up(1) or down(2)
  int created;       // Whether list is already created on screen
};

GLOBAL char *v_title; // Window title
GLOBAL char *v_text;  // Window text
GLOBAL int v_accept;  // Accept / Cancel

GLOBAL int end_dialog;

GLOBAL int debugger_step, call_to_debug, process_stoped;

//----------------------------------------------------------------------------
//      Debug functions
//----------------------------------------------------------------------------

void init_debug(void);
void end_debug(void);
void debug(void);
void init_colors(void);
void init_big(void);

//----------------------------------------------------------------------------
//  Debugger breakpoints
//----------------------------------------------------------------------------

#define max_breakpoint 32

struct _breakpoint {
  int line;   // Source line for that breakpoint
  int offset; // mem[] offset for debug fixup
  int code;   // code to fixup
};

GLOBAL struct _breakpoint breakpoint[max_breakpoint];

GLOBAL int ibreakpoint;

GLOBAL int new_palette, new_mode;

#endif

GLOBAL int v_function; // Currently executing function

//----------------------------------------------------------------------------

void frame_start(void);
void frame_end(void);

//----------------------------------------------------------------------------

byte average_color(byte a, byte b);

GLOBAL int dr, dg, db;

//----------------------------------------------------------------------------
GLOBAL byte kbdFLAGS[128];

#define key(x) kbdFLAGS[x]

void kbd_init(void);
void kbd_reset(void);
void poll_keyboard(void);
void flush_buffer(void);

//----------------------------------------------------------------------------

GLOBAL int x0s, x1s, y0s, y1s; // Region occupied by a sprite when painted

//----------------------------------------------------------------------------

struct _callback_data {
  unsigned short mouse_action;
  unsigned short mouse_bx;
  unsigned short mouse_cx;
  unsigned short mouse_dx;
};

typedef struct _callback_data callback_data;
extern callback_data cbd;

void mouse_on(void);
void mouse_off(void);
void set_mouse(int, int);

extern int _mouse_x, _mouse_y;

//-----------------------------------------------------------------------------
// Functions to be implemented in DLLs
//-----------------------------------------------------------------------------

// DLL_2

GLOBAL void (*set_video_mode)();
GLOBAL void (*process_palette)();
GLOBAL void (*process_active_palette)();

GLOBAL void (*process_sound)(char *sound, int sound_lenght);

GLOBAL void (*process_fpg)(char *fpg, int fpg_lenght);
GLOBAL void (*process_map)(char *map, int map_lenght);
GLOBAL void (*process_fnt)(char *fnt, int fnt_lenght);

GLOBAL void (*background_to_buffer)();
GLOBAL void (*buffer_to_video)();

GLOBAL void (*post_process_scroll)();
GLOBAL void (*post_process_m7)();
GLOBAL void (*pre_process_buffer)();
GLOBAL void (*post_process_buffer)();

GLOBAL void (*post_process)();

GLOBAL void (*putsprite)(byte *si, int x, int y, int w, int h, int xg, int yg, int ang, int size,
                         int flags);

GLOBAL void (*ss_init)();
GLOBAL void (*ss_frame)();
GLOBAL void (*ss_end)();

void text_out(char *texts, int x, int y);
int _random(int min, int max);

GLOBAL int ss_time;
GLOBAL int ss_time_counter;
GLOBAL int ss_status;
GLOBAL int ss_exit;

//----------------------------------------------------------------------------

GLOBAL char packfile[128];
GLOBAL int npackfiles;

struct _packdir {
  char filename[16];
  int offset;
  int len;
  int len_desc;
};

GLOBAL struct _packdir *packdir;

GLOBAL byte *packptr;

int validate_address(int dir); // Validate addresses (0 if out of bounds)

GLOBAL char divpath[PATH_MAX + 1];
GLOBAL unsigned divnum;

//-----------------------------------------------------------------------------
//  Video modes
//-----------------------------------------------------------------------------

GLOBAL int vesa_version;

void detect_vesa(void);

//----------------------------------------------------------------------------
//  Dump information to a file
//----------------------------------------------------------------------------

void debug_info(char *msg);
void debug_data(int val);

//----------------------------------------------------------------------------

GLOBAL int demo;

//----------------------------------------------------------------------------
//      Runtime error messages
//----------------------------------------------------------------------------

void e(int text_id);

GLOBAL int skipped[128];
GLOBAL int num_skipped;

extern char *fname[];
