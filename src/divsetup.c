
//-----------------------------------------------------------------------------
//      Setup.cfg configuration module
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#include "newfuncs.h"


void cfg_show_font(void);
void cfg_show_mouse(void);
void cfg_show_cursor(void);

//-----------------------------------------------------------------------------
//      Module-level variables
//-----------------------------------------------------------------------------

int stnot_big;
int stoldnot_big;
int stbig;
int stoldbig;
int stvga_an;
int stvga_al;

extern char user1[];
extern char user2[];

extern int help_item;
extern int cierra_rapido;
extern int skip_window_render;

extern int SelColorFont;
extern int SelColorOk;
void Selcolor0(void);

void gama0(void);

void wallpaper_thumb(void);
void prepare_wallpaper_temp(void);
void rescalar(byte *si,int sian,int sial,byte *di,int dian,int dial);

char vgasizes[32*16];

struct t_listbox lvgasizes={64+24,22,vgasizes,16,6,80};

void Vid_Setup1(void) {
char cWork[10];
  _show_items();
  create_listbox(&lvgasizes);
  sprintf(cWork,"%dx%d",stvga_an,stvga_al);

  wwrite_in_box(v.ptr,v.an/big2,v.an/big2-3,v.al/big2,5   ,12,0,(byte *)vga_marker,c1);
  wwrite_in_box(v.ptr,v.an/big2,v.an/big2-3,v.al/big2,4   ,12,0,(byte *)vga_marker,c3);

  wwrite(v.ptr,v.an/big2,v.al/big2,5   ,22,0,texts[177],c1);
  wwrite(v.ptr,v.an/big2,v.al/big2,4   ,22,0,texts[177],c3);
  wwrite(v.ptr,v.an/big2,v.al/big2,5+30,22,0,(byte *)cWork,c1);
  wwrite(v.ptr,v.an/big2,v.al/big2,4+30,22,0,(byte *)cWork,c4);
}

void Vid_Setup2(void) {
int need_refresh=0;
  _process_items();
  update_listbox(&lvgasizes);

  switch(v.active_item)
  {
    case 0: end_dialog=1; v_accept=1; break;
    case 1: end_dialog=1; v_accept=0; break;
  }

  if(stnot_big!=stoldnot_big)
  {
    stbig=!stnot_big;
    need_refresh=1;
  }
  if(stbig!=stoldbig)
  {
    stnot_big=!stbig;
    need_refresh=1;
  }

  if((lvgasizes.zone>=10)&&(mouse_b&1))
  {
    stvga_an    =modos[(lvgasizes.zone-10)+lvgasizes.first_visible].width;
    stvga_al    =modos[(lvgasizes.zone-10)+lvgasizes.first_visible].height;
    need_refresh=1;
  }

  if(stbig && stvga_an<640)
  {
    stbig=0;
    stnot_big=1;
    need_refresh=1;
  }

  stoldbig=stbig;
  stoldnot_big=stnot_big;

  if(need_refresh){
    call((voidReturnType)v.paint_handler);
    v.redraw=1;
  }

}
extern int soundstopped;

void Vid_Setup3(void) {
  if(v_accept)
  if(VS_WIDTH!=stvga_an || VS_HEIGHT!=stvga_al || VS_BIG!=stbig)
  {
    VS_BIG  =stbig;
    VS_WIDTH=stvga_an;
    VS_HEIGHT =stvga_al;
	v_title=(char *)texts[385];
    v_text =(char *)texts[386];
//    show_dialog((voidReturnType)info0);
    exit_requested=1; // Direct exit without asking
    return_mode=2;
    
  }
}

void Vid_Setup0(void) {
  int n;

  v.type=1;

  v.title=texts[881]+1;
  v.an=126+24+30;
  v.al=66+26;
  v.paint_handler=(voidReturnType)Vid_Setup1;
  v.click_handler=(voidReturnType)Vid_Setup2;
  v.close_handler=(voidReturnType)Vid_Setup3;

  stnot_big=!VS_BIG;
  stbig=VS_BIG;
  stoldbig=stbig;
  stoldnot_big=stnot_big;

  stvga_an  =VS_WIDTH;
  stvga_al  =VS_HEIGHT;

  for (n=0;n<num_modes;n++) {
    memset(&vgasizes[n*16],0,16);
    div_snprintf(&vgasizes[n*16], 16, "%s%d x %d",(modos[n].mode)?"SDL ":"VGA ",modos[n].width,modos[n].height);
  }

  lvgasizes.created=0;
  lvgasizes.total_items=num_modes;

  _button(100,7     ,v.al-14,0);
  _button(101,v.an-8,v.al-14,2);
  _flag(175,4,22+14+16,&stnot_big);
  _flag(176,4,22+26+16,&stbig);

  v_accept=0;
}

//-----------------------------------------------------------------------------
//  Background wallpaper selection
//-----------------------------------------------------------------------------

char Tap_name[_MAX_PATH];
char Tap_pathname[_MAX_PATH];
int Tap_mosaico;
int Tap_gama;
struct _gcolor gama_vieja[9];

