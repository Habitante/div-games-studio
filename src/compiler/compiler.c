
// Strings, new: p_string, string_global, string_local, p_*char, l*chr, ...
// 00 - intermediate code optimization
// 00 - tables and structs up to 3 dimensions
// 01 - add global strings
// 02 - test in PRG, trigger all new errors
// 03 - add new instruction set to interpreter
// 04 - prepare debugger for string support
// 05 - add local and private strings
// 06 - runtime range checking (strings with 0xdad00000 prefix)
// 07 - HEADER_LENGTH=9, instead of 36
// 08 - global member strings
// 09 - local member strings
// 10 - struct initialization handling member strings
// 11 - STRING p,q; defines 2 strings (instead of one string and one int)
// 12 - allow initializing records in structs globally (struct pepe[9] x=10; end)
// 13 - compiler_options statement (_max_process,_extended_conditions,_simple_conditions,_case_sensitive,_ignore_errors,_free_sintax)
// 14 - auto "\0" and ZERO2SPACE when putting a char in a string (strfix)
// 15 - new type system (int/char/string) in expressions and conditions (tipo_expresion)
// 16 - auto conversion of "A" to char("A") in char assignments (literal/constant literal/string -> char)
// 17 - operator overloading of = += + for string operations (strcpy,strcat,stradd)
// 18 - add and remove chars with string++/--, string-=n, string-n (strdec,strsub,strlen)
// 19 - allow defining literals across multiple source lines (consecutive ones merge into one)
// 20 - fixed syntax colorizer bug (on reaching eof with unclosed /*)
// 21 - string comparisons (strigu/dis/may/men/mai/mei)
// 22 - new system with 4 "in-flight" strings to allow more string operations (+/-)
// 23 - discarded the idea of converting offsets to bytes (intended for &string[n],
//      bytes and words: but in [ptr+n] we can't know by how much to multiply n)
// 24 - fixed some bugs related to remote access
// 25 - merged mode7 reserved words with mode 8 ones
// 26 - added _no_check (skip range and id checks) and _no_strfix options
// 27 - ASCII functions: char,strcpy,strcat,strlen,strcmp,strchr,strstr,strset,upper,lower and strdel
// 28 - enabled a patch to tell the compiler that some functions return a string
// 29 - allow receiving a parameter in a global or local string/char (cpachr, cpastr)
// 30 - enabled multi-par/pri system and fixed some issues with process calls
// 31 - enabled receiving a parameter in a private string, including its length (e.g., proc(string s[2]))
// 32 - allow redeclaring a string parameter inside the private section (with the same length)
// 33 - "function" type processes (don't return to caller until finished) (FCount,CallerId)
// 34 - fixed function return handling (to the caller of the last process)
// 35 - allow STEP over functions (does a "go here!" to the next statement)
// 36 - add global bytes and a new, again, data initialization system
//      (see p_byte, byte_global and p_pointerbyte)
// 37 - prepare debugger for byte and word support
// 38 - final debugger changes (in View Data), and Spanish translation.
// 39 - add local and private bytes (byte_local)
// 40 - byte members of structs (global/local and private)
// 41 - struct initialization handling byte members (frm,ifrm_max)
// 42 - added word handling instructions to interpreter (???wor)
// 43 - words same as bytes (word_global word_local *word ???wor)
// 44 - allow receiving a byte or word as parameter (and redeclaring as private)
// 45 - colorizer bug (below process ... put ?????????)
// 46 - show functions in process lists (F5) too (editor and debugger)
// 47 - bug where sprites disappeared completely in mode 8
// 48 - move IMPORTs to program start and emit proper error for old-style IMPORTs (and adapt_palette help)
// 49 - removed listing formatter (left in divfrm.cpp in case it's needed later, separate from divc.cpp)
// 50 - allow functions to return a value with return after FRAME (multi-stack system)
// 51 - added optional INT type for conventional integer data in DIV 1
// 52 - INT pointer declaration for global/local/private and struct members (tpigl,tpilo) up to 3 dimensions
// 53 - INT pointer usage, code generation for them (and testing)
// 54 - debugger support for global and local INT pointers (p, p[0], p[1], ...)
// 55 - int pointers as parameters and redeclaration in PRIVATE section
// 56 - forbid private variable redeclaration (only once as parameter and once as private)
// 57 - byte/word pointers, global/local/private, member or not, declaration and code
// 58 - debugger support for byte and word pointers (global and local)
// 59 - removed the ability to use * to define a pointer (debugger uses POINTER instead)
//      (important to discourage using *pointer later ...)
// 60 - test int/word/byte pointers (glo/loc/pri/mem/par)
// 61 - implemented "string pointer" exactly like "byte pointer" (only display differs in debug)
// 62 - struct pointers: STRUCT POINTER STRUCTNAME var; (defaults to 0)
// 63 - fixed access to members of the original struct
// 64 - debugger support for struct pointers (!!! harder than expected)
// 65 - validate_address() function in DEBUG to prevent page faults where possible
// 66 - fixed countless problems with pointers, the debugger, and everything else ...
// 67 - make int/byte/word/string pointer p1,p2; define two pointers
// 68 - screen_buffer function to copy a screen region to a graphic region
// 69 - argc/argv like in C, with program parameters
// 70 - new function loading system, defining return type and parameter types
//      (and removed previously special-cased string function exceptions)
// 71 - sort and shuffle functions (struct_name,field,mode)
// 72 - minimized keyboard ring buffer during game execution
// 73 - check "NULL pointed data" read/write access
// 74 - options _no_id_check _no_range_check _no_null_check (grouped under _no_check)
// 75 - compile option _no_optimize (to disable optimization)

//    fopen, fclose, fread, fwrite, fseek, ftell, filelen (ver flushall, fcloseall)
//    findfisrt, findnext, fullpath, splitpath, set/getcwd, set/getdrive, fprintf
//    remove (danidel), chdir, mkdir

//    - document new errors (strings, bytes, words and pointers) in help
//    - malloc/free functions
//    - write book

// A - Allow includes of {constants/globals/locals/processes}
// C - Multi-assignments, using hidden private variables (x=3,45,y*10,2..4,-8;)
// D - think of some "macro" trick easy to implement that works well
// E - Finish the auto-formatter for listings
// F - Generate an informative NFO of the compiled program (TAB+LST+EML)

// -----------------------------------------------------------------------------

// SOLUTION 1: BYTE/WORD POINTERS LIMITED TO BYTE/WORD TABLES

// NOTE: ptrbyte++ increments by 4 bytes (sizeof(int)), not 1 byte,
//       and ptrword++ increments by 2 words, not 1 word.

// (*)  To make *ptrbyte equivalent to ptrbyte[0] (presumably by adding
//      a new type like p_pointerb?), we should emit an error on
//      ++, --, +=, etc. on a p_pointerb/w ...
//    - Problem: how to forbid ptrbyte=ptrbyte+1;?
//    - We could also allow the operations, even if the results are wrong

//    - Even so, how to explain that ptrint++ works, but ptrbyte++ doesn't?

//    - Advantages: it's (almost) done, no incompatibilities with
//      DIV1, little time available ..., still allows
//      dynamic memory usage ... and we can always switch to SOLUTION 3
//      later

//    - perhaps this shouldn't be called pointer ... but something like
//      "dynamic tables" (since you can malloc them)

//    - Also useful for passing tables and structs as parameters.
//    - Perhaps we should forbid using "p_mul" (*) to differentiate them from
//      C pointers.

//    - The question is: forbid arithmetic (add, etc.) on ptrbyte/word? -> NO

//    - Allow *ptrbyte as ptrbyte[0]? -> NO, since that would suggest
//      byte access with *ptrbyte, which isn't possible

// SOLUTION 2: BYTE/WORD POINTERS RELATIVE TO &MEM[0]

//    - NOTE: byte and word pointers could be different types
//    - (pointerb/w could be a byte/word offset relative to &memb[0])
//    - New design for tpwgl tpwlo tpbgl tpblo? (keeping them always multiplied *2 or *4)
//    - a) during initialization must multiply *2 or *4 the constant (ptrbyte=&tablabyte)
//    - b) how the hell do you do ptrbyte=&whatever? (parameter passing, assignment, etc...)
//    - c) and what about ptrword=ptrbyte?
//    - d) moreover, what if a ptrbyte is sent as parameter but received as ptrint?

//    - Could only be done by dividing all ptrbyte reads by 4
//      and multiplying writes by 4, and even then:
//      ptrbyte1=ptrbyte2 would FAIL!!! (or passing/receiving a ptrbyte parameter)
//      [ptrbyte+n] ... and *ptrbyte?

//    - This is nonsense, since ptrbyte=ptrbyte+1 would still add 4!!!

//    - Advantages: not sure if the middle ground is the
//      right approach here...

// SOLUTION 3: CONVERT ALL OFFSETS TO BYTE

//    - There should be some way to have all offsets multiplied by 4 (in bytes)

//    - Problems: ["ptr"+n] (by how much to multiply n), plus how to resolve
//      [ptrint+ptrbyte+2*x...], interpreter access to all mem data
//      mem[id+_Graph], the IDs themselves (multiply or not? -> better not, let p_punto multiply,
//      since they must also be odd), old programs using pointers
//      (p=&kk; *p..., p++, *p... should emit a compile error when using *p or [p],
//      forcing redeclaration of p as ptrint), ptrint++ becomes ptrint+=4 and ptrword++ becomes ptrword+=2,

//    - sizeof() would also change to bytes, and file operations too,
//      increasing DIV1 incompatibility (since users may have used
//      save("pepe.dat",&table,10) ...

//    - Summary: the big problems are the interpreter (especially
//      the "#define memo(x) *(int*)(memb+x)" approach),
//      [ptr???+???] (and ptrint++), and DIV1 incompatibilities

//    - Advantages: free access to any data type, string simplification,
//      making "byte a" actually occupy only one byte, ...
//      it's the correct way to do it, and if not done now it may
//      never be done.

// SOLUTION 4: CONVERT OFFSETS TO FLOAT

//    - I know this sounds crazy at first, but wouldn't it be ideal
//      to have byte offsets as .25, .50 and .75?

//    - The cool thing is ptrint++ would still add 1.0

//    - and for byte access you'd add .25, the problem is everything
//      would be full of _CHP calls and run very slowly

