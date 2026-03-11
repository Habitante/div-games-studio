//-----------------------------------------------------------------------------
//      Module for creating explosions
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"

//-----------------------------------------------------------------------------
//      Definitions
//-----------------------------------------------------------------------------

#define max_pun 64      // Maximum number of control points
#define max_exp 8       // Maximum number of base explosions
#define DEEP    4
// #pragma on (check_stack)  /* Watcom-specific, not needed */

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef byte
#define byte unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef word
#define word unsigned short
#endif

//-----------------------------------------------------------------------------
//      Internal function declarations
//-----------------------------------------------------------------------------

void init_rnd(int,char*); // Initialize the random generator with an int and key
byte rndb(void);	    	  // Get a byte from the random generator
int  rnd(void); 		      // Get an int from the random generator

void gama0(void);
byte exp_colores[128];
struct _gcolor exp_gama[9];

//-----------------------------------------------------------------------------
//      Tables and global variables
//-----------------------------------------------------------------------------

int cx,cy;
int ExpGama;

struct tp {			// Structure for explosion points
  int x,y;                      // Point coordinates
  int radio;                    // Effective radius of the point
  int fuerza;                   // Central force of the point
  int xr,yr;                    // Coord. * 10000
  int ix,iy;                    // Increments
};

struct te {                     // Structure for a base explosion
  struct tp p[max_pun];
  byte * ptr;
} e[max_exp];

//-----------------------------------------------------------------------------
//      Parameters for creating the explosion
//-----------------------------------------------------------------------------

int exp_ancho=32;      // 8..256
int exp_alto=32;       // 8..256
int n_pun;      // 16..64
int n_exp;      // 1..8
int n_frames;   // 1..n
int _n_frames=6;   // 1..n
int paso_frame;
int tipo_exp=1;
int per_points;
char *Buff_exp;

//-----------------------------------------------------------------------------
//	Create the point array
//-----------------------------------------------------------------------------

void create_points(void) {
  int m,n,rx,ry;
  double ang,dist;

  for (m=0;m<n_exp;m++) {

    for (n=0;n<n_pun;n++) {

      ang=(double)rnd()/10000.0; // Angle (?)
      dist=(double)(rnd()%10000)/10000.0; // Distance (0 .. 0.8)

      e[m].p[n].ix=cos(ang)*10000.0;
      e[m].p[n].iy=sin(ang)*10000.0;

      e[m].p[n].x=cx+(int)(cos(ang)*dist*(float)(exp_ancho/2));
      e[m].p[n].y=cy+(int)(sin(ang)*dist*(float)(exp_alto/2));

      e[m].p[n].xr=e[m].p[n].x*10000;
      e[m].p[n].yr=e[m].p[n].y*10000;

      if (e[m].p[n].x<cx) rx=e[m].p[n].x; else rx=exp_ancho-e[m].p[n].x;
      if (e[m].p[n].y<cy) ry=e[m].p[n].y; else ry=exp_alto-e[m].p[n].y;
      if (rx<ry) e[m].p[n].radio=rx*10000; else e[m].p[n].radio=ry*10000;

      e[m].p[n].fuerza=e[m].p[n].radio+rnd()%(e[m].p[n].radio)*4;
    }
  }
}

//-----------------------------------------------------------------------------
//      Advance one frame in the explosion
//-----------------------------------------------------------------------------

void advance_points(void) {
  int m,n;
  for (m=0;m<n_exp;m++) {
        for (n=0;n<n_pun;n++)
        {
                e[m].p[n].xr+=(e[m].p[n].ix*paso_frame)/10000;
                e[m].p[n].yr+=(e[m].p[n].iy*paso_frame)/10000;
                e[m].p[n].x=e[m].p[n].xr/10000;
                e[m].p[n].y=e[m].p[n].yr/10000;
                if (e[m].p[n].fuerza>e[m].p[n].radio)
                        e[m].p[n].fuerza=(int)((float)e[m].p[n].fuerza*(float)0.86);//0.98
                e[m].p[n].radio-=paso_frame;
        }
  }
}

//-----------------------------------------------------------------------------
//	Render the explosion
//-----------------------------------------------------------------------------

