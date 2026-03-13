
//----------------------------------------------------------------------------
//      Debugger core — initialization, error dialogs, debug entry points,
//      process identification and visualization
//----------------------------------------------------------------------------

#include "debugger_internal.h"

//--------------------------------------------------------------------------
//      Global variable definitions (shared via debugger_internal.h)
//--------------------------------------------------------------------------

int debug_active = 1; // If set to 0, the debugger and its windows
                      // can never be invoked

char combo_error[128]; // buffer for composing compound error messages

extern float m_x, m_y;

int skip_flush = 0;
int profiler_x, profiler_y;

int show_items_called = 0;
int get_pos = 0, get_cursor = 0; // Clock and cursor position in get fields
int superget = 0;

//--------------------------------------------------------------------------

int line0;    // First line number in the debugger code window
byte *pline0; // Pointer to the first line in the debugger code window

int mem1, mem2;               // Bounds of the current statement in the mem[] array
int line1, col1, line2, col2; // Bounds of the current statement

int line_sel; // Selected line number

int x_start = 54; // Initial x offset in the source code window

//--------------------------------------------------------------------------

int smouse_x, smouse_y, mouse_x = 0, mouse_y = 0, mouse_b;

int debug_clock;
int ticks_debug;

//----------------------------------------------------------------------------

int button = 0;
int ids_old = 0, ids_inc;

byte *source = NULL, *end_source = NULL;

int *dbg_lines = NULL;
int num_statements;

//----------------------------------------------------------------------------
//      Debugger variables
//----------------------------------------------------------------------------

clock_t system_clock; // = clock();//(void*) 0x46c; // System clock

char get[256];

char *vnom = NULL; // Name vector (symbol name table)

struct object *o = NULL;

int num_objects = 0; // Number of objects in the object table

int num_predefined;

int *used;  // Marks which objects have been included in var[]
int *visor; // Display mode used for each object

struct variables *var = NULL;

int num_var = 0; // Number of variables included in var[]

int var_ini;    // First variable displayed in the window
int var_select; // Selected variable

int show_const = 0, show_global = 0, show_local = 1, show_private = 1;

int pre_defined = 0, user_defined = 1;

int current_scope; // Process being inspected

//----------------------------------------------------------------------------

int iids, *ids; // Process identifiers, in execution order
int ids_ini;    // First process displayed in the window
int ids_select; // Process highlighted for info display
int ids_next;   // Next process in execution order '>'

//----------------------------------------------------------------------------

int lp1[512];    // Line numbers where processes are defined
char *lp2[512];  // Pointers to the process definition lines
int lp_num;      // Number of processes in the list
int lp_ini;      // First entry displayed in the window
int lp_select;   // Selected entry
int lp_sort = 0; // Flag: sort the list alphabetically

//----------------------------------------------------------------------------
//      Profiler variables
//----------------------------------------------------------------------------

byte c_r_low0, c_g_low0, c_b_low0;

int obj_start; // Start of the first object (&obj[0])
int obj_size;  // Size of each object (struct object)

// The block of an ID is: (mem[ID+_Bloque]-obj_start)/obj_size;

unsigned f_time[256];     // Time consumed by the different functions
unsigned frame_time[256]; // Time consumed by the different functions

//---------------------------------------------------------------------------- //      Debug initialization //----------------------------------------------------------------------------

