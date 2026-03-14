
//-----------------------------------------------------------------------------
//      Mouse access module
//-----------------------------------------------------------------------------

#include "global.h"

extern int help_paint_active;
extern SDL_Window *OSDEP_window;
byte m_b;

float m_x = 0.0, m_y = 0.0;
int mouse_in_window = 1;
int joymx = 0, joymy = 0;
void read_mouse2(void);
void release_drag(void);
int collides_with(int a, int x, int y, int w, int h);
void sound_init(void);
void sound_end(void);
int div_started = 0;


//-----------------------------------------------------------------------------
//      Check if the mouse is inside a box
//-----------------------------------------------------------------------------

int mouse_in(int x, int y, int x2, int y2) {
  return (mouse_x >= x && mouse_x <= x2 && mouse_y >= y && mouse_y <= y2);
}

int wmouse_in(int x, int y, int w, int h) {
  return (wmouse_x >= x && wmouse_x <= x + w - 1 && wmouse_y >= y && wmouse_y <= y + h - 1);
}

//-----------------------------------------------------------------------------
//      Place your mouse in the indicated position
//-----------------------------------------------------------------------------

void set_mouse(word x, word y) {
  m_x = (float)x;
  m_y = (float)y;
}

//-----------------------------------------------------------------------------
//      Read mouse button and position
//-----------------------------------------------------------------------------

