// Debugger Code Window — source viewer, navigation, main debug dialog
#include "debugger_internal.h"

//----------------------------------------------------------------------------
//	Main debugging window
//----------------------------------------------------------------------------

void debug1(void) {
  char msg[256];
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2, n;

  wbox(ptr, w, h, c2, 2, 10, w - 4, h - 12);
  wbox(ptr, w, h, c12, 2, 10, w - 4, 144 - 16 - 32 - 10);

  _show_items2();
  itoa(process_count, msg, 10);
  div_strcat(msg, sizeof(msg), "/");
  itoa((imem_max - id_start) / iloc_len, msg + strlen(msg), 10);

  div_strcat(msg, sizeof(msg), (char *)text[62]);
  wwrite(ptr, w, h, 4, 11, 0, (byte *)msg, c1);
  wwrite(ptr, w, h, 3, 11, 0, (byte *)msg, c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128, 128 - 5 - 16 - 32); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122, 19, 9, 128 - 5 - 16 - 32);
  wrectangle(ptr, w, h, c0, 122, 27, 9, 112 - 5 - 16 - 32);

  wrectangle(ptr, w, h, c0, 133, 19, 120 + 304 - 256, 11); // Process name box

  wrectangle(ptr, w, h, c0, 133, 31, 48, 48); // Graphic box

  wput(ptr, w, h, 123, 20, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, 123, 139 - 5 - 16 - 32, -40);

  determine_ids();

  for (n = 0; n < iids; n++)
    if (ids[n] == ids_old)
      break;
  if (n < iids) { // If the previously selected process is still in the list ...
    ids_select = n;
    ids_ini = ids_select - ids_inc;
    if (ids_ini && ids_ini + 13 > iids)
      ids_ini = iids - 13;
    if (ids_ini < 0)
      ids_ini = 0;
  } else {
    if (ids_next + 12 <= iids) {
      if (ids_next)
        ids_ini = ids_next - 1;
      else
        ids_ini = 0;
    } else if ((ids_ini = iids - 13) < 0)
      ids_ini = 0;
    ids_select = ids_next;
  }

  draw_proc_list();
  determine_code();

  wbox(ptr, w, h, c0, 1, 144 - 16 - 32, w - 2, 1);
  wrectangle(ptr, w, h, c0, 47, 146 - 16 - 32, w - 50, 43 + 16 + 32);
}

void paint_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 129 - 16 - 32, n;
  float x;

  wbox(ptr, w, h, c2, 123, 28, 7, 105 - 16 - 32); // Clear the slider bar

  if (iids == 1)
    n = min;
  else {
    x = (float)ids_select / (float)(iids - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122, n, 9, 5);
  wput(ptr, w, h, 123, n + 1, -43);
}

byte *change_mode(void) {
  new_mode = 0;
  free(v.ptr);
  v.w = 304 * big2;
  v.h = (146 + 46) * big2;
  if ((v.ptr = (byte *)malloc(v.w * v.h)) == NULL)
    exer(1);
  if (v.x < 0) {
    v.x = 0;
  }
  if (v.y < 0) {
    v.y = 0;
  }
  if (v.x + v.w > vga_width)
    v.x = vga_width - v.w;
  if (v.y + v.h > vga_height)
    v.y = vga_height - v.h;
  repaint_window();
  call(v.paint_handler);
  v.redraw = 1;
  full_redraw = 1;
  return (v.ptr);
}

