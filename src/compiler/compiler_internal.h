//-----------------------------------------------------------------------------
//      DIV Compiler - Shared Internal Definitions
//
//      This header is included by compiler.c, compiler_parser.c, and
//      compiler_expression.c.  It holds every constant, type, and extern
//      declaration that the three translation units share.
//-----------------------------------------------------------------------------

#ifndef COMPILER_INTERNAL_H
#define COMPILER_INTERNAL_H

#include "global.h"
#include "div_string.h"

#ifdef ZLIB
#include <zlib.h>
#endif

// The compiler needs signed memptrsize for pointer offset arithmetic.
// osdep.h defines it as unsigned; override here for all compiler files.
#undef memptrsize
#if __WORDSIZE == 64
#define memptrsize int64_t
#else
#define memptrsize int32_t
#endif

//-----------------------------------------------------------------------------
//      Constants
//-----------------------------------------------------------------------------

#define version "0c88b" // Compiler version

#ifndef SHARE
#define MAX_OBJECTS   8192 // Compiler object maximum limit
#define ID_AVG_LENGTH 16   // Average identifier length (+4+4+1)
#else
#define MAX_OBJECTS   700 // The shareware version has 626 ...
#define ID_AVG_LENGTH 20  // Average identifier length (+4+4+1)
#endif

#define MAX_NODES       128  // Maximum number of lexer symbol nodes
#define MAX_EXP_SIZE    512  // Maximum number of elements in an expression
#define HEADER_LENGTH   9    // Header length at program start
#define EVAL_STACK_SIZE 2048 // Execution stack length

#define DEFAULT_BUFFER  (16384 * 8)
#define SAFETY_BUFFER   (4096 * 8)
#define BUFFER_INCREASE (16384 * 8)

#define cr  13 // Carriage return
#define lf  10 // Line feed
#define tab 9  // Tab

//-----------------------------------------------------------------------------

#define l_err 0 // lex_case values, when not pointers to lex_simb
#define l_cr  1 // End of line (l_err = unexpected character)
#define l_id  2 // Identifier or reserved word
#define l_spc 3 // Spaces and tabs
#define l_lit 4 // Literal
#define l_num 5 // Numeric constant

//-----------------------------------------------------------------------------
//      Syntactic values / tokens (current_token)
//-----------------------------------------------------------------------------

#define p_end_of_file 0x00 // End of file <EOF>

#define p_program 0x01
#define p_const   0x02
#define p_global  0x03
#define p_local   0x04
#define p_begin   0x05
#define p_end     0x06
#define p_process 0x07
#define p_private 0x08
#define p_struct  0x09
#define p_import  0x0A
#define p_include 0x75

#define p_setup_program 0x0B

#define p_string 0x0C
#define p_byte   0x0D
#define p_word   0x0E
#define p_int    0x0F

#define p_compiler_options 0x10
#define p_function         0x11

#define p_until 0x16
#define p_else  0x17

#define p_return 0x18

#define p_from    0x19
#define p_to      0x1a
#define p_step    0x1b
#define p_if      0x20
#define p_loop    0x21
#define p_while   0x22
#define p_repeat  0x23
#define p_for     0x24
#define p_switch  0x25
#define p_case    0x26
#define p_default 0x27

#define p_frame 0x28

#define p_break    0x29
#define p_continue 0x2A

#define p_clone 0x2B

#define p_debug 0x2C

#define p_ptocoma 0x30
#define p_coma    0x31
#define p_corab   0x32
#define p_corce   0x33

#define p_asig 0x38

#define p_dup 0x3C

#define p_abrir  0x40
#define p_cerrar 0x41

#define p_suma  0x44
#define p_resta 0x45

#define p_shr 0x46
#define p_shl 0x47

#define p_or  0x48
#define p_xor 0x49
#define p_and 0x4a

#define p_xorptr 0x4b
#define p_andofs 0x4c

#define p_neg     0x4e
#define p_pointer 0x4f

#define p_offset 0x50
#define p_not    0x51

#define p_add 0x52
#define p_sub 0x53