//    - Also, how would offset or sizeof() return a float if the
//      language doesn't support them?

// NOTE: Offsets are NOT multiplied by 4 in this version.
//       (This was attempted once but reverted.)

// -----------------------------------------------------------------------------

// Intermediate code optimization

// 1st step: Create a mnemonic generation function, instead of
//           direct access to mem[], to isolate the optimization problem

// can't generate bytecode directly, but can use the "byte" system
// as EXE compression, decompressing in the interpreter on load

// -----------------------------------------------------------------------------

// Multidimensional tables and structs
// review errors: 35, ...
// TODO: Review help text for single-dimension tables and structs

//-----------------------------------------------------------------------------
//      DIV - Internal Compiler
//
//      Orchestration, lexer, symbol table loading, error handling,
//      output generation. Parser and expression/codegen are in
//      compiler_parser.c and compiler_expression.c respectively.
//-----------------------------------------------------------------------------

#include "compiler_internal.h"
#include "div_stub.h"

#ifndef ZLIB
#define uLongf unsigned long
#endif

//-----------------------------------------------------------------------------
//      Error recovery (setjmp/longjmp)
//-----------------------------------------------------------------------------

static jmp_buf buf;

void comp(void) {
  if (!setjmp(buf))
    compile();
}

void comp_exit(void) {
  longjmp(buf, 1);
}

//-----------------------------------------------------------------------------
//      Compiler global variables (definitions — declared extern in header)
//-----------------------------------------------------------------------------

byte file_in[_MAX_PATH + 1], file_out[_MAX_PATH + 1];

FILE *fin, *fout;
byte *_source = NULL;
byte *source, *ierror, *ierror_end;

int strings_length;
int strings_start;
int max_process;
int ignore_errors;
int free_sintax;
int extended_conditions;
int simple_conditions;
int check_range;
int check_id;
int check_null;
int enable_strfix;
int optimize;

int old_line;
byte *old_ierror, *old_ierror_end;

byte *last_line, zero = 0;

int start_addr, end_addr;
int start_line, start_col;
int end_line, end_col;

struct object obj[MAX_OBJECTS], *iobj;

byte *vnom = NULL;
union name_index_u name_index;

byte *objects_start;
byte *vhash[256];

struct exp_ele tabexp[MAX_EXP_SIZE], *_exp;
struct lex_ele lex_simb[MAX_NODES], *ilex_simb, *lex_case[256];

int current_token, token_value;
struct object *o;
struct object *current_scope;
struct object *lexical_scope;
struct object *member;

int source_line, num_nodes, num_objects;
int num_predefined;

int in_params;
int cross_process_access;

int tbreak[max_break];
int itbreak;
int tcont[max_cont];
int itcont;

int eval_stack[EVAL_STACK_SIZE + MAX_EXP_SIZE + 64];

int32_t *mem_ory = NULL, *frm = NULL;
int32_t *mem = NULL, *loc = NULL;
int imem, imem_max;
int iloc, iloc_max;
int itxt, ifrm_max;

byte *imemptr;
int local_var_len;

int comment_depth = 0;
int emit_tokens = 0;

int error_number = -1;
int error_line;
int error_col;
int error_27 = 27;

int program_type = 0;

struct peephole_entry code[16];

FILE *linf; // EXEC.LIN
FILE *lprg; // EXEC.PGM
FILE *lins; // EXEC.INS

FILE *def;
byte *_buf;

void init_compiler(void) {
  int n;
  for (n = 0; n < 256; n++)
    if (lower[n] == ' ')
      lower[n] = 0;
}

//-----------------------------------------------------------------------------
//      Prepare compilation for a program
//-----------------------------------------------------------------------------

void show_compile_message(byte *p) {
  if (compile_mode) {
    fprintf(stdout, "%s\n", p);
  } else {
    wbox(v.ptr, v.w / big2, v.h / big2, c2, 2, 20, v.w / big2 - 4, 7);
    wwrite(v.ptr, v.w / big2, v.h / big2, 3, 20, 0, p, c3);
    flush_window(0);
    flush_copy();
  }
}

/* Top-level compilation entry point.
 * Resets all compiler state, loads lexer/object tables, allocates output
 * buffers, runs the parser, fills the executable header, and writes the
 * compressed bytecode to system/EXEC.EXE.
 */
void compile(void) {
  int n;
  uLongf m;
  struct object *i;
  byte *q, *p;
  FILE *f;

  program_type = 0;

  free_resources();

  show_compile_message(texts[201]);

  vnom = NULL;
  mem_ory = NULL;
  mem = NULL;
  loc = NULL;
  frm = NULL;
  def = NULL;
  _buf = NULL;
  _source = NULL;

  itbreak = 0;
  itcont = 0;
  emit_tokens = 0;
  comment_depth = 0;

  member = NULL;
  current_scope = NULL;
  lexical_scope = NULL;

  delete_code();

  memset(obj, 0, sizeof(obj));
  iobj = obj;
  num_objects = 0;
  memset(lex_simb, 0, sizeof(lex_simb));
  ilex_simb = lex_simb;
  num_nodes = 0;
  memset(vhash, 0, sizeof(vhash));

  for (n = 0; n < 256; n++)
    if (lower[n])
      if (n >= '0' && n <= '9')
        lex_case[n] = (struct lex_ele *)l_num;
      else
        lex_case[n] = (struct lex_ele *)l_id;
    else
      lex_case[n] = (struct lex_ele *)l_err;

  if ((vnom = (byte *)malloc(MAX_OBJECTS * ID_AVG_LENGTH + 1024)) == NULL)
    c_error(0, 0);

  name_index.b = vnom;

  analyze_ltlex();

  lex_case[' '] = (struct lex_ele *)l_spc;
  lex_case[tab] = (struct lex_ele *)l_spc;
  lex_case[cr] = (struct lex_ele *)l_cr;

  objects_start = name_index.b;

  // NOTE: These fopen failures would report "insufficient memory" (error 0),
  // but the actual cause is a file-creation failure, not a memory issue.
  linf = fopen("system/exec.fs", "wb");
  if (linf) {
    fwrite(&fsmode, 1, 1, linf);
    fclose(linf);
  }
  linf = fopen("system/exec.path", "wb");
  if (linf) {
    fputs((char *)&file_types[8], linf);
    fclose(linf);
  }
  if ((linf = fopen("system/exec.lin", "wb")) == NULL)
    c_error(0, 0);
  if ((lprg = fopen("system/exec.pgm", "wb")) == NULL)
    c_error(0, 0);


  imem_max = DEFAULT_BUFFER;
  imem = 0;
  if ((mem_ory = mem = (int *)malloc(imem_max * sizeof(memptrsize))) == NULL)
    c_error(0, 0);
  memset(mem, 0, imem_max * sizeof(memptrsize));

  iloc_max = DEFAULT_BUFFER / 2;
  iloc = 0;
  local_var_len = 0;
  if ((loc = (int *)malloc(iloc_max * sizeof(memptrsize))) == NULL)
    c_error(0, 0);
  memset(loc, 0, iloc_max * sizeof(memptrsize));

  ifrm_max = DEFAULT_BUFFER / 2;
  if ((frm = (int *)malloc(ifrm_max * sizeof(memptrsize))) == NULL)
    c_error(0, 0);
  memset(frm, 0, ifrm_max * sizeof(memptrsize));

  show_compile_message(texts[203]);

  imem = HEADER_LENGTH;

  preload_objects(); // No literals in the preloaded objects

  show_compile_message(texts[200]);

  source = source_ptr;
  _source = source;
  *(source + source_len) = cr;
  *(source + source_len + 1) = cr;

  emit_tokens = 1;
  last_line = source;
  cross_process_access = 0;
  in_params = 0;
  source_line = 1;

  itxt = strings_start = imem;
  psintactico(); // To obtain "strings_length"
  imem += strings_length;

  test_buffer(&mem, &imem_max, imem);

  num_predefined = num_objects;

  emit_tokens = 1;
  last_line = source;
  fwrite(&zero, 1, 1, lprg);
  cross_process_access = 0;
  in_params = 0;
  source_line = 1;

  parser();

  i = obj;
  while (i < iobj) {
    if (i->used) {
      source_line = i->line;
      ierror = i->ierror;
      c_error(0, 121);
    }
    i++;
  }

  mem[2] = imem;
  mem[3] = max_process; // Previously HEADER_LENGTH, now unused
  mem[4] = 0;           // Previously mem[1]-mem[3] (global data length), now unused
  mem[5] = local_var_len - iloc;
  mem[6] = iloc;
  mem[7] = 0;           // Previously imem+iloc (text start), now unused
  mem[8] = imem + iloc; // Number of elements used in mem[]

  show_compile_message(texts[204]);

  save_dbg();
  save_exec_bin();

#ifdef listados
  list_objects();
  list_assembler();
#endif

  show_compile_message(texts[205]);

#ifdef SHARE
  mem[0] += 1024;
#endif

  if (program_type == 1)
    if ((f = fopen("install/setup.ovl", "wb")) != NULL) {
      fwrite(div_stub, 1, 602, f);
      p = (byte *)malloc((imem + iloc) * 4);
      m = (imem + iloc) * 4 + 1024;
      q = (byte *)malloc(m);
      if (p != NULL && q != NULL) {
        fwrite(mem, 4, 9, f);
        memcpy(p, &mem[9], (imem - 9) * 4);
        memcpy(p + (imem - 9) * 4, loc, iloc * 4);
        n = (imem - 9 + iloc) * 4;
#ifndef ZLIB
        if (true)
#else
        if (!compress(q, (uLongf *)&m, p, (unsigned long)n))
#endif
        {
          fwrite(&n, 1, 4, f); // mem[0]..mem[8],uncompressed_data_length,compressed_data...
          fwrite(q, 1, m, f);
          free(q);
          free(p);
          fclose(f);
        } else {
          free(q);
          free(p);
          fclose(f);
          c_error(0, 0);
        }
      } else {
        if (p != NULL)
          free(p);
        if (q != NULL)
          free(q);
        fclose(f);
        c_error(0, 0);
      }
    }

  if (run_mode == 3)
    mem[0] += 128;
  if (ignore_errors)
    mem[0] += 512;

  if ((f = fopen("system/EXEC.EXE", "wb")) != NULL) {
    fwrite(div_stub, 1, 602, f);
    p = (byte *)malloc((imem + iloc) * 4);
    m = (imem + iloc) * 4 + 1024;
    q = (byte *)malloc(m);
    if (p != NULL && q != NULL) {
      fwrite(mem, 4, 9, f); // mem[0..8]
      memcpy(p, &mem[9], (imem - 9) * 4);
      memcpy(p + (imem - 9) * 4, loc, iloc * 4);
      n = (imem - 9 + iloc) * 4;
#ifndef ZLIB
      if (true)
#else
      if (!compress(q, (uLongf *)&m, p, n))
#endif
      {
        fwrite(&n, 1, 4, f); // mem[0]..mem[8],uncompressed_data_length,compressed_data...
        fwrite(q, 1, m, f);
        free(q);
        free(p);
        fclose(f);
      } else {
        free(q);
        free(p);
        fclose(f);
        c_error(0, 0);
      }
    } else {
      if (p != NULL)
        free(p);
      if (q != NULL)
        free(q);
      fclose(f);
      c_error(0, 0);
    }
  }

  // _case_sensitive
  memcpy(lower + 129,
         "ueaaaa\x87"
         "eeeiiiaae\x91\x91ooouuyou\x9b\x9c\x9d\x9e\x9f"
         "aiou",
         35);
  memcpy(lower + 'A', "abcdefghijklmnopqrstuvwxyz", 26);
  lower['\xa5'] = '\xa4';

  comp_exit();
}

