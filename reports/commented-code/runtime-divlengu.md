# runtime/divlengu.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 3 |
| Total commented-out lines | ~8 |
| Safe to remove | 3 blocks, ~8 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 31, 34
**Category:** ORIGINAL_DEAD
**Summary:** `//#ifdef DEBUG` and `//#endif` around the `FILE * f; int n;` variable declarations inside `inicializa_textos()`. Already commented out in the original DOS source (DIVLENGU.CPP lines 20, 23). The variables are needed in both DEBUG and non-DEBUG paths, so the `#ifdef` was disabled to avoid a compile error.
**Recommendation:** REMOVE (the `//` prefixes on these two lines serve no purpose; the declarations are already unconditional)

---

### Block 2: Line 56
**Category:** MIKE_ADDED_DEAD
**Summary:** `//DebugData(n);` -- In the original DOS source this was an active call `DebugData(n);` (DIVLENGU.CPP line 45). Mike commented it out during the SDL2 port, likely because the DebugData function was unavailable or irrelevant. The line is inside the `#else` (non-DEBUG) branch of `inicializa_textos()`.
**Recommendation:** REMOVE (DebugData is a DOS-era debug-print macro with no SDL2 equivalent)

---

### Block 3: Lines 66-70
**Category:** ORIGINAL_DEAD
**Summary:** Alternative text initialization that loads from a compiled-in `textosing` array instead of a file. Commented out in the original DOS source (DIVLENGU.CPP lines 55-59). This was a development shortcut that was replaced by file-based loading.
**Recommendation:** REMOVE

---
