
//-----------------------------------------------------------------------------
//      Thumbnail browser window handlers
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#include "divsound.h"
#include "ifs.h"


///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////

void create_thumb_MAP(struct t_listboxbr * l);
void create_thumb_PAL(struct t_listboxbr * l);
void create_thumb_FNT(struct t_listboxbr * l);
void create_thumb_IFS(struct t_listboxbr * l);
void create_thumb_PCM(struct t_listboxbr * l);
void show_thumb(struct t_listboxbr * l, int num);




#define incremento_maximo 6553600
int incremento=incremento_maximo;

int opc_img[8]={0,0,1,1,0,1,1,1}; // thumbnails on/off per window type (v_type)
                                  // 2-MAP, 3-PAL, 5-FNT, 6-IFS, 7-PCM
int opc_pru=0; // Sound preview on/off for sounds and music
char input2[32];
byte color_tag;
int num_taggeds;
int ini_tagged;
int song_playing=0;


#define FILE_CHUNK 2048

#ifdef MIXER
Mix_Chunk * smp=NULL;

#endif

//-----------------------------------------------------------------------------
//      Module variables
//-----------------------------------------------------------------------------

/*struct t_listboxbr{
  int x,y;              // Listbox position in the window
  char * list;         // List pointer
  int item_width;       // Characters per item
  int columns;          // Number of columns in browser listbox
  int lines;            // Number of lines
  int an,al;            // Pixel size of each cell

  int first_visible;    // First visible item index (from 0)
  int max_items;        // Total item count (0 n/a)
  int s0,s1,slide;      // Start, end and current position of the slide bar
  int zone;             // Selected zone
  int buttons;          // Pressed button: up(1) or down(2)
  int created;          // Whether list is already created on screen
};*/


extern char archivo[max_archivos*an_archivo];
//[?*max_archivos]
struct t_listboxbr larchivosbr={77,27,archivo,an_archivo,4,4,51,31};

extern char directorio[max_directorios*an_directorio];
struct t_listbox ldirectoriosbr={3,27,directorio,an_directorio,10,65};

#define max_unidades 26
#define an_unidad (4+1)
extern char unidad[max_unidades*an_unidad];
struct t_listbox lunidadesbr={3,121,unidad,an_unidad,4,28};

#define max_ext 26
#define an_ext (5+1)
extern char ext[max_ext*an_ext];
struct t_listbox lextbr={40,121,ext,an_ext,4,28};

#define ancho_br 297
#define alto_br  180

//-----------------------------------------------------------------------------
//  Structure to store map thumbnail reductions
//-----------------------------------------------------------------------------

t_thumb thumb[max_archivos];

//-----------------------------------------------------------------------------
//  WAV sound structure
//-----------------------------------------------------------------------------

struct _WAV_info {
  byte *buffer;
  int length;
} WAV_info;

//-----------------------------------------------------------------------------
//  Prototypes
//-----------------------------------------------------------------------------

void analyze_input(void);
void open_dir_br(void);
void print_path_br(void);
void paint_listbox_br(struct t_listboxbr * l);
void paint_slider_br(struct t_listboxbr * l);
void create_listbox_br(struct t_listboxbr * l);
void update_listbox_br(struct t_listboxbr * l);

void create_test_text(char *,char);

void browser1(void);
void browser2(void);
void browser3(void);

int cargadac_MAP(char *name);
int cargadac_PCX(char *name);
int cargadac_BMP(char *name);
int cargadac_JPG(char *name);
int cargadac_FNT(char *name);
int cargadac_FPG(char *name);
int cargadac_PAL(char *name);

extern int SongType;
extern int SongCode;
extern int last_mod_clean;
void FreeMOD(void);
int  IsWAV(char *FileName);
int  Mem_GetHeapFree(void);

TABLAIFS tifs[256];
FILE *fifs;
void load_letter(uint8_t letra);

//-----------------------------------------------------------------------------
//  Print the current directory or file path
//-----------------------------------------------------------------------------

int wbox_ancho;

void print_path_br(void) {
  int an=v.an/big2,al=v.al/big2;

  if(v_thumb==7)
    wbox_ancho=an-4-12-text_len(texts[419])-12-text_len(texts[567]);
  else if(v_type==16)
    wbox_ancho=an-4-12-text_len(texts[567]);
  else if(v_thumb)
    wbox_ancho=an-4-12-text_len(texts[419]);
  else
    wbox_ancho=an-6;

  wbox(v.ptr,an,al,c12,3,11,wbox_ancho,8);

  strcpy(full,tipo[v_type].path);
  if (tipo[v_type].path[strlen(tipo[v_type].path)-1]!='/') {
    strcat(full,"/"); } strcat(full,file_mask);

  wwrite_in_box(v.ptr,an,wbox_ancho+2,al,5,12,0,(byte *)full,c1);
  wwrite_in_box(v.ptr,an,wbox_ancho+2,al,4,12,0,(byte *)full,c3);

}

//-----------------------------------------------------------------------------
//  Create thumbnails
//-----------------------------------------------------------------------------

int _omx,_omy,omx,omy,oclock=0;
int num;

void create_thumbs(void) {
  if (opc_img[v_thumb]) {
    do {
      switch(v_thumb) // 2-MAP, 3-PAL, 5-FNT, 6-IFS, 7-PCM
      {
        case 2: create_thumb_MAP(&larchivosbr); break;
        case 3: create_thumb_PAL(&larchivosbr); break;
        case 5: create_thumb_FNT(&larchivosbr); break;
        case 6: create_thumb_IFS(&larchivosbr); break;
        case 7: create_thumb_PCM(&larchivosbr); break;
      }
      if (num>-1) {
        if (thumb[num].ptr!=NULL && thumb[num].status==0) {
          show_thumb(&larchivosbr,num); break;
        } else if (thumb[num].ptr==NULL && thumb[num].status==-1) {
          show_thumb(&larchivosbr,num);
          break; // NOTE: Break added to prevent flickering (at cost of some speed)
        } else break;
      } else break;
    } while (1);
  }
}