void free_resources(void) {
  if (def != NULL)
    fclose(def);
  if (_buf != NULL)
    free(_buf);
  if (frm != NULL)
    free(frm);
  if (loc != NULL)
    free(loc);
  if (mem != NULL)
    free(mem);
  if (lins != NULL)
    fclose(lins);
  if (lprg != NULL)
    fclose(lprg);
  if (linf != NULL)
    fclose(linf);
  if (vnom != NULL)
    free(vnom);

  def = NULL;
  _buf = NULL;
  frm = NULL;
  loc = NULL;
  mem = NULL;
  lins = NULL;
  lprg = NULL;
  linf = NULL;
  vnom = NULL;
}

//-----------------------------------------------------------------------------
//      Internal compiler errors
//-----------------------------------------------------------------------------

int _le, _t;
byte *_ie;

void save_error(word tipo) { // Save an error position (0 .. 3)
  switch (tipo) {
  case 0:
    _le = source_line;
    _ie = ierror;
    break;
  case 1:
    _le = old_line;
    _ie = old_ierror_end;
    break;
  case 2:
    _le = old_line;
    _ie = old_ierror;
    break;
  case 3:
    _le = old_line;
    _ie = old_ierror_end;
    break;
  }
  _t = tipo;
}

// 0 - Start of current token
// 1 - End of previous token plus one space (if possible)
// 2 - Start of previous token
// 3 - End of previous token
// 4 - Error saved with save_error(0..3)

void c_error(word tipo, word e) {
  int column = 0;
  byte *_p = NULL, *p = NULL;

  error_number = e;

  switch (tipo) {
  case 0:
    error_line = source_line;
    p = ierror;
    break;
  case 1:
    error_line = old_line;
    _p = p = old_ierror_end;
    break;
  case 2:
    error_line = old_line;
    p = old_ierror;
    break;
  case 3:
    error_line = old_line;
    _p = p = old_ierror_end;
    break;
  case 4:
    error_line = _le;
    _p = p = _ie;
    tipo = _t;
    break;
  }

  if (e >= 10) {
    while (--p >= source_ptr) {
      if (*p == cr)
        break;
      column++;
    }
    if (p < source_ptr)
      column++;
  }

  if (tipo == 1) {
    if (*(_p + 1) == ' ' || *(_p + 1) == cr || *(_p + 1) == lf) {
      column++;
    }
    column++;
  } else if (tipo == 3)
    column++;

  error_col = column;

  // _case_sensitive
  memcpy(lower + 129,
         "ueaaaa\x87"
         "eeeiiiaae\x91\x91ooouuyou\x9b\x9c\x9d\x9e\x9f"
         "aiou",
         35);
  memcpy(lower + 'A', "abcdefghijklmnopqrstuvwxyz", 26);
  lower['\xa5'] = '\xa4';

  comp_exit();
}

//-----------------------------------------------------------------------------
//  With the first token read, save the start of a statement
//-----------------------------------------------------------------------------

void statement_start(void) {
  byte *p = ierror - 1;
  start_addr = imem;
  start_line = source_line;
  start_col = 0;
  if (p < _source)
    p = _source;

  while (*p != cr && *p != lf && p > _source) {
    start_col++;
    p--;
  }
}

//-----------------------------------------------------------------------------
//  With the first token not belonging to the statement, save the end of a statement
//-----------------------------------------------------------------------------

void statement_end(void) {
  byte *p = old_ierror_end - 1;
  end_addr = imem - 1;
  end_line = old_line;
  end_col = 0;

  if (p < _source)
    p = _source;

  while (*p != cr && *p != lf && p > _source) {
    end_col++;
    p--;
  }
}

//-----------------------------------------------------------------------------
//  Write a record (start_addr,end_addr,start_line,...) to EXEC.LIN
//-----------------------------------------------------------------------------

void record_statement(void) {
  fwrite(&start_addr, 4, 1, linf);
  fwrite(&end_addr, 4, 1, linf);
  fwrite(&start_line, 4, 1, linf);
  fwrite(&start_col, 4, 1, linf);
  fwrite(&end_line, 4, 1, linf);
  fwrite(&end_col, 4, 1, linf);
}

//-----------------------------------------------------------------------------
//      Check code generation buffer limits
//-----------------------------------------------------------------------------

void test_buffer(int **buffer, int *maximo, int n) {
  int max;

  if (n + BUFFER_INCREASE > 2 * 1000 * 1000)
    c_error(0, 0);

  if (n > *maximo - SAFETY_BUFFER) {
    max = *maximo;
    *maximo = n + BUFFER_INCREASE;
    max = *maximo - max;
    if (*buffer == mem) {
      if ((*buffer = mem_ory = (int *)realloc(*buffer, *maximo * sizeof(memptrsize))) == NULL)
        c_error(0, 0);
    } else {
      if ((*buffer = (int *)realloc(*buffer, *maximo * sizeof(memptrsize))) == NULL)
        c_error(0, 0);
    }
    memset((byte *)*buffer + (*maximo - max) * sizeof(memptrsize), 0, max * sizeof(memptrsize));
  }
}

//-----------------------------------------------------------------------------
//      Preload lexical structures, analyze the ltlex.def file
//-----------------------------------------------------------------------------

