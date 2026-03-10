
///////////////////////////////////////////////////////////////////////////////
//      Module that contains the basic (graphics) primitives
///////////////////////////////////////////////////////////////////////////////

#include "global.h"

void draw_help(int); // Drawing support


void draw_selection_box(int _x0,int _y0,int _x1,int _y1);
void draw_selection_mask(void);
void fill_inter(int an,int al);
void blit_interpolated(int an);
void fill_scan(word x,word y);
void fill_draw(void);
void copy_block(byte *d,byte *s,int an,int al);
void xchg_block(byte *d,byte *s,int an,int al);


///////////////////////////////////////////////////////////////////////////////
//      Global module variables
///////////////////////////////////////////////////////////////////////////////

word * m0, * m1;
word an_original;

extern int num_punto;

///////////////////////////////////////////////////////////////////////////////
//      Functions to print text
///////////////////////////////////////////////////////////////////////////////

void writetxt(int x,int y,int centro,byte * ptr) {
  wwrite(copia,vga_width/big2,vga_height,x,y,centro,ptr,text_color);
}

///////////////////////////////////////////////////////////////////////////////
//      Draw a box 
///////////////////////////////////////////////////////////////////////////////

void box(byte c,int x0,int y0,int x1,int y1) {
  wbox(copia,vga_width,vga_height,c,x0,y0,x1-x0+1,y1-y0+1);
}

///////////////////////////////////////////////////////////////////////////////
//      Draw a rectangle
///////////////////////////////////////////////////////////////////////////////

void rectangle(byte c,int x,int y,int an,int al) {
  wrectangle(copia,vga_width,vga_height,c,x,y,an,al);
}

///////////////////////////////////////////////////////////////////////////////
//      Save ( flag = 0 ) or retrieve ( flag = 1) of a virtual box
///////////////////////////////////////////////////////////////////////////////

void save_mouse_bg(byte * p, int x, int y, int n, int flag) {
  byte *q;
  int an,al;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  al=*((word*)(graf[n]+2));
  an=*((word*)graf[n]);

  x-=*((word*)(graf[n]+4));
  y-=*((word*)(graf[n]+6));


	if((x>vga_width) | (y>vga_height))
		return;

  blit_partial(x,y,an,al);

  q=copia+y*vga_width+x;

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_width) resto_x=x+an-vga_width; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_height) resto_y=y+al-vga_height; else resto_y=0;
  long_y=al-salta_y-resto_y;

  p+=an*salta_y+salta_x; q+=vga_width*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (flag) *q=*p; else *p=*q; p++; q++;
    } while (--an);
    q+=vga_width-(an=long_x); p+=resto_x;
  } while (--long_y);
}

///////////////////////////////////////////////////////////////////////////////
//      Put a graphic
///////////////////////////////////////////////////////////////////////////////

void put(int x,int y,int n) {
  wput_in_box(copia,vga_width,vga_width,vga_height,x,y,n);
}

void put_bw(int x,int y,int n) { // Puts a contrasting graphic (mouse edition )
  int al,an;
  byte *p,*q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  p=graf[n]+8;

  al=*((word*)(graf[n]+2));
  an=*((word*)graf[n]);

  x-=*((word*)(graf[n]+4));
  y-=*((word*)(graf[n]+6));

  if (x>=vga_width || y>=vga_height || x+an<=0 || y+al<=0) return;

  q=copia+y*vga_width+x;

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_width) resto_x=x+an-vga_width; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_height) resto_y=y+al-vga_height; else resto_y=0;
  long_y=al-salta_y-resto_y;

  p+=an*salta_y+salta_x; q+=vga_width*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;

  al=(int)c0*3; salta_x=dac[al]+dac[al+1]+dac[al+2];
  al=(int)c4*3; salta_x+=dac[al]+dac[al+1]+dac[al+2]; salta_x/=2;

  do {
    do {
      if (*p) {
        al=(int)(*q)*3;
        if (dac[al]+dac[al+1]+dac[al+2]>salta_x) *q=c0; else *q=c4;
      } p++; q++;
    } while (--an);
    q+=vga_width-(an=long_x); p+=resto_x;
  } while (--long_y);
}

///////////////////////////////////////////////////////////////////////////////
//      Swap two memory blocks ( without requiring a third party)
///////////////////////////////////////////////////////////////////////////////

void memxchg(byte *d, byte *s, int n) {
  int m,x;

  if ((m=n/4)) do {
    x=*(int*)s; *(int*)s=*(int*)d; *(int*)d=x;
    d+=4; s+=4;
  } while (--m);

  if (n&=3) do {
    x=*s; *s=*d; *d=x;
    d++; s++;
  } while (--n);
}

///////////////////////////////////////////////////////////////////////////////
//      MEGARUTINA para reponer el fondo de una caja en edición
///////////////////////////////////////////////////////////////////////////////

static int zoom_region=0;
int zoom_an,zoom_al;
byte *zoom_p,*zoom_q;

