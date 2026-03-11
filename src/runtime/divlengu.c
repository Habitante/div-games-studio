
//-----------------------------------------------------------------------------
//      Módulo para la gestión de lang_buffer (lenguaje.div) y de encriptación
//-----------------------------------------------------------------------------

#include "inter.h"

byte * lang_buffer;
byte * lang_buffer_end;

int numero=0; // Número de texts
byte *p_lengu,*q_lengu; // Punteros de lectura y escritura respectivamente.


///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////
void analyze_texts(void);
void coder(byte * ptr, int len, char * clave);
void analyze_number(void);
void analyze_text(void);
void analyze_comment(void);


//-----------------------------------------------------------------------------
//      Inicializa el sistema de lang_buffer
//-----------------------------------------------------------------------------

void initialize_texts(byte * fichero) {

  FILE * f;
  int n;

  memset(text,0,max_system_texts*4);

#ifdef DEBUG

  if ((f=fopen((char *)fichero,"rb"))!=NULL) {
    fseek(f,0,SEEK_END); n=ftell(f);
    if ((lang_buffer=(byte *)malloc(n))!=NULL) {
      fseek(f,0,SEEK_SET);
      n=fread(lang_buffer,1,n,f);
      fclose(f);
      lang_buffer_end=lang_buffer+n;
      analyze_texts();
    } else fclose(f);
  }

#else

  if ((f=fopen((char *)fichero,"rb"))!=NULL) {
    fseek(f,-4,SEEK_END);
    fread(&n,4,1,f);
    fseek(f,-4-n,SEEK_END);
    if ((lang_buffer=(byte *)malloc(n+1))!=NULL) {
      n=fread(lang_buffer,1,n,f);
      fclose(f);
      lang_buffer_end=lang_buffer+n;
      analyze_texts();
    } else fclose(f);
  } 

#endif

}

//-----------------------------------------------------------------------------
//      Función de análisis de los lang_buffer
//-----------------------------------------------------------------------------

void analyze_texts(void) {

  q_lengu=p_lengu=lang_buffer;

  if (!strcmp((char *)p_lengu,"Zk!")) {
    p_lengu+=4;
    coder(p_lengu,lang_buffer_end-lang_buffer-4,"lave");
  }

  while(p_lengu<lang_buffer_end)
    if (*p_lengu>='0' && *p_lengu<='9') analyze_number();
    else if (*p_lengu=='"') analyze_text();
    else if (*p_lengu=='#') analyze_comment();
    else p_lengu++;
}

void analyze_number(void) {

  numero=0; do {
    numero=numero*10+*p_lengu-'0'; p_lengu++;
  } while (*p_lengu>='0' && *p_lengu<='9' && p_lengu<lang_buffer_end);
  if (numero>=max_system_texts) numero=0;
}

void analyze_comment(void) {

  while (*p_lengu!='\n' && *p_lengu!='\r' && p_lengu<lang_buffer_end) p_lengu++;
}

void analyze_text(void) {

  text[numero]=q_lengu; p_lengu++; numero++;
  while (*p_lengu!='"' && p_lengu<lang_buffer_end && *p_lengu!='\r' && *p_lengu!='\n') {
    if (*p_lengu=='\\') { p_lengu++;
      if (*p_lengu=='n') {
        *q_lengu++=13; *q_lengu++=10; p_lengu++;
      } else if (*p_lengu=='#') {
        *q_lengu++=13; p_lengu++;
      } else *q_lengu++=*p_lengu++;
    } else *q_lengu++=*p_lengu++;
  } p_lengu++; *q_lengu++=0;
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

union {byte b[128]; int d[32];} seed_coder; // Seed aleatorio (127 bytes + PTR)

byte rndb(void) {
byte ptr;
  ptr=seed_coder.b[127];
  if ((seed_coder.b[127]=(ptr+seed_coder.b[ptr])&127)==127) seed_coder.b[127]=0;
  return(seed_coder.b[seed_coder.b[127]]+=++ptr);
}

int rnd_coder(void) {
  return((((rndb()&127)*256+rndb())*256+rndb())*256+rndb());
}

int rnd2(int min,int max) {
  if (max<min) {max^=min; min^=max; max^=min; }
  return((rnd_coder()%(max-min+1))+min);
}

void init_rnd_coder(int n, char * clave){
  volatile int a;
  for (a=0;a<32;a++) seed_coder.d[a]=n;
  if (clave!=NULL) {
    for (a=0;a<127;a++) {
      if (!clave[a]) break;
      seed_coder.b[a]^=clave[a];
    }
  }
  seed_coder.b[127]&=127;
  for (a=0;a<2048;a++) rndb();
}

//-----------------------------------------------------------------------------
//  Función de encriptación/desencriptación
//-----------------------------------------------------------------------------

void coder(byte * ptr, int len, char * clave) {
  int n;

  init_rnd_coder(len,clave);
  for (n=0;n<len;n++) {
    ptr[n]^=rndb();
  }
}

//-----------------------------------------------------------------------------