#define p_mul   0x54
#define p_div   0x55
#define p_mod   0x56
#define p_multi 0x57

#define p_inc 0x58
#define p_dec 0x59

#define p_igu 0x5b
#define p_dis 0x5c

#define p_may 0x5d
#define p_men 0x5e
#define p_mei 0x5f
#define p_mai 0x60

#define p_sizeof 0x66

#define p_type 0x67

#define p_whoami 0x68

#define p_punto 0x69

#define p_rango 0x6a

#define p_add_asig 0x6b
#define p_sub_asig 0x6c
#define p_mul_asig 0x6d
#define p_div_asig 0x6e
#define p_mod_asig 0x6f
#define p_and_asig 0x70
#define p_or_asig  0x71
#define p_xor_asig 0x72
#define p_shr_asig 0x73
#define p_shl_asig 0x74

#define p_ini_rem 0x7d // Begin comment
#define p_end_rem 0x7e // End comment
#define p_rem     0x7f // Single-line comment

#define p_strigu 0xc0
#define p_strdis 0xc1
#define p_strmay 0xc2
#define p_strmen 0xc3
#define p_strmei 0xc4
#define p_strmai 0xc5

#define p_pointerword  0xd0
#define p_sumaword     0xd1
#define p_restaword    0xd2
#define p_incword      0xd3
#define p_decword      0xd4
#define p_asigword     0xd5
#define p_add_asigword 0xd6
#define p_sub_asigword 0xd7
#define p_mul_asigword 0xd8
#define p_div_asigword 0xd9
#define p_mod_asigword 0xda
#define p_and_asigword 0xdb
#define p_or_asigword  0xdc
#define p_xor_asigword 0xdd
#define p_shr_asigword 0xde
#define p_shl_asigword 0xdf

#define p_pointerchar  0xe0
#define p_sumachar     0xe1
#define p_restachar    0xe2
#define p_incchar      0xe3
#define p_decchar      0xe4
#define p_asigchar     0xe5
#define p_add_asigchar 0xe6
#define p_sub_asigchar 0xe7
#define p_mul_asigchar 0xe8
#define p_div_asigchar 0xe9
#define p_mod_asigchar 0xea
#define p_and_asigchar 0xeb
#define p_or_asigchar  0xec
#define p_xor_asigchar 0xed
#define p_shr_asigchar 0xee
#define p_shl_asigchar 0xef

#define p_strcpy 0xf0
#define p_strfix 0xf1
#define p_strcat 0xf2
#define p_stradd 0xf3
#define p_strdec 0xf4
#define p_strsub 0xf5
#define p_strlen 0xf6

#define p_pointerbyte 0xf7

#define p_lit 0xfc // Pointer to literal (txt) in token_value
#define p_id  0xfd // or a ptr to vnom (to an object ptr)
#define p_num 0xfe // Number in token_value

//-----------------------------------------------------------------------------
//      Object types in obj[]
//-----------------------------------------------------------------------------

#define tnone 0
#define tcons 1
#define tvglo 2
#define ttglo 3
#define tcglo 4
#define tvloc 5
#define ttloc 6
#define tcloc 7
#define tproc 8
#define tfunc 9
#define tsglo 10
#define tsloc 11
#define tfext 12 // External library function

#define tbglo 13 // Global byte
#define twglo 14
#define tbloc 15
#define twloc 16

#define tpigl 17 // Global int pointer (an addressable ttglo)
#define tpilo 18 // Local int pointer (same)

#define tpwgl 19 // Word pointers
#define tpwlo 20
#define tpbgl 21 // Byte pointers
#define tpblo 22

#define tpcgl 23 // String pointers
#define tpclo 24
#define tpsgl 25 // Struct pointers
#define tpslo 26

//-----------------------------------------------------------------------------
//      Expression element types in tabexp[]
//-----------------------------------------------------------------------------

#define econs   0 // Constant
#define eoper   1 // Operator
#define erango  2 // Range check
#define ewhoami 3 // Process identifier
#define ecall   4 // Process creation
#define efunc   5 // Internal function call
#define efext   6 // External function call
#define echeck  7 // Identifier validity check
#define estring 8 // Text string (its constant offset)
#define enull   9 // NULL pointer check

