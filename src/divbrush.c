
//-----------------------------------------------------------------------------
// Brush/texture thumbnail infrastructure for the paint editor.
// Extracted from divmap3d.c after removing the MODE8/3D map editor.
//-----------------------------------------------------------------------------

#include "global.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

#define FONDO 3

//-----------------------------------------------------------------------------
// FPG file format types (were in divmap3d.c, needed for thumbnail loading)
//-----------------------------------------------------------------------------

typedef struct {
  char magic1[3];
  char magic2[4];
  char version;
  byte pal[768];
  byte gamas[576];
} FPG_header;

typedef struct {
  int  cod;
  int  tam;
  char descrip[32];
  char filename[12];
  int  ancho;
  int  alto;
  int  puntos;
} FPG_info;

typedef struct {
  word x;
  word y;
} FPG_points;

typedef struct {
  FPG_info   info;
  FPG_points *puntos;
  byte       *imagen;
} FPG_data;

//-----------------------------------------------------------------------------
// External prototypes
//-----------------------------------------------------------------------------

void paint_slider_br(struct t_listboxbr * l);

// Forward declarations for this file
void MapperBrowseFPG1(void);
void MapperBrowseFPG2(void);
void M3D_paint_listboxbr(struct t_listboxbr * l);
void M3D_show_thumb(struct t_listboxbr * l, int num);
void M3D_create_listboxbr(struct t_listboxbr * l);

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

// Path to brush FPG file (replaces m3d_edit.fpg_path)
byte brush_fpg_path[256];

int       tex_sop[11] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };

int scroll_x, scroll_y;
int num_bandera=0;

byte FPG_pal[768];
byte FPG_xlat[256];
int  FPG_thumbpos;

char cadenas[5][10];

int altura_techo=2048;
int altura_suelo=1024;

#define max_texturas 1000
#define an_textura (3+1) // 000 - 999
char m3d_fpgcodesbr[max_texturas*an_textura];
char textura[max_texturas*an_textura];
char fondo[max_texturas*an_textura];
int  t_maximo;
int  f_maximo;
struct t_listboxbr ltexturasbr={3-2,11-2,m3d_fpgcodesbr,an_textura,4,4,32,32};

struct _thumb_tex {
  int an,al;
  int RealAn, RealAl;
  char * ptr;
  int status;
  int FilePos;
  int Code;
  int Cuad;
} thumb_tex[max_texturas];

extern int _omx, _omy, omx, omy, oclock, num, incremento;

struct {
  int pos;
  int total;
} FPG_progress;

int TipoTex;

struct t_listboxbr copia_br;

int t_pulsada=1;

float zoom_level;

extern int num_pincel;
extern int TipoBrowser;
extern struct _thumb_map {
  int an,al;
  int RealAn, RealAl;
  char * ptr;
  int status;
  int FilePos;
  int Code;
  int Cuad;
} thumb_map[];

//-----------------------------------------------------------------------------
// Comparison functions for qsort
//-----------------------------------------------------------------------------

int strcmpsort(const void *a, const void *b) {
	return strcmp((char *)a,(char *)b);
}

int cmpcode(const void * a, const void * b)
{
  return(((struct _thumb_tex *)a)->Code-((struct _thumb_tex *)b)->Code);
}

//-----------------------------------------------------------------------------
// M3D_create_thumbs - Load FPG file and create brush thumbnails
//-----------------------------------------------------------------------------

