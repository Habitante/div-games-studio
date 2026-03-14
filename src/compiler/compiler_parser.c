
//-----------------------------------------------------------------------------
//      DIV Compiler - Parser
//
//      Top-level parsing (declarations, structs, statements).
//      Extracted from compiler.c during Phase 2C-3.
//-----------------------------------------------------------------------------

#include "compiler_internal.h"

//-----------------------------------------------------------------------------
//      Pointer declaration analysis (int, word or byte)
//-----------------------------------------------------------------------------

struct object *analyze_pointer(int tipo, int offset) {
  struct object *ob;
  int len1, len2, len3;

  if (current_token != p_id)
    c_error(1, 148);
  ob = o;
  if ((*ob).type != tnone) {
    if (in_params == -1 && (*ob).param == 1 && (*ob).scope == current_scope) {
      if ((*ob).type == tipo) { // Redeclaring a pointer parameter as private
        save_error(0);
        lexer();
        len1 = -1;
        len2 = -1;
        len3 = -1;
        if (current_token == p_corab) {
          lexer();
          if ((len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if ((len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if ((len3 = constant()) < 0)
                c_error(4, 35);
            }
          }
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
        }
        if (len1 != (*ob).ptr_int_local.len1 || len2 != (*ob).ptr_int_local.len2 ||
            len3 != (*ob).ptr_int_local.len3)
          c_error(4, 141);
        else if (current_token == p_asig)
          c_error(0, 54);
        else {
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
          (*ob).param++; // Prevent re-declaring it again
          return (NULL);
        }
      } else
        c_error(0, 117);
    } else
      c_error(0, 117);
  } else
    lexer();
  if (in_params == -1)
    (*ob).scope = current_scope;
  (*ob).ptr_int_local.len1 = -1;
  (*ob).ptr_int_local.len2 = -1;
  (*ob).ptr_int_local.len3 = -1;
  if (current_token == p_corab) {
    lexer();
    if (((*ob).ptr_int_local.len1 = constant()) < 0)
      c_error(4, 35);
    if (current_token == p_coma) {
      lexer();
      if (((*ob).ptr_int_local.len2 = constant()) < 0)
        c_error(4, 35);
      if (current_token == p_coma) {
        lexer();
        if (((*ob).ptr_int_local.len3 = constant()) < 0)
          c_error(4, 35);
      }
    }
    if (current_token != p_corce) {
      c_error(3, 26);
    }
    lexer();
  }
  (*ob).ptr_int_local.total_len = 0;
  if ((*ob).ptr_int_local.len1 > -1)
    (*ob).ptr_int_local.total_len = (*ob).ptr_int_local.len1 + 1;
  if ((*ob).ptr_int_local.len2 > -1)
    (*ob).ptr_int_local.total_len *= (*ob).ptr_int_local.len2 + 1;
  if ((*ob).ptr_int_local.len3 > -1)
    (*ob).ptr_int_local.total_len *= (*ob).ptr_int_local.len3 + 1;

  (*ob).type = tipo;
  (*ob).ptr_int_local.offset = offset;

  return (ob);
}

//-----------------------------------------------------------------------------

int analyze_pointer_struct(int tipo, int offset, struct object *structure) {
  struct object *ob;
  int dim1, dim2, dim3;

  if (current_token != p_id)
    c_error(1, 148);
  ob = o;
  if ((*ob).type != tnone) {
    if (in_params == -1 && (*ob).param == 1 && (*ob).scope == current_scope) {
      if ((*ob).type == tipo) { // Redeclaring a pointer parameter as private
        save_error(0);
        lexer();
        dim1 = -1;
        dim2 = -1;
        dim3 = -1;
        if (current_token == p_corab) {
          lexer();
          if ((dim1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if ((dim2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if ((dim3 = constant()) < 0)
                c_error(4, 35);
            }
          }
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
        }
        if (dim1 != (*ob).ptr_struct_global.dim1 || dim2 != (*ob).ptr_struct_global.dim2 ||
            dim3 != (*ob).ptr_struct_global.dim3)
          c_error(4, 141);
        else if (current_token == p_asig)
          c_error(0, 54);
        else {
          (*ob).param++;
          return (0); // Prevent re-declaring it again
        }
      } else
        c_error(0, 117);
    } else
      c_error(0, 117);
  } else
    lexer();
  if (in_params == -1)
    (*ob).scope = current_scope;
  (*ob).ptr_struct_global.dim1 = -1;
  (*ob).ptr_struct_global.dim2 = -1;
  (*ob).ptr_struct_global.dim3 = -1;
  if (current_token == p_corab) {
    lexer();
    if (((*ob).ptr_struct_global.dim1 = constant()) < 0)
      c_error(4, 35);
    if (current_token == p_coma) {
      lexer();
      if (((*ob).ptr_struct_global.dim2 = constant()) < 0)
        c_error(4, 35);
      if (current_token == p_coma) {
        lexer();
        if (((*ob).ptr_struct_global.dim3 = constant()) < 0)
          c_error(4, 35);
      }
    }
    if (current_token != p_corce) {
      c_error(3, 26);
    }
    lexer();
  }
  (*ob).ptr_struct_global.total_items = 0;
  if ((*ob).ptr_struct_global.dim1 > -1)
    (*ob).ptr_struct_global.total_items = (*ob).ptr_struct_global.dim1 + 1;
  if ((*ob).ptr_struct_global.dim2 > -1)
    (*ob).ptr_struct_global.total_items *= (*ob).ptr_struct_global.dim2 + 1;
  if ((*ob).ptr_struct_global.dim3 > -1)
    (*ob).ptr_struct_global.total_items *= (*ob).ptr_struct_global.dim3 + 1;

  (*ob).type = tipo;                            // tpsgl or tpslo
  (*ob).ptr_struct_global.offset = offset;      // pointer offset
  (*ob).ptr_struct_global.ostruct = structure; // original struct

  return (1);
}

//-----------------------------------------------------------------------------
//      Parser (drives the entire compilation process)
//-----------------------------------------------------------------------------

int analyze_struct(int offstruct) { // after " struct id [ <const> ] " // id==member
  int len = 0, dup, i, _itxt, _imem;
  struct object *ob;
  struct object *old_member, *member2;
  byte *oimemptr;

  while (current_token == p_ptocoma)
    lexer();

  while (current_token != p_end) {
    if (current_token == p_struct) { // Struct member ---

      lexer();

      if (current_token == p_pointer) { // Defining a struct pointer

        old_member = member;
        member = NULL;
        lexer();
        member = old_member;
        if (current_token != p_id) {
          c_error(1, 110);
        }
        ob = o;

        if ((*ob).type == tnone)
          c_error(0, 150); // Pointer can't be defined this way
        if ((*ob).type != tsglo && (*ob).type != tsloc)
          c_error(0, 150);
        lexer();
puntero_a_struct:
        analyze_pointer_struct(tpsgl, len, ob);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;

        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero_a_struct;
        } else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else {
        if (current_token != p_id)
          c_error(1, 110);
        ob = o;
        old_member = member;
        member = ob;
        lexer();
        if ((*ob).type != tnone)
          c_error(2, 22);

        (*ob).type = tsglo;
        (*ob).struct_global.offset = len;
        if (current_token == p_corab) {
          member2 = member;
          member = NULL;
          lexer();
          if (((*ob).struct_global.dim1 = constant()) < 0)
            c_error(4, 123);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).struct_global.dim2 = constant()) < 0)
              c_error(4, 123);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).struct_global.dim3 = constant()) < 0)
                c_error(4, 123);
            } else
              (*ob).struct_global.dim3 = -1;
          } else {
            (*ob).struct_global.dim2 = -1;
            (*ob).struct_global.dim3 = -1;
          }
          member = member2;
          (*ob).struct_global.total_items = (*ob).struct_global.dim1 + 1;
          if ((*ob).struct_global.dim2 > -1)
            (*ob).struct_global.total_items *= (*ob).struct_global.dim2 + 1;
          if ((*ob).struct_global.dim3 > -1)
            (*ob).struct_global.total_items *= (*ob).struct_global.dim3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
        } else {
          (*ob).struct_global.total_items = 1;
          (*ob).struct_global.dim1 = 0;
          (*ob).struct_global.dim2 = -1;
          (*ob).struct_global.dim3 = -1;
        }
        if (((*ob).struct_global.item_len = analyze_struct(offstruct + len)) == 0)
          c_error(0, 57);
        i = offstruct + len;
        dup = (*ob).struct_global.total_items + 1;
        if (dup > 1) {
          test_buffer(&mem, &imem_max,
                      i + (*ob).struct_global.item_len * (*ob).struct_global.total_items);
          test_buffer(&frm, &ifrm_max,
                      i + (*ob).struct_global.item_len * (*ob).struct_global.total_items);
          while (--dup) {
            if (i != offstruct + len) {
              memcpy(&mem[i], &mem[offstruct + len], (*ob).struct_global.item_len << 2);
              memcpy(&frm[i], &frm[offstruct + len], (*ob).struct_global.item_len << 2);
            }
            i += (*ob).struct_global.item_len;
          }
        }
        len += (*ob).struct_global.item_len * (*ob).struct_global.total_items;
        member = old_member;
        lexer();
        while (current_token == p_ptocoma)
          lexer();
      }
    } else if (current_token == p_string) { // String member

      lexer();

      if (current_token == p_pointer) { // Byte pointer member

        lexer();
puntero1:
        ob = analyze_pointer(tpcgl, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;

        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero1;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 125);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = tcglo;
        (*ob).string_global.offset = len + 1;
        if (current_token == p_corab) {
          lexer();
          if (current_token == p_corce) {
            lexer();
            (*ob).string_global.total_len = 255;
          } else {
            if (((*ob).string_global.total_len = constant()) < 0)
              c_error(4, 127);
            if ((*ob).string_global.total_len > 0xFFFFF)
              c_error(4, 135);
            if (current_token != p_corce)
              c_error(3, 26);
            lexer();
          }
        } else
          (*ob).string_global.total_len = 255;

        test_buffer(&frm, &ifrm_max, offstruct + len);
        frm[offstruct + len] = 0xDAD00000;

        if (current_token == p_asig) {
          save_error(1);
          _itxt = itxt;
          lexer();
          if (current_token != p_lit &&
              !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
            c_error(3, 128);
          if (strlen((char *)&mem[token_value]) > (*ob).string_global.total_len + 1)
            c_error(4, 129);
          test_buffer(&mem, &imem_max, offstruct + len);
          mem[offstruct + len] = 0xDAD00000 | (*ob).string_global.total_len;
          memmove((char *)&mem[offstruct + len + 1], (char *)&mem[token_value],
                  strlen((char *)&mem[token_value]) + 1);
          len += 1 + ((*ob).string_global.total_len + 5) / 4;
          itxt = _itxt; // Remove the string from the text segment
          lexer();
        } else {
          test_buffer(&mem, &imem_max, offstruct + len);
          mem[offstruct + len] = 0xDAD00000 | (*ob).string_global.total_len;
          len += 1 + ((*ob).string_global.total_len + 5) / 4;
        }
      }

      if (current_token == p_coma)
        current_token = p_string;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_byte) { // Byte member ---

      lexer();

      if (current_token == p_pointer) { // Byte pointer member

        lexer();
puntero2:
        ob = analyze_pointer(tpbgl, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;

        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero2;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 142);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = tbglo;
        (*ob).byte_global.offset = len;
        if (current_token == p_corab) {
          lexer();
          if (((*ob).byte_global.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).byte_global.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).byte_global.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).byte_global.len3 = -1;
          } else {
            (*ob).byte_global.len2 = -1;
            (*ob).byte_global.len3 = -1;
          }
          (*ob).byte_global.total_len = (*ob).byte_global.len1 + 1;
          if ((*ob).byte_global.len2 > -1)
            (*ob).byte_global.total_len *= (*ob).byte_global.len2 + 1;
          if ((*ob).byte_global.len3 > -1)
            (*ob).byte_global.total_len *= (*ob).byte_global.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
          test_buffer(&mem, &imem_max, offstruct + len + ((*ob).byte_global.total_len + 3) / 4);

          test_buffer(&frm, &ifrm_max, offstruct + len + ((*ob).byte_global.total_len + 3) / 4);
          memset(&frm[offstruct + len], 2, (*ob).byte_global.total_len);

          if (current_token == p_asig) {
            _imem = imem;
            imem = offstruct + len;
            save_error(1);
            lexer();
            oimemptr = (byte *)&mem[imem];
            tglo_init(2);
            if (imemptr - oimemptr > (*ob).byte_global.total_len)
              c_error(4, 33);
            imem = _imem;
          }
          (*ob).byte_global.total_len = ((*ob).byte_global.total_len + 3) / 4;
        } else {
          (*ob).type = tbglo;
          (*ob).byte_global.offset = len;
          (*ob).byte_global.len1 = 0;
          (*ob).byte_global.len2 = -1;
          (*ob).byte_global.len3 = -1;
          (*ob).byte_global.total_len = 1; // 1 int

          test_buffer(&frm, &ifrm_max, offstruct + len);
          frm[offstruct + len] = 2;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            test_buffer(&mem, &imem_max, offstruct + len);
            mem[offstruct + len] = constant();
            if (mem[offstruct + len] < 0 || mem[offstruct + len] > 255)
              c_error(4, 143);
          }
        }
        len += (*ob).byte_global.total_len;
      }

      if (current_token == p_coma)
        current_token = p_byte;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_word) { // Word member ---

      lexer();

      if (current_token == p_pointer) { // Word pointer member

        lexer();
puntero3:
        ob = analyze_pointer(tpwgl, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero3;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 145);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = twglo;
        (*ob).word_global.offset = len;
        if (current_token == p_corab) {
          lexer();
          if (((*ob).word_global.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).word_global.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).word_global.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).word_global.len3 = -1;
          } else {
            (*ob).word_global.len2 = -1;
            (*ob).word_global.len3 = -1;
          }
          (*ob).word_global.total_len = (*ob).word_global.len1 + 1;
          if ((*ob).word_global.len2 > -1)
            (*ob).word_global.total_len *= (*ob).word_global.len2 + 1;
          if ((*ob).word_global.len3 > -1)
            (*ob).word_global.total_len *= (*ob).word_global.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
          test_buffer(&mem, &imem_max, offstruct + len + ((*ob).word_global.total_len + 1) / 2);

          test_buffer(&frm, &ifrm_max, offstruct + len + ((*ob).word_global.total_len + 1) / 2);
          memset(&frm[offstruct + len], 1, (*ob).word_global.total_len * 2);

          if (current_token == p_asig) {
            _imem = imem;
            imem = offstruct + len;
            save_error(1);
            lexer();
            oimemptr = (byte *)&mem[imem];
            tglo_init(1);
            if (imemptr - oimemptr > (*ob).word_global.total_len * 2)
              c_error(4, 33);
            imem = _imem;
          }
          (*ob).word_global.total_len = ((*ob).word_global.total_len + 1) / 2;
        } else {
          (*ob).type = twglo;
          (*ob).word_global.offset = len;
          (*ob).word_global.len1 = 0;
          (*ob).word_global.len2 = -1;
          (*ob).word_global.len3 = -1;
          (*ob).word_global.total_len = 1; // 1 int

          test_buffer(&frm, &ifrm_max, offstruct + len);
          frm[offstruct + len] = 1;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            test_buffer(&mem, &imem_max, offstruct + len);
            mem[offstruct + len] = constant();
            if (mem[offstruct + len] < 0 || mem[offstruct + len] > 65535)
              c_error(4, 144);
          }
        }
        len += (*ob).word_global.total_len;
      }

      if (current_token == p_coma)
        current_token = p_word;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_int || current_token == p_id || current_token == p_pointer) {
      if (current_token == p_int) {
        lexer();
        if (current_token != p_id && current_token != p_pointer)
          c_error(1, 149);
      }

      if (current_token == p_pointer) { // Int pointer member

        lexer();
puntero4:
        ob = analyze_pointer(tpigl, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero4;
        }

      } else {
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        if (current_token == p_corab) {
          lexer(); // Table member ---
          (*ob).type = ttglo;
          (*ob).table_global.offset = len;
          if (((*ob).table_global.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).table_global.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).table_global.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).table_global.len3 = -1;
          } else {
            (*ob).table_global.len2 = -1;
            (*ob).table_global.len3 = -1;
          }
          (*ob).table_global.total_len = (*ob).table_global.len1 + 1;
          if ((*ob).table_global.len2 > -1)
            (*ob).table_global.total_len *= (*ob).table_global.len2 + 1;
          if ((*ob).table_global.len3 > -1)
            (*ob).table_global.total_len *= (*ob).table_global.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();

          test_buffer(&mem, &imem_max, offstruct + len + (*ob).table_global.total_len);
          if (current_token == p_asig) {
            _imem = imem;
            imem = offstruct + len;
            save_error(1);
            lexer();
            tglo_init(0);
            if (imem - (offstruct + len) > (*ob).table_global.total_len)
              c_error(4, 33);
            imem = _imem;
          }
          len += (*ob).table_global.total_len;

        } else { // Variable member ---

          (*ob).type = tvglo;
          (*ob).var_global.offset = len;
          test_buffer(&mem, &imem_max, offstruct + len);
          if (current_token == p_asig) {
            lexer();
            mem[offstruct + len] = constant();
          }
          len += 1;
        }
      }
      if (!free_sintax)
        if (current_token != p_ptocoma && current_token != p_coma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
    } else
      c_error(0, 58);
  }
  return (len);
}

