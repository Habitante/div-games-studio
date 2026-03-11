
//-----------------------------------------------------------------------------
//      Source code syntax colorizer functions
//-----------------------------------------------------------------------------

#include "global.h"

void col_analyze_ltlex(void);

//-----------------------------------------------------------------------------

#define max_obj     768 // Maximum compiler object count
#define long_med_id 16  // Average identifier length (+4+4+1)

#define max_nodos 128 // Maximum lexer symbol nodes

#define cr  13 // Carriage return
#define lf  10 // Line feed
#define tab 9  // Tab

#define l_err 0 // lex_case values when not pointers to lex_simb
#define l_cr  1 // End of line (l_err = unexpected character)
#define l_id  2 // Identifier or reserved word
#define l_spc 3 // Spaces and tabs
#define l_lit 4 // Literal
#define l_num 5 // Numeric constant

#define p_ultima 0x00 // End of file <EOF>

#define p_process 0x07 // Process

#define p_ini_rem 0x7d // Comment start
#define p_end_rem 0x7e // Comment end
#define p_rem     0x7f // Single-line comment

#define p_id  0xfd // Identifier
#define p_num 0xfe // Number

#define p_spc 0x100 // Spaces
#define p_sym 0x101 // Symbol
#define p_lit 0x102 // Quoted literal
#define p_res 0x103 // Reserved word id
#define p_pre 0x104 // Predefined id

//-----------------------------------------------------------------------------

struct clex_ele {
  byte caracter;
  byte token;
  struct clex_ele *alternativa;
  struct clex_ele *siguiente;
} clex_simb[max_nodos], *iclex_simb, *clex_case[256];

int cnum_nodos; // Number of nodes used in clex_simb

int iscoment; // Whether inside a comment
int numrem;   // Nesting level of /* and */

byte *cvnom = NULL; // Name vector (hash_chain:int, token (or iobj):int, asciiz)
union {
  byte *b;
  byte **p;
} icvnom;

byte *cvhash[256]; // Pointers into the name vector

int cpieza;

byte *csource;

int incluye_nombres;

//-----------------------------------------------------------------------------
//      Error function (!!!) (only used when analyzing ltlex)
//-----------------------------------------------------------------------------

void col_error(int n, int m) {
  n = n;
  m = m;
}

//-----------------------------------------------------------------------------
//      Syntax colorizer initialization
//-----------------------------------------------------------------------------

void init_lexcolor() {
  int n;
  memset(clex_simb, 0, sizeof(clex_simb));
  iclex_simb = clex_simb;
  cnum_nodos = 0;
  memset(cvhash, 0, sizeof(cvhash));

  for (n = 0; n < 256; n++)
    if (lower[n])
      if (n >= '0' && n <= '9')
        clex_case[n] = (struct clex_ele *)l_num;
      else
        clex_case[n] = (struct clex_ele *)l_id;
    else
      clex_case[n] = (struct clex_ele *)l_err;

  if ((cvnom = (byte *)malloc(max_obj * long_med_id + 1024)) == NULL)
    col_error(0, 0);

  icvnom.b = cvnom;

  col_analyze_ltlex();

  clex_case[' '] = (struct clex_ele *)l_spc;
  clex_case[tab] = (struct clex_ele *)l_spc;
  clex_case[cr] = (struct clex_ele *)l_cr;
  clex_case[0] = (struct clex_ele *)l_cr;

  incluye_nombres = 0;

  //  col_analyze_ltobj();
}

void end_lexcolor() {
  free(cvnom);
}

//-----------------------------------------------------------------------------
//      Lexer (reads the next token from *csource)
//-----------------------------------------------------------------------------

