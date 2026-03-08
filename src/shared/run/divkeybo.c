
//-----------------------------------------------------------------------------
//      Pruebas con la interrupción de teclado
//-----------------------------------------------------------------------------

#include "inter.h"
#include "osdep.h"


#define JOY_DEADZONE 500
int joymx = 0, joymy=0;

#ifdef DEBUG
extern int mouse_b;//, mouse_y;
#endif


//-----------------------------------------------------------------------------
// IRQ Data
//-----------------------------------------------------------------------------

typedef void (__interrupt __far *TIRQHandler)(void);

TIRQHandler OldIrqHandler;
TIRQHandler OldIrq23;
TIRQHandler OldIrq1b;

// DOS BIOS shift-status byte at 0x417 -- only meaningful under real-mode DOS.
// On SDL2 builds shift_status is maintained by the SDL event loop instead.
#ifdef DOS
byte * shift = (byte *) 0x417; // Shift status
#endif

byte buf[64*3]; // {ascii,scan_code,shift_status}
int ibuf=0; // Puntero al buffer, inicio de la cola
int fbuf=0; // Puntero al buffer, fin de la cola

//-----------------------------------------------------------------------------
// Interrupt handler related functions.
//-----------------------------------------------------------------------------

int ctrl_c=0,alt_x=0;
int app_paused=0;
int mouse_in_window=1;

void __far __interrupt __loadds IrqHandler(void)
{
#ifdef DOS
  int n;
  int scancode;

  scancode=inp(0x60);

  if (((*shift)&4) && scancode==46) {
    kbdFLAGS[46]=1; ctrl_c=1;
    outp(0x20,0x20);
  } else {

    if (scancode<128) {
      kbdFLAGS[scancode]=1;
    } else if (scancode>129) {
      kbdFLAGS[scancode-128]=0;
    } else {
      n=128; do { kbdFLAGS[--n]=0; } while (n);
    }

    OldIrqHandler();

    if ((*kb_end-28)%32+30==*kb_start) *kb_start=(*kb_start-28)%32+30;

  }
#endif
}

void __far __interrupt __loadds Irq23(void){}
void __far __interrupt __loadds Irq1b(void){}

//-----------------------------------------------------------------------------
// Obtains the address of an IRQ handler.
//-----------------------------------------------------------------------------

TIRQHandler GetIRQVector(int n)
{
#ifdef DOS
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x3500 + n;   // DOS4GW redefines the DOS get vector call.
    sregs.ds = sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
    return (TIRQHandler)(MK_FP((word)sregs.es, outregs.x.ebx));
#endif
  return NULL;
}

//-----------------------------------------------------------------------------
// Sets the address of an IRQ handler.
//-----------------------------------------------------------------------------

void SetIRQVector(int n, TIRQHandler vec)
{
#ifdef DOS
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x2500 + n;   // DOS set vector call.
    inregs.x.edx = FP_OFF (vec);
    sregs.ds     = FP_SEG (vec);     // Handler pointer.
    sregs.es     = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
#endif
}

//-----------------------------------------------------------------------------
// Function to initialize the handler.
//-----------------------------------------------------------------------------

void kbdInit(void)
{
	OSDEP_keyInit();
	
#ifdef DOS
    if (GetIRQVector(9) != IrqHandler) {   // If not already installed.
      OldIrqHandler=GetIRQVector(9);       // Get old handler.
      SetIRQVector(9,IrqHandler);          // Set our handler.
      OldIrq1b=GetIRQVector(0x1b);         // Get old handler.
      SetIRQVector(0x1b,Irq1b);            // Set our handler.
      OldIrq23=GetIRQVector(0x23);         // Get old handler.
      SetIRQVector(0x23,Irq23);            // Set our handler.
    }

    signal(SIGBREAK,SIG_IGN);
    signal(SIGINT,SIG_IGN);
#endif
}