int paint_explosion(void) {
  int m,x,y,dx,dy,n;
  int dist,exp_Color,exp_Coloracum;

        Progress((char *)texts[434],0,exp_alto);
        for (y=0;y<exp_alto;y++)
        {
        Progress((char *)texts[434],y,exp_alto);
        poll_keyboard(); if (key(_ESC)) { Progress((char *)texts[434],exp_alto,exp_alto); return(1); }
                for (x=0;x<exp_ancho;x++)
                {
                        exp_Coloracum=0;
                        for (m=0;m<n_exp;m++)
                        {
                                exp_Color=0;
                                for (n=0;n<n_pun;n++)
                                {
                                        dx=abs(x-e[m].p[n].x);
                                        dy=abs(y-e[m].p[n].y);
                                        if ((dx<e[m].p[n].radio/10000) && (dy<e[m].p[n].radio/10000))
                                        {
                                                dist=sqrt(dx*dx+dy*dy);
                                                if (dist<e[m].p[n].radio/10000)
                                                        exp_Color+=(((e[m].p[n].radio/10000)-dist)*(e[m].p[n].fuerza/10000))/(e[m].p[n].radio/10000);
                                        }
                                }
                                if (exp_Color>255)
                                        exp_Color=255;
                                switch(tipo_exp)
                                {
                                        case 0:
                                        case 3:
                                                exp_Coloracum+=exp_Color;
                                                break;
                                        case 1:
                                        case 2:
                                                if(m%2)
                                                        exp_Coloracum-=exp_Color;
                                                else
                                                        exp_Coloracum+=exp_Color;
                                                break;
                                }
                        }
                        switch(tipo_exp)
                        {
                                case 0:
                                case 3:
                                        Buff_exp[y*exp_ancho+x]=exp_Coloracum/n_exp;
                                        break;
                                case 1:
                                case 2:
                                        if(exp_Coloracum>255)
                                                exp_Coloracum=255;
                                        if(exp_Coloracum<0)
                                                exp_Coloracum=0;
                                        Buff_exp[y*exp_ancho+x]=exp_Coloracum;
                                                break;
                                }
                }
        }
        Progress((char *)texts[434],exp_alto,exp_alto);
        for (n=0;n<(exp_ancho*exp_alto)*per_points/100;n++)
        {
                x=(rnd()%(exp_ancho-2))+1;
                y=(rnd()%(exp_alto -2))+1;
                if(Buff_exp[y*exp_ancho+x]>DEEP*2)
                        Buff_exp[y*exp_ancho+x]-=DEEP*2;

                if(Buff_exp[y*exp_ancho+x-1]>DEEP)
                        Buff_exp[y*exp_ancho+x-1]-=DEEP;

                if(Buff_exp[y*exp_ancho+x+1]>DEEP)
                        Buff_exp[y*exp_ancho+x+1]-=DEEP;

                if(Buff_exp[(y-1)*exp_ancho+x]>DEEP)
                        Buff_exp[(y-1)*exp_ancho+x]-=DEEP;
                if(Buff_exp[(y+1)*exp_ancho+x]>DEEP)
                        Buff_exp[(y+1)*exp_ancho+x]-=DEEP;
        }
        return(0);
}

char cFrames[4];
char cexp_ancho[5];
char cexp_alto[5];
char cper_points[5];
int exp_Color0=-1,exp_Color1=-1,exp_Color2=-1;
int TipoA=1,TipoB=0,TipoC=0;
int OldTipoA=1,OldTipoB=0,OldTipoC=0;

void Explode1(void) {
  int an=v.an/big2, al=v.al/big2;
  int x;

  _show_items();

  create_gradient_colors(exp_gama, exp_colores);

  wbox(v.ptr,an,al,c0,2,31,an-4,1);

  wbox(v.ptr,an,al,c0,2,74,an-4,1);

  wwrite(v.ptr,an,al,an-70,11,0,texts[181],c3);
  wrectangle(v.ptr,an,al,c0,an-70,18,66,11);
  for (x=0;x<64;x++) wbox(v.ptr,an,al,exp_colores[x*2],an-69+x,18+1,1,9);
}

void Selcolor0();
extern int SelColorFont;
extern int SelColorOk;

void Explode2(void) {
  int an=v.an/big2;//, al=v.al/big2;
  int need_refresh=0;

  _process_items();
  switch(v.active_item)
  {
    case 0: end_dialog=1; v_accept=1; break;
    case 1: end_dialog=1; v_accept=0; break;
  }
  if(!(TipoA+TipoB+TipoC))
  {
        TipoA=OldTipoA;
        TipoB=OldTipoB;
        TipoC=OldTipoC;
        need_refresh=1;
  }
  if(TipoA!=OldTipoA)
  {
        TipoA=1;
        TipoB=TipoC=0;
        OldTipoB=OldTipoC=0;
        need_refresh=1;
  }
  if(TipoB!=OldTipoB)
  {
        TipoB=1;
        TipoA=TipoC=0;
        OldTipoA=OldTipoC=0;
        need_refresh=1;
  }
  if(TipoC!=OldTipoC)
  {
        TipoC=1;
        TipoA=TipoB=0;
        OldTipoA=OldTipoB=0;
        need_refresh=1;
  }
  OldTipoA=TipoA;
  OldTipoB=TipoB;
  OldTipoC=TipoC;
  if (wmouse_in(an-70,18,66,11) && (mouse_b&1))
  {
    gradient_buf=exp_colores;
    gradient_config=exp_gama;
    show_dialog((voidReturnType)gama0);
    if (v_accept) need_refresh=1;
  }
  if(need_refresh){
        call((voidReturnType )v.paint_handler);
        v.redraw=1;
  }
}

