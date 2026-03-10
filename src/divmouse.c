
//-----------------------------------------------------------------------------
//      Mouse access module
//-----------------------------------------------------------------------------

#include "global.h"

extern int help_paint_active;
byte m_b;

float m_x=0.0,m_y=0.0;
int mouse_in_window=1;
int joymx = 0, joymy=0;
void read_mouse2(void);
void libera_drag(void);
int collides_with(int a, int x, int y, int an, int al);
void InitSound(void);
void EndSound(void);
int div_started = 0;


//-----------------------------------------------------------------------------
//      Check if the mouse is inside a box
//-----------------------------------------------------------------------------

int mouse_in(int x, int y, int x2, int y2) {
	return(mouse_x>=x && mouse_x<=x2 && mouse_y>=y && mouse_y<=y2);
}

int wmouse_in(int x, int y, int an, int al) {
	return(wmouse_x>=x && wmouse_x<=x+an-1 &&
		wmouse_y>=y && wmouse_y<=y+al-1);
}

//-----------------------------------------------------------------------------
//      Place your mouse in the indicated position
//-----------------------------------------------------------------------------

void set_mouse(word x, word y) {
	m_x=(float)x;
	m_y=(float)y;
}

//-----------------------------------------------------------------------------
//      Read mouse button and position
//-----------------------------------------------------------------------------

