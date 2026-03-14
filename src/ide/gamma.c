
//-----------------------------------------------------------------------------
//      Módulo que contiene el código de control de gamas
//-----------------------------------------------------------------------------

#include "global.h"

//-----------------------------------------------------------------------------
//      Constantes del módulo
//-----------------------------------------------------------------------------

#define GAMMA_WIDTH 138
#define GAMMA_HEIGHT  60
#define GAMMA_X   4
#define GAMMA_Y   12
#define GAMMA_W   130

//-----------------------------------------------------------------------------
//  Estructura para guardar el estado de los colores de control de la gama
//-----------------------------------------------------------------------------

byte saved_colors[128];
struct _gcolor saved_gamma[9];

struct {
  int x;
  int y;
} button_coords[9];

//-----------------------------------------------------------------------------
//  Prototipos
//-----------------------------------------------------------------------------

void gamma0(void);
void gamma1(void);
void gamma2(void);
void gamma3(void);

//-----------------------------------------------------------------------------
//  Variables y funciones externas
//-----------------------------------------------------------------------------

extern int sel_color_font;
extern int sel_color_ok;
void selcolor0(void);

//-----------------------------------------------------------------------------
//  Crea una gama a partir de los colores selecionados
//-----------------------------------------------------------------------------

void create_gradient_colors(struct _gcolor *gradient_config, byte *gradient_buf) {
  int con, boton1, boton2, n;
  float tmp_r, tmp_g, tmp_b;

  for (n = 0; n < 9; n++)
    if (gradient_config[n].selec) {
      gradient_config[n].color =
          find_color(gradient_config[n].r, gradient_config[n].g, gradient_config[n].b);
    }

  if (!gradient_config[0].selec) {
    gradient_config[0].color = find_color(0, 0, 0);
    gradient_config[0].r = dac[gradient_config[0].color * 3];
    gradient_config[0].g = dac[gradient_config[0].color * 3 + 1];
    gradient_config[0].b = dac[gradient_config[0].color * 3 + 2];
  }

  if (!gradient_config[8].selec) {
    gradient_config[8].color = find_color(63, 63, 63);
    gradient_config[8].r = dac[gradient_config[8].color * 3];
    gradient_config[8].g = dac[gradient_config[8].color * 3 + 1];
    gradient_config[8].b = dac[gradient_config[8].color * 3 + 2];
  }

  tmp_r = gradient_config[0].r;
  tmp_g = gradient_config[0].g;
  tmp_b = gradient_config[0].b;

  boton1 = 0, boton2 = 0;
  while (1) {
    boton2++;
    if (gradient_config[boton2].selec || boton2 == 8) {
      while (boton1 + 1 < boton2) {
        tmp_r += ((float)gradient_config[boton2].r - tmp_r) / (float)(boton2 - boton1);
        tmp_g += ((float)gradient_config[boton2].g - tmp_g) / (float)(boton2 - boton1);
        tmp_b += ((float)gradient_config[boton2].b - tmp_b) / (float)(boton2 - boton1);

        if (tmp_r > 63.0)
          tmp_r = 63.0;
        else if (tmp_r < 0.0)
          tmp_r = 0.0;
        if (tmp_g > 63.0)
          tmp_g = 63.0;
        else if (tmp_g < 0.0)
          tmp_g = 0.0;
        if (tmp_b > 63.0)
          tmp_b = 63.0;
        else if (tmp_b < 0.0)
          tmp_b = 0.0;

        boton1++;

        gradient_config[boton1].color = find_color(tmp_r, tmp_g, tmp_b);
        gradient_config[boton1].r = dac[gradient_config[boton1].color * 3];
        gradient_config[boton1].g = dac[gradient_config[boton1].color * 3 + 1];
        gradient_config[boton1].b = dac[gradient_config[boton1].color * 3 + 2];

        if (gradient_config[boton1].color != gradient_config[boton1 - 1].color) {
          tmp_r = gradient_config[boton1].r;
          tmp_g = gradient_config[boton1].g;
          tmp_b = gradient_config[boton1].b;
        }
      }
      boton1 = boton2;
    }
    if (boton2 == 8)
      break;
  }

  for (con = 0; con < 8; con++)
    gradient_buf[con * 16] = gradient_config[con].color;
  gradient_buf[127] = gradient_config[8].color;

  tmp_r = gradient_config[0].r;
  tmp_g = gradient_config[0].g;
  tmp_b = gradient_config[0].b;

  boton1 = 0, boton2 = 0;
  while (1) {
    boton2++;
    if (boton2 % 16 == 0 || boton2 == 127) {
      while (boton1 + 1 < boton2) {
        tmp_r += ((float)dac[gradient_buf[boton2] * 3] - tmp_r) / (float)(boton2 - boton1);
        tmp_g += ((float)dac[gradient_buf[boton2] * 3 + 1] - tmp_g) / (float)(boton2 - boton1);
        tmp_b += ((float)dac[gradient_buf[boton2] * 3 + 2] - tmp_b) / (float)(boton2 - boton1);

        if (tmp_r > 63.0)
          tmp_r = 63.0;
        else if (tmp_r < 0.0)
          tmp_r = 0.0;
        if (tmp_g > 63.0)
          tmp_g = 63.0;
        else if (tmp_g < 0.0)
          tmp_g = 0.0;
        if (tmp_b > 63.0)
          tmp_b = 63.0;
        else if (tmp_b < 0.0)
          tmp_b = 0.0;

        boton1++;

        gradient_buf[boton1] = find_color(tmp_r, tmp_g, tmp_b);

        if (gradient_buf[boton1] != gradient_buf[boton1 - 1]) {
          tmp_r = dac[gradient_buf[boton1] * 3];
          tmp_g = dac[gradient_buf[boton1] * 3 + 1];
          tmp_b = dac[gradient_buf[boton1] * 3 + 2];
        }
      }
      boton1 = boton2;
    }
    if (boton2 == 127)
      break;
  }
}