void analyze_ltlex(void) {
  byte *buf, cont = 1;
  int len;
  struct lex_ele *e;

  int t;  //token
  byte h; //hash (for id)
  byte *_ivnom;
  byte **ptr;

  if ((def = fopen("system/ltlex.def", "rb")) == NULL)
    c_error(0, 1);
  fseek(def, 0, SEEK_END);
  len = ftell(def);
  if ((_buf = buf = (byte *)malloc(len + 2)) == NULL)
    c_error(0, 0);
  fseek(def, 0, SEEK_SET);
  len = fread(buf, 1, len, def);
  *(buf + len) = cr;
  *(buf + len + 1) = cr;

  source_line = 1;
  do {
    switch (*buf++) {
    case ' ':
    case tab:
      break;
    case cr:
      if (*buf == cr)
        cont = 0;
      else {
        source_line++;
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
        c_error(0, 2);
      *buf = lower[*buf];
      if (*buf >= '0' && *buf <= '9')
        t += (*buf++ - '0');
      else if (*buf >= 'a' && *buf <= 'f')
        t += (*buf++ - 'a' + 10);
      else
        c_error(0, 2);
      if (*buf == cr || *buf == ' ' || *buf == tab)
        break;
      else if (lower[*buf]) { //Analyzes a keyword
        _ivnom = name_index.b;
        *name_index.p++ = 0;
        *name_index.p++ = (byte *)t;
        h = 0;
        while ((*name_index.b = lower[*buf++]))
          h = ((byte)(h << 1) + (h >> 7)) ^ (*name_index.b++);
        ptr = &vhash[h];
        while (*ptr)
          ptr = (byte **)*ptr;
        *ptr = _ivnom;

        buf--;
        name_index.b++;
      } else if (t >= 0x78 && t <= 0x7b) { //Analyzes a literal delimiter
        lex_case[*buf] = (struct lex_ele *)l_lit;
      } else { //Analyzes a new symbol
        if ((e = lex_case[*buf]) == 0) {
          if (num_nodes++ == MAX_NODES)
            c_error(0, 3);
          e = lex_case[*buf] = ilex_simb++;
          (*e).character = *buf++;
        } else
          buf++;
        while (*buf != ' ' && *buf != tab && *buf != cr) {
          if (lower[*buf])
            c_error(0, 4);
          if ((*e).next == 0) {
            if (num_nodes++ == MAX_NODES)
              c_error(0, 3);
            else
              e = (*e).next = ilex_simb++;
          } else {
            e = (*e).next;
            while ((*e).character != *buf && (*e).alternative)
              e = (*e).alternative;
            if ((*e).character != *buf) {
              if (num_nodes++ == MAX_NODES)
                c_error(0, 3);
              else
                e = (*e).alternative = ilex_simb++;
            }
          }
          (*e).character = *buf++;
        }
        (*e).token = t;
      }
      break;
    }
  } while (cont);

  free(_buf);
  _buf = NULL;
  fclose(def);
  def = NULL;
}

//-----------------------------------------------------------------------------
//      Preload the object table, analyze the ltobj.def file
//-----------------------------------------------------------------------------

// WARNING: Memory limits are not checked during object preloading.

void preload_objects(void) {
  int len, ret, cod;
  struct object *ob;
  struct object *ob2;

  if ((def = fopen("system/ltobj.def", "rb")) == NULL)
    c_error(0, 6);
  fseek(def, 0, SEEK_END);
  len = ftell(def);
  if ((_buf = source = (byte *)malloc(len + 2)) == NULL)
    c_error(0, 0);
  fseek(def, 0, SEEK_SET);
  len = fread(source, 1, len, def);
  *(source + len) = cr;
  *(source + len + 1) = cr;

  source_line = 1;
  lexer();

  while (current_token != p_end_of_file)
    switch (current_token) {
    case p_const:
      lexer();
      if (current_token != p_id)
        c_error(0, 7);
      ob = o;
      if ((*ob).type != tnone)
        c_error(0, 7);
      (*ob).type = tcons;
      lexer();
      if (current_token != p_asig)
        c_error(0, 7);
      lexer();
      if (current_token == p_num)
        (*ob).cons.value = token_value;
      else if (current_token == p_sub) {
        lexer();
        if (current_token != p_num)
          c_error(0, 7);
        (*ob).cons.value = -token_value;
      } else
        c_error(0, 7);
      (*ob).cons.literal = 0;
      lexer();
      break;
    case p_global:
      lexer();
      if (current_token == p_id) {
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 7);
        lexer();
        if (current_token == p_corab) {
          (*ob).type = ttglo;
          (*ob).table_global.offset = imem;
          lexer();
          if (current_token != p_num)
            c_error(0, 7);
          (*ob).table_global.len1 = len = token_value;
          (*ob).table_global.len2 = -1;
          (*ob).table_global.len3 = -1;
          (*ob).table_global.total_len = len + 1;
          do {
            mem[imem++] = 0;
          } while (len--);
          lexer();
          if (current_token != p_corce)
            c_error(0, 7);
          lexer();
        } else {
          (*ob).type = tvglo;
          (*ob).var_global.offset = imem;
          if (current_token != p_asig) {
            c_error(0, 7);
          }
          lexer();
          if (current_token == p_num)
            mem[imem++] = token_value;
          else if (current_token == p_sub) {
            lexer();
            if (current_token != p_num)
              c_error(0, 7);
            mem[imem++] = -token_value;
          } else
            c_error(0, 7);
          lexer();
        }
      } else if (current_token == p_struct) {
        lexer();
        if (current_token != p_id)
          c_error(0, 7);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 7);
        lexer();
        if (current_token != p_corab)
          c_error(0, 7);
        (*ob).type = tsglo;
        (*ob).struct_global.offset = imem;
        lexer();
        if (current_token != p_num)
          c_error(0, 7);
        (*ob).struct_global.dim1 = token_value;
        (*ob).struct_global.dim2 = (*ob).struct_global.dim3 = -1;
        (*ob).struct_global.total_items = token_value + 1;
        lexer();
        if (current_token != p_corce)
          c_error(0, 7);
        (*ob).struct_global.item_len = 0;
        member = ob;
        lexer();
        if (current_token == p_end) {
          c_error(0, 7);
        }
        len = 0;
        while (current_token != p_end) {
          if (current_token == p_string) {
            lexer();
            if (current_token != p_id)
              c_error(0, 7);
            if ((*o).type != tnone)
              c_error(0, 7);
            ob2 = o;
            lexer();
            (*ob2).type = tcglo;
            (*ob2).string_global.offset = len + 1;
            if (current_token != p_corab) {
              c_error(0, 7);
            }
            lexer();
            if (((*ob2).string_global.total_len = constant()) < 0) {
              c_error(0, 7);
            }
            if ((*ob2).string_global.total_len > 0xFFFFF) {
              c_error(0, 7);
            }
            if (current_token != p_corce) {
              c_error(0, 7);
            }
            lexer();

            mem[imem] = 0xDAD00000 | (*ob2).string_global.total_len;
            len += 1 + ((*ob2).string_global.total_len + 5) / 4;
            mem[imem + 1] = 0;
            imem += 1 + ((*ob2).string_global.total_len + 5) / 4;
            (*ob).struct_global.item_len += 1 + ((*ob2).string_global.total_len + 5) / 4;

          } else {
            if (current_token != p_id)
              c_error(0, 7);
            if ((*o).type != tnone)
              c_error(0, 7);
            ob2 = o;
            lexer();
            if (current_token == p_corab) {
              (*ob2).type = ttglo;
              (*ob2).table_global.offset = len;
              lexer();
              if (current_token != p_num)
                c_error(0, 7);
              (*ob2).table_global.len1 = token_value;
              (*ob2).table_global.len2 = -1;
              (*ob2).table_global.len3 = -1;
              (*ob2).table_global.total_len = token_value + 1;
              len += token_value + 1;
              memset(&mem[imem], 0, (token_value + 1) * 4);
              imem += token_value + 1;
              (*ob).struct_global.item_len += token_value + 1;
              lexer();
              if (current_token != p_corce)
                c_error(0, 7);
              lexer();
            } else {
              (*ob2).type = tvglo;
              (*ob2).var_global.offset = len++;
              mem[imem++] = 0;
              (*ob).struct_global.item_len++;
            }
          }
        }
        member = NULL;
        lexer();

        if ((*ob).struct_global.total_items > 1) {
          len = ((*ob).struct_global.total_items - 1) * (*ob).struct_global.item_len - 1;
          do {
            mem[imem] = mem[imem - (*ob).struct_global.item_len];
            imem++;
          } while (len--);
        }

      } else
        c_error(0, 7);
      break;
    case p_local:
      lexer();
      if (current_token == p_id) {
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 7);
        (*ob).type = tvloc;
        (*ob).var_local.offset = iloc;
        lexer();
        if (current_token != p_asig)
          c_error(0, 7);
        lexer();
        if (current_token == p_num)
          loc[iloc++] = token_value;
        else if (current_token == p_sub) {
          lexer();
          if (current_token != p_num)
            c_error(0, 7);
          loc[iloc++] = -token_value;
        } else
          c_error(0, 7);
        lexer();
      } else if (current_token == p_struct) {
        lexer();
        if (current_token != p_id)
          c_error(0, 7);
        ob = o;
        if ((*ob).type != tnone)
          c_error(0, 7);
        lexer();
        if (current_token != p_corab)
          c_error(0, 7);
        (*ob).type = tsloc;
        (*ob).struct_local.offset = iloc;
        lexer();
        if (current_token != p_num)
          c_error(0, 7);
        (*ob).struct_local.dim1 = token_value;
        (*ob).struct_local.dim2 = (*ob).struct_local.dim3 = -1;
        (*ob).struct_local.total_items = token_value + 1;
        lexer();
        if (current_token != p_corce)
          c_error(0, 7);
        (*ob).struct_local.item_len = 0;
        member = ob;
        lexer();
        if (current_token == p_end)
          c_error(0, 7);

        len = 0;

        while (current_token != p_end) {
          if (current_token != p_id)
            c_error(0, 7);
          if ((*o).type != tnone)
            c_error(0, 7);
          (*o).type = tvloc;
          (*o).var_local.offset = len;
          (*ob).struct_local.item_len++;
          lexer();
          if (current_token == p_asig) {
            lexer();
            if (current_token == p_num)
              loc[iloc + len] = token_value;
            else if (current_token == p_sub) {
              lexer();
              if (current_token != p_num)
                c_error(0, 7);
              loc[iloc + len] = -token_value;
            } else
              c_error(0, 7);
            lexer();
          } else
            loc[iloc + len] = 0;
          len++;
        }

        iloc += (*ob).struct_local.total_items * (*ob).struct_local.item_len;

        member = NULL;
        lexer();
      } else
        c_error(0, 7);
      break;

    case p_function: // function 0 int signal(0,0) // ....
      lexer();
      if (current_token != p_num)
        c_error(0, 7);
      cod = token_value;
      lexer();
      if (current_token == p_int)
        ret = 0;
      else if (current_token == p_string)
        ret = 1;
      else
        c_error(0, 7);
      lexer();
      if (current_token != p_id)
        c_error(0, 7);
      ob = o;
      if ((*ob).type != tnone)
        c_error(0, 7);
      (*ob).type = tfunc;
      lexer();
      if (current_token != p_abrir)
        c_error(0, 7);
      lexer();
      len = 0;
      while (current_token != p_cerrar) {
        if (current_token != p_num)
          c_error(0, 7);
        (*ob).func.par[len++] = token_value;
        lexer();
        if (current_token == p_coma)
          lexer();
      }
      (*ob).func.code = cod;
      (*ob).func.ret = ret;
      (*ob).func.num_params = len;
      lexer();
      break;

    default:
      c_error(0, 7);
      break;
    }

  free(_buf);
  _buf = NULL;
  fclose(def);
  def = NULL;
}

//-----------------------------------------------------------------------------
//  Predict the next lexical token to be read (and where it will be)
//-----------------------------------------------------------------------------

int next_pieza;
int next_line;
byte *next_source;

byte *next_lexico(byte *_source, int comment_depth, int source_line) { // Never generates errors

  byte **ptr, *_ivnom, h;
  struct lex_ele *e;

  if (!comment_depth && source_line) {
    old_line = source_line;
    old_ierror = ierror;
    old_ierror_end = ierror_end;
  }

lex_scan:

  if (source_line)
    ierror = _source;
  next_source = _source;

  switch ((uintptr_t)lex_case[*_source]) {
  case l_err:
    if (comment_depth) {
      _source++;
      goto lex_scan;
    }
    next_pieza = 0;
    break;

  case l_cr:
    if (source_line) {
      if (emit_tokens) {
        fwrite(last_line, 1, _source - last_line, lprg);
        fwrite(&zero, 1, 1, lprg);
      }

      source_line++;
      if ((*++_source) == lf) {
        _source++;
        last_line = _source;
        goto lex_scan;
      }
      next_pieza = 0;
      break;

    } else {
      if ((*++_source) == lf) {
        _source++;
        goto lex_scan;
      }
      next_pieza = 0;
      break;
    }

  case l_id:
    if (comment_depth) {
      _source++;
      goto lex_scan;
    }
    _ivnom = name_index.b;
    *name_index.p++ = 0;
    *name_index.p++ = 0;
    h = 0;
    while ((*name_index.b = lower[*_source++]))
      h = ((byte)(h << 1) + (h >> 7)) ^ (*name_index.b++);
    name_index.b++;
    _source--;
    ptr = &vhash[h];
    while (*ptr != NULL && strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8))
      ptr = (byte **)*ptr;
    if (!strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8)) { // id found
      name_index.b = _ivnom;                                 // remove it from vnom
      next_pieza = (intptr_t)*(ptr + 1);
      if (next_pieza < 256 && next_pieza >= 0) { // reserved word (token)
        if (next_pieza == p_rem) {
          while (*_source != cr)
            _source++;
          goto lex_scan;
        }
      } else { // object (existing id)
        next_pieza = p_id;
      }
    } else {
      name_index.b = _ivnom; // remove it from vnom
      next_pieza = p_id;     // new id
    }
    break;

  case l_spc:
    while ((*++_source) == ' ') {}
    goto lex_scan;

  case l_lit:
    if (comment_depth) {
      _source++;
      goto lex_scan;
    }
    next_pieza = p_lit;
    break;

  case l_num:
    if (comment_depth) {
      _source++;
      goto lex_scan;
    }
    next_pieza = p_num;
    break;

  default: // pointer to a lex_ele
    e = lex_case[*_source++];
    next_pieza = (*e).token;
    _ivnom = _source;

    while ((e = (*e).next)) {
      while (*_source != (*e).character && (*e).alternative)
        e = (*e).alternative;
      if (*_source++ == (*e).character && (*e).token) {
        next_pieza = (*e).token;
        _ivnom = _source;
      }
    }
    _source = _ivnom;

    if (next_pieza == p_rem && !comment_depth) {
      while (*_source != cr) {
        _source++;
      }
      goto lex_scan;
    }

    if (next_pieza == p_ini_rem) {
      comment_depth++;
      do {
        _source = next_lexico(_source, comment_depth, source_line);
        source_line = next_line;
      } while (next_pieza != p_end_rem);
      comment_depth--;
      goto lex_scan;
    }
    break;
  }
  next_line = source_line;
  return (_source);
}


