
//-----------------------------------------------------------------------------
//      Módulo que contiene el código de la calculator
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"


///////////////////////////////////////////////////////////////////////////////
//   Prototypes for this file
///////////////////////////////////////////////////////////////////////////////
void get_token(void);
void expres0(void);
void expres1(void);
void expres2(void);
void expres3(void);
void expres4(void);
void expres5(void);
void calculator(void);

//-----------------------------------------------------------------------------
//      Variables del módulo
//-----------------------------------------------------------------------------

extern char get_buffer[long_line];
extern char *get;

struct _calc {
  char ctext[128];
  char cget[128];
  char cresult[128];
  int cint, chex;
} *pcalc, *readcalc;

int superget = 0;

//-----------------------------------------------------------------------------
//    Evaluador de expresiones (se le pasa el puntero en `expression')
//    Si todo fue bien, devuelve token=p_num y token_number=n
//-----------------------------------------------------------------------------


enum tokens {
  p_inicio,
  p_ultimo,
  p_error,
  p_num,
  p_abrir,
  p_cerrar,
  p_add,
  p_sub,
  p_mul,
  p_div,
  p_mod,
  p_shl,
  p_shr,
  p_xor,
  p_or,
  p_and,
  p_not,
  p_sqrt,
  p_neg
};

int token;               // Del tipo enumerado anterior
double token_number;          // Cuando token==p_num
static char *expression; // Puntero a la expresión asciiz

struct { // Para contener la expression analizada
  int token;
  double number;
} expressions[64];

int num_expressions; // Número de elementos introducidos en expressions[]

double do_evaluate(void);

void do_calculate(void) {
  double result;
  token = p_inicio;        // No hay ningun token inicialmente
  num_expressions = 0;             // Inicializa el contador de expresiones
  get_token();             // Obtiene el primer token
  expres0();               // Se analiza la expression
  if (token == p_ultimo) { // Se analizó con éxito la expression
    result = do_evaluate();
    if (token != p_error) { // Se evaluó con éxito
      token = p_num;
      token_number = result;
    }
  } else
    token = p_error;
}

double do_evaluate(void) {
  double calc_stack[64];
  int sp = 0, n = 0;

  do {
    switch (expressions[n].token) {
    case p_num:
      calc_stack[++sp] = expressions[n].number;
      break;
    case p_or:
      calc_stack[sp - 1] = (double)((memptrsize)calc_stack[sp - 1] | (memptrsize)calc_stack[sp]);
      sp--;
      break;
    case p_xor:
      calc_stack[sp - 1] = (double)((memptrsize)calc_stack[sp - 1] ^ (memptrsize)calc_stack[sp]);
      sp--;
      break;
    case p_and:
      calc_stack[sp - 1] = (double)((memptrsize)calc_stack[sp - 1] & (memptrsize)calc_stack[sp]);
      sp--;
      break;
    case p_add:
      calc_stack[sp - 1] += calc_stack[sp];
      sp--;
      break;
    case p_sub:
      calc_stack[sp - 1] -= calc_stack[sp];
      sp--;
      break;
    case p_mul:
      calc_stack[sp - 1] *= calc_stack[sp];
      sp--;
      break;
    case p_div:
      if (calc_stack[sp] == 0.0) {
        token = p_error;
        n = num_expressions;
      } else {
        calc_stack[sp - 1] /= calc_stack[sp];
        sp--;
      }
      break;
    case p_mod:
      if ((memptrsize)calc_stack[sp] == 0) {
        token = p_error;
        n = num_expressions;
      } else {
        calc_stack[sp - 1] = (double)((memptrsize)calc_stack[sp - 1] % (memptrsize)calc_stack[sp]);
        sp--;
      }
      break;
    case p_neg:
      calc_stack[sp] = -calc_stack[sp];
      break;
    case p_not:
      calc_stack[sp] = (double)((memptrsize)calc_stack[sp] ^ -1);
      break;
    case p_shr:
      calc_stack[sp - 1] = (double)((memptrsize)calc_stack[sp - 1] >> (memptrsize)calc_stack[sp]);
      sp--;
      break;
    case p_shl:
      calc_stack[sp - 1] = (double)((memptrsize)calc_stack[sp - 1] << (memptrsize)calc_stack[sp]);
      sp--;
      break;
    case p_sqrt:
      if (calc_stack[sp] < 0) {
        token = p_error;
        n = num_expressions;
      } else {
        if (calc_stack[sp] < 2147483648)
          calc_stack[sp] = sqrt(calc_stack[sp]);
        else {
          token = p_error;
          n = num_expressions;
        }
      }
      break;

    default:
      token = p_error;
      n = num_expressions;
      break;
    }
    if (pcalc->cint)
      calc_stack[sp] = (double)((memptrsize)calc_stack[sp]);
  } while (++n < num_expressions);

  if (sp != 1)
    token = p_error;

  return (calc_stack[sp]);
}