//-----------------------------------------------------------------------------
//  Codigo principal del show_dialog de gamas
//-----------------------------------------------------------------------------

void gamma0(void) {
  int con;

  v.type = WIN_DIALOG;

  v.title = texts[420];
  v.w = GAMMA_WIDTH;
  v.h = GAMMA_HEIGHT;

  v.paint_handler = (void_return_type_t)gamma1;
  v.click_handler = (void_return_type_t)gamma2;
  v.close_handler = (void_return_type_t)gamma3;

  for (con = 0; con < 9; con++) {
    button_coords[con].y = 33;
  }

  button_coords[0].x = 10;
  button_coords[1].x = button_coords[0].x + 14;
  button_coords[2].x = button_coords[0].x + 14 * 2;
  button_coords[3].x = button_coords[0].x + 14 * 3;
  button_coords[4].x = button_coords[0].x + 14 * 4;
  button_coords[5].x = button_coords[0].x + 14 * 5;
  button_coords[6].x = button_coords[0].x + 14 * 6;
  button_coords[7].x = button_coords[0].x + 14 * 7;
  button_coords[8].x = button_coords[0].x + 14 * 8;

  // Aqui se inicializa el estado de la gama y los botones (r, g, b y selec)

  create_dac4();
  memcpy(saved_colors, gradient_buf, 128);
  memcpy(saved_gamma, gradient_config, sizeof(saved_gamma));

  // Botones Aceptar/Cancelar

  _button(100, 7, v.h - 14, ALIGN_TL);
  _button(101, v.w - 8, v.h - 14, ALIGN_TR);

  v_accept = 0;
}

void gamma1(void) {
  int w = v.w / big2, h = v.h / big2;
  int con;

  _show_items();

  create_gradient_colors(gradient_config, gradient_buf);

  // Mostrar botones

  for (con = 0; con < 9; con++) {
    if (gradient_config[con].selec)
      wput(v.ptr, w, h, button_coords[con].x, button_coords[con].y, -44);
    else
      wput(v.ptr, w, h, button_coords[con].x, button_coords[con].y, -34);

    wrectangle(v.ptr, w, h, c0, button_coords[con].x - 4, button_coords[con].y - 15, 14, 14);
    wbox(v.ptr, w, h, gradient_config[con].color, button_coords[con].x - 4 + 1,
         button_coords[con].y - 15 + 1, 12, 12);
  }

  // Mostrar gama

  wrectangle(v.ptr, w, h, c0, GAMMA_X, GAMMA_Y, GAMMA_W, 4);
  for (con = 0; con < 128; con++) {
    wbox(v.ptr, w, h, gradient_buf[con], GAMMA_X + con + 1, GAMMA_Y + 1, 1, 2);
  }
}

void gamma2(void) {
  int con;

  _process_items();

  switch (v.active_item) {
  case 0:
    end_dialog = 1;
    v_accept = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  }

  if (!(mouse_b & 1) && (prev_mouse_buttons & 1)) {
    for (con = 0; con < 9; con++) {
      if (wmouse_in(button_coords[con].x, button_coords[con].y, 7, 7)) {
        gradient_config[con].selec = !gradient_config[con].selec;
        call((void_return_type_t)v.paint_handler);
        v.redraw = 1;
      } else if (wmouse_in(button_coords[con].x - 3, button_coords[con].y - 15, 14, 14)) {
        show_dialog((void_return_type_t)selcolor0);
        if (sel_color_ok) {
          gradient_config[con].selec = 1;
          gradient_config[con].color = sel_color_font;
          gradient_config[con].r = dac[sel_color_font * 3];
          gradient_config[con].g = dac[sel_color_font * 3 + 1];
          gradient_config[con].b = dac[sel_color_font * 3 + 2];
        }
      }
    }
    call((void_return_type_t)v.paint_handler);
    v.redraw = 1;
  }
}

void gamma3(void) {
  if (!v_accept) {
    memcpy(gradient_buf, saved_colors, 128);
    memcpy(gradient_config, saved_gamma, sizeof(saved_gamma));
  }
}
