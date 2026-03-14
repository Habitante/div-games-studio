# DIV Games Studio — 64-bit VM Migration Roadmap

## 1. Vision

Every value in the DIV language becomes a **64-bit 32.32 fixed-point number**.
The upper 32 bits hold the signed integer part; the lower 32 bits hold the
fractional part. There is no `FLOAT` type, no `INT` type — just numbers.
Some have decimals, some don't. The language stays simple.

```
PROGRAM physics_demo;
GLOBAL
    gravity = 9.8;
    speed;
PRIVATE
    dx, dy;
BEGIN
    dy = 0;
    LOOP
        dy += gravity * 0.016;
        y += dy;
        FRAME;
    END
END
```

No casts, no type keywords, no conversions. Variables are variables.
This returns to the DIV1 philosophy of simplicity.

### 32.32 Fixed-Point Format

```
Bit layout:  [31 sign + integer bits] . [32 fractional bits]

Integer 5:     0x00000005_00000000
Value 3.14:    0x00000003_23D70A3D
Value -1.5:    0xFFFFFFFE_80000000
Value 0.001:   0x00000000_00418937
Zero:          0x00000000_00000000
```

- **Integer range**: -2,147,483,648 to +2,147,483,647 (same as current `int`)
- **Fractional precision**: 1/2^32 ~ 2.33x10^-10 (~9.3 decimal digits)
- **One unit** = `1LL << 32` = 4,294,967,296

### Why 32.32 and Not IEEE Float

| Aspect | 32.32 Fixed-Point | IEEE Float Type |
|--------|-------------------|-----------------|
| Type system changes | None — one numeric type | New FLOAT keyword, type tracking, casts |
| New opcodes | 0 (modify 3: mul, div, mod) | ~15 float-specific opcodes |
| Compiler complexity | Unchanged | Expression type propagation throughout |
| Integer range | ±2 billion (same as today) | ±2B int, ±3.4e38 float (two ranges) |
| Fractional precision | ~9.3 decimal digits | ~7 digits (float32) |
| Deterministic | Yes — identical on every platform | IEEE has rounding mode variation |
| Division semantics | `10 / 3 = 3.333...` | Still truncates for int operands |
| Bitwise operations | Work naturally (integer part) | Meaningless on floats |
| Language feel | Same DIV simplicity | Adds complexity (two worlds) |

---

## 2. Architectural Boundary

The 64-bit change is **scoped to the VM program memory only**. The runtime
already has a clean boundary between the VM world and everything else.

### VM World (goes 64-bit)

These subsystems use `mem[]` and `stack[]` and need migration:

| File | `mem[]` refs | `memb[]` refs | `stack[]` refs | Notes |
|------|-------------|---------------|----------------|-------|
| `runtime/debug/kernel.inc` | 189 | 32 | many | Bytecode dispatch — the core |
| `runtime/functions.c` | 172 | 7 | 635 | Built-in function implementations |
| `runtime/interpreter.c` | 118 | 4 | 3 | Process scheduling, init/shutdown |
| `runtime/render.c` | 112 | 0 | 0 | Reads process fields for sprite drawing |
| `shared/run/collision.c` | 77 | 0 | 19 | Reads process fields for hit tests |
| `shared/run/pathfind.c` | 7 | 0 | 25 | Reads process position fields |
| `compiler/compiler.c` | — | — | — | Output buffer, lexer, constants |
| `compiler/compiler_parser.c` | — | — | — | Variable allocation into mem[] |
| `compiler/compiler_expression.c` | — | — | — | Code generation into mem[] |
| `compiler/compiler_internal.h` | — | — | — | Type/opcode definitions |
| `runtime/inter.h` | — | — | — | Process field offsets, struct overlays |

### External World (untouched)

These subsystems never reference `mem[]` and require **zero changes**:

| File | What it does |
|------|-------------|
| `formats/image.c` | MAP/PCX/BMP/JPEG loading — reads files into separate structs |
| `formats/fpg.c` | FPG loading — populates `g[]` array, not mem[] |
| `shared/run/video.c` | Palette + screen blit — works with `screen_buffer` (byte*) |
| `shared/run/sound.c` | SDL2_mixer audio — works with `Mix_Chunk*` |
| `shared/osdep/osd_sdl2.c` | Platform abstraction — SDL surfaces/textures |
| `editor/*.c` | Paint/font/code editors — IDE only |
| `ide/*.c` | IDE shell — menus, windows, dialogs |
| `runner/runner.c` | Launcher — no VM interaction |
| All file formats on disk | FPG, MAP, FNT, PAL stay 32-bit on disk |