void Get_Tapiz() {
	int len,n;
	char cwork[1024];
	byte *ptr=NULL;
	FILE *f;

	v_mode=1; v_type=9;
	v_text=(char *)texts[182];
	show_dialog((voidReturnType)browser0);

	strcpy(full,tipo[v_type].path);

	if (full[strlen(full)-1]!='/') strcat(full,"/");
		strcat(full,input);

	if (v_finished) {

		if(v_exists) {

			if ((f=fopen(full,"rb"))!=NULL) { // A file was selected

			fseek(f,0,SEEK_END);
			len=ftell(f);
			fseek(f,0,SEEK_SET);

			if (len>1024) 
				n=1024; 
			else 
				n=len;

			if (fread(cwork,1,n,f)==n) {

				if ( is_MAP((byte *)cwork) || is_PCX((byte *)cwork) || is_BMP((byte *)cwork) ) {

					DIV_STRCPY(Tap_name,input);
					DIV_STRCPY(Tap_pathname,full);

				} else {

					if ((ptr=(byte *) malloc(len))!=NULL) {

						fseek(f,0,SEEK_SET);

						if (fread(ptr,1,len,f)==len) {

							if (is_JPG(ptr,len)) {

								DIV_STRCPY(Tap_name,input);
								DIV_STRCPY(Tap_pathname,full);

							} else { 
								v_text=(char *)texts[46]; 
								show_dialog((voidReturnType)err0); 
							}

						} else { 
							v_text=(char *)texts[44]; 
							show_dialog((voidReturnType)err0); 
						}

						free(ptr);

					} else { 
						v_text=(char *)texts[45]; 
						show_dialog((voidReturnType)err0); 
					}

				}

			} else { 
				v_text=(char *)texts[44]; 
				show_dialog((voidReturnType)err0); 
			}

			fclose(f);

			} else { 
				v_text=(char *)texts[44]; 
				show_dialog((voidReturnType)err0); 
			}

		} else { 
			v_text=(char *)texts[43]; 
			show_dialog((voidReturnType)err0); 
		}
	}
}

void Tap_Setup1(void) {
  int an=v.an/big2,al=v.al/big2;
  int x;

  _show_items();
  for(x=strlen(Tap_pathname)-1;x>=0;x--) if(IS_PATH_SEP(Tap_pathname[x]) || x==0) {
    DIV_STRCPY(Tap_name,&Tap_pathname[x+1]);
    x=-1;
  }

  wrectangle(v.ptr,an,al,c0,4,41,128+2,88+2);
  wallpaper_thumb();

  wbox(v.ptr,an,al,c12,4,20,an-21,8);
  wwrite(v.ptr,an,al,4,12,0,texts[178],c3);
  wwrite(v.ptr,an,al,5,21,0,(byte *)Tap_name,c4);

  wrectangle(v.ptr,an,al,c0,an-38,24+8,34,7);
  for (x=0;x<32;x++) wbox(v.ptr,an,al,wallpaper_gradient[x*4],an-37+x,24+9,1,5);
}

void Tap_Setup2(void) {
  int an=v.an/big2,al=v.al/big2;
  int need_refresh=0;

  _process_items();
  switch(v.active_item)
  {
    case 0: end_dialog=1; v_accept=1; break;
    case 1: end_dialog=1; v_accept=0; break;
    case 2:
      Get_Tapiz();
      wbox(v.ptr,an,al,c3,4,20,an-15,8);
      wwrite(v.ptr,an,al,4,20,0,(byte *)Tap_name,c4);
      need_refresh=1;
      break;
    case 3: need_refresh=1; break;
    case 4: need_refresh=1; break;
  }

  if (wmouse_in(an-38,24+8,34,7) && (mouse_b&1))
  {
    gradient_buf=wallpaper_gradient;
    gradient_config=Setupfile.gradient_config;
    show_dialog((voidReturnType)gama0);
    if (v_accept) need_refresh=1;
  }
  if (need_refresh)
  {
    call((voidReturnType)v.paint_handler);
    v.redraw=1;
  }
}

void Tap_Setup3(void) {
  if(v_accept)
  {
    Setupfile.Desktop_Tile=Tap_mosaico;
    strcpy(Setupfile.Desktop_Image,Tap_pathname);

    Setupfile.Desktop_Gama=Tap_gama;

    prepare_wallpaper();
    update_box(0,0,vga_width,vga_height);
  } else {
    memcpy(Setupfile.gradient_config,gama_vieja,sizeof(gama_vieja));
    create_gradient_colors(Setupfile.gradient_config,wallpaper_gradient);
  }
}

void Tap_Setup0(void)
{
  int an,al;

  v.type=1;
  v.title=texts[882];
  v.an=138;
  v.al=150;
  an=v.an/big2,al=v.al/big2;
  strcpy(Tap_pathname,Setupfile.Desktop_Image);
  Tap_mosaico=Setupfile.Desktop_Tile;
  Tap_gama=Setupfile.Desktop_Gama;

  v.paint_handler=(voidReturnType)Tap_Setup1;
  v.click_handler=(voidReturnType)Tap_Setup2;
  v.close_handler=(voidReturnType)Tap_Setup3;

  _button(100,7,       v.al-14, 0);
  _button(101,v.an-8,  v.al-14, 2);
  _button(121,v.an-12, 18,      0);

  _flag(179, 4,                            24+8, &Tap_mosaico);
  _flag(181, v.an-49-text_len(texts[181]), 24+8, &Tap_gama);

  memcpy(gama_vieja,Setupfile.gradient_config,sizeof(gama_vieja));

  v_accept=0;
}

typedef struct _meminfo{
        unsigned long Bloque_mas_grande_disponible; // Largest block available
        unsigned Maximo_de_paginas_desbloqueadas; // Maximum unlocked pages
        unsigned Pagina_bloqueable_mas_grande; // Largest lockable page
        unsigned Espacio_de_direccionamiento_lineal; // Linear address space
        unsigned Numero_de_paginas_libres_disponibles; // Number of free pages available
        unsigned Numero_de_paginas_fisicas_libres; // Number of free physical pages
        unsigned Total_de_paginas_fisicas; // Total physical pages
        unsigned Espacio_de_direccionamiento_lineal_libre; // Free linear address space
        unsigned Tamano_del_fichero_de_paginas; // Page file size
        unsigned reserved[3];
}meminfo;