//-----------------------------------------------------------------------------
//      EML Opcodes (bytecode instruction set)
//-----------------------------------------------------------------------------

#define lnop 0  // No operation
#define lcar 1  // Push a constant onto the stack
#define lasi 2  // Pop value and offset, store value at [offset]
#define lori 3  // Logical or
#define lxor 4  // Xor, exclusive or
#define land 5  // Logical and, condition operator
#define ligu 6  // Equal, logical comparison
#define ldis 7  // Not equal
#define lmay 8  // Greater than
#define lmen 9  // Less than
#define lmei 10 // Less than or equal
#define lmai 11 // Greater than or equal
#define ladd 12 // Add
#define lsub 13 // Subtract
#define lmul 14 // Multiply
#define ldiv 15 // Integer division
#define lmod 16 // Modulo
#define lneg 17 // Negate
#define lptr 18 // Pointer: pop offset and push [offset]
#define lnot 19 // Bitwise not
#define laid 20 // Add id to the constant on stack
#define lcid 21 // Push id onto the stack
#define lrng 22 // Range check
#define ljmp 23 // Jump to a mem[] address
#define ljpf 24 // Jump if false
#define lfun 25 // Call an internal function
#define lcal 26 // Create a new process
#define lret 27 // Process self-termination
#define lasp 28 // Discard a stacked value
#define lfrm 29 // Suspend process for this frame
#define lcbp 30 // Initialize local parameter pointer
#define lcpa 31 // Pop offset, read parameter and bp++
#define ltyp 32 // Define current process type
#define lpri 33 // Jump to address, load private vars
#define lcse 34 // Switch: if != expression, jump
#define lcsr 35 // Switch: if not in range, jump
#define lshr 36 // Right shift
#define lshl 37 // Left shift
#define lipt 38 // Pre-increment and pointer
#define lpti 39 // Pointer and post-increment
#define ldpt 40 // Pre-decrement and pointer
#define lptd 41 // Pointer and post-decrement
#define lada 42 // Add-assign
#define lsua 43 // Sub-assign
#define lmua 44 // Mul-assign
#define ldia 45 // Div-assign
#define lmoa 46 // Mod-assign
#define lana 47 // And-assign
#define lora 48 // Or-assign
#define lxoa 49 // Xor-assign
#define lsra 50 // Shr-assign
#define lsla 51 // Shl-assign
#define lpar 52 // Define number of private parameters
#define lrtf 53 // Process self-termination, returns value
#define lclo 54 // Clone the current process
#define lfrf 55 // Pseudo-frame (partial frame)
#define limp 56 // Import an external DLL
#define lext 57 // Call an external function
#define lchk 58 // Check identifier validity
#define ldbg 59 // Invoke the debugger

// Optimization fusions (DIV 2.0)

#define lcar2      60
#define lcar3      61
#define lcar4      62
#define lasiasp    63
#define lcaraid    64
#define lcarptr    65
#define laidptr    66
#define lcaraidptr 67
#define lcaraidcpa 68
#define laddptr    69
#define lfunasp    70
#define lcaradd    71
#define lcaraddptr 72
#define lcarmul    73
#define lcarmuladd 74
#define lcarasiasp 75
#define lcarsub    76
#define lcardiv    77

// Character handling instructions

#define lptrchr 78
#define lasichr 79
#define liptchr 80
#define lptichr 81
#define ldptchr 82
#define lptdchr 83
#define ladachr 84
#define lsuachr 85
#define lmuachr 86
#define ldiachr 87
#define lmoachr 88
#define lanachr 89
#define lorachr 90
#define lxoachr 91
#define lsrachr 92
#define lslachr 93
#define lcpachr 94

// String handling instructions

#define lstrcpy 95
#define lstrfix 96
#define lstrcat 97
#define lstradd 98
#define lstrdec 99
#define lstrsub 100
#define lstrlen 101
#define lstrigu 102
#define lstrdis 103
#define lstrmay 104
#define lstrmen 105
#define lstrmei 106
#define lstrmai 107
#define lcpastr 108

