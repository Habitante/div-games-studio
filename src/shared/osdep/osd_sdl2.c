// OSDEP — SDL2 platform abstraction layer

#include "osd_sdl2.h"

// key buffer
uint8_t OSDEP_key[2048];
SDL_Renderer* OSDEP_renderer=NULL;
SDL_Window* OSDEP_window=NULL;
SDL_Surface* OSDEP_buffer8=NULL;
SDL_Surface* OSDEP_buffer32=NULL;
SDL_Texture* OSDEP_texture=NULL;
char windowtitle[1024];

extern int vwidth;
extern int vheight;

void OSDEP_Init(void) {

	SDL_Init( SDL_INIT_EVERYTHING);
// on windows, get printf working.
#ifdef __WIN32
	freopen( "CON", "w", stdout );
	freopen( "CON", "w", stderr );
#endif

}

void OSDEP_Quit(void) {
	SDL_Quit();
}

// Timer
uint32_t OSDEP_GetTicks(void) {
	return SDL_GetTicks();
}

// Display
void OSDEP_SetCaption(char *title, char *icon) {
	strcpy(windowtitle,title);
	if (OSDEP_window)
		SDL_SetWindowTitle(OSDEP_window, title);
}

OSDEP_VMode ** OSDEP_ListModes(void) {
	return NULL; // Mode enumeration handled by detect_vesa() fallback
}

void OSDEP_WarpMouse(int x, int y) {
	SDL_WarpMouseInWindow(OSDEP_window, x, y);
}