### Bridge Points

The two worlds connect at exactly four interface patterns:

**Pattern 1 — Renderer reads process fields** (~112 spots in `render.c`):
```c
// Before:
int x = mem[ide + _X];
// After:
int x = INT_PART(mem[ide + _X]);
```

**Pattern 2 — Collision reads process fields** (~77 spots in `collision.c`):
Same as Pattern 1.

**Pattern 3 — Built-in functions bridge both ways** (`functions.c`):
```c
// Reading from VM world into C:
int pixel_x = INT_PART(stack[sp]);

// Writing from C into VM world:
stack[sp] = FROM_INT(result);
```

**Pattern 4 — System state written to globals** (`interpreter.c`):
```c
// Mouse/keyboard/timer values written into mem[] predefined globals:
mouse->x = FROM_INT(sdl_mouse_x);
```

**Bridge macros:**
```c
#define INT_PART(v)  ((int32_t)((v) >> 32))
#define FROM_INT(v)  (((int64_t)(int32_t)(v)) << 32)
```

---

## 3. What Changes in the VM

### 3.1 Type Widening (mechanical)

All VM data types widen from 32 to 64 bits:

```c
// inter.h — before:
GLOBAL int *mem, imem, iloc, iloc_pub_len, iloc_len;
GLOBAL int stack[EVAL_STACK_SIZE + MAX_EXP_SIZE + 64];
GLOBAL int ip, sp, bp, id, ide, id2;
GLOBAL byte *memb;

// inter.h — after:
GLOBAL int64_t *mem;
GLOBAL int imem, iloc, iloc_pub_len, iloc_len;  // indices stay int (slot counts)
GLOBAL int64_t stack[EVAL_STACK_SIZE + MAX_EXP_SIZE + 64];
GLOBAL int64_t ip, sp, bp;  // could stay int if they're just indices
GLOBAL int id, ide, id2;    // process IDs are slot indices
GLOBAL byte *memb;          // still (byte*)mem
```

The predefined C structs that overlay `mem[]` change field types:

```c
// Before:
struct _mouse { int x, y, z, file, graph, ...; };
// After:
struct _mouse { int64_t x, y, z, file, graph, ...; };
```

Slot counts and process field offsets (`_X = 26`, `_Y = 27`, etc.) remain
numerically unchanged — they index 64-bit slots instead of 32-bit slots.

### 3.2 Process Creation

The template copy in `interpreter.c` changes stride:

```c
// Before (line 396, 401):
memcpy(&mem[id], &mem[iloc], iloc_pub_len << 2);   // × 4 bytes per slot
// After:
memcpy(&mem[id], &mem[iloc], iloc_pub_len << 3);   // × 8 bytes per slot
```

There are ~14 occurrences of `<< 2` or `* 4` in `interpreter.c` related to
mem[] slot sizing that need updating to `<< 3` or `* 8`.

### 3.3 Arithmetic Opcodes (3 logic changes)

**`lmul` (opcode 14) — 32.32 multiply:**
```c
// Before:
case lmul: stack[sp-1] *= stack[sp]; sp--; break;

// After:
case lmul: {
    __int128 r = (__int128)stack[sp-1] * stack[sp];
    stack[--sp] = (int64_t)(r >> 32);
    break;
}
```

GCC supports `__int128` on i386 — it generates four 32×32→64 multiplies.
Performance is ~15 instructions per multiply, invisible at DIV game scale.

**`ldiv` (opcode 15) — 32.32 divide:**
```c
// Before:
case ldiv:
    if (stack[sp] == 0) { stack[--sp] = 0; /* error */ }
    else { stack[sp-1] /= stack[sp]; sp--; }
    break;

// After:
case ldiv:
    if (stack[sp] == 0) { stack[--sp] = 0; /* error */ }
    else {
        __int128 r = ((__int128)stack[sp-1]) << 32;
        stack[--sp] = (int64_t)(r / stack[sp]);
    }
    break;
```