void draw_edit_background(int x,int y,int an,int al) {

  int _x0,_y0,_x1,_y1;
  int _zoom_x,_zoom_y;
  int _zoom_win_x,_zoom_win_y,_zoom_win_width,_zoom_win_height;
  int _big=big;

  if (x<0) { an+=x; x=0; }
  if (y<0) { al+=y; y=0; }
  if (x+an>vga_width) an=vga_width-x;
  if (y+al>vga_height) al=vga_height-y;
  if (an<=0 || al<=0) return;

  blit_partial(x,y,an,al);

  // TODO: Improve update_box() for better window background repaint

  if (zoom_win_x || zoom_win_y) if (x<zoom_win_x || y<zoom_win_y || x+an>zoom_win_x+zoom_win_width || y+al>zoom_win_y+zoom_win_height) update_box(x,y,an,al);

  if (_big) { big=0; big2=1; }

  if (zoom_win_y) {
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c2,zoom_win_x-2-x,zoom_win_y-2-y,zoom_win_width+4,1);
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c0,zoom_win_x-1-x,zoom_win_y-1-y,zoom_win_width+2,1);
  }
  if (zoom_win_y+zoom_win_height<vga_height) {
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c2,zoom_win_x-2-x,zoom_win_y+zoom_win_height+1-y,zoom_win_width+4,1);
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c0,zoom_win_x-1-x,zoom_win_y+zoom_win_height-y,zoom_win_width+2,1);
  }

  if (zoom_win_x) {
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c2,zoom_win_x-2-x,zoom_win_y-1-y,1,zoom_win_height+2);
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c0,zoom_win_x-1-x,zoom_win_y-y,1,zoom_win_height);
  }
  if (zoom_win_x+zoom_win_width<vga_width) {
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c2,zoom_win_x+zoom_win_width+1-x,zoom_win_y-1-y,1,zoom_win_height+2);
    wbox_in_box(copia+y*vga_width+x,vga_width,an,al,c0,zoom_win_x+zoom_win_width-x,zoom_win_y-y,1,zoom_win_height);
  }

  if (_big) { big=1; big2=2; }

  // Llama a zoom_map para que actualice la caja correspondiente

  // 1 - Intersección entre zoom y ventana en la copia

  _x0=(x>zoom_win_x)?x:zoom_win_x;
  _y0=(y>zoom_win_y)?y:zoom_win_y;
  _x1=(x+an<zoom_win_x+zoom_win_width)?x+an-1:zoom_win_x+zoom_win_width-1;
  _y1=(y+al<zoom_win_y+zoom_win_height)?y+al-1:zoom_win_y+zoom_win_height-1;

  if (_x1<_x0 || _y1<_y0) return;

  // 2 - Calcular el pimer pixel y ultimo pixel del mapa

  _x0=((_x0-zoom_win_x)>>zoom)+zoom_x;
  _y0=((_y0-zoom_win_y)>>zoom)+zoom_y;
  _x1=((_x1-zoom_win_x)>>zoom)+zoom_x;
  _y1=((_y1-zoom_win_y)>>zoom)+zoom_y;

  zoom_p=map+_y0*map_width+_x0;
  zoom_q=copia+(zoom_win_y+((_y0-zoom_y)<<zoom))*vga_width+zoom_win_x+((_x0-zoom_x)<<zoom);
  zoom_an=_x1-_x0+1;
  zoom_al=_y1-_y0+1;

  _zoom_x=zoom_x; _zoom_y=zoom_y; _zoom_win_x=zoom_win_x; _zoom_win_y=zoom_win_y; _zoom_win_width=zoom_win_width; _zoom_win_height=zoom_win_height;
  zoom_win_x=zoom_win_x+((_x0-zoom_x)<<zoom); zoom_win_y=zoom_win_y+((_y0-zoom_y)<<zoom);
  zoom_x=_x0; zoom_y=_y0;
  zoom_win_width=(_x1-_x0+1)<<zoom; zoom_win_height=(_y1-_y0+1)<<zoom;
  blit_partial(zoom_win_x,zoom_win_y,zoom_win_width,zoom_win_height);
  zoom_region=1; zoom_map(); zoom_region=0;
  zoom_x=_zoom_x; zoom_y=_zoom_y; zoom_win_x=_zoom_win_x; zoom_win_y=_zoom_win_y; zoom_win_width=_zoom_win_width; zoom_win_height=_zoom_win_height;

}

//-----------------------------------------------------------------------------
//      Zoom, blit_screen de map(map_width*map_height) a copia segun zoom,zoom_x/y
//-----------------------------------------------------------------------------

int an,al;