void read_mouse(void) {
  int n = 0;
  int s, shift = 0;
  static int mouse_captured = 0;
  // Track the m_x/m_y values we last programmed via set_mouse().
  // Used instead of mouse_x/mouse_y for detecting real mouse movement,
  // because test_mouse()/select_zoom() can modify mouse_x/mouse_y
  // between frames (big-mode toolbar coordinate adjustment), which
  // would otherwise trick read_mouse() into thinking the real mouse
  // moved and permanently lock out keyboard navigation.
  static int last_set_mx = 0, last_set_my = 0;

  prev_mouse_buttons = mouse_b;
  if (vwidth == 0 && vheight == 0) {
    vwidth = vga_width;
    vheight = vga_height;
  }
  read_mouse2();

  if (draw_mode < 100 && hotkey && !help_paint_active)
    poll_keyboard();

  // In paint mode, capture the mouse so SDL keeps sending motion events
  // even when the cursor leaves the window (windowed mode scroll).
  // In fullscreen, grab the window so the mouse can't escape to other
  // monitors. Both are released when leaving paint mode.
  {
    int want_capture = (draw_mode < 100 && hotkey && !help_paint_active);
    if (want_capture != mouse_captured) {
      SDL_CaptureMouse(want_capture ? SDL_TRUE : SDL_FALSE);
      if (OSDEP_IsFullScreen())
        SDL_SetWindowGrab(OSDEP_window, want_capture ? SDL_TRUE : SDL_FALSE);
      mouse_captured = want_capture;
    }
  }

  // When mouse is outside the window, force out-of-bounds so no UI
  // element thinks it's being hovered (en_caja checks will all fail).
  if (!mouse_in_window) {
    if (draw_mode < 100 && hotkey && !help_paint_active) {
      // Paint mode with capture active: let out-of-bounds coordinates
      // flow through so move_zoom() can use them for canvas scrolling.
      // real_mouse_x/y get the raw (possibly negative / over-bounds) values;
      // mouse_x/y are clamped to screen bounds by the clamp_mouse code below.
      real_mouse_x = (int)m_x;
      real_mouse_y = (int)m_y;
      mouse_x = (int)m_x;
      mouse_y = (int)m_y;
      mouse_b = m_b;
      shift = 1;
      goto clamp_mouse;
    }
    mouse_x = -1;
    mouse_y = -1;
    real_mouse_x = -1;
    real_mouse_y = -1;
    mouse_shift_x = -1;
    mouse_shift_y = -1;
    coord_x = -1;
    coord_y = -1;
    last_set_mx = (int)m_x;
    last_set_my = (int)m_y;
    return;
  }

  real_mouse_x = (int)m_x;
  real_mouse_y = (int)m_y;

  // Fullscreen edge-scroll: the original DOS code used raw mouse deltas
  // (INT 33h function 0Bh) accumulated into m_x/m_y in software, so the
  // position could drift past screen bounds freely. In SDL2 fullscreen,
  // the OS clamps the cursor at the screen edge and stops generating
  // motion events. Instead, we detect the cursor sitting at the edge and
  // push real_mouse_x/y past bounds so move_zoom() scrolls.
  // Throttled to ~30 scrolls/sec (every 33ms) for comfortable speed.
  if (OSDEP_IsFullScreen() && draw_mode < 100 && hotkey && !help_paint_active) {
    static int last_edge_time = 0;
    int at_edge = 0;
    if (real_mouse_x <= 0 || real_mouse_x >= vga_width - 1)
      at_edge = 1;
    if (real_mouse_y <= 0 || real_mouse_y >= vga_height - 1)
      at_edge = 1;
    if (at_edge && mclock - last_edge_time >= 0) {
      int texel = 1 << zoom;
      last_edge_time = mclock;
      if (real_mouse_x <= 0)
        real_mouse_x = -texel;
      else if (real_mouse_x >= vga_width - 1)
        real_mouse_x = vga_width + texel;
      if (real_mouse_y <= 0)
        real_mouse_y = -texel;
      else if (real_mouse_y >= vga_height - 1)
        real_mouse_y = vga_height + texel;
    }
  }

  if ((int)m_x != last_set_mx || (int)m_y != last_set_my || mouse_b != m_b) {
    mouse_x = (int)m_x;
    mouse_y = (int)m_y;
    mouse_b = m_b;

    shift = 1;

    if (draw_mode < 100 && hotkey && !help_paint_active) {
      if (key(_SPC)) {
        if (mouse_b != MB_KEYBOARD_CLICK) {
          mouse_b = MB_KEYBOARD_CLICK;
        }
      } else if (mouse_b == MB_KEYBOARD_CLICK) {
        mouse_b = 0;
      }
    }
    if (vga_width != vwidth || vga_height != vheight) {
      mouse_x = (int)(m_x * (float)((float)vga_width / (float)vwidth));   // / (float)vga_width);
      mouse_y = (int)(m_y * (float)((float)vga_height / (float)vheight)); // / (float)vga_height);
    }


  } else if (draw_mode < 100 && hotkey && !help_paint_active) { // Keys are only active in edit mode

    if (!(shift_status & MOD_CTRL)) {
      mouse_x = mouse_shift_x;
      mouse_y = mouse_shift_y;

      if ((shift_status & MOD_SHIFT) || key(_L_SHIFT) || key(_R_SHIFT))
        s = 8;
      else
        s = 1;

      if (key(_C_RIGHT) || key(_RIGHT) || key(_P)) {
        kbdFLAGS[_C_RIGHT] = 0;
        kbdFLAGS[_RIGHT] = 0;
        kbdFLAGS[_P] = 0;
        mouse_x += (1 << zoom) * s;
        shift = 1;
      }

      if (key(_C_LEFT) || key(_LEFT) || key(_O)) {
        kbdFLAGS[_C_LEFT] = 0;
        kbdFLAGS[_LEFT] = 0;
        kbdFLAGS[_O] = 0;
        mouse_x -= (1 << zoom) * s;
        shift = 1;
      }

      if (key(_C_DOWN) || key(_DOWN) || key(_A)) {
        kbdFLAGS[_C_DOWN] = 0;
        kbdFLAGS[_DOWN] = 0;
        kbdFLAGS[_A] = 0;
        mouse_y += (1 << zoom) * s;
        shift = 1;
      }

      if (key(_C_UP) || key(_UP) || key(_Q)) {
        kbdFLAGS[_C_UP] = 0;
        kbdFLAGS[_UP] = 0;
        kbdFLAGS[_Q] = 0;
        mouse_y -= (1 << zoom) * s;
        shift = 1;
      }
      if (key(_SPC)) {
        if (mouse_b != MB_KEYBOARD_CLICK) {
          mouse_b = MB_KEYBOARD_CLICK;
        }
      } else if (mouse_b == MB_KEYBOARD_CLICK) {
        mouse_b = 0;
      }
      if (shift) {
        real_mouse_x = mouse_x;
        real_mouse_y = mouse_y;
        set_mouse(mouse_x, mouse_y);
      } else
        mouse_shift = 0;
    }
  }

clamp_mouse:
  if (shift) {
    if (mouse_x < 0) {
      mouse_x = 0;
      n++;
    } else if (mouse_x >= vga_width) {
      mouse_x = vga_width - 1;
      n++;
    }

    if (mouse_y < 0) {
      mouse_y = 0;
      n++;
    } else if (mouse_y >= vga_height) {
      mouse_y = vga_height - 1;
      n++;
    }

    if (n)
      set_mouse(mouse_x, mouse_y);
  }

  if (shift) {
    mouse_shift = 0;
    mouse_shift_x = mouse_x;
    mouse_shift_y = mouse_y;
  }

  coord_x = zoom_x + (mouse_shift_x - zoom_win_x) / (1 << zoom);
  coord_y = zoom_y + (mouse_shift_y - zoom_win_y) / (1 << zoom);

  if (free_drag) {
    switch (dragging) {
    case 0:
      if ((mouse_b & MB_LEFT) && !(prev_mouse_buttons & MB_LEFT)) {
        dragging = 1;
        drag_x = mouse_x;
        drag_y = mouse_y;
      }
      break;

    case 1:
      dragging = 0;
      break;

    case 2:
      if (mouse_b & MB_LEFT) {
        if (abs(mouse_x - drag_x) > 1 || abs(mouse_y - drag_y) > 1) {
          dragging = 3;
          wmouse_x = -1;
          wmouse_y = -1;
          mouse_b &= ~MB_LEFT;
          call((void_return_type_t)v.click_handler);
          drag_source = v.order;
          mouse_b |= MB_LEFT;
          mouse_graf = drag_graphic;
        }
      } else {
        dragging = 0;
      }
      break;

    case 3:
      if (!(mouse_b & MB_LEFT)) {
        dragging = 4;
      }
      break;

    case 4:
    case 5:
      release_drag();
      break;
    }
  }

  last_set_mx = (int)m_x;
  last_set_my = (int)m_y;
}

