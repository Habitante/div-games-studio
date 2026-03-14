// Debugger Inspector — variable browsing, editing, and visualization
#include "debugger_internal.h"

int member;

void create_variable_list(void) {
  int n, newest, include_flag;

  memset(used, 0, sizeof(int) * num_objects);

  num_var = 0;

  // Add variables in alphabetical order to the list var[num_var]

  do {
    newest = 0;
    for (n = 0; n < num_objects; n++) { // Find the smallest alphabetically

      if (!pre_defined && n < num_predefined)
        continue;
      if (!user_defined && n >= num_predefined)
        continue;
      if (used[n])
        continue;
      if (o[n].miembro)
        continue;

      switch (o[n].type) {
      case tcons:
        include_flag = show_const;
        break;
      case tcglo:
      case tvglo:
      case ttglo:
      case tsglo:
      case tbglo:
      case twglo:
      case tpigl:
      case tpwgl:
      case tpbgl:
      case tpcgl:
      case tpsgl:
        include_flag = show_global;
        break;
      case tcloc:
      case tvloc:
      case ttloc:
      case tsloc:
      case tbloc:
      case twloc:
      case tpilo:
      case tpwlo:
      case tpblo:
      case tpclo:
      case tpslo:
        if (o[n].scope) {
          if (o[n].scope == current_scope)
            include_flag = show_private;
          else
            include_flag = 0;
        } else
          include_flag = show_local;
        break;
      default:
        include_flag = 0;
        break;
      }
      if (include_flag) {
        if (!newest || (newest && strcmp(vnom + o[n].name, vnom + o[newest].name) == -1)) {
          newest = n;
        }
      }
    }
    if (newest) {
      var[num_var].object = newest;
      var[num_var].tab = 0;
      var[num_var].miembro = 0;
      used[newest] = 1;

      switch (o[newest].type) {
      case tpigl:
      case tpwgl:
      case tpbgl:
      case tpcgl:
      case tpsgl:
      case tpilo:
      case tpwlo:
      case tpblo:
      case tpclo:
      case tpslo:
        var[num_var].indice = -1;
        break;
      default:
        var[num_var].indice = 0;
        break;
      }
      num_var++;

      if (o[newest].type == tsglo || o[newest].type == tsloc) {
        member = newest + 1;
        include_members(num_var, 1, var[num_var - 1].indice);
      }
      if (o[newest].type == tpsgl || o[newest].type == tpslo) {
        member = o[newest].v1 + 1;
        include_members(num_var, 1, var[num_var - 1].indice);
      }
    }
  } while (newest);
}

void exclude_members(int parent, int level, int index) {
  int m;
  m = o[member].miembro;
  while (m == o[member].miembro) {
    if (index == -1)
      var[parent].object = -member;
    else
      var[parent].object = member;
    switch (o[member].type) {
    case tpigl:
    case tpwgl:
    case tpbgl:
    case tpcgl:
    case tpsgl:
    case tpilo:
    case tpwlo:
    case tpblo:
    case tpclo:
    case tpslo:
      var[parent].indice = -1;
      break;
    }
    parent++;
    if (o[member].type == tsglo || o[member].type == tsloc) {
      member++;
      exclude_members(parent, level + 1, index);
    } else
      member++;
  }
}

void include_members(int parent, int level, int index) {
  int m;

  m = o[member].miembro;

  while (m == o[member].miembro) {
    if (index == -1)
      var[num_var].object = -member;
    else
      var[num_var].object = member;
    var[num_var].tab = level;
    var[num_var].miembro = parent;
    used[member] = 1;

    switch (o[member].type) {
    case tpigl:
    case tpwgl:
    case tpbgl:
    case tpcgl:
    case tpsgl:
    case tpilo:
    case tpwlo:
    case tpblo:
    case tpclo:
    case tpslo:
      var[num_var].indice = -1;
      break;
    default:
      var[num_var].indice = 0;
      break;
    }
    num_var++;

    if (o[member].type == tsglo || o[member].type == tsloc) {
      member++;
      include_members(num_var, level + 1, index);
    } else
      member++;
  }
}

//----------------------------------------------------------------------------
//  Inspect window
//----------------------------------------------------------------------------

int _var_ini = -1, _var_select = -1;

void change0(void);
void changestring0(void);

void inspect1(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(ptr, w, h, 4, h - 14, 0, text[38], c3);

  wrectangle(ptr, w, h, c0, 3, 19, 128 + 32 + 64, 83); // Process listbox bounds
  wrectangle(ptr, w, h, c0, 122 + 32 + 64, 19, 9, 83);
  wrectangle(ptr, w, h, c0, 122 + 32 + 64, 27, 9, 83 - 16);

  wput(ptr, w, h, 123 + 32 + 64, 20, -39); // Up/down buttons (pressed: 41,42)
  wput(ptr, w, h, 123 + 32 + 64, 134 - 40, -40);

  create_variable_list();

  if (_var_ini != -1 && num_var > 1) {
    var_ini = _var_ini;
    var_select = _var_select;

    if (var_ini + 10 > num_var && var_ini)
      var_ini = num_var - 10;
    if (var_ini < 0)
      var_ini = 0;
    if (var_select >= num_var)
      var_select = num_var - 1;
    if (var_select < var_ini)
      var_select = var_ini;
    if (var_select >= var_ini + 10)
      var_select = var_ini + 9;

  } else {
    var_ini = 0;
    var_select = 0;
  }

  paint_var_list();

  paint_segment2();
}

