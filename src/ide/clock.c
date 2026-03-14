// Clock desktop widget -- extracted from divcdrom.c during Phase 0 cleanup
// (The clock was bundled with the CD player code; the CD player is gone.)

#include <stdio.h>
#include <time.h>
#include "global.h"
#include "div_string.h"

static int old_dig1 = -1, old_dig2 = -1, old_dig3 = -1, old_dig4 = -1;
static char cTimeForIcon[18];
static struct tm *timeinfo;
static time_t dtime;

static void show_time(void) {
  byte *ptr = v.ptr;
  int w, h;
  char cBuff[3];
  int dig1, dig2, dig3, dig4;

  if (v.foreground != WF_MINIMIZED || v.h > v._h_saved) {
    w = v.w;
    h = v.h;
  } else {
    w = v._w_saved;
    h = v._h_saved;
  }
  if (big) {
    w /= 2;
    h /= 2;
  }

  time(&dtime);
  timeinfo = localtime(&dtime);
  DIV_SPRINTF(cTimeForIcon, "%s [%02d%c%02d]", texts[151], timeinfo->tm_hour,
              timeinfo->tm_sec % 2 ? ':' : '.', timeinfo->tm_min);

  div_snprintf(cBuff, sizeof(cBuff), "%02d", timeinfo->tm_hour);
  dig1 = (cBuff[0] == '0') ? 10 + 200 : (cBuff[0] - '0') + 200;
  dig2 = (cBuff[1] - '0') + 200;

  div_snprintf(cBuff, sizeof(cBuff), "%02d", timeinfo->tm_min);
  dig3 = (cBuff[0] - '0') + 200;
  dig4 = (cBuff[1] - '0') + 200;

  if (old_dig1 != dig1) {
    wput(ptr, w, h, 8, 14, dig1);
    v.redraw = 1;
    old_dig1 = dig1;
  }
  if (old_dig2 != dig2) {
    wput(ptr, w, h, 15, 14, dig2);
    v.redraw = 1;
    old_dig2 = dig2;
  }
  if (old_dig3 != dig3) {
    wput(ptr, w, h, 26, 14, dig3);
    v.redraw = 1;
    old_dig3 = dig3;
  }
  if (old_dig4 != dig4) {
    wput(ptr, w, h, 33, 14, dig4);
    v.redraw = 1;
    old_dig4 = dig4;
  }

  if (timeinfo->tm_sec % 2) {
    wbox(ptr, w, h, c4, 23, 17, 1, 1);
    wbox(ptr, w, h, c4, 23, 19, 1, 1);
  } else {
    wbox(ptr, w, h, c2, 23, 17, 1, 1);
    wbox(ptr, w, h, c2, 23, 19, 1, 1);
  }
  v.redraw = 1;
}

void clock1(void) {
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  if (big) {
    w /= 2;
    h /= 2;
  }
  wput(ptr, w, h, 2, 10, 218);
  old_dig1 = -1;
  old_dig2 = -1;
  old_dig3 = -1;
  old_dig4 = -1;
  show_time();
}

void clock2(void) {
  show_time();
}

void clock3(void) {}

void clock0(void) {
  v.type = WIN_CLOCK;
  v.w = 47;
  v.h = 30;
  v.title = texts[151];
  time(&dtime);
  timeinfo = localtime(&dtime);
  DIV_SPRINTF(cTimeForIcon, "%s [%02d%c%02d]", texts[151], timeinfo->tm_hour,
              timeinfo->tm_sec % 2 ? ';' : ' ', timeinfo->tm_min);
  v.name = (byte *)cTimeForIcon;
  v.paint_handler = clock1;
  v.click_handler = clock2;
  v.close_handler = clock3;
  old_dig1 = -1;
  old_dig2 = -1;
  old_dig3 = -1;
  old_dig4 = -1;
}

void show_clock(void) {
  new_window(clock0);
}