void zoom_map(void) {

  int32_t n,m,c;
  byte *p,*q;
  int _big=big;

  if (!zoom_region) {

    p=map+zoom_y*map_width+zoom_x;
    q=copia;

    if ((map_width<<zoom)<vga_width) { // Copia de este trozo en select_zoom()
      zoom_win_width=map_width<<zoom; zoom_win_x=(vga_width-zoom_win_width)/2; q+=zoom_win_x; an=map_width;
    } else {
      zoom_win_x=0; an=vga_width>>zoom; zoom_win_width=an<<zoom;
    }

    if ((map_height<<zoom)<vga_height) {
      zoom_win_height=map_height<<zoom; zoom_win_y=(vga_height-zoom_win_height)/2; q+=zoom_win_y*vga_width; al=map_height;
    } else {
      zoom_win_y=0; al=vga_height>>zoom; zoom_win_height=al<<zoom;
    }

    if (!zoom_background) {
      if (zoom_win_x || zoom_win_y) {
        update_background();
        full_redraw=1;
      } else {
        if (_big) { big=0; big2=1; }
        if (zoom_win_y+zoom_win_height<vga_height) {
          wbox(copia,vga_width,vga_height,c2,zoom_win_x-2,zoom_win_y+zoom_win_height+1,zoom_win_width+4,1);
          wbox(copia,vga_width,vga_height,c0,zoom_win_x-1,zoom_win_y+zoom_win_height,zoom_win_width+2,1);
          blit_partial(0,zoom_win_y+zoom_win_height,vga_width,2); }
        if (zoom_win_x+zoom_win_width<vga_width) {
          wbox(copia,vga_width,vga_height,c2,zoom_win_x-2,zoom_win_y-2,zoom_win_width+4,1);
          wbox(copia,vga_width,vga_height,c0,zoom_win_x-1,zoom_win_y-1,zoom_win_width+2,1);
          blit_partial(zoom_win_y+zoom_win_width,0,2,vga_height); }
        if (_big) { big=1; big2=2; }
      }
      zoom_background=1;
    }

    if (zoom_win_x || zoom_win_y) blit_partial(zoom_win_x,zoom_win_y,zoom_win_width,zoom_win_height); else full_redraw=1;

  } else {
    p=zoom_p; q=zoom_q; an=zoom_an; al=zoom_al;
  }

  switch (zoom) {
  case 0:
    m=al; do {
      memcpy(q,p,an);
      p+=map_width;
      q+=vga_width;
    } while (--m);
    break;

  case 1:
    m=al; do {
      n=an; do {
        c=*p; c+=c*256;
        *(word*)q=c;
        *(word*)(q+vga_width)=c;
        p++; q+=2;
      } while (--n);
      p+=map_width-an;
      q+=vga_width*2-an*2;
    } while (--m);
    break;

  case 2:
    m=al; do {
      n=an; do {
        c=*p; c+=c*256; c+=c*65536;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q-=vga_width*3-4;
        p++;
      } while (--n);
      p+=map_width-an;
      q+=vga_width*4-an*4;
    } while (--m);
    break;

  case 3:
    m=al; do {
      n=an; do {
        c=*p; c+=c*256; c+=c*65536;
        *(int*)q=c; q+=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q-=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q+=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q-=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q+=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q-=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q+=4;
        *(int*)q=c; q+=vga_width;
        *(int*)q=c; q-=4;
        *(int*)q=c; q-=vga_width*7-8;
        p++;
      } while (--n);
      p+=map_width-an;
      q+=vga_width*8-an*8;
    } while (--m);
    break;
  }

  if (!zoom_region) cclock=(*system_clock)>>1;

  if (sel_status) switch(mode_selection) {
    case -1: for (n=0;n<512;n+=2) if (v.mapa->puntos[n]!=-1) {
      draw_selection_box(v.mapa->puntos[n],v.mapa->puntos[n+1],v.mapa->puntos[n],v.mapa->puntos[n+1]);
      if (num_punto==n/2) {
        draw_selection_box(v.mapa->puntos[n]-1,v.mapa->puntos[n+1]-1,v.mapa->puntos[n]+1,v.mapa->puntos[n+1]+1);
      }
    } break;
    case 0: draw_selection_box(sel_x0,sel_y0,sel_x1,sel_y1); break;
    case 1: draw_selection_mask(); break;
    case 2: draw_selection_mask(); break;
    case 3: draw_selection_mask();
      if (sel_x1>=0) draw_selection_box(sel_x0,sel_y0,sel_x1,sel_y1);
      break;
    case 4: draw_selection_box(sel_x0,sel_y0,sel_x1,sel_y1); break;
    case 5: draw_selection_mask(); break;
  }
}

//-----------------------------------------------------------------------------
//      Sub barra de fill interpolativo
//-----------------------------------------------------------------------------

void interpolation_mode(void) {

  int an,al;

  an=sel_mask_x1-sel_mask_x0+1+4; al=sel_mask_y1-sel_mask_y0+1+4;

  if ((m0=(word*)malloc(an*al*2))!=NULL) {
  if ((m1=(word*)malloc(an*al*2))!=NULL) {

    make_nearest_gradient();

    bar[0]=101+zoom; bar[1]=120; bar[2]=154; bar[3]=0;
    draw_bar(0); put_bar(10,2,118); need_zoom=1;

    memset(m0,0,an*al*2);
    memset(m1,0,an*al*2);

    do {
      draw_help(1295);
      read_mouse(); select_zoom(); test_mouse();

      if (((mouse_b&1) && selected_icon==2)) {
        fill_inter(an,al); need_zoom=1;
      } else if ((mouse_b&1) && selected_icon==1) {
        fill_inter(an,al); need_zoom=1;
        do { read_mouse(); } while (mouse_b&1);
      }

      blit_interpolated(an);

      blit_edit();
    } while (!(mouse_b&2) && !key(_ESC) && draw_mode<100 &&
      !(mouse_b && mouse_in(toolbar_x,toolbar_y+10,toolbar_x+9,toolbar_y+18)));

    blit_interpolated(an);

    if (key(_ESC)||(mouse_b && mouse_in(toolbar_x,toolbar_y+10,toolbar_x+9,toolbar_y+18)))
      { put_bar(2,10,45); flush_bars(0);
        put(mouse_x,mouse_y,mouse_graf); blit_screen(copia); }

    free(m1); free(m0);
  } else free(m0);
  }

  if (m0==NULL || m1==NULL) {
    v_text=(char *)texto[45]; show_dialog(err0);
  }

  draw_mode+=100;
}

//-----------------------------------------------------------------------------
//      Interpolación
//-----------------------------------------------------------------------------

