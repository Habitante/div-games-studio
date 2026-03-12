
// Strings, new: p_string, string_global, string_local, p_*char, l*chr, ...
// 00 - intermediate code optimization
// 00 - tables and structs up to 3 dimensions
// 01 - add global strings
// 02 - test in PRG, trigger all new errors
// 03 - add new instruction set to interpreter
// 04 - prepare debugger for string support
// 05 - add local and private strings
// 06 - runtime range checking (strings with 0xdad00000 prefix)
// 07 - long_header=9, instead of 36
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
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"

#ifdef ZLIB
#include <zlib.h>
#endif

void list_objects(void);
void list_assembler(void);

void delete_code(void);
void preload_objects(void);
void psintactico(void);
void test_buffer(int **buffer, int *maximo, int n);
void save_dbg(void);
void save_exec_bin(void);
void tloc_init(int tipo);
void g2(int op, int pa);
void analyze_private(void);
void g1(int op);
void expression_cpa(void);
void tglo_init2(int tipo);
void condition(void);
void con00(int tipo_exp);
void generate_expression(void);
void con0(void);
void con1(void);
void con2(void);
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
void gen(int param, int op, int pa);
void remove_code(int i);
void add_code(int dir, int param, int op);


//-----------------------------------------------------------------------------
//      Constants
//-----------------------------------------------------------------------------

#define version "0c88b" //Compiler version

#ifndef SHARE
#define max_obj     8192 //Compiler object maximum limit
#define long_med_id 16   //Average identifier length (+4+4+1)
#else
#define max_obj     700 //The shareware version has 626 ...
#define long_med_id 20  //Average identifier length (+4+4+1)
#endif

#define max_nodos   128  //Maximum number of lexer symbol nodes
#define max_exp     512  //Maximum number of elements in an expression
#define long_header 9    //Header length at program start
#define long_pila   2048 //Execution stack length

#define default_buffer    (16384 * 8)
#define security_distance (4096 * 8)
#define buffer_grow       (16384 * 8)

#define cr  13 //Carriage return
#define lf  10 //Line feed
#define tab 9  //Tab

//-----------------------------------------------------------------------------

#define l_err 0 //lex_case values, when not pointers to lex_simb
#define l_cr  1 //End of line (l_err = unexpected character)
#define l_id  2 //Identifier or reserved word
#define l_spc 3 //Spaces and tabs
#define l_lit 4 //Literal
#define l_num 5 //Numeric constant

//-----------------------------------------------------------------------------
//      Syntactic values / tokens (current_token)
//-----------------------------------------------------------------------------

#define p_ultima 0x00 //End of file <EOF>

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

#define p_ini_rem 0x7d //Begin comment
#define p_end_rem 0x7e //End comment
#define p_rem     0x7f //Single-line comment

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

#define p_lit 0xfc //Pointer to literal (txt) in token_value
#define p_id  0xfd //or a ptr to vnom (to an object ptr)
#define p_num 0xfe //Number in token_value

//-----------------------------------------------------------------------------

#define tnone 0 // Object types in obj[]
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

#define econs   0 // Constant            // Element types in tabexp[]
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

//-Mnemonic--Code--Operands (EML code generation, "*" = "not yet used")

