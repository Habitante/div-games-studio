#ifndef __IFS_H_
#define __IFS_H_

#define IFS_MEM_ERROR    10
#define IFS_FORMAT_ERROR 20
#define IFS_OPEN_ERROR   30
#define IFS_READ_ERROR   40
#define IFS_WRITE_ERROR  50
#define IFS_PARAM_ERROR  60

#define NUM 1
#define MAY 2
#define MIN 4
#define SIM 8
#define EXT 16


// Structures for the input file (IFS)
typedef struct _ifs_table_entry {
  int32_t offset;
  int32_t size;
} ifs_table_entry;

typedef struct _ifs_header_t {
  uint8_t id[4];
  uint32_t offset8;
  uint32_t offset10;
  uint32_t offset12;
  uint32_t offset14;
  uint32_t offset128;
} ifs_header_t;

// Structure for the output file (FNT)
typedef struct _fnt_table_entry {
  int width;
  int height;
  int incY;
  int offset;
} fnt_table_entry;

typedef struct _fnt_header_t {
  char id[7];
  char eof;
} fnt_header_t;

// Required parameters
typedef struct _IFS {
  char ifsName[256];
  char fntName[256];
  char size_x;
  char size_y;
  char outline;
  char brightness; // (0-4) None, Up-Left, Down-Right, Up-Right, Down-Left
  short shadow_x;
  short shadow_y;
  unsigned short body_tex_w; // body letter texture
  unsigned short body_tex_h;
  char body_tex_color;
  char body_tex_mode;
  unsigned short shadow_tex_w; // shadow texture
  unsigned short shadow_tex_h;
  char shadow_tex_color;
  char shadow_tex_mode;
  unsigned short outline_tex_w; // outline texture
  unsigned short outline_tex_h;
  char outline_tex_color;
  char outline_tex_mode;
  char table[256];      // which characters to generate
  char highlights[256]; // color mappings for highlights
  char shadows[256];    // color mappings for shadows

} IFS;

extern char *body_tex_buffer, *out_tex_buffer, *shadow_tex_buffer;
extern IFS ifs;
int jorge_create_font(int GenCode);
int show_char(int WhatChar, int cx, int cy, char *ptr, int w);
int show_char_buffer(int WhatChar, int cx, int cy, char *ptr, int w, char *buffer);
void get_char_size(int WhatChar, int *width, int *height);
void get_char_size_buffer(int WhatChar, int *width, int *height, char *buffer);
void convert_fnt_to_pal(char *Buffer);
int show_progress(char *titulo, int current, int total);
#endif