void fill_inter(int an,int al) { // Rellena con medias una zona

  int x,y;
  word *si,*di,n0,n1,n2,n3;
  int n;

  // 1 - Desplaza m0 hacia derecha abajo un pixel

  for (y=al-1;y>0;y--) for (x=an-1;x>0;x--) *(m0+y*an+x)=*(m0+(y-1)*an+x-1);
  for (x=0;x<an;x++) { *(m0+x)=0; } for (y=0;y<al;y++) { *(m0+y*an)=0; }

  // 2 - Refresca en m0 el alambre de puntos que rodean a la zona

  for (y=sel_mask_y0-1;y<=sel_mask_y1+1;y++) if (y>=0 && y<map_height)
    for (x=sel_mask_x0-1;x<=sel_mask_x1+1;x++) if (x>=0 && x<map_width) {
      if (is_near_selection_mask(x,y)) {
        n=*(original+(y-sel_mask_y0+1)*(an_original)+(x-sel_mask_x0+1));
        n=(dac[n*3]+dac[n*3+1]+dac[n*3+2])*256+256;
        *(m0+(y-sel_mask_y0+2)*an+(x-sel_mask_x0+2))=n;
      } else if (!is_selection_mask(x,y)) {
        *(m0+(y-sel_mask_y0+2)*an+(x-sel_mask_x0+2))=0;
      }
    }

  // Interpola de m0 a m1

  si=m0; di=m1;

  for (y=0;y<al-1;y++) {
    for (x=0;x<an-1;x++) {

        if ((n0=*si)) // Pone en *(di) la media de los cuatro *(si+?)
             if ((n1=*(si+1)))
                  if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n0+n1+n2+n3)>>2;
                       else *di=(n0+n1+n2)/3;
                  else if ((n3=*(si+an+1)))
                            *di=(n0+n1+n3)/3;
                       else *di=(n0+n1)>>1;
             else if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n0+n2+n3)/3;
                       else *di=(n0+n2)>>1;
                  else if ((n3=*(si+an+1)))
                            *di=(n0+n3)>>1;
                       else *di=n0;
        else if ((n1=*(si+1)))
                  if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n1+n2+n3)/3;
                       else *di=(n1+n2)>>1;
                  else if ((n3=*(si+an+1)))
                            *di=(n1+n3)>>1;
                       else *di=n1;
             else if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n2+n3)>>1;
                       else *di=n2;
                  else if ((n3=*(si+an+1)))
                            *di=n3;
                       else *di=0;
      si++; di++;
    } si++; di++;
  }

  // Y ahora interpola de m1 a m0

  si=m1; di=m0;

  for (y=0;y<al-1;y++) {
    for (x=0;x<an-1;x++) {

        if ((n0=*si)) // Pone en *(di) la media de los cuatro *(si+?)
             if ((n1=*(si+1)))
                  if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n0+n1+n2+n3)>>2;
                       else *di=(n0+n1+n2)/3;
                  else if ((n3=*(si+an+1)))
                            *di=(n0+n1+n3)/3;
                       else *di=(n0+n1)>>1;
             else if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n0+n2+n3)/3;
                       else *di=(n0+n2)>>1;
                  else if ((n3=*(si+an+1)))
                            *di=(n0+n3)>>1;
                       else *di=n0;
        else if ((n1=*(si+1)))
                  if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n1+n2+n3)/3;
                       else *di=(n1+n2)>>1;
                  else if ((n3=*(si+an+1)))
                            *di=(n1+n3)>>1;
                       else *di=n1;
             else if ((n2=*(si+an)))
                       if ((n3=*(si+an+1)))
                            *di=(n2+n3)>>1;
                       else *di=n2;
                  else if ((n3=*(si+an+1)))
                            *di=n3;
                       else *di=0;

      si++; di++;
    } si++; di++;
  }

}

//-----------------------------------------------------------------------------
//      Selecciona un bloque por un relleno (x,y,fill_dac)
//-----------------------------------------------------------------------------

void fill_select(word x,word y) {

  int _mode_fill;

  _mode_fill=mode_fill; mode_fill=1;
  if ((fss=(word*)malloc(60000))!=NULL) {
    fsp=fss; fsp_max=fss+30000; fill_scan(x,y); free(fss);
  } else {
    v_text=(char *)texto[45]; show_dialog(err0);
  }
  mode_fill=_mode_fill;
}

//-----------------------------------------------------------------------------
//      Relleno de una zona (que contiene colores fill_dac) con un color
//-----------------------------------------------------------------------------

void fill(word x,word y) {

  int a,b,c,d;

  if ((fss=(word*)malloc(60000))!=NULL) {

    fsp=fss; fsp_max=fss+30000;

    memset(selection_mask,0,((map_width*map_height+31)/32)*4);

    if (mode_fill==2) {
      memset(fill_dac,1,256); // Rellena hasta el color seleccionado
      fill_dac[color]=0;
    } else {
      memset(fill_dac,0,256); // Rellena el color de map(x,y)
      fill_dac[*(map+y*map_width+x)]=1;
    }

    sel_mask_x0=map_width; sel_mask_y0=map_height; sel_mask_x1=0; sel_mask_y1=0;

    fill_scan(x,y);

    if (mode_fill==3) {
      if (sel_mask_x0) a=sel_mask_x0-1; else a=sel_mask_x0;
      if (sel_mask_y0) b=sel_mask_y0-1; else b=sel_mask_y0;
      if (sel_mask_x1<map_width-1) c=sel_mask_x1+1; else c=sel_mask_x1;
      if (sel_mask_y1<map_height-1) d=sel_mask_y1+1; else d=sel_mask_y1;
      c=c-a+1; d=d-b+1; an_original=c;
      if ((original=(byte*)save_undo(a,b,c,d))!=NULL) {
        if (!sel_mask_x0) original--;
        if (!sel_mask_y0) original-=an_original;
        interpolation_mode();
      }
    } else {
      if (save_undo(sel_mask_x0,sel_mask_y0,sel_mask_x1-sel_mask_x0+1,sel_mask_y1-sel_mask_y0+1))
        fill_draw();
    }

    free(fss);
  } else {
    v_text=(char *)texto[45]; show_dialog(err0);
  }
}

//-----------------------------------------------------------------------------
//      Algoritmo de fill (por scanes)
//-----------------------------------------------------------------------------

