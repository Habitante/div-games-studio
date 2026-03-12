
//-----------------------------------------------------------------------------
//      DIV Compiler - Expressions & Code Generation
//
//      Expression parsing (precedence climbing), condition evaluation,
//      factor/atom handling, bytecode emission with peephole optimizer.
//      Extracted from compiler.c during Phase 2C-3.
//-----------------------------------------------------------------------------

#include "compiler_internal.h"

//-----------------------------------------------------------------------------
//      Condition analysis (generates code to evaluate it)
//-----------------------------------------------------------------------------

int tipo_expresion = 0; // 0 - int, 1 - char or byte, 2 - string
int tipo_factor = 0;    // same

void condition(void) {
  byte *__ierror, *_ierror;
  int _linea;

  if (extended_conditions) {
    expression();
    return;
  } // Same as DIV 1

  _exp = tabexp;
  save_error(0);
  _linea = source_line;
  _ierror = ierror;
  con00(0);
  swap(source_line, _linea);
  __ierror = ierror;
  ierror = _ierror;
  generate_expression();
  source_line = _linea;
  ierror = __ierror;
}

void con00(int tipo_exp) {
  int te = tipo_expresion;
  tipo_expresion = tipo_exp;
  con0();
  tipo_expresion = te;
}

void con0() { // Right-to-left associativity operators <-
  int p;

  con1();
  if ((p = current_token) == p_asig || (p >= p_add_asig && p <= p_shl_asig)) {
    c_error(0, 140);
  }
}

void con1() { // Left-to-right associativity operators ->
  int p;
  con2();
  while ((p = current_token) >= p_or && p <= p_andofs) {
    if (p >= p_xorptr)
      p -= p_xorptr - p_xor; // Convert xorptr,andofs to xor,and
    lexer();
    con2();
    (*_exp).type = eoper;
    (*_exp++).token = p;
    tipo_factor = 0;
  }
}

void con2() {
  int p, tf;
  exp3();
  if (simple_conditions) {
    p = current_token;
    while ((p >= p_igu && p <= p_mai) || p == p_asig) {
      tf = tipo_factor;
      if (p == p_asig)
        p = p_igu;
      lexer();
      exp3();
      if (tf == 2 || tipo_factor == 2) {
        p += p_strigu - p_igu;
        (*_exp).type = eoper;
        (*_exp++).token = p;
      } else {
        (*_exp).type = eoper;
        (*_exp++).token = p;
      }
      tipo_factor = 0;
      p = current_token;
    }
  } else {
    while ((p = current_token) >= p_igu && p <= p_mai) {
      tf = tipo_factor;
      lexer();
      exp3();
      if (tf == 2 || tipo_factor == 2) {
        p += p_strigu - p_igu;
        (*_exp).type = eoper;
        (*_exp++).token = p;
      } else {
        (*_exp).type = eoper;
        (*_exp++).token = p;
      }
      tipo_factor = 0;
    }
  }
}

//-----------------------------------------------------------------------------
//      Expression analysis (generates code to evaluate it)
//-----------------------------------------------------------------------------

/* Expression parser entry point (recursive descent).
 * Builds a postfix expression tree in tabexp[] via exp00(), then
 * emits bytecode by calling generate_expression(). Saves and restores
 * error-reporting state around the parse.
 */
void expression(void) {
  byte *__ierror, *_ierror;
  int _linea;

  _exp = tabexp;
  save_error(0);
  _linea = source_line;
  _ierror = ierror;
  exp00(0);
  swap(source_line, _linea);
  __ierror = ierror;
  ierror = _ierror;
  generate_expression();
  source_line = _linea;
  ierror = __ierror;
}

void expression_cpa(void) {
  byte *__ierror, *_ierror;
  int _linea;

  _exp = tabexp;
  save_error(0);
  _linea = source_line;
  _ierror = ierror;
  exp00(0);
  swap(source_line, _linea);
  __ierror = ierror;
  ierror = _ierror;
  if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointer) {
    _exp--;
    generate_expression();
    g1(lcpa);
  } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerchar) {
    _exp--;
    generate_expression();
    if (enable_strfix) {
      g2(lcar, 0);
      g1(lstrfix);
      g1(lasp);
    }
    g1(lcpachr);
  } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerbyte) {
    _exp--;
    generate_expression();
    g1(lcpachr);
  } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerword) {
    _exp--;
    generate_expression();
    g1(lcpawor);
  } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_string) {
    _exp--;
    generate_expression();
    g1(lcpastr);
  } else
    c_error(4, 44);
  source_line = _linea;
  ierror = __ierror;
}

void generate_expression(void) {
  struct exp_ele *e = tabexp;
  struct object *ob;

  do {
    switch ((*e).type) {
    case econs:
      g2(lcar, (*e).value);
      break;
    case estring:
      g2(lcar, (*e).value);
      break;
    case erango:
      g2(lrng, (*e).value);
      break;
    case ewhoami:
      g1(lcid);
      break;
    case ecall:
      ob = (*e).object;
      g2(lcal, (*ob).proc.offset);
      if ((*ob).used) {
        (*ob).proc.offset = imem - 1;
      }
      break;
    case efunc:
      ob = (*e).object;
      g2(lfun, (*ob).func.code);
      break;
    case efext:
      ob = (*e).object;
      g2(lext, (*ob).func_extern.code);
      break;
    case echeck:
      g1(lchk);
      break;
    case enull:
      g1(lnul);
      break;
    case eoper:
      switch ((*e).token) {
      case p_asig:
        g1(lasi);
        break;
      case p_or:
        g1(lori);
        break;
      case p_xor:
        g1(lxor);
        break;
      case p_and:
        g1(land);
        break;
      case p_igu:
        g1(ligu);
        break;
      case p_dis:
        g1(ldis);
        break;
      case p_may:
        g1(lmay);
        break;
      case p_men:
        g1(lmen);
        break;
      case p_mei:
        g1(lmei);
        break;
      case p_mai:
        g1(lmai);
        break;
      case p_add:
        g1(ladd);
        break;
      case p_sub:
        g1(lsub);
        break;
      case p_mul:
        g1(lmul);
        break;
      case p_div:
        g1(ldiv);
        break;
      case p_mod:
        g1(lmod);
        break;
      case p_neg:
        g1(lneg);
        break;
      case p_pointer:
        g1(lptr);
        break;
      case p_not:
        g1(lnot);
        break;
      case p_punto:
        g1(laid);
        break;
      case p_shr:
        g1(lshr);
        break;
      case p_shl:
        g1(lshl);
        break;
      case p_inc:
        g1(lipt);
        break;
      case p_suma:
        g1(lpti);
        break;
      case p_dec:
        g1(ldpt);
        break;
      case p_resta:
        g1(lptd);
        break;
      case p_add_asig:
        g1(lada);
        break;
      case p_sub_asig:
        g1(lsua);
        break;
      case p_mul_asig:
        g1(lmua);
        break;
      case p_div_asig:
        g1(ldia);
        break;
      case p_mod_asig:
        g1(lmoa);
        break;
      case p_and_asig:
        g1(lana);
        break;
      case p_or_asig:
        g1(lora);
        break;
      case p_xor_asig:
        g1(lxoa);
        break;
      case p_shr_asig:
        g1(lsra);
        break;
      case p_shl_asig:
        g1(lsla);
        break;

      case p_pointerword:
        g1(lptrwor);
        break;
      case p_sumaword:
        g1(lptiwor);
        break;
      case p_restaword:
        g1(lptdwor);
        break;
      case p_incword:
        g1(liptwor);
        break;
      case p_decword:
        g1(ldptwor);
        break;
      case p_asigword:
        g1(lasiwor);
        break;
      case p_add_asigword:
        g1(ladawor);
        break;
      case p_sub_asigword:
        g1(lsuawor);
        break;
      case p_mul_asigword:
        g1(lmuawor);
        break;
      case p_div_asigword:
        g1(ldiawor);
        break;
      case p_mod_asigword:
        g1(lmoawor);
        break;
      case p_and_asigword:
        g1(lanawor);
        break;
      case p_or_asigword:
        g1(lorawor);
        break;
      case p_xor_asigword:
        g1(lxoawor);
        break;
      case p_shr_asigword:
        g1(lsrawor);
        break;
      case p_shl_asigword:
        g1(lslawor);
        break;

      case p_pointerchar:
        g1(lptrchr);
        break;
      case p_sumachar:
        g1(lptichr);
        break;
      case p_restachar:
        g1(lptdchr);
        break;
      case p_incchar:
        g1(liptchr);
        break;
      case p_decchar:
        g1(ldptchr);
        break;
      case p_asigchar:
        g1(lasichr);
        break;
      case p_add_asigchar:
        g1(ladachr);
        break;
      case p_sub_asigchar:
        g1(lsuachr);
        break;
      case p_mul_asigchar:
        g1(lmuachr);
        break;
      case p_div_asigchar:
        g1(ldiachr);
        break;
      case p_mod_asigchar:
        g1(lmoachr);
        break;
      case p_and_asigchar:
        g1(lanachr);
        break;
      case p_or_asigchar:
        g1(lorachr);
        break;
      case p_xor_asigchar:
        g1(lxoachr);
        break;
      case p_shr_asigchar:
        g1(lsrachr);
        break;
      case p_shl_asigchar:
        g1(lslachr);
        break;

      case p_strcpy:
        g1(lstrcpy);
        break;
      case p_strfix:
        g1(lstrfix);
        break;
      case p_strcat:
        g1(lstrcat);
        break;
      case p_stradd:
        g1(lstradd);
        break;
      case p_strdec:
        g1(lstrdec);
        break;
      case p_strsub:
        g1(lstrsub);
        break;
      case p_strlen:
        g1(lstrlen);
        break;
      case p_strigu:
        g1(lstrigu);
        break;
      case p_strdis:
        g1(lstrdis);
        break;
      case p_strmay:
        g1(lstrmay);
        break;
      case p_strmen:
        g1(lstrmen);
        break;
      case p_strmei:
        g1(lstrmei);
        break;
      case p_strmai:
        g1(lstrmai);
        break;

      case p_pointerbyte:
        g1(lptrchr);
        break;
      case p_string:
        break; // Like a null p_pointer, no-op

      default:
        c_error(4, 41);
      }
      break;
    default:
      c_error(4, 41);
    }
  } while (++e != _exp);
}