void M3D_create_thumbs(struct t_listboxbr * l, int prog)
{
  FILE       *FPG_F;
  FPG_header  FPG_H;
  FPG_data    FPG_D;
  char        cwork[4];
  int         n, con;
  int         m;
  int         man, mal;
  byte        *temp, *temp2;
  float       coefredy, coefredx, a, b;
  int         x, y;

  if(brush_fpg_path[0]==0) return;

  if(TipoTex>3) FPG_thumbpos = 0;

  thumb_tex[0].ptr    = NULL;
  thumb_tex[0].status = 0;
  thumb_tex[0].Code   = 0;
  strcpy(&textura[0], "000");
  strcpy(&fondo[0], "000");
  t_maximo=f_maximo=1;

  sprintf(cadenas[0], "%d", scroll_x);
  sprintf(cadenas[1], "%d", scroll_y);
  sprintf(cadenas[2], "%d", num_bandera);
  sprintf(cadenas[3], "%d", altura_techo);
  sprintf(cadenas[4], "%d", altura_suelo);

  // Free existing thumbnails
  for(n=0; n<max_texturas; n++)
  {
    if(thumb_tex[n].ptr!=NULL)
    {
      free(thumb_tex[n].ptr);
      thumb_tex[n].ptr=NULL;
    }
  }

  if(brush_fpg_path[0]==0) return;
  debugprintf("Trying to load %s\n",(char *)brush_fpg_path);

  if((FPG_F=fopen((char *)brush_fpg_path,"rb"))==NULL)
  {
    v_texto=(char *)texto[43];
    dialogo(err0);
    return;
  }
  fseek(FPG_F, 0, SEEK_END);
  FPG_progress.pos   = 1352;
  FPG_progress.total = ftell(FPG_F) * 2;
  fseek(FPG_F, 0, SEEK_SET);

  if(fread(&FPG_H, 1, sizeof(FPG_header), FPG_F)!=sizeof(FPG_header))
  {
    fclose(FPG_F);
    v_texto=(char *)texto[44];
    dialogo(err0);
    return;
  }

  memcpy(FPG_pal, FPG_H.pal, 768);
  create_dac4();

  if (modo<100) {
    for(n=0; n<256; n++) {
      switch ((FPG_pal[n*3]+FPG_pal[n*3+1]+FPG_pal[n*3+2])/22) {
        case 1: FPG_xlat[n]=c0; break;
        case 2: FPG_xlat[n]=c01; break;
        case 3: FPG_xlat[n]=c1; break;
        case 4: FPG_xlat[n]=c12; break;
        case 5: FPG_xlat[n]=c2; break;
        case 6: FPG_xlat[n]=c23; break;
        case 7: FPG_xlat[n]=c3; break;
        case 8: FPG_xlat[n]=c34; break;
        case 9: FPG_xlat[n]=c4; break;
      }
    } FPG_xlat[0]=0;
  } else {
    for(n=0; n<256; n++)
      FPG_xlat[n] = fast_find_color(FPG_pal[n*3], FPG_pal[n*3+1], FPG_pal[n*3+2]);
  }

  // In paint mode, start from index 0 (no empty texture slot)
  n=t_maximo=f_maximo=0;
  for(;;)
  {
    if(fread(&(FPG_D.info), 1, sizeof(FPG_info), FPG_F)!=sizeof(FPG_info))
    {
      if(feof(FPG_F))
      {
        l->maximo=f_maximo=n;
        break;
      }
      else
      {
        fclose(FPG_F);
        if (prog) Progress((char *)texto[93], FPG_progress.total, FPG_progress.total);
        v_texto=(char *)texto[44];
        dialogo(err0);
        return;
      }
    }
    fseek( FPG_F, sizeof(FPG_points)*FPG_D.info.puntos, SEEK_CUR );
    FPG_progress.pos+=(64+4*FPG_D.info.puntos+FPG_D.info.ancho*FPG_D.info.alto);
    if (prog) Progress((char *)texto[93], FPG_progress.pos, FPG_progress.total);

    thumb_tex[n].Cuad=0;
    for(con=0; con<11; con++)
    {
      if(FPG_D.info.ancho==tex_sop[con] && FPG_D.info.alto==tex_sop[con])
      {
        thumb_tex[n].Cuad=1;
      }
    }

    thumb_tex[n].an   = FPG_D.info.ancho;
    thumb_tex[n].al   = FPG_D.info.alto;
    thumb_tex[n].Code = FPG_D.info.cod;

    sprintf(cwork, "%03d", FPG_D.info.cod);
    if(thumb_tex[n].Cuad)
    {
      strcpy(textura+t_maximo*an_textura, cwork);
      t_maximo++;
    }
    strcpy(fondo+n*an_textura, cwork);

    thumb_tex[n].FilePos = ftell(FPG_F);

    if((FPG_D.imagen=(byte *)malloc(FPG_D.info.ancho*FPG_D.info.alto))==NULL)
    {
      for(n=0; n<max_texturas; n++)
      {
        if(thumb_tex[n].ptr!=NULL)
        {
          free(thumb_tex[n].ptr);
          thumb_tex[n].ptr=NULL;
        }
      }
      fclose(FPG_F);
      if (prog) Progress((char *)texto[93], FPG_progress.total, FPG_progress.total);
      v_texto=(char *)texto[45];
      dialogo(err0);
      return;
    }

    if( fread(FPG_D.imagen, 1, FPG_D.info.ancho*FPG_D.info.alto, FPG_F)
        != FPG_D.info.ancho*FPG_D.info.alto )
    {
      for(n=0; n<max_texturas; n++)
      {
        if(thumb_tex[n].ptr!=NULL)
        {
          free(thumb_tex[n].ptr);
          thumb_tex[n].ptr=NULL;
        }
      }
      fclose(FPG_F);
      free(FPG_D.imagen);
      if (prog) Progress((char *)texto[93], FPG_progress.total, FPG_progress.total);
      v_texto=(char *)texto[44];
      dialogo(err0);
      return;
    }

    thumb_tex[n].ptr = (char *)FPG_D.imagen;

    n++;
  }

  fclose(FPG_F);

  // In paint mode, start from index 0 (no empty texture slot)
  for(con=0; con<l->maximo; con++)
  {
    for(;;)
    {
      man  = thumb_tex[con].RealAn = thumb_tex[con].an;
      mal  = thumb_tex[con].RealAl = thumb_tex[con].al;
      temp = (byte *)thumb_tex[con].ptr;

      if(man<=32*big2 && mal<=32*big2)
      {
        for(n=thumb_tex[con].an*thumb_tex[con].al-1; n>=0; n--)
        {
          temp[n]=FPG_xlat[temp[n]];
        }
      }
      else
      {
        coefredx=man/((float)32*2*(float)big2);
        coefredy=mal/((float)32*2*(float)big2);
        thumb_tex[con].an=(float)man/coefredx+0.5;
        thumb_tex[con].al=(float)mal/coefredy+0.5;
        thumb_tex[con].an&=-2; thumb_tex[con].al&=-2;
        if(thumb_tex[con].an<2) thumb_tex[con].an=2;
        if(thumb_tex[con].al<2) thumb_tex[con].al=2;

        if((temp2=(byte *)malloc(thumb_tex[con].an*thumb_tex[con].al))==NULL)
        {
          free(thumb_tex[con].ptr);
          thumb_tex[con].ptr    = NULL;
          thumb_tex[con].status = 0;
          break;
        }

        memset(temp2,0,thumb_tex[con].an*thumb_tex[con].al);
        a=(float)0.0;
        for(y=0;y<thumb_tex[con].al;y++) {
          b=(float)0.0;
          for(x=0;x<thumb_tex[con].an;x++) {
            temp2[y*thumb_tex[con].an+x]=temp[((int)a)*man+(int)b];
            b+=coefredx;
          } a+=coefredy;
        }

        free(thumb_tex[con].ptr);

        for(n=thumb_tex[con].an*thumb_tex[con].al-1; n>=0; n--)
        {
          temp2[n]=FPG_xlat[temp2[n]];
        }

        if((thumb_tex[con].ptr=(char *)malloc((thumb_tex[con].an*thumb_tex[con].al)/4))==NULL)
        {
          free(temp2);
          thumb_tex[con].ptr    = NULL;
          thumb_tex[con].status = 0;
          break;
        }
        for (y=0;y<thumb_tex[con].al;y+=2)
        {
          for (x=0;x<thumb_tex[con].an;x+=2)
          {
            n=*(ghost+temp2[x+y*thumb_tex[con].an]*256+temp2[x+1+y*thumb_tex[con].an]);
            m=*(ghost+temp2[x+(y+1)*thumb_tex[con].an]*256+temp2[x+1+(y+1)*thumb_tex[con].an]);
            thumb_tex[con].ptr[x/2+(y/2)*(thumb_tex[con].an/2)]=*(ghost+n*256+m);
          }
        }
        thumb_tex[con].an/=2; thumb_tex[con].al/=2;
        free(temp2);
      }
      thumb_tex[con].status = 1;
      FPG_progress.pos+=(FPG_progress.total-FPG_progress.pos)/(l->maximo-con);
      if (prog) Progress((char *)texto[93], FPG_progress.pos, FPG_progress.total);
      break;
    }
  }

  if(FPG_progress.pos < FPG_progress.total)
  {
    if (prog) Progress((char *)texto[93], FPG_progress.total, FPG_progress.total);
  }

  qsort(thumb_tex,l->maximo,sizeof(struct _thumb_tex),cmpcode);
  qsort(textura,t_maximo,an_textura,strcmpsort);
  qsort(fondo,l->maximo,an_textura,strcmpsort);
}

