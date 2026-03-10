#include "global.h"
#include "fpgfile.hpp"

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

int nueva_ventana_carga(voidReturnType init_handler,int nx,int ny);
int nuevo_mapa_carga(int nx,int ny,char *nombre,byte *mapilla);
void carga_programa0(void);
void carga_Fonts0(void);
void carga_help(int n,int helpal,int helpline,int x1,int x2);
void help_xref(int n,int linea);
void mixer0(void);
void OpenDesktopSound(FILE *f);
void SaveDesktopSound(pcminfo *mypcminfo, FILE *f);
void OpenDesktopSong(void);

int CDinit(void);
int get_cd_error(void);
extern short CDPlaying;

// M3D_info removed (MODE8/3D map editor deleted)

extern char Load_FontPathName[256];
extern char Load_FontName[14];

//extern
struct _calc {
  char ctext[128];
  char cget[128];
  char cresult[128];
  int cint,chex;
};
extern struct _calc * pcalc;
extern struct _calc * readcalc;
void calc0(void);

extern int helpidx[4096];              // Por cada término {inicio,longitud}
extern int help_item;                  // Indica sobre que término se pide ayuda
extern int help_len;                   // Longitud del help_buffer
extern int help_an,help_al;            // Ancho y alto de la ventana de ayuda
extern int help_l,help_lines;          // Línea actual, y lineas totales
extern byte help_title[128];           // Título del término
extern byte *help_buffer,*h_buffer;    // Buffer para contener la ayuda, auxiliar
extern byte *help_line;                // Puntero a la línea actual
extern byte *help_end;                 // Final de help_buffer;
extern int loaded[64],n_loaded;        // Imágenes cargadas, hasta un máximo de 32
extern int backto[64];      // Cola circular para almacenar los topicos consultados {n,línea}
extern int i_back,a_back,f_back; // Inicio y final de la cola circular (ambos 0,2,..62)

void Fonts1(void); void Fonts2(void); void Fonts3(void);

void Load_Font_session(FILE *file);
int Save_Font_session(FILE *file,int);
FILE *desktop;

char pathtmp[1024];

int getFileCreationTime(char *path) {
    struct stat attr;
    stat(path, &attr);
	return (int)attr.st_mtime;
}