void init_debug(void) {
  FILE *f;
  int n;

  for (n = 0; n < MAX_WINDOWS; n++)
    window[n].type = 0;
  if ((mouse_background = (byte *)malloc(2048)) == NULL)
    exer(1);
  init_big();

  if ((f = fopen("system/exec.dbg", "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    n = ftell(f) - 4;
    if ((o = (struct object *)malloc(n)) != NULL) {
      fseek(f, 0, SEEK_SET);
      fread(&num_objects, 4, 1, f);
      fread(&num_predefined, 4, 1, f);
      fread(&obj_start, 4, 1, f);
      fread(&obj_size, 4, 1, f);
      fread(o, 1, n, f);
      fclose(f);
      vnom = (char *)o + num_objects * sizeof(struct object) + 4;
    } else {
      fclose(f);
      exer(1);
    }
  } else {
    for (n = 0; n < max_breakpoint; n++)
      breakpoint[n].line = -1;
    debug_active = 0;
    return;
  }

  if ((f = fopen("system/exec.lin", "rb")) != NULL) {
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    if ((dbg_lines = (int *)malloc(n)) != NULL) {
      fseek(f, 0, SEEK_SET);
      fread(dbg_lines, 1, n, f);
      fclose(f);
      num_statements = n / (6 * 4);
    } else {
      fclose(f);
      exer(1);
    }
  } else
    dbg_lines = NULL;

  if (dbg_lines != NULL) {
    if ((f = fopen("system/exec.pgm", "rb")) != NULL) {
      fseek(f, 0, SEEK_END);
      n = ftell(f);
      if ((source = (byte *)malloc(n)) != NULL) {
        fseek(f, 0, SEEK_SET);
        fread(source, 1, n, f);
        fclose(f);
        end_source = source + n;
      } else {
        fclose(f);
        exer(1);
      }
    } else {
      free(dbg_lines);
      dbg_lines = NULL;
    }
  }

  if ((var = (struct variables *)malloc(sizeof(struct variables) * num_objects)) == NULL)
    exer(1);
  if ((ids = (int *)malloc(sizeof(int) * max_procesos)) == NULL)
    exer(1);
  if ((used = (int *)malloc(sizeof(int) * num_objects)) == NULL)
    exer(1);
  if ((visor = (int *)malloc(sizeof(int) * num_objects)) == NULL)
    exer(1);

  // Set filters for objects

  memset(visor, 0, sizeof(int) * num_objects);
  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tcglo || o[n].type == tcloc)
      visor[n] = 2;
    if (o[n].type == tcons && o[n].v1 == 1)
      visor[n] = 2;
    if ((o[n].type == tvloc || o[n].type == tvglo || o[n].type == tcons) &&
        (vnom[o[n].name] == 'a' || vnom[o[n].name] == '\xa0') && vnom[o[n].name + 1] == 'n' &&
        vnom[o[n].name + 2] == 'g')
      visor[n] = 4;
    if (o[n].type == tcons && !strcmp(vnom + o[n].name, "pi"))
      visor[n] = 4;
    if ((o[n].type == tvloc || o[n].type == tvglo) &&
        (vnom[o[n].name + 2] == 'e' ||
         (vnom[o[n].name + 2] >= '0' && vnom[o[n].name + 2] <= '9')) &&
        vnom[o[n].name] == 'i' && vnom[o[n].name + 1] == 'd')
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "caller_id"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "father"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "bigbro"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "smallbro"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "son"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "process_id"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "id_scan"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "type_scan"))
      visor[n] = 3;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "is_executed"))
      visor[n] = 1;
    if (o[n].type == tvloc && !strcmp(vnom + o[n].name, "is_painted"))
      visor[n] = 1;
    if (o[n].type == tvglo && !strcmp(vnom + o[n].name, "fading"))
      visor[n] = 1;
    if (o[n].type == ttglo && !strcmp(vnom + o[n].name, "argv"))
      visor[n] = 2;

    if (o[n].type == ttglo && !strcmp(vnom + o[n].name, "name") && o[n - 2].type == tsglo &&
        !strcmp(vnom + o[n - 2].name, "dirinfo"))
      visor[n] = 2;
    if (o[n].type == tvglo && !strcmp(vnom + o[n].name, "attrib") && o[n - 13].type == tsglo &&
        !strcmp(vnom + o[n - 13].name, "fileinfo"))
      visor[n] = 6;
  }

  // Initialize process block timings

  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tproc) {
      o[n].v4 = 0; // Time_exec
      o[n].v5 = 0; // Time_paint
    }
  }
  memset(f_time, 0, 256 * 4);

  for (n = 0; n < max_breakpoint; n++)
    breakpoint[n].line = -1;
}

void end_debug(void) {
  free(mouse_background);
  free(o);
  free(var);
  free(used);
  free(ids);
  free(visor);
  free(source);
  free(dbg_lines);
}
//-----------------------------------------------------------------------------
//  Initialize the font
//-----------------------------------------------------------------------------

int old_big = -1;