//-----------------------------------------------------------------------------
// Function to uninitialize the handler.
//-----------------------------------------------------------------------------

void kbdReset(void)
{
#ifdef DOS
    if (GetIRQVector(9) == IrqHandler) {   // If it was installed.
      SetIRQVector(9,OldIrqHandler);       // Uninstall it.
      SetIRQVector(0x1b,OldIrq1b);         // Uninstall it.
      SetIRQVector(0x23,OldIrq23);         // Uninstall it.
    }
#endif
}

//-----------------------------------------------------------------------------
//      Espera una tecla, actualiza variables (ascii,scan_code y shift_status)
//-----------------------------------------------------------------------------

void tecla_bios(void) {
#ifdef DOS
  int as,sc,ss;
  union REGS r;
  struct SREGS s;

  if (_bios_keybrd(_KEYBRD_READY)) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    do {
      r.h.ah=0; int386x(0x16,&r,&r,&s);
      as=r.h.al; sc=r.h.ah;
      r.h.ah=2; int386x(0x16,&r,&r,&s); ss=r.h.al;
      buf[fbuf]=as; buf[fbuf+1]=sc; buf[fbuf+2]=ss;
      if ((fbuf+=3)==64*3) fbuf=0;

    } while (_bios_keybrd(_KEYBRD_READY));
  }

  if (ctrl_c==1) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=2; int386x(0x16,&r,&r,&s); ss=r.h.al;
    buf[fbuf]=0; buf[fbuf+1]=46; buf[fbuf+2]=ss;
    if ((fbuf+=3)==64*3) fbuf=0; ctrl_c=0;
  }
#endif
}


void checkmod(OSDEPMod mod) {
	
	if( mod == KMOD_NONE ){
            return;
        }
        if( mod & KMOD_LCTRL ) shift_status |=4; 
       if( mod & KMOD_RCTRL ) shift_status |=4;
        if( mod & KMOD_RSHIFT ) shift_status |=1;
        if( mod & KMOD_LSHIFT ) shift_status |=2;
        if( mod & KMOD_RALT ) shift_status |=8;
        if( mod & KMOD_LALT ) shift_status |=8;
        if( mod & KMOD_CTRL ) shift_status |=4;
        if( mod & KMOD_ALT ) shift_status |=8;
        
        if (mod & KMOD_CAPS) shift_status |=64;
        if (mod & KMOD_NUM) shift_status |=32;
}

#include <SDL2/SDL_events.h>

void PrintEvent(const SDL_Event * event)
{
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
            SDL_Log("Window %d moved to %d,%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            SDL_Log("Window %d resized to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
			vwidth = event->window.data1;
			vheight = event->window.data2;
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            mouse_in_window=1;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            mouse_in_window=0;
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        case SDL_WINDOWEVENT_RESTORED:
            app_paused=0;
            Mix_Resume(-1);
            Mix_ResumeMusic();
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
        case SDL_WINDOWEVENT_MINIMIZED:
            app_paused=1;
            Mix_Pause(-1);
            Mix_PauseMusic();
            break;
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event->window.windowID);
            break;
        default:
            SDL_Log("Window %d got unknown event %d",
                    event->window.windowID, event->window.event);
            break;
        }
    }
}


int8_t hx=0,hy=0; // hat xy positions