`10 / 3` now yields `3.333...` instead of `3`. This is a deliberate
semantic change. Programs needing integer truncation use `trunc()`.

**`lmod` (opcode 16) — modulo:**
```c
// After:
case lmod:
    if (stack[sp] == 0) { stack[--sp] = 0; /* error */ }
    else {
        __int128 r = ((__int128)stack[sp-1]) << 32;
        stack[--sp] = (int64_t)(r % stack[sp]);
    }
    break;
```

### 3.4 Boolean Test Fix (1 opcode)

**`ljpf` (opcode 24) — jump if false:**
```c
// Before (tests bit 0 — broken for 32.32 where integer 1 = 1<<32):
case ljpf: if (stack[sp--] & 1) ip++; else ip = mem[ip]; break;

// After (tests nonzero — correct for any representation):
case ljpf: if (stack[sp--]) ip++; else ip = mem[ip]; break;
```

This is actually more correct than the original even for 32-bit, and it's
a one-character change.

### 3.5 Peephole Optimization Fusions (opcodes 60-77)

These 18 fused opcodes combine sequences like `lcar; lmul` into single
instructions. Most only do loads/stores/adds and need only type widening.
The two that involve multiplication or division need the 32.32 treatment:

- `lcarmul` (73): fused `lcar; lmul` — needs `__int128` path
- `lcardiv` (77): fused `lcar; ldiv` — needs `__int128` path

All others (60-72, 74-76) are mechanical widenings.

---

## 4. BYTE/WORD Opcode Removal

### 4.1 Background

BYTE and WORD types were added in DIV2 (compiler versions 36-44), late in
development. They allowed memory-efficient packing of 8-bit and 16-bit
values into the `int` memory array. They required:

- 17 byte opcodes (78-94): `lptrchr`, `lasichr`, ..., `lcpachr`
- 17 word opcodes (109-125): `lptrwor`, `lasiwor`, ..., `lcpawor`
- `memb[]` overlay (`byte *memb = (byte*)mem`) for byte-level access
- `memw[]` overlay (`word *memw = (word*)mem`) for word-level access
- Compiler type tracking: `tbglo`, `twglo`, `tbloc`, `twloc` (4 types)
- Compiler parser: `p_byte`, `p_word` declaration paths
- Compiler codegen: `p_pointerchar`, `p_asigchar`, etc. token families

**No predefined global or local in `ltobj.def` uses BYTE or WORD.**
They were purely user-opt-in for DOS-era memory savings.

### 4.2 Removal Scope

| Location | Items removed | Count |
|----------|---------------|-------|
| `kernel.inc` — byte case handlers | `case lptrchr` through `case lcpachr` | 17 cases |
| `kernel.inc` — word case handlers | `case lptrwor` through `case lcpawor` | 17 cases |
| `compiler_internal.h` — opcode defines | `lptrchr`..`lcpachr`, `lptrwor`..`lcpawor` | 34 defines |
| `compiler_internal.h` — token defines | `p_pointerchar`..`p_pointerbyte`, `p_pointerword`..`p_shl_asigword` | ~32 defines |
| `compiler_internal.h` — object types | `tbglo`, `twglo`, `tbloc`, `twloc` | 4 types |
| `compiler_expression.c` — codegen paths | Byte/word expression handling | ~29 references |
| `compiler_parser.c` — declaration paths | `p_byte`/`p_word` branches + type assignments | ~56 references |
| `inter.h` — opcode defines (duplicate) | Same byte/word opcodes | 34 defines |
| `inter.h` — `memw` declaration | `GLOBAL word *memw` | 1 declaration |

**Total removal: ~34 opcodes, ~4 types, ~85 compiler references, ~225 lines of kernel.inc.**

### 4.3 Backward Compatibility

The `BYTE` and `WORD` keywords are kept in `ltlex.def` so old source code
parses without errors. The compiler treats them as synonyms for the default
type — every variable occupies a full 64-bit slot regardless. A `BYTE x;`
declaration simply creates a normal variable. Values 0-255 sit perfectly
in a 64-bit 32.32 slot with zero fractional part.