void fill_scan(word x,word y) {
  int x0,x1;

  nuevo_scan:

  x0=x; do x0--; while (x0>=0 && fill_dac[*(map+y*map_width+x0)]); x0++;
  x1=x; do x1++; while (x1<map_width && fill_dac[*(map+y*map_width+x1)]); x1--;

  if (x0<sel_mask_x0) { sel_mask_x0=x0; } if (x1>sel_mask_x1) { sel_mask_x1=x1; }
  if (y<sel_mask_y0) { sel_mask_y0=y; } if (y>sel_mask_y1) { sel_mask_y1=y; }

  for (x=x0;x<=x1;x++) set_selection_mask(x,y);

  if (mode_fill==1) { if (x0>0) x0--; if (x1<map_width-1) x1++; }

  x=x0; sigue_scan_0:

  if (y>0) while (x<=x1) {
    if (!is_selection_mask(x,y-1)) // Si no está pintado
      if (fill_dac[*(map+(y-1)*map_width+x)]) // Si se tiene que pintar
        if (fsp<fsp_max) { // Si se puede pintar
          *fsp=x0; fsp++; *fsp=x1; fsp++;
          *fsp=x+2; fsp++; *fsp=y; fsp++;
          *fsp=0; fsp++; y--;
          goto nuevo_scan;
        }
    x++;
  }

  x=x0; sigue_scan_1:

  if (y<map_height-1) while (x<=x1) {
    if (!is_selection_mask(x,y+1)) // Si no está pintado
      if (fill_dac[*(map+(y+1)*map_width+x)]) // Si se tiene que pintar
        if (fsp<fsp_max) { // Si se puede pintar
          *fsp=x0; fsp++; *fsp=x1; fsp++;
          *fsp=x+2; fsp++; *fsp=y; fsp++;
          *fsp=1; fsp++; y++;
          goto nuevo_scan;
        }
    x++;
  }

  if (fsp>fss) { // Mientras queden scanes sin terminar
    fsp-=2; y=*fsp; fsp--; x=*fsp;
    fsp--; x1=*fsp; fsp--; x0=*fsp;
    if (*(fsp+4)) goto sigue_scan_1; else goto sigue_scan_0;
  }

}

//-----------------------------------------------------------------------------
//      Relleno del fill
//-----------------------------------------------------------------------------

extern byte * textura_color;
byte get_color(int x,int y);

void fill_draw(void) {
  int x,y;

  if (textura_color) {
    for (y=sel_mask_y0;y<=sel_mask_y1;y++)
      for (x=sel_mask_x0;x<=sel_mask_x1;x++)
        if (is_selection_mask(x,y)) *(map+y*map_width+x)=get_color(x,y);
  } else {
    for (y=sel_mask_y0;y<=sel_mask_y1;y++)
      for (x=sel_mask_x0;x<=sel_mask_x1;x++)
        if (is_selection_mask(x,y)) *(map+y*map_width+x)=color;
  }
}

//-----------------------------------------------------------------------------
//      Volcado de la zona interpolada
//-----------------------------------------------------------------------------

void blit_interpolated(int an) { // Vuelca a pantalla la zona interpolada

  int x,y,n;

      for (y=sel_mask_y0;y<=sel_mask_y1;y++)
      for (x=sel_mask_x0;x<=sel_mask_x1;x++)
        if (is_selection_mask(x,y)) {
          n=*(m0+(y-sel_mask_y0+1)*an+(x-sel_mask_x0+1)); if (n>256) n-=256; else n=0;
          *(map+y*map_width+x)=nearest_gradient[n/256]; }
}

//-----------------------------------------------------------------------------
//      Dibuja una selección de mapa de bits (según zoom,zoom_x/y,sel_mask_*)
//-----------------------------------------------------------------------------