int Mem_GetHeapFree()
{
	return 0;
}
#ifdef WIN32
#include <windows.h>
#endif

void GetFreeMem(meminfo *Meminfo)
{
#ifdef WIN32
   MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
	Meminfo->Bloque_mas_grande_disponible = status.ullAvailPhys;
	return;
#else
    FILE *mem = fopen("/proc/meminfo", "r");

    if(mem == NULL) {
		Meminfo->Bloque_mas_grande_disponible = -1;
		return;
	}
    char line[256];
    while(fgets(line, sizeof(line), mem))
    {
        int64_t ram;
        if(sscanf(line, "MemFree: %ds kB", &ram) == 1)
        {
            fclose(mem);
            Meminfo->Bloque_mas_grande_disponible = ram*1024;
			return;
        }
    }

    // If we got here, then we couldn't find the proper line in the meminfo file:
    // do something appropriate like return an error code, throw an exception, etc.
    fclose(mem);
    Meminfo->Bloque_mas_grande_disponible = -1;
    return;
#endif

}

void MemInfo1(void) {
  char cWork[256];
  meminfo Mi_meminfo;
  char sizes[][3]={"KB","MB","GB","TB","PB"};
  byte csize=0;
  int mem,x,nuvent=0,meminmaps=0;
  float fmem;
  int an=v.an/big2,al=v.al/big2;

  _show_items();

  wwrite(v.ptr,an,al,an/2+1,12,1,texts[463],c1);
  wwrite(v.ptr,an,al,an/2,12,1,texts[463],c4);

  wrectangle(v.ptr,an,al,c0,1,21,an-2,20);
  wbox(v.ptr,an,al,c1,2,22,an-4,18);

  wwrite_in_box(v.ptr+2,an,an-4,al,(an-4)/2+1,24,1,(byte *)user1,c0);
  wwrite_in_box(v.ptr+2,an,an-4,al,(an-4)/2,24,1,(byte *)user1,c3);
  wwrite_in_box(v.ptr+2,an,an-4,al,(an-4)/2+1,32,1,(byte *)user2,c0);
  wwrite_in_box(v.ptr+2,an,an-4,al,(an-4)/2,32,1,(byte *)user2,c3);

  GetFreeMem(&Mi_meminfo);
  for(x=0;x<max_windows;x++)
  {
    if(window[x].type) nuvent++;
    if(window[x].type==100)
    {
      meminmaps+=window[x].mapa->map_width*window[x].mapa->map_height;
    }
  }
  if((mem=Mem_GetHeapFree())==-1) strcpy(cWork,(char *)texts[193]);
  else {
    mem=(Mi_meminfo.Bloque_mas_grande_disponible+mem)/1024;
    fmem = mem;
    fprintf(stdout,"Memory free: %d\n",mem);
    while(fmem>1024) {
		fmem=fmem/1024;
		csize++;
	}
	fprintf(stdout,"Memory free: %d (%f)\n",mem, fmem);

   sprintf(cWork,(char *)texts[195],fmem,sizes[csize]);
  }
  wwrite(v.ptr,an,al,an/2+1,44,1,(byte *)cWork,c1);
  wwrite(v.ptr,an,al,an/2,44,1,(byte *)cWork,c4);

  mem=meminmaps/1024;
  if(mem/1000) sprintf(cWork,(char *)texts[196],mem/1000,mem%1000,"KB");
  else         sprintf(cWork,(char *)texts[197],mem%1000,"KB");
  wwrite(v.ptr,an,al,an/2+1,52,1,(byte *)cWork,c1);
  wwrite(v.ptr,an,al,an/2,52,1,(byte *)cWork,c4);

  sprintf(cWork,(char *)texts[198],nuvent*100/max_windows,'%');
  wwrite(v.ptr,an,al,an/2+1,60,1,(byte *)cWork,c1);
  wwrite(v.ptr,an,al,an/2,60,1,(byte *)cWork,c4);
}

void MemInfo2(void) {
  _process_items();
  if(v.active_item==0) end_dialog=1;
}

void MemInfo0(void) {
  v.type=1;
  v.title=texts[199];
  v.an=168;
  v.al=88;
  v.paint_handler=(voidReturnType)MemInfo1;
  v.click_handler=(voidReturnType)MemInfo2;

  _button(100,v.an/2,v.al-14,1);
}

//-----------------------------------------------------------------------------
//  Configuration window
//-----------------------------------------------------------------------------

char color_cfg[12];
char tabul[4];
char undomem[16];
char old_editor_font;
int setup_switches[4];
int old_paint_cursor;

void Cfg_colors(void) {
  int an=v.an/big2,al=v.al/big2;
  wbox(v.ptr,an,al,color_cfg[0],13,22,7,7);
  wbox(v.ptr,an,al,color_cfg[1],53+25,22,7,7);
  wbox(v.ptr,an,al,color_cfg[2],93+34,22,7,7);
  wbox(v.ptr,an,al,color_cfg[3],13,45,7,7);
  wbox(v.ptr,an,al,color_cfg[4],53+25,45,7,7);
  wbox(v.ptr,an,al,color_cfg[5],93+34,45,7,7);
  wbox(v.ptr,an,al,color_cfg[6],13,45+10,7,7);
  wbox(v.ptr,an,al,color_cfg[7],53+25,45+10,7,7);
  wbox(v.ptr,an,al,color_cfg[8],93+34,45+10,7,7);
  wbox(v.ptr,an,al,color_cfg[10],13,45+20,7,7);
  wbox(v.ptr,an,al,color_cfg[11],53+25,45+20,7,7);
}