//---------------------------------------------------------------------------

int analyze_struct_local(int offstruct) { // after " struct id [ <const> ] " // id==member
  int len = 0, dup, i, _itxt, _iloc;
  struct object *ob = NULL;
  struct object *old_member, *member2;
  byte *oimemptr;

  while (current_token == p_ptocoma)
    lexer();

  while (current_token != p_end) {
    if (current_token == p_struct) { // Struct member ---

      lexer();

      if (current_token == p_pointer) { // Defining a struct pointer

        old_member = member;
        member = NULL;
        lexer();
        member = old_member;
        if (current_token != p_id) {
          c_error(1, 110);
        }
        ob = o;

        if ((*ob).type == tnone)
          c_error(0, 150); // Pointer can't be defined this way
        if ((*ob).type != tsglo && (*ob).type != tsloc)
          c_error(0, 150);
        lexer();
puntero_a_struct:
        analyze_pointer_struct(tpslo, len, ob);
        test_buffer(&loc, &iloc_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          loc[offstruct + len] = constant();
        }
        len += 1;

        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero_a_struct;
        } else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else {
        if (current_token != p_id)
          c_error(1, 110);
        ob = o;

        old_member = member;
        member = ob;
        lexer();
        if ((*ob).type != tnone)
          c_error(2, 22);

        (*ob).type = tsloc;
        (*ob).struct_local.offset = len;
        if (current_token == p_corab) {
          member2 = member;
          member = NULL;
          lexer();
          if (((*ob).struct_local.dim1 = constant()) < 0)
            c_error(4, 123);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).struct_local.dim2 = constant()) < 0)
              c_error(4, 123);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).struct_local.dim3 = constant()) < 0)
                c_error(4, 123);
            } else
              (*ob).struct_local.dim3 = -1;
          } else {
            (*ob).struct_local.dim2 = -1;
            (*ob).struct_local.dim3 = -1;
          }
          member = member2;
          (*ob).struct_local.total_items = (*ob).struct_local.dim1 + 1;
          if ((*ob).struct_local.dim2 > -1)
            (*ob).struct_local.total_items *= (*ob).struct_local.dim2 + 1;
          if ((*ob).struct_local.dim3 > -1)
            (*ob).struct_local.total_items *= (*ob).struct_local.dim3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
        } else {
          (*ob).struct_local.total_items = 1;
          (*ob).struct_local.dim1 = 0;
          (*ob).struct_local.dim2 = -1;
          (*ob).struct_local.dim3 = -1;
        }
        if (((*ob).struct_local.item_len = analyze_struct_local(offstruct + len)) == 0)
          c_error(0, 57);
        i = offstruct + len;
        dup = (*ob).struct_local.total_items + 1;
        if (dup > 1) {
          test_buffer(&loc, &iloc_max,
                      i + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
          test_buffer(&frm, &ifrm_max,
                      i + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
          while (--dup) {
            memcpy(&loc[i], &loc[offstruct + len], (*ob).struct_local.item_len << 2);
            memcpy(&frm[i], &frm[offstruct + len], (*ob).struct_local.item_len << 2);
            i += (*ob).struct_local.item_len;
          }
        }
        len += (*ob).struct_local.item_len * (*ob).struct_local.total_items;
        member = old_member;
        lexer();
        while (current_token == p_ptocoma)
          lexer();
      }

    } else if (current_token == p_string) { // String member

      lexer();

      if (current_token == p_pointer) { // Byte pointer member

        lexer();
puntero1:
        ob = analyze_pointer(tpclo, len);
        test_buffer(&loc, &iloc_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          loc[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero1;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 125);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = tcloc;
        (*ob).string_local.offset = len + 1;
        if (current_token == p_corab) {
          lexer();
          if (current_token == p_corce) {
            lexer();
            (*ob).string_local.total_len = 255;
          } else {
            if (((*ob).string_local.total_len = constant()) < 0)
              c_error(4, 127);
            if ((*ob).string_local.total_len > 0xFFFFF)
              c_error(4, 135);
            if (current_token != p_corce)
              c_error(3, 26);
            lexer();
          }
        } else
          (*ob).string_local.total_len = 255;

        test_buffer(&frm, &ifrm_max, offstruct + len);
        frm[offstruct + len] = 0xDAD00000;

        if (current_token == p_asig) {
          save_error(1);
          _itxt = itxt;
          lexer();
          if (current_token != p_lit &&
              !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
            c_error(3, 128);
          if (strlen((char *)&mem[token_value]) > (*ob).string_local.total_len + 1)
            c_error(4, 129);
          test_buffer(&loc, &iloc_max, offstruct + len);
          loc[offstruct + len] = 0xDAD00000 | (*ob).string_local.total_len;
          div_strcpy((char *)&loc[offstruct + len + 1], (*ob).string_local.total_len + 1,
                     (char *)&mem[token_value]);
          len += 1 + ((*ob).string_local.total_len + 5) / 4;
          itxt = _itxt; // Remove the string from the text segment
          lexer();
        } else {
          test_buffer(&loc, &iloc_max, offstruct + len);
          loc[offstruct + len] = 0xDAD00000 | (*ob).string_local.total_len;
          len += 1 + ((*ob).string_local.total_len + 5) / 4;
        }
      }

      if (current_token == p_coma)
        current_token = p_string;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_byte) { // Byte member ---

      lexer();

      if (current_token == p_pointer) { // Byte pointer member

        lexer();
puntero2:
        ob = analyze_pointer(tpblo, len);
        test_buffer(&loc, &iloc_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          loc[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero2;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 142);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = tbloc;
        (*ob).byte_local.offset = len;
        if (current_token == p_corab) {
          lexer();
          if (((*ob).byte_local.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).byte_local.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).byte_local.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).byte_local.len3 = -1;
          } else {
            (*ob).byte_local.len2 = -1;
            (*ob).byte_local.len3 = -1;
          }
          (*ob).byte_local.total_len = (*ob).byte_local.len1 + 1;
          if ((*ob).byte_local.len2 > -1)
            (*ob).byte_local.total_len *= (*ob).byte_local.len2 + 1;
          if ((*ob).byte_local.len3 > -1)
            (*ob).byte_local.total_len *= (*ob).byte_local.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();

          test_buffer(&loc, &iloc_max, offstruct + len + ((*ob).byte_local.total_len + 3) / 4);

          test_buffer(&frm, &ifrm_max, offstruct + len + ((*ob).byte_local.total_len + 3) / 4);
          memset(&frm[offstruct + len], 2, (*ob).byte_local.total_len);

          if (current_token == p_asig) {
            _iloc = iloc;
            iloc = offstruct + len;
            save_error(1);
            lexer();
            oimemptr = (byte *)&loc[iloc];
            tloc_init(2);
            if (imemptr - oimemptr > (*ob).byte_local.total_len)
              c_error(4, 33);
            iloc = _iloc;
          }
          (*ob).byte_local.total_len = ((*ob).byte_local.total_len + 3) / 4;
        } else {
          (*ob).type = tbloc;
          (*ob).byte_local.offset = len;
          (*ob).byte_local.len1 = 0;
          (*ob).byte_local.len2 = -1;
          (*ob).byte_local.len3 = -1;
          (*ob).byte_local.total_len = 1; // 1 int

          test_buffer(&frm, &ifrm_max, offstruct + len);
          frm[offstruct + len] = 2;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            test_buffer(&loc, &iloc_max, offstruct + len);
            loc[offstruct + len] = constant();
            if (loc[offstruct + len] < 0 || loc[offstruct + len] > 255)
              c_error(4, 143);
          }
        }
        len += (*ob).byte_local.total_len;
      }

      if (current_token == p_coma)
        current_token = p_byte;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_word) { // Word member ---

      lexer();

      if (current_token == p_pointer) { // Word pointer member

        lexer();
puntero3:
        ob = analyze_pointer(tpwlo, len);
        test_buffer(&loc, &iloc_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          loc[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero3;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 145);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = twloc;
        (*ob).word_local.offset = len;
        if (current_token == p_corab) {
          lexer();
          if (((*ob).word_local.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).word_local.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).word_local.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).word_local.len3 = -1;
          } else {
            (*ob).word_local.len2 = -1;
            (*ob).word_local.len3 = -1;
          }
          (*ob).word_local.total_len = (*ob).word_local.len1 + 1;
          if ((*ob).word_local.len2 > -1)
            (*ob).word_local.total_len *= (*ob).word_local.len2 + 1;
          if ((*ob).word_local.len3 > -1)
            (*ob).word_local.total_len *= (*ob).word_local.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();

          test_buffer(&loc, &iloc_max, offstruct + len + ((*ob).word_local.total_len + 1) / 2);

          test_buffer(&frm, &ifrm_max, offstruct + len + ((*ob).word_local.total_len + 1) / 2);
          memset(&frm[offstruct + len], 1, (*ob).word_local.total_len * 2);

          if (current_token == p_asig) {
            _iloc = iloc;
            iloc = offstruct + len;
            save_error(1);
            lexer();
            oimemptr = (byte *)&loc[iloc];
            tloc_init(1);
            if (imemptr - oimemptr > (*ob).word_local.total_len * 2)
              c_error(4, 33);
            iloc = _iloc;
          }
          (*ob).word_local.total_len = ((*ob).word_local.total_len + 1) / 2;
        } else {
          (*ob).type = twloc;
          (*ob).word_local.offset = len;
          (*ob).word_local.len1 = 0;
          (*ob).word_local.len2 = -1;
          (*ob).word_local.len3 = -1;
          (*ob).word_local.total_len = 1; // 1 int

          test_buffer(&frm, &ifrm_max, offstruct + len);
          frm[offstruct + len] = 1;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            test_buffer(&loc, &iloc_max, offstruct + len);
            loc[offstruct + len] = constant();
            if (loc[offstruct + len] < 0 || loc[offstruct + len] > 65535)
              c_error(4, 144);
          }
        }
        len += (*ob).word_local.total_len;
      }

      if (current_token == p_coma)
        current_token = p_word;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_int || current_token == p_id || current_token == p_pointer) {
      if (current_token == p_int) {
        lexer();
        if (current_token != p_id && current_token != p_pointer)
          c_error(1, 149);
      }

      if (current_token == p_pointer) { // Int pointer member

        lexer();
puntero4:
        ob = analyze_pointer(tpilo, len);
        test_buffer(&loc, &iloc_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          loc[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero4;
        }

      } else {
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        if (current_token == p_corab) {
          lexer(); // Table member ---
          (*ob).type = ttloc;
          (*ob).table_local.offset = len;
          if (((*ob).table_local.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).table_local.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).table_local.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).table_local.len3 = -1;
          } else {
            (*ob).table_local.len2 = -1;
            (*ob).table_local.len3 = -1;
          }
          (*ob).table_local.total_len = (*ob).table_local.len1 + 1;
          if ((*ob).table_local.len2 > -1)
            (*ob).table_local.total_len *= (*ob).table_local.len2 + 1;
          if ((*ob).table_local.len3 > -1)
            (*ob).table_local.total_len *= (*ob).table_local.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();

          test_buffer(&loc, &iloc_max, offstruct + len + (*ob).table_local.total_len);
          if (current_token == p_asig) {
            _iloc = iloc;
            iloc = offstruct + len;
            save_error(1);
            lexer();
            tloc_init(0);
            if (iloc - (offstruct + len) > (*ob).table_local.total_len)
              c_error(4, 33);
            iloc = _iloc;
          }
          len += (*ob).table_local.total_len;

        } else { // Variable member ---

          (*ob).type = tvloc;
          (*ob).var_local.offset = len;
          test_buffer(&loc, &iloc_max, offstruct + len);
          if (current_token == p_asig) {
            lexer();
            loc[offstruct + len] = constant();
          }
          len += 1;
        }
      }
      if (!free_sintax)
        if (current_token != p_ptocoma && current_token != p_coma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
    } else
      c_error(0, 58);
    (*ob).scope = lexical_scope;
  }
  return (len);
}