void color_lex(void) {
  byte **ptr, *_ivnom, h, *_source = csource;
  struct clex_ele *e;
  int n;

  switch ((uintptr_t)clex_case[*_source]) { // Pointer to a clex_ele or l_???

  case l_err:
    cpieza = p_rem;
    _source++;
    break;

  case l_cr:
    if ((*++_source) == lf)
      _source++;
    cpieza = p_ultima;
    break; // NOTE: EOL/EOF handling inherited from DIV Professional

  case l_id:
    if (iscoment > 0) {
      cpieza = p_rem;
      _source++;
      break;
    }
    _ivnom = icvnom.b;
    *icvnom.p++ = 0;
    *icvnom.p++ = (byte *)0x100;
    h = 0;
    while ((*icvnom.b = lower[*_source++]))
      h = ((byte)(h << 1) + (h >> 7)) ^ (*icvnom.b++);
    icvnom.b++;
    _source--;
    if (icvnom.b - cvnom > max_obj * long_med_id) {
      icvnom.b = _ivnom;
      cpieza = p_id;
      break;
    }
    ptr = &cvhash[h];
    while (*ptr && strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8))
      ptr = (byte **)*ptr;
    if (!strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8)) { // id found
      icvnom.b = _ivnom;                                     // remove it from cvnom
      cpieza = (intptr_t)*(ptr + 1);
      if (cpieza < 256 && cpieza >= 0) { // reserved word (token)
        if (cpieza == p_rem) {
          while (*_source != cr && *_source)
            _source++;
        } else
          cpieza = p_res;
      } else { // object (previous id)
        cpieza = p_pre;
      }
    } else { // new id
      if (incluye_nombres) {
        *ptr = _ivnom; // add a new id
      } else {
        icvnom.b = _ivnom; // remove it from cvnom
      }
      cpieza = p_id;
    }
    break;

  case l_spc:
    while ((*++_source) == ' ') {}
    cpieza = p_spc;
    break;

  case l_lit:
    if (iscoment > 0) {
      cpieza = p_rem;
      _source++;
      break;
    }
    cpieza = p_lit;
    h = *_source;
    _ivnom = icvnom.b; // Literal delimited by h
    do {
      _source++;
      if (*_source == cr || !*_source)
        break;
      if (*_source == h)
        if (*(_source + 1) == h)
          *_ivnom = *++_source;
        else
          *_ivnom = 0;
      else
        *_ivnom = *_source;
    } while (*_ivnom++);
    _source++;
    n = (strlen((char *)icvnom.b) + 4) >> 2;
    break;

  case l_num:
    if (iscoment > 0) {
      cpieza = p_rem;
      _source++;
      break;
    }
    cpieza = p_num;
    if (*_source == '0' && lower[*(_source + 1)] == 'x') {
      _source += 2;
      while ((uintptr_t)clex_case[*_source] == l_num ||
             (lower[*_source] >= 'a' && lower[*_source] <= 'f')) {
        _source++;
      }
    } else
      do {
        _source++;
      } while ((uintptr_t)clex_case[*_source] == l_num);
    break;

  default: // pointer to a clex_ele

    e = clex_case[*_source++];
    _ivnom = _source;
    cpieza = (*e).token;
    while ((e = (*e).siguiente)) {
      while (*_source != (*e).caracter && (*e).alternativa)
        e = (*e).alternativa;
      if (*_source++ == (*e).caracter && (*e).token) {
        cpieza = (*e).token;
        _ivnom = _source;
      }
    }
    _source = _ivnom;

    if (cpieza == p_rem && iscoment <= 0) {
      while (*_source != cr && *_source)
        _source++;
    } else if (cpieza == p_ini_rem) {
      cpieza = p_rem;
      iscoment++;
      numrem++;
    } else if (cpieza == p_end_rem) {
      cpieza = p_rem;
      iscoment--;
      numrem--;
    } else if (iscoment > 0)
      cpieza = p_rem;
    else
      cpieza = p_sym;

    break;
  }
  csource = _source;
}

//-----------------------------------------------------------------------------

FILE *cdef; // For the "ltlex.def" analyzer
byte *_cbuf;

//-----------------------------------------------------------------------------
//      Preload lexical structures by parsing the ltlex.def file
//-----------------------------------------------------------------------------