void init_big(void) {
  FILE *f;
  int n;
  byte *ptr;

  if (vga_width >= 640) {
    big = 1;
    big2 = 2;
  } else {
    big = 0;
    big2 = 1;
  }

  if (old_big >= 0 && old_big != big) {
    free(text_font);
    free(graf_ptr);
  }
  old_big = big;

  if (big)
    f = fopen("system/grande.fon", "rb");
  else
    f = fopen("system/pequeno.fon", "rb");
  if (f == NULL)
    exer(5);
  fseek(f, 0, SEEK_END);
  n = ftell(f);
  if ((text_font = (byte *)malloc(n)) != NULL) {
    fseek(f, 0, SEEK_SET);
    fread(text_font, 1, n, f);
    fclose(f);
  } else {
    fclose(f);
    exer(1);
  }

  if (big)
    f = fopen("system/GRAF_G.DIV", "rb");
  else
    f = fopen("system/GRAF_P.DIV", "rb");
  if (f == NULL)
    exer(6);
  else {
    fseek(f, 0, SEEK_END);
    n = ftell(f) - 1352;
    if (n > 0 && (graf_ptr = (byte *)malloc(n)) != NULL) {
      memset(graf, 0, sizeof(graf));
      ptr = graf_ptr;
      fseek(f, 1352, SEEK_SET);
      fread(graf_ptr, 1, n, f);
      fclose(f);
      while (graf_ptr < ptr + n && *((int *)graf_ptr) < 256) {
        if (*(int *)(graf_ptr + 60)) {
          graf[*((int *)graf_ptr)] = graf_ptr + 60;
          *(word *)(graf_ptr + 60) = *(int *)(graf_ptr + 52);
          *(word *)(graf_ptr + 62) = *(int *)(graf_ptr + 56);
          graf_ptr += *(int *)(graf_ptr + 52) * *(int *)(graf_ptr + 56) + 68;
        } else {
          graf[*((int *)graf_ptr)] = graf_ptr + 56;
          *(word *)(graf_ptr + 56) = *(int *)(graf_ptr + 52);
          *(word *)(graf_ptr + 58) = *(int *)(graf_ptr + 56);
          *(int *)(graf_ptr + 60) = 0;
          graf_ptr += *(int *)(graf_ptr + 52) * *(int *)(graf_ptr + 56) + 64;
        }
      }
      graf_ptr = ptr;
    } else {
      fclose(f);
      exer(1);
    }
  }
}

//-----------------------------------------------------------------------------
//  Initialize colors (after palette change)
//-----------------------------------------------------------------------------

void init_colors(void) {
  find_color(0, 0, 0);
  c0 = find_col;
  find_color(63, 63, 63);
  c4 = find_col;
  c2 = average_color(c0, c4);
  c1 = average_color(c0, c2);
  c3 = average_color(c2, c4);
  c01 = average_color(c0, c1);
  c12 = average_color(c1, c2);
  c23 = average_color(c2, c3);
  c34 = average_color(c3, c4);
  find_color(63, 0, 0);
  c_r = find_col;
  find_color(0, 63, 0);
  c_g = find_col;
  find_color(0, 0, 63);
  c_b = find_col;
  find_color(32, 0, 0);
  c_r_low = find_col;
  find_color(0, 32, 0);
  c_g_low = find_col;
  find_color(0, 0, 32);
  c_b_low = find_col;
  find_color(16, 0, 0);
  c_r_low0 = find_col;
  find_color(0, 16, 0);
  c_g_low0 = find_col;
  find_color(0, 0, 16);
  c_b_low0 = find_col;
}

//-----------------------------------------------------------------------------
//      ERROR dialog box
//-----------------------------------------------------------------------------

void err1(void) {
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, (byte *)v_text, c3);
}

void err2(void) {
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
}

void err0(void) {
  v.type = 1;
  v.title = text[1];
  v.w = text_len((byte *)v_text) + 8;
  v.h = 38;
  v.paint_handler = err1;
  v.click_handler = err2;
  _button(text[7], v.w / 2, v.h - 14, 1);
}

//-----------------------------------------------------------------------------
//      ERROR dialog box inside a language function
//-----------------------------------------------------------------------------

char *te;