void read_mouse(void) {
	int n=0;
	int s,shift=0;

	prev_mouse_buttons=mouse_b;
	if(vwidth == 0 && vheight == 0 ) {
		vwidth = vga_width;
		vheight = vga_height;
	}
	read_mouse2();

	if (draw_mode<100 && hotkey && !help_paint_active)
		poll_keyboard();

	// When mouse is outside the window, force out-of-bounds so no UI
	// element thinks it's being hovered (en_caja checks will all fail).
	// Also update mouse_shift and coord so the paint cursor goes offscreen.
	if (!mouse_in_window) {
		mouse_x=-1;
		mouse_y=-1;
		real_mouse_x=-1;
		real_mouse_y=-1;
		mouse_shift_x=-1;
		mouse_shift_y=-1;
		coord_x=-1;
		coord_y=-1;
		return;
	}

	real_mouse_x=(int)m_x;
	real_mouse_y=(int)m_y;
	

	if (mouse_x!=(int)m_x || mouse_y!=(int)m_y || mouse_b!=m_b) {

		mouse_x=(int)m_x;
		mouse_y=(int)m_y;
		mouse_b=m_b;

		shift=1;

		if (draw_mode<100 && hotkey && !help_paint_active) {
			if (key(_SPC)) {
				if (mouse_b!=0x8001) {
					mouse_b=0x8001;
				}
			} else if (mouse_b==0x8001) {
				mouse_b=0;
			}
		}
	if(vga_width != vwidth || vga_height != vheight) {
		mouse_x = (int)(m_x*(float)((float)vga_width / (float)vwidth));// / (float)vga_width);
		mouse_y = (int)(m_y*(float)((float)vga_height / (float)vheight));// / (float)vga_height);

	}


	} else if (draw_mode<100 && hotkey && !help_paint_active) { // Keys are only active in edit mode

		if (!(shift_status&4)) {

			mouse_x=mouse_shift_x;
			mouse_y=mouse_shift_y;

			if ((shift_status&3) || key(_L_SHIFT) || key(_R_SHIFT)) 
				s=8; 
			else 
				s=1;

			if (key(_C_RIGHT) || key(_RIGHT) || key(_P)) {
				kbdFLAGS[_C_RIGHT]=0; 
				kbdFLAGS[_RIGHT]=0; 
				kbdFLAGS[_P]=0;
				mouse_x+=(1<<zoom)*s; 
				shift=1;
			}

			if (key(_C_LEFT) || key(_LEFT) || key(_O)) {
				kbdFLAGS[_C_LEFT]=0;
				kbdFLAGS[_LEFT]=0;
				kbdFLAGS[_O]=0;
				mouse_x-=(1<<zoom)*s;
				shift=1;
			}

			if (key(_C_DOWN) || key(_DOWN) || key(_A)) {
				kbdFLAGS[_C_DOWN]=0;
				kbdFLAGS[_DOWN]=0; 
				kbdFLAGS[_A]=0;
				mouse_y+=(1<<zoom)*s; 
				shift=1;
			}

			if (key(_C_UP) || key(_UP) || key(_Q)) {
				kbdFLAGS[_C_UP]=0; 
				kbdFLAGS[_UP]=0; 
				kbdFLAGS[_Q]=0;
				mouse_y-=(1<<zoom)*s; 
				shift=1;
			}
			if (key(_SPC)) {
				if (mouse_b!=0x8001) {
					mouse_b=0x8001;
				}
			} else if (mouse_b==0x8001) {
				mouse_b=0;
			}
			if (shift) {
				real_mouse_x=mouse_x;
				real_mouse_y=mouse_y;
				set_mouse(mouse_x,mouse_y);
			} else 
				mouse_shift=0;
		}
	}

	if (shift) {

		if (mouse_x<0) { 
			mouse_x=0; 
			n++; 
		} else if (mouse_x>=vga_width) { 
			mouse_x=vga_width-1; 
			n++; 
		}
		
		if (mouse_y<0) {
			mouse_y=0;
			n++; 
		} else if (mouse_y>=vga_height) {
			mouse_y=vga_height-1; 
			n++; 
		}

		if (n) 
			set_mouse(mouse_x,mouse_y);
	}

	if (shift) {
		mouse_shift=0;
		mouse_shift_x=mouse_x;
		mouse_shift_y=mouse_y;
	}

	coord_x=zoom_x+(mouse_shift_x-zoom_win_x)/(1<<zoom);
	coord_y=zoom_y+(mouse_shift_y-zoom_win_y)/(1<<zoom);

	if(free_drag) {
		switch(dragging) {
			case 0:
			if ((mouse_b&1) && !(prev_mouse_buttons&1)) {
				dragging=1; drag_x=mouse_x; drag_y=mouse_y;
			} 
			break;
		
			case 1:
				dragging=0;
				break;
			
			case 2:
				if (mouse_b&1) {
					if (abs(mouse_x-drag_x)>1 || abs(mouse_y-drag_y)>1) {
						dragging=3;
						wmouse_x=-1;
						wmouse_y=-1;
						mouse_b &=~1;
						call((voidReturnType )v.click_handler);
						drag_source=v.order;
						mouse_b |=1;
						mouse_graf=drag_graphic;
					}
				} else {
					dragging=0;
				} 
			break;
			
			case 3:
				if (!(mouse_b&1)) {
				dragging=4;
				} 
			break;
			
			case 4:
			case 5:
				libera_drag();
			break;
		}
	}
}

void libera_drag(void) {
	int n;
	dragging=0;
	for (n=0;n<max_windows;n++) {
		if (ventana[n].type && ventana[n].order==drag_source) 
			break;
	}
	
	if (n<max_windows && ventana[n].type==101 && ventana[n].mapa!=NULL) {
		free(ventana[n].mapa->map);
		free(ventana[n].mapa);
		ventana[n].mapa=NULL;

		if (n) {
			wup(n);
		}

		wmouse_x=-1; wmouse_y=-1; call((voidReturnType )v.paint_handler);

		if (n) {
			wdown(n);
		}

		if(draw_mode>=100) { flush_window(n); } ventana[n].redraw=0;
	}
}

//-----------------------------------------------------------------------------
//  New mouse routines ( without interruption )
//-----------------------------------------------------------------------------

#define JOY_DEADZONE 500