### 4.4 Also Removed: `memw[]`

With word opcodes gone, the `memw` pointer overlay is unused and deleted:

```c
// Removed from inter.h:
GLOBAL word *memw;
// Removed from interpreter.c initialization:
memw = (word *)mem;
```

`memb` is kept — it's still used by string operations.

### 4.5 Also Removed: `lnul` (opcode 126)

The null pointer check opcode is DEBUG-only and checks `if (!stack[sp])`.
This can be retained or removed at discretion. If retained, it works
unchanged with 64-bit values (zero is zero at any width). Listed here
because it sits between the word opcodes and is easy to overlook.

---

## 5. String Operations

### 5.1 How Strings Currently Work

Strings are stored directly in `mem[]`. Character bytes are packed into
int-sized slots:

```
mem[offset-1] = 0xDAD00000 | max_length   // metadata tag (debug only)
mem[offset]   = 'H','e','l','l'           // 4 chars per 32-bit slot
mem[offset+1] = 'o','\0', 0, 0           // rest + null + padding
```

Two access patterns exist in the string opcodes (95-108):

**Pattern A — Cast to `char*`** (used by `lstrcpy`, `lstrcat`, `lstrlen`,
all 6 string comparisons, `lcpastr`):
```c
memmove((char *)&mem[stack[sp-1]], (char *)&mem[stack[sp]], strlen(...)+1);
```

**Pattern B — Explicit `memb[]` indexing** (used by `lstrfix`, `lstrdec`,
`lstrsub`):
```c
memb[stack[sp-2] * 4 + oo] = ' ';
```

### 5.2 Impact of 64-bit Migration

**Pattern A is width-agnostic.** Casting `int64_t*` to `char*` gives a
valid byte pointer. `strlen()`, `strcmp()`, `memmove()` operate on raw
bytes and don't care about slot width. These opcodes need **zero changes**
to their string logic — only the type of `stack[]` values changes.

Opcodes using Pattern A only (no memb[] changes needed):
- `lstrcpy` (95), `lstrcat` (97), `lstradd` (98), `lstrlen` (101)
- `lstrigu` (102), `lstrdis` (103), `lstrmay` (104), `lstrmen` (105)
- `lstrmei` (106), `lstrmai` (107), `lcpastr` (108)

**Pattern B needs the multiplier update.** There are ~8 spots across
3 opcodes:

| Opcode | Name | `memb[]` refs | Change |
|--------|------|---------------|--------|
| 96 | `lstrfix` | 2 | `* 4` → `* 8` |
| 99 | `lstrdec` | 3 | `* 4` → `* 8` |
| 100 | `lstrsub` | 3 | `* 4` → `* 8` |

### 5.3 String Allocation Formula

```c
// Before: slots needed = (max_length + 5) / 4
// After:  slots needed = (max_length + 9) / 8
```

Each slot holds 8 characters instead of 4. Strings use less slots but
each slot is wider, so total memory per string is similar.

### 5.4 Null String Temporary Buffers

The 4-slot rotating buffer for intermediate string results
(`nullstring[0..3]`) in `interpreter.c` needs its allocation and metadata
updated:

```c
// Before:
nullstring[0] = imem_max + 1 + 258 * 0;
mem[nullstring[0] - 1] = 0xDAD00402;
// After (258 int32 slots = 129 int64 slots for ~1024 bytes):
nullstring[0] = imem_max + 1 + 129 * 0;
mem[nullstring[0] - 1] = 0xDAD00402LL;  // or a new 64-bit magic
```

### 5.5 Debug Magic Tag

The `0xDAD00000 | length` metadata at `mem[offset-1]` is only checked in
`#ifdef DEBUG` builds. Options:

1. Keep the same 32-bit pattern in the lower word of the 64-bit slot
2. Use a full 64-bit magic: `0xDAD00000_00000000LL | length`
3. Redesign with a separate validation array (cleanest, most work)

Option 1 is the path of least resistance for initial migration.

### 5.6 Stack Convention: Char vs String Pointer

The string opcodes distinguish single characters from string pointers
by testing `(unsigned)stack[sp] > 255`. In 32.32, an integer value of 255
is `255LL << 32` = `0x000000FF_00000000`, which is much larger than 255.