//-----------------------------------------------------------------------------
//      Constant expression analysis
//      Local values and calls to other processes are not handled
//-----------------------------------------------------------------------------

int constant(void) {
  byte *__ierror, *_ierror;
  struct exp_ele *e = _exp = tabexp;
  int _linea, i = 0;

  save_error(0);

  _linea = source_line;
  _ierror = ierror;
  exp00(0);
  swap(source_line, _linea);
  __ierror = ierror;
  ierror = _ierror;

  do
    switch ((*e).type) {
    case econs:
      eval_stack[++i] = (*e).value;
      break;
    case estring:
      eval_stack[++i] = (*e).value;
      break;

    case eoper:
      switch ((*e).token) {
      case p_or:
        eval_stack[i - 1] |= eval_stack[i];
        i--;
        break;
      case p_xor:
        eval_stack[i - 1] ^= eval_stack[i];
        i--;
        break;
      case p_and:
        eval_stack[i - 1] &= eval_stack[i];
        i--;
        break;
      case p_igu:
        eval_stack[i - 1] = eval_stack[i - 1] == eval_stack[i];
        i--;
        break;
      case p_dis:
        eval_stack[i - 1] = eval_stack[i - 1] != eval_stack[i];
        i--;
        break;
      case p_may:
        eval_stack[i - 1] = eval_stack[i - 1] > eval_stack[i];
        i--;
        break;
      case p_men:
        eval_stack[i - 1] = eval_stack[i - 1] < eval_stack[i];
        i--;
        break;
      case p_mei:
        eval_stack[i - 1] = eval_stack[i - 1] <= eval_stack[i];
        i--;
        break;
      case p_mai:
        eval_stack[i - 1] = eval_stack[i - 1] >= eval_stack[i];
        i--;
        break;
      case p_add:
        eval_stack[i - 1] += eval_stack[i];
        i--;
        break;
      case p_sub:
        eval_stack[i - 1] -= eval_stack[i];
        i--;
        break;
      case p_mul:
        eval_stack[i - 1] *= eval_stack[i];
        i--;
        break;
      case p_div:
        if (eval_stack[i] == 0)
          c_error(4, 34);
        eval_stack[i - 1] /= eval_stack[i];
        i--;
        break;
      case p_mod:
        if (eval_stack[i] == 0)
          c_error(4, 34);
        eval_stack[i - 1] %= eval_stack[i];
        i--;
        break;
      case p_neg:
        eval_stack[i] = -eval_stack[i];
        break;

        // I think pointers (and ranges, consequently) were removed because of FROM,
        // since it caused confusion to treat a variable as a constant ...

      case p_not:
        eval_stack[i] ^= -1;
        break;
      case p_shr:
        eval_stack[i - 1] >>= eval_stack[i];
        i--;
        break;
      case p_shl:
        eval_stack[i - 1] <<= eval_stack[i];
        i--;
        break;
      default:
        c_error(4, 30);
      }
      break;
    default:
      c_error(4, 30);
    }
  while (++e != _exp);

  source_line = _linea;
  ierror = __ierror;
  return (eval_stack[i]);
}

//-----------------------------------------------------------------------------
//      Expression analysis into *tabexp
//-----------------------------------------------------------------------------

void exp00(int tipo_exp) {
  int te = tipo_expresion;
  tipo_expresion = tipo_exp;
  exp0();
  tipo_expresion = te;
}

void exp0() { // Right-to-left associativity operators <-
  int p;

  exp1();
  if ((p = current_token) == p_asig || (p >= p_add_asig && p <= p_shl_asig)) {
    if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointer) {
      _exp--;
      lexer();
      exp00(0);
      (*_exp).type = eoper;
      (*_exp++).token = p;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerchar) {
      if (p == p_asig)
        p = p_asigchar;
      else
        p += p_add_asigchar - p_add_asig;
      _exp--;
      lexer();
      exp00(1); // e.g. string[1]="A";
      if (enable_strfix) {
        (*_exp).type = eoper;
        (*_exp++).token = p_strfix;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerbyte) {
      if (p == p_asig)
        p = p_asigchar;
      else
        p += p_add_asigchar - p_add_asig;
      _exp--;
      lexer();
      exp00(1); // e.g. my_bytes[1]="A";
      (*_exp).type = eoper;
      (*_exp++).token = p;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerword) {
      if (p == p_asig)
        p = p_asigword;
      else
        p += p_add_asigword - p_add_asig;
      _exp--;
      lexer();
      exp00(0);
      (*_exp).type = eoper;
      (*_exp++).token = p;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_string) {
      if (p == p_asig) { // string="..."; // Do strcpy
        _exp--;
        lexer();
        exp00(2);
        (*_exp).type = eoper;
        (*_exp++).token = p_strcpy;
        (*_exp).type = eoper;
        (*_exp++).token = p_string; // A string remains on the stack
      } else if (p == p_add_asig) { // string+="..."; // Do strcat
        _exp--;
        lexer();
        exp00(2);
        (*_exp).type = eoper;
        (*_exp++).token = p_strcat;
        (*_exp).type = eoper;
        (*_exp++).token = p_string; // A string remains on the stack
      } else if (p == p_sub_asig) { // string-=n; // Remove characters
        _exp--;
        lexer();
        exp00(-1);
        (*_exp).type = eoper;
        (*_exp++).token = p_strsub;
        (*_exp).type = eoper;
        (*_exp++).token = p_string; // A string remains on the stack
      } else
        c_error(0, 133);
    } else
      c_error(0, 39);
  }
  tipo_factor = 0;
}

void exp1() { // Left-to-right associativity operators ->
  int p;
  div_exp2();
  while ((p = current_token) >= p_or && p <= p_andofs) {
    if (p >= p_xorptr)
      p -= p_xorptr - p_xor; // Convert xorptr,andofs to xor,and
    lexer();
    div_exp2();
    (*_exp).type = eoper;
    (*_exp++).token = p;
    tipo_factor = 0;
  }
}

void div_exp2() {
  int p, tf;
  exp3();
  while ((p = current_token) >= p_igu && p <= p_mai) {
    tf = tipo_factor;
    lexer();
    exp3();
    if (tf == 2 || tipo_factor == 2) {
      // NOTE: When both operands are strings, comparison uses strcmp (not pointer equality).
      p += p_strigu - p_igu;
      (*_exp).type = eoper;
      (*_exp++).token = p;
      tipo_factor = 0;
    } else {
      (*_exp).type = eoper;
      (*_exp++).token = p;
    }
    tipo_factor = 0;
  }
}

