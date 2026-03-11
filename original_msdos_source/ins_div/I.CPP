
/*
************* OJO *************** NO dejar instalar en C:\

o mejor !!!, dar problemas cuando en el directorio principal (donde ir  el D.EXE)
haya otras cosas

(ya que podr¡an querer instalar div en una unidad SUBST o una partici¢n nueva)
*/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Instalador de DIV Games Studio 2
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Textos
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#define uchar unsigned char
#define byte unsigned char
#define ushort unsigned short
#define word unsigned short
#define ulong unsigned int
#define dword unsigned int

void inicializa_textos(byte * fichero);
#define max_textos_sistema 256
extern byte *text[max_textos_sistema];

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

// Coordenadas

#define SHARE_X             0
#define SHARE_Y             162

// Tama¤os

#define MOUSE_ANCHO         36
#define MOUSE_ALTO          42
#define O_W                 36
#define O_H                 18
#define O_S                 10

// Errores cr¡ticos de ejecuci¢n (tabulados los no utilizados

#define textoerr text[10]
#define texto0 text[11]
#define texto1 text[12]
#define texto2 text[13]
#define texto3 text[14]
#define texto4 text[15]
#define texto5 text[16]
#define texto6 text[17]
#define texto7 text[18]
#define texto8 text[19]

// Errores no cr¡ticos

#define e0a text[20]
#define e0b text[21]
#define e1a text[22]
#define e1b text[23]
#define e2a text[24]
#define e2b text[25]
#define e3a text[26]
#define e3b text[27]
#define e4a text[28]
#define e4b text[29]
#define i0a text[30]
#define i0b text[31]
#define i1a text[32]
#define i1b text[33]

// Instrucci¢n para ejecutar DIV

#define msgexe text[34]

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#include <i86.h>
#include <bios.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <graph.h>
#include <direct.h>
#include <math.h>
#include <signal.h>
#include "svga.h"
#include "divkeybo.cpp"

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

struct {
  int x,y; // x,y del bot¢n max
  int sel; // 0-max, 1-med, 2-min
} custom[5]={
    314,218,0,  // System
    314,262,0,  // Examples
    314,305,0,  // Fonts
    314,349,0,  // Artwork
    314,393,0}; // Sounds

int cluster_size=0; // Tama¤o del cluster

struct {    // Estructura informativa de espacio ocupado por min, med y max
  int min,med,max;
} bytes[5]; // System, examples, fonts, artwork, sounds

struct {     // Estructura informativa de los juegos (tama¤o y seleccionados)
  int sel;   // Indica si est  seleccionado
  int size;  // Tama¤o en bytes del juego
  char name[64];
} games[64]; // Examples

int num_games=0;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#define max_fpgs 64     // Numero m ximo de fpgs cargados

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Variables globales
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int error=0; // Indicador del c¢digo de error producido

int option=0; // Opci¢n seleccionada (1-install, 2-help y 3-exit)

byte * vga = (byte *) 0xA0000; // Pantalla fisica
byte * copia; // Copia virtual de pantalla
byte * copia2; // Copia virtual de pantalla
byte dac[768]; // Paleta del programa
short scan[480*4]; // Por scan [x,an,x,an] se definen hasta 2 segmentos a volcar
int LinealMode; // Modo de la svga
int volcado_completo=0; // Indica si se debe realizar un volcado completo
int vga_an,vga_al; // Ancho y alto de pantalla

char full[_MAX_PATH+1];

byte fondo[MOUSE_ANCHO*MOUSE_ALTO]; // Fondo del rat¢n
byte fondo2[420*40]; // Fondo del total de MB de ejemplos
int mouse_x,mouse_y,mouse_b,old_mouse_b,mouse_ok;

int low=0;

int errores_lectura;

int windows=0;  // Si recibe el par metro "-win", se crear  c:\pathinfo.dat

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

struct t_g { // Estructura para un fpg
  int * * fpg; // Fichero cargado en memoria
  int * * grf; // Punteros a los gr ficos (g[n].grf[000..999])
};

#define max_fpgs 64     // Numero m ximo de fpgs cargados

struct t_g g[max_fpgs]; // Array de los fpg

FILE * es;       // Lectura de ficheros en la interpretaci¢n (fpg, voc, ...)

int file_len;    // Lectura de ficheros en la interpretaci¢n

word * ptr;      // Puntero general para un malloc en ejecuci¢n

char key;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

typedef struct _TABLAFNT{
    int ancho;
    int alto;
    int incY;
    int offset;
}TABLAFNT;

typedef struct _fnt_info{
  int ancho;            // Ancho medio del font
  int espacio;          // Longitud en pixels del espacio en blanco
  int espaciado;        // Espaciado entre car cteres (adem s del propio ancho)
  int alto;             // Altura m xima del font
} fnt_info;

#define max_fonts 32    // N£mero m ximo de fonts en ejecuci¢n

byte * fonts[max_fonts]; // Fonts cargados en ejecuci¢n (0-no cargado)

TABLAFNT * fnt;

fnt_info f_i[max_fonts];

byte *f1,*f2,*f3; // Fonts del programa de instalaci¢n

byte * load_fnt(char * filename);

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

long InstallUnid=0; // Unidad en la que se instala

char cwork[256]; // Gen‚rico

char ActDir[256]; // Directorio del install.exe

FILE *fin=NULL,*fout=NULL;

char unid[4]="C:\\";

char buffer[256]; // Directorio de instalaci¢n por defecto

int totalbytes; // Espacio en bytes ocupado por la instalaci¢n

byte * lista=NULL;  // Puntero a la lista de ficheros de instalaci¢n
byte * bak=NULL;    // Puntero a la copia de la lista
int lista_len; // Longitud de la lista de ficheros

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

struct diskfree_t structdiskfree; // Detectar disco duros
long tablaMBfree[28];
char Unidades[28*2];
long localDrives=0;

void GetHardThings();

byte * cbuf;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Tabla de conversi¢n de chars Windows a chars MSDOS
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

char OEM2ANSI[256]={
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,164,0,0,0,0,182,167,0,0,0,0,0,0,0,0,0,0,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  199,252,233,226,228,224,229,231,234,235,232,239,238,236,196,197,201,230,198,244,246,242,251,249,255,214,220,248,163,216,215,131,
  225,237,243,250,241,209,170,186,191,174,172,189,188,161,171,187,95,95,95,166,166,193,194,192,169,166,166,43,43,162,165,43,
  43,45,45,43,45,43,227,195,43,43,45,45,166,45,43,164,240,208,202,203,200,105,205,206,207,43,43,95,95,166,204,95,
  211,223,212,210,245,213,181,254,222,218,219,217,253,221,175,180,173,177,95,190,182,167,247,184,176,168,183,185,179,178,95,32};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Critical error handler
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

unsigned c_e_0,c_e_1;
unsigned far * c_e_2;

int error_critico=0;