void DownLoad_Desktop()
{
int iWork,x,numvent=0,n;
int man,mal;
pcminfo *mypcminfo;
modinfo *mymodinfo;

        // Pone una cabecera de identificación
        desktop=fopen("system/session.dtf","wb");
        n=fwrite("dtf\x1a\x0d\x0a\x0",8,1,desktop);
        // guarda la antigua resolución
        iWork=Setupfile.Vid_modeAlto+Setupfile.Vid_modeAncho*10000+(Setupfile.Vid_modeBig<<31);
        n=fwrite(&iWork,1,4,desktop);
        // reserva espacio para el numero de ventanas
        n=fwrite(&numvent,1,4,desktop);
        // guarda paleta /4
        n=fwrite(dac,768,1,desktop);
        // guarda tabla ghost
        n=fwrite(ghost,65536,1,desktop);
        // Mira y guarda una por una las ventanas utilizadas
        for(x=max_windows-1;x>=0;x--)
        {
                if(ventana[x].tipo!=0 && ventana[x].titulo)
                {
                        numvent++;
                        n=fwrite(&ventana[x],1,sizeof(struct tventana),desktop);
                        switch(ventana[x].tipo)
                        {
                                //Estructura de ventana
                                case    2: //menu
                                        iWork=-1;
                                        if(ventana[x].paint_handler==menu_principal1)
                                                        iWork=0;
                                        if(ventana[x].paint_handler==menu_programas1)
                                                        iWork=1;
                                        if(ventana[x].paint_handler==menu_paletas1)
                                                        iWork=2;
                                        if(ventana[x].paint_handler==menu_mapas1)
                                                        iWork=3;
                                        if(ventana[x].paint_handler==menu_graficos1)
                                                        iWork=4;
                                        if(ventana[x].paint_handler==menu_fuentes1)
                                                        iWork=5;
                                        if(ventana[x].paint_handler==menu_sonidos1)
                                                        iWork=6;
                                        if(ventana[x].paint_handler==menu_sistema1)
                                                        iWork=7;
                                        if(ventana[x].paint_handler==menu_edicion1)
                                                        iWork=8;
                                        // menu_mapas3D1 check removed (MODE8/3D map editor deleted)
                                        n=fwrite(&iWork,1,4,desktop);
                                        
                                        break;
                                case    3: //palet
                                        break;
                                case    4: //timer
                                        if(ventana[x].paint_handler==Clock1)
                                                iWork=1;
                                        n=fwrite(&iWork,1,4,desktop);
                                        break;
                                case    5: //papelera
                                        break;
                                case    8: //mixer
                                        break;
                                case    100: //map
                                        // estructura tmapa
                                        man=ventana[x].mapa->map_an;
                                        mal=ventana[x].mapa->map_al;
                                        n=fwrite(ventana[x].mapa,1,sizeof(struct tmapa),desktop);
                                        // Grafico
                                        n=fwrite((char *)ventana[x].mapa->map,man*mal,1,desktop);
                                        break;
                                case    101: //fpg
                                        // estructura fpg
                                        n=fwrite(ventana[x].aux,1,sizeof(FPG),desktop);
                                        break;
                                case    102: //prg
                                        if(ventana[x].prg!=NULL)
                                        {
                                                iWork=0;
                                                n=fwrite(&iWork,1,4,desktop);
                                                n=fwrite(ventana[x].prg,1,sizeof(struct tprg),desktop);
                                                n=fwrite(ventana[x].prg->buffer,1,ventana[x].prg->buffer_lon,desktop);
                                                iWork=(uintptr_t)ventana[x].prg->lptr-(uintptr_t)ventana[x].prg->buffer;
                                                n=fwrite(&iWork,1,4,desktop);
                                                iWork=(uintptr_t)ventana[x].prg->vptr-(uintptr_t)ventana[x].prg->buffer;
                                                n=fwrite(&iWork,1,4,desktop);
                                        }
                                        else
                                        {
                                                //if(ventana[x].aux!=NULL)
                                                if (!strcmp((char *)ventana[x].nombre,(char *)texto[83])) {
                                                        iWork=1;
                                                        n=fwrite(&iWork,1,4,desktop);
                                                        iWork=Save_Font_session(desktop,iWork);
                                                } else if (!strcmp((char *)ventana[x].nombre,(char *)texto[413])) {
                                                  iWork=3;
                                                  n=fwrite(&iWork,1,4,desktop);
                                                  strcpy(((struct _calc*)(ventana[x].aux))->ctext,((struct _calc*)(ventana[x].aux))->cget);
                                                  n=fwrite(ventana[x].aux,1,sizeof(struct _calc),desktop);
                                                } else {
                                                        iWork=2;
                                                        n=fwrite(&iWork,1,4,desktop);
                                                        n=fwrite(&help_item,1,4,desktop);
                                                        n=fwrite(&help_al,1,4,desktop);
                                                        n=fwrite(&help_l,1,4,desktop);
                                                }
                                        }
                                        break;
                                case    104: //fnt
                                        // Descargarse
                                        n=fwrite(ventana[x].aux,1,14,desktop);
                                        n=fwrite(ventana[x].aux+14,1,_MAX_PATH-14,desktop);
                                        break;
                                 case    105: //pcm
                                        mypcminfo=(pcminfo *)ventana[x].aux;
                                        SaveDesktopSound(mypcminfo, desktop);
                                        break;
                                 // case 106 (map3d) removed (MODE8/3D map editor deleted)
                                 case    107: //mod
                                        mymodinfo=(modinfo *)ventana[x].aux;
                                        n=fwrite(mymodinfo->name,1,14,desktop);
                                        n=fwrite(mymodinfo->pathname,1,256,desktop);
                                        break;
                        }
                }
			if(ventana[x].ptr!=NULL)
				free(ventana[x].ptr);
        }
        fseek(desktop,0,SEEK_SET);
        fseek(desktop,8+4,SEEK_SET);
        n=fwrite(&numvent,1,4,desktop);
        fclose(desktop);
}