//-----------------------------------------------------------------------------
//      Lexer (reads a new token from *source)
//-----------------------------------------------------------------------------

int is_wav(char *filename);
int in_fopen = 0;

/* Lexer: reads the next token from the source buffer.
 * Sets global 'current_token' to the token type, 'token_value' for numeric/literal
 * values, and 'o' for identifier objects. Also handles hash-table lookup
 * and insertion of new identifiers into the symbol table.
 */
void lexer(void) {
  struct object **ptr_o;
  byte **ptr, *_ivnom, h, *_source = source;
  struct lex_ele *e;
  char cwork[66];
  FILE *f;

  int packable;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  int n;

  if (!comment_depth) {
    old_line = source_line;
    old_ierror = ierror;
    old_ierror_end = ierror_end;
  }

lex_scan:
  ierror = _source;
  switch ((uintptr_t)lex_case[*_source]) { // Pointer to a lex_ele or l_???

  case l_err:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
    } else
      c_error(0, 10);
    break;

  case l_cr:

    if (emit_tokens) {
      fwrite(last_line, 1, _source - last_line, lprg);
      fwrite(&zero, 1, 1, lprg);
    }

    source_line++;
    if ((*++_source) == lf) {
      _source++;
      last_line = _source;
      goto lex_scan;
    }
    current_token = p_end_of_file;
    break; // eof

  case l_id:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
      break;
    }
    _ivnom = name_index.b;
    *name_index.p++ = 0;
    *name_index.p++ = 0;
    h = 0;
    while ((*name_index.b = lower[*_source++]))
      h = ((byte)(h << 1) + (h >> 7)) ^ (*name_index.b++);
    name_index.b++;
    _source--;
    if (name_index.b - vnom > MAX_OBJECTS * ID_AVG_LENGTH)
      c_error(0, 100);

    ptr = &vhash[h];
    while (*ptr && strcmp((char *)(ptr + 2), ((char *)_ivnom + ptr8)) != 0) {
      ptr = (byte **)*ptr;
    }

    if (!strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8)) { // id found
      name_index.b = _ivnom;                                 // remove it from vnom
      current_token = (intptr_t)*(ptr + 1);
      if (current_token < 256 && current_token >= 0) { // reserved word (t	oken)

        if (current_token == p_rem) {
          while (*_source != cr)
            _source++;
          goto lex_scan;
        }

      } else { // object (existing id)
        ptr_o = (struct object **)(ptr + 1);
        o = *ptr_o;
        current_token = p_id;
        while (o != NULL &&
               (((*o).scope && lexical_scope != (*o).scope) || ((*o).member != member)))
          o = (*o).prev;
        if (o == NULL) { // Not found
          o = iobj++;
          (*o).prev = *ptr_o;
          *ptr_o = o;
          (*o).name = (byte *)(ptr_o + 1);
          (*o).member = member;
          (*o).param = 0;
          if (in_params)
            (*o).scope = current_scope;
#ifndef SHARE
          if (num_objects++ == MAX_OBJECTS)
            c_error(0, 102);
#else
          if (num_objects++ == MAX_OBJECTS)
            c_error(0, 101);
#endif
        } else {
          if ((*o).type == tcons)
            token_value = (*o).cons.value;
        }
      }
    } else {
      *ptr = _ivnom;
      ptr_o = (struct object **)(_ivnom + ptr4);
      *ptr_o = o = iobj++;
      current_token = p_id; // new id
      (*o).name = (byte *)_ivnom + ptr8;
      (*o).member = member;
      if (in_params)
        (*o).scope = current_scope;
      if (num_objects++ == MAX_OBJECTS)
        c_error(0, 102);
    }
    break;

  case l_spc:
    while ((*++_source) == ' ') {}
    goto lex_scan;

  case l_lit:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
      break;
    }
    current_token = p_lit;
    h = *_source;
    _ivnom = name_index.b; // Literal between two h delimiters
    do {
      if (*++_source == cr)
        c_error(0, 11);
      if (*_source == h)
        if (*(_source + 1) == h)
          *_ivnom = *++_source;
        else
          *_ivnom = 0;
      else
        *_ivnom = *_source;
    } while (*_ivnom++);
    _source++;

    next_lexico(_source, 0, 0);
    while (next_pieza == p_lit) {
      next_lexico(_source, 0, source_line);
      source_line = next_line;
      _source = next_source;
      h = *_source;
      _ivnom--;
      do {
        if (*++_source == cr)
          c_error(0, 11);
        if (*_source == h)
          if (*(_source + 1) == h)
            *_ivnom = *++_source;
          else
            *_ivnom = 0;
        else
          *_ivnom = *_source;
      } while (*_ivnom++);
      _source++;
      next_lexico(_source, 0, 0);
    }

    n = (strlen((char *)name_index.b) + ptr4) / 4;
    memcpy(&mem_ory[itxt], name_index.b, strlen((char *)name_index.b) + 1);

    if (name_index.b[0] != '.' && (name_index.b[0] != '/' && name_index.b[1] != 0) &&
        strcmp("/", (char *)name_index.b) && (f = div_open_file((char *)name_index.b)) != NULL) {
      fprintf(stdout, "FOUND FILE: [%s] [%s] [%s]\n", (char *)name_index.b, full,
              (char *)&file_types[8]);

      packable = 0;

      // Determine whether the file is packable ...

      _splitpath(full, (char *)drive, (char *)dir, (char *)fname, (char *)ext);
      strupr((char *)ext);

      if (!stricmp((char *)ext, ".MOD") || !stricmp((char *)ext, ".S3M") ||
          !stricmp((char *)ext, ".XM")) {
        packable = 1;
      } else if (!stricmp((char *)ext, ".PCM")) {
        packable = 1;
      } else if (!stricmp((char *)ext, ".PCX")) {
        if (fread(cwork, 1, 66, f) > 0) {
          if (cwork[2] == 1 && cwork[3] == 8 && cwork[65] == 1)
            packable = 1;
        }
      } else if (fread(cwork, 1, 8, f) > 0) {
        if (!strcmp((char *)cwork, "pal\x1a\x0d\x0a"))
          packable = 1;
        if (!strcmp((char *)cwork, "map\x1a\x0d\x0a"))
          packable = 1;
        if (!strcmp((char *)cwork, "fpg\x1a\x0d\x0a"))
          packable = 1;
        if (!strcmp((char *)cwork, "fnt\x1a\x0d\x0a"))
          packable = 1;
        if (!strcmp((char *)cwork, "wld\x1a\x0d\x0a"))
          packable = 1;
        if (!strcmp((char *)cwork, "wld\x1a\x0d\x0a\x01"))
          packable = 1;
      }

      fclose(f);

      if (is_wav(full))
        packable = 1;

      // ???
      packable = 1;

      if (!packable || in_fopen)
        fwrite("+", 1, 1, lins);
      fwrite(full, 1, strlen(full) + 1, lins);
      fflush(lins);
    }

    token_value = itxt;
    itxt += n;
    break;

  case l_num:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
      break;
    }

    current_token = p_num;
    token_value = 0;
    if (*_source == '0' && lower[*(_source + 1)] == 'x') {
      _source += 2;
      while ((uintptr_t)lex_case[*_source] == l_num ||
             (lower[*_source] >= 'a' && lower[*_source] <= 'f')) {
        if ((uintptr_t)lex_case[*_source] == l_num)
          token_value = token_value * 16 + *_source++ - 0x30;
        else
          token_value = token_value * 16 + lower[*_source++] - 'a' + 10;
      }
    } else
      do {
        token_value = token_value * 10 + *_source++ - 0x30;
      } while ((uintptr_t)lex_case[*_source] == l_num);
    break;

  default: // pointer to a lex_ele

    e = lex_case[*_source++];
    _ivnom = _source;
    current_token = (*e).token;
    while ((e = (*e).next)) {
      while (*_source != (*e).character && (*e).alternative)
        e = (*e).alternative;
      if (*_source++ == (*e).character && (*e).token) {
        current_token = (*e).token;
        _ivnom = _source;
      }
    }
    _source = _ivnom;

    if (current_token == p_rem && !comment_depth) {
      while (*_source != cr)
        _source++;
      goto lex_scan;
    }
    if (current_token == p_ini_rem) {
      comment_depth++;
      do {
        source = _source;
        lexer();
        _source = source;
      } while (current_token != p_end_rem);
      comment_depth--;
      goto lex_scan;
    }

    if (current_token == p_end_of_file) {
      if (comment_depth)
        c_error(0, 55);
      else
        c_error(0, 12);
    }

    break;
  }
  source = _source;
  ierror_end = _source - 1;
}

//-----------------------------------------------------------------------------
//      Skip consecutive commas or semicolons
//-----------------------------------------------------------------------------

void skip_semicolons(void) {
  do {
    lexer();
  } while (current_token == p_ptocoma);
}

//-----------------------------------------------------------------------------
//      Write the object table file
//-----------------------------------------------------------------------------

#ifdef listados