void paint_segment2(void) {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int min = 27, max = 89, n;
  float x;

  wbox(ptr, w, h, c2, 123 + 32 + 64, 28, 7, 65); // Clear the slider bar

  if (num_var <= 1)
    n = min;
  else {
    x = (float)var_select / (float)(num_var - 1);
    n = min * (1 - x) + max * x;
  }

  wrectangle(ptr, w, h, c0, 122 + 32 + 64, n, 9, 5);
  wput(ptr, w, h, 123 + 32 + 64, n + 1, -43);
}

void inspect2(void) {
  int n, tipo;
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  float x;

  _process_items();
  if (scan_code == 80 && var_select + 1 < num_var) {
    if (var_ini + 10 == ++var_select)
      var_ini++;
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 72 && var_select) {
    if (var_ini == var_select--)
      var_ini--;
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 81) {
    for (n = 0; n < 10; n++)
      if (var_select + 1 < num_var) {
        if (var_ini + 10 == ++var_select)
          var_ini++;
      }
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }
  if (scan_code == 73) {
    for (n = 0; n < 10; n++)
      if (var_select) {
        if (var_ini == var_select--)
          var_ini--;
      }
    paint_var_list();
    flush_buffer();
    v.redraw = 1;
  }

  if (wmouse_in(3, 21, 128 + 32 + 64 - 9, 80) && (mouse_b & 1)) {
    n = var_ini + (wmouse_y - 21) / 8;
    if (n < num_var) {
      var_select = n;
      paint_var_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122 + 32 + 64, 19, 9, 9)) {
    if (mouse_b & 1) {
      if (button == 0) {
        wput(ptr, w, h, 123 + 32 + 64, 20, -41);
        button = 1;
        if (var_select) {
          if (var_ini == var_select--)
            var_ini--;
          paint_var_list();
          v.redraw = 1;
        }
      }
    } else if (button == 1) {
      wput(ptr, w, h, 123 + 32 + 64, 20, -39);
      button = 0;
      v.redraw = 1;
    }
    mouse_graf = 7;
  } else if (button == 1) {
    wput(ptr, w, h, 123 + 32 + 64, 20, -39);
    button = 0;
    v.redraw = 1;
  }

  if (wmouse_in(123 + 32 + 64, 28, 7, 65)) {
    mouse_graf = 13;
    if (num_var > 1 && (mouse_b & 1)) {
      x = (float)(wmouse_y - 28) / 64.0;
      var_select = x * (num_var - 1);
      if (var_select < var_ini)
        var_ini = var_select;
      if (var_select >= var_ini + 10)
        var_ini = var_select - 9;
      paint_var_list();
      v.redraw = 1;
    }
  }

  if (wmouse_in(122 + 32 + 64, 93, 9, 9)) {
    if (mouse_b & 1) {
      if (button == 0) {
        wput(ptr, w, h, 123 + 32 + 64, 94, -42);
        button = 2;
        if (var_select + 1 < num_var) {
          if (var_ini + 10 == ++var_select)
            var_ini++;
          paint_var_list();
          v.redraw = 1;
        }
      }
    } else if (button == 2) {
      wput(ptr, w, h, 123 + 32 + 64, 94, -40);
      button = 0;
      v.redraw = 1;
    }
    mouse_graf = 9;
  } else if (button == 2) {
    wput(ptr, w, h, 123 + 32 + 64, 94, -40);
    button = 0;
    v.redraw = 1;
  }

  if (num_var) {
    if (var[var_select].object > 0) {
      if (scan_code == 75)
        goto dec_index;
      if (scan_code == 77)
        goto inc_index;
    }
  }

  if (scan_code == 71) { // Home
    if (var[var_select].object > 0)
      switch (o[var[var_select].object].type) {
      case tpigl:
      case tpilo:
      case tpwgl:
      case tpwlo:
      case tpbgl:
      case tpblo:
      case tpcgl:
      case tpclo:
        var[var_select].indice = -1;
        paint_var_list();
        v.redraw = 1;
        break;
      case tpsgl:
      case tpslo:
        if (var[var_select].indice > -1) {
          var[var_select].indice = -1;
          if (!var[var_select].miembro) {
            member = o[var[var_select].object].v1 + 1;
            exclude_members(var_select + 1, 1, var[var_select].indice);
            if (var_ini + 10 > num_var && var_ini)
              var_ini = num_var - 10;
            if (var_ini < 0)
              var_ini = 0;
            paint_segment2();
          }
          paint_var_list();
          paint_segment2();
          v.redraw = 1;
        }
        break;
      default:
        var[var_select].indice = 0;
        paint_var_list();
        v.redraw = 1;
        break;
      }
  }

  if (scan_code == 79) { // End
    if (var[var_select].object > 0)
      switch (o[var[var_select].object].type) {
      case tpigl:
      case tpilo:
      case tpwgl:
      case tpwlo:
      case tpbgl:
      case tpblo:
      case tpcgl:
      case tpclo:
        if (var[var_select].indice == -1) {
          tipo = o[var[var_select].object].type;
          switch (o[var[var_select].object].type) {
          case tpigl:
          case tpwgl:
          case tpbgl:
          case tpcgl:
          case tpsgl:
            o[var[var_select].object].type = tvglo;
            break;
          case tpilo:
          case tpwlo:
          case tpblo:
          case tpclo:
          case tpslo:
            o[var[var_select].object].type = tvloc;
            break;
          }
          n = memo(get_offset(var_select));
          o[var[var_select].object].type = tipo;
          if (!n)
            break; // Not if the pointer is NULL
        }
        if (o[var[var_select].object].v2 == -1) {
          var[var_select].indice = 0;
          break;
        }
        /* fall through */
      case ttglo:
      case ttloc:
      case tbglo:
      case tbloc:
      case twglo:
      case twloc:
        var[var_select].indice = o[var[var_select].object].v2 + 1;
        if (o[var[var_select].object].v3 > -1)
          var[var_select].indice *= o[var[var_select].object].v3 + 1;
        if (o[var[var_select].object].v4 > -1)
          var[var_select].indice *= o[var[var_select].object].v4 + 1;
        var[var_select].indice--;
        break;
      case tpsgl:
      case tpslo:
        if (var[var_select].indice == -1) {
          if (memo(get_offset(var_select)) == 0)
            break; // Don't show the struct if the pointer is NULL
          var[var_select].indice = 0;
          if (!var[var_select].miembro) {
            member = o[var[var_select].object].v1 + 1;
            exclude_members(var_select + 1, 1, var[var_select].indice);
            paint_segment2();
          }
          paint_var_list();
          break;
        }
        if (o[var[var_select].object].v3 == -1)
          var[var_select].indice = 0;
        else
          var[var_select].indice = o[var[var_select].object].v2 - 1;
        if (!var[var_select].miembro) {
          member = o[var[var_select].object].v1 + 1;
          exclude_members(var_select + 1, 1, var[var_select].indice);
          paint_segment2();
        }
        paint_var_list();
        break;
      case tsglo:
      case tsloc:
        var[var_select].indice = o[var[var_select].object].v2 - 1;
        break;
      }
    paint_var_list();
    v.redraw = 1;
  }

  if (num_var == 0 && v.active_item < 9)
    v.active_item = -1;
  if (var[var_select].object < 0 && v.active_item < 9)
    v.active_item = -1;

  switch (v.active_item) {
  case 0: // Change
    if (o[var[var_select].object].type != tcons) {
      if (o[var[var_select].object].type == tsglo || o[var[var_select].object].type == tsloc ||
          ((o[var[var_select].object].type == tpsgl || o[var[var_select].object].type == tpslo) &&
           var[var_select].indice >= 0)) {
        v_text = (char *)text[39];
        show_dialog(err0);
      } else if (o[var[var_select].object].type == tcglo ||
                 o[var[var_select].object].type == tcloc) {
        show_dialog(changestring0);
        paint_var_list();
        v.redraw = 1;
      } else {
        show_dialog(change0);
        paint_var_list();
        v.redraw = 1;
      }
    } else {
      v_text = (char *)text[40];
      show_dialog(err0);
    }
    break;
  case 1: // Index--
dec_index:
    switch (o[var[var_select].object].type) {
    case ttglo:
    case ttloc:
    case tsglo:
    case tsloc:
    case tbglo:
    case tbloc:
    case twglo:
    case twloc:
      if (var[var_select].indice > 0)
        var[var_select].indice--;
      paint_var_list();
      v.redraw = 1;
      break;
    case tpigl:
    case tpilo:
    case tpwgl:
    case tpwlo:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      if (var[var_select].indice > -1)
        var[var_select].indice--;
      paint_var_list();
      v.redraw = 1;
      break;
    case tpsgl:
    case tpslo:
      if (var[var_select].indice > -1) {
        var[var_select].indice--;
        if (!var[var_select].miembro) {
          member = o[var[var_select].object].v1 + 1;
          exclude_members(var_select + 1, 1, var[var_select].indice);
          paint_segment2();
        }
        paint_var_list();
        v.redraw = 1;
      }
      break;
    }
    break;
  case 2: // Index++
inc_index:
    switch (o[var[var_select].object].type) {
    case tpigl:
    case tpilo:
    case tpwgl:
    case tpwlo:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      if (var[var_select].indice == -1) {
        tipo = o[var[var_select].object].type;
        switch (o[var[var_select].object].type) {
        case tpigl:
        case tpwgl:
        case tpbgl:
        case tpcgl:
        case tpsgl:
          o[var[var_select].object].type = tvglo;
          break;
        case tpilo:
        case tpwlo:
        case tpblo:
        case tpclo:
        case tpslo:
          o[var[var_select].object].type = tvloc;
          break;
        }
        n = memo(get_offset(var_select));
        o[var[var_select].object].type = tipo;
        if (!n)
          break; // Not if the pointer is NULL
      }
      if (o[var[var_select].object].v2 == -1) {
        var[var_select].indice++;
        paint_var_list();
        v.redraw = 1;
        break;
      }
      /* fall through */
    case ttglo:
    case ttloc:
    case tbglo:
    case tbloc:
    case twglo:
    case twloc:
      n = o[var[var_select].object].v2 + 1;
      if (o[var[var_select].object].v3 > -1)
        n *= o[var[var_select].object].v3 + 1;
      if (o[var[var_select].object].v4 > -1)
        n *= o[var[var_select].object].v4 + 1;
      if (var[var_select].indice < n - 1)
        var[var_select].indice++;
      paint_var_list();
      v.redraw = 1;
      break;
    case tpsgl:
    case tpslo:
      if (var[var_select].indice == -1) {
        if (memo(get_offset(var_select)) == 0)
          break; // Don't show the struct if the pointer is NULL
      }
      if (o[var[var_select].object].v3 == -1)
        var[var_select].indice++;
      else if (var[var_select].indice < o[var[var_select].object].v2 - 1)
        var[var_select].indice++;
      if (!var[var_select].miembro) {
        member = o[var[var_select].object].v1 + 1;
        exclude_members(var_select + 1, 1, var[var_select].indice);
        paint_segment2();
      }
      paint_var_list();
      v.redraw = 1;
      break;
    case tsglo:
    case tsloc:
      if (var[var_select].indice < o[var[var_select].object].v2 - 1)
        var[var_select].indice++;
      paint_var_list();
      v.redraw = 1;
      break;
    }
    break;
  case 3: // View as angle
    if (visor[var[var_select].object] == 4)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 4;
    paint_var_list();
    v.redraw = 1;
    break;
  case 4: // View as process
    if (visor[var[var_select].object] == 3)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 3;
    paint_var_list();
    v.redraw = 1;
    break;
  case 5: // View as text
    if (visor[var[var_select].object] == 2)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 2;
    paint_var_list();
    v.redraw = 1;
    break;
  case 6: // View as boolean
    if (visor[var[var_select].object] == 1)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 1;
    paint_var_list();
    v.redraw = 1;
    break;
  case 7: // View as Hex
    if (visor[var[var_select].object] == 5)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 5;
    paint_var_list();
    v.redraw = 1;
    break;
  case 8: // View as Bin
    if (visor[var[var_select].object] == 6)
      visor[var[var_select].object] = 0;
    else
      visor[var[var_select].object] = 6;
    paint_var_list();
    v.redraw = 1;
    break;
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14: // Flags
    create_variable_list();
    var_ini = 0;
    var_select = 0;
    paint_var_list();
    paint_segment2();
    v.redraw = 1;
    break;
  }
}