#define lnop 0  // *            No operation
#define lcar 1  // value        Push a constant onto the stack
#define lasi 2  //              Pop value and offset, store value at [offset]
#define lori 3  //              Logical or
#define lxor 4  //              Xor, exclusive or
#define land 5  //              Logical and, condition operator
#define ligu 6  //              Equal, logical comparison
#define ldis 7  //              Not equal, true if 2 values differ
#define lmay 8  //              Greater than, signed comparison
#define lmen 9  //              Less than, same
#define lmei 10 //              Less than or equal
#define lmai 11 //              Greater than or equal
#define ladd 12 //              Add two constants
#define lsub 13 //              Subtract, binary operation
#define lmul 14 //              Multiply
#define ldiv 15 //              Integer division
#define lmod 16 //              Modulo, division remainder
#define lneg 17 //              Negate, change sign of a constant
#define lptr 18 //              Pointer, pop offset and push [offset]
#define lnot 19 //              Bitwise not, bit by bit
#define laid 20 //              Add id to the constant on stack
#define lcid 21 //              Push id onto the stack
#define lrng 22 // range        Perform a range check
#define ljmp 23 // offset       Jump to a mem[] address
#define ljpf 24 // offset       Jump if false to an address
#define lfun 25 // code         Call an internal function, e.g. signal()
#define lcal 26 // offset       Create a new process in the program
#define lret 27 //              Process self-termination
#define lasp 28 //              Discard a stacked value
#define lfrm 29 //              Suspend process execution for this frame
#define lcbp 30 // num_params      Initialize local parameter pointer
#define lcpa 31 //              Pop offset, read parameter [offset] and bp++
#define ltyp 32 // block        Define current process type (collisions)
#define lpri 33 // offset       Jump to address, and load private vars
#define lcse 34 // offset       If switch != expression, jump to offset
#define lcsr 35 // offset       If switch not in range, jump to offset
#define lshr 36 //              Right shift (C-style, >>)
#define lshl 37 //              Left shift (C-style, <<)
#define lipt 38 //              Pre-increment and pointer
#define lpti 39 //              Pointer and post-increment
#define ldpt 40 //              Pre-decrement and pointer
#define lptd 41 //              Pointer and post-decrement
#define lada 42 //              Add-assign
#define lsua 43 //              Sub-assign
#define lmua 44 //              Mul-assign
#define ldia 45 //              Div-assign
#define lmoa 46 //              Mod-assign
#define lana 47 //              And-assign
#define lora 48 //              Or-assign
#define lxoa 49 //              Xor-assign
#define lsra 50 //              Shr-assign
#define lsla 51 //              Shl-assign
#define lpar 52 // num_par_pri  Define number of private parameters
#define lrtf 53 //              Process self-termination, returns a value
#define lclo 54 // offset       Clone the current process
#define lfrf 55 //              Pseudo-frame (frame at percentage, frame(100)==frame)
#define limp 56 // offset text  Import an external DLL
#define lext 57 // code         Call an external function
#define lchk 58 //              Check identifier validity
#define ldbg 59 //              Invoke the debugger

// Instructions added for optimization (DIV 2.0)

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

// Instructions added for character handling

#define lptrchr 78 // Pointer, pop (index, offset) and push [offset+byte index]
#define lasichr 79 // Pop (value, index, offset) and store value at [offset+byte index]
#define liptchr 80 // Pre-increment and pointer
#define lptichr 81 // Pointer and post-increment
#define ldptchr 82 // Pre-decrement and pointer
#define lptdchr 83 // Pointer and post-decrement
#define ladachr 84 // Add-assign
#define lsuachr 85 // Sub-assign
#define lmuachr 86 // Mul-assign
#define ldiachr 87 // Div-assign
#define lmoachr 88 // Mod-assign
#define lanachr 89 // And-assign
#define lorachr 90 // Or-assign
#define lxoachr 91 // Xor-assign
#define lsrachr 92 // Shr-assign
#define lslachr 93 // Shl-assign
#define lcpachr 94 // Pop offset, read parameter [offset] and bp++

// Instructions added for string handling

#define lstrcpy 95  // Pop si, di, and do strcpy(mem[di],[si]) (leave di on stack)
#define lstrfix 96  // Expand a string before inserting a char into it
#define lstrcat 97  // Concatenate two strings (operates like strcpy)
#define lstradd 98  // Add two "in-flight" strings and leave pointer on stack
#define lstrdec 99  // Add or remove chars from a string
#define lstrsub 100 // Remove chars from a string (-=)
#define lstrlen 101 // Replace a string with its length
#define lstrigu 102 // String equality comparison
#define lstrdis 103 // Strings not equal
#define lstrmay 104 // String greater than
#define lstrmen 105 // String less than
#define lstrmei 106 // String greater than or equal
#define lstrmai 107 // String less than or equal
#define lcpastr 108 // Load a parameter into a string

// Instructions added for word handling

#define lptrwor 109 // Pointer, pop (index, offset) and push [offset+word index]
#define lasiwor 110 // Pop (value, index, offset) and store value at [offset+word index]
#define liptwor 111 // Pre-increment and pointer
#define lptiwor 112 // Pointer and post-increment
#define ldptwor 113 // Pre-decrement and pointer
#define lptdwor 114 // Pointer and post-decrement
#define ladawor 115 // Add-assign
#define lsuawor 116 // Sub-assign
#define lmuawor 117 // Mul-assign
#define ldiawor 118 // Div-assign
#define lmoawor 119 // Mod-assign
#define lanawor 120 // And-assign
#define lorawor 121 // Or-assign
#define lxoawor 122 // Xor-assign
#define lsrawor 123 // Shr-assign
#define lslawor 124 // Shl-assign
#define lcpawor 125 // Pop offset, read parameter [offset] and bp++

// Miscellaneous

#define lnul 126 // Check that a pointer is not NULL

//-----------------------------------------------------------------------------
//      Prototypes
//-----------------------------------------------------------------------------