//-----------------------------------------------------------------------------
// M3D_show_thumb - Display a thumbnail in the browser
//-----------------------------------------------------------------------------

#define MAPBR 8

void M3D_show_thumb(struct t_listboxbr * l, int num)
{
  byte * ptr=v.ptr,c;
  int an=v.an/big2,al=v.al/big2;
  int px,py,x,y,ly,incy;
  int num_tex;
  char *p;
  struct _thumb_tex thumb_tmp[max_texturas];

  if(TipoBrowser==MAPBR) memcpy(&thumb_tmp, &thumb_map, sizeof(struct _thumb_map)*max_windows);
  else                   memcpy(&thumb_tmp, &thumb_tex, sizeof(struct _thumb_tex)*max_texturas);

  if (num>=l->inicial && num<l->inicial+l->lineas*l->columnas) {
    if(TipoBrowser==MAPBR) {
      num_tex=num;
    } else {
      num_tex=atoi(m3d_fpgcodesbr+num*an_textura);
      if(num_tex)
      {
        for(x=0; x<max_texturas; x++)
        {
          if(thumb_tmp[x].Code==num_tex)
          {
            num_tex=x;
            break;
          }
        }
      }
    }

    px=(l->x+1+(l->an+1)*((num-l->inicial)%l->columnas))*big2+(l->an*big2-thumb_tmp[num_tex].an)/2;
    if ((incy=((l->al-8)*big2-thumb_tmp[num_tex].al)/2)<0) incy=0;
    py=(l->y+1+(l->al+1)*((num-l->inicial)/l->columnas))*big2+incy;

    ly=(l->y+(l->al+1)*((num-l->inicial)/l->columnas)+l->al-8)*big2;

    wbox(ptr,an,al,c01,(px-(l->an*big2-thumb_tmp[num_tex].an)/2)/big2,(py-incy)/big2+l->al-8,l->an,8);

    if (thumb_tmp[num_tex].ptr!=NULL && thumb_tmp[num_tex].status) {
      if (TipoBrowser==4 /*BRUSH*/) for(y=0;y<thumb_tmp[num_tex].al;y++) {
        for(x=0;x<thumb_tmp[num_tex].an;x++) {
          if (py+y>ly) c=*(ghost+c0*256+thumb_tmp[num_tex].ptr[y*thumb_tmp[num_tex].an+x]);
          else c=thumb_tmp[num_tex].ptr[y*thumb_tmp[num_tex].an+x];
          if (c) v.ptr[(py+y)*v.an+(px+x)]=c;
        }
      } else for(y=0;y<thumb_tmp[num_tex].al;y++) {
        for(x=0;x<thumb_tmp[num_tex].an;x++) {
          if (py+y>ly)
            v.ptr[(py+y)*v.an+(px+x)]=*(ghost+c0*256+thumb_tmp[num_tex].ptr[y*thumb_tmp[num_tex].an+x]);
          else
            v.ptr[(py+y)*v.an+(px+x)]=thumb_tmp[num_tex].ptr[y*thumb_tmp[num_tex].an+x];
        }
      }
    }

    px-=(l->an*big2-thumb_tmp[num_tex].an)/2;
    py-=incy;
    px/=big2; py/=big2;

    if (thumb_tmp[num_tex].ptr==NULL && !thumb_tmp[num_tex].status && num!=0) {
      wput(ptr,an,al,px+(l->an-21)/2,py+1,60);
    }

    if (TipoBrowser!=MAPBR) {
      if (modo<100 && num_tex==num_pincel) {
        wbox(ptr,an,al,c_b_low,px,py+l->al-8,l->an,8);
      }

      py+=l->al-1;
      p=l->lista+l->lista_an*num;

      if (l->zona-10==num-l->inicial) x=c4; else x=c3;
      wwrite(ptr,an,al,px+l->an/2+1,py,7,(byte *)p,c0);
      wwrite(ptr,an,al,px+l->an/2,py,7,(byte *)p,x);
    }

    v.volcar=1;
  }
}