int VidModeChanged=0;

int modo_anterior;

int Can_UpLoad_Desktop()
{
char cWork[8];
int iWork;
        VidModeChanged=0;
        desktop=fopen("system/session.dtf","rb");
        if(desktop==NULL)
                return(0);
        // read the header id
        fread(cWork,8,1,desktop);
        // Check old resolution
        fread(&iWork,1,4,desktop);
        if(iWork!=Setupfile.Vid_modeAlto+Setupfile.Vid_modeAncho*10000+(Setupfile.Vid_modeBig<<31)) {
                modo_anterior=iWork;
                VidModeChanged=1;
        }
        // se salta el contador de ventanas
        fread(cWork,4,1,desktop);
        // lee paleta /4
        fread(dac,768,1,desktop);
        // lee tabla ghost
        fread(ghost,65536,1,desktop);
        fclose(desktop);
return(1);
}

struct tventana ventana_aux;
struct tmapa    maux;
FPG             faux;
struct tprg     paux;
char *          baux;

int UpLoad_Desktop()
{
	int iWork,iWork2,iWork3,x,numvent;
	FILE *f;
	
	int dtime = getFileCreationTime("system/session.dtf");
		
        desktop=fopen("system/session.dtf","rb");
        if(desktop==NULL)
                return(0);
		
		
        fseek(desktop,8+4,SEEK_SET);
        fread(&numvent,1,4,desktop);
        fseek(desktop,8+4+4+768+65536,SEEK_SET);
        // Load each of the windows one by one
        for(x=0;x<numvent;x++)
        {
                // Window struct data
                fread(&ventana_aux,1,sizeof(struct tventana),desktop);
                switch(ventana_aux.tipo)
                {
                        case    2: //menu
                                fread(&iWork,1,4,desktop);
                                switch(iWork)
                                {
                                        case    0:
                                                nueva_ventana_carga(menu_principal0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    1:
                                                nueva_ventana_carga(menu_programas0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    2:
                                                nueva_ventana_carga(menu_paletas0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    3:
                                                nueva_ventana_carga(menu_mapas0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    4:
                                                nueva_ventana_carga(menu_graficos0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    5:
                                                nueva_ventana_carga(menu_fuentes0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    6:
                                                nueva_ventana_carga(menu_sonidos0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    7:
                                                nueva_ventana_carga(menu_sistema0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        case    8:
                                                nueva_ventana_carga(menu_edicion0,ventana_aux.x,ventana_aux.y);
                                                break;
                                        // case 9 (menu_mapas3D0) removed (MODE8/3D map editor deleted)
                                        default:
                                                break;
                                }
                                if(!Interpretando)
                                        actualiza_caja(0,0,vga_an,vga_al);
                                break;
                        case    3:
                                nueva_ventana_carga(paleta0,ventana_aux.x,ventana_aux.y);
                                if(!Interpretando)
                                        actualiza_caja(0,0,vga_an,vga_al);
                                break;
                        case    4: //timer
                                fread(&iWork,1,4,desktop);
                                switch(iWork)
                                {
                                        case 0: break; // removed: CD player
                                                break;
                                        case 1: //Reloj
                                                nueva_ventana_carga(Clock0,ventana_aux.x,ventana_aux.y);
                                                break;
                                }
                                if(!Interpretando)
                                        actualiza_caja(0,0,vga_an,vga_al);
                                break;
                        case    5: //papelera
                                nueva_ventana_carga(Bin0,ventana_aux.x,ventana_aux.y);
                                if(!Interpretando)
                                        actualiza_caja(0,0,vga_an,vga_al);
                                break;
                        case    8: //mixer
                                nueva_ventana_carga(mixer0,ventana_aux.x,ventana_aux.y);
                                if(!Interpretando)
                                        actualiza_caja(0,0,vga_an,vga_al);
                                break;
                        case    100: //map
                                // estructura tmapa
                                fread(&maux,1,sizeof(struct tmapa),desktop);
                                baux=(char *)malloc(maux.map_an*maux.map_al);
                                if (baux==NULL) {
                                  fseek(desktop,maux.map_an*maux.map_al,SEEK_CUR);
                                  if(!Interpretando) actualiza_caja(0,0,vga_an,vga_al);
                                  break;
                                }
                                fread(baux,maux.map_an,maux.map_al,desktop);
                                map_an=maux.map_an;
                                map_al=maux.map_al;
                                if (nuevo_mapa_carga(ventana_aux.x,ventana_aux.y,maux.filename,(byte *)baux)) {
                                  free(baux);
                                  if(!Interpretando) actualiza_caja(0,0,vga_an,vga_al);
                                  break;
                                }
                                v.mapa->TengoNombre=maux.TengoNombre;
                                v.mapa->codigo=maux.codigo;
                                v.mapa->Codigo=maux.Codigo;
                                memcpy((char *)v.mapa->path,(char *)maux.path,_MAX_PATH+1);
                                memcpy((char *)v.mapa->filename,(char *)maux.filename,255);
                                v.mapa->zoom=maux.zoom;
                                v.mapa->zoom_x=maux.zoom_x;
                                v.mapa->zoom_y=maux.zoom_y;
                                v.mapa->zoom_cx=maux.zoom_cx;
                                v.mapa->zoom_cy=maux.zoom_cy;
                                v.mapa->grabado=maux.grabado;
                                memcpy((char *)v.mapa->descripcion,(char *)maux.descripcion,32);
                                memcpy((char *)v.mapa->puntos,(char *)maux.puntos,512*2);
                                // Grafico
                                call((voidReturnType )v.paint_handler);
                                wvolcado(copia,vga_an,vga_al,v.ptr,v.x,v.y,v.an,v.al,0);
                                if(!Interpretando)
                                        actualiza_caja(0,0,vga_an,vga_al);
                                break;
                        case    101: //fpg
                                // estructura fpg
                                fread(&faux,1,sizeof(FPG),desktop);
                                strcpy(input,(char *)faux.NombreFpg);
                                strcpy(full,(char *)faux.ActualFile);
                                if ((f=fopen(full,"rb"))!=NULL) {
                                  fclose(f);
                                  v_aux=(byte *)malloc(sizeof(FPG));
                                  if(v_aux!=NULL) {
                                    memcpy(v_aux,&faux,sizeof(FPG));
                                    nueva_ventana_carga(FPG0A,ventana_aux.x,ventana_aux.y);
                                    wvolcado(copia,vga_an,vga_al,v.ptr,v.x,v.y,v.an,v.al,0);
                                    if(!Interpretando) actualiza_caja(0,0,vga_an,vga_al);
                                  }
                                }
                                break;
                        case    102: //prg
                               fread(&iWork,1,4,desktop);
                                if(iWork==0)
                                {
                                        if ((v_prg=(struct tprg*)malloc(sizeof(struct tprg)))!=NULL)
                                        {
                                                fread(v_prg,1,sizeof(struct tprg),desktop);
                                                v_prg->buffer=(byte *)malloc(v_prg->buffer_lon);
                                                fread(v_prg->buffer,1,v_prg->buffer_lon,desktop);

                                                fread(&iWork,1,4,desktop);
                                                v_prg->lptr=v_prg->buffer+iWork;
                                                fread(&iWork,1,4,desktop);
                                                v_prg->vptr=v_prg->buffer+iWork;

                                                nueva_ventana_carga(carga_programa0,ventana_aux.x,ventana_aux.y);

                                                wvolcado(copia,vga_an,vga_al,v.ptr,v.x,v.y,v.an,v.al,0);
                                                if(!Interpretando)
                                                        actualiza_caja(0,0,vga_an,vga_al);
                                        }                                        
										// check if prg on disk is newer than session
										strcpy(pathtmp,v_prg->path);
										strcat(pathtmp,"/");
										strcat(pathtmp,v_prg->filename);

										if(dtime < getFileCreationTime(&pathtmp[0])) {
											v_titulo=v_prg->filename;
											v_texto="File on disk is newer, reload?";
											dialogo(aceptar0);

											if(v_aceptar) {
												strcpy(tipo[0].path,v_prg->path);
												strcpy(input,v_prg->filename);
												
												// close old prg
												cierra_ventana();
												v_terminado = 1;
												
												// load replacement prg							
												abrir_programa();
											}
										}
                                        break;
                                }
                                else
                                {
                                        if(iWork==1)
                                                nueva_ventana_carga(carga_Fonts0,ventana_aux.x,ventana_aux.y);
                                        else
                                        {
                                                if(iWork==2)
                                                {
                                                        fread(&iWork,1,4,desktop);
                                                        fread(&iWork2,1,4,desktop);
                                                        fread(&iWork3,1,4,desktop);
                                                        carga_help(iWork,iWork2,iWork3,ventana_aux.x,ventana_aux.y);
                                                }
                                                else
                                                {
                                                  if (iWork==3) {
                                                        readcalc=(struct _calc *)malloc(sizeof(struct _calc));
                                                        pcalc=NULL;
                                                        if (readcalc!=NULL) {
                                                          fread(readcalc,1,sizeof(struct _calc),desktop);
                                                          nueva_ventana_carga(calc0,ventana_aux.x,ventana_aux.y);
                                                        } else {
                                                          fseek(desktop,sizeof(struct _calc),SEEK_CUR);
                                                        }
                                                  } else {
                                                        v_texto=(char *)texto[45];
                                                        dialogo(err0);
                                                  }
                                                }
                                        }
                                }
                                break;
                        case    104: //fnt
                                // Descargarse
                                fread(Load_FontName,1,14,desktop);
                                fread(Load_FontPathName,1,_MAX_PATH-14,desktop);

                                debugprintf("fontname [%s], fontpathname [%s]\n",Load_FontName,Load_FontPathName);
								fflush(stdout);
								
                                if ((f=fopen(Load_FontPathName,"rb"))!=NULL) {
                                  fclose(f);
                                  nueva_ventana_carga(ShowFont0,ventana_aux.x,ventana_aux.y);
                                  wvolcado(copia,vga_an,vga_al,v.ptr,v.x,v.y,v.an,v.al,0);
                                  if(!Interpretando) actualiza_caja(0,0,vga_an,vga_al);
                                }
                                break;
                        case    105: //pcm
                                OpenDesktopSound(desktop);
                                break;
                        // case 106 (map3d) removed (MODE8/3D map editor deleted)
                        case    107: //mod
                                fread(SongName,1,14,desktop);
                                fread(SongPathName,1,256,desktop);
                                if ((f=fopen(SongPathName,"rb"))!=NULL) {
                                  fclose(f);
                                    OpenDesktopSong();
                                }
                                break;
                }
        }
        fclose(desktop);
        if(Interpretando)
                actualiza_caja(0,0,vga_an,vga_al);
return(1);
}


//-----------------------------------------------------------------------------
//      Load new window (1 on Error)
//-----------------------------------------------------------------------------

int nueva_ventana_carga(voidReturnType init_handler,int nx,int ny)
{

	  byte * ptr;
	  int n,m,x,y,an,al;
	  int vtipo;

	uint32_t colorkey=0;

  if (!ventana[max_windows-1].tipo) {
    addwindow();

    //---------------------------------------------------------------------------
    // Los siguientes valores los debe definir init_handler, valores por defecto:
    //---------------------------------------------------------------------------

    v.orden=siguiente_orden++;
    v.tipo=0;
    v.primer_plano=1;
    v.nombre=(byte *)"?";
    v.titulo=(byte *)"?";
    v.paint_handler=dummy_handler;
    v.click_handler=dummy_handler;
    v.close_handler=dummy_handler;
    v.x=0;
    v.y=0;
    v.an=vga_an;
    v.al=vga_al;
    v._an=0;
    v._al=0;
    v.estado=0;
    v.botones=0;
    v.volcar=0;
    v.items=0;
    v.selected_item=-1;
    v.prg=NULL;

    call((voidReturnType )init_handler);

    if (big) { if (v.an>0) { v.an=v.an*2; v.al=v.al*2; } else v.an=-v.an; }

    an=v.an; al=v.al;

    //---------------------------------------------------------------------------
    // Algoritmo de emplazamiento de ventanas ...
    //---------------------------------------------------------------------------

    if(!VidModeChanged) { x=nx; y=ny; } else emplazar(v.lado*2+1,&x,&y,an,al);

    v.x=x; v.y=y;

    //---------------------------------------------------------------------------
    // Comprueba que si se trata de un mapa no haya otro activado
    //---------------------------------------------------------------------------

    if(VidModeChanged) {
      if (v.tipo>=100 && ventana_aux.primer_plano!=2) {
        v.estado=1; // Se activa
        for (m=1;m<max_windows;m++) if (ventana[m].tipo==v.tipo && ventana[m].estado) {
          ventana[m].estado=0;
          wgra(ventana[m].ptr,ventana[m].an/big2,ventana[m].al/big2,c1,2,2,ventana[m].an/big2-20,7);
          if (text_len(ventana[m].titulo)+3>ventana[m].an/big2-20) {
            wwrite_in_box(ventana[m].ptr,ventana[m].an/big2,ventana[m].an/big2-19,ventana[m].al/big2,4,2,0,ventana[m].titulo,c0);
            wwrite_in_box(ventana[m].ptr,ventana[m].an/big2,ventana[m].an/big2-19,ventana[m].al/big2,3,2,0,ventana[m].titulo,c2);
          } else {
            wwrite(ventana[m].ptr,ventana[m].an/big2,ventana[m].al/big2,2+(ventana[m].an/big2-20)/2,3,1,ventana[m].titulo,c0);
            wwrite(ventana[m].ptr,ventana[m].an/big2,ventana[m].al/big2,2+(ventana[m].an/big2-20)/2,2,1,ventana[m].titulo,c2);
          }
          vtipo=v.tipo; v.tipo=0; vuelca_ventana(m); v.tipo=vtipo;
          break;
        }
      }
    }

    //---------------------------------------------------------------------------
    // Comprueba que si se trata de un menú no este ya generado
    //---------------------------------------------------------------------------
    if ((ptr=(byte *)malloc(an*al))!=NULL) { // Ventana, free en cierra_ventana

      //---------------------------------------------------------------------------
      // Pasa a segundo plano las ventanas que corresponda
      //---------------------------------------------------------------------------
        vtipo=v.tipo; v.tipo=0;

        if(!VidModeChanged) {
          swap(v.an,ventana_aux.an);
          swap(v.al,ventana_aux.al);
          for (n=1;n<max_windows;n++) {
            if (ventana[n].tipo && ventana[n].primer_plano==1) {
              if (colisionan(0,n)) {
                ventana[n].primer_plano=0; vuelca_ventana(n);
              }
            }
          }
          swap(v.an,ventana_aux.an);
          swap(v.al,ventana_aux.al);
        } else  {
          for (n=1;n<max_windows;n++) {
            if (ventana[n].tipo && ventana[n].primer_plano==1) {
              if (colisionan(0,n)) {
                ventana[n].primer_plano=0; vuelca_ventana(n);
              }
            }
          }
        }
        v.tipo=vtipo;

      //---------------------------------------------------------------------------
      // Inicializa la ventana
      //---------------------------------------------------------------------------

      v.ptr=ptr;

		window_surface(an,al,0);
		
      memset(ptr,c0,an*al); if (big) { an/=2; al/=2; }

      wrectangle(ptr,an,al,c2,0,0,an,al);

      wput(ptr,an,al,an-9,2,35);
      wput(ptr,an,al,an-17,2,37);
      wgra(ptr,an,al,c_b_low,2,2,an-20,7);
      if (text_len(v.titulo)+3>an-20) {
        wwrite_in_box(ptr,an,an-19,al,4,2,0,v.titulo,c1);
        wwrite_in_box(ptr,an,an-19,al,3,2,0,v.titulo,c4);
      }
      else {
        wwrite(ptr,an,al,3+(an-20)/2,2,1,v.titulo,c1);
        wwrite(ptr,an,al,2+(an-20)/2,2,1,v.titulo,c4);
      }

      call((voidReturnType )v.paint_handler);

      if(!VidModeChanged) {
        v.primer_plano=ventana_aux.primer_plano;
        v.estado=ventana_aux.estado;
        v.an=ventana_aux.an;
        v.al=ventana_aux.al;
        v._x=ventana_aux._x;
        v._y=ventana_aux._y;
        v._an=ventana_aux._an;
        v._al=ventana_aux._al;
      }

      if (v.tipo>=100 && v.estado==0) {
        wgra(v.ptr,an,al,c1,2,2,an-20,7);
        if (text_len(v.titulo)+3>an-20) {
          wwrite_in_box(v.ptr,an,an-19,al,4,2,0,v.titulo,c0);
          wwrite_in_box(v.ptr,an,an-19,al,3,2,0,v.titulo,c2);
        } else {
          wwrite(v.ptr,an,al,2+(an-20)/2,3,1,v.titulo,c0);
          wwrite(v.ptr,an,al,2+(an-20)/2,2,1,v.titulo,c2);
        }
      }

      if (big) { an*=2; al*=2; }

      if(!Interpretando && exploding_windows) {
        if (v.primer_plano==2) {
			v.exploding=1;
			explode(v.x,v.y,v.an,v.al);
			v.exploding=0;
		} else {
			v.exploding=1;
			explode(x,y,an,al);
			v.exploding=0;
		}
      }

      if (v.primer_plano!=2) {
        if (v.primer_plano==1)
          wvolcado(copia,vga_an,vga_al,ptr,x,y,an,al,0);
        else wvolcado_oscuro(copia,vga_an,vga_al,ptr,x,y,an,al,0);
        volcado_parcial(x,y,an,al);
      } else {
        vuelca_ventana(0);
        volcado_parcial(v.x,v.y,v.an,v.al);
      }

    //---------------------------------------------------------------------------
    // No se pudo abrir la ventana, (no hay memoria o menú duplicado)
    //---------------------------------------------------------------------------

    } else { divdelete(0); return(1); }

  } else return(1);

  return(0);

}


//-----------------------------------------------------------------------------
//      Carga un mapa nuevo (1 si Error)
//-----------------------------------------------------------------------------

int nuevo_mapa_carga(int nx,int ny,char *nombre,byte *mapilla)
{
  int n;

  //1º Pide memoria para un struct tmapa
  if ((v_mapa=(struct tmapa *)malloc(sizeof(struct tmapa)))!=NULL) {
	memset(v_mapa,0,sizeof(struct tmapa));
	
    // 2º Pide memoria para el mapa
    v_mapa->map=mapilla;

    //4º Fija el resto de variables
    memcpy((char *)v_mapa->filename,(char *)nombre,255);
    *v_mapa->path='\0';
    v_mapa->map_an=map_an;
    v_mapa->map_al=map_al;
    v_mapa->TengoNombre=0;
    v_mapa->Codigo=0;
    v_mapa->descripcion[0]=0;
    for (n=0;n<512;n++) v_mapa->puntos[n]=-1;
    nueva_ventana_carga(mapa0,nx,ny);

    return(0);

  } else { v_texto=(char *)texto[45]; dialogo(err0); }

  return(1);
}

// nuevo_mapa3d_carga removed (MODE8/3D map editor deleted)

void test_cursor(void);

void carga_programa0(void)
{

  v.tipo=102;

  v.prg=v_prg;

  if (v.prg->an<4*big2) v.prg->an=4*big2;
  if (v.prg->al<2*big2) v.prg->al=2*big2;

  v.an=(4+8)*big2+editor_font_an*v_prg->an;
  v.al=(12+16)*big2+editor_font_al*v_prg->al;

  if (v.an>vga_an) {
    v.prg->an=(vga_an-12*big2)/editor_font_an; // Calcula tamaño (en chr) maximizada
    v.an=(4+8)*big2+editor_font_an*v.prg->an;
    ventana_aux.an=v.an;
  }

  if (v.al>vga_al) {
    v.prg->al=(vga_al-28*big2)/editor_font_al;
    v.al=(12+16)*big2+editor_font_al*v.prg->al;
    ventana_aux.al=v.al;
  }

  if (big)
  {
    if (v.an&1) v.an++;
    if (v.al&1) v.al++;
    v.an=-v.an; // Para indicar que no se multiplique la ventana por 2
  }

  v.titulo=(byte *)v_prg->filename;
  v.nombre=(byte *)v_prg->filename;

  v.paint_handler=programa1;
  v.click_handler=programa2;
  v.close_handler=programa3;
  v.volcar=2;

  test_cursor();
}

extern struct t_listbox lfontsizes;

void carga_Fonts0(void)
{
        Load_Font_session(desktop);
}

void vuelca_help(void);
void barra_vertical(void);

void carga_help(int n,int helpal,int helpline,int x1,int x2)
{
  FILE * f;
  byte * p;
  int m_back;

  if (helpidx[n*2] && helpidx[n*2+1]) {
    if((f=fopen("help/help.div","rb"))!=NULL) {
      fseek(f,helpidx[n*2],SEEK_SET);
      if ((h_buffer=(byte*)malloc(helpidx[n*2+1]))!=NULL) {
        if ((help_buffer=(byte*)malloc(helpidx[n*2+1]+1024))!=NULL) {
          if (i_back!=a_back) {
            m_back=a_back-2; if (m_back<0) m_back=62;
            if (backto[m_back]==help_item && backto[m_back+1]==-1) backto[m_back+1]=help_l;
          } help_item=n;
          backto[a_back]=n; backto[a_back+1]=-1;
          f_back=a_back=(a_back+2)%64;
          if (i_back==f_back) i_back=(i_back+2)%64;
          fread(h_buffer,1,helpidx[n*2+1],f);
          p=h_buffer; while (*p!='}') p++; *p=0;
          strcpy((char *)help_title,(char *)h_buffer);
          help_an=(vga_an-12*big2-1)/font_an;
          if (help_an>120) help_an=120;
          help_al=(vga_al/2-12*big2-1)/font_al;
          help_al=helpal;

          help_l=0;
          tabula_help(p+1,help_buffer,helpidx[n*2+1]-(p+1-h_buffer));
          nueva_ventana_carga(help0,x1,x2);

          for (n=0;n<helpline;n++) {
            if (help_l+help_al<help_lines) { while (*(help_line++)); help_l++; }
          }

          if(v.primer_plano==2) { swap(v.an,v._an); swap(v.al,v._al); }
          vuelca_help(); barra_vertical();
          if(v.primer_plano==2) { swap(v.an,v._an); swap(v.al,v._al); }

          vuelca_ventana(0);

        } free(h_buffer);
      } fclose(f);
    }
  }

}