void create_thumb_MAP(struct t_listboxbr * l){
  int tipomapa,estado=0,n,m;
  int man,mal;
  FILE * f;
  byte * temp, * temp2;
  byte pal[768];
  byte xlat[256];

  float coefredy,coefredx,a,b;
  int x,y;

  num=-1;

  if ((n=abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b*10)) {
    incremento=(float)incremento/((float)n/20.0+1.0);
    incremento/=256; incremento*=256;
    if (incremento<512) incremento=512;
  }
  if (ascii) {
    incremento=512;
  } else if (incremento<incremento_maximo) incremento+=512;

  if (l->total_items) {

    num=l->first_visible;
    do {
      if (thumb[num].ptr==NULL && thumb[num].status>-1) {
        if (mouse_b==0) estado=1;
        break;
      }
      if (thumb[num].ptr!=NULL && thumb[num].status>0) {
        estado=2; break;
      }
      if (++num==l->total_items) num=0;
    } while (num!=l->first_visible);

    if (estado==0) { num=-1; return; }

    if (estado==1) { // Read a new thumbnail
		char filename[255];
		strcpy(filename,l->list+(l->item_width*num));
		strupr(filename);
		
      if (strchr(l->list+(l->item_width*num),'.')!=NULL &&
	  strcmp(strchr(filename,'.'),".MAP") &&
          strcmp(strchr(filename,'.'),".PCX") &&
          strcmp(strchr(filename,'.'),".BMP") &&
          strcmp(strchr(filename,'.'),".JPG") &&
          strcmp(strchr(filename,'.'),".JPE")) {

        estado=0; thumb[num].status=-1;

      } else if ((f=fopen(l->list+(l->item_width*num),"rb"))!=NULL) {
        fseek(f,0,SEEK_END);
        thumb[num].filesize=ftell(f);
        fseek(f,0,SEEK_SET);
        if (thumb[num].filesize<=FILE_CHUNK) incremento=FILE_CHUNK;
        if ((thumb[num].ptr=(char *)malloc(thumb[num].filesize))!=NULL) {
          if (thumb[num].filesize>incremento) {
            if (fread(thumb[num].ptr,1,incremento,f)==incremento) {
              thumb[num].status=incremento;
            } else {
              free(thumb[num].ptr); thumb[num].ptr=NULL; estado=0; thumb[num].status=-1;
            }
          } else {
            if (fread(thumb[num].ptr,1,thumb[num].filesize,f)==thumb[num].filesize) {
              thumb[num].status=thumb[num].filesize;
            } else {
              free(thumb[num].ptr); thumb[num].ptr=NULL; estado=0; thumb[num].status=-1;
            }
          }
        } else { estado=0; thumb[num].status=-1; }
        fclose(f);
      } else { estado=0; thumb[num].status=-1; }
      return;
    } else if (estado==2 && thumb[num].status!=thumb[num].filesize) { // Continue reading a thumbnail

      if ((f=fopen(l->list+(l->item_width*num),"rb"))!=NULL) {
        fseek(f,thumb[num].status,SEEK_SET);
        if (thumb[num].filesize-thumb[num].status>incremento) {
          if (fread(thumb[num].ptr+thumb[num].status,1,incremento,f)==incremento) {
            thumb[num].status+=incremento;
          } else {
            free(thumb[num].ptr); thumb[num].ptr=NULL; estado=0; thumb[num].status=-1;
          }
        } else {
          if (fread(thumb[num].ptr+thumb[num].status,1,thumb[num].filesize-thumb[num].status,f)==thumb[num].filesize-thumb[num].status) {
            thumb[num].status=thumb[num].filesize;
          } else {
            free(thumb[num].ptr); thumb[num].ptr=NULL; estado=0; thumb[num].status=-1;
          }
        }
        fclose(f);
      } else { estado=0; thumb[num].status=-1; }
      return;

    }

    // Now create the thumbnail if the file is fully loaded

    if (estado==2 && thumb[num].status==thumb[num].filesize &&
        abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b+ascii==0) {

      thumb[num].status=0;

      man=map_width; mal=map_height;
      if (is_MAP((byte *)thumb[num].ptr)) tipomapa=1;
      else if (is_PCX((byte *)thumb[num].ptr)) tipomapa=2;
      else if (is_BMP((byte *)thumb[num].ptr)) tipomapa=3;
      else if (is_JPG((byte *)thumb[num].ptr,thumb[num].filesize)) tipomapa=4;
      else tipomapa=0;
      swap(man,map_width); swap(mal,map_height);

      if (tipomapa) {
        if ((temp=(byte*)malloc(man*mal+man))!=NULL) {
		  memset(temp,0,man*mal+man);
          swap(man,map_width); swap(mal,map_height);
          n=1;
          switch (tipomapa) {
            case 1: descomprime_MAP((byte *)thumb[num].ptr,temp,0); break;
            case 2: descomprime_PCX((byte *)thumb[num].ptr,temp,0); break;
            case 3: descomprime_BMP((byte *)thumb[num].ptr,temp,0); break;
            case 4:
              n=descomprime_JPG((byte *)thumb[num].ptr,temp,0,thumb[num].filesize);
            break;
          } swap(man,map_width); swap(mal,map_height);
          free(thumb[num].ptr);
          if(!n) {
            free(temp);
            thumb[num].ptr=NULL;
            thumb[num].status=-1;
            return;
          }
          memcpy(pal,dac4,768);
          create_dac4();
          for (n=0;n<256;n++) xlat[n]=fast_find_color(pal[n*3],pal[n*3+1],pal[n*3+2]);

          if (man<=51*big2 && mal<=31*big2) { // Graphic fits as-is

            thumb[num].an=man; thumb[num].al=mal;
            for (n=thumb[num].an*thumb[num].al-1;n>=0;n--) {
              temp[n]=xlat[temp[n]];
            } thumb[num].ptr=(char *)temp;

          } else { // Create the thumbnail

            coefredx=man/((float)51*2*(float)big2);
            coefredy=mal/((float)31*2*(float)big2);
            if(coefredx>coefredy) coefredy=coefredx; else coefredx=coefredy;
            thumb[num].an=(float)man/coefredx+0.5;
            thumb[num].al=(float)mal/coefredy+0.5;
            thumb[num].an&=-2; thumb[num].al&=-2;
            if(thumb[num].an<2) thumb[num].an=2;
            if(thumb[num].al<2) thumb[num].al=2;

            if (coefredx*(float)(thumb[num].an-1)>=(float)man)
              coefredx=(float)(man-1)/(float)(thumb[num].an-1);
            if (coefredy*(float)(thumb[num].al-1)>=(float)mal)
              coefredy=(float)(mal-1)/(float)(thumb[num].al-1);

            if ((temp2=(byte *)malloc(thumb[num].an*thumb[num].al))!=NULL) {

              memset(temp2,0,thumb[num].an*thumb[num].al);
              a=(float)0.0;
              for(y=0;y<thumb[num].al;y++) {
                b=(float)0.0;
                for(x=0;x<thumb[num].an;x++) {
                  temp2[y*thumb[num].an+x]=temp[((memptrsize)a)*man+(memptrsize)b];
                  b+=coefredx;
                } a+=coefredy;
              }

              // Apply the xlat[] table to the thumbnail

              for (n=thumb[num].an*thumb[num].al-1;n>=0;n--) {
                temp2[n]=xlat[temp2[n]];
              }

              if ((thumb[num].ptr=(char *)malloc((thumb[num].an*thumb[num].al)/4))!=NULL) {
				memset(thumb[num].ptr,0,(thumb[num].an*thumb[num].al)/4)	;
				
                for (y=0;y<thumb[num].al;y+=2) {
                  for (x=0;x<thumb[num].an;x+=2) {
                    n=*(ghost+temp2[x+y*thumb[num].an]*256+temp2[x+1+y*thumb[num].an]);
                    m=*(ghost+temp2[x+(y+1)*thumb[num].an]*256+temp2[x+1+(y+1)*thumb[num].an]);
                    thumb[num].ptr[x/2+(y/2)*(thumb[num].an/2)]=*(ghost+n*256+m);
                  }
                }

                thumb[num].an/=2; thumb[num].al/=2;

              } else {
                thumb[num].ptr=NULL;
                thumb[num].status=-1;
              } free(temp2);

            } else {
              thumb[num].ptr=NULL;
              thumb[num].status=-1;
            } free(temp);
          }

        } else {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          thumb[num].status=-1;
        }
      } else {
        free(thumb[num].ptr);
        thumb[num].ptr=NULL;
        thumb[num].status=-1;
      }

    }

  }

  if (oclock<*system_clock) {
    _omx=omx; _omy=omy;
    omx=mouse_x; omy=mouse_y;
    oclock=*system_clock;
  }

}

void create_thumb_PAL(struct t_listboxbr * l)
{
  int estado=0,n,tipo;
  byte pal[768];
  byte xlat[256];
  int x,y;
  char filename[255];
  num=-1;

  if ((n=abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b*10)) {
    incremento=0;
  } else if (ascii) {
    incremento=0;
  } else if (incremento<incremento_maximo) incremento+=128;

  if (l->total_items && incremento>=FILE_CHUNK)
  {
    num=l->first_visible;
    do
    {
      if (thumb[num].ptr==NULL && thumb[num].status>-1)
      {
        estado=1;
        break;
      }
      if (++num==l->total_items) num=0;
    } while (num!=l->first_visible);

    if (estado==0) { num=-1; return; }
	strcpy(filename,l->list+(l->item_width*num));
	strupr(filename);
    // Start reading a new thumbnail
    if (!strcmp(strchr(filename,'.'),".FPG"))
      tipo=1;
    else if(!strcmp(strchr(filename,'.'),".FNT"))
      tipo=2;
    else if(!strcmp(strchr(filename,'.'),".PCX"))
      tipo=3;
    else if(!strcmp(strchr(filename,'.'),".BMP"))
      tipo=4;
    else if(!strcmp(strchr(filename,'.'),".MAP"))
      tipo=5;
    else if(!strcmp(strchr(filename,'.'),".PAL"))
      tipo=6;
    else if(!strcmp(strchr(filename,'.'),".JPG"))
      tipo=7;
    else if(!strcmp(strchr(filename,'.'),".JPE"))
      tipo=7;
    else
    {
      estado=0;
      thumb[num].status=-1;
      return;
    }
    switch(tipo)
    {
      case 1: tipo=cargadac_FPG(l->list+l->item_width*num); break;
      case 2: tipo=cargadac_FNT(l->list+l->item_width*num); break;
      case 3: tipo=cargadac_PCX(l->list+l->item_width*num); break;
      case 4: tipo=cargadac_BMP(l->list+l->item_width*num); break;
      case 5: tipo=cargadac_MAP(l->list+l->item_width*num); break;
      case 6: tipo=cargadac_PAL(l->list+l->item_width*num); break;
      case 7: tipo=cargadac_JPG(l->list+l->item_width*num); break;
    }
    if(!tipo)
    {
      estado=0;
      thumb[num].status=-1;
      return;
    }

    // Copy palette to pal, restore dac4 and create xlat lookup from pal
    memcpy(pal,dac4,768);
    create_dac4();
    for (n=0;n<256;n++) xlat[n]=fast_find_color(pal[n*3],pal[n*3+1],pal[n*3+2]);

    // Create the palette thumbnail
    thumb[num].an = 32*big2;
    thumb[num].al = 16*big2;
    if ((thumb[num].ptr=(char *)malloc((thumb[num].an*thumb[num].al)))==NULL)
    {
      estado=0;
      thumb[num].status=-1;
      return;
    }
    memset(thumb[num].ptr,0,(thumb[num].an*thumb[num].al));
    
    for (y=0; y<thumb[num].al; y+=big2)
    {
      for (x=0; x<thumb[num].an; x+=big2*2)
      {
        if(big)
        {
          thumb[num].ptr[y*thumb[num].an+x]       = xlat[y*8+x/4];
          thumb[num].ptr[y*thumb[num].an+x+1]     = xlat[y*8+x/4];
          thumb[num].ptr[(y+1)*thumb[num].an+x]   = xlat[y*8+x/4];
          thumb[num].ptr[(y+1)*thumb[num].an+x+1] = xlat[y*8+x/4];
          thumb[num].ptr[y*thumb[num].an+x+2]     = xlat[y*8+x/4];
          thumb[num].ptr[y*thumb[num].an+x+3]     = xlat[y*8+x/4];
          thumb[num].ptr[(y+1)*thumb[num].an+x+2] = xlat[y*8+x/4];
          thumb[num].ptr[(y+1)*thumb[num].an+x+3] = xlat[y*8+x/4];
        }
        else
        {
          thumb[num].ptr[y*thumb[num].an+x]   = xlat[y*16+x/2];
          thumb[num].ptr[y*thumb[num].an+x+1] = xlat[y*16+x/2];
        }
      }
    }
    thumb[num].status = 0;
  }

  if (oclock<*system_clock) {
    _omx=omx; _omy=omy;
    omx=mouse_x; omy=mouse_y;
    oclock=*system_clock;
  }
}