//-----------------------------------------------------------------------------
// M3D_create_listboxbr - Create a thumbnail listbox browser
//-----------------------------------------------------------------------------

void M3D_create_listboxbr(struct t_listboxbr * l)
{
  byte *ptr=v.ptr;
  int an=v.an/big2,al=v.al/big2;
  int x,y;

  if (!l->creada) {
    l->slide=l->s0=l->y+9;
    l->s1=l->y+(l->al*l->lineas+l->lineas+1)-12;
    l->botones=0;
    l->creada=1;
    l->zona=0;
    l->inicial=FPG_thumbpos;
    if ((l->inicial+(l->lineas-1)*l->columnas)>=l->maximo) {
      l->inicial=0;
    }
  }

  wbox(ptr,an,al,c1,l->x,l->y,(l->an+1)*l->columnas,(l->al+1)*l->lineas);

  for (y=0;y<l->lineas;y++)
    for (x=0;x<l->columnas;x++)
      wrectangle(ptr,an,al,c0,l->x+(x*(l->an+1)),l->y+(y*(l->al+1)),l->an+2,l->al+2);

  wrectangle(ptr,an,al,c0,l->x+(l->an+1)*l->columnas,l->y,9,(l->al+1)*l->lineas+1);
  wrectangle(ptr,an,al,c0,l->x+(l->an+1)*l->columnas,l->y+8,9,(l->al+1)*l->lineas-15);
  wput(ptr,an,al,l->x+(l->an+1)*l->columnas+1,l->y+1,-39);
  wput(ptr,an,al,l->x+(l->an+1)*l->columnas+1,l->y+(l->al+1)*l->lineas-7,-40);

  M3D_paint_listboxbr(l);
  paint_slider_br(l);
}

