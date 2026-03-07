# Commented-Out Code Audit: det_vesa.c

**Source:** `c:\Src\div\src\det_vesa.c` (62 lines)
**Original:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DET_VESA.CPP` (79 lines)
**Auditor:** Claude Opus 4.6
**Date:** 2026-03-07

## Summary

| # | Lines | Classification | Action | Description |
|---|-------|---------------|--------|-------------|
| 1 | 6-16 | ORIGINAL_DEAD | REMOVE | Old struct _modos / globals declaration |

**Totals:** 1 block found, 0 REVIEW, 1 REMOVE, 0 KEEP

## Block Details

### Block 1 (lines 6-16) - ORIGINAL_DEAD - REMOVE
```c
/*
struct _modos {
  short ancho;
  short alto;
  short modo;
};
struct _modos modos[32];
int num_modos;
int VersionVesa;
char marcavga[128];
*/
```
This struct and global variable block is commented out in **both** the current file and the original DOS source. In the original, they are declared in `global.h` instead. The comment block is identical in both files (the original also has `/* ... */` around the same declarations, preceded by the same `#include` of `global.h`).

Note: The original file also had a leading `/* #include <stdio.h> ... */` block (lines 1-5) which Mike removed entirely rather than leaving commented out. The `#include "inc\vesa.h"` from the original was also removed (VESA detection replaced by SDL mode listing).

## Notes

This file was almost entirely rewritten by Mike for SDL2. The original used DPMI/VESA BIOS calls (`vbeInit()`, `vbeGetModeInfo()`, `VbeInfoBlock`, `ModeInfoBlock`) to enumerate 8-bit video modes. Mike replaced it with `OSDEP_ListModes()` which uses SDL's mode enumeration. The only commented-out code that survived is the struct/globals block which was already dead in the original.

The current implementation has a minor issue: when `modes == 0 || modes == -1`, it hardcodes 8 modes but sets `num_modos=0` (never updated for the hardcoded list), which means `qsort` at the end sorts 0 elements. However, this is a separate bug, not a commented-out code issue.