void expres0() { // xor or and
  int p;
  expres1();
  while ((p = token) >= p_xor && p <= p_and) {
    get_token();
    expres1();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres1() { // << >>
  int p;
  expres2();
  while ((p = token) >= p_shl && p <= p_shr) {
    get_token();
    expres2();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres2() { // + -
  int p;
  expres3();
  while ((p = token) >= p_add && p <= p_sub) {
    get_token();
    expres3();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres3() { // * / %
  int p;
  expres4();
  while ((p = token) >= p_mul && p <= p_mod) {
    get_token();
    expres4();
    expressions[num_expressions].token = p;
    num_expressions++;
  }
}

void expres4() { // signo !
  int p;

  while ((p = token) == p_add) {
    get_token();
  }

  if (p == p_not || p == p_sub) {
    if (p == p_sub)
      p = p_neg;
    get_token();
    expres4();
    expressions[num_expressions].token = p;
    num_expressions++;
  } else
    expres5();
}

void expres5() {
  if (token == p_abrir) {
    get_token();
    expres0();
    if (token != p_cerrar) {
      token = p_error;
      return;
    } else
      get_token();
  } else if (token == p_sqrt) {
    get_token();
    expres5();
    expressions[num_expressions].token = p_sqrt;
    num_expressions++;
  } else if (token == p_num) {
    expressions[num_expressions].token = p_num;
    expressions[num_expressions].number = token_number;
    num_expressions++;
    get_token();
  } else {
    token = p_error;
  }
}

double get_num(void);

void get_token(void) {
  char cwork[32];
  int n;

  if (token != p_error) {
reget_token:
    switch (*expression++) {
    case ' ':
      goto reget_token;
    case 0:
      token = p_ultimo;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      token = p_num;
      expression--;
      token_number = get_num();
      break;
    case '(':
      token = p_abrir;
      break;
    case ')':
      token = p_cerrar;
      break;
    case '+':
      token = p_add;
      break;
    case '-':
      token = p_sub;
      break;
    case '*':
      token = p_mul;
      break;
    case '/':
      token = p_div;
      break;
    case '%':
      token = p_mod;
      break;
    case '<':
      if (*expression++ == '<')
        token = p_shl;
      else
        token = p_error;
      break;
    case '>':
      if (*expression++ == '>')
        token = p_shr;
      else
        token = p_error;
      break;
    case '^':
      if (*expression == '^')
        expression++;
      token = p_xor;
      break;
    case '|':
      if (*expression == '|')
        expression++;
      token = p_or;
      break;
    case '&':
      if (*expression == '&')
        expression++;
      token = p_and;
      break;
    case '!':
      token = p_not;
      break;
    default:
      if (lower[*(expression - 1)] >= 'a' && lower[*(expression - 1)] <= 'z') {
        n = 1;
        cwork[0] = lower[*(expression - 1)];
        while (n < 31 && lower[*expression] >= 'a' && lower[*expression] <= 'z') {
          cwork[n++] = lower[*expression++];
        }
        cwork[n] = 0;
        if (!strcmp(cwork, "mod"))
          token = p_mod;
        else if (!strcmp(cwork, "not"))
          token = p_not;
        else if (!strcmp(cwork, "xor"))
          token = p_xor;
        else if (!strcmp(cwork, "or"))
          token = p_or;
        else if (!strcmp(cwork, "and"))
          token = p_and;
        else if (!strcmp(cwork, "sqrt"))
          token = p_sqrt;
        else if (!strcmp(cwork, "pi")) {
          token = p_num;
          token_number = 3.14159265359;
        } else
          token = p_error;
      } else {
        token = p_error;
      }
      break;
    }
  }
}

double get_num(void) { // Lee el número que hay en *expression (double en hex o dec)
  double x = 0;
  double dec = 10;

  if (*expression == '0' && lower[*(expression + 1)] == 'x') { // Numeros en hex

    expression += 2;
    if ((*expression >= '0' && *expression <= '9') ||
        (lower[*expression] >= 'a' && lower[*expression] <= 'f')) {
      do {
        if (*expression >= '0' && *expression <= '9')
          x = x * 16 + *expression++ - 0x30;
        else
          x = x * 16 + lower[*expression++] - 'a' + 10;
      } while ((*expression >= '0' && *expression <= '9') ||
               (lower[*expression] >= 'a' && lower[*expression] <= 'f'));
    } else
      token = p_error;

  } else {
    while (*expression >= '0' && *expression <= '9') {
      x *= 10;
      x += (*expression - '0');
      expression++;
    }
    if (*expression == '.') {
      expression++;
      if (*expression < '0' || *expression > '9')
        token = p_error;
      else
        do {
          x += (double)(*expression - '0') / dec;
          dec *= 10;
          expression++;
        } while (*expression >= '0' && *expression <= '9');
    }
  }

  return (x);
}

//-----------------------------------------------------------------------------
//      Cuadro de diálogo para la calculator
//-----------------------------------------------------------------------------

void calc1(void) {
  int w = v.w / big2, h = v.h / big2;

  pcalc = (struct _calc *)v.aux;

  get = pcalc->cget;
  _show_items();
  get = get_buffer;
  wbox(v.ptr, w, h, c12, 3, 11, w - 6 - 22 - 26, 8);
  wrectangle(v.ptr, w, h, c1, 3, 11, w - 6 - 22 - 26, 8);
  wwrite(v.ptr, w, h, 4, 12, 0, (byte *)pcalc->cresult, c3);
}

byte oldchex = 0;
byte oldcint = 0;

void calc2(void) {
  int w = v.w / big2, h = v.h / big2;

  pcalc = (struct _calc *)v.aux;

  // handle hex/int radio buttons
  if (pcalc->chex == 0 && pcalc->cint == 0) {
    if (oldcint == 0)
      pcalc->cint = 1;
    else
      pcalc->chex = 1;
    call(v.paint_handler);
  }

  if (pcalc->chex != oldchex) {
    pcalc->cint = 0;
    call(v.paint_handler);
  }

  if (pcalc->cint) {
    pcalc->chex = 0;
    call(v.paint_handler);
  }

  oldcint = pcalc->cint;
  oldchex = pcalc->chex;

  if (v.state)
    superget = 1;
  else
    superget = 0;
  get = pcalc->cget;
  _process_items();
  call(v.paint_handler);

  get = get_buffer;
  superget = 0;

  if (v.active_item >= 0 && strlen(pcalc->ctext)) { // Se evalúa la expresión
    expression = pcalc->ctext;
    do_calculate();

    if (token == p_num) {
      if (pcalc->chex)
        div_snprintf(pcalc->cresult, sizeof(pcalc->cresult), "0x%x",
                     (unsigned int)(memptrsize)token_number);
      else if (pcalc->cint)
        div_snprintf(pcalc->cresult, sizeof(pcalc->cresult), "%d", (int)(memptrsize)token_number);
      else
        div_snprintf(pcalc->cresult, sizeof(pcalc->cresult), "%g", token_number);
    } else
      div_strcpy(pcalc->cresult, sizeof(pcalc->cresult), (char *)texts[417]);
    wbox(v.ptr, w, h, c12, 4, 12, w - 8 - 22 - 26, 6);
    wwrite(v.ptr, w, h, 4, 12, 0, (byte *)pcalc->cresult, c3);
    v.redraw = 1;
  }
}

void calc3(void) {
  free(v.aux);
}

void calc0(void) {
  v.type = 102;
  v.w = 120;
  v.h = 34;
  v.title = texts[413];
  v.name = texts[413];
  v.paint_handler = calc1;
  v.click_handler = calc2;
  v.close_handler = calc3;

  if (pcalc == NULL)
    pcalc = readcalc;
  else {
    pcalc->chex = 0;
    pcalc->cint = 0;
    div_strcpy(pcalc->ctext, sizeof(pcalc->ctext), "");
    div_strcpy(pcalc->cresult, sizeof(pcalc->cresult), "0");
  }

  v.aux = (byte *)pcalc;

  _get(414, 4, 13, v.w - 8, (byte *)pcalc->ctext, 256, 0, 0);
  _flag(416, v.w - 23 - 26, 12, &pcalc->chex);
  _flag(415, v.w - 23, 12, &pcalc->cint);
}

void calculator(void) {
  pcalc = (struct _calc *)malloc(sizeof(struct _calc));
  if (pcalc != NULL) {
    new_window(calc0);
  }
}

//-----------------------------------------------------------------------------