//-----------------------------------------------------------------------------
// M3D_update_listboxbr - Update a thumbnail listbox browser
//-----------------------------------------------------------------------------

void M3D_update_listboxbr(struct t_listboxbr * l)
{
  byte * ptr=v.ptr, *p;
  int an=v.an/big2,al=v.al/big2;
  int n,old_zona=l->zona,x,y;

  if (wmouse_in(l->x,l->y,(l->an+1)*l->columnas,(l->al+1)*l->lineas)) {
    l->zona=(wmouse_x-l->x)/(l->an+1)+((wmouse_y-l->y)/(l->al+1))*l->columnas;
    if (l->zona>=l->maximo-l->inicial || l->zona>=l->lineas*l->columnas) l->zona=1;
    else l->zona+=10;
  } else if (wmouse_in(l->x+(l->an+1)*l->columnas,l->y,9,9)) l->zona=2;
  else if (wmouse_in(l->x+(l->an+1)*l->columnas,l->y+(l->al+1)*l->lineas-8,9,9)) l->zona=3;
  else if (wmouse_in(l->x+(l->an+1)*l->columnas,l->y+9,9,(l->al+1)*l->lineas-17)) l->zona=4;
  else l->zona=0;

  if (TipoBrowser!=MAPBR) {
    if (old_zona!=l->zona) if (old_zona>=10) {
      x=l->x+1+((old_zona-10)%l->columnas)*(l->an+1);
      y=l->y+l->al+((old_zona-10)/l->columnas)*(l->al+1);
      p=(byte *)l->lista+l->lista_an*(l->inicial+old_zona-10);
      wwrite(ptr,an,al,x+l->an/2,y,7,p,c3);
      v.volcar=1;
    }
  }

  if (l->zona==2 && (mouse_b&1)) {
    if (old_mouse_b&1) { retrazo(); retrazo(); retrazo(); retrazo(); }
      if (l->inicial) {
        l->inicial-=l->columnas; M3D_paint_listboxbr(l); v.volcar=1; }
      wput(ptr,an,al,l->x+(l->an+1)*l->columnas+1,l->y+1,-41);
      l->botones|=1; v.volcar=1;
  } else if (l->botones&1) {
    wput(ptr,an,al,l->x+(l->an+1)*l->columnas+1,l->y+1,-39);
    l->botones^=1; v.volcar=1;
  }

  if (l->zona==3 && (mouse_b&1)) {
    if (old_mouse_b&1) { retrazo(); retrazo(); retrazo(); retrazo(); }
    n=l->maximo-l->inicial;
    if (n>l->lineas*l->columnas) {
      l->inicial+=l->columnas; M3D_paint_listboxbr(l); v.volcar=1; }
    wput(ptr,an,al,l->x+(l->an+1)*l->columnas+1,l->y+(l->al+1)*l->lineas-7,-42);
    l->botones|=2; v.volcar=1;
  } else if (l->botones&2) {
    wput(ptr,an,al,l->x+(l->an+1)*l->columnas+1,l->y+(l->al+1)*l->lineas-7,-40);
    l->botones^=2; v.volcar=1;
  }

  if (l->zona==4 && (mouse_b&1)) {
    l->slide=wmouse_y-1;
    if (l->slide<l->s0) l->slide=l->s0;
    else if (l->slide>l->s1) l->slide=l->s1;

    if (l->maximo>l->lineas*l->columnas) {
      n=(l->maximo-l->lineas*l->columnas+l->columnas-1)/l->columnas;

      n=0.5+(float)(n*(l->slide-l->s0))/(l->s1-l->s0);

      if (n!=l->inicial/l->columnas) { l->inicial=n*l->columnas; M3D_paint_listboxbr(l); }
    } paint_slider_br(l); v.volcar=1;

  } else {

    if (l->maximo<=l->lineas*l->columnas) n=l->s0;
    else {
      n=(l->maximo-l->lineas*l->columnas+l->columnas-1)/l->columnas;

      n=(l->s0*(n-l->inicial/l->columnas)+l->s1*(l->inicial/l->columnas))/n;
    }
    if (n!=l->slide) { l->slide=n; paint_slider_br(l); v.volcar=1; }
  }

  if (TipoBrowser!=MAPBR) {
    if (old_zona!=l->zona) if (l->zona>=10) {
      x=l->x+1+((l->zona-10)%l->columnas)*(l->an+1);
      y=l->y+l->al+((l->zona-10)/l->columnas)*(l->al+1);
      p=(byte *)l->lista+l->lista_an*(l->inicial+l->zona-10);
      wwrite(ptr,an,al,x+l->an/2,y,7,p,c4);
      v.volcar=1;
    }
  }

  switch (l->zona) {
    case 2: mouse_graf=7; break;
    case 3: mouse_graf=9; break;
    case 4: mouse_graf=13; break;
  }
}

