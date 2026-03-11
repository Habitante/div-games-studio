
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Convierte de PCM a WAV
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define byte unsigned char
#define word unsigned short

int SaveWAV          (byte *byte_ptr, int length, FILE *f);
int IsWAV            (char *FileName);

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
  byte *ptr;
  char *filename=NULL;
  int  len;
  FILE *f;

  printf("PCM2WAV Conversor de archivos PCM a WAV (c) 1999 Freeware");

  if (argc<2) {
    printf("\n\nUso: PCM2WAV.EXE <Archivos> ฤู");
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

    if( IsWAV(full) ) goto siguiente_archivo;

    printf("\n\nLeyendo el archivo %s",full);

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

    _splitpath(full,drive,dir,fname,ext);
    _makepath(full,drive,dir,fname,".WAV");

    if ((f=fopen(full,"wb"))==0) {
      printf("\n\nError: No se pudo abrir el archivo para escritura.");
      free(ptr);
      goto siguiente_archivo;
    }

    printf("...Grabando");
    if(!SaveWAV(ptr, len, f)) {
      printf("...Error");
      free(ptr);
      fclose(f);
      goto siguiente_archivo;
    }
    printf("...Ok");

    free(ptr);
    fclose(f);

    siguiente_archivo: rc=_dos_findnext(&ft);
  }
}

typedef struct _HeadDC {
  unsigned int   dwUnknow;
  unsigned short wFormatTag;
  unsigned short wChannels;
  unsigned int   dwSamplePerSec;
  unsigned int   dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBits;
} HeadDC;

int SaveWAV(byte *byte_ptr, int length, FILE *dstfile)
{
  HeadDC MyHeadDC;
  int    con;
  float  paso,pos;

  fputc('R',dstfile);
  fputc('I',dstfile);
  fputc('F',dstfile);
  fputc('F',dstfile);

  length+=36;
  if(fwrite(&length,1,4,dstfile)!=4) {
    return(0);
  }
  length-=36;

  fputc('W',dstfile);
  fputc('A',dstfile);
  fputc('V',dstfile);
  fputc('E',dstfile);
  fputc('f',dstfile);
  fputc('m',dstfile);
  fputc('t',dstfile);
  fputc(' ',dstfile);

  MyHeadDC.dwUnknow         = 16;
  MyHeadDC.wFormatTag       = 1;
  MyHeadDC.wChannels        = 1;
  MyHeadDC.dwSamplePerSec   = 11025;
  MyHeadDC.dwAvgBytesPerSec = 11025;
  MyHeadDC.wBlockAlign      = 1;
  MyHeadDC.wBits            = 8;

  if(fwrite(&MyHeadDC,1,sizeof(HeadDC),dstfile)!=sizeof(HeadDC)) {
    return(0);
  }

  fputc('d',dstfile);
  fputc('a',dstfile);
  fputc('t',dstfile);
  fputc('a',dstfile);

  if(fwrite(&length,1,4,dstfile)!=4) {
    return(0);
  }

  if(fwrite(byte_ptr,1,length,dstfile)!=length)
  {
    return(0);
  }

  return(1);
}

int IsWAV(char *FileName)
{
  FILE *f;
  int  ok=1;

  if((f=fopen(FileName,"rb"))==NULL) return(0);

  if(fgetc(f)!='R') ok=0;
  if(fgetc(f)!='I') ok=0;
  if(fgetc(f)!='F') ok=0;
  if(fgetc(f)!='F') ok=0;
  fseek(f, 4, SEEK_CUR);
  if(fgetc(f)!='W') ok=0;
  if(fgetc(f)!='A') ok=0;
  if(fgetc(f)!='V') ok=0;
  if(fgetc(f)!='E') ok=0;

  fclose(f);

  return(ok);
}