This test must change to compare against the 32.32 representation:

```c
// Before:
if ((unsigned)stack[sp] > 255)  // string pointer
// After:
if ((unsigned)(stack[sp] >> 32) > 255)  // or: if (INT_PART(stack[sp]) > 255)
```

This affects `lstrcpy`, `lstrcat`, `lstradd`, `lstrdec`, `lstrlen`, and
all 6 comparison opcodes — approximately 20 spots in `kernel.inc`.

---

## 6. Lexer Changes

### 6.1 Float Literal Parsing

The lexer in `compiler.c` (function `lexer()`, `case l_num`) currently
parses integers only. It needs to recognize decimal points:

```c
case l_num:
    // ... existing integer parsing into token_value ...
    // After integer part, check for decimal point:
    if (*_source == '.' && (uintptr_t)lex_case[*(_source + 1)] == l_num) {
        _source++;  // skip '.'
        int64_t frac = 0;
        int64_t scale = 1;
        while ((uintptr_t)lex_case[*_source] == l_num) {
            frac = frac * 10 + (*_source++ - '0');
            scale *= 10;
        }
        token_value = ((int64_t)token_value << 32)
                    | (int64_t)((double)frac / scale * 4294967296.0);
    } else {
        token_value = (int64_t)token_value << 32;  // integer → 32.32
    }
    break;
```

**Disambiguation**: The `.` character is also the struct-member-access
operator (`p_punto`). The check `lex_case[*(_source + 1)] == l_num`
ensures that `.field` (identifier after dot) is NOT treated as a decimal.
Only `.` followed by a digit is a decimal point.

### 6.2 Token Value Width

`token_value` changes from `int` to `int64_t`. All integer literals
are stored in 32.32 format (shifted left 32). This propagates through:

- `compiler_internal.h`: `extern int token_value` → `extern int64_t token_value`
- `compiler.c`: declaration of `token_value`
- `compiler_expression.c`: constant handling in `factor()`, `constant()`
- Hexadecimal literals (`0xFF`) also shift: result `<< 32`

### 6.3 Constant Folding

The compiler's constant evaluator (`constant()` in `compiler_expression.c`)
performs compile-time arithmetic on constants. It needs the same 32.32
mul/div treatment:

```c
// Compile-time multiply:
__int128 r = (__int128)a * b;
result = (int64_t)(r >> 32);

// Compile-time divide:
__int128 r = ((__int128)a) << 32;
result = (int64_t)(r / b);
```

---

## 7. Built-in Function Migration

### 7.1 Overview

150 built-in functions are defined in `ltobj.def` and implemented in
`functions.c`. They interact with the VM through `stack[]` (popping
arguments, pushing results) and `mem[]` (reading/writing process fields
and globals).

### 7.2 Categories and Migration Impact

**No changes needed (pure external operations):**
Functions that load files, play sounds, or manipulate SDL state without
doing arithmetic on VM values. They just pop handles/flags from the stack
and call external APIs. ~60 functions.

**Bridge-only changes (INT_PART / FROM_INT at boundaries):**
Functions that read process fields or coordinates from `mem[]`, do C-level
math, and push results back. Change: extract integer part on read, convert
back to 32.32 on write. ~50 functions.

Examples:
- `put_pixel(x, y, color)`: extract int x, int y, int color from stack
- `signal(id, sig)`: extract int id, int signal from stack
- `load_fpg(filename)`: extract string pointer, return int handle

**Trig functions — major simplification (7 functions):**

Currently these convert millidegree-scaled integers to float and back:
```c
// Before (sin, function 125):
void _sin(void) {
    float angle = (float)stack[sp] / radian;     // millidegrees → radians
    stack[sp] = (int)((float)sin(angle) * 1000);  // result × 1000
}
```

With 32.32, they work with actual degree values and return actual results:
```c
// After:
void _sin(void) {
    double a = (double)stack[sp] / 4294967296.0;  // 32.32 → double (degrees)
    double r = sin(a * M_PI / 180.0);             // degrees → radians → sin
    stack[sp] = (int64_t)(r * 4294967296.0);       // double → 32.32
}
```