//-----------------------------------------------------------------------------
// M3D_paint_listboxbr - Paint all thumbnails in the listbox
//-----------------------------------------------------------------------------

void M3D_paint_listboxbr(struct t_listboxbr * l)
{
  byte * ptr=v.ptr;
  int an=v.an/big2,al=v.al/big2;
  int n,y,x;

  for(y=0;y<l->lineas;y++)
    for(x=0; x<l->columnas; x++) {
      wbox(ptr,an,al,c1,l->x+(x*(l->an+1))+1,l->y+(y*(l->al+1))+1,l->an,l->al-8);
      wbox(ptr,an,al,c01,l->x+(x*(l->an+1))+1,l->y+(y*(l->al+1))+1+l->al-8,l->an,8);
    }

  if (wmouse_in(l->x,l->y,(l->an+1)*l->columnas,(l->al+1)*l->lineas)) {
    l->zona=((mouse_x-v.x)/big2-l->x)/(l->an+1)+(((mouse_y-v.y)/big2-l->y)/(l->al+1))*l->columnas;
    if (l->zona>=l->maximo-l->inicial || l->zona>=l->lineas*l->columnas) l->zona=1;
    else l->zona+=10;
  } else if (wmouse_in(l->x+(l->an+1)*l->columnas,l->y,9,9)) l->zona=2;
  else if (wmouse_in(l->x+(l->an+1)*l->columnas,l->y+(l->al+1)*l->lineas-8,9,9)) l->zona=3;
  else if (wmouse_in(l->x+(l->an+1)*l->columnas,l->y+9,9,(l->al+1)*l->lineas-17)) l->zona=4;
  else l->zona=0;

  n=l->maximo-l->inicial;
  if (n>l->lineas*l->columnas) n=l->lineas*l->columnas;
  while (n>0) M3D_show_thumb(l,l->inicial+--n);
}

