
//-----------------------------------------------------------------------------
//      Keyboard interrupt handler
//-----------------------------------------------------------------------------

#include "inter.h"
#include "osdep.h"


#define JOY_DEADZONE 500
int joymx = 0, joymy = 0;

#ifdef DEBUG
extern int mouse_b; //, mouse_y;
#endif


//-----------------------------------------------------------------------------
// IRQ Data
//-----------------------------------------------------------------------------

typedef void(__interrupt __far *TIRQHandler)(void);

TIRQHandler OldIrqHandler;
TIRQHandler OldIrq23;
TIRQHandler OldIrq1b;

byte buf[64 * 3]; // {ascii,scan_code,shift_status}
int ibuf = 0;     // Buffer pointer, start of queue
int fbuf = 0;     // Buffer pointer, end of queue

//-----------------------------------------------------------------------------
// Interrupt handler related functions.
//-----------------------------------------------------------------------------

int ctrl_c = 0, alt_x = 0;
int app_paused = 0;
int mouse_in_window = 1;

void __far __interrupt __loadds IrqHandler(void) {}

//-----------------------------------------------------------------------------
// Obtains the address of an IRQ handler.
//-----------------------------------------------------------------------------

TIRQHandler GetIRQVector(int n) {
  return NULL;
}

//-----------------------------------------------------------------------------
// Sets the address of an IRQ handler.
//-----------------------------------------------------------------------------

void SetIRQVector(int n, TIRQHandler vec) {}

//-----------------------------------------------------------------------------
// Function to initialize the handler.
//-----------------------------------------------------------------------------

void kbdInit(void) {
  OSDEP_keyInit();
}

//-----------------------------------------------------------------------------
// Function to uninitialize the handler.
//-----------------------------------------------------------------------------

void kbdReset(void) {}

//-----------------------------------------------------------------------------
//      Wait for a key, update variables (ascii, scan_code and shift_status)
//-----------------------------------------------------------------------------

void bios_key(void) {}


void checkmod(OSDEPMod mod) {
  if (mod == KMOD_NONE) {
    return;
  }
  if (mod & KMOD_LCTRL)
    shift_status |= 4;
  if (mod & KMOD_RCTRL)
    shift_status |= 4;
  if (mod & KMOD_RSHIFT)
    shift_status |= 1;
  if (mod & KMOD_LSHIFT)
    shift_status |= 2;
  if (mod & KMOD_RALT)
    shift_status |= 8;
  if (mod & KMOD_LALT)
    shift_status |= 8;
  if (mod & KMOD_CTRL)
    shift_status |= 4;
  if (mod & KMOD_ALT)
    shift_status |= 8;

  if (mod & KMOD_CAPS)
    shift_status |= 64;
  if (mod & KMOD_NUM)
    shift_status |= 32;
}

#include <SDL2/SDL_events.h>

