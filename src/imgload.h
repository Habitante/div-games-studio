#ifndef _NEWFUNCS_H
#define _NEWFUNCS_H

#include "global.h"
#include "osdep.h"

#ifdef IMAGE
#include "SDL_image.h"
#endif

SDL_Surface *div_img_load(char *file);
SDL_Surface *div_img_load_mem(void *ptr, byte len);


#endif