void debug2(void) {
  int n, m, iip; //,id;
  float x;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;

  _process_items();

  if (skip_flush)
    goto next_frame;

  if (!(shift_status & 15) && ascii == 0) {
    if (scan_code == 80) {
      f_down();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 72) {
      f_up();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 81) {
      for (n = 0; n < 11; n++)
        f_down();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 73) {
      for (n = 0; n < 11; n++)
        f_up();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 71) {
      f_home();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 77) {
      f_right();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    if (scan_code == 75) {
      f_left();
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }

  } else if (shift_status & 15) { // (Shift || Control || Alt)

    if (kbdFLAGS[80]) {
      kbdFLAGS[80] = 0;
      if (ids_select + 1 < iids) {
        if (ids_ini + 9 == ++ids_select)
          ids_ini++;
        draw_proc_list();
        v.redraw = 1;
      }
    }
    if (kbdFLAGS[72]) {
      kbdFLAGS[72] = 0;
      if (ids_select) {
        if (ids_ini == ids_select--)
          ids_ini--;
        draw_proc_list();
        v.redraw = 1;
      }
    }
    if (scan_code == 81 || scan_code == 118) {
      for (n = 0; n < 9; n++)
        if (ids_select + 1 < iids) {
          if (ids_ini + 9 == ++ids_select)
            ids_ini++;
        }
      draw_proc_list();
      v.redraw = 1;
    }
    if (scan_code == 73 || scan_code == 132) {
      for (n = 0; n < 9; n++)
        if (ids_select) {
          if (ids_ini == ids_select--)
            ids_ini--;
        }
      draw_proc_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(3, 21, 128 - 9, 120 - 16 - 32) && (mouse_b & 1)) {
    n = ids_ini + (wmouse_y - 21) / 8;
    if (n < iids) {
      ids_select = n;
      draw_proc_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (button == 0) {
        wput(ptr, w, h, 123, 20, -41);
        button = 1;
        if (ids_select) {
          if (ids_ini == ids_select--)
            ids_ini--;
          draw_proc_list();
          v.redraw = 1;
        }
      }
    } else if (button == 1) {
      wput(ptr, w, h, 123, 20, -39);
      button = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (button == 1) {
    wput(ptr, w, h, 123, 20, -39);
    button = 0;
    v.redraw = 1;
  }

  if (wmouse_in(123, 28, 7, 105 - 16 - 32)) {
    mouse_graf = 13;
    if (iids > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - 28) / (104.0 - 16.0 - 32.0);
      ids_select = x * (iids - 1);
      if (ids_select < ids_ini)
        ids_ini = ids_select;
      if (ids_select >= ids_ini + 9)
        ids_ini = ids_select - 8;
      draw_proc_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122, 133 - 16 - 32, 9, 9)) {
    if (mouse_b & 1) {
      if (button == 0) {
        wput(ptr, w, h, 123, 134 - 16 - 32, -42);
        button = 2;
        if (ids_select + 1 < iids) {
          if (ids_ini + 9 == ++ids_select)
            ids_ini++;
          draw_proc_list();
          v.redraw = 1;
        }
      }
    } else if (button == 2) {
      wput(ptr, w, h, 123, 134 - 16 - 32, -40);
      button = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (button == 2) {
    wput(ptr, w, h, 123, 134 - 16 - 32, -40);
    button = 0;
    v.redraw = 1;
  }

  if ((mouse_b & 1) && wmouse_in(48 + 5, 147 - 16 - 32, w - 52 - 5, 41 + 16 + 32)) {
    line_sel = line0 + (wmouse_y - (147 - 16 - 32)) / 8;
    if (line_sel == line0 + 11)
      line_sel = line0 + 10;
    paint_code();
    v.redraw = 1;
  }

  if (scan_code == _F4)
    goto go_here;
  if (scan_code == _F5)
    goto goto_proc;
  if (scan_code == _F6)
    goto exec_proc;
  if (scan_code == _F7)
    goto inspect_proc;
  if (scan_code == _F8)
    goto trace_proc;
  if (scan_code == _F9)
    goto set_break;
  if (scan_code == _F10)
    goto step_proc;
  if (scan_code == 32 || kbdFLAGS[_F11])
    goto profile_window;
  if (scan_code == 33 || kbdFLAGS[_F12])
    goto next_frame;

  switch (v.active_item) {
  case 0: // Next frame
next_frame:
    ids_old = ids[ids_select];
    ids_inc = ids_select - ids_ini;
    do {
      exec_process();
      if (new_mode)
        ptr = change_mode();
      if (call_to_debug) {
        call(v.paint_handler);
        v.redraw = 1;
        break;
      }
    } while (ide);
    if (call_to_debug) {
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      break;
    }
    if (process_count) {
      frame_clock = debug_clock;
      ticks = ticks_debug;
      set_mouse(smouse_x, smouse_y);
      blits_skipped = 0;
      skip_blit = 0;
      dacout_r = dr;
      dacout_g = dg;
      dacout_b = db;
      debugger_step = 1;
      frame_end();
      frame_start();
      debugger_step = 0;
      debug_clock = frame_clock;
      ticks_debug = ticks;
      smouse_x = mouse->x;
      smouse_y = mouse->y;
      set_mouse(mouse_x, mouse_y);
      memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
      call(v.paint_handler);
      for (n = 0; n < iids; n++)
        if (ids[n] == ids_old)
          break;
      if (n < iids) { // If the previously selected process is still in the list ...
        ids_select = n;
        ids_ini = ids_select - ids_inc;
        if (ids_ini && ids_ini + 13 > iids)
          ids_ini = iids - 13;
        if (ids_ini < 0)
          ids_ini = 0;
        draw_proc_list();
        determine_code();
      }
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      dread_mouse();
      _process_items();
      v.redraw = 1;
      full_redraw = 1;
      if (skip_flush)
        show_dialog(profile0);
    } else
      end_dialog = 1;
    break;
  case 1: // Goto
goto_proc:
    show_dialog(process_list0);
    //int line0;     // First line number in the debugger code window
    //byte * pline0; // Pointer to the first line in the debugger code window
    //int line_sel; // Selected line number
    if (v_accept) {
      x_start = 54;
      while (line0 > lp1[lp_select]) {
        line0--;
        pline0--;
        do {
          pline0--;
        } while (*pline0);
        pline0++;
      }
      while (line0 < lp1[lp_select]) {
        line0++;
        pline0 += strlen((char *)pline0) + 1;
      }
      line_sel = line0;
      paint_code();
      flush_buffer();
      v.redraw = 1;
    }
    break;
  case 2: // Breakpoint
set_break:
    for (n = 0; n < max_breakpoint; n++)
      if (breakpoint[n].line == line_sel)
        break;
    if (n < max_breakpoint) { // Deactivate a breakpoint
      breakpoint[n].line = -1;
      mem[abs(breakpoint[n].offset)] = breakpoint[n].code;
      paint_code();
      v.redraw = 1;
    } else {
      for (n = 0; n < max_breakpoint; n++)
        if (breakpoint[n].line == -1)
          break;
      if (n < max_breakpoint) {
        if ((m = get_ip(line_sel)) >= 0) {
          breakpoint[n].line = line_sel;
          breakpoint[n].offset = m;
          breakpoint[n].code = mem[m];
          mem[m] = ldbg;
          paint_code();
          v.redraw = 1;
        }
      } else {
        v_text = (char *)text[63];
        show_dialog(err0);
      }
    }
    break;
  case 3: // Go here!
go_here:
    for (n = 0; n < max_breakpoint; n++)
      if (breakpoint[n].line == -1)
        break;
    if (n < max_breakpoint) {
      if ((m = get_ip(line_sel)) >= 0) {
        breakpoint[n].code = line_sel;
        do {
          trace_process();
          if (new_mode)
            ptr = change_mode();
          if (call_to_debug) {
            call(v.paint_handler);
            v.redraw = 1;
            break;
          }
        } while (ide && ((ip >= mem1 && ip <= mem2) || mem[ip] == lasp || mem[ip] == lasiasp ||
                         mem[ip] == lcarasiasp || mem[ip] == lfunasp));
        if (call_to_debug) {
          full_redraw = 1;
          call_to_debug = 0;
          if (new_palette) {
            new_palette = 0;
            repaint_window();
          }
          break;
        }
        breakpoint[n].line = breakpoint[n].code;
        breakpoint[n].offset = -m;
        breakpoint[n].code = mem[m];
        mem[m] = ldbg;
        end_dialog = 1;
      }
    } else {
      v_text = (char *)text[63];
      show_dialog(err0);
    }
    break;
  case 4: // Trace
trace_proc:
    do {
      trace_process();
      if (new_mode)
        ptr = change_mode();
      if (call_to_debug) {
        call(v.paint_handler);
        v.redraw = 1;
        break;
      }
    } while (ide && ((ip >= mem1 && ip <= mem2) || mem[ip] == lasp || mem[ip] == lasiasp ||
                     mem[ip] == lcarasiasp || mem[ip] == lfunasp));
    if (call_to_debug) {
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      break;
    }
    if (process_count) {
      if (procesos_ejecutados()) {
        frame_clock = debug_clock;
        ticks = ticks_debug;
        set_mouse(smouse_x, smouse_y);
        blits_skipped = 0;
        skip_blit = 0;
        dacout_r = dr;
        dacout_g = dg;
        dacout_b = db;
        debugger_step = 1;
        frame_end();
        frame_start();
        debugger_step = 0;
        debug_clock = frame_clock;
        ticks_debug = ticks;
        smouse_x = mouse->x;
        smouse_y = mouse->y;
        set_mouse(mouse_x, mouse_y);
        memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
        full_redraw = 1;
      } else if (new_palette)
        set_dac();
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      ids_old = -1;
      call(v.paint_handler);
      v.redraw = 1;
    } else
      end_dialog = 1;
    break;
  case 5: // Step
step_proc:

    // If it calls a "function", do a Go Here! at the end

    for (iip = mem1; iip < mem2; iip++) {
      // WARNING: To detect if the next statement calls a function, we scan
      // for the 'lcal' opcode (value 26). This is imprecise because a
      // constant integer with the same value could cause a false match.

      if (mem[iip] == lcal)
        if (memo(mem[iip + 1]) == ltyp && memo(mem[iip + 1] + 2) == lnop) {
          for (n = 0; n < max_breakpoint; n++)
            if (breakpoint[n].line == -1)
              break;
          if (n < max_breakpoint) {
            breakpoint[n].offset = mem2 + 1;
            if (mem[breakpoint[n].offset] == lasp)
              breakpoint[n].offset++;
            breakpoint[n].line = 0;
            get_line(breakpoint[n].offset);
            breakpoint[n].code = mem[breakpoint[n].offset];
            mem[breakpoint[n].offset] = ldbg;
            end_dialog = 1;
            break;
          }
        }
    } // If no breakpoints left, it will do a Trace.

    process_level = 0;
    do {
      trace_process();
      if (new_mode)
        ptr = change_mode();
      if (call_to_debug) {
        call(v.paint_handler);
        v.redraw = 1;
        break;
      }
    } while (ide && ((ip >= mem1 && ip <= mem2) || mem[ip] == lasp || mem[ip] == lasiasp ||
                     mem[ip] == lcarasiasp || mem[ip] == lfunasp || process_level > 0));
    if (call_to_debug) {
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      break;
    }
    if (process_count) {
      if (procesos_ejecutados()) {
        frame_clock = debug_clock;
        ticks = ticks_debug;
        set_mouse(smouse_x, smouse_y);
        blits_skipped = 0;
        skip_blit = 0;
        dacout_r = dr;
        dacout_g = dg;
        dacout_b = db;
        debugger_step = 1;
        frame_end();
        frame_start();
        debugger_step = 0;
        debug_clock = frame_clock;
        ticks_debug = ticks;
        smouse_x = mouse->x;
        smouse_y = mouse->y;
        set_mouse(mouse_x, mouse_y);
        memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
        full_redraw = 1;
      } else if (new_palette)
        set_dac();
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      ids_old = -1;
      call(v.paint_handler);
      v.redraw = 1;
    } else
      end_dialog = 1;
    break;
  case 6: // Inspect
inspect_proc:
    current_scope = mem[ids[ids_select] + _Bloque];
    show_dialog(inspect0);
    draw_proc_list();
    v.redraw = 1;
    break;

  case 7: // Profile
profile_window:
    show_dialog(profile0);
    break;
  case 8: // Ex. Process
exec_proc:
    exec_process();
    if (new_mode)
      ptr = change_mode();
    if (call_to_debug) {
      call(v.paint_handler);
      v.redraw = 1;
      full_redraw = 1;
      call_to_debug = 0;
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
    }
    if (process_count) {
      if (procesos_ejecutados()) {
        frame_clock = debug_clock;
        ticks = ticks_debug;
        set_mouse(smouse_x, smouse_y);
        blits_skipped = 0;
        skip_blit = 0;
        dacout_r = dr;
        dacout_g = dg;
        dacout_b = db;
        debugger_step = 1;
        frame_end();
        frame_start();
        debugger_step = 0;
        debug_clock = frame_clock;
        ticks_debug = ticks;
        smouse_x = mouse->x;
        smouse_y = mouse->y;
        set_mouse(mouse_x, mouse_y);
        memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
        full_redraw = 1;
      } else if (new_palette)
        set_dac();
      if (new_palette) {
        new_palette = 0;
        repaint_window();
      }
      ids_old = -1;
      call(v.paint_handler);
      v.redraw = 1;
    } else
      end_dialog = 1;
    break;
  }
}

void debug3(void) {
  ids_old = ids[ids_select];
  ids_inc = ids_select - ids_ini;
}

void debug0(void) {
  v.type = WIN_DIALOG;
  v.title = text[64];
  v.w = 304;
  v.h = 146 + 46; // TODO: Keep in sync with change_mode dialog dimensions
  v.paint_handler = debug1;
  v.click_handler = debug2;
  v.close_handler = debug3;

  _button(text[65], v.w - 8, 102 - 18, 2);
  _button(text[66], 7 + 17, v.h - 14 - 15 * 4, 35 * 256 + 1);
  _button(text[67], 7 + 17, v.h - 14 - 15 * 3, 35 * 256 + 1);
  _button(text[68], 7 + 17, v.h - 14 - 15 * 2, 35 * 256 + 1);
  _button(text[69], 7 + 17, v.h - 14 - 15 * 1, 35 * 256 + 1);
  _button(text[70], 7 + 17, v.h - 14 - 15 * 0, 35 * 256 + 1);
  _button(text[71], v.w - 40, 35, 1);
  _button(text[72], 136, 102 - 18, 0);
  _button(text[73],
          (v.w - 8 - text_len((byte *)"\15Sgte. Frame") + 136 + text_len((byte *)"\15Perfiles")) /
              2,
          102 - 18, 1);
}

//----------------------------------------------------------------------------
//  Functions that paint source code in the debugger window
//----------------------------------------------------------------------------

void get_line(int n) { // From an IP address, get the statement position
  int x = 0;
  if (dbg_lines == NULL)
    return;
  for (x = 0; x < num_statements; x++)
    if (n >= dbg_lines[x * 6] && n <= dbg_lines[x * 6 + 1])
      break;
  if (x < num_statements) {
    mem1 = dbg_lines[x * 6];
    mem2 = dbg_lines[x * 6 + 1];
    line1 = dbg_lines[x * 6 + 2] - 1;
    col1 = dbg_lines[x * 6 + 3];
    line2 = dbg_lines[x * 6 + 4] - 1;
    col2 = dbg_lines[x * 6 + 5];
  } else {
    mem1 = dbg_lines[0];
    mem2 = dbg_lines[1];
    line1 = dbg_lines[2] - 1;
    col1 = dbg_lines[3];
    line2 = dbg_lines[4] - 1;
    col2 = dbg_lines[5];
  }
}

int get_ip(int n) { // From a line number, get the IP address of the statement
  int x = 0;
  if (dbg_lines == NULL)
    return (0);
  for (x = 0; x < num_statements; x++)
    if (n == dbg_lines[x * 6 + 2] - 1)
      break;
  if (x < num_statements)
    return (dbg_lines[x * 6]);
  else
    return (-1);
}

void determine_code(void) { // Determine what to show for "ids_next"
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2, l;
  char msg[256];

  if (dbg_lines == NULL)
    return;

  wbox(ptr, w, h, c1, 4, 147 - 16 - 32, 41, 8); // Clear the space for writing the ID
  wrectangle(ptr, w, h, c0, 3, 146 - 16 - 32, 43, 10);
  itoa(ids[ids_next], msg, 10);
  wwrite_in_box(ptr, w, 46, h, 24, 148 - 16 - 32, 1, (byte *)msg, c0);
  wwrite_in_box(ptr, w, 46, h, 23, 148 - 16 - 32, 1, (byte *)msg, c34);

  if (process_stoped)
    get_line(ip);
  else
    get_line(mem[ids[ids_next] + _IP]); // Get line/column/mem 1/2

  l = line0 = line1 - 3;
  if (l < 0)
    l = line0 = 0;

  pline0 = source + 1;
  while (l--)
    pline0 += strlen((char *)pline0) + 1;

  line_sel = line1;

  paint_code();
}

void paint_code(void) { // Paint the source code
  byte *p = pline0, c;
  int n, x, l = line0;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;

  if (dbg_lines == NULL)
    return;

  wbox(ptr, w, h, c1, 48 + 5, 147 - 16 - 32, w - 52 - 5, 41 + 16 + 32); // Clear the window

  wbox(ptr, w, h, c12, 48, 147 - 16 - 32, 5, 41 + 16 + 32);

  wrectangle(ptr, w, h, c0, 47 + 5, 146 - 16 - 32, 1, 43 + 16 + 32);

  for (n = 0; n < 11; n++, l++) {
    if (p >= end_source)
      break;
    if (l == line_sel)
      wbox(ptr, w, h, c0, 48 + 5, 147 - 16 - 32 + (line_sel - line0) * 8, w - 52 - 5, 9);
    for (x = 0; x < max_breakpoint; x++)
      if (breakpoint[x].line == l)
        wbox(ptr, w, h, c_r_low, 48 + 5, 148 - 16 - 32 + (l - line0) * 8, w - 52 - 5, 7);
    if (l == line1) {
      wput(ptr, w, h, 48, 148 - 16 - 32 + n * 8, 36);
      c = *(p + col1);
      *(p + col1) = 0;
      x = x_start - 54;
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p, c3);
      if (text_len(p)) {
        x += text_len(p) + 1;
      }
      *(p + col1) = c;
      if (l == line2) {
        c = *(p + col2 + 1);
        *(p + col2 + 1) = 0;
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x + 1, 148 - 16 - 32 + n * 8, 0, p + col1, c0);
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + col1, c4);
        x += text_len(p + col1) + 1;
        *(p + col2 + 1) = c;
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + col2 + 1, c3);
      } else {
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x + 1, 148 - 16 - 32 + n * 8, 0, p + col1, c0);
        wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + col1, c4);
      }
    } else if (l == line2) {
      c = *(p + col2 + 1);
      *(p + col2 + 1) = 0;
      x = x_start - 54;
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x + 1, 148 - 16 - 32 + n * 8, 0, p, c0);
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p, c4);
      x += text_len(p) + 1;
      *(p + col2 + 1) = c;
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x, 148 - 16 - 32 + n * 8, 0, p + col2 + 1, c3);
    } else if (l > line1 && l < line2) {
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x_start - 54 + 1, 148 - 16 - 32 + n * 8, 0, p,
                    c0);
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x_start - 54, 148 - 16 - 32 + n * 8, 0, p, c4);
    } else {
      wwrite_in_box(ptr + 54 * big2, w, w - 59, h, x_start - 54, 148 - 16 - 32 + n * 8, 0, p, c3);
    }
    p += strlen((char *)p) + 1;
  }
}

void f_home(void) {
  x_start = 54;
}

void f_right(void) {
  x_start -= 6;
}

void f_left(void) {
  if (x_start < 54)
    x_start += 6;
}

void f_up(void) {
  if (line_sel) {
    line_sel--;
    if (line_sel < line0) {
      line0--;
      pline0--;
      do {
        pline0--;
      } while (*pline0);
      pline0++;
    }
  }
}

void f_down(void) {
  byte *p;
  int n;

  n = line_sel - line0 + 1;
  p = (byte *)pline0;
  while (n-- && p < end_source)
    p += strlen((char *)p) + 1;
  if (p < end_source) {
    line_sel++;
    if (line_sel == line0 + 11) {
      line0++;
      pline0 += strlen((char *)pline0) + 1;
    }
  }
}