int OSDEP_IsFullScreen(void) {
    if (OSDEP_window == NULL) return 0;
    Uint32 flags = SDL_GetWindowFlags(OSDEP_window);
    return (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}

void OSDEP_SetWindowSize(int w, int h) {
	SDL_SetWindowSize(OSDEP_window, w, h);
	SDL_RenderPresent(OSDEP_renderer);

	SDL_GetRendererOutputSize(OSDEP_renderer,
                       &vwidth, &vheight);

SDL_Log("VW: %d VH: %d W: %d H: %d\n",vwidth, vheight, w,h);

}

/* Create or reconfigure the SDL2 window, renderer, surfaces, and texture.
 * On first call creates the window; on subsequent calls resizes or toggles
 * fullscreen. Always (re)creates the 8-bit and 32-bit surfaces plus the
 * streaming texture at the requested logical resolution.
 */
OSDEP_Surface * OSDEP_SetVideoMode(int width, int height, int bpp, char fs) {

	SDL_Log("Setting Videomode to %d x %d (fs=%d)\n", width, height, fs);

    if(OSDEP_window != NULL) {
    	// Window exists — update fullscreen state and size
    	if (fs) {
    		SDL_SetWindowFullscreen(OSDEP_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    	} else {
    		SDL_SetWindowFullscreen(OSDEP_window, 0);
    		SDL_SetWindowSize(OSDEP_window, width, height);
    	}
    } else {
    	// Create new window
    	Uint32 flags = SDL_WINDOW_RESIZABLE;
    	if (fs) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    	SDL_CreateWindowAndRenderer(width, height, flags, &OSDEP_window, &OSDEP_renderer);
    	SDL_ShowCursor(SDL_DISABLE);

    	if (windowtitle[0])
    		SDL_SetWindowTitle(OSDEP_window, windowtitle);
	}

	// Set logical size so SDL handles scaling and mouse coordinate mapping.
	// Content is centered with letterboxing to maintain aspect ratio.
	SDL_RenderSetLogicalSize(OSDEP_renderer, width, height);

	// Recreate surfaces and texture at the new logical size
    if(OSDEP_texture !=NULL) {
	    SDL_DestroyTexture(OSDEP_texture);
	    OSDEP_texture = NULL;
    }
    if(OSDEP_buffer32 !=NULL) {
    	SDL_FreeSurface(OSDEP_buffer32);
    	OSDEP_buffer32 = NULL;
    }
    if(OSDEP_buffer8 !=NULL) {
    	SDL_FreeSurface(OSDEP_buffer8);
    	OSDEP_buffer8 = NULL;
    }

	OSDEP_buffer8 = SDL_CreateRGBSurface(0, width, height, 8,
		0,0,0,0);

	OSDEP_buffer32 = SDL_CreateRGBSurface(0, width, height, 32,
		0,0,0,0);

	OSDEP_texture = SDL_CreateTexture(OSDEP_renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		width, height);

	return OSDEP_buffer8;
}


/* Present the current frame: blits the 8-bit paletted surface (buffer8) to
 * a 32-bit surface (palette conversion), uploads to the GPU texture, and
 * calls SDL_RenderPresent(). This is the final step of every frame.
 */
void OSDEP_Flip(OSDEP_Surface *s) {
	void *pixels;
  int pitch;

  /* blit the 320x200x8 Surface to a 32bit helper                */
  /* Surface for pixel format conversion (8bit palettized to     */
  /* 32bit). SDL2 Textures don't work with 8bit pixels formats.  */

 /* move it to the texture */
  SDL_BlitSurface(OSDEP_buffer8, NULL, OSDEP_buffer32, NULL);

    /* Modify the texture's pixels */
	SDL_UpdateTexture(OSDEP_texture, NULL, OSDEP_buffer32->pixels, OSDEP_buffer32->pitch);

    /* Make the modified texture visible by rendering it */
    SDL_RenderCopy(OSDEP_renderer, OSDEP_texture, NULL, NULL);

	SDL_RenderPresent(OSDEP_renderer);
}

void OSDEP_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h) {
	OSDEP_Flip(screen);
}

/* Update the 256-color palette on the given 8-bit surface. Called whenever
 * the game palette changes (fade effects, load_pal, etc.). The colors array
 * must contain at least firstcolor+ncolors entries.
 */
int OSDEP_SetPalette(OSDEP_Surface *surface, OSDEP_Color *colors, int firstcolor, int ncolors) {
	SDL_SetPaletteColors(surface->format->palette, colors + firstcolor, firstcolor, ncolors);
	return 1;
}

// Joysticks
int32_t OSDEP_NumJoysticks(void) {
	fprintf(stdout, "%s\n", __FUNCTION__);
	return SDL_NumJoysticks();
}

int OSDEP_JoystickNumButtons(int n) {
	OSDEP_Joystick *joy = SDL_JoystickOpen(n);
	int count = 0;
	if (joy) {
		count = SDL_JoystickNumButtons(joy);
		SDL_JoystickClose(joy);
	}
	return count;
}

int OSDEP_JoystickNumHats(int n) {
	OSDEP_Joystick *joy = SDL_JoystickOpen(n);
	int count = 0;
	if (joy) {
		count = SDL_JoystickNumHats(joy);
		SDL_JoystickClose(joy);
	}
	return count;
}

int OSDEP_JoystickNumAxes(int n) {
	OSDEP_Joystick *joy = SDL_JoystickOpen(n);
	int count = 0;
	if (joy) {
		count = SDL_JoystickNumAxes(joy);
		SDL_JoystickClose(joy);
	}
	return count;
}

uint8_t OSDEP_JoystickGetButton(OSDEP_Joystick *joystick, int button) {
	return SDL_JoystickGetButton(joystick, button);
}

int16_t OSDEP_JoystickGetAxis(OSDEP_Joystick *joystick, int axis) {
	return SDL_JoystickGetAxis(joystick, axis);

}
OSDEP_Joystick * OSDEP_JoystickOpen(int n) {
	return SDL_JoystickOpen(n);
}

void OSDEP_JoystickClose(OSDEP_Joystick *joy) {
	SDL_JoystickClose(joy);
	return;
}

char * OSDEP_JoystickName(int n) {
	return (char *)SDL_JoystickNameForIndex(n);
}

void OSDEP_keyInit(void) {
	
	OSDEP_key[SDLK_ESCAPE]=1;
	OSDEP_key[SDLK_F1-0x3FFFFD1A]=59;
	OSDEP_key[SDLK_F2-0x3FFFFD1A]=60;
	OSDEP_key[SDLK_F3-0x3FFFFD1A]=61;
	OSDEP_key[SDLK_F4-0x3FFFFD1A]=62;
	OSDEP_key[SDLK_F5-0x3FFFFD1A]=63;
	OSDEP_key[SDLK_F6-0x3FFFFD1A]=64;
	OSDEP_key[SDLK_F7-0x3FFFFD1A]=65;
	OSDEP_key[SDLK_F8-0x3FFFFD1A]=66;
	OSDEP_key[SDLK_F9-0x3FFFFD1A]=67;
	OSDEP_key[SDLK_F10-0x3FFFFD1A]=68;
	OSDEP_key[SDLK_F11-0x3FFFFD1A]=87;
	OSDEP_key[SDLK_F12-0x3FFFFD1A]=88;
	OSDEP_key[SDLK_PRINTSCREEN-0x3FFFFD1A]=55;
	OSDEP_key[SDLK_SCROLLLOCK-0x3FFFFD1A]=70;

	OSDEP_key[SDLK_BACKQUOTE]=41;
	OSDEP_key[SDLK_UP-0x3FFFFD1A]=72;
	OSDEP_key[SDLK_DOWN-0x3FFFFD1A]=80;
	OSDEP_key[SDLK_LEFT-0x3FFFFD1A]=75;
	OSDEP_key[SDLK_RIGHT-0x3FFFFD1A]=77;

	OSDEP_key[SDLK_1]=2;
	OSDEP_key[SDLK_2]=3;
	OSDEP_key[SDLK_3]=4;
	OSDEP_key[SDLK_4]=5;
	OSDEP_key[SDLK_5]=6;
	OSDEP_key[SDLK_6]=7;
	OSDEP_key[SDLK_7]=8;
	OSDEP_key[SDLK_8]=9;
	OSDEP_key[SDLK_9]=10;
	OSDEP_key[SDLK_0]=11;
	OSDEP_key[SDLK_MINUS]=12;
	OSDEP_key[SDLK_PLUS]=13;
	OSDEP_key[SDLK_BACKSPACE]=14;


	OSDEP_key[SDLK_TAB]=15;
	OSDEP_key[SDLK_q]=16;
	OSDEP_key[SDLK_w]=17;
	OSDEP_key[SDLK_e]=18;
	OSDEP_key[SDLK_r]=19;
	OSDEP_key[SDLK_t]=20;
	OSDEP_key[SDLK_y]=21;
	OSDEP_key[SDLK_u]=22;
	OSDEP_key[SDLK_i]=23;
	OSDEP_key[SDLK_o]=24;
	OSDEP_key[SDLK_p]=25;
	OSDEP_key[SDLK_LEFTBRACKET]=26;
	OSDEP_key[SDLK_RIGHTBRACKET]=27;
	OSDEP_key[SDLK_RETURN]=28;

	OSDEP_key[SDLK_CAPSLOCK-0x3FFFFD1A]=58;
	OSDEP_key[SDLK_a]=30;
	OSDEP_key[SDLK_s]=31;
	OSDEP_key[SDLK_d]=32;
	OSDEP_key[SDLK_f]=33;
	OSDEP_key[SDLK_g]=34;
	OSDEP_key[SDLK_h]=35;
	OSDEP_key[SDLK_j]=36;
	OSDEP_key[SDLK_k]=37;
	OSDEP_key[SDLK_l]=38;
	OSDEP_key[SDLK_SEMICOLON]=39;
	OSDEP_key[SDLK_QUOTE]=40;
	OSDEP_key[SDLK_BACKSLASH]=43;

	OSDEP_key[SDLK_LSHIFT-0x3FFFFD1A]=42;
	OSDEP_key[SDLK_z]=44;
	OSDEP_key[SDLK_x]=45;
	OSDEP_key[SDLK_c]=46;
	OSDEP_key[SDLK_v]=47;
	OSDEP_key[SDLK_b]=48;
	OSDEP_key[SDLK_n]=49;
	OSDEP_key[SDLK_m]=50;
	OSDEP_key[SDLK_COMMA]=51;
	OSDEP_key[SDLK_PERIOD]=51;
	OSDEP_key[SDLK_SLASH]=51;
	OSDEP_key[SDLK_RSHIFT-0x3FFFFD1A]=54;

	OSDEP_key[SDLK_LCTRL-0x3FFFFD1A]=29;
	OSDEP_key[SDLK_RCTRL-0x3FFFFD1A]=29;
	OSDEP_key[SDLK_LALT-0x3FFFFD1A]=56;
	OSDEP_key[SDLK_RALT-0x3FFFFD1A]=56;
	OSDEP_key[SDLK_SPACE]=57;
	OSDEP_key[SDLK_INSERT-0x3FFFFD1A]=82;
	OSDEP_key[SDLK_HOME-0x3FFFFD1A]=71;
	OSDEP_key[SDLK_PAGEUP-0x3FFFFD1A]=73;
	OSDEP_key[SDLK_DELETE]=83;
	OSDEP_key[SDLK_END-0x3FFFFD1A]=79;
	OSDEP_key[SDLK_PAGEDOWN-0x3FFFFD1A]=81;

	OSDEP_key[SDLK_NUMLOCKCLEAR-0x3FFFFD1A]=69;

	OSDEP_key[SDLK_KP_DIVIDE-0x3FFFFD1A]=53;
	OSDEP_key[SDLK_KP_MULTIPLY-0x3FFFFD1A]=53;
	OSDEP_key[SDLK_KP_MINUS-0x3FFFFD1A]=74;

	OSDEP_key[SDLK_BACKSLASH]=53;
	OSDEP_key[SDLK_KP_ENTER-0x3FFFFD1A]=28;
	OSDEP_key[SDLK_KP_MULTIPLY-0x3FFFFD1A]=55;
	OSDEP_key[SDLK_KP_MINUS-0x3FFFFD1A]=74;
	OSDEP_key[SDLK_KP_PLUS-0x3FFFFD1A]=78;
	OSDEP_key[SDLK_KP_ENTER-0x3FFFFD1A]=28;


//OSDEP_key[SDLK_LSHIFT-0x3FFFFD1A]=43;

#ifdef NOTYET
//const _wave=41
	const _c_center=76
#endif


}