void _err1(void) {
  int w = v.w / big2, h = v.h / big2, n;
  char cwork[256];
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, text[8], c4);
  if (num_objects) {
    for (n = 0; n < num_objects; n++)
      if (o[n].type == tproc && o[n].v0 == mem[id + _Bloque])
        break;
    if (n < num_objects)
      wwrite(v.ptr, w, h, 46, 12, 0, (byte *)vnom + o[n].name, c3);
    else
      wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
  } else
    wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
  wwrite(v.ptr, w, h, 4, 12 + 8, 0, (byte *)text[10], c4);
  if (v_function >= 0)
    wwrite(v.ptr, w, h, 46, 12 + 8, 0, (byte *)fname[v_function], c3);
  else
    wwrite(v.ptr, w, h, 46, 12 + 8, 0, text[9], c3);

  wwrite(v.ptr, w, h, 4, 12 + 16, 0, (byte *)text[1], c4);
  itoa(num_error, cwork, 10);
  wwrite(v.ptr, w, h, 46, 12 + 16, 0, (byte *)cwork, c3);

  wwrite(v.ptr, w, h, 4, 12 + 24, 0, (byte *)v_text, c3);
}

void _err2(void) {
  int n;
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
  if (v.active_item == 1) {
    end_dialog = 1;
    n = 0;
    while (n < num_skipped) {
      if (skipped[n] == num_error)
        break;
      n++;
    }
    if (n >= num_skipped && num_skipped < 128) {
      skipped[num_skipped++] = num_error;
    }
  }
  if (v.active_item == 2) {
    if (debug_active)
      call_to_debug = 1;
    end_dialog = 1;
  }
  if (v.active_item == 3)
    exer(0);
}

void _err0(void) {
  v.type = 1;
  v.title = text[1];
  v.w = text_len((byte *)v_text) + 8;
  if (v.w < 218 + 16)
    v.w = 218 + 16;
  v.h = 38 + 16 + 8;
  v.paint_handler = _err1;
  v.click_handler = _err2;

  //  TODO: add the missing one and check that 218 is enough
  _button(text[11], 7, v.h - 14, 0);
  _button(text[12], 7 + text_len(text[11]) + 14, v.h - 14, 0);
  _button(text[13], v.w - 8 - text_len(text[14]) - 14, v.h - 14, 2);
  _button(text[14], v.w - 8, v.h - 14, 2);
}

//----------------------------------------------------------------------------
//      Error messages - Debugger version
//----------------------------------------------------------------------------

extern int ignore_errors;

void e(int text_id) {
#ifdef __EMSCRIPTEN__
  return;
#endif
  int smouse_x, smouse_y;
  int reloj_e = frame_clock, n;
  int ticks_e = ticks;
  int dr = dacout_r, dg = dacout_g, db = dacout_b;

  if (v_function == -1)
    return; // Some errors are better off ignored...

  num_error = text_id;
  te = (char *)text[text_id];
  n = 0;
  while (n < num_skipped) {
    if (skipped[n] == num_error)
      break;
    n++;
  }
  if (ignore_errors || n < num_skipped)
    return;

  smouse_x = mouse->x;
  smouse_y = mouse->y;
  set_mouse(mouse_x, mouse_y);
  if (!v.type)
    memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 16;

  mouse_graf = 1;
  v_text = (char *)text[text_id];
  show_dialog(_err0);
  dacout_r = dr;
  dacout_g = dg;
  dacout_b = db;
  frame_clock = reloj_e;
  ticks = ticks_e;
  set_mouse(smouse_x, smouse_y);
}

//-----------------------------------------------------------------------------
//      Dialog box when a DEBUG statement is reached
//-----------------------------------------------------------------------------

void deb1(void) {
  int w = v.w / big2, h = v.h / big2, n;
  _show_items();
  wwrite(v.ptr, w, h, 4, 12, 0, text[8], c4);
  if (num_objects) {
    for (n = 0; n < num_objects; n++)
      if (o[n].type == tproc && o[n].v0 == mem[id + _Bloque])
        break;
    if (n < num_objects)
      wwrite(v.ptr, w, h, 46, 12, 0, (byte *)vnom + o[n].name, c3);
    else
      wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
  } else
    wwrite(v.ptr, w, h, 46, 12, 0, text[9], c3);
}

void deb2(void) {
  _process_items();
  if (v.active_item == 2) {
    mem[ip - 1] = 0; // NOP out the debug statement
    end_dialog = 1;
  }
  if (v.active_item == 0) {
    if (debug_active) {
      call_to_debug = 1;
      process_stoped = id;
    }
    end_dialog = 1;
  }
  if (v.active_item == 1)
    exer(0);
}