void checkmod(OSDEPMod mod) {
	if( mod != KMOD_NONE ){
		if( mod & KMOD_LCTRL || mod & KMOD_CTRL  || mod & KMOD_RCTRL ) 
			shift_status |=4; 

		if( mod & KMOD_RSHIFT ) 
			shift_status |=1;

		if( mod & KMOD_LSHIFT ) 
			shift_status |=2;

		if( mod & KMOD_LALT ||  mod & KMOD_ALT  ||  mod & KMOD_RALT ) 
			shift_status |=8;

		if (mod & KMOD_CAPS) 
			shift_status |=64;

		if (mod & KMOD_NUM) 
			shift_status |=32;
	}

//	fprintf(stdout, "%d\n", shift_status);
}

int soundstopped=0;


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
            SDL_Log("Window %d resized to %dx%d (fs=%d)",
                    event->window.windowID, event->window.data1,
                    event->window.data2, OSDEP_IsFullScreen());
            // Ignore resize events in fullscreen — the logical resolution
            // stays the same, SDL_RenderSetLogicalSize handles scaling
            if (!OSDEP_IsFullScreen()) {
                vga_width = event->window.data1;
                vga_height = event->window.data2;
                vwidth = vga_width;
                vheight = vga_height;
                EndSound();
                soundstopped = 1;
            }
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            SDL_Log("Window %d minimized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            SDL_Log("Window %d restored", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            mouse_in_window=1;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            mouse_in_window=0;
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus",
                    event->window.windowID);
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

/* Main event pump: polls all SDL events and updates input state.
 * Processes mouse motion/buttons, keyboard scancodes, joystick axes,
 * text input, mouse wheel, window events, and quit requests.
 */