//---------------------------------------------------------------------------

int analyze_struct_private(int offstruct) { // after " struct id [ <const> ] " // id=member
  int len = 0, dup, i, _itxt, _imem;
  struct object *ob = NULL;
  struct object *old_member, *member2;
  byte *oimemptr;

  while (current_token == p_ptocoma)
    lexer();

  while (current_token != p_end) {
    if (current_token == p_struct) { // Struct member ---

      lexer();

      if (current_token == p_pointer) { // Defining a struct pointer

        old_member = member;
        member = NULL;
        lexer();
        member = old_member;
        if (current_token != p_id) {
          c_error(1, 110);
        }
        ob = o;

        if ((*ob).type == tnone)
          c_error(0, 150); // Pointer can't be defined this way
        if ((*ob).type != tsglo && (*ob).type != tsloc)
          c_error(0, 150);
        lexer();
puntero_a_struct:
        analyze_pointer_struct(tpslo, len, ob);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;

        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero_a_struct;
        } else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else {
        if (current_token != p_id)
          c_error(1, 110);
        ob = o;

        old_member = member;
        member = ob;
        lexer();
        if ((*ob).type != tnone)
          c_error(2, 22);

        (*ob).type = tsloc;
        (*ob).struct_local.offset = len;
        if (current_token == p_corab) {
          member2 = member;
          member = NULL;
          lexer();
          if (((*ob).struct_local.dim1 = constant()) < 0)
            c_error(4, 123);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).struct_local.dim2 = constant()) < 0)
              c_error(4, 123);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).struct_local.dim3 = constant()) < 0)
                c_error(4, 123);
            } else
              (*ob).struct_local.dim3 = -1;
          } else {
            (*ob).struct_local.dim2 = -1;
            (*ob).struct_local.dim3 = -1;
          }
          member = member2;
          (*ob).struct_local.total_items = (*ob).struct_local.dim1 + 1;
          if ((*ob).struct_local.dim2 > -1)
            (*ob).struct_local.total_items *= (*ob).struct_local.dim2 + 1;
          if ((*ob).struct_local.dim3 > -1)
            (*ob).struct_local.total_items *= (*ob).struct_local.dim3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
        } else {
          (*ob).struct_local.total_items = 1;
          (*ob).struct_local.dim1 = 0;
          (*ob).struct_local.dim2 = -1;
          (*ob).struct_local.dim3 = -1;
        }
        if (((*ob).struct_local.item_len = analyze_struct_private(offstruct + len)) == 0)
          c_error(0, 57);
        i = offstruct + len;
        dup = (*ob).struct_local.total_items + 1;
        if (dup > 1) {
          test_buffer(&mem, &imem_max,
                      i + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
          test_buffer(&frm, &ifrm_max,
                      i + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
          while (--dup) {
            memcpy(&mem[i], &mem[offstruct + len], (*ob).struct_local.item_len << 2);
            memcpy(&frm[i], &frm[offstruct + len], (*ob).struct_local.item_len << 2);
            i += (*ob).struct_local.item_len;
          }
        }
        len += (*ob).struct_local.item_len * (*ob).struct_local.total_items;
        member = old_member;
        lexer();
        while (current_token == p_ptocoma)
          lexer();
      }

    } else if (current_token == p_string) { // String member

      lexer();

      if (current_token == p_pointer) { // Byte pointer member

        lexer();
puntero1:
        ob = analyze_pointer(tpclo, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero1;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 125);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = tcloc;
        (*ob).string_local.offset = len + 1;
        if (current_token == p_corab) {
          lexer();
          if (current_token == p_corce) {
            lexer();
            (*ob).string_local.total_len = 255;
          } else {
            if (((*ob).string_local.total_len = constant()) < 0)
              c_error(4, 127);
            if ((*ob).string_local.total_len > 0xFFFFF)
              c_error(4, 135);
            if (current_token != p_corce)
              c_error(3, 26);
            lexer();
          }
        } else
          (*ob).string_local.total_len = 255;

        test_buffer(&frm, &ifrm_max, offstruct + len);
        frm[offstruct + len] = 0xDAD00000;

        if (current_token == p_asig) {
          save_error(1);
          _itxt = itxt;
          lexer();
          if (current_token != p_lit &&
              !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
            c_error(3, 128);
          if (strlen((char *)&mem[token_value]) > (*ob).string_local.total_len + 1)
            c_error(4, 129);
          test_buffer(&mem, &imem_max, offstruct + len);
          mem[offstruct + len] = 0xDAD00000 | (*ob).string_local.total_len;
          memmove((char *)&mem[offstruct + len + 1], (char *)&mem[token_value],
                  strlen((char *)&mem[token_value]) + 1);
          len += 1 + ((*ob).string_local.total_len + 5) / 4;
          itxt = _itxt; // Remove the string from the text segment
          lexer();
        } else {
          test_buffer(&mem, &imem_max, offstruct + len);
          mem[offstruct + len] = 0xDAD00000 | (*ob).string_local.total_len;
          len += 1 + ((*ob).string_local.total_len + 5) / 4;
        }
      }

      if (current_token == p_coma)
        current_token = p_string;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_byte) { // Byte member ---

      lexer();

      if (current_token == p_pointer) { // Byte pointer member

        lexer();
puntero2:
        ob = analyze_pointer(tpblo, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero2;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 142);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = tbloc;
        (*ob).byte_local.offset = len;
        if (current_token == p_corab) {
          lexer();
          if (((*ob).byte_local.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).byte_local.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).byte_local.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).byte_local.len3 = -1;
          } else {
            (*ob).byte_local.len2 = -1;
            (*ob).byte_local.len3 = -1;
          }
          (*ob).byte_local.total_len = (*ob).byte_local.len1 + 1;
          if ((*ob).byte_local.len2 > -1)
            (*ob).byte_local.total_len *= (*ob).byte_local.len2 + 1;
          if ((*ob).byte_local.len3 > -1)
            (*ob).byte_local.total_len *= (*ob).byte_local.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
          test_buffer(&mem, &imem_max, offstruct + len + ((*ob).byte_local.total_len + 3) / 4);

          test_buffer(&frm, &ifrm_max, offstruct + len + ((*ob).byte_local.total_len + 3) / 4);
          memset(&frm[offstruct + len], 2, (*ob).byte_local.total_len);

          if (current_token == p_asig) {
            _imem = imem;
            imem = offstruct + len;
            save_error(1);
            lexer();
            oimemptr = (byte *)&mem[imem];
            tglo_init(2);
            if (imemptr - oimemptr > (*ob).byte_local.total_len)
              c_error(4, 33);
            imem = _imem;
          }
          (*ob).byte_local.total_len = ((*ob).byte_local.total_len + 3) / 4;
        } else {
          (*ob).type = tbloc;
          (*ob).byte_local.offset = len;
          (*ob).byte_local.len1 = 0;
          (*ob).byte_local.len2 = -1;
          (*ob).byte_local.len3 = -1;
          (*ob).byte_local.total_len = 1; // 1 int

          test_buffer(&frm, &ifrm_max, offstruct + len);
          frm[offstruct + len] = 2;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            test_buffer(&mem, &imem_max, offstruct + len);
            mem[offstruct + len] = constant();
            if (mem[offstruct + len] < 0 || mem[offstruct + len] > 255)
              c_error(4, 143);
          }
        }
        len += (*ob).byte_local.total_len;
      }

      if (current_token == p_coma)
        current_token = p_byte;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_word) { // Word member ---

      lexer();

      if (current_token == p_pointer) { // Word pointer member

        lexer();
puntero3:
        ob = analyze_pointer(tpwlo, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero3;
        }

      } else {
        if (current_token != p_id)
          c_error(1, 145);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        (*ob).type = twloc;
        (*ob).word_local.offset = len;
        if (current_token == p_corab) {
          lexer();
          if (((*ob).word_local.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).word_local.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).word_local.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).word_local.len3 = -1;
          } else {
            (*ob).word_local.len2 = -1;
            (*ob).word_local.len3 = -1;
          }
          (*ob).word_local.total_len = (*ob).word_local.len1 + 1;
          if ((*ob).word_local.len2 > -1)
            (*ob).word_local.total_len *= (*ob).word_local.len2 + 1;
          if ((*ob).word_local.len3 > -1)
            (*ob).word_local.total_len *= (*ob).word_local.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();
          test_buffer(&mem, &imem_max, offstruct + len + ((*ob).word_local.total_len + 1) / 2);

          test_buffer(&frm, &ifrm_max, offstruct + len + ((*ob).word_local.total_len + 1) / 2);
          memset(&frm[offstruct + len], 1, (*ob).word_local.total_len * 2);

          if (current_token == p_asig) {
            _imem = imem;
            imem = offstruct + len;
            save_error(1);
            lexer();
            oimemptr = (byte *)&mem[imem];
            tglo_init(1);
            if (imemptr - oimemptr > (*ob).word_local.total_len * 2)
              c_error(4, 33);
            imem = _imem;
          }
          (*ob).word_local.total_len = ((*ob).word_local.total_len + 1) / 2;
        } else {
          (*ob).type = twloc;
          (*ob).word_local.offset = len;
          (*ob).word_local.len1 = 0;
          (*ob).word_local.len2 = -1;
          (*ob).word_local.len3 = -1;
          (*ob).word_local.total_len = 1; // 1 int

          test_buffer(&frm, &ifrm_max, offstruct + len);
          frm[offstruct + len] = 1;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            test_buffer(&mem, &imem_max, offstruct + len);
            mem[offstruct + len] = constant();
            if (mem[offstruct + len] < 0 || mem[offstruct + len] > 65535)
              c_error(4, 144);
          }
        }
        len += (*ob).word_local.total_len;
      }

      if (current_token == p_coma)
        current_token = p_word;
      else {
        if (!free_sintax)
          if (current_token != p_ptocoma)
            c_error(3, 137);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }

    } else if (current_token == p_int || current_token == p_id || current_token == p_pointer) {
      if (current_token == p_int) {
        lexer();
        if (current_token != p_id && current_token != p_pointer)
          c_error(1, 149);
      }

      if (current_token == p_pointer) { // Int pointer member

        lexer();
puntero4:
        ob = analyze_pointer(tpilo, len);
        test_buffer(&mem, &imem_max, offstruct + len);
        if (current_token == p_asig) {
          lexer();
          mem[offstruct + len] = constant();
        }
        len += 1;
        if (current_token == p_coma) {
          lexer();
          if (current_token == p_pointer)
            lexer();
          goto puntero4;
        }

      } else {
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 114);
        lexer();
        if (current_token == p_corab) {
          lexer(); // Table member ---
          (*ob).type = ttloc;
          (*ob).table_local.offset = len;
          if (((*ob).table_local.len1 = constant()) < 0)
            c_error(4, 35);
          if (current_token == p_coma) {
            lexer();
            if (((*ob).table_local.len2 = constant()) < 0)
              c_error(4, 35);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).table_local.len3 = constant()) < 0)
                c_error(4, 35);
            } else
              (*ob).table_local.len3 = -1;
          } else {
            (*ob).table_local.len2 = -1;
            (*ob).table_local.len3 = -1;
          }
          (*ob).table_local.total_len = (*ob).table_local.len1 + 1;
          if ((*ob).table_local.len2 > -1)
            (*ob).table_local.total_len *= (*ob).table_local.len2 + 1;
          if ((*ob).table_local.len3 > -1)
            (*ob).table_local.total_len *= (*ob).table_local.len3 + 1;
          if (current_token != p_corce) {
            c_error(3, 26);
          }
          lexer();

          test_buffer(&mem, &imem_max, offstruct + len + (*ob).table_local.total_len);
          if (current_token == p_asig) {
            _imem = imem;
            imem = offstruct + len;
            save_error(1);
            lexer();
            tglo_init(0);
            if (imem - (offstruct + len) > (*ob).table_local.total_len)
              c_error(4, 33);
            imem = _imem;
          }
          len += (*ob).table_local.total_len;

        } else { // Variable member ---

          (*ob).type = tvloc;
          (*ob).var_local.offset = len;

          test_buffer(&mem, &imem_max, offstruct + len);
          if (current_token == p_asig) {
            lexer();
            mem[offstruct + len] = constant();
          }
          len += 1;
        }
      }

      if (!free_sintax)
        if (current_token != p_ptocoma && current_token != p_coma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();

    } else
      c_error(0, 58);
    (*ob).scope = lexical_scope;
  }
  return (len);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void parser(void) {
  struct object *ob, *member2;
  int _imem, _imem_old, num_params = 0, n;
  byte *old_source, *nombre_dll, *oimemptr, *nombre_include;
  int _itxt, dup;
  char cWork[256];

  int num_extern;

  lexer();

  //---------------------------------------------------------------------------
  // Compiler options
  //---------------------------------------------------------------------------
  // p_compiler_options {option} {;}
  //---------------------------------------------------------------------------

  max_process = 0; // Default option values
  ignore_errors = 0;
  free_sintax = 0;
  extended_conditions = 0;
  simple_conditions = 0;
  check_range = 1;
  check_id = 1;
  check_null = 1;
  enable_strfix = 1;
  optimize = 1;

  if (current_token == p_compiler_options) {
    lexer();
    do {
      if (current_token == p_ptocoma) {
        lexer();
        break;
      }
      if (current_token == p_coma)
        lexer();
      if (current_token == p_id && (*o).type == tcons) {
        switch ((*o).cons.value) {
        case 0: // _max_process
          lexer();
          if (current_token != p_asig)
            c_error(3, 139);
          lexer();
          max_process = constant();
          if (max_process < 0)
            max_process = 0;
          break;
        case 1: // _extended_conditions
          lexer();
          extended_conditions = 1;
          break;
        case 2: // _simple_conditions
          lexer();
          simple_conditions = 1;
          break;
        case 3: // _case_sensitive
          lexer();
          memcpy(lower + 129,
                 "\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x84\x86\x82\x91\x91\x93\x94"
                 "\x95\x96\x97\x98\x94\x81\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3",
                 35);
          memcpy(lower + 'A', "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
          lower['\xa5'] = '\xa5';
          break;
        case 4: // _ignore_errors
          lexer();
          ignore_errors = 1;
          break;
        case 5: // _free_sintax
          lexer();
          free_sintax = 1;
          break;
        case 6: // _no_check
          lexer();
          check_range = 0;
          check_id = 0;
          check_null = 0;
          break;
        case 7: // _no_strfix
          lexer();
          enable_strfix = 0;
          break;
        case 8: // _no_optimization
          lexer();
          optimize = 0;
          break;
        case 9: // _no_range_check
          lexer();
          check_range = 0;
          break;
        case 10: // _no_id_check
          lexer();
          check_id = 0;
          break;
        case 11: // _no_null_check
          lexer();
          check_null = 0;
          break;
        default:
          c_error(0, 138);
          break;
        }
      } else {
        if (!free_sintax) {
          if (current_token == p_program)
            c_error(3, 66);
          else
            c_error(0, 138);
        } else
          break;
      }
    } while (1);
  }

  //---------------------------------------------------------------------------
  // Header
  //---------------------------------------------------------------------------
  // p_program p_id {;}
  //---------------------------------------------------------------------------

  save_error(0);
  if (current_token != p_program && current_token != p_setup_program)
    c_error(4, 20);

  if (current_token == p_setup_program) {
    program_type = 1;
    div_strcpy((char *)cWork, sizeof(cWork), "install/setup.ins");
  } else
    div_strcpy((char *)cWork, sizeof(cWork), "system/exec.ins");
  if ((lins = fopen((char *)cWork, "wb")) == NULL)
    c_error(0, 0);

  statement_start();
  lexer();
  if (current_token != p_id)
    c_error(1, 21);
  ob = o;
  if ((*ob).type != tnone)
    c_error(0, 115);
  (*ob).type = tproc;
  (*ob).proc.scope = current_scope = ob;
  (*ob).proc.offset = 0;
  (*ob).proc.num_params = 0;
  lexer();
  if (!free_sintax)
    if (current_token != p_ptocoma)
      c_error(3, 66);
  while (current_token == p_ptocoma || current_token == p_coma)
    lexer();
  statement_end();

  while (current_token == p_include) {
    statement_start();
    lexer();
    if (current_token != p_lit &&
        !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
      c_error(1, 62);
    old_source = source;
    nombre_include = (byte *)&mem[token_value];


    //

    source = old_source;
    lexer();
    if (!free_sintax)
      if (current_token != p_ptocoma)
        c_error(3, 66);
    while (current_token == p_ptocoma || current_token == p_coma)
      lexer();
    statement_end();
  }
  //---------------------------------------------------------------------------
  // Constants section
  //---------------------------------------------------------------------------
  // [ p_const {;} { p_id p_asig <exp> {; | ,} } ]
  //---------------------------------------------------------------------------

  if (current_token == p_const) {
    skip_semicolons();
    while (current_token == p_id) {
      ob = o;
      if ((*ob).type != tnone && (*ob).type != tcons)
        c_error(0, 116);
      (*ob).type = tcons;
      lexer();
      if (current_token != p_asig)
        c_error(3, 119);
      lexer();
      if (current_token == p_lit ||
          (current_token == p_id && (*o).type == tcons && (*o).cons.literal))
        (*ob).cons.literal = 1;
      else
        (*ob).cons.literal = 0;
      (*ob).cons.value = constant();
      if (!free_sintax)
        if (current_token != p_ptocoma && current_token != p_coma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
    }
  }

  //---------------------------------------------------------------------------
  // Global variables, can be used in constant expressions
  //---------------------------------------------------------------------------
  // <variable> := p_id [ p_asig <exp> ] {;}
  //
  // <tabla> := p_id p_corab ( p_corce p_asig <init>
  //                       | <exp> p_corce [ p_asig <init> ] ) {;}
  //
  // <init>  := ( <string> | <exp> [ [p_dup] p_abrir <init> p_cerrar ] )
  //            [ p_coma <init>]
  //---------------------------------------------------------------------------

  if (current_token == p_global) {
    skip_semicolons();
    while (current_token == p_id || current_token == p_int || current_token == p_pointer ||
           current_token == p_struct || current_token == p_string || current_token == p_byte ||
           current_token == p_word) {
      if (current_token == p_struct) { // Struct global

        lexer();

        if (current_token == p_pointer) { // Defining a struct pointer

          lexer();
          if (current_token != p_id)
            c_error(1, 110);
          ob = o;
          if ((*ob).type == tnone)
            c_error(0, 150); // Pointer can't be defined this way
          if ((*ob).type != tsglo && (*ob).type != tsloc)
            c_error(0, 150);
          lexer();
puntero_a_struct:
          analyze_pointer_struct(tpsgl, imem, ob);
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, imem++);

          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero_a_struct;
          } else {
            if (!free_sintax)
              if (current_token != p_ptocoma)
                c_error(3, 137);
            while (current_token == p_ptocoma || current_token == p_coma)
              lexer();
          }

        } else {
          if (current_token != p_id)
            c_error(1, 110);
          ob = o;
          member = ob;
          lexer();

          if ((*ob).type != tnone)
            c_error(2, 22);

          (*ob).type = tsglo;
          (*ob).struct_global.offset = _imem = imem;
          if (current_token == p_corab) {
            member2 = member;
            member = NULL;
            lexer();
            if (((*ob).struct_global.dim1 = constant()) < 0)
              c_error(4, 123);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).struct_global.dim2 = constant()) < 0)
                c_error(4, 123);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).struct_global.dim3 = constant()) < 0)
                  c_error(4, 123);
              } else
                (*ob).struct_global.dim3 = -1;
            } else {
              (*ob).struct_global.dim2 = -1;
              (*ob).struct_global.dim3 = -1;
            }
            member = member2;
            (*ob).struct_global.total_items = (*ob).struct_global.dim1 + 1;
            if ((*ob).struct_global.dim2 > -1)
              (*ob).struct_global.total_items *= (*ob).struct_global.dim2 + 1;
            if ((*ob).struct_global.dim3 > -1)
              (*ob).struct_global.total_items *= (*ob).struct_global.dim3 + 1;
            if (current_token != p_corce) {
              c_error(3, 26);
            }
            lexer();
          } else {
            (*ob).struct_global.total_items = 1;
            (*ob).struct_global.dim1 = 0;
            (*ob).struct_global.dim2 = -1;
            (*ob).struct_global.dim3 = -1;
          }
          if (((*ob).struct_global.item_len = analyze_struct(_imem)) == 0)
            c_error(0, 57);
          member = NULL;
          lexer();
          imem = (*ob).struct_global.offset;
          dup = (*ob).struct_global.total_items + 1;
          if (dup > 1) {
            test_buffer(&mem, &imem_max,
                        imem + (*ob).struct_global.item_len * (*ob).struct_global.total_items);
            test_buffer(&frm, &ifrm_max,
                        imem + (*ob).struct_global.item_len * (*ob).struct_global.total_items);
            while (--dup) {
              if (imem != _imem) {
                memcpy(&mem[imem], &mem[_imem], (*ob).struct_global.item_len << 2);
                memcpy(&frm[imem], &frm[_imem], (*ob).struct_global.item_len << 2);
              }
              imem += (*ob).struct_global.item_len;
            }
          }
          imem = _imem;
          if (current_token == p_asig) {
            save_error(1);
            lexer();
            tglo_init(0);
            if (imem - _imem - 1 >= (*ob).struct_global.item_len * (*ob).struct_global.total_items)
              c_error(4, 120);
          }
          while (current_token == p_ptocoma)
            lexer();

          imem = _imem + (*ob).struct_global.item_len * (*ob).struct_global.total_items;
          test_buffer(&mem, &imem_max, imem);
        }

      } else if (current_token == p_string) { // Global string

        lexer();

        if (current_token == p_pointer) { // Byte pointer

          lexer();
puntero1:
          ob = analyze_pointer(tpcgl, imem);
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero1;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 125);
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 126);
          lexer();
          (*ob).type = tcglo;
          _imem = imem;
          (*ob).string_global.offset = _imem + 1;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              (*ob).string_global.total_len = 255;
            } else {
              if (((*ob).string_global.total_len = constant()) < 0)
                c_error(4, 127);
              if ((*ob).string_global.total_len > 0xFFFFF)
                c_error(4, 135);
              if (current_token != p_corce)
                c_error(3, 26);
              lexer();
            }
          } else
            (*ob).string_global.total_len = 255;
          if (current_token == p_asig) {
            save_error(1);
            _itxt = itxt;
            lexer();
            if (current_token != p_lit &&
                !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
              c_error(3, 128);
            if (strlen((char *)&mem[token_value]) > (*ob).string_global.total_len + 1)
              c_error(4, 129);
            imem = _imem + 1 +
                   ((*ob).string_global.total_len + 5) / 4; // e.g. c[32] -> c[0]..c[32],NUL
            test_buffer(&mem, &imem_max, imem);
            memmove((char *)&mem[_imem + 1], (char *)&mem[token_value],
                    strlen((char *)&mem[token_value]) + 1);
            itxt = _itxt; // Remove the string from the text segment
            lexer();
          } else {
            imem = _imem + 1 + ((*ob).string_global.total_len + 5) / 4;
            test_buffer(&mem, &imem_max, imem);
          }
          mem[_imem] = 0xDAD00000 | (*ob).string_global.total_len;
        }

        if (current_token == p_coma)
          current_token = p_string;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_byte) { // Global byte array

        lexer();

        if (current_token == p_pointer) { // Byte pointer

          lexer();
puntero2:
          ob = analyze_pointer(tpbgl, imem);
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero2;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 142);
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 117);
          lexer();
          (*ob).type = tbglo;
          (*ob).byte_global.offset = _imem = imem;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              oimemptr = (byte *)&mem[imem];
              tglo_init(2);
              (*ob).byte_global.len1 = imemptr - oimemptr - 1;
              (*ob).byte_global.len2 = -1;
              (*ob).byte_global.len3 = -1;
              (*ob).byte_global.total_len = ((*ob).byte_global.len1 + 4) / 4;
            } else {
              if (((*ob).byte_global.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).byte_global.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).byte_global.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).byte_global.len3 = -1;
              } else {
                (*ob).byte_global.len2 = -1;
                (*ob).byte_global.len3 = -1;
              }

              (*ob).byte_global.total_len = (*ob).byte_global.len1 + 1;
              if ((*ob).byte_global.len2 > -1)
                (*ob).byte_global.total_len *= (*ob).byte_global.len2 + 1;
              if ((*ob).byte_global.len3 > -1)
                (*ob).byte_global.total_len *= (*ob).byte_global.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                oimemptr = (byte *)&mem[imem];
                tglo_init(2);
                if (imemptr - oimemptr > (*ob).byte_global.total_len)
                  c_error(4, 33);
              }
              (*ob).byte_global.total_len = ((*ob).byte_global.total_len + 3) / 4;
            }
          } else { // Global byte
            (*ob).type = tbglo;
            (*ob).byte_global.offset = imem;
            (*ob).byte_global.len1 = 0;
            (*ob).byte_global.len2 = -1;
            (*ob).byte_global.len3 = -1;
            (*ob).byte_global.total_len = 1; // 1 int
            if (current_token == p_asig) {
              save_error(1);
              lexer();
              mem[imem] = constant();
              if (mem[imem] < 0 || mem[imem] > 255)
                c_error(4, 143);
            }
          }
          imem = _imem + (*ob).byte_global.total_len;
          test_buffer(&mem, &imem_max, imem);
        }

        if (current_token == p_coma)
          current_token = p_byte;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_word) { // Global word array

        lexer();

        if (current_token == p_pointer) { // Word pointer

          lexer();
puntero3:
          ob = analyze_pointer(tpwgl, imem);
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero3;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 145);
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 117);
          lexer();
          (*ob).type = twglo;
          (*ob).word_global.offset = _imem = imem;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              oimemptr = (byte *)&mem[imem];
              tglo_init(1);
              (*ob).word_global.len1 = (imemptr - oimemptr) / 2 - 1;
              (*ob).word_global.len2 = -1;
              (*ob).word_global.len3 = -1;
              (*ob).word_global.total_len = ((*ob).word_global.len1 + 2) / 2;
            } else {
              if (((*ob).word_global.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).word_global.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).word_global.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).word_global.len3 = -1;
              } else {
                (*ob).word_global.len2 = -1;
                (*ob).word_global.len3 = -1;
              }

              (*ob).word_global.total_len = (*ob).word_global.len1 + 1;
              if ((*ob).word_global.len2 > -1)
                (*ob).word_global.total_len *= (*ob).word_global.len2 + 1;
              if ((*ob).word_global.len3 > -1)
                (*ob).word_global.total_len *= (*ob).word_global.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                oimemptr = (byte *)&mem[imem];
                tglo_init(1);
                if (imemptr - oimemptr > (*ob).word_global.total_len * 2)
                  c_error(4, 33);
              }
              (*ob).word_global.total_len = ((*ob).word_global.total_len + 1) / 2;
            }
          } else { // Global word
            (*ob).type = twglo;
            (*ob).word_global.offset = imem;
            (*ob).word_global.len1 = 0;
            (*ob).word_global.len2 = -1;
            (*ob).word_global.len3 = -1;
            (*ob).word_global.total_len = 1; // 1 int
            if (current_token == p_asig) {
              save_error(1);
              lexer();
              mem[imem] = constant();
              if (mem[imem] < 0 || mem[imem] > 65535)
                c_error(4, 144);
            }
          }
          imem = _imem + (*ob).word_global.total_len;
          test_buffer(&mem, &imem_max, imem);
        }

        if (current_token == p_coma)
          current_token = p_word;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_int || current_token == p_id || current_token == p_pointer) {
        if (current_token == p_int) {
          lexer();
          if (current_token != p_id && current_token != p_pointer)
            c_error(1, 149);
        }

        if (current_token == p_pointer) { // Int pointer

          lexer();
puntero4:
          ob = analyze_pointer(tpigl, imem);
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero4;
          }

        } else {
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 117);
          lexer();
          if (current_token == p_corab) { // Global table
            lexer();
            (*ob).type = ttglo;
            (*ob).table_global.offset = _imem = imem;
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              tglo_init(3);
              (*ob).table_global.len1 = imem - _imem - 1;
              (*ob).table_global.len2 = -1;
              (*ob).table_global.len3 = -1;
              (*ob).table_global.total_len = imem - _imem;
            } else {
              if (((*ob).table_global.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).table_global.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).table_global.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).table_global.len3 = -1;
              } else {
                (*ob).table_global.len2 = -1;
                (*ob).table_global.len3 = -1;
              }
              (*ob).table_global.total_len = (*ob).table_global.len1 + 1;
              if ((*ob).table_global.len2 > -1)
                (*ob).table_global.total_len *= (*ob).table_global.len2 + 1;
              if ((*ob).table_global.len3 > -1)
                (*ob).table_global.total_len *= (*ob).table_global.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                tglo_init(3);
                if (imem - _imem > (*ob).table_global.total_len)
                  c_error(4, 33);
              }
            }
            imem = _imem + (*ob).table_global.total_len;
            test_buffer(&mem, &imem_max, imem);

          } else { // Global variable

            (*ob).type = tvglo;
            (*ob).var_global.offset = imem;
            if (current_token == p_asig) {
              lexer();
              mem[imem] = constant();
            }
            test_buffer(&mem, &imem_max, ++imem);
          }
        }
        if (!free_sintax)
          if (current_token != p_ptocoma && current_token != p_coma)
            c_error(3, 66);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }
    }
  }

  //---------------------------------------------------------------------------
  // Local variables, cannot be used in constant expressions
  //---------------------------------------------------------------------------
  // <variable> := p_id [ p_asig <exp> ] {;}
  //
  // <tabla> := p_id p_corab ( p_corce p_asig <init>
  //                       | <exp> p_corce [ p_asig <init> ] ) {;}
  //---------------------------------------------------------------------------

  if (current_token == p_local) {
    skip_semicolons();
    while (current_token == p_id || current_token == p_int || current_token == p_pointer ||
           current_token == p_struct || current_token == p_string || current_token == p_byte ||
           current_token == p_word) {
      if (current_token == p_struct) {
        lexer();

        if (current_token == p_pointer) { // Defining a struct pointer

          lexer();
          if (current_token != p_id)
            c_error(1, 110);
          ob = o;
          if ((*ob).type == tnone)
            c_error(0, 150); // Pointer can't be defined this way
          if ((*ob).type != tsglo && (*ob).type != tsloc)
            c_error(0, 150);
          lexer();
puntero_a_struct_local:
          analyze_pointer_struct(tpslo, iloc, ob);
          if (current_token == p_asig) {
            lexer();
            loc[iloc] = constant();
          }
          test_buffer(&loc, &iloc_max, iloc++);

          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero_a_struct_local;
          } else {
            if (!free_sintax)
              if (current_token != p_ptocoma)
                c_error(3, 137);
            while (current_token == p_ptocoma || current_token == p_coma)
              lexer();
          }

        } else {
          if (current_token != p_id)
            c_error(1, 110);
          ob = o;
          member = ob;
          lexer();

          if ((*ob).type != tnone)
            c_error(2, 22);

          (*ob).type = tsloc;
          (*ob).struct_local.offset = _imem = iloc;
          if (current_token == p_corab) {
            member2 = member;
            member = NULL;
            lexer();
            if (((*ob).struct_local.dim1 = constant()) < 0)
              c_error(4, 123);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).struct_local.dim2 = constant()) < 0)
                c_error(4, 123);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).struct_local.dim3 = constant()) < 0)
                  c_error(4, 123);
              } else
                (*ob).struct_local.dim3 = -1;
            } else {
              (*ob).struct_local.dim2 = -1;
              (*ob).struct_local.dim3 = -1;
            }
            member = member2;
            (*ob).struct_local.total_items = (*ob).struct_local.dim1 + 1;
            if ((*ob).struct_local.dim2 > -1)
              (*ob).struct_local.total_items *= (*ob).struct_local.dim2 + 1;
            if ((*ob).struct_local.dim3 > -1)
              (*ob).struct_local.total_items *= (*ob).struct_local.dim3 + 1;
            if (current_token != p_corce) {
              c_error(3, 26);
            }
            lexer();
          } else {
            (*ob).struct_local.total_items = 1;
            (*ob).struct_local.dim1 = 0;
            (*ob).struct_local.dim2 = -1;
            (*ob).struct_local.dim3 = -1;
          }
          if (((*ob).struct_local.item_len = analyze_struct_local(_imem)) == 0)
            c_error(0, 57);
          member = NULL;
          lexer();
          iloc = (*ob).struct_local.offset;
          dup = (*ob).struct_local.total_items + 1;
          if (dup > 1) {
            test_buffer(&loc, &iloc_max,
                        iloc + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
            test_buffer(&frm, &ifrm_max,
                        imem + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
            while (--dup) {
              memcpy(&loc[iloc], &loc[_imem], (*ob).struct_local.item_len << 2);
              memcpy(&frm[iloc], &frm[_imem], (*ob).struct_local.item_len << 2);
              iloc += (*ob).struct_local.item_len;
            }
          }
          iloc = _imem;
          if (current_token == p_asig) {
            save_error(1);
            lexer();
            tloc_init(0);
            if (iloc - _imem - 1 >= (*ob).struct_local.item_len * (*ob).struct_local.total_items)
              c_error(4, 120);
          }
          while (current_token == p_ptocoma)
            lexer();
          iloc = _imem + (*ob).struct_local.item_len * (*ob).struct_local.total_items;
          test_buffer(&loc, &iloc_max, iloc);
        }

      } else if (current_token == p_string) { // Local string

        lexer();

        if (current_token == p_pointer) { // Byte pointer

          lexer();
puntero5:
          ob = analyze_pointer(tpclo, iloc);
          if (current_token == p_asig) {
            lexer();
            loc[iloc] = constant();
          }
          test_buffer(&loc, &iloc_max, ++iloc);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero5;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 125);
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 126);
          lexer();
          (*ob).type = tcloc;
          _imem = iloc;
          (*ob).string_local.offset = _imem + 1;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              (*ob).string_local.total_len = 255;
            } else {
              if (((*ob).string_local.total_len = constant()) < 0)
                c_error(4, 127);
              if ((*ob).string_local.total_len > 0xFFFFF)
                c_error(4, 135);
              if (current_token != p_corce)
                c_error(3, 26);
              lexer();
            }
          } else
            (*ob).string_local.total_len = 255;
          if (current_token == p_asig) {
            save_error(1);
            _itxt = itxt;
            lexer();
            if (current_token != p_lit &&
                !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
              c_error(3, 128);
            if (strlen((char *)&mem[token_value]) > (*ob).string_local.total_len + 1)
              c_error(4, 129);
            iloc =
                _imem + 1 + ((*ob).string_local.total_len + 5) / 4; // e.g. c[32] -> c[0]..c[32],NUL
            test_buffer(&loc, &iloc_max, iloc);
            div_strcpy((char *)&loc[_imem + 1], (*ob).string_local.total_len + 1,
                       (char *)&mem[token_value]);
            itxt = _itxt; // Remove the string from the text segment
            lexer();
          } else {
            iloc = _imem + 1 + ((*ob).string_local.total_len + 5) / 4;
            test_buffer(&loc, &iloc_max, iloc);
          }
          loc[_imem] = 0xDAD00000 | (*ob).string_local.total_len;
        }

        if (current_token == p_coma)
          current_token = p_string;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_byte) { // Local byte array

        lexer();

        if (current_token == p_pointer) { // Byte pointer

          lexer();
puntero6:
          ob = analyze_pointer(tpblo, iloc);
          if (current_token == p_asig) {
            lexer();
            loc[iloc] = constant();
          }
          test_buffer(&loc, &iloc_max, ++iloc);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero6;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 142);
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 117);
          lexer();
          (*ob).type = tbloc;
          (*ob).byte_local.offset = _imem = iloc;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              oimemptr = (byte *)&loc[iloc];
              tloc_init(2);
              (*ob).byte_local.len1 = imemptr - oimemptr - 1;
              (*ob).byte_local.len2 = -1;
              (*ob).byte_local.len3 = -1;
              (*ob).byte_local.total_len = ((*ob).byte_local.len1 + 4) / 4;
            } else {
              if (((*ob).byte_local.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).byte_local.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).byte_local.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).byte_local.len3 = -1;
              } else {
                (*ob).byte_local.len2 = -1;
                (*ob).byte_local.len3 = -1;
              }

              (*ob).byte_local.total_len = (*ob).byte_local.len1 + 1;
              if ((*ob).byte_local.len2 > -1)
                (*ob).byte_local.total_len *= (*ob).byte_local.len2 + 1;
              if ((*ob).byte_local.len3 > -1)
                (*ob).byte_local.total_len *= (*ob).byte_local.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                oimemptr = (byte *)&loc[iloc];
                tloc_init(2);
                if (imemptr - oimemptr > (*ob).byte_local.total_len)
                  c_error(4, 33);
              }
              (*ob).byte_local.total_len = ((*ob).byte_local.total_len + 3) / 4;
            }
          } else { // Byte local
            (*ob).type = tbloc;
            (*ob).byte_local.offset = iloc;
            (*ob).byte_local.len1 = 0;
            (*ob).byte_local.len2 = -1;
            (*ob).byte_local.len3 = -1;
            (*ob).byte_local.total_len = 1; // 1 int
            if (current_token == p_asig) {
              save_error(1);
              lexer();
              loc[iloc] = constant();
              if (loc[iloc] < 0 || loc[iloc] > 255)
                c_error(4, 143);
            }
          }
          iloc = _imem + (*ob).byte_local.total_len;
          test_buffer(&loc, &iloc_max, iloc);
        }

        if (current_token == p_coma)
          current_token = p_byte;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_word) { // Local word array

        lexer();

        if (current_token == p_pointer) { // Word pointer

          lexer();
puntero7:
          ob = analyze_pointer(tpwlo, iloc);
          if (current_token == p_asig) {
            lexer();
            loc[iloc] = constant();
          }
          test_buffer(&loc, &iloc_max, ++iloc);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero7;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 145);
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 117);
          lexer();
          (*ob).type = twloc;
          (*ob).word_local.offset = _imem = iloc;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              oimemptr = (byte *)&loc[iloc];
              tloc_init(1);
              (*ob).word_local.len1 = (imemptr - oimemptr) / 2 - 1;
              (*ob).word_local.len2 = -1;
              (*ob).word_local.len3 = -1;
              (*ob).word_local.total_len = ((*ob).word_local.len1 + 2) / 2;
            } else {
              if (((*ob).word_local.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).word_local.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).word_local.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).word_local.len3 = -1;
              } else {
                (*ob).word_local.len2 = -1;
                (*ob).word_local.len3 = -1;
              }

              (*ob).word_local.total_len = (*ob).word_local.len1 + 1;
              if ((*ob).word_local.len2 > -1)
                (*ob).word_local.total_len *= (*ob).word_local.len2 + 1;
              if ((*ob).word_local.len3 > -1)
                (*ob).word_local.total_len *= (*ob).word_local.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                oimemptr = (byte *)&loc[iloc];
                tloc_init(1);
                if (imemptr - oimemptr > (*ob).word_local.total_len * 2)
                  c_error(4, 33);
              }
              (*ob).word_local.total_len = ((*ob).word_local.total_len + 1) / 2;
            }
          } else { // Word local
            (*ob).type = twloc;
            (*ob).word_local.offset = iloc;
            (*ob).word_local.len1 = 0;
            (*ob).word_local.len2 = -1;
            (*ob).word_local.len3 = -1;
            (*ob).word_local.total_len = 1; // 1 int
            if (current_token == p_asig) {
              save_error(1);
              lexer();
              loc[iloc] = constant();
              if (loc[iloc] < 0 || loc[iloc] > 65535)
                c_error(4, 144);
            }
          }
          iloc = _imem + (*ob).word_local.total_len;
          test_buffer(&loc, &iloc_max, iloc);
        }

        if (current_token == p_coma)
          current_token = p_word;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_int || current_token == p_id || current_token == p_pointer) {
        if (current_token == p_int) {
          lexer();
          if (current_token != p_id && current_token != p_pointer)
            c_error(1, 149);
        }

        if (current_token == p_pointer) { // Int pointer member

          lexer();
puntero8:
          ob = analyze_pointer(tpilo, iloc);
          if (current_token == p_asig) {
            lexer();
            loc[iloc] = constant();
          }
          test_buffer(&loc, &iloc_max, ++iloc);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero8;
          }

        } else {
          ob = o;
          if ((*ob).type != tnone)
            c_error(0, 117);
          lexer();
          if (current_token == p_corab) { // Local table
            lexer();
            (*ob).type = ttloc;
            (*ob).table_local.offset = _imem = iloc;
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              tloc_init(3);
              (*ob).table_local.len1 = iloc - _imem - 1;
              (*ob).table_local.len2 = -1;
              (*ob).table_local.len3 = -1;
              (*ob).table_local.total_len = iloc - _imem;
            } else {
              if (((*ob).table_local.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).table_local.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).table_local.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).table_local.len3 = -1;
              } else {
                (*ob).table_local.len2 = -1;
                (*ob).table_local.len3 = -1;
              }
              (*ob).table_local.total_len = (*ob).table_local.len1 + 1;
              if ((*ob).table_local.len2 > -1)
                (*ob).table_local.total_len *= (*ob).table_local.len2 + 1;
              if ((*ob).table_local.len3 > -1)
                (*ob).table_local.total_len *= (*ob).table_local.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                tloc_init(3);
                if (iloc - _imem > (*ob).table_local.total_len)
                  c_error(4, 33);
              }
            }
            iloc = _imem + (*ob).table_local.total_len;
            test_buffer(&loc, &iloc_max, iloc);

          } else { // Variable local

            (*ob).type = tvloc;
            (*ob).var_local.offset = iloc;
            if (current_token == p_asig) {
              lexer();
              loc[iloc] = constant();
            }
            test_buffer(&loc, &iloc_max, ++iloc);
          }
        }
        if (!free_sintax)
          if (current_token != p_ptocoma && current_token != p_coma)
            c_error(3, 66);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }
    }
  }

  //---------------------------------------------------------------------------
  // Main code
  //---------------------------------------------------------------------------

  // Generate jump to start of code (HEADER_LENGTH)

  mem[0] = program_type;
  mem[1] = imem;
  local_var_len = iloc;

  g2(ltyp, (intptr_t)current_scope);
  g2(lcbp, 0);
  if (current_token == p_import)
    c_error(0, 147); // Warn DIV 1 users
  // The first FRAME and PRIVATE variable loading are executed
  // together in the BEGIN of the main program.

  end_addr = imem;

  in_params = -1;
  lexical_scope = current_scope;
  n = iloc;
  analyze_private();
  if (iloc > local_var_len) {
    local_var_len = iloc;
  }
  iloc = n;
  in_params = 0;

  g1(lfrm);

  if (current_token != p_begin)
    c_error(0, 29);
  statement_start();
  start_addr = end_addr;
  lexer();
  statement_end();
  record_statement();

  statement();

  if (current_token != p_end) {
    c_error(0, 40);
  }
  statement_start();
  g1(lret);
  skip_semicolons();
  statement_end();
  record_statement();

  //---------------------------------------------------------------------------
  // Processes
  //---------------------------------------------------------------------------
  // p_process p_id p_abrir [ id { , id } ] p_cerrar { ; } p_begin ... p_end
  //---------------------------------------------------------------------------

  save_error(0);
  while (current_token == p_process || current_token == p_function) {
    n = current_token;
    statement_start();
    lexer();
    if (current_token != p_id) {
      c_error(1, 111);
    }
    ob = o;
    lexer();
    if ((*ob).type == tproc && (*ob).used) {
      num_params = (*ob).proc.num_params;
      lexical_scope = current_scope = ob;
      _imem = (*ob).proc.offset;
      while (_imem) {
        _imem_old = mem[_imem];
        mem[_imem] = imem;
        _imem = _imem_old;
      }
    } else if ((*ob).type == tnone) {
      (*ob).used = 0;
      (*ob).type = tproc;
      (*ob).proc.scope = lexical_scope = current_scope = ob;
    } else
      c_error(2, 118);
    (*ob).proc.offset = imem;
    (*ob).proc.num_params = 0;

    g2(ltyp, (intptr_t)current_scope);
    if (n == p_function)
      g1(lnop);
    g2(lcbp, 0);
    _imem = imem - 1;
    if (current_token != p_abrir)
      c_error(3, 36);

    in_params = 1;
    n = iloc;
    lexer();

    while (current_token != p_cerrar) {
      (*ob).proc.num_params++;
      expression_cpa();
      if (current_token != p_cerrar) {
        if (current_token != p_coma)
          c_error(3, 25);
        else {
          lexer();
          if (current_token == p_cerrar)
            c_error(3, 45);
        }
      }
    }
    if ((*ob).used) {
      if (num_params == (*ob).proc.num_params)
        (*ob).used = 0;
      else
        c_error(0, 46);
    }

    skip_semicolons();
    statement_end();

    if (in_params > 1) {
      g2(lpar, in_params - 1);
    }

    in_params = -1; // Allow parameters to be redeclared as PRIVATE

    num_params = mem[_imem] = (*ob).proc.num_params;

    analyze_private();
    in_params = 0;

    if (iloc > local_var_len) {
      local_var_len = iloc;
    }
    iloc = n;

    if (current_token != p_begin) {
      c_error(0, 29);
    }
    lexer();
    end_addr = imem - 1;
    record_statement();
    statement();
    if (current_token != p_end) {
      c_error(0, 40);
    }
    statement_start();
    g1(lret);
    skip_semicolons();
    statement_end();
    record_statement();

    save_error(0);
  }

  if (current_token != p_end_of_file)
    c_error(4, 52);
}

