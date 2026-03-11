
//-----------------------------------------------------------------------------
//      Keyboard interrupt handler
//-----------------------------------------------------------------------------

#include "global.h"
#include <time.h>

//-----------------------------------------------------------------------------
// IRQ Data
//-----------------------------------------------------------------------------
byte buf[64 * 3]; // {ascii,scan_code,shift_status}
int ibuf = 0;     // Buffer pointer, start of queue
int fbuf = 0;     // Buffer pointer, end of queue

//-----------------------------------------------------------------------------
// Interrupt handler related functions.
//-----------------------------------------------------------------------------

int ctrl_c = 0;

void __far __interrupt __loadds IrqHandler(void) {}
//-----------------------------------------------------------------------------
// Function to initialize the handler.
//-----------------------------------------------------------------------------

void kbdInit(void) {
  // call osdep keyboard init
  OSDEP_keyInit();
}

//-----------------------------------------------------------------------------
// Function to uninitialize the handler.
//-----------------------------------------------------------------------------

void kbdReset(void) {
  // nothing to do
}

//-----------------------------------------------------------------------------
//      Wait for a key, update variables (ascii, scan_code and shift_status)
//-----------------------------------------------------------------------------

void bios_key(void) {
  // nothing to do
}

extern int frame_clock;

//extern float m_x=0.0,m_y;
void poll_keyboard(void) {
  mclock = SDL_GetTicks() / 100; //(int)clock()/20000;
  frame_clock = SDL_GetTicks() / 100;  //(int)clock();
}

//-----------------------------------------------------------------------------
//     Empty the keyboard buffer (real and internal)
//-----------------------------------------------------------------------------

void flush_buffer(void) {
  ibuf = fbuf = 0; // Empty the internal buffer
}
