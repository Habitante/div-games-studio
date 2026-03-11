
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Encriptador/Desencriptador de ficheros
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

byte * clave=NULL;

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Programa principal
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void main(int argc, char * argv[]) {

  unsigned rc;
  struct find_t ft;
  char full[_MAX_PATH+1];
  char drive[_MAX_DRIVE+1];
  char dir[_MAX_DIR+1];
  char fname[_MAX_FNAME+1];
  char ext[_MAX_EXT+1];
  int len,n,modo=0,enc;
  byte *ptr,*filename=NULL;
  FILE * f;

  printf("DIVCODER versiขn 1.5a (c) 1997,98 por Daniel Navarro Medrano.");

  if (argc<2) {
    printf("\n\nUso: DC.EXE [Opciones] <Archivo> ฤู");
    printf("\n\nOpciones: -e     Encriptar los archivos especificados");
    printf("\n          -d     Desencriptar los archivos especificados");
    printf("\n          -cXXXX Utilizar clave (cualquier longitud, sin espacios)");
    exit(0);
  }

  n=1; do {
    if (argv[n][0]=='-' || argv[n][0]=='//') {
      switch(tolower(argv[n][1])) {
        case 'e': modo=1; break;
        case 'd': modo=2; break;
        case 'c':
          clave=&(argv[n][2]);
          strlwr(clave);
          break;
        default:
          strlwr(argv[n]);
          printf("\n\nError: Opciขn %s no conocida.",argv[n]);
          exit(0);
      }
    } else {
      filename=argv[n]; break;
    }
  } while (++n<argc);

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

    printf("\n\nLeyendo el archivo %s ... ",full);

    if ((f=fopen(ft.name,"rb"))==0) {
      printf("\n\nError: Archivo no encontrado.");
      goto siguiente_archivo;
    }

    fseek(f,0,SEEK_END); len=ftell(f);

    if ((ptr=(byte *)malloc(len+4))==NULL) {
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

    *(int*)(ptr+len)=0;

    if (modo==1) {
      if (!strcmp(ptr,"Zk!")) {
        printf("\nEl archivo ya est  encriptado.");
        free(ptr);
        goto siguiente_archivo;
      }
    } else if (modo==2) {
      if (strcmp(ptr,"Zk!")) {
        printf("\nEl archivo ya est  desencriptado.");
        free(ptr);
        goto siguiente_archivo;
      }
    }

    _splitpath(full,drive,dir,fname,ext);
    _makepath(full,drive,dir,fname,".Zk!");

    printf("\nGuardando una copia en %s ...",full);

    if ((f=fopen(full,"wb"))==0) {
      printf("\n\nError: No se pudo abrir el archivo para escritura.");
      free(ptr);
      goto siguiente_archivo;
    }

    if (fwrite(ptr,1,len,f)!=len) {
      printf("\n\nError: No se pudo guardar correctamente el archivo.");
      fclose(f);
      free(ptr);
      goto siguiente_archivo;
    }

    fclose(f);

    if (strcmp(ptr,"Zk!")) {
      printf("\nEncriptando %u bytes ...",len); enc=1;
      init_rnd(len); for (n=0;n<len;n++) ptr[n]^=rndb();
    } else {
      printf("\nDesencriptando %u bytes ...",len-4); enc=0;
      init_rnd(len-4); for (n=4;n<len;n++) ptr[n]^=rndb();
    }

    printf("\nGuardando el nuevo fichero ...");

    if ((f=fopen(ft.name,"wb"))==0) {
      printf("\n\nError: No se pudo abrir el archivo para escritura.");
      free(ptr);
      goto siguiente_archivo;
    }

    if (enc) {
      if (fwrite("Zk!",1,4,f)!=4) {
        printf("\n\nError: No se pudo guardar correctamente el archivo.");
        fclose(f);
        free(ptr);
        goto siguiente_archivo;
      }
      if (fwrite(ptr,1,len,f)!=len) {
        printf("\n\nError: No se pudo guardar correctamente el archivo.");
        fclose(f);
        free(ptr);
        goto siguiente_archivo;
      }
    } else {
      if (fwrite(ptr+4,1,len-4,f)!=len-4) {
        printf("\n\nError: No se pudo guardar correctamente el archivo.");
        fclose(f);
        free(ptr);
        goto siguiente_archivo;
      }
    }

    fclose(f);
    free(ptr);

    printf("\nBorrando la copia de seguridad ...");

    remove(full);

    siguiente_archivo: rc=_dos_findnext(&ft);
  }

}

void DaniDel(char *name) {
  unsigned rc;
  struct find_t ft;

  rc=_dos_findfirst(name,_A_NORMAL,&ft);
  while(!rc) {
    remove(ft.name);
    rc=_dos_findnext(&ft);
  }
}

//ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Generador de nฃmeros aleatorios
//ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

union {byte b[128]; int d[32];} seed; // Seed aleatorio (127 bytes + PTR)

byte rndb(void) {
byte ptr;
  ptr=seed.b[127];
  if ((seed.b[127]=(ptr+seed.b[ptr])&127)==127) seed.b[127]=0;
  return(seed.b[seed.b[127]]+=++ptr);
}

int rnd(void) {
  return((((rndb()&127)*256+rndb())*256+rndb())*256+rndb());
}

int rnd2(int min,int max) {
  if (max<min) {max^=min; min^=max; max^=min; }
  return((rnd()%(max-min+1))+min);
}

void init_rnd(int n){
  register a;
  for (a=0;a<32;a++) seed.d[a]=n;
  if (clave!=NULL) {
    for (a=0;a<127;a++) {
      if (!clave[a]) break;
      seed.b[a]^=clave[a];
    }
  }
  seed.b[127]&=127;
  for (a=0;a<2048;a++) rndb();
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