void Cfg_Setup1(void) {
  int an=v.an/big2,al=v.al/big2;
  int inc=31;
  _show_items();

  wwrite(v.ptr,an,al,5,12,0,texts[322],c1);
  wwrite(v.ptr,an,al,4,12,0,texts[322],c3);
  wrectangle(v.ptr,an,al,c0,12,21,9,9);
  wwrite(v.ptr,an,al,23,22,0,texts[323],c3);
  wrectangle(v.ptr,an,al,c0,52+25,21,9,9);
  wwrite(v.ptr,an,al,63+25,22,0,texts[324],c3);
  wrectangle(v.ptr,an,al,c0,92+34,21,9,9);
  wwrite(v.ptr,an,al,103+34,22,0,texts[325],c3);

  wbox(v.ptr,an,al,c0,2,32,an-4,1);

  wwrite(v.ptr,an,al,5,35,0,texts[326],c1);
  wwrite(v.ptr,an,al,4,35,0,texts[326],c3);
  wrectangle(v.ptr,an,al,c0,12,44,9,9);
  wwrite(v.ptr,an,al,23,45,0,texts[323],c3);
  wrectangle(v.ptr,an,al,c0,52+25,44,9,9);
  wwrite(v.ptr,an,al,63+25,45,0,texts[324],c3);
  wrectangle(v.ptr,an,al,c0,92+34,44,9,9);
  wwrite(v.ptr,an,al,103+34,45,0,texts[327],c3);

  wrectangle(v.ptr,an,al,c0,12,44+10,9,9);
  wwrite(v.ptr,an,al,23,45+10,0,texts[405],c3);
  wrectangle(v.ptr,an,al,c0,52+25,44+10,9,9);
  wwrite(v.ptr,an,al,63+25,45+10,0,texts[406],c3);
  wrectangle(v.ptr,an,al,c0,92+34,44+10,9,9);
  wwrite(v.ptr,an,al,103+34,45+10,0,texts[407],c3);

  wrectangle(v.ptr,an,al,c0,12,44+20,9,9);
  wwrite(v.ptr,an,al,23,45+20,0,texts[409],c3);
  wrectangle(v.ptr,an,al,c0,52+25,44+20,9,9);
  wwrite(v.ptr,an,al,63+25,45+20,0,texts[410],c3);

  wwrite(v.ptr,an,al,22+48,56+30,0,texts[328],c3);
  wrectangle(v.ptr,an,al,c0,52+50,55+30,9,9);
  wrectangle(v.ptr,an,al,c0,60+50,55+30,33,9);
  wrectangle(v.ptr,an,al,c0,92+50,55+30,9,9);
  wput(v.ptr,an,al,53+50,56+30,-51);
  wput(v.ptr,an,al,93+50,56+30,-52);

  wrectangle(v.ptr,an,al,c0,52+50,55+30+inc,9,9);
  wrectangle(v.ptr,an,al,c0,60+50,55+30+inc,33,9);
  wrectangle(v.ptr,an,al,c0,92+50,55+30+inc,9,9);
  wput(v.ptr,an,al,53+50,56+30+inc,-51);
  wput(v.ptr,an,al,93+50,56+30+inc,-52);
  wwrite(v.ptr,an,al,101+18+50,56+30+inc,1,texts[544],c3);

  wbox(v.ptr,an,al,c0,2,66+30,an-4,1);

  wwrite(v.ptr,an,al,5,69+30,0,texts[329],c1);
  wwrite(v.ptr,an,al,4,69+30,0,texts[329],c3);

  wbox(v.ptr,an,al,c0,2,97+30,an-4,1);

  wwrite(v.ptr,an,al,5,100+30,0,texts[333],c1);
  wwrite(v.ptr,an,al,4,100+30,0,texts[333],c3);

  Cfg_colors();
  cfg_show_font();
  cfg_show_mouse();
  cfg_show_cursor();
}

void cfg_show_cursor(void) {
  int an=v.an/big2,al=v.al/big2;

  wwrite(v.ptr,an,al,103+30,70+30,0,texts[327],c3);
  wrectangle(v.ptr,an,al,c0,91+30,68+30,10,10);
  wbox(v.ptr,an,al,c1,92+30,69+30,8,8);
  if (big) {
    switch(paint_cursor) {
      case 0: wput(v.ptr,v.an,v.al,(92+3+30)*2,(69+3+30)*2,19); break;
      case 1: wput(v.ptr,v.an,v.al,(92+2+30)*2,(69+2+30)*2,22); break;
      case 2: wput(v.ptr,v.an,v.al,(92+2+30)*2,(69+2+30)*2,26); break;
    }
  } else {
    switch(paint_cursor) {
      case 0: wput(v.ptr,v.an,v.al,92+3+30,69+3+30,20); break;
      case 1: wput(v.ptr,v.an,v.al,92+2+30,69+2+30,23); break;
      case 2: wput(v.ptr,v.an,v.al,92+2+30,69+2+30,27); break;
    }
  }
}