//---------------------------------------------------------------------------
// Parse private variables of a block, like locals
//---------------------------------------------------------------------------

// WARNING: Remote access to private variables (#id.private_var) must not
//          be allowed, unless #id is the same process type as the current
//          one (i.e., a sibling process).

void analyze_private(void) {
  struct object *ob = NULL, *member2;
  int _imem, _imem_old, _itxt, dup;
  byte *oimemptr;

  if (current_token == p_private) {
    skip_semicolons();

    g2(lpri, 0);
    _imem_old = imem - 1;

    while (current_token == p_id || current_token == p_int || current_token == p_pointer ||
           current_token == p_struct || current_token == p_string || current_token == p_byte ||
           current_token == p_word) {
      if (current_token == p_struct) {
        lexer();

        if (current_token == p_pointer) { // Defining a struct pointer

          lexer();
          if (current_token != p_id)
            c_error(1, 110);
          ob = o;
          if ((*ob).type == tnone)
            c_error(0, 150); // Pointer can't be defined this way
          if ((*ob).type != tsglo && (*ob).type != tsloc)
            c_error(0, 150);
          lexer();
puntero_a_struct:
          if (analyze_pointer_struct(tpslo, iloc, ob) == 1) {
            iloc++;
            if (current_token == p_asig) {
              lexer();
              mem[imem] = constant();
            }
            test_buffer(&mem, &imem_max, ++imem);
          }

          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero_a_struct;
          } else {
            if (!free_sintax)
              if (current_token != p_ptocoma)
                c_error(3, 137);
            while (current_token == p_ptocoma || current_token == p_coma)
              lexer();
          }

        } else {
          if (current_token != p_id)
            c_error(1, 110);
          ob = o;
          member = ob;
          lexer();

          if ((*ob).type != tnone)
            c_error(2, 22);

          (*ob).type = tsloc;
          (*ob).struct_local.offset = _imem = imem;
          if (current_token == p_corab) {
            member2 = member;
            member = NULL;
            lexer();
            if (((*ob).struct_local.dim1 = constant()) < 0)
              c_error(4, 123);
            if (current_token == p_coma) {
              lexer();
              if (((*ob).struct_local.dim2 = constant()) < 0)
                c_error(4, 123);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).struct_local.dim3 = constant()) < 0)
                  c_error(4, 123);
              } else
                (*ob).struct_local.dim3 = -1;
            } else {
              (*ob).struct_local.dim2 = -1;
              (*ob).struct_local.dim3 = -1;
            }
            member = member2;
            (*ob).struct_local.total_items = (*ob).struct_local.dim1 + 1;
            if ((*ob).struct_local.dim2 > -1)
              (*ob).struct_local.total_items *= (*ob).struct_local.dim2 + 1;
            if ((*ob).struct_local.dim3 > -1)
              (*ob).struct_local.total_items *= (*ob).struct_local.dim3 + 1;
            if (current_token != p_corce) {
              c_error(3, 26);
            }
            lexer();
          } else {
            (*ob).struct_local.total_items = 1;
            (*ob).struct_local.dim1 = 0;
            (*ob).struct_local.dim2 = -1;
            (*ob).struct_local.dim3 = -1;
          }
          if (((*ob).struct_local.item_len = analyze_struct_private(_imem)) == 0)
            c_error(0, 57);

          member = NULL;
          lexer();

          imem = (*ob).struct_local.offset;
          dup = (*ob).struct_local.total_items + 1;
          if (dup > 1) {
            test_buffer(&mem, &imem_max,
                        imem + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
            test_buffer(&frm, &ifrm_max,
                        imem + (*ob).struct_local.item_len * (*ob).struct_local.total_items);
            while (--dup) {
              if (imem != _imem) {
                memcpy(&mem[imem], &mem[_imem], (*ob).struct_local.item_len << 2);
                memcpy(&frm[imem], &frm[_imem], (*ob).struct_local.item_len << 2);
              }
              imem += (*ob).struct_local.item_len;
            }
          }
          imem = _imem;

          if (current_token == p_asig) {
            save_error(1);
            lexer();
            tglo_init(0);
            if (imem - _imem - 1 >= (*ob).struct_local.item_len * (*ob).struct_local.total_items)
              c_error(4, 120);
          }
          while (current_token == p_ptocoma)
            lexer();

          imem = _imem + (*ob).struct_local.item_len * (*ob).struct_local.total_items;
          (*ob).struct_local.offset = iloc;
          iloc += (*ob).struct_local.item_len * (*ob).struct_local.total_items;
          test_buffer(&mem, &imem_max, imem);
        }

      } else if (current_token == p_string) { // Private string

        lexer();

        if (current_token == p_pointer) { // Byte pointer

          lexer();
puntero1:
          ob = analyze_pointer(tpclo, iloc);
          if (ob == NULL)
            continue;
          else
            iloc++;
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero1;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 125);
          ob = o;
          if ((*ob).type != tnone) { // Check if a parameter is redeclared ...
            if (in_params == -1 && (*ob).param == 1 && (*ob).scope == current_scope) {
              if ((*ob).type == tcloc) { // String parameter redeclared
                save_error(0);
                lexer();
                if (current_token == p_corab) {
                  lexer();
                  if (current_token == p_corce) {
                    lexer();
                    dup = 255;
                  } else {
                    dup = constant();
                    if (current_token != p_corce)
                      c_error(3, 26);
                    lexer();
                  }
                } else
                  dup = 255;
                if (dup != (*ob).string_local.total_len)
                  c_error(4, 141);
                else if (current_token == p_asig)
                  c_error(0, 54);
                else {
                  while (current_token == p_ptocoma || current_token == p_coma)
                    lexer();
                  (*ob).param++;
                  continue;
                }
              } else
                c_error(0, 126);
            } else
              c_error(0, 126);
          } else
            lexer();

          (*ob).type = tcloc;
          _imem = imem;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              (*ob).string_local.total_len = 255;
            } else {
              if (((*ob).string_local.total_len = constant()) < 0)
                c_error(4, 127);
              if ((*ob).string_local.total_len > 0xFFFFF)
                c_error(4, 135);
              if (current_token != p_corce)
                c_error(3, 26);
              lexer();
            }
          } else
            (*ob).string_local.total_len = 255;
          if (current_token == p_asig) {
            save_error(1);
            _itxt = itxt;
            lexer();
            if (current_token != p_lit &&
                !(current_token == p_id && (*o).type == tcons && (*o).cons.literal))
              c_error(3, 128);
            if (strlen((char *)&mem[token_value]) > (*ob).string_local.total_len + 1)
              c_error(4, 129);
            imem =
                _imem + 1 + ((*ob).string_local.total_len + 5) / 4; // e.g. c[32] -> c[0]..c[32],NUL
            test_buffer(&mem, &imem_max, imem);
            memmove((char *)&mem[_imem + 1], (char *)&mem[token_value],
                    strlen((char *)&mem[token_value]) + 1);
            itxt = _itxt; // Remove the string from the text segment
            lexer();
          } else {
            imem = _imem + 1 + ((*ob).string_local.total_len + 5) / 4;
            test_buffer(&mem, &imem_max, imem);
          }
          mem[_imem] = 0xDAD00000 | (*ob).string_local.total_len;
          (*ob).string_local.offset = iloc + 1;
          iloc += 1 + ((*ob).string_local.total_len + 5) / 4;
        }

        if (current_token == p_coma)
          current_token = p_string;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_byte) { // Private byte array

        lexer();

        if (current_token == p_pointer) { // Byte pointer

          lexer();
puntero2:
          ob = analyze_pointer(tpblo, iloc);
          if (ob == NULL)
            continue;
          else
            iloc++;
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero2;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 142);
          ob = o;
          if ((*ob).type != tnone) {
            if (in_params == -1 && (*ob).param == 1 && (*ob).scope == current_scope) {
              if ((*ob).type == tbloc) { // Byte parameter redeclared
                lexer();
                if (current_token == p_corab)
                  c_error(2, 53);
                else if (current_token == p_asig)
                  c_error(0, 54);
                else {
                  while (current_token == p_ptocoma || current_token == p_coma) {
                    lexer();
                  }
                  (*ob).param++;
                  continue;
                }
              } else
                c_error(0, 117);
            } else
              c_error(0, 117);
          } else
            lexer();

          (*ob).type = tbloc;
          _imem = imem;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              oimemptr = (byte *)&mem[imem];
              tglo_init(2);
              (*ob).byte_local.len1 = imemptr - oimemptr - 1;
              (*ob).byte_local.len2 = -1;
              (*ob).byte_local.len3 = -1;
              (*ob).byte_local.total_len = ((*ob).byte_local.len1 + 4) / 4;
            } else {
              if (((*ob).byte_local.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).byte_local.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).byte_local.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).byte_local.len3 = -1;
              } else {
                (*ob).byte_local.len2 = -1;
                (*ob).byte_local.len3 = -1;
              }

              (*ob).byte_local.total_len = (*ob).byte_local.len1 + 1;
              if ((*ob).byte_local.len2 > -1)
                (*ob).byte_local.total_len *= (*ob).byte_local.len2 + 1;
              if ((*ob).byte_local.len3 > -1)
                (*ob).byte_local.total_len *= (*ob).byte_local.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                oimemptr = (byte *)&mem[imem];
                tglo_init(2);
                if (imemptr - oimemptr > (*ob).byte_local.total_len)
                  c_error(4, 33);
              }
              (*ob).byte_local.total_len = ((*ob).byte_local.total_len + 3) / 4;
            }
          } else { // Private byte
            (*ob).type = tbloc;
            (*ob).byte_local.offset = imem;
            (*ob).byte_local.len1 = 0;
            (*ob).byte_local.len2 = -1;
            (*ob).byte_local.len3 = -1;
            (*ob).byte_local.total_len = 1; // 1 int
            if (current_token == p_asig) {
              save_error(1);
              lexer();
              mem[imem] = constant();
              if (mem[imem] < 0 || mem[imem] > 255)
                c_error(4, 143);
            }
          }
          imem = _imem + (*ob).byte_local.total_len;
          test_buffer(&mem, &imem_max, imem);
          (*ob).byte_local.offset = iloc;
          iloc += (*ob).byte_local.total_len;
        }

        if (current_token == p_coma)
          current_token = p_byte;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_word) { // Private word array

        lexer();

        if (current_token == p_pointer) { // Word pointer

          lexer();
puntero3:
          ob = analyze_pointer(tpwlo, iloc);
          if (ob == NULL)
            continue;
          else
            iloc++;
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero3;
          }

        } else {
          if (current_token != p_id)
            c_error(1, 145);
          ob = o;
          if ((*ob).type != tnone) {
            if (in_params == -1 && (*ob).param == 1 && (*ob).scope == current_scope) {
              if ((*ob).type == twloc) { // Word parameter redeclared
                lexer();
                if (current_token == p_corab)
                  c_error(2, 53);
                else if (current_token == p_asig)
                  c_error(0, 54);
                else {
                  while (current_token == p_ptocoma || current_token == p_coma) {
                    lexer();
                  }
                  (*ob).param++;
                  continue;
                }
              } else
                c_error(0, 117);
            } else
              c_error(0, 117);
          } else
            lexer();

          (*ob).type = twloc;
          _imem = imem;
          if (current_token == p_corab) {
            lexer();
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              oimemptr = (byte *)&mem[imem];
              tglo_init(1);
              (*ob).word_local.len1 = (imemptr - oimemptr) / 2 - 1;
              (*ob).word_local.len2 = -1;
              (*ob).word_local.len3 = -1;
              (*ob).word_local.total_len = ((*ob).word_local.len1 + 2) / 2;
            } else {
              if (((*ob).word_local.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).word_local.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).word_local.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).word_local.len3 = -1;
              } else {
                (*ob).word_local.len2 = -1;
                (*ob).word_local.len3 = -1;
              }

              (*ob).word_local.total_len = (*ob).word_local.len1 + 1;
              if ((*ob).word_local.len2 > -1)
                (*ob).word_local.total_len *= (*ob).word_local.len2 + 1;
              if ((*ob).word_local.len3 > -1)
                (*ob).word_local.total_len *= (*ob).word_local.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                oimemptr = (byte *)&mem[imem];
                tglo_init(1);
                if (imemptr - oimemptr > (*ob).word_local.total_len * 2)
                  c_error(4, 33);
              }
              (*ob).word_local.total_len = ((*ob).word_local.total_len + 1) / 2;
            }
          } else { // Private word
            (*ob).type = twloc;
            (*ob).word_local.offset = imem;
            (*ob).word_local.len1 = 0;
            (*ob).word_local.len2 = -1;
            (*ob).word_local.len3 = -1;
            (*ob).word_local.total_len = 1; // 1 int
            if (current_token == p_asig) {
              save_error(1);
              lexer();
              mem[imem] = constant();
              if (mem[imem] < 0 || mem[imem] > 65535)
                c_error(4, 144);
            }
          }
          imem = _imem + (*ob).word_local.total_len;
          test_buffer(&mem, &imem_max, imem);
          (*ob).word_local.offset = iloc;
          iloc += (*ob).word_local.total_len;
        }

        if (current_token == p_coma)
          current_token = p_word;
        else {
          if (!free_sintax)
            if (current_token != p_ptocoma)
              c_error(3, 137);
          while (current_token == p_ptocoma || current_token == p_coma)
            lexer();
        }

      } else if (current_token == p_int || current_token == p_id || current_token == p_pointer) {
        if (current_token == p_int) {
          lexer();
          if (current_token != p_id && current_token != p_pointer)
            c_error(1, 149);
        }

        if (current_token == p_pointer) { // Int pointer member

          lexer();
puntero4:
          ob = analyze_pointer(tpilo, iloc);
          if (ob == NULL)
            continue;
          else
            iloc++;
          if (current_token == p_asig) {
            lexer();
            mem[imem] = constant();
          }
          test_buffer(&mem, &imem_max, ++imem);
          if (current_token == p_coma) {
            lexer();
            if (current_token == p_pointer)
              lexer();
            goto puntero4;
          }

        } else {
          // If the object is not tnone, it's a redeclared parameter or an error

          ob = o;
          if ((*ob).type != tnone) {
            if (in_params == -1 && (*ob).param == 1 && (*ob).scope == current_scope) {
              if ((*ob).type == tvloc) { // Local variable redeclared
                lexer();
                if (current_token == p_corab)
                  c_error(2, 53);
                else if (current_token == p_asig)
                  c_error(0, 54);
                else {
                  while (current_token == p_ptocoma || current_token == p_coma) {
                    lexer();
                  }
                  (*ob).param++;
                  continue;
                }
              } else
                c_error(0, 117);
            } else
              c_error(0, 117);
          } else
            lexer();

          if (current_token == p_corab) { // Private table
            lexer();
            (*ob).type = ttloc;
            _imem = imem;
            if (current_token == p_corce) {
              lexer();
              if (current_token != p_asig)
                c_error(3, 23);
              lexer();
              tglo_init(3);
              (*ob).table_local.len1 = imem - _imem - 1;
              (*ob).table_local.len2 = -1;
              (*ob).table_local.len3 = -1;
              (*ob).table_local.total_len = imem - _imem;
            } else {
              if (((*ob).table_local.len1 = constant()) < 0)
                c_error(4, 35);
              if (current_token == p_coma) {
                lexer();
                if (((*ob).table_local.len2 = constant()) < 0)
                  c_error(4, 35);
                if (current_token == p_coma) {
                  lexer();
                  if (((*ob).table_local.len3 = constant()) < 0)
                    c_error(4, 35);
                } else
                  (*ob).table_local.len3 = -1;
              } else {
                (*ob).table_local.len2 = -1;
                (*ob).table_local.len3 = -1;
              }
              (*ob).table_local.total_len = (*ob).table_local.len1 + 1;
              if ((*ob).table_local.len2 > -1)
                (*ob).table_local.total_len *= (*ob).table_local.len2 + 1;
              if ((*ob).table_local.len3 > -1)
                (*ob).table_local.total_len *= (*ob).table_local.len3 + 1;
              if (current_token != p_corce) {
                c_error(3, 26);
              }
              lexer();
              if (current_token == p_asig) {
                save_error(1);
                lexer();
                tglo_init(3);
                if (imem - _imem > (*ob).table_local.total_len)
                  c_error(4, 33);
              }
            }
            imem = _imem + (*ob).table_local.total_len;
            test_buffer(&mem, &imem_max, imem);
            (*ob).table_local.offset = iloc;
            iloc += (*ob).table_local.total_len;

          } else { // Private variable

            (*ob).type = tvloc;
            (*ob).var_local.offset = iloc++;
            if (current_token == p_asig) {
              lexer();
              mem[imem] = constant();
            }
            test_buffer(&mem, &imem_max, ++imem);
          }
        }
        if (!free_sintax)
          if (current_token != p_ptocoma && current_token != p_coma)
            c_error(3, 66);
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
      }
      (*ob).scope = current_scope;
    }

    mem[_imem_old] = imem;
  };
}

