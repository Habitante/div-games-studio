
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Cuentabytes para el instalador de DIV Games Studio
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

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

#define byte unsigned char
#define word unsigned short

byte rndb(void);

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Programa principal
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void main(int argc, char * argv[]) {

  char full[_MAX_PATH+1];
  char drive[_MAX_DRIVE+1];
  char dir[_MAX_DIR+1];
  char fname[_MAX_FNAME+1];
  char ext[_MAX_EXT+1];
  char cwork[256];
  int len,n,total=0;
  byte *ptr,*p;
  FILE * f, * f2;

  printf("CUENTABYTES versiขn 1.03b (c) Noviembre 1997 por Daniel Navarro Medrano.");

  if (argc<2) {
    printf("\n - Utilizar el comando: dir c:\master /s /a-d /oe /b >[archivo]"
           "\n - Para aคadir las longitudes, utilizar: c.exe [archivo] ฤู");
    exit(0);
  }

  if (_fullpath(full,strupr(argv[1]),_MAX_PATH)==NULL) {
    printf("\nError en el nombre del archivo.");
    exit(0);
  }

  printf("\n\nLeyendo el archivo %s ... ",full);

  if ((f=fopen(argv[1],"rb"))==0) {
    printf("\nArchivo no encontrado.");
    exit(0);
  }

  fseek(f,0,SEEK_END); len=ftell(f);

  if ((ptr=(byte *)malloc(len))==NULL) {
    printf("\nMemoria insuficiente.");
    exit(0);
  }

  fseek(f,0,SEEK_SET);

  if (fread(ptr,1,len,f)!=len) {
    printf("\nNo se pudo leer el archivo.");
    exit(0);
  }

  fclose(f);

  _splitpath(full,drive,dir,fname,ext);
  _makepath(full,drive,dir,fname,".BAK");

  printf("\nGuardando una copia en %s ...",full);

  if ((f=fopen(full,"wb"))==0) {
    printf("\nNo se pudo abrir el archivo para escritura.");
    exit(0);
  }

  if (fwrite(ptr,1,len,f)!=len) {
    printf("\nNo se pudo guardar correctamente el archivo.");
    exit(0);
  }

  fclose(f);

  printf("\nGuardando el nuevo fichero ...\n");

  if ((f=fopen(argv[1],"wb"))==0) {
    printf("No se pudo abrir el archivo para escritura.");
    exit(0);
  }

  for (n=0;n<len;n++) if (ptr[n]==10 || ptr[n]==13) ptr[n]=0;

  p=ptr;

  while (p<ptr+len) {

    n=0; // No se pudo determinar su longitud

    if ((f2=fopen(p,"rb"))!=0) {
      fseek(f2,0,SEEK_END); n=ftell(f2); fclose(f2);
    } else {
      printf("Problemas con el archivo %s ...\n",p);
    }

    total+=n;

    sprintf(cwork,"%s %u\x0d\x0a",p,n);

    if (fwrite(cwork,1,strlen(cwork),f)!=strlen(cwork)) {
      printf("No se pudo guardar correctamente el archivo.");
      exit(0);
    }

    p+=strlen(p)+2;
  }

  fclose(f);

  printf("Borrando la copia de seguridad ...");

  remove(full);

  printf("\n\tTotal: %f Mb.",(float)total/1048576.0);

}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
