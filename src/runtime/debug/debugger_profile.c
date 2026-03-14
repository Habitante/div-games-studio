// Debugger Profiler — execution time profiling dialog

#include "debugger_internal.h"

// Profile - Execution time profiling
//-----------------------------------------------------------------------------

//int lp1[512];     // Object number for each process
//int lp_num;       // Number of process blocks in the list
//int lp_ini;       // First block displayed in the window
//int lp_select;    // Selected process block

//int obj_start; // Start of the first object (&obj[0])
//int obj_size;  // Size of each object (struct object)
//(The block of an ID is: (mem[ID+_Bloque]-obj_start)/obj_size;)

//unsigned f_time[256]; // Time consumed by the different functions
unsigned f_time_total;
unsigned f_exec_total;
unsigned f_paint_total;
unsigned f_max;

int lp2_num;
int lp2_ini;
int lp2_select; //???

//----------------------------------------------------------------------------

unsigned int get_tick(void);

#ifdef __WATCOMC__
#pragma aux get_tick = "mov ecx,10h" \
                       "db 0fh,032h" parm[] value[eax] modify[eax ecx edx]
#endif

// "rdmsr"
// "mov cl,4"
// "shrd eax,edx,cl"

void reset_tick(void);

#ifdef __WATCOMC__
#pragma aux reset_tick = "mov ecx,10h" \
                         "xor edx,edx" \
                         "xor eax,eax" \
                         "db 0fh,030h" parm[] modify[ecx eax edx]
#endif

// "wrmsr"

#define IntIncr (unsigned int)(11932 / 256)

unsigned int get_ticks(void) {
  return OSDEP_GetTicks();
}

void function_exec(int id, int n) { // Number, cycles
  if (n > 0)
    f_time[id] += n;
}

void process_exec(int id, int n) { // Id, cycles
  if (!debug_active)
    return;
  if (n > 0)
    o[(mem[id + _Bloque] - obj_start) / obj_size].v4 += n;
}

void process_paint(int id, int n) { // Id, cycles
  if (!debug_active)
    return;
  if (n > 0)
    o[(mem[id + _Bloque] - obj_start) / obj_size].v5 += n;
}

//----------------------------------------------------------------------------

#define lpy   (24 + 15) // Listbox with process profiles
#define lpnum 6
#define lpal  10

#define lp2y   (86 + 15) // Listbox with process profiles
#define lp2num 6
#define lp2al  10
#define lp2esp 98

int absolut = 0;

void create_profile_list(void) {
  int n;

  lp_num = 0;
  lp_ini = 0;
  lp_select = 0; // Processes
  lp2_num = 0;
  lp2_ini = 0;
  lp2_select = 0; // Functions

  f_exec_total = 0;
  f_paint_total = 0;
  f_max = 0;

  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tproc) {
      f_exec_total += o[n].v4;
      f_paint_total += o[n].v5;
      if (o[n].v4 > f_max)
        f_max = o[n].v4;
      if (o[n].v5 > f_max)
        f_max = o[n].v5;
    }
  }

  for (n = 0; n < 240; n++) {
    if (f_time[n] > f_max)
      f_max = f_time[n];
  }

  f_exec_total += f_time[255];
  f_paint_total += f_time[254];
  f_paint_total += f_time[253];
  f_paint_total += f_time[252];
  f_paint_total += f_time[251];
  f_paint_total += f_time[250];

  if (f_time[255] > f_max)
    f_max = f_time[255];
  if (f_time[254] > f_max)
    f_max = f_time[254];
  if (f_time[253] > f_max)
    f_max = f_time[253];
  if (f_time[252] > f_max)
    f_max = f_time[252];
  if (f_time[251] > f_max)
    f_max = f_time[251];
  if (f_time[250] > f_max)
    f_max = f_time[250];

  f_time_total = f_exec_total + f_paint_total;

  if (f_time_total == 0) {
    f_time_total = 1;
    f_max = 1;
  }

  for (n = 0; n < num_objects; n++) {
    if (o[n].type == tproc && (o[n].v4 > f_time_total / 10000 || o[n].v5 > f_time_total / 10000)) {
      lp1[lp_num++] = n;
    }
  }

  for (n = 0; n < 240; n++) {
    if (f_time[n] > f_time_total / 10000) {
      lp2[lp2_num++] = (char *)n;
    }
  }
}

