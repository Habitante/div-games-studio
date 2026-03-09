// Clock desktop widget -- extracted from divcdrom.c during Phase 0 cleanup
// (The clock was bundled with the CD player code; the CD player is gone.)

#include <stdio.h>
#include <time.h>
#include "global.h"
#include "div_string.h"

static int ORDig1=-1, ORDig2=-1, ORDig3=-1, ORDig4=-1;
static char cTimeForIcon[18];
static struct tm *timeinfo;
static time_t dtime;

static void Show_Time(void) {
  byte *ptr = v.ptr;
  int an, al;
  char cBuff[3];
  int Dig1, Dig2, Dig3, Dig4;

  if (v.primer_plano != 2 || v.al > v._al) {
    an = v.an; al = v.al;
  } else {
    an = v._an; al = v._al;
  }
  if (big) { an /= 2; al /= 2; }

  time(&dtime);
  timeinfo = localtime(&dtime);
  DIV_SPRINTF(cTimeForIcon, "%s [%02d%c%02d]", texto[151],
          timeinfo->tm_hour, timeinfo->tm_sec % 2 ? ':' : '.', timeinfo->tm_min);

  sprintf(cBuff, "%02d", timeinfo->tm_hour);
  Dig1 = (cBuff[0] == '0') ? 10 + 200 : (cBuff[0] - '0') + 200;
  Dig2 = (cBuff[1] - '0') + 200;

  sprintf(cBuff, "%02d", timeinfo->tm_min);
  Dig3 = (cBuff[0] - '0') + 200;
  Dig4 = (cBuff[1] - '0') + 200;

  if (ORDig1 != Dig1) { wput(ptr, an, al, 8, 14, Dig1); v.volcar = 1; ORDig1 = Dig1; }
  if (ORDig2 != Dig2) { wput(ptr, an, al, 15, 14, Dig2); v.volcar = 1; ORDig2 = Dig2; }
  if (ORDig3 != Dig3) { wput(ptr, an, al, 26, 14, Dig3); v.volcar = 1; ORDig3 = Dig3; }
  if (ORDig4 != Dig4) { wput(ptr, an, al, 33, 14, Dig4); v.volcar = 1; ORDig4 = Dig4; }

  if (timeinfo->tm_sec % 2) {
    wbox(ptr, an, al, c4, 23, 17, 1, 1);
    wbox(ptr, an, al, c4, 23, 19, 1, 1);
  } else {
    wbox(ptr, an, al, c2, 23, 17, 1, 1);
    wbox(ptr, an, al, c2, 23, 19, 1, 1);
  }
  v.volcar = 1;
}

void Clock1(void) {
  byte *ptr = v.ptr;
  int an = v.an, al = v.al;
  if (big) { an /= 2; al /= 2; }
  wput(ptr, an, al, 2, 10, 218);
  ORDig1 = -1; ORDig2 = -1; ORDig3 = -1; ORDig4 = -1;
  Show_Time();
}

void Clock2(void) {
  Show_Time();
}

void Clock3(void) {
}

void Clock0(void) {
  v.tipo = 4;
  v.an = 47;
  v.al = 30;
  v.titulo = texto[151];
  time(&dtime);
  timeinfo = localtime(&dtime);
  DIV_SPRINTF(cTimeForIcon, "%s [%02d%c%02d]", texto[151],
          timeinfo->tm_hour, timeinfo->tm_sec % 2 ? ';' : ' ', timeinfo->tm_min);
  v.nombre = (byte *)cTimeForIcon;
  v.paint_handler = Clock1;
  v.click_handler = Clock2;
  v.close_handler = Clock3;
  ORDig1 = -1; ORDig2 = -1; ORDig3 = -1; ORDig4 = -1;
}

void muestra_reloj(void) {
  nueva_ventana(Clock0);
}