void inspect3(void) {
  _var_ini = var_ini;
  _var_select = var_select;
}

char titulo[256];

void inspect0(void) {
  int n, x = 50;

  v.type = WIN_DIALOG;
  v.title = (byte *)titulo;
  div_strcpy(titulo, sizeof(titulo), (char *)text[41]);

  for (n = 0; n < num_objects; n++)
    if (o[n].type == tproc && o[n].v0 == mem[ids[ids_select] + _Bloque])
      break;
  if (n < num_objects)
    div_strcat(titulo, sizeof(titulo), vnom + o[n].name);
  else
    div_strcat(titulo, sizeof(titulo), (char *)text[9]);

  div_strcat(titulo, sizeof(titulo), "(");
  itoa(ids[ids_select], titulo + strlen(titulo), 10);
  div_strcat(titulo, sizeof(titulo), ")");

  v.w = 209 + 64;
  v.h = 121;
  v.paint_handler = inspect1;
  v.click_handler = inspect2;
  v.close_handler = inspect3;

  _button(text[42], 185 + 64, 72, 1);

  _button(text[43], 169 + 64, 92, 0);
  _button(text[44], 190 + 64, 92, 0);

  _button(text[45], x, v.h - 14, 0);
  x += text_len(text[45] + 1) + 8;
  _button(text[46], x, v.h - 14, 0);
  x += text_len(text[46] + 1) + 8;
  _button(text[47], x, v.h - 14, 0);
  x += text_len(text[47] + 1) + 8;
  _button(text[48], x, v.h - 14, 0);
  x += text_len(text[48] + 1) + 8;
  _button(text[49], x, v.h - 14, 0);
  x += text_len(text[49] + 1) + 8;
  _button(text[50], x, v.h - 14, 0);

  _flag(text[51], 4, 11, &pre_defined);
  _flag(text[52], 32 + text_len(text[51]), 11, &user_defined);

  _flag((byte *)"Const", v.w - 44, 19, &show_const);
  _flag((byte *)"Global", v.w - 44, 19 + 10, &show_global);
  _flag((byte *)"Local", v.w - 44, 19 + 20, &show_local);
  _flag((byte *)"Private", v.w - 44, 19 + 30, &show_private);
}