extern int TamaY,TamaX;
extern int spacelen;
extern char TestString2[21];
extern char MiTabladeLetras[256];

void create_thumb_FNT(struct t_listboxbr * l)
{
  int estado=0, n, m, init, x, y, len;
  int fan, _fal=0, fal, cnt;
  byte * temp, * temp2;
  byte pal[768];
  byte xlat[256];
  char CopiaMiTabladeLetras[256];
  float coefredy, coefredx, a, b;
  FILE *f;
  char filename[255];
  num=-1;

  if ((n=abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b*10))
  {
    incremento=(float)incremento/((float)n/20.0+1.0);
    incremento/=256; incremento*=256;
    if (incremento<512) incremento=512;
  }
  if (ascii)
  {
    incremento=512;
  } else if (incremento<incremento_maximo) incremento+=512;

  if (l->total_items)
  {
    num=l->first_visible;
    do
    {
      if (thumb[num].ptr==NULL && thumb[num].status>-1)
      {
        if (mouse_b==0) estado=1;
        break;
      }
      if (thumb[num].ptr!=NULL && thumb[num].status>0)
      {
        estado=2;
        break;
      }
      if (++num==l->total_items) num=0;
    } while (num!=l->first_visible);

    if (estado==0) { num=-1; return; }

    // read a new thumbnail
    if (estado==1)
    {
		strcpy(filename,l->list+(l->item_width*num));
		strupr(filename);
		printf("%s %s\n",filename,l->list+(l->item_width*num));
		fflush(stdout);
      if(!strcmp(strchr(filename,'.'),".FNT"));
      else
      {
        estado=0;
        thumb[num].status=-1;
        return;
      }
      if((f=fopen(l->list+(l->item_width*num),"rb"))==NULL)
      {
	    estado=0;
        thumb[num].status=-1;
        return;
      }
      fseek(f,0,SEEK_END);
      thumb[num].filesize=ftell(f);
      fseek(f,0,SEEK_SET);
      if (thumb[num].filesize<=FILE_CHUNK) incremento=FILE_CHUNK;
      if ((thumb[num].ptr=(char *)malloc(thumb[num].filesize))==NULL)
      {
        fclose(f);
        estado=0;
        thumb[num].status=-1;
        return;
      }
      memset(thumb[num].ptr,0,thumb[num].filesize);
      
      if (thumb[num].filesize > incremento)
      {
        if (fread(thumb[num].ptr,1,incremento,f)!=incremento)
        {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          fclose(f);
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status=incremento;
      }
      else
      {
        if (fread(thumb[num].ptr,1,thumb[num].filesize,f)!=thumb[num].filesize)
        {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          fclose(f);
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status=thumb[num].filesize;
      }
      fclose(f);
    }

    // Continue reading a thumbnail
    else if (estado==2 && thumb[num].status!=thumb[num].filesize)
    {
      if ((f=fopen(l->list+(l->item_width*num),"rb"))==NULL)
      {
        estado=0;
        thumb[num].status=-1;
      }
      fseek(f,thumb[num].status,SEEK_SET);
      if (thumb[num].filesize-thumb[num].status>incremento)
      {
        if (fread(thumb[num].ptr+thumb[num].status,1,incremento,f)!=incremento)
        {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          fclose(f);
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status+=incremento;
      }
      else
      {
        if (fread(thumb[num].ptr+thumb[num].status,1,thumb[num].filesize-thumb[num].status,f)!=thumb[num].filesize-thumb[num].status)
        {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          fclose(f);
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status=thumb[num].filesize;
      }
      fclose(f);
      return;
    }

    // Now create the thumbnail if the file is fully loaded
    if (estado==2 && thumb[num].status==thumb[num].filesize &&
        abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b+ascii==0)
    {
      thumb[num].status=0;

      memcpy(pal, &thumb[num].ptr[8], 768);

      memcpy(CopiaMiTabladeLetras,MiTabladeLetras,256);

      create_test_text(TestString2,thumb[num].ptr[1352]);

      memset(MiTabladeLetras,0,256);
      for(x=0;x<strlen(TestString2);x++) MiTabladeLetras[TestString2[x]]=1;

      TamaX=0; TamaY=0;

      spacelen=0; cnt=0;
      for(x=0;x<255;x+=2)
      {
        GetCharSizeBuffer(x,&fan,&fal,thumb[num].ptr);
        if(fan>1) { cnt++; spacelen+=fan; }
        if(_fal<fal) _fal=fal;
      }
      if (cnt) spacelen=(spacelen/cnt)/2; else spacelen=1;

      for(x=0;x<strlen(TestString2);x++)
      {
        GetCharSizeBuffer(TestString2[x],&fan,&fal,thumb[num].ptr);
        if(fan<=1) fan=0;
        TamaX+=fan;
        if(TamaY<fal) TamaY=fal+1;
      }
      if (TamaX==0) TamaX=1;
      if (TamaY==0) TamaY=_fal;

      memcpy(MiTabladeLetras,CopiaMiTabladeLetras,256);

      if((temp=(byte *)malloc(TamaX*TamaY))==NULL)
      {
        free(thumb[num].ptr);
        thumb[num].ptr=NULL;
        estado=0;
        thumb[num].status=-1;
        return;
      }
      memset(temp,0,TamaX*TamaY);

      init=0;
      for(x=0;x<strlen(TestString2);x++)
      {
        len=ShowCharBuffer(TestString2[x],init,0,(char *)temp,TamaX,thumb[num].ptr);
        if (len<=1) len=0;
        init+=len;
      }
      free(thumb[num].ptr);

      xlat[0]=c1;
      create_dac4();
      for (n=1;n<256;n++) xlat[n]=fast_find_color(pal[n*3],pal[n*3+1],pal[n*3+2]);
      for (n=TamaX*TamaY-1;n>=0;n--) temp[n]=xlat[temp[n]];

      if (TamaX>101*big2 || TamaY>22*big2)
      {
        // Create the thumbnail
        coefredx = TamaX/((float)101*2*(float)big2);
        coefredy = TamaY/((float)22*2*(float)big2);
        if(coefredx>coefredy) coefredy=coefredx;
        else                  coefredx=coefredy;
        thumb[num].an = (float)TamaX/coefredx+0.5;
        thumb[num].al = (float)TamaY/coefredy+0.5;
        thumb[num].an &= -2; thumb[num].al &= -2;
        if(thumb[num].an<2) thumb[num].an=2;
        if(thumb[num].al<2) thumb[num].al=2;

        if (coefredx*(float)(thumb[num].an-1)>=(float)TamaX)
          coefredx=(float)(TamaX-1)/(float)(thumb[num].an-1);
        if (coefredy*(float)(thumb[num].al-1)>=(float)TamaY)
          coefredy=(float)(TamaY-1)/(float)(thumb[num].al-1);

        if ((temp2=(byte *)malloc(thumb[num].an*thumb[num].al))!=NULL)
        {
		  memset(temp2,0,thumb[num].an*thumb[num].al);
			
          a=(float)0.0;
          for(y=0;y<thumb[num].al;y++)
          {
            b=(float)0.0;
            for(x=0;x<thumb[num].an;x++)
            {
              temp2[y*thumb[num].an+x] = temp[((memptrsize)a)*TamaX+(memptrsize)b];
              b+=coefredx;
            }
            a+=coefredy;
          }

          if ((thumb[num].ptr=(char *)malloc((thumb[num].an*thumb[num].al)/4))!=NULL)
          {
			memset(thumb[num].ptr,0,(thumb[num].an*thumb[num].al)/4);
			
            for (y=0;y<thumb[num].al;y+=2)
            {
              for (x=0;x<thumb[num].an;x+=2)
              {
                n=*(ghost+temp2[x+y*thumb[num].an]*256+temp2[x+1+y*thumb[num].an]);
                m=*(ghost+temp2[x+(y+1)*thumb[num].an]*256+temp2[x+1+(y+1)*thumb[num].an]);
                thumb[num].ptr[x/2+(y/2)*(thumb[num].an/2)]=*(ghost+n*256+m);
              }
            }
            thumb[num].an/=2; thumb[num].al/=2;
          }
          else
          {
            thumb[num].status=-1;
          }
          free(temp2);
        }
        else
        {
          thumb[num].ptr=NULL;
          thumb[num].status=-1;
        }
        free(temp);
      }
      else
      {
        thumb[num].ptr=(char *)temp;
        thumb[num].an=TamaX;
        thumb[num].al=TamaY;
      }
    }
  }

  if (oclock<*system_clock)
  {
    _omx=omx;
    _omy=omy;
    omx=mouse_x;
    omy=mouse_y;
    oclock=*system_clock;
  }
}

void create_thumb_IFS(struct t_listboxbr * l)
{
  int estado=0,n,pos,ancho,alto,x,y,xini;
  char *str;
  char filename[255];
  
  num=-1;

  if (l->total_items)
  {
    num=l->first_visible;
    do
    {
      if (thumb[num].ptr==NULL && thumb[num].status>-1)
      {
        estado=1;
        break;
      }
      if (++num==l->total_items) num=0;
    } while (num!=l->first_visible);

    if (estado==0) { num=-1; return; }
	strcpy(filename,l->list+(l->item_width*num));
	strupr(filename);
	
    if(strcmp(strchr(filename,'.'),".IFS"))
    {
      thumb[num].status=-1;
      return;
    }

    // Start reading a new thumbnail
    if ((fifs=fopen(l->list+(l->item_width*num),"rb"))==NULL)
    {
      thumb[num].status=-1;
      return;
    }
    fseek(fifs,4,SEEK_SET);
    fread(&n,1,4,fifs);
    fseek(fifs,n,SEEK_SET);
    fread(tifs,sizeof(TABLAIFS),256,fifs);

    str=(char *)texts[246];

    pos=0; ancho=0; alto=0;

    while (pos<strlen(str)) {
      load_letter(str[pos++]);
      if (map_width) {
        ancho+=map_width+1;
        if (map_height>alto) alto=map_height;
      }
      if (map!=NULL) { free(map); map=NULL; }
    }

    if(ancho>101*big2) ancho=101*big2;
    if ((thumb[num].ptr=(char*)malloc(ancho*alto))==NULL) {
      thumb[num].status=-1;
      fclose(fifs);
      return;
    }
	memset(thumb[num].ptr,0,ancho*alto);
	
    thumb[num].an=ancho;
    thumb[num].al=alto;

    memset(thumb[num].ptr,c1,ancho*alto);

    pos=0; xini=0;
    while(pos<strlen(str)) {
      load_letter(str[pos++]);
      if (map_width) {
        for (y=0;y<map_height;y++) {
          for (x=0;x<map_width;x++) {
            if((xini+x)<101*big2) thumb[num].ptr[x+xini+y*ancho]=map[x+y*map_width];
          }
        }
        xini+=map_width+1;
      }
      if (map!=NULL) { free(map); map=NULL; }
    }

    fclose(fifs);
    thumb[num].status=0;
  }
}

typedef struct _HeadDC{
  unsigned int   dwUnknow;
  unsigned short wFormatTag;
  unsigned short wChannels;
  unsigned int   dwSamplePerSec;
  unsigned int   dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBits;
} HeadDC;

typedef struct _meminfo{
        unsigned Bloque_mas_grande_disponible;
        unsigned Maximo_de_paginas_desbloqueadas;
        unsigned Pagina_bloqueable_mas_grande;
        unsigned Espacio_de_direccionamiento_lineal;
        unsigned Numero_de_paginas_libres_disponibles;
        unsigned Numero_de_paginas_fisicas_libres;
        unsigned Total_de_paginas_fisicas;
        unsigned Espacio_de_direccionamiento_lineal_libre;
        unsigned Tamano_del_fichero_de_paginas;
        unsigned reservado[3];
}meminfo;

void GetFreeMem(meminfo *Meminfo);

void create_thumb_PCM(struct t_listboxbr * l)
{
  int estado=0,x,y,y0,y1,p0,p1,n;
  FILE *f;
  char *temp, *WAV;
  float step,position=0;
  int First=1,lx,ly,pos;
  meminfo Mi_meminfo;
  int mem;

  int    Exit=0;
  HeadDC MyHeadDC;
  int    length;
  char   *BufferIn;
  char   *BufferOut;
  short  *BuffAux;

  char filename[255];
  
  num=-1;

  if ((n=abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b*10))
  {
    incremento=(float)incremento/((float)n/20.0+1.0);
    incremento/=256; incremento*=256;
    if (incremento<512) incremento=512;
  }
  if (ascii)
  {
    incremento=512;
  } else if (incremento<incremento_maximo) incremento+=512;

  if (l->total_items)
  {
    num=l->first_visible;
    do
    {
      if (thumb[num].ptr==NULL && thumb[num].status>-1)
      {
        if (mouse_b==0) estado=1;
        break;
      }
      if (thumb[num].ptr!=NULL && thumb[num].status>0)
      {
        estado=2;
        break;
      }
      if (++num==l->total_items) num=0;
    } while (num!=l->first_visible);

    if (estado==0) { num=-1; return; }

    // Start reading a new thumbnail
    if (estado==1)
    {
#ifdef NOTYET
      _heapshrink();
      GetFreeMem(&Mi_meminfo);
      if((mem=Mem_GetHeapFree())==-1)
      {
        estado=0;
        thumb[num].status=-1;
        return;
      }
#endif
      mem=(Mi_meminfo.Bloque_mas_grande_disponible+mem-1000000)/2;
      if((f=fopen(l->list+(l->item_width*num),"rb"))==NULL)
      {
        estado=0;
        thumb[num].status=-1;
        return;
      }
      fseek(f,0,SEEK_END);
      thumb[num].filesize=ftell(f);
      mem=thumb[num].filesize*2;
      fseek(f,0,SEEK_SET);
      if(thumb[num].filesize>mem)
      {
        fclose(f);
        estado=0;
        thumb[num].status=-1;
        return;
      }
      strcpy(filename,l->list+(l->item_width*num));
      strupr(filename);
      
      if (IsWAV(l->list+(l->item_width*num)));
      else if(!strcmp(strchr(filename,'.'),".PCM") );
      else
      {
        fclose(f);
        estado=0;
        thumb[num].status=-1;
        return;
      }
      if (thumb[num].filesize<=FILE_CHUNK) incremento=FILE_CHUNK;
      if ((thumb[num].ptr=(char *)malloc(thumb[num].filesize))==NULL)
      {
        fclose(f);
        estado=0;
        thumb[num].status=-1;
        return;
      }
      memset(thumb[num].ptr,0,thumb[num].filesize);
      
      if (thumb[num].filesize > incremento)
      {
        if (fread(thumb[num].ptr,1,incremento,f)!=incremento)
        {
          fclose(f);
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status=incremento;
      }
      else
      {
        if (fread(thumb[num].ptr,1,thumb[num].filesize,f)!=thumb[num].filesize)
        {
          fclose(f);
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status=thumb[num].filesize;
      }
      fclose(f);
    }

    // Continue reading a thumbnail
    else if (estado==2 && thumb[num].status!=thumb[num].filesize)
    {
      if ((f=fopen(l->list+(l->item_width*num),"rb"))==NULL)
      {
        estado=0;
        thumb[num].status=-1;
      }
      fseek(f,thumb[num].status,SEEK_SET);
      if (thumb[num].filesize-thumb[num].status>incremento)
      {
        if (fread(thumb[num].ptr+thumb[num].status,1,incremento,f)!=incremento)
        {
          fclose(f);
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status+=incremento;
      }
      else
      {
        if (fread(thumb[num].ptr+thumb[num].status,1,thumb[num].filesize-thumb[num].status,f)!=thumb[num].filesize-thumb[num].status)
        {
          fclose(f);
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          estado=0;
          thumb[num].status=-1;
          return;
        }
        thumb[num].status=thumb[num].filesize;
      }
      fclose(f);
      return;
    }

    // Now create the thumbnail if the file is fully loaded
    if (estado==2 && thumb[num].status==thumb[num].filesize &&
        abs(_omx-mouse_x)+abs(_omy-mouse_y)+mouse_b+ascii==0)
    {
      thumb[num].status=0;

// Start of conversion

      if(IsWAV(l->list+(l->item_width*num)))
      {
        WAV    = thumb[num].ptr;
        length = thumb[num].filesize;
        pos    = 0;

        while(!Exit && pos<length)
        {
          if(WAV[pos++]=='W')
            if(WAV[pos++]=='A')
              if(WAV[pos++]=='V')
                if(WAV[pos++]=='E')
                  if(WAV[pos++]=='f')
                    if(WAV[pos++]=='m')
                      if(WAV[pos++]=='t')
                        Exit=1;
        }
        if(Exit==0)
        {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          thumb[num].status=-1;
          return;
        }
        pos++;
        memcpy(&MyHeadDC, &WAV[pos], sizeof(HeadDC));
        pos+=sizeof(HeadDC);

        Exit=0;
        while(!Exit && pos<length)
        {
          if(WAV[pos++]=='d')
            if(WAV[pos++]=='a')
              if(WAV[pos++]=='t')
                if(WAV[pos++]=='a')
                  Exit=1;
        }
        length=*(int *)(WAV+pos);
        pos+=4;
        if((BufferIn=(char *)malloc(length))==NULL)
        {
          free(thumb[num].ptr);
          thumb[num].ptr=NULL;
          thumb[num].status=-1;
          return;
        }
        memcpy(BufferIn, &WAV[pos], length);
        free(thumb[num].ptr);
        thumb[num].ptr=NULL;

        // Step 1: convert 16-bit to 8-bit
        if((MyHeadDC.dwAvgBytesPerSec/MyHeadDC.dwSamplePerSec)/MyHeadDC.wChannels==2)
        {
                length/=2;
                BuffAux=(short *)BufferIn;
                if((BufferOut=(char *)malloc(length))==NULL)
                {
                  free(BufferIn);
                  thumb[num].status=-1;
                  return;
                }
                //printf("16 to 8 ...\n");
                for(x=0;x<length;x++) {
                        BufferOut[x]=0x80+(BuffAux[x]>>8);
                }
                free(BufferIn);
                BufferIn=BufferOut;
        }

        // Step 2: convert stereo to mono
        if(MyHeadDC.wChannels==2)
        {
                length/=2;
                if((BufferOut=(char *)malloc(length))==NULL)
                {
                  free(BufferIn);
                  thumb[num].status=-1;
                  return;
                }
                //printf("ST a MONO ...\n");
                for(x=0;x<length;x++) {
                        BufferOut[x]=BufferIn[x*2];
                }
                free(BufferIn);
                BufferIn=BufferOut;
        }

        thumb[num].ptr      = BufferIn;
        thumb[num].filesize = length;
      }

// End of conversion

      thumb[num].an = 51*big2;
      thumb[num].al = 24*big2;
      if ((temp=(char*)malloc(thumb[num].filesize))==NULL)
      {
        free(thumb[num].ptr);
        thumb[num].ptr=NULL;
        thumb[num].status=-1;
        return;
      }
      memcpy(temp, thumb[num].ptr, thumb[num].filesize);
      free(thumb[num].ptr);

      if ((thumb[num].ptr=(char*)malloc(thumb[num].an*thumb[num].al))==NULL)
      {
        free(temp);
        thumb[num].status=-1;
        return;
      }
      memset(thumb[num].ptr,c1,thumb[num].an*thumb[num].al);

      if (thumb[num].filesize>1) {
      if (thumb[num].filesize<3*thumb[num].an)
      {
        step = (float)thumb[num].an/(float)(thumb[num].filesize-1);
        for(x=0; x<thumb[num].filesize; x++)
        {
          y = (temp[x]*thumb[num].al/256);
          if(First)
          {
            First = 0;
            lx    = (memptrsize)position;
            ly    = y;
          }
          else
          {
            wline(thumb[num].ptr, thumb[num].an, thumb[num].an, thumb[num].al, lx, ly, (memptrsize)position, y, c_g_low);
            lx = (memptrsize)position;
            ly = y;
          }
          position += step;
        }
      }
      else
      {
        step=(float)thumb[num].filesize/(float)thumb[num].an;
        //printf("Drawing ...\n");
        for(x=0;x<thumb[num].an;x++)
        {

          p0=(memptrsize)position;
          position+=step;
          p1=(memptrsize)position;

          y0=y1=temp[p0]*thumb[num].al/256;

          do
          {
            y=temp[p0]*thumb[num].al/256;
            if (y<y0) y0=y; else if (y>y1) y1=y;
            p0+=2;
          } while (p0<p1);

          y=y0;
          do
          {
            thumb[num].ptr[x+y*thumb[num].an]=c_g_low;
          } while (y++<y1);
        }
      }
      } free(temp);
    }
  }

  if (oclock<*system_clock)
  {
    _omx=omx;
    _omy=omy;
    omx=mouse_x;
    omy=mouse_y;
    oclock=*system_clock;
  }
}

void load_letter(uint8_t letra) {
  long offset;
  short x,y,j,t;
  uint8_t rtbyte, error=0;
  short pixels,despY;

  map_width=map_height=0;
  offset=tifs[letra].desp;
  if (fseek(fifs,offset,SEEK_SET)) error=1;
  if (fread(&map_height,2,1,fifs)<1) error=1;
  if (fread(&despY,2,1,fifs)<1) error=1;
  if (fread(&map_width,2,1,fifs)<1) error=1;
  if (fread(&pixels,2,1,fifs)<1) error=1;
  if (error) { map_width=0; return; }

  t=map_width*8-pixels; map_height+=despY;

  if (!map_width || !map_height) { map_width=0; return; }

  map=(byte*)malloc(map_height*map_width*8);
  if (map==NULL) { map_width=0; return; }
  memset(map,c1,map_height*map_width*8);

  for (y=0;y<map_height-despY;y++)
    for (x=0;x<map_width;x++) {
      if (fread(&rtbyte,1,1,fifs)<1) { free(map); map=NULL; map_width=0; return; }
      for (j=0; j<8; j++)
        if (rtbyte & (1<<j)) map[((y+despY)*pixels+x*8+j)-t]=c3;
    }

  map_width=pixels;
}

//-----------------------------------------------------------------------------
//  Display a thumbnail in the window
//-----------------------------------------------------------------------------

void show_thumb(struct t_listboxbr * l, int num) {
  byte * ptr=v.ptr;
  int an=v.an/big2,al=v.al/big2;
  int px,py,x,y,ly,incy;
  char *p;

  if (num>=l->first_visible && num<l->first_visible+l->lines*l->columns) {

    px=(l->x+1+(l->an+1)*((num-l->first_visible)%l->columns))*big2+(l->an*big2-thumb[num].an)/2;
    if ((incy=((l->al-8)*big2-thumb[num].al)/2)<0) incy=0;
    py=(l->y+1+(l->al+1)*((num-l->first_visible)/l->columns))*big2+incy;

    ly=(l->y+(l->al+1)*((num-l->first_visible)/l->columns)+l->al-8)*big2;

    if (thumb[num].ptr!=NULL && thumb[num].status==0) {
      for(y=0;y<thumb[num].al;y++)
        for(x=0;x<thumb[num].an;x++)
          if (py+y>ly)
          {
            if(!thumb[num].tagged)
              v.ptr[(py+y)*v.an+(px+x)]=*(ghost+c0*256+thumb[num].ptr[y*thumb[num].an+x]);
          }
          else v.ptr[(py+y)*v.an+(px+x)]=thumb[num].ptr[y*thumb[num].an+x];
    }

    px-=(l->an*big2-thumb[num].an)/2;
    py-=incy;
    px/=big2; py/=big2;

    if (thumb[num].ptr==NULL && thumb[num].status==-1) {
      wput(ptr,an,al,px+(l->an-21)/2,py+1,60);
    }

    py+=l->al-1;
    p=l->list+l->item_width*num;

    if (l->zone-10==num-l->first_visible) x=c4; else x=c3;
    if (text_len((byte *)p)<l->an-2 && opc_img[v_thumb]) {
      wwrite(ptr,an,al,px+l->an/2+1,py,7,(byte *)p,c0);
      wwrite(ptr,an,al,px+l->an/2,py,7,(byte *)p,x);
    } else {
      wwrite_in_box(ptr,an,px+l->an-1,al,px+2,py,6,(byte *)p,c0);
      wwrite_in_box(ptr,an,px+l->an-1,al,px+1,py,6,(byte *)p,x);
    }

    v.redraw=1;
  }
}

//-----------------------------------------------------------------------------
//  Browser main code
//-----------------------------------------------------------------------------

void browser0(void) {
  unsigned n,m,x;

  v.type=1; // Dialog
  v.title=(byte *)v_text;
  v_thumb=v_type;

  num_taggeds = 0;
  ini_tagged  = 0;
  input[0]    = 0;

  // v_thumb: 0-n/a, 2-MAP, 3-PAL, 5-FNT, 6-IFS, 7-PCM

  if (v_thumb==9) v_thumb=2;
  if (v_thumb==10) v_thumb=3;

  if (v_thumb<2 || v_thumb==4 || v_thumb>7) {
    v_thumb=0;
  }

  if(v_thumb==7 || v_type==16)
  {
    FreeMOD();
    SongCode++;
    last_mod_clean=0;
  }

  if (v_thumb==0) {
    v.an= ancho_br-1;
    v.al=alto_br-1;
    lextbr.y = 119;
    lunidadesbr.y = 119;
  } else {
    v.an=ancho_br;
    v.al=alto_br;
    lextbr.y = 121;
    lunidadesbr.y = 121;
  }

  for (n=0;n<max_archivos;n++) {
    thumb[n].ptr=NULL;
  }

  v.paint_handler=browser1;
  v.click_handler=browser2;
  v.close_handler=browser3;

  lextbr.total_items=0; n=0; // Create the list of extensions

  if (v_type==2 && v_mode>0) v_type=14;
  if (v_type==7 && v_mode>0) v_type=11;

  while ((x=tipo[v_type].ext[n++])) {
    m=0; while (x && x!=' ') {
      ext[an_ext*lextbr.total_items+m++]=x;
      if ((x=tipo[v_type].ext[n])) n++;
    } ext[an_ext*(lextbr.total_items++)+m]=0;
  }

  if (v_type==14) v_type=2;
  if (v_type==11) v_type=7;

  if (tipo[v_type].default_choice>=0 && tipo[v_type].default_choice<lextbr.total_items)
    strcpy(input,&ext[an_ext*tipo[v_type].default_choice]);
  else strcpy(input,&ext[0]);

  DIV_STRCPY(file_mask,input);
  v_finished=0;

  _get(126,4,v.al-21,v.an-(24+text_len(texts[100])+text_len(texts[101])),(byte *)input,512,0,0);

  _button(100,v.an-12-text_len(texts[101]),v.al-14,2);
  _button(101,v.an-8,v.al-14,2);

  if (v_thumb) _flag(419,v.an-12-text_len(texts[419]),12,&opc_img[v_thumb]);

  _dos_setdrive(toupper(*tipo[v_type].path)-'A'+1,&n);
  chdir(tipo[v_type].path);

  open_dir_br(); // Create file and directory lists

  if (v_mode==1) *input=0;
  if (v_mode==2) strcpy(input,input2);

  larchivosbr.created=0; ldirectoriosbr.created=0;
  lunidadesbr.created=0; lextbr.created=0;

  n=0; while (drives[n]) { // Create the drive list <X:>
    *(unidad+an_unidad*n)='<'; *(unidad+an_unidad*n+1)=drives[n];
    *(unidad+an_unidad*n+2)=':'; *(unidad+an_unidad*n+3)='>';
    *(unidad+an_unidad*n+4)=0; n++;
  } lunidadesbr.total_items=n;

  if(v_thumb==7)
    _flag(567,v.an-12-text_len(texts[419])-12-text_len(texts[567]),12,&opc_pru);
  else if(v_type==16)
    _flag(567,v.an-12-text_len(texts[567]),12,&opc_pru);

  if (opc_img[v_thumb])
  {
    switch(v_thumb) // 2-MAP, 3-PAL, 5-FNT, 6-IFS, 7-PCM
    {
      case 5:
      case 6:
        larchivosbr.columns=2;
        larchivosbr.an=103;
      break;
      default:
        larchivosbr.columns=4;
        larchivosbr.an=51;
      break;
    }
    larchivosbr.lines=4;
    larchivosbr.al=31;
  }
  else
  {
    larchivosbr.columns=3;
    larchivosbr.an=68;
    larchivosbr.lines=14;
    larchivosbr.al=8;
  }

  if(v_thumb==7 && !SoundActive) opc_pru=0;

}

void browser1(void) {
  int an=v.an/big2,al=v.al/big2;

  _show_items(); print_path_br();

  wwrite(v.ptr,an,al,77, 20,0,texts[127],c3); // Files
  wwrite(v.ptr,an,al, 3, 20,0,texts[128],c3);

  if(v_thumb) {
    wwrite(v.ptr,an,al, 3,113,0,texts[129],c3);
    wwrite(v.ptr,an,al,40,113,0,texts[130],c3);
  } else {
    wwrite(v.ptr,an,al, 3,111,0,texts[129],c3);
    wwrite(v.ptr,an,al,40,111,0,texts[130],c3);
  }

  create_listbox_br(&larchivosbr);
  create_listbox(&ldirectoriosbr);
  create_listbox(&lunidadesbr);
  create_listbox(&lextbr);
}

int bload=0;
int ns,chn;

// For loading PCM in the browser
#ifdef MIXER
Mix_Chunk *DIVMIX_LoadPCM(char *path);
#endif

void browser2(void) {
  unsigned n, selected;
  unsigned pos1, pos2;
  byte *ptr;

#ifdef MIXER
  Mix_Chunk *SI = NULL;
#endif

  int need_refresh=0;
  int estado;

  create_thumbs();

  estado=v.item[0].state;
  _process_items();

  if (v.active_item==3)
  {
    if (opc_img[v_thumb])
    {
      switch(v_thumb)
      {
        case 5:
        case 6:
          larchivosbr.columns=2;
          larchivosbr.an=103;
        break;
        default:
          larchivosbr.columns=4;
          larchivosbr.an=51;
        break;
      }
      larchivosbr.lines=4;
      larchivosbr.al=31;
      larchivosbr.first_visible=0;
      larchivosbr.slide=larchivosbr.s0;
      larchivosbr.s1=larchivosbr.y+(larchivosbr.al*larchivosbr.lines+larchivosbr.lines+1)-12;
      browser1(); v.redraw=1;
    }
    else
    {
      larchivosbr.columns=3;
      larchivosbr.an=68;
      larchivosbr.lines=14;
      larchivosbr.al=8;
      larchivosbr.first_visible=0;
      larchivosbr.slide=larchivosbr.s0;
      larchivosbr.s1=larchivosbr.y+(larchivosbr.al*larchivosbr.lines+larchivosbr.lines+1)-12;
      for (n=0;n<max_archivos;n++) {
        if (thumb[n].ptr!=NULL) {
          free(thumb[n].ptr); thumb[n].ptr=NULL;
        }
        thumb[n].status=0;
      }
      browser1(); v.redraw=1;
    }
  }

  update_listbox_br(&larchivosbr);
  update_listbox(&ldirectoriosbr);
  update_listbox(&lunidadesbr);
  update_listbox(&lextbr);

  if (v.item[0].state>=2 && num_taggeds>0) {
    for(num=0; num<larchivosbr.total_items; num++) thumb[num].tagged=0;
    num_taggeds=0;
    need_refresh=1;
  }

  switch(v.active_item) {
    case 1:
      if(num_taggeds)
      {
        v_finished=1; v_exists=1;
        return;
      }
      analyze_input();
    break;
    case 2: end_dialog=1; break;
  }

  if ((mouse_b&1) && !(prev_mouse_buttons&1))
  {
    if (larchivosbr.zone>=10)
    {
      if(v_type!=6 && !v_mode)
      {
        if(shift_status&3) // SHIFT
        {
          for(num=0; num<larchivosbr.total_items; num++) thumb[num].tagged=0;
          num_taggeds=0;
          selected=larchivosbr.zone-10+larchivosbr.first_visible;
          if(selected>ini_tagged) pos1=ini_tagged, pos2=selected;
          else                    pos2=ini_tagged, pos1=selected;
          for(n=pos1; n<=pos2; n++)
          {
            thumb[n].tagged=1;
            num_taggeds++;
          }
        }
        else
        if(shift_status&4) // CTRL
        {
          selected=larchivosbr.zone-10+larchivosbr.first_visible;
          ini_tagged = selected;
          if(thumb[selected].tagged)
          {
            thumb[selected].tagged=0;
            num_taggeds--;
          }
          else
          {
            thumb[selected].tagged=1;
            num_taggeds++;
          }
        }
        else
        {
          for(num=0; num<larchivosbr.total_items; num++) thumb[num].tagged=0;
          selected=larchivosbr.zone-10+larchivosbr.first_visible;
          thumb[selected].tagged = 1;
          num_taggeds            = 1;
          ini_tagged             = selected;
        }
      }
      else
      {
        for(num=0; num<larchivosbr.total_items; num++) thumb[num].tagged=0;
        selected=larchivosbr.zone-10+larchivosbr.first_visible;
        thumb[selected].tagged = 1;
        num_taggeds            = 1;
        ini_tagged             = selected;
      }

      browser1(); v.redraw=1;
      strcpy(full,archivo+(larchivosbr.zone-10+larchivosbr.first_visible)*an_archivo);

      // TODO: Handle CTRL and SHIFT modifier keys
      if (strcmp(input,full) || ((v_thumb==7 || v_type==16) && opc_pru))
      {
        strcpy(input, full);
        browser1();
        v.redraw=1;
#ifdef MIXER

        if(v_thumb==7 && opc_pru) {
#ifdef NOTYET
          if ( judascfg_device == DEV_NOSOUND) {
            if ( SoundError ) {
              v_text=texts[549]; show_dialog(err0);
            } else {
              v_text=texts[548]; show_dialog(err0);
            } return;
          } else 
#else
			if(true) 
#endif
			{
            strcpy(full,tipo[v_type].path);
            if (tipo[v_type].path[strlen(tipo[v_type].path)-1]!='/')
              strcat(full,"/");
            strcat(full,archivo+(larchivosbr.zone-10+larchivosbr.first_visible)*an_archivo);

			Mix_HaltChannel(-1);
			if ( smp !=NULL)
				Mix_FreeChunk(smp);
			
			smp=NULL;
			
            smp = Mix_LoadWAV(full);
            if(smp==NULL) 
            {
				// try loading PCM
                smp = DIVMIX_LoadPCM(full);
              
            }
            if ( smp == NULL ) {
				debugprintf("failed to load %s\n",full);
				
			     }
            else
            {
			Mix_PlayChannel(0,smp,0);
              while (mouse_b&1) read_mouse();
            }
          }
        } else if(v_type==16 && opc_pru) {
#ifdef NOTYET
          strcpy(full,tipo[v_type].path);
          if (tipo[v_type].path[strlen(tipo[v_type].path)-1]!='/')
            strcat(full,"/");
          strcat(full,archivo+(larchivosbr.zone-10+larchivosbr.first_visible)*an_archivo);

          if(judas_channel[0].smp) judas_stopsample(0);
          if(smp!=NULL) { judas_freesample(smp); smp=NULL; }

          FreeMOD();

          judas_loadxm(full);
          if(judas_error == JUDAS_OK) { judas_playxm(1); SongType=XM; }
          else if(judas_error == JUDAS_WRONG_FORMAT)
          {
            judas_loads3m(full);
            if(judas_error == JUDAS_OK) { judas_plays3m(1); SongType=S3M; }
            else if(judas_error == JUDAS_WRONG_FORMAT)
            {
              judas_loadmod(full);
              if(judas_error == JUDAS_OK) { judas_playmod(1); SongType=MOD; }
            }
          }
          while (mouse_b&1) read_mouse();
#endif
        }
      } else {
        if(num_taggeds==1) v_exists=1, v_finished=1;
#endif
      }

    } else if (ldirectoriosbr.zone>=10) { v.redraw=1;
      if (tipo[v_type].path[strlen(tipo[v_type].path)-1]!='/')
        strcat(tipo[v_type].path,"/");
      strcat(tipo[v_type].path,directorio+(ldirectoriosbr.zone-10+
        ldirectoriosbr.first_visible)*an_directorio);
      chdir(tipo[v_type].path);
      getcwd(tipo[v_type].path,PATH_MAX+1);
      print_path_br();
      larchivosbr.created=0;
      ldirectoriosbr.created=0;
      tipo[v_type].first_visible=0;
      open_dir_br();

      create_listbox_br(&larchivosbr);
      create_listbox(&ldirectoriosbr);
    } else if (lunidadesbr.zone>=10) {
      _dos_setdrive(drives[lunidadesbr.zone-10+lunidadesbr.first_visible]-'A'+1,&n);
      getcwd(tipo[v_type].path,PATH_MAX+1);
      if (tipo[v_type].path[0]==drives[lunidadesbr.zone-10+lunidadesbr.first_visible]) {
        print_path_br(); v.redraw=1;
        larchivosbr.created=0;
        ldirectoriosbr.created=0;
        tipo[v_type].first_visible=0;
        open_dir_br();

        create_listbox_br(&larchivosbr);
        create_listbox(&ldirectoriosbr);
      } else {
        _dos_setdrive(tipo[v_type].path[0]-'A'+1,&n);
        v_text=(char *)texts[42]; show_dialog(err0); return;
      }
    } else if (lextbr.zone>=10) { v.redraw=1;
      tipo[v_type].default_choice=lextbr.zone-10+lextbr.first_visible;
      strcpy(input,ext+(lextbr.zone-10+lextbr.first_visible)*an_ext);
      DIV_STRCPY(file_mask,input);
      tipo[v_type].first_visible=0;
      print_path_br();
      larchivosbr.created=0;
      open_dir_br();

      browser1();
      v.redraw=1;
    }
  }

  if (v_finished) end_dialog=1;

  if(need_refresh)
  {
    call((voidReturnType )v.paint_handler);
    v.redraw=1;
  }
}

void browser3(void) {
  unsigned n;

  if(bload==1) {
    bload=0;
  }

  _dos_setdrive(toupper(*tipo[1].path)-'A'+1,&n); chdir(tipo[1].path);
  tipo[v_type].first_visible=larchivosbr.first_visible;

  for (n=0;n<max_archivos;n++) if (thumb[n].ptr!=NULL) {
    free(thumb[n].ptr);
  }

  if(song_playing)
  {
    song_playing=0;
  }

  if(v_type==16) FreeMOD();
#ifdef NOTYET
  if(v_thumb==7) {
    if(judas_channel[0].smp) judas_stopsample(0);
    if(smp!=NULL) { judas_freesample(smp); smp=NULL; }
  }
#endif
}

//-----------------------------------------------------------------------------
//  Read directories and files
//-----------------------------------------------------------------------------

void open_dir_br(void) {
  unsigned n,m;
  struct find_t fileinfo;

  n=0; m=_dos_findfirst(file_mask,_A_NORMAL,&fileinfo);
  while (m==0 && n<max_archivos) {
    strcpy(archivo+n++*an_archivo,fileinfo.name);
    m=_dos_findnext(&fileinfo);
  } larchivosbr.total_items=n;
  qsort(archivo,larchivosbr.total_items,(size_t)an_archivo,(int (*)(const void *, const void *))strcmp);

  n=0; m=_dos_findfirst("*.*",_A_SUBDIR,&fileinfo);
  while (m==0 && n<max_directorios) {
    if (strcmp(fileinfo.name,".") && (fileinfo.attrib&16)) {
      strcpy(directorio+n++*an_directorio,fileinfo.name);
   }
    m=_dos_findnext(&fileinfo);
  } ldirectoriosbr.total_items=n;
  qsort(directorio,ldirectoriosbr.total_items,an_directorio,(int (*)(const void *, const void *))strcmp);

  for (n=0;n<max_archivos;n++) {
    if (thumb[n].ptr!=NULL) {
      free(thumb[n].ptr); thumb[n].ptr=NULL;
    }
    thumb[n].status=0;
    thumb[n].tagged=0;
  }
  num_taggeds = 0;
  ini_tagged  = 0;
}

//-----------------------------------------------------------------------------
//  Create the browser-style listbox window
//-----------------------------------------------------------------------------

void paint_listbox_br(struct t_listboxbr * l) {
  byte * ptr=v.ptr;
  int an=v.an/big2,al=v.al/big2;
  int n,y,x;

  color_tag = c_b_low;

  for(y=0;y<l->lines;y++)
    for(x=0; x<l->columns; x++) {
      wbox(ptr,an,al,c1,l->x+(x*(l->an+1))+1,l->y+(y*(l->al+1))+1,l->an,l->al-8);
      if(thumb[l->first_visible+y*l->columns+x].tagged)
        wbox(ptr,an,al,color_tag,l->x+(x*(l->an+1))+1,l->y+(y*(l->al+1))+1+l->al-8,l->an,8);
      else
        wbox(ptr,an,al,c01,l->x+(x*(l->an+1))+1,l->y+(y*(l->al+1))+1+l->al-8,l->an,8);
    }

  if (wmouse_in(l->x,l->y,(l->an+1)*l->columns,(l->al+1)*l->lines)) { // Calculate zone
    l->zone=((mouse_x-v.x)/big2-l->x)/(l->an+1)+(((mouse_y-v.y)/big2-l->y)/(l->al+1))*l->columns;
    if (l->zone>=l->total_items-l->first_visible || l->zone>=l->lines*l->columns) l->zone=1;
    else l->zone+=10;
  } else if (wmouse_in(l->x+(l->an+1)*l->columns,l->y,9,9)) l->zone=2;
  else if (wmouse_in(l->x+(l->an+1)*l->columns,l->y+(l->al+1)*l->lines-8,9,9)) l->zone=3;
  else if (wmouse_in(l->x+(l->an+1)*l->columns,l->y+9,9,(l->al+1)*l->lines-17)) l->zone=4;
  else l->zone=0;

  n=l->total_items-l->first_visible;
  if (n>l->lines*l->columns) n=l->lines*l->columns;
  while (n>0) show_thumb(l,l->first_visible+--n);

}

void paint_slider_br(struct t_listboxbr * l) {

  byte * ptr=v.ptr;
  int an=v.an,al=v.al;
  if (big) { an/=2; al/=2; }

  wbox(ptr,an,al,c2,l->x+(l->an+1)*l->columns+1,l->y+9,7,(l->al+1)*l->lines-17);
  if (l->slide>l->s0) wbox(ptr,an,al,c0,l->x+(l->an+1)*l->columns+1,l->slide-1,7,1);
  if (l->slide<l->s1) wbox(ptr,an,al,c0,l->x+(l->an+1)*l->columns+1,l->slide+3,7,1);
  wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->slide,43);

}

void create_listbox_br(struct t_listboxbr * l) {

  byte * ptr=v.ptr;
  int an=v.an/big2,al=v.al/big2;
  int x,y;

  if (!l->created) {
    l->slide=l->s0=l->y+9;
    l->s1=l->y+(l->al*l->lines+l->lines+1)-12;
    l->buttons=0;
    l->created=1;
    l->zone=0;
    if (l==&larchivosbr) {
      l->first_visible=tipo[v_type].first_visible;
      if ((l->first_visible+(l->lines-1)*l->columns)>=l->total_items) {
        l->first_visible=0;
      }
    } else l->first_visible=0;
  }

  wbox(ptr,an,al,c1,l->x,l->y,(l->an+1)*l->columns,(l->al+1)*l->lines);

  for (y=0;y<l->lines;y++)
    for (x=0;x<l->columns;x++)
      wrectangle(ptr,an,al,c0,l->x+(x*(l->an+1)),l->y+(y*(l->al+1)),l->an+2,l->al+2);

  wrectangle(ptr,an,al,c0,l->x+(l->an+1)*l->columns,l->y,9,(l->al+1)*l->lines+1);
  wrectangle(ptr,an,al,c0,l->x+(l->an+1)*l->columns,l->y+8,9,(l->al+1)*l->lines-15);
  wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->y+1,-39);
  wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->y+(l->al+1)*l->lines-7,-40);

  paint_listbox_br(l);
  paint_slider_br(l);

}

//-----------------------------------------------------------------------------
//  Update the browser listbox
//-----------------------------------------------------------------------------

void update_listbox_br(struct t_listboxbr * l) {
  byte * ptr=v.ptr, *p;
  int an=v.an/big2,al=v.al/big2;
  int n,old_zona=l->zone,x,y;

  if (wmouse_in(l->x,l->y,(l->an+1)*l->columns,(l->al+1)*l->lines)) { // Calculate zone
    l->zone=(wmouse_x-l->x)/(l->an+1)+((wmouse_y-l->y)/(l->al+1))*l->columns;
    if (l->zone>=l->total_items-l->first_visible || l->zone>=l->lines*l->columns) l->zone=1;
    else l->zone+=10;
  } else if (wmouse_in(l->x+(l->an+1)*l->columns,l->y,9,9)) l->zone=2;
  else if (wmouse_in(l->x+(l->an+1)*l->columns,l->y+(l->al+1)*l->lines-8,9,9)) l->zone=3;
  else if (wmouse_in(l->x+(l->an+1)*l->columns,l->y+9,9,(l->al+1)*l->lines-17)) l->zone=4;
  else l->zone=0;

  if (old_zona!=l->zone) if (old_zona>=10) { // Unhighlight zone
    x=l->x+1+((old_zona-10)%l->columns)*(l->an+1);
    y=l->y+l->al+((old_zona-10)/l->columns)*(l->al+1);
    p=(byte *)l->list+l->item_width*(l->first_visible+old_zona-10);
    if (text_len(p)<l->an-2 && opc_img[v_thumb]) {
      wwrite(ptr,an,al,x+l->an/2,y,7,p,c3);
    } else {
      wwrite_in_box(ptr,an,x+l->an-1,al,x+1,y,6,p,c3);
    } v.redraw=1;
  }

  if ((l->zone>0 && mouse_b&8) || (l->zone==2 && (mouse_b&1))) {
    if (prev_mouse_buttons&1) { retrace_wait(); retrace_wait(); retrace_wait(); retrace_wait(); }
      if (l->first_visible) {
        l->first_visible-=l->columns; paint_listbox_br(l); v.redraw=1; }
      wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->y+1,-41);
      l->buttons|=1; v.redraw=1;
  } else if (l->buttons&1) {
    wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->y+1,-39);
    l->buttons^=1; v.redraw=1;
  }

  if ((l->zone>0 && mouse_b&4) || (l->zone==3 && (mouse_b&1))) {
    if (prev_mouse_buttons&1) { retrace_wait(); retrace_wait(); retrace_wait(); retrace_wait(); }
    n=l->total_items-l->first_visible;
    if (n>l->lines*l->columns) {
      l->first_visible+=l->columns; paint_listbox_br(l); v.redraw=1; }
    wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->y+(l->al+1)*l->lines-7,-42);
    l->buttons|=2; v.redraw=1;
  } else if (l->buttons&2) {
    wput(ptr,an,al,l->x+(l->an+1)*l->columns+1,l->y+(l->al+1)*l->lines-7,-40);
    l->buttons^=2; v.redraw=1;
  }

  if (l->zone==4 && (mouse_b&1)) {
    l->slide=wmouse_y-1;
    if (l->slide<l->s0) l->slide=l->s0;
    else if (l->slide>l->s1) l->slide=l->s1;

    if (l->total_items>l->lines*l->columns) {
      n=(l->total_items-l->lines*l->columns+l->columns-1)/l->columns;

      n=0.5+(float)(n*(l->slide-l->s0))/(l->s1-l->s0);

      if (n!=l->first_visible/l->columns) { l->first_visible=n*l->columns; paint_listbox_br(l); }
    } paint_slider_br(l); v.redraw=1;

  } else {

    if (l->total_items<=l->lines*l->columns) n=l->s0;
    else {
      n=(l->total_items-l->lines*l->columns+l->columns-1)/l->columns;

      n=(l->s0*(n-l->first_visible/l->columns)+l->s1*(l->first_visible/l->columns))/n;
    }
    if (n!=l->slide) { l->slide=n; paint_slider_br(l); v.redraw=1; }
  }

  if (old_zona!=l->zone) if (l->zone>=10) { // Highlight zone
    x=l->x+1+((l->zone-10)%l->columns)*(l->an+1);
    y=l->y+l->al+((l->zone-10)/l->columns)*(l->al+1);
    p=(byte *)l->list+l->item_width*(l->first_visible+l->zone-10);
    if (text_len(p)<l->an-2 && opc_img[v_thumb]) {
      wwrite(ptr,an,al,x+l->an/2,y,7,p,c4);
    } else {
      wwrite_in_box(ptr,an,x+l->an-1,al,x+1,y,6,p,c4);
    } v.redraw=1;
  }

  switch (l->zone) {
    case 2: mouse_graf=7; break;
    case 3: mouse_graf=9; break;
    case 4: mouse_graf=13; break;
  }
}

//-----------------------------------------------------------------------------

