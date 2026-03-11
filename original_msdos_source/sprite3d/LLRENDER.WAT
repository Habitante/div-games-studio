#include <mem.h>
#include <math.h>
#include <graph.h>
#include "llrender.hpp"
#include "global.hpp"
#include "t.h"

long complex_x_ini,complex_x_fin;
long complex_y_ini,complex_y_fin;

short *tabla_rgba;
long num_llrenders=0;

float LMax=1.8;
float LMin=0.7;

extern "C" {
  extern int direccion_textura;
  extern int clock1,clock2;
}

#define PI ((float)3.141592)
float filtro_iluminacion[256];

long bilinear_mul[256][256];
long bilinear_mul_256[256][256];
int u_1,v_1,u_0,v_0;
float fu,fv,inversa;

inline long ajuste_flotante(float x)
{
  return ((schop(x*65536.0)+0xffff)>>16);
}

inline long ajuste_entero(long x)
{
  x += 0xffff;
  return (x>>16);
}

void (*nucleo_texturas)(int v, int u, int du, int dv, int ancho, short *destino);

int clocks();
#pragma aux clocks =  \
        "push edx"    \
        "db 0fh,31h"  \
        "pop edx"     \
        value [eax]   \

void inicio_division(float *);
#pragma aux inicio_division = \
        "fld1"                \
        "fdiv dword ptr[EAX]" \
        parm caller [EAX]     \

void fin_division();
#pragma aux fin_division =    \
        "fstp dword ptr[inversa]" \

void asignacion_u_v();
#pragma aux asignacion_u_v =      \
        "fld dword ptr[inversa]"  \
        "fld st(0)"               \
        "fmul dword ptr[fu]"      \
        "fistp dword ptr[u_0]"    \
        "fmul dword ptr[fv]"      \
        "fistp dword ptr[v_0]"    \

void lf_set(void *, long, long);
#pragma aux lf_set = \
	"rep stosw" \
        parm caller [edi] [eax] [ecx]\
	modify exact [edi eax ecx] \


//=============================================================================

short *destino;
short color_b;
int r,g,b,intensidad,alfa;

void nucleo1();
#pragma aux nucleo1 = \
        "pushad" \
        "mov edi,destino" \
        "mov esi,tabla_rgba" \
        "xor edx,edx" \
        "mov ebx,r" \
        "shr ebx,3" \
        "and ebx,7e0h" \
        "mov ax,color_b" \
        "and eax,0f800h" \
        "add eax,ebx" \
        "add eax,alfa" \
        "mov dx,[esi+eax*2]" \
        "shl edx,5" \
        "mov ebx,g" \
        "shr ebx,3" \
        "and ebx,7e0h" \
        "mov ax,color_b" \
        "and eax,07c0h" \
        "shl eax,5" \
        "add eax,ebx" \
        "add eax,alfa" \
        "add dx,[esi+eax*2]" \
        "shl edx,6" \
        "mov ebx,b" \
        "shr ebx,3" \
        "and ebx,7e0h" \
        "mov ax,color_b" \
        "and eax,1fh" \
        "shl eax,11" \
        "add eax,ebx" \
        "add eax,alfa" \
        "add dx,[esi+eax*2]" \
        "mov [edi],dx" \
        "popad" \

//=============================================================================

llrender::llrender(sprite3d *newcamera) //Video *vid)
{
  short alfa,color,gouraud;

  dist_max=27000;
//  video=vid;
  mmdist[0]=dist_max*0.33;
  mmdist[1]=dist_max*0.66;
  for (int i=0;i<256;i++)
    for (int j=0;j<256;j++)
    {
      bilinear_mul[i][j]=i*j;
      bilinear_mul_256[i][j]=i*j/256;
    }
// Datos cambiantes
  ancho_pantalla=newcamera->ancho; //video->Ancho;
  alto_pantalla=newcamera->alto; //video->Alto;
  fancho_pantalla=ancho_pantalla;
  falto_pantalla = alto_pantalla;
  copia=newcamera->buffer; //video->Buffer;
  vancho=newcamera->ancho*2;
  fin_copia=copia+vancho*alto_pantalla;
  for (i=0;i<128;i++) {
    filtro_iluminacion[i]=128*(sin((float)i*PI/256+3*PI/2)+1.0);
//    filtro_iluminacion[i]=64+64*(sin((float)i*PI/256+3*PI/2)+1.0);
//    printf("Filtro de iluminacion de %d: %f\n",i,filtro_iluminacion[i]);
  }
  for (i=128;i<256;i++) {
    filtro_iluminacion[i]=128+(127-filtro_iluminacion[255-i])*.3;
//    filtro_iluminacion[i]=255-filtro_iluminacion[255-i];
//    printf("Filtro de iluminacion de %d: %f\n",i,filtro_iluminacion[i]);
  }
  for (i=0;i<256;i++)
    filtro_iluminacion[i]+=128;

  if (num_llrenders==0) {

    tabla_rgba=(short *)malloc(65536*2);

    for (i=0;i<65536;i++) {

      alfa=(short)(i&0x1f);         //0-32 Nivel de iluminacion
      color=(short)((i>>10)&0x3e);  //0-64 Color de textura
      gouraud=(short)((i>>5)&0x3f); //0-64 Color de iluminacion

    	float t = (LMax-LMin)*((float)alfa/32.0) + LMin;
      int   c = (int)(t*(float)color);
      tabla_rgba[i]=(short)( c > 63 ? 31 : c>>1 );

    //  tabla_rgba[i]=(((i>>10)&0x3e)*(31-alfa)+((i>>5)&0x3f)*alfa)/64;
    }
  }
  num_llrenders++;
}

llrender::~llrender()
{
  num_llrenders--;
  if (num_llrenders==0) {
    free(tabla_rgba);
  }
}

void llrender::render(lptface faces,int nfaces)
{
  int i=0;

  do {
    clipping_3d_near(&faces[i]);
  } while (++i<nfaces);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Pinta un triangulo en el buffer de video
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::pinta_triangulo(lptface face)
{
int mmlvl;
int zmed;
int color;

  if (face->tipo_map==-1) return;

  if (face->v1->estado==0) {
    face->v1->px=ajuste_flotante(face->v1->fpx);
    face->v1->py=ajuste_flotante(face->v1->fpy);
    face->v1->estado=1;
  }

  if (face->v2->estado==0) {
    face->v2->px=ajuste_flotante(face->v2->fpx);
    face->v2->py=ajuste_flotante(face->v2->fpy);
    face->v2->estado=1;
  }

  if (face->v3->estado==0) {
    face->v3->px=ajuste_flotante(face->v3->fpx);
    face->v3->py=ajuste_flotante(face->v3->fpy);
    face->v3->estado=1;
  }

  //---------------------------------------------------------------------------
  //  Calculo la bounding box del objeto en pantalla
  //---------------------------------------------------------------------------

  if (face->v1->px<complex_x_ini) complex_x_ini=face->v1->px;
  if (face->v2->px<complex_x_ini) complex_x_ini=face->v2->px;
  if (face->v3->px<complex_x_ini) complex_x_ini=face->v3->px;
  if (face->v1->px>complex_x_fin) complex_x_fin=face->v1->px+1;
  if (face->v2->px>complex_x_fin) complex_x_fin=face->v2->px+1;
  if (face->v3->px>complex_x_fin) complex_x_fin=face->v3->px+1;
  if (face->v1->py<complex_y_ini) complex_y_ini=face->v1->py;
  if (face->v2->py<complex_y_ini) complex_y_ini=face->v2->py;
  if (face->v3->py<complex_y_ini) complex_y_ini=face->v3->py;
  if (face->v1->py>complex_y_fin) complex_y_fin=face->v1->py+1;
  if (face->v2->py>complex_y_fin) complex_y_fin=face->v2->py+1;
  if (face->v3->py>complex_y_fin) complex_y_fin=face->v3->py+1;

  // Si el poligono no apunta a la c mara no se ve y por tanto no se pinta
  if( Cara_Oculta(face) ) return;
  #ifdef DEBUG
    line(face->v1->px, face->v1->py, face->v2->px, face->v2->py, 0xFFFF);
    line(face->v2->px, face->v2->py, face->v3->px, face->v3->py, 0xFFFF);
    line(face->v3->px, face->v3->py, face->v1->px, face->v1->py, 0xFFFF);
  #else

  if (face->tipo_map==WIRE) {
    color=(schop(face->material->dr*65536.0)&0xf800)+
          (schop(face->material->dg*2048.0)&0x07e0)+
          (schop(face->material->db*32.0)&0x1f);
    line(face->v1->px, face->v1->py, face->v2->px, face->v2->py, color);
    line(face->v2->px, face->v2->py, face->v3->px, face->v3->py, color);
    line(face->v3->px, face->v3->py, face->v1->px, face->v1->py, color);
    return;
  }

  zmed=schop(face->v1->fpz+face->v2->fpz+face->v3->fpz)>>2;

  //  Calculo el Mipmap asociado a la textura
  if(zmed<mmdist[0])
    mmlvl=0;
  else
    if(zmed<mmdist[1])
      mmlvl=1;
    else
      if(zmed<dist_max)
	      mmlvl=2;
      else
	      mmlvl=3;


  switch (face->tipo_map) {
    case FLAT:
      Pinta_Triangulo(face,mmlvl,T_FLAT);
      break;
    case ANTIALIAS:
      Pinta_Triangulo(face,mmlvl,T_FLAT);
      break;
    case GOURAUD:
      Pinta_Triangulo(face,mmlvl,T_GOURAUD);
      break;
    case TEXTURA:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA);
      break;
    case TEXT_GOUR:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_GOURAUD);
      break;
    case TEXT_ILUM:
//      Pinta_Text_Ilum(face,mmlvl);
//      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_TRANSLUCIDO);
      break;
    case TEXT_PER:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_PERSPECTIVA);
      break;
    case TEXT_PER_TRANS:
//      Pinta_Text_Persp_Translucido(face,mmlvl);
      break;
    case TEXT_TRANS:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_TRANSLUCIDO);
      break;
    case SOMBRA:
      Pinta_Triangulo(face,mmlvl,T_FLAT);
      break;
    case TRANS_PER:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_PERSPECTIVA | T_MASCARA);
      break;
    case TEXT_BILINEAR:
//      Pinta_Text_Persp_Bilinear(face,mmlvl);
      break;
    case TEXTURA_Z_CONST:
//      Pinta_Text_Persp_Z(face,mmlvl);
      break;
    case TRANSPARENTE:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_MASCARA);
      break;
    case TRANSPARENTE_MASCARA:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_TRANSLUCIDO | T_MASCARA);
      break;
    case FLAT_TRANS:
      Pinta_Triangulo(face,mmlvl,T_FLAT | T_TRANSLUCIDO);
      break;
    case GOUR_MASK:
      Pinta_Triangulo(face,mmlvl,T_TEXTURA | T_MASCARA | T_GOURAUD);
      break;
  }

  #endif
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Recorrido de la seccion derecha
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Seccion_Derecha(void)
{
  lptvertex v1=array_derecho[seccion_derecha];
  lptvertex v2=array_derecho[seccion_derecha-1];
  int comprobacion;

  int altura=v2->py-v1->py+1;
  if (altura==0) return(0);
  float faltura=v2->fpy-v1->fpy+1;

  if (v1->fpx<v2->fpx) comprobacion=1;
  else comprobacion=0;

  //Calculo de las deltas

  dx_der=(float)((((v2->fpx-v1->fpx)*65536.0)+32766)/faltura);

  if (comprobacion) {
    x_der=(v1->fpx*65536.0)+dx_der;
  }
  else {
    x_der=(v1->fpx*65536.0);
  }

  altura_seccion_derecha=altura;
  return(altura);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Recorrido de la seccion izquierda
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Seccion_Izquierda(void)
{
  lptvertex v1=array_izquierdo[seccion_izquierda];
  lptvertex v2=array_izquierdo[seccion_izquierda-1];
  int comprobacion;

  int altura=v2->py-v1->py+1;
  float inv_altura=65536.0/(float)altura;
//  if (altura==0) return(0);

  if (v1->px>v2->px) comprobacion=1;
  else comprobacion=0;

  //Calculo de las deltas

  dx_izq=(((v2->px-v1->px)<<16)+32766)/altura;
  x_izq=(v1->px<<16);
  if (comprobacion) x_izq+=dx_izq;

  du_izq=(v2->ru-v1->ru)*inv_altura;
//  du_izq=((v2->ru-v1->ru)*65536)/altura;
  u_izq=v1->ru*65536;
  if (comprobacion) u_izq+=du_izq;

  dv_izq=(v2->rv-v1->rv)*inv_altura;
//  dv_izq=((v2->rv-v1->rv)*65536)/altura;
  v_izq=v1->rv*65536;
  if (comprobacion) v_izq+=dv_izq;

  if (seccion_izquierda==2) altura--;
  altura_seccion_izquierda=altura;
  return(altura);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Recorrido de la seccion izquierda
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Seccion_Izquierda_Flat(void)
{
  lptvertex v1=array_izquierdo[seccion_izquierda];
  lptvertex v2=array_izquierdo[seccion_izquierda-1];

  int altura=v2->py-v1->py+1;
//  if (altura==0) return(0);

  //Calculo de las deltas
  dx_izq=(((v2->px-v1->px)<<16)+32766)/altura;
  x_izq=v1->px<<16;
  if (v1->px>v2->px) x_izq+=dx_izq;

  if (seccion_izquierda==2) altura--;
  altura_seccion_izquierda=altura;
  return(altura);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Recorrido de la seccion izquierda con gouraud
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Seccion_Izquierda_Gouraud(void)
{
  lptvertex v1=array_izquierdo[seccion_izquierda];
  lptvertex v2=array_izquierdo[seccion_izquierda-1];
  int comprobacion;

  int altura=v2->py-v1->py+1;
  float inv_altura=65536.0/(v2->fpy-v1->fpy+1);

  if (v1->fpx>v2->fpx) comprobacion=1;
  else comprobacion=0;

  //Calculo de las deltas

  dx_izq=(long)((((v2->fpx-v1->fpx)*65536.0)+32766.0)/(float)altura);
  x_izq=(v1->fpx*65536);
  if (comprobacion) x_izq+=dx_izq;

  du_izq=(v2->ru-v1->ru)*inv_altura;
//  du_izq=((v2->ru-v1->ru)*65536)/altura;
  u_izq=v1->ru*65536;
  if (comprobacion) u_izq+=du_izq;

  dv_izq=(v2->rv-v1->rv)*inv_altura;
//  dv_izq=((v2->rv-v1->rv)*65536)/altura;
  v_izq=v1->rv*65536;
  if (comprobacion) v_izq+=dv_izq;

  dr_izq=((v2->r-v1->r)*256)/altura;
  r_izq=v1->r*256;
  if (comprobacion) r_izq+=dr_izq;
  dg_izq=((v2->g-v1->g)*256)/altura;
  g_izq=v1->g*256;
  if (comprobacion) g_izq+=dg_izq;
  db_izq=((v2->b-v1->b)*256)/altura;
  b_izq=v1->b*256;
  if (comprobacion) b_izq+=db_izq;

  altura_seccion_izquierda=altura;
  return(altura);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Recorrido de la seccion izquierda con gouraud y perspectiva
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Seccion_Izquierda_Z(void)
{
  lptvertex v1=array_izquierdo[seccion_izquierda];
  lptvertex v2=array_izquierdo[seccion_izquierda-1];
  int comprobacion;

  int altura=v2->py-v1->py+1;
  float inv_altura=65536.0/(float)altura;

  //Calculo de las deltas

  dx_izq=(((v2->px-v1->px)<<16)+32766)/altura;
  if (v1->px>v2->px) {
    comprobacion=1;
    x_izq=((v1->px)<<16)+dx_izq;
  }
  else {
    comprobacion=0;
    x_izq=v1->px<<16;
  }

  dr_izq=((v2->r-v1->r)*256)/altura;
  r_izq=v1->r*256;
  dg_izq=((v2->g-v1->g)*256)/altura;
  g_izq=v1->g*256;
  db_izq=((v2->b-v1->b)*256)/altura;
  b_izq=v1->b*256;

  duz_izq=(v2->ru-v1->ru)*inv_altura;
  uz_izq=v1->ru*65536;
  if (comprobacion) uz_izq+=duz_izq;
  dvz_izq=(v2->rv-v1->rv)*inv_altura;
  vz_izq=v1->rv*65536;
  if (comprobacion) vz_izq+=dvz_izq;
  dunoz_izq=(1/v2->fpz-1/v1->fpz)/(float)altura;
  unoz_izq=1/v1->fpz;
  if (comprobacion) unoz_izq+=dunoz_izq;

  altura_seccion_izquierda=altura;
  return(altura);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Recorrido de la seccion derecha
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Seccion_Derecha_Z(void)
{
  lptvertex v1=array_derecho[seccion_derecha];
  lptvertex v2=array_derecho[seccion_derecha-1];
  int comprobacion;

  int altura=v2->py-v1->py+1;
  float inv_altura=65536.0/(float)altura;

  if (v1->px<v2->px) comprobacion=1;
  else comprobacion=0;

  //Calculo de las deltas

  dx_der=(((v2->px-v1->px)<<16)+32766)/altura;

  if (comprobacion)
    x_der=(v1->px<<16)+dx_der;
  else
    x_der=(v1->px<<16);

  duz_der=(v2->ru-v1->ru)*inv_altura;
  uz_der=v1->ru*65536;
  if (comprobacion) uz_der+=duz_der;
  dvz_der=(v2->rv-v1->rv)*inv_altura;
  vz_der=v1->rv*65536;
  if (comprobacion) vz_der+=dvz_der;
  dunoz_der=(1/v2->fpz-1/v1->fpz)/(float)altura;
  unoz_der=1/v1->fpz;
  if (comprobacion) unoz_der+=dunoz_der;

  altura_seccion_derecha=altura;
  return(altura);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura sobre un triangulo con gouraud
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
/*
void llrender::Pinta_Text_Ilum(lptface face,int mmlevel)
{
  int altura;
  float faltura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla, *destino;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto =mat->textura->Alto[mmlevel];
  short *dir_textura=(short *)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=(float)(ancho-1)+0.9999;
  float altot=(float)(ancho-1)+0.9999;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;
  short color;
  long mascara=(ancho-1)*65536;
  long rotacion_mascara;
  int i1,i2,i3;
  float dxdy,prestep;
  int preincremento;

  i1=(long)v1->intensidad;
  i2=(long)v2->intensidad;
  i3=(long)v3->intensidad;
  if (i1<0) i1=0;
  if (i2<0) i2=0;
  if (i3<0) i3=0;
  if (i1>255) i1=255;
  if (i2>255) i2=255;
  if (i3>255) i3=255;
  int intensidad=(filtro_iluminacion[i1]+filtro_iluminacion[i2]+filtro_iluminacion[i3])/3;

  switch (ancho) {
    case 8:	rotacion_mascara=13;
		break;
    case 16:	rotacion_mascara=12;
		break;
    case 32:	rotacion_mascara=11;
		break;
    case 64:	rotacion_mascara=10;
		break;
    case 128:	rotacion_mascara=9;
		break;
    case 256:	rotacion_mascara=8;
		break;
    default:	rotacion_mascara=8;
		break;
  }

  v1->rv=v1->v*altot; v2->rv=v2->v*altot; v3->rv=v3->v*altot;
  v1->ru=v1->u*anchot; v2->ru=v2->u*anchot; v3->ru=v3->u*anchot;

  v1->r=filtro_iluminacion[i1];
  if (v1->r>255) v1->r=255;
  if (v1->r<0) v1->r=0;
  v1->g=filtro_iluminacion[i1];
  if (v1->g>255) v1->g=255;
  if (v1->g<0) v1->g=0;
  v1->b=filtro_iluminacion[i1];
  if (v1->b>255) v1->b=255;
  if (v1->b<0) v1->b=0;

  v2->r=filtro_iluminacion[i2];
  if (v2->r>255) v2->r=255;
  if (v2->r<0) v2->r=0;
  v2->g=filtro_iluminacion[i2];
  if (v2->g>255) v2->g=255;
  if (v2->g<0) v2->g=0;
  v2->b=filtro_iluminacion[i2];
  if (v2->b>255) v2->b=255;
  if (v2->b<0) v2->b=0;

  v3->r=filtro_iluminacion[i3];
  if (v3->r>255) v3->r=255;
  if (v3->r<0) v3->r=0;
  v3->g=filtro_iluminacion[i3];
  if (v3->g>255) v3->g=255;
  if (v3->g<0) v3->g=0;
  v3->b=filtro_iluminacion[i3];
  if (v3->b>255) v3->b=255;
  if (v3->b<0) v3->b=0;

  //Ajustamos los vertices al pixel m s pr¢ximo

  v1->py=ajuste_flotante(v1->fpy);
  v2->py=ajuste_flotante(v2->fpy);
  v3->py=ajuste_flotante(v3->fpy);

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->fpy > v2->fpy) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->fpy > v3->fpy) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->fpy > v3->fpy) { vertice=v2; v2=v3; v3=vertice; }

  //Calculamos la longitud de la linea m s larga
//        long scanlines = ajuste(v2.y) - ajuste(v1.y);
//        if(scanlines <= 0) return;              // Section had zero height

//        long height = v2.y - v1.y;
//        long dxdy = ((v2.x - v1.x) << 16) / height;

//        long prestep = (ajuste(v1.y) << 16) - v1.y;
//        long left_x = v1.x + ((prestep * dxdy) >> 16);

  altura=v3->py-v1->py;
  if (altura<=0) return;

      faltura=(v3->fpy-v1->fpy);
      dxdy=(v3->fpx-v1->fpx)/faltura;
      dx_izq=(long)(dxdy*65536.0);
//      prestep=0;//(float)v1->py-v1->fpy;
      prestep=(float)v1->py-v1->fpy;
      x_izq=(long)((v1->fpx+prestep*dxdy)*65536.0);

  faltura=(v3->fpy-v1->fpy);
  temp=(v2->fpy-v1->fpy)/faltura;
  largo=temp*(v3->fpx-v1->fpx)+(v1->fpx-v2->fpx);
  if (largo==0) return;

  if (largo<0) {
    seccion_derecha=2;
    seccion_izquierda=1;
    altura_seccion_izquierda=v3->py-v1->py;
    if (altura_seccion_izquierda<=0)
      return;
    else {
      faltura=(v3->fpy-v1->fpy);
      dxdy=(v3->fpx-v1->fpx)/faltura;
      dx_izq=(long)(dxdy*65536.0);
//      prestep=0;//(float)v1->py-v1->fpy;
      prestep=(float)v1->py-v1->fpy;
      x_izq=(long)((v1->fpx+prestep*dxdy)*65536.0);
      //Calculo de las deltas
      du_izq=(long)(65536.0*(v3->ru-v1->ru)/faltura);
      u_izq=(long)(v1->ru*65536.0);
      dv_izq=(long)(65536.0*(v3->rv-v1->rv)/faltura);
      v_izq=(long)(v1->rv*65536.0);
      dr_izq=(long)(((v3->r-v1->r)*256.0)/faltura);
      r_izq=(long)(v1->r*256.0);
      dg_izq=(long)(((v3->g-v1->g)*256.0)/faltura);
      g_izq=(long)(v1->g*256.0);
      db_izq=(long)(((v3->b-v1->b)*256.0)/faltura);
      b_izq=(long)(v1->b*256.0);
    }
    //
    altura_seccion_derecha=v2->py-v1->py;
    if (altura_seccion_derecha<=0) {
      altura_seccion_derecha=v3->py-v2->py;
      if (altura_seccion_derecha<=0) return;
      else {
        //Calculo de las deltas
        dx_der=(long)(((v3->fpx-v2->fpx)*65536.0)/(v3->fpy-v2->fpy));
        faltura=(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)/faltura;
//        prestep=0;//(float)v2->py-v2->fpy;
        prestep=(float)v2->py-v2->fpy;
        x_der=(long)((v2->fpx+prestep*dxdy)*65536.0);
//        x_der=(long)(v2->fpx*65536.0);
      }
    }
    else {
      //Calculo de las deltas
      dx_der=(long)(((v2->fpx-v1->fpx)*65536.0)/(v2->fpy-v1->fpy));
      faltura=(v2->fpy-v1->fpy);
      dxdy=(v2->fpx-v1->fpx)/faltura;
//      prestep=0;//(float)v1->py-v1->fpy;
      prestep=(float)v1->py-v1->fpy;
      x_der=(long)((v1->fpx+prestep*dxdy)*65536.0);
//      x_der=(long)(v1->fpx*65536.0);
    }
    //
  }
  else {
    seccion_izquierda=2;
    seccion_derecha=1;

    //
    altura_seccion_derecha=v3->py-v1->py;
    if (altura_seccion_derecha<=0) return;
    else {
      //Calculo de las deltas
      dx_der=(long)(((v3->fpx-v1->fpx)*65536.0)/(v3->fpy-v1->fpy));
      faltura=(v3->fpy-v1->fpy);
      dxdy=(v3->fpx-v1->fpx)/faltura;
//      prestep=0;//(float)v1->py-v1->fpy;
      prestep=(float)v1->py-v1->fpy;
      x_der=(long)((v1->fpx+prestep*dxdy)*65536.0);
//      x_der=(long)(v1->fpx*65536.0);
    }
    //
    altura_seccion_izquierda=v2->py-v1->py;
    if (altura_seccion_izquierda<=0) {
      seccion_izquierda--;
      altura_seccion_izquierda=v3->py-v2->py;
      if (altura_seccion_izquierda<=0)
        return;
      else {
        faltura=(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)/faltura;
        dx_izq=(long)(dxdy*65536.0);
//        prestep=0;//(float)v2->py-v2->fpy;
        prestep=(float)v2->py-v2->fpy;
        x_izq=(long)((v2->fpx+prestep*dxdy)*65536.0);
        //Calculo de las deltas
        du_izq=(long)(65536.0*(v3->ru-v2->ru)/faltura);
        u_izq=(long)(v2->ru*65536.0);
        dv_izq=(long)(65536.0*(v3->rv-v2->rv)/faltura);
        v_izq=(long)(v2->rv*65536.0);
        dr_izq=(long)(((v3->r-v2->r)*256.0)/faltura);
        r_izq=(long)(v2->r*256.0);
        dg_izq=(long)(((v3->g-v2->g)*256.0)/faltura);
        g_izq=(long)(v2->g*256.0);
        db_izq=(long)(((v3->b-v2->b)*256.0)/faltura);
        b_izq=(long)(v2->b*256.0);
      }
    }
    else {
      faltura=(v2->fpy-v1->fpy);
      dxdy=(v2->fpx-v1->fpx)/faltura;
      dx_izq=(long)(dxdy*65536.0);
//      prestep=0;//(float)v1->py-v1->fpy;
      prestep=(float)v1->py-v1->fpy;
      x_izq=(long)((v1->fpx+prestep*dxdy)*65536.0);
      //Calculo de las deltas
      du_izq=(long)(65536.0*(v2->ru-v1->ru)/faltura);
      u_izq=(long)(v1->ru*65536.0);
      dv_izq=(long)(65536.0*(v2->rv-v1->rv)/faltura);
      v_izq=(long)(v1->rv*65536.0);
      dr_izq=(long)(((v2->r-v1->r)*256.0)/faltura);
      r_izq=(long)(v1->r*256.0);
      dg_izq=(long)(((v2->g-v1->g)*256.0)/faltura);
      g_izq=(long)(v1->g*256.0);
      db_izq=(long)(((v2->b-v1->b)*256.0)/faltura);
      b_izq=(long)(v1->b*256.0);
    }
  }

  dudx=(long)(65536.0*(temp*(v3->ru-v1->ru)+(v1->ru-v2->ru))/largo);
  dvdx=(long)(65536.0*(temp*(v3->rv-v1->rv)+(v1->rv-v2->rv))/largo);

  drdx=(long)( 256.0*((temp*(v3->r-v1->r)+(v1->r-v2->r))/largo) );
  dgdx=(long)( 256.0*((temp*(v3->g-v1->g)+(v1->g-v2->g))/largo) );
  dbdx=(long)( 256.0*((temp*(v3->b-v1->b)+(v1->b-v2->b))/largo) );

  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=ajuste_entero(x_izq);
    ancho=ajuste_entero(x_der)-x1;

    if (ancho>0) {
      destino=ptr_pantalla+x1;
      preincremento=(x1<<16)-x_izq;
      u=u_izq+((preincremento*dudx)>>16);
      v=v_izq+((preincremento*dvdx)>>16);
      do {
	      color=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)];
        _asm {
          pushad
          mov edi,destino
          mov ebx,intensidad
          mov edx,ebx
          shl ebx,8
          and ebx,0f800h
          shl edx,3
          and edx,07e0h

          mov ax,color
          and eax,0f800h
          add eax,ebx
          sub eax,8000h
          jns label0
          xor eax,eax
          jmp label1
        label0:
          test eax,10000h
          jz label1
          mov eax,0f800h
        label1:
          mov si,color
          and esi,01fh
          mov ebx,intensidad
          shr ebx,3
          and ebx,01fh
          add esi,ebx
          sub esi,16
          jns label2
          xor esi,esi
          jmp label3
        label2:
          test esi,20h
          jz label3
          mov esi,01fh
        label3:
          add eax,esi
          mov si,color
          and esi,07e0h
          add esi,edx
          sub esi,1024
          jns label4
          xor esi,esi
          jmp label5
        label4:
          test esi,800h
          jz label5
          mov esi,07e0h
        label5:
          add eax,esi
          mov [edi],ax
          popad
        }
      	destino++;
      	u+=dudx;
      	v+=dvdx;
      } while (--ancho);
    }

    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      altura_seccion_izquierda=v3->py-v2->py;
      if (altura_seccion_izquierda<=0)
        return;
      else {
        faltura=(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)/faltura;
        dx_izq=(long)(dxdy*65536.0);
//        prestep=0;//(float)v2->py-v2->fpy;
        prestep=(float)v2->py-v2->fpy;
        x_izq=(long)((v2->fpx+prestep*dxdy)*65536.0);
        //Calculo de las deltas
        du_izq=(long)(65536.0*(v3->ru-v2->ru)/faltura);
        u_izq=(long)(v2->ru*65536.0);
        dv_izq=(long)(65536.0*(v3->rv-v2->rv)/faltura);
        v_izq=(long)(v2->rv*65536.0);
        dr_izq=(long)(((v3->r-v2->r)*256.0)/faltura);
        r_izq=(long)(v2->r*256.0);
        dg_izq=(long)(((v3->g-v2->g)*256.0)/faltura);
        g_izq=(long)(v2->g*256.0);
        db_izq=(long)(((v3->b-v2->b)*256.0)/faltura);
        b_izq=(long)(v2->b*256.0);
      }
    }
    else {
      x_izq+=dx_izq;
      u_izq+=du_izq;
      v_izq+=dv_izq;
      r_izq+=dr_izq;
      g_izq+=dg_izq;
      b_izq+=db_izq;
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      //
      altura_seccion_derecha=v3->py-v2->py;
      if (altura_seccion_derecha<=0) return;
      else {
        //Calculo de las deltas
        dx_der=(long)(((v3->fpx-v2->fpx)*65536.0)/(v3->fpy-v2->fpy));
        faltura=(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)/faltura;
//        prestep=0;//(float)v2->py-v2->fpy;
        prestep=(float)v2->py-v2->fpy;
        x_der=(long)((v2->fpx+prestep*dxdy)*65536.0);
//        x_der=(long)(v2->fpx*65536.0);
      }
      //
    }
    else {
      x_der+=dx_der;
    }
  }
}
*/
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura sobre un triangulo con correccion de perspectiva
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
/*
void llrender::Pinta_Text_Persp_Z(lptface face,int mmlevel)
{
  int altura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla, *destino;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto=mat->textura->Alto[mmlevel];
  direccion_textura=(long)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=ancho;
  float altot=alto;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;

  switch (ancho) {
    case 8:   nucleo_texturas=nucleo8_8;
              break;
    case 16:  nucleo_texturas=nucleo8_16;
              break;
    case 32:  nucleo_texturas=nucleo8_32;
              break;
    case 64:  nucleo_texturas=nucleo8_64;
              break;
    case 128: nucleo_texturas=nucleo8_128;
              break;
    case 256: nucleo_texturas=nucleo8_256;
              break;
  }

  v1->ru=v1->u*anchot/v1->fpz; v2->ru=v2->u*anchot/v2->fpz; v3->ru=v3->u*anchot/v3->fpz;
  v1->rv=v1->v*altot/v1->fpz; v2->rv=v2->v*altot/v2->fpz; v3->rv=v3->v*altot/v3->fpz;

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->py > v2->py) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->py > v3->py) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->py > v3->py) { vertice=v2; v2=v3; v3=vertice; }

  //Calculamos la longitud de la linea m s larga

  altura=v3->py-v1->py+1;
  if (altura==0) return;

  temp=((v2->py-v1->py+1)<<16)/altura;
  largo=temp*(v3->px-v1->px)+((v1->px-v2->px)<<16);
  if (largo==0) return;

  if (largo<0) {
    array_derecho[0]=v3; array_derecho[1]=v2; array_derecho[2]=v1;
    seccion_derecha=2;
    array_izquierdo[0]=v3; array_izquierdo[1]=v1;
    seccion_izquierda=1;

    if (Seccion_Izquierda_Z()<=0) return;
    if (Seccion_Derecha_Z()<=0) {
      seccion_derecha--;
      if (Seccion_Derecha_Z()<=0) return;
    }

    if (largo>-0x1000) largo=-0x1000;
  }
  else {
    array_izquierdo[0]=v3; array_izquierdo[1]=v2; array_izquierdo[2]=v1;
    seccion_izquierda=2;
    array_derecho[0]=v3; array_derecho[1]=v1;
    seccion_derecha=1;

    if (Seccion_Derecha_Z()<=0) return;
    if (Seccion_Izquierda_Z()<=0) {
      seccion_izquierda--;
      if (Seccion_Izquierda_Z()<=0) return;
    }

    if (largo<0x1000) largo=0x1000;
  }
  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=x_izq>>16;
    ancho=(x_der>>16)-x1;

    inicio_division(&unoz_izq);
    destino=ptr_pantalla+x1;

    if (ancho>0) {
      fu=uz_izq;
      fv=vz_izq;
      fin_division();
      asignacion_u_v();
      inicio_division(&unoz_der);
      u=u_0;
      v=v_0;
      fu=uz_der;
      fv=vz_der;
      fin_division();
      asignacion_u_v();
      dudx=(u_0-u)/ancho;
      dvdx=(v_0-v)/ancho;
      ancho++;
      nucleo_texturas(v,u,dudx,dvdx,ancho,destino);
    }
    else fin_division();

    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      if (Seccion_Izquierda_Z()<=0) return;
      ptr_pantalla-=ancho_pantalla;
    }
    else {
      x_izq+=dx_izq;
      u_izq+=du_izq;
      v_izq+=dv_izq;
      uz_izq+=duz_izq;
      vz_izq+=dvz_izq;
      unoz_izq+=dunoz_izq;
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      if (Seccion_Derecha_Z()<=0) return;
      ptr_pantalla-=ancho_pantalla;
    }
    else {
      x_der+=dx_der;
      uz_der+=duz_der;
      vz_der+=dvz_der;
      unoz_der+=dunoz_der;
    }
  }
}
*/
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura sobre un triangulo con correccion de perspectiva
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
/*
void llrender::Pinta_Text_Persp_Trans(lptface face,int mmlevel)
{
  int altura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla, *destino, *fin_destino;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto =mat->textura->Alto[mmlevel];
  short *dir_textura=(short *)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=ancho;
  float altot=alto;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;
  short color;
  float unoz_aux,dunoz_aux,fu,fv,fdudx,fdvdx;
  long mascara=(ancho-1)*65536;
  long rotacion_mascara;

  switch (ancho) {
    case 8:	rotacion_mascara=13;
		break;
    case 16:	rotacion_mascara=12;
		break;
    case 32:	rotacion_mascara=11;
		break;
    case 64:	rotacion_mascara=10;
		break;
    case 128:	rotacion_mascara=9;
		break;
    case 256:	rotacion_mascara=8;
		break;
    default:	rotacion_mascara=8;
		break;
  }

  v1->ru=v1->u*anchot/v1->fpz; v2->ru=v2->u*anchot/v2->fpz; v3->ru=v3->u*anchot/v3->fpz;
  v1->rv=v1->v*altot/v1->fpz; v2->rv=v2->v*altot/v2->fpz; v3->rv=v3->v*altot/v3->fpz;

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->py > v2->py) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->py > v3->py) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->py > v3->py) { vertice=v2; v2=v3; v3=vertice; }

  //Calculamos la longitud de la linea m s larga

  altura=v3->py-v1->py+1;
  if (altura==0) return;

  temp=((v2->py-v1->py+1)<<16)/altura;
  largo=temp*(v3->px-v1->px)+((v1->px-v2->px)<<16);
  if (largo==0) return;

  if (largo<0) {
    array_derecho[0]=v3; array_derecho[1]=v2; array_derecho[2]=v1;
    seccion_derecha=2;
    array_izquierdo[0]=v3; array_izquierdo[1]=v1;
    seccion_izquierda=1;

    if (Seccion_Izquierda_Z()<=0) return;
    if (Seccion_Derecha_Z()<=0) {
      seccion_derecha--;
      if (Seccion_Derecha_Z()<=0) return;
    }

    if (largo>-0x1000) largo=-0x1000;
  }
  else {
    array_izquierdo[0]=v3; array_izquierdo[1]=v2; array_izquierdo[2]=v1;
    seccion_izquierda=2;
    array_derecho[0]=v3; array_derecho[1]=v1;
    seccion_derecha=1;

    if (Seccion_Derecha_Z()<=0) return;
    if (Seccion_Izquierda_Z()<=0) {
      seccion_izquierda--;
      if (Seccion_Izquierda_Z()<=0) return;
    }

    if (largo<0x1000) largo=0x1000;
  }
  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=x_izq>>16;
    ancho=(x_der>>16)-x1;

    if (ancho>0) {
      destino=ptr_pantalla+x1;

      fu=uz_izq;
      fv=vz_izq;
      unoz_aux=unoz_izq;
      fdudx=(uz_der-uz_izq)/(float)ancho;
      fdvdx=(vz_der-vz_izq)/(float)ancho;
      dunoz_aux=(unoz_der-unoz_izq)/(float)ancho;
      ancho++;
      do {
        if (ancho>32) {
          ancho-=32;
          fin_destino=destino+32;
          u=fu/unoz_aux;
          v=fv/unoz_aux;
          fu+=32*fdudx;
          fv+=32*fdvdx;
          unoz_aux+=32*dunoz_aux;
          dudx=(fu/unoz_aux-u)/32;
          dvdx=(fv/unoz_aux-v)/32;
        }
        else {
          fin_destino=destino+ancho;
          u=fu/unoz_aux;
          v=fv/unoz_aux;
          fu+=ancho*fdudx;
          fv+=ancho*fdvdx;
          unoz_aux+=ancho*dunoz_aux;
          dudx=(fu/unoz_aux-u)/ancho;
          dvdx=(fv/unoz_aux-v)/ancho;
          ancho=0;
        }
        do {
          if (color=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)]) *destino=color;
          destino++;
          u+=dudx;
          v+=dvdx;
        } while (destino<fin_destino);
      } while (ancho>0);
    }

    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      if (Seccion_Izquierda_Z()<=0) return;
    }
    else {
      x_izq+=dx_izq;
      u_izq+=du_izq;
      v_izq+=dv_izq;
      uz_izq+=duz_izq;
      vz_izq+=dvz_izq;
      unoz_izq+=dunoz_izq;
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      if (Seccion_Derecha_Z()<=0) return;
    }
    else {
      x_der+=dx_der;
      uz_der+=duz_der;
      vz_der+=dvz_der;
      unoz_der+=dunoz_der;
    }
  }
}
*/
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura transluc. sobre un triangulo con correccion de perspectiva
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
/*
void llrender::Pinta_Text_Persp_Translucido(lptface face,int mmlevel)
{
  int altura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla, *destino, *fin_destino;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto =mat->textura->Alto[mmlevel];
  short *dir_textura=(short *)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=ancho;
  float altot=alto;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;
  long color;
  float unoz_aux,dunoz_aux,fu,fv,fdudx,fdvdx;
  long mascara=(ancho-1)*65536;
  long rotacion_mascara;

  switch (ancho) {
    case 8:	rotacion_mascara=13;
		break;
    case 16:	rotacion_mascara=12;
		break;
    case 32:	rotacion_mascara=11;
		break;
    case 64:	rotacion_mascara=10;
		break;
    case 128:	rotacion_mascara=9;
		break;
    case 256:	rotacion_mascara=8;
		break;
    default:	rotacion_mascara=8;
		break;
  }

  v1->ru=v1->u*anchot/v1->fpz; v2->ru=v2->u*anchot/v2->fpz; v3->ru=v3->u*anchot/v3->fpz;
  v1->rv=v1->v*altot/v1->fpz; v2->rv=v2->v*altot/v2->fpz; v3->rv=v3->v*altot/v3->fpz;

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->py > v2->py) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->py > v3->py) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->py > v3->py) { vertice=v2; v2=v3; v3=vertice; }

  //Calculamos la longitud de la linea m s larga

  altura=v3->py-v1->py+1;
  if (altura==0) return;

  temp=((v2->py-v1->py+1)<<16)/altura;
  largo=temp*(v3->px-v1->px)+((v1->px-v2->px)<<16);
  if (largo==0) return;

  if (largo<0) {
    array_derecho[0]=v3; array_derecho[1]=v2; array_derecho[2]=v1;
    seccion_derecha=2;
    array_izquierdo[0]=v3; array_izquierdo[1]=v1;
    seccion_izquierda=1;

    if (Seccion_Izquierda_Z()<=0) return;
    if (Seccion_Derecha_Z()<=0) {
      seccion_derecha--;
      if (Seccion_Derecha_Z()<=0) return;
    }

    if (largo>-0x1000) largo=-0x1000;
  }
  else {
    array_izquierdo[0]=v3; array_izquierdo[1]=v2; array_izquierdo[2]=v1;
    seccion_izquierda=2;
    array_derecho[0]=v3; array_derecho[1]=v1;
    seccion_derecha=1;

    if (Seccion_Derecha_Z()<=0) return;
    if (Seccion_Izquierda_Z()<=0) {
      seccion_izquierda--;
      if (Seccion_Izquierda_Z()<=0) return;
    }

    if (largo<0x1000) largo=0x1000;
  }
  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=x_izq>>16;
    ancho=(x_der>>16)-x1;

    if (ancho>0) {
      destino=ptr_pantalla+x1;

      fu=uz_izq;
      fv=vz_izq;
      unoz_aux=unoz_izq;
      fdudx=(uz_der-uz_izq)/(float)ancho;
      fdvdx=(vz_der-vz_izq)/(float)ancho;
      dunoz_aux=(unoz_der-unoz_izq)/(float)ancho;
      ancho++;
      do {
      	if (ancho>32) {
      	  ancho-=32;
      	  fin_destino=destino+32;
      	  u=fu/unoz_aux;
      	  v=fv/unoz_aux;
      	  fu+=32*fdudx;
  	      fv+=32*fdvdx;
      	  unoz_aux+=32*dunoz_aux;
      	  dudx=(fu/unoz_aux-u)/32;
      	  dvdx=(fv/unoz_aux-v)/32;
      	}
      	else {
      	  fin_destino=destino+ancho;
      	  u=fu/unoz_aux;
      	  v=fv/unoz_aux;
      	  fu+=ancho*fdudx;
          fv+=ancho*fdvdx;
      	  unoz_aux+=ancho*dunoz_aux;
      	  dudx=(fu/unoz_aux-u)/ancho;
      	  dvdx=(fv/unoz_aux-v)/ancho;
      	  ancho=0;
      	}
      	do {
      	  if (color=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)]) {
            _asm {
              pushad
              mov edi,destino
              mov ebx,color
              mov edx,ebx
              mov esi,ebx
              and ebx,0ffh
              shr edx,8
              and edx,0ffh
              shr esi,16
              and esi,0ffh

              xor eax,eax
              mov al,[edi]
              add eax,ebx
              shr eax,1
              mov [edi],al
              inc edi
              mov al,[edi]
              add eax,edx
              shr eax,1
              mov [edi],al
              inc edi
              mov al,[edi]
              add eax,esi
              shr eax,1
              mov [edi],al
              add edi,2
              popad
            }
          }
      	  destino++;
      	  u+=dudx;
      	  v+=dvdx;
      	} while (destino<fin_destino);
      } while (ancho>0);
    }

    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      if (Seccion_Izquierda_Z()<=0) return;
    }
    else {
      x_izq+=dx_izq;
      u_izq+=du_izq;
      v_izq+=dv_izq;
      uz_izq+=duz_izq;
      vz_izq+=dvz_izq;
      unoz_izq+=dunoz_izq;
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      if (Seccion_Derecha_Z()<=0) return;
    }
    else {
      x_der+=dx_der;
      uz_der+=duz_der;
      vz_der+=dvz_der;
      unoz_der+=dunoz_der;
    }
  }
}
*/
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura iluminada sobre un triangulo con correccion de perspectiva
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
/*
void llrender::Pinta_Text_Persp_Ilum(lptface face,int mmlevel)
{
  int altura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla, *destino, *fin_destino;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto =mat->textura->Alto[mmlevel];
  short *dir_textura=(short *)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=ancho;
  float altot=alto;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;
  short color;
  int intensidad=(filtro_iluminacion[(long)v1->intensidad]+filtro_iluminacion[(long)v2->intensidad]+filtro_iluminacion[(long)v3->intensidad])/3;
  float unoz_aux,dunoz_aux,fu,fv,fdudx,fdvdx;
  long mascara=(ancho-1)*65536;
  long rotacion_mascara;

  switch (ancho) {
    case 8:	rotacion_mascara=13;
		break;
    case 16:	rotacion_mascara=12;
		break;
    case 32:	rotacion_mascara=11;
		break;
    case 64:	rotacion_mascara=10;
		break;
    case 128:	rotacion_mascara=9;
		break;
    case 256:	rotacion_mascara=8;
		break;
    default:	rotacion_mascara=8;
		break;
  }

  v1->ru=v1->u*anchot/v1->fpz; v2->ru=v2->u*anchot/v2->fpz; v3->ru=v3->u*anchot/v3->fpz;
  v1->rv=v1->v*altot/v1->fpz; v2->rv=v2->v*altot/v2->fpz; v3->rv=v3->v*altot/v3->fpz;

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->py > v2->py) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->py > v3->py) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->py > v3->py) { vertice=v2; v2=v3; v3=vertice; }

  //Calculamos la longitud de la linea m s larga

  altura=v3->py-v1->py+1;
  if (altura==0) return;

  temp=((v2->py-v1->py+1)<<16)/altura;
  largo=temp*(v3->px-v1->px)+((v1->px-v2->px)<<16);
  if (largo==0) return;

  if (largo<0) {
    array_derecho[0]=v3; array_derecho[1]=v2; array_derecho[2]=v1;
    seccion_derecha=2;
    array_izquierdo[0]=v3; array_izquierdo[1]=v1;
    seccion_izquierda=1;

    if (Seccion_Izquierda_Z()<=0) return;
    if (Seccion_Derecha_Z()<=0) {
      seccion_derecha--;
      if (Seccion_Derecha_Z()<=0) return;
    }

    if (largo>-0x1000) largo=-0x1000;
  }
  else {
    array_izquierdo[0]=v3; array_izquierdo[1]=v2; array_izquierdo[2]=v1;
    seccion_izquierda=2;
    array_derecho[0]=v3; array_derecho[1]=v1;
    seccion_derecha=1;

    if (Seccion_Derecha_Z()<=0) return;
    if (Seccion_Izquierda_Z()<=0) {
      seccion_izquierda--;
      if (Seccion_Izquierda_Z()<=0) return;
    }

    if (largo<0x1000) largo=0x1000;
  }
  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=x_izq>>16;
    ancho=(x_der>>16)-x1;

    if (ancho>0) {
      destino=ptr_pantalla+x1;

      fu=uz_izq;
      fv=vz_izq;
      unoz_aux=unoz_izq;
      fdudx=(uz_der-uz_izq)/(float)ancho;
      fdvdx=(vz_der-vz_izq)/(float)ancho;
      dunoz_aux=(unoz_der-unoz_izq)/(float)ancho;
      ancho++;
      do {
      	if (ancho>32) {
      	  ancho-=32;
      	  fin_destino=destino+32;
      	  u=fu/unoz_aux;
      	  v=fv/unoz_aux;
      	  fu+=32*fdudx;
  	      fv+=32*fdvdx;
      	  unoz_aux+=32*dunoz_aux;
      	  dudx=(fu/unoz_aux-u)/32;
      	  dvdx=(fv/unoz_aux-v)/32;
      	}
      	else {
      	  fin_destino=destino+ancho;
      	  u=fu/unoz_aux;
      	  v=fv/unoz_aux;
      	  fu+=ancho*fdudx;
          fv+=ancho*fdvdx;
      	  unoz_aux+=ancho*dunoz_aux;
      	  dudx=(fu/unoz_aux-u)/ancho;
      	  dvdx=(fv/unoz_aux-v)/ancho;
      	  ancho=0;
      	}
      	do {
          color=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)];
          _asm {
            pushad
            mov edi,destino
            mov ebx,intensidad
            mov edx,ebx
            shl ebx,8
            and ebx,0f800h
            shl edx,3
            and edx,07e0h

            mov ax,color
            and eax,0f800h
            add eax,ebx
            sub eax,8000h
            jns label0
            xor eax,eax
            jmp label1
          label0:
            test eax,10000h
            jz label1
            mov eax,0f800h
          label1:
            mov si,color
            and esi,01fh
            mov ebx,intensidad
            shr ebx,3
            and ebx,01fh
            add esi,ebx
            sub esi,16
            jns label2
            xor esi,esi
            jmp label3
          label2:
            test esi,20h
            jz label3
            mov esi,01fh
          label3:
            add eax,esi
            mov si,color
            and esi,07e0h
            add esi,edx
            sub esi,1024
            jns label4
            xor esi,esi
            jmp label5
          label4:
            test esi,800h
            jz label5
            mov esi,07e0h
          label5:
            add eax,esi
            mov [edi],ax
            popad
          }
        	destino++;
      	  u+=dudx;
      	  v+=dvdx;
      	} while (destino<fin_destino);
      } while (ancho>0);
    }

    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      if (Seccion_Izquierda_Z()<=0) return;
    }
    else {
      x_izq+=dx_izq;
      u_izq+=du_izq;
      v_izq+=dv_izq;
      uz_izq+=duz_izq;
      vz_izq+=dvz_izq;
      unoz_izq+=dunoz_izq;
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      if (Seccion_Derecha_Z()<=0) return;
    }
    else {
      x_der+=dx_der;
      uz_der+=duz_der;
      vz_der+=dvz_der;
      unoz_der+=dunoz_der;
    }
  }
}
*/
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura sobre un triangulo con correccion de perspectiva
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
/*
void llrender::Pinta_Text_Persp_Bilinear(lptface face,int mmlevel)
{
  int altura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla, *destino, *fin_destino;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto =mat->textura->Alto[mmlevel];
  short *dir_textura=(short *)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=ancho;
  float altot=alto;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;
  long color1,color2,color3,color4;
  long color_r,color_g,color_b,dist_u,dist_v,ancho_u=ancho;
  long porcentaje1,porcentaje2,porcentaje3,porcentaje4;
  short *dir_color;
  float unoz_aux,dunoz_aux,fu,fv,fdudx,fdvdx;
  long mascara=(ancho-1)*65536;
  long rotacion_mascara;
  int var_aux=0;

  switch (ancho) {
    case 8:	rotacion_mascara=13;
		break;
    case 16:	rotacion_mascara=12;
		break;
    case 32:	rotacion_mascara=11;
		break;
    case 64:	rotacion_mascara=10;
		break;
    case 128:	rotacion_mascara=9;
		break;
    case 256:	rotacion_mascara=8;
		break;
    default:	rotacion_mascara=8;
		break;
  }

  v1->ru=v1->u*anchot/v1->fpz; v2->ru=v2->u*anchot/v2->fpz; v3->ru=v3->u*anchot/v3->fpz;
  v1->rv=v1->v*altot/v1->fpz; v2->rv=v2->v*altot/v2->fpz; v3->rv=v3->v*altot/v3->fpz;

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->py > v2->py) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->py > v3->py) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->py > v3->py) { vertice=v2; v2=v3; v3=vertice; }

  //Calculamos la longitud de la linea m s larga

  altura=v3->py-v1->py+1;
  if (altura==0) return;

  temp=((v2->py-v1->py+1)<<16)/altura;
  largo=temp*(v3->px-v1->px)+((v1->px-v2->px)<<16);
  if (largo==0) return;

  if (largo<0) {
    array_derecho[0]=v3; array_derecho[1]=v2; array_derecho[2]=v1;
    seccion_derecha=2;
    array_izquierdo[0]=v3; array_izquierdo[1]=v1;
    seccion_izquierda=1;

    if (Seccion_Izquierda_Z()<=0) return;
    if (Seccion_Derecha_Z()<=0) {
      seccion_derecha--;
      if (Seccion_Derecha_Z()<=0) return;
    }

    if (largo>-0x1000) largo=-0x1000;
  }
  else {
    array_izquierdo[0]=v3; array_izquierdo[1]=v2; array_izquierdo[2]=v1;
    seccion_izquierda=2;
    array_derecho[0]=v3; array_derecho[1]=v1;
    seccion_derecha=1;

    if (Seccion_Derecha_Z()<=0) return;
    if (Seccion_Izquierda_Z()<=0) {
      seccion_izquierda--;
      if (Seccion_Izquierda_Z()<=0) return;
    }

    if (largo<0x1000) largo=0x1000;
  }
  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=x_izq>>16;
    ancho=(x_der>>16)-x1;

    if (ancho>0) {
      destino=ptr_pantalla+x1;

      fu=uz_izq;
      fv=vz_izq;
      unoz_aux=unoz_izq;
      fdudx=(uz_der-uz_izq)/(float)ancho;
      fdvdx=(vz_der-vz_izq)/(float)ancho;
      dunoz_aux=(unoz_der-unoz_izq)/(float)ancho;
      ancho++;
      do {
      	if (ancho>16) {
      	  ancho-=16;
      	  fin_destino=destino+16;
      	  u=fu/unoz_aux;
      	  v=fv/unoz_aux;
      	  fu+=16*fdudx;
      	  fv+=16*fdvdx;
      	  unoz_aux+=16*dunoz_aux;
      	  dudx=(fu/unoz_aux-u)/16;
      	  dvdx=(fv/unoz_aux-v)/16;
      	}
      	else {
      	  fin_destino=destino+ancho;
      	  u=fu/unoz_aux;
      	  v=fv/unoz_aux;
      	  fu+=ancho*fdudx;
      	  fv+=ancho*fdvdx;
      	  unoz_aux+=ancho*dunoz_aux;
      	  dudx=(fu/unoz_aux-u)/ancho;
      	  dvdx=(fv/unoz_aux-v)/ancho;
      	  ancho=0;
      	}
      	do {
          dir_color=dir_textura+((v&mascara)>>rotacion_mascara)+((u&mascara)>>16);
          color1=*dir_color;
          color2=*(dir_color+1);
          color3=*(dir_color+ancho_u);
          color4=*(dir_color+ancho_u+1);
          dist_u=(u>>8)&255;
          dist_v=(v>>8)&255;
          porcentaje1=bilinear_mul_256[dist_u][dist_v];
          porcentaje2=bilinear_mul_256[255-dist_u][dist_v];
          porcentaje3=bilinear_mul_256[dist_u][255-dist_v];
          porcentaje4=bilinear_mul_256[255-dist_u][255-dist_v];
          color_r=(((color1&0xff0000)*porcentaje4)+((color2&0xff0000)*porcentaje3)+
                   ((color3&0xff0000)*porcentaje2)+((color4&0xff0000)*porcentaje1))/256;
          color_g=(((color1&0xff00)*porcentaje4)+((color2&0xff00)*porcentaje3)+
                   ((color3&0xff00)*porcentaje2)+((color4&0xff00)*porcentaje1))/256;
          color_b=(((color1&0xff)*porcentaje4)+((color2&0xff)*porcentaje3)+
                   ((color3&0xff)*porcentaje2)+((color4&0xff)*porcentaje1))/256;

//          color_r=( bilinear_mul[video->Dac[color1]][porcentaje4]+bilinear_mul[video->Dac[color2]][porcentaje3]+
//                    bilinear_mul[video->Dac[color3]][porcentaje2]+bilinear_mul[video->Dac[color4]][porcentaje1] )*2;
//          color_g=( bilinear_mul[video->Dac[color1+1]][porcentaje4]+bilinear_mul[video->Dac[color2+1]][porcentaje3]+
//                    bilinear_mul[video->Dac[color3+1]][porcentaje2]+bilinear_mul[video->Dac[color4+1]][porcentaje1] )*4;
//          color_b=( bilinear_mul[video->Dac[color1+2]][porcentaje4]+bilinear_mul[video->Dac[color2+2]][porcentaje3]+
//                    bilinear_mul[video->Dac[color3+2]][porcentaje2]+bilinear_mul[video->Dac[color4+2]][porcentaje1] )*4;
//          *destino++=video->Lux[(color_r&0x7C00)+((color_g>>6)&0x03E0)+(color_b>>11)];

          *destino++=(short)(color_r&0xff0000)+(color_g&0xff00)+(color_b&0xff);
      	  u+=dudx;
      	  v+=dvdx;
        } while (destino<fin_destino);
      } while (ancho>0);
    }

    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      if (Seccion_Izquierda_Z()<=0) return;
    }
    else {
      x_izq+=dx_izq;
      u_izq+=du_izq;
      v_izq+=dv_izq;
      uz_izq+=duz_izq;
      vz_izq+=dvz_izq;
      unoz_izq+=dunoz_izq;
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      if (Seccion_Derecha_Z()<=0) return;
    }
    else {
      x_der+=dx_der;
      uz_der+=duz_der;
      vz_der+=dvz_der;
      unoz_der+=dunoz_der;
    }
  }
}
*/
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Comprueba si una cara est  oculta
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int llrender::Cara_Oculta(lptface face)
{
  float producto_escalar;

  producto_escalar= (face->v2->fpx - face->v1->fpx)*(face->v3->fpy - face->v1->fpy)-
                    (face->v2->fpy - face->v1->fpy)*(face->v3->fpx - face->v1->fpx);

  if (producto_escalar<0) return(0);
/*
  if ((face->v2->fpx - face->v1->fpx)*(face->v3->fpy - face->v1->fpy)<
      (face->v2->fpy - face->v1->fpy)*(face->v3->fpx - face->v1->fpx))
    return 0;
*/
  return 1;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::clipping_3d_near(lptface face_in)
{
  long i=0;
  tface face;
  tvertex vertex[4];
  float fwork;

  //  Calculamos cuantos puntos est n situados detras del plano de clipping
  i=face_in->v1->sutherland_cohen+face_in->v2->sutherland_cohen+face_in->v3->sutherland_cohen;
  if (!i) {
    pinta_triangulo(face_in);
    return;
  }
  i=(i>>8)&3;
  if (i==3) return; // El pol¡gono est  fuera de pantalla
  if (i==0) {
    clipping_3d_left(face_in); // El pol¡gono est  dentro de pantalla
    return;
  }

  memcpy(&face,face_in,sizeof(tface));
  face.v1->estado=0;
  face.v2->estado=0;
  face.v3->estado=0;

  if (i==2) { // El resultado del corte es un tri ngulo
    // Partimos de un punto situado delante del plano de clipping
    if (face.v1->fpz>=1.0) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpz>=1.0) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    // Corte 1 con el plano de clipping
    fwork=(1.0-vertex[0].fpz)/(vertex[1].fpz-vertex[0].fpz);
    vertex[1].sx=(vertex[1].sx-vertex[0].sx)*fwork+vertex[0].sx;
    vertex[1].sy=(vertex[1].sy-vertex[0].sy)*fwork+vertex[0].sy;
    vertex[1].u=(vertex[1].u-vertex[0].u)*fwork+vertex[0].u;
    vertex[1].v=(vertex[1].v-vertex[0].v)*fwork+vertex[0].v;
    vertex[1].r=(vertex[1].r-vertex[0].r)*fwork+vertex[0].r;
    vertex[1].g=(vertex[1].g-vertex[0].g)*fwork+vertex[0].g;
    vertex[1].b=(vertex[1].b-vertex[0].b)*fwork+vertex[0].b;
    vertex[1].intensidad=(vertex[1].intensidad-vertex[0].intensidad)*fwork+vertex[0].intensidad;
    // Proyectar
    vertex[1].fpx=(fancho_pantalla/2)+(vertex[1].sx*proyeccion_x);
    vertex[1].fpy=(falto_pantalla/2)-(vertex[1].sy*proyeccion_y);
    vertex[1].fpz=1.0;
    vertex[1].sutherland_cohen=0;
    if (vertex[1].fpx<0.0) vertex[1].sutherland_cohen+=1;
    if (vertex[1].fpx>fancho_pantalla) vertex[1].sutherland_cohen+=4;
    if (vertex[1].fpy<0.0) vertex[1].sutherland_cohen+=16;
    if (vertex[1].fpy>falto_pantalla) vertex[1].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=(1.0-vertex[0].fpz)/(vertex[2].fpz-vertex[0].fpz);
    vertex[2].sx=(vertex[2].sx-vertex[0].sx)*fwork+vertex[0].sx;
    vertex[2].sy=(vertex[2].sy-vertex[0].sy)*fwork+vertex[0].sy;
    vertex[2].u=(vertex[2].u-vertex[0].u)*fwork+vertex[0].u;
    vertex[2].v=(vertex[2].v-vertex[0].v)*fwork+vertex[0].v;
    vertex[2].r=(vertex[2].r-vertex[0].r)*fwork+vertex[0].r;
    vertex[2].g=(vertex[2].g-vertex[0].g)*fwork+vertex[0].g;
    vertex[2].b=(vertex[2].b-vertex[0].b)*fwork+vertex[0].b;
    vertex[2].intensidad=(vertex[2].intensidad-vertex[0].intensidad)*fwork+vertex[0].intensidad;
    // Proyectar
    vertex[2].fpx=(fancho_pantalla/2)+(vertex[2].sx*proyeccion_x);
    vertex[2].fpy=(falto_pantalla/2)-(vertex[2].sy*proyeccion_y);
    vertex[2].fpz=1.0;
    vertex[2].sutherland_cohen=0;
    if (vertex[2].fpx<0.0) vertex[2].sutherland_cohen+=1;
    if (vertex[2].fpx>fancho_pantalla) vertex[2].sutherland_cohen+=4;
    if (vertex[2].fpy<0.0) vertex[2].sutherland_cohen+=16;
    if (vertex[2].fpy>falto_pantalla) vertex[2].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_left(&face);
  }
  else { // El resultado del corte son dos tri ngulos
    // Partimos de un punto situado detras del plano de clipping
    if (face.v1->fpz<1.0) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpz<1.0) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    // Corte 1 con el plano de clipping
    fwork=(1.0-vertex[0].fpz)/(vertex[2].fpz-vertex[0].fpz);
    vertex[3].sx=(vertex[2].sx-vertex[0].sx)*fwork+vertex[0].sx;
    vertex[3].sy=(vertex[2].sy-vertex[0].sy)*fwork+vertex[0].sy;
    vertex[3].u=(vertex[2].u-vertex[0].u)*fwork+vertex[0].u;
    vertex[3].v=(vertex[2].v-vertex[0].v)*fwork+vertex[0].v;
    vertex[3].r=(vertex[2].r-vertex[0].r)*fwork+vertex[0].r;
    vertex[3].g=(vertex[2].g-vertex[0].g)*fwork+vertex[0].g;
    vertex[3].b=(vertex[2].b-vertex[0].b)*fwork+vertex[0].b;
    vertex[3].intensidad=(vertex[2].intensidad-vertex[0].intensidad)*fwork+vertex[0].intensidad;
    // Proyectar
    vertex[3].fpx=(fancho_pantalla/2)+(vertex[3].sx*proyeccion_x);
    vertex[3].fpy=(falto_pantalla/2)-(vertex[3].sy*proyeccion_y);
    vertex[3].fpz=1.0;
    vertex[3].sutherland_cohen=0;
    if (vertex[3].fpx<0.0) vertex[3].sutherland_cohen+=1;
    if (vertex[3].fpx>fancho_pantalla) vertex[3].sutherland_cohen+=4;
    if (vertex[3].fpy<0.0) vertex[3].sutherland_cohen+=16;
    if (vertex[3].fpy>falto_pantalla) vertex[3].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=(1.0-vertex[0].fpz)/(vertex[1].fpz-vertex[0].fpz);
    vertex[0].sx=(vertex[1].sx-vertex[0].sx)*fwork+vertex[0].sx;
    vertex[0].sy=(vertex[1].sy-vertex[0].sy)*fwork+vertex[0].sy;
    vertex[0].u=(vertex[1].u-vertex[0].u)*fwork+vertex[0].u;
    vertex[0].v=(vertex[1].v-vertex[0].v)*fwork+vertex[0].v;
    vertex[0].r=(vertex[1].r-vertex[0].r)*fwork+vertex[0].r;
    vertex[0].g=(vertex[1].g-vertex[0].g)*fwork+vertex[0].g;
    vertex[0].b=(vertex[1].b-vertex[0].b)*fwork+vertex[0].b;
    vertex[0].intensidad=(vertex[1].intensidad-vertex[0].intensidad)*fwork+vertex[0].intensidad;
    // Proyectar
    vertex[0].fpx=(fancho_pantalla/2)+(vertex[0].sx*proyeccion_x);
    vertex[0].fpy=(falto_pantalla/2)-(vertex[0].sy*proyeccion_y);
    vertex[0].fpz=1.0;
    vertex[0].sutherland_cohen=0;
    if (vertex[0].fpx<0.0) vertex[0].sutherland_cohen+=1;
    if (vertex[0].fpx>fancho_pantalla) vertex[0].sutherland_cohen+=4;
    if (vertex[0].fpy<0.0) vertex[0].sutherland_cohen+=16;
    if (vertex[0].fpy>falto_pantalla) vertex[0].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_left(&face);
    face.v1=&vertex[0];
    face.v2=&vertex[2];
    face.v3=&vertex[3];
    face.v3->estado=0;
    clipping_3d_left(&face);
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::clipping_3d_left(lptface face_in)
{
  long i=0;
  tface face;
  tvertex vertex[4];
  float fwork;
  float cero_z,uno_z,dos_z;

  //  Calculamos cuantos puntos est n situados fuera de la pantalla
  i=face_in->v1->sutherland_cohen+face_in->v2->sutherland_cohen+face_in->v3->sutherland_cohen;
  if (!i) {
    pinta_triangulo(face_in);
    return;
  }
  i=i&3;
  if (i==3) return; // El pol¡gono est  fuera de pantalla
  if (i==0) {
    clipping_3d_right(face_in); // El pol¡gono est  dentro de pantalla
    return;
  }

  memcpy(&face,face_in,sizeof(tface));
  face.v1->estado=0;
  face.v2->estado=0;
  face.v3->estado=0;

  if (i==1) { // El resultado del corte son dos tri ngulos
    // Partimos de un punto situado encima de la pantalla
    if (face.v1->fpx<0) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpx<0) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el plano de clipping
    fwork=-vertex[0].fpx/(vertex[2].fpx-vertex[0].fpx);
    vertex[3].fpx=0;
    vertex[3].fpy=(vertex[2].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[3].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].fpz=1/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].sutherland_cohen=0;
    if (vertex[3].fpx>fancho_pantalla) vertex[3].sutherland_cohen+=4;
    if (vertex[3].fpy<0.0) vertex[3].sutherland_cohen+=16;
    if (vertex[3].fpy>falto_pantalla) vertex[3].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=-vertex[0].fpx/(vertex[1].fpx-vertex[0].fpx);
    vertex[0].fpx=0;
    vertex[0].fpy=(vertex[1].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[0].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].fpz=1/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].sutherland_cohen=0;
    if (vertex[0].fpx>fancho_pantalla) vertex[0].sutherland_cohen+=4;
    if (vertex[0].fpy<0.0) vertex[0].sutherland_cohen+=16;
    if (vertex[0].fpy>falto_pantalla) vertex[0].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_right(&face);
    face.v1=&vertex[0];
    face.v2=&vertex[2];
    face.v3=&vertex[3];
    face.v3->estado=0;
    clipping_3d_right(&face);
  }
  else { // El resultado del corte es un tri ngulo
    // Partimos de un punto situado debajo del margen superior de pantalla
    if (face.v1->fpx>=0) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpx>=0) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el margen superior de la pantalla
    fwork=-vertex[0].fpx/(vertex[1].fpx-vertex[0].fpx);
    vertex[1].fpx=0;
    vertex[1].fpy=(vertex[1].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[1].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].fpz=1/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].sutherland_cohen=0;
    if (vertex[1].fpx>fancho_pantalla) vertex[1].sutherland_cohen+=4;
    if (vertex[1].fpy<0.0) vertex[1].sutherland_cohen+=16;
    if (vertex[1].fpy>falto_pantalla) vertex[1].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=-vertex[0].fpx/(vertex[2].fpx-vertex[0].fpx);
    vertex[2].fpx=0;
    vertex[2].fpy=(vertex[2].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[2].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].fpz=1/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].sutherland_cohen=0;
    if (vertex[2].fpx>fancho_pantalla) vertex[2].sutherland_cohen+=4;
    if (vertex[2].fpy<0.0) vertex[2].sutherland_cohen+=16;
    if (vertex[2].fpy>falto_pantalla) vertex[2].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_right(&face);
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::clipping_3d_right(lptface face_in)
{
  long i=0;
  tface face;
  tvertex vertex[4];
  float fwork;
  float cero_z,uno_z,dos_z;

  //  Calculamos cuantos puntos est n situados fuera de la pantalla
  i=face_in->v1->sutherland_cohen+face_in->v2->sutherland_cohen+face_in->v3->sutherland_cohen;
  if (!i) {
    pinta_triangulo(face_in);
    return;
  }
  i=(i>>2)&3;
  if (i==3) return; // El pol¡gono est  fuera de pantalla
  if (i==0) {
    clipping_3d_up(face_in); // El pol¡gono est  dentro de pantalla
    return;
  }

  memcpy(&face,face_in,sizeof(tface));
  face.v1->estado=0;
  face.v2->estado=0;
  face.v3->estado=0;

  if (i==1) { // El resultado del corte son dos tri ngulos
    // Partimos de un punto situado encima de la pantalla
    if (face.v1->fpx>fancho_pantalla) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpx>fancho_pantalla) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el plano de clipping
    fwork=(fancho_pantalla-vertex[0].fpx)/(vertex[2].fpx-vertex[0].fpx);
    vertex[3].fpx=fancho_pantalla;
    vertex[3].fpy=(vertex[2].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[3].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].fpz=1/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].sutherland_cohen=0;
    if (vertex[3].fpy<0.0) vertex[3].sutherland_cohen+=16;
    if (vertex[3].fpy>falto_pantalla) vertex[3].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=(fancho_pantalla-vertex[0].fpx)/(vertex[1].fpx-vertex[0].fpx);
    vertex[0].fpx=fancho_pantalla;
    vertex[0].fpy=(vertex[1].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[0].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].fpz=1/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].sutherland_cohen=0;
    if (vertex[0].fpy<0.0) vertex[0].sutherland_cohen+=16;
    if (vertex[0].fpy>falto_pantalla) vertex[0].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_up(&face);
    face.v1=&vertex[0];
    face.v2=&vertex[2];
    face.v3=&vertex[3];
    face.v3->estado=0;
    clipping_3d_up(&face);
  }
  else { // El resultado del corte es un tri ngulo
    // Partimos de un punto situado debajo del margen superior de pantalla
    if (face.v1->fpx<=fancho_pantalla) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpx<=fancho_pantalla) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el margen superior de la pantalla
    fwork=(fancho_pantalla-vertex[0].fpx)/(vertex[1].fpx-vertex[0].fpx);
    vertex[1].fpx=fancho_pantalla;
    vertex[1].fpy=(vertex[1].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[1].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].fpz=1/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].sutherland_cohen=0;
    if (vertex[1].fpy<0.0) vertex[1].sutherland_cohen+=16;
    if (vertex[1].fpy>falto_pantalla) vertex[1].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=(fancho_pantalla-vertex[0].fpx)/(vertex[2].fpx-vertex[0].fpx);
    vertex[2].fpx=fancho_pantalla;
    vertex[2].fpy=(vertex[2].fpy-vertex[0].fpy)*fwork+vertex[0].fpy;
    vertex[2].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].fpz=1/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].sutherland_cohen=0;
    if (vertex[2].fpy<0.0) vertex[2].sutherland_cohen+=16;
    if (vertex[2].fpy>falto_pantalla) vertex[2].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_up(&face);
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::clipping_3d_up(lptface face_in)
{
  long i=0;
  tface face;
  tvertex vertex[4];
  float fwork;
  float cero_z,uno_z,dos_z;

  //  Calculamos cuantos puntos est n situados fuera de la pantalla
  i=face_in->v1->sutherland_cohen+face_in->v2->sutherland_cohen+face_in->v3->sutherland_cohen;
  if (!i) {
    pinta_triangulo(face_in);
    return;
  }
  i=(i>>4)&3;
  if (i==3) return; // El pol¡gono est  fuera de pantalla
  if (i==0) {
    clipping_3d_down(face_in); // El pol¡gono est  dentro de pantalla
    return;
  }

  memcpy(&face,face_in,sizeof(tface));
  face.v1->estado=0;
  face.v2->estado=0;
  face.v3->estado=0;

  if (i==1) { // El resultado del corte son dos tri ngulos
    // Partimos de un punto situado encima de la pantalla
    if (face.v1->fpy<0) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpy<0) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el plano de clipping
    fwork=-vertex[0].fpy/(vertex[2].fpy-vertex[0].fpy);
    vertex[3].fpx=(vertex[2].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[3].fpy=0;
    vertex[3].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].fpz=1/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].sutherland_cohen=0;
    if (vertex[3].fpy>falto_pantalla) vertex[3].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=-vertex[0].fpy/(vertex[1].fpy-vertex[0].fpy);
    vertex[0].fpx=(vertex[1].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[0].fpy=0;
    vertex[0].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].fpz=1/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].sutherland_cohen=0;
    if (vertex[0].fpy>falto_pantalla) vertex[0].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_down(&face);
    face.v1=&vertex[0];
    face.v2=&vertex[2];
    face.v3=&vertex[3];
    face.v3->estado=0;
    clipping_3d_down(&face);
  }
  else { // El resultado del corte es un tri ngulo
    // Partimos de un punto situado debajo del margen superior de pantalla
    if (face.v1->fpy>=0) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpy>=0) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el margen superior de la pantalla
    fwork=-vertex[0].fpy/(vertex[1].fpy-vertex[0].fpy);
    vertex[1].fpx=(vertex[1].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[1].fpy=0;
    vertex[1].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].fpz=1/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].sutherland_cohen=0;
    if (vertex[1].fpy>falto_pantalla) vertex[1].sutherland_cohen+=64;

    // Corte 2 con el plano de clipping
    fwork=-vertex[0].fpy/(vertex[2].fpy-vertex[0].fpy);
    vertex[2].fpx=(vertex[2].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[2].fpy=0;
    vertex[2].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].fpz=1/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].sutherland_cohen=0;
    if (vertex[2].fpy>falto_pantalla) vertex[2].sutherland_cohen+=64;

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    clipping_3d_down(&face);
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::clipping_3d_down(lptface face_in)
{
  long i=0;
  tface face;
  tvertex vertex[4];
  float fwork;
  float cero_z,uno_z,dos_z;

  //  Calculamos cuantos puntos est n situados fuera de la pantalla
  i=face_in->v1->sutherland_cohen+face_in->v2->sutherland_cohen+face_in->v3->sutherland_cohen;
  if (!i) {
    pinta_triangulo(face_in);
    return;
  }
  i=(i>>6)&3;
  if (i==3) return; // El pol¡gono est  fuera de pantalla
  if (i==0) {
    pinta_triangulo(face_in); // El pol¡gono est  dentro de pantalla
    return;
  }

  memcpy(&face,face_in,sizeof(tface));
  face.v1->estado=0;
  face.v2->estado=0;
  face.v3->estado=0;

  if (i==1) { // El resultado del corte son dos tri ngulos
    // Partimos de un punto situado encima de la pantalla

    if (face.v1->fpy>falto_pantalla) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpy>falto_pantalla) {
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el plano de clipping
    fwork=(falto_pantalla-vertex[0].fpy)/(vertex[2].fpy-vertex[0].fpy);
    vertex[3].fpx=(vertex[2].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[3].fpy=falto_pantalla;
    vertex[3].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[3].fpz=1/((dos_z-cero_z)*fwork+cero_z);

    // Corte 2 con el plano de clipping
    fwork=(falto_pantalla-vertex[0].fpy)/(vertex[1].fpy-vertex[0].fpy);
    vertex[0].fpx=(vertex[1].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[0].fpy=falto_pantalla;
    vertex[0].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[0].fpz=1/((uno_z-cero_z)*fwork+cero_z);

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    pinta_triangulo(&face);
    face.v1=&vertex[0];
    face.v2=&vertex[2];
    face.v3=&vertex[3];
    face.v3->estado=0;
    pinta_triangulo(&face);
  }
  else { // El resultado del corte es un tri ngulo
    // Partimos de un punto situado debajo del margen superior de pantalla
    if (face.v1->fpy<=falto_pantalla) {
      memcpy(&vertex[0],face.v1,sizeof(tvertex));
      memcpy(&vertex[1],face.v2,sizeof(tvertex));
      memcpy(&vertex[2],face.v3,sizeof(tvertex));
    }
    else if (face.v2->fpy<=falto_pantalla){
      memcpy(&vertex[0],face.v2,sizeof(tvertex));
      memcpy(&vertex[1],face.v3,sizeof(tvertex));
      memcpy(&vertex[2],face.v1,sizeof(tvertex));
    }
    else {
      memcpy(&vertex[0],face.v3,sizeof(tvertex));
      memcpy(&vertex[1],face.v1,sizeof(tvertex));
      memcpy(&vertex[2],face.v2,sizeof(tvertex));
    }
    cero_z=1.0/vertex[0].fpz;
    uno_z=1.0/vertex[1].fpz;
    dos_z=1.0/vertex[2].fpz;
    // Corte 1 con el margen superior de la pantalla
    fwork=(falto_pantalla-vertex[0].fpy)/(vertex[1].fpy-vertex[0].fpy);
    vertex[1].fpx=(vertex[1].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[1].fpy=falto_pantalla;
    vertex[1].u=((vertex[1].u*uno_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].v=((vertex[1].v*uno_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].r=((vertex[1].r*uno_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].g=((vertex[1].g*uno_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].b=((vertex[1].b*uno_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].intensidad=((vertex[1].intensidad*uno_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((uno_z-cero_z)*fwork+cero_z);
    vertex[1].fpz=1/((uno_z-cero_z)*fwork+cero_z);

    // Corte 2 con el plano de clipping
    fwork=(falto_pantalla-vertex[0].fpy)/(vertex[2].fpy-vertex[0].fpy);
    vertex[2].fpx=(vertex[2].fpx-vertex[0].fpx)*fwork+vertex[0].fpx;
    vertex[2].fpy=falto_pantalla;
    vertex[2].u=((vertex[2].u*dos_z-vertex[0].u*cero_z)*fwork+vertex[0].u*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].v=((vertex[2].v*dos_z-vertex[0].v*cero_z)*fwork+vertex[0].v*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].r=((vertex[2].r*dos_z-vertex[0].r*cero_z)*fwork+vertex[0].r*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].g=((vertex[2].g*dos_z-vertex[0].g*cero_z)*fwork+vertex[0].g*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].b=((vertex[2].b*dos_z-vertex[0].b*cero_z)*fwork+vertex[0].b*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].intensidad=((vertex[2].intensidad*dos_z-vertex[0].intensidad*cero_z)*fwork+vertex[0].intensidad*cero_z)/((dos_z-cero_z)*fwork+cero_z);
    vertex[2].fpz=1/((dos_z-cero_z)*fwork+cero_z);

    // Pintar
    face.v1=&vertex[0];
    face.v2=&vertex[1];
    face.v3=&vertex[2];
    pinta_triangulo(&face);
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#define swap(a,b) {(a)^=(b);(b)^=(a);(a)^=(b);}

void llrender::line(int x0, int y0, int x1, int y1, int color)
{
int dx,dy,a,b,d,x,y;
short *scopia=(short *)copia;
short *destino;
int    svancho=vancho/2;

  if (x0>=ancho_pantalla) x0=ancho_pantalla-1;
  if (y0>=alto_pantalla) y0=alto_pantalla-1;
  if (x1>=ancho_pantalla) x1=ancho_pantalla-1;
  if (y1>=alto_pantalla) y1=alto_pantalla-1;

  if (x0>x1)
  {
    x=x1;
    dx=x0-x1;
  }
  else
  {
    x=x0;
    dx=x1-x0;
  }
  if (y0>y1)
  {
    y=y1;
    dy=y0-y1;
  }
  else
  {
    y=y0;
    dy=y1-y0;
  }

  if (!dx && !dy) {
//    scopia[y0*svancho+x0]=color;
    destino=scopia+y0*svancho+x0;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
  }
  else
  {
//    scopia[y0*svancho+x0]=color;
    destino=scopia+y0*svancho+x0;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
    if (dy<=dx)
    {
      if (x0>x1)
      {
//	scopia[y1*svancho+x1]=color;
    destino=scopia+y1*svancho+x1;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
	x0--;
	swap(x0,x1);
	swap(y0,y1);
      }
      d=2*dy-dx;
      a=2*dy;
      b=2*(dy-dx);
      x=x0;
      y=y0;
      if (y0<=y1)
	while (x<x1)
	{
	  if (d<=0)
	  {
	    d+=a;
	    x++;
	  }
	  else
	  {
	    d+=b;
	    x++;
	    y++;
	  }
//	scopia[y*svancho+x]=color;
    destino=scopia+y*svancho+x;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
	}
      else
	while (x<x1)
	{
	  if (d<=0)
	  {
	    d+=a;
	    x++;
	  }
	  else
	  {
	    d+=b;
	    x++;
	    y--;
	  }
//	  scopia[y*svancho+x]=color;
    destino=scopia+y*svancho+x;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
	}
    }
    else
    {
      if (y0>y1)
      {
//	scopia[y1*svancho+x1]=color;
    destino=scopia+y1*svancho+x1;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
	y0--;
	swap(x0,x1);
	swap(y0,y1);
      }
      d=2*dx-dy;
      a=2*dx;
      b=2*(dx-dy);
      x=x0;
      y=y0;
      if (x0<=x1)
	while (y<y1)
	{
	  if (d<=0)
	  {
	    d+=a;
	    y++;
	  }
	  else
	  {
	    d+=b;
	    y++;
	    x++;
	  }
//	  scopia[y*svancho+x]=color;
    destino=scopia+y*svancho+x;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
	}
      else
	while (y<y1)
	{
	  if (d<=0)
	  {
	    d+=a;
	    y++;
	  }
	  else
	  {
	    d+=b;
	    y++;
	    x--;
	  }
//	  scopia[y*svancho+x]=color;
    destino=scopia+y*svancho+x;
    _asm {
      pushad
      mov edi,destino
      mov ebx,color
      mov edx,ebx
      and ebx,0f81fh
      and edx,07e0h

      mov ax,[edi]
      and eax,0f81fh
      add eax,ebx
      shr eax,1
      and eax,0f81fh
      mov bp,[edi]
      and ebp,07e0h
      add ebp,edx
      shr ebp,1
      and ebp,07e0h
      add eax,ebp
      mov [edi],ax
      popad
    }
	}
    }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Mapea una textura sobre un triangulo con gouraud
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void llrender::Pinta_Triangulo(lptface face, int mmlevel, int tipo)
{
  int altura;
  float faltura,temp,largo;
  int x1,u,v,dudx,dvdx;
  short *ptr_pantalla;
///////////////////////////////////////////////////////
  lptmaterial mat=face->material;
  int ancho=mat->textura->Ancho[mmlevel];
  int alto =mat->textura->Alto[mmlevel];
  short *dir_textura=(short *)mat->textura->Levels[mmlevel];
  direccion_textura=(long)mat->textura->Levels[mmlevel];
///////////////////////////////////////////////////////
  float anchot=(float)ancho;
  float altot=(float)ancho;
  lptvertex v1=face->v1;
  lptvertex v2=face->v2;
  lptvertex v3=face->v3;
  lptvertex vertice;
  long mascara=(ancho-1)*65536;
  long rotacion_mascara;
  float unoz_aux,dunoz_aux,fdudx,fdvdx;
  float dxdy,prestep;
  int preincremento;
  long v1_r,v1_g,v1_b;
  long v2_r,v2_g,v2_b;
  long v3_r,v3_g,v3_b;
  long v1_alpha,v2_alpha,v3_alpha;
  long col_r,col_g,col_b,color;
  float ru1,ru2,ru3,rv1,rv2,rv3;
  short *tabla_rgb=tabla_rgba;

  if (!(tipo & T_TEXTURA)) {
    col_r=schop(face->material->dr*65536.0)&0xf800;
    col_g=schop(face->material->dg*2048.0)&0x07e0;
    col_b=schop(face->material->db*32.0)&0x1f;
    color=col_r+col_g+col_b;
  }

  switch (ancho) {
    case 8:   if (tipo & T_MASCARA) nucleo_texturas=mask_nucleo8_8;
              else nucleo_texturas=nucleo8_8;
              rotacion_mascara=13;
              break;
    case 16:  if (tipo & T_MASCARA) nucleo_texturas=mask_nucleo8_16;
              else nucleo_texturas=nucleo8_16;
              rotacion_mascara=12;
              break;
    case 32:  if (tipo & T_MASCARA) nucleo_texturas=mask_nucleo8_32;
              else nucleo_texturas=nucleo8_32;
              rotacion_mascara=11;
              break;
    case 64:  if (tipo & T_MASCARA) nucleo_texturas=mask_nucleo8_64;
              else nucleo_texturas=nucleo8_64;
              rotacion_mascara=10;
              break;
    case 128: if (tipo & T_MASCARA) nucleo_texturas=mask_nucleo8_128;
              else nucleo_texturas=nucleo8_128;
              rotacion_mascara=9;
              break;
    case 256: if (tipo & T_MASCARA) nucleo_texturas=mask_nucleo8_256;
              else nucleo_texturas=nucleo8_256;
              rotacion_mascara=8;
              break;
  }

  //Ordenamos los vertices, v1 el de m s arriba y v3 el de m s abajo

  if (v1->fpy > v2->fpy) { vertice=v1; v1=v2; v2=vertice; }
  if (v1->fpy > v3->fpy) { vertice=v1; v1=v3; v3=vertice; }
  if (v2->fpy > v3->fpy) { vertice=v2; v2=v3; v3=vertice; }

  if (tipo & T_TEXTURA) {
    if (tipo & T_PERSPECTIVA) {
      ru1=65536.0*v1->u*anchot/v1->fpz; ru2=65536.0*v2->u*anchot/v2->fpz; ru3=65536.0*v3->u*anchot/v3->fpz;
      rv1=65536.0*v1->v*altot/v1->fpz; rv2=65536.0*v2->v*altot/v2->fpz; rv3=65536.0*v3->v*altot/v3->fpz;
    }
    else {
      rv1=65536.0*v1->v*altot; rv2=65536.0*v2->v*altot; rv3=65536.0*v3->v*altot;
      ru1=65536.0*v1->u*anchot; ru2=65536.0*v2->u*anchot; ru3=65536.0*v3->u*anchot;
    }
  }

  if (tipo & T_GOURAUD) {
/*
    v1->r=filtro_iluminacion[i1]/8;
    v1->g=filtro_iluminacion[i1]/8;
    v1->b=filtro_iluminacion[i1]/8;

    v2->r=filtro_iluminacion[i2]/8;
    v2->g=filtro_iluminacion[i2]/8;
    v2->b=filtro_iluminacion[i2]/8;

    v3->r=filtro_iluminacion[i3]/8;
    v3->g=filtro_iluminacion[i3]/8;
    v3->b=filtro_iluminacion[i3]/8;
    i1/=8;
    i2/=8;
    i3/=8;
*/
    v1_alpha=v1->intensidad*31;
    v2_alpha=v2->intensidad*31;
    v3_alpha=v3->intensidad*31;

    v1_r=v1->r*63;
    v1_g=v1->g*63;
    v1_b=v1->b*63;

    v2_r=v2->r*63;
    v2_g=v2->g*63;
    v2_b=v2->b*63;

    v3_r=v3->r*63;
    v3_g=v3->g*63;
    v3_b=v3->b*63;
  }

  //Calculamos la longitud de la linea m s larga
  altura=v3->py-v1->py;
  if ((v3->fpy-v1->fpy)<0) return;
//  if (altura<=0) return;

  faltura=1.0/(v3->fpy-v1->fpy);
  temp=(v2->fpy-v1->fpy)*faltura;
  largo=temp*(v3->fpx-v1->fpx)+(v1->fpx-v2->fpx);
  if (largo==0) return;

  if (largo<0) {
    seccion_derecha=2;
    seccion_izquierda=1;
    altura_seccion_izquierda=altura;
    dxdy=(v3->fpx-v1->fpx)*faltura;
    dx_izq=schop(dxdy*65536.0);
    prestep=(float)v1->py-v1->fpy;
    x_izq=schop((v1->fpx+prestep*dxdy)*65536.0);
    //Calculo de las deltas
    if (tipo & T_TEXTURA) {
      if (tipo & T_PERSPECTIVA) {
        duz_izq=(ru3-ru1)*faltura;
        uz_izq=ru1+duz_izq*prestep;
        dvz_izq=(rv3-rv1)*faltura;
        vz_izq=rv1+dvz_izq*prestep;
        dunoz_izq=(1/v3->fpz-1/v1->fpz)*faltura;
        unoz_izq=1/v1->fpz+dunoz_izq*prestep;
      }
      else {
        du_izq=schop((ru3-ru1)*faltura);
        u_izq=schop(ru1+(float)du_izq*prestep);
        dv_izq=schop((rv3-rv1)*faltura);
        v_izq=schop(rv1+(float)dv_izq*prestep);
      }
    }
    if (tipo & T_GOURAUD) {
      dr_izq=schop(((v3_r-v1_r)*256.0)*faltura);
      r_izq=schop(v1_r*256.0+(float)dr_izq*prestep);
      dg_izq=schop(((v3_g-v1_g)*256.0)*faltura);
      g_izq=schop(v1_g*256.0+(float)dg_izq*prestep);
      db_izq=schop(((v3_b-v1_b)*256.0)*faltura);
      b_izq=schop(v1_b*256.0+(float)db_izq*prestep);
      dintensidad_izq=schop(((v3_alpha-v1_alpha)*256.0)*faltura);
      intensidad_izq=schop(v1_alpha*256.0+(float)dintensidad_izq*prestep);
    }
    altura_seccion_derecha=v2->py-v1->py;
    if (altura_seccion_derecha<=0) {
      altura_seccion_derecha=v3->py-v2->py;
      if (altura_seccion_derecha<=0) return;
      else {
        //Calculo de las deltas
        dx_der=schop(((v3->fpx-v2->fpx)*65536.0)/(v3->fpy-v2->fpy));
        faltura=1.0/(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)*faltura;
        prestep=(float)v2->py-v2->fpy;
        x_der=schop((v2->fpx+prestep*dxdy)*65536.0);
        if ((tipo & T_TEXTURA) && (tipo & T_PERSPECTIVA)) {
          duz_der=(ru3-ru2)*faltura;
          uz_der=ru2+duz_der*prestep;
          dvz_der=(rv3-rv2)*faltura;
          vz_der=rv2+dvz_der*prestep;
          dunoz_der=(1/v3->fpz-1/v2->fpz)*faltura;
          unoz_der=1/v2->fpz+dunoz_der*prestep;
        }
      }
    }
    else {
      //Calculo de las deltas
      dx_der=schop(((v2->fpx-v1->fpx)*65536.0)/(v2->fpy-v1->fpy));
      faltura=1.0/(v2->fpy-v1->fpy);
      dxdy=(v2->fpx-v1->fpx)*faltura;
      prestep=(float)v1->py-v1->fpy;
      x_der=schop((v1->fpx+prestep*dxdy)*65536.0);
      if ((tipo & T_TEXTURA) && (tipo & T_PERSPECTIVA)) {
        duz_der=(ru2-ru1)*faltura;
        uz_der=ru1+duz_der*prestep;
        dvz_der=(rv2-rv1)*faltura;
        vz_der=rv1+dvz_der*prestep;
        dunoz_der=(1/v2->fpz-1/v1->fpz)*faltura;
        unoz_der=1/v1->fpz+dunoz_der*prestep;
      }
    }
  }
  else {
    seccion_izquierda=2;
    seccion_derecha=1;
    altura_seccion_derecha=altura;
    dx_der=schop(((v3->fpx-v1->fpx)*65536.0)/(v3->fpy-v1->fpy));
    faltura=1.0/(v3->fpy-v1->fpy);
    dxdy=(v3->fpx-v1->fpx)*faltura;
    prestep=(float)v1->py-v1->fpy;
    x_der=schop((v1->fpx+prestep*dxdy)*65536.0);
    if ((tipo & T_TEXTURA) && (tipo & T_PERSPECTIVA)) {
      duz_der=(ru3-ru1)*faltura;
      uz_der=ru1+duz_der*prestep;
      dvz_der=(rv3-rv1)*faltura;
      vz_der=rv1+dvz_der*prestep;
      dunoz_der=(1/v3->fpz-1/v1->fpz)*faltura;
      unoz_der=1/v1->fpz+dunoz_der*prestep;
    }
    altura_seccion_izquierda=v2->py-v1->py;
    if (altura_seccion_izquierda<=0) {
      seccion_izquierda--;
      altura_seccion_izquierda=v3->py-v2->py;
      if (altura_seccion_izquierda<=0)
        return;
      else {
        faltura=1.0/(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)*faltura;
        dx_izq=schop(dxdy*65536.0);
        prestep=(float)v2->py-v2->fpy;
        x_izq=schop((v2->fpx+prestep*dxdy)*65536.0);
        //Calculo de las deltas
        if (tipo & T_TEXTURA) {
          if (tipo & T_PERSPECTIVA) {
            duz_izq=(ru3-ru2)*faltura;
            uz_izq=ru2+duz_izq*prestep;
            dvz_izq=(rv3-rv2)*faltura;
            vz_izq=rv2+dvz_izq*prestep;
            dunoz_izq=(1/v3->fpz-1/v2->fpz)*faltura;
            unoz_izq=1/v2->fpz+dunoz_izq*prestep;
          }
          else {
            du_izq=schop((ru3-ru2)*faltura);
            u_izq=schop(ru2+(float)du_izq*prestep);
            dv_izq=schop((rv3-rv2)*faltura);
            v_izq=schop(rv2+(float)dv_izq*prestep);
          }
        }
        if (tipo & T_GOURAUD) {
          dr_izq=schop(((v3_r-v2_r)*256.0)*faltura);
          r_izq=schop(v2_r*256.0+(float)dr_izq*prestep);
          dg_izq=schop(((v3_g-v2_g)*256.0)*faltura);
          g_izq=schop(v2_g*256.0+(float)dg_izq*prestep);
          db_izq=schop(((v3_b-v2_b)*256.0)*faltura);
          b_izq=schop(v2_b*256.0+(float)db_izq*prestep);
          dintensidad_izq=schop(((v3_alpha-v2_alpha)*256.0)*faltura);
          intensidad_izq=schop(v2_alpha*256.0+(float)dintensidad_izq*prestep);
        }
      }
    }
    else {
      faltura=1.0/(v2->fpy-v1->fpy);
      dxdy=(v2->fpx-v1->fpx)*faltura;
      dx_izq=schop(dxdy*65536.0);
      prestep=(float)v1->py-v1->fpy;
      x_izq=schop((v1->fpx+prestep*dxdy)*65536.0);
      //Calculo de las deltas
      if (tipo & T_TEXTURA) {
        if (tipo & T_PERSPECTIVA) {
          duz_izq=(ru2-ru1)*faltura;
          uz_izq=ru1+duz_izq*prestep;
          dvz_izq=(rv2-rv1)*faltura;
          vz_izq=rv1+dvz_izq*prestep;
          dunoz_izq=(1/v2->fpz-1/v1->fpz)*faltura;
          unoz_izq=1/v1->fpz+dunoz_izq*prestep;
        }
        else {
          du_izq=schop((ru2-ru1)*faltura);
          u_izq=schop(ru1+(float)du_izq*prestep);
          dv_izq=schop((rv2-rv1)*faltura);
          v_izq=schop(rv1+(float)dv_izq*prestep);
        }
      }
      if (tipo & T_GOURAUD) {
        dr_izq=schop(((v2_r-v1_r)*256.0)*faltura);
        r_izq=schop(v1_r*256.0+(float)dr_izq*prestep);
        dg_izq=schop(((v2_g-v1_g)*256.0)*faltura);
        g_izq=schop(v1_g*256.0+(float)dg_izq*prestep);
        db_izq=schop(((v2_b-v1_b)*256.0)*faltura);
        b_izq=schop(v1_b*256.0+(float)db_izq*prestep);
        dintensidad_izq=schop(((v2_alpha-v1_alpha)*256.0)*faltura);
        intensidad_izq=schop(v1_alpha*256.0+(float)dintensidad_izq*prestep);
      }
    }
  }

  if ((tipo & T_TEXTURA) && !(tipo & T_PERSPECTIVA)) {
    dudx=schop((temp*(ru3-ru1)+(ru1-ru2))/largo);
    dvdx=schop((temp*(rv3-rv1)+(rv1-rv2))/largo);
  }

  if (tipo & T_GOURAUD) {
    drdx=schop( 256.0*((temp*(v3_r-v1_r)+(v1_r-v2_r))/largo) );
    dgdx=schop( 256.0*((temp*(v3_g-v1_g)+(v1_g-v2_g))/largo) );
    dbdx=schop( 256.0*((temp*(v3_b-v1_b)+(v1_b-v2_b))/largo) );
    dintensidaddx=schop( 256.0*((temp*(v3_alpha-v1_alpha)+(v1_alpha-v2_alpha))/largo) );
  }

  ptr_pantalla=(short *)(copia+(v1->py)*vancho);

  for (;;) { //Bucle que recorre el pol¡gono de arriba a abajo
    x1=ajuste_entero(x_izq);
    ancho=ajuste_entero(x_der)-x1;

    if (tipo & T_TEXTURA) {
      if (tipo & T_PERSPECTIVA) {
        anchot=16.0/(float)ancho;

        inicio_division(&unoz_izq);
        destino=ptr_pantalla+x1;

        if (ancho>16) {
          preincremento=(x1<<16)-x_izq;
//          fu=uz_izq+(float)preincremento*duz_izq/65536.0;
//          fv=vz_izq+(float)preincremento*dvz_izq/65536.0;
          fu=uz_izq;
          fv=vz_izq;
          fin_division();
          fdudx=(uz_der-uz_izq)*anchot;
          fdvdx=(vz_der-vz_izq)*anchot;
          dunoz_aux=(unoz_der-unoz_izq)*anchot;
          unoz_aux=unoz_izq+dunoz_aux;
          asignacion_u_v();
          inicio_division(&unoz_aux);
          u=u_0;
          v=v_0;
          fu+=fdudx;
          fv+=fdvdx;
          fin_division();
          asignacion_u_v();
          do {
          	dudx=(u_0-u)>>4;
          	dvdx=(v_0-v)>>4;
          	fu+=fdudx;
          	fv+=fdvdx;
            unoz_aux+=dunoz_aux;
            inicio_division(&unoz_aux);
            nucleo_texturas(v,u,dudx,dvdx,16,destino);
            fin_division();
            u=u_0;
            v=v_0;
            asignacion_u_v();
            destino+=16;
          	ancho-=16;
          } while (ancho>16);
          if (ancho>0) {
            dudx=(u_0-u)>>4;
            dvdx=(v_0-v)>>4;
            nucleo_texturas(v,u,dudx,dvdx,ancho,destino);
          }
        }
        else if (ancho>0) {
//          fu=uz_izq+(float)preincremento*duz_izq/65536.0;
//          fv=vz_izq+(float)preincremento*dvz_izq/65536.0;
          fu=uz_izq;
          fv=vz_izq;
          fin_division();
          asignacion_u_v();
          inicio_division(&unoz_der);
          u=u_0;
          v=v_0;
          fu=uz_der;
          fv=vz_der;
          fin_division();
          asignacion_u_v();
          if (ancho>1) {
            dudx=(u_0-u)/ancho;
            dvdx=(v_0-v)/ancho;
          }
          else {
            dudx=u_0-u;
            dvdx=v_0-v;
          }
          nucleo_texturas(v,u,dudx,dvdx,ancho,destino);
        }
        else fin_division();
      }
      else if (tipo & T_GOURAUD) {
        if (tipo & T_MASCARA) {
          if (ancho>0) {
            destino=ptr_pantalla+x1;
            preincremento=(x1<<16)-x_izq;
            u=u_izq+((preincremento*dudx)>>16);
            v=v_izq+((preincremento*dvdx)>>16);
            r=r_izq; g=g_izq; b=b_izq;
            intensidad=intensidad_izq;
            do {
              alfa=intensidad>>8;
    	        if ((color_b=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)])!=0) {

//                tabla_rgb[((r>>3)&0x7e0+color_b&0xf800+alfa)]

                nucleo1();
              }
    	        r+=drdx; g+=dgdx; b+=dbdx;
              intensidad+=dintensidaddx;
            	destino++;
            	u+=dudx;
            	v+=dvdx;
            } while (--ancho);
          }
        }
        else {
          if (ancho>0) {
            destino=ptr_pantalla+x1;
            preincremento=(x1<<16)-x_izq;
            u=u_izq+((preincremento*dudx)>>16);
            v=v_izq+((preincremento*dvdx)>>16);
            r=r_izq; g=g_izq; b=b_izq;
            intensidad=intensidad_izq;
            do {
              alfa=intensidad>>8;
    	        color_b=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)];
              nucleo1();
    	        r+=drdx; g+=dgdx; b+=dbdx;
              intensidad+=dintensidaddx;
            	destino++;
            	u+=dudx;
            	v+=dvdx;
            } while (--ancho);
          }
        }
      }
/*
      else if (tipo & T_TRANSLUCIDO) {
        if (tipo & T_MASCARA) {
          if (ancho>0) {
            destino=ptr_pantalla+x1;
            preincremento=(x1<<16)-x_izq;
            u=u_izq+((preincremento*dudx)>>16);
            v=v_izq+((preincremento*dvdx)>>16);
            do {
      	      if (color=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)]) {
                _asm {
                  pushad
                  mov edi,destino
                  mov ebx,color
                  mov edx,ebx
                  and ebx,0f81fh
                  and edx,07e0h

                  mov ax,[edi]
                  and eax,0f81fh
                  add eax,ebx
                  shr eax,1
                  and eax,0f81fh
                  mov bp,[edi]
                  and ebp,07e0h
                  add ebp,edx
                  shr ebp,1
                  and ebp,07e0h
                  add eax,ebp
                  mov [edi],ax
                  popad
                }
              }
          	  destino++;
            	u+=dudx;
            	v+=dvdx;
            } while (--ancho);
          }
        }
        else {
          if (ancho>0) {
            destino=ptr_pantalla+x1;
            preincremento=(x1<<16)-x_izq;
            u=u_izq+((preincremento*dudx)>>16);
            v=v_izq+((preincremento*dvdx)>>16);
            do {
    	        color=dir_textura[((v&mascara)>>rotacion_mascara)+((u&mascara)>>16)];
              _asm {
                pushad
                mov edi,destino
                mov ebx,color
                mov edx,ebx
                and ebx,0f81fh
                and edx,07e0h

                mov ax,[edi]
                and eax,0f81fh
                add eax,ebx
                shr eax,1
                and eax,0f81fh
                mov bp,[edi]
                and ebp,07e0h
                add ebp,edx
                shr ebp,1
                and ebp,07e0h
                add eax,ebp
                mov [edi],ax
                popad
              }
            	destino++;
          	  u+=dudx;
            	v+=dvdx;
            } while (--ancho);
          }
        }
      }
*/
      else {
        if (ancho>0) {
          destino=ptr_pantalla+x1;
          nucleo_texturas(v_izq,u_izq,dudx,dvdx,ancho,destino);
        }
      }
    }
/*
    else {
      if (tipo & T_TRANSLUCIDO) {
        if (ancho>0) {
          destino=ptr_pantalla+x1;
          _asm {
            pushad
            mov edi,destino
            mov ebx,color
            mov ecx,ancho
            mov edx,ebx
            and ebx,0f81fh
            and edx,07e0h
          bucle1:
            mov ax,[edi]
            and eax,0f81fh
            add eax,ebx
            shr eax,1
            and eax,0f81fh
            mov bp,[edi]
            and ebp,07e0h
            add ebp,edx
            shr ebp,1
            and ebp,07e0h
            add eax,ebp
            mov [edi],ax
            add edi,2
            dec ecx
            jnz bucle1
            popad
          }
        }
      }
      else if (tipo & T_GOURAUD) {
        if (ancho>0) {
          destino=ptr_pantalla+x1;
          r=r_izq; g=g_izq; b=b_izq;
          do {
            _asm {
              pushad
              mov edi,destino
              mov ebx,r
              cmp ebx,10000h
              jbe label1
              mov ebx,0f800h
            label1:
              and ebx,0f800h
              mov edx,g
              cmp edx,10000h
              jbe label2
              mov edx,0fc00h
            label2:
              and edx,0fc00h
              mov esi,b
              cmp esi,10000h
              jbe label3
              mov esi,0f800h
            label3:
              and esi,0f800h
              mov eax,ebx
              shr edx,5
              add eax,edx
              shr esi,11
              add eax,esi
              mov [edi],ax
              popad
            }
            destino++;
	          r+=drdx; g+=dgdx; b+=dbdx;
          } while (--ancho);
        }
      }
      else {
        if (ancho>0) {
          destino=ptr_pantalla+x1;
          lf_set(destino,color,ancho);
        }
      }
    }
*/
    ptr_pantalla+=ancho_pantalla;

    if (--altura_seccion_izquierda<=0) {
      if (--seccion_izquierda<=0) return;
      altura_seccion_izquierda=v3->py-v2->py;
      if (altura_seccion_izquierda<=0)
        return;
      else {
        faltura=1.0/(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)*faltura;
        dx_izq=schop(dxdy*65536.0);
        prestep=(float)v2->py-v2->fpy;
        x_izq=schop((v2->fpx+prestep*dxdy)*65536.0);
        //Calculo de las deltas
        if (tipo & T_TEXTURA) {
          if (tipo & T_PERSPECTIVA) {
            duz_izq=(ru3-ru2)*faltura;
            uz_izq=ru2+duz_izq*prestep;
            dvz_izq=(rv3-rv2)*faltura;
            vz_izq=rv2+dvz_izq*prestep;
            dunoz_izq=(1/v3->fpz-1/v2->fpz)*faltura;
            unoz_izq=1/v2->fpz+dunoz_izq*prestep;
          }
          else {
            du_izq=schop((ru3-ru2)*faltura);
            u_izq=schop(ru2+(float)du_izq*prestep);
            dv_izq=schop((rv3-rv2)*faltura);
            v_izq=schop(rv2+(float)dv_izq*prestep);
          }
        }
        if (tipo & T_GOURAUD) {
          dr_izq=schop(((v3_r-v2_r)*256.0)*faltura);
          r_izq=schop(v2_r*256.0+(float)dr_izq*prestep);
          dg_izq=schop(((v3_g-v2_g)*256.0)*faltura);
          g_izq=schop(v2_g*256.0+(float)dg_izq*prestep);
          db_izq=schop(((v3_b-v2_b)*256.0)*faltura);
          b_izq=schop(v2_b*256.0+(float)db_izq*prestep);
          dintensidad_izq=schop(((v3_alpha-v2_alpha)*256.0)*faltura);
          intensidad_izq=schop(v2_alpha*256.0+(float)dintensidad_izq*prestep);
        }
      }
    }
    else {
      x_izq+=dx_izq;
      if (tipo & T_TEXTURA) {
        if (tipo & T_PERSPECTIVA) {
          uz_izq+=duz_izq;
          vz_izq+=dvz_izq;
          unoz_izq+=dunoz_izq;
        }
        else {
          u_izq+=du_izq;
          v_izq+=dv_izq;
        }
      }
      if (tipo & T_GOURAUD) {
        r_izq+=dr_izq;
        g_izq+=dg_izq;
        b_izq+=db_izq;
        intensidad_izq+=dintensidad_izq;
      }
    }

    if (--altura_seccion_derecha<=0) {
      if (--seccion_derecha<=0) return;
      altura_seccion_derecha=v3->py-v2->py;
      if (altura_seccion_derecha<=0) return;
      else {
        //Calculo de las deltas
        dx_der=schop(((v3->fpx-v2->fpx)*65536.0)/(v3->fpy-v2->fpy));
        faltura=1.0/(v3->fpy-v2->fpy);
        dxdy=(v3->fpx-v2->fpx)*faltura;
        prestep=(float)v2->py-v2->fpy;
        x_der=schop((v2->fpx+prestep*dxdy)*65536.0);
        if ((tipo & T_TEXTURA) && (tipo & T_PERSPECTIVA)) {
          duz_der=(ru3-ru2)*faltura;
          uz_der=ru2+duz_der*prestep;
          dvz_der=(rv3-rv2)*faltura;
          vz_der=rv2+dvz_der*prestep;
          dunoz_der=(1/v3->fpz-1/v2->fpz)*faltura;
          unoz_der=1/v2->fpz+dunoz_der*prestep;
        }
      }
    }
    else {
      x_der+=dx_der;
      if ((tipo & T_TEXTURA) && (tipo & T_PERSPECTIVA)) {
        uz_der+=duz_der;
        vz_der+=dvz_der;
        unoz_der+=dunoz_der;
      }
    }
  }
}