void PrintEvent(const SDL_Event *event) {
  if (event->type == SDL_WINDOWEVENT) {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SHOWN:
      SDL_Log("Window %d shown", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_HIDDEN:
      SDL_Log("Window %d hidden", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_EXPOSED:
      SDL_Log("Window %d exposed", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_MOVED:
      SDL_Log("Window %d moved to %d,%d", event->window.windowID, event->window.data1,
              event->window.data2);
      break;
    case SDL_WINDOWEVENT_RESIZED:
      SDL_Log("Window %d resized to %dx%d", event->window.windowID, event->window.data1,
              event->window.data2);
      vwidth = event->window.data1;
      vheight = event->window.data2;
      break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      SDL_Log("Window %d size changed to %dx%d", event->window.windowID, event->window.data1,
              event->window.data2);
      break;
    case SDL_WINDOWEVENT_MAXIMIZED:
      SDL_Log("Window %d maximized", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_ENTER:
      mouse_in_window = 1;
      break;
    case SDL_WINDOWEVENT_LEAVE:
      mouse_in_window = 0;
      break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
    case SDL_WINDOWEVENT_RESTORED:
      app_paused = 0;
      Mix_Resume(-1);
      Mix_ResumeMusic();
      break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
    case SDL_WINDOWEVENT_MINIMIZED:
      app_paused = 1;
      Mix_Pause(-1);
      Mix_PauseMusic();
      break;
    case SDL_WINDOWEVENT_CLOSE:
      SDL_Log("Window %d closed", event->window.windowID);
      break;
    default:
      SDL_Log("Window %d got unknown event %d", event->window.windowID, event->window.event);
      break;
    }
  }
}


int8_t hx = 0, hy = 0; // hat xy positions

byte oldhatval;
void poll_keyboard(void) {
  SDL_Event event;
  if (vwidth == 0 && vheight == 0) {
    vwidth = vga_width;
    vheight = vga_height;
  }
  while (SDL_PollEvent(&event)) {
    // check keys
    PrintEvent(&event);

    /* If a quit event has been sent */
    if (event.type == SDL_QUIT)
      alt_x = 1;

    if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
        // handle special keys
      case SDLK_LSHIFT:
        shift_status |= 2;
        break;
      case SDLK_RSHIFT:
        shift_status |= 1;
        break;
      case SDLK_LCTRL:
        shift_status |= 4;
        break;
      case SDLK_RCTRL:
        shift_status |= 4;
        break;
      case SDLK_LALT:
        shift_status |= 8;
        break;
      case SDLK_RALT:
        shift_status |= 8;
        break;
      case SDLK_INSERT:
        shift_status |= 128;
      }

      scan_code = OSDEP_key[event.key.keysym.sym < 2048 ? event.key.keysym.sym
                                                        : event.key.keysym.sym - 0x3FFFFD1A];

      ascii = event.key.keysym.scancode;
      checkmod((OSDEPMod)event.key.keysym.mod);

      kbdFLAGS[scan_code] = 1;
    }


    if (event.type == SDL_KEYUP) {
      shift_status = 0;
      checkmod((OSDEPMod)event.key.keysym.mod);

      kbdFLAGS[OSDEP_key[event.key.keysym.sym < 2048 ? event.key.keysym.sym
                                                     : event.key.keysym.sym - 0x3FFFFD1A]] = 0;
    }

    if (event.type == SDL_MOUSEMOTION) {
      mouse->x = event.motion.x;
      mouse->y = event.motion.y;

      if (vga_width != vwidth || vga_height != vheight) {
        mouse->x = (int)(event.motion.x * (float)((float)vga_width / (float)vwidth));
        mouse->y = (int)(event.motion.y * (float)((float)vga_height / (float)vheight));
        SDL_Log("Mouse: VX: %d VY: %d x: %d y: %d\n", mouse->x, mouse->y, event.motion.x,
                event.motion.y);
      }
    }
    /* If a button on the mouse is pressed. */
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        mouse->left = 1;
#ifdef DEBUG
        mouse_b |= 1;
#endif
      }
      if (event.button.button == SDL_BUTTON_RIGHT) {
        mouse->right = 1;
#ifdef DEBUG
        mouse_b |= 2;
#endif
      }
      if (event.button.button == SDL_BUTTON_MIDDLE) {
        mouse->middle = 1;
#ifdef DEBUG
        mouse_b |= 4;
#endif
      }
    }

    if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        mouse->left = 0;
#ifdef DEBUG
        mouse_b ^= 1;
#endif
      }

      if (event.button.button == SDL_BUTTON_RIGHT) {
        mouse->right = 0;
#ifdef DEBUG
        mouse_b ^= 2;
#endif
      }

      if (event.button.button == SDL_BUTTON_MIDDLE) {
        mouse->middle = 0;
#ifdef DEBUG
        mouse_b ^= 4;
#endif
      }
    }

    if ((shift_status & 8) && scan_code == _x)
      alt_x = 1;

  } // end while
}

//-----------------------------------------------------------------------------
//      Flush the keyboard buffer (real and internal)
//-----------------------------------------------------------------------------

void flush_buffer(void) {
  ibuf = fbuf = 0; // Flush the internal buffer
}
