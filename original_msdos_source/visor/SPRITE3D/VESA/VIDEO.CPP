#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include <time.h>
#include "svga.h"

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Definici¢n de variables globales
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

char *vga=(char *)0xa0000;
extern char *dir_fondo;
char dac[768];

extern lf3dl_init laal_struct;

extern int inicio_aviso;
extern int tiempo_aviso;

extern int inicio_tanteo;
extern int tiempo_tanteo;

extern int inicio_marcador;
extern int tiempo_marcador;



//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Activa un modo de video de la VGA
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void set_video_mode(int);
#pragma aux set_video_mode=	"int 10h" \
				parm [eax];

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Inicializa el modo de video
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int video_on(void)
{
  if (!laal_struct.modo_vesa)
    {
    set_video_mode(0x013);
    laal_struct.bpp=8;
    laal_struct.ancho_pantalla=320;
    laal_struct.alto_pantalla=200;
    return(0);
  }
  if (set_vesa_mode(modo_de_video())==-1) {
    printf("\nModo de video no disponible:\n");
    printf("\tbpp=%d\n",laal_struct.bpp);
    printf("\tancho_pantalla=%d\n",laal_struct.ancho_pantalla);
    printf("\talto_pantalla=%d\n",laal_struct.alto_pantalla);
    return(-1);
  }
  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Finaliza el modo de video
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void video_off(void)
{
  set_video_mode(3);
}


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Activa el modo de video de la SVGA
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int set_vesa_mode(int modo)
{
int version=SV_init();

  if (modo==-1) return(-1);
  if (version<0x102) {
    printf("Se requiere Vesa 1.2 o superior\n");
    return(-1);
  }
  if (version<0x200)
  {
    laal_struct.vesa_20=0;
    SV_setMode(modo);
  }
  else
  {
    laal_struct.vesa_20=1;
    modo|=vbeLinearBuffer;
    VBE_getModeInfo(modo,&mi_modo);
    SV_setMode(modo);
    vga=(char *)videoMem;
  }
  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Espera el retrazo vertical
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void retrazo()
{
  while (inp(0x3da)&8);
  while ((inp(0x3da)&8)==0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Coloca el dac de video
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void set_dac()
{
  short n=0;
  retrazo();
  outp(0x3c8,0);
  do {
    outp(0x3c9,dac[n++]/4);
  } while (n<768);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Hace un volcado lineal y borra la pantalla
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void volcado_lineal(char *, char *, int);
#pragma aux volcado_lineal= \
        "push ebx" \
        "mov eax,73737373h" \
        "mov ebx,ecx" \
        "shr ebx,8" \
"bucle_volcado:" \
        "push esi" \
        "mov ecx,64" \
	"rep movsd" \
        "mov esi,edi" \
        "pop edi" \
        "mov ecx,64" \
        "rep stosd" \
        "xchg esi,edi" \
        "dec ebx" \
        "jnz bucle_volcado" \
        "pop ebx" \
	parm [edi] [esi] [ecx] \
        modify [eax] \

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Borra la copia de pantalla
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void borrar_copia()
{
  int factor;

  switch(laal_struct.bpp) {
    case 8:  factor=1;
	     break;
    case 15: factor=2;
	     break;
    case 16: factor=2;
	     break;
    case 24: factor=3;
	     break;
    case 32: factor=4;
	     break;
  }
  memset(laal_struct.copia,0,laal_struct.ancho_pantalla*
	 laal_struct.alto_pantalla*factor);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Vuelca de copia a pantalla
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void volcar()
{
  void *fuente;
  long pagina=0;
  long screen_size;

  if (tiempo_marcador) {
    if ((clock()-inicio_marcador)<tiempo_marcador) pinta_marcador();
    else tiempo_marcador=0;
  }
  else if (tiempo_tanteo) {
    if ((clock()-inicio_tanteo)<tiempo_tanteo) pinta_tanteo();
    else tiempo_tanteo=0;
  }
  else if (tiempo_aviso) {
    if ((clock()-inicio_aviso)<tiempo_aviso) pinta_aviso();
    else tiempo_aviso=0;
  }

  switch(laal_struct.bpp) {
    case 8:	screen_size=laal_struct.ancho_pantalla*laal_struct.alto_pantalla;
		break;
    case 15:	screen_size=laal_struct.ancho_pantalla*laal_struct.alto_pantalla*2;
		break;
    case 16:	screen_size=laal_struct.ancho_pantalla*laal_struct.alto_pantalla*2;
		break;
    case 24:	screen_size=laal_struct.ancho_pantalla*laal_struct.alto_pantalla*3;
		break;
    case 32:	screen_size=laal_struct.ancho_pantalla*laal_struct.alto_pantalla*4;
  }

}