void release_drag(void) {
  int n;
  dragging = 0;
  for (n = 0; n < MAX_WINDOWS; n++) {
    if (window[n].type && window[n].order == drag_source)
      break;
  }

  if (n < MAX_WINDOWS && window[n].type == WIN_FPG && window[n].mapa != NULL) {
    free(window[n].mapa->map);
    free(window[n].mapa);
    window[n].mapa = NULL;

    if (n) {
      wup(n);
    }

    wmouse_x = -1;
    wmouse_y = -1;
    call((void_return_type_t)v.paint_handler);

    if (n) {
      wdown(n);
    }

    if (draw_mode >= 100) {
      flush_window(n);
    }
    window[n].redraw = 0;
  }
}

//-----------------------------------------------------------------------------
//  New mouse routines ( without interruption )
//-----------------------------------------------------------------------------

#define JOY_DEADZONE 500

void checkmod(OSDEPMod mod) {
  if (mod != KMOD_NONE) {
    if (mod & KMOD_LCTRL || mod & KMOD_CTRL || mod & KMOD_RCTRL)
      shift_status |= MOD_CTRL;

    if (mod & KMOD_RSHIFT)
      shift_status |= MOD_RSHIFT;

    if (mod & KMOD_LSHIFT)
      shift_status |= MOD_LSHIFT;

    if (mod & KMOD_LALT || mod & KMOD_ALT || mod & KMOD_RALT)
      shift_status |= MOD_ALT;

    if (mod & KMOD_CAPS)
      shift_status |= 64;

    if (mod & KMOD_NUM)
      shift_status |= 32;
  }

  //	fprintf(stdout, "%d\n", shift_status);
}