//----------------------------------------------------------------------------
//  Draw the variable list inside the listbox
//----------------------------------------------------------------------------

// Functions to prevent page faults

byte *clamp_byte(byte *dir) {
  uintptr_t offset;
  offset = ((uintptr_t)dir - (uintptr_t)mem) / 4;
  if (validate_address(offset))
    return (dir);
  else
    return ((byte *)mem);
}

word *clamp_word(word *dir) {
  uintptr_t offset;
  offset = ((uintptr_t)dir - (uintptr_t)mem) / 4;
  if (validate_address(offset))
    return (dir);
  else
    return ((word *)mem);
}

int memo(int dir) {
  return (mem[validate_address(dir)]);
}

void paint_var_list(void) {
  char msg[512];
  byte *ptr = v.ptr, x;
  int w = v.w / big2, h = v.h / big2;
  int m, tipo, fin;

  wbox(ptr, w, h, c1, 4, 20, 150 + 64, 81); // Process listbox fill

  scan_code = 0;
  ascii = 0;
  fin = var_ini + 10;

  for (m = var_ini; m < fin && m < num_var; m++) {
    if (m == var_select) {
      wbox(ptr, w, h, c01, 4, 20 + (m - var_ini) * 8, 150 + 64, 9); // Process listbox fill
      x = c4;
    } else
      x = c3;
    if (var[m].object < 0) {
      switch (o[abs(var[m].object)].type) {
      case tcons:
        div_strcpy(msg, sizeof(msg), "CONST ");
        break;
      case tvglo:
      case tvloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        break;
      case tcglo:
      case tcloc:
        div_strcpy(msg, sizeof(msg), "STRING ");
        break;
      case tbglo:
      case tbloc:
        div_strcpy(msg, sizeof(msg), "BYTE ");
        break;
      case twglo:
      case twloc:
        div_strcpy(msg, sizeof(msg), "WORD ");
        break;
      case ttglo:
      case ttloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        break;
      case tpigl:
      case tpilo:
        div_strcpy(msg, sizeof(msg), "INT POINTER ");
        break;
      case tpwgl:
      case tpwlo:
        div_strcpy(msg, sizeof(msg), "WORD POINTER ");
        break;
      case tpbgl:
      case tpblo:
        div_strcpy(msg, sizeof(msg), "BYTE POINTER ");
        break;
      case tpsgl:
      case tpslo:
        div_strcpy(msg, sizeof(msg), "STRUCT POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[o[abs(var[m].object)].v1].name);
        div_strcat(msg, sizeof(msg), vnom + o[o[abs(var[m].object)].v1].name);
        strupr(msg);
        div_strcat(msg, sizeof(msg), " ");
        break;
      case tpcgl:
      case tpclo:
        div_strcpy(msg, sizeof(msg), "STRING POINTER ");
        break;
      case tsglo:
      case tsloc:
        div_strcpy(msg, sizeof(msg), "STRUCT ");
        break;
      }
      div_strcat(msg, sizeof(msg), vnom + o[abs(var[m].object)].name);
      x = c2;
    } else
      switch (o[var[m].object].type) {
      case tcons:
        div_strcpy(msg, sizeof(msg), "CONST ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        visualize(o[var[m].object].v0, var[m].object, msg);
        break;
      case tvglo:
      case tvloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        visualize(memo(get_offset(m)), var[m].object, msg);
        break;
      case tcglo:
      case tcloc:
        div_strcpy(msg, sizeof(msg), "STRING ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        visualize(get_offset(m), var[m].object, msg);
        break;
      case tbglo:
      case tbloc:
        div_strcpy(msg, sizeof(msg), "BYTE ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v3 == -1 && o[var[m].object].v2 == 0)
          goto show_variable;
        else
          goto show_tabla;
      case twglo:
      case twloc:
        div_strcpy(msg, sizeof(msg), "WORD ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v3 == -1 && o[var[m].object].v2 == 0)
          goto show_variable;
        else
          goto show_tabla;
      case ttglo:
      case ttloc:
        div_strcpy(msg, sizeof(msg), "INT ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v3 == -1 && o[var[m].object].v2 == 0)
          goto show_variable;
        else
          goto show_tabla;
      case tpigl:
      case tpilo:
        div_strcpy(msg, sizeof(msg), "INT POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
      case tpwgl:
      case tpwlo:
        div_strcpy(msg, sizeof(msg), "WORD POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
      case tpbgl:
      case tpblo:
        div_strcpy(msg, sizeof(msg), "BYTE POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
      case tpsgl:
      case tpslo:
        div_strcpy(msg, sizeof(msg), "STRUCT POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[o[var[m].object].v1].name);
        strupr(msg);
        div_strcat(msg, sizeof(msg), " ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_indice;
      case tpcgl:
      case tpclo:
        div_strcpy(msg, sizeof(msg), "STRING POINTER ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (var[m].indice == -1)
          goto show_variable;
        else
          goto show_tabla;
show_tabla:
        div_strcat(msg, sizeof(msg), "[");
        if (o[var[m].object].v3 == -1) { // 1-dimensional array
          itoa(var[m].indice, msg + strlen(msg), 10);
        } else if (o[var[m].object].v4 == -1) { // 2-dimensional array
          itoa(var[m].indice % (o[var[m].object].v2 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa(var[m].indice / (o[var[m].object].v2 + 1), msg + strlen(msg), 10);
        } else { // 3-dimensional array
          itoa(var[m].indice % (o[var[m].object].v2 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v2 + 1)) % (o[var[m].object].v3 + 1),
               msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v2 + 1)) / (o[var[m].object].v3 + 1),
               msg + strlen(msg), 10);
        }
        div_strcat(msg, sizeof(msg), "]");
show_variable:
        if (var[m].indice == -1) {
          tipo = o[var[m].object].type;
          switch (o[var[m].object].type) {
          case tpigl:
          case tpwgl:
          case tpbgl:
          case tpcgl:
          case tpsgl:
            o[var[m].object].type = tvglo;
            break;
          case tpilo:
          case tpwlo:
          case tpblo:
          case tpclo:
          case tpslo:
            o[var[m].object].type = tvloc;
            break;
          }
          if (!memo(get_offset(m)))
            div_strcat(msg, sizeof(msg), " = NULL");
          else
            visualize(memo(get_offset(m)), var[m].object, msg);
          o[var[m].object].type = tipo;
        } else if (o[var[m].object].type == ttglo || o[var[m].object].type == ttloc ||
                   o[var[m].object].type == tpigl || o[var[m].object].type == tpilo) {
          visualize(memo(get_offset(m)), var[m].object, msg);
        } else if (o[var[m].object].type == twglo || o[var[m].object].type == twloc ||
                   o[var[m].object].type == tpwgl || o[var[m].object].type == tpwlo) {
          visualize(*get_offset_word(m), var[m].object, msg);
        } else if (o[var[m].object].type == tbglo || o[var[m].object].type == tbloc ||
                   o[var[m].object].type == tpbgl || o[var[m].object].type == tpblo) {
          visualize(*get_offset_byte(m), var[m].object, msg);
        } else if (o[var[m].object].type == tpcgl || o[var[m].object].type == tpclo) {
          visualize(*get_offset_byte(m), var[m].object, msg);
        }
        break;
      case tsglo:
      case tsloc:
        div_strcpy(msg, sizeof(msg), "STRUCT ");
        div_strcat(msg, sizeof(msg), vnom + o[var[m].object].name);
        if (o[var[m].object].v4 == -1 && o[var[m].object].v3 == 0)
          break;
show_indice:
        div_strcat(msg, sizeof(msg), "[");
        if (o[var[m].object].v4 == -1) { // 1-dimensional struct
          itoa(var[m].indice, msg + strlen(msg), 10);
        } else if (o[var[m].object].v5 == -1) { // 2-dimensional struct
          itoa(var[m].indice % (o[var[m].object].v3 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa(var[m].indice / (o[var[m].object].v3 + 1), msg + strlen(msg), 10);
        } else { // 3-dimensional struct
          itoa(var[m].indice % (o[var[m].object].v3 + 1), msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v3 + 1)) % (o[var[m].object].v4 + 1),
               msg + strlen(msg), 10);
          div_strcat(msg, sizeof(msg), ",");
          itoa((var[m].indice / (o[var[m].object].v3 + 1)) / (o[var[m].object].v4 + 1),
               msg + strlen(msg), 10);
        }
        div_strcat(msg, sizeof(msg), "]");
        break;
      }
    wwrite_in_box(ptr, w, 144 + 9 + 64, h, 10 + var[m].tab * 8, 21 + (m - var_ini) * 8, 0,
                  (byte *)msg, x);
  }

  paint_segment2();
}

//----------------------------------------------------------------------------
//  Display an object according to its visor[] mode
//----------------------------------------------------------------------------

void visualize(int value, int object, char *str) {
  int n;
  div_strcat(str, 512, " = ");
  switch (visor[object]) {
  case 0: // Integer
    itoa(value, str + strlen(str), 10);
    break;
  case 1: // Boolean
    if (value & 1)
      div_strcat(str, 512, "TRUE");
    else
      div_strcat(str, 512, "FALSE");
    break;
  case 2: // Text
    if (value >= 256) {
      div_strcat(str, 512, "\"");
      if (strlen((char *)&mem[value]) + strlen(str) < 511) {
        div_strcat(str, 512, (char *)&mem[value]);
      } else {
        str[strlen(str) + 256] = 0;
        memcpy(str + strlen(str), (char *)&mem[value], 256);
      }
      div_strcat(str, 512, "\"");
    } else if (value >= 0 && value <= 255) {
      if (value == 0)
        div_strcat(str, 512, "<EOL>");
      else
        div_snprintf(str + strlen(str), 512 - strlen(str), "\"%c\"", value);
    } else
      itoa(value, str + strlen(str), 10);
    break;
  case 3: // Process
    if (value == id_init) {
      div_strcat(str, 512, "div_main()");
    } else if (value) {
      for (n = 0; n < iids; n++)
        if (ids[n] == value)
          break;
      if (n < iids) {
        for (n = 0; n < num_objects; n++)
          if (o[n].type == tproc && o[n].v0 == mem[value + _Bloque])
            break;
        if (n < num_objects) {
          div_strcat(str, 512, (char *)vnom + o[n].name);
          div_strcat(str, 512, "(");
          n = 1;
        } else
          n = 0;
      } else
        n = 0;
      itoa(value, str + strlen(str), 10);
      if (n)
        div_strcat(str, 512, ")");
    } else
      div_strcat(str, 512, (char *)text[53]);
    break;
  case 4: // Angle
    while (value > pi)
      value -= 2 * pi;
    while (value < -pi)
      value += 2 * pi;
    div_snprintf(str + strlen(str), 512 - strlen(str), "%.3f\xa7, %.4f rad", (float)value / 1000.0,
                 (float)value / radian);
    break;
  case 5: // Hex
    div_snprintf(str + strlen(str), 512 - strlen(str), "0x%X", value);
    break;
  case 6: // Bin
    for (n = 0; n < 32; n++)
      if (value & 0x80000000)
        break;
      else
        value <<= 1;
    if (n < 32) {
      for (; n < 32; n++) {
        if (value & 0x80000000)
          div_strcat(str, 512, "1");
        else
          div_strcat(str, 512, "0");
        value <<= 1;
      }
      div_strcat(str, 512, " Bin");
    } else {
      div_strcat(str, 512, "0 Bin");
    }
    break;
  }
}

//----------------------------------------------------------------------------
//  Get the offset within mem[] for a variable in the list
//----------------------------------------------------------------------------

int get_offset(int m) {
  return (validate_address(_get_offset(m)));
}

int _get_offset(int m) {
  if (var[m].miembro == 0)
    switch (o[var[m].object].type) {
    case tvglo:
      return (o[var[m].object].v0);
    case tcglo:
      return (o[var[m].object].v0);
    case ttglo:
      if (var[m].indice >= 0)
        return (o[var[m].object].v0 + var[m].indice);
      /* fall through */
    case tpigl:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0) + var[m].indice);
      else
        return (o[var[m].object].v0);
    case tbglo:
    case tbloc:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      return (1);
    case twglo:
    case twloc:
    case tpwgl:
    case tpwlo:
      return (2);
    case tsglo:
      return (o[var[m].object].v0 + var[m].indice * o[var[m].object].v1);
    case tpsgl:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0) + var[m].indice * o[o[var[m].object].v1].v1);
      else
        return (o[var[m].object].v0);
    case tvloc:
      return (o[var[m].object].v0 + ids[ids_select]);
    case tcloc:
      return (o[var[m].object].v0 + ids[ids_select]);
    case ttloc:
      return (o[var[m].object].v0 + ids[ids_select] + var[m].indice);
    case tpilo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + ids[ids_select]) + var[m].indice);
      else
        return (o[var[m].object].v0 + ids[ids_select]);
    case tsloc:
      return (o[var[m].object].v0 + var[m].indice * o[var[m].object].v1 + ids[ids_select]);
    case tpslo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + ids[ids_select]) +
                var[m].indice * o[o[var[m].object].v1].v1);
      else
        return (o[var[m].object].v0 + ids[ids_select]);
    }
  else {
    switch (o[var[m].object].type) {
    case tvglo:
    case tvloc:
      return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    case tcglo:
    case tcloc:
      return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    case ttglo:
    case ttloc:
      return (o[var[m].object].v0 + get_offset(var[m].miembro - 1) + var[m].indice);
    case tpigl:
    case tpilo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) + var[m].indice);
      else
        return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    case tbglo:
    case tbloc:
    case tpbgl:
    case tpblo:
    case tpcgl:
    case tpclo:
      return (1);
    case twglo:
    case twloc:
    case tpwgl:
    case tpwlo:
      return (2);
    case tsglo:
    case tsloc:
      return (o[var[m].object].v0 + var[m].indice * o[var[m].object].v1 +
              get_offset(var[m].miembro - 1));
    case tpsgl:
    case tpslo:
      if (var[m].indice >= 0)
        return (memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) +
                var[m].indice * o[o[var[m].object].v1].v1);
      else
        return (o[var[m].object].v0 + get_offset(var[m].miembro - 1));
    }
  }
  return (0);
}

