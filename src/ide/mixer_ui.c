
//-----------------------------------------------------------------------------
//      Módulo que contiene el código del mixer
//-----------------------------------------------------------------------------

#include "global.h"
#include "mixer.h"

//-----------------------------------------------------------------------------
//  Constantes
//-----------------------------------------------------------------------------

#define MIX_BAR1 7
#define MIX_BAR2 25
#define MIX_BAR3 43

//-----------------------------------------------------------------------------
//  Prototipos
//-----------------------------------------------------------------------------

void mixer0(void);
void mixer1(void);
void mixer2(void);

//-----------------------------------------------------------------------------
//  Variables
//-----------------------------------------------------------------------------

char dummy_mixer_array[21 * 1];
struct t_listbox lvol_fx = {MIX_BAR1, 11, dummy_mixer_array, 1, 5, 1};
struct t_listbox lvol_cd = {MIX_BAR2, 11, dummy_mixer_array, 1, 5, 1};
struct t_listbox lvol_ma = {MIX_BAR3, 11, dummy_mixer_array, 1, 5, 1};

//-----------------------------------------------------------------------------
//  Mixer window
//-----------------------------------------------------------------------------

void mixer0(void) {
  v.type = WIN_MIXER;
  v.title = texts[421];
  v.name = texts[421];
  v.w = 59;
  v.h = 76;

  v.paint_handler = (void_return_type_t)mixer1;
  v.click_handler = (void_return_type_t)mixer2;
}

void mixer1(void) {
  int w = v.w / big2, h = v.h / big2;

  _show_items();

  lvol_fx.created = 0;
  lvol_fx.total_items = 20;
  lvol_cd.created = 0;
  lvol_cd.total_items = 20;
  lvol_ma.created = 0;
  lvol_ma.total_items = 20;

  create_listbox(&lvol_fx);
  create_listbox(&lvol_cd);
  create_listbox(&lvol_ma);

  lvol_fx.first_visible = 15 - setup_file.vol_fx;
  lvol_cd.first_visible = 15 - setup_file.vol_cd;
  lvol_ma.first_visible = 15 - setup_file.vol_ma;

  create_listbox(&lvol_fx);
  create_listbox(&lvol_cd);
  create_listbox(&lvol_ma);

  update_listbox(&lvol_fx);
  update_listbox(&lvol_cd);
  update_listbox(&lvol_ma);

  wrectangle(v.ptr, w, h, c0, MIX_BAR1 - 4, 56, 17, 17);
  wrectangle(v.ptr, w, h, c0, MIX_BAR2 - 4, 56, 17, 17);
  wrectangle(v.ptr, w, h, c0, MIX_BAR3 - 4, 56, 17, 17);

  if (setup_file.mut_fx)
    wput(v.ptr, w, h, MIX_BAR1 - 3, 57, -225);
  else
    wput(v.ptr, w, h, MIX_BAR1 - 3, 57, -226);
  if (setup_file.mut_cd)
    wput(v.ptr, w, h, MIX_BAR2 - 3, 57, -227);
  else
    wput(v.ptr, w, h, MIX_BAR2 - 3, 57, -228);
  if (setup_file.mut_ma)
    wput(v.ptr, w, h, MIX_BAR3 - 3, 57, -229);
  else
    wput(v.ptr, w, h, MIX_BAR3 - 3, 57, -230);
}

void mixer2(void) {
  byte Tocado = 0;

  _process_items();

  v_pause = 1;
  update_listbox(&lvol_fx);
  update_listbox(&lvol_cd);
  update_listbox(&lvol_ma);
  v_pause = 0;

  if (mouse_b && lvol_fx.zone == 4) {
    setup_file.vol_fx = 15 - lvol_fx.first_visible;
    Tocado = 1;
  }
  if (!(mouse_b & 1) && (prev_mouse_buttons & 1) && (lvol_fx.zone == 2 || lvol_fx.zone == 3)) {
    setup_file.vol_fx = 15 - lvol_fx.first_visible;
    Tocado = 1;
  }
  if (mouse_b && lvol_cd.zone == 4) {
    setup_file.vol_cd = 15 - lvol_cd.first_visible;
    Tocado = 1;
  }
  if (!(mouse_b & 1) && (prev_mouse_buttons & 1) && (lvol_cd.zone == 2 || lvol_cd.zone == 3)) {
    setup_file.vol_cd = 15 - lvol_cd.first_visible;
    Tocado = 1;
  }
  if (mouse_b && lvol_ma.zone == 4) {
    setup_file.vol_ma = 15 - lvol_ma.first_visible;
    Tocado = 1;
  }
  if (!(mouse_b & 1) && (prev_mouse_buttons & 1) && (lvol_ma.zone == 2 || lvol_ma.zone == 3)) {
    setup_file.vol_ma = 15 - lvol_ma.first_visible;
    Tocado = 1;
  }

  if (!(mouse_b & 1) && (prev_mouse_buttons & 1)) {
    if (wmouse_in(MIX_BAR1 - 5, 57, 17, 17)) {
      setup_file.mut_fx = !setup_file.mut_fx;
      Tocado = 1;
    }
    if (wmouse_in(MIX_BAR2 - 5, 57, 17, 17)) {
      setup_file.mut_cd = !setup_file.mut_cd;
      Tocado = 1;
    }
    if (wmouse_in(MIX_BAR3 - 5, 57, 17, 17)) {
      setup_file.mut_ma = !setup_file.mut_ma;
      Tocado = 1;
    }
  }

  if (Tocado) // Actualiza los valores del mixer
  {
    Tocado = 0;
    lvol_fx.first_visible = 15 - setup_file.vol_fx;
    lvol_cd.first_visible = 15 - setup_file.vol_cd;
    lvol_ma.first_visible = 15 - setup_file.vol_ma;
    set_mixer();
    call((void_return_type_t)v.paint_handler);
    v.redraw = 1;
  }
}