void exp3() {
  int p;
  exp4();
  while ((p = current_token) >= p_shr && p <= p_shl) {
    lexer();
    exp4();
    (*_exp).type = eoper;
    (*_exp++).token = p;
    tipo_factor = 0;
  }
}

void exp4() {
  int p, tf;
  exp5();
  while ((p = current_token) >= p_suma && p <= p_sub && (p <= p_resta || p >= p_add)) {
    tf = tipo_factor;
    if (p <= p_resta)
      p += p_add - p_suma; // Convert suma,resta to add,sub
    lexer();
    exp5();
    if (p == p_add && (tf == 2 || tipo_factor == 2 || tipo_expresion == 2)) {
      (*_exp).type = eoper;
      (*_exp++).token = p_stradd;
      (*_exp).type = eoper;
      (*_exp++).token = p_string; // A string is on the stack
      tipo_factor = 2;
    } else if (p == p_sub && tf == 2) {
      if (tipo_factor == 2) {
        (*_exp).type = eoper;
        (*_exp++).token = p_strlen;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_strdec;
      (*_exp).type = eoper;
      (*_exp++).token = p_string; // A string is on the stack
      tipo_factor = 2;
    } else {
      (*_exp).type = eoper;
      (*_exp++).token = p;
      tipo_factor = 0;
    }
  }
}

void exp5() {
  int p;
  unary();
  while ((p = current_token) >= p_mul && p <= p_multi) {
    if (p == p_multi)
      p = p_mul; // Convert multi to mul
    lexer();
    unary();
    (*_exp).type = eoper;
    (*_exp++).token = p;
    tipo_factor = 0;
  }
}

void unary() {
  int p;

  while ((p = current_token) == p_add) {
    lexer();
  }
  if ((p >= p_xorptr && p <= p_mul) || (p == p_inc || p == p_dec)) {
    if (in_params > 0 && p == p_pointer) {
      factor();
      return;
    }

    if (p >= p_sub && p <= p_mul)
      p -= p_sub - p_neg; // Convert sub,mul to neg,pointer
    else if (p >= p_xorptr && p <= p_andofs)
      p += p_pointer - p_xorptr; //xorptr,andofs to pointer,offset

    lexer();
    unary(); // p = ( neg, pointer, offset, not )

    if (p == p_offset) {
      if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointer)
        _exp--;
      else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerchar) {
        c_error(4, 132); // &string[<exp>] ???
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerbyte) {
        if ((*(_exp - 2)).type == econs && (*(_exp - 2)).value == 0)
          _exp -= 2;
        else
          c_error(4, 132); // &bytes[<exp>] ???
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerword) {
        if ((*(_exp - 2)).type == econs && (*(_exp - 2)).value == 0)
          _exp -= 2;
        else
          c_error(4, 132); // &words[<exp>] ???
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_string) {
        // &string == string
      } else
        c_error(4, 32);
    } else if (p == p_inc || p == p_dec) {
      if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointer) {
        _exp--;
        (*_exp).type = eoper;
        (*_exp++).token = p;
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerchar) {
        _exp--;
        (*_exp).type = eoper;
        (*_exp++).token = p + p_incchar - p_inc;
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerbyte) {
        _exp--;
        (*_exp).type = eoper;
        (*_exp++).token = p + p_incchar - p_inc;
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerword) {
        _exp--;
        (*_exp).type = eoper;
        (*_exp++).token = p + p_incword - p_inc;
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_string) {
        _exp--;
        (*_exp).type = econs;
        if (p == p_inc)
          (*_exp++).value = -1;
        else
          (*_exp++).value = 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_strsub;
        (*_exp).type = eoper;
        (*_exp++).token = p_string;
      } else
        c_error(4, 122);
    } else if (p == p_pointer) {
      if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerchar) {
        // *string[0] == string[0]
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerbyte) {
        // *string[0] == string[0]
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerword) {
        // *string[0] == string[0]
      } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_string) {
        // *string == string
      } else {
        (*_exp).type = eoper;
        (*_exp++).token = p;
      }
    } else {
      (*_exp).type = eoper;
      (*_exp++).token = p;
    }

  } else
    exp6();
}

void exp6() { // Operator for accessing other processes' local variables or tables
  struct exp_ele *e;

  factor();
  while (current_token == p_punto) {
    if (comprueba_id)
      (*_exp++).type = echeck;
    e = _exp + 1;
    cross_process_access = 1;
    lexer();
    factor();
    cross_process_access = 0;

    // WARNING: p_punto (dot-access) MUST be the SECOND element that factor()
    // pushes into the expression. The code below replaces it with p_add.
    // (Alternative: search for p_punto between e-1 and _exp.)

    if ((*e).type == eoper && (*e).token == p_punto)
      (*e).token = p_add;
    else
      c_error(4, 43);
  }
}

int struct_pointer; // p_pointer / p_pointerchar / p_string / p_pointerbyte / p_pointerword