byte *_get_offset_byte(int m) {
  if (var[m].miembro == 0)
    switch (o[var[m].object].type) {
    case tbglo:
      return (&memb[o[var[m].object].v0 * 4 + var[m].indice]);
    case tbloc:
      return (&memb[o[var[m].object].v0 * 4 + var[m].indice + ids[ids_select] * 4]);
    case tpbgl:
    case tpcgl:
      if (var[m].indice >= 0)
        return (&memb[memo(o[var[m].object].v0) * 4 + var[m].indice]);
      else
        return (&memb[o[var[m].object].v0 * 4]);
    case tpblo:
    case tpclo:
      if (var[m].indice >= 0)
        return (&memb[memo(o[var[m].object].v0 + ids[ids_select]) * 4 + var[m].indice]);
      else
        return (&memb[o[var[m].object].v0 * 4 + ids[ids_select] * 4]);
    }
  else
    switch (o[var[m].object].type) {
    case tbglo:
    case tbloc:
    case tpcgl:
    case tpclo:
      return (&memb[o[var[m].object].v0 * 4 + var[m].indice + get_offset(var[m].miembro - 1) * 4]);
    case tpbgl:
    case tpblo:
      if (var[m].indice >= 0)
        return (
            &memb[memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) * 4 + var[m].indice]);
      else
        return (&memb[o[var[m].object].v0 * 4 + get_offset(var[m].miembro - 1) * 4]);
    }
  return (0);
}