//---------------------------------------------------------------------------
// Table initialization (to mem[imem++])
//---------------------------------------------------------------------------
// <init>  := ( <string> | <exp> [ [p_dup] p_abrir <init> p_cerrar ] )
//            [ p_coma <init>]
//---------------------------------------------------------------------------


void tglo_init(memptrsize tipo) { // Type: 0-Int or mixed (struct), 1-Word, 2-Byte
  if (tipo == 3) {
    memset(frm, 0, ifrm_max * sizeof(memptrsize));
    tipo = 0;
  }
  imemptr = (byte *)&mem[imem];
  tglo_init2(tipo);
  if (!free_sintax)
    if (current_token != p_ptocoma)
      c_error(3, 66);
}

void tloc_init(int tipo) {
  int *ptr;

  if (tipo == 3) {
    memset(frm, 0, ifrm_max * sizeof(memptrsize));
    tipo = 0;
  }

  ptr = mem;
  mem = loc;
  loc = ptr;
  swap(imem, iloc);
  swap(imem_max, iloc_max);

  imemptr = (byte *)&mem[imem];
  tglo_init2(tipo);

  ptr = mem;
  mem = loc;
  loc = ptr;
  swap(imem, iloc);
  swap(imem_max, iloc_max);

  if (!free_sintax)
    if (current_token != p_ptocoma)
      c_error(3, 66);
}