`sin(45.0)` returns `0.7071...` instead of `707`. This is a semantic
change that makes the trig API dramatically simpler. The `radian` constant
and `* 1000` scaling convention are eliminated.

**Affected trig functions (7):** `sin` (125), `cos` (126), `tan` (127),
`asin` (128), `acos` (129), `atan` (130), `atan2` (131)

**Related functions also simplified (5):** `get_angle` (012),
`get_dist` (013), `get_distx` (010), `get_disty` (011), `fget_angle` (050)

**String functions — minor changes:**
The 11 string built-in functions (080 `char`, 085-094 `strcpy` through
`strdel`, 138 `itoa`) interact with `memb[]` for character access. They
need the same `* 4` → `* 8` treatment where they use byte offsets, plus
the `> 255` threshold fix for char-vs-pointer distinction.

**New function needed: `trunc()`:**
Since `10 / 3` now gives `3.333...`, programs that need integer division
semantics need a `trunc()` or `int()` function:
```c
// trunc(x): return the integer part, discarding fractional bits
void _trunc(void) {
    stack[sp] = stack[sp] & 0xFFFFFFFF00000000LL;
}
```

**New function needed: `frac()`:**
Return just the fractional part:
```c
void _frac(void) {
    int64_t sign = (stack[sp] < 0) ? -1 : 1;
    stack[sp] = (stack[sp] & 0x00000000FFFFFFFFLL) * sign;
}
```

**New function: `itoa()` update:**
The existing `itoa` (138) converts an integer to a string. It needs to
handle the fractional part to produce output like `"3.14"` instead of
just `"3"`. This also affects `write_int` (017) for displaying numeric
variables on screen.

### 7.3 Complete Function List by Migration Category

**Category A — No changes (pure external):** ~30 functions
- Sound: 037-041, 074, 077, 097-105, 139
- Animation: 043-046
- Scrolling: 004-005, 048, 054-055, 064
- System: 047, 060, 121-123, 142-146

**Category B — Bridge only (INT_PART/FROM_INT):** ~75 functions
- Graphics: 002-003, 007, 015, 020, 023-032, 056, 063, 072-073, 075-076,
  079, 084, 095, 124, 135, 148-149
- Display: 014, 033, 036, 058-059, 068, 078, 132-134
- Process: 000, 006, 008-009, 022, 066-067
- Input: 001, 070-071
- File I/O: 034-035, 106-120
- Text: 016-019, 136
- Pathfinding: 081-083
- Memory: 140-141
- Misc: 096, 147

**Category C — Trig simplification:** 12 functions
- 010-013, 049-050, 065, 069, 125-131

**Category D — String adaptation:** 12 functions
- 080, 085-094, 138

**Category E — New functions:** 2-3 functions
- `trunc()`, `frac()`, possibly `round()`

---

## 8. Compiler Output Format

### 8.1 EXEC.EXE Changes

The compiled bytecode file (`system/EXEC.EXE`) currently stores 32-bit
words. It changes to 64-bit words:

- `save_exec_bin()` in `compiler.c`: writes `int64_t` values from `mem[]`
- zlib compression still applies (compresses the 64-bit stream)
- The 602-byte DOS stub and 9-word header stay structurally the same
  (but header values are now 64-bit)

### 8.2 Compiler Memory

```c
// compiler_internal.h — before:
extern int32_t *mem_ory, *frm;
extern int32_t *mem, *loc;

// After:
extern int64_t *mem_ory, *frm;
extern int64_t *mem, *loc;
```

`g1()` and `g2()` emit 64-bit values. The peephole optimizer's ring
buffer (`struct peephole_entry`) uses int for opcodes, which stays the
same — opcodes are small numbers.

### 8.3 Debug Info Files

- `exec.lin` (line number mapping): format is source position → bytecode
  offset. Offsets are slot indices, not byte offsets, so the format doesn't
  change structurally.
- `exec.pgm` (source listing): plain text, no change.

---

## 9. Debugger Impact

The debugger (`src/runtime/debug/debugger*.c`) displays variable values
and allows inspection of process fields. Changes:

- Variable display: format 32.32 values as `integer.fraction` instead of
  plain integers. E.g., display `0x00000003_23D70A3D` as `3.14`.
- Process field display: same formatting.
- Memory view: show 64-bit slots.
- Breakpoint addresses: slot indices, unchanged.

