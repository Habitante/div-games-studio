# Commented-Out Code Audit: divsetup.c

**Source:** `c:\Src\div\src\divsetup.c` (1246 lines)
**Original:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVSETUP.CPP` (1073 lines)
**Auditor:** Claude Opus 4.6
**Date:** 2026-03-07

## Summary

| # | Lines | Classification | Action | Description |
|---|-------|---------------|--------|-------------|
| 1 | 115-120 | MIKE_REMOVED | REMOVE | Vid_Setup3 commented-out SDL/sound code |
| 2 | 123 | MIKE_ADDED_DEAD | REMOVE | SDL_putenv call |
| 3 | 381-386 | MIKE_ADDED_DEAD | REMOVE | NOTYET: sysconf memory detection |
| 4 | 388-410 | MIKE_ADDED_DEAD | REMOVE | Linux /proc/meminfo MemFree (unreachable) |
| 5 | 412-434 | MIKE_ADDED_DEAD | REMOVE | NOTYET: Linux /proc/meminfo MemTotal |
| 6 | 436-449 | MIKE_ADDED_DEAD | REMOVE | NOTYET: DOS _heapwalk implementation |
| 7 | 471 | MIKE_ADDED_DEAD | REMOVE | Error handler comment stub |
| 8 | 492-500 | ORIGINAL_DEAD | REMOVE | DOS DPMI int386x call in GetFreeMem |
| 9 | 545-547 | MIKE_ADDED_DEAD | REMOVE | Commented memory formatting code |
| 10 | 577 | ORIGINAL_DEAD | REMOVE | _heapshrink call in MemInfo0 |
| 11 | 604-606 | ORIGINAL_DEAD | REMOVE | Old wbox calls in Cfg_colors (color_cfg 9-11) |
| 12 | 643-648 | ORIGINAL_DEAD | REMOVE | Old wrectangulo/wwrite in Cfg_Setup1 |
| 13 | 756-758 | ORIGINAL_DEAD | REMOVE | Old wmouse_in calls in Cfg_Setup2 |

**Totals:** 13 blocks found, 0 REVIEW, 13 REMOVE, 0 KEEP

## Block Details

### Block 1 (lines 115-120) - MIKE_REMOVED - REMOVE
```c
//    vvga_an = VS_ANCHO;
//	vvga_al = VS_ALTO;
//	EndSound();
//	soundstopped=1;
```
In `Vid_Setup3`. The original DOS code did not have these lines. Mike added them during SDL2 port (they reference `vvga_an`, `vvga_al`, `EndSound`, `soundstopped` which are SDL2-era additions), then commented them out. The function now sets `salir_del_entorno=1` to force a restart instead.

### Block 2 (line 123) - MIKE_ADDED_DEAD - REMOVE
```c
//	SDL_putenv("SDL_VIDEO_WINDOW_POS=center");
```
SDL1-era API call. Not in original DOS source. Dead code.

### Block 3 (lines 381-386) - MIKE_ADDED_DEAD - REMOVE
```c
#ifdef NOTYET
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size * 1000;
#endif
```
Mike's Linux memory detection attempt using sysconf. Guarded by `NOTYET` (never defined). The function `Mem_GetHeapFree` now just `return 0;` on line 380. The original DOS version used `_heapwalk`.

### Block 4 (lines 388-410) - MIKE_ADDED_DEAD - REMOVE
```c
#ifndef WIN32
    FILE *meminfo = fopen("/proc/meminfo", "r");
    ...
    // reads MemFree
#endif
```
Linux /proc/meminfo reader. Unreachable because `return 0;` on line 380 returns before this code. Dead code.

### Block 5 (lines 412-434) - MIKE_ADDED_DEAD - REMOVE
```c
#ifdef NOTYET
    FILE *meminfo = fopen("/proc/meminfo", "r");
    ...
    // reads MemTotal
#endif
```
Another NOTYET-guarded Linux memory reader variant. Dead code.

### Block 6 (lines 436-449) - MIKE_ADDED_DEAD - REMOVE
```c
#ifdef NOTYET
  struct _heapinfo miheap;
  int status=0,total=0;
  miheap._pentry=NULL;
  _heapmin();
  ...
