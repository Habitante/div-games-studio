#include "global.h"
#include "div_string.h"

//-----------------------------------------------------------------------------
//  Detection of available video modes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int compare_mode(void const *aa, void const *bb) {
  short *a, *b;
  a = (short *)aa;
  b = (short *)bb;
  return ((int)(*a) * 10000 + *(a + 1) > (int)(*b) * 10000 + *(b + 1));
}

void detect_vesa(void) { // Detects available video modes
  short *modelist;
  int n;

  OSDEP_VMode **modes;
  int i;

  num_modes = 0;


  modes = OSDEP_ListModes();

  /* Check is there are any modes available */
  if (modes == 0 || modes == (OSDEP_VMode **)-1) {
    // No mode enumeration — list standard resolutions for windowed mode
    modos[0].width = 320;
    modos[0].height = 240;
    modos[0].mode = 1;
    modos[1].width = 640;
    modos[1].height = 480;
    modos[1].mode = 1;
    modos[2].width = 800;
    modos[2].height = 600;
    modos[2].mode = 1;
    modos[3].width = 1024;
    modos[3].height = 768;
    modos[3].mode = 1;
    modos[4].width = 1280;
    modos[4].height = 1024;
    modos[4].mode = 1;
    modos[5].width = 1920;
    modos[5].height = 1080;
    modos[5].mode = 1;
    modos[6].width = 1280;
    modos[6].height = 720;
    modos[6].mode = 1;
    modos[7].width = 376;
    modos[7].height = 282;
    modos[7].mode = 1;
    num_modes = 8;
  } else {
    for (i = 0; modes[i]; ++i) {
      modos[i].width = modes[i]->w;
      modos[i].height = modes[i]->h;
      modos[i].mode = 1;
    }
    num_modes = i;
  }

  div_snprintf(vga_marker, sizeof(vga_marker), "SDL Video Driver");

  qsort((void *)&(modos[0].width), num_modes, sizeof(struct _modos), compare_mode);
}