void read_mouse2(void) {

	scan_code  =0;
	ascii=0;
	m_b &=~4; // clear wheel-down (one-shot per frame)
	m_b &=~8; // clear wheel-up (one-shot per frame)

	SDL_Event event;
	int n=0;
	
while(SDL_PollEvent(&event) )
        {
	PrintEvent(&event);

            // Text input — only set ascii, not scan_code.
            // scan_code is set by SDL_KEYDOWN via OSDEP_key[] (DOS scan codes).
            // Setting scan_code to ASCII here caused collisions: e.g. 'M'=77
            // matches Right Arrow, so Shift+M triggered Shift+Right in editor.
            if(event.type == SDL_TEXTINPUT) {
            	ascii = event.text.text[0];
            }
            if(event.type == SDL_MOUSEWHEEL) {

            	if(event.wheel.y>0) {
					m_b |= 8;
	        	} else {
					m_b |= 4;
        		}
            }

			if(event.type == SDL_JOYAXISMOTION) {			// Analog joystick movement
				
			switch(event.jaxis.axis)
			{
				case 0:		// axis 0 (left-right)
					joymx=0;
					if(event.jaxis.value < -JOY_DEADZONE)
					{
						joymx=-2;//=event.jaxis.value;
						// left movement
					}
					else if(event.jaxis.value > JOY_DEADZONE)
					{
						joymx=2;//=event.jaxis.value;
						// right movement
					}
				break;
				case 1:		// axis 1 (up-down)
					joymy=0;
					if(event.jaxis.value < -JOY_DEADZONE)
					{
						joymy=-2;//=event.jaxis.value;
						// up movement
					}
					else if(event.jaxis.value > JOY_DEADZONE)
					{
						joymy=2;//=event.jaxis.value;
						// down movement
					}
				break;

				default:
				break;
			}
            /* If a quit event has been sent */
            }
            if (event.type == SDL_QUIT)
            {
                /* Quit the application */
                exit_requested=1;
            }
              if (event.type == SDL_MOUSEMOTION)
            {
				if(fsmode==1) {
					m_x += event.motion.xrel*(1+(Setupfile.mouse_ratio/2));
					m_y += event.motion.yrel*(1+(Setupfile.mouse_ratio/2));
				} else {
					m_x = event.motion.x;
					m_y = event.motion.y;
				}

			}
            /* If a button on the mouse is pressed. */
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
				
				if(event.button.button == SDL_BUTTON_LEFT)
				{
					m_b |= 1;

				}
				if(event.button.button == SDL_BUTTON_RIGHT)
				{
					m_b |= 2;
				}
			//	printf("click\n");
//				m_b = 1;
			}
			 if (event.type == SDL_KEYDOWN)
            {
				
			checkmod((OSDEPMod)event.key.keysym.mod);

			if( event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL ) 
				shift_status |=4; 
			else { // don't set scan code when lctrl pressed 
				scan_code = OSDEP_key[event.key.keysym.sym<2048?event.key.keysym.sym:event.key.keysym.sym-0x3FFFFD1A];
			}
		//		ascii = scan_code;

				// fix backspace in editor
				if(scan_code==14)
					ascii = 8;
				// fix return in editor
				if(scan_code=='\x1c')
					ascii=13;

				if(scan_code==15)
					ascii=9;
//				fprintf(stdout, "ascii: %d scancode: %d 0x%x\n", ascii, scan_code,scan_code);
				key(scan_code)=1;
			}
			if(event.type == SDL_KEYUP) 
			{
				shift_status =0;
				checkmod((OSDEPMod)event.key.keysym.mod);

				scan_code = OSDEP_key[event.key.keysym.sym<2048?event.key.keysym.sym:event.key.keysym.sym-0x3FFFFD1A];
		
				//scan_code = event.key.keysym.scancode;
				key(scan_code)=0;
				scan_code=0;
			}

			if (event.type == SDL_MOUSEBUTTONUP)
            {	
				
				if(event.button.button ==SDL_BUTTON_LEFT)
					m_b &= ~1;

				if(event.button.button ==SDL_BUTTON_RIGHT)
					m_b &= ~2;
			}
			
        }
	if(soundstopped==1) {

		//SDL_SetVideoMode(event.resize.w, event.resize.h, 8,  SDL_HWSURFACE | SDL_RESIZABLE);
		//				bW = buffer->w; bH = buffer->h;
		
		if(vga_width<640)
			vga_width=640;

		
		if(vga_height<480)
			vga_height=480;
		
		
		if(vga_width&1)
			vga_width++;

		if(vga_height&1)
			vga_height++;

		VS_WIDTH=vga_width;
		VS_HEIGHT=vga_height;


		if(copia) {
			free(copia-6);
			copia=NULL;
		}
		if(toolbar) {
			free(toolbar);
			toolbar=NULL;
		}
			
		toolbar=(byte*)malloc(vga_width*19*big2);
		if (!toolbar) return;
		toolbar_x=8*big2; toolbar_y=vga_height-27*big2; gradient=0; current_mouse=21; sel_status=0;

		copia=(byte*)malloc(vga_width*vga_height+6)+6;
		setup_video_mode();
		prepare_wallpaper();
	
		if(strcmp((char *)v.title, (char *)texto[35])) {

		for(n=max_windows;n>=0;n--) {
			if(ventana[n].type) {

				if (ventana[n].x+ventana[n].an>vga_width) 
					ventana[n].x=vga_width-ventana[n].an;
		
		
				if (ventana[n].y+ventana[n].al>vga_height) 
					ventana[n].y=vga_height-ventana[n].al;

	
				if(ventana[n].x<=0)
					ventana[n].y=0;
	
				if(ventana[n].y<=0)
					ventana[n].y=0;
	
		//	printf("n=%d\n",n);

			if (collides_with(n,ventana[n].x,ventana[n].y,ventana[n].an,ventana[n].al))	
				place_window(1,&ventana[n].x,&ventana[n].y,ventana[n].an,ventana[n].al);		

			}
		}	
			
		}
		update_box(0,0,vga_width,vga_height);
//		full_redraw=1;
		blit_screen(copia);
		InitSound();
		soundstopped=0;
	}
}
