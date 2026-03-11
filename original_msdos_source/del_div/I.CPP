
// Queda borrar los accesos directos del men£ y escritorio

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Desinstalador de DIV Games Studio 2
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

// Archivos que borra del directorio principal de DIV Games Studio 2

char * delfile[]={
  "d.exe","*.pif","*.map","*.prg","*.txt","*.cfg","*.swp","*.fpg","*.fnt",
  "*.pcm","*.pal","*.dll","*.fli","*.flc","*.pcx","*.bmp","*.jpg","*.dat",
  "*.bak","*.wld","*.wav"};

// Subdirectorios que borra, con todo su contenido (a partir del principal)

char * divdir[]={
  "dat","dll","fli","fnt","fpg","genspr","help","ifs","install","map","mod",
  "pal","pcm","prg","setup","system","wld"};

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

// Tama¤os

#define MOUSE_ANCHO         36
#define MOUSE_ALTO          42

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

#define max_fpgs 64     // Numero m ximo de fpgs cargados

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Variables globales
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int error=0; // Indicador del c¢digo de error producido

byte * vga = (byte *) 0xA0000; // Pantalla fisica
byte * copia; // Copia virtual de pantalla
byte * copia2; // Copia virtual de pantalla
byte dac[768]; // Paleta del programa
short scan[480*4]; // Por scan [x,an,x,an] se definen hasta 2 segmentos a volcar
int LinealMode; // Modo de la svga
int volcado_completo=0; // Indica si se debe realizar un volcado completo
int vga_an,vga_al; // Ancho y alto de pantalla

char ActDir[_MAX_PATH+1];
char Dir[_MAX_PATH+1];
char full[_MAX_PATH+1];

byte fondo[MOUSE_ANCHO*MOUSE_ALTO]; // Fondo del rat¢n
byte fondo2[420*40]; // Fondo del total de MB de ejemplos
int mouse_x,mouse_y,mouse_b,old_mouse_b,mouse_ok;

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

byte *f1,*f2; // Fonts del programa de instalaci¢n

byte * load_fnt(char * filename);

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

char winpath1[256]; // Path del grupo Hammer
char winpath2[128]; // Path del escritorio