---

## 10. `ltobj.def` Changes

### 10.1 Predefined Constants

Constants like `true`, `false`, screen resolutions, key codes, etc. are
defined as integer values. In 32.32 format they shift left 32:

```
// Before in ltobj.def:     stored as:
const true 1                → 0x00000001_00000000
const _320x200 320200       → shifted
```

The `preload_objects()` function in `compiler.c` reads these values and
stores them in the object table. It needs to shift constant values to
32.32 format during loading.

### 10.2 Predefined Variable Layouts

The global struct declarations (`mouse`, `scroll`, `m7`, `joy`, `setup`,
`dirinfo`, `fileinfo`, `video_modes`) define slot counts. Slot counts
don't change — each struct member is one slot, now 64-bit instead of
32-bit. The `end_struct` offset calculation in `inter.h` stays the same.

### 10.3 New Function Declarations

```
function 150 int trunc(0)       // Truncate to integer (discard fraction)
function 151 int frac(0)        // Return fractional part
function 152 int round(0)       // Round to nearest integer
```

Function numbers 150+ are free (current max is 149).

---

## 11. Sprint Plan

### Sprint 1: Foundation — Type Widening

**Goal:** `mem[]` and `stack[]` are 64-bit. The program compiles and runs
with integer-only code (no float literals yet). All values are `n << 32`.

Tasks:
1. `inter.h`: Change `mem`, `stack`, and key variable types to `int64_t`
2. `interpreter.c`: Update process creation (`<< 2` → `<< 3`), memory
   allocation (`* 4` → `* 8`), and process field access
3. `kernel.inc`: Widen all opcode implementations (type changes only,
   no logic changes yet)
4. `compiler_internal.h`: Change `mem`, `token_value`, output types
5. `compiler.c`: Update `save_exec_bin()`, `preload_objects()` (shift
   constants left 32)
6. `compiler_parser.c` / `compiler_expression.c`: Widen output buffer
   operations
7. Build and verify: existing integer-only programs compile and run
   with values stored as `n << 32`

**Verification:** Compile a simple DIV program (no floats), run it.
Process positions, collision, rendering all work correctly.

### Sprint 2: Arithmetic — 32.32 Math

**Goal:** Multiply and divide produce correct 32.32 results. Boolean
tests work.

Tasks:
1. `kernel.inc`: Implement `__int128` multiply for `lmul` and fused
   opcodes `lcarmul` (73)
2. `kernel.inc`: Implement `__int128` divide for `ldiv` and fused
   `lcardiv` (77)
3. `kernel.inc`: Implement `__int128` modulo for `lmod`
4. `kernel.inc`: Fix `ljpf` boolean test (`& 1` → `!= 0`)
5. `compiler_expression.c`: Update constant folding with 32.32 math
6. Test: `10 / 3` yields `3.333...`, `5 * 0.5` yields `2.5` (once
   literals work)

### Sprint 3: Lexer — Float Literals

**Goal:** The lexer parses `3.14`, `0.5`, `.001` as 32.32 constants.

Tasks:
1. `compiler.c` lexer: Add decimal point recognition in `l_num` case
2. Handle disambiguation with `.` struct-member operator
3. Hex literals: ensure `0xFF` shifts to 32.32
4. `token_value` widened to `int64_t` (may already be done in Sprint 1)
5. Test: `x = 3.14; y = x * 2;` compiles and `y` holds `6.28`

### Sprint 4: Bridge — Renderer and Collision

**Goal:** Sprites render at correct positions, collision works.

Tasks:
1. `render.c`: Add `INT_PART()` to all ~112 process field reads
2. `collision.c`: Add `INT_PART()` to all ~77 process field reads
3. `pathfind.c`: Add `INT_PART()` to ~7 reads
4. `interpreter.c`: Add `FROM_INT()` to all system state writes (mouse,
   keyboard, timers)
5. Verify: a DIV game with sprites, scrolling, and collision works

### Sprint 5: Built-in Functions

**Goal:** All 150 built-in functions work with 32.32 values.

Tasks:
1. Bridge functions (~75): Add INT_PART/FROM_INT at stack boundaries
2. Trig functions (7): Rewrite to use degree values and return actual
   results (eliminate millidegree and ×1000 conventions)
