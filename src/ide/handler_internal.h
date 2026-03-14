//-----------------------------------------------------------------------------
//      Shared header for handler_*.c modules
//
//      handler.c          - Menu infrastructure + top-level menus + finders
//      handler_dialogs.c  - Dialog boxes, listbox UI, file browser, progress
//      handler_map.c      - Map viewer, open/save/new map, resize, search
//      handler_fonts.c    - Font map generation
//-----------------------------------------------------------------------------

#ifndef HANDLER_INTERNAL_H
#define HANDLER_INTERNAL_H

#include "global.h"
#include "div_string.h"
#include "sound.h"
#include "fpg.h"

//-----------------------------------------------------------------------------
//      Module-shared variables (defined in handler.c)
//-----------------------------------------------------------------------------

extern byte aux_palette[PALETTE_SIZE];
extern int new_status;

//-----------------------------------------------------------------------------
//      handler.c - Menu infrastructure and window finders
//-----------------------------------------------------------------------------

void create_menu(int menu);
void paint_menu(int menu);
void update_menu(int menu, int min, int max);

int find_program_window(void);
void save_prg_buffer(memptrsize n);
int find_map_window(void);
int find_and_load_map(void);
int find_fpg_window(void);
int find_font_window(void);
int find_pcm_window(void);
int find_mod_window(void);
int check_file(void);
void update_title(void);

//-----------------------------------------------------------------------------
//      handler_dialogs.c - Dialogs, listbox UI, file browser
//-----------------------------------------------------------------------------

// Most dialog init functions (err0, accept0, copyright0, info0, etc.)
// and listbox functions (create_listbox, update_listbox, paint_listbox)
// are already declared in global.h.

void analyze_input(void);
void errhlp0(void);

// Listbox data buffers (extern'd by browser.c)
extern char files_buf[];
extern char dirs_buf[];
extern char drives_buf[];

extern struct t_listbox file_list;
extern struct t_listbox dir_list;
extern struct t_listbox drive_list;
extern struct t_listbox ext_list;

// Shared dialog state
extern byte work_palette[PALETTE_SIZE];
extern char user1[128];
extern char user2[128];

//-----------------------------------------------------------------------------
//      handler_map.c - Map viewer and operations
//-----------------------------------------------------------------------------

void open_map(void);
void save_map(void);
void map_search(void);
void calculate_foreground(void);

extern FILE *file_paint_fpg;

//-----------------------------------------------------------------------------
//      handler_fonts.c - Font map generation
//-----------------------------------------------------------------------------

void print_fontmap(void);
void generate_fontmap(void);
void generate_character(byte *di, int w, int h, int inc, char *si);

#endif // HANDLER_INTERNAL_H