byte *get_offset_byte(int m) {
  return (clamp_byte(_get_offset_byte(m)));
}

word *_get_offset_word(int m) {
  if (var[m].miembro == 0)
    switch (o[var[m].object].type) {
    case twglo:
      return ((word *)&memb[o[var[m].object].v0 * 4 + var[m].indice * 2]);
    case twloc:
      return ((word *)&memb[o[var[m].object].v0 * 4 + var[m].indice * 2 + ids[ids_select] * 4]);
    case tpwgl:
      if (var[m].indice >= 0)
        return ((word *)&memb[memo(o[var[m].object].v0) * 4 + var[m].indice * 2]);
      else
        return ((word *)&memb[o[var[m].object].v0 * 4]);
    case tpwlo:
      if (var[m].indice >= 0)
        return ((word *)&memb[memo(o[var[m].object].v0 + ids[ids_select]) * 4 + var[m].indice * 2]);
      else
        return ((word *)&memb[o[var[m].object].v0 * 4 + ids[ids_select] * 4]);
    }
  else
    switch (o[var[m].object].type) {
    case twglo:
    case twloc:
      return ((word *)&memb[o[var[m].object].v0 * 4 + var[m].indice * 2 +
                            get_offset(var[m].miembro - 1) * 4]);
    case tpwgl:
    case tpwlo:
      if (var[m].indice >= 0)
        return ((word *)&memb[memo(o[var[m].object].v0 + get_offset(var[m].miembro - 1)) * 4 +
                              var[m].indice * 2]);
      else
        return ((word *)&memb[o[var[m].object].v0 * 4 + get_offset(var[m].miembro - 1) * 4]);
    }
  return (0);
}

