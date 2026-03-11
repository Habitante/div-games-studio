
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mem.h>

#define Cmpsize 16 // Bytes que deben coincidir (m¡nimo 5)
#define Patsize 4 // Tama¤o m¡nimo de patch

#define byte unsigned char

byte * buscabytes(void);

typedef struct {
  unsigned off;
  unsigned len;
} tTablaPatch;

tTablaPatch TablaPatch[16384];

unsigned iPatch=0;
unsigned p1=0,l1=0;
unsigned p2=0,l2=0;

byte *Old, *New, *Pat;

byte *iNew, *EndNew, *EndOld;

unsigned Newsize,Oldsize,LenPat;

char FileName1[13], FileName2[13], FileName3[13];

void main(void) {
  FILE *f;
  byte * ptr;
  int n;

  strcpy(FileName1, "d.old");
  strcpy(FileName2, "d.new");
  strcpy(FileName3, "d.pat");

  if( (f=fopen(FileName2, "rb"))==NULL ) {
    printf("Fichero %s no encontrado.", FileName2);
    exit(1);
  }
  fseek(f, 0, SEEK_END); Newsize=ftell(f); fseek(f, 0, SEEK_SET);
  if( (New=(byte *)malloc(Newsize))==NULL ) {
    printf("No hay suficiente memoria.");
    exit(1);
  }
  if( fread(New, 1, Newsize, f)!=Newsize ) {
    printf("Error al leer fichero %s.", FileName2);
    exit(1);
  }
  fclose(f);

  iNew=New;
  EndNew=New+Newsize;

  if( (f=fopen(FileName1, "rb"))==NULL ) {
    printf("Fichero %s no encontrado.", FileName1);
    exit(1);
  }
  fseek(f, 0, SEEK_END); Oldsize=ftell(f); fseek(f, 0, SEEK_SET);
  if( (Old=(byte *)malloc(Oldsize+Newsize))==NULL ) {
    printf("No hay suficiente memoria.");
    exit(1);
  }
  if( fread(Old, 1, Oldsize, f)!=Oldsize ) {
    printf("Error al leer fichero %s.", FileName1);
    exit(1);
  }
  fclose(f);

  Pat=Old+Oldsize;
  EndOld=Pat;

  // Bucle principal

  while (iNew+Cmpsize<EndNew) {

     // 1§ Busca los Cmpsize bytes de iNew en Old

     if ((ptr=buscabytes())!=NULL) { // Patch de Old

       TablaPatch[iPatch].off=(unsigned)ptr;
       TablaPatch[iPatch].len=LenPat;

       iPatch++; iNew+=LenPat;

       p1++; l1+=LenPat;

     } else {                        // Patch nuevo

       if (iPatch && (unsigned)Pat==TablaPatch[iPatch-1].off+TablaPatch[iPatch-1].len) {

         if (iNew+Patsize<=EndNew) LenPat=Patsize; // Amplia un Patch
         else LenPat=(unsigned)EndNew-(unsigned)iNew;

         memcpy(Pat,iNew,LenPat);

         TablaPatch[iPatch-1].len+=LenPat;
         iNew+=LenPat; Pat+=LenPat;

         l2+=LenPat;

       } else {                                    // Crea uno nuevo

         TablaPatch[iPatch].off=(unsigned)Pat;

         if (iNew+Patsize<=EndNew) LenPat=Patsize;
         else LenPat=(unsigned)EndNew-(unsigned)iNew;

         memcpy(Pat,iNew,LenPat);

         TablaPatch[iPatch].len=LenPat;
         iPatch++; iNew+=LenPat; Pat+=LenPat;

         p2++; l2+=LenPat;

       }

     }

  }

  if (iNew<EndNew) {

    TablaPatch[iPatch].off=(unsigned)Pat;

    LenPat=(unsigned)EndNew-(unsigned)iNew;

    memcpy(Pat,iNew,LenPat);

    TablaPatch[iPatch].len=LenPat;
    iPatch++; iNew+=LenPat; Pat+=LenPat;

    p2++; l2+=LenPat;

  }

  for (n=0;n<iPatch;n++) {
    TablaPatch[n].off-=(unsigned)Old;
  }

  if( (f=fopen(FileName3, "wb"))==NULL ) {
    printf("No se pudo crear %s.", FileName3);
    exit(1);
  }
  if( fwrite(TablaPatch, sizeof(tTablaPatch), iPatch, f)!=iPatch ) {
    printf("Error al guardar fichero %s.", FileName3);
    exit(1);
  }
  if( fwrite(EndOld, 1, (unsigned)Pat-(unsigned)EndOld, f)!=(unsigned)Pat-(unsigned)EndOld ) {
    printf("Error al guardar fichero %s.", FileName3);
    exit(1);
  }
  fclose(f);

  printf("%d Patch de OLD (%d bytes)\n",p1,l1);
  printf("%d Patch nuevos (%d bytes)\n",p2,l2);

}

byte * buscabytes(void) {
  byte * ptr=Old;
  byte * ptrend=Old+Oldsize;
  byte * p,* q;
  unsigned inicio;

  inicio=*(unsigned*)(iNew+1);

  while (ptr<ptrend-Cmpsize) {

    if ((p=(byte*)memchr(ptr,*iNew,(unsigned)ptrend-(unsigned)ptr-Cmpsize))==NULL) return(NULL);

    // Mira si ha encontrado la cadena ...

    if (inicio==*(unsigned*)(p+1)) { // Comprueba los siguientes 4 bytes

      // Ya coinciden 5 bytes ...

      q=iNew+5; p+=5;

      while(q<EndNew && p<EndOld) {
        if (*p!=*q) break;
        p++; q++;
      }

      LenPat=(unsigned)q-(unsigned)iNew;

      p-=LenPat;

      if (LenPat>=Cmpsize) return(p); // ­­ Lo encontramos !!

    }

    // Si no, contin£a buscando ...

    ptr=p+1;
  }

  return(NULL);
}
