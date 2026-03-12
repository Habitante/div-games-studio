
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
  struct clex_ele *alternative;
  struct clex_ele *next;
} clex_symbols[max_nodos], *iclex_symbols, *clex_case[256];

int cnum_nodos; // Number of nodes used in clex_symbols

int in_comment; // Whether inside a comment
int numrem;     // Nesting level of /* and */

byte *cvnom = NULL; // Name vector (hash_chain:int, token (or iobj):int, asciiz)
union {
  byte *b;
  byte **p;
} icvnom;

byte *cvhash[256]; // Pointers into the name vector

int color_token;

byte *color_source;

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
  memset(clex_symbols, 0, sizeof(clex_symbols));
  iclex_symbols = clex_symbols;
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
//      Lexer (reads the next token from *color_source)
//-----------------------------------------------------------------------------

void color_lex(void) {
  byte **ptr, *_ivnom, h, *_source = color_source;
  struct clex_ele *e;
  int n;

  switch ((uintptr_t)clex_case[*_source]) { // Pointer to a clex_ele or l_???

  case l_err:
    color_token = p_rem;
    _source++;
    break;

  case l_cr:
    if ((*++_source) == lf)
      _source++;
    color_token = p_ultima;
    break; // NOTE: EOL/EOF handling inherited from DIV Professional

  case l_id:
    if (in_comment > 0) {
      color_token = p_rem;
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
      color_token = p_id;
      break;
    }
    ptr = &cvhash[h];
    while (*ptr && strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8))
      ptr = (byte **)*ptr;
    if (!strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8)) { // id found
      icvnom.b = _ivnom;                                     // remove it from cvnom
      color_token = (intptr_t)*(ptr + 1);
      if (color_token < 256 && color_token >= 0) { // reserved word (token)
        if (color_token == p_rem) {
          while (*_source != cr && *_source)
            _source++;
        } else
          color_token = p_res;
      } else { // object (previous id)
        color_token = p_pre;
      }
    } else { // new id
      if (incluye_nombres) {
        *ptr = _ivnom; // add a new id
      } else {
        icvnom.b = _ivnom; // remove it from cvnom
      }
      color_token = p_id;
    }
    break;

  case l_spc:
    while ((*++_source) == ' ') {}
    color_token = p_spc;
    break;

  case l_lit:
    if (in_comment > 0) {
      color_token = p_rem;
      _source++;
      break;
    }
    color_token = p_lit;
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
    if (in_comment > 0) {
      color_token = p_rem;
      _source++;
      break;
    }
    color_token = p_num;
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
    color_token = (*e).token;
    while ((e = (*e).next)) {
      while (*_source != (*e).caracter && (*e).alternative)
        e = (*e).alternative;
      if (*_source++ == (*e).caracter && (*e).token) {
        color_token = (*e).token;
        _ivnom = _source;
      }
    }
    _source = _ivnom;

    if (color_token == p_rem && in_comment <= 0) {
      while (*_source != cr && *_source)
        _source++;
    } else if (color_token == p_ini_rem) {
      color_token = p_rem;
      in_comment++;
      numrem++;
    } else if (color_token == p_end_rem) {
      color_token = p_rem;
      in_comment--;
      numrem--;
    } else if (in_comment > 0)
      color_token = p_rem;
    else
      color_token = p_sym;

    break;
  }
  color_source = _source;
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

  int t = 0; //token (color_token)
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
          e = clex_case[*buf] = iclex_symbols++;
          (*e).caracter = *buf++;
        } else
          buf++;
        while (*buf != ' ' && *buf != tab && *buf != cr) {
          if (lower[*buf])
            col_error(0, 4);
          if ((*e).next == 0)
            if (cnum_nodos++ == max_nodos)
              col_error(0, 3);
            else
              e = (*e).next = iclex_symbols++;
          else {
            e = (*e).next;
            while ((*e).caracter != *buf && (*e).alternative)
              e = (*e).alternative;
            if ((*e).caracter != *buf) {
              if (cnum_nodos++ == max_nodos)
                col_error(0, 3);
              else
                e = (*e).alternative = iclex_symbols++;
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
  color_source = buf;
  do {
    color_lex();
  } while (color_source < buf + len);
  incluye_nombres = 0;

  free(_cbuf);
  _cbuf = NULL;
  fclose(cdef);
  cdef = NULL;
}