void factor(void) {
  struct object *ob;
  struct object *obs;
  int p, offset, _imem;
  struct exp_ele *e;

  tipo_factor = 0;

  switch (current_token) { // factor analysis

  case p_abrir:
    lexer();
    exp00(tipo_expresion);
    if (current_token != p_cerrar) {
      c_error(3, 25);
    }
    lexer();
    break;

  case p_corab:
    lexer();
    exp00(0);
    if (current_token != p_corce)
      c_error(3, 26);
    (*_exp).type = eoper;
    (*_exp++).token = p_pointer;
    lexer();
    break;

  case p_whoami:
    (*_exp++).type = ewhoami;
    lexer();
    break;

  case p_num:
    (*_exp).type = econs;
    (*_exp++).value = token_value;
    lexer();
    break;

  case p_lit:
    tipo_factor = 2;
    if (tipo_expresion == 1) {
      (*_exp).type = econs;
      (*_exp++).value = (byte)mem[token_value];
    } else {
      (*_exp).type = estring;
      (*_exp++).value = token_value;
      if (tipo_expresion == -1) {
        (*_exp).type = eoper;
        (*_exp++).token = p_strlen;
      }
    }
    lexer();
    break;

  case p_type:
    lexer();
    if (current_token != p_id)
      c_error(0, 112);
    switch ((*o).type) {
    case tnone:
      (*o).line = source_line;
      (*o).ierror = ierror;
      (*_exp).type = econs;
      (*_exp++).value = (memptrsize)o;
      (*o).used = 1;
      break;
    case tproc:
      (*_exp).type = econs;
      (*_exp++).value = (memptrsize)o;
      break;
    case tsglo:
      if ((*o).struct_global.offset == long_header) { // type mouse ≡ 0
        (*_exp).type = econs;
        (*_exp++).value = 0;
        break;
      }
      /* fall through */
    default:
      c_error(0, 60);
    }
    lexer();
    break;

  case p_sizeof:

    lexer();
    if (current_token != p_abrir)
      c_error(3, 36);
    lexer();
    if (current_token != p_id)
      c_error(3, 113);
    ;
    (*_exp).type = econs;
    switch ((*o).type) {
    case tsglo:
    case tsloc:
      (*_exp++).value = (*o).struct_global.item_len * (*o).struct_global.total_items;
      break;
    case tpsgl:
    case tpslo:
      (*_exp++).value = (*((*o).ptr_struct_global.ostruct)).struct_global.item_len *
                        (*o).struct_global.total_items;
      break;
    case ttglo:
    case ttloc:
    case tbglo:
    case tbloc:
    case twglo:
    case twloc:
      (*_exp++).value = (*o).table_global.total_len;
      break;
    case tcglo:
    case tcloc:
      (*_exp++).value = ((*o).string_global.total_len + 5) / 4;
      break;
    case tvglo:
    case tvloc:
      (*_exp++).value = 1;
      break;
    default:
      c_error(0, 61);
    }
    lexer();
    if (current_token != p_cerrar)
      c_error(3, 25);
    lexer();
    break;

  case p_struct: // Check for struct pointer parameter declaration
    if (in_params <= 0) {
      if (error_27 == 27)
        c_error(1, 27);
      else
        c_error(0, error_27);
    } else {
      lexer();
      if (current_token != p_pointer)
        c_error(0, 151); // Pointer can't be defined this way
      lexer();
      if (current_token != p_id)
        c_error(1, 110);
      obs = o;
      if ((*obs).type == tnone)
        c_error(0, 150); // Pointer can't be defined this way
      if ((*obs).type != tsglo && (*obs).type != tsloc)
        c_error(0, 150);
      lexer();
      ob = o;
      analyze_pointer_struct(tpslo, iloc++, obs);
      (*ob).line = source_line;
      (*ob).ierror = ierror;
      (*ob).param = 1;
      in_params++;
      (*_exp).type = econs;
      (*_exp++).value = (*ob).ptr_struct_local.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
    }
    break;

  case p_string: // Check for string parameter declaration
    if (in_params <= 0) {
      if (error_27 == 27)
        c_error(1, 27);
      else
        c_error(0, error_27);
    } else {
      lexer();
      if (current_token == p_pointer) { // Receives a pointer to string
        lexer();
        ob = analyze_pointer(tpclo, iloc++);
        (*ob).line = source_line;
        (*ob).ierror = ierror;
        (*ob).param = 1;
        in_params++;
        (*_exp).type = econs;
        (*_exp++).value = (*ob).ptr_string_local.offset;
        (*_exp).type = eoper;
        (*_exp++).token = p_punto;
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        break;
      }
      if (current_token != p_id)
        c_error(1, 125);
      ob = o;
      if ((*ob).type != tnone)
        c_error(0, 126);
      (*ob).line = source_line;
      (*ob).ierror = ierror;
      (*ob).param = 1;
      (*ob).type = tcloc;
      lexer();
      if (current_token == p_corab) {
        lexer();
        if (current_token == p_corce) {
          lexer();
          (*ob).string_local.total_len = 255;
        } else {
          e = _exp;
          if (((*ob).string_local.total_len = constant()) < 0)
            c_error(4, 127);
          if ((*ob).string_local.total_len > 0xFFFFF)
            c_error(4, 135);
          if (current_token != p_corce)
            c_error(3, 26);
          lexer();
          _exp = e;
        }
      } else
        (*ob).string_local.total_len = 255;
      if (in_params > 1) {
        g2(lpar, in_params - 1);
      }
      in_params = 1;
      g2(lpri, 0);
      _imem = imem;
      imem += 1 + ((*ob).string_local.total_len + 5) / 4;
      test_buffer(&mem, &imem_max, imem);
      mem[_imem] = 0xDAD00000 | (*ob).string_local.total_len;
      memset(&mem[imem + 1], 0, (((*ob).string_local.total_len + 5) & -5));
      (*ob).string_local.offset = iloc + 1;
      iloc += 1 + ((*ob).string_local.total_len + 5) / 4;
      mem[_imem - 1] = imem; // pri
      (*_exp).type = estring;
      (*_exp++).value = (*ob).string_local.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      (*_exp).type = eoper;
      (*_exp++).token = p_string;
    }
    break;

  case p_byte: // Check for byte parameter declaration
    if (in_params <= 0) {
      if (error_27 == 27)
        c_error(1, 27);
      else
        c_error(0, error_27);
    } else {
      lexer();
      if (current_token == p_pointer) {
        lexer();
        ob = analyze_pointer(tpblo, iloc++);
        (*ob).line = source_line;
        (*ob).ierror = ierror;
        (*ob).param = 1;
        in_params++;
        (*_exp).type = econs;
        (*_exp++).value = (*ob).ptr_byte_local.offset;
        (*_exp).type = eoper;
        (*_exp++).token = p_punto;
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        break;
      }
      if (current_token != p_id)
        c_error(1, 142);
      ob = o;
      if ((*ob).type != tnone)
        c_error(0, 126);
      (*ob).line = source_line;
      (*ob).ierror = ierror;
      (*ob).param = 1;
      (*ob).type = tbloc;
      lexer();
      if (current_token == p_corab)
        c_error(2, 146);
      in_params++;
      (*ob).byte_local.offset = iloc++;
      (*ob).byte_local.len1 = 0;
      (*ob).byte_local.len2 = -1;
      (*ob).byte_local.len3 = -1;
      (*ob).byte_local.total_len = 1;
      (*_exp).type = econs;
      (*_exp++).value = (*ob).byte_local.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      (*_exp).type = econs;
      (*_exp++).value = 0;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointerbyte;
    }
    break;

  case p_word: // Check for word parameter declaration
    if (in_params <= 0) {
      if (error_27 == 27)
        c_error(1, 27);
      else
        c_error(0, error_27);
    } else {
      lexer();
      if (current_token == p_pointer) {
        lexer();
        ob = analyze_pointer(tpwlo, iloc++);
        (*ob).line = source_line;
        (*ob).ierror = ierror;
        (*ob).param = 1;
        in_params++;
        (*_exp).type = econs;
        (*_exp++).value = (*ob).ptr_word_local.offset;
        (*_exp).type = eoper;
        (*_exp++).token = p_punto;
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        break;
      }
      if (current_token != p_id)
        c_error(1, 145);
      ob = o;
      if ((*ob).type != tnone)
        c_error(0, 126);
      (*ob).line = source_line;
      (*ob).ierror = ierror;
      (*ob).param = 1;
      (*ob).type = twloc;
      lexer();
      if (current_token == p_corab)
        c_error(2, 146);
      in_params++;
      (*ob).word_local.offset = iloc++;
      (*ob).word_local.len1 = 0;
      (*ob).word_local.len2 = -1;
      (*ob).word_local.len3 = -1;
      (*ob).word_local.total_len = 1;
      (*_exp).type = econs;
      (*_exp++).value = (*ob).word_local.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      (*_exp).type = econs;
      (*_exp++).value = 0;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointerword;
    }
    break;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  case p_int: // Check for int parameter declaration
    if (in_params <= 0) {
      if (error_27 == 27)
        c_error(1, 27);
      else
        c_error(0, error_27);
    } else {
      lexer();
      if (current_token == p_pointer) {
      case p_pointer: // Pointer-to-int parameter declaration
#pragma GCC diagnostic pop
        if (in_params <= 0) {
          if (error_27 == 27)
            c_error(1, 27);
          else
            c_error(0, error_27);
        }
        in_params++;
        lexer();
        ob = analyze_pointer(tpilo, iloc++);
        (*ob).line = source_line;
        (*ob).ierror = ierror;
        (*ob).param = 1;
        (*_exp).type = econs;
        (*_exp++).value = (*ob).ptr_int_local.offset;
        (*_exp).type = eoper;
        (*_exp++).token = p_punto;
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        break;
      } else if (current_token != p_id)
        c_error(1, 149);
    }

  case p_id:
    switch ((*o).type) {
    case tnone:
      ob = o;
      (*ob).line = source_line;
      (*ob).ierror = ierror;
      lexer();
      if (current_token != p_abrir) {
        if (in_params > 0) {
          if (current_token == p_corab)
            c_error(2, 146);
          in_params++;
          (*ob).type = tvloc;
          (*ob).var_local.offset = iloc++;
          (*ob).param = 1;
          (*_exp).type = econs;
          (*_exp++).value = (*ob).var_local.offset;
          (*_exp).type = eoper;
          (*_exp++).token = p_punto;
          (*_exp).type = eoper;
          (*_exp++).token = p_pointer;
          break;
        } else
          c_error(2, 28);
      }
      (*ob).used = 1;
      (*ob).type = tproc;
      (*ob).proc.scope = ob;
      (*ob).proc.offset = 0;
      (*ob).proc.num_params = 0;
      lexer();
      while (current_token != p_cerrar) {
        (*ob).proc.num_params++;
        exp00(0);
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
      (*_exp).type = ecall;
      (*_exp++).object = ob;
      lexer();
      break;

    case tcons:
      if ((*o).cons.literal) {
        tipo_factor = 2;
        if (tipo_expresion == 1) {
          (*_exp).type = econs;
          (*_exp++).value = (byte)mem[token_value];
        } else if (tipo_expresion == -1) {
          (*_exp).type = econs;
          (*_exp++).value = token_value;
          (*_exp).type = eoper;
          (*_exp++).token = p_strlen;
        } else {
          (*_exp).type = econs;
          (*_exp++).value = (*o).cons.value;
        }
      } else {
        (*_exp).type = econs;
        (*_exp++).value = (*o).cons.value;
      }
      lexer();
      break;

    case tvglo:
      (*_exp).type = econs;
      (*_exp++).value = (*o).var_global.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      lexer();
      break;
      break;

    case ttglo:
    case tpigl:
      ob = o;
      offset = (*ob).table_global.offset;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = offset;
      if (current_token == p_corab) {
        if ((*ob).type == tpigl) {
          (*_exp).type = eoper;
          (*_exp++).token = p_pointer;
          if (comprueba_null)
            (*_exp++).type = enull;
        }
        lexer();
        exp00(0);
        if ((*ob).table_global.len1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).table_global.len1;
          }
        if ((*ob).table_global.len2 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).table_global.len2;
          }
          if ((*ob).table_global.len3 > -1) {
            if (current_token != p_coma)
              c_error(3, 130);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).table_global.len3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).table_global.len2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).table_global.len1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      break;

    case tbglo:
    case tpbgl:
    case tpcgl:
      ob = o;
      offset = (*ob).byte_global.offset;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = offset;
      if ((*ob).type == tpbgl || (*ob).type == tpcgl) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (current_token != p_corab)
          break;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      if (current_token == p_corab) {
        lexer();
        exp00(0);
        if ((*ob).byte_global.len1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).byte_global.len1;
          }
        if ((*ob).byte_global.len2 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).byte_global.len2;
          }
          if ((*ob).byte_global.len3 > -1) {
            if (current_token != p_coma)
              c_error(3, 130);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).byte_global.len3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).byte_global.len2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).byte_global.len1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
      } else { // mi_byte ≡ mi_byte[0]
        (*_exp).type = econs;
        (*_exp++).value = 0;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_pointerbyte;
      break;

    case twglo:
    case tpwgl:
      ob = o;
      offset = (*ob).word_global.offset;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = offset;
      if ((*ob).type == tpwgl) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (current_token != p_corab)
          break;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      if (current_token == p_corab) {
        lexer();
        exp00(0);
        if ((*ob).word_global.len1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).word_global.len1;
          }
        if ((*ob).word_global.len2 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).word_global.len2;
          }
          if ((*ob).word_global.len3 > -1) {
            if (current_token != p_coma)
              c_error(3, 130);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).word_global.len3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).word_global.len2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).word_global.len1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
      } else { // mi_byte ≡ mi_byte[0]
        (*_exp).type = econs;
        (*_exp++).value = 0;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_pointerword;
      break;

    case tcglo:
      ob = o;
      offset = (*ob).string_global.offset;
      lexer();
      (*_exp).type = estring;
      (*_exp++).value = offset;
      if (current_token == p_corab) {
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).string_global.total_len;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
        (*_exp).type = eoper;
        (*_exp++).token = p_pointerchar;
      } else {
        tipo_factor = 2;
        if (tipo_expresion == 1) {
          (*_exp).type = econs;
          (*_exp++).value = 0;
          (*_exp).type = eoper;
          (*_exp++).token = p_pointerchar;
        } else {
          (*_exp).type = eoper;
          (*_exp++).token = p_string;
          if (tipo_expresion == -1) {
            (*_exp).type = eoper;
            (*_exp++).token = p_strlen;
          }
        }
      }
      break;

    case tsglo:
    case tpsgl:
      ob = o;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = (*ob).struct_global.offset;
      if ((*ob).type == tpsgl) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (current_token != p_corab && current_token != p_punto)
          break;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      if (current_token == p_corab) {
        lexer();
        exp00(0);
        if ((*ob).struct_global.dim1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).struct_global.dim1;
          }
        if ((*ob).struct_global.dim2 > -1) {
          if (current_token != p_coma)
            c_error(3, 131);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).struct_global.dim2;
          }
          if ((*ob).struct_global.dim3 > -1) {
            if (current_token != p_coma)
              c_error(3, 131);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).struct_global.dim3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).struct_global.dim2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).struct_global.dim1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce) {
          c_error(3, 26);
        }
        lexer();
        if ((*ob).type == tpsgl) {
          (*_exp).type = econs;
          (*_exp++).value = (*((*ob).ptr_struct_global.ostruct)).struct_global.item_len;
        } else {
          (*_exp).type = econs;
          (*_exp++).value = (*ob).struct_global.item_len;
        }
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token == p_punto) {
        struct_pointer = p_pointer;
        if ((*ob).type == tpsgl)
          member = (*ob).ptr_struct_global.ostruct;
        else
          member = ob;
        lexer();
        factor_struct();
        (*_exp).type = eoper;
        (*_exp++).token = struct_pointer;
      } else {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
      }
      break;

    case tvloc:
      if (cross_process_access && (*o).scope)
        c_error(0, 56);
      (*_exp).type = econs;
      (*_exp++).value = (*o).var_local.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      lexer();
      break;

    case ttloc:
    case tpilo:
      if (cross_process_access && (*o).scope)
        c_error(0, 56);
      ob = o;
      offset = (*ob).table_local.offset;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      if (current_token == p_corab) {
        if ((*ob).type == tpilo) {
          (*_exp).type = eoper;
          (*_exp++).token = p_pointer;
          if (comprueba_null)
            (*_exp++).type = enull;
        }
        cross_process_access = 0;
        lexer();
        exp00(0);
        if ((*ob).table_local.len1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).table_local.len1;
          }
        if ((*ob).table_local.len2 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).table_local.len2;
          }
          if ((*ob).table_local.len3 > -1) {
            if (current_token != p_coma)
              c_error(3, 130);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).table_local.len3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).table_local.len2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).table_local.len1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      break;

    case tbloc:
    case tpblo:
    case tpclo:
      if (cross_process_access && (*o).scope)
        c_error(0, 56);
      ob = o;
      offset = (*ob).byte_local.offset;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      if ((*ob).type == tpblo || (*ob).type == tpclo) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (current_token != p_corab)
          break;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      if (current_token == p_corab) {
        cross_process_access = 0;
        lexer();
        exp00(0);
        if ((*ob).byte_local.len1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).byte_local.len1;
          }
        if ((*ob).byte_local.len2 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).byte_local.len2;
          }
          if ((*ob).byte_local.len3 > -1) {
            if (current_token != p_coma)
              c_error(3, 130);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).byte_local.len3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).byte_local.len2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).byte_local.len1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
      } else { // mi_byte ≡ mi_byte[0]
        (*_exp).type = econs;
        (*_exp++).value = 0;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_pointerbyte;
      break;

    case twloc:
    case tpwlo:
      if (cross_process_access && (*o).scope)
        c_error(0, 56);
      ob = o;
      offset = (*ob).word_local.offset;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      if ((*ob).type == tpwlo) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (current_token != p_corab)
          break;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      if (current_token == p_corab) {
        cross_process_access = 0;
        lexer();
        exp00(0);
        if ((*ob).word_local.len1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).word_local.len1;
          }
        if ((*ob).word_local.len2 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).word_local.len2;
          }
          if ((*ob).word_local.len3 > -1) {
            if (current_token != p_coma)
              c_error(3, 130);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).word_local.len3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).word_local.len2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).word_local.len1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
      } else { // mi_byte ≡ mi_byte[0]
        (*_exp).type = econs;
        (*_exp++).value = 0;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_pointerword;
      break;

    case tcloc:
      if (cross_process_access && (*o).scope)
        c_error(0, 56);
      ob = o;
      offset = (*ob).string_local.offset;
      lexer();
      (*_exp).type = estring;
      (*_exp++).value = offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      if (current_token == p_corab) {
        cross_process_access = 0;
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).string_local.total_len;
        }
        if (current_token != p_corce)
          c_error(3, 26);
        lexer();
        (*_exp).type = eoper;
        (*_exp++).token = p_pointerchar;
      } else {
        tipo_factor = 2;
        if (tipo_expresion == 1) {
          (*_exp).type = econs;
          (*_exp++).value = 0;
          (*_exp).type = eoper;
          (*_exp++).token = p_pointerchar;
        } else {
          (*_exp).type = eoper;
          (*_exp++).token = p_string;
          if (tipo_expresion == -1) {
            (*_exp).type = eoper;
            (*_exp++).token = p_strlen;
          }
        }
      }
      break;

    case tsloc:
    case tpslo:
      if (cross_process_access && (*o).scope)
        c_error(0, 56);
      ob = o;
      lexer();
      (*_exp).type = econs;
      (*_exp++).value = (*ob).struct_local.offset;
      (*_exp).type = eoper;
      (*_exp++).token = p_punto;
      if ((*ob).type == tpslo) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (current_token != p_corab && current_token != p_punto)
          break;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      if (current_token == p_corab) {
        cross_process_access = 0;
        lexer();
        exp00(0);
        if ((*ob).struct_local.dim1 > -1)
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).struct_local.dim1;
          }
        if ((*ob).struct_local.dim2 > -1) {
          if (current_token != p_coma)
            c_error(3, 131);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).struct_local.dim2;
          }
          if ((*ob).struct_local.dim3 > -1) {
            if (current_token != p_coma)
              c_error(3, 131);
            lexer();
            exp00(0);
            if (check_range) {
              (*_exp).type = erango;
              (*_exp++).value = (*ob).struct_local.dim3;
            }
            (*_exp).type = econs;
            (*_exp++).value = (*ob).struct_local.dim2 + 1;
            (*_exp).type = eoper;
            (*_exp++).token = p_mul;
            (*_exp).type = eoper;
            (*_exp++).token = p_add;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).struct_local.dim1 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        if (current_token != p_corce) {
          c_error(3, 26);
        }
        lexer();
        if ((*ob).type == tpslo) {
          (*_exp).type = econs;
          (*_exp++).value = (*((*ob).ptr_struct_local.ostruct)).struct_local.item_len;
        } else {
          (*_exp).type = econs;
          (*_exp++).value = (*ob).struct_local.item_len;
        }
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token == p_punto) {
        struct_pointer = p_pointer;
        if ((*ob).type == tpslo)
          member = (*ob).ptr_struct_global.ostruct;
        else
          member = ob;
        lexer();
        factor_struct();
        (*_exp).type = eoper;
        (*_exp++).token = struct_pointer;
      } else {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
      }
      break;

    case tproc:
      ob = o;
      lexer();
      if (current_token != p_abrir)
        c_error(3, 36);
      lexer();
      p = 0;
      while (current_token != p_cerrar) { // p≡num_params
        p++;
        exp00(0);
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
      if (p != (*ob).proc.num_params)
        c_error(1, 46);
      (*_exp).type = ecall;
      (*_exp++).object = ob;
      lexer();
      break;

    case tfunc:
      ob = o;
      lexer();
      if (current_token != p_abrir)
        c_error(3, 36);

      if ((*ob).func.code == 122) { // fopen("..."
        in_fopen = 1;
        lexer();
        in_fopen = 0;
      } else
        lexer();

      p = 0;
      while (current_token != p_cerrar) { // p≡num_params

        if ((*ob).func.par[p] == 1) { // Type 1 parameter (struct,field)

          // RECEIVES: struct_name,field_name
          // GENERATES: struct_offset,item_len,total_len,field_offset,field_type

          if (current_token != p_id)
            c_error(0, 152);
          if ((*o).member != NULL)
            c_error(0, 152);
          if ((*o).type == tsglo) {
            (*_exp).type = econs;
            (*_exp++).value = (*o).struct_global.offset;
          } else if ((*o).type == tsloc) {
            (*_exp).type = econs;
            (*_exp++).value = (*o).struct_local.offset;
            (*_exp).type = eoper;
            (*_exp++).token = p_punto;
          } else
            c_error(0, 152);
          (*_exp).type = econs;
          (*_exp++).value = (*o).struct_global.item_len;
          (*_exp).type = econs;
          (*_exp++).value = (*o).struct_global.total_items;
          obs = o;
          lexer();
          if (current_token != p_coma)
            c_error(0, 46);
          member = obs;
          lexer();
          member = NULL;
          if (current_token != p_id)
            c_error(0, 153);
          if ((*o).member != obs)
            c_error(0, 153);
          (*_exp).type = econs;
          (*_exp++).value = (*o).var_global.offset;
          (*_exp).type = econs;
          if ((*o).type == tcglo || (*o).type == tcloc)
            (*_exp++).value = 1;
          else if ((*o).type == tpcgl || (*o).type == tpclo)
            (*_exp++).value = 2;
          else
            (*_exp++).value = 0;
          lexer();
        } else { // Type 0 parameter (generic)
          exp00(0);
        }
        p++;

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
      if (p != (*ob).func.num_params)
        c_error(1, 46);
      (*_exp).type = efunc;
      (*_exp++).object = ob;
      if ((*ob).func.ret == 1)
        tipo_factor = 2;
      lexer();
      break;

    case tfext:
      ob = o;
      lexer();
      if (current_token != p_abrir)
        c_error(3, 36);
      lexer();
      p = 0;
      while (current_token != p_cerrar) { // p≡num_params
        p++;
        exp00(0);
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
      if (p != (*ob).func_extern.num_params)
        c_error(1, 46);
      (*_exp).type = efext;
      (*_exp++).object = ob;
      lexer();
      break;

    default:
      if (error_27 == 27)
        c_error(1, 27);
      else
        c_error(0, error_27);
      break;
    }
    break;

  default:
    if (error_27 == 27)
      c_error(1, 27);
    else
      c_error(0, error_27);
    break;
  }

  if (current_token == p_inc || current_token == p_dec) {
    if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointer) {
      _exp--;
      (*_exp).type = eoper;
      if (current_token == p_inc)
        (*_exp++).token = p_suma;
      else
        (*_exp++).token = p_resta;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerchar) {
      _exp--;
      (*_exp).type = eoper;
      if (current_token == p_inc)
        (*_exp++).token = p_sumachar;
      else
        (*_exp++).token = p_restachar;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerbyte) {
      _exp--;
      (*_exp).type = eoper;
      if (current_token == p_inc)
        (*_exp++).token = p_sumachar;
      else
        (*_exp++).token = p_restachar;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_pointerword) {
      _exp--;
      (*_exp).type = eoper;
      if (current_token == p_inc)
        (*_exp++).token = p_sumaword;
      else
        (*_exp++).token = p_restaword;
    } else if ((*(_exp - 1)).type == eoper && (*(_exp - 1)).token == p_string) {
      _exp--;
      (*_exp).type = econs;
      if (current_token == p_inc)
        (*_exp++).value = -1;
      else
        (*_exp++).value = 1;
      (*_exp).type = eoper;
      (*_exp++).token = p_strsub;
      (*_exp).type = eoper;
      (*_exp++).token = p_string;
    } else
      c_error(4, 122);
    lexer();
  }
}