int __far critical_error(unsigned deverr,unsigned errcode,unsigned far*devhdr) {
  c_e_0=deverr; c_e_1=errcode; c_e_2=devhdr;
  error_critico=1;
  return(_HARDERR_IGNORE);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Programa principal
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

char winpath1[128]; // Path del grupo Hammer
char winpath2[128]; // Path del escritorio

int main(int argc, char * argv[]) {
  FILE *f;
  unsigned n,m;

  if (argc<1) exit(0);

  _fullpath(full,argv[0],_MAX_PATH+1);
  strupr(full);

  n=strlen(full);
  while (n && full[n]!='\\') n--;
  full[n]=0;
  if (full[n-1]==':') strcat(full,"\\");

  _dos_setdrive((int)toupper(full[0])-'A'+1,&n);
  chdir(full);

  inicializa_textos("data\\system\\lenguaje.ins");

  if (argc>=2) if (!strcmp(argv[1],"-win")) {
    f=fopen(text[36],"rt");
    fscanf(f,"%s\n%s\n",winpath1,winpath2);
    fclose(f);

    remove(text[36]);

    for (n=0;n<strlen(winpath1);n++) {
      for (m=0;m<256;m++) if (OEM2ANSI[m]==winpath1[n]) break;
      if (m<256) winpath1[n]=m;
    }
    for (n=0;n<strlen(winpath2);n++) {
      for (m=0;m<256;m++) if (OEM2ANSI[m]==winpath2[n]) break;
      if (m<256) winpath2[n]=m;
    }

    windows=1;
  }

  getcwd(ActDir,255);

  mouse_ok=1;
  if (!is_mouse_ok()) {
    system(text[35]);
    if (!is_mouse_ok()) {
      mouse_ok=0;
    }
  }

  strcpy(buffer,text[76]);

  signal(SIGBREAK, SIG_IGN);      // anti ctrl-break
  signal(SIGINT, SIG_IGN);        // anti ctrl-c
  _harderr(critical_error);

  kbdInit();

  GetHardThings();

  InstallUnid=0;
  if (localDrives==0) _error(6);
  if (localDrives>4) localDrives=4;

  if (!lee_listas()) _error(8);

  if ((copia=(byte*)malloc(640*480))==NULL) _error(5);
  if ((copia2=(byte*)malloc(640*480))==NULL) _error(5);

  if ((cbuf=(byte*)malloc(256*1024))==NULL) _error(5);

  if (load_fpg(text[2])==-1) _error(4);

  if ((f1=load_fnt(text[3]))==NULL) _error(4);
  if ((f2=load_fnt(text[4]))==NULL) _error(4);
  if ((f3=load_fnt(text[5]))==NULL) _error(4);

  memset(copia,0,640*480);
  _setvideomode(_VRES16COLOR);
  init_mouse();

  svmode(); if (error==1) _error(3);

  set_paleta(0);
  set_dac();
  init_volcado();
  init_mouse();

  put(0,1,0,0);

  do {
    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  } while ((!ascii || ascii=='§') && !mouse_b);

  put(0,2,0,0);

  strupr(text[1]);
  if (!strcmp(text[1],"SHARE")) put(0,8,SHARE_X,SHARE_Y);

  memcpy(copia2,copia,640*480);
  show_data();

  fade();

  ascii=0;

  while (ascii!=27 && option!=3) {

    if (option==1) {
      if (tablaMBfree[InstallUnid]<totalbytes) err(e0a,e0b);
      else
      {
        memcpy(lista, bak, lista_len);
        install();
      }
    }
    if (option==2) ayuda();
    show_data();
    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  }

  rvmode();
  kbdReset();

  if (windows) {
    f=fopen(text[37],"wt");
    fprintf(f,text[38]);
    fclose(f);
    flushall();
  }

  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Funci¢n para poner el modo de v¡deo 640x480
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void svmode(void) {
  int mode=0x101;
  int VersionVesa;

  vga_an=640; vga_al=480;

  VersionVesa=SV_init();
  if(VersionVesa<0x102) {
    LinealMode=0;
    if (!_setvideomode(_VRES256COLOR)) error=1; else vga=(char *)0x0A0000;
  } else {
    if(VersionVesa<0x200) {
      LinealMode=0;
      if(!SV_setMode(mode)) {
        if (!_setvideomode(_VRES256COLOR)) error=1; else vga=(char *)0x0A0000;
      } else vga=(char *)0x0A0000;
    } else {
      LinealMode=1;
      mode|=vbeLinearBuffer;
      if(!SV_setMode(mode)) {
        LinealMode=0;
        mode^=vbeLinearBuffer;
        if(!SV_setMode(mode)) {
          if (!_setvideomode(_VRES256COLOR)) error=1; else vga=(char *)0x0A0000;
        } else vga=(char *)videoMem;
      } else vga=(char *)videoMem;
    }
  }
}

void rvmode(void) {
  SV_restoreMode();
  _setvideomode(3);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Activa una paleta
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void retrazo(void) {
  while (inp(0x3da)&8);
  while ((inp(0x3da)&8)==0);
}

void set_dac(void) {
  union REGS regs;
  int n=0;

  retrazo();
  outp(0x3c8,0);
  do {
    outp(0x3c9,dac[n++]);
  } while (n<768);
  regs.w.ax=0x1001;
  regs.h.bh=0;
  int386(0x010,&regs,&regs);

}

void set_paleta(int x) { // Fija los primeros 128 colores como los segundos - n
  int n;
  for (n=0;n<384;n++) {
    if (dac[n+384]>x) dac[n]=dac[n+384]-x; else dac[n]=0;
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Selecciona una ventana para su posterior volcado
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void init_volcado(void) { memset(&scan[0],0,480*8); volcado_completo=0; }

void volcado_parcial(int x,int y,int an,int al) {
  int ymax,xmax,n,d1,d2,x2;

  if (an==vga_an && al==vga_al && x==0 && y==0) { volcado_completo=1; return; }

  if (an>0 && al>0 && x<vga_an && y<vga_al) {
    if (x<0) { an+=x; x=0; } if (y<0) { al+=y; y=0; }
    if (x+an>vga_an) an=vga_an-x; if (y+al>vga_al) al=vga_al-y;
    if (an<=0 || al<=0) return;
    xmax=x+an-1; ymax=y+al-1;

    switch(vga_an*1000+vga_al) {
      case 320240: case 320400: case 360240: case 360360: case 376282: // Modos X
        x>>=2; xmax>>=2; an=xmax-x+1; break;
    }

    while (y<=ymax) { n=y*4;
      if (scan[n+1]==0) {         // Caso 1, el scan estaba vac¡o ...
        scan[n]=x; scan[n+1]=an;
      } else if (scan[n+3]==0) {  // Caso 2, ya hay un scan definido ...
        if (x>scan[n]+scan[n+1] || x+an<scan[n]) { // ... hueco entre medias
          if (x>scan[n]) {
            scan[n+2]=x; scan[n+3]=an;
          } else {
            scan[n+2]=scan[n]; scan[n+3]=scan[n+1];
            scan[n]=x; scan[n+1]=an;
          }
        } else { // ... no hay hueco, amplia el primer scan
          if (x<(x2=scan[n])) scan[n]=x;
          if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
          else scan[n+1]=x2+scan[n+1]-scan[n];
        }
      } else {                    // Caso 3, hay 2 scanes definidos ...
        if (x<=scan[n]+scan[n+1] && x+an>=scan[n+2]) {
          // Caso 3.1, se tapa el hueco anterior -> queda un solo scan
          if (x<scan[n]) scan[n]=x;
          if (x+an>scan[n+2]+scan[n+3]) scan[n+1]=x+an-scan[n]; else scan[n+1]=scan[n+2]+scan[n+3]-scan[n];
          scan[n+2]=0; scan[n+3]=0;
        } else {
          if (x>scan[n]+scan[n+1] || x+an<scan[n]) { // No choca con 1§
            if (x>scan[n+2]+scan[n+3] || x+an<scan[n+2]) { // No choca con 2§
              // Caso 3.4, el nuevo no colisiona con ninguno, se calcula el espacio
              // hasta ambos, y se fusiona con el m s cercano
              if (x+an<scan[n]) d1=scan[n]-(x+an); else d1=x-(scan[n]+scan[n+1]);
              if (x+an<scan[n+2]) d2=scan[n+2]-(x+an); else d2=x-(scan[n+2]+scan[n+3]);
              if (d1<=d2) {
                // Caso 3.4.1 se fusiona con el primero
                if (x<(x2=scan[n])) scan[n]=x;
                if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
                else scan[n+1]=x2+scan[n+1]-scan[n];
              } else {
                // Caso 3.4.2 se fusiona con el segundo
                if (x<(x2=scan[n+2])) scan[n+2]=x;
                if (x+an>x2+scan[n+3]) scan[n+3]=x+an-scan[n+2];
                else scan[n+3]=x2+scan[n+3]-scan[n+2];
              }
            } else {
              // Caso 3.3, el nuevo colisiona con el 2§, se fusionan
              if (x<(x2=scan[n+2])) scan[n+2]=x;
              if (x+an>x2+scan[n+3]) scan[n+3]=x+an-scan[n+2];
              else scan[n+3]=x2+scan[n+3]-scan[n+2];
            }
          } else {
            // Caso 3.2, el nuevo colisiona con el 1§, se fusionan
            if (x<(x2=scan[n])) scan[n]=x;
            if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
            else scan[n+1]=x2+scan[n+1]-scan[n];
          }
        }
      } y++;
    }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Volcado en SVGA
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void volcar(void) {
  salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
  put(0,10,mouse_x-2,mouse_y-2);
  volcado();
  salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
}

void volcado(void) {
  if (volcado_completo) {
    volcadocsvga(copia);
  } else {
    volcadopsvga(copia);
  } init_volcado();
}

void volcadopsvga(byte *p) {
  int y=0,page,old_page=-1751,point,t1,t2,n;
  char *q=vga;

  if(LinealMode) {
   while (y<vga_al) {
     n=y*4;
     if (scan[n+1]) memcpy(q+scan[n],p+scan[n],scan[n+1]);
     if (scan[n+3]) memcpy(q+scan[n+2],p+scan[n+2],scan[n+3]);
     q+=vga_an; p+=vga_an; y++;
   }
  } else while (y<vga_al) {
    n=y*4;
    if (scan[n+1]) {
      page=(y*vga_an+scan[n])/65536;
      point=(y*vga_an+scan[n])%65536;
      if (point+scan[n+1]>65536) {
        t1=65536-point;
        t2=scan[n+1]-t1;
        if (page!=old_page) SV_setBank((signed long)page);
        memcpy(vga+point,p+scan[n],t1);
        SV_setBank((signed long)page+1); old_page=page+1;
        memcpy(vga,p+scan[n]+t1,t2);
      } else {
        if (page!=old_page) SV_setBank((signed long)(old_page=page));
        memcpy(vga+point,p+scan[n],scan[n+1]);
      }
    }
    if (scan[n+3]) {
      page=(y*vga_an+scan[n+2])/65536;
      point=(y*vga_an+scan[n+2])%65536;
      if (point+scan[n+3]>65536) {
        t1=65536-point;
        t2=scan[n+3]-t1;
        if (page!=old_page) SV_setBank((signed long)page);
        memcpy(vga+point,p+scan[n+2],t1);
        SV_setBank((signed long)page+1); old_page=page+1;
        memcpy(vga,p+scan[n+2]+t1,t2);
      } else {
        if (page!=old_page) SV_setBank((signed long)(old_page=page));
        memcpy(vga+point,p+scan[n+2],scan[n+3]);
      }
    } p+=vga_an; y++;
  }
}

void volcadocsvga(byte *p) {
  int cnt=vga_an*vga_al;
  int tpv=0,ActPge=0;

  if(LinealMode) memcpy(vga,p,cnt);
  else while(cnt>0) {
    SV_setBank((signed long)ActPge++);
    tpv=cnt>65536?65536:cnt;
    memcpy(vga,p,tpv);
    p+=tpv;
    cnt-=tpv;
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Snapshot
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void snapshot(byte *p) {
  FILE * f;
  int n=0;
  char cwork[128];

  do {
    sprintf(cwork,text[39],n++);
    if ((f=fopen(cwork,"rb"))!=NULL) fclose(f);
  } while (f!=NULL);

  f=fopen(cwork,"wb");
  graba_PCX(p,f);
  fclose(f);
}

typedef struct _pcx_header {
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
  short  xmin,ymin;
  short  xmax,ymax;
  short  hres;
  short  vres;
  char   palette16[48];
  char   reserved;
  char   color_planes;
  short  bytes_per_line;
  short  palette_type;
  short  Hresol;
  short  Vresol;
  char  filler[54];
}pcx_header;

struct pcx_struct {
  pcx_header header;
  unsigned char far *cimage;
  unsigned char palette[3*256];
  unsigned char far *image;
  int clength;
};

int graba_PCX(byte *mapa,FILE *f) {
  byte p[768];
  int x;
  byte *cbuffer;
  struct pcx_struct pcx;
  int ptr=0;
  int cptr=0;
  int Desborde=0;
  char ActPixel;
  char cntPixel=0;
  char Paletilla=12;

        pcx.header.manufacturer=10;
        pcx.header.version=5;
        pcx.header.encoding=1;
        pcx.header.bits_per_pixel=8;
        pcx.header.xmin=0;
        pcx.header.ymin=0;
        pcx.header.xmax=vga_an-1;
        pcx.header.ymax=vga_al-1;
        pcx.header.hres=vga_an;
        pcx.header.vres=vga_al;
        pcx.header.color_planes=1;
        pcx.header.bytes_per_line=vga_an;
        pcx.header.palette_type=0;
        pcx.header.Hresol=vga_an;
        pcx.header.Vresol=vga_al;

        if ((cbuffer=(unsigned char *)malloc(vga_an*vga_al*2))==NULL) return(1);

        ActPixel=mapa[ptr];
        while (ptr < vga_an*vga_al)
        {
                while((mapa[ptr]==ActPixel) && (ptr<vga_an*vga_al))
                {
                        cntPixel++;
                        Desborde++;
                        ptr++;
                        if(Desborde==vga_an)
                        {
                                Desborde=0;
                                break;
                        }
                        if(cntPixel==63)
                                break;
                }
                if(cntPixel==1)
                {
                        if(ActPixel>63)
                                cbuffer[cptr++] = 193;
                        cbuffer[cptr++] = ActPixel;
                }
                else
                {
                        cbuffer[cptr++] = 192+cntPixel;
                        cbuffer[cptr++] = ActPixel;
                }
                ActPixel=mapa[ptr];
                cntPixel=0;
        }

        fwrite(&pcx.header,1,sizeof(pcx_header),f);
        fwrite(cbuffer,1,cptr,f);
        fwrite(&Paletilla,1,1,f);
        for (x=0;x<768;x++) p[x]=dac[x]*4;
        fwrite(p,1,768,f);
        free(cbuffer);
        return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Funcion para localizar y abrir un fichero (pal,fpg,fnt,...)
//  Esta funci¢n debe seguir el mismo algoritmo en F.CPP y DIVC.CPP
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

FILE * open_file(char * file) {
  FILE * f;
  char drive[_MAX_DRIVE+1];
  char dir[_MAX_DIR+1];
  char fname[_MAX_FNAME+1];
  char ext[_MAX_EXT+1];

  strcpy(full,file);
  if ((f=fopen(full,"rb"))==NULL) {                     // "paz\fixero.est"
    if (_fullpath(full,file,_MAX_PATH)==NULL) return(NULL);
    _splitpath(full,drive,dir,fname,ext);
    if (strchr(ext,'.')==NULL) strcpy(full,ext); else strcpy(full,strchr(ext,'.')+1);
    if (strlen(full) && file[0]!='\\') strcat(full,"\\");
    strcat(full,file);
    if ((f=fopen(full,"rb"))==NULL) {                   // "est\paz\fixero.est"
      strcpy(full,fname);
      strcat(full,ext);
      if ((f=fopen(full,"rb"))==NULL) {                 // "fixero.est"
        if (strchr(ext,'.')==NULL) strcpy(full,ext); else strcpy(full,strchr(ext,'.')+1);
        if (strlen(full)) strcat(full,"\\");
        strcat(full,fname);
        strcat(full,ext);
        if ((f=fopen(full,"rb"))==NULL) {               // "est\fixero.est"
          strcpy(full,"");
          return(NULL);
        } else return(f);
      } else return(f);
    } else return(f);
  } else return(f);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Load_fpg(fichero) - Devuelve el c¢digo del fichero 0..max_fpgs (-1 error)
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int load_fpg(char * filename) {
  int n=0;
  int * * lst;
  byte * ptr;

  while (n<max_fpgs) {
    if (g[n].fpg==0) break; n++;
  } if (n==max_fpgs) return(-1);

  if ((lst=malloc(sizeof(int*)*1000))==NULL) return(-1);
  memset(lst,0,sizeof(int*)*1000);

  if ((es=open_file(filename))==NULL) return(-1); else {
    fseek(es,0,SEEK_END); file_len=ftell(es);
    if ((ptr=(byte *)malloc(file_len))!=NULL) {
      g[n].fpg=(int**)ptr;
      fseek(es,0,SEEK_SET);
      fread(ptr,1,file_len,es); fclose(es);
      if (strcmp(ptr,"fpg\x1a\x0d\x0a")) return(-1);
      memcpy(dac,ptr+8,768);
      g[n].grf=lst; ptr+=1352; // Longitud cabecera fpg
      while (ptr<(byte*)g[n].fpg+file_len && *(int*)ptr<1000 && *(int*)ptr>0 ) {
        lst[*(int*)ptr]=(int*)ptr;
        ptr+=*(int*)(ptr+4);
      }
      return(n);
    } else { fclose(es); return(-1); }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Load_fnt(fichero) - Devuelve codigo_font
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

byte * load_fnt(char * filename) {
  byte * ptr;
  int n,an,al,nan,ifonts;

  for (ifonts=0;ifonts<max_fonts;ifonts++) if (!fonts[ifonts]) break;
  if (ifonts==max_fonts) return(NULL);

  if ((es=open_file(filename))==NULL) return(NULL); else {
    fseek(es,0,SEEK_END); file_len=ftell(es);
    if ((ptr=(byte*)malloc(file_len))!=NULL) {
      fonts[ifonts]=ptr; fseek(es,0,SEEK_SET);
      fread(ptr,1,file_len,es); fclose(es);
      if (strcmp(ptr,"fnt\x1a\x0d\x0a")) {
        fonts[ifonts]=0; free(ptr); return(NULL);
      }
      an=0; al=0; nan=0;

      fnt=(TABLAFNT*)((byte*)ptr+1356);

      for (n=0;n<256;n++) {
        if (fnt[n].ancho) { an+=fnt[n].ancho; nan++; }
        if (fnt[n].alto+fnt[n].incY>al) al=fnt[n].alto+fnt[n].incY;
      }

      f_i[ifonts].ancho=an/nan;
      f_i[ifonts].espacio=(an/nan)/2;
      f_i[ifonts].espaciado=0;
      f_i[ifonts].alto=al;
      return(ptr);
    } else { fclose(es); return(NULL); }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Pinta un gr fico
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void put(int file, int graf, int x, int y) {
  int *ptr;
  byte *p,*q=copia+y*vga_an+x;
  int an,al;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  ptr=g[file].grf[graf];
  an=ptr[13]; al=ptr[14];
  p=(byte*)ptr+64+ptr[15]*4;
  volcado_parcial(x,y,an,al);

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_an) resto_x=x+an-vga_an; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_x<=0 || long_y<=0) return;

  p+=an*salta_y+salta_x;
  q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

void put_clip(int file, int graf, int ancho, int x, int y, int centro) {
  int *ptr;
  byte *p,*q;
  int an,al,xg,yg;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (!ancho) return;

  ptr=g[file].grf[graf];
  an=ptr[13]; al=ptr[14];
  p=(byte*)ptr+64+ptr[15]*4;

  if (ptr[15]==0 || *((word*)ptr+32)==65535) { xg=ptr[13]/2; yg=ptr[14]/2;
  } else { xg=*((word*)ptr+32); yg=*((word*)ptr+33); }

  if (centro) { x-=xg; y-=yg; }

  q=copia+y*vga_an+x;

  volcado_parcial(x,y,an,al);

  if (x<0) salta_x=-x; else salta_x=0;
  if (an>ancho) resto_x=an-ancho; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_x<=0 || long_y<=0) return;

  p+=an*salta_y+salta_x;
  q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Guarda y/o recupera una zona de pantalla (buffer,x,y,an,al,0:guarda 1:recupera)
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void salvaguarda(byte * p, int x, int y, int an, int al, int flag) {
  byte *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  volcado_parcial(x,y,an,al);

  q=copia+y*vga_an+x;

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_an) resto_x=x+an-vga_an; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_x<=0 || long_y<=0) return;

  p+=an*salta_y+salta_x; q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (flag) *q=*p; else *p=*q; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Rutinas del rat¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int is_mouse_ok(void) {
  struct SREGS sregs;
  union REGS inregs, outregs;

  segread(&sregs);
  inregs.w.ax = 0;
  int386 (0x33, &inregs, &outregs);
  if (outregs.w.ax!=0xffff) return(0); else return(1);
}

void init_mouse(void) {
  int vga_an=640,vga_al=480;
  union REGS regs;

  memset(&regs,0,sizeof(regs));
  regs.w.ax=0x01a; // Resoluci¢n del rat¢n (?)
  regs.w.bx=50;
  regs.w.cx=50;
  regs.w.dx=50;
  int386 (0x33, &regs, &regs);

  memset(&regs,0,sizeof(regs));
  regs.w.ax=7;
  regs.w.dx=vga_an*2;
  int386(0x033,&regs,&regs);

  memset(&regs,0,sizeof(regs));
  regs.w.ax=8;
  regs.w.dx=vga_al;
  int386(0x033,&regs,&regs);

  memset(&regs,0,sizeof(regs));
  regs.w.ax=4;
  regs.w.cx=vga_an;
  regs.w.dx=vga_al/2;
  int386(0x033,&regs,&regs);
}

int vx=0,vy=0;

void read_mouse(void) {
  union REGS regs;
  int keymouse=0;

  old_mouse_b=mouse_b;

  if (mouse_ok) {
    memset(&regs,0,sizeof(regs));
    regs.w.ax=3;
    int386(0x033,&regs,&regs);
    mouse_b=regs.w.bx;
    mouse_x=regs.w.cx/2;
    mouse_y=regs.w.dx;
  } else mouse_b=0;

  tecla();

  if(key(_ENTER)) mouse_b|=1;

  if ((shift_status&4) && (shift_status&8) && scan_code==_P) snapshot(copia);

  if (key(_LEFT)&&!key(_RIGHT)) { vx-=1; keymouse|=1; }
  if (key(_RIGHT)&&!key(_LEFT)) { vx+=1; keymouse|=1; }
  if (key(_UP)&&!key(_DOWN)) { vy-=1; keymouse|=2; }
  if (key(_DOWN)&&!key(_UP)) { vy+=1; keymouse|=2; }

  if (vx || vy) { //keymouse) {
    if (vx<-8) vx=-8; if (vx>8) vx=8;
    if (vy<-6) vy=-6; if (vy>6) vy=6;
    mouse_x+=vx;
    mouse_y+=vy;
    if(mouse_x<0) mouse_x=0;
    if(mouse_y<0) mouse_y=0;
    if(mouse_x>=vga_an) mouse_x=vga_an-1;
    if(mouse_y>=vga_al) mouse_y=vga_al-1;
    set_mouse(mouse_x,mouse_y);
    retrazo();
  }

  if (!(keymouse&1)) {
    if (vx>1) vx-=2;
    else if (vx<-1) vx+=2;
    else vx=0;
  }
  if (!(keymouse&2)) {
    if (vy>1) vy-=2;
    else if (vy<-1) vy+=2;
    else vy=0;
  }
}

void set_mouse(word x, word y) {
  union REGS inregs, outregs;
  inregs.w.ax = 4;
  inregs.w.cx = x*2; inregs.w.dx = y;
  int386 (0x33, &inregs, &outregs);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Imprime un texto
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int text_len(byte * font, char *ptr) {
  int an,al=0,fuente;
  byte * ptr2;

  fuente=0; while (fuente<max_fonts) {
    if (font==fonts[fuente]) break;
    fuente++;
  } if (fuente==max_fonts) return(0);

  fnt=(TABLAFNT*)((byte*)font+1356);

  ptr2=ptr; an=0;
  while(*ptr2) {
    al=0;
    if (fnt[*ptr2].ancho==0) {
      an+=f_i[fuente].espacio; ptr2++;
    } else {
      an+=fnt[*ptr2++].ancho+1;
      al=1;
    }
  } if (al) an--;

  return(an);
}

void text_out(byte * font, int x,int y, int centro, char *ptr) {
  int an,al=0;
  byte * ptr2;
  int fuente;

  fuente=0; while (fuente<max_fonts) {
    if (font==fonts[fuente]) break;
    fuente++;
  } if (fuente==max_fonts) return;

  fnt=(TABLAFNT*)((byte*)font+1356);

  ptr2=ptr; an=0;
  while(*ptr2) {
    al=0;
    if (fnt[*ptr2].ancho==0) {
      an+=f_i[fuente].espacio; ptr2++;
    } else {
      an+=fnt[*ptr2++].ancho+1;
      al=1;
    }
  } if (al) an--;

  al=f_i[fuente].alto;

  switch (centro) {
    case 0: break;
    case 1: x=x-(an>>1); break;
    case 2: x=x-an; break;
    case 3: y=y-(al>>1); break;
    case 4: x=x-(an>>1); y=y-(al>>1); break;
    case 5: x=x-an; y=y-(al>>1); break;
    case 6: y=y-al; break;
    case 7: x=x-(an>>1); y=y-al; break;
    case 8: x=x-an; y=y-al; break;
  }

  if (y<vga_al && y+al>0) {

    if (y>=0 && y+al<=vga_al) { // El texto coge entero (coord. y)

      while (*ptr && x+fnt[*ptr].ancho<=0) {
        if (fnt[*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          x=x+fnt[*ptr].ancho+1; ptr++;
        }
      }

	    if (*ptr && x<0) {
        if (fnt[*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texc(font+fnt[*ptr].offset,x,y+fnt[*ptr].incY,fnt[*ptr].ancho,fnt[*ptr].alto);
          x=x+fnt[*ptr].ancho+1; ptr++;
        }
      }

    	while (*ptr && x+fnt[*ptr].ancho<=vga_an) {
        if (fnt[*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texn(font+fnt[*ptr].offset,x,y+fnt[*ptr].incY,fnt[*ptr].ancho,fnt[*ptr].alto);
          x=x+fnt[*ptr].ancho+1; ptr++;
        }
      }

      if (*ptr && x<vga_an) {
        if (fnt[*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texc(font+fnt[*ptr].offset,x,y+fnt[*ptr].incY,fnt[*ptr].ancho,fnt[*ptr].alto);
        }
      }

    } else {

    	while (*ptr && x+fnt[*ptr].ancho<=0) {
        if (fnt[*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          x=x+fnt[*ptr].ancho+1; ptr++;
        }
      }

    	while (*ptr && x<vga_an) {
        if (fnt[*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texc(font+fnt[*ptr].offset,x,y+fnt[*ptr].incY,fnt[*ptr].ancho,fnt[*ptr].alto);
          x=x+fnt[*ptr].ancho+1; ptr++;
        }
      }

    }
  }
}

void texn(byte * p, int x, int y, byte an, int al) {

  byte *q=copia+y*vga_an+x;
  int ancho=an;

  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=ancho);
  } while (--al);
}

void texc(byte * p, int x, int y, byte an, int al) {

  byte *q=copia+y*vga_an+x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_an) resto_x=x+an-vga_an; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (long_x<=0) return;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_y<=0) return;

  p+=an*salta_y+salta_x; q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Muestra la instalaci¢n actualmente seleccionada (max .. min)
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void show_custom(void) {
  int n,x,total,subtotal;

  if (mouse_b&1) {
    for (n=0;n<5;n++) {
      if (mouse_y>=custom[n].y && mouse_y<custom[n].y+O_H) {
        if (mouse_x>custom[n].x && mouse_x<custom[n].x+O_W) custom[n].sel=0;
        if (mouse_x>custom[n].x+O_W+O_S && mouse_x<custom[n].x+O_W+O_S+O_W) custom[n].sel=1;
        if (mouse_x>custom[n].x+(O_W+O_S)*2 && mouse_x<custom[n].x+(O_W+O_S)*2+O_W) custom[n].sel=2;
      }
    }
    if (mouse_y>custom[1].y && mouse_y<custom[1].y+O_H)
      if (mouse_x>custom[1].x+O_W+O_S && mouse_x<custom[1].x+O_W+O_S+O_W) {
        examples();
      }
  }

  for (total=0,n=0;n<5;n++) {
    put(0,100,custom[n].x,custom[n].y);
    put(0,102,custom[n].x+O_W+O_S,custom[n].y);
    put(0,104,custom[n].x+(O_W+O_S)*2,custom[n].y);
    put(0,101+custom[n].sel*2,custom[n].x+(O_W+O_S)*custom[n].sel,custom[n].y);

    if (n==1 && custom[1].sel==1) {
      for (subtotal=0,x=0;x<64;x++) if (games[x].sel) subtotal+=games[x].size;
    } else switch(custom[n].sel) {
      case 0: subtotal=bytes[n].min+bytes[n].med+bytes[n].max; break;
      case 1: subtotal=bytes[n].min+bytes[n].med; break;
      case 2: subtotal=bytes[n].min; break;
    }

    sprintf(cwork,"%u.%uMb",subtotal/(1024*1024),((subtotal/1024)%1024)/102);
    total+=subtotal;

    restore(custom[n].x-77,custom[n].y+2,55,32);
    volcado_parcial(custom[n].x-77,custom[n].y+2,55,32);
    text_out(f1,custom[n].x-16,custom[n].y-1,2,cwork);
  }

  totalbytes=total;

  total/=1000; // total/=1024; // OJO!

  sprintf(cwork,"%u",total/1024);
  restore(515,225,75,35);
  volcado_parcial(515,225,75,35);
  text_out(f1,552,240,4,cwork);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Detecta un cambio en el 'Select Hard Drive'
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void check_drives(void) {

  if ((mouse_b&1)) {
    if( mouse_x>257 && mouse_x<257+89 && mouse_y>52 && mouse_y<52+39 ) {
      switch(InstallUnid)
      {
        case 0: restore(257,52,89,39); volcado_parcial(257,52,89,39); break;
        case 1: restore(353,52,89,39); volcado_parcial(353,52,89,39); break;
        case 2: restore(450,52,89,39); volcado_parcial(450,52,89,39); break;
        case 3: restore(546,52,89,39); volcado_parcial(546,52,89,39); break;
      }
      InstallUnid=0;
    }
    if(localDrives>1)
    if( mouse_x>353 && mouse_x<353+89 && mouse_y>52 && mouse_y<52+39 ) {
      switch(InstallUnid)
      {
        case 0: restore(257,52,89,39); volcado_parcial(257,52,89,39); break;
        case 1: restore(353,52,89,39); volcado_parcial(353,52,89,39); break;
        case 2: restore(450,52,89,39); volcado_parcial(450,52,89,39); break;
        case 3: restore(546,52,89,39); volcado_parcial(546,52,89,39); break;
      }
      InstallUnid=1;
    }
    if(localDrives>2)
    if( mouse_x>450 && mouse_x<450+89 && mouse_y>52 && mouse_y<52+39 ) {
      switch(InstallUnid)
      {
        case 0: restore(257,52,89,39); volcado_parcial(257,52,89,39); break;
        case 1: restore(353,52,89,39); volcado_parcial(353,52,89,39); break;
        case 2: restore(450,52,89,39); volcado_parcial(450,52,89,39); break;
        case 3: restore(546,52,89,39); volcado_parcial(546,52,89,39); break;
      }
      InstallUnid=2;
    }
    if(localDrives>3)
    if( mouse_x>546 && mouse_x<546+89 && mouse_y>52 && mouse_y<52+39 ) {
      switch(InstallUnid)
      {
        case 0: restore(257,52,89,39); volcado_parcial(257,52,89,39); break;
        case 1: restore(353,52,89,39); volcado_parcial(353,52,89,39); break;
        case 2: restore(450,52,89,39); volcado_parcial(450,52,89,39); break;
        case 3: restore(546,52,89,39); volcado_parcial(546,52,89,39); break;
      }
      InstallUnid=3;
    }
  }

  switch(InstallUnid)
  {
    case 0: put(0,200,257,52); break;
    case 1: put(0,201,353,52); break;
    case 2: put(0,202,450,52); break;
    case 3: put(0,203,546,52); break;
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Restaura una regi¢n de pantalla (de copia2 a copia) - no comprueba nada
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void restore(int x, int y, int an, int al) {
  byte *si,*di;
  si=copia2+y*vga_an+x;
  di=copia+y*vga_an+x;
  while (al--) {
    memcpy(di,si,an);
    si+=vga_an;
    di+=vga_an;
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Discos duros disponibles
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void GetHardThings(void) {
  long x,MBfree;
  union REGS regs;

  localDrives=0;
  memset(Unidades,0,28*2);
  for (x=0;x<=23;x++) {
    error_critico=0;
    regs.w.ax=0x4409;
    regs.w.bx=x+3;
    int386(0x21, &regs, &regs);           // informaci¢n de dispositivo
    if (!(regs.w.cflag & INTR_CF)) {      // si no hubo error
      if (!(regs.w.dx & (1<<9))) {        // i/o no permitida
        Unidades[localDrives*2]='C'+x;
        structdiskfree.avail_clusters=0;
        _dos_getdiskfree(x+3, &structdiskfree);
        MBfree=structdiskfree.sectors_per_cluster * structdiskfree.bytes_per_sector;
        if (cluster_size<MBfree) cluster_size=MBfree;
        MBfree*=structdiskfree.avail_clusters;
        tablaMBfree[localDrives]=MBfree;
        if (MBfree>0 && !error_critico) localDrives++;
      }
    }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Ventana de ayuda
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void ayuda(void) {
  int aceptar=0;

  lowpal();

  put(0,7,105,14);
  put(0,50,275,419);

  fade();

  do {
    if (low<24) { low+=2; set_paleta(low); set_dac(); }

    if (mouse_x>275 && mouse_x<275+75 && mouse_y>419 && mouse_y<419+21) {
      put(0,51,275,419); aceptar=1;
    } else {
      put(0,50,275,419); aceptar=0;
    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

  } while (!(!(mouse_b&1) && (old_mouse_b&1)) || !aceptar);

  mouse_b=0; clear_window();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Cuando se produce un error
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void err(char *h,char *e) { // Ventana con un mensaje de error
  int aceptar=0;
  int max_an;
  char cwork[256];
  char cwork2[256];
  char pal[1024];
  int i=0;
  int xx0=141+8,xx1=141+440-8,yy0=180+48,yy1=180+114;

  lowpal();

  put(0,5,141,180);
  put(0,50,307,270);

  text_out(f2,xx0,206,0,h);

  max_an=xx1-xx0;

  strcpy(pal,&e[i]);
  if (strchr(pal,' ')) {
    *(strchr(pal,' '))=(char)0;
  } i+=strlen(pal)+1;

  strcpy(cwork,pal);

  while (yy0<yy1) {
    strcpy(cwork2,cwork);

    strcpy(pal,&e[i]);
    if (strchr(pal,' ')) {
      *(strchr(pal,' '))=(char)0;
    } i+=strlen(pal)+1;

    strcat(cwork," ");
    strcat(cwork,pal);

    if (text_len(f1,cwork)>max_an) {
      text_out(f1,xx0,yy0,0,cwork2);
      strcpy(cwork,pal);
      yy0+=text_al(f1)-3;
    }

    if (i>=strlen(e)) {
      text_out(f1,xx0,yy0,0,cwork);
      break;
    }
  }
  fade();

  do {
    if (low<24) { low+=2; set_paleta(low); set_dac(); }

    if (mouse_x>307 && mouse_x<307+75 && mouse_y>270 && mouse_y<270+21) {
      put(0,51,307,270); aceptar=1;
    } else {
      put(0,50,307,270); aceptar=0;
    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

  } while (!(!(mouse_b&1) && (old_mouse_b&1)) || !aceptar);

  clear_window();

}

void clear_window(void) {

  restore(0,0,640,480);
  show_data(); lowpal();
  fade();

  do {
    if (low>0) { low-=2; set_paleta(low); set_dac(); }
    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  } while (low!=0);

  hipal();

  while (mouse_b&1) {
    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  }
}

void lowpal(void) {
  byte *p=copia;
  byte *pend=copia+640*480;
  do { *p-=128; } while (p++<pend);
}

void hipal(void) {
  byte *p=copia;
  byte *pend=copia+640*480;
  do { *p+=128; } while (p++<pend);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Di logo informativo
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void info(char *h,char *e) { // Ventana con un mensaje
  int aceptar=0;
  int max_an;
  char cwork[256];
  char cwork2[256];
  char pal[1024];
  int i=0;
  int xx0=141+8,xx1=141+440-8,yy0=180+48,yy1=180+114;

  lowpal();

  put(0,9,141,180);
  put(0,50,307,270);

  text_out(f2,xx0,206,0,h);

  max_an=xx1-xx0;

  strcpy(pal,&e[i]);
  if (strchr(pal,' ')) {
    *(strchr(pal,' '))=(char)0;
  } i+=strlen(pal)+1;

  strcpy(cwork,pal);

  while (yy0<yy1) {
    strcpy(cwork2,cwork);

    strcpy(pal,&e[i]);
    if (strchr(pal,' ')) {
      *(strchr(pal,' '))=(char)0;
    } i+=strlen(pal)+1;

    strcat(cwork," ");
    strcat(cwork,pal);

    if (text_len(f1,cwork)>max_an) {
      text_out(f1,xx0,yy0,0,cwork2);
      strcpy(cwork,pal);
      yy0+=text_al(f1)-3;
    }

    if (i>=strlen(e)) {
      text_out(f1,xx0,yy0,0,cwork);
      break;
    }
  }

  fade();

  do {
    if (low<24) { low+=2; set_paleta(low); set_dac(); }

    if (mouse_x>307 && mouse_x<307+75 && mouse_y>270 && mouse_y<270+21) {
      put(0,51,307,270); aceptar=1;
    } else {
      put(0,50,307,270); aceptar=0;
    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

  } while (!(!(mouse_b&1) && (old_mouse_b&1)) || !aceptar);

  clear_window();

}

int text_al(byte * font) {
  int fuente;

  fuente=0; while (fuente<max_fonts) {
    if (font==fonts[fuente]) break;
    fuente++;
  } if (fuente==max_fonts) return(0);

  return(f_i[fuente].alto);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Sale al MS-DOS
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void _error(word e) {

  if (e!=7) {
    if (fin!=NULL) fclose(fin);
    if (fout!=NULL) fclose(fout);
    chdir(ActDir);
  }

  strcpy(cwork,textoerr);
  switch (e) {
    case 0:strcat(cwork,texto0); break;
    case 1:strcat(cwork,texto1); break;
    case 2:strcat(cwork,texto2); break;
    case 3:strcat(cwork,texto3); break;
    case 4:strcat(cwork,texto4); break;
    case 5:strcat(cwork,texto5); break;
    case 6:strcat(cwork,texto6); break;
    case 7:strcat(cwork,texto7); break;
    case 8:strcat(cwork,texto8); break;
  }

  _setvideomode(_DEFAULTMODE);
  kbdReset();
  printf("%s",cwork);
  if (windows) getch();
  exit(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Input de la ruta de instalaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int * system_clock = (void*) 0x46c; // Reloj del sistema
#define maxlen 240

void input(void) {
  char c;

  restore(255,124,341,18);
  volcado_parcial(255,124,341,18);

  c=toupper(ascii); if (c=='¤') c='¥';

  if((c>='0'&&c<='9')||(c>='A'&&c<='Z')||c=='\\'||c=='_'||c=='!'||c=='-'||c=='&'||c=='$'||c=='¥'||c=='%'||c=='.') {
    if (strlen(buffer)<maxlen) {
      buffer[strlen(buffer)+1]=0;
      buffer[strlen(buffer)]=c;
      if (text_len(f1,buffer)>300) buffer[strlen(buffer)-1]=0;
    }
  } else if (c==8 && strlen(buffer)) buffer[strlen(buffer)-1]=0;

  unid[0]=Unidades[InstallUnid*2];

  strcpy(cwork,unid);
  strcat(cwork,buffer);
  if ((*system_clock)&4) strcat(cwork,"");

  text_out(f1,256,122,0,cwork);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Comprueba las opciones del men£
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void options(void) {

  restore(10,333,150,147);
  volcado_parcial(10,333,150,147);

  option=0;

  if (mouse_x> 68 && mouse_x< 68+66 && mouse_y>359 && mouse_y<359+44) option=1;
  if (mouse_x> 10 && mouse_x< 10+48 && mouse_y>333 && mouse_y<333+44) option=2;
  if (mouse_x>106 && mouse_x<106+43 && mouse_y>426 && mouse_y<426+44) option=3;

  if (!(mouse_b&1)) {
    switch(option) {
      case 1: put(0,109,68,359);  break;
      case 2: put(0,108,10,333);  break;
      case 3: put(0,110,106,426); break;
    } option=0;
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Transici¢n de pantalla
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void fade(void) {
  int n;      // 0..640
  int i[480]; // 2000..2400 (de 20 a 24 pixels)
  int x,y;

  salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
  put(0,10,mouse_x-2,mouse_y-2);

  init_volcado();

  for (n=0;n<480;n++) i[n]=rand()%400+2000;

  for (n=0;n<32;n++) {
    for (y=0;y<480;y++) {
      x=n*i[y]/100;
      if (x<617) {
        volcado_parcial(x,y,24,1);
      } else if (x<640) {
        volcado_parcial(x,y,640-x,1);
      }
    } retrazo(); volcado();
  }

  salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Muestra los datos de la pantalla de instalaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void show_data(void) {

  check_drives();

  text_out(f2,330,73,4,&Unidades[0*2]);
  sprintf(cwork,"%u",tablaMBfree[0]/(1024*1024));
  text_out(f1,316-4,61,2,cwork);
  if(localDrives>1)
  {
    text_out(f2,426,73,4,&Unidades[1*2]);
    sprintf(cwork,"%u",tablaMBfree[1]/(1024*1024));
    text_out(f1,412-4,61,2,cwork);
  }
  if(localDrives>2)
  {
    text_out(f2,523,73,4,&Unidades[2*2]);
    sprintf(cwork,"%u",tablaMBfree[2]/(1024*1024));
    text_out(f1,509-4,61,2,cwork);
  }
  if(localDrives>3)
  {
    text_out(f2,619,73,4,&Unidades[3*2]);
    sprintf(cwork,"%u",tablaMBfree[3]/(1024*1024));
    text_out(f1,605-4,61,2,cwork);
  }
  show_custom();
  input();
  options();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Lee las listas de ficheros de FILES.INI - Devuelve 1 si no hay error
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

// int cluster_size; // Tama¤o del cluster
// struct {    // Estructura informativa de espacio ocupado por min, med y max
//   int min,med,max;
// } bytes[5]; // System, examples, fonts, artwork, sounds
// struct {     // Estructura informativa de los juegos (tama¤o y seleccionados)
//   int sel;   // Indica si est  seleccionado
//   int size;  // Tama¤o en bytes del juego
// } games[64]; // Examples

int lee_listas(void) {
  int len,n,sec=0;
  FILE *f;
  byte *ptr,*p;

  memset(bytes,0,sizeof(bytes));
  for (n=0;n<64;n++) {
    games[n].size=0;
    games[n].name[0]=0;
  }

  if(lista!=NULL)
  {
    free(lista);
    lista=NULL;
  }
  if ((f=fopen(text[40],"rb"))==0) return(0);
  fseek(f,0,SEEK_END); len=ftell(f);
  if ((ptr=(byte *)malloc(len))==NULL) return(0);
  if ((bak=(byte *)malloc(len))==NULL) return(0);
  fseek(f,0,SEEK_SET);
  if (fread(ptr,1,len,f)!=len) return(0);
  fclose(f);

  // Convierte cr,lf en 0,0

  for (n=0;n<len;n++) if (ptr[n]==10 || ptr[n]==13) ptr[n]=0;

  lista=p=ptr; lista_len=len;
  memcpy(bak, lista, lista_len);

  while (p<ptr+len) {
    if (strlen(p)>01) {

      if (*p=='[') { // Comienza una nueva secci¢n
        if (!strcmp(p,"[SYSTEM0]")) sec=1;
        else if (!strcmp(p,"[SYSTEM1]")) sec=2;
        else if (!strcmp(p,"[SYSTEM2]")) sec=3;
        else if (!strcmp(p,"[FONTS0]")) sec=4;
        else if (!strcmp(p,"[FONTS1]")) sec=5;
        else if (!strcmp(p,"[FONTS2]")) sec=6;
        else if (!strcmp(p,"[ARTWORK0]")) sec=7;
        else if (!strcmp(p,"[ARTWORK1]")) sec=8;
        else if (!strcmp(p,"[ARTWORK2]")) sec=9;
        else if (!strcmp(p,"[SOUNDS0]")) sec=10;
        else if (!strcmp(p,"[SOUNDS1]")) sec=11;
        else if (!strcmp(p,"[SOUNDS2]")) sec=12;
        else if (!strcmp(p,"[GAME0]")) sec=13;
        else if (!strcmp(p,"[GAME1]")) sec=14;
        else if (!strcmp(p,"[GAME2]")) sec=15;
        else if (!strcmp(p,"[GAME3]")) sec=16;
        else if (!strcmp(p,"[GAME4]")) sec=17;
        else if (!strcmp(p,"[GAME5]")) sec=18;
        else if (!strcmp(p,"[GAME6]")) sec=19;
        else if (!strcmp(p,"[GAME7]")) sec=20;
        else if (!strcmp(p,"[GAME8]")) sec=21;
        else if (!strcmp(p,"[GAME9]")) sec=22;
        else if (!strcmp(p,"[GAME10]")) sec=23;
        else if (!strcmp(p,"[GAME11]")) sec=24;
        else if (!strcmp(p,"[GAME12]")) sec=25;
        else if (!strcmp(p,"[GAME13]")) sec=26;
        else if (!strcmp(p,"[GAME14]")) sec=27;
        else if (!strcmp(p,"[GAME15]")) sec=28;
        else if (!strcmp(p,"[GAME16]")) sec=29;
        else if (!strcmp(p,"[GAME17]")) sec=30;
        else if (!strcmp(p,"[GAME18]")) sec=31;
        else if (!strcmp(p,"[GAME19]")) sec=32;
        else if (!strcmp(p,"[GAME20]")) sec=33;
        else if (!strcmp(p,"[GAME21]")) sec=34;
        else if (!strcmp(p,"[GAME22]")) sec=35;
        else if (!strcmp(p,"[GAME23]")) sec=36;
        else if (!strcmp(p,"[GAME24]")) sec=37;
        else if (!strcmp(p,"[GAME25]")) sec=38;
        else if (!strcmp(p,"[GAME26]")) sec=39;
        else if (!strcmp(p,"[GAME27]")) sec=40;
        else if (!strcmp(p,"[GAME28]")) sec=41;
        else if (!strcmp(p,"[GAME29]")) sec=42;
        else if (!strcmp(p,"[GAME30]")) sec=43;
        else if (!strcmp(p,"[GAME31]")) sec=44;
        else if (!strcmp(p,"[GAME32]")) sec=45;
        else if (!strcmp(p,"[GAME33]")) sec=46;
        else if (!strcmp(p,"[GAME34]")) sec=47;
        else if (!strcmp(p,"[GAME35]")) sec=48;
        else if (!strcmp(p,"[GAME36]")) sec=49;
        else if (!strcmp(p,"[GAME37]")) sec=50;
        else if (!strcmp(p,"[GAME38]")) sec=51;
        else if (!strcmp(p,"[GAME39]")) sec=52;
        else if (!strcmp(p,"[GAME40]")) sec=53;
        else if (!strcmp(p,"[GAME41]")) sec=54;
        else if (!strcmp(p,"[GAME42]")) sec=55;
        else if (!strcmp(p,"[GAME43]")) sec=56;
        else if (!strcmp(p,"[GAME44]")) sec=57;
        else if (!strcmp(p,"[GAME45]")) sec=58;
        else if (!strcmp(p,"[GAME46]")) sec=59;
        else if (!strcmp(p,"[GAME47]")) sec=60;
        else if (!strcmp(p,"[GAME48]")) sec=61;
        else if (!strcmp(p,"[GAME49]")) sec=62;
        else if (!strcmp(p,"[GAME50]")) sec=63;
        else if (!strcmp(p,"[GAME50]")) sec=64;
        else if (!strcmp(p,"[GAME51]")) sec=65;
        else if (!strcmp(p,"[GAME52]")) sec=66;
        else if (!strcmp(p,"[GAME53]")) sec=67;
        else if (!strcmp(p,"[GAME54]")) sec=68;
        else if (!strcmp(p,"[GAME55]")) sec=69;
        else if (!strcmp(p,"[GAME56]")) sec=70;
        else if (!strcmp(p,"[GAME57]")) sec=71;
        else if (!strcmp(p,"[GAME58]")) sec=72;
        else if (!strcmp(p,"[GAME59]")) sec=73;
        else if (!strcmp(p,"[GAME60]")) sec=74;
        else if (!strcmp(p,"[GAME61]")) sec=75;
        else if (!strcmp(p,"[GAME62]")) sec=76;
        else if (!strcmp(p,"[GAME63]")) sec=77;
        else sec=0;

      } else if (*p=='"') {

        *strchr(p+1,'"')=0;
        strcpy(games[sec-13].name,p+1);
        if (num_games<sec-12) num_games=sec-12;
        p+=strlen(p)+1;

      } else {

        n=((atoi(strchr(p,' ')+1)+cluster_size-1)/cluster_size)*cluster_size;

        switch(sec) {
          case 1: bytes[0].min+=n; break;
          case 2: bytes[0].med+=n; break;
          case 3: bytes[0].max+=n; break;
          case 4: bytes[2].min+=n; break;
          case 5: bytes[2].med+=n; break;
          case 6: bytes[2].max+=n; break;
          case 7: bytes[3].min+=n; break;
          case 8: bytes[3].med+=n; break;
          case 9: bytes[3].max+=n; break;
          case 10: bytes[4].min+=n; break;
          case 11: bytes[4].med+=n; break;
          case 12: bytes[4].max+=n; break;
          case 13: case 14: case 15: case 16: case 17: case 18: case 19:
          case 20: case 21: case 22: case 23: case 24: case 25: case 26:
          case 27: case 28: case 29: case 30: case 31: case 32: case 33:
          case 34: case 35: case 36: case 37: case 38: case 39: case 40:
          case 41: case 42: case 43: case 44: case 45: case 46: case 47:
          case 48: case 49: case 50: case 51: case 52: case 53: case 54:
          case 55: case 56: case 57: case 58: case 59: case 60: case 61:
          case 62: case 63: case 64: case 65: case 66: case 67: case 68:
          case 69: case 70: case 71: case 72: case 73: case 74: case 75:
          case 76: case 77:
            bytes[1].max+=n;
            games[sec-13].size+=n;
            break;
        }
      }
    } p+=strlen(p)+2;
  }

  for (n=0;n<64;n++) if (games[n].size==0) games[n].sel=0; else games[n].sel=1;

  return(1);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Ventana de ayuda
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#define EXAMPLES_X 125
#define EXAMPLES_Y  80

void examples(void) {
  int aceptar=0,x,y,n,total;
  char cwork[256];

  lowpal();

  put(0,4,94,34);
  put(0,50,284,409);
  salvaguarda(&fondo2,110,370,420,40,0);

  for (x=0;x<2;x++) for (y=0;y<num_games/2;y++) if (x*(num_games/2)+y<num_games) {
    if (games[x*(num_games/2)+y].sel) {
      put(0,23,EXAMPLES_X+x*210,EXAMPLES_Y+22*y);
    } else {
      put(0,22,EXAMPLES_X+x*210,EXAMPLES_Y+22*y);
    }
    if (games[x*(num_games/2)+y].size) {
      sprintf(cwork,"%s (%u.%uMb)",games[x*(num_games/2)+y].name,games[x*(num_games/2)+y].size/(1024*1024),((games[x*(num_games/2)+y].size/1024)%1024)/102);
      text_out(f1,EXAMPLES_X+20+x*210,EXAMPLES_Y-5+22*y,0,cwork);
    } else {
      text_out(f3,EXAMPLES_X+20+x*210,EXAMPLES_Y-5+22*y,0,games[x*(num_games/2)+y].name);
    }
  }

  for (total=0,x=0;x<64;x++) if (games[x].sel) total+=games[x].size;
  sprintf(cwork,"Total %u.%u Mb",total/(1024*1024),((total/1024)%1024)/102);
  text_out(f1,320,380,1,cwork);

  fade();

  old_mouse_b=1;

  do {
    if (low<24) { low+=2; set_paleta(low); set_dac(); }

    if (mouse_x>284 && mouse_x<284+73 && mouse_y>409 && mouse_y<409+18) {
      put(0,51,284,409); aceptar=1;
    } else {
      put(0,50,284,409); aceptar=0;
    }

    if ((mouse_b&1) && !(old_mouse_b&1)) for (x=0;x<2;x++) for (y=0;y<num_games/2;y++) if (x*(num_games/2)+y<num_games) {

      if (games[x*(num_games/2)+y].size) {
        if (mouse_x>EXAMPLES_X+x*210 && mouse_x<EXAMPLES_X+x*210+200 &&
            mouse_y>EXAMPLES_Y+22*y && mouse_y<EXAMPLES_Y+22*y+22) {
          games[x*(num_games/2)+y].sel^=1;
          if (games[x*(num_games/2)+y].sel) put(0,23,EXAMPLES_X+x*210,EXAMPLES_Y+22*y);
          else                              put(0,22,EXAMPLES_X+x*210,EXAMPLES_Y+22*y);
        }
      }

      salvaguarda(&fondo2,110,370,420,40,1);
      for (total=0,n=0;n<64;n++) if (games[n].sel) total+=games[n].size;
      sprintf(cwork,"Total %u.%u Mb",total/(1024*1024),((total/1024)%1024)/102);
      text_out(f1,320,380,1,cwork);

    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

  } while (!(!(mouse_b&1) && (old_mouse_b&1)) || !aceptar);

  clear_window();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Ventana de instalaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void install(void) {
  int cdgraf=0,x,xporcen=0;
  float porcen=0.0;
  int copiar=0,ficheros=0,total_ficheros=0;
  byte * p, * nombre, * nombre2;
  char cwork[512];
  char cwork2[512];
  char cwork3[512];
  char cwork4[512];
  char cwork_juanjo[256];
  char dirdest[512]="";

  lowpal();

  put(0,6,96,200);

  fade();

  do {
    if (low<24) { low+=2; set_paleta(low); set_dac(); }
    cdgraf=(cdgraf+1)%4;
    put(0,25+cdgraf,509,205);
    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  } while (low<24);

  for (x=0;x<64;x++) {
    if (games[x].size==0) games[x].sel=0;
    else if (custom[1].sel==0) games[x].sel=1;
    else if (custom[1].sel==2) games[x].sel=0;
  }

  // Cuenta los ficheros

  p=lista;

  while (p<lista+lista_len) {
    if (strlen(p)>1) {
      if (*p=='[') { // Comienza una nueva secci¢n
        if (!strcmp(p,"[SYSTEM0]")) copiar=1;
        else if (!strcmp(p,"[SYSTEM1]")) copiar=(custom[0].sel<2);
        else if (!strcmp(p,"[SYSTEM2]")) copiar=(custom[0].sel<1);
        else if (!strcmp(p,"[FONTS0]")) copiar=1;
        else if (!strcmp(p,"[FONTS1]")) copiar=(custom[2].sel<2);
        else if (!strcmp(p,"[FONTS2]")) copiar=(custom[2].sel<1);
        else if (!strcmp(p,"[ARTWORK0]")) copiar=1;
        else if (!strcmp(p,"[ARTWORK1]")) copiar=(custom[3].sel<2);
        else if (!strcmp(p,"[ARTWORK2]")) copiar=(custom[3].sel<1);
        else if (!strcmp(p,"[SOUNDS0]")) copiar=1;
        else if (!strcmp(p,"[SOUNDS1]")) copiar=(custom[4].sel<2);
        else if (!strcmp(p,"[SOUNDS2]")) copiar=(custom[4].sel<1);
        else if (!strcmp(p,"[GAME0]")) copiar=(games[0].sel);
        else if (!strcmp(p,"[GAME1]")) copiar=(games[1].sel);
        else if (!strcmp(p,"[GAME2]")) copiar=(games[2].sel);
        else if (!strcmp(p,"[GAME3]")) copiar=(games[3].sel);
        else if (!strcmp(p,"[GAME4]")) copiar=(games[4].sel);
        else if (!strcmp(p,"[GAME5]")) copiar=(games[5].sel);
        else if (!strcmp(p,"[GAME6]")) copiar=(games[6].sel);
        else if (!strcmp(p,"[GAME7]")) copiar=(games[7].sel);
        else if (!strcmp(p,"[GAME8]")) copiar=(games[8].sel);
        else if (!strcmp(p,"[GAME9]")) copiar=(games[9].sel);
        else if (!strcmp(p,"[GAME10]")) copiar=(games[10].sel);
        else if (!strcmp(p,"[GAME11]")) copiar=(games[11].sel);
        else if (!strcmp(p,"[GAME12]")) copiar=(games[12].sel);
        else if (!strcmp(p,"[GAME13]")) copiar=(games[13].sel);
        else if (!strcmp(p,"[GAME14]")) copiar=(games[14].sel);
        else if (!strcmp(p,"[GAME15]")) copiar=(games[15].sel);
        else if (!strcmp(p,"[GAME16]")) copiar=(games[16].sel);
        else if (!strcmp(p,"[GAME17]")) copiar=(games[17].sel);
        else if (!strcmp(p,"[GAME18]")) copiar=(games[18].sel);
        else if (!strcmp(p,"[GAME19]")) copiar=(games[19].sel);
        else if (!strcmp(p,"[GAME20]")) copiar=(games[20].sel);
        else if (!strcmp(p,"[GAME21]")) copiar=(games[21].sel);
        else if (!strcmp(p,"[GAME22]")) copiar=(games[22].sel);
        else if (!strcmp(p,"[GAME23]")) copiar=(games[23].sel);
        else if (!strcmp(p,"[GAME24]")) copiar=(games[24].sel);
        else if (!strcmp(p,"[GAME25]")) copiar=(games[25].sel);
        else if (!strcmp(p,"[GAME26]")) copiar=(games[26].sel);
        else if (!strcmp(p,"[GAME27]")) copiar=(games[27].sel);
        else if (!strcmp(p,"[GAME28]")) copiar=(games[28].sel);
        else if (!strcmp(p,"[GAME29]")) copiar=(games[29].sel);
        else if (!strcmp(p,"[GAME30]")) copiar=(games[30].sel);
        else if (!strcmp(p,"[GAME31]")) copiar=(games[31].sel);
        else if (!strcmp(p,"[GAME32]")) copiar=(games[32].sel);
        else if (!strcmp(p,"[GAME33]")) copiar=(games[33].sel);
        else if (!strcmp(p,"[GAME34]")) copiar=(games[34].sel);
        else if (!strcmp(p,"[GAME35]")) copiar=(games[35].sel);
        else if (!strcmp(p,"[GAME36]")) copiar=(games[36].sel);
        else if (!strcmp(p,"[GAME37]")) copiar=(games[37].sel);
        else if (!strcmp(p,"[GAME38]")) copiar=(games[38].sel);
        else if (!strcmp(p,"[GAME39]")) copiar=(games[39].sel);
        else if (!strcmp(p,"[GAME40]")) copiar=(games[40].sel);
        else if (!strcmp(p,"[GAME41]")) copiar=(games[41].sel);
        else if (!strcmp(p,"[GAME42]")) copiar=(games[42].sel);
        else if (!strcmp(p,"[GAME43]")) copiar=(games[43].sel);
        else if (!strcmp(p,"[GAME44]")) copiar=(games[44].sel);
        else if (!strcmp(p,"[GAME45]")) copiar=(games[45].sel);
        else if (!strcmp(p,"[GAME46]")) copiar=(games[46].sel);
        else if (!strcmp(p,"[GAME47]")) copiar=(games[47].sel);
        else if (!strcmp(p,"[GAME48]")) copiar=(games[48].sel);
        else if (!strcmp(p,"[GAME49]")) copiar=(games[49].sel);
        else if (!strcmp(p,"[GAME50]")) copiar=(games[50].sel);
        else if (!strcmp(p,"[GAME51]")) copiar=(games[51].sel);
        else if (!strcmp(p,"[GAME52]")) copiar=(games[52].sel);
        else if (!strcmp(p,"[GAME53]")) copiar=(games[53].sel);
        else if (!strcmp(p,"[GAME54]")) copiar=(games[54].sel);
        else if (!strcmp(p,"[GAME55]")) copiar=(games[55].sel);
        else if (!strcmp(p,"[GAME56]")) copiar=(games[56].sel);
        else if (!strcmp(p,"[GAME57]")) copiar=(games[57].sel);
        else if (!strcmp(p,"[GAME58]")) copiar=(games[58].sel);
        else if (!strcmp(p,"[GAME59]")) copiar=(games[59].sel);
        else if (!strcmp(p,"[GAME50]")) copiar=(games[50].sel);
        else if (!strcmp(p,"[GAME61]")) copiar=(games[61].sel);
        else if (!strcmp(p,"[GAME62]")) copiar=(games[62].sel);
        else if (!strcmp(p,"[GAME63]")) copiar=(games[63].sel);

        else copiar=0;

      } else if (*p=='"') {

      } else if (copiar) total_ficheros++;
    } p+=strlen(p)+2;
  }

  // Proceso de instalaci¢n

  errores_lectura=0;

  unid[0]=Unidades[InstallUnid*2];

  if (strlen(buffer))
    if (buffer[strlen(buffer)-1]=='\\')
      buffer[strlen(buffer)-1]=0;

  if (!strlen(buffer)) { // No deja instalar en C:
    chdir(ActDir);
    restore(0,0,640,480); mouse_b=0; show_data();
    err(e2a,e2b);
    return;
  }

  for(x=0;x<strlen(buffer);x++) { // Crea directorios
    if(buffer[x]=='\\') {
      strcpy(cwork,unid);
      strcat(cwork,buffer);
      cwork[3+x]=0;
      mkdir(cwork);
    }
  } strcpy(cwork,unid); strcat(cwork,buffer); mkdir(cwork);

  if(chdir(cwork)!=0) { // Si se produce un error, borra los directorios vacios
    for(x=0;x<strlen(buffer);x++) {
      if(buffer[x]=='\\') {
        strcpy(cwork,unid);
        strcat(cwork,buffer);
        cwork[3+x]=0;
        rmdir(cwork);
       }
    } strcpy(cwork,unid); strcat(cwork,buffer); rmdir(cwork);
    chdir(ActDir);
    restore(0,0,640,480); mouse_b=0; show_data();
    err(e2a,e2b);
    return;
  }

  // Ya tenemos en cwork[] el directorio destino

  // Copia los ficheros ...

  p=lista; copiar=0;

  while (p<lista+lista_len && ascii!=27) {
    if (strlen(p)>1) {
      if (*p=='[') { // Comienza una nueva secci¢n
        if (!strcmp(p,"[SYSTEM0]")) copiar=1;
        else if (!strcmp(p,"[SYSTEM1]")) copiar=(custom[0].sel<2);
        else if (!strcmp(p,"[SYSTEM2]")) copiar=(custom[0].sel<1);
        else if (!strcmp(p,"[FONTS0]")) copiar=1;
        else if (!strcmp(p,"[FONTS1]")) copiar=(custom[2].sel<2);
        else if (!strcmp(p,"[FONTS2]")) copiar=(custom[2].sel<1);
        else if (!strcmp(p,"[ARTWORK0]")) copiar=1;
        else if (!strcmp(p,"[ARTWORK1]")) copiar=(custom[3].sel<2);
        else if (!strcmp(p,"[ARTWORK2]")) copiar=(custom[3].sel<1);
        else if (!strcmp(p,"[SOUNDS0]")) copiar=1;
        else if (!strcmp(p,"[SOUNDS1]")) copiar=(custom[4].sel<2);
        else if (!strcmp(p,"[SOUNDS2]")) copiar=(custom[4].sel<1);
        else if (!strcmp(p,"[GAME0]")) copiar=(games[0].sel);
        else if (!strcmp(p,"[GAME1]")) copiar=(games[1].sel);
        else if (!strcmp(p,"[GAME2]")) copiar=(games[2].sel);
        else if (!strcmp(p,"[GAME3]")) copiar=(games[3].sel);
        else if (!strcmp(p,"[GAME4]")) copiar=(games[4].sel);
        else if (!strcmp(p,"[GAME5]")) copiar=(games[5].sel);
        else if (!strcmp(p,"[GAME6]")) copiar=(games[6].sel);
        else if (!strcmp(p,"[GAME7]")) copiar=(games[7].sel);
        else if (!strcmp(p,"[GAME8]")) copiar=(games[8].sel);
        else if (!strcmp(p,"[GAME9]")) copiar=(games[9].sel);
        else if (!strcmp(p,"[GAME10]")) copiar=(games[10].sel);
        else if (!strcmp(p,"[GAME11]")) copiar=(games[11].sel);
        else if (!strcmp(p,"[GAME12]")) copiar=(games[12].sel);
        else if (!strcmp(p,"[GAME13]")) copiar=(games[13].sel);
        else if (!strcmp(p,"[GAME14]")) copiar=(games[14].sel);
        else if (!strcmp(p,"[GAME15]")) copiar=(games[15].sel);
        else if (!strcmp(p,"[GAME16]")) copiar=(games[16].sel);
        else if (!strcmp(p,"[GAME17]")) copiar=(games[17].sel);
        else if (!strcmp(p,"[GAME18]")) copiar=(games[18].sel);
        else if (!strcmp(p,"[GAME19]")) copiar=(games[19].sel);
        else if (!strcmp(p,"[GAME20]")) copiar=(games[20].sel);
        else if (!strcmp(p,"[GAME21]")) copiar=(games[21].sel);
        else if (!strcmp(p,"[GAME22]")) copiar=(games[22].sel);
        else if (!strcmp(p,"[GAME23]")) copiar=(games[23].sel);
        else if (!strcmp(p,"[GAME24]")) copiar=(games[24].sel);
        else if (!strcmp(p,"[GAME25]")) copiar=(games[25].sel);
        else if (!strcmp(p,"[GAME26]")) copiar=(games[26].sel);
        else if (!strcmp(p,"[GAME27]")) copiar=(games[27].sel);
        else if (!strcmp(p,"[GAME28]")) copiar=(games[28].sel);
        else if (!strcmp(p,"[GAME29]")) copiar=(games[29].sel);
        else if (!strcmp(p,"[GAME30]")) copiar=(games[30].sel);
        else if (!strcmp(p,"[GAME31]")) copiar=(games[31].sel);
        else if (!strcmp(p,"[GAME32]")) copiar=(games[32].sel);
        else if (!strcmp(p,"[GAME33]")) copiar=(games[33].sel);
        else if (!strcmp(p,"[GAME34]")) copiar=(games[34].sel);
        else if (!strcmp(p,"[GAME35]")) copiar=(games[35].sel);
        else if (!strcmp(p,"[GAME36]")) copiar=(games[36].sel);
        else if (!strcmp(p,"[GAME37]")) copiar=(games[37].sel);
        else if (!strcmp(p,"[GAME38]")) copiar=(games[38].sel);
        else if (!strcmp(p,"[GAME39]")) copiar=(games[39].sel);
        else if (!strcmp(p,"[GAME40]")) copiar=(games[40].sel);
        else if (!strcmp(p,"[GAME41]")) copiar=(games[41].sel);
        else if (!strcmp(p,"[GAME42]")) copiar=(games[42].sel);
        else if (!strcmp(p,"[GAME43]")) copiar=(games[43].sel);
        else if (!strcmp(p,"[GAME44]")) copiar=(games[44].sel);
        else if (!strcmp(p,"[GAME45]")) copiar=(games[45].sel);
        else if (!strcmp(p,"[GAME46]")) copiar=(games[46].sel);
        else if (!strcmp(p,"[GAME47]")) copiar=(games[47].sel);
        else if (!strcmp(p,"[GAME48]")) copiar=(games[48].sel);
        else if (!strcmp(p,"[GAME49]")) copiar=(games[49].sel);
        else if (!strcmp(p,"[GAME50]")) copiar=(games[50].sel);
        else if (!strcmp(p,"[GAME51]")) copiar=(games[51].sel);
        else if (!strcmp(p,"[GAME52]")) copiar=(games[52].sel);
        else if (!strcmp(p,"[GAME53]")) copiar=(games[53].sel);
        else if (!strcmp(p,"[GAME54]")) copiar=(games[54].sel);
        else if (!strcmp(p,"[GAME55]")) copiar=(games[55].sel);
        else if (!strcmp(p,"[GAME56]")) copiar=(games[56].sel);
        else if (!strcmp(p,"[GAME57]")) copiar=(games[57].sel);
        else if (!strcmp(p,"[GAME58]")) copiar=(games[58].sel);
        else if (!strcmp(p,"[GAME59]")) copiar=(games[59].sel);
        else if (!strcmp(p,"[GAME50]")) copiar=(games[50].sel);
        else if (!strcmp(p,"[GAME61]")) copiar=(games[61].sel);
        else if (!strcmp(p,"[GAME62]")) copiar=(games[62].sel);
        else if (!strcmp(p,"[GAME63]")) copiar=(games[63].sel);
        else copiar=0;
        p+=strlen(p)+2;

      } else if (*p=='"') {

        p+=strlen(p)+2;

      } else if (copiar) {

        // Visualiza un frame

        cdgraf=(cdgraf+1)%4;
        put(0,25+cdgraf,509,205);

        ficheros++;
        porcen=(float)(ficheros*100)/(float)total_ficheros;

        // if (porcen<100) porcen+=0.01;

        x=(int)((307.0*porcen)/100.0);
        while (xporcen<x)
          put_clip(0, 111, xporcen++, 159, 273, 0);

        read_mouse();
        salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
        put(0,10,mouse_x-2,mouse_y-2);
        volcado();
        salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

        // Copia un archivo

        strcpy(cwork3,cwork);
        if (cwork3[strlen(cwork3)-1]!='\\') {
          cwork3[strlen(cwork3)+1]=0;
          cwork3[strlen(cwork3)]='\\';
        }

        nombre=p; p+=strlen(p)+2;

        if (strchr(nombre,' ')) *(strchr(nombre,' '))=0;

        strcat(cwork3,nombre); nombre2=&cwork3[0];

        // ... nombre = "SYSTEM\GRANDE.FON"
        // ... nombre2 = "C:\DIV\SYSTEM\GRANDE.FON"

        // Comprueba si el directorio destino actual es diferente...

        strcpy(cwork4,nombre2);
        for (x=strlen(cwork4)-1;x>=0;x--) if (cwork4[x]=='\\') break;
        cwork4[x]=0;
        if (strcmp(cwork4,dirdest)) {

          for(x=3;x<strlen(nombre2);x++) { // Crea directorios
            if(nombre2[x]=='\\') {
              strcpy(cwork2,nombre2);
              cwork2[x]=0;
              mkdir(cwork2);
            }
          }

        } strcpy(dirdest,cwork4);

        if (ActDir[strlen(ActDir)-1]=='\\') {
          sprintf(cwork4,"%s%s%s",ActDir,text[41],nombre);
        } else {
          sprintf(cwork4,"%s\\%s%s",ActDir,text[41],nombre);
        }

        if ((x=dcopy(cwork4,nombre2))) {
          switch(x) {
            case 1: case 3:
              sprintf(cwork_juanjo,e3b,cwork4);

              if (ActDir[strlen(ActDir)-1]=='\\') {
                sprintf(cwork4,"%s%s%s",ActDir,text[42],nombre);
              } else {
                sprintf(cwork4,"%s\\%s%s",ActDir,text[42],nombre);
              }

              if ((x=dcopy(cwork4,nombre2))) {
                switch(x) {
                  case 1: case 3:
                    errores_lectura++;
                    remove(nombre2);
                    restore(0,0,640,480); mouse_b=0; show_data();
                    err(e3a,cwork_juanjo);
                    restore(0,0,640,480); mouse_b=0; show_data();
                    lowpal();
                    put(0,6,96,200);
                    put_clip(0, 111, xporcen, 159, 273, 0);
                    fade();
                    do {
                      if (low<24) { low+=2; set_paleta(low); set_dac(); }
                      cdgraf=(cdgraf+1)%4;
                      put(0,25+cdgraf,509,205);
                      read_mouse();
                      salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
                      put(0,10,mouse_x-2,mouse_y-2);
                      volcado();
                      salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
                    } while (low<24);
                    break;
                  case 2: case 4:
                    restore(0,0,640,480); mouse_b=0; show_data();
                    err(e4a,e4b);
                    chdir(ActDir);
                    return;
                }
              }

              break;

            case 2: case 4:
              restore(0,0,640,480); mouse_b=0; show_data();
              err(e4a,e4b);
              chdir(ActDir);
              return;
          }
        }

      } else p+=strlen(p)+2;
    } else p+=strlen(p)+2;
  }

  restore(0,0,640,480); mouse_b=0; show_data(); // clear_window();

  if (p>=lista+lista_len) {

    if (!errores_lectura) {
      info(i0a,i0b);
    } else {
      sprintf(cwork_juanjo,i1b,errores_lectura);
      info(i1a,cwork_juanjo);
    }

    // Para MSDOS

    _dos_setdrive((cwork[0]-'A')+1,NULL);
    chdir(cwork);

    for (x=50;x<60;x++) remove(text[x]);

    rvmode();
    kbdReset();

    if (windows) crear_pif(cwork); else printf(msgexe);

    exit(0);

  } else {
    err(e1a,e1b);
    chdir(ActDir);
  }

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Crea los PIF (DIV2/DIV2SAFE/SOUNDCFG.PIF) y SYSTEM\PATHINFO.INI
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#include "pif\div2.h"

void crear_pif(char * cwork) {
  char cwork2[512];
  char cwork3[512];
  FILE * f;

  f=fopen(text[60],"wb");
  fwrite(winpath1,1,strlen(winpath1)+1,f);
  fwrite(winpath2,1,strlen(winpath2)+1,f);
  fclose(f);

  check_oldpif();

  strcpy(cwork3,cwork);
  if (cwork3[strlen(cwork3)-1]!='\\') {
    cwork3[strlen(cwork3)+1]=0;
    cwork3[strlen(cwork3)]='\\';
  }

  strcpy(cwork2,cwork3);
  cwork2[strlen(cwork2)-1]=0;
  strcpy(&div2[0x65],cwork2);
  strcpy(cwork2,cwork3);
  strcat(cwork2,text[61]);
  strcpy(&div2[0x24],cwork2);
  strcpy(cwork2,cwork3);
  strcat(cwork2,text[62]);
  strcpy(&div2[0x273],cwork2);

  f=fopen(text[67],"wb");
  fwrite(div2,1,967,f);
  fclose(f);

  strcpy(cwork2,cwork3);
  strcat(cwork2,text[63]);
  strcpy(&div2[0x273],cwork2);
  strcpy(&div2[0xa5],text[68]);
  strcpy(&div2[0x1c5],text[68]);

  f=fopen(text[69],"wb");
  fwrite(div2,1,967,f);
  fclose(f);

  strcpy(cwork2,cwork3);
  strcat(cwork2,text[66]);
  strcpy(&div2[0x273],cwork2);
  strcpy(cwork2,cwork3);
  strcat(cwork2,text[70]);
  strcpy(&div2[0x24],cwork2);
  strcpy(&div2[0xa5],text[71]);
  strcpy(&div2[0x1c5],text[71]);

  f=fopen(text[72],"wb");
  fwrite(div2,1,967,f);
  fclose(f);

/*
  strcpy(cwork2,cwork3);
  strcat(cwork2,text[64]);
  strcpy(&div2[0x273],cwork2);
  strcpy(cwork2,cwork3);
  strcat(cwork2,text[78]);
  strcpy(&div2[0x24],cwork2);
  strcpy(&div2[0xa5],"");
  strcpy(&div2[0x1c5],"");

  f=fopen(text[77],"wb");
  fwrite(div2,1,967,f);
  fclose(f);
*/

  f=fopen(text[37],"wt");
  fprintf(f,"%s\n",cwork3);
  fclose(f);

  flushall();

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Check_oldpif - Si encuentra un PIF de DIV 1 inv lido -> lo borra
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void check_oldpif(void) {
  unsigned n;
  byte pif[1024];
  FILE * f;
  char winpath[128];
  char cwork[128],*name;
  char drive[_MAX_DRIVE+1];
  char dir[_MAX_DIR+1];
  char fname[_MAX_FNAME+1];
  char ext[_MAX_EXT+1];

  if ((f=fopen(text[60],"rb"))==NULL) return;
  fread(winpath,1,128,f);
  fclose(f);

  sprintf(cwork,text[73],winpath);

  if ((f=fopen(cwork,"rb"))!=NULL) {

    fseek(f,0,SEEK_END);
    n=ftell(f);
    fseek(f,0,SEEK_SET);

    if (n<=1024) {

      memset(pif,0,1024);
      fread(pif,1,n,f);
      fclose(f);

      name=&pif[0x273]; // Path del icono

      _splitpath(name,drive,dir,fname,ext);
      strupr(fname); strupr(ext);

      if (!strcmp(fname,text[74]) && !strcmp(ext,text[75])) { // Es el PIF de DIV 1

        name=&pif[0x24];

        if ((f=fopen(name,"rb"))!=NULL) {

          fseek(f,0,SEEK_END);
          n=ftell(f);
          fclose(f);

          if (n>1024*1024) remove(cwork); // Si no es el EXE de DIV 1, borra el PIF

        } else remove(cwork); // Si no existe el EXE, tambi‚n borra el PIF

      }

    } else fclose(f);
  }

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mi propio copy (0 ok, 1 !fopen origen, 2 !fopen dest, 3 !read, 4 !write, 5 ESC)
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int dcopy(char * origen, char * destino) {  // Usa cbuf[] de un 1/4 de mega
  FILE *f,*g;
  int len,n;

  if ((f=fopen(origen,"rb"))==NULL) return(1);
  if ((g=fopen(destino,"wb"))==NULL) { fclose(f); return(2); }

  fseek(f,0,SEEK_END);
  len=ftell(f);
  fseek(f,0,SEEK_SET);

  do {
    if (len>256*1024) {
      n=256*1024; len-=256*1024;
    } else {
      n=len; len=0;
    }
    if (fread(cbuf,1,n,f)!=n) {
      fclose(g);
      fclose(f);
      return(3);
    }
    if (fwrite(cbuf,1,n,g)!=n) {
      fclose(g);
      fclose(f);
      return(4);
    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
    if (ascii==27) return(5);

  } while (len>0);

  fclose(g);
  fclose(f);
  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
