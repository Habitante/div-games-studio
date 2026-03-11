/*
 * Timer handler, calls a user function at the desired PIT clock frequency.
 *
 * Doesn't "officially" belong to JUDAS but feel free to use! This is
 * blasphemy-ware too!
 */

#include "global.h"

//#include <conio.h>
//#include <dos.h>
//#include <mem.h>
//#include "judas\judasmem.h"

int timer_init(unsigned short frequency, void (*function)());
void timer_uninit(void);
void timer_handler(void) {}

extern void(__interrupt __far *timer_oldvect)();
void(__interrupt __far *timer_newvect)() = &timer_handler;
extern void (*timer_function)();
//extern unsigned timer_count;
extern unsigned short timer_frequency;
extern unsigned short timer_systemcount;
extern unsigned short timer_ds;
extern int timer_code_lock_start;
extern int timer_code_lock_end;

int timer_init(unsigned short frequency, void (*function)()) {
  return 1;
}

void timer_uninit(void) {}