void draw_selection_mask(void) {

  int x0,y0,x1,y1; // Intersección de la zona seleccionada y lo visto en zoom
  int x,y,c;
  byte *p,g0=c0,g4=c4;
  word g04,g40;
  int inc32,*_sel_mask;

  x0=zoom_x; if (sel_mask_x0>x0) x0=sel_mask_x0;
  y0=zoom_y; if (sel_mask_y0>y0) y0=sel_mask_y0;
  x1=zoom_x+(zoom_win_width>>zoom)-1; if (sel_mask_x1<x1) x1=sel_mask_x1;
  y1=zoom_y+(zoom_win_height>>zoom)-1; if (sel_mask_y1<y1) y1=sel_mask_y1;

  // NOTE: Old direct-pointer calculation replaced by the per-zoom switch below
  // p=copia+(zoom_win_y+((y0-zoom_y)<<zoom))*vga_width+zoom_win_x+((x0-zoom_x)<<zoom);

  if (x1>=x0 && y1>=y0) switch(zoom) {

    case 0: for (y=y0;y<=y1;y++) {
      inc32=y*map_width+x0; _sel_mask=selection_mask+(inc32>>5); inc32&=31;
      p=copia+(zoom_win_y+y-zoom_y)*vga_width+(zoom_win_x+x0-zoom_x); c=(cclock^y^x0)&1;
      for (x=x0;x<=x1;x++,p++,c^=1) {
        if (*_sel_mask) { if ((*_sel_mask)&(1<<inc32)) {
            if (y==0 || x==0 || y==map_height-1 || x==map_width-1)  {
              if (c) *p=g0; else *p=g4;
            } else if (!is_selection_mask(x,y-1) || !is_selection_mask(x-1,y) || !is_selection_mask(x,y+1) || !is_selection_mask(x+1,y)) {
              if (c) *p=g0; else *p=g4;
            }
        } }
        if (!(inc32=(inc32+1)&31)) _sel_mask++;
      }
    } break;

    case 1: if (cclock&1) { g0=c4; g4=c0; }
    g04=g0+g4*256; g40=g4+g0*256;
    for (y=y0;y<=y1;y++) {
      inc32=y*map_width+x0; _sel_mask=selection_mask+(inc32>>5); inc32&=31;
      p=copia+(zoom_win_y+(y-zoom_y)*2)*vga_width+zoom_win_x+(x0-zoom_x)*2;
      for (x=x0;x<=x1;x++,p+=2) {
        if (*_sel_mask) if ((*_sel_mask)&(1<<inc32)) {
          if (y==0 || (y!=0 && !is_selection_mask(x,y-1))) {
            *(word*)p=g04;
            if (y==map_height-1 || (y<map_height-1 && !is_selection_mask(x,y+1))) {
              *(word*)(p+vga_width)=g40;
            } else {
              if (x==0 || (x!=0 && !is_selection_mask(x-1,y))) {
                *(p+vga_width)=g4;
              }
              if (x==map_width-1 || (x<map_width-1 && !is_selection_mask(x+1,y))) {
                *(p+vga_width+1)=g0;
              }
            }
          } else if (x==0 || (x!=0 && !is_selection_mask(x-1,y))) {
            *p=g0;
            *(p+vga_width)=g4;
            if (x==map_width-1 || (x<map_width-1 && !is_selection_mask(x+1,y))) {
              *(p+1)=g4;
              *(p+vga_width+1)=g0;
            } else if (y==map_height-1 || (y<map_height-1 && !is_selection_mask(x,y+1))) {
              *(p+vga_width+1)=g0;
            }
          } else if (y==map_height-1 || (y<map_height-1 && !is_selection_mask(x,y+1))) {
            *(word*)(p+vga_width)=g40;
            if (x==map_width-1 || (x<map_width-1 && !is_selection_mask(x+1,y))) {
              *(p+1)=g4;
            }
          } else if (x==map_width-1 || (x<map_width-1 && !is_selection_mask(x+1,y))) {
             *(p+1)=g4;
             *(p+vga_width+1)=g0;
          }
        }
        if (!(inc32=(inc32+1)&31)) _sel_mask++;
      }
    } break;

    case 2: if (cclock&1) { g0=c4; g4=c0; }
    for (y=y0;y<=y1;y++) {
      p=copia+(zoom_win_y+(y-zoom_y)*4)*vga_width+zoom_win_x+(x0-zoom_x)*4;
      for (x=x0;x<=x1;x++,p+=4) if (is_selection_mask(x,y)) {
        if (y==0 || (y!=0 && !is_selection_mask(x,y-1))) {
          *p=g0; *(p+1)=g4;
          *(p+2)=g0; *(p+3)=g4; }
        if (x==0 || (x!=0 && !is_selection_mask(x-1,y))) {
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p-=vga_width*3; }
        if (y==map_height-1 || (y<map_height-1 && !is_selection_mask(x,y+1))) {
          p+=vga_width*3;
          *p=g4; p++;
          *p=g0; p++;
          *p=g4; p++;
          *p=g0; p-=vga_width*3+3; }
        if (x==map_width-1 || (x<map_width-1 && !is_selection_mask(x+1,y))) {
          p+=3;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p-=vga_width*3+3; }
      }
    } break;

    case 3: if (cclock&1) { g0=c4; g4=c0; }
    for (y=y0;y<=y1;y++) {
      p=copia+(zoom_win_y+(y-zoom_y)*8)*vga_width+zoom_win_x+(x0-zoom_x)*8;
      for (x=x0;x<=x1;x++,p+=8) if (is_selection_mask(x,y)) {
        if (y==0 || (y!=0 && !is_selection_mask(x,y-1))) {
          *p=g0; *(p+1)=g4;
          *(p+2)=g0; *(p+3)=g4;
          *(p+4)=g0; *(p+5)=g4;
          *(p+6)=g0; *(p+7)=g4; }
        if (x==0 || (x!=0 && !is_selection_mask(x-1,y))) {
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p-=vga_width*7; }
        if (y==map_height-1 || (y<map_height-1 && !is_selection_mask(x,y+1))) {
          p+=vga_width*7;
          *p=g4; p++;
          *p=g0; p++;
          *p=g4; p++;
          *p=g0; p++;
          *p=g4; p++;
          *p=g0; p++;
          *p=g4; p++;
          *p=g0; p-=vga_width*7+7; }
        if (x==map_width-1 || (x<map_width-1 && !is_selection_mask(x+1,y))) {
          p+=7;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p+=vga_width;
          *p=g4; p+=vga_width;
          *p=g0; p-=vga_width*7+7; }
      }
    } break;
  }

}

//-----------------------------------------------------------------------------
//      Dibuja una caja de selección (según zoom,zoom_x/y)
//-----------------------------------------------------------------------------

void draw_selection_box(int _x0,int _y0,int _x1,int _y1) {

  int x0,y0,x1,y1,n;
  byte *p;

  if (_x0>_x1) swap(_x0,_x1); if (_y0>_y1) swap(_y0,_y1);

  x0=zoom_win_x+(_x0<<zoom)-(zoom_x<<zoom);
  y0=zoom_win_y+(_y0<<zoom)-(zoom_y<<zoom);
  x1=zoom_win_x+(_x1<<zoom)-(zoom_x<<zoom)+(1<<zoom)-1;
  y1=zoom_win_y+(_y1<<zoom)-(zoom_y<<zoom)+(1<<zoom)-1;

  if (x1>=zoom_win_x && x0<zoom_win_x+zoom_win_width && y1>=zoom_win_y && y0<zoom_win_y+zoom_win_height) {

    if (y0>=zoom_win_y) {
      if (x0<zoom_win_x) { x0=zoom_win_x; } if (x1>=zoom_win_x+zoom_win_width) { x1=zoom_win_x+zoom_win_width-1; }
      n=x1-x0+1;
      p=copia+vga_width*y0+x0;
      if ((y0^cclock)&1) do {
        if ((uintptr_t)p&1) *p=c0; else *p=c4; p++;
      } while (--n);
      else do {
        if ((uintptr_t)p&1) *p=c4; else *p=c0; p++;
      } while (--n);
    }

    if (y1<zoom_win_y+zoom_win_height) {
      if (x0<zoom_win_x) { x0=zoom_win_x; } if (x1>=zoom_win_x+zoom_win_width) { x1=zoom_win_x+zoom_win_width-1; }
      n=x1-x0+1;
      p=copia+vga_width*y1+x0;
      if ((y1^cclock)&1) do {
        if ((uintptr_t)p&1) *p=c0; else *p=c4; p++;
      } while (--n);
      else do {
        if ((uintptr_t)p&1) *p=c4; else *p=c0; p++;
      } while (--n);
    }

    x0=zoom_win_x+(_x0<<zoom)-(zoom_x<<zoom);
    x1=zoom_win_x+(_x1<<zoom)-(zoom_x<<zoom)+(1<<zoom)-1;

    if (x0>=zoom_win_x) {
      if (y0<zoom_win_y) { y0=zoom_win_y; } if (y1>=zoom_win_y+zoom_win_height) { y1=zoom_win_y+zoom_win_height-1; }
      n=y1-y0+1;
      p=copia+vga_width*y0+x0;
      if ((x0^cclock)&1) do {
        if (y0&1) *p=c0; else *p=c4; p+=vga_width; y0++;
      } while (--n);
      else do {
        if (y0&1) *p=c4; else *p=c0; p+=vga_width; y0++;
      } while (--n);
      y0=zoom_win_y+(_y0<<zoom)-(zoom_y<<zoom);
    }

    if (x1<zoom_win_x+zoom_win_width) {
      if (y0<zoom_win_y) { y0=zoom_win_y; } if (y1>=zoom_win_y+zoom_win_height) { y1=zoom_win_y+zoom_win_height-1; }
      n=y1-y0+1;
      p=copia+vga_width*y0+x1;
      if ((x1^cclock)&1) do {
        if (y0&1) *p=c0; else *p=c4; p+=vga_width; y0++;
      } while (--n);
      else do {
        if (y0&1) *p=c4; else *p=c0; p+=vga_width; y0++;
      } while (--n);
    }
  }
}