void c_error(word, word);
void analyze_commands(word, byte **);
void prepare_compilation(void);
void analyze_ltlex(void);
void lexer(void);
void parser(void);
void tglo_init(memptrsize);
void statement(void);
void expression(void);
int constant(void);

static jmp_buf buf;

void comp(void) {
  if (!setjmp(buf))
    compile();
}

void comp_exit(void) {
  longjmp(buf, 1);
}


//-----------------------------------------------------------------------------
//      Compiler global variables
//-----------------------------------------------------------------------------

#include "div_stub.h"

byte file_in[_MAX_PATH + 1], file_out[_MAX_PATH + 1];

FILE *fin, *fout;
byte *_source = NULL;
byte *source, *ierror, *ierror_end;

int longitud_textos;
int inicio_textos;
int max_process;
int ignore_errors;
int free_sintax;
int extended_conditions;
int simple_conditions;
int check_range;
int comprueba_id;
int comprueba_null;
int enable_strfix;
int optimize;

//-----------------------------------------------------------------------------

FILE *div_open_file(char *file);

//-----------------------------------------------------------------------------

int old_line; // File position of the token before the last one read
byte *old_ierror, *old_ierror_end;

// The last token read is at (source_line,ierror,ierror_end)

byte *ultima_linea, cero = 0;

//-----------------------------------------------------------------------------

// Info for EXEC.LIN (position in mem and source of each statement)

int start_addr, end_addr;  // Start and end addresses in mem[] of the statement
int start_line, start_col; // Position where the statement begins in source
int end_line, end_col;     // Position where the statement ends in source

void statement_start(void);  // Set start_addr, start_line, start_col
void statement_end(void);    // Set end_addr, end_line, end_col
void record_statement(void); // Write the six variables to file

//-----------------------------------------------------------------------------

// Object table --------------------------------- *iobj = first free object

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
      int total_len;        // total number of elements
      int len1, len2, len3; // len2/3==-1 if n/a
    } byte_global, ptr_byte_global;
    struct { // Global word
      int offset;
      int total_len;        // total number of elements
      int len1, len2, len3; // len2/3==-1 if n/a
    } word_global, ptr_word_global;
    struct { // Global string
      int offset;
      int total_len; // total number of elements
    } string_global, ptr_string_global;
    struct { // Struct
      int offset;
      int item_len;         // Number of fields
      int total_items;      // Total number of records
      int dim1, dim2, dim3; // -1 if n/a
    } struct_global, struct_local;
    struct { // Struct pointer
      int offset;
      struct object *ostruct; // Pointer to struct
      int total_items;        // Total number of records
      int dim1, dim2, dim3;   // -1 if n/a
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
      int total_len;        // total number of elements
      int len1, len2, len3; // len2/3==-1 if n/a
    } byte_local, ptr_byte_local;
    struct { // Local word
      int offset;
      int total_len;        // total number of elements
      int len1, len2, len3; // len2/3==-1 if n/a
    } word_local, ptr_word_local;
    struct { // Local string
      int offset;
      int total_len; // total number of elements
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
} obj[max_obj], *iobj; // ----------------------------------------------------

//-----------------------------------------------------------------------------

byte *vnom = NULL; // Name vector (hash_chain:int, current_token (or iobj):int, asciiz)
union {
  byte *b;
  byte **p;
} name_index;

byte *objects_start; // For generating the object table listing

byte *vhash[256]; // Pointer to the vector of names

//-----------------------------------------------------------------------------

struct exp_ele {
  byte type; // econs, eoper, erango, ewhoami, ecall
  union {
    int value;
    int token;
    struct object *object;
  };
} tabexp[max_exp], *_exp;

struct lex_ele {
  byte character;
  byte token;
  struct lex_ele *alternative;
  struct lex_ele *next;
} lex_simb[max_nodos], *ilex_simb, *lex_case[256];

int current_token, token_value;
struct object *o;             // When current_token=p_id, object of type (**o).type
struct object *current_scope; // Block currently being analyzed
struct object *lexical_scope; // 0 until program's private section

struct object *member; // !=0 when declaring/accessing a struct member

int source_line, num_nodes, num_objects;

int num_predefined;

//-----------------------------------------------------------------------------

int in_params;

int cross_process_access; // To disallow id.private

//-----------------------------------------------------------------------------

#define max_break 512
int tbreak[max_break];
int itbreak;

#define max_cont 256
int tcont[max_cont];
int itcont;

//-----------------------------------------------------------------------------

int eval_stack[long_pila + max_exp + 64]; // expression evaluation (compilation and execution)