void deb0(void) {
  v.type = 1;
  v.title = text[15];
  v.w = 190;
  v.h = 38;
  v.paint_handler = deb1;
  v.click_handler = deb2;
  _button(text[16], v.w / 2, v.h - 14, 1);
  _button(text[17], v.w - 8, v.h - 14, 2);
  _button(text[18], 7, v.h - 14, 0);
}

void deb(void) {
  int smouse_x, smouse_y;
  int reloj_e = frame_clock;
  int ticks_e = ticks;
  int dr = dacout_r, dg = dacout_g, db = dacout_b;

  smouse_x = mouse->x;
  smouse_y = mouse->y;
  set_mouse(mouse_x, mouse_y);
  if (!v.type)
    memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);
  dacout_r = 0;
  dacout_g = 0;
  dacout_b = 0;
  dacout_speed = 16;

  mouse_graf = 1;
  show_dialog(deb0);

  dacout_r = dr;
  dacout_g = dg;
  dacout_b = db;
  frame_clock = reloj_e;
  ticks = ticks_e;
  set_mouse(smouse_x, smouse_y);
}

//----------------------------------------------------------------------------
//    Determine if all processes have been executed
//----------------------------------------------------------------------------

int procesos_ejecutados(void) {
  int max = 0x80000000, ide = 0;
  if (process_stoped)
    return (0);
  id = id_old;
  do {
    if (mem[id + _Status] == 2 && !mem[id + _Executed] && mem[id + _Priority] > max) {
      ide = id;
      max = mem[id + _Priority];
    }
    if (id == id_end)
      id = id_start;
    else
      id += iloc_len;
  } while (id != id_old);
  return (!ide);
}

//----------------------------------------------------------------------------
//      Build ids[] (process list, in execution order)
//----------------------------------------------------------------------------

void determine_ids(void) {
  int max, ide, id;

  iids = 0; // Number of processes
  ids_next = -1;

  // Mark all processes as not yet in the list

  for (ide = id_start; ide <= id_end; ide += iloc_len)
    mem[ide + _Painted] = 0;

  // First add already-executed processes to the list

  id_old = id_start;

  do {
    ide = 0;
    max = 0x80000000;
    id = id_old;
    do {
      if (mem[id + _Status] == 2 && mem[id + _Executed] && !mem[id + _Painted] &&
          mem[id + _Priority] > max) {
        ide = id;
        max = mem[id + _Priority];
      }
      if (id == id_end)
        id = id_start;
      else
        id += iloc_len;
    } while (id != id_old);
    if (ide) {
      mem[ide + _Painted] = 1;
      if (iids < max_procesos)
        ids[iids++] = ide;
    }
  } while (ide);

  // Then the stopped process, if any

  if (process_stoped) {
    mem[process_stoped + _Painted] = 1;
    if (ids_next == -1)
      ids_next = iids;
    if (iids < max_procesos)
      ids[iids++] = process_stoped;
  }

  // Then add processes yet to be executed

  do {
    ide = 0;
    max = 0x80000000;
    id = id_old;
    do {
      if (mem[id + _Status] == 2 && !mem[id + _Executed] && !mem[id + _Painted] &&
          mem[id + _Priority] > max) {
        ide = id;
        max = mem[id + _Priority];
      }
      if (id == id_end)
        id = id_start;
      else
        id += iloc_len;
    } while (id != id_old);
    if (ide) {
      mem[ide + _Painted] = 1;
      if (ids_next == -1)
        ids_next = iids;
      if (iids < max_procesos)
        ids[iids++] = ide;
    }
  } while (ide);

  // Then add remaining processes (any status)

  do {
    ide = 0;
    max = 0x80000000;
    id = id_old;
    do {
      if (mem[id + _Status] != 0 && !mem[id + _Painted] && mem[id + _Priority] > max) {
        ide = id;
        max = mem[id + _Priority];
      }
      if (id == id_end)
        id = id_start;
      else
        id += iloc_len;
    } while (id != id_old);
    if (ide) {
      mem[ide + _Painted] = 1;
      if (iids < max_procesos)
        ids[iids++] = ide;
    }
  } while (ide);
}