// WARNING: tglo_init must NOT call test_buffer (buffer may be swapped via tloc_init).

void tglo_init2(int tipo) {
  int value, dup, _imem, len, n;
  byte *oimemptr;

  while (1) {
    // Check if data initialization ends

    if (current_token == p_cerrar || current_token == p_ptocoma) {
      if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 1 || tipo == 1) {
        imemptr += 2;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 1 && tipo != 1)
          imemptr = (byte *)&mem[imem];
      } else if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 2 || tipo == 2) {
        imemptr++;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 2 && tipo != 2)
          imemptr = (byte *)&mem[imem];
      } else {
        imemptr += 4;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
      }
      return;
    }

    // A lone comma, like "3,,4", advances one position (and defines a 0)

    if (current_token == p_coma) {
      if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 1 || tipo == 1) {
        (*(word *)imemptr) = 0;
        imemptr += 2;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 1 && tipo != 1)
          imemptr = (byte *)&mem[imem];
      } else if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 2 || tipo == 2) {
        *imemptr++ = 0;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 2 && tipo != 2)
          imemptr = (byte *)&mem[imem];
      } else {
        imemptr += 4;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
      }
      lexer();
      continue;
    }

    if (current_token == p_abrir) { // Dup x 1

      dup = 1;

    } else if (current_token == p_dup) { // Dup x 2

      dup = 2;
      lexer();
      if (current_token != p_abrir)
        c_error(3, 36);

    } else if (current_token == p_lit ||
               (current_token == p_id && (*o).type == tcons && (*o).cons.literal)) {
      value = token_value;
      lexer();
      if (current_token == p_abrir || current_token == p_dup)
        c_error(2, 38);

      // Store a literal in a string-type field

      if ((((memptrsize)imemptr - (memptrsize)mem) % 4) == 0) {
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (frm[imem] == 0xdad00000) {
          if (strlen((char *)&mem_ory[value]) > (mem[imem] & 0xFFFFF) + 1)
            c_error(2, 129);
          _imem = imem;
          imem += 1 + ((mem[imem] & 0xFFFFF) + 5) / 4;
          imemptr = (byte *)&mem[imem];
          div_strcpy((char *)&mem[_imem + 1], (mem[_imem] & 0xFFFFF) + 2, (char *)&mem_ory[value]);
          if (current_token != p_coma)
            return;
          lexer();
          continue;
        }
      }

      // Store a literal value in a non-string data type

      if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 1 ||
          tipo == 1) { // A string in words (short pointer)

        imemptr = (byte *)(((memptrsize)imemptr + 1) & -2); // Align to even
        *((word *)imemptr) = value;
        imemptr += 2;
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 1 && tipo != 1)
          imemptr = (byte *)&mem[imem];
        if (current_token != p_coma)
          return;
        lexer();
        continue;

      } else if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 2 ||
                 tipo == 2) { // A string in bytes

        oimemptr = imemptr;
        while (*(oimemptr - (memptrsize)mem + (memptrsize)frm) == 2)
          oimemptr++;
        if (tipo == 0)
          if (strlen((char *)&mem_ory[value]) > (memptrsize)(oimemptr - imemptr))
            c_error(2, 33);
        div_strcpy((char *)imemptr, (memptrsize)(oimemptr - imemptr) + 1, (char *)&mem_ory[value]);
        imemptr += strlen((char *)imemptr);
        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 2 && tipo != 2)
          imemptr = (byte *)&mem[imem];
        if (current_token != p_coma)
          return;
        lexer();
        continue;

      } else { // A string in an int (long pointer, as in DIV 1)

        imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
        mem[imem++] = value;
        imemptr = (byte *)&mem[imem];
        if (current_token != p_coma)
          return;
        lexer();
        continue;
      }

    } else {
      value = constant();
      if (current_token != p_abrir) {
        if (current_token != p_dup) {
          if ((((memptrsize)imemptr - (memptrsize)mem) % 4) == 0) {
            imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
            if (frm[imem] == 0xdad00000)
              c_error(2, 128);
          }

          // Store a numeric value in memory

          if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 1 || tipo == 1) { // Into a word

            if (value < 0 || value > 65535)
              c_error(2, 144);
            imemptr = (byte *)(((memptrsize)imemptr + 1) & -2); // Align to even
            *((word *)imemptr) = value;
            imemptr += 2;
            imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
            if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 1 && tipo != 1)
              imemptr = (byte *)&mem[imem];
            if (current_token != p_coma)
              return;
            lexer();
            continue;

          } else if (*(imemptr - (memptrsize)mem + (memptrsize)frm) == 2 ||
                     tipo == 2) { // Into a byte

            if (value < 0 || value > 255)
              c_error(2, 143);
            *imemptr++ = value;
            imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
            if (*(imemptr - (memptrsize)mem + (memptrsize)frm) != 2 && tipo != 2)
              imemptr = (byte *)&mem[imem];
            if (current_token != p_coma)
              return;
            lexer();
            continue;

          } else { // Into an int (the default in DIV 1)

            imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
            mem[imem++] = value;
            imemptr = (byte *)&mem[imem];
            if (current_token != p_coma)
              return;
            lexer();
            continue;
          }

        } else {
          dup = value;
          lexer();
          if (current_token != p_abrir)
            c_error(3, 36);
        }
      } else
        dup = value;
    }

    // Duplicate a sequence of values

    if (dup < 1)
      c_error(2, 38);
    lexer();

    // Parse the sequence as another initialization ...

    oimemptr = imemptr;
    tglo_init2(tipo);
    if (current_token != p_cerrar)
      c_error(3, 25);

    // And duplicate it the specified number of times

    if (dup > 1) {
      len = imemptr - oimemptr;
      while (--dup) {
        for (n = 0; n < len; n++) {
          // Check that strings align with themselves in dup()...
          if ((((memptrsize)imemptr - (memptrsize)mem) % 4) == 0) {
            imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
            _imem = ((memptrsize)oimemptr + n - (memptrsize)mem + 3) / 4;
            if (frm[imem] == 0xdad00000) {
              if (frm[_imem] != 0xdad00000)
                c_error(3, 136);
              else if (mem[imem] != mem[_imem])
                c_error(3, 136);
            } else if (frm[_imem] == 0xdad00000)
              c_error(3, 136);
          }

          // Check that words align with words
          if (*(byte *)((memptrsize)imemptr - (memptrsize)mem + (memptrsize)frm) == 1) {
            if (*(byte *)((memptrsize)oimemptr + n - (memptrsize)mem + (memptrsize)frm) != 1)
              c_error(3, 136);
          } else if (*(byte *)((memptrsize)oimemptr + n - (memptrsize)mem + (memptrsize)frm) == 1)
            c_error(3, 136);

          // Check that bytes align with bytes
          if (*(byte *)((memptrsize)imemptr - (memptrsize)mem + (memptrsize)frm) == 2) {
            if (*(byte *)((memptrsize)oimemptr + n - (memptrsize)mem + (memptrsize)frm) != 2)
              c_error(3, 136);
          } else if (*(byte *)((memptrsize)oimemptr + n - (memptrsize)mem + (memptrsize)frm) == 2)
            c_error(3, 136);

          *imemptr++ = *(oimemptr + n);
        }
      }
    }

    lexer();
    if (current_token != p_coma) {
      imem = ((memptrsize)imemptr - (memptrsize)mem + 3) / 4;
      return;
    }
    lexer();
  }
}

