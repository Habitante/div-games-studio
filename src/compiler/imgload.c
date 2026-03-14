#include "imgload.h"


SDL_Surface *div_img_load(char *file) {
  // try to load using IMG_load

#ifdef IMAGE
  SDL_Surface *ret = IMG_Load(file);

  if (ret != NULL)
    return ret;
#endif


  // try to load MAP
  // load_map(file);

  return NULL;
}

SDL_Surface *div_img_load_mem(void *ptr, byte len) {
  return NULL;

#ifdef IMAGE


#endif


  return NULL;
}
