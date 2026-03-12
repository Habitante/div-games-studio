
//-----------------------------------------------------------------------------
//      Code editor — shared internal header
//-----------------------------------------------------------------------------
//  Shared state, constants, and forward declarations used across:
//    editor.c, editor_edit.c, editor_render.c, editor_file.c
//-----------------------------------------------------------------------------

#ifndef EDITOR_INTERNAL_H
#define EDITOR_INTERNAL_H

#include "global.h"
#include "div_string.h"

//-----------------------------------------------------------------------------
//      Constants
//-----------------------------------------------------------------------------

#define BUFFER_INCREASE 16384 // Edit buffer growth increment
#define buffer_min  2048  // Minimum margin space

#define cr  13 // Carriage return
#define lf  10 // newline / linefeed
#define tab 9  // tabulation

//-----------------------------------------------------------------------------
//      Lexical colorizer token types
//-----------------------------------------------------------------------------

#define p_end_of_file 0x00  // End of file <EOF>
#define p_rem    0x7f  // Single-line comment
#define p_id     0xfd  // Identifier
#define p_num    0xfe  // Number
#define p_spc    0x100 // Whitespace
#define p_sym    0x101 // Symbol
#define p_lit    0x102 // Quoted literal
#define p_res    0x103 // Reserved keyword
#define p_pre    0x104 // Predefined identifier

//-----------------------------------------------------------------------------
//      Externs from other modules
//-----------------------------------------------------------------------------

extern int ibuf; // A pointer to the queue buffer
extern int fbuf; // Pointer to queue buffer end

extern int error_number; // Error number
extern int error_line;   // Line error (note, can be num_lines + 1)
extern int error_col;    // Error column num

extern int color_token;    // Token read by color_lex()
extern byte *color_source; // Pointer to source, for compiling the program
extern int in_comment;     // Whether inside a comment
extern int numrem;

extern byte m_b;

extern char user1[128], user2[128];

#define MAX_FILES 512
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[MAX_FILES];
extern int num_taggeds;

//-----------------------------------------------------------------------------
//      Shared editor state (defined in editor.c)
//-----------------------------------------------------------------------------

extern char line_buffer[LONG_LINE + 4]; // Copy of the line being edited
extern int cursor_mode;                 // 0-insert, 1-overwrite
extern long block_state;                // 0-none, 1-pivot, 2-complete
extern byte *block_start, *block_end;   // Block line pointers
extern int block_col1, block_col2;      // Block column bounds
extern struct tprg *kprg;               // Prg that owns the block
extern char *clipboard;                 // Clipboard pointer
extern int clipboard_len;               // Clipboard byte length
extern int clipboard_lines;             // Clipboard line break count
extern int clipboard_type;              // 0-char block, 1-line block
extern int edit_block_mode;             // 0-none, 1-single-line, 2-multi-line
extern int skip_full_blit;              // Skip full screen blits flag
extern char color_cursor;               // Color of char under cursor
extern int forced_slider;               // Forced vertical slider position
extern char colin[1024];                // Line syntax coloring buffer
extern int _cached_in_comment;
extern char *_cached_color_source;
extern char *c_oldline;
extern int old_comment_state;
extern struct tprg *edited;             // Currently edited program
extern int t_p;                         // Clipboard type for cut operations

//-----------------------------------------------------------------------------
//      Functions in editor.c (core)
//-----------------------------------------------------------------------------

void editor(void);
void check_memory(int block_len);
int linelen(byte *p);
void program0(void);

//-----------------------------------------------------------------------------
//      Functions in editor_edit.c (editing operations)
//-----------------------------------------------------------------------------

void f_mark(void);
void f_unmark(void);
void f_cut_block(int mode);
void f_cut(int mode);
void f_paste_block(void);
void test_cursor2(void);

void f_right(void);
void f_left(void);
void f_down(void);
void f_up(void);
void f_home(void);
void f_end(void);
void f_scroll(int n);
void f_mscroll(int n);
void f_page_down(void);
void f_page_up(void);
void f_insert(void);
void f_delete_char(void);
void f_backspace(void);
void f_tab(void);
void f_untab(void);
void f_overwrite(void);
void f_insert_char(void);
void f_enter(void);
void f_delete(void);
void f_word_right(void);
void f_word_right2(void);
void f_word_left(void);
void f_eof(void);
void f_bof(void);
void f_eop(void);
void f_bop(void);

void advance_vptr(void);
void advance_lptr(void);
void retreat_vptr(void);
void retreat_lptr(void);
byte *advance(byte *q);
byte *retreat(byte *q);

void read_line(void);
void write_line(void);
void delete_line(void);
void remove_spaces(void);

//-----------------------------------------------------------------------------
//      Functions in editor_render.c (rendering)
//-----------------------------------------------------------------------------

void color_lex(void);
void fill_color_line(void);
void _completo(void);
void _parcial(void);
void scrollbars(void);
int get_slide_y(void);
int get_slide_x(void);
void info_bar(void);
void resize(void);
void maximize(void);
void repaint_window(void);
void test_cursor(void);
int in_block(void);
void text_cursor(void);
void error_cursor(void);
void error_cursor2(void);
void delete_text_cursor(void);
void put_char(byte *ptr, int w, byte c, int block);
void put_char2(byte *ptr, int w, byte c, byte color);
void put_char3(byte *ptr, int w, byte c, int block, byte color);

//-----------------------------------------------------------------------------
//      Functions in editor_file.c (file I/O, dialogs)
//-----------------------------------------------------------------------------

void open_program(void);
void program0_new(void);
void save_program(void);
void open_program_external(char *nombre, char *path);
void find_text0(void);
void find_text(void);
void replace_text0(void);
void replace_text(void);
void process_list0(void);
void goto_error(void);
void print_program(void);

#endif // EDITOR_INTERNAL_H