#endif
```
NOTYET-guarded copy of the original DOS `_heapwalk` code. Mike preserved it inside an `#ifdef NOTYET` block. The original DOS function body is now dead.

### Block 7 (line 471) - MIKE_ADDED_DEAD - REMOVE
```c
//        ... // handle error
```
Stub comment where error handling should go. Not real code but noise.

### Block 8 (lines 492-500) - ORIGINAL_DEAD - REMOVE
```c
#ifdef NOTYET
  union REGS regs;
  struct SREGS sregs;
  regs.x.eax=0x0500;
  memset( &sregs, 0 , sizeof(sregs) );
  sregs.es  =FP_SEG(Meminfo);
  regs.x.edi=FP_OFF(Meminfo);
  int386x(0x031,&regs,&regs,&sregs);
#endif
```
The original DOS `GetFreeMem` function body (DPMI interrupt 0x31, function 0x0500). Mike wrapped it in `#ifdef NOTYET` and replaced it with the Win32/Linux implementations above. The original code is dead.

### Block 9 (lines 545-547) - MIKE_ADDED_DEAD - REMOVE
```c
//	fmem=fmem*1000;
//    if(fmem/1000) sprintf(cWork,(char *)texto[194],fmem,sizes[csize]);
   // else
```
In `MemInfo1`. Mike's attempt at formatting memory sizes with the new float-based display. Partially commented out, partially rewritten. The active `sprintf` on line 548 replaced this.

### Block 10 (line 577) - ORIGINAL_DEAD - REMOVE
```c
//  _heapshrink();
```
In `MemInfo0`. The original had `_heapshrink();` (a Watcom C runtime call). Mike commented it out since it doesn't exist in GCC. Safe to remove.

### Block 11 (lines 604-606) - ORIGINAL_DEAD - REMOVE
```c
//  wbox(v.ptr,an,al,color_cfg[9],13,45+20,7,7);
//  wbox(v.ptr,an,al,color_cfg[10],53+25,45+20,7,7);
//  wbox(v.ptr,an,al,color_cfg[11],93+34,45+20,7,7);
```
In `Cfg_colors`. These are the original 3 wbox calls for zones 9-11, commented out **in both the current file and the original DOS source**. Daniel commented these out in the original to rearrange the color picker layout (skipping index 9, using 10 and 11 at the zone-10/11 positions). Already dead in original.

### Block 12 (lines 643-648) - ORIGINAL_DEAD - REMOVE
```c
//  wrectangulo(v.ptr,an,al,c0,12,44+20,9,9);
//  wwrite(v.ptr,an,al,23,45+20,0,texto[408],c3);
//  wrectangulo(v.ptr,an,al,c0,52+25,44+20,9,9);
//  wwrite(v.ptr,an,al,63+25,45+20,0,texto[409],c3);
//  wrectangulo(v.ptr,an,al,c0,92+34,44+20,9,9);
//  wwrite(v.ptr,an,al,103+34,45+20,0,texto[410],c3);
```
In `Cfg_Setup1`. Same pattern as block 11 -- the original 6 draw calls for the 3rd row of color config. Already commented out in the original DOS source. Replaced by the 4 draw calls for zones 11-12 immediately below.

### Block 13 (lines 756-758) - ORIGINAL_DEAD - REMOVE
```c
//  if (wmouse_in(12,44+20,9,9)) zona=10;
//  if (wmouse_in(52+25,44+20,9,9)) zona=11;
//  if (wmouse_in(92+34,44+20,9,9)) zona=12;
```
In `Cfg_Setup2`. Same UI rearrangement as blocks 11-12. Already commented out in the original DOS source. The active code below uses zona=11 and zona=12 for the two remaining color pickers.

## Notes

The `Mem_GetHeapFree` function (lines 378-451) is a mess of layered dead code. The function has `return 0;` at line 380, making everything below it unreachable. Then it has multiple `#ifdef NOTYET` / `#ifndef WIN32` blocks that are also dead. The whole function could be replaced with `int Mem_GetHeapFree() { return 0; }` and all the dead code removed.

Similarly, `GetFreeMem` (lines 456-501) has two implementations (#ifdef WIN32 / #else) that are both Mike's work, plus the original DOS DPMI code in `#ifdef NOTYET`. The NOTYET block should be removed.