3. Related math functions (5): Update `get_angle`, `get_dist`, etc.
4. Add new functions: `trunc()`, `frac()`, `round()`
5. Update `itoa` and `write_int` for decimal display
6. Update `ltobj.def` with new function declarations

### Sprint 6: BYTE/WORD Removal

**Goal:** 34 opcodes and all byte/word type handling removed.

Tasks:
1. `kernel.inc`: Delete 17 byte opcode cases (78-94) and 17 word
   opcode cases (109-125)
2. `compiler_internal.h`: Remove `tbglo`, `twglo`, `tbloc`, `twloc`
   types and all byte/word opcode defines
3. `compiler_parser.c`: Replace `p_byte`/`p_word` declaration branches
   with passthrough to default type (~56 references)
4. `compiler_expression.c`: Remove byte/word codegen paths (~29 refs)
5. `inter.h`: Remove `memw` declaration and word opcode defines
6. `interpreter.c`: Remove `memw = (word *)mem` initialization
7. Keep `BYTE` and `WORD` keywords in `ltlex.def` (parse as no-ops)
8. Build and verify zero warnings

### Sprint 7: String Adaptation

**Goal:** String operations work correctly with 64-bit slots.

Tasks:
1. `kernel.inc`: Update ~8 `memb[]` spots in `lstrfix`/`lstrdec`/`lstrsub`
   (`* 4` → `* 8`)
2. `kernel.inc`: Update ~20 char-vs-pointer tests
   (`(unsigned)stack[sp] > 255` → `INT_PART(stack[sp]) > 255`)
3. `interpreter.c`: Update `nullstring[]` allocation and metadata
4. `functions.c`: Update string built-in functions (080, 085-094)
5. `compiler_parser.c`: Update string allocation formula
6. Test: string assignment, concatenation, comparison, `write()` display

### Sprint 8: Debugger and Polish

**Goal:** Debugger shows values correctly. Full integration test.

Tasks:
1. Debugger: Format 32.32 values for display
2. Debugger: Update variable inspection for 64-bit slots
3. Update `ltobj.def` trig function documentation (degrees, not
   millidegrees; actual values, not ×1000)
4. End-to-end test: compile and run multiple DIV example programs
5. Update documentation (`compiler-pipeline.md`, `vm-and-runtime.md`)

---

## 12. Risk Assessment

| Risk | Mitigation |
|------|------------|
| Missed `* 4` → `* 8` somewhere | Systematic search for `<< 2`, `* 4`, `/ 4` in VM files |
| Missed `INT_PART()` at bridge | Runtime will show garbled positions — visible immediately |
| `__int128` not available on target | GCC i386 supports it; fallback: manual 64×64→128 |
| Existing programs break (`10/3` semantics) | Accepted — old compiled .EXE files incompatible anyway |
| Performance regression | 64-bit ops on 32-bit are ~2× slower but DIV runs at 24 FPS with trivial workloads |
| String edge cases | Thorough testing of string concat, comparison, and the `> 255` threshold |
| Debugger crash on 64-bit values | Sprint 8 dedicated to debugger |

---

## 13. Verification Checklist

After each sprint, verify:

- [ ] All 4 targets build with zero warnings: `d`, `div`, `divrun`, `divdbg`
- [ ] Simple program compiles: `PROGRAM test; BEGIN LOOP FRAME; END END`
- [ ] Integer arithmetic: `x = 10; y = x + 3;` → y is 13
- [ ] Float arithmetic: `x = 3.14; y = x * 2;` → y is 6.28
- [ ] Division: `x = 10 / 3;` → x is 3.333...
- [ ] Comparison: `IF (x > 3.0) ... END` works
- [ ] Sprites render at correct screen positions
- [ ] Collision detection works
- [ ] Scrolling works
- [ ] Trig: `sin(90.0)` returns `1.0` (not `1000`)
- [ ] Strings: `strcpy(s, "hello"); strcat(s, " world");` works
- [ ] `write_int` displays fractional values
- [ ] BYTE/WORD keywords accepted silently (no errors, no special behavior)
- [ ] Debugger shows `3.14` not `0x0000000323D70A3D`