void cfg_show_font(void) {
  int min,max,slider;
  int an=v.an/big2,al=v.al/big2;

  wbox(v.ptr,an,al,c2,101+50,56+30,37,7);
  switch(editor_font) {
    case 0: wwrite(v.ptr,an,al,101+18+50,56+30,1,(byte *)"6x8",c3); break;
    case 1: wwrite(v.ptr,an,al,101+18+50,56+30,1,(byte *)"8x8",c3); break;
    case 2: wwrite(v.ptr,an,al,101+18+50,56+30,1,(byte *)"8x11",c3); break;
    case 3: wwrite(v.ptr,an,al,101+18+50,56+30,1,(byte *)"9x16",c3); break;
  }

  wbox(v.ptr,an,al,c2,61+50,56+30,31,7);
  min=61+50; max=89+50;
  slider=min+(editor_font*(max-min))/3;
  wbox(v.ptr,an,al,c0,slider-1,56+30,1,7);
  wbox(v.ptr,an,al,c0,slider+3,56+30,1,7);
  wput(v.ptr,an,al,slider,56+30,55);
}

void cfg_show_mouse(void) {
  int min,max,slider;
  int an=v.an/big2,al=v.al/big2;
  int inc=31;

  wbox(v.ptr,an,al,c2,61+50,56+30+inc,31,7);
  min=61+50; max=89+50;
  slider=min+(Setupfile.mouse_ratio*(max-min))/9;
  wbox(v.ptr,an,al,c0,slider-1,56+30+inc,1,7);
  wbox(v.ptr,an,al,c0,slider+3,56+30+inc,1,7);
  wput(v.ptr,an,al,slider,56+30+inc,55);
}

void Cfg_Setup2(void) {
  int an=v.an/big2,al=v.al/big2;
  int zona=0;
  int inc=31;

  _process_items();

  if (wmouse_in(12,21,9,9)) zona=1;
  if (wmouse_in(52+25,21,9,9)) zona=2;
  if (wmouse_in(92+34,21,9,9)) zona=3;

  if (wmouse_in(12,44,9,9)) zona=4;
  if (wmouse_in(52+25,44,9,9)) zona=5;
  if (wmouse_in(92+34,44,9,9)) zona=6;
  if (wmouse_in(12,44+10,9,9)) zona=7;
  if (wmouse_in(52+25,44+10,9,9)) zona=8;
  if (wmouse_in(92+34,44+10,9,9)) zona=9;
  if (wmouse_in(12,44+20,9,9)) zona=11;
  if (wmouse_in(52+25,44+20,9,9)) zona=12;

  if (wmouse_in(52+50,55+30,9,9)) zona=13; //7
  if (wmouse_in(92+50,55+30,9,9)) zona=14; //8
  if (wmouse_in(91+30,68+30,10,10)) zona=15; //9

  if (wmouse_in(52+50,55+30+inc,9,9)) zona=16;
  if (wmouse_in(92+50,55+30+inc,9,9)) zona=17;

  if ((zona>=1 && zona<=12) || zona==15) mouse_graf=2;
  if (zona==13) mouse_graf=10;
  if (zona==14) mouse_graf=11;

  if (zona==13 && (mouse_b&1) && wmouse_x!=-1) {
    if (!(v.buttons&1)) {
      wput(v.ptr,an,al,53+50,56+30,-53); v.buttons^=1; v.redraw=1;
    }
  } else if (v.buttons&1) {
    wput(v.ptr,an,al,53+50,56+30,-51); v.buttons^=1; v.redraw=1;
    if (editor_font>0) { editor_font--; cfg_show_font(); }
  }

  if (zona==14 && (mouse_b&1) && wmouse_x!=-1) {
    if (!(v.buttons&2)) {
      wput(v.ptr,an,al,93+50,56+30,-54); v.buttons^=2; v.redraw=1;
    }
  } else if (v.buttons&2) {
    wput(v.ptr,an,al,93+50,56+30,-52); v.buttons^=2; v.redraw=1;
    if (editor_font<3) { editor_font++; cfg_show_font(); }
  }

  if (zona==16 && (mouse_b&1) && wmouse_x!=-1) {
    if (!(v.buttons&4)) {
      wput(v.ptr,an,al,53+50,56+30+inc,-53); v.buttons^=4; v.redraw=1;
    }
  } else if (v.buttons&4) {
    wput(v.ptr,an,al,53+50,56+30+inc,-51); v.buttons^=4; v.redraw=1;
    if (Setupfile.mouse_ratio>0) { Setupfile.mouse_ratio--; cfg_show_mouse(); }
  }

  if (zona==17 && (mouse_b&1) && wmouse_x!=-1) {
    if (!(v.buttons&8)) {
      wput(v.ptr,an,al,93+50,56+30+inc,-54); v.buttons^=8; v.redraw=1;
    }
  } else if (v.buttons&8) {
    wput(v.ptr,an,al,93+50,56+30+inc,-52); v.buttons^=8; v.redraw=1;
    if (Setupfile.mouse_ratio<9) { Setupfile.mouse_ratio++; cfg_show_mouse(); }
  }

  if (!(mouse_b&1) && (prev_mouse_buttons&1) && zona==15) {
    paint_cursor=(paint_cursor+1)%3;
    cfg_show_cursor(); v.redraw=1;
  }

  if ((mouse_b&1) && zona>=1 && zona<=12) {
    SelColorFont=color_cfg[zona-1]; show_dialog((voidReturnType)Selcolor0);
    if(SelColorOk) {
      color_cfg[zona-1]=SelColorFont;
      Cfg_colors(); v.redraw=1;
    }
  }

  switch(v.active_item) {
    case 0: end_dialog=1; v_accept=1; break;
    case 1: end_dialog=1; v_accept=0; break;
  }
}

void Cfg_Setup3(void) {
  if (!v_accept) {
    exploding_windows=setup_switches[0];
    move_full_window=setup_switches[1];
    auto_save_session=setup_switches[2];
    colorizer=setup_switches[3];
    paint_cursor=old_paint_cursor;
  } else {
	  c1=color_cfg[0];
	  prepare_wallpaper();
  }
}

