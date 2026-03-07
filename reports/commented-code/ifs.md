# Commented-Out Code Audit: ifs.c

**Source:** `c:\Src\div\src\ifs.c` (1009 lines)
**Original:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\IFS.CPP` (1007 lines)
**Auditor:** Claude Opus 4.6
**Date:** 2026-03-07

## Summary

| # | Lines | Classification | Action | Description |
|---|-------|---------------|--------|-------------|
| 1 | 2 | MIKE_REMOVED | REMOVE | Commented-out `#include <conio.h>` |
| 2 | 22-56 | ORIGINAL_DEAD | REMOVE | Old OEM2ANSI lookup table |
| 3 | 76 | ORIGINAL_DEAD | REMOVE | Commented-out `fcloseall()` |
| 4 | 466-475 | ORIGINAL_DEAD | REMOVE | Old shadow position code in PintaSombra |
| 5 | 699-708 | ORIGINAL_DEAD | REMOVE | Old shadow position code in unirSombraConResto |
| 6 | 757-758 | ORIGINAL_DEAD | REMOVE | Old OEM2ANSI check in Jorge_Crea_el_font |
| 7 | 762-763 | ORIGINAL_DEAD | REMOVE | Old OEM2ANSI CargaLetra call |
| 8 | 785 | ORIGINAL_DEAD | REMOVE | Old incY calculation |
| 9 | 842 | ORIGINAL_DEAD | REMOVE | Old tablaFNT space width |
| 10 | 968 | MIKE_REMOVED | REMOVE | Old memcpy in ShowCharBuffer |

**Totals:** 10 blocks found, 0 REVIEW, 10 REMOVE, 0 KEEP

## Block Details

### Block 1 (line 2) - MIKE_REMOVED - REMOVE
```c
//#include <conio.h>
```
The original had `#include <conio.h>`. Mike commented it out since `conio.h` is a DOS/Windows-specific header not needed in the SDL2 build (no console I/O used in this file).

### Block 2 (lines 22-56) - ORIGINAL_DEAD - REMOVE
```c
/*
char OEM2ANSI[256]={     // Tabla antigua
//Diferencias
                 0,  0,  0,  0, ...
...
                 0,  0,  0,178,  0,  0};
//Diferencias
*/
```
Old version of the OEM-to-ANSI character conversion table. Already commented out in the original DOS source (labeled "Tabla antigua" -- "old table"). The active table on lines 12-20 is the replacement. Both files are identical here.

### Block 3 (line 76) - ORIGINAL_DEAD - REMOVE
```c
//    fcloseall();
```
In `CloseAndFreeAll`. Already commented out in the original DOS source. Replaced by individual `fclose` calls below. The original used `fcloseall()` (a Watcom C runtime function) which was then replaced with explicit close calls for `fichIFS` and `fichFNT`.

### Block 4 (lines 466-475) - ORIGINAL_DEAD - REMOVE
```c
/*
    if (ifs.sombraX < 0 && ifs.sombraY < 0)
        ptr2=shadowBuffer;
    else if (ifs.sombraX > 0 && ifs.sombraY > 0)
            ptr2=shadowBuffer + absSombraX + absSombraY * (anchoreal+absSombraX);
    else if (ifs.sombraX > 0)
            ptr2=shadowBuffer + absSombraX;
    else
        ptr2=shadowBuffer + absSombraY*(anchoreal+absSombraX);
*/
```
In `PintaSombra`. Old shadow position calculation. Already commented out in the original DOS source and replaced by the simpler if/else chain on lines 461-464. Both files match exactly.

### Block 5 (lines 699-708) - ORIGINAL_DEAD - REMOVE
```c
/*
    if (ifs.sombraX < 0 && ifs.sombraY < 0)
        ptr2=shadowBuffer + absSombraX + absSombraY*anchoreal;
    else if (ifs.sombraX > 0 && ifs.sombraY > 0)
        ptr2=shadowBuffer;
    else if (ifs.sombraX > 0)
        ptr2=shadowBuffer + absSombraY*anchoreal;
    else
        ptr2=shadowBuffer + absSombraX;
*/
```
In `unirSombraConResto`. Old shadow union position calculation. Already commented out in the original DOS source. Same pattern as block 4 -- replaced by the simpler logic on lines 694-697. Both files match exactly.

### Block 6 (lines 757-758) - ORIGINAL_DEAD - REMOVE
```c
//            if (OEM2ANSI[x]!=0)
            if (x!=0)
```
In `Jorge_Crea_el_font`. The old check `if (OEM2ANSI[x]!=0)` was replaced by `if (x!=0)`. Already commented out in the original DOS source, with the replacement active on line 760.

### Block 7 (lines 762-763) - ORIGINAL_DEAD - REMOVE
```c
//             if ((ret=CargaLetra(OEM2ANSI[x])))
             if ((ret=CargaLetra(x)))
```
Same function. The old call through OEM2ANSI was replaced with direct `CargaLetra(x)`. Already commented out in the original DOS source.

### Block 8 (line 785) - ORIGINAL_DEAD - REMOVE
```c
//                incY=altoBody*despY/Alto;
```
Old incY calculation formula. Already commented out in the original DOS source, replaced by the scan-based loop above it (lines 781-784).

### Block 9 (line 842) - ORIGINAL_DEAD - REMOVE
```c
//    tablaFNT[' '].ancho=ifs.tamX;
```
Old code to force space character width. Already commented out in the original DOS source.

### Block 10 (line 968) - MIKE_REMOVED - REMOVE
```c
//                memcpy(ptr+((cy+iy)*an+cx)+y*an,rawBuffer+y*tablaFNT[WhatChar].ancho,tablaFNT[WhatChar].ancho);
```
In `ShowCharBuffer`. The original DOS source had this `memcpy` as the active code. Mike replaced it with a pixel-by-pixel loop (lines 963-966) that skips color 0 (transparency). The commented line is the old non-transparent version.

## Notes

This file is remarkably clean. Almost all commented-out code was already dead in the original DOS source (Daniel's own cleanup of the OEM2ANSI conversion logic). Only two changes are Mike's: the `#include <conio.h>` removal and the `memcpy` replacement in `ShowCharBuffer`.

The `ShowCharBuffer` change (block 10) is a legitimate bugfix -- the old `memcpy` would overwrite transparent pixels, while the new loop preserves them. The commented-out line can be safely removed.