//-----------------------------------------------------------------------------
//      Statement block analysis;
//-----------------------------------------------------------------------------

void statement() {
  int im1, im2, im3, im4;
  int dir = 0, from, to, step;

  while (current_token >= p_return) {
    test_buffer(&mem, &imem_max, imem);
    switch (current_token) {
    case p_return:
      statement_start();
      lexer();
      if (current_token == p_abrir) {
        lexer();
        if (current_token != p_cerrar) {
          expression();
          if (current_token != p_cerrar)
            c_error(3, 25);
          g1(lrtf);
        } else {
          g1(lret);
        }
        lexer();
      } else {
        g1(lret);
      }
      if (!free_sintax)
        if (current_token != p_ptocoma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
      statement_end();
      record_statement();
      break;
    case p_if:
      statement_start();
      lexer();
      if (!free_sintax)
        if (current_token != p_abrir)
          c_error(3, 36);
      if (current_token == p_abrir)
        lexer();
      condition();
      if (!free_sintax)
        if (current_token != p_cerrar)
          c_error(3, 25);
      if (current_token == p_cerrar)
        lexer();
      g2(ljpf, 0);
      im1 = imem - 1;
      statement_end();
      record_statement();
      statement();
      if (current_token == p_else) {
        statement_start();
        lexer();
        g2(ljmp, 0);
        mem[im1] = imem;
        im1 = imem - 1;
        statement_end();
        record_statement();
        statement();
      }
      mem[im1] = imem;
      if (current_token != p_end)
        c_error(0, 40);
      lexer();
      break;
    case p_loop:
      tbreak[itbreak++] = 0;
      tcont[itcont++] = 0;
      lexer();
      im1 = imem;
      statement();
      if (current_token != p_end)
        c_error(0, 40);
      statement_start();
      lexer();
      g2(ljmp, im1);
      while (tbreak[--itbreak] != 0)
        mem[tbreak[itbreak]] = imem;
      while (tcont[--itcont] != 0)
        mem[tcont[itcont]] = im1;
      statement_end();
      record_statement();
      break;
    case p_while:
      statement_start();
      tbreak[itbreak++] = 0;
      tcont[itcont++] = 0;
      im1 = imem;
      lexer();
      if (!free_sintax)
        if (current_token != p_abrir)
          c_error(3, 36);
      if (current_token == p_abrir)
        lexer();
      condition();
      if (!free_sintax)
        if (current_token != p_cerrar)
          c_error(3, 25);
      if (current_token == p_cerrar)
        lexer();
      g2(ljpf, 0);
      im2 = imem - 1;
      statement_end();
      record_statement();
      statement();
      if (current_token != p_end) {
        c_error(0, 40);
      }
      statement_start();
      lexer();
      g2(ljmp, im1);
      mem[im2] = imem;
      while (tbreak[--itbreak] != 0)
        mem[tbreak[itbreak]] = imem;
      while (tcont[--itcont] != 0)
        mem[tcont[itcont]] = im1;
      statement_end();
      record_statement();
      break;
    case p_repeat:
      tbreak[itbreak++] = 0;
      tcont[itcont++] = 0;
      lexer();
      im1 = imem;
      statement();
      if (current_token != p_until)
        c_error(0, 42);
      statement_start();
      lexer();
      if (!free_sintax)
        if (current_token != p_abrir)
          c_error(3, 36);
      if (current_token == p_abrir)
        lexer();
      condition();
      if (!free_sintax)
        if (current_token != p_cerrar)
          c_error(3, 25);
      if (current_token == p_cerrar)
        lexer();
      g2(ljpf, im1);
      while (tbreak[--itbreak] != 0)
        mem[tbreak[itbreak]] = imem;
      while (tcont[--itcont] != 0)
        mem[tcont[itcont]] = im1;
      statement_end();
      record_statement();
      break;
    case p_from:
      statement_start();
      tbreak[itbreak++] = 0;
      tcont[itcont++] = 0;
      lexer();
      if (current_token != p_id)
        c_error(0, 67);

      if ((*o).type == tvglo) {
        dir = (*o).var_global.offset;
        g2(lcar, dir);
      } else if ((*o).type == tvloc && (!(*o).scope || (*o).scope == current_scope)) {
        dir = -(*o).var_local.offset;
        g2(lcar, -dir);
        g1(laid);
      } else
        c_error(0, 67);

      lexer();
      if (current_token != p_asig) {
        c_error(3, 68);
      }
      lexer();
      from = constant();
      if (current_token != p_to) {
        c_error(1, 69);
      }
      lexer();
      to = constant();
      if (from == to)
        c_error(4, 71);
      if (current_token == p_step) {
        lexer();
        step = constant();
        if (from < to && step <= 0)
          c_error(4, 70);
        if (from > to && step >= 0)
          c_error(4, 70);
      } else {
        if (from < to)
          step = 1;
        else
          step = -1;
      }
      g2(lcar, from); // Assign the from value
      g1(lasi);
      g1(lasp);

      im1 = imem; // Loop start

      if (dir >= 0) { // Loop continuation condition check
        g2(lcar, dir);
      } else {
        g2(lcar, -dir);
        g1(laid);
      }
      g1(lptr);
      g2(lcar, to);
      if (step > 0)
        g1(lmei);
      else
        g1(lmai);
      g2(ljpf, 0);
      im2 = imem - 1;

      statement_end();
      record_statement();
      if (!free_sintax) {
        if (current_token != p_ptocoma)
          c_error(3, 66);
      }
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();

      statement();
      if (current_token != p_end) {
        c_error(0, 40);
      }
      statement_start();
      lexer();

      im3 = imem; // Continue position

      if (dir >= 0) { // Increment and jump back to loop start
        g2(lcar, dir);
      } else {
        g2(lcar, -dir);
        g1(laid);
      }
      g2(lcar, step);
      g1(lada);
      g1(lasp);
      g2(ljmp, im1);
      mem[im2] = imem;

      while (tbreak[--itbreak] != 0)
        mem[tbreak[itbreak]] = imem;
      while (tcont[--itcont] != 0)
        mem[tcont[itcont]] = im3;
      statement_end();
      record_statement();

      break;

    case p_for:
      statement_start();
      tbreak[itbreak++] = 0;
      tcont[itcont++] = 0;
      lexer();
      if (current_token != p_abrir) {
        c_error(3, 36);
      }
      lexer();
      if (current_token != p_ptocoma) {
        expression();
        g1(lasp);
        while (current_token == p_coma) {
          lexer();
          expression();
          g1(lasp);
        }
      }
      im1 = imem;
      if (current_token != p_ptocoma) {
        c_error(3, 47);
      }
      lexer();
      if (current_token == p_ptocoma) {
        g2(lcar, 1);
      } else
        expression();
      g2(ljpf, 0);
      im2 = imem - 1;
      while (current_token == p_coma) {
        lexer();
        expression();
        g2(ljpf, im2);
        im2 = imem - 1;
      }
      g2(ljmp, 0);
      im3 = imem - 1;
      if (current_token != p_ptocoma) {
        c_error(3, 47);
      }
      lexer();
      if (current_token != p_cerrar) {
        expression();
        g1(lasp);
        while (current_token == p_coma) {
          lexer();
          expression();
          g1(lasp);
        }
      }
      g2(ljmp, im1);
      if (current_token != p_cerrar) {
        c_error(3, 25);
      }
      lexer();
      statement_end();
      record_statement();
      mem[im3++] = imem;
      statement();
      if (current_token != p_end) {
        c_error(0, 40);
      }
      statement_start();
      lexer();
      g2(ljmp, im3);
      do {
        im1 = mem[im2];
        mem[im2] = imem;
        im2 = im1;
      } while (im2);
      while (tbreak[--itbreak] != 0)
        mem[tbreak[itbreak]] = imem;
      while (tcont[--itcont] != 0)
        mem[tcont[itcont]] = im3;
      statement_end();
      record_statement();
      break;
    case p_switch:
      statement_start();
      lexer();
      if (!free_sintax)
        if (current_token != p_abrir)
          c_error(3, 36);
      if (current_token == p_abrir)
        lexer();
      condition();
      if (!free_sintax)
        if (current_token != p_cerrar)
          c_error(3, 25);
      if (current_token == p_cerrar)
        lexer();
      while (current_token == p_ptocoma) {
        lexer();
      }
      statement_end();
      record_statement();
      im1 = 0;
      im2 = 0;
      while (current_token != p_end) {
        statement_start();
        if (current_token == p_case) {
          im3 = 0;
          do {
            lexer();
            if (im1)
              mem[im1] = imem;
            expression();
            if (current_token != p_rango) {
              g2(lcse, 0);
              im1 = imem - 1;
            } else {
              lexer();
              expression();
              g2(lcsr, 0);
              im1 = imem - 1;
            }

            if (current_token == p_coma) {
              g2(ljmp, im3);
              im3 = imem - 1;
            }

          } while (current_token == p_coma);

          while (im3) {
            im4 = mem[im3];
            mem[im3] = imem;
            im3 = im4;
          }

        } else if (current_token == p_default) {
          lexer();
          if (im1) {
            mem[im1] = imem;
          }
          im1 = 0;
        } else
          c_error(0, 50);
        if (!free_sintax) {
          if (current_token != p_ptocoma)
            c_error(3, 65);
        }
        while (current_token == p_ptocoma || current_token == p_coma)
          lexer();
        g1(lasp);
        statement_end();
        record_statement();
        statement();
        if (current_token != p_end) {
          c_error(0, 51);
        }
        statement_start();
        g2(ljmp, im2);
        im2 = imem - 1;
        skip_semicolons();
        statement_end();
        record_statement();
      }
      statement_start();
      if (im1) {
        mem[im1] = imem;
      }
      g1(lasp);
      while (im2) {
        im1 = mem[im2];
        mem[im2] = imem;
        im2 = im1;
      }
      lexer();
      statement_end();
      record_statement();
      break;
    case p_frame:
      statement_start();
      lexer();
      if (current_token == p_abrir) {
        lexer();
        if (current_token != p_cerrar) {
          expression();
          if (current_token != p_cerrar)
            c_error(3, 25);
          g1(lfrf);
        } else {
          g1(lfrm);
        }
        lexer();
      } else {
        g1(lfrm);
      }
      if (!free_sintax)
        if (current_token != p_ptocoma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
      statement_end();
      record_statement();
      break;
    case p_debug:
      statement_start();
      g1(ldbg);
      lexer();
      if (!free_sintax)
        if (current_token != p_ptocoma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
      statement_end();
      record_statement();
      break;
    case p_break:
      statement_start();
      if (itbreak == 0) {
        c_error(0, 48);
      }
      lexer();
      if (!free_sintax) {
        if (current_token != p_ptocoma)
          c_error(3, 66);
      }
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
      g2(ljmp, 0);
      tbreak[itbreak++] = imem - 1;
      statement_end();
      record_statement();
      break;
    case p_continue:
      statement_start();
      if (itcont == 0) {
        c_error(0, 49);
      }
      lexer();
      if (!free_sintax) {
        if (current_token != p_ptocoma)
          c_error(3, 66);
      }
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
      g2(ljmp, 0);
      tcont[itcont++] = imem - 1;
      statement_end();
      record_statement();
      break;
    case p_clone:
      statement_start();
      lexer();
      g2(lclo, 0);
      im1 = imem - 1;
      statement_end();
      record_statement();
      statement();
      if (current_token != p_end) {
        c_error(0, 40);
      }
      lexer();
      mem[im1] = imem;
      break;
    case p_ptocoma:
      lexer();
      break;
    default:
      statement_start();
      error_27 = 124;
      expression();
      do {
        _exp--;
      } while ((*_exp).type == eoper && (*_exp).token == p_string);
      error_27 = 27;
      switch ((*_exp).type) {
      case ecall:
        break;
      case efunc:
        break;
      case efext:
        break;
      case eoper:
        switch ((*_exp).token) {
        case p_asig:
        case p_inc:
        case p_suma:
        case p_dec:
        case p_resta:
        case p_add_asig:
        case p_sub_asig:
        case p_mul_asig:
        case p_div_asig:
        case p_mod_asig:
        case p_and_asig:
        case p_or_asig:
        case p_xor_asig:
        case p_shr_asig:
        case p_shl_asig:

        case p_asigword:
        case p_incword:
        case p_sumaword:
        case p_decword:
        case p_restaword:
        case p_add_asigword:
        case p_sub_asigword:
        case p_mul_asigword:
        case p_div_asigword:
        case p_mod_asigword:
        case p_and_asigword:
        case p_or_asigword:
        case p_xor_asigword:
        case p_shr_asigword:
        case p_shl_asigword:

        case p_asigchar:
        case p_incchar:
        case p_sumachar:
        case p_decchar:
        case p_restachar:
        case p_add_asigchar:
        case p_sub_asigchar:
        case p_mul_asigchar:
        case p_div_asigchar:
        case p_mod_asigchar:
        case p_and_asigchar:
        case p_or_asigchar:
        case p_xor_asigchar:
        case p_shr_asigchar:
        case p_shl_asigchar:

        case p_strcpy:
        case p_strcat:
        case p_strsub:
          break;
        default:
          c_error(4, 64);
          break;
        }
        break;
      default:
        c_error(4, 64);
      }
      if (!free_sintax)
        if (current_token != p_ptocoma)
          c_error(3, 66);
      while (current_token == p_ptocoma || current_token == p_coma)
        lexer();
      g1(lasp);
      statement_end();
      record_statement();
      break;
    }
  }
}