void Explode3(void)
{

        if(!v_accept)
                return;

        _n_frames=n_frames=atoi(cFrames);
        exp_ancho=atoi(cexp_ancho);
        exp_alto =atoi(cexp_alto);
        per_points    =atoi(cper_points);
        if(TipoA)
                tipo_exp=0;
        if(TipoB)
                tipo_exp=1;
        if(TipoC)
                tipo_exp=2;
}

void Explode0(void) {

  v.type=1;

  v.title=texts[300];
  v.an=128;
  v.al=90+5;
  v.paint_handler=(voidReturnType)Explode1;
  v.click_handler=(voidReturnType)Explode2;
  v.close_handler=(voidReturnType)Explode3;

  _button(100,7     ,v.al-14,0);
  _button(101,v.an-8,v.al-14,2);
  TipoA=TipoB=TipoC=OldTipoA=OldTipoB=OldTipoC=0;
  if (exp_Color0==-1) exp_Color0=c4;
  if (exp_Color1==-1) exp_Color1=c2;
  if (exp_Color2==-1) exp_Color2=c0;
  switch(tipo_exp)
  {
        case 0:
                TipoA=OldTipoA=1;
                break;
        case 1:
                TipoB=OldTipoB=1;
                break;
        case 2:
                TipoC=OldTipoC=1;
                break;
  }

   DIV_SPRINTF(cFrames,"%d",n_frames);
   DIV_SPRINTF(cexp_ancho,"%d",exp_ancho);
   DIV_SPRINTF(cexp_alto,"%d",exp_alto);
   DIV_SPRINTF(cper_points,"%d",per_points);

  _get(133,4,11,21,(byte *)cexp_ancho,5,8,2000);
  _get(134,69-36,11,21,(byte *)cexp_alto ,5,8,2000);

  _flag(301,4,16+19,&TipoA);
  _flag(302,4,16+34,&TipoB);
  _flag(303,4,16+48,&TipoC);

  _get(304,69-24+30,16+19,21,(byte *)cFrames,3,1,48);
  _get(305,69-24+30,16+38,21,(byte *)cper_points ,3,0,100);

  v_accept=0;
}

void GenExplodes()
{
int ExpDac[256];
int n=0,nf,y;
int x;
        n_frames=_n_frames;
        create_dac4();

        show_dialog((voidReturnType)Explode0);
        if(!v_accept)
                return;
        for(x=0;x<256;x++) ExpDac[x] = exp_colores[x/2];
  init_rnd(*system_clock,NULL);
  Buff_exp=(char *)malloc(exp_ancho*exp_alto);
  n_pun=32;
  switch(tipo_exp)
  {
        case 0:
                n_exp=4;
                break;
        case 1:
                n_exp=3;
                break;
        case 2:
                n_exp=5;
                break;
        case 3:
                n_exp=1;
                break;
  }

  cx=exp_ancho/2;
  cy=exp_alto/2;
  nf=n_frames;
  create_points();
  n_frames*=2;
  if(exp_ancho<exp_alto)
        paso_frame=(exp_ancho*10000)/n_frames;
  else
        paso_frame=(exp_alto*10000)/n_frames;
  do {
    if (paint_explosion()) break;

    map_width=exp_ancho;
    map_height=exp_alto;

    if (new_map(NULL)) break;

    for(x=0;x<map_width*map_height;x++)
        v.mapa->map[x]=ExpDac[Buff_exp[x]];

    v.mapa->zoom_cx=v.mapa->map_width/2;
    v.mapa->zoom_cy=v.mapa->map_height/2;

    x=(v.mapa->zoom_cx-vga_width/2);
    if (x<0)
        x=0;
    else
        if (x+vga_width>v.mapa->map_width)
          x=v.mapa->map_width-vga_width;
    y=(v.mapa->zoom_cy-vga_height/2);
    if (y<0)
                y=0;
        else
                if(y+vga_height>v.mapa->map_height)
                        y=v.mapa->map_height-vga_height;
    v.mapa->zoom=0;
    v.mapa->zoom_x=x;
    v.mapa->zoom_y=y;

    for (n=0;n<512;n++)
        v.mapa->puntos[n]=-1;
    v.mapa->fpg_code=0;
    call((voidReturnType )v.paint_handler);
    wvolcado(screen_buffer,vga_width,vga_height,v.ptr,v.x,v.y,v.an,v.al,0);

    advance_points();
    n_frames-=2;
  } while (n_frames);

  free(Buff_exp);
}