void Cfg_Setup0(void) {
  v.type=1;
  v.title=texts[883];
  v.an=192;
  v.al=200;
  v.paint_handler=(voidReturnType)Cfg_Setup1;
  v.click_handler=(voidReturnType)Cfg_Setup2;
  v.close_handler=(voidReturnType)Cfg_Setup3;

  _button(100,7,v.al-14,0);
  _button(101,v.an-8,v.al-14,2);
  itoa(undo_memory/1024,undomem,10);
  itoa(tab_size,tabul,10);
  _get(411,12,76,40,(byte *)tabul,4,1,16);
  _get(330,12,78+30,84,(byte *)undomem,16,256,4096);
  _flag(331,12,110+30,&exploding_windows);
  _flag(336,12,120+30,&move_full_window);
  _flag(332,12,130+30,&auto_save_session);
  _flag(412,12,140+30,&colorizer);

  setup_switches[0]=exploding_windows;
  setup_switches[1]=move_full_window;
  setup_switches[2]=auto_save_session;
  setup_switches[3]=colorizer;

  old_paint_cursor=paint_cursor;

  color_cfg[0]=c2; color_cfg[1]=c4; color_cfg[2]=c_b_low;
  color_cfg[3]=ce1; color_cfg[4]=ce4; color_cfg[5]=c_y;
  color_cfg[6]=c_com; color_cfg[7]=c_sim; color_cfg[8]=c_res;
  color_cfg[9]=c_pre; color_cfg[10]=c_num; color_cfg[11]=c_lit;
  old_editor_font=editor_font;
  v_accept=0;
}

void create_saved_window(voidReturnType init_handler,int nx,int ny);
extern struct twindow window_aux;
void test_cursor(void);
void resize_program(void);
extern int VidModeChanged;

void create_title_bar(void);

