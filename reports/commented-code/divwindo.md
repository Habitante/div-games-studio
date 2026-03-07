# Commented-Out Code Audit: divwindo.c

**Source:** `c:\Src\div\src\divwindo.c` (787 lines)
**Original:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVWINDO.CPP` (539 lines)
**Auditor:** Claude Opus 4.6
**Date:** 2026-03-07

## Summary

| # | Lines | Classification | Action | Description |
|---|-------|---------------|--------|-------------|
| 1 | 367 | MIKE_ADDED_DEAD | REMOVE | printf debug in wvolcado |
| 2 | 371 | MIKE_ADDED_DEAD | REMOVE | printf debug in wvolcado (ventana ptr) |
| 3 | 374 | MIKE_ADDED_DEAD | REMOVE | printf debug in wvolcado (vn) |
| 4 | 417 | MIKE_ADDED_DEAD | REMOVE | printf debug in wvolcado_oscuro |
| 5 | 421 | MIKE_ADDED_DEAD | REMOVE | printf debug in wvolcado_oscuro (ventana ptr) |
| 6 | 424 | MIKE_ADDED_DEAD | REMOVE | printf debug in wvolcado_oscuro (vn) |
| 7 | 443-451 | MIKE_REMOVED | REMOVE | Replaced memcpy loop in wvolcado_oscuro |
| 8 | 468-469 | MIKE_ADDED_DEAD | REMOVE | SDL_BlitSurface call in wvolcado_oscuro |
| 9 | 483 | ORIGINAL_DEAD | REMOVE | Old struct declaration in char_len |
| 10 | 491-495 | ORIGINAL_DEAD | REMOVE | Old struct declaration in text_len |
| 11 | 510 | MIKE_ADDED_DEAD | REMOVE | printf debug in text_len |
| 12 | 519-525 | ORIGINAL_DEAD | REMOVE | Old struct declaration in text_len2 |
| 13 | 547 | MIKE_ADDED_DEAD | REMOVE | printf debug in wwrite |
| 14 | 559 | MIKE_ADDED_DEAD | REMOVE | printf debug in wwrite_in_box |
| 15 | 563-568 | ORIGINAL_DEAD | REMOVE | Old struct declaration in wwrite_in_box |

**Totals:** 15 blocks found, 0 REVIEW, 15 REMOVE, 0 KEEP

## Block Details

### Block 1 (line 367) - MIKE_ADDED_DEAD - REMOVE
```c
//	printf("copia: %x %x\n",copia,p);
```
Debug printf not present in original. Mike added during SDL2 porting.

### Block 2 (lines 371) - MIKE_ADDED_DEAD - REMOVE
```c
	//		printf("copia: %x\n",ventana[vn].ptr);
```
Debug printf not present in original. Note: this comment breaks the if/if chain -- the `if(ventana[vn].ptr!=NULL)` on line 370 becomes a dangling statement because the printf that was its body is commented out. The second `if(ventana[vn].ptr==p)` on line 373 runs unconditionally. **This is a latent bug** -- the original had no such loop at all.

### Block 3 (lines 374) - MIKE_ADDED_DEAD - REMOVE
```c
//			printf("vn is: %d\n",vn);
```
Debug printf not present in original.

### Block 4 (line 417) - MIKE_ADDED_DEAD - REMOVE
```c
//	printf("copia: %x %x\n",copia,p);
```
Debug printf not present in original. Same pattern as block 1.

### Block 5 (line 421) - MIKE_ADDED_DEAD - REMOVE
```c
	//		printf("copia: %x\n",ventana[vn].ptr);
```
Debug printf not present in original. Same dangling-if bug as block 2.

### Block 6 (line 424) - MIKE_ADDED_DEAD - REMOVE
```c
//			printf("vn is: %d\n",vn);
```
Debug printf not present in original.

### Block 7 (lines 443-451) - MIKE_REMOVED - REMOVE
```c
/*do {
	  memcpy(q,p,an);
	  q+=an;
	  p+=an;
  q+=an_copia-(an=long_x);
  p+=resto_x;
} while (--long_y);

  */
```
This was an earlier version of the ghost-blending loop in `wvolcado_oscuro`. The original DOS code used the pixel-by-pixel ghost loop (which is the active code now). This commented block appears to be an intermediate attempt Mike tried and then replaced. The active code matches the original's logic. Safe to remove.

### Block 8 (lines 468-469) - MIKE_ADDED_DEAD - REMOVE
```c
//if(ventana[vn].surfaceptr!=NULL && vn<max_windows)
//	SDL_BlitSurface(ventana[vn].surfaceptr,NULL,copia_surface,&trc);
```
SDL surface blitting attempt not present in original. Dead SDL2 porting code.

### Block 9 (line 483) - ORIGINAL_DEAD - REMOVE
```c
//  struct { byte an; word dir; } * car;
```
Old inline struct declaration in `char_len`. The original DOS source had this as the active code (`struct { byte an; word dir; } * car;`). Mike replaced it with the `sscar` typedef (lines 478, 482). The commented-out line is the superseded version.

### Block 10 (lines 491-495) - ORIGINAL_DEAD - REMOVE
```c
	/*  struct {
	byte an;
	word dir;
	} * car;
	*/
```
Same pattern as block 9 but for `text_len`. Replaced by `sscar *car`.

### Block 11 (line 510) - MIKE_ADDED_DEAD - REMOVE
```c
//	printf("%d\n",an);
```
Debug printf not present in original.

### Block 12 (lines 519-525) - ORIGINAL_DEAD - REMOVE
```c
/*  struct {
    byte an;
    word dir;
  } * car;

*/
```
Same pattern as blocks 9-10 but for `text_len2`. Replaced by `sscar *car`.

### Block 13 (line 547) - MIKE_ADDED_DEAD - REMOVE
```c
//	printf("wwrite: %x %d %d %d %d %d %s %d\n",copia,an_copia, al_copia, x,y,centro, ptr, x);
```
Debug printf not present in original.

### Block 14 (line 559) - MIKE_ADDED_DEAD - REMOVE
```c
//	printf("Writing in box: %s\n",ptr);
```
Debug printf not present in original.

### Block 15 (lines 563-568) - ORIGINAL_DEAD - REMOVE
```c
	/*
	struct {
	byte an;
	word dir;
	} * car;
	*/
```
Same pattern as blocks 9-12 but for `wwrite_in_box`. Replaced by `sscar *car`.

## Bugs Found During Audit

**BUG (blocks 2 and 5):** In both `wvolcado` and `wvolcado_oscuro`, Mike added a `for` loop searching for `ventana[vn].ptr==p`, but the structure is:
```c
if(ventana[vn].ptr!=NULL)
//    printf(...)           <-- was the if's body
if(ventana[vn].ptr==p) {   <-- this always runs!
```
The original DOS code had neither the loop nor the variable `vn`. The `SDL_Rect trc` and loop are entirely Mike additions that serve no purpose (the `trc` rect is set but never used). The entire loop + trc block can be removed from both functions.