// Word handling instructions

#define lptrwor 109
#define lasiwor 110
#define liptwor 111
#define lptiwor 112
#define ldptwor 113
#define lptdwor 114
#define ladawor 115
#define lsuawor 116
#define lmuawor 117
#define ldiawor 118
#define lmoawor 119
#define lanawor 120
#define lorawor 121
#define lxoawor 122
#define lsrawor 123
#define lslawor 124
#define lcpawor 125

// Miscellaneous

#define lnul 126 // Check that a pointer is not NULL

//-----------------------------------------------------------------------------
//      Type definitions
//-----------------------------------------------------------------------------

struct object {
  byte type;             // Object type
  byte used;             // Whether the object was used before being defined
  byte *name;            // Pointer to name, for listing output
  byte *ierror;          // Pointer to source code for determining column if needed
  int line;              // Source line number, for error reporting
  int param;             // Whether this object was declared in parameters
  struct object *prev;   // Previous object with the same name
  struct object *scope;  // Block for this process (0-global/local N-private)
  struct object *member; // Which struct it belongs to (0-n/a)
  union {
    struct { // Constant
      int value;
      int literal; // 0-no, 1-yes
    } cons;
    struct { // Global variable
      int offset;
    } var_global;
    struct { // Global table
      int offset;
      int total_len;        // total number of elements
      int len1, len2, len3; // len2/3==-1 if n/a
    } table_global, ptr_int_global;
    struct { // Global byte
      int offset;
      int total_len;
      int len1, len2, len3;
    } byte_global, ptr_byte_global;
    struct { // Global word
      int offset;
      int total_len;
      int len1, len2, len3;
    } word_global, ptr_word_global;
    struct { // Global string
      int offset;
      int total_len;
    } string_global, ptr_string_global;
    struct { // Struct
      int offset;
      int item_len;
      int total_items;
      int dim1, dim2, dim3;
    } struct_global, struct_local;
    struct { // Struct pointer
      int offset;
      struct object *ostruct;
      int total_items;
      int dim1, dim2, dim3;
    } ptr_struct_global, ptr_struct_local;
    struct { // Local variable
      int offset;
    } var_local;
    struct { // Local table
      int offset;
      int total_len;
      int len1, len2, len3;
    } table_local, ptr_int_local;
    struct { // Local byte
      int offset;
      int total_len;
      int len1, len2, len3;
    } byte_local, ptr_byte_local;
    struct { // Local word
      int offset;
      int total_len;
      int len1, len2, len3;
    } word_local, ptr_word_local;
    struct { // Local string
      int offset;
      int total_len;
    } string_local, ptr_string_local;
    struct { // Process
      struct object *scope;
      int offset;
      int num_params;
    } proc;
    struct { // Internal function
      int code;
      int num_params;
      int ret;      // 0-Int 1-String
      byte par[12]; // 0-Int 1-(Struct_name,name)
    } func;
    struct { // External function
      int code;
      int num_params;
    } func_extern;
  };
};

struct exp_ele {
  byte type; // econs, eoper, erango, ewhoami, ecall
  union {
    int value;
    int token;
    struct object *object;
  };
};

struct lex_ele {
  byte character;
  byte token;
  struct lex_ele *alternative;
  struct lex_ele *next;
};

struct peephole_entry { // Peephole optimization window
  int dir;              // Address
  int param;            // Whether this opcode has a parameter
  int op;               // Opcode
};

//-----------------------------------------------------------------------------
//      Break/continue stack sizes
//-----------------------------------------------------------------------------

#define max_break 512
#define max_cont  256

//-----------------------------------------------------------------------------
//      Shared variables (defined in compiler.c)
//-----------------------------------------------------------------------------

extern byte file_in[_MAX_PATH + 1], file_out[_MAX_PATH + 1];
extern FILE *fin, *fout;
extern byte *_source;
extern byte *source, *ierror, *ierror_end;

