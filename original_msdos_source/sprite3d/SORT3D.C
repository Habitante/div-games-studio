#include <dos.h>
#include <stdio.h>
#include <conio.h>

typedef struct _o3d_header{
  char  Chunk[4];
  short Version;

  short nObjects;
  short nMaterials;
  short nDummies;
  short nTapes;
  short nAnims;

  int   oObjects;
  int   oMaterials;
  int   oDummies;
  int   oTapes;
  int   oAnims;
  float bbox[6];
}o3d_header;

typedef struct _o3d_object{
  short nVertices;
  short nCaras;
  short material;
  char  Nombre[20];
  float bbox[6];
}o3d_object;

typedef struct _o3d_vertex{
  float x,y,z;
  float nx,ny,nz;
  float u,v;
}o3d_vertex;

typedef struct _o3d_face{
  short a,b,c;
  short flags;
}o3d_face;

typedef struct _o3d_matrix{
  float p[16];
}o3d_matrix;


void main(int argc,char *argv[])
{
int rc,x;
struct find_t  files;
char cwork[128];


  printf("\nOrdenador de objetos V1.0 by Fernando Perez Alonso.\n\n");
  if(argc!=3)
  {
    printf("Usar: SORTO3D <nombre_directorio> <objeto_base>\n\n");
    return;
  }
  strcpy(cwork,argv[1]);
  strcat(cwork,"\\");
  strcat(cwork,"*.o3d");
  rc=_dos_findfirst(cwork,_A_NORMAL,&files);
  GetBaseo3d(argv[2]);
  while(!rc)
  {
    strcpy(cwork,argv[1]);
    strcat(cwork,"\\");
    strcat(cwork,files.name);
    Sorto3d(cwork);
    rc=_dos_findnext(&files);
  }
}

char Names[128][20];
o3d_object  *lpobj;
o3d_vertex  *lpvert[128];
o3d_face    *lpface[128];
o3d_matrix  matr[128][128];
o3d_matrix  basematr[128][128];
o3d_object  wobj;
o3d_vertex  wvert;
o3d_face    wface;
o3d_header  O3D_HEADER;
int nobjects;

void GetBaseo3d(char *name)
{
FILE *f;
static int primero=1;
int z,x,y;
float *p;

  f=fopen(name,"rb");
  fread(&O3D_HEADER,sizeof(O3D_HEADER),1,f);
  printf("OBJETO BASE: %s\n",name);
  nobjects=O3D_HEADER.nObjects;
  for(x=0;x<O3D_HEADER.nObjects;x++)
  {
    fread(&wobj,sizeof(o3d_object),1,f);
    strcpy(Names[x],wobj.Nombre);
    for(y=0;y<wobj.nVertices;y++) fread(&wvert,sizeof(o3d_vertex),1,f);
    for(y=0;y<wobj.nCaras;y++) fread(&wface,sizeof(o3d_face),1,f);
  }
  fclose(f);
}


void Sorto3d(char *name)
{
FILE *f;
static int primero=1;
int z,x,y;
float *p;

  f=fopen(name,"rb+");
  fread(&O3D_HEADER,sizeof(O3D_HEADER),1,f);
  // lee los objetos

  if(O3D_HEADER.nObjects!=nobjects) printf("El objeto %s no es ordenable.(%d\%d)\n",name,O3D_HEADER.nObjects,nobjects);
  printf("OBJETO ORDENADO: %s\n",name);
  lpobj=(o3d_object *)malloc(sizeof(o3d_object)*O3D_HEADER.nObjects);
  for(x=0;x<O3D_HEADER.nObjects;x++)
  {
    fread(&lpobj[x],sizeof(o3d_object),1,f);

    lpvert[x]=(o3d_vertex *)malloc(sizeof(o3d_vertex)*lpobj[x].nVertices);
    fread(lpvert[x],sizeof(o3d_vertex),lpobj[x].nVertices,f);

    lpface[x]=(o3d_face *)malloc(sizeof(o3d_face)*lpobj[x].nCaras);
    fread(lpface[x],sizeof(o3d_face),lpobj[x].nCaras,f);
  }
  // Busca las animaciones y las lee
  fseek(f,O3D_HEADER.oAnims,SEEK_SET);
  for(y=0;y<O3D_HEADER.nAnims;y++)
    for(x=0;x<O3D_HEADER.nObjects;x++)
      fread(&matr[y][x],sizeof(o3d_matrix),1,f);
  // Salva los objetos
  fseek(f,O3D_HEADER.oObjects,SEEK_SET);
  for(z=0;z<O3D_HEADER.nObjects;z++)
    for(x=0;x<O3D_HEADER.nObjects;x++)
      if(!strcmp(Names[z],lpobj[x].Nombre))
      {
        fwrite(&lpobj[x],sizeof(o3d_object),1,f);
        fwrite(lpvert[x],sizeof(o3d_vertex),lpobj[x].nVertices,f);
        fwrite(lpface[x],sizeof(o3d_face),lpobj[x].nCaras,f);
      }

  // Salva las animaciones
  fseek(f,O3D_HEADER.oAnims,SEEK_SET);
  for(y=0;y<O3D_HEADER.nAnims;y++)
    for(z=0;z<O3D_HEADER.nObjects;z++)
      for(x=0;x<O3D_HEADER.nObjects;x++)
        if(!strcmp(Names[z],lpobj[x].Nombre))
          fwrite(&matr[y][x],sizeof(o3d_matrix),1,f);

  free(lpobj);
  for(x=0;x<O3D_HEADER.nObjects;x++)
  {
    free(lpvert[x]);
    free(lpface[x]);
  }
  fclose(f);
}

