# runtime/debug/decompiler.c - Commented Code Audit

This file has NO original DOS counterpart. It was written entirely by Mike (MikeDX) as an experimental bytecode-to-PRG decompiler for DIV2 executables. It is wrapped in `#ifndef __EMSCRIPTEN__` and appears to be a work-in-progress development tool, never integrated into the runtime proper.

| Metric | Value |
|--------|-------|
| Total blocks | 34 |
| Total commented-out lines | ~157 |
| Safe to remove | 34 blocks, ~157 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Line 20
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// getvarname %d iloc_len %d\n",i,iloc_len);` -- debug trace in `getvarname()`.
**Recommendation:** REMOVE

---

### Block 2: Lines 78-80
**Category:** DEBUG_LEFTOVER
**Summary:** Two commented-out fprintf debug traces and a default-name comment inside `getvarname()` (lines 78-80: `// default name` and `//fprintf(prg,"// getvarname %d iloc_len %d iloc %d\n",...)`).
**Recommendation:** REMOVE

---

### Block 3: Line 111
**Category:** DEBUG_LEFTOVER
**Summary:** `//if(i<mem[7]-36)` -- a conditional guard on `fprintf(prg,"LOCAL\n")` in `printlocal()`, commented out during development.
**Recommendation:** REMOVE

---

### Block 4: Line 119
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"BEGIN\n");` in `printlocal()` -- decompiler output that was disabled.
**Recommendation:** REMOVE

---

### Block 5: Lines 214-218
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out conditional and counter (`if(mem[i+1]<i)`, `printf("jump %d %d\n",...)`, `j++`) in the `ljmp` case of the first pass. Debug tracing and counter that were superseded.
**Recommendation:** REMOVE

---

### Block 6: Line 226
**Category:** DEBUG_LEFTOVER
**Summary:** `//f++;` in the `ljpf` backward-jump case. Unused counter increment.
**Recommendation:** REMOVE

---

### Block 7: Line 242
**Category:** DEBUG_LEFTOVER
**Summary:** `//printf("func %d\n",mem[i+1]);` in the `lfun` case. Debug trace.
**Recommendation:** REMOVE

---

### Block 8: Lines 266-268
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out default case with `printf("called func %d\n",mem[i]);` inside the function-dispatch switch.
**Recommendation:** REMOVE

---

### Block 9: Line 275
**Category:** DEBUG_LEFTOVER
**Summary:** `//printf("LASP: %d\n",i);` in the `lasp` case. Debug trace.
**Recommendation:** REMOVE

---

### Block 10: Line 322
**Category:** DEBUG_LEFTOVER
**Summary:** `//printf("%d:%5u\n",i,mem[i]);` in the main decompilation loop. Debug trace.
**Recommendation:** REMOVE

---

### Block 11: Line 339
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// i: %d jmp %d jpb %d jpf: %d\n",...)` in the jump-processing section. Debug trace.
**Recommendation:** REMOVE

---

### Block 12: Lines 349-355
**Category:** DEAD_CODE
**Summary:** A `/* ... */` block containing a `jpf` end-marker loop that was replaced by a different approach (the `end[]` array). The closing `* */` has extra space indicating hasty commenting.
**Recommendation:** REMOVE

---

### Block 13: Line 370
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// sp: %d %d\n",sp,mem[i+1]);` in the `lcar` case. Debug trace.
**Recommendation:** REMOVE

---

### Block 14: Line 373
**Category:** DEBUG_LEFTOVER
**Summary:** `//memset(cmd,0,255);` in the `lcar` case. Disabled buffer clear.
**Recommendation:** REMOVE

---

### Block 15: Line 375
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// GLOBAL VAR %s\n",cmd);` in the `lcar` case. Debug trace.
**Recommendation:** REMOVE

---

### Block 16: Lines 383-388
**Category:** DEBUG_LEFTOVER
**Summary:** Multiple commented-out fprintf/sprintf/fflush calls and a dangling `//}` in the `lasi` case. Development debugging artifacts.
**Recommendation:** REMOVE

---

### Block 17: Lines 389-429
**Category:** DEAD_CODE
**Summary:** A large `/* ... */` block containing the DIV local struct layout (process_id, id_scan, x, y, graph, etc.) as a reference comment for the decompiler author. This is a data-layout reference, but it duplicates documentation available in the compiler source and adds 40 lines of noise.
**Recommendation:** REMOVE

---

### Block 18: Lines 430-435
**Category:** DEBUG_LEFTOVER
**Summary:** Six commented-out lines of variable-name resolution attempts (`getvarname`, `memset`, `fprintf`, `fflush`) in the `lasi` handler.
**Recommendation:** REMOVE

---

### Block 19: Lines 437-439
**Category:** DEBUG_LEFTOVER
**Summary:** Three commented-out lines: `localvar=locvar[sp-2]`, `getvarname(...)`, and a blank `//`. Superseded approach to variable naming.
**Recommendation:** REMOVE

---

