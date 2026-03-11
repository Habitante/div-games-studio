
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Convierte de PCX a MAP
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define byte unsigned char
#define word unsigned short

int  es_PCX             (byte *buffer);
int  descomprime_PCX    (byte *buffer, byte *mapa);
byte *descomprime_rle   (byte * buffer,unsigned int bytes_line,unsigned int last_byte,byte * pDest);
int  es_MAP             (byte * buffer);
int  graba_MAP          (byte * mapa, int an, int al, FILE * f);

typedef struct tagRGBQUAD
{
  unsigned char   rgbBlue;
  unsigned char   rgbGreen;
  unsigned char   rgbRed;
  unsigned char   rgbReserved;
} RGBQUAD;

int  map_an, map_al;
byte dac4[768];
byte dac[768];

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Programa principal
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void main(int argc, char * argv[])
{
  unsigned rc;
  struct find_t ft;
  char full[_MAX_PATH+1];
  char drive[_MAX_DRIVE+1];
  char dir[_MAX_DIR+1];
  char fname[_MAX_FNAME+1];
  char ext[_MAX_EXT+1];
  byte *ptr,*ptr2;
  char *filename=NULL;
  int  len;
  FILE *f;

  printf("PCX2MAP conversor de archivos PCX a MAP (c) 1999 Freeware");

  if (argc<2) {
    printf("\n\nUso: PCX2MAP.EXE <Archivos> ฤู");
    exit(0);
  }

  filename=argv[1];

  if (filename==NULL) {
    printf("\n\nError: Se debe especificar un nombre de archivo.");
    exit(0);
  }

  rc=_dos_findfirst(filename,_A_NORMAL,&ft);

  if (rc) {
    printf("\n\nError: No se encontrข ningฃn archivo con ese nombre.");
    exit(0);
  }

  while(!rc) {

    if (_fullpath(full,strupr(ft.name),_MAX_PATH)==NULL) {
      printf("\n\nError: Error en el nombre del archivo.");
      exit(0);
    }

    printf("\n\nLeyendo el archivo %s",ft.name);

    if ((f=fopen(full,"rb"))==0) {
      printf("\n\nError: Archivo no encontrado.");
      goto siguiente_archivo;
    }

    fseek(f,0,SEEK_END); len=ftell(f);

    if ((ptr=(byte *)malloc(len))==NULL) {
      printf("\n\nError: Memoria insuficiente.");
      fclose(f);
      goto siguiente_archivo;
    }

    fseek(f,0,SEEK_SET);

    if (fread(ptr,1,len,f)!=len) {
      printf("\n\nError: No se pudo leer el archivo.");
      free(ptr);
      fclose(f);
      goto siguiente_archivo;
    }

    fclose(f);

    if( !es_PCX(ptr) || es_MAP(ptr) ) {
      free(ptr);
      goto siguiente_archivo;
    }

    _splitpath(full,drive,dir,fname,ext);
    _makepath(full,drive,dir,fname,".MAP");

    if ((ptr2=(byte *)malloc(map_an*map_al))==NULL) {
      printf("\n\nError: Memoria insuficiente.");
      free(ptr);
      goto siguiente_archivo;
    }

    if ((f=fopen(full,"wb"))==0) {
      printf("\n\nError: No se pudo abrir el archivo para escritura.");
      free(ptr);
      free(ptr2);
      goto siguiente_archivo;
    }

    printf("...Descomprimiendo");
    if(!descomprime_PCX(ptr, ptr2)) {
      printf("...Error");
      fclose(f);
      free(ptr);
      free(ptr2);
      goto siguiente_archivo;
    }
    printf("...Grabando");
    graba_MAP(ptr2, map_an, map_al, f);

    free(ptr);
    free(ptr2);
    fclose(f);

    siguiente_archivo: rc=_dos_findnext(&ft);
  }
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Formato PCX
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Structs
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

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

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Funciones
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

int es_PCX(byte *buffer)
{
  int loes=0;

  if(buffer[2]==1 && buffer[3]==8 && buffer[65]==1) loes=1;

  map_an=*(word*)(buffer+8)-*(word*)(buffer+4)+1;
  map_al=*(word*)(buffer+10)-*(word*)(buffer+6)+1;

  return(loes);
}

int descomprime_PCX(byte *buffer, byte *mapa)
{
  unsigned int con;
  unsigned int pixel=0, pixel_line=0;
  unsigned int last_byte,bytes_line;
  char ch, rep;
  pcx_header header;
  byte *pSrc, *pDest, *pSrcLine, *AuxPtr;
  int  rgb_color, x, y;
  byte rgb_blue, rgb_green, rgb_red;
  byte color16;
  int  con16;
  RGBQUAD Pcxdac[256];

  memcpy((byte *)&header,buffer,sizeof(pcx_header));
  buffer+=128;                                  // Comienzo de la imagen

  map_an = header.xmax - header.xmin + 1;
  map_al = header.ymax - header.ymin + 1;

  if((!map_an && !map_al) || map_an<0 || map_al<0) return(0);

  memset (mapa, 0, map_an * map_al);

  last_byte  = header.bytes_per_line * header.color_planes * map_al ;
  bytes_line = header.bytes_per_line * header.color_planes;

  pDest = mapa;

  if(header.bits_per_pixel==8 && header.color_planes==1)
  {
    do {
      ch=*buffer++;                               // Copia uno por defecto.
      if((ch&192)==192) {                         // Si RLE entonces
        rep=(ch&63);                              // rep = nง de veces a copiar.
        ch=*buffer++;
      } else rep=1;
      pixel+=rep;                                 // Controla que no nos salgamos.
      pixel_line+=rep;
      if(pixel>last_byte) {
        rep-=pixel-last_byte;
        for(con=0;con<rep;con++) *pDest++=ch;
        break;
      }
      if(pixel_line==bytes_line) {
        pixel_line=0;
        rep-=bytes_line-map_an;
      }
      for(con=0;con<rep;con++) *pDest++=ch;
    } while(1);

    memcpy(dac, buffer, 768);
    for (con=0; con<768; con++) dac[con] /= 4;
  }
  else return(0);

  return(1);
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Rutina de descompresiขn de RLE optimizada
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

byte * descomprime_rle(byte * buffer,unsigned int bytes_line,unsigned int last_byte,byte * pDest) {

  unsigned int con;
  unsigned int pixel=0, pixel_line=0;
  char ch, rep;

  do {
    ch=*buffer++;                               // Copia uno por defecto.
    if((ch&192)==192) {                         // Si RLE entonces
      rep=(ch&63);                              // rep = nง de veces a copiar.
      ch=*buffer++;
    } else rep=1;
    pixel+=rep;                                 // Controla que no nos salgamos.
    pixel_line+=rep;
    if(pixel>last_byte) {
      rep-=pixel-last_byte;
      for(con=0;con<rep;con++) *pDest++=ch;
      break;
    }
    if(pixel_line==bytes_line) {
      pixel_line=0;
      if(bytes_line&1) buffer++;
    }
    for(con=0;con<rep;con++) *pDest++=ch;
  } while(1);

  return(buffer);
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Formato MAP
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

int es_MAP (byte * buffer) {
  if (!strcmp(buffer,"map\x1a\x0d\x0a")) {
    map_an=*(word*)(buffer+8);
    map_al=*(word*)(buffer+10);
    return(1);
  } else return(0);
}

void graba_MAP (byte * mapa, int an, int al, FILE * f) {
  int y;
  char cwork[32]="";
  char reglas[576];

  fwrite("map\x1a\x0d\x0a\x00\x00",8,1,f);      // +000 Cabecera y version
  fwrite(&an,2,1,f);                   // +008 Ancho
  fwrite(&al,2,1,f);                   // +010 Alto
  y=1; fwrite(&y,4,1,f);// +012 Cขdigo

  fwrite(cwork,32,1,f);// +016 Descripcion
  fwrite(dac,768,1,f);                          // +048 Paleta

  for (y=0;y<16;y++) {
    reglas[y*36]=16;
    reglas[y*36+1]=0;
    reglas[y*36+2]=0;
    reglas[y*36+3]=0;
    memset(&reglas[y*36+4],y*16,32);
  } fwrite(reglas,1,sizeof(reglas),f);            // +816 Reglas de color

  y=0; fwrite(&y,2,1,f);                     // +1392 Numero de puntos
  fwrite(mapa,an*al,1,f);
}

