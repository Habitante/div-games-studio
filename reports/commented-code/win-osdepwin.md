# win/osdepwin.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 17 |
| Total commented-out lines | ~68 |
| Safe to remove | 15 blocks, ~56 lines |
| Needs review | 1 block, ~3 lines |
| Keep | 1 block |

## Blocks

### Block 1: Line 6
**Category:** DEAD_CODE
**Summary:** `#include <fnmatch.h>` commented out. This header is not available on Windows (MinGW) and is unused since osdepwin.c uses `_findfirst`/`_findnext` instead of `scandir`+`fnmatch`.
**Recommendation:** REMOVE

---

### Block 2: Line 16
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out `assert(strcmp(opentype, "r") == 0)` inside `fmemopen()`. A debug assertion that was never enabled.
**Recommendation:** REMOVE

---

### Block 3: Lines 76, 84
**Category:** DEBUG_LEFTOVER
**Summary:** Two commented-out `printf` debug traces in `_dos_setdrive()` for drive-change diagnostics.
**Recommendation:** REMOVE

---

### Block 4: Line 92
**Category:** DEAD_CODE
**Summary:** Commented-out `int len = ...floor(log10l(abs(n)))+1` inside the `#ifdef NOYET` block of `itoa()`. An earlier algorithm for computing digit count, replaced by the `len` parameter.
**Recommendation:** REMOVE

---

### Block 5: Line 95
**Category:** DEAD_CODE
**Summary:** Commented-out `char *buf = calloc(...)` inside the `#ifdef NOYET` block of `itoa()`. Earlier version allocated its own buffer; now takes buffer as parameter.
**Recommendation:** REMOVE

---

### Block 6: Line 134
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out `printf("Input %s\n",Path)` inside `_splitpath()`.
**Recommendation:** REMOVE

---

### Block 7: Lines 184, 186
**Category:** DEBUG_LEFTOVER
**Summary:** Line 184 is a truncated comment `// Filename[` (incomplete expression). Line 186 is `printf("Extension is %s\n",...)`. Both are debug leftovers in `_splitpath()`.
**Recommendation:** REMOVE

---

### Block 8: Line 241
**Category:** DEBUG_LEFTOVER
**Summary:** Commented-out `printf("Chdir %s\n",Directory)` inside `_chdir()`.
**Recommendation:** REMOVE

---

### Block 9: Lines 250
**Category:** INCOMPLETE_PORT
**Summary:** `realpath(_Path, _FullPath)` commented out in `_fullpath()`, replaced with `strcpy`. The function currently does no path resolution -- just copies the input verbatim.
**Recommendation:** REVIEW -- `realpath()` is available on MinGW and would provide correct absolute-path resolution. The current `strcpy` fallback is a known shortcut.

---

### Block 10: Line 266
**Category:** DEAD_CODE
**Summary:** Commented-out `char findname[_MAX_PATH]` declaration, superseded by the `findname[2048]` on line 263.
**Recommendation:** REMOVE

---

### Block 11: Lines 272, 289-291, 295, 299-300
**Category:** DEBUG_LEFTOVER
**Summary:** Multiple commented-out `printf` debug traces and an old `scandir` call inside `_dos_findfirst()`. Part of the iterative porting from POSIX to Win32 `_findfirst` API.
**Recommendation:** REMOVE

---

### Block 12: Lines 303-331 (dead code after `return`)
**Category:** DEAD_CODE
**Summary:** ~29 lines of unreachable code after `return (ret)` on line 297 in `_dos_findfirst()`. Contains a second complete implementation attempt using `scandir`/`namelist` that was superseded by the `_findfirst` approach above. Includes commented-out printfs, an old `strcpy(findmask,...)` path, and cleanup logic. All dead -- execution never reaches past line 297.
**Recommendation:** REMOVE

---

### Block 13: Lines 333-341
**Category:** DEAD_CODE
**Summary:** A `/* ... */` block comment containing an old `_dos_findfirst` implementation that populated `result->attrib` and `result->name` from `namelist[0]`. Superseded by the current `_findnext`-based approach.
**Recommendation:** REMOVE

---

### Block 14: Lines 344, 357, 367, 379, 386, 394, 408, 413, 426, 448-451, 454
**Category:** DEBUG_LEFTOVER
**Summary:** Scattered commented-out `printf` debug traces and commented-out `return` statements throughout `_dos_findnext()`. All are development-time debug prints.
**Recommendation:** REMOVE

---

### Block 15: Lines 427-447
**Category:** DEAD_CODE
**Summary:** A `/* ... */` block containing an abandoned `fnmatch`-based matching loop in `_dos_findnext()`. This was an earlier POSIX approach replaced by the current string comparison logic.
**Recommendation:** REMOVE

---

### Block 16: Lines 457-486
**Category:** DEAD_CODE
**Summary:** `#ifdef NOTYET` block containing an entire alternate `_dos_findnext` tail using POSIX `scandir`/`fnmatch`/`DT_DIR`. This is the Linux implementation path, unreachable on Windows. Includes commented-out debug `printf`s and `free(namelist)` lines within it.
**Recommendation:** REMOVE

---

### Block 17: Line 491
**Category:** LEGITIMATE
**Summary:** `// printf("TODO - setfileattr\n")` is a TODO reminder noting that `_dos_setfileattr` is a stub. Serves as documentation of incomplete functionality.
**Recommendation:** KEEP

---

## Review Items

### Block 9 (Line 250): `_fullpath` using `strcpy` instead of `realpath`
The function `_fullpath()` currently just copies the input path with no resolution. The commented-out `realpath()` call would provide correct behavior. On MinGW32, `_fullpath()` is actually available as a CRT function, so this entire wrapper may be unnecessary on the Windows build (it is already guarded by `#if !defined(__MINGW32__)`). Verify whether this code path is even compiled for the Windows target before investing effort here.