### Block 20: Line 448
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// UNIMP! || (ORI)\n");` -- "unimplemented" marker that was resolved.
**Recommendation:** REMOVE

---

### Block 21: Lines 453-454
**Category:** DEBUG_LEFTOVER
**Summary:** `//sp--;` and `//fprintf(prg,"// cmd: %s\n",...)` in the `lori` case. Stack adjustment and debug trace that were superseded.
**Recommendation:** REMOVE

---

### Block 22: Line 467
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// UNIMP! && (AND)\n");` -- resolved "unimplemented" marker.
**Recommendation:** REMOVE

---

### Block 23: Line 472
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// condstack %s\n",condstack[con]);` in the `land` case. Debug trace.
**Recommendation:** REMOVE

---

### Block 24: Lines 477-478
**Category:** DEBUG_LEFTOVER
**Summary:** Two commented-out fprintf debug traces in the `ligu` (==) case.
**Recommendation:** REMOVE

---

### Block 25: Lines 485-486
**Category:** DEBUG_LEFTOVER
**Summary:** Two commented-out fprintf debug traces in the `ldis` (!=) case.
**Recommendation:** REMOVE

---

### Block 26: Lines 494, 498
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out "UNIMP! >" marker and stack trace in the `lmay` (>) case.
**Recommendation:** REMOVE

---

### Block 27: Lines 502-503, 511-512, 520-521
**Category:** DEBUG_LEFTOVER
**Summary:** Pairs of commented-out "UNIMP!" markers and stack traces in the `lmen` (<), `lmei` (<=), and `lmai` (>=) cases. Same pattern as Block 26.
**Recommendation:** REMOVE

---

### Block 28: Line 529
**Category:** DEBUG_LEFTOVER
**Summary:** `//fprintf(prg,"// %s add %s\n",...)` in the `ladd` case. Debug trace.
**Recommendation:** REMOVE

---

### Block 29: Lines 530-538
**Category:** DEAD_CODE
**Summary:** `/* ... */` block with an alternative approach to building add expressions using `getvarname`/`sprintf`. Superseded by the `cstack[]` string approach.
**Recommendation:** REMOVE

---

### Block 30: Lines 543-545
**Category:** DEBUG_LEFTOVER
**Summary:** Three commented-out lines (`sp--`, `sprintf`, `strcpy`) in the `ladd` case. Superseded stack manipulation.
**Recommendation:** REMOVE

---

### Block 31: Lines 550-561
**Category:** DEAD_CODE
**Summary:** A `//fprintf` line followed by a `/* ... */` block (11 lines) in the `lsub` case. Same pattern as Block 29 -- an alternative `getvarname`/`sprintf` approach that was superseded.
**Recommendation:** REMOVE

---

### Block 32: Lines 572, 652, 667, 682-685, and scattered throughout cases 700-960
**Category:** DEBUG_LEFTOVER
**Summary:** Approximately 20 additional single-line `//fprintf(prg,...)` and `//sp--` and `//sp=0` and `//con++` comments scattered throughout the function dispatch cases (`lmul`, `ljpf`, `lfun`, `lcal`, `lasp`, `lcbp`, `lcpa`, `ltyp`, `lpri`, `lpar`). All are debug traces or superseded stack adjustments.
**Recommendation:** REMOVE

---

### Block 33: Lines 1101-1112
**Category:** DEAD_CODE
**Summary:** A `/* ... */` block (10 lines) containing a partial `switch(mem[i+2])` for the `lchk` case that only handled `case 28:` (angle). Was replaced by the generic `getvarname(mem[i+2],name)` approach above it. Followed by two `//` lines with a superseded `i++` and debug fprintf.
**Recommendation:** REMOVE

---

### Block 34: Lines 1131-1201
**Category:** INCOMPLETE_PORT
**Summary:** A large `/* ... */` block (71 lines) containing case handlers for extended VM opcodes: combined instructions (`lcar2`, `lcar3`, `lcar4`, `lasiasp`, `lcaraid`, etc.), word-width operations (`lptrwor`, `lasiwor`, etc.), char-width operations (`lptrchr`, `lasichr`, etc.), and string operations (`lstrcpy`, `lstrcat`, `lstrlen`, etc.). These only print to the `sta` dump file (no decompilation logic). They are commented out, likely because the corresponding opcode constants were not defined or the decompiler was not extended to handle them.
**Recommendation:** REMOVE -- These are stub handlers that only do `fprintf(sta,...)` for a hex dump, with no decompilation logic. If the decompiler is ever completed, these would need to be rewritten with proper stack/PRG output anyway. The opcode names serve as documentation of the VM instruction set, but that information is better maintained in the interpreter source (`i.c`).

---

## Summary

This entire file is an incomplete, experimental bytecode decompiler written by Mike. Every single commented-out block is either a debug trace (`fprintf`/`printf` for development), a superseded implementation approach, or stub code for unimplemented features. The file has zero overlap with the original DOS codebase. All 34 blocks (~157 lines) are safe to remove, which would significantly improve readability of this already dense and hard-to-follow decompiler.