void main(int argc, char * argv[]) {
  FILE *f;
  unsigned n;
  int boton;

  if (argc<1) exit(0);

  _fullpath(full,argv[0],_MAX_PATH+1);
  strupr(full);

  n=strlen(full);
  while (n && full[n]!='\\') n--;
  full[n]=0;
  if (full[n-1]==':') strcat(full,"\\");

  _dos_setdrive((int)toupper(full[0])-'A'+1,&n);
  chdir(full);

  inicializa_textos("lenguaje.ins");

  strcpy(winpath1,"");
  strcpy(winpath2,"");

  if ((f=fopen(text[9],"rb"))!=NULL) {
    fread(winpath1,1,256,f);
    fclose(f);
    strcpy(winpath2,winpath1+strlen(winpath1)+1);
  }

  mouse_ok=1;
  if (!is_mouse_ok()) {
    system("MOUSE.COM >NUL");
    if (!is_mouse_ok()) {
      mouse_ok=0;
    }
  }

  signal(SIGBREAK, SIG_IGN);      // anti ctrl-break
  signal(SIGINT, SIG_IGN);        // anti ctrl-c
  _harderr(critical_error);

  kbdInit();

  if ((copia=(byte*)malloc(640*480))==NULL) _error(5);
  if ((copia2=(byte*)malloc(640*480))==NULL) _error(5);

  if (load_fpg(text[6])==-1) _error(4);

  if ((f1=load_fnt(text[7]))==NULL) _error(4);
  if ((f2=load_fnt(text[8]))==NULL) _error(4);

  memset(copia,0,640*480);
  _setvideomode(_VRES16COLOR);
  init_mouse();

  svmode(); if (error==1) _error(3);

  set_paleta(0);
  set_dac();
  init_volcado();
  init_mouse();

  put(0,2,0,0);

  showtext(text[80],text[81]);

  memcpy(copia2,copia,640*480);

  ascii=0;

  while (ascii!=27 && (!(!(mouse_b&1) && (old_mouse_b&1)) || boton!=2)) {

    boton=0;

    if (mouse_x>349 && mouse_x<349+75 && mouse_y>270 && mouse_y<270+21) {
      put(0,51,349,270); boton=1;
    } else {
      put(0,50,349,270);
    }

    if (mouse_x>456 && mouse_x<456+75 && mouse_y>270 && mouse_y<270+21) {
      put(0,53,456,270); boton=2;
    } else {
      put(0,52,456,270);
    }

    if (!(mouse_b&1) && (old_mouse_b&1) && boton==1) {
      uninstall();
    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  }

  rvmode();
  kbdReset();

  exit(0);
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
//  Quita una ventana
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void clear_window(void) {

  restore(0,0,640,480);
  fade();

  while (mouse_b&1) {
    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Di logo informativo
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void showtext(char *h,char *e) {
  int max_an;
  char cwork[256];
  char cwork2[256];
  char pal[1024];
  int i=0;
  int xx0=100+8,xx1=100+440-8,yy0=183+28,yy1=183+114;

  text_out(f2,xx0+84,186,0,h);

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
}

// Imagen = 5-Error, 9-Informaci¢n

void info(int imagen, char *h,char *e) { // Ventana con un mensaje
  int aceptar=0;

  put(0,imagen,100,183);
  put(0,50,456,270);

  showtext(h,e); fade();

  do {
    if (mouse_x>456 && mouse_x<456+75 && mouse_y>270 && mouse_y<270+21) {
      put(0,51,456,270); aceptar=1;
    } else {
      put(0,50,456,270); aceptar=0;
    }

    read_mouse();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
    put(0,10,mouse_x-2,mouse_y-2);
    volcado();
    salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);

  } while (!(!(mouse_b&1) && (old_mouse_b&1)) || !aceptar);

  rvmode();
  kbdReset();
  exit(0);

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
  char cwork[128];

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
  exit(0);
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
//  Funci¢n principal de desinstalaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void por(float porcen);

int num_dir=0,num_dir2=0;

void uninstall(void) { // Ventana con un mensaje
  int x;
  unsigned n;
  FILE * f;

  put(0,6,100,183);

  fade();

  chdir(".."); // Se va al directorio principal (el del D.EXE)

  if ((f=fopen(text[82],"rb"))!=NULL) fclose(f);
  else info(5,text[83],text[84]);

  getcwd(ActDir,_MAX_PATH+1);
  for (x=strlen(ActDir)-1;x>=0;x--) if (ActDir[x]=='\\') break;
  ActDir[x]=0; strcpy(Dir,ActDir+strlen(ActDir)+1);

  num_dir=cuentadir(0);

  deletedir(0);

  por(100.0);

  chdir("..");          // P.ej: Se va al directorio C:\UTIL
  rmdir(Dir);           // y borra el directorio C:\UTIL\DIV

  // Borra los accesos directos

  if (strlen(winpath1)>4 && strlen(winpath2)>4) {

    _dos_setdrive((int)toupper(winpath1[0])-'A'+1,&n);
    chdir(winpath1);

    deletefile(text[87]);
    deletefile(text[88]);
    deletefile(text[89]);
    deletefile(text[90]);
    deletefile(text[91]);

    rmdir(winpath1);

    _dos_setdrive((int)toupper(winpath2[0])-'A'+1,&n);
    chdir(winpath2);

    deletefile(text[87]);

    chdir("\\");
  }

  info(9,text[85],text[86]);

}

void por(float porcen) { // Muestra el porcentaje y da un Frame
  static xporcen=0;
  int x;

  if (porcen>100.0) porcen=100.0;

  x=(int)((307.0*porcen)/100.0);
  while (xporcen<x) put_clip(0, 111, xporcen++, 163, 273-17, 0);

  read_mouse();
  salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,0);
  put(0,10,mouse_x-2,mouse_y-2);
  volcado();
  salvaguarda(&fondo,mouse_x-2,mouse_y-2,MOUSE_ANCHO,MOUSE_ALTO,1);
}

void deletedir(int nivel) { // Funci¢n principal de borrado de directorios
  unsigned rc;
  struct find_t ft;
  int x;

  rc=_dos_findfirst("*.*",_A_SUBDIR,&ft);
  while(!rc) {
    if (ft.attrib&_A_SUBDIR) {
      strlwr(ft.name);
      if (strcmp(ft.name,".") && strcmp(ft.name,"..") && strcmp(ft.name,"\\")) {
        for (x=0;x<sizeof(divdir)/sizeof(divdir[0]);x++)
          if (!strcmp(ft.name,divdir[x])) break;
        if (x<sizeof(divdir)/sizeof(divdir[0]) || nivel) {
          chdir(ft.name);
          deletedir(nivel+1);
          chdir("..");
          rmdir(ft.name);
        }
      }
    } rc=_dos_findnext(&ft);
  }

  por((100.0*(float)num_dir2++)/(float)num_dir);

  if (nivel) deletefile("*.*"); else {
    for (x=0;x<sizeof(delfile)/sizeof(delfile[0]);x++) {
      deletefile(delfile[x]);
    }
  }

}

void deletefile(char * name) { // Borra archivos en el directorio actual
  unsigned rc;
  char cwork[_MAX_PATH];
  struct find_t ft;

  rc=_dos_findfirst(name,_A_NORMAL,&ft);
  while(!rc) {
    if (_fullpath(cwork, ft.name, _MAX_PATH)==NULL) strcpy(cwork,ft.name);
    _dos_setfileattr(cwork,_A_NORMAL);
    remove(cwork);
    rc=_dos_findnext(&ft);
  }
}

int cuentadir(int nivel) { // Cuenta los directorios, para la barra de progreso
  unsigned rc;
  struct find_t ft;
  int n=0,x;

  rc=_dos_findfirst("*.*",_A_SUBDIR,&ft);
  while(!rc) {
    if (ft.attrib&_A_SUBDIR) {
      strlwr(ft.name);
      if (strcmp(ft.name,".") && strcmp(ft.name,"..") && strcmp(ft.name,"\\")) {
        for (x=0;x<sizeof(divdir)/sizeof(divdir[0]);x++)
          if (!strcmp(ft.name,divdir[x])) break;
        if (x<sizeof(divdir)/sizeof(divdir[0]) || nivel) {
          chdir(ft.name);
          n+=cuentadir(nivel+1)+1;
          chdir("..");
        }
      }
    } rc=_dos_findnext(&ft);
  }
  return(n);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
