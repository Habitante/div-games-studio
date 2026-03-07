# inter.h - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 7 |
| Total commented-out lines | ~36 |
| Safe to remove | 5 blocks, ~18 lines |
| Needs review | 0 blocks |
| Keep | 2 blocks, ~18 lines |

## Blocks

### Block 1: Lines 44-47
**Category:** MIKE_REMOVED
**Summary:** Four DOS-only headers (`<bios.h>`, `<dos.h>`, `<i86.h>`, `<graph.h>`) that were active in the original INTER.H (lines 26-29). Mike commented them out because they don't exist on non-DOS platforms.
**Recommendation:** REMOVE -- These are DOS-only Watcom C headers with no SDL2 equivalents. They will never be used again.

---

### Block 2: Lines 57-58
**Category:** MIKE_REMOVED
**Summary:** Two Judas sound library includes (`"..\judas\judas.h"` and `"..\judas\timer.h"`) that were active in the original (lines 37-38). Mike replaced Judas with SDL_mixer; `divsound.h` on line 59 provides the replacement interface.
**Recommendation:** REMOVE -- Judas sound library is fully replaced by SDL2_mixer. These headers don't exist in the tree.

---

### Block 3: Line 156
**Category:** ORIGINAL_DEAD
**Summary:** `//void call(int *(*func)() );` -- an old prototype for the assembly `call()` function. Already commented out in the original INTER.H (it was never there in active form; the active prototype was `void call(int);` at line 127 of the original). Mike replaced it with the `voidReturnType` version on line 40 of the current file.
**Recommendation:** REMOVE -- Superseded by the active declaration on line 40.

---

### Block 4: Lines 755-758
**Category:** ORIGINAL_DEAD
**Summary:** Four comment lines documenting usage of the `tfast` table for scroll optimization. Identical to the original INTER.H lines 721-724. These are documentation comments explaining how to use the struct above them, but they contain code-like syntax (`malloc()`, `fast[n].nt`).
**Recommendation:** KEEP -- These are explanatory API-usage comments for the `tfast` struct, not dead code. They document the intended usage pattern.

---

### Block 5: Lines 764-773
**Category:** ORIGINAL_DEAD
**Summary:** Ten lines of commented-out `GLOBAL` variable declarations for old scroll variables. Identical to the original INTER.H lines 730-739. These were the pre-struct scroll variables; Daniel commented them out when he moved scroll state into `struct _iscroll`.
**Recommendation:** REMOVE -- These old global declarations were superseded by the `_iscroll` struct. Already dead in the original.

---

### Block 6: Lines 827-840
**Category:** LEGITIMATE
**Summary:** Fourteen-line comment documenting the FPG file format (header structure, field sizes, data layout). Present identically in the original (lines 791-804). Contains `=` signs and field names that look code-like but this is purely a file-format specification comment.
**Recommendation:** KEEP -- This is valuable documentation of the binary FPG format, not dead code.

---

### Block 7: Line 967
**Category:** MIKE_REMOVED
**Summary:** `//void find_color(int r,int g,int b);` -- prototype was active in the original (line 930: `void find_color(int r,int g,int b);`). The function still exists but its prototype is now declared elsewhere (line 111 declares `void find_color(byte,byte,byte);` with byte params instead of int).
**Recommendation:** REMOVE -- The active prototype with `byte` parameters is on line 111. This `int`-parameter version is dead and the signature mismatch could cause confusion.
