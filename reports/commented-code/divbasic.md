# Commented-Out Code Audit: divbasic.c

**Source:** `c:\Src\div\src\divbasic.c` (1145 lines)
**Original:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVBASIC.CPP` (1130 lines)
**Auditor:** Claude Opus 4.6
**Date:** 2026-03-07

## Summary

| # | Lines | Classification | Action | Description |
|---|-------|---------------|--------|-------------|

**Totals:** 0 blocks found

## Analysis

This file contains **zero** commented-out code blocks. Every line of code is active.

Comparing current vs. original:
- The file has been lightly edited for the SDL2 port (e.g., `int32_t` instead of `int` for variable `n` in `zoom_map`, `memptrsize` cast instead of `(int)` cast in `draw_selection_box`, `byte *` return type for `save_undo` instead of `int`, `(char *)texto` casts added).
- Comments like `// OJO !!!` on line 727 and `// Repone el fondo...` on line 188 are legitimate documentation comments, not dead code.
- The `// OJO` comment on line 728 (`// p=copia+...`) is a note showing a formula that was considered but not used -- it's a legitimate explanatory comment present in the original source.

No action required.
