
//-----------------------------------------------------------------------------
//      Módulo para la gestión de lang_buffer (lenguaje.div) y de encriptación
//-----------------------------------------------------------------------------

#include "global.h"

byte *lang_buffer;
byte *lang_buffer_end;

int text_count = 0; // Number of texts
byte *p, *q;        // Punteros de lectura y escritura respectivamente.


///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////
void analyze_texts(void);
void coder(byte *ptr, int len, char *key);
void analyze_number(void);
void analyze_text(void);
void analyze_comment(void);


//-----------------------------------------------------------------------------
//      Inicializa el sistema de lang_buffer
//-----------------------------------------------------------------------------

void initialize_texts(byte *filename) {
  FILE *f;
  int n;

  memset(texts, 0, max_texts * 4);

  if ((f = fopen((char *)filename, "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    if ((lang_buffer = (byte *)malloc(n)) != NULL) {
      fseek(f, 0, SEEK_SET);
      n = fread(lang_buffer, 1, n, f);
      fclose(f);
      lang_buffer_end = lang_buffer + n;
      analyze_texts();
    } else
      fclose(f);
  }
}

//-----------------------------------------------------------------------------
//      Función de análisis de los lang_buffer
//-----------------------------------------------------------------------------

void analyze_texts(void) {
  q = p = lang_buffer;

  if (!strcmp((char *)p, "Zk!")) {
    p += 4;
    coder(p, lang_buffer_end - lang_buffer - 4, "lave");
  }

  while (p < lang_buffer_end)
    if (*p >= '0' && *p <= '9')
      analyze_number();
    else if (*p == '"')
      analyze_text();
    else if (*p == '#')
      analyze_comment();
    else
      p++;
}

void analyze_number(void) {
  text_count = 0;
  do {
    text_count = text_count * 10 + *p - '0';
    p++;
  } while (*p >= '0' && *p <= '9' && p < lang_buffer_end);
  if (text_count >= max_texts)
    text_count = 0;
}

void analyze_comment(void) {
  while (*p != '\n' && *p != '\r' && p < lang_buffer_end)
    p++;
}

void analyze_text(void) {
  texts[text_count] = q;
  p++;
  text_count++;
  while (*p != '"' && p < lang_buffer_end && *p != '\r' && *p != '\n') {
    if (*p == '\\') {
      p++;
      if (*p == 'n') {
        *q++ = 13;
        *q++ = 10;
        p++;
      } else if (*p == '#') {
        *q++ = 13;
        p++;
      } else
        *q++ = *p++;
    } else
      *q++ = *p++;
  }
  p++;
  *q++ = 0;
}

//-----------------------------------------------------------------------------
//      Finaliza el sistema de lang_buffer
//-----------------------------------------------------------------------------

void finalize_texts(void) {
  free(lang_buffer);
}

//----------------------------------------------------------------------------
//      Generador de números aleatorios
//----------------------------------------------------------------------------

union {
  byte b[128];
  int d[32];
} seed; // Seed aleatorio (127 bytes + PTR)

byte rndb(void) {
  byte ptr;
  ptr = seed.b[127];
  if ((seed.b[127] = (ptr + seed.b[ptr]) & 127) == 127)
    seed.b[127] = 0;
  return (seed.b[seed.b[127]] += ++ptr);
}

int rnd(void) {
  return ((((rndb() & 127) * 256 + rndb()) * 256 + rndb()) * 256 + rndb());
}

int rnd2(int min, int max) {
  if (max < min) {
    max ^= min;
    min ^= max;
    max ^= min;
  }
  return ((rnd() % (max - min + 1)) + min);
}

void init_rnd(int n, char *key) {
  register int a;
  for (a = 0; a < 32; a++)
    seed.d[a] = n;
  if (key != NULL) {
    for (a = 0; a < 127; a++) {
      if (!key[a])
        break;
      seed.b[a] ^= key[a];
    }
  }
  seed.b[127] &= 127;
  for (a = 0; a < 2048; a++)
    rndb();
}

//-----------------------------------------------------------------------------
//  Función de encriptación/desencriptación
//-----------------------------------------------------------------------------

void coder(byte *ptr, int len, char *key) {
  int n;

  init_rnd(len, key);
  for (n = 0; n < len; n++) {
    ptr[n] ^= rndb();
  }
}

//-----------------------------------------------------------------------------
