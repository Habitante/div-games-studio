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


// ESTRUCTURAS PARA EL FICHERO DE ENTRADA (IFS)
typedef struct _TABLAIFS {
  int32_t offset;
  int32_t size;
} TABLAIFS;

typedef struct _IFSHEADER {
  uint8_t id[4];
  uint32_t offset8;
  uint32_t offset10;
  uint32_t offset12;
  uint32_t offset14;
  uint32_t offset128;
} IFSHEADER;

// ESTRUCTURA PARA EL FICHERO DE SALIDA (FNT)
typedef struct _TABLAFNT {
  int width;
  int height;
  int incY;
  int offset;
} TABLAFNT;

typedef struct _FNTHEADER {
  char id[7];
  char eof;
} FNTHEADER;

// PARAMETROS NECESARIOS
typedef struct _IFS {
  char ifsName[256];
  char fntName[256];
  char tamX;
  char tamY;
  char outline;
  char brillo; // (0-4) Sin, Arr-Izq, Abj-drcha, Arr-dcha, Abj-izq.
  short sombraX;
  short sombraY;
  unsigned short bodyTexAncho; // textura de la letra
  unsigned short bodyTexAlto;
  char bodyTexColor;
  char bodyTexModo;
  unsigned short shadowTexAncho; // textura de la Sombra
  unsigned short shadowTexAlto;
  char shadowTexColor;
  char shadowTexModo;
  unsigned short outTexAncho; // textura del Outline
  unsigned short outTexAlto;
  char outTexColor;
  char outTexModo;
  char tabla[256];   // para indicar que letras se generan
  char claros[256];  // correspondencias para los brillos
  char oscuros[256]; // correspondencias para los brillos

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