int soundstopped = 0;


#include <SDL2/SDL_events.h>

void print_event(const SDL_Event *event) {
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
      full_redraw = 1;
      break;
    case SDL_WINDOWEVENT_MOVED:
      SDL_Log("Window %d moved to %d,%d", event->window.windowID, event->window.data1,
              event->window.data2);
      break;
    case SDL_WINDOWEVENT_RESIZED:
      SDL_Log("Window %d resized to %dx%d (fs=%d)", event->window.windowID, event->window.data1,
              event->window.data2, OSDEP_IsFullScreen());
      // Ignore resize events in fullscreen — the logical resolution
      // stays the same, SDL_RenderSetLogicalSize handles scaling
      if (!OSDEP_IsFullScreen()) {
        vga_width = event->window.data1;
        vga_height = event->window.data2;
        vwidth = vga_width;
        vheight = vga_height;
        sound_end();
        soundstopped = 1;
      }
      full_redraw = 1;
      break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      SDL_Log("Window %d size changed to %dx%d", event->window.windowID, event->window.data1,
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
      full_redraw = 1;
      break;
    case SDL_WINDOWEVENT_ENTER:
      mouse_in_window = 1;
      break;
    case SDL_WINDOWEVENT_LEAVE:
      mouse_in_window = 0;
      break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
      SDL_Log("Window %d gained keyboard focus", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
      SDL_Log("Window %d lost keyboard focus", event->window.windowID);
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

/* Main event pump: polls all SDL events and updates input state.
 * Processes mouse motion/buttons, keyboard scancodes, joystick axes,
 * text input, mouse wheel, window events, and quit requests.
 */
void read_mouse2(void) {
  scan_code = 0;
  ascii = 0;
  m_b &= ~MB_SCROLL_DOWN; // clear wheel-down (one-shot per frame)
  m_b &= ~MB_SCROLL_UP; // clear wheel-up (one-shot per frame)

  SDL_Event event;
  int n = 0;

  while (SDL_PollEvent(&event)) {
    print_event(&event);

    // Text input — only set ascii, not scan_code.
    // scan_code is set by SDL_KEYDOWN via OSDEP_key[] (DOS scan codes).
    // Setting scan_code to ASCII here caused collisions: e.g. 'M'=77
    // matches Right Arrow, so Shift+M triggered Shift+Right in editor.
    if (event.type == SDL_TEXTINPUT) {
      ascii = event.text.text[0];
    }
    if (event.type == SDL_MOUSEWHEEL) {
      if (event.wheel.y > 0) {
        m_b |= MB_SCROLL_UP;
      } else {
        m_b |= MB_SCROLL_DOWN;
      }
    }

    if (event.type == SDL_JOYAXISMOTION) { // Analog joystick movement

      switch (event.jaxis.axis) {
      case 0: // axis 0 (left-right)
        joymx = 0;
        if (event.jaxis.value < -JOY_DEADZONE) {
          joymx = -2; //=event.jaxis.value;
                      // left movement
        } else if (event.jaxis.value > JOY_DEADZONE) {
          joymx = 2; //=event.jaxis.value;
                     // right movement
        }
        break;
      case 1: // axis 1 (up-down)
        joymy = 0;
        if (event.jaxis.value < -JOY_DEADZONE) {
          joymy = -2; //=event.jaxis.value;
                      // up movement
        } else if (event.jaxis.value > JOY_DEADZONE) {
          joymy = 2; //=event.jaxis.value;
                     // down movement
        }
        break;

      default:
        break;
      }
      /* If a quit event has been sent */
    }
    if (event.type == SDL_QUIT) {
      /* Quit the application */
      exit_requested = 1;
    }
    if (event.type == SDL_MOUSEMOTION) {
      // SDL_RenderSetLogicalSize maps physical coords to logical coords
      // in both windowed and fullscreen — always use absolute position.
      m_x = event.motion.x;
      m_y = event.motion.y;
    }
    /* If a button on the mouse is pressed. */
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        m_b |= MB_LEFT;
      }
      if (event.button.button == SDL_BUTTON_RIGHT) {
        m_b |= MB_RIGHT;
      }
      //	printf("click\n");
      //				m_b = 1;
    }
    if (event.type == SDL_KEYDOWN) {
      checkmod((OSDEPMod)event.key.keysym.mod);

      if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL)
        shift_status |= MOD_CTRL;
      else { // don't set scan code when lctrl pressed
        scan_code = OSDEP_key[event.key.keysym.sym < 2048 ? event.key.keysym.sym
                                                          : event.key.keysym.sym - 0x3FFFFD1A];
      }
      //		ascii = scan_code;

      // fix backspace in editor
      if (scan_code == 14)
        ascii = 8;
      // fix return in editor
      if (scan_code == '\x1c')
        ascii = 13;

      if (scan_code == 15)
        ascii = 9;
      //				fprintf(stdout, "ascii: %d scancode: %d 0x%x\n", ascii, scan_code,scan_code);
      key(scan_code) = 1;
    }
    if (event.type == SDL_KEYUP) {
      shift_status = 0;
      checkmod((OSDEPMod)event.key.keysym.mod);

      scan_code = OSDEP_key[event.key.keysym.sym < 2048 ? event.key.keysym.sym
                                                        : event.key.keysym.sym - 0x3FFFFD1A];

      //scan_code = event.key.keysym.scancode;
      key(scan_code) = 0;
      scan_code = 0;
    }

    if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == SDL_BUTTON_LEFT)
        m_b &= ~MB_LEFT;

      if (event.button.button == SDL_BUTTON_RIGHT)
        m_b &= ~MB_RIGHT;
    }
  }
  if (soundstopped == 1) {
    //SDL_SetVideoMode(event.resize.w, event.resize.h, 8,  SDL_HWSURFACE | SDL_RESIZABLE);
    //				bW = buffer->w; bH = buffer->h;

    if (vga_width < 640)
      vga_width = 640;


    if (vga_height < 480)
      vga_height = 480;


    if (vga_width & 1)
      vga_width++;

    if (vga_height & 1)
      vga_height++;

    VS_WIDTH = vga_width;
    VS_HEIGHT = vga_height;


    if (screen_buffer) {
      free(screen_buffer - 6);
      screen_buffer = NULL;
    }
    if (toolbar) {
      free(toolbar);
      toolbar = NULL;
    }

    toolbar = (byte *)malloc(vga_width * 19 * big2);
    if (!toolbar)
      return;
    toolbar_x = 8 * big2;
    toolbar_y = vga_height - 27 * big2;
    gradient = 0;
    current_mouse = 21;
    sel_status = 0;

    screen_buffer = (byte *)malloc(vga_width * vga_height + 6) + 6;
    setup_video_mode();
    prepare_wallpaper();

    if (strcmp((char *)v.title, (char *)texts[35])) {
      for (n = MAX_WINDOWS; n >= 0; n--) {
        if (window[n].type) {
          if (window[n].x + window[n].w > vga_width)
            window[n].x = vga_width - window[n].w;


          if (window[n].y + window[n].h > vga_height)
            window[n].y = vga_height - window[n].h;


          if (window[n].x <= 0)
            window[n].y = 0;

          if (window[n].y <= 0)
            window[n].y = 0;

          //	printf("n=%d\n",n);

          if (collides_with(n, window[n].x, window[n].y, window[n].w, window[n].h))
            place_window(1, &window[n].x, &window[n].y, window[n].w, window[n].h);
        }
      }
    }
    update_box(0, 0, vga_width, vga_height);
    //		full_redraw=1;
    blit_screen(screen_buffer);
    sound_init();
    soundstopped = 0;
  }
}