//-----------------------------------------------------------------------------
// MapperBrowseFPG - Brush/texture selection dialog
//-----------------------------------------------------------------------------

extern int m_maximo;
extern struct t_listboxbr lthumbmapbr;

#define BRUSH 4

void MapperBrowseFPG0(void)
{
  v.tipo = 1;
  v.an   = 147-4;
  v.al   = 147-4;
  if (TipoTex>3) {
    if(TipoBrowser==BRUSH) {
      v.titulo = texto[572];
      v.nombre = texto[572];
    } else {
      v.titulo = texto[573];
      v.nombre = texto[573];
    }
  } else {
    v.titulo = texto[433];
    v.nombre = texto[433];
  }

  v.paint_handler=MapperBrowseFPG1;
  v.click_handler=MapperBrowseFPG2;

  if(modo<100) {
    if(TipoBrowser==MAPBR) {
      memcpy(&copia_br, &ltexturasbr, sizeof(ltexturasbr));
      memcpy(&ltexturasbr, &lthumbmapbr, sizeof(ltexturasbr));
      ltexturasbr.maximo=m_maximo;
    } else {
      memcpy(m3d_fpgcodesbr, fondo, max_texturas*an_textura);
      ltexturasbr.maximo=f_maximo;
    }
  } else {
    if(TipoTex==FONDO) {
      memcpy(m3d_fpgcodesbr, fondo, max_texturas*an_textura);
      ltexturasbr.maximo=f_maximo;
    } else {
      memcpy(m3d_fpgcodesbr, textura, max_texturas*an_textura);
      ltexturasbr.maximo=t_maximo;
    }
  }

  num=0;

  ltexturasbr.creada   = 0;
  ltexturasbr.columnas = 4;
  ltexturasbr.an       = 32;
  ltexturasbr.al       = 32;
  ltexturasbr.lineas   = 4;

  v_terminado=0; t_pulsada=1;
}

void MapperBrowseFPG1(void)
{
  _show_items();

  M3D_create_listboxbr(&ltexturasbr);
}

void MapperBrowseFPG2(void) {
  int old_pincel;

  if (!key(_T) && !key(_U)) t_pulsada=0;

  _process_items();

  M3D_update_listboxbr(&ltexturasbr);

  if((mouse_b&1) && !(old_mouse_b&1))
  {
    if(ltexturasbr.zona>=10)
    {
      // Paint mode only (modo<100) - 3D map mode removed
      if(TipoBrowser==BRUSH) {
        old_pincel=num_pincel;
        num_pincel=ltexturasbr.inicial+ltexturasbr.zona-10;
        M3D_show_thumb(&ltexturasbr,old_pincel);
        M3D_show_thumb(&ltexturasbr,num_pincel);
        num_pincel=old_pincel;
      }
      FPG_thumbpos = 0;
      v_terminado  = 1;
      fin_dialogo  = 1;
    }
  }

  if(!t_pulsada) {
    if(TipoBrowser==BRUSH) {
      if(key(_T)) fin_dialogo=1;
    } else if(TipoBrowser==MAPBR) {
      if(key(_U)) fin_dialogo=1;
    }
  }
}

void MapperBrowseFPG3(void) {}