void list_objects(void) {
  FILE *sta;
  int n, o, l;

  sta = fopen("div.tab", "wb");

  fprintf(sta, "\xfe Objetos de %s - DIV Versi\xa2n " version "\n\n", file_in);

  for (n = 1; n <= 8; n++)
    fprintf(sta, "[%u]: %u\n", n, mem[n]);
  fprintf(sta, "\n");

  for (n = 0; n < num_objects; n++) {
    switch (obj[n].type) {
    case tnone:
      fprintf(sta, "%5u\ttnone: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      break;
    case tcons:
      fprintf(sta, "%5u\ttcons: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\tvalor=%u\n", obj[n].cons.value);
      break;
    case tvglo:
      fprintf(sta, "%5u\ttvglo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].var_global.offset);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      else
        fprintf(sta, "\tvalor=%u\n", mem[o]);
      break;
    case ttglo:
      fprintf(sta, "%5u\tttglo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].table_global.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].table_global.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].table_global.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].table_global.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].table_global.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tpbgl:
    case tpwgl:
    case tpcgl:
    case tpigl:
      fprintf(sta, "%5u\ttp?gl: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].table_global.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].table_global.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].table_global.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].table_global.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].table_global.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tsglo:
      fprintf(sta, "%5u\ttsglo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].struct_global.offset);
      fprintf(sta, "\titems1=%u\n", obj[n].struct_global.dim1);
      fprintf(sta, "\titems2=%u\n", obj[n].struct_global.dim2);
      fprintf(sta, "\titems3=%u\n", obj[n].struct_global.dim3);
      fprintf(sta, "\ttotalitems=%u\n", l = obj[n].struct_global.total_items);
      fprintf(sta, "\tlen_item=%u\n", obj[n].struct_global.item_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tpsgl:
      fprintf(sta, "%5u\ttpsgl: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].struct_global.offset);
      fprintf(sta, "\titems1=%u\n", obj[n].struct_global.dim1);
      fprintf(sta, "\titems2=%u\n", obj[n].struct_global.dim2);
      fprintf(sta, "\titems3=%u\n", obj[n].struct_global.dim3);
      fprintf(sta, "\ttotalitems=%u\n", l = obj[n].struct_global.total_items);
      fprintf(sta, "\tlen_item=%u\n", obj[n].struct_global.item_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tvloc:
      fprintf(sta, "%5u\ttvloc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].var_local.offset);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      else
        fprintf(sta, "\tvalor=%u\n", loc[o]);
      break;
    case ttloc:
      fprintf(sta, "%5u\tttloc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].table_local.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].table_local.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].table_local.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].table_local.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].table_local.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tpblo:
    case tpwlo:
    case tpclo:
    case tpilo:
      fprintf(sta, "%5u\ttp?lo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].table_local.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].table_local.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].table_local.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].table_local.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].table_local.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tsloc:
      fprintf(sta, "%5u\ttsloc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].struct_local.offset);
      fprintf(sta, "\titems1=%u\n", obj[n].struct_local.dim1);
      fprintf(sta, "\titems2=%u\n", obj[n].struct_local.dim2);
      fprintf(sta, "\titems3=%u\n", obj[n].struct_local.dim3);
      fprintf(sta, "\ttotalitems=%u\n", l = obj[n].struct_local.total_items);
      fprintf(sta, "\tlen_item=%u\n", obj[n].struct_local.item_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tpslo:
      fprintf(sta, "%5u\ttpslo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].struct_local.offset);
      fprintf(sta, "\titems1=%u\n", obj[n].struct_local.dim1);
      fprintf(sta, "\titems2=%u\n", obj[n].struct_local.dim2);
      fprintf(sta, "\titems3=%u\n", obj[n].struct_local.dim3);
      fprintf(sta, "\ttotalitems=%u\n", l = obj[n].struct_local.total_items);
      fprintf(sta, "\tlen_item=%u\n", obj[n].struct_local.item_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tproc:
      fprintf(sta, "%5u\ttproc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\file_type_info=%u\n", (memptrsize)(obj[n].proc.scope));
      fprintf(sta, "\toffset=%u\n", obj[n].proc.offset);
      fprintf(sta, "\tnum_par=%u\n", obj[n].proc.num_params);
      break;
    case tfunc:
      fprintf(sta, "%5u\ttfunc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\tcode=%u\n", obj[n].func.code);
      fprintf(sta, "\tnum_par=%u\n", obj[n].func.num_params);
      break;
    case tfext:
      fprintf(sta, "%5u\ttfext: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\tcode=%u\n", obj[n].func_extern.code);
      fprintf(sta, "\tnum_par=%u\n", obj[n].func_extern.num_params);
      break;
    case tbglo:
      fprintf(sta, "%5u\ttbglo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].byte_global.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].byte_global.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].byte_global.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].byte_global.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].byte_global.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case tbloc:
      fprintf(sta, "%5u\ttbloc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].byte_global.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].byte_global.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].byte_global.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].byte_global.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].byte_global.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case twglo:
      fprintf(sta, "%5u\ttwglo: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].byte_global.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].byte_global.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].byte_global.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].byte_global.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].byte_global.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    case twloc:
      fprintf(sta, "%5u\ttwloc: %s", n, obj[n].name);
      if (obj[n].used)
        fprintf(sta, " (*) used\n");
      else
        fprintf(sta, "\n");
      fprintf(sta, "\toffset=%u\n", o = obj[n].byte_global.offset);
      fprintf(sta, "\tlen1=%u\n", obj[n].byte_global.len1);
      fprintf(sta, "\tlen2=%u\n", obj[n].byte_global.len2);
      fprintf(sta, "\tlen3=%u\n", obj[n].byte_global.len3);
      fprintf(sta, "\ttotalen=%u\n", obj[n].byte_global.total_len);
      if (obj[n].member)
        fprintf(sta, "\tmember of %s\n", (*obj[n].member).name);
      break;
    }
    fprintf(sta, "\tbloque(%u), prev(%u)\n", obj[n].scope, obj[n].prev);
    fprintf(sta, "\n");
  }

  fclose(sta);
}

#endif

//-----------------------------------------------------------------------------
//      Write the debugging info file
//-----------------------------------------------------------------------------

// NOTE: dim1,dim2,dim3 (or len1,len2,len3 for tables) are not saved yet

void save_dbg(void) {
  FILE *sta;
  int n;

  struct {
    int type;
    int name;
    int scope;
    int miembro;
    int v0, v1, v2, v3, v4, v5;
  } ob;

  sta = fopen("system/exec.dbg", "wb");

  fwrite(&num_objects, 4, 1, sta);
  fwrite(&num_predefined, 4, 1, sta);
  n = (memptrsize)&obj[0];
  fwrite(&n, 4, 1, sta);
  n = sizeof(struct object);
  fwrite(&n, 4, 1, sta);

  for (n = 0; n < num_objects; n++) {
    ob.type = (memptrsize)obj[n].type;
    ob.name = (memptrsize)obj[n].name - (memptrsize)vnom;
    ob.scope = (memptrsize)obj[n].scope;
    ob.miembro = (memptrsize)obj[n].member;
    ob.v0 = (memptrsize)obj[n].struct_global.offset;
    ob.v1 = (memptrsize)obj[n].struct_global.item_len;
    ob.v2 = (memptrsize)obj[n].struct_global.total_items;
    ob.v3 = (memptrsize)obj[n].struct_global.dim1;
    ob.v4 = (memptrsize)obj[n].struct_global.dim2;
    ob.v5 = (memptrsize)obj[n].struct_global.dim3;
    if (obj[n].type == tpsgl || obj[n].type == tpslo)
      ob.v1 = (ob.v1 - (memptrsize)&obj[0]) / sizeof(struct object);
    // WARNING: Do not add objects after this point -- &obj[0] and sizeof(struct object) are used above as base address and stride.
    fwrite(&ob, sizeof(ob), 1, sta);
  }

  n = (memptrsize)name_index.b - (memptrsize)vnom;
  fwrite(&n, 4, 1, sta);

  fwrite(vnom, 1, n, sta);

  fclose(sta);
}

//-----------------------------------------------------------------------------
//    Write the execution parameters for the interpreter
//-----------------------------------------------------------------------------

void save_exec_bin(void) {
  FILE *sta;

  sta = fopen("system/exec.bin", "wb");

  fwrite(&setup_file.vol_fx, 1, 1, sta);
  fwrite(&setup_file.vol_cd, 1, 1, sta);
  fwrite(&setup_file.vol_ma, 1, 1, sta);

  fclose(sta);
}

//-----------------------------------------------------------------------------
//      Write the assembler listing file
//-----------------------------------------------------------------------------

#ifdef listados

void list_assembler(void) {
  FILE *sta;
  int i = mem[1];

  sta = fopen("div.eml", "wt");

  fprintf(sta, "\xfe C\xa2digo EML de %s - DIV Versi\xa2n " version "\n", file_in);

  while (i < imem) {
    switch ((byte)mem[i]) {
    case lnop:
      fprintf(sta, "%5u\tnop", i);
      break;
    case lcar:
      fprintf(sta, "%5u\tcar %u", i, mem[i + 1]);
      i++;
      break;
    case lasi:
      fprintf(sta, "%5u\tasi", i);
      break;
    case lori:
      fprintf(sta, "%5u\tori", i);
      break;
    case lxor:
      fprintf(sta, "%5u\txor", i);
      break;
    case land:
      fprintf(sta, "%5u\tand", i);
      break;
    case ligu:
      fprintf(sta, "%5u\tigu", i);
      break;
    case ldis:
      fprintf(sta, "%5u\tdis", i);
      break;
    case lmay:
      fprintf(sta, "%5u\tmay", i);
      break;
    case lmen:
      fprintf(sta, "%5u\tmen", i);
      break;
    case lmei:
      fprintf(sta, "%5u\tmei", i);
      break;
    case lmai:
      fprintf(sta, "%5u\tmai", i);
      break;
    case ladd:
      fprintf(sta, "%5u\tadd", i);
      break;
    case lsub:
      fprintf(sta, "%5u\tsub", i);
      break;
    case lmul:
      fprintf(sta, "%5u\tmul", i);
      break;
    case ldiv:
      fprintf(sta, "%5u\tdiv", i);
      break;
    case lmod:
      fprintf(sta, "%5u\tmod", i);
      break;
    case lneg:
      fprintf(sta, "%5u\tneg", i);
      break;
    case lptr:
      fprintf(sta, "%5u\tptr", i);
      break;
    case lnot:
      fprintf(sta, "%5u\tnot", i);
      break;
    case laid:
      fprintf(sta, "%5u\taid", i);
      break;
    case lcid:
      fprintf(sta, "%5u\tcid", i);
      break;
    case lrng:
      fprintf(sta, "%5u\trng %u", i, mem[i + 1]);
      i++;
      break;
    case ljmp:
      fprintf(sta, "%5u\tjmp %u", i, mem[i + 1]);
      i++;
      break;
    case ljpf:
      fprintf(sta, "%5u\tjpf %u", i, mem[i + 1]);
      i++;
      break;
    case lfun:
      fprintf(sta, "%5u\tfun %u", i, mem[i + 1]);
      i++;
      break;
    case lcal:
      fprintf(sta, "%5u\tcal %u", i, mem[i + 1]);
      i++;
      break;
    case lret:
      fprintf(sta, "%5u\tret", i);
      break;
    case lasp:
      fprintf(sta, "%5u\tasp", i);
      break;
    case lfrm:
      fprintf(sta, "%5u\tfrm", i);
      break;
    case lcbp:
      fprintf(sta, "%5u\tcbp %u", i, mem[i + 1]);
      i++;
      break;
    case lcpa:
      fprintf(sta, "%5u\tcpa", i);
      break;
    case ltyp:
      fprintf(sta, "\n%5u\ttyp %u", i, mem[i + 1]);
      i++;
      break;
    case lpri:
      fprintf(sta, "%5u\tpri %u", i, mem[i + 1]);
      i++;
      break;
    case lcse:
      fprintf(sta, "%5u\tcse %u", i, mem[i + 1]);
      i++;
      break;
    case lcsr:
      fprintf(sta, "%5u\tcsr %u", i, mem[i + 1]);
      i++;
      break;
    case lshr:
      fprintf(sta, "%5u\tshr", i);
      break;
    case lshl:
      fprintf(sta, "%5u\tshl", i);
      break;
    case lipt:
      fprintf(sta, "%5u\tipt", i);
      break;
    case lpti:
      fprintf(sta, "%5u\tpti", i);
      break;
    case ldpt:
      fprintf(sta, "%5u\tdpt", i);
      break;
    case lptd:
      fprintf(sta, "%5u\tptd", i);
      break;
    case lada:
      fprintf(sta, "%5u\tada", i);
      break;
    case lsua:
      fprintf(sta, "%5u\tsua", i);
      break;
    case lmua:
      fprintf(sta, "%5u\tmua", i);
      break;
    case ldia:
      fprintf(sta, "%5u\tdia", i);
      break;
    case lmoa:
      fprintf(sta, "%5u\tmoa", i);
      break;
    case lana:
      fprintf(sta, "%5u\tana", i);
      break;
    case lora:
      fprintf(sta, "%5u\tora", i);
      break;
    case lxoa:
      fprintf(sta, "%5u\txoa", i);
      break;
    case lsra:
      fprintf(sta, "%5u\tsra", i);
      break;
    case lsla:
      fprintf(sta, "%5u\tsla", i);
      break;
    case lpar:
      fprintf(sta, "%5u\tpar %u", i, mem[i + 1]);
      i++;
      break;
    case lrtf:
      fprintf(sta, "%5u\trtf", i);
      break;
    case lclo:
      fprintf(sta, "%5u\tclo %u", i, mem[i + 1]);
      i++;
      break;
    case lfrf:
      fprintf(sta, "%5u\tfrf", i);
      break;
    case limp:
      fprintf(sta, "%5u\timp %u", i, mem[i + 1]);
      i++;
      break;
    case lext:
      fprintf(sta, "%5u\text %u", i, mem[i + 1]);
      i++;
      break;
    case lchk:
      fprintf(sta, "%5u\tchk", i);
      break;
    case ldbg:
      fprintf(sta, "%5u\tdbg", i);
      break;

    case lcar2:
      fprintf(sta, "%5u\tcar2 %u %u", i, mem[i + 1], mem[i + 2]);
      i += 2;
      break;
    case lcar3:
      fprintf(sta, "%5u\tcar3 %u %u %u", i, mem[i + 1], mem[i + 2], mem[i + 3]);
      i += 3;
      break;
    case lcar4:
      fprintf(sta, "%5u\tcar4 %u %u %u %u", i, mem[i + 1], mem[i + 2], mem[i + 3], mem[i + 4]);
      i += 4;
      break;
    case lasiasp:
      fprintf(sta, "%5u\tasiasp", i);
      break;
    case lcaraid:
      fprintf(sta, "%5u\tcaraid %u", i, mem[i + 1]);
      i++;
      break;
    case lcarptr:
      fprintf(sta, "%5u\tcarptr %u", i, mem[i + 1]);
      i++;
      break;
    case laidptr:
      fprintf(sta, "%5u\taidptr", i);
      break;
    case lcaraidptr:
      fprintf(sta, "%5u\tcaraidptr %u", i, mem[i + 1]);
      i++;
      break;
    case lcaraidcpa:
      fprintf(sta, "%5u\tcaraidcpa %u", i, mem[i + 1]);
      i++;
      break;
    case laddptr:
      fprintf(sta, "%5u\taddptr", i);
      break;
    case lfunasp:
      fprintf(sta, "%5u\tfunasp %u", i, mem[i + 1]);
      i++;
      break;
    case lcaradd:
      fprintf(sta, "%5u\tcaradd %u", i, mem[i + 1]);
      i++;
      break;
    case lcaraddptr:
      fprintf(sta, "%5u\tcaraddptr %u", i, mem[i + 1]);
      i++;
      break;
    case lcarmul:
      fprintf(sta, "%5u\tcarmul %u", i, mem[i + 1]);
      i++;
      break;
    case lcarmuladd:
      fprintf(sta, "%5u\tcarmuladd %u", i, mem[i + 1]);
      i++;
      break;
    case lcarasiasp:
      fprintf(sta, "%5u\tcarasiasp %u", i, mem[i + 1]);
      i++;
      break;
    case lcarsub:
      fprintf(sta, "%5u\tcarsub %u", i, mem[i + 1]);
      i++;
      break;
    case lcardiv:
      fprintf(sta, "%5u\tcardiv %u", i, mem[i + 1]);
      i++;
      break;

    case lptrwor:
      fprintf(sta, "%5u\tptrwor", i);
      break;
    case lasiwor:
      fprintf(sta, "%5u\tasiwor", i);
      break;
    case liptwor:
      fprintf(sta, "%5u\tiptwor", i);
      break;
    case lptiwor:
      fprintf(sta, "%5u\tptiwor", i);
      break;
    case ldptwor:
      fprintf(sta, "%5u\tdptwor", i);
      break;
    case lptdwor:
      fprintf(sta, "%5u\tptdwor", i);
      break;
    case ladawor:
      fprintf(sta, "%5u\tadawor", i);
      break;
    case lsuawor:
      fprintf(sta, "%5u\tsuawor", i);
      break;
    case lmuawor:
      fprintf(sta, "%5u\tmuawor", i);
      break;
    case ldiawor:
      fprintf(sta, "%5u\tdiawor", i);
      break;
    case lmoawor:
      fprintf(sta, "%5u\tmoawor", i);
      break;
    case lanawor:
      fprintf(sta, "%5u\tanawor", i);
      break;
    case lorawor:
      fprintf(sta, "%5u\torawor", i);
      break;
    case lxoawor:
      fprintf(sta, "%5u\txoawor", i);
      break;
    case lsrawor:
      fprintf(sta, "%5u\tsrawor", i);
      break;
    case lslawor:
      fprintf(sta, "%5u\tslawor", i);
      break;
    case lcpawor:
      fprintf(sta, "%5u\tcpawor", i);
      break;

    case lptrchr:
      fprintf(sta, "%5u\tptrchr", i);
      break;
    case lasichr:
      fprintf(sta, "%5u\tasichr", i);
      break;
    case liptchr:
      fprintf(sta, "%5u\tiptchr", i);
      break;
    case lptichr:
      fprintf(sta, "%5u\tptichr", i);
      break;
    case ldptchr:
      fprintf(sta, "%5u\tdptchr", i);
      break;
    case lptdchr:
      fprintf(sta, "%5u\tptdchr", i);
      break;
    case ladachr:
      fprintf(sta, "%5u\tadachr", i);
      break;
    case lsuachr:
      fprintf(sta, "%5u\tsuachr", i);
      break;
    case lmuachr:
      fprintf(sta, "%5u\tmuachr", i);
      break;
    case ldiachr:
      fprintf(sta, "%5u\tdiachr", i);
      break;
    case lmoachr:
      fprintf(sta, "%5u\tmoachr", i);
      break;
    case lanachr:
      fprintf(sta, "%5u\tanachr", i);
      break;
    case lorachr:
      fprintf(sta, "%5u\torachr", i);
      break;
    case lxoachr:
      fprintf(sta, "%5u\txoachr", i);
      break;
    case lsrachr:
      fprintf(sta, "%5u\tsrachr", i);
      break;
    case lslachr:
      fprintf(sta, "%5u\tslachr", i);
      break;
    case lcpachr:
      fprintf(sta, "%5u\tcpachr", i);
      break;

    case lstrcpy:
      fprintf(sta, "%5u\tstrcpy", i);
      break;
    case lstrfix:
      fprintf(sta, "%5u\tstrfix", i);
      break;
    case lstrcat:
      fprintf(sta, "%5u\tstrcat", i);
      break;
    case lstradd:
      fprintf(sta, "%5u\tstradd", i);
      break;
    case lstrdec:
      fprintf(sta, "%5u\tstrdec", i);
      break;
    case lstrsub:
      fprintf(sta, "%5u\tstrsub", i);
      break;
    case lstrlen:
      fprintf(sta, "%5u\tstrlen", i);
      break;
    case lstrigu:
      fprintf(sta, "%5u\tstrigu", i);
      break;
    case lstrdis:
      fprintf(sta, "%5u\tstrdis", i);
      break;
    case lstrmay:
      fprintf(sta, "%5u\tstrmay", i);
      break;
    case lstrmen:
      fprintf(sta, "%5u\tstrmen", i);
      break;
    case lstrmei:
      fprintf(sta, "%5u\tstrmei", i);
      break;
    case lstrmai:
      fprintf(sta, "%5u\tstrmai", i);
      break;
    case lcpastr:
      fprintf(sta, "%5u\tcpastr", i);
      break;

    default:
      fprintf(sta, "***");
      break;
    }
    fprintf(sta, "\n");
    i++;
  }
  fclose(sta);
}

#endif

//----------------------------------------------------------------------------
//  Function to locate and open a file (pal,fpg,fnt,...)
//  This function must follow the same algorithm as in F.CPP and DIVC.CPP
//----------------------------------------------------------------------------
FILE *__fpopen(byte *file, char *mode) {
  char fprgpath[_MAX_PATH * 2];
  FILE *f;

  div_strcpy(fprgpath, sizeof(fprgpath), (char *)&file_types[8]);
  div_strcat(fprgpath, sizeof(fprgpath), "/");
  div_strcat(fprgpath, sizeof(fprgpath), full);

  if ((f = fopen(fprgpath, mode))) { // prgpath/file
    div_strcpy(full, sizeof(full), fprgpath);
    printf("Found %s in prg dir [%s]\n", file, prgpath);
    return f;
  }

  return NULL;
}


FILE *fpopen(byte *file) {
  return __fpopen(file, "rb");
}

FILE *open_multi(char *file, char *mode) {
  FILE *f;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  char remote[255];

  char *ff = (char *)file;

  while (*ff != 0) {
    if (*ff == '\\')
      *ff = '/';
    ff++;
  }

  div_strcpy(full, sizeof(full), (char *)file); // full filename
  if ((f = fpopen((byte *)full)))
    return f;

  if ((f = fopen(full, mode))) // "paz\fixero.est"
    return f;


  if (_fullpath(full, (char *)file, _MAX_PATH) == NULL)
    return (NULL);

  _splitpath(full, drive, dir, fname, ext);

  if (strchr(ext, '.') == NULL) {
    div_strcpy(full, sizeof(full), ext);
  } else {
    div_strcpy(full, sizeof(full), strchr(ext, '.') + 1);
  }

  if (strlen(full) && file[0] != '/')
    div_strcat(full, sizeof(full), "/");

  div_strcat(full, sizeof(full), (char *)file);

  if ((f = fopen(full, mode))) // "est\paz\fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  strupr(full);

  if ((f = fopen(full, mode))) // "est\paz\fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  div_strcpy(full, sizeof(full), fname);
  div_strcat(full, sizeof(full), ext);

  if ((f = fopen(full, mode))) // "fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  strupr(full);

  if ((f = fopen(full, mode))) // "fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  strlwr(full);

  if ((f = fopen(full, mode))) // "fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  if (strchr(ext, '.') == NULL)
    div_strcpy(full, sizeof(full), ext);
  else
    div_strcpy(full, sizeof(full), strchr(ext, '.') + 1);

  if (strlen(full))
    div_strcat(full, sizeof(full), "/");

  div_strcat(full, sizeof(full), fname);
  div_strcat(full, sizeof(full), ext);

  if ((f = fopen(full, mode))) // "est\fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  strlwr(full);

  if ((f = fopen(full, mode))) // "est\fixero.est"
    return f;

  if ((f = fpopen((byte *)full)))
    return f;

  return NULL;
}

FILE *div_open_file(char *file) {
  FILE *f, *fe;
  char *ff = (char *)file;

#ifdef DEBUG
  printf("opening file: [%s]\n", file);
#endif

  if (strlen((const char *)file) < 1)
    return NULL;

  if (strlen((char *)file) == 0)
    return NULL;

  f = open_multi(file, "r");

  if (!f)
    div_strcpy(full, sizeof(full), "");
  return (f);
}

//-----------------------------------------------------------------------------
//      Compile dialog
//-----------------------------------------------------------------------------

uint8_t e_msg[32];
int compilado = 0;

void compile_pass1(void) {
  _show_items();
  wwrite(v.ptr, v.w / big2, v.h / big2, 3, 12, 0, texts[206], c3);
  wwrite(v.ptr, v.w / big2, v.h / big2, 6 + text_len(texts[206]), 12, 0, window[v_window + 1].title,
         c4);
}

extern uint8_t cerror[128];
void get_error(int n);

/* Compile dialog click handler (UI callback).
 * Runs the actual compilation via comp() on first click, then displays
 * the result (error message or success). Subsequent clicks handle the
 * OK/Help buttons in the dialog.
 */
void compile_pass2(void) {
  if (compilado == 0) {
    compilado = 1;
    mouse_graf = 3;
    error_number = -1;
    comp();
    if (error_number >= 0) {
      get_error(500 + error_number);
      show_compile_message(cerror);
    } else {
      show_compile_message(texts[202]);
      if (run_mode)
        end_dialog = 1;
    }
    mouse_graf = 1;
    v.redraw = 1;
  }
  _process_items();
  if (v.active_item == 0)
    end_dialog = 1;
  else if (v.active_item == 1) {
    v_help = 1;
    end_dialog = 1;
  }
}

void compile_pass0(void) {
  v.type = 1;
  v.w = 300;
  v.h = 46;
  switch (run_mode) {
  case 0:
    v.title = texts[200];
    break;
  case 1:
    v.title = texts[235];
    break;
  case 2:
    v.title = texts[236];
    break;
  case 3:
    v.title = texts[241];
    break;
  }
  v.paint_handler = compile_pass1;
  v.click_handler = compile_pass2;
  _button(100, 7, v.h - 14, 0);
  _button(125, v.w - 8, v.h - 14, 2);
  compilado = 0;
  v_help = 0;
}

void compile_program(void) {
  show_dialog(compile_pass0);
  poll_keyboard();
  scan_code = 0;
  ascii = 0;
  mouse_b = 0;
}


void plexico(void) {
  byte **ptr, *_ivnom, h, *_source = source;
  struct lex_ele *e;

  if (!comment_depth) {
    old_line = source_line;
    old_ierror = ierror;
    old_ierror_end = ierror_end;
  }

lex_scan:
  ierror = _source;
  switch ((uintptr_t)lex_case[*_source]) { // Pointer to a lex_ele or l_???

  case l_err:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
    } else
      c_error(0, 10);
    break;

  case l_cr:
    source_line++;
    if ((*++_source) == lf) {
      _source++;
      last_line = _source;
      goto lex_scan;
    }
    current_token = p_end_of_file;
    break; // eof

  case l_id:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
      break;
    }
    _ivnom = name_index.b;
    *name_index.p++ = 0;
    *name_index.p++ = 0;
    h = 0;
    while ((*name_index.b = lower[*_source++]))
      h = ((byte)(h << 1) + (h >> 7)) ^ (*name_index.b++);
    name_index.b++;
    _source--;
    if (name_index.b - vnom > MAX_OBJECTS * ID_AVG_LENGTH)
      c_error(0, 100);
    ptr = &vhash[h];
    while (*ptr && strcmp((char *)(ptr + 2), (char *)(_ivnom + ptr8)))
      ptr = (byte **)*ptr;
    if (!strcmp((char *)(ptr + 2), (char *)_ivnom + ptr8)) { // id found
      name_index.b = _ivnom;                                 // remove it from vnom
      current_token = (intptr_t)*(ptr + 1);
      if (current_token < 256 && current_token >= 0) { // reserved word (token)
        if (current_token == p_rem) {
          while (*_source != cr)
            _source++;
          goto lex_scan;
        }
      } else { // object (existing id)
        current_token = p_id;
      }
    } else {
      name_index.b = _ivnom; // remove it from vnom
      current_token = p_id;  // new id
    }
    break;

  case l_spc:
    while ((*++_source) == ' ') {}
    goto lex_scan;

  case l_lit:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
      break;
    }
    current_token = p_lit;
    h = *_source;
    _ivnom = name_index.b; // Literal between two h delimiters
    do {
      if (*++_source == cr)
        c_error(0, 11);
      if (*_source == h)
        if (*(_source + 1) == h)
          *_ivnom = *++_source;
        else
          *_ivnom = 0;
      else
        *_ivnom = *_source;
    } while (*_ivnom++);
    _source++;
    strings_length += (strlen((char *)name_index.b) + ptr4) / 4;
    name_index.b = _ivnom; // remove it from vnom
    break;

  case l_num:
    if (comment_depth) {
      current_token = p_rem;
      _source++;
      break;
    }
    current_token = p_num;
    token_value = 0;
    if (*_source == '0' && lower[*(_source + 1)] == 'x') {
      _source += 2;
      while ((uintptr_t)lex_case[*_source] == l_num ||
             (lower[*_source] >= 'a' && lower[*_source] <= 'f')) {
        if ((uintptr_t)lex_case[*_source] == l_num)
          token_value = token_value * 16 + *_source++ - 0x30;
        else
          token_value = token_value * 16 + lower[*_source++] - 'a' + 10;
      }
    } else
      do {
        token_value = token_value * 10 + *_source++ - 0x30;
      } while ((uintptr_t)lex_case[*_source] == l_num);
    break;

  default: // pointer to a lex_ele
    e = lex_case[*_source++];
    _ivnom = _source;
    current_token = (*e).token;
    while ((e = (*e).next)) {
      while (*_source != (*e).character && (*e).alternative)
        e = (*e).alternative;
      if (*_source++ == (*e).character && (*e).token) {
        current_token = (*e).token;
        _ivnom = _source;
      }
    }
    _source = _ivnom;

    if (current_token == p_rem && !comment_depth) {
      while (*_source != cr)
        _source++;
      goto lex_scan;
    }

    if (current_token == p_ini_rem) {
      comment_depth++;
      do {
        source = _source;
        plexico();
        _source = source;
      } while (current_token != p_end_rem);
      comment_depth--;
      goto lex_scan;
    }

    if (current_token == p_end_of_file) {
      if (comment_depth)
        c_error(0, 55);
      else
        c_error(0, 12);
    }
    break;
  }
  source = _source;
  ierror_end = _source - 1;
}

void psintactico(void) {
  byte *_ivnom = name_index.b;

  strings_length = 0;

  do {
    plexico();
  } while (current_token != p_end_of_file);

  name_index.b = _ivnom;
  source = source_ptr;
  _source = source;
  comment_depth = 0;
}