void Cfg_Setup_end(void) {
  int n,ew,found;
  byte * ptr;
  int an,al;
  FILE * f=NULL;
  struct twindow vp[24];
  int i;

  for (i=0,n=0;n<max_windows;n++) {
    if (window[n].click_handler==(voidReturnType)program2) i++;
  }

  if (i>24 || !v_accept) editor_font=old_editor_font;

  if (v_accept) {
    if (undo_memory/1024!=atoi(undomem)) {
      ptr=(byte*)realloc(undo,atoi(undomem)*1024);
      if (ptr!=NULL) {
        undo=ptr; undo_memory=atoi(undomem)*1024;
        undo_index=0; for (n=0;n<max_undos;n++) undo_table[n].mode=-1;
      }
    }
    tab_size=atoi(tabul);
  }

  if (v_accept && (color_cfg[0]!=c2 || color_cfg[1]!=c4 || color_cfg[2]!=c_b_low ||
      color_cfg[3]!=ce1 || color_cfg[4]!=ce4 || color_cfg[5]!=c_y ||
      color_cfg[6]!=c_com || color_cfg[7]!=c_sim || color_cfg[8]!=c_res ||
      color_cfg[9]!=c_pre || color_cfg[10]!=c_num || color_cfg[11]!=c_lit ||
      old_editor_font!=editor_font || colorizer!=setup_switches[3])) {

    if (old_editor_font!=editor_font) {
      switch(editor_font) {
        case 0: f=fopen("system/sys06x08.bin","rb"); break;
        case 1: f=fopen("system/sys08x08.bin","rb"); break;
        case 2: f=fopen("system/sys08x11.bin","rb"); break;
        case 3: f=fopen("system/sys09x16.bin","rb"); break;
      }
      if (f!=NULL) {
		printf("loading new font %d %d\n",editor_font,old_editor_font);  
        fseek(f,0,SEEK_END); n=ftell(f);
        if ((ptr=(byte *)malloc(n))!=NULL) {
          fseek(f,0,SEEK_SET);
          if (fread(ptr,1,n,f)==n) {
            switch(editor_font) {
              case 0: font_width=6; font_height=8; break;
              case 1: font_width=8; font_height=8; break;
              case 2: font_width=8; font_height=11; break;
              case 3: font_width=9; font_height=16; break;
            } char_size=font_width*font_height;
            free(font); font=ptr;
          } else free(ptr);
        } fclose(f);
      }
    }

    c2=color_cfg[0]; c4=color_cfg[1]; c_b_low=color_cfg[2];
    ce1=color_cfg[3]; ce4=color_cfg[4]; c_y=color_cfg[5];
    c_com=color_cfg[6]; c_sim=color_cfg[7]; c_res=color_cfg[8];
    c_pre=color_cfg[9]; c_num=color_cfg[10]; c_lit=color_cfg[11];
    c1=average_color(c0,c2); c3=average_color(c2,c4); ce01=average_color(c0,ce1); ce2=average_color(ce1,ce4);
    c01=average_color(c0,c1); c12=average_color(c1,c2); c23=average_color(c2,c3); c34=average_color(c3,c4);
    zoom_move=c3;

    colors_rgb[0]=dac[c2*3]; colors_rgb[1]=dac[c2*3+1]; colors_rgb[2]=dac[c2*3+2];
    colors_rgb[3]=dac[c4*3]; colors_rgb[4]=dac[c4*3+1]; colors_rgb[5]=dac[c4*3+2];
    colors_rgb[6]=dac[c_b_low*3]; colors_rgb[7]=dac[c_b_low*3+1]; colors_rgb[8]=dac[c_b_low*3+2];
    colors_rgb[9]=dac[ce1*3]; colors_rgb[10]=dac[ce1*3+1]; colors_rgb[11]=dac[ce1*3+2];
    colors_rgb[12]=dac[ce4*3]; colors_rgb[13]=dac[ce4*3+1]; colors_rgb[14]=dac[ce4*3+2];
    colors_rgb[15]=dac[c_y*3]; colors_rgb[16]=dac[c_y*3+1]; colors_rgb[17]=dac[c_y*3+2];

    colors_rgb[18]=dac[c_com*3]; colors_rgb[19]=dac[c_com*3+1]; colors_rgb[20]=dac[c_com*3+2];
    colors_rgb[21]=dac[c_sim*3]; colors_rgb[22]=dac[c_sim*3+1]; colors_rgb[23]=dac[c_sim*3+2];
    colors_rgb[24]=dac[c_res*3]; colors_rgb[25]=dac[c_res*3+1]; colors_rgb[26]=dac[c_res*3+2];
    colors_rgb[27]=dac[c_pre*3]; colors_rgb[28]=dac[c_pre*3+1]; colors_rgb[29]=dac[c_pre*3+2];
    colors_rgb[30]=dac[c_num*3]; colors_rgb[31]=dac[c_num*3+1]; colors_rgb[32]=dac[c_num*3+2];
    colors_rgb[33]=dac[c_lit*3]; colors_rgb[34]=dac[c_lit*3+1]; colors_rgb[35]=dac[c_lit*3+2];

    skip_window_render=1; cierra_rapido=1;
    ew=exploding_windows; exploding_windows=0;

    if (old_editor_font!=editor_font) {
      for (n=0;n<max_windows;n++) if (window[n].click_handler==(voidReturnType)help2) {
        move(0,n); close_window(); help_item++; break;
      } if (n==max_windows) help_item=0;
      i=0; do {
        found=0;
        for (n=max_windows-1;n>=0;n--) if (window[n].click_handler==(voidReturnType)program2) {
          memcpy(&vp[i].type,&window[n].type,sizeof(struct twindow)); i++;
          window[n].close_handler=(voidReturnType)dummy_handler;
          move(0,n); close_window();
          found=1; break;
        }
      } while (found);
    }

    create_title_bar();

    for (n=0;n<max_windows;n++) if (window[n].type) {
      xchg(0,n);

      if (v.foreground==2) {
        swap(v.an,v._an); swap(v.al,v._al);
        swap(v.x,v._x); swap(v.y,v._y);
      }

      ptr=v.ptr;
      an=v.an/big2; al=v.al/big2;
      wrectangle(ptr,an,al,c2,0,0,an,al);
      wput(ptr,an,al,an-9,2,35);
      wput(ptr,an,al,an-17,2,37);

      if (v.type>=100 && !v.state) {
        wgra(v.ptr,an,al,c1,2,2,an-20,7);
        if (text_len(v.title)+3>an-20) {
          wwrite_in_box(v.ptr,an,an-19,al,4,2,0,v.title,c0);
          wwrite_in_box(v.ptr,an,an-19,al,3,2,0,v.title,c2);
        } else {
          wwrite(v.ptr,an,al,2+(an-20)/2,3,1,v.title,c0);
          wwrite(v.ptr,an,al,2+(an-20)/2,2,1,v.title,c2);
        }
      } else {
        wgra(ptr,an,al,c_b_low,2,2,an-20,7);
        if (text_len(v.title)+3>an-20) {
          wwrite_in_box(ptr,an,an-19,al,4,2,0,v.title,c1);
          wwrite_in_box(ptr,an,an-19,al,3,2,0,v.title,c4);
        } else {
          wwrite(ptr,an,al,3+(an-20)/2,2,1,v.title,c1);
          wwrite(ptr,an,al,2+(an-20)/2,2,1,v.title,c4);
        }
      }

      call((voidReturnType)v.paint_handler);

      if (v.foreground==2) {
        swap(v.an,v._an); swap(v.al,v._al);
        swap(v.x,v._x); swap(v.y,v._y);
      }

      xchg(0,n);
    }

    if (old_editor_font!=editor_font) {
      for (n=0;n<i;n++) {
        memcpy(&window_aux.type,&vp[n].type,sizeof(struct twindow));
        v_prg=window_aux.prg; VidModeChanged=33;
        create_saved_window((voidReturnType)resize_program,window_aux.x,window_aux.y);
      }
      if (help_item) help(help_item-1);
    }

    skip_window_render=0; cierra_rapido=0; exploding_windows=ew;

    update_box(0,0,vga_width,vga_height); blit_screen(screen_buffer);
  }
}

void resize_program(void) {

  v.type=102; v.prg=v_prg;
  if (v.prg->an<4*big2) v.prg->an=4*big2;
  if (v.prg->al<2*big2) v.prg->al=2*big2;
  v.an=(4+8)*big2+font_width*v_prg->an;
  v.al=(12+16)*big2+font_height*v_prg->al;

  if (v.an>vga_width) {
    v.prg->an=(vga_width-12*big2)/font_width; // Calculate maximized size (in chars)
    v.an=(4+8)*big2+font_width*v.prg->an;
    window_aux.an=v.an;
  }
  if (v.al>vga_height) {
    v.prg->al=(vga_height-28*big2)/font_height;
    v.al=(12+16)*big2+font_height*v.prg->al;
    window_aux.al=v.al;
  }
  if (big) {
    if (v.an&1) { v.an++; } if (v.al&1) { v.al++; } v.an=-v.an;
  }

  v.title=(byte *)v_prg->filename;
  v.name=(byte *)v_prg->filename;
  v.paint_handler=(voidReturnType)program1;
  v.click_handler=(voidReturnType)program2;
  v.close_handler=(voidReturnType)program3;
  v.redraw=2;
  test_cursor();
}