word *get_offset_word(int m) {
  return (clamp_word(_get_offset_word(m)));
}

//-----------------------------------------------------------------------------
//      Change the value of a variable (in Inspect)
//-----------------------------------------------------------------------------

#define y_bt 34
char search_text[32];
int value;

void change1(void) {
  _show_items();
}

void change2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    if (get_offset(var_select) == 1) {
      if (atoi(search_text) < 0 || atoi(search_text) > 255) {
        v_text = (char *)text[54];
        show_dialog(err0);
      } else {
        *get_offset_byte(var_select) = (byte)atoi(search_text);
        end_dialog = 1;
      }
    } else if (get_offset(var_select) == 2) {
      if (atoi(search_text) < 0 || atoi(search_text) > 65535) {
        v_text = (char *)text[55];
        show_dialog(err0);
      } else {
        *get_offset_word(var_select) = (word)atoi(search_text);
        end_dialog = 1;
      }
    } else {
      mem[get_offset(var_select)] = atoi(search_text);
      end_dialog = 1;
    }
    break;
  case 2:
    end_dialog = 1;
    break;
  }
}

void change0(void) {
  v.type = WIN_DIALOG;
  v.title = text[56];
  v.w = 126;
  v.h = 14 + y_bt;
  v.paint_handler = change1;
  v.click_handler = change2;

  if (get_offset(var_select) == 1) {
    value = *get_offset_byte(var_select);
  } else if (get_offset(var_select) == 2) {
    value = *get_offset_word(var_select);
  } else {
    value = memo(get_offset(var_select));
  }

  itoa(value, search_text, 10);
  _get((byte *)text[57], 4, 11, v.w - 8, (byte *)search_text, 32, 0, 0);
  _button(text[7], 7, y_bt, 0);
  _button(text[58], v.w - 8, y_bt, 2);
}