//-----------------------------------------------------------------------------
//      Funciones del mapa de bits, poner un pixel
//-----------------------------------------------------------------------------

void set_selection_mask(int x,int y) { // Pone un pixel en el mapa de bits

  x+=y*map_width; // Nº de bit en el buffer
  y=x>>5; // Nº de int en el buffer
  x&=31; // Nº de bit en el int
  *(selection_mask+y)|=1<<x;
}

//-----------------------------------------------------------------------------
//      Funciones del mapa de bits, consultar un pixel
//-----------------------------------------------------------------------------

int is_selection_mask(int x,int y) { // Consulta un pixel en el mapa de bits

  x+=y*map_width; // Nº de bit en el buffer
  y=x>>5; // Nº de int en el buffer
  x&=31; // Nº de bit en el int
  return(*(selection_mask+y)&(1<<x));
}

int is_near_selection_mask(int x,int y) {

  int c;

  if (x) c=is_selection_mask(x-1,y); else c=0;
  if (x<map_width-1) c|=is_selection_mask(x+1,y);
  if (y) c|=is_selection_mask(x,y-1);
  if (y<map_height-1) c|=is_selection_mask(x,y+1);

  return(c&& !is_selection_mask(x,y));
}

//-----------------------------------------------------------------------------
//      UNDO - Guarda la zona ocupada por una acción, antes de realizarla claro
//-----------------------------------------------------------------------------

byte * save_undo(int x, int y, int an, int al) {

  int a,start,end; // Inicio y fin del bloque guardado (en undo[])
  byte *ret=0;

  if (x<0) { an+=x; x=0; }
  if (y<0) { al+=y; y=0; }
  if (x+an>map_width) { an=map_width-x; }
  if (y+al>map_height) { al=map_height-y; }
  if (an<=0 || al<=0) return((byte *)-1);

  // Determina la zona de zoom a refrescar

  if (x<zoom_x) need_zoom_x=zoom_win_x-((zoom_x-x)<<zoom);
  else need_zoom_x=zoom_win_x+((x-zoom_x)<<zoom);
  if (y<zoom_y) need_zoom_y=zoom_win_y+((y-zoom_y)<<zoom);
  else need_zoom_y=zoom_win_y+((y-zoom_y)<<zoom);
  need_zoom_width=an<<zoom; need_zoom_height=al<<zoom;

  if (need_zoom_x+need_zoom_width<=zoom_win_x || need_zoom_y+need_zoom_height<=zoom_win_y ||
      need_zoom_x>=zoom_win_x+zoom_win_width || need_zoom_y>=zoom_win_y+zoom_win_height) {
    need_zoom_width=0; need_zoom_height=0;
  } else {
    if (need_zoom_x<zoom_win_x) { need_zoom_width-=zoom_win_x-need_zoom_x; need_zoom_x=zoom_win_x; }
    if (need_zoom_y<zoom_win_y) { need_zoom_height-=zoom_win_y-need_zoom_y; need_zoom_y=zoom_win_y; }
    if (need_zoom_x+need_zoom_width>zoom_win_x+zoom_win_width) need_zoom_width=zoom_win_x+zoom_win_width-need_zoom_x;
    if (need_zoom_y+need_zoom_height>zoom_win_y+zoom_win_height) need_zoom_height=zoom_win_y+zoom_win_height-need_zoom_y;
    if (!need_zoom) need_zoom=-1;
  }

  if (undo_index) a=undo_index-1; else a=max_undos-1;

  start=0; if (undo_table[a].mode!=-1) start=undo_table[a].end;

  if (start+an*al>undo_memory) start=0;

  // Si una acción ocupa más de undo_memory, entonces no la guardamos.

  if ((end=start+an*al)<=undo_memory) {

    undo_table[undo_index].start=start; undo_table[undo_index].end=end;

    undo_table[undo_index].x=x; undo_table[undo_index].y=y;
    undo_table[undo_index].an=an; undo_table[undo_index].al=al;

    undo_table[undo_index].mode=draw_mode;

    // Nos cargamos los bloques machacados

    for (a=0;a<max_undos;a++)
      if (a!=undo_index && undo_table[a].mode!=-1)
        if ((undo_table[a].start<start && undo_table[a].end>start)||
           (undo_table[a].start>=start && undo_table[a].start<end))
           undo_table[a].mode=-1;

    // Guardamos la zona que ocupará la acción realizada.

    copy_block(undo+start,map+x+y*map_width,an,al);

    undo_table[undo_index].code=current_map_code;

    undo_index=(undo_index+1)%max_undos; undo_table[undo_index].mode=-1; // Se prohibe undo_next()

    ret=(byte *)(undo+start);

  } else {
    draw_edit_background(0,0,vga_width,vga_height);
    flush_bars(1);
    full_redraw=1; blit_screen(copia);
    v_text=(char *)texto[320]; show_dialog(err0); undo_error=1;
  }

  return(ret);
}

