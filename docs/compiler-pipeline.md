# DIV Games Studio - Compiler Pipeline

The DIV compiler lives entirely in `src/compiler/compiler.c` (~7,800 lines). It compiles
DIV source code into a bytecode format (EML -- "Ensamblador de la Maquina
Logica", Logical Machine Assembler) that runs on the DIV virtual machine.

---

## 1. Compilation Entry Point

### `compile()` at line 997

Called from `comp()` (line 689), which wraps it in a `setjmp`/`longjmp` for
error recovery. The compilation pipeline:

1. **Reset state** -- Clear object table, lexer tables, hash table, peephole buffer
2. **Allocate `vnom`** -- Name vector for identifiers (`max_obj * long_med_id` bytes)
3. **`analyze_ltlex()`** -- Load lexer definition from `system/ltlex.def`
4. **`preload_objects()`** -- Load predefined objects from `system/ltobj.def`
   (built-in constants, globals, locals, structs, functions)
5. **Allocate `mem[]`** -- Main output buffer (starts at 128KB, grows as needed)
6. **Allocate `loc[]`** -- Local variable template buffer
7. **Allocate `frm[]`** -- Frame/initialization data buffer
8. **`psintactico()`** -- Pre-parse to determine string literal total length
9. **`parser()`** -- Full parse + code generation
10. **Check unused objects** -- Error if any identifier was used but never defined
11. **Fill header** -- `mem[0..8]` with program metadata
12. **`save_dbg()`** -- Write debug info to `system/exec.lin`, `exec.pgm`
13. **`save_exec_bin()`** -- Write `system/EXEC.EXE` (stub + compressed bytecode)

### Output Files

| File | Content |
|------|---------|
| `system/EXEC.EXE` | 602-byte DOS stub + 9-word header + zlib-compressed bytecode |
| `system/exec.lin` | Line number info (source position to bytecode mapping) |
| `system/exec.pgm` | Source listing for the debugger |
| `system/exec.ins` | Packed file list (resources referenced by the program) |
| `system/exec.fs` | Fullscreen mode flag (1 byte) |
| `system/exec.path` | Working directory path |

### Executable Header: `mem[0..8]`

| Index | Content |
|-------|---------|
| `mem[0]` | Flags (bit 7=debug mode, bit 9=ignore_errors, +128=debugger, +512=ignore, +1024=demo) |
| `mem[1]` | (unused) |
| `mem[2]` | `imem` -- Total code+data size |
| `mem[3]` | `max_process` -- Maximum process count (from `compiler_options`) |
| `mem[4]` | (unused, was global data length) |
| `mem[5]` | `local_var_len - iloc` -- Private variable section size |
| `mem[6]` | `iloc` -- Start of local variable template |
| `mem[7]` | (unused, was text start) |
| `mem[8]` | `imem + iloc` -- Total memory elements needed |

---

## 2. Lexer

### Lexer Initialization: `analyze_ltlex()` at line 1353

The lexer is table-driven. Its tables are loaded from `system/ltlex.def`, a
definition file that maps:

- **Hex-prefixed keywords** (`&XX identifier`) -- e.g., `&01 program` maps
  the keyword `program` to token `0x01` (`p_program`)
- **Symbol sequences** (`&XX chars`) -- e.g., `&38 =` maps `=` to `p_asig`,
  `&44 +` maps `+` to `p_suma`
- **Literal delimiters** (`&78`-`&7b`) -- marks characters like `"` as literal
  delimiters

The lexer uses two main data structures:

1. **`lex_case[256]`** -- For each byte value, either a pointer to a
   `lex_ele` node (for symbols) or one of the special values:
   - `l_err` (0) = illegal character
   - `l_cr` (1) = end of line / EOF
   - `l_id` (2) = identifier or keyword
   - `l_spc` (3) = whitespace
   - `l_lit` (4) = literal string delimiter
   - `l_num` (5) = digit (start of number)

2. **`vhash[256]`** -- Hash table for identifier/keyword lookup, using a
   rotate-XOR hash of the lowercased name

3. **`lex_simb[128]`** -- Trie nodes for multi-character symbols (e.g., `==`,
   `>=`, `<<`, `//`)

### The `lower[256]` Table

Defined in `src/ide/main.c` (line 106). Maps every byte to its lowercase equivalent
for case-insensitive matching. Characters not in `lower` (mapping to 0) are
treated as invalid identifier characters. This table determines the valid
character set for identifiers:

- Letters a-z, A-Z (mapped to lowercase)
- Digits 0-9
- Underscore `_`
- Accented characters (Latin-1 range 0x80-0xFF, mapped to lowercase equivalents)
- `#` and `$` are included as valid identifier characters

### Tokenization: `lexer()` at line 1754

The main lexer function. Sets global `current_token` and `token_value`
(numeric value for numbers/literals). Also tracks `source_line`, `ierror`,
`ierror_end` for error reporting.

Token types (selected):

| Token | Hex | Meaning |
|-------|-----|---------|
| `p_ultima` | 0x00 | End of file |
| `p_program` | 0x01 | `PROGRAM` keyword |
| `p_const` | 0x02 | `CONST` section |
| `p_global` | 0x03 | `GLOBAL` section |
| `p_local` | 0x04 | `LOCAL` section |
| `p_begin` | 0x05 | `BEGIN` |
| `p_end` | 0x06 | `END` |
| `p_process` | 0x07 | `PROCESS` definition |
| `p_private` | 0x08 | `PRIVATE` section |
| `p_struct` | 0x09 | `STRUCT` definition |
| `p_function` | 0x11 | `FUNCTION` definition |
| `p_if` | 0x20 | `IF` statement |
| `p_loop` | 0x21 | `LOOP` statement |
| `p_while` | 0x22 | `WHILE` statement |
| `p_repeat` | 0x23 | `REPEAT` statement |
| `p_for` | 0x24 | `FOR` statement |
| `p_switch` | 0x25 | `SWITCH` statement |
| `p_frame` | 0x28 | `FRAME` statement (yield) |
| `p_clone` | 0x2B | `CLONE` statement |
| `p_asig` | 0x38 | `=` (assignment) |
| `p_abrir` | 0x40 | `(` |
| `p_cerrar` | 0x41 | `)` |
| `p_suma` | 0x44 | `+` |
| `p_resta` | 0x45 | `-` |
| `p_id` | 0xFD | Identifier (object pointer in `o`) |
| `p_num` | 0xFE | Numeric constant (value in `token_value`) |
| `p_lit` | 0xFC | String literal (offset in `token_value`) |

### Number Parsing

Supports decimal and hexadecimal (`0x` prefix). No floating-point -- DIV is
an integer-only language.

### String Literal Handling

String literals (delimited by `"`) are:
- Stored directly in the `mem[]` output buffer at position `itxt`
- Consecutive string literals are automatically concatenated
- The lexer checks if the literal names a file that exists on disk, and if so,
  logs it to `exec.ins` for packaging

### Comment Handling

- `//` -- Single-line comment (to end of line)
- `/* ... */` -- Multi-line comment (nestable via `coment` counter)

---

## 3. Symbol Table

### Object Table: `obj[max_obj]` at line 843

```c
struct object {
    byte type;              // Object type (tnone, tcons, tvglo, ttglo, ...)
    byte used;              // 1 if referenced before definition
    byte * name;            // Pointer to name string in vnom
    byte * ierror;          // Source position for error reporting
    int line;               // Source line number
    int param;              // 1 if declared as a parameter
    struct object * prev;      // Previous object with same name (scope chain)
    struct object * scope;     // Owning process (NULL for global/local scope)
    struct object * member;    // Owning struct (NULL if not a struct member)
    union { ... };          // Type-specific data (offset, dimensions, etc.)
};
```

### Object Types

| Type | Code | Description |
|------|------|-------------|
| `tnone` | 0 | Undefined |
| `tcons` | 1 | Named constant |
| `tvglo` | 2 | Global variable |
| `ttglo` | 3 | Global array (table) |
| `tcglo` | 4 | Global string |
| `tvloc` | 5 | Local variable |
| `ttloc` | 6 | Local array |
| `tcloc` | 7 | Local string |
| `tproc` | 8 | Process/program |
| `tfunc` | 9 | Built-in function |
| `tsglo` | 10 | Global struct |
| `tsloc` | 11 | Local struct |
| `tfext` | 12 | External (DLL) function |
| `tbglo` | 13 | Global byte variable |
| `twglo` | 14 | Global word variable |
| `tbloc` | 15 | Local byte variable |
| `twloc` | 16 | Local word variable |
| `tpigl`-`tpslo` | 17-26 | Various pointer types |

### Scope Resolution

Scope is managed through:
- **`current_scope`** -- Current process being compiled (NULL during
  global/local sections)
- **`lexical_scope`** -- Set to `current_scope` when entering private sections
- **`member`** -- Set to the current struct when parsing struct member definitions

When looking up an identifier:
1. The hash table (`vhash[]`) finds all objects with that name
2. The chain of `prev` pointers links objects with the same name in
   different scopes
3. The lookup walks the chain, matching `scope` and `member` to find the
   correct scope

### Predefined Objects: `preload_objects()` at line 1434

Loaded from `system/ltobj.def`. This file defines:
- Built-in constants (e.g., `true`, `false`, `_max_process`, screen resolutions)
- Predefined global variables (the process-accessible runtime state)
- Predefined local variables (per-process fields like `x`, `y`, `graph`, etc.)
- Predefined structs (`mouse`, `scroll`, `m7`, `joy`, `setup`, etc.)
- Built-in function signatures (parameter types and return types)

---

## 4. Parser

### Grammar Overview

DIV uses a recursive descent parser. `parser()` (line 3172) handles the
top-level program structure:

```
PROGRAM ::= [compiler_options] PROGRAM id ";"
            [CONST { id "=" expr ";" }]
            [GLOBAL { variable_decl }]
            [LOCAL { variable_decl }]
            [IMPORT { import_decl }]
            BEGIN
              statements
            END
            { process_or_function }

PROCESS ::= PROCESS id "(" [params] ")"
            [PRIVATE { variable_decl }]
            BEGIN
              statements
            END

FUNCTION ::= FUNCTION id "(" [params] ")"
             [PRIVATE { variable_decl }]
             BEGIN
               statements
             END
```

### Variable Declarations

Variables can be declared as:
- Simple: `x;` or `x = 5;`
- Array: `x[10];` or `x[] = 1, 2, 3;`
- Multi-dimensional: `x[3][4][5];` (up to 3 dimensions)
- Typed: `INT x;`, `BYTE x;`, `WORD x;`, `STRING x[20];`
- Pointer: `INT POINTER p;`, `BYTE POINTER p;`
- Struct: `STRUCT name[N] field1; field2; END`

### Statement Parsing: `statement()` at line 4867

Handles all control flow constructs:

| Statement | Structure |
|-----------|-----------|
| `IF` | `IF (cond) stmts [ELSE stmts] END` |
| `WHILE` | `WHILE (cond) stmts END` |
| `REPEAT` | `REPEAT stmts UNTIL (cond)` |
| `LOOP` | `LOOP stmts END` (infinite loop) |
| `FOR` | `FOR (init; cond; step) stmts END` (C-style) |
| `FROM` | `FROM var=a TO b [STEP s] stmts END` (counted loop) |
| `SWITCH` | `SWITCH (expr) CASE val: stmts ... [DEFAULT: stmts] END` |
| `RETURN` | `RETURN [(expr)]` |
| `FRAME` | `FRAME [( percent )]` (yield execution for this frame) |
| `CLONE` | `CLONE stmts END` (fork current process) |
| `BREAK` | `BREAK` (exit innermost loop) |
| `CONTINUE` | `CONTINUE` (jump to loop increment) |
| `DEBUG` | `DEBUG` (invoke debugger) |

Statements also include assignments (`expr;`) which are parsed via `expression()`.

### Expression Parsing: `expression()` at line 5284

Expressions use a two-phase approach:

1. **Phase 1: Build expression tree** -- `exp00()` through `factor()` parse the
   expression into `tabexp[]`, an array of `exp_ele` elements in postfix
   (reverse Polish) notation.

2. **Phase 2: Generate code** -- `generate_expression()` (line 5322) walks
   `tabexp[]` and emits bytecode instructions.

The expression parser is recursive descent with explicit precedence levels:

```
exp00 -> con00 (handles type coercion)
  con0 -> con1 [|| con1]*           -- logical OR
    con1 -> con2 [&& con2]*         -- logical AND
      con2 -> exp0 [==|!=|>|<|>=|<= exp0]*  -- comparison
        exp0 -> exp1 [| exp1]*      -- bitwise OR
          exp1 -> exp2 [^ exp2]*    -- bitwise XOR
            exp2 -> exp3 [& exp3]*  -- bitwise AND
              exp3 -> exp4 [<<|>> exp4]*  -- shifts
                exp4 -> exp5 [+|- exp5]*  -- addition
                  exp5 -> exp6 [*|/|% exp6]*  -- multiplication
                    exp6 -> unary  -- unary operators
                      unary -> factor  -- atoms
```

`factor()` handles:
- Constants and literals
- Identifiers (variables, arrays, struct access via `.`)
- Process creation (calling a process name like a function)
- Built-in function calls
- Parenthesized sub-expressions
- Prefix `++`, `--`, `-`, `NOT`, `OFFSET`, `SIZEOF`, `POINTER`
- Postfix `++`, `--`

---

## 5. Code Generation

### Bytecode Format: EML

The DIV VM uses a stack-based bytecode. Instructions are stored as 32-bit
integers in the `mem[]` array. Each instruction is either:
- **1-word** (opcode only): `g1(opcode)`
- **2-word** (opcode + operand): `g2(opcode, operand)`

### Code Generation Functions

```c
void g1(int op);           // Emit single-word instruction
void g2(int op, int pa);   // Emit two-word instruction (opcode + parameter)
```

Both functions feed through a peephole optimizer (`code[16]` ring buffer) that
combines common instruction pairs into fused opcodes (see optimization
instructions lcar2 through lcardiv, opcodes 60-77).

### Instruction Set (EML Opcodes)

**Core instructions (0-59):**

| Opcode | Name | Operands | Description |
|--------|------|----------|-------------|
| 0 | `lnop` | -- | No operation |
| 1 | `lcar` | value | Push constant onto stack |
| 2 | `lasi` | -- | Assignment: pop value and address, store value at address |
| 3 | `lori` | -- | Bitwise OR of top two stack values |
| 4 | `lxor` | -- | Bitwise XOR |
| 5 | `land` | -- | Bitwise AND |
| 6-11 | `ligu`..`lmai` | -- | Comparison operators (==, !=, >, <, <=, >=) |
| 12-16 | `ladd`..`lmod` | -- | Arithmetic (add, sub, mul, div, mod) |
| 17 | `lneg` | -- | Negate top of stack |
| 18 | `lptr` | -- | Dereference: replace address with value at that address |
| 19 | `lnot` | -- | Bitwise NOT (XOR with -1) |
| 20 | `laid` | -- | Add current process ID to top of stack |
| 21 | `lcid` | -- | Push current process ID |
| 22 | `lrng` | max | Range check (debug: error if TOS < 0 or > max) |
| 23 | `ljmp` | addr | Unconditional jump |
| 24 | `ljpf` | addr | Jump if false (TOS bit 0 = 0) |
| 25 | `lfun` | code | Call built-in function |
| 26 | `lcal` | addr | Create new process (spawn) |
| 27 | `lret` | -- | Return / kill current process |
| 28 | `lasp` | -- | Pop (discard top of stack) |
| 29 | `lfrm` | -- | FRAME: yield execution for this frame |
| 30 | `lcbp` | n_params | Set base pointer for parameter passing |
| 31 | `lcpa` | -- | Copy parameter value |
| 32 | `ltyp` | block_id | Set process type (for collision detection) |
| 33 | `lpri` | addr | Jump to address and load private variables |
| 34 | `lcse` | addr | SWITCH CASE: compare and jump if not equal |
| 35 | `lcsr` | addr | SWITCH CASE RANGE: jump if not in range |
| 36-37 | `lshr`,`lshl` | -- | Bit shift right/left |
| 38-41 | `lipt`..`lptd` | -- | Pre/post increment/decrement with pointer |
| 42-51 | `lada`..`lsla` | -- | Compound assignment operators (+=, -=, etc.) |
| 52 | `lpar` | n_priv | Define number of private parameters |
| 53 | `lrtf` | -- | Return with value (kills process, pushes return value) |
| 54 | `lclo` | addr | CLONE: fork current process |
| 55 | `lfrf` | -- | FRAME with percentage (partial frame) |
| 56 | `limp` | offset | Import DLL |
| 57 | `lext` | code | Call external (DLL) function |
| 58 | `lchk` | -- | Validate process ID |
| 59 | `ldbg` | -- | Invoke debugger |

**Optimization fusions (60-77):**

| Opcode | Name | Equivalent |
|--------|------|------------|
| 60 | `lcar2` | `lcar; lcar` (push two constants) |
| 63 | `lasiasp` | `lasi; lasp` (assign and discard) |
| 64 | `lcaraid` | `lcar; laid` (push constant + add ID) |
| 65 | `lcarptr` | `lcar; lptr` (push address, dereference) |
| 67 | `lcaraidptr` | `lcar; laid; lptr` (push offset, add ID, dereference) |
| 75 | `lcarasiasp` | `lcar; lasi; lasp` (push address, assign, discard) |

**Byte/char operations (78-94):** Mirror the int operations but operate on
individual bytes within the int32 memory array, using byte indexing.

**String operations (95-108):** `lstrcpy`, `lstrcat`, `lstradd`, `lstrdec`,
`lstrsub`, `lstrlen`, `lstrigu`..`lstrmai`, `lcpastr`.

**Word operations (109-125):** Mirror byte operations for 16-bit values.

**Miscellaneous (126):** `lnul` -- null pointer check.

### Process Compilation

When a `PROCESS` or `FUNCTION` is encountered:

1. A `tproc` object is created in the symbol table
2. Parameters are parsed and stored as local variables
3. `PRIVATE` variables are parsed
4. The process body is compiled between `BEGIN`/`END`
5. A `ltyp` instruction is emitted to set the process type
6. A `lpri` instruction loads private variable initializers
7. Code ends with `lret` (implicit if no explicit `RETURN`)

The difference between PROCESS and FUNCTION:
- FUNCTION has `_FCount > 0`, which makes the caller sleep until the function
  returns (synchronous call)
- PROCESS is asynchronous -- the caller continues executing

### Memory Layout

The compiled program's `mem[]` array contains:

```
[0..8]        Header (9 words)
[9..iloc-1]   Global data (variables, arrays, structs, strings, code)
[iloc..]      Local variable template (copied to each new process)
```

When a process is created at runtime, `iloc_len` words are allocated from the
process pool, and the local template is copied in. Process-specific data
(like `_IP`, `_Status`, `_X`, `_Y`, etc.) lives at fixed offsets within
each process's local block.

---

## 6. Include / Import

### INCLUDE

`INCLUDE "filename"` at the program level causes the compiler to read and
compile the contents of the named file. The source pointer is saved and
restored. Include files can contain any declarations that would be valid at
the point of inclusion.

### IMPORT

`IMPORT "library.dll"` imports external DLL functions. In the original DOS
version, this loaded custom DLLs. The SDL2 port has removed the DLL loading
system, but the compiler still processes IMPORT declarations.

### Compiler Options

`COMPILER_OPTIONS` at the start of a program can set:
- `_max_process = N` -- Maximum concurrent processes
- `_extended_conditions` -- Allow non-boolean expressions in conditions
- `_simple_conditions` -- Use simple (C-style) condition evaluation
- `_case_sensitive` -- Enable case-sensitive identifiers
- `_ignore_errors` -- Continue on runtime errors
- `_free_syntax` -- Relax syntax requirements (no semicolons needed)
- `_no_check` -- Disable all runtime checks
- `_no_strfix` -- Disable automatic string padding
- `_no_optimization` -- Disable peephole optimizer