extern int strings_length;
extern int strings_start;
extern int max_process;
extern int ignore_errors;
extern int free_sintax;
extern int extended_conditions;
extern int simple_conditions;
extern int check_range;
extern int check_id;
extern int check_null;
extern int enable_strfix;
extern int optimize;

extern int old_line;
extern byte *old_ierror, *old_ierror_end;
extern byte *last_line, zero;

extern int start_addr, end_addr;
extern int start_line, start_col;
extern int end_line, end_col;

extern struct object obj[MAX_OBJECTS], *iobj;
extern byte *vnom;
union name_index_u {
  byte *b;
  byte **p;
};
extern union name_index_u name_index;
extern byte *objects_start;
extern byte *vhash[256];

extern struct exp_ele tabexp[MAX_EXP_SIZE], *_exp;
extern struct lex_ele lex_simb[MAX_NODES], *ilex_simb, *lex_case[256];

extern int current_token, token_value;
extern struct object *o;
extern struct object *current_scope;
extern struct object *lexical_scope;
extern struct object *member;

extern int source_line, num_nodes, num_objects;
extern int num_predefined;
extern int in_params;
extern int cross_process_access;

extern int tbreak[max_break];
extern int itbreak;
extern int tcont[max_cont];
extern int itcont;

extern int eval_stack[EVAL_STACK_SIZE + MAX_EXP_SIZE + 64];

extern int32_t *mem_ory, *frm;
extern int32_t *mem, *loc;
extern int imem, imem_max;
extern int iloc, iloc_max;
extern int itxt, ifrm_max;
extern byte *imemptr;
extern int local_var_len;

extern int comment_depth;
extern int emit_tokens;

extern int error_number;
extern int error_line;
extern int error_col;
extern int error_27;

extern int program_type;

extern struct peephole_entry code[16];

extern int in_fopen;

extern FILE *linf; // EXEC.LIN
extern FILE *lprg; // EXEC.PGM
extern FILE *lins; // EXEC.INS

//-----------------------------------------------------------------------------
//      Function prototypes — compiler.c (orchestration, lexer, init, output)
//-----------------------------------------------------------------------------

void comp(void);
void comp_exit(void);
void init_compiler(void);
void show_compile_message(byte *p);
void compile(void);
void free_resources(void);

void save_error(word tipo);
void c_error(word, word);
void statement_start(void);
void statement_end(void);
void record_statement(void);
void test_buffer(int **buffer, int *maximo, int n);

void analyze_ltlex(void);
void preload_objects(void);
void lexer(void);
void skip_semicolons(void);

void list_objects(void);
void save_dbg(void);
void save_exec_bin(void);

void parse_syntax(void);
void scan_token(void);

FILE *div_open_file(char *file);

//-----------------------------------------------------------------------------
//      Function prototypes — compiler_parser.c (declarations, statements)
//-----------------------------------------------------------------------------

struct object *analyze_pointer(int tipo, int offset);
int analyze_pointer_struct(int tipo, int offset, struct object *structure);
int analyze_struct(int offstruct);
int analyze_struct_local(int offstruct);
int analyze_struct_private(int offstruct);
void parser(void);
void analyze_private(void);
void tglo_init(memptrsize tipo);
void tloc_init(int tipo);
void tglo_init2(int tipo);
void statement(void);

//-----------------------------------------------------------------------------
//      Function prototypes — compiler_expression.c (expressions, codegen)
//-----------------------------------------------------------------------------

void condition(void);
void con00(int tipo_exp);
void con0(void);
void con1(void);
void con2(void);
void expression(void);
void expression_cpa(void);
void generate_expression(void);
int constant(void);
void exp00(int tipo_exp);
void exp0(void);
void exp1(void);
void div_exp2(void);
void exp3(void);
void exp4(void);
void exp5(void);
void exp6(void);
void unary(void);
void factor(void);
void factor_struct(void);

void g1(int op);
void g2(int op, int pa);
void gen(int param, int op, int pa);
void remove_code(int i);
void delete_code(void);
void add_code(int dir, int param, int op);

void list_assembler(void);

#endif // COMPILER_INTERNAL_H