//----------------------------------------------------------------------------
//      Draw the process list and info about the selected process
//----------------------------------------------------------------------------

void draw_proc_list(void) {
  char msg[512];
  byte *ptr = v.ptr;
  int n, m, x;
  int w = v.w / big2, h = v.h / big2;

  wbox(ptr, w, h, c1, 4, 20, 118, 126 - 5 - 16 - 32); // Process listbox fill

  for (m = ids_ini; m < ids_ini + 9 && m < iids; m++) {
    if (m == ids_select) {
      x = c4;
      wbox(ptr, w, h, c01, 4, 20 + (m - ids_ini) * 8, 118, 9); // Selected process highlight
    } else
      x = c3;
    for (n = 0; n < num_objects; n++)
      if (o[n].type == tproc && o[n].v0 == mem[ids[m] + _Bloque])
        break;
    if (n < num_objects)
      div_strcpy(msg, sizeof(msg), (char *)vnom + o[n].name);
    else
      div_strcpy(msg, sizeof(msg), (char *)text[9]);
    div_strcat(msg, sizeof(msg), "(");
    itoa(mem[ids[m] + _Id], msg + strlen(msg), 10);
    div_strcat(msg, sizeof(msg), ")");
    wwrite_in_box(ptr, w, 90, h, 10, 21 + (m - ids_ini) * 8, 0, (byte *)msg, x);
    switch (mem[ids[m] + _Status]) {
    case 0:
      msg[0] = '-';
      break;
    case 1:
      msg[0] = 'K';
      break;
    case 2:
      msg[0] = 'A';
      break;
    case 3:
      msg[0] = 'S';
      break;
    case 4:
      msg[0] = 'F';
      break;
    default:
      msg[0] = '?';
      break;
    }
    msg[1] = 0;
    wwrite(ptr, w, h, 93, 21 + (m - ids_ini) * 8, 0, (byte *)msg, x);
    if (mem[ids[m] + _Executed])
      div_snprintf(msg, sizeof(msg), "%03d", mem[ids[m] + _Frame] + 100);
    else
      div_snprintf(msg, sizeof(msg), "%03d", mem[ids[m] + _Frame]);
    div_strcat(msg, sizeof(msg), "%");
    wwrite_in_box(ptr, w, 121, h, 101, 21 + (m - ids_ini) * 8, 0, (byte *)msg, x);
    if (m == ids_next)
      wput(ptr, w, h, 4, 21 + (m - ids_ini) * 8, 36);
  }

  wrectangle(ptr, w, h, c0, 91, 19, 1, 128 - 5 - 16 - 32);
  wrectangle(ptr, w, h, c0, 91 + 8, 19, 1, 128 - 5 - 16 - 32);

  wbox(ptr, w, h, c12, 133, 11, 119 + 304 - 256, 7); // Process identifier
  div_strcpy(msg, sizeof(msg), (char *)text[19]);
  itoa(ids[ids_select], msg + strlen(msg), 10);
  switch (mem[ids[ids_select] + _Status]) {
  case 1:
    div_strcat(msg, sizeof(msg), (char *)text[20]);
    break;
  case 2:
    div_strcat(msg, sizeof(msg), (char *)text[21]);
    break;
  case 3:
    div_strcat(msg, sizeof(msg), (char *)text[22]);
    break;
  case 4:
    div_strcat(msg, sizeof(msg), (char *)text[23]);
    break;
  default:
    div_strcat(msg, sizeof(msg), (char *)text[24]);
    break;
  }
  wwrite(ptr, w, h, 134, 11, 0, (byte *)msg, c1);
  wwrite(ptr, w, h, 133, 11, 0, (byte *)msg, c3);

  for (n = 0; n < num_objects; n++)
    if (o[n].type == tproc && o[n].v0 == mem[ids[ids_select] + _Bloque])
      break;
  if (n < num_objects)
    div_strcpy(msg, sizeof(msg), vnom + o[n].name);
  else
    div_strcpy(msg, sizeof(msg), (char *)text[9]);

  wbox(ptr, w, h, c1, 134, 20, 118 + 304 - 256, 9); // Process name box
  wwrite_in_box(ptr, w, 252, h, 135, 21, 0, (byte *)msg, c3);

  wbox(ptr, w, h, c12, 200 - 16 + 304 - 256 - 4, 47, 54 + 16 + 4, 32); // Main process variables

  if (mem[ids[ids_select] + _Resolution])
    div_snprintf(msg, sizeof(msg), "x=%f",
                 (float)mem[ids[ids_select] + _X] / (float)mem[ids[ids_select] + _Resolution]);
  else
    div_snprintf(msg, sizeof(msg), "x=%d", mem[ids[ids_select] + _X]);
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48, 1, (byte *)msg, c3);
  if (mem[ids[ids_select] + _Resolution])
    div_snprintf(msg, sizeof(msg), "y=%f",
                 (float)mem[ids[ids_select] + _Y] / (float)mem[ids[ids_select] + _Resolution]);
  else
    div_snprintf(msg, sizeof(msg), "y=%d", mem[ids[ids_select] + _Y]);
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48 + 8, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48 + 8, 1, (byte *)msg, c3);

  switch (mem[ids[ids_select] + _Ctype]) {
  case 0:
    div_strcpy(msg, sizeof(msg), (char *)text[25]);
    break;
  case 1:
    div_strcpy(msg, sizeof(msg), (char *)text[26]);
    break;
  case 2:
    div_strcpy(msg, sizeof(msg), (char *)text[27]);
    break;
  case 3:
    div_strcpy(msg, sizeof(msg), (char *)text[28]);
    break;
  default:
    div_strcpy(msg, sizeof(msg), (char *)text[29]);
    break;
  }
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48 + 16, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48 + 16, 1, (byte *)msg, c3);

  switch (mem[ids[ids_select] + _Flags] & 7) {
  case 0:
    div_strcpy(msg, sizeof(msg), (char *)text[30]);
    break;
  case 1:
    div_strcpy(msg, sizeof(msg), (char *)text[31]);
    break;
  case 2:
    div_strcpy(msg, sizeof(msg), (char *)text[32]);
    break;
  case 3:
    div_strcpy(msg, sizeof(msg), (char *)text[33]);
    break;
  case 4:
    div_strcpy(msg, sizeof(msg), (char *)text[34]);
    break;
  case 5:
    div_strcpy(msg, sizeof(msg), (char *)text[35]);
    break;
  case 6:
    div_strcpy(msg, sizeof(msg), (char *)text[36]);
    break;
  case 7:
    div_strcpy(msg, sizeof(msg), (char *)text[37]);
    break;
  }
  wwrite_in_box(ptr, w, w - 2, h, w - 39, 48 + 24, 1, (byte *)msg, c1);
  wwrite_in_box(ptr, w, w - 2, h, w - 40, 48 + 24, 1, (byte *)msg, c3);

  wbox(ptr, w, h, c1, 134, 32, 46, 46); // Process graphic
  process_graph(ids[ids_select], ptr + 32 * big2 * v.w + 134 * big2, v.w, 46 * big2, 46 * big2);

  paint_segment();
}