int x_wallpaper_width, x_wallpaper_height;
byte *x_wallpaper_map, *x_wallpaper=NULL;

void wallpaper_thumb(void)
{
	int x, y, an, al, thumb_pos;

	float coefredx, coefredy, a, b;
	byte *ptr;


	SDL_Rect rc;
	an=128*big2; al=88*big2;

	prepare_wallpaper_temp();


  if(x_wallpaper==NULL) return;

  // Create the wallpaper thumbnail

  coefredx=x_wallpaper_width/((float)128*(float)big2);
  coefredy=x_wallpaper_height/((float) 88*(float)big2);

  if ((ptr=(byte *)malloc(an*al))==NULL) return;

  memset(ptr,0,an*al);

  a=(float)0.0;
  for(y=0;y<al;y++)
  {
    b=(float)0.0;
    for(x=0;x<an;x++)
    {
      ptr[y*an+x]=x_wallpaper_map[((int)a)*x_wallpaper_width+(int)b];
      b+=coefredx;
    }
    a+=coefredy;
  }
  free(x_wallpaper);
  x_wallpaper=NULL;

  // Draw it on screen

  thumb_pos=5*big2+(42*big2)*v.an;

  for (y=0; y<al; y++)
  {
    for (x=0; x<an; x++)
    {
      v.ptr[y*v.an+x+thumb_pos]=ptr[y*an+x];
    }
  }
  free(ptr);
  
}

byte x_cwallpaper[256];

extern SDL_Surface *vga;


void prepare_wallpaper_temp(void) {
  FILE * f;
  int x,y,lon,tap_an,tap_al,con_x=0,con_y=0,an,al,n;
  byte *p,*q;
  byte * temp;
  byte pal[768];
  byte old_dac[768];
  byte old_dac4[768];


  // TODO: Support tiling/rescaling and loading a user-specified map file
  // instead of always using "wallpaper.map"

  if ((f=fopen(Tap_pathname,"rb"))==NULL) return;
  fseek(f,0,SEEK_END); lon=ftell(f); fseek(f,0,SEEK_SET);
  if (x_wallpaper!=NULL) { free(x_wallpaper); x_wallpaper=NULL; }
  if ((x_wallpaper=(byte*)malloc(lon))==NULL) { fclose(f); return; }
  if (fread(x_wallpaper,1,lon,f)!=lon)
  {
    fclose(f);
    free(x_wallpaper);
    x_wallpaper=NULL;
    return;
  }
  fclose(f);

  tap_an=map_width; tap_al=map_height;
  if (is_MAP(x_wallpaper)) x=1;
  else if (is_PCX(x_wallpaper)) x=2;
  else if (is_BMP(x_wallpaper)) x=3;
  else if (is_JPG(x_wallpaper,lon)) x=4;
  else x=0;
  an=map_width; al=map_height;
  swap(map_width,tap_an); swap(map_height,tap_al);

  if (!x) { free(x_wallpaper); x_wallpaper=NULL; return; }

  if ((temp=(byte*)malloc(tap_an*tap_al+tap_an))==NULL)
  {
    free(x_wallpaper);
    x_wallpaper=NULL;
    return;
  }

  swap(map_width,tap_an); swap(map_height,tap_al);

  memcpy(old_dac4,dac4,768);
  memcpy(old_dac,dac,768);

  n=1;
  switch (x) {
    case 1: descomprime_MAP(x_wallpaper,temp,0); break;
    case 2: descomprime_PCX(x_wallpaper,temp,0); break;
    case 3: descomprime_BMP(x_wallpaper,temp,0); break;
    case 4: n=descomprime_JPG(x_wallpaper,temp,0,lon); break;
  } swap(map_width,tap_an); swap(map_height,tap_al);
  free(x_wallpaper);
  x_wallpaper=NULL;
  if(!n) {
    free(temp);
    x_wallpaper_map=x_wallpaper=NULL;
    v_text=(char *)texts[44]; show_dialog((voidReturnType)err0);
    return;
  }

  memcpy(pal,dac4,768);
  create_dac4();

  if (!Tap_gama) {
    for (x=0,p=pal;x<256;x++,p+=3)
      x_cwallpaper[x]=fast_find_color(*p,*(p+1),*(p+2));
    p=temp; q=p+tap_an*tap_al;
    do *p=x_cwallpaper[*p]; while (++p<q); // Remap to the current palette
  } else {
    for (x=0,p=pal;x<256;x++,p+=3)
      x_cwallpaper[x]=wallpaper_gradient[(*p+*(p+1)+*(p+2))*2/3];
    p=temp; q=p+tap_an*tap_al;
    do *p=x_cwallpaper[*p]; while (++p<q);
  }

  if ((p=(byte *)malloc(vga_width*vga_height))==NULL) { free(temp); x_wallpaper=NULL; return; }

  if (Tap_mosaico) {
    x_wallpaper_width=vga_width; x_wallpaper_height=vga_height; x_wallpaper_map=x_wallpaper=p;
    // Build the tiled mosaic

    for(y=0; y<vga_height; y++)
    {
      for(x=0, con_x=0; x<vga_width; x++, con_x++)
      {
        if(con_x==an) con_x=0;
        *p++ = temp[con_y*an+con_x];
      }
      con_y++;
      if(con_y==al) con_y=0;
    }

    free(temp);
  } else {    
    rescalar(temp,an,al,p,vga_width,vga_height);
    free(temp);
    x_wallpaper_width=vga_width; x_wallpaper_height=vga_height; x_wallpaper_map=x_wallpaper=p;
  }

  memcpy(dac,old_dac,768);
  memcpy(dac4,old_dac4,768);

}