int32_t *mem_ory = NULL, *frm = NULL;
int32_t *mem = NULL, *loc = NULL;
int imem, imem_max;
int iloc, iloc_max;
int itxt, ifrm_max;

byte *imemptr;

int local_var_len; // end of local variables including private ones

// mem[]        destination memory vector
// imem         pointer for data and code generation
// imem_max     end of destination memory (maximum index of mem[])
// iloc         start in mem[] of local variables (initialized)
// local_var_len     length of local variables

//------------- Expression table insertion, variable access

//var_global           &var_global ptr
//table_global[_exp]     &table_global <_exp> add rng ptr

//var_local           &var_local aid ptr
//table_local[_exp]      &table_local <_exp> add rng aid ptr

//proc.var_local      &proc ptr &var_local add ptr
//proc.table_local[_exp] &proc ptr &table_local <_exp> add rng add ptr

//-----------------------------------------------------------------------------
//      Formatted listing variables
//-----------------------------------------------------------------------------

int comment_depth = 0; // 0-Code, 1-Inside comment, 2-Nested, ...
int emit_tokens = 0;   // Whether to generate tokens in listing

//-----------------------------------------------------------------------------
//      Error information
//-----------------------------------------------------------------------------

int error_number = -1;
int error_line;
int error_col;

int error_27 = 27; // To emit "statement expected"

//-----------------------------------------------------------------------------

int program_type = 0;

//-----------------------------------------------------------------------------

struct {     // Peephole optimization window
  int dir;   // Address
  int param; // Whether this opcode has a parameter
  int op;    // Opcode
} code[16];  // code[15] must always hold the last generated instruction

//-----------------------------------------------------------------------------
//      Compiler initialization
//-----------------------------------------------------------------------------

FILE *linf; // EXEC.LIN
FILE *lprg; // EXEC.PGM
FILE *lins; // EXEC.INS

FILE *def; // For the "ltlex.def" analyzer
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
  if (compilemode) {
    fprintf(stdout, "%s\n", p);
  } else {
    wbox(v.ptr, v.w / big2, v.h / big2, c2, 2, 20, v.w / big2 - 4, 7);
    wwrite(v.ptr, v.w / big2, v.h / big2, 3, 20, 0, p, c3);
    flush_window(0);
    flush_copy();
  }
}