void factor_struct(void) {
  struct object *ob;

  if (current_token != p_id)
    c_error(3, 58);
  switch ((*o).type) {
  case tvglo:
    (*_exp).type = econs;
    (*_exp++).value = (*o).var_global.offset;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    break;

  case ttglo:
  case tpigl:
    (*_exp).type = econs;
    (*_exp++).value = (*o).table_global.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if (current_token == p_corab) {
      if ((*ob).type == tpigl) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      lexer();
      exp00(0);
      if ((*ob).table_global.len1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).table_global.len1;
        }
      if ((*ob).table_global.len2 > -1) {
        if (current_token != p_coma)
          c_error(3, 130);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).table_global.len2;
        }
        if ((*ob).table_global.len3 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).table_global.len3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).table_global.len2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).table_global.len1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
      (*_exp).type = eoper;
      (*_exp++).token = p_add;
    }
    break;

  case tbglo:
  case tpbgl:
  case tpcgl:
    (*_exp).type = econs;
    (*_exp++).value = (*o).byte_global.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if ((*ob).type == tpbgl || (*ob).type == tpcgl) {
      if (current_token != p_corab)
        break;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      if (comprueba_null)
        (*_exp++).type = enull;
    }
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if ((*ob).byte_global.len1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).byte_global.len1;
        }
      if ((*ob).byte_global.len2 > -1) {
        if (current_token != p_coma)
          c_error(3, 130);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).byte_global.len2;
        }
        if ((*ob).byte_global.len3 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).byte_global.len3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).byte_global.len2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).byte_global.len1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
    } else { // mi_byte ≡ mi_byte[0]
      (*_exp).type = econs;
      (*_exp++).value = 0;
    }
    struct_pointer = p_pointerbyte;
    break;

  case twglo:
  case tpwgl:
    (*_exp).type = econs;
    (*_exp++).value = (*o).word_global.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if ((*ob).type == tpwgl) {
      if (current_token != p_corab)
        break;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      if (comprueba_null)
        (*_exp++).type = enull;
    }
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if ((*ob).word_global.len1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).word_global.len1;
        }
      if ((*ob).word_global.len2 > -1) {
        if (current_token != p_coma)
          c_error(3, 130);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).word_global.len2;
        }
        if ((*ob).word_global.len3 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).word_global.len3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).word_global.len2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).word_global.len1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
    } else { // mi_byte ≡ mi_byte[0]
      (*_exp).type = econs;
      (*_exp++).value = 0;
    }
    struct_pointer = p_pointerword;
    break;

  case tcglo:
    (*_exp).type = estring;
    (*_exp++).value = (*o).string_global.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if (check_range) {
        (*_exp).type = erango;
        (*_exp++).value = (*ob).string_global.total_len;
      }
      if (current_token != p_corce)
        c_error(3, 26);
      lexer();
      struct_pointer = p_pointerchar;
    } else {
      tipo_factor = 2;
      if (tipo_expresion == 1) {
        (*_exp).type = econs;
        (*_exp++).value = 0;
        struct_pointer = p_pointerchar;
      } else {
        if (tipo_expresion == -1) {
          (*_exp).type = eoper;
          (*_exp++).token = p_strlen;
        }
        struct_pointer = p_string;
      }
    }
    break;

  case tsglo:
  case tpsgl:
    (*_exp).type = econs;
    (*_exp++).value = (*o).struct_global.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if ((*ob).type == tpsgl) {
      if (current_token != p_corab && current_token != p_punto)
        break;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      if (comprueba_null)
        (*_exp++).type = enull;
    }
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if ((*ob).struct_global.dim1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).struct_global.dim1;
        }
      if ((*ob).struct_global.dim2 > -1) {
        if (current_token != p_coma)
          c_error(3, 131);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).struct_global.dim2;
        }
        if ((*ob).struct_global.dim3 > -1) {
          if (current_token != p_coma)
            c_error(3, 131);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).struct_global.dim3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).struct_global.dim2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).struct_global.dim1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
      if ((*ob).type == tpsgl) {
        (*_exp).type = econs;
        (*_exp++).value = (*((*ob).ptr_struct_global.ostruct)).struct_global.item_len;
      } else {
        (*_exp).type = econs;
        (*_exp++).value = (*ob).struct_global.item_len;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_mul;
      (*_exp).type = eoper;
      (*_exp++).token = p_add;
    }
    if (current_token == p_punto) {
      if ((*ob).type == tpsgl)
        member = (*ob).ptr_struct_global.ostruct;
      else
        member = ob;
      lexer();
      factor_struct();
    }
    break;

  case tvloc:
    (*_exp).type = econs;
    (*_exp++).value = (*o).var_local.offset;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    break;

  case ttloc:
  case tpilo:
    (*_exp).type = econs;
    (*_exp++).value = (*o).table_local.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if (current_token == p_corab) {
      if ((*ob).type == tpilo) {
        (*_exp).type = eoper;
        (*_exp++).token = p_pointer;
        if (comprueba_null)
          (*_exp++).type = enull;
      }
      lexer();
      exp00(0);
      if ((*ob).table_local.len1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).table_local.len1;
        }
      if ((*ob).table_local.len2 > -1) {
        if (current_token != p_coma)
          c_error(3, 130);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).table_local.len2;
        }
        if ((*ob).table_local.len3 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).table_local.len3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).table_local.len2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).table_local.len1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
      (*_exp).type = eoper;
      (*_exp++).token = p_add;
    }
    break;

  case tbloc:
  case tpblo:
  case tpclo:
    (*_exp).type = econs;
    (*_exp++).value = (*o).byte_local.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if ((*ob).type == tpblo || (*ob).type == tpcgl) {
      if (current_token != p_corab)
        break;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      if (comprueba_null)
        (*_exp++).type = enull;
    }
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if ((*ob).byte_local.len1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).byte_local.len1;
        }
      if ((*ob).byte_local.len2 > -1) {
        if (current_token != p_coma)
          c_error(3, 130);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).byte_local.len2;
        }
        if ((*ob).byte_local.len3 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).byte_local.len3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).byte_local.len2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).byte_local.len1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
    } else { // mi_byte ≡ mi_byte[0]
      (*_exp).type = econs;
      (*_exp++).value = 0;
    }
    struct_pointer = p_pointerbyte;
    break;

  case twloc:
  case tpwlo:
    (*_exp).type = econs;
    (*_exp++).value = (*o).word_local.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if ((*ob).type == tpwlo) {
      if (current_token != p_corab)
        break;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      if (comprueba_null)
        (*_exp++).type = enull;
    }
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if ((*ob).word_local.len1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).word_local.len1;
        }
      if ((*ob).word_local.len2 > -1) {
        if (current_token != p_coma)
          c_error(3, 130);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).word_local.len2;
        }
        if ((*ob).word_local.len3 > -1) {
          if (current_token != p_coma)
            c_error(3, 130);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).word_local.len3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).word_local.len2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).word_local.len1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
    } else { // mi_byte ≡ mi_byte[0]
      (*_exp).type = econs;
      (*_exp++).value = 0;
    }
    struct_pointer = p_pointerword;
    break;

  case tcloc:
    (*_exp).type = estring;
    (*_exp++).value = (*o).string_local.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if (check_range) {
        (*_exp).type = erango;
        (*_exp++).value = (*ob).string_local.total_len;
      }
      if (current_token != p_corce)
        c_error(3, 26);
      lexer();
      struct_pointer = p_pointerchar;
    } else {
      tipo_factor = 2;
      if (tipo_expresion == 1) {
        (*_exp).type = econs;
        (*_exp++).value = 0;
        struct_pointer = p_pointerchar;
      } else {
        if (tipo_expresion == -1) {
          (*_exp).type = eoper;
          (*_exp++).token = p_strlen;
        }
        struct_pointer = p_string;
      }
    }
    break;

  case tsloc:
  case tpslo:
    (*_exp).type = econs;
    (*_exp++).value = (*o).struct_local.offset;
    ob = o;
    (*_exp).type = eoper;
    (*_exp++).token = p_add;
    member = NULL;
    lexer();
    if ((*ob).type == tpslo) {
      if (current_token != p_corab && current_token != p_punto)
        break;
      (*_exp).type = eoper;
      (*_exp++).token = p_pointer;
      if (comprueba_null)
        (*_exp++).type = enull;
    }
    if (current_token == p_corab) {
      lexer();
      exp00(0);
      if ((*ob).struct_local.dim1 > -1)
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).struct_local.dim1;
        }
      if ((*ob).struct_local.dim2 > -1) {
        if (current_token != p_coma)
          c_error(3, 131);
        lexer();
        exp00(0);
        if (check_range) {
          (*_exp).type = erango;
          (*_exp++).value = (*ob).struct_local.dim2;
        }
        if ((*ob).struct_local.dim3 > -1) {
          if (current_token != p_coma)
            c_error(3, 131);
          lexer();
          exp00(0);
          if (check_range) {
            (*_exp).type = erango;
            (*_exp++).value = (*ob).struct_local.dim3;
          }
          (*_exp).type = econs;
          (*_exp++).value = (*ob).struct_local.dim2 + 1;
          (*_exp).type = eoper;
          (*_exp++).token = p_mul;
          (*_exp).type = eoper;
          (*_exp++).token = p_add;
        }
        (*_exp).type = econs;
        (*_exp++).value = (*ob).struct_local.dim1 + 1;
        (*_exp).type = eoper;
        (*_exp++).token = p_mul;
        (*_exp).type = eoper;
        (*_exp++).token = p_add;
      }
      if (current_token != p_corce) {
        c_error(3, 26);
      }
      lexer();
      if ((*ob).type == tpslo) {
        (*_exp).type = econs;
        (*_exp++).value = (*((*ob).ptr_struct_local.ostruct)).struct_local.item_len;
      } else {
        (*_exp).type = econs;
        (*_exp++).value = (*ob).struct_local.item_len;
      }
      (*_exp).type = eoper;
      (*_exp++).token = p_mul;
      (*_exp).type = eoper;
      (*_exp++).token = p_add;
    }
    if (current_token == p_punto) {
      if ((*ob).type == tpslo)
        member = (*ob).ptr_struct_local.ostruct;
      else
        member = ob;
      lexer();
      factor_struct();
    }
    break;

  default:
    c_error(3, 58);
    break;
  }
}

