# Commented-Out Code Audit: divtimer.c

**Source:** `c:\Src\div\src\divtimer.c` (93 lines)
**Original:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVTIMER.CPP` (79 lines)
**Auditor:** Claude Opus 4.6
**Date:** 2026-03-07

## Summary

| # | Lines | Classification | Action | Description |
|---|-------|---------------|--------|-------------|
| 1 | 10-13 | MIKE_REMOVED | REMOVE | DOS includes: conio.h, dos.h, mem.h, judasmem.h |
| 2 | 22-29 | MIKE_REMOVED | REMOVE | DOS-specific `#pragma aux` for timer_get_ds |
| 3 | 35 | MIKE_REMOVED | REMOVE | Commented-out `extern unsigned timer_count` |
| 4 | 44-60 | MIKE_REMOVED | REMOVE | timer_init DOS body in NOTYET block |
| 5 | 66-77 | MIKE_REMOVED | REMOVE | timer_uninit DOS body in NOTYET block |
| 6 | 81-83 | MIKE_REMOVED | REMOVE | timer_lock DOS body in NOTYET block |
| 7 | 89-91 | MIKE_REMOVED | REMOVE | timer_unlock DOS body in NOTYET block |

**Totals:** 7 blocks found, 0 REVIEW, 7 REMOVE, 0 KEEP

## Block Details

### Block 1 (lines 10-13) - MIKE_REMOVED - REMOVE
```c
//#include <conio.h>
//#include <dos.h>
//#include <mem.h>
//#include "judas\judasmem.h"
```
Original DOS includes. `conio.h`, `dos.h`, and `mem.h` are Watcom/DOS-specific headers. `judasmem.h` is the Judas sound library memory management header. All four were active in the original and commented out by Mike.

### Block 2 (lines 22-29) - MIKE_REMOVED - REMOVE
```c
#ifdef DOS

#pragma aux timer_get_ds = \
"mov ax, ds" \
modify [ax] \
value [ax];

#endif
```
Watcom C inline assembly pragma to read the DS segment register. The original had this without the `#ifdef DOS` guard. Mike wrapped it in `#ifdef DOS` which is never defined, making it dead. The function `timer_get_ds` is declared but never called in the SDL2 build.

### Block 3 (line 35) - MIKE_REMOVED - REMOVE
```c
//extern unsigned timer_count;
```
Original had `extern unsigned timer_count;` as active code. Mike commented it out. The variable is only used inside the `#ifdef NOTYET` blocks.

### Block 4 (lines 44-60) - MIKE_REMOVED - REMOVE
```c
#ifdef NOTYET
	if (timer_initialized) return 1;
        if (!timer_lock()) return 0;
        timer_function = function;
        timer_count = 0;
        ...
        timer_initialized = 1;
#endif
```
The entire body of `timer_init` from the original DOS source. Programs the PIT (Programmable Interval Timer) chip via I/O ports 0x40/0x43 and hooks interrupt 8. Mike wrapped it in `#ifdef NOTYET`. The function now just `return 1;`.

### Block 5 (lines 66-77) - MIKE_REMOVED - REMOVE
```c
#ifdef NOTYET
	if (!timer_initialized) return;
        _disable();
        _dos_setvect(8, timer_oldvect);
        ...
        timer_initialized = 0;
#endif
```
The entire body of `timer_uninit` from the original. Restores interrupt 8 and resets PIT. Mike wrapped in `#ifdef NOTYET`. The function is now empty.

### Block 6 (lines 81-83) - MIKE_REMOVED - REMOVE
```c
#ifdef NOTYET
        if (!judas_memlock(&timer_code_lock_start, ...)) return 0;
#endif
```
Body of `timer_lock` from original. Locks timer code pages in memory (DOS DPMI requirement). Wrapped in `#ifdef NOTYET`.

### Block 7 (lines 89-91) - MIKE_REMOVED - REMOVE
```c
#ifdef NOTYET
        judas_memunlock(&timer_code_lock_start, ...);
#endif
```
Body of `timer_unlock` from original. Unlocks timer code pages. Wrapped in `#ifdef NOTYET`.

## Notes

This entire file is effectively dead. All four functions (`timer_init`, `timer_uninit`, `timer_lock`, `timer_unlock`) have empty bodies (or just `return 1`). The SDL2 port doesn't use PIT timer interrupts -- SDL handles timing internally.

The file should probably be considered for removal entirely in a future cleanup pass. The `timer_handler` function is defined as an empty `void timer_handler(void) {}` on line 19 and may still be referenced somewhere, but the timer infrastructure itself is fully dead.
