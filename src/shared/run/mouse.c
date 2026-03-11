
#include "inter.h"

float m_x=0.0,m_y=0.0;
static float vx=0.0,vy=0.0;
extern int mouse_in_window;

//----------------------------------------------------------------------------
//      Set_mouse(x,y)
//----------------------------------------------------------------------------

void mouse_on (void) {
}

void mouse_off (void) {
}

//-----------------------------------------------------------------------------
//      Define the mouse movement region (full screen)
//-----------------------------------------------------------------------------

void mouse_window(void) {
}



void check_mouse(void) {
  mouse->cursor=0; // 0 = mouse present (always true on SDL)
}

void set_mouse(int x,int y) {
  m_x=(float)x;
  m_y=(float)y;
}

#ifdef DEBUG
extern int mouse_x,mouse_y;
#endif

void readmouse(void) {
  int n=0;

  poll_keyboard();
  // When mouse is outside the window, report out-of-bounds
  if (!mouse_in_window) {
    _mouse_x=-1;
    _mouse_y=-1;
    mouse->x=-1;
    mouse->y=-1;
  } else {
    _mouse_x=(int)mouse->x;
    _mouse_y=(int)mouse->y;

    if (_mouse_x<0) { _mouse_x=0; n++; }
    else if (_mouse_x>=vga_width) { _mouse_x=vga_width-1; n++; }
    if (_mouse_y<0) { _mouse_y=0; n++; }
    else if (_mouse_y>=vga_height) { _mouse_y=vga_height-1; n++; }

    if (n) set_mouse(_mouse_x,_mouse_y);

    mouse->x=_mouse_x;
    mouse->y=_mouse_y;
  }

  // Keyboard-driven mouse cursor (accessibility: arrow keys simulate mouse)
  if (mouse->cursor&1) {
    int keymouse=0;
    float vmax=1.0f-(float)mouse->speed/15.0f;
    if (vmax<0.1f) vmax=0.1f;

    if(key(_ENTER)) mouse->left=1;

    if (key(_LEFT)&&!key(_RIGHT))  { vx-=vmax; keymouse|=1; }
    if (key(_RIGHT)&&!key(_LEFT))  { vx+=vmax; keymouse|=1; }
    if (key(_UP)&&!key(_DOWN))     { vy-=vmax; keymouse|=2; }
    if (key(_DOWN)&&!key(_UP))     { vy+=vmax; keymouse|=2; }

    if (vx<-vmax*10.0f) vx=-vmax*10.0f;
    if (vx> vmax*10.0f) vx= vmax*10.0f;
    if (vy<-vmax*10.0f) vy=-vmax*10.0f;
    if (vy> vmax*10.0f) vy= vmax*10.0f;

    m_x+=vx; m_y+=vy;

    if (!(keymouse&1)) { if (vx>1.0f) vx-=2.0f; else if (vx<-1.0f) vx+=2.0f; else vx=0.0f; }
    if (!(keymouse&2)) { if (vy>1.0f) vy-=2.0f; else if (vy<-1.0f) vy+=2.0f; else vy=0.0f; }

    if (m_x<0) m_x=0;
    if (m_x>=vga_width) m_x=(float)(vga_width-1);
    if (m_y<0) m_y=0;
    if (m_y>=vga_height) m_y=(float)(vga_height-1);

    mouse->x=(int)m_x;
    mouse->y=(int)m_y;
    _mouse_x=(int)m_x;
    _mouse_y=(int)m_y;
  }

#ifdef DEBUG
			mouse_x = _mouse_x;
			mouse_y = _mouse_y;
#endif

}