#ifndef ZLIB
#define uLongf unsigned long
#endif

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

  if ((vnom = (byte *)malloc(max_obj * long_med_id + 1024)) == NULL)
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


  imem_max = default_buffer;
  imem = 0;
  if ((mem_ory = mem = (int *)malloc(imem_max * sizeof(memptrsize))) == NULL)
    c_error(0, 0);
  memset(mem, 0, imem_max * sizeof(memptrsize));

  iloc_max = default_buffer / 2;
  iloc = 0;
  local_var_len = 0;
  if ((loc = (int *)malloc(iloc_max * sizeof(memptrsize))) == NULL)
    c_error(0, 0);
  memset(loc, 0, iloc_max * sizeof(memptrsize));

  ifrm_max = default_buffer / 2;
  if ((frm = (int *)malloc(ifrm_max * sizeof(memptrsize))) == NULL)
    c_error(0, 0);
  memset(frm, 0, ifrm_max * sizeof(memptrsize));

  show_compile_message(texts[203]);

  imem = long_header;

  preload_objects(); // No literals in the preloaded objects

  show_compile_message(texts[200]);

  source = source_ptr;
  _source = source;
  *(source + source_len) = cr;
  *(source + source_len + 1) = cr;

  emit_tokens = 1;
  ultima_linea = source;
  cross_process_access = 0;
  in_params = 0;
  source_line = 1;

  itxt = inicio_textos = imem;
  psintactico(); // To obtain "longitud_textos"
  imem += longitud_textos;

  test_buffer(&mem, &imem_max, imem);

  num_predefined = num_objects;

  emit_tokens = 1;
  ultima_linea = source;
  fwrite(&cero, 1, 1, lprg);
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
  mem[3] = max_process; // Previously long_header, now unused
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
  int columna = 0;
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
      columna++;
    }
    if (p < source_ptr)
      columna++;
  }

  if (tipo == 1) {
    if (*(_p + 1) == ' ' || *(_p + 1) == cr || *(_p + 1) == lf) {
      columna++;
    }
    columna++;
  } else if (tipo == 3)
    columna++;

  error_col = columna;

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

  if (n + buffer_grow > 2 * 1000 * 1000)
    c_error(0, 0);

  if (n > *maximo - security_distance) {
    max = *maximo;
    *maximo = n + buffer_grow;
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
          if (num_nodes++ == max_nodos)
            c_error(0, 3);
          e = lex_case[*buf] = ilex_simb++;
          (*e).character = *buf++;
        } else
          buf++;
        while (*buf != ' ' && *buf != tab && *buf != cr) {
          if (lower[*buf])
            c_error(0, 4);
          if ((*e).next == 0) {
            if (num_nodes++ == max_nodos)
              c_error(0, 3);
            else
              e = (*e).next = ilex_simb++;
          } else {
            e = (*e).next;
            while ((*e).character != *buf && (*e).alternative)
              e = (*e).alternative;
            if ((*e).character != *buf) {
              if (num_nodes++ == max_nodos)
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

  while (current_token != p_ultima)
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
        fwrite(ultima_linea, 1, _source - ultima_linea, lprg);
        fwrite(&cero, 1, 1, lprg);
      }

      source_line++;
      if ((*++_source) == lf) {
        _source++;
        ultima_linea = _source;
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
int en_fopen = 0;

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

  int empaquetable;
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
      fwrite(ultima_linea, 1, _source - ultima_linea, lprg);
      fwrite(&cero, 1, 1, lprg);
    }

    source_line++;
    if ((*++_source) == lf) {
      _source++;
      ultima_linea = _source;
      goto lex_scan;
    }
    current_token = p_ultima;
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
    if (name_index.b - vnom > max_obj * long_med_id)
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
          if (num_objects++ == max_obj)
            c_error(0, 102);
#else
          if (num_objects++ == max_obj)
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
      if (num_objects++ == max_obj)
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

      empaquetable = 0;

      // Determine whether the file is packable ...

      _splitpath(full, (char *)drive, (char *)dir, (char *)fname, (char *)ext);
      strupr((char *)ext);

      if (!stricmp((char *)ext, ".MOD") || !stricmp((char *)ext, ".S3M") ||
          !stricmp((char *)ext, ".XM")) {
        empaquetable = 1;
      } else if (!stricmp((char *)ext, ".PCM")) {
        empaquetable = 1;
      } else if (!stricmp((char *)ext, ".PCX")) {
        if (fread(cwork, 1, 66, f) > 0) {
          if (cwork[2] == 1 && cwork[3] == 8 && cwork[65] == 1)
            empaquetable = 1;
        }
      } else if (fread(cwork, 1, 8, f) > 0) {
        if (!strcmp((char *)cwork, "pal\x1a\x0d\x0a"))
          empaquetable = 1;
        if (!strcmp((char *)cwork, "map\x1a\x0d\x0a"))
          empaquetable = 1;
        if (!strcmp((char *)cwork, "fpg\x1a\x0d\x0a"))
          empaquetable = 1;
        if (!strcmp((char *)cwork, "fnt\x1a\x0d\x0a"))
          empaquetable = 1;
        if (!strcmp((char *)cwork, "wld\x1a\x0d\x0a"))
          empaquetable = 1;
        if (!strcmp((char *)cwork, "wld\x1a\x0d\x0a\x01"))
          empaquetable = 1;
      }

      fclose(f);

      if (is_wav(full))
        empaquetable = 1;

      // ???
      empaquetable = 1;

      if (!empaquetable || en_fopen)
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

    if (current_token == p_ultima) {
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

int analyze_pointer_struct(int tipo, int offset, struct object *estructura) {
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
  (*ob).ptr_struct_global.ostruct = estructura; // original struct

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
  comprueba_id = 1;
  comprueba_null = 1;
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
          comprueba_id = 0;
          comprueba_null = 0;
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
          comprueba_id = 0;
          break;
        case 11: // _no_null_check
          lexer();
          comprueba_null = 0;
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

  // Generate jump to start of code (long_header)

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

  if (current_token != p_ultima)
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

#undef memptrsize
#if __WORDSIZE == 64
#define memptrsize int64_t
#else
#define memptrsize int32_t
#endif


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
  int dir, from, to, step;

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
        en_fopen = 1;
        lexer();
        en_fopen = 0;
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
      ultima_linea = _source;
      goto lex_scan;
    }
    current_token = p_ultima;
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
    if (name_index.b - vnom > max_obj * long_med_id)
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
    longitud_textos += (strlen((char *)name_index.b) + ptr4) / 4;
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

    if (current_token == p_ultima) {
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

  longitud_textos = 0;

  do {
    plexico();
  } while (current_token != p_ultima);

  name_index.b = _ivnom;
  source = source_ptr;
  _source = source;
  comment_depth = 0;
}