byte oldhatval;
void tecla(void) {
SDL_Event event;
if(vwidth == 0 && vheight == 0) {
	vwidth = vga_an;
	vheight = vga_al;
}
	while(SDL_PollEvent(&event)) {	
		// check keys
	PrintEvent(&event);

            /* If a quit event has been sent */
		if (event.type == SDL_QUIT)
			alt_x=1;
            
		if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
// handle special keys
			case SDLK_LSHIFT:
				shift_status|=2;
				break;
			case SDLK_RSHIFT:
				shift_status|=1;
				break;
			case SDLK_LCTRL:
				shift_status|=4;
				break;
			case SDLK_RCTRL:
				shift_status|=4;
				break;
			case SDLK_LALT:
				shift_status|=8;
				break;
			case SDLK_RALT:
				shift_status|=8;
				break;
			case SDLK_INSERT:
				shift_status|=128;
		}

			scan_code = OSDEP_key[event.key.keysym.sym<2048?event.key.keysym.sym:event.key.keysym.sym-0x3FFFFD1A];
		
		ascii = event.key.keysym.scancode;
		checkmod((OSDEPMod) event.key.keysym.mod);

		kbdFLAGS[scan_code]=1;
	}
	
	
	if(event.type == SDL_KEYUP) {
		shift_status=0;
		checkmod((OSDEPMod) event.key.keysym.mod);
		
		kbdFLAGS[OSDEP_key[event.key.keysym.sym<2048?event.key.keysym.sym:event.key.keysym.sym-0x3FFFFD1A]]=0;
	}
	
	if (event.type == SDL_MOUSEMOTION) {
		mouse->x = event.motion.x;
		mouse->y = event.motion.y;

		if(vga_an != vwidth || vga_al != vheight) {
			mouse->x = (int)(event.motion.x*(float)((float)vga_an / (float)vwidth));
			mouse->y = (int)(event.motion.y*(float)((float)vga_al / (float)vheight));
			SDL_Log("Mouse: VX: %d VY: %d x: %d y: %d\n",mouse->x, mouse->y, event.motion.x,event.motion.y);
		}

	}
		/* If a button on the mouse is pressed. */
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if(event.button.button == SDL_BUTTON_LEFT) {
			mouse->	left = 1;
#ifdef DEBUG
			mouse_b|=1;
#endif
		}
		if(event.button.button == SDL_BUTTON_RIGHT) {
			mouse->right = 1;
#ifdef DEBUG
			mouse_b|=2;
#endif
		}
		if(event.button.button == SDL_BUTTON_MIDDLE) {
			mouse->middle = 1;
#ifdef DEBUG
			mouse_b|=4;
#endif
		}
	}
		
	if (event.type == SDL_MOUSEBUTTONUP) {

		if(event.button.button == SDL_BUTTON_LEFT) {
			mouse->left = 0;
#ifdef DEBUG
			mouse_b ^=1;
#endif
		}
	
		if(event.button.button == SDL_BUTTON_RIGHT) {
			mouse->right = 0;
#ifdef DEBUG
			mouse_b ^=2;
#endif
		}
	
		if(event.button.button == SDL_BUTTON_MIDDLE) {
			mouse->middle = 0;
#ifdef DEBUG
			mouse_b ^=4;
#endif
		}	
	}

	if ((shift_status&8) && scan_code==_x) 
		alt_x=1; 

	} // end while


#ifdef DOS
  union REGS r;
  struct SREGS s;

  tecla_bios();

  if (ibuf!=fbuf) {
    ascii=buf[ibuf]; scan_code=buf[ibuf+1]; shift_status=buf[ibuf+2];
    if ((shift_status&8) && scan_code==_x) alt_x=1; else alt_x=0;
    if ((ibuf+=3)==buf_max*3) ibuf=0;
  } else {
    ascii=0; scan_code=0;
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=2; int386x(0x16,&r,&r,&s); shift_status=r.h.al;
  }
#endif

}

//-----------------------------------------------------------------------------
//      Vacia el buffer de teclado (real e interno)
//-----------------------------------------------------------------------------

void vacia_buffer(void) {
#ifdef DOS
  union REGS r;
  struct SREGS s;

  s.ds=s.es=s.fs=s.gs=FP_SEG(&s);

  while(_bios_keybrd(_KEYBRD_READY)) { r.h.ah=0; int386x(0x16,&r,&r,&s); }
  ascii=0; scan_code=0;
  r.h.ah=2; int386x(0x16,&r,&r,&s); shift_status=r.h.al;
#endif
  ibuf=fbuf=0; // Vacia el buffer interno
}