void col_analyze_ltlex(void) {
  byte *buf, cont = 1;
  int len;
  struct clex_ele *e;

  int t = 0; //token (cpieza)
  byte h;    //hash (for id)
  byte *_ivnom;
  byte **ptr;

  if ((cdef = fopen("system/ltlex.def", "rb")) == NULL) {
    col_error(0, 1);
    return;
  }
  fseek(cdef, 0, SEEK_END);
  len = ftell(cdef);
  if ((_cbuf = buf = (byte *)malloc(len + 2)) == NULL) {
    col_error(0, 0);
    return;
  }
  fseek(cdef, 0, SEEK_SET);
  len = fread(buf, 1, len, cdef);
  *(buf + len) = cr;
  *(buf + len + 1) = cr;

  do
    switch (*buf++) {
    case ' ':
    case tab:
      break;
    case cr:
      if (*buf == cr)
        cont = 0;
      else {
        buf++;
      }
      break;
    case ';':
      while (*buf != cr) {
        buf++;
      }
      break;
    case '&':
      *buf = lower[*buf];
      if (*buf >= '0' && *buf <= '9')
        t = (*buf++ - '0') << 4;
      else if (*buf >= 'a' && *buf <= 'f')
        t = (*buf++ - 'a' + 10) << 4;
      else
        col_error(0, 2);
      *buf = lower[*buf];
      if (*buf >= '0' && *buf <= '9')
        t += (*buf++ - '0');
      else if (*buf >= 'a' && *buf <= 'f')
        t += (*buf++ - 'a' + 10);
      else
        col_error(0, 2);
      if (*buf == cr || *buf == ' ' || *buf == tab)
        break;
      else if (lower[*buf]) { //Parse a reserved word
        _ivnom = icvnom.b;
        *icvnom.p++ = 0;
        *icvnom.p++ = (byte *)t;
        h = 0;
        while ((*icvnom.b = lower[*buf++]))
          h = ((byte)(h << 1) + (h >> 7)) ^ (*icvnom.b++);
        ptr = &cvhash[h];
        while (*ptr)
          ptr = (byte **)*ptr;
        *ptr = _ivnom;
        buf--;
        icvnom.b++;
      } else if (t >= 0x78 && t <= 0x7b) { //Parse a literal delimiter
        clex_case[*buf] = (struct clex_ele *)l_lit;
      } else { //Parse a new symbol
        if ((e = clex_case[*buf]) == 0) {
          if (cnum_nodos++ == max_nodos)
            col_error(0, 3);
          e = clex_case[*buf] = iclex_simb++;
          (*e).caracter = *buf++;
        } else
          buf++;
        while (*buf != ' ' && *buf != tab && *buf != cr) {
          if (lower[*buf])
            col_error(0, 4);
          if ((*e).siguiente == 0)
            if (cnum_nodos++ == max_nodos)
              col_error(0, 3);
            else
              e = (*e).siguiente = iclex_simb++;
          else {
            e = (*e).siguiente;
            while ((*e).caracter != *buf && (*e).alternativa)
              e = (*e).alternativa;
            if ((*e).caracter != *buf) {
              if (cnum_nodos++ == max_nodos)
                col_error(0, 3);
              else
                e = (*e).alternativa = iclex_simb++;
            }
          }
          (*e).caracter = *buf++;
        }
        (*e).token = t;
      }
      break;
    }
  while (cont);

  free(_cbuf);
  _cbuf = NULL;
  fclose(cdef);
  cdef = NULL;
}

//-----------------------------------------------------------------------------
//  Add predefined words to the hash vector
//-----------------------------------------------------------------------------

void col_analyze_ltobj(void) {
  byte *buf;
  int len;

  if ((cdef = fopen("system/ltobj.def", "rb")) == NULL) {
    col_error(0, 1);
    return;
  }
  fseek(cdef, 0, SEEK_END);
  len = ftell(cdef);
  if ((_cbuf = buf = (byte *)malloc(len + 2)) == NULL) {
    col_error(0, 0);
    return;
  }
  fseek(cdef, 0, SEEK_SET);
  len = fread(buf, 1, len, cdef);
  *(buf + len) = cr;
  *(buf + len + 1) = cr;

  incluye_nombres = 1;
  csource = buf;
  do {
    color_lex();
  } while (csource < buf + len);
  incluye_nombres = 0;

  free(_cbuf);
  _cbuf = NULL;
  fclose(cdef);
  cdef = NULL;
}