//-----------------------------------------------------------------------------
//  Code generation functions
//-----------------------------------------------------------------------------

// void g1(int op) { mem[imem++]=op; }
// void g2(int op, int pa) { mem[imem++]=op; mem[imem++]=pa; }

//-----------------------------------------------------------------------------
//  Peephole optimization of EML intermediate code
//-----------------------------------------------------------------------------

// TODO: Consider removing the 'dir' field from the peephole struct and its
//       associated validation check, if it proves redundant.

//struct {      // Peephole optimization window
//  int dir;    // Address (position in mem[])
//  int param;  // Number of parameters for the instruction
//  int op;     // Opcode
//} code[16];   // code[15] always holds the last generated instruction

void g1(int op) {
  if (optimize)
    gen(0, op, 0);
  else
    mem[imem++] = op;
}

void g2(int op, int pa) {
  if (optimize)
    gen(1, op, pa);
  else {
    mem[imem++] = op;
    mem[imem++] = pa;
  }
}

int optimizado;

void gen(int param, int op, int pa) {
  optimizado = 0;

  switch (op) {
  case lcar:
    if (code[15].op == lcar) {
      code[15].op = mem[imem - 2] = lcar2;
      code[15].param = 2;
      mem[imem++] = pa;
      optimizado = 1;
    } else if (code[15].op == lcar2) {
      code[15].op = mem[imem - 3] = lcar3;
      code[15].param = 3;
      mem[imem++] = pa;
      optimizado = 1;
    } else if (code[15].op == lcar3) {
      code[15].op = mem[imem - 4] = lcar4;
      code[15].param = 4;
      mem[imem++] = pa;
      optimizado = 1;
    }
    break;
  case lasp:
    if (code[15].op == lasi) {
      if (code[14].op == lcar) {
        remove_code(1);
        code[15].op = mem[imem - 2] = lcarasiasp;
      } else {
        code[15].op = mem[imem - 1] = lasiasp;
        optimizado = 1;
      }
    } else if (code[15].op == lfun) {
      code[15].op = mem[imem - 2] = lfunasp;
      optimizado = 1;
    }
    break;
  case laid:
    if (code[15].op == lcar) {
      code[15].op = mem[imem - 2] = lcaraid;
      optimizado = 1;
    }
    break;
  case lptr:
    if (code[15].op == lcar) {
      code[15].op = mem[imem - 2] = lcarptr;
      optimizado = 1;
    } else if (code[15].op == lcaradd) {
      code[15].op = mem[imem - 2] = lcaraddptr;
      optimizado = 1;
    } else if (code[15].op == laid) {
      code[15].op = mem[imem - 1] = laidptr;
      optimizado = 1;
    } else if (code[15].op == ladd) {
      code[15].op = mem[imem - 1] = laddptr;
      optimizado = 1;
    } else if (code[15].op == lcaraid) {
      code[15].op = mem[imem - 2] = lcaraidptr;
      optimizado = 1;
    }
    break;
  case lcpa:
    if (code[15].op == lcaraid) {
      code[15].op = mem[imem - 2] = lcaraidcpa;
      optimizado = 1;
    }
    break;
  case ladd:
    if (code[15].op == lcar2) {
      code[15].op = mem[imem - 3] = lcar;
      code[15].param = 1;
      mem[imem - 2] += mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar3) {
      code[15].op = mem[imem - 4] = lcar2;
      code[15].param = 2;
      mem[imem - 2] += mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar4) {
      code[15].op = mem[imem - 5] = lcar3;
      code[15].param = 3;
      mem[imem - 2] += mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar) {
      if (mem[imem - 1] == 0)
        remove_code(1);
      else {
        code[15].op = mem[imem - 2] = lcaradd;
        optimizado = 1;
      }
    } else if (code[15].op == lcarmul) {
      code[15].op = mem[imem - 2] = lcarmuladd;
      optimizado = 1;
    }
    break;
  case lmul:
    if (code[15].op == lcar2) {
      code[15].op = mem[imem - 3] = lcar;
      code[15].param = 1;
      mem[imem - 2] *= mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar3) {
      code[15].op = mem[imem - 4] = lcar2;
      code[15].param = 2;
      mem[imem - 2] *= mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar4) {
      code[15].op = mem[imem - 5] = lcar3;
      code[15].param = 3;
      mem[imem - 2] *= mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar) {
      if (mem[imem - 1] == 1)
        remove_code(1);
      else {
        code[15].op = mem[imem - 2] = lcarmul;
        optimizado = 1;
      }
    }
    break;
  case lsub:
    if (code[15].op == lcar2) {
      code[15].op = mem[imem - 3] = lcar;
      code[15].param = 1;
      mem[imem - 2] -= mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar3) {
      code[15].op = mem[imem - 4] = lcar2;
      code[15].param = 2;
      mem[imem - 2] -= mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar4) {
      code[15].op = mem[imem - 5] = lcar3;
      code[15].param = 3;
      mem[imem - 2] -= mem[imem - 1];
      imem--;
      optimizado = 1;
    } else if (code[15].op == lcar) {
      if (mem[imem - 1] == 0)
        remove_code(1);
      else {
        code[15].op = mem[imem - 2] = lcarsub;
        optimizado = 1;
      }
    }
    break;
  case ldiv:
    if (code[15].op == lcar2) {
      if (mem[imem - 1] != 0) {
        code[15].op = mem[imem - 3] = lcar;
        code[15].param = 1;
        mem[imem - 2] /= mem[imem - 1];
        imem--;
        optimizado = 1;
      }
    } else if (code[15].op == lcar3) {
      if (mem[imem - 1] != 0) {
        code[15].op = mem[imem - 4] = lcar2;
        code[15].param = 2;
        mem[imem - 2] /= mem[imem - 1];
        imem--;
        optimizado = 1;
      }
    } else if (code[15].op == lcar4) {
      if (mem[imem - 1] != 0) {
        code[15].op = mem[imem - 5] = lcar3;
        code[15].param = 3;
        mem[imem - 2] /= mem[imem - 1];
        imem--;
        optimizado = 1;
      }
    } else if (code[15].op == lcar) {
      if (mem[imem - 1] == 1)
        remove_code(1);
      else if (mem[imem - 1] != 0) {
        code[15].op = mem[imem - 2] = lcardiv; // A cardiv will never be "cardiv 0"
        optimizado = 1;
      }
    }
    break;
  case lneg:
    if (code[15].op == lcar || code[15].op == lcar2 || code[15].op == lcar3 ||
        code[15].op == lcar4) {
      mem[imem - 1] = -mem[imem - 1];
      optimizado = 1;
    }
    break;
  case lnot:
    if (code[15].op == lcar || code[15].op == lcar2 || code[15].op == lcar3 ||
        code[15].op == lcar4) {
      mem[imem - 1] = mem[imem - 1] ^ -1;
      optimizado = 1;
    }
    break;
  }

  if (!optimizado) {
    if (imem - code[15].dir > 2)
      delete_code();
    add_code(imem, param, op);
    if (param) {
      mem[imem++] = op;
      mem[imem++] = pa;
    } else
      mem[imem++] = op;
  }
}

void remove_code(int i) {
  int n;
  optimizado = 1;
  while (i--) {
    imem -= code[15].param + 1;
    for (n = 15; n > 0; n--) {
      code[n].dir = code[n - 1].dir;
      code[n].param = code[n - 1].param;
      code[n].op = code[n - 1].op;
    }
    code[0].dir = 0;
    code[0].param = 0;
    code[0].op = 0;
  }
}

void delete_code(void) {
  int n;
  for (n = 0; n < 16; n++) {
    code[n].dir = 0;
    code[n].param = 0;
    code[n].op = 0;
  }
}

void add_code(int dir, int param, int op) {
  int n;
  for (n = 0; n < 15; n++) {
    code[n].dir = code[n + 1].dir;
    code[n].param = code[n + 1].param;
    code[n].op = code[n + 1].op;
  }
  code[15].dir = dir;
  code[15].param = param;
  code[15].op = op;
}

//-----------------------------------------------------------------------------
//      Precompilation
//-----------------------------------------------------------------------------