//----------------------------------------------------------------------------
//      Draw a process graphic for the debugger
//----------------------------------------------------------------------------

void process_graph(int id, byte *q, int van, int w, int h) {
  int file, graph, angle;
  int x, y, n, m;
  int xr, ixr, yr, iyr, old_w;
  short xg, yg;
  int *ptr;
  byte *p;

  file = mem[id + _File];
  graph = mem[id + _Graph];
  angle = mem[id + _Angle];

  if (file < 0 || file > max_fpgs)
    return;

  if (mem[id + _Ctype] == 2 || mem[id + _Ctype] == 3)
    angle = 0;
  else if ((n = mem[id + _XGraph]) > 0) {
    m = mem[n];
    if (m < 1 || m > 256)
      return;
    while (angle >= 2 * pi)
      angle -= 2 * pi;
    while (angle < 0)
      angle += 2 * pi;
    mem[id + _Flags] &= 254;
    graph = ((angle + (2 * pi) / (m * 2)) * m) / (2 * pi);
    angle = 0;
    if (graph >= m)
      graph = 0;
    if ((graph = mem[n + 1 + graph]) < 0) {
      graph = -graph;
      mem[id + _Flags] |= 1;
    }
  }

  if (file)
    max_grf = 1000;
  else
    max_grf = 2000;
  if (graph <= 0 || graph >= max_grf)
    return;
  if (g[file].grf == NULL)
    return;
  if ((ptr = g[file].grf[graph]) == NULL)
    return;

  x = mem[id + _X];
  y = mem[id + _Y];
  if (mem[id + _Resolution] > 0) {
    x /= mem[id + _Resolution];
    y /= mem[id + _Resolution];
  }

  if (ptr[15] == 0 || *((word *)ptr + 32) == 65535) {
    xg = ptr[13] / 2;
    yg = ptr[14] / 2;
  } else {
    xg = *((word *)ptr + 32);
    yg = *((word *)ptr + 33);
  }

  if (angle) {
    clipx0 = x;
    clipy0 = y;
    clipx1 = ptr[13];
    clipy1 = ptr[14];
    sp_size(&clipx0, &clipy0, &clipx1, &clipy1, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);
  } else if (mem[id + _Size] != 100) {
    clipx0 = x;
    clipy0 = y;
    clipx1 = ptr[13];
    clipy1 = ptr[14];
    sp_size_scaled(&clipx0, &clipy0, &clipx1, &clipy1, xg, yg, mem[id + _Size], mem[id + _Flags]);
  } else {
    if (mem[id + _Flags] & 1)
      clipx0 = x - (ptr[13] - 1 - xg);
    else
      clipx0 = x - xg;
    if (mem[id + _Flags] & 2)
      clipy0 = y - (ptr[14] - 1 - yg);
    else
      clipy0 = y - yg;
    clipx1 = clipx0 + ptr[13] - 1;
    clipy1 = clipy0 + ptr[14] - 1;
  }

  buffer_w = clipx1 - clipx0 + 1;
  buffer_h = clipy1 - clipy0 + 1;
  if ((buffer = (byte *)malloc(buffer_w * buffer_h)) == NULL)
    return;
  memset(buffer, 0, buffer_w * buffer_h);

  // Now paint the sprite into the buffer (clip...)

  put_collision(buffer, ptr, x, y, xg, yg, angle, mem[id + _Size], mem[id + _Flags]);

  // window q, van, of an x al

  wbox(v.ptr, v.w / big2, v.h / big2, c0, 134, 32, w / big2, h / big2); // Set black background

  if (buffer_w <= w && buffer_h <= h) { // If it fits in the window, center it

    q += (w - buffer_w) / 2 + ((h - buffer_h) / 2) * van;
    p = buffer;
    old_w = buffer_w;
    do {
      do {
        *q++ = *p++;
      } while (--buffer_w);
      q += van - (buffer_w = old_w);
    } while (--buffer_h);

  } else { // If it doesn't fit, scale it down

    ixr = (float)(buffer_w * 256) / (float)w;
    iyr = (float)(buffer_h * 256) / (float)h;

    if (ixr > iyr) {
      yg = h - (float)(buffer_h * 256) / (float)ixr;
      q += (yg / 2) * van;
      h -= yg;
      iyr = ixr;
    } else if (iyr > ixr) {
      xg = w - (float)(buffer_w * 256) / (float)iyr;
      q += xg / 2;
      w -= xg;
      ixr = iyr;
    }
    old_w = w;
    yr = 0;

    if (w && h)
      do {
        p = buffer + (yr >> 8) * buffer_w;
        xr = 0;
        do {
          *q = *(p + (xr >> 8));
          q++;
          xr += ixr;
        } while (--w);
        yr += iyr;
        q += van - (w = old_w);
      } while (--h);
  }

  free(buffer);
}

//----------------------------------------------------------------------------
//      Debug function (invoked by pressing F12 during execution)
//----------------------------------------------------------------------------

void debug(void) {
  smouse_x = mouse->x;
  smouse_y = mouse->y;
  set_mouse(mouse_x, mouse_y);

  debug_clock = frame_clock;
  ticks_debug = ticks;

  dr = dacout_r;
  dg = dacout_g;
  db = dacout_b;

  memcpy(screen_buffer_debug, screen_buffer, vga_width * vga_height);

  new_palette = 0;
  mouse_graf = 1;
  show_dialog(debug0);

  dacout_r = dr;
  dacout_g = dg;
  dacout_b = db;

  frame_clock = debug_clock;
  ticks = ticks_debug;

  set_mouse(smouse_x, smouse_y);
}