void paint_profile_list(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int m, x, porcen, porcen2;
  char cwork[256];

  wbox(ptr, w, h, c1, 4, lpy, w - 16, lpnum * lpal - 1); // Process listbox bounds
  for (m = lp_ini; m < lp_ini + lpnum && m < lp_num; m++) {
    if (m == lp_select) {
      wbox(ptr, w, h, c01, 4, lpy + (m - lp_ini) * lpal, w - 16 - 130,
           lpal - 1); // Process listbox fill
      x = c4;
    } else
      x = c3;
    wrectangle(ptr, w, h, c0, w - 12 - 130, lpy - 1 + (m - lp_ini) * lpal, 66,
               lpal + 1); // Bar bounds

    wbox(ptr, w, h, c_g_low0, w - 11 - 130, lpy + (m - lp_ini) * lpal, 64,
         lpal - 1); // Execution

    porcen = (unsigned)(((double)o[lp1[m]].v4 * (double)10000.0) / (double)f_time_total);
    porcen2 = (unsigned)(((double)o[lp1[m]].v4 * (double)10000.0) / (double)f_max);
    wbox(ptr, w, h, c_g_low, w - 11 - 130, lpy + (m - lp_ini) * lpal, (64 * porcen2) / 10000,
         lpal - 1);
    div_snprintf(cwork, sizeof(cwork), "%d.%02d%c", porcen / 100, porcen % 100, '%');
    if (absolut) {
      if (game_frames > 0)
        div_snprintf(cwork, sizeof(cwork), "%d", (int)((o[lp1[m]].v4 * 100) / game_frames));
      else
        div_snprintf(cwork, sizeof(cwork), "0");
    }

    wwrite(ptr, w, h, w - 10 - 130 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork, c_g_low0);
    wwrite(ptr, w, h, w - 11 - 130 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork, c34);

    wbox(ptr, w, h, c_r_low0, w - 11 - 65, lpy + (m - lp_ini) * lpal, 64, lpal - 1); // Rendering

    porcen = (unsigned)(((double)o[lp1[m]].v5 * (double)10000.0) / (double)f_time_total);
    porcen2 = (unsigned)(((double)o[lp1[m]].v5 * (double)10000.0) / (double)f_max);
    wbox(ptr, w, h, c_r_low, w - 11 - 65, lpy + (m - lp_ini) * lpal, (64 * porcen2) / 10000,
         lpal - 1);
    div_snprintf(cwork, sizeof(cwork), "%d.%02d%c", porcen / 100, porcen % 100, '%');
    if (absolut) {
      if (game_frames > 0)
        div_snprintf(cwork, sizeof(cwork), "%d", (int)((o[lp1[m]].v5 * 100) / game_frames));
      else
        div_snprintf(cwork, sizeof(cwork), "0");
    }

    wwrite(ptr, w, h, w - 10 - 65 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork, c_r_low0);
    wwrite(ptr, w, h, w - 11 - 65 + 32, lpy + 1 + (m - lp_ini) * lpal, 1, (byte *)cwork, c34);

    wwrite_in_box(ptr, w, w - 13 - 131, h, 5, lpy + 1 + (m - lp_ini) * lpal, 0,
                  (byte *)vnom + o[lp1[m]].name, x);
    wbox(ptr, w, h, c0, 4, lpy + (m - lp_ini) * lpal + lpal - 1, w - 16, 1);
  }

  wbox(ptr, w, h, c1, 4, lp2y, w - lp2esp - 16, lp2num * lp2al - 1); // Process listbox bounds
  for (m = lp2_ini; m < lp2_ini + lp2num && m < lp2_num; m++) {
    if (m == lp2_select) {
      wbox(ptr, w, h, c01, 4, lp2y + (m - lp2_ini) * lp2al, w - lp2esp - 16 - 65,
           lp2al - 1); // Process listbox fill
      x = c4;
    } else
      x = c3;
    wrectangle(ptr, w, h, c0, w - lp2esp - 12 - 65, lp2y - 1 + (m - lp2_ini) * lp2al, 1,
               lp2al + 1); // Bar bounds

    wbox(ptr, w, h, c_g_low0, w - lp2esp - 11 - 65, lp2y + (m - lp2_ini) * lp2al, 64,
         lp2al - 1); // Execution

    porcen =
        (unsigned)(((double)f_time[(uintptr_t)lp2[m]] * (double)10000.0) / (double)f_time_total);
    porcen2 = (unsigned)(((double)f_time[(uintptr_t)lp2[m]] * (double)10000.0) / (double)f_max);
    wbox(ptr, w, h, c_g_low, w - lp2esp - 11 - 65, lp2y + (m - lp2_ini) * lp2al,
         (64 * porcen2) / 10000, lpal - 1);
    div_snprintf(cwork, sizeof(cwork), "%d.%02d%c", porcen / 100, porcen % 100, '%');
    if (absolut) {
      if (game_frames > 0)
        div_snprintf(cwork, sizeof(cwork), "%d",
                     (int)((f_time[(uintptr_t)lp2[m]] * 100) / game_frames));
      else
        div_snprintf(cwork, sizeof(cwork), "0");
    }

    wwrite(ptr, w, h, w - lp2esp - 10 - 33, lp2y + 1 + (m - lp2_ini) * lp2al, 1, (byte *)cwork,
           c_g_low0);
    wwrite(ptr, w, h, w - lp2esp - 11 - 33, lp2y + 1 + (m - lp2_ini) * lp2al, 1, (byte *)cwork,
           c34);

    wwrite_in_box(ptr, w, w - lp2esp - 13 - 66, h, 5, lp2y + 1 + (m - lp2_ini) * lp2al, 0,
                  (byte *)fname[(uintptr_t)lp2[m]], x);
    wbox(ptr, w, h, c0, 4, lp2y + (m - lp2_ini) * lp2al + lp2al - 1, w - lp2esp - 16, 1);
  }

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 0 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_g_low0, w - lp2esp - 1, lp2al * 0 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[255] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[255] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_g_low, w - lp2esp - 1, lp2al * 0 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[77], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[77],
                   (int)((f_time[255] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[77]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 0, 1, (byte *)cwork, c_g_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 0, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 1 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 1 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[254] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[254] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 1 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[78], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[78],
                   (int)((f_time[254] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[78]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 1, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 1, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 2 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 2 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[253] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[253] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 2 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[79], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[79],
                   (int)((f_time[253] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[79]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 2, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 2, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 3 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 3 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[252] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[252] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 3 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "Scroll %d.%02d%c", porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "Scroll %d", (int)((f_time[252] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "Scroll 0");
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 3, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 3, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 4 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 4 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[251] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[251] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 4 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[80], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[80],
                   (int)((f_time[251] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[80]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 4, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 4, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w - lp2esp - 2, lp2al * 5 + lp2y - 1, lp2esp - 1, lp2al + 1);
  wbox(ptr, w, h, c_r_low0, w - lp2esp - 1, lp2al * 5 + lp2y, lp2esp - 3, lp2al - 1);
  porcen = (unsigned)(((double)f_time[250] * (double)10000.0) / (double)f_time_total);
  porcen2 = (unsigned)(((double)f_time[250] * (double)10000.0) / (double)f_max);
  wbox(ptr, w, h, c_r_low, w - lp2esp - 1, lp2al * 5 + lp2y, ((lp2esp - 3) * porcen2) / 10000,
       lp2al - 1);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[81], porcen / 100, porcen % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[81],
                   (int)((f_time[250] * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s 0", text[81]);
  }
  wwrite(ptr, w, h, w - lp2esp + 48, lp2y + 1 + lp2al * 5, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w - lp2esp + 47, lp2y + 1 + lp2al * 5, 1, (byte *)cwork, c34);

  paint_profile_segment();
}

void paint_profile_segment(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = lpy + 7, max = lpy + lpnum * lpal - 13, n;
  float x;

  wbox(ptr, w, h, c2, w - 11, lpy + 8, 7, max - min + 3); // Clear the slider bar
  if (lp_num <= 1)
    n = min;
  else {
    x = (float)lp_select / (float)(lp_num - 1);
    n = min * (1 - x) + max * x;
  }
  wrectangle(ptr, w, h, c0, w - 12, n, 9, 5);
  wput(ptr, w, h, w - 11, n + 1, -43);

  min = lp2y + 7;
  max = lp2y + lp2num * lp2al - 13, n;

  wbox(ptr, w, h, c2, w - lp2esp - 11, lp2y + 8, 7, max - min + 3); // Clear the slider bar
  if (lp2_num <= 1)
    n = min;
  else {
    x = (float)lp2_select / (float)(lp2_num - 1);
    n = min * (1 - x) + max * x;
  }
  wrectangle(ptr, w, h, c0, w - lp2esp - 12, n, 9, 5);
  wput(ptr, w, h, w - lp2esp - 11, n + 1, -43);
}

void profile1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  char cwork[256];
  int n;
  unsigned x;

  _show_items();

  create_profile_list();

  wrectangle(ptr, w, h, c0, 3, 12, w - 6, 11); // Overall speed %
  wbox(ptr, w, h, c_b_low0, 4, 13, w - 8, 9);
  x = (unsigned)(ffps2 * 100.0);
  if (x > 10000)
    x = 10000;
  if (game_frames) {
    n = (int)(((double)f_time_total / game_frames) * 50.0);
    if (n > 10000)
      n = 10000;
    else if (n < 0)
      n = 0;
    x = 10000 - n;
  } else
    x = 0;
  wbox(ptr, w, h, c_b_low, 4, 13, ((w - 8) * x) / 10000, 9);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[82], x / 100, x % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s = %d", text[83],
                   (int)(((double)(f_exec_total + f_paint_total) / game_frames) * 100.0));
    else
      div_snprintf(cwork, sizeof(cwork), "%s = 0", text[83]);
  }
  wwrite(ptr, w, h, w / 2, 14, 1, (byte *)cwork, c_b_low0);
  wwrite(ptr, w, h, w / 2, 14, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, 3, 11 + 14, w / 2 - 4, 11); // Execution %
  wbox(ptr, w, h, c_g_low0, 4, 12 + 14, w / 2 - 6, 9);
  x = (unsigned)(((double)f_exec_total * (double)10000.0) / (double)f_time_total);
  wbox(ptr, w, h, c_g_low, 4, 12 + 14, ((w / 2 - 6) * x) / 10000, 9);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[84], x / 100, x % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[84],
                   (int)((f_exec_total * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s = 0", text[84]);
  }
  wwrite(ptr, w, h, 4 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c_g_low0);
  wwrite(ptr, w, h, 3 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, w / 2 + 1, 11 + 14, w / 2 - 4, 11); // Rendering %
  wbox(ptr, w, h, c_r_low0, w / 2 + 2, 12 + 14, w / 2 - 6, 9);
  x = (unsigned)(((double)f_paint_total * (double)10000.0) / (double)f_time_total);
  wbox(ptr, w, h, c_r_low, w / 2 + 2, 12 + 14, ((w / 2 - 6) * x) / 10000, 9);
  div_snprintf(cwork, sizeof(cwork), "%s %d.%02d%c", text[85], x / 100, x % 100, '%');
  if (absolut) {
    if (game_frames > 0)
      div_snprintf(cwork, sizeof(cwork), "%s %d", text[85],
                   (int)((f_paint_total * 100) / game_frames));
    else
      div_snprintf(cwork, sizeof(cwork), "%s = 0", text[85]);
  }
  wwrite(ptr, w, h, w / 2 + 3 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c_r_low0);
  wwrite(ptr, w, h, w / 2 + 2 + (w / 2 - 4) / 2, 13 + 14, 1, (byte *)cwork, c34);

  wrectangle(ptr, w, h, c0, 3, lpy - 1, w - 6, lpal + 1); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 3, lpy - 1, w - 6, lpnum * lpal + 1);
  wrectangle(ptr, w, h, c0, w - 12, lpy - 1, 9, lpnum * lpal + 1);
  wrectangle(ptr, w, h, c0, w - 12, lpy + 7, 9, lpnum * lpal + 1 - 16);
  wput(ptr, w, h, w - 11, lpy, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -40);

  wrectangle(ptr, w, h, c0, 3, lp2y - 1, w - lp2esp - 6, lp2al + 1); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 3, lp2y - 1, w - lp2esp - 6, lp2num * lp2al + 1);
  wrectangle(ptr, w, h, c0, w - lp2esp - 12, lp2y - 1, 9, lp2num * lp2al + 1);
  wrectangle(ptr, w, h, c0, w - lp2esp - 12, lp2y + 7, 9, lp2num * lp2al + 1 - 16);
  wput(ptr, w, h, w - lp2esp - 11, lp2y, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -40);

  paint_profile_list();
}

int lp2_button;

void profile2(void) {
  int n;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  float x;

  _process_items();

  if (skip_flush) {
    skip_flush = 0;
    return;
  }

  if (wmouse_in(3, lpy, w - 15, lpnum * lpal) && (mouse_b & MB_LEFT)) {
    n = lp_ini + (wmouse_y - lpy) / lpal;
    if (n < lp_num) {
      if (lp_select != n) {
        lp_select = n;
        paint_profile_list();
        v.redraw = 1;
      }
    }
  }

  if (wmouse_in(w - 12, lpy - 1, 9, 9)) {
    if (mouse_b & MB_LEFT) {
      if (lp_button == 0) {
        wput(ptr, w, h, w - 11, lpy, -41);
        lp_button = 1;
        if (lp_select) {
          if (lp_ini == lp_select--)
            lp_ini--;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp_button == 1) {
      wput(ptr, w, h, w - 11, lpy, -39);
      lp_button = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp_button == 1) {
    wput(ptr, w, h, w - 11, lpy, -39);
    lp_button = 0;
    v.redraw = 1;
  }


  if (wmouse_in(w - 11, lpy + 8, 7, lpnum * lpal - 17)) {
    mouse_graf = 13;
    if (lp_num > 1 && (mouse_b & MB_LEFT)) {
      x = (float)(wmouse_y - lpy - 8) / (float)(lpnum * lpal - 18);
      lp_select = x * (lp_num - 1);
      if (lp_select < lp_ini)
        lp_ini = lp_select;
      if (lp_select >= lp_ini + lpnum)
        lp_ini = lp_select - lpnum + 1;
      paint_profile_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(w - 12, lpy + lpnum * lpal - 9, 9, 9)) {
    if (mouse_b & MB_LEFT) {
      if (lp_button == 0) {
        wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -42);
        lp_button = 2;
        if (lp_select + 1 < lp_num) {
          if (lp_ini + lpnum == ++lp_select)
            lp_ini++;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp_button == 2) {
      wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -40);
      lp_button = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp_button == 2) {
    wput(ptr, w, h, w - 11, lpy + lpnum * lpal - 8, -40);
    lp_button = 0;
    v.redraw = 1;
  }

  // Functions listbox

  if (wmouse_in(3, lp2y, w - lp2esp - 15, lp2num * lp2al) && (mouse_b & MB_LEFT)) {
    n = lp2_ini + (wmouse_y - lp2y) / lp2al;
    if (n < lp2_num) {
      if (lp2_select != n) {
        lp2_select = n;
        paint_profile_list();
        v.redraw = 1;
      }
    }
  }

  if (wmouse_in(w - lp2esp - 12, lp2y - 1, 9, 9)) {
    if (mouse_b & MB_LEFT) {
      if (lp2_button == 0) {
        wput(ptr, w, h, w - lp2esp - 11, lp2y, -41);
        lp2_button = 1;
        if (lp2_select) {
          if (lp2_ini == lp2_select--)
            lp2_ini--;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp2_button == 1) {
      wput(ptr, w, h, w - lp2esp - 11, lp2y, -39);
      lp2_button = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (lp2_button == 1) {
    wput(ptr, w, h, w - lp2esp - 11, lp2y, -39);
    lp2_button = 0;
    v.redraw = 1;
  }


  if (wmouse_in(w - lp2esp - 11, lp2y + 8, 7, lp2num * lp2al - 17)) {
    mouse_graf = 13;
    if (lp2_num > 1 && (mouse_b & MB_LEFT)) {
      x = (float)(wmouse_y - lp2y - 8) / (float)(lp2num * lp2al - 18);
      lp2_select = x * (lp2_num - 1);
      if (lp2_select < lp2_ini)
        lp2_ini = lp2_select;
      if (lp2_select >= lp2_ini + lp2num)
        lp2_ini = lp2_select - lp2num + 1;
      paint_profile_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(w - lp2esp - 12, lp2y + lp2num * lp2al - 9, 9, 9)) {
    if (mouse_b & MB_LEFT) {
      if (lp2_button == 0) {
        wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -42);
        lp2_button = 2;
        if (lp2_select + 1 < lp2_num) {
          if (lp2_ini + lp2num == ++lp2_select)
            lp2_ini++;
          paint_profile_list();
          v.redraw = 1;
        }
      }
    } else if (lp2_button == 2) {
      wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -40);
      lp2_button = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (lp2_button == 2) {
    wput(ptr, w, h, w - lp2esp - 11, lp2y + lp2num * lp2al - 8, -40);
    lp2_button = 0;
    v.redraw = 1;
  }

  if (scan_code == 33 || kbdFLAGS[_F12])
    goto profiler_next_frame;

  switch (v.active_item) {
  case 0:
profiler_next_frame:
    skip_flush = 1;
    profiler_x = v.x;
    profiler_y = v.y;
    end_dialog = 1;
    break;
  case 1:
    for (n = 0; n < num_objects; n++) {
      if (o[n].type == tproc) {
        o[n].v4 = 0; // Time_exec
        o[n].v5 = 0; // Time_paint
      }
    }
    memset(f_time, 0, 256 * 4);
    ffps2 = 0.0f;
    game_ticks = 0.0f;
    game_frames = 0.0f;
    call(v.paint_handler);
    v.redraw = 1;
    break;
  case 2:
    end_dialog = 1;
    break;
  case 3:
    call(v.paint_handler);
    v.redraw = 1;
    break;
  }
}

void profile0(void) {
  v.type = WIN_DIALOG;
  v.title = text[86];

  v.w = 256;
  v.h = 165 + 15;
  v.paint_handler = profile1;
  v.click_handler = profile2;

  _button(text[87], 7, v.h - 14, ALIGN_TL);
  _button(text[88], 7 + 3 + text_len((byte *)"\15 Siguiente Frame "), v.h - 14, ALIGN_TL);
  _button(text[89], v.w - 8, v.h - 14, ALIGN_TR);
  _flag(text[90],
        7 + 3 + text_len((byte *)"\15 Siguiente Frame ") + 3 + text_len((byte *)"\15 Comenzar "),
        v.h - 14, &absolut);
}

//----------------------------------------------------------------------------
