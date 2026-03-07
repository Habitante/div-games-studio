# shared/osdep.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 13 |
| Total commented-out lines | ~34 |
| Safe to remove | 12 blocks, ~31 lines |
| Needs review | 1 block, ~3 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 34, 40
**Category:** DEBUG_LEFTOVER
**Summary:** Two commented-out `printf` debug traces in `_strupr()` -- one prints the input string, the other prints the uppercased result.
**Recommendation:** REMOVE

---

### Block 2: Line 72
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out `printf("lower'd string: [%s]\n",st)` in `_strlwr()`.
**Recommendation:** REMOVE

---

### Block 3: Lines 83, 89, 91
**Category:** DEBUG_LEFTOVER
**Summary:** Two commented-out `printf` debug traces in `_dos_setdrive()` and a commented-out `chdir(c)` call. The `chdir` was disabled (the Linux version has no drive concept), while the prints are debug leftovers.
**Recommendation:** REMOVE

---

### Block 4: Line 99
**Category:** DEAD_CODE
**Summary:** Commented-out `int len = ...floor(log10l(abs(n)))+1` in `itoa()`. Same as the Windows counterpart -- earlier algorithm for computing buffer length, replaced by the `len` parameter.
**Recommendation:** REMOVE

---

### Block 5: Line 102
**Category:** DEAD_CODE
**Summary:** Commented-out `char *buf = calloc(...)` in `itoa()`. Earlier version allocated its own buffer; now takes buffer as parameter.
**Recommendation:** REMOVE

---

### Block 6: Line 140
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out `printf("Input %s\n",Path)` in `_splitpath()`.
**Recommendation:** REMOVE

---

### Block 7: Lines 190, 192
**Category:** DEBUG_LEFTOVER
**Summary:** Line 190 is truncated `// Filename[` (incomplete expression). Line 192 is `printf("Extension is %s\n",...)`. Debug leftovers in `_splitpath()`.
**Recommendation:** REMOVE

---

### Block 8: Line 247
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out `printf("Chdir %s\n",Directory)` in `_chdir()`.
**Recommendation:** REMOVE

---

### Block 9: Line 256
**Category:** INCOMPLETE_PORT
**Summary:** `realpath(_Path, _FullPath)` commented out in `_fullpath()`, replaced with a plain `strcpy`. The function does no actual path resolution.
**Recommendation:** REVIEW -- `realpath()` is available on Linux/macOS and would provide correct absolute-path resolution. The `strcpy` fallback is a known shortcut from the porting effort.

---

### Block 10: Lines 272, 277, 285, 299, 302
**Category:** DEBUG_LEFTOVER
**Summary:** Multiple commented-out `printf` debug traces and a `// int n;` in `_dos_findfirst()`. Development-time diagnostics.
**Recommendation:** REMOVE

---

### Block 11: Lines 307-315
**Category:** DEAD_CODE
**Summary:** A `/* ... */` block comment containing an old `_dos_findfirst` implementation that returned result from `namelist[0]`. Superseded by the current `scandir` + `_dos_findnext` approach.
**Recommendation:** REMOVE

---

### Block 12: Lines 318, 334, 336, 341, 348, 352-353, 358
**Category:** DEBUG_LEFTOVER
**Summary:** Scattered commented-out lines in `_dos_findnext()`: debug `printf`s (`TODO - findnext`, `Matching`, `free'ing`, `not matched`), a disabled `strlwr(findname)` call, and `free(namelist)/namelist=NULL` cleanup that was disabled. All are iterative-development leftovers.
**Recommendation:** REMOVE

---

### Block 13: Line 358
**Category:** DEBUG_LEFTOVER
**Summary:** `// printf("TODO - setfileattr\n")` in `_dos_setfileattr()`. A stub reminder, but less useful as documentation than the one in osdepwin.c since this file also has the same stub pattern.
**Recommendation:** REMOVE

---

## Review Items

### Block 9 (Line 256): `_fullpath` using `strcpy` instead of `realpath`
Same situation as the Windows counterpart in osdepwin.c. The `realpath()` call is commented out and replaced with `strcpy()`. On Linux/macOS this function should use `realpath()` for correct behavior. Consider whether this code path is even reached when building for the Windows target (the Windows build uses osdepwin.c instead).