//-----------------------------------------------------------------------------
//      UNDO - Restaura la zona ocupada por la última acción.
//-----------------------------------------------------------------------------

int undo_back(void) {
  int x,y,an,al;
  int a;

  if (undo_index) a=undo_index-1; else a=max_undos-1;

  if (undo_table[a].mode!=-1 && undo_table[a].code==current_map_code) {

    undo_index=a; retrace_wait();

    x=undo_table[a].x; y=undo_table[a].y;
    an=undo_table[a].an; al=undo_table[a].al;

    xchg_block(undo+undo_table[a].start,map+x+y*map_width,an,al);

    // Determina la zona de zoom a refrescar

    if (x<zoom_x) need_zoom_x=zoom_win_x-((zoom_x-x)<<zoom);
    else need_zoom_x=zoom_win_x+((x-zoom_x)<<zoom);
    if (y<zoom_y) need_zoom_y=zoom_win_y+((y-zoom_y)<<zoom);
    else need_zoom_y=zoom_win_y+((y-zoom_y)<<zoom);
    need_zoom_width=an<<zoom; need_zoom_height=al<<zoom;

    if (need_zoom_x+need_zoom_width<=zoom_win_x || need_zoom_y+need_zoom_height<=zoom_win_y ||
        need_zoom_x>=zoom_win_x+zoom_win_width || need_zoom_y>=zoom_win_y+zoom_win_height) {
      need_zoom_width=0; need_zoom_height=0;
    } else {
      if (need_zoom_x<zoom_win_x) { need_zoom_width-=zoom_win_x-need_zoom_x; need_zoom_x=zoom_win_x; }
      if (need_zoom_y<zoom_win_y) { need_zoom_height-=zoom_win_y-need_zoom_y; need_zoom_y=zoom_win_y; }
      if (need_zoom_x+need_zoom_width>zoom_win_x+zoom_win_width) need_zoom_width=zoom_win_x+zoom_win_width-need_zoom_x;
      if (need_zoom_y+need_zoom_height>zoom_win_y+zoom_win_height) need_zoom_height=zoom_win_y+zoom_win_height-need_zoom_y;
      if (!need_zoom) need_zoom=-1;
    } return(1);
  } else return(0);
}

//-----------------------------------------------------------------------------
//      UNDO - Rehace la untima acción desecha con undo_back()
//-----------------------------------------------------------------------------

void undo_next(void) {
  int x,y,an,al;

  if (undo_table[undo_index].mode!=-1 && undo_table[undo_index].code==current_map_code) {

    retrace_wait();

    x=undo_table[undo_index].x; y=undo_table[undo_index].y;
    an=undo_table[undo_index].an; al=undo_table[undo_index].al;

    xchg_block(undo+undo_table[undo_index].start,map+x+y*map_width,an,al);

    // Determina la zona de zoom a refrescar

    if (x<zoom_x) need_zoom_x=zoom_win_x-((zoom_x-x)<<zoom);
    else need_zoom_x=zoom_win_x+((x-zoom_x)<<zoom);
    if (y<zoom_y) need_zoom_y=zoom_win_y+((y-zoom_y)<<zoom);
    else need_zoom_y=zoom_win_y+((y-zoom_y)<<zoom);
    need_zoom_width=an<<zoom; need_zoom_height=al<<zoom;

    if (need_zoom_x+need_zoom_width<=zoom_win_x || need_zoom_y+need_zoom_height<=zoom_win_y ||
        need_zoom_x>=zoom_win_x+zoom_win_width || need_zoom_y>=zoom_win_y+zoom_win_height) {
      need_zoom_width=0; need_zoom_height=0;
    } else {
      if (need_zoom_x<zoom_win_x) { need_zoom_width-=zoom_win_x-need_zoom_x; need_zoom_x=zoom_win_x; }
      if (need_zoom_y<zoom_win_y) { need_zoom_height-=zoom_win_y-need_zoom_y; need_zoom_y=zoom_win_y; }
      if (need_zoom_x+need_zoom_width>zoom_win_x+zoom_win_width) need_zoom_width=zoom_win_x+zoom_win_width-need_zoom_x;
      if (need_zoom_y+need_zoom_height>zoom_win_y+zoom_win_height) need_zoom_height=zoom_win_y+zoom_win_height-need_zoom_y;
      if (!need_zoom) need_zoom=-1;
    }

    undo_index=(undo_index+1)%max_undos;
  }
}

//-----------------------------------------------------------------------------
//      Copia un bloque del mapa a memoria
//-----------------------------------------------------------------------------

void copy_block(byte *d,byte *s,int an,int al) {

  do {
    memcpy(d,s,an);
    d+=an; s+=map_width;
  } while (--al);
}

//-----------------------------------------------------------------------------
//      Intercambia un bloque entre el mapa y memoria
//-----------------------------------------------------------------------------

void xchg_block(byte *d,byte *s,int an,int al) {

  do {
    memxchg(d,s,an);
    d+=an; s+=map_width;
  } while (--al);
}