//-----------------------------------------------------------------------------
//      Change the value of a string (in Inspect)
//-----------------------------------------------------------------------------

char enterstring[256];

void changestring1(void) {
  _show_items();
}

void changestring2(void) {
  _process_items();
  switch (v.active_item) {
  case 1:
    if (strlen(enterstring) <= o[var[var_select].object].v1 + 1)
      div_strcpy((char *)&mem[get_offset(var_select)], o[var[var_select].object].v1 + 1,
                 enterstring);
    else {
      v_text = (char *)text[59];
      show_dialog(err0);
    }
    end_dialog = 1;
    break;
  case 2:
    end_dialog = 1;
    break;
  }
}

void changestring0(void) {
  v.type = WIN_DIALOG;
  v.title = text[60];
  v.w = 226;
  v.h = 14 + y_bt;
  v.paint_handler = changestring1;
  v.click_handler = changestring2;
  if (strlen((char *)&mem[get_offset(var_select)]) < 256)
    div_strcpy(enterstring, sizeof(enterstring), (char *)&mem[get_offset(var_select)]);
  else
    div_strcpy(enterstring, sizeof(enterstring), "");
  _get((byte *)text[61], 4, 11, v.w - 8, (byte *)enterstring, 256, 0, 0);
  _button(text[7], 7, y_bt, 0);
  _button(text[58], v.w - 8, y_bt, 2);
}
